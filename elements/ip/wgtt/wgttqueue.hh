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
#include <syslog.h>
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





CLICK_ENDDECLS
#endif