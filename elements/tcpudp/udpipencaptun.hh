#ifndef CLICK_UDPIPENCAPTUN_HH
#define CLICK_UDPIPENCAPTUN_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <clicknet/udp.h>
#include <click/hashtable.hh>
#include <click/timer.hh>
CLICK_DECLS

/*
=c

UDPIPEncapTun(SRC, SPORT, DST, DPORT [, CHECKSUM])

=s udp

encapsulates packets in static UDP/IP headers and tunnels to most recent
host

=d

*/

class UDPIPEncapTun : public Element { public:

    typedef HashTable<uint32_t, int> Set;

    UDPIPEncapTun() CLICK_COLD;
    ~UDPIPEncapTun() CLICK_COLD;

    const char *class_name() const	{ return "UDPIPEncapTun"; }
    const char *port_count() const	{ return "2/2"; }
    const char *flags() const		{ return "A"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
    bool can_live_reconfigure() const	{ return true; }
    void add_handlers() CLICK_COLD;


    void push(int port, Packet *p_in);

    Packet *simple_action(Packet *);

    int initialize(ErrorHandler *);
    void run_timer(Timer *);
  private:

    Set _set;
    Timer _timer;

    struct in_addr _saddr;
    struct in_addr _daddr;
    uint16_t _counter;
    uint16_t _sport;
    uint16_t _dport;
    bool _cksum;
    bool _use_dst_anno;
#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
    bool _aligned;
    bool _checked_aligned;
#endif
    atomic_uint32_t _id;

    static String read_handler(Element *, void *) CLICK_COLD;

    uint32_t build_key(struct click_ip *, struct click_udp *);

};

CLICK_ENDDECLS
#endif
