/*
 element for 802.11r client, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
#ifndef CLICK_RCLIENTCONTROL_HH
#define CLICK_RCLIENTCONTROL_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/wgtt.h>
#include <clicknet/ether.h>
#include <syslog.h>
#include <sys/time.h>

CLICK_DECLS


class RClientControl : public Element { public:


    const char *class_name() const	{ return "RClientControl"; }
    const char *port_count() const	{ return "4/2"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
   
    void push(int port, Packet *);
    void push_control(Packet *);
    void push_80211(Packet *);
    void push_downdata(Packet *);
    void push_updata(Packet *);
    void reset();

  private:
    double rssi[MAX_N_AP];
    char rssi_threshold;
    double alpha;

    unsigned char identity;
    unsigned char state;
    unsigned char current_ap;

    click_ether * _ethh;
    unsigned char control_content[4];
    
    const unsigned char ether_type_ip_suffix = 0x00;


    int interval;
    int print_interval;
    // after issue switch, a time lock will be set for 1 second
    bool time_lock;
    double last_time;
    struct timeval tv;

    

};

CLICK_ENDDECLS
#endif
