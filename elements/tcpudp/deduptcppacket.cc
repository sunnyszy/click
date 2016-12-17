/*
 Deduplication.
 Input: upload raw data packet (eth+ip+x), must checkIPHeader first
 Output: deduplicated data packet
 Created by Zhenyu Song: sunnyszy@gmail.com
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
  // printf("Packet in\n");
  // construct link_key
  WritablePacket *p = p_in->uniqueify();
  struct click_ip *iph = p->ip_header();

  // printf("IP id: %x", iph->ip_id);
  uint64_t tmp_link_key = ((((uint64_t)(iph->ip_id))&0x000000000000ffff)<<32)+
    (((uint64_t)((iph->ip_src).s_addr))&0x00000000ffffffff);
  // printf("key: %lx\n", tmp_link_key);
  std::set<uint64_t>::iterator it;
  it = _set.find(tmp_link_key);
  if((iph -> ip_id) != 0)
  {
    if( it != _set.end())
    {
        // printf("Packet out.drop\n");
        drop(p);
        return;
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
  }
  // printf("Packet out.push\n");
  output(0).push(p);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(DeDupTCPPacket)
