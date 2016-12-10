#ifndef CLICK_PACKETSELECTIONSERIAL_HH
#define CLICK_PACKETSELECTIONSERIAL_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/ether.h>
#include <click/confparse.hh>
#include <clicknet/wgtt.h>

CLICK_DECLS





class PacketSelectionSerial : public Element { public:


    PacketSelectionSerial() CLICK_COLD;
    ~PacketSelectionSerial() CLICK_COLD;

    const char *class_name() const	{ return "PacketSelectionSerial"; }
    const char *port_count() const	{ return "1/1"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void reset_ap();
    void push(int port, Packet *p_in);
    void push_control(Packet *p_in);
    void push_status(Packet *p_in);
    unsigned char find_best_ap();
    
  private:
    
    unsigned char *state;
    static const unsigned char n_compare = 5;
    int **score;
    unsigned char *next_score_id;
    unsigned char *output_port;
    unsigned char control_content[2];
    int interval;
    int first_start;
    int print_interval;

    click_ether * _ethh;


};




CLICK_ENDDECLS
#endif
