#ifndef CLICK_IDADDER_HH
#define CLICK_IDADDER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/ether.h>
#include <clicknet/wgtt.h>

CLICK_DECLS


class IDAdder : public Element { public:


    IDAdder() CLICK_COLD;
    ~IDAdder() CLICK_COLD;

    const char *class_name() const	{ return "IDAdder"; }
    const char *port_count() const	{ return "1/1"; }
    const char *flags() const		{ return "A"; }

    // int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
  	int initialize(ErrorHandler*) CLICK_COLD;
    void push(int port, Packet *p_in);

    
  private:
    
    unsigned char counter;
    click_ether _ethh;


};




CLICK_ENDDECLS
#endif
