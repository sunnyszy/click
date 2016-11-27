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
#include "idadder.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>


 CLICK_DECLS




IDAdder::IDAdder()
{
  counter = 0;

}

IDAdder::~IDAdder()
{

}


void IDAdder::push(int port, Packet *p_in)
{
  // printf("into idadder\n");
  WritablePacket *p = p_in->put(1);
  memcpy(p->end_data()-1, &counter, 1);
  counter ++;
  printf("Tail: %X\n", *(p->end_data()-1));
  printf("counter: %u\n", counter);
  // p_in -> kill();
  output(0).push(p);
  
}



CLICK_ENDDECLS
EXPORT_ELEMENT(IDAdder)
ELEMENT_MT_SAFE(IDAdder)
