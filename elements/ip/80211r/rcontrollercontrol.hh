/*
 element for 802.11r ap, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
#ifndef CLICK_RCONTROLLERCONTROL_HH
#define CLICK_RCONTROLLERCONTROL_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/wgtt.h>
#include <clicknet/ether.h>
#include <syslog.h>

CLICK_DECLS


class RControlerControl : public Element { public:


    const char *class_name() const	{ return "RControlerControl"; }
    const char *port_count() const	{ return "4/2"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *);
    void push_up_control(Packet *);
    void push_down_data(Packet *, int);


  private:
    unsigned char outport[MAX_N_CLIENT];
    unsigned char control_content[4];
    click_ether * _ethh;
    int tmp_start[3];

    

};

CLICK_ENDDECLS
#endif
