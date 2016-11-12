// -*- c-basic-offset: 4 -*-
/*
 * ipfragmenter.{cc,hh} -- element fragments IP packets
 * Robert Morris, Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2002 International Computer Science Institute
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
#include "csisep.hh"
#include <clicknet/ip.h>
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
CLICK_DECLS

CSISep::CSISep()
{
}

CSISep::~CSISep()
{
}


void
CSISep::fragment(Packet *p_in)
{
    Packet *first_fragment = p_in->clone();
    WritablePacket *p_master = first_fragment->uniqueify();
    p_master->take(CSI_LEN);
    output(0).push(p_master);

    WritablePacket *p_csi = Packet::make(CSI_LEN);
    memcpy(p_csi, p_in+(p_in->length()-CSI_LEN), CSI_LEN);

    if (noutputs() == 2)
    output(1).push(p_csi);
  else
    p_csi->kill();
}

void
CSISep::push(int, Packet *p)
{
	fragment(p);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(CSISep)
ELEMENT_MT_SAFE(CSISep)
