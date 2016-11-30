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
#include "wgttqueue.hh"
#include <click/args.hh>
#include <click/error.hh>
CLICK_DECLS

WGTTQueue::WGTTQueue()
{
    _head = 0;
    _tail = 0;
    _iph = (click_ip*)CLICK_LALLOC(sizeof(click_ip));
    _ethh = (click_ether*)CLICK_LALLOC(sizeof(click_ether));
    _q = (Packet **) CLICK_LALLOC(sizeof(Packet *) * RING_SIZE);
    printf("wgtt init succeed\n");
}



int
WGTTQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{
    //printf("In configure\n");
    if (Args(conf, this, errh)
        .read_p("IDENTITY", IntArg(), identity)
        .complete() < 0)
    return -1;
    

    printf("wgtt configure succeed\n");
    return 0;
}

int
WGTTQueue::initialize(ErrorHandler *errh)
{
    //printf("wgtt in initialize\n");


    if(identity==1)
        _block = false;
    else
        _block = true;

    // printf("After configure _block\n");
    //TODO: checksum not set
    
    memset(_iph, 0, sizeof(click_ip));
    _iph->ip_v = 4;
    _iph->ip_hl = sizeof(click_ip) >> 2;
    _iph->ip_ttl = 250;
    switch(identity)
    {
    case 1: _iph->ip_src.s_addr = AP1_IP;break;
    case 2: _iph->ip_src.s_addr = AP2_IP;break;
    }
    _iph->ip_p = 27;//control msg   
    _iph->ip_tos = 0;
    _iph->ip_off = 0;
    _iph->ip_sum = 0;
    _iph->ip_len = htons(22);
    
    _ethh->ether_type = htons(0x0800);
    // printf("identity: %X\n", identity);
    switch(identity)
    {
        case 1: cp_ethernet_address(AP1_MAC, _ethh->ether_shost);break;
        case 2: cp_ethernet_address(AP2_MAC, _ethh->ether_shost);break;
    }

    assert(_head == 0 && _tail == 0);
    // printf("wgtt after !_q\n");
    if (_q == 0)
    return errh->error("out of memory");
    // printf("wgtt initialize succeed, ready to start\n");
    return 0;
}



void
WGTTQueue::push(int, Packet *p_in)
{
    // printf("wgttQueue in push\n");
    switch(pkt_type(p_in))
    {
    case CONTROL:  push_control(p_in);break;
    case DATA:   push_data(p_in);break;
    }
}

void WGTTQueue::push_control(Packet *p_in)
{
    if(ap_id(p_in) == CONTROLLER)//stop
    {
        // printf("wgttQueue: receive switch req\n");
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
            case 0: _iph->ip_dst.s_addr = AP1_IP;cp_ethernet_address(AP1_MAC, _ethh->ether_dhost);break;
            case 1: _iph->ip_dst.s_addr = AP2_IP;cp_ethernet_address(AP2_MAC, _ethh->ether_dhost);break;
        }
        memcpy(p->data()+sizeof(click_ether), _iph, sizeof(click_ip));
        // p->set_ip_header(ip, sizeof(click_ip));
        //ether part
        memcpy(p->data(), _ethh, sizeof(click_ether));

        p_in -> kill();
        // printf("wgttQueue send ap-ap seq\n");
        checked_output_push(1, p);
        
    }
    else
    {

        // printf("wgttQueue receive ap-ap seq\n");
        // printf("wgttQueue _head: %X, _tail: %X\n", _head, _tail);
        // printf("wgttQueue in dering-prepare\n");
        const unsigned char & start_seq = start_seq(p_in);
        while(_head != start_seq)
        {   
            if(_q[_head] != 0)
                _q[_head] -> kill();
            _head = (_head+1)%RING_SIZE;
        }

        

        WritablePacket *p = Packet::make(sizeof(click_ether)+sizeof(click_ip)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = 135;
        control_content[1] = 0;
        memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &control_content, 2);
        // //ip part
        _iph->ip_dst.s_addr = CONTROLLER_IP;cp_ethernet_address(CONTROLLER_MAC, _ethh->ether_dhost);

        memcpy(p->data()+sizeof(click_ether), _iph, sizeof(click_ip));
        // p->set_ip_header(ip, sizeof(click_ip));
        //ether part
        memcpy(p->data(), _ethh, sizeof(click_ether));

        p_in -> kill();
        // printf("ap-c packet push\n");
        _block = false;
        // printf("wgttQueue send switch ack\n");
        checked_output_push(1, p);
        
    }
}

void WGTTQueue::push_data(Packet *p_in)
{
    // printf("wgttQueue in push data\n");
    const unsigned char & seq = seq(p_in);
    while(_tail != seq)
    {
        // printf("wgttQueue in enring-prepare\n");
        // printf("wgttQueue _head: %X, _tail: %X\n", _head, _tail);
        enRing(0);
    }
    p_in -> pull(21);
    // printf("wgttQueue enring, _head: %X, _tail: %X\n", _head, _tail);
    enRing(p_in);
}

Packet *
WGTTQueue::pull(int port)
{
    return deRing();
}




CLICK_ENDDECLS
ELEMENT_PROVIDES(Storage)
EXPORT_ELEMENT(WGTTQueue)
