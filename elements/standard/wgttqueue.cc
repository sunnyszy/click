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
    _ethh = new click_ether[MAX_N_AP+1];
    next_client = 0;
    printf("wgtt init succeed\n");
}



int
WGTTQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{
    //printf("In configure\n");
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
    
    printf("wgtt configure succeed\n");
    return 0;
}

int
WGTTQueue::initialize(ErrorHandler *errh)
{
    //printf("wgtt in initialize\n");
    int i;
    for(i=0; i<MAX_N_CLIENT; i++)
        _block[i] = (identity!=first_start[i]);

    for(i=0; i < MAX_N_AP+1; i++)
    {
        _ethh[i].ether_type = htons(ETHER_PROTO_BASE+CONTROL_SUFFIX);
        switch(identity-1)
        {
            case 0:cp_ethernet_address(AP1_MAC, _ethh->ether_shost);break;
            case 1:cp_ethernet_address(AP2_MAC, _ethh->ether_shost);break;
            case 2:cp_ethernet_address(AP3_MAC, _ethh->ether_shost);break;
            case 3:cp_ethernet_address(AP4_MAC, _ethh->ether_shost);break;
            case 4:cp_ethernet_address(AP5_MAC, _ethh->ether_shost);break;
            case 5:cp_ethernet_address(AP6_MAC, _ethh->ether_shost);break;
            case 6:cp_ethernet_address(AP7_MAC, _ethh->ether_shost);break;
            case 7:cp_ethernet_address(AP8_MAC, _ethh->ether_shost);break;
        }
        switch(i)
        {
            case 0:cp_ethernet_address(AP1_MAC, _ethh->ether_shost);break;
            case 1:cp_ethernet_address(AP2_MAC, _ethh->ether_shost);break;
            case 2:cp_ethernet_address(AP3_MAC, _ethh->ether_shost);break;
            case 3:cp_ethernet_address(AP4_MAC, _ethh->ether_shost);break;
            case 4:cp_ethernet_address(AP5_MAC, _ethh->ether_shost);break;
            case 5:cp_ethernet_address(AP6_MAC, _ethh->ether_shost);break;
            case 6:cp_ethernet_address(AP7_MAC, _ethh->ether_shost);break;
            case 7:cp_ethernet_address(AP8_MAC, _ethh->ether_shost);break;
        }
    }

    if (_q == 0)
        return errh->error("out of memory");
    printf("wgtt initialize succeed, ready to start\n");
    return 0;
}



void
WGTTQueue::push(int, Packet *p_in)
{
    // printf("wgttQueue in push\n");
    switch(pkt_type(p_in))
    {
    case CONTROL_SUFFIX:  push_control(p_in);break;
    case DATA_SUFFIX:   push_data(p_in);break;
    }
}

void WGTTQueue::push_control(Packet *p_in)
{
    int i,j;
    unsigned char c = client_ip(p_in)- CLIENT1_IP_SUFFIX; //index for client
    if(status_ap(p_in) == CONTROLLER_IN_MAC_SUFFIX) //from controller
    {
        if(client_ip(p_in) == RESET_CONTENT) //reset
        {
            printf("wgttQueue: receive reset req for client: %d\n", c+1);
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
            printf("wgttQueue: receive switch req for client: %d\n", c+1);
            _block[c] = true;
            const unsigned char & dst_ap = start_ap(p_in);
            WritablePacket *p = Packet::make(sizeof(click_ether)+2);
            // // data part
            control_content[0] = client_ip(p_in);
            control_content[1] = _head[c];
            printf("wgttQueue: switch to ap: %d\n", dst_ap+1);
            printf("wgttQueue: switch id: %X\n", _head[c]);
            memcpy(p->data()+sizeof(click_ether), &control_content, 2);
            memcpy(p->data(), &(_ethh[dst_ap]), sizeof(click_ether));

            p_in -> kill();
            printf("wgttQueue send ap-ap seq\n");
            checked_output_push(1, p);
        }
        
    }
    else //from ap
    {
        printf("wgttQueue receive ap-ap seq for client: %d\n", c+1);

        const unsigned char & start_seq = start_seq(p_in);
        while(_head[c] != start_seq)
        {   
            if(_q[c][_head[c]] != 0)
                _q[c][_head[c]] -> kill();
            _head[c] = (_head[c]+1)%RING_SIZE;
        }
        // printf("wgttQueue finish ap-ap dequeue\n");
        

        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = client_ip(p_in);
        control_content[1] = 0;
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        
        //ether part
        memcpy(p->data(), &(_ethh[MAX_N_AP]), sizeof(click_ether));

        p_in -> kill();
        // printf("ap-c packet push\n");
        _block[c] = false;
        printf("wgttQueue send switch ack\n");
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
    // printf("wgttQueue in push data for client: %d\n", c+1);
    while(_tail[c] != seq)
    {
        // printf("wgttQueue: before for client: %d\n", c+1);
        enRing(c, 0);
    }
    p_in -> pull(15);
    // printf("wgttQueue after enring, _head: %X, _tail: %X\n", _head[c], _tail[c]);
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
