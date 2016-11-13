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
#include "packetselection.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ip.h>
 CLICK_DECLS

PacketSelection::PacketSelection()
{
  int i;
  score = new double[n_outport];
  early_counter = new int[n_outport];
  for(i=0; i<n_outport; i++)
  {
    score[i] = 0;
    early_counter[i] = 0;
  }
}

PacketSelection::~PacketSelection()
{

}



int
PacketSelection::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (Args(conf, this, errh)
      .read_p("ALPHA", DoubleArg(), alpha)
      .complete() < 0)
    return -1;

  return 0;
}

void
PacketSelection::push(int port, Packet *p_in)
{
  if(port<n_outport)
    state_change(port, p_in);
  else
    destination_change(p_in);

}

int
PacketSelection::bit_convert(int data, int maxbit)
{
    if ( data & (1 << (maxbit - 1)))
    {
        /* negative */
        data -= (1 << maxbit);    
    }
    return data;
}

double 
PacketSelection::csi_get_score(Packet *p_in)
{
// －－－－－－－－－－
    uint8_t bits_left;
    uint32_t bitmask, idx, current_data, h_data;
    int real,imag;
    uint8_t* csi_addr = (unsigned char *)p_in->data();
    /* init bits_left
     * we process 16 bits at a time*/
    bits_left = 16; 

    /* according to the h/w, we have 10 bit resolution 
     * for each real and imag value of the csi matrix H 
     */
    bitmask = (1 << 10) - 1;
    idx = 0;
    /* get 16 bits for processing */
    h_data = csi_addr[idx++];
    h_data += (csi_addr[idx++] << 8);
    current_data = h_data & ((1 << 16) -1);
    
    /* bits number less than 10, get next 16 bits */
    if((bits_left - 10) < 0){
        h_data = csi_addr[idx++];
        h_data += (csi_addr[idx++] << 8);
        current_data += h_data << bits_left;
        bits_left += 16;
    }
    
    imag = current_data & bitmask;
    imag = bit_convert(imag, 10);

    bits_left -= 10;
    current_data = current_data >> 10;
    
    /* bits number less than 10, get next 16 bits */
    if((bits_left - 10) < 0){
        h_data = csi_addr[idx++];
        h_data += (csi_addr[idx++] << 8);
        current_data += h_data << bits_left;
        bits_left += 16;
    }

    real = current_data & bitmask;
    real = bit_convert(real,10);
    return (real^2+imag^2);
}




void
PacketSelection::state_change(int port, Packet *p_in)
{
  WritablePacket *p = p_in->uniqueify();
  struct click_ip *iph = p->ip_header();


  double csi_score = csi_get_score(p);


  if(early_counter[port]<fresh_time)
    early_counter[port]++;
  score[port] = alpha*csi_score + (1-alpha)*score[port];
  p -> kill();

}

void
PacketSelection::destination_change(Packet *p_in)
{
  double tmp_score[n_outport];
  int i, max_id;
  double max = -99;

  // mask score before fresh time
  for(i=0;i<n_outport;i++)
  {
    if(early_counter[i]<fresh_time)
      tmp_score[i] = 0;
    else
      tmp_score[i] = score[i];
  }

  for(i=0;i<n_outport;i++)
  {
    if(tmp_score[i]>max)
    {
      max = tmp_score[i];
      max_id = i;
    }
  }

  output(max_id).push(p_in);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketSelection)
ELEMENT_MT_SAFE(PacketSelection)
