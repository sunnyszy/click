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
#include "packetselectionSerial.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ip.h>
#include <math.h>

 CLICK_DECLS




PacketSelectionSerial::PacketSelectionSerial()
{
  int i,j;
  interval = 20;
  score = new unsigned char*[n_ap];
  next_score_id = new unsigned char[n_ap];
  output_port = new unsigned char[n_client];
  for(i=0; i<n_ap; i++)
  {
    score[i] = new unsigned char[n_compare];
    for(j=0; j<n_compare;j++)
    {
      score[i][j] = 200;//a quite small score
     
    }
    next_score_id[i] = 0;
  }
  
  for(i=0; i<n_client; i++)
  {
    output_port[i] = 0;
  }

  state = new unsigned char[n_client];
  state[0] = IDLE; 
  //TODO: checksum not set
  memset(&_iph, 0, sizeof(click_ip));
  _iph.ip_v = 4;
  _iph.ip_hl = sizeof(click_ip) >> 2;
  _iph.ip_ttl = 250;
  _iph.ip_src.s_addr = CONTROLLER_IP;
  _iph.ip_p = 27;//control msg   
  _iph.ip_tos = 0;
  _iph.ip_off = 0;
  _iph.ip_sum = 0;
  _iph.ip_len = htons(22);



  _ethh = new click_ether;
  
  _ethh->ether_type = htons(0x0800);
  bool result = cp_ethernet_address(CONTROLLER_MAC, _ethh->ether_shost);

  printf("Packetselection: init finish, ready to start\n");

}

PacketSelectionSerial::~PacketSelectionSerial()
{

}



int PacketSelectionSerial::configure(Vector<String> &conf, ErrorHandler *errh)
{
  printf("PacketSelectionSerial in\n");
  if (Args(conf, this, errh)
      .read_p("INTERVAL", IntArg(), interval)
      .complete() < 0)
    return -1;

  printf("PacketSelectionSerial out. interval: %X\n", interval);
  return 0;
}

void PacketSelectionSerial::push(int port, Packet *p_in)
{
  static unsigned char lock = 0;

  if(!lock)
  { 
    lock++;
    reset_ap();
  }

  // printf("pkt_type: %x\n", pkt_type(p_in));
  switch(pkt_type(p_in))
  {
    case CONTROL:  push_control(p_in);break;
    case STATUS:   push_status(p_in);break;
  }

}

void PacketSelectionSerial::reset_ap()
{
  for(int i=0;i<2;i++){ 
  WritablePacket *p = Packet::make(sizeof(click_ether)+sizeof(click_ip)+2);
  // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
  // // data part
  control_content[0] = 0xff;
  control_content[1] = 0xff;
  memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &control_content, 2);
  // //ip part
  _iph.ip_dst.s_addr = (i == 0)? AP1_IP : AP2_IP;

  memcpy(p->data()+sizeof(click_ether), &_iph, sizeof(click_ip));
  // p->set_ip_header(ip, sizeof(click_ip));
  //ether part
  if(i == 0)
    cp_ethernet_address(AP1_MAC, _ethh->ether_dhost);
  else
    cp_ethernet_address(AP2_MAC, _ethh->ether_dhost);
  memcpy(p->data(), _ethh, sizeof(click_ether));


  printf("controller reset ap %X\n", i);
  output(0).push(p);
  }
}


void PacketSelectionSerial::push_control(Packet *p_in)
{
  unsigned char c = 0;
  state[c] = IDLE;
  printf("switch request ack.\n");
  p_in -> kill();
}

void PacketSelectionSerial::push_status(Packet *p_in)
{
  //printf("In push status.\n");
  unsigned char a;
  //printf("ap id: %x\n", ap_id(p_in));
  switch(ap_id(p_in))
  {
    case AP1: a = 0; break;
    case AP2: a = 1; break;
  }
  // update_score(&ap_score(p_in), &c)
  // printf("ap id: %x, score: %x\n", ap_id(p_in), ap_score(p_in));
  // printf("next_score_id[a]: %x\n", next_score_id[a]);
  score[a][next_score_id[a]] = ap_score(p_in);
  next_score_id[a] = (next_score_id[a] + 1)%n_compare;
  // able to change state

  static unsigned int tmp_counter = 0;


  if(state[0] == IDLE)
  {
      tmp_counter++;
      // printf("state idle\n");
      // unsigned char best_ap = find_best_ap();

      // WGTT
      unsigned char best_ap = output_port[0];
      if(!(tmp_counter%interval))
          best_ap = 1 - output_port[0];

      if(best_ap != output_port[0])
      {
        // send message
        // send_meg(best_ap)
        WritablePacket *p = Packet::make(sizeof(click_ether)+sizeof(click_ip)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = 135;
        control_content[1] = best_ap;
        memcpy(p->data()+sizeof(click_ether)+sizeof(click_ip), &control_content, 2);
        // //ip part
        _iph.ip_dst.s_addr = (output_port[0] == 0)? AP1_IP : AP2_IP;

        memcpy(p->data()+sizeof(click_ether), &_iph, sizeof(click_ip));
        // p->set_ip_header(ip, sizeof(click_ip));
        //ether part
        if(output_port[0] == 0)
          cp_ethernet_address(AP1_MAC, _ethh->ether_dhost);
        else
          cp_ethernet_address(AP2_MAC, _ethh->ether_dhost);
        memcpy(p->data(), _ethh, sizeof(click_ether));


        printf("controller issu switch to ap %X\n", best_ap);
        state[0] = SWITCH_REQ;
        output_port[0] = best_ap;
        output(0).push(p);
        
      }
  }
  p_in -> kill();

}

// incomplete version, only for 2 ap and 1 client
unsigned char PacketSelectionSerial::find_best_ap()
{
  unsigned char &current = output_port[0];
  unsigned char potential = (current+1)%2;
  bool flip_flag = true;
  int i;

  for(i=0; i<n_compare; i++)
    if(score[potential][n_compare-i-1]>=score[current][i])
    {
      flip_flag = false;
      break;
    }

  if(flip_flag)
    return potential;
  else
    return current;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketSelectionSerial)
ELEMENT_MT_SAFE(PacketSelectionSerial)
