// -*- c-basic-offset: 4 -*-
#ifndef CLICK_WGTTQUEUE_HH
#define CLICK_WGTTQUEUE_HH
#include <click/element.hh>
#include <click/standard/storage.hh>
CLICK_DECLS


class WGTTQueue : public Element, public Storage { public:

    WGTTQueue() CLICK_COLD;

    int drops() const				{ return _drops; }
    int highwater_length() const		{ return _highwater_length; }

    inline bool enq(Packet*);
    inline void lifo_enq(Packet*);
    inline Packet* deq();

    // to be used with care
    Packet* packet(int i) const			{ return _q[i]; }
    void reset();				// NB: does not do notification

    template <typename Filter> Packet* yank1(Filter);
    template <typename Filter> Packet* yank1_peek(Filter);
    template <typename Filter> int yank(Filter, Vector<Packet *> &);

    const char *class_name() const		{ return "WGTTQueue"; }
    const char *port_count() const		{ return PORTS_1_1X2; }
    const char *processing() const		{ return "h/lh"; }
    void* cast(const char*);

    int configure(Vector<String>&, ErrorHandler*) CLICK_COLD;
    int initialize(ErrorHandler*) CLICK_COLD;
    void cleanup(CleanupStage) CLICK_COLD;
    bool can_live_reconfigure() const		{ return true; }
    int live_reconfigure(Vector<String>&, ErrorHandler*);
    void take_state(Element*, ErrorHandler*);
    void add_handlers() CLICK_COLD;

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

    static String read_handler(Element*, void*) CLICK_COLD;
    static int write_handler(const String&, Element*, void*, ErrorHandler*) CLICK_COLD;

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

template <typename Filter>
Packet *
WGTTQueue::yank1(Filter filter)
    /* Remove from the queue and return the first packet that matches
       'filter(Packet *)'. The returned packet must be deallocated by the
       caller. */
{
    for (Storage::index_type trav = head(); trav != tail(); trav = next_i(trav))
	if (filter(_q[trav])) {
	    Packet *p = _q[trav];
	    int prev = prev_i(trav);
	    while (trav != head()) {
		_q[trav] = _q[prev];
		trav = prev;
		prev = prev_i(prev);
	    }
	    set_head(next_i(head()));
	    return p;
	}
    return 0;
}

template <typename Filter>
Packet *
WGTTQueue::yank1_peek(Filter filter)
    /* return the first packet that matches
       'filter(Packet *)'. The returned packet must *NOT* be deallocated by the
       caller. */
{
    for (Storage::index_type trav = head(); trav != tail(); trav = next_i(trav))
	if (filter(_q[trav])) {
	    Packet *p = _q[trav];
	    return p;
	}
    return 0;
}

template <typename Filter>
int
WGTTQueue::yank(Filter filter, Vector<Packet *> &yank_vec)
    /* Removes from the queue and adds to 'yank_vec' all packets in the queue
       that match 'filter(Packet *)'. Packets are added to 'yank_vec' in LIFO
       order, so 'yank_vec.back()' will equal the first packet in the queue
       that matched 'filter()'. Caller should deallocate any packets returned
       in 'yank_vec'. Returns the number of packets yanked. */
{
    Storage::index_type write_ptr = tail();
    int nyanked = 0;
    for (Storage::index_type trav = tail(); trav != head(); ) {
	trav = prev_i(trav);
	if (filter(_q[trav])) {
	    yank_vec.push_back(_q[trav]);
	    nyanked++;
	} else {
	    write_ptr = prev_i(write_ptr);
	    _q[write_ptr] = _q[trav];
	}
    }
    set_head(write_ptr);
    return nyanked;
}

CLICK_ENDDECLS
#endif
