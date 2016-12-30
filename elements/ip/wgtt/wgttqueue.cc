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
CLICK_DECLS

WGTTQueue::WGTTQueue()
{
    int i;
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
    syslog (LOG_INFO, "wgtt init succeed\n");
}



int
WGTTQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{
    //syslog (LOG_INFO, "In configure\n");
    int tmp[4], i;
    if (Args(conf, this, errh)
        .read_p("IDENTITY", IntArg(), identity)
        .read_p("FIRSTSTART1", IntArg(), tmp[0])
        .read_p("FIRSTSTART2", IntArg(), tmp[1])
        .read_p("FIRSTSTART3", IntArg(), tmp[2])
        .read_p("FIRSTSTART4", IntArg(), tmp[3])
        .complete() < 0)
    return -1;
    for(i=0;i<MAX_N_CLIENT;i++)
    {
        first_start[i] = tmp[i];
    }
    
    syslog (LOG_INFO, "wgtt configure succeed\n");
    return 0;
}

int
WGTTQueue::initialize(ErrorHandler *errh)
{
    syslog (LOG_INFO, "wgtt in initialize\n");
    int i;
    for(i=0; i<MAX_N_CLIENT; i++)
    {
        _block[i] = (identity!=first_start[i]);
        syslog (LOG_INFO, "wgttqueue: block[%d] is %d.\n", i, _block[i]);
        syslog (LOG_INFO, "wgttqueue: identity is %d, first_start is%d.\n", identity, first_start[i]);
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
    syslog (LOG_INFO, "wgtt initialize succeed, ready to start\n");
    return 0;
}


inline void
WGTTQueue::enRing(unsigned char c, Packet *p)
{
    if((_tail[c]+1)%RING_SIZE == _head[c])//override
    {
        // syslog (LOG_INFO, "WGTTQueue override\n");
        if(_q[c][_head[c]])
            _q[c][_head[c]] -> kill();
        _head[c] = (_head[c]+1)%RING_SIZE;
    }
    // syslog (LOG_INFO, "WGTTQueue before _q[_tail] = p\n");
    // Packet *tmp = _q[_tail];
    // syslog (LOG_INFO, "_tail: %x\n", _tail);
    _q[c][_tail[c]] = p;
    // syslog (LOG_INFO, "WGTTQueue finish _q[_tail] = p\n");
    _tail[c] = (_tail[c]+1)%RING_SIZE;
    // syslog (LOG_INFO, "WGTTQueue finish enRing\n");
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
            //     syslog (LOG_INFO, "wgttQueue: queue %d is inactive\n", next_client+1);
            // if(_head[next_client]==_tail[next_client])
            //     syslog (LOG_INFO, "wgttQueue: queue %d is empty\n", next_client+1);
            continue;
        }
        while((_head[next_client]+1)%MAX_N_CLIENT != _tail[next_client]
             && !_head[next_client])
            _head[next_client] = (_head[next_client]+1)%RING_SIZE;
        flag = true;
        p = _q[next_client][_head[next_client]];
        _head[next_client] = (_head[next_client]+1)%RING_SIZE;
        syslog (LOG_INFO, "wgttQueue: deque pkt from queue: %d\n", next_client+1);
        break;
    }

    next_client = next_client_after;

    if(flag)
    {
        // syslog (LOG_INFO, "wgttQueue: deque succeed\n");
        return p;
    }
    else
        return 0;
}


void
WGTTQueue::push(int, Packet *p_in)
{
    syslog (LOG_INFO, "wgttQueue in push\n");
    switch(pkt_type(p_in))
    {
    case CONTROL_SUFFIX:  push_control(p_in);break;
    case DATA_SUFFIX:   push_data(p_in);break;
    }
    syslog (LOG_INFO, "wgttQueue out push\n");
}

void WGTTQueue::push_control(Packet *p_in)
{
    int i,j;
    unsigned char c = client_ip(p_in)- CLIENT1_IP_SUFFIX; //index for client
    if(status_ap(p_in) == CONTROLLER_IN_MAC_SUFFIX) //from controller
    {
        if(client_ip(p_in) == RESET_CONTENT) //reset
        {
            syslog (LOG_INFO, "wgttQueue: receive reset req for client: %d\n", c+1);
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
            syslog (LOG_INFO, "wgttQueue: receive switch req for client: %d\n", c+1);
            _block[c] = true;
            const unsigned char & dst_ap = start_ap(p_in);
            WritablePacket *p = Packet::make(sizeof(click_ether)+2);
            // // data part
            control_content[0] = client_ip(p_in);
            control_content[1] = _head[c];
            syslog (LOG_INFO, "wgttQueue: switch to ap: %d\n", dst_ap+1);
            syslog (LOG_INFO, "wgttQueue: switch id: %X\n", _head[c]);
            memcpy(p->data()+sizeof(click_ether), &control_content, 2);
            memcpy(p->data(), &(_ethh[dst_ap+1]), sizeof(click_ether));

            p_in -> kill();
            syslog (LOG_INFO, "wgttQueue send ap-ap seq\n");
            checked_output_push(1, p);
        }
        
    }
    else //from ap
    {
        syslog (LOG_INFO, "wgttQueue receive ap-ap seq for client: %d\n", c+1);

        const unsigned char & start_seq = start_seq(p_in);
        while(_head[c] != start_seq)
        {   
            if(_q[c][_head[c]] != 0)
                _q[c][_head[c]] -> kill();
            _head[c] = (_head[c]+1)%RING_SIZE;
        }
        // syslog (LOG_INFO, "wgttQueue finish ap-ap dequeue\n");
        

        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = client_ip(p_in);
        control_content[1] = 0;
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        
        //ether part
        memcpy(p->data(), &(_ethh[0]), sizeof(click_ether));

        p_in -> kill();
        // syslog (LOG_INFO, "ap-c packet push\n");
        _block[c] = false;
        syslog (LOG_INFO, "wgttQueue send switch ack\n");
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
    //     syslog (LOG_INFO, "wgttQueue in push data for active client: %d\n", c+1);
    // else
    //     syslog (LOG_INFO, "wgttQueue in push data for inactive client: %d\n", c+1);
    while(_tail[c] != seq)
    {
        // syslog (LOG_INFO, "wgttQueue: before for client: %d\n", c+1);
        enRing(c, 0);
    }
    p_in -> pull(15);
    syslog (LOG_INFO, "wgttQueue after enring, _head: %X, _tail: %X\n", _head[c], _tail[c]);
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
