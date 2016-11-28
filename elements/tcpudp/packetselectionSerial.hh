#ifndef CLICK_PACKETSELECTIONSERIAL_HH
#define CLICK_PACKETSELECTIONSERIAL_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <click/confparse.hh>

CLICK_DECLS

#define STATUS 4
#define DATA 17
#define CONTROL 27
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


#define pkt_type(p) *(p->data()+9)
#define ip_id(p) *(p->data()+20)
#define ap_id(p) *(p->data()+15)
#define ap_score(p) *(p->data()+20)



class PacketSelectionSerial : public Element { public:


    PacketSelectionSerial() CLICK_COLD;
    ~PacketSelectionSerial() CLICK_COLD;

    const char *class_name() const	{ return "PacketSelectionSerial"; }
    const char *port_count() const	{ return "1/1"; }
    const char *flags() const		{ return "A"; }

    // int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *p_in);
    void push_control(Packet *p_in);
    void push_status(Packet *p_in);
    unsigned char find_best_ap();
    
  private:
    
    unsigned char *state;
    static const unsigned char n_client = 1;
    static const unsigned char n_ap = 2;
    static const unsigned char n_compare = 5;
    unsigned char **score;
    unsigned char *next_score_id;
    unsigned char *output_port;
    unsigned char control_content[2];

    click_ip _iph;
    click_ether * _ethh;


};




CLICK_ENDDECLS
#endif
