/*
 Deduplication.
 Input: upload raw data packet (eth+ip+x), must checkIPHeader first
 Output: deduplicated data packet
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#ifndef CLICK_DEDUPTCPPACKET_HH
#define CLICK_DEDUPTCPPACKET_HH
#include <click/element.hh>
#include <click/atomic.hh>

#include <set>
#include <queue>

CLICK_DECLS



class DeDupTCPPacket : public Element { public:

  DeDupTCPPacket();

  const char *class_name() const		{ return "DeDupTCPPacket"; }
  const char *port_count() const		{ return PORTS_1_1X2; }
  const char *processing() const		{ return PUSH; }

  void push(int, Packet *);


 private:

  std::set<uint64_t> _set;
  std::queue<uint64_t> _queue;
  static const uint16_t max_elem_num = 1000;

  Packet *drop(Packet *);

};

CLICK_ENDDECLS
#endif
