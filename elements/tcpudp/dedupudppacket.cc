/*
 * dedupudppacket.{cc,hh} -- deduplicates identical UDP packets
 * (checksums, lengths)
 * Hansen Qian (hq@cs.princeton.edu)
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
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
#include "dedupudppacket.hh"
#include <clicknet/ip.h>
#include <clicknet/udp.h>
#include <click/args.hh>
#include <arpa/inet.h>
CLICK_DECLS

DeDupUDPPacket::DeDupUDPPacket()
  : _set(0, 256), _timer(this)
{
  // sets _timer to call this->run_timer(&_timer) when it fires.
}

DeDupUDPPacket::~DeDupUDPPacket()
{
  // Does something need to be here?
}

int
DeDupUDPPacket::configure(Vector<String> &conf, ErrorHandler *errh)
{
  return 0;
}

int
DeDupUDPPacket::initialize(ErrorHandler *)
{
  _timer.initialize(this);
  _timer.schedule_now();
  return 0;
}

void
DeDupUDPPacket::cleanup(CleanupStage)
{

}

void
DeDupUDPPacket::run_timer(Timer *timer)
{
  assert(timer == &_timer);
  if (_set.size() > 0) {
    _set.clear();
  }
  _timer.reschedule_after_sec(2);
}

Packet *
DeDupUDPPacket::drop(Packet *p)
{
  click_chatter("Duplicate UDP Packet, dropping.");
  if (noutputs() == 2)
    output(1).push(p);
  else
    p->kill();

  return 0;
}

Packet *
DeDupUDPPacket::simple_action(Packet *p_in)
{
  WritablePacket *p = p_in->uniqueify();
  click_ip *iph = p->ip_header();
  click_udp *udph = p->udp_header();
  uint64_t key;
  unsigned len, iph_len, udph_len, plen;

  if (!p->has_network_header() || iph->ip_p != IP_PROTO_UDP)
    return drop(p);

  iph_len = iph->ip_hl << 2;
  len = ntohs(udph->uh_ulen);
  if (len < sizeof(click_udp)
      || p->length() < len + iph_len + p->network_header_offset())
    return drop(p);

  // Deduplication Code here:

  key = build_key(iph, udph, len);

  if (_set.get(key) != _set.default_value()) {
    // In the table
    return drop(p);
  }

  _set.set(key, 1);
  // Cleared every 2 seconds by the timer.

  return p;
}

uint64_t
DeDupUDPPacket::build_key(struct click_ip *iph, struct click_udp *udph, unsigned len)
{
  uint16_t csum, uh_sum;
  uint64_t key = 0;
  uint16_t temp_sport, temp_sum;
  struct in_addr temp_src;

  // Save
  memcpy(&temp_src, &(iph->ip_src), sizeof(struct in_addr));
  temp_sport = udph->uh_sport;
  temp_sum = udph->uh_sum;

  // Replace
  inet_pton(AF_INET, "127.0.0.1", &(iph->ip_src));
  udph->uh_sport = 1234;
  udph->uh_sum = 0;

  // Calculate
  csum = click_in_cksum((unsigned char *)udph, len);
  uh_sum = click_in_cksum_pseudohdr(csum, iph, len);

  // Restore
  memcpy(&(iph->ip_src), &temp_src, sizeof(struct in_addr));
  udph->uh_sport = temp_sport;
  udph->uh_sum = temp_sum;

  key = iph->ip_dst.s_addr;
  key = key << 32;

  key = key | (uint64_t) udph->uh_dport;
  key = key << 16;

  key = key | (uint64_t) uh_sum;

  return key;
}

void
DeDupUDPPacket::add_handlers()
{

}

CLICK_ENDDECLS
EXPORT_ELEMENT(DeDupUDPPacket)
