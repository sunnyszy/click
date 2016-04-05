#ifndef CLICK_DEDUPTCPPACKET_HH
#define CLICK_DEDUPTCPPACKET_HH
#include <click/element.hh>
#include <click/atomic.hh>
#include <click/hashtable.hh>
#include <click/timer.hh>
CLICK_DECLS

/*
=c

DeDupTCPPacket()

=s tcp

drops duplicate TCP packets

=d

Deduplicates TCP packets, if the same one is seen multiple times.

Expects TCP/IP packets as input. Checks that the TCP header length is valid.
Keys each packet on the TCP sequence number, destination port, and
checksum. If the key is found in the hashset, the packet is dropped.
Otherwise, the packet is allowed through, and the key is stored into the
hashset.

The hashset is emptied once it reaches 500 elements.

Install with

            make elemlist
            make install

=a CheckTCPHeader, CheckIPHeader, CheckUDPHeader, CheckICMPHeader,
MarkIPHeader */

class DeDupTCPPacket : public Element { public:

  typedef HashTable<uint64_t, int> Set;

  DeDupTCPPacket();
  ~DeDupTCPPacket();

  const char *class_name() const		{ return "DeDupTCPPacket"; }
  const char *port_count() const		{ return PORTS_1_1X2; }
  const char *processing() const		{ return AGNOSTIC; }

  int configure();
  void add_handlers();

  Packet *simple_action(Packet *);

  int initialize(ErrorHandler *);
  void cleanup(CleanupStage);

  void run_timer(Timer *);

 private:

  Set _set;
  Timer _timer;

  Packet *drop(Packet *);

  uint64_t build_key(const click_ip *, const click_tcp *, unsigned);

};

CLICK_ENDDECLS
#endif
