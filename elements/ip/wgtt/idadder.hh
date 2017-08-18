/*
 idadder program, add id and push for queue ring in ap processing. 
 Input: [data pkt]
 Output:[eth][id][data pkt], id is one Byte
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#ifndef CLICK_IDADDER_HH
#define CLICK_IDADDER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/ether.h>
#include <clicknet/wgtt.h>
#include <syslog.h>
CLICK_DECLS


class IDAdder : public Element { public:


    IDAdder() CLICK_COLD;

    const char *class_name() const	{ return "IDAdder"; }
    const char *port_count() const	{ return "1/1"; }
    const char *flags() const		{ return "A"; }

    void push(int port, Packet *p_in);

    
  private:
    
    uint16_t counter[MAX_N_CLIENT];
    click_ether _ethh;


};




CLICK_ENDDECLS
#endif
