// -*- c-basic-offset: 4 -*-
/*
 * simplequeue.{cc,hh} -- queue element
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
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
#include "apswitchserial.hh"
#include <click/args.hh>
#include <click/error.hh>
CLICK_DECLS

APSwitchSerial::APSwitchSerial()
{
    _q = new Packet*[RING_SIZE];
    _head = 0;
    _tail = 0;
    _qControl = new Packet*[QUEUE_SIZE];
    _headControl = 0;
    _tailControl = 0;
}

APSwitchSerial::~APSwitchSerial()
{

}


int
APSwitchSerial::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (Args(conf, this, errh)
        .read_p("IDENTITY", IntArg(), identity)
        .complete() < 0)
	return -1;
    if(identity==1)
        _block = false;
    else
        _block = true;


    //TODO: checksum not set
    memset(&_iph, 0, sizeof(click_ip));
    _iph.ip_v = 4;
    _iph.ip_hl = sizeof(click_ip) >> 2;
    _iph.ip_ttl = 250;
    switch(identity)
    {
    case 1: _iph.ip_src.s_addr = AP1_IP;break;
    case 2: _iph.ip_src.s_addr = AP2_IP;break;
    }
    _iph.ip_p = 6;//control msg   
    _iph.ip_tos = 0;
    _iph.ip_off = 0;
    _iph.ip_sum = 0;
    _iph.ip_len = htons(22);


    _ethh = new click_ether;
  
    _ethh->ether_type = htons(0x0800);
    switch(identity)
    {
        case 1: cp_ethernet_address(AP1_MAC, _ethh->ether_shost);break;
        case 2: cp_ethernet_address(AP2_MAC, _ethh->ether_shost);break;
    }

    return 0;
}

void
APSwitchSerial::push(int, Packet *p_in)
{
    switch(pkt_type(p_in))
    {
    case CONTROL:  push_control(p_in);break;
    case DATA:   push_data(p_in);break;
    }
}


void APSwitchSerial::push_control(Packet *p_in)
{
    if(ap_id(p_in) == CONTROLLER)//stop
    {
        _block = true;
        const unsigned char & dst_ap_id = start_ap(p_in);

        WritablePacket *p = Packet::make(sizeof(click_ether)+sizeof(click_ip)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = 135;
        control_content[1] = _head;
        memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &control_content, 2);
        // //ip part
        switch(dst_ap_id)
        {
            case 1: _iph.ip_dst.s_addr = AP1_IP;cp_ethernet_address(AP1_MAC, _ethh->ether_dhost);break;
            case 2: _iph.ip_dst.s_addr = AP2_IP;cp_ethernet_address(AP2_MAC, _ethh->ether_dhost);break;
        }
        memcpy(p->data()+sizeof(click_ether), &_iph, sizeof(click_ip));
        // p->set_ip_header(ip, sizeof(click_ip));
        //ether part
        memcpy(p->data(), _ethh, sizeof(click_ether));

        enque(p);
        printf("ap2ap packet push\n");
    }
    else
    {
        const unsigned char & start_seq = start_seq(p_in);
        while(_head != start_seq)
            deRing();

        

        WritablePacket *p = Packet::make(sizeof(click_ether)+sizeof(click_ip)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = 135;
        control_content[1] = 0;
        memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &control_content, 2);
        // //ip part
        _iph.ip_dst.s_addr = CONTROLLER_IP;cp_ethernet_address(CONTROLLER_MAC, _ethh->ether_dhost);

        memcpy(p->data()+sizeof(click_ether), &_iph, sizeof(click_ip));
        // p->set_ip_header(ip, sizeof(click_ip));
        //ether part
        memcpy(p->data(), _ethh, sizeof(click_ether));

        enque(p);
        printf("ap-c packet push\n");
        _block = false;

    }
}

void APSwitchSerial::push_data(Packet *p_in)
{
    unsigned char seq = start_seq(p_in);
    p_in -> take(1);
    while(_tail != seq)
        enRing(0);
    enRing(p_in);
}




Packet *
APSwitchSerial::pull(int port)
{
    if(port == 0)
        return deRing();
    else
        return deque();
}



CLICK_ENDDECLS
EXPORT_ELEMENT(APSwitchSerial)
ELEMENT_MT_SAFE(APSwitchSerial)
