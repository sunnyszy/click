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
{
  _set.clear();
  while (!_queue.empty())
    _queue.pop();
}

DeDupTCPPacket::~DeDupTCPPacket()
{
  // Does something need to be here?
}



Packet *
DeDupTCPPacket::drop(Packet *p)
{
  //click_chatter("TCP: duplicate, dropping");
  if (noutputs() == 2)
    output(1).push(p);
  else
    p->kill();

  return 0;
}

void
DeDupTCPPacket::push(int port, Packet *p_in)
{
  // construct link_key
  WritablePacket *p = p_in->uniqueify();
  struct click_ip *iph = p->ip_header();

  uint64_t tmp_link_key = ((((uint64_t)(iph->ip_id))&0x000000000000ffff)<<32)+
    (((uint64_t)((iph->ip_src).s_addr))&0x00000000ffffffff);
    // printf("key: %lx\n", tmp_link_key);
  std::set<uint64_t>::iterator it;
  it = _set.find(tmp_link_key);
  if( it != _set.end())
  {
      drop(p);
  }
  else
  {
    //if (_set.size() >= max_elem_num)
    if (_queue.size() >= max_elem_num)
    {
      // printf("exceed the max size\n");
      uint64_t & link_key_tobe_delete = _queue.front();
      _queue.pop();
       it = _set.find(link_key_tobe_delete);
      _set.erase(it);
      // delete link_key_tobe_delete;
    }
      _set.insert(tmp_link_key);
      _queue.push(tmp_link_key);
  }
  output(0).push(p);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(DeDupTCPPacket)
