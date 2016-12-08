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
	// printf("idadder in init\n");
	counter = 0;
	
	printf("idadder init finish\n");


}


IDAdder::~IDAdder()
{

}

int
IDAdder::initialize(ErrorHandler *errh)
{
    
    printf("idadder initial finish, ready to start\n");
    return 0;
}


void IDAdder::push(int port, Packet *p_in)
{
	// printf("idadder in push\n");
	static bool lock = false;
	if(!false)
	{
		lock = true;
		_ethh.ether_type = htons(ETHER_PROTO_BASE+CONTROL_SUFFIX);
		cp_ethernet_address(CONTROLLER_IN_MAC, _ethh.ether_shost);
	}
	// printf("IDadder: counter: %X\n", counter);

	p_in->push(sizeof(click_ether)+1);
	for(int i = 1;i<N_AP;i++)
	{	
		Packet *p_tmp = p_in->clone();
		WritablePacket *p = p_tmp->uniqueify();

		switch(i)
        {
          case 0:cp_ethernet_address(AP0_MAC, _ethh.ether_dhost);break;
          case 1:cp_ethernet_address(AP1_MAC, _ethh.ether_dhost);break;
          case 2:cp_ethernet_address(AP2_MAC, _ethh.ether_dhost);break;
        }

		memcpy(p->data()+sizeof(click_ether), &counter, 1);
		memcpy(p->data(), &_ethh, sizeof(click_ether));
		// printf("idadder push %dth\n", i);
		output(0).push(p);
	}

	WritablePacket *p = p_in->uniqueify();
	cp_ethernet_address(AP0_MAC, _ethh.ether_dhost);
	memcpy(p->data()+sizeof(click_ether), &counter, 1);
	memcpy(p->data(), &_ethh, sizeof(click_ether));
	counter ++;
	output(0).push(p);
}



CLICK_ENDDECLS
EXPORT_ELEMENT(IDAdder)
ELEMENT_MT_SAFE(IDAdder)
