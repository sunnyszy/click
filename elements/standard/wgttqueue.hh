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

    inline void enRing(Packet*);//flag: whether override
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

    Packet* volatile * _q;

    volatile unsigned char _head;
    volatile unsigned char _tail;

    volatile bool _block;
    unsigned char identity;
    unsigned char control_content[2];

    click_ether * _ethh;
    
};

inline void
WGTTQueue::enRing(Packet *p)
{
    if((_tail+1)%RING_SIZE == _head)//override
    {
        // printf("WGTTQueue override\n");
        if(_q[_head] != 0)
            _q[_head] -> kill();
        _head = (_head+1)%RING_SIZE;
    }
    // printf("WGTTQueue before _q[_tail] = p\n");
    // Packet *tmp = _q[_tail];
    // printf("_tail: %x\n", _tail);
    _q[_tail] = p;
    // printf("WGTTQueue finish _q[_tail] = p\n");
    _tail = (_tail+1)%RING_SIZE;
    // printf("WGTTQueue finish enRing\n");
}

inline Packet *
WGTTQueue::deRing()
{
    if(_block || _head==_tail)
        return 0;
    Packet *p = _q[_head];
    _head = (_head+1)%RING_SIZE;
    // if(p != 0)
    // {
    //     printf("wgttQueue in pull\n");
    //     printf("wgttQueue _head: %X, _tail: %X\n", _head, _tail);
    // }
    return p;
}



CLICK_ENDDECLS
#endif
