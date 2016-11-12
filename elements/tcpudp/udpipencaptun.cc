/*
 * udpipencaptun.{cc,hh} -- element encapsulates packet in UDP/IP header
 * Benjie Chen, Eddie Kohler, Hansen Qian
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2007 Regents of the University of California
 * Copyright (c) 2016 Princeton University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include <clicknet/ip.h>
#include "udpipencaptun.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/standard/alignmentinfo.hh>
#include <arpa/inet.h>
 CLICK_DECLS

 UDPIPEncapTun::UDPIPEncapTun()
 : _set(0, 512), _packet_counts(0, 512), _timer(this),
   _cksum(true), _use_dst_anno(false)
 {
  _id = 0;

  _buf_ptr = 0;
  for (int i = 0; i < BUF_SIZE; i++) {
    _circ_buf[i] = 0;
  }
#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
  _checked_aligned = false;
#endif
}

UDPIPEncapTun::~UDPIPEncapTun()
{
}

int
UDPIPEncapTun::initialize(ErrorHandler *)
{
  _timer.initialize(this);
  _timer.schedule_now();
  return 0;
}

void
UDPIPEncapTun::run_timer(Timer *timer)
{
  HashTable_iterator<Pair<const uint32_t, int> > it;
  uint32_t max_key = _daddr.s_addr;
  int max_val = 0;
  struct in_addr temp_in_addr;
  char strbuf[INET6_ADDRSTRLEN];

  assert(timer == &_timer);
  if (_set.size() > 50) {
    _set.clear();
  }

  for (it = _packet_counts.begin(); it != _packet_counts.end(); it++) {
    if (it.get()->second > max_val) {
      max_key = it.get()->first;
      max_val = it.get()->second;
    }
  }
  temp_in_addr.s_addr = max_key;
  memcpy(&_daddr, &temp_in_addr, sizeof(struct in_addr));


  inet_ntop(AF_INET, &_daddr, strbuf, INET6_ADDRSTRLEN);
  click_chatter("Tunneling through AP address: %s", strbuf);

  _timer.reschedule_after_sec(1.5);
}

int
UDPIPEncapTun::configure(Vector<String> &conf, ErrorHandler *errh)
{
  IPAddress saddr;
  uint16_t sport, dport;
  bool cksum;
  String daddr_str;

  if (Args(conf, this, errh)
      .read_mp("SRC", saddr)
      .read_mp("SPORT", IPPortArg(IP_PROTO_UDP), sport)
      .read_mp("DST", AnyArg(), daddr_str)
      .read_mp("DPORT", IPPortArg(IP_PROTO_UDP), dport)
      .read_p("CHECKSUM", BoolArg(), cksum)
      .complete() < 0)
    return -1;

  if (daddr_str.equals("DST_ANNO", 8)) {
    _daddr = IPAddress();
    _use_dst_anno = true;
  } else if (IPAddressArg().parse(daddr_str, _daddr, this))
    _use_dst_anno = false;
  else
    return errh->error("bad DST");

  _saddr = saddr;
  _sport = htons(sport);
  _dport = htons(dport);

#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
  if (!_checked_aligned) {
    int ans, c, o;
    ans = AlignmentInfo::query(this, 0, c, o);
    _aligned = (ans && c == 4 && o == 0);
    if (!_aligned)
      errh->warning("IP header unaligned, cannot use fast IP checksum");
    if (!ans)
      errh->message("(Try passing the configuration through %<click-align%>.)");
    _checked_aligned = true;
  }
#endif

  return 0;
}

void
UDPIPEncapTun::push(int port, Packet *p_in)
{
  WritablePacket *p;
  struct click_ip *iph;
  struct click_udp *udph;
  uint32_t key, new_addr, old_addr;
  int temp_count;
  char strbuf[INET6_ADDRSTRLEN];
  int cmpres = 0;
  bool alreadySeen;

  if (port == 0) {
    // Downlink traffic--encap (use simple_action)
    // Input 0 -> Output 0
    if ((p_in = simple_action(p_in)))
      output(0).push(p_in);
    return;
  }


  p = p_in->uniqueify();
  iph = p->ip_header();
  udph = p->udp_header();

  // Uplink traffic
  // Input 1 -> Output 1
  key = build_key(iph, udph);
  alreadySeen = _set.get(key) != _set.default_value();
  if (alreadySeen == false) {
    // If we haven't seen this packet before
    //inet_ntop(AF_INET, &(iph->ip_src), strbuf, INET6_ADDRSTRLEN);
    //click_chatter("Unique Packet from %s", strbuf);

    // Add into hashset
    _set.set(key, 1);

    // Accounting Code
    new_addr = iph->ip_src.s_addr;

    old_addr = _circ_buf[_buf_ptr];

    if (new_addr != old_addr) {
      if (old_addr != 0) {
        temp_count = _packet_counts.get(old_addr); // 0 if old_addr not in table
        _packet_counts.set(old_addr, temp_count - 1);
      }
      _circ_buf[_buf_ptr] = new_addr;

      temp_count = _packet_counts.get(new_addr);
      _packet_counts.set(new_addr, temp_count + 1);
    }

    _buf_ptr = (_buf_ptr + 1) % BUF_SIZE;


    /*
    cmpres = memcmp(&_daddr, &(iph->ip_src), sizeof(struct in_addr));
    if (cmpres != 0) {
      // If this comes from a different AP
      _counter++;
      // Only change _daddr after we've seen 5 packets that confirm the new
      // _daddr is closer
      if (_counter >= 10) {
        memcpy(&_daddr, &(iph->ip_src), sizeof(struct in_addr));
        click_chatter("============================== Changing Tunnel Destination: %s", strbuf);
        _counter = 0;
      }
    } else {
      // If the source IP Address is the same, reset counter to 0.
      // We prefer to stick with the AP that works than switch
      _counter = 0;
    }
    */
  }

  output(1).push(p);
  return;
}

