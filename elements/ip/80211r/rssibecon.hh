/*
 element for 802.11r becon sending, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#ifndef CLICK_RSSIBECON_HH
#define CLICK_RSSIBECON_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __arm__
#include <clicknet/wgtt.h>
extern "C"
{
  #include "iwinfo.h"
}
#endif 


CLICK_DECLS

class RSSIBecon : public Element { public:

  RSSIBecon() CLICK_COLD;
  ~RSSIBecon() CLICK_COLD;

  const char *class_name() const		{ return "RSSIBecon"; }
  const char *port_count() const		{ return "1/1"; }
  const char *processing() const		{ return PUSH; }

  int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
  
  void push(int, Packet *);
  void fragment(Packet *);

 private:


#ifdef __arm__
  int len;
  const struct iwinfo_ops *iw;
  char buf[IWINFO_BUFSIZE];
  char ifname[6];
  struct iwinfo_assoclist_entry *e;
#endif 


};

CLICK_ENDDECLS
#endif
