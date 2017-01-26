/*
 element for 802.11r ap, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
#ifndef CLICK_RAPCONTROL_HH
#define CLICK_RAPCONTROL_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/wgtt.h>
#include <clicknet/ether.h>
#include <syslog.h>

CLICK_DECLS


class RAPControl : public Element { public:


    const char *class_name() const	{ return "RAPControl"; }
    const char *port_count() const	{ return "4/3"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *);
    void push_up_control(Packet *);
    void push_up_data(Packet *);
    void push_down_control(Packet *);
    void push_down_data(Packet *);


  private:
    unsigned char identity;
    unsigned char state[MAX_N_CLIENT];
    unsigned char first_start[MAX_N_CLIENT];

    click_ether * _ethh;
    unsigned char control_content[4];
    int tmp_start[3];//set this fix because # of click configuration interface

    

};

CLICK_ENDDECLS
#endif
