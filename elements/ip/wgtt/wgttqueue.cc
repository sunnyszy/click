/*
 A queue store data pkt for each client. Cache the pkt in a ring buffer
 Input: data pkt, control pkt
 Output: data pkt, control pkt
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "wgttqueue.hh"
#include <click/args.hh>
#include <click/error.hh>
//WGTT: ioctl
//#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
CLICK_DECLS

// magic number
#define MEMDEV_IOC_MAGIC  'k'

// command
#define MEMDEV_IOCGETDATA _IOR(MEMDEV_IOC_MAGIC, 0, int)
#define MEMDEV_IOCSETDATA _IOW(MEMDEV_IOC_MAGIC, 1, int)
int fd = 0;
int cmd;
int arg = 0;

WGTTQueue::WGTTQueue()
{
    int i;
    openlog("APControl_WGTTQueue", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);
    _q = new Packet **[MAX_N_CLIENT];
    for(i=0; i<MAX_N_CLIENT; i++)
    {
        _q[i] = new Packet *[RING_SIZE];
        _head[i] = 0;
        _tail[i] = 0;
    }
    // 0: controller, 1-MAX_N_AP: ap
    _ethh = new click_ether[MAX_N_AP+1];
    next_client = 0;
    syslog (LOG_DEBUG, "init succeed\n");
}

WGTTQueue::~WGTTQueue()
{
    if(identity == 4)
    {
     close(fd);
    }
}


int
WGTTQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{
    int tmp[4], i;
    syslog (LOG_DEBUG, "enter configure\n");
    if (Args(conf, this, errh)
        .read_p("IDENTITY", IntArg(), identity)
        .read_p("FIRSTSTART1", IntArg(), tmp[0])
        .read_p("FIRSTSTART2", IntArg(), tmp[1])
        .read_p("FIRSTSTART3", IntArg(), tmp[2])
        .read_p("FIRSTSTART4", IntArg(), tmp[3])
        .complete() < 0)
    {
        syslog (LOG_DEBUG, "parse configure fail\n");
        return -1;
    }
    
    for(i=0;i<MAX_N_CLIENT;i++)
    {
        first_start[i] = tmp[i];
    }
    
    if(identity == 4)
    {
    /*打开设备文件*/
    fd = open("/dev/memdev0",O_RDWR);
    if (fd < 0)
    {
        printf("Open Dev Mem0 Error!\n");
        return -1;
    }
    
    /* 调用命令MEMDEV_IOCSETDATA */
    printf("<--- Call MEMDEV_IOCSETDATA --->\n");
    cmd = MEMDEV_IOCSETDATA;
    arg = 4;
    if (ioctl(fd, cmd, &arg) < 0)
        {
            printf("Call cmd MEMDEV_IOCSETDATA fail\n");
            return -1;
    }
    }

    syslog (LOG_DEBUG, "configure succeed\n");
    return 0;
}

int
WGTTQueue::initialize(ErrorHandler *errh)
{
    int i;
    for(i=0; i<MAX_N_CLIENT; i++)
    {
        _block[i] = (identity!=first_start[i]);
        syslog (LOG_DEBUG, "block[%d] is %d.\n", i, _block[i]);
        syslog (LOG_DEBUG, "identity is %d, first_start is%d.\n", identity, first_start[i]);
    }

    for(i=0; i < MAX_N_AP+1; i++)
    {
        _ethh[i].ether_type = htons(ETHER_PROTO_BASE+CONTROL_SUFFIX);
        switch(identity-1)
        {
            case 0:cp_ethernet_address(AP1_MAC, _ethh[i].ether_shost);break;
            case 1:cp_ethernet_address(AP2_MAC, _ethh[i].ether_shost);break;
            case 2:cp_ethernet_address(AP3_MAC, _ethh[i].ether_shost);break;
            case 3:cp_ethernet_address(AP4_MAC, _ethh[i].ether_shost);break;
            case 4:cp_ethernet_address(AP5_MAC, _ethh[i].ether_shost);break;
            case 5:cp_ethernet_address(AP6_MAC, _ethh[i].ether_shost);break;
            case 6:cp_ethernet_address(AP7_MAC, _ethh[i].ether_shost);break;
            case 7:cp_ethernet_address(AP8_MAC, _ethh[i].ether_shost);break;
        }
        switch(i)
        {
            case 0:cp_ethernet_address(CONTROLLER_IN_MAC, _ethh[i].ether_dhost);break;
            case 1:cp_ethernet_address(AP1_MAC, _ethh[i].ether_dhost);break;
            case 2:cp_ethernet_address(AP2_MAC, _ethh[i].ether_dhost);break;
            case 3:cp_ethernet_address(AP3_MAC, _ethh[i].ether_dhost);break;
            case 4:cp_ethernet_address(AP4_MAC, _ethh[i].ether_dhost);break;
            case 5:cp_ethernet_address(AP5_MAC, _ethh[i].ether_dhost);break;
            case 6:cp_ethernet_address(AP6_MAC, _ethh[i].ether_dhost);break;
            case 7:cp_ethernet_address(AP7_MAC, _ethh[i].ether_dhost);break;
            case 8:cp_ethernet_address(AP8_MAC, _ethh[i].ether_dhost);break;
            
        }
    }

    if (_q == 0)
        return errh->error("out of memory");
    syslog (LOG_DEBUG, "initialize succeed, ready to start\n");
    return 0;
}


