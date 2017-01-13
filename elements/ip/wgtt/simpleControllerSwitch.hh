/*
 Controller program, generate simple data and switch control
 Input: trigger packet
 Output: control & data
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
#ifndef CLICK_SIMPLECONTROLLERSWITCH_HH
#define CLICK_SIMPLECONTROLLERSWITCH_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/ether.h>
#include <click/confparse.hh>
#include <clicknet/wgtt.h>
#include <clicknet/tcp.h>
#include <sys/time.h>
#include <syslog.h>
CLICK_DECLS

class SimpleControllerSwitch : public Element { public:


    SimpleControllerSwitch() CLICK_COLD;

    const char *class_name() const	{ return "SimpleControllerSwitch"; }
    const char *port_count() const	{ return "1/2"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void reset_ap();
    void push(int port, Packet *p_in);
    
    void push_status(Packet *p_in);
    
  private:
    
    unsigned char output_port;
    unsigned char control_content[2];

    // used for debug. 
    // By setting a positive number, manually switch after every ${interval} pkt 
    // Between ap 1 - 2
    int interval;

    click_ether * _ethh;
    click_tcp _tcp;


};




CLICK_ENDDECLS
#endif
