/*
 * deduptcppacket.{cc,hh} -- deduplicates identical TCP packets
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
#include "deduptcppacket.hh"
#include <clicknet/ip.h>
#include <clicknet/tcp.h>
#include <click/args.hh>
#include <arpa/inet.h>
CLICK_DECLS

DeDupTCPPacket::DeDupTCPPacket()
  : _set(0, 512), _timer(this)
{
  // sets _timer to call this->run_timer(&_timer) when it fires.
}

DeDupTCPPacket::~DeDupTCPPacket()
{
  // Does something need to be here?
}

int
DeDupTCPPacket::configure(Vector<String> &conf, ErrorHandler *errh)
{
  return 0;
}

int
DeDupTCPPacket::initialize(ErrorHandler *)
{
  _timer.initialize(this);
  _timer.schedule_now();
  return 0;
}

void
DeDupTCPPacket::cleanup(CleanupStage)
{

}

void
DeDupTCPPacket::run_timer(Timer *timer)
{
  assert(timer == &_timer);
  if (_set.size() > 0) {
    _set.clear();
  }
  _timer.reschedule_after_sec(2);
}

Packet *
DeDupTCPPacket::drop(Packet *p)
{
//  click_chatter("TCP: duplicate, dropping");
  if (noutputs() == 2)
    output(1).push(p);
  else
    p->kill();

  return 0;
}

Packet *
DeDupTCPPacket::simple_action(Packet *p_in)
{
  WritablePacket *p = p_in->uniqueify();
  struct click_ip *iph = p->ip_header();
  struct click_tcp *tcph = p->tcp_header();
  uint64_t key;
  unsigned len, iph_len, tcph_len, plen;
  click_tcp temp_tcph;

  memcpy(&temp_tcph, tcph, sizeof(struct click_tcp));

  plen = ntohs(iph->ip_len) - (iph->ip_hl << 2);
  if (!p->has_network_header() || iph->ip_p != IP_PROTO_TCP
      || !p->has_transport_header() || plen < sizeof(click_tcp)
      || plen > (unsigned)p->transport_length()) {
    return drop(p);
  }

  iph_len = iph->ip_hl << 2;
  len = ntohs(iph->ip_len) - iph_len;
  tcph_len = tcph->th_off << 2;
  if (tcph_len < sizeof(click_tcp) || len < tcph_len
      || p->length() < len + iph_len + p->network_header_offset()) {
    return drop(p);
  }

  // Deduplication Code here:

  key = build_key(iph, tcph, plen);

  if (_set.get(key) != _set.default_value()) {
    // In the table
    return drop(p);
  }
//  click_chatter("TCP: successful");

  _set.set(key, 1);
  // Cleared every 2 seconds by the timer.

  return p;
}

uint64_t
DeDupTCPPacket::build_key(struct click_ip *iph, struct click_tcp *tcph, unsigned plen)
{
  uint16_t csum, th_sum;
  uint64_t key;
  uint16_t temp_sport, temp_sum;
  struct in_addr temp_src;

  // Save
  memcpy(&temp_src, &(iph->ip_src), sizeof(struct in_addr));
  temp_sport = tcph->th_sport;
  temp_sum = tcph->th_sum;

  // Replace
  inet_pton(AF_INET, "127.0.0.1", &(iph->ip_src));
  tcph->th_sport = 1234;
  tcph->th_sum = 0;

  // Calculate
  csum = click_in_cksum((unsigned char *)tcph, plen);
  th_sum = click_in_cksum_pseudohdr(csum, iph, plen);

  // Restore
  memcpy(&(iph->ip_src), &temp_src, sizeof(struct in_addr));
  tcph->th_sport = temp_sport;
  tcph->th_sum = temp_sum;

  key = tcph->th_seq;
  key = key << 32;

  key = key | (uint64_t) tcph->th_dport;
  key = key << 16;

  key = key | (uint64_t) th_sum;

  return key;
}

void
DeDupTCPPacket::add_handlers()
{

}

CLICK_ENDDECLS
EXPORT_ELEMENT(DeDupTCPPacket)