inline void
WGTTQueue::enRing(unsigned char c, Packet *p)
{
    if((_tail[c]+1)%RING_SIZE == _head[c])//override
    {
        // syslog (LOG_DEBUG, "WGTTQueue override\n");
        if(_q[c][_head[c]])
            _q[c][_head[c]] -> kill();
        _head[c] = (_head[c]+1)%RING_SIZE;
    }
    // syslog (LOG_DEBUG, "WGTTQueue before _q[_tail] = p\n");
    // Packet *tmp = _q[_tail];
    // syslog (LOG_DEBUG, "_tail: %x\n", _tail);
    _q[c][_tail[c]] = p;
    // syslog (LOG_DEBUG, "WGTTQueue finish _q[_tail] = p\n");
    _tail[c] = (_tail[c]+1)%RING_SIZE;
    // syslog (LOG_DEBUG, "WGTTQueue finish enRing\n");
}

inline Packet *
WGTTQueue::deRing()
{
    int i;
    bool flag = false;//no pick out
    Packet *p;
    //next_client after function
    unsigned char next_client_after = (next_client+1)%MAX_N_CLIENT; 
    for(i=0; i<MAX_N_CLIENT; i++, next_client = (next_client+1)%MAX_N_CLIENT)
    {
        if(_block[next_client] || _head[next_client]==_tail[next_client])
        {
            // if(_block[next_client])
            //     syslog (LOG_DEBUG, "wgttQueue: queue %d is inactive\n", next_client+1);
            // if(_head[next_client]==_tail[next_client])
            //     syslog (LOG_DEBUG, "wgttQueue: queue %d is empty\n", next_client+1);
            continue;
        }
        while((_head[next_client]+1)%MAX_N_CLIENT != _tail[next_client]
             && !_head[next_client])
            _head[next_client] = (_head[next_client]+1)%RING_SIZE;
        flag = true;
        p = _q[next_client][_head[next_client]];
        _head[next_client] = (_head[next_client]+1)%RING_SIZE;
        // syslog (LOG_DEBUG, "deque pkt from queue: %d\n", next_client+1);
        break;
    }

    next_client = next_client_after;

    if(flag && p)
    {
        // syslog (LOG_DEBUG, "wgttQueue: deque succeed\n");
        // WGTT: dont forget this is for debug
        // this is for debug
        
        if(identity == 5)
        {
            static unsigned int tmp_counter = 0;
            tmp_counter ++;
            if(tmp_counter >= 10)
            {
                _block[0] == true;
                syslog(LOG_DEBUG, "wgttQueue: AP5 disable\n");
                return 0;
            }
        }

        return p;
    }
    else
        return 0;
}


void
WGTTQueue::push(int, Packet *p_in)
{
    // syslog (LOG_DEBUG, "in push\n");
    switch(pkt_type(p_in))
    {
    case CONTROL_SUFFIX:  push_control(p_in);break;
    case DATA_SUFFIX:   push_data(p_in);break;
    }
    // syslog (LOG_DEBUG, "out push\n");
}

