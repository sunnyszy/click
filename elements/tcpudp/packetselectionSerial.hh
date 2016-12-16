/*
 Controller program, issusing switching between different ap. 
 Input: control/status packet
 Output: control packet
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
#ifndef CLICK_PACKETSELECTIONSERIAL_HH
#define CLICK_PACKETSELECTIONSERIAL_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/ether.h>
#include <click/confparse.hh>
#include <clicknet/wgtt.h>
#include <sys/time.h>

CLICK_DECLS

class PacketSelectionSerial : public Element { public:


    PacketSelectionSerial() CLICK_COLD;

    const char *class_name() const	{ return "PacketSelectionSerial"; }
    const char *port_count() const	{ return "1/1"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void reset_ap();
    void push(int port, Packet *p_in);
    void push_control(Packet *p_in);
    void push_status(Packet *p_in);
    // find best ap for client c
    unsigned char find_best_ap(unsigned char c);
    
  private:
    
    unsigned char state[MAX_N_CLIENT];
    static const unsigned char n_compare = 5;
    // [client, ap, n_compare]
    int ***score;
    // [client, ap]
    unsigned char ** next_score_id;
    unsigned char output_port[MAX_N_CLIENT];
    unsigned char control_content[2];
    // which ap will first start
    unsigned char first_start[MAX_N_CLIENT];

    // used for debug. 
    // By setting a positive number, manually switch after every ${interval} pkt 
    // Between ap 1 - 2
    int interval;
    // Printing screen after ${print_interval} pkt
    int print_interval;

    // after issue switch, a time lock will be set for 1 second
    bool time_lock[MAX_N_CLIENT];
    double last_time[MAX_N_CLIENT];
    struct timeval tv;

    click_ether * _ethh;


};




CLICK_ENDDECLS
#endif
