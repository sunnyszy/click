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
	_iph = (click_ip*)CLICK_LALLOC(sizeof(click_ip));
	
	printf("idadder init finish\n");


}


IDAdder::~IDAdder()
{

}

int
IDAdder::initialize(ErrorHandler *errh)
{

	memset(_iph, 0, sizeof(click_ip));
	_iph->ip_v = 4;
	_iph->ip_hl = sizeof(click_ip) >> 2;
	_iph->ip_ttl = 250;
	_iph->ip_src.s_addr = CONTROLLER_IP;
	_iph->ip_p = 17;//data msg   
	_iph->ip_tos = 0;
	_iph->ip_off = 0;
	_iph->ip_sum = 0;
	

    
    printf("idadder initial finish, ready to start\n");
    return 0;
}


void IDAdder::push(int port, Packet *p_in)
{
	// printf("idadder in push\n");
	static unsigned char tmp_counter = 0;
	if(tmp_counter ==0)
	{
		tmp_counter++;
		_ethh.ether_type = htons(0x0800);
		cp_ethernet_address(CONTROLLER_MAC, _ethh.ether_shost);
		
	}
	printf("IDadder: counter: %X\n", counter);


	_iph->ip_len = p_in->length()+sizeof(click_ip)+1;
	p_in->push(sizeof(click_ip)+sizeof(click_ether)+1);
	for(int i = 0;i<1;i++)
	{	
		Packet *p_tmp = p_in->clone();
		WritablePacket *p = p_tmp->uniqueify();

		if(i == 0)
		{
			_iph->ip_dst.s_addr = AP1_IP;
			cp_ethernet_address(AP1_MAC, _ethh.ether_dhost);
		}
		else
		{
			_iph->ip_dst.s_addr = AP2_IP;
			cp_ethernet_address(AP2_MAC, _ethh.ether_dhost);
		}
		memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &counter, 1);
		memcpy(p->data()+sizeof(click_ether), _iph, sizeof(click_ip));
		memcpy(p->data(), &_ethh, sizeof(click_ether));
		// printf("idadder push %dth\n", i);
		output(0).push(p);
	}

	WritablePacket *p = p_in->uniqueify();
	_iph->ip_dst.s_addr = AP2_IP;
	cp_ethernet_address(AP2_MAC, _ethh.ether_dhost);
	memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &counter, 1);
	memcpy(p->data()+sizeof(click_ether), _iph, sizeof(click_ip));
	memcpy(p->data(), &_ethh, sizeof(click_ether));
	counter ++;
	output(0).push(p);

	
  
}



CLICK_ENDDECLS
EXPORT_ELEMENT(IDAdder)
ELEMENT_MT_SAFE(IDAdder)
