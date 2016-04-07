#ifndef CLICK_DEDUPIPPACKET_HH
#define CLICK_DEDUPIPPACKET_HH
#include <click/element.hh>
#include <click/atomic.hh>
#include <click/hashtable.hh>
#include <click/timer.hh>
CLICK_DECLS

/*
=c

DeDupIPPacket()

=s tcp

drops duplicate IP packets

=d

Deduplicates IP packets, if the same one is seen multiple times.

Keys each packet on the IP destination IP, and
checksum (ignoring source IP or source Port). If the key has been seen before,
the packet is dropped.
Otherwise, the packet is allowed through, and the key is stored for a maximum
of two seconds. The memory usage is reset every two seconds to prevent
memory usage.

Expects IP packets (after CheckIPHeader or MarkIPHeader) as input.

Install with

            make elemlist
            make install

=a DedupTCPPacket, DedupUDPPacket, CheckTCPHeader, CheckIPHeader,
CheckUDPHeader, MarkIPHeader */

class DeDupIPPacket : public Element { public:

  typedef HashTable<uint64_t, int> Set;

  DeDupIPPacket();
  ~DeDupIPPacket();

  const char *class_name() const        { return "DeDupIPPacket"; }
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

  uint64_t build_key(struct click_ip *, unsigned);

};

CLICK_ENDDECLS
#endif
