#ifndef CLICK_PACKETSELECTION_HH
#define CLICK_PACKETSELECTION_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>

CLICK_DECLS


class PacketSelection : public Element { public:


    PacketSelection() CLICK_COLD;
    ~PacketSelection() CLICK_COLD;

    const char *class_name() const	{ return "PacketSelection"; }
    const char *port_count() const	{ return "4/3"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *p_in);

    void state_change(int, Packet *);
    void destination_change(Packet *);
    int csi_get_score(Packet *);
    int bit_convert(int, int);

  private:
    double alpha;
    double *score;
    int print_counter;
    
    static const int n_outport = 3;
    int fresh_time;
    int fresh_counter;
    int bigger_time;
    int bigger_counter;
    int output_port;
    bool lock;
    int fix;
    

};

CLICK_ENDDECLS
#endif