Packet *
UDPIPEncapTun::simple_action(Packet *p_in)
{
  WritablePacket *p = p_in->push(sizeof(click_udp) + sizeof(click_ip));
  struct click_ip *ip = reinterpret_cast<click_ip *>(p->data());
  struct click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
  char strbuf[INET6_ADDRSTRLEN];

#if !HAVE_INDIFFERENT_ALIGNMENT
  assert((uintptr_t)ip % 4 == 0);
#endif
  // set up IP header
  ip->ip_v = 4;
  ip->ip_hl = sizeof(click_ip) >> 2;
  ip->ip_len = htons(p->length());
  ip->ip_id = htons(_id.fetch_and_add(1));
  ip->ip_p = IP_PROTO_UDP;
  ip->ip_src = _saddr;
  if (_use_dst_anno)
    ip->ip_dst = p->dst_ip_anno();
  else {
    ip->ip_dst = _daddr;
    p->set_dst_ip_anno(IPAddress(_daddr));
  }
  ip->ip_tos = 0;
  ip->ip_off = 0;
  ip->ip_ttl = 250;

  inet_ntop(AF_INET, &ip->ip_dst, strbuf, INET6_ADDRSTRLEN);
  //click_chatter("============================== transmitting to: %s", strbuf);

  ip->ip_sum = 0;
#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
  if (_aligned)
    ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
  else
    ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
#elif HAVE_FAST_CHECKSUM
  ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
#else
  ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
#endif

  p->set_ip_header(ip, sizeof(click_ip));

  // set up UDP header
  udp->uh_sport = _sport;
  udp->uh_dport = _dport;
  uint16_t len = p->length() - sizeof(click_ip);
  udp->uh_ulen = htons(len);
  udp->uh_sum = 0;
  if (_cksum) {
    unsigned csum = click_in_cksum((unsigned char *)udp, len);
    udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
  }

  return p;
}

uint32_t
UDPIPEncapTun::build_key(struct click_ip *iph, struct click_udp *udph)
{
  uint32_t key;
  uint16_t csum, ip_sum, uh_sum;
  uint16_t temp_uh_sum, temp_ip_sum, temp_ip_id;
  unsigned plen = ntohs(udph->uh_ulen);
  struct in_addr temp_src;

  // Save
  memcpy(&temp_src, &(iph->ip_src), sizeof(struct in_addr));
  temp_uh_sum = udph->uh_sum;
  temp_ip_sum = iph->ip_sum;
  temp_ip_id = iph->ip_id;

  // Replace
  inet_pton(AF_INET, "127.0.0.1", &(iph->ip_src));
  udph->uh_sum = 0;
  iph->ip_sum = 0;
  iph->ip_id = 0;

  // Calculate
  csum = click_in_cksum((unsigned char *)udph, plen);
  uh_sum = click_in_cksum_pseudohdr(csum, iph, plen);
#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
    if (_aligned)
      ip_sum = ip_fast_csum((unsigned char *)iph, sizeof(click_ip) >> 2);
    else
      ip_sum = click_in_cksum((unsigned char *)iph, sizeof(click_ip));
#elif HAVE_FAST_CHECKSUM
    ip_sum = ip_fast_csum((unsigned char *)iph, sizeof(click_ip) >> 2);
#else
    ip_sum = click_in_cksum((unsigned char *)iph, sizeof(click_ip));
#endif

  // Restore
  memcpy(&(iph->ip_src), &temp_src, sizeof(struct in_addr));
  udph->uh_sum = temp_uh_sum;
  iph->ip_sum = temp_ip_sum;
  iph->ip_id = temp_ip_id;

  key = ip_sum;
  key = key << 16;

  key = key | uh_sum;
  return key;
}

String UDPIPEncapTun::read_handler(Element *e, void *thunk)
{
  UDPIPEncapTun *u = static_cast<UDPIPEncapTun *>(e);
  switch ((uintptr_t) thunk) {
    case 0:
    return IPAddress(u->_saddr).unparse();
    case 1:
    return String(ntohs(u->_sport));
    case 2:
    return IPAddress(u->_daddr).unparse();
    case 3:
    return String(ntohs(u->_dport));
    default:
    return String();
  }
}

void UDPIPEncapTun::add_handlers()
{
  add_read_handler("src", read_handler, 0);
  add_write_handler("src", reconfigure_keyword_handler, "0 SRC");
  add_read_handler("sport", read_handler, 1);
  add_write_handler("sport", reconfigure_keyword_handler, "1 SPORT");
  add_read_handler("dst", read_handler, 2);
  add_write_handler("dst", reconfigure_keyword_handler, "2 DST");
  add_read_handler("dport", read_handler, 3);
  add_write_handler("dport", reconfigure_keyword_handler, "3 DPORT");
}

CLICK_ENDDECLS
EXPORT_ELEMENT(UDPIPEncapTun)
ELEMENT_MT_SAFE(UDPIPEncapTun)
