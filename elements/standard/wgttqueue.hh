// -*- c-basic-offset: 4 -*-
#ifndef CLICK_WGTTQUEUE_HH
#define CLICK_WGTTQUEUE_HH
#include <click/element.hh>
#include <click/standard/storage.hh>
CLICK_DECLS


class WGTTQueue : public Element, public Storage { public:

    WGTTQueue() CLICK_COLD;

    

    inline bool enq(Packet*);
    inline void lifo_enq(Packet*);
    inline Packet* deq();


    const char *class_name() const		{ return "WGTTQueue"; }
    const char *port_count() const		{ return "1/2"; }
    const char *processing() const		{ return "h/lh"; }

    int configure(Vector<String>&, ErrorHandler*) CLICK_COLD;

    void push(int port, Packet*);
    Packet* pull(int port);

  protected:

    Packet* volatile * _q;
    volatile int dequeue_time;
    volatile int dequeue_counter;
    volatile int _drops;
    int _highwater_length;

    friend class MixedQueue;
    friend class TokenQueue;
    friend class InOrderQueue;
    friend class ECNQueue;
};


inline bool
WGTTQueue::enq(Packet *p)
{
    assert(p);
    Storage::index_type h = head(), t = tail(), nt = next_i(t);
    if (nt != h) {
	_q[t] = p;
	set_tail(nt);
	int s = size(h, nt);
	if (s > _highwater_length)
	    _highwater_length = s;
	return true;
    } else {
	p->kill();
	_drops++;
	return false;
    }
}

inline void
WGTTQueue::lifo_enq(Packet *p)
{
    // XXX NB: significantly more dangerous in a multithreaded environment
    // than plain (FIFO) enq().
    assert(p);
    Storage::index_type h = head(), t = tail(), ph = prev_i(h);
    if (ph == t) {
	t = prev_i(t);
	_q[t]->kill();
	set_tail(t);
    }
    _q[ph] = p;
    set_head_release(ph);
}

inline Packet *
WGTTQueue::deq()
{
    if(dequeue_counter >= dequeue_time)
        return 0;
    Storage::index_type h = head(), t = tail();
    if (h != t) {
	Packet *p = _q[h];
	set_head(next_i(h));
	assert(p);
    if (dequeue_counter == dequeue_time - 1)
    {
    }
    dequeue_counter ++;
    
	return p;
    } else
    {
        // printf("deque function\n");
	   return 0;
    }
}


CLICK_ENDDECLS
#endif
