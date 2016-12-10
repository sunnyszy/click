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
    _ethh = new click_ether[MAX_N_AP+1];
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

    
    _ethh->ether_type = htons(ETHER_PROTO_BASE+CONTROL_SUFFIX);
    // printf("identity: %X\n", identity);

    for(int i=0; i < MAX_N_AP+1; i++)
    {
        switch(identity)
        {
            case 1: cp_ethernet_address(AP1_MAC, _ethh[i].ether_shost);break;
            case 2: cp_ethernet_address(AP2_MAC, _ethh[i].ether_shost);break;
            case 3: cp_ethernet_address(AP3_MAC, _ethh[i].ether_shost);break;
            case 4: cp_ethernet_address(AP4_MAC, _ethh[i].ether_shost);break;
            case 5: cp_ethernet_address(AP5_MAC, _ethh[i].ether_shost);break;
            case 6: cp_ethernet_address(AP6_MAC, _ethh[i].ether_shost);break;
            case 7: cp_ethernet_address(AP7_MAC, _ethh[i].ether_shost);break;
            case 8: cp_ethernet_address(AP8_MAC, _ethh[i].ether_shost);break;
        }
    }

    assert(_head == 0 && _tail == 0);
    // printf("wgtt after !_q\n");
    if (_q == 0)
    return errh->error("out of memory");
    printf("wgtt initialize succeed, ready to start\n");
    return 0;
}



void
WGTTQueue::push(int, Packet *p_in)
{
    // printf("wgttQueue in push\n");
    switch(pkt_type(p_in))
    {
    case CONTROL_SUFFIX:  push_control(p_in);break;
    case DATA_SUFFIX:   push_data(p_in);break;
    }
}

void WGTTQueue::push_control(Packet *p_in)
{
    if(status_ap(p_in) == CONTROLLER_IN_MAC_SUFFIX)//stop
    {
        if(client_ip(p_in) == RESET_CONTENT)
        {
            printf("wgttQueue: receive reset req\n");
            _tail = 0;
            _head = 0;
            _block = (identity == 1)? false:true;
            for(unsigned int i=0;i<256;i++)
            {   
                if(_q[i] != 0)
                    _q[i] -> kill();
            }
        }
        else
        {
        printf("wgttQueue: receive switch req\n");
        _block = true;
        const unsigned char & dst_ap = start_ap(p_in);
        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // // data part
        control_content[0] = 0;
        control_content[1] = _head;
        printf("wgttQueue: switch id: %X\n", _head);
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        // //ip part
        memcpy(p->data(), &(_ethh[dst_ap]), sizeof(click_ether));

        p_in -> kill();
        printf("wgttQueue send ap-ap seq\n");
        checked_output_push(1, p);
        }
        
    }
    else
    {

        printf("wgttQueue receive ap-ap seq\n");
        // printf("wgttQueue _head: %X, _tail: %X\n", _head, _tail);
        // printf("wgttQueue in dering-prepare\n");
        const unsigned char & start_seq = start_seq(p_in);
        while(_head != start_seq)
        {   
            if(_q[_head] != 0)
                _q[_head] -> kill();
            _head = (_head+1)%RING_SIZE;
        }
        printf("wgttQueue finish ap-ap dequeue\n");
        

        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = 0;
        control_content[1] = 0;
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        
        //ether part
        memcpy(p->data(), &(_ethh[MAX_N_AP]), sizeof(click_ether));

        p_in -> kill();
        // printf("ap-c packet push\n");
        _block = false;
        printf("wgttQueue send switch ack\n");
        checked_output_push(1, p);
        
    }
}

void WGTTQueue::push_data(Packet *p_in)
{
    printf("wgttQueue in push data\n");
    const unsigned char & seq = queue_seq(p_in);
    while(_tail != seq)
    {
        // printf("wgttQueue in enring-prepare\n");
        // printf("wgttQueue _head: %X, _tail: %X\n", _head, _tail);
        enRing(0);
    }
    p_in -> pull(15);
    printf("wgttQueue after enring, _head: %X, _tail: %X\n", _head, _tail);
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
