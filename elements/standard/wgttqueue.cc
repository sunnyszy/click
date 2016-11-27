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
    : _q(0), dequeue_counter(0)
{
}



int
WGTTQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{
    unsigned new_capacity = 1000;
    int tmp_dequeue_time;
    if (Args(conf, this, errh)
        .read_p("CAPACITY", new_capacity)
        .read_p("DEQUEUETIME", tmp_dequeue_time)
        .complete() < 0)
	return -1;
    dequeue_time = tmp_dequeue_time;
    _capacity = new_capacity;
    return 0;
}



void
WGTTQueue::push(int, Packet *p)
{
    // If you change this code, also change NotifierQueue::push()
    // and FullNoteQueue::push().
    Storage::index_type h = head(), t = tail(), nt = next_i(t);

    // should this stuff be in SimpleQueue::enq?
    if (nt != h) {
	_q[t] = p;
	set_tail(nt);

	int s = size(h, nt);
	if (s > _highwater_length)
	    _highwater_length = s;
    // printf("enque\n");

    } else {
	// if (!(_drops % 100))
	if (_drops == 0 && _capacity > 0)
	    click_chatter("%p{element}: overflow", this);
	_drops++;
    // printf("overflow\n");
	checked_output_push(1, p);
    }
}

Packet *
WGTTQueue::pull(int)
{
    return deq();
}




CLICK_ENDDECLS
ELEMENT_PROVIDES(Storage)
EXPORT_ELEMENT(WGTTQueue)
