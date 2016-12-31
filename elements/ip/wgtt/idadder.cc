/*
 idadder program, add id and push for queue ring in ap processing. 
 Input: [data pkt]
 Output:[eth][id][data pkt], id is one Byte
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "idadder.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>

CLICK_DECLS

IDAdder::IDAdder()
{
	int i;
	for(i=0; i<MAX_N_CLIENT; i++)
		counter[i] = 0;
	syslog (LOG_DEBUG, "idadder init finish\n");
}

void IDAdder::push(int port, Packet *p_in)
{
	// a bug here, can not set ether net type, src at initialization
	static bool lock = false;
	if(!false)
	{
		lock = true;
		_ethh.ether_type = htons(ETHER_PROTO_BASE+DATA_SUFFIX);
		cp_ethernet_address(CONTROLLER_IN_MAC, _ethh.ether_shost);
	}
	// syslog (LOG_DEBUG, "IDadder: counter: %X\n", counter);

	p_in->push(sizeof(click_ether)+1);
	for(int i = 1;i<MAX_N_AP;i++)
	{	
		Packet *p_tmp = p_in->clone();
		WritablePacket *p = p_tmp->uniqueify();
		// data
		memcpy(p->data()+sizeof(click_ether), &counter[port], 1);
		// eth

		switch(i)
		{
			case 0:cp_ethernet_address(AP1_MAC, _ethh.ether_dhost);break;
			case 1:cp_ethernet_address(AP2_MAC, _ethh.ether_dhost);break;
			case 2:cp_ethernet_address(AP3_MAC, _ethh.ether_dhost);break;
			case 3:cp_ethernet_address(AP4_MAC, _ethh.ether_dhost);break;
			case 4:cp_ethernet_address(AP5_MAC, _ethh.ether_dhost);break;
			case 5:cp_ethernet_address(AP6_MAC, _ethh.ether_dhost);break;
			case 6:cp_ethernet_address(AP7_MAC, _ethh.ether_dhost);break;
			case 7:cp_ethernet_address(AP8_MAC, _ethh.ether_dhost);break;
		}
		memcpy(p->data(), &_ethh, sizeof(click_ether));
		// syslog (LOG_DEBUG, "idadder push %dth\n", i);
		output(0).push(p);
	}
	WritablePacket *p = p_in->uniqueify();
	//data
	memcpy(p->data()+sizeof(click_ether), &counter[port], 1);
	//eth
	cp_ethernet_address(AP1_MAC, _ethh.ether_dhost);
	memcpy(p->data(), &_ethh, sizeof(click_ether));
	counter[port]++;
	output(0).push(p);
}



CLICK_ENDDECLS
EXPORT_ELEMENT(IDAdder)
ELEMENT_MT_SAFE(IDAdder)
