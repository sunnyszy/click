#ifndef CLICK_IDADDER_HH
#define CLICK_IDADDER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>


CLICK_DECLS

#define STATUS 4
#define DATA 17
#define CONTROL 6
#define IDLE 0
#define SWITCH_REQ 1
#define CLIENT1 135
#define AP1 2
#define AP2 4
#define AP1_IP 0x0201a8c0
#define AP2_IP 0x0401a8c0
#define CONTROLLER_IP 0x0101a8c0
#define AP1_MAC "C0:56:27:72:A3:5B"
#define AP2_MAC "60:38:E0:03:FA:0B"
#define CONTROLLER_MAC "38:c9:86:40:c8:05"



class IDAdder : public Element { public:


    IDAdder() CLICK_COLD;
    ~IDAdder() CLICK_COLD;

    const char *class_name() const	{ return "IDAdder"; }
    const char *port_count() const	{ return "1/1"; }
    const char *flags() const		{ return "A"; }

    // int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *p_in);

    
  private:
    
    unsigned char counter;


};




CLICK_ENDDECLS
#endif
