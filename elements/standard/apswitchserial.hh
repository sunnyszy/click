// -*- c-basic-offset: 4 -*-
#ifndef CLICK_APSWITCHSERIAL_HH
#define CLICK_APSWITCHSERIAL_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <click/confparse.hh>
CLICK_DECLS

#define RING_SIZE 256
#define QUEUE_SIZE 256
#define STATUS 4
#define DATA 17
#define CONTROL 6
#define IDLE 0
#define SWITCH_REQ 1
#define CLIENT1 135
#define CONTROLLER 1
#define AP1 2
#define AP2 4
#define IP_BASE 0x01a8c0
#define AP1_IP 0x0201a8c0
#define AP2_IP 0x0401a8c0
#define CONTROLLER_IP 0x0101a8c0
#define AP1_MAC "C0:56:27:72:A3:5B"
#define AP2_MAC "60:38:E0:03:FA:0B"
#define CONTROLLER_MAC "38:c9:86:40:c8:05"

#define pkt_type(p) *(p->data()+9)
#define ip_id(p) *(p->data()+20)
#define ap_id(p) *(p->data()+15)
#define start_ap(p) *(p->data()+21)
#define start_seq(p) *(p->data()+21)
#define ap_score(p) *(p->data()+20)


class APSwitchSerial : public Element { public:

    APSwitchSerial() CLICK_COLD;
    ~APSwitchSerial() CLICK_COLD;

    inline void enRing(Packet*);//flag: whether override
    inline Packet* deRing();
    inline void enque(Packet*);//flag: whether override
    inline Packet* deque();

    const char *class_name() const		{ return "APSwitchSerial"; }
    const char *port_count() const		{ return "1/2"; }
    const char *processing() const		{ return "h/lh"; }

    int configure(Vector<String>&, ErrorHandler*) CLICK_COLD;

    void push(int port, Packet*);
    void push_control(Packet *p_in);
    void push_data(Packet *p_in);

    Packet* pull(int port);

  private:

    Packet* volatile * _q;
    Packet* volatile * _qControl;
    volatile unsigned char _head;
    volatile unsigned char _tail;

    volatile unsigned char _headControl;
    volatile unsigned char _tailControl;

    volatile bool _block;
    unsigned char identity;
    unsigned char control_content[2];

    click_ip _iph;
    click_ether * _ethh;

};


inline void
APSwitchSerial::enRing(Packet *p)
{
    if((_tail+1)%RING_SIZE == _head)//override
    {
        _q[_head] -> kill();
        _head = (_head+1)%RING_SIZE;
    }
    _q[_tail] = p;
    _tail = (_tail+1)%RING_SIZE;
}

inline Packet *
APSwitchSerial::deRing()
{
    if(_block || _head==_tail)
        return 0;
    Packet *p = _q[_head];
    _head = (_head+1)%RING_SIZE;
	return p;
}

inline void
APSwitchSerial::enque(Packet *p)
{
    if((_tailControl+1)%QUEUE_SIZE == _headControl)//override
    {
        _qControl[_headControl] -> kill();
        _headControl = (_headControl+1)%QUEUE_SIZE;
    }
    _qControl[_tailControl] = p;
    _tailControl = (_tailControl+1)%QUEUE_SIZE;
}

inline Packet *
APSwitchSerial::deque()
{
    if(_headControl==_tailControl)
        return 0;
    Packet *p = _qControl[_headControl];
    _headControl = (_headControl+1)%QUEUE_SIZE;
    return p;
}

CLICK_ENDDECLS
#endif
