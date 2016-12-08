#ifndef CLICK_CSISEP_HH
#define CLICK_CSISEP_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <fcntl.h>
#include <unistd.h>
#include <clicknet/wgtt.h>

#ifndef __APPLE__
extern "C"
{
    #include "iwinfo.h"
}
#endif

struct my_test_struct {
  uint8_t mac;
  int8_t signal;
  int8_t noise;
  uint32_t rx_rate;
  uint32_t tx_rate;
  
};


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

 private:

  int sample_rate;
  int sample_counter;

#ifndef __APPLE__
  int len;
  const struct iwinfo_ops *iw;
  char buf[IWINFO_BUFSIZE];
  char ifname[6];
  struct iwinfo_assoclist_entry *e;
#endif 


};

CLICK_ENDDECLS
#endif
