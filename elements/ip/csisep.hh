#ifndef CLICK_CSISEP_HH
#define CLICK_CSISEP_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
CLICK_DECLS

class CSISep : public Element { public:

  CSISep() CLICK_COLD;
  ~CSISep() CLICK_COLD;

  const char *class_name() const		{ return "CSISep"; }
  const char *port_count() const		{ return PORTS_1_1X2; }
  const char *processing() const		{ return PUSH; }

  void push(int, Packet *);

 private:

  static const uint32_t CSI_LEN = 280;

  void fragment(Packet *);

};

CLICK_ENDDECLS
#endif
