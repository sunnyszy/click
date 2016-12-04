#ifndef CLICK_CSISEP_HH
#define CLICK_CSISEP_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <fcntl.h>
#include <unistd.h>

#ifndef __APPLE__
extern "C"
{
    #include "iwinfo.h"
}
#endif


CLICK_DECLS

class CSISep : public Element { public:

  CSISep() CLICK_COLD;
  ~CSISep() CLICK_COLD;

  const char *class_name() const		{ return "CSISep"; }
  const char *port_count() const		{ return PORTS_1_1X2; }
  const char *processing() const		{ return PUSH; }

  int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
  
  void push(int, Packet *);
  void fragment(Packet *);
  int get_rssi();

 private:

  int sample_rate;
  int sample_counter;

#ifndef __APPLE__
  int len;
  const struct iwinfo_ops *iw;
  char buf[IWINFO_BUFSIZE];
  const char ifname[6] = "wlan1";
#endif 


};

CLICK_ENDDECLS
#endif
