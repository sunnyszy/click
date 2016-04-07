/*
 * dedupippacket.{cc,hh} -- deduplicates identical IP packets
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
#include "dedupippacket.hh"
#include <clicknet/ip.h>
#include <click/args.hh>
#include <arpa/inet.h>
CLICK_DECLS

DeDupIPPacket::DeDupIPPacket()
  : _set(0, 256), _timer(this)
{
  // sets _timer to call this->run_timer(&_timer) when it fires.
}

DeDupIPPacket::~DeDupIPPacket()
{
  // Does something need to be here?
}

int
DeDupIPPacket::configure(Vector<String> &conf, ErrorHandler *errh)
{
  return 0;
}

int
DeDupIPPacket::initialize(ErrorHandler *)
{
  _timer.initialize(this);
  _timer.schedule_now();
  return 0;
}

void
DeDupIPPacket::cleanup(CleanupStage)
{

}

void
DeDupIPPacket::run_timer(Timer *timer)
{
  assert(timer == &_timer);
  if (_set.size() > 0) {
    _set.clear();
  }
  _timer.reschedule_after_sec(2);
}

Packet *
DeDupIPPacket::drop(Packet *p)
{
  click_chatter("Duplicate Packet, dropping.");
  if (noutputs() == 2)
    output(1).push(p);
  else
    p->kill();

  return 0;
}

Packet *
DeDupIPPacket::simple_action(Packet *p_in)
{
  WritablePacket *p = p_in->uniqueify();
  click_ip *iph = p->ip_header();
  uint64_t key;
  unsigned len;

  len = iph->ip_hl << 2;
  // Deduplication Code here:

  key = build_key(iph, len);

  if (_set.get(key) != _set.default_value()) {
    // In the table
    return drop(p);
  }

  _set.set(key, 1);
  // Cleared every 2 seconds by the timer.

  return p;
}

uint64_t
DeDupIPPacket::build_key(struct click_ip *iph, unsigned len)
{
  uint16_t ip_sum;
  uint64_t key = 0;
  uint16_t temp_sum;
  struct in_addr temp_src;

  // Save
  memcpy(&temp_src, &(iph->ip_src), sizeof(struct in_addr));
  temp_sum = iph->ip_sum;

  // Replace
  inet_pton(AF_INET, "127.0.0.1", &(iph->ip_src));
  iph->ip_sum = 0;

  // Calculate
  ip_sum = click_in_cksum((unsigned char *)iph, len);

  // Restore
  memcpy(&(iph->ip_src), &temp_src, sizeof(struct in_addr));
  iph->ip_sum = temp_sum;

  key = iph->ip_dst.s_addr;
  key = key << 32;

  key = key | (uint64_t) iph->ip_len;
  key = key << 16;

  key = key | (uint64_t) ip_sum;

  return key;
}

void
DeDupIPPacket::add_handlers()
{

}

CLICK_ENDDECLS
EXPORT_ELEMENT(DeDupIPPacket)
