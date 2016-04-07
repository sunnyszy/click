#ifndef CLICK_DEDUPUDPPACKET_HH
#define CLICK_DEDUPUDPPACKET_HH
#include <click/element.hh>
#include <click/atomic.hh>
#include <click/hashtable.hh>
#include <click/timer.hh>
CLICK_DECLS

/*
=c

DeDupUDPPacket()

=s tcp

drops duplicate UDP packets

=d

Deduplicates UDP packets, if the same one is seen multiple times.

Expects UDP/IP packets as input. Checks that the UDP header length is valid.
Keys each packet on the UDP destination IP, destination port, and
checksum (ignoring source IP or source Port). If the key has been seen before,
the packet is dropped.
Otherwise, the packet is allowed through, and the key is stored for a maximum
of two seconds. The memory usage is reset every two seconds to prevent
memory usage.

Install with

            make elemlist
            make install

=a DedupTCPPacket, DedupIPPacket, CheckTCPHeader, CheckIPHeader,
CheckUDPHeader, MarkIPHeader */

class DeDupUDPPacket : public Element { public:

  typedef HashTable<uint64_t, int> Set;

  DeDupUDPPacket();
  ~DeDupUDPPacket();

  const char *class_name() const        { return "DeDupUDPPacket"; }
  const char *port_count() const        { return PORTS_1_1X2; }
  const char *processing() const        { return AGNOSTIC; }

  int configure(Vector<String> &, ErrorHandler *);
  void add_handlers();

  Packet *simple_action(Packet *);

  int initialize(ErrorHandler *);
  void cleanup(CleanupStage);

  void run_timer(Timer *);

 private:

  Set _set;
  Timer _timer;

  Packet *drop(Packet *);

  uint64_t build_key(struct click_ip *, struct click_udp *, unsigned);

};

CLICK_ENDDECLS
#endif
