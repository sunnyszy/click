/*
 idadder program, add id and push for queue ring in ap processing. 
 Input: [data pkt]
 Output:[eth][id][data pkt], id is one Byte
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
// -*- c-basic-offset: 4 -*-
#ifndef CLICK_WGTTQUEUE_HH
#define CLICK_WGTTQUEUE_HH
#include <click/element.hh>
#include <click/standard/storage.hh>
#include <clicknet/ether.h>
#include <click/confparse.hh>
#include <clicknet/wgtt.h>

CLICK_DECLS

class WGTTQueue : public Element, public Storage { public:

    WGTTQueue() CLICK_COLD;

    inline void enRing(unsigned char, Packet*);//flag: whether override
    inline Packet* deRing();
    inline void enque(Packet*);//flag: whether override
    inline Packet* deque();

    const char *class_name() const		{ return "WGTTQueue"; }
    const char *port_count() const		{ return "1/2"; }
    const char *processing() const		{ return "h/lh"; }

    int configure(Vector<String>&, ErrorHandler*) CLICK_COLD;
    int initialize(ErrorHandler*) CLICK_COLD;

    void push(int port, Packet*);
    void push_control(Packet *p_in);
    void push_data(Packet *p_in);
    Packet* pull(int port);

  protected:

    Packet** volatile * _q;

    volatile unsigned char _head[MAX_N_CLIENT];
    volatile unsigned char _tail[MAX_N_CLIENT];
    volatile bool _block[MAX_N_CLIENT];
    volatile unsigned char next_client;

    int identity;
    int first_start[MAX_N_CLIENT];
    unsigned char control_content[2];

    click_ether * _ethh;
    
};

inline void
WGTTQueue::enRing(unsigned char c, Packet *p)
{
    if((_tail[c]+1)%RING_SIZE == _head[c])//override
    {
        // printf("WGTTQueue override\n");
        if(_q[c][_head[c]])
            _q[c][_head[c]] -> kill();
        _head[c] = (_head[c]+1)%RING_SIZE;
    }
    // printf("WGTTQueue before _q[_tail] = p\n");
    // Packet *tmp = _q[_tail];
    // printf("_tail: %x\n", _tail);
    _q[c][_tail[c]] = p;
    // printf("WGTTQueue finish _q[_tail] = p\n");
    _tail[c] = (_tail[c]+1)%RING_SIZE;
    // printf("WGTTQueue finish enRing\n");
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
            //     printf("wgttQueue: queue %d is inactive\n", next_client+1);
            // if(_head[next_client]==_tail[next_client])
            //     printf("wgttQueue: queue %d is empty\n", next_client+1);
            continue;
        }
        while((_head[next_client]+1)%MAX_N_CLIENT != _tail[next_client]
             && !_head[next_client])
            _head[next_client] = (_head[next_client]+1)%RING_SIZE;
        flag = true;
        p = _q[next_client][_head[next_client]];
        _head[next_client] = (_head[next_client]+1)%RING_SIZE;
        // printf("wgttQueue: deque pkt from queue: %d\n", next_client+1);
        break;
    }

    next_client = next_client_after;

    if(flag)
    {
        // printf("wgttQueue: deque succeed\n");
        return p;
    }
    else
        return 0;
}



CLICK_ENDDECLS
#endif
