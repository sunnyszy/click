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
#include "packetselectionPeriodicSwitch.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ip.h>
#include <math.h>

 CLICK_DECLS

PacketSelectionPeriodicSwitch::PacketSelectionPeriodicSwitch()
{
  output_port = 0;
  switch_counter = 0;
  switch_flag = 0xff;

}

PacketSelectionPeriodicSwitch::~PacketSelectionPeriodicSwitch()
{

}



int
PacketSelectionPeriodicSwitch::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (Args(conf, this, errh)
      .read_p("SWITCHTIME", IntArg(), switch_time)
      .complete() < 0)
    return -1;

  return 0;
}

void
PacketSelectionPeriodicSwitch::push(int port, Packet *p_in)
{
  if(port<n_outport)
    p_in -> kill();
  else
    destination_change(p_in);

}


void
PacketSelectionPeriodicSwitch::destination_change(Packet *p_in)
{

  WritablePacket *p_master = p_in->uniqueify();
  switch_counter ++;
  if(switch_counter >= switch_time)
  {
    switch_counter = 0;
    output_port = (output_port + 1)%n_outport;
  }
  output(output_port).push(p_master);

  WritablePacket *p_switch_flag = Packet::make(1);
  memcpy(p_switch_flag->data(), &switch_flag, 1);
  output(output_port).push(p_switch_flag);

}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketSelectionPeriodicSwitch)
ELEMENT_MT_SAFE(PacketSelectionPeriodicSwitch)