void WGTTQueue::push_control(Packet *p_in)
{
    int i,j;
    unsigned char c = client_ip(p_in)- CLIENT1_IP_SUFFIX; //index for client
    syslog (LOG_DEBUG, "receive control msg\n");

    struct timeval ts;
    

    if(status_ap(p_in) == CONTROLLER_IN_MAC_SUFFIX) //from controller
    {
        if(client_ip(p_in) == RESET_CONTENT) //reset
        {
            syslog (LOG_DEBUG, "receive reset req for client: %d\n", c+1);
            for(i=0; i<MAX_N_CLIENT;i++)
            {
                _tail[i] = 0;
                _head[i] = 0;   
                _block[i] = (identity != first_start[i]);
                for(j=0;j<256;j++)
                {   
                    if(_q[i][j] != 0)
                        _q[i][j] -> kill();
                }
            }
        }
        else
        {

            gettimeofday(&ts, NULL); 
            syslog (LOG_DEBUG, "click receive cont-ap control at: %lld.%.6ld\n", (long long)ts.tv_sec, ts.tv_usec);
            _block[c] = true;

            if(identity == 4)
            {  
            gettimeofday(&ts, NULL); 
            syslog (LOG_DEBUG, "click tx to kernel at: %lld.%.6ld\n", (long long)ts.tv_sec, ts.tv_usec);
            cmd = MEMDEV_IOCSETDATA;
            arg = 0;
            if (ioctl(fd, cmd, &arg) < 0)
                {
                    // printf("Call cmd MEMDEV_IOCSETDATA fail\n");
                    return;
            }

            
            // printf("<--- Call MEMDEV_IOCGETDATA --->\n");
            cmd = MEMDEV_IOCGETDATA;
            if (ioctl(fd, cmd, &arg) < 0)
                {
                    // printf("Call cmd MEMDEV_IOCGETDATA fail\n");
                    return;
            }
            // printf("<--- In User Space MEMDEV_IOCGETDATA Get Data is %d --->\n\n",arg);    
            gettimeofday(&ts, NULL); 
            syslog (LOG_DEBUG, "click rx from kernel at: %lld.%.6ld\n", (long long)ts.tv_sec, ts.tv_usec);
            
            }

            const unsigned char & dst_ap = start_ap(p_in);
            WritablePacket *p = Packet::make(sizeof(click_ether)+2);
            // // data part
            control_content[0] = client_ip(p_in);
            control_content[1] = _head[c];
            syslog (LOG_DEBUG, "switch to ap: %d\n", dst_ap+1);
            // syslog (LOG_DEBUG, "switch id: %X\n", _head[c]);
            memcpy(p->data()+sizeof(click_ether), &control_content, 2);
            memcpy(p->data(), &(_ethh[dst_ap+1]), sizeof(click_ether));

            p_in -> kill();
            // syslog (LOG_DEBUG, "send ap-ap seq\n");
            gettimeofday(&ts, NULL); 
            syslog (LOG_DEBUG, "click tx to ap at: %lld.%.6ld\n", (long long)ts.tv_sec, ts.tv_usec);
            checked_output_push(1, p);
        }
        
    }
    else //from ap
    {
        gettimeofday(&ts, NULL); 
        syslog (LOG_DEBUG, "click rx from ap at: %lld.%.6ld\n", (long long)ts.tv_sec, ts.tv_usec);
            

        const unsigned char & start_seq = start_seq(p_in);
        while(_head[c] != start_seq)
        {   
            if(_q[c][_head[c]] != 0)
                _q[c][_head[c]] -> kill();
            _head[c] = (_head[c]+1)%RING_SIZE;
        }
        // syslog (LOG_DEBUG, "wgttQueue finish ap-ap dequeue\n");
        

        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = client_ip(p_in);
        control_content[1] = 0;
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        
        //ether part
        memcpy(p->data(), &(_ethh[0]), sizeof(click_ether));

        p_in -> kill();
        // syslog (LOG_DEBUG, "ap-c packet push\n");
        _block[c] = false;
        gettimeofday(&ts, NULL); 
        syslog (LOG_DEBUG, "click tx to controller at: %lld.%.6ld\n", (long long)ts.tv_sec, ts.tv_usec);
        checked_output_push(1, p);
    }
}

void WGTTQueue::push_data(Packet *p_in)
{
    const unsigned char & seq = queue_seq(p_in);
    unsigned char c;
    switch(data_client(p_in))
    {
        case CLIENT1_MAC_SUFFIX: c=0;break;
        case CLIENT2_MAC_SUFFIX: c=1;break;
        case CLIENT3_MAC_SUFFIX: c=2;break;
        case CLIENT4_MAC_SUFFIX: c=3;break;
    }
    // if(_tail[c])
    //     syslog (LOG_DEBUG, "wgttQueue in push data for active client: %d\n", c+1);
    // else
    //     syslog (LOG_DEBUG, "wgttQueue in push data for inactive client: %d\n", c+1);
    while(_tail[c] != seq)
    {
        // syslog (LOG_DEBUG, "wgttQueue: before for client: %d\n", c+1);
        enRing(c, 0);
    }
    p_in -> pull(15);
    // syslog (LOG_DEBUG, "after enring, _head: %X, _tail: %X\n", _head[c], _tail[c]);
    enRing(c, p_in);
}

Packet *
WGTTQueue::pull(int port)
{
    return deRing();
}




CLICK_ENDDECLS
ELEMENT_PROVIDES(Storage)
EXPORT_ELEMENT(WGTTQueue)
