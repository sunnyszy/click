#ifndef CLICK_PACKETSELECTIONPERIODICSWITCH_HH
#define CLICK_PACKETSELECTIONPERIODICSWITCH_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>

CLICK_DECLS


class PacketSelectionPeriodicSwitch : public Element { public:


    PacketSelectionPeriodicSwitch() CLICK_COLD;
    ~PacketSelectionPeriodicSwitch() CLICK_COLD;

    const char *class_name() const	{ return "PacketSelectionPeriodicSwitch"; }
    const char *port_count() const	{ return "4/3"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *p_in);

    void destination_change(Packet *);


  private:
    
    static const int n_outport = 2;
    int output_port;
    int switch_time;
    int switch_counter;
    unsigned char switch_flag;
    

};

CLICK_ENDDECLS
#endif
