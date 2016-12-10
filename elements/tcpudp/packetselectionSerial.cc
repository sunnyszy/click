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
#include <math.h>

 CLICK_DECLS




PacketSelectionSerial::PacketSelectionSerial()
{
  int i,j;
  interval = 20;
  score = new int*[N_AP];
  next_score_id = new unsigned char[N_AP];
  output_port = new unsigned char[N_CLIENT];
  for(i=0; i<N_AP; i++)
  {
    score[i] = new int[n_compare];
    for(j=0; j<n_compare;j++)
    {
      score[i][j] = 200;//a quite small score
     
    }
    next_score_id[i] = 0;//a pointer
  }

  state = new unsigned char[N_CLIENT];
  state[0] = IDLE; 

  _ethh = new click_ether;
  
  _ethh->ether_type = htons(CONTROL_SUFFIX+ETHER_PROTO_BASE);
  bool result = cp_ethernet_address(CONTROLLER_IN_MAC, _ethh->ether_shost);

  printf("Packetselection: init finish, ready to start\n");

}

PacketSelectionSerial::~PacketSelectionSerial()
{

}



int PacketSelectionSerial::configure(Vector<String> &conf, ErrorHandler *errh)
{
  int i;
  printf("PacketSelectionSerial in\n");
  if (Args(conf, this, errh)
      .read_p("INTERVAL", IntArg(), interval)
      .read_p("FIRSTSTART", IntArg(), first_start)
      .read_p("PRINTINTERVAL", IntArg(), print_interval)
      .complete() < 0)
    return -1;

  for(i=0; i<N_CLIENT; i++)
  {
    output_port[i] = first_start-1;
  }

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
    case CONTROL_SUFFIX:  push_control(p_in);break;
    case STATUS_SUFFIX:   push_status(p_in);break;
  }

}

void PacketSelectionSerial::reset_ap()
{
  for(int i=0;i<N_AP;i++){ 
  WritablePacket *p = Packet::make(sizeof(click_ether)+2);
  // // data part
  control_content[0] = RESET_CONTENT;
  control_content[1] = RESET_CONTENT;
  memcpy(p->data()+sizeof(click_ether), &control_content, 2);
  //ether part
  switch(i)
  {
    case 0:cp_ethernet_address(AP1_MAC, _ethh->ether_dhost);break;
    case 1:cp_ethernet_address(AP2_MAC, _ethh->ether_dhost);break;
    case 2:cp_ethernet_address(AP3_MAC, _ethh->ether_dhost);break;
    case 3:cp_ethernet_address(AP4_MAC, _ethh->ether_dhost);break;
    case 4:cp_ethernet_address(AP5_MAC, _ethh->ether_dhost);break;
    case 5:cp_ethernet_address(AP6_MAC, _ethh->ether_dhost);break;
    case 6:cp_ethernet_address(AP7_MAC, _ethh->ether_dhost);break;
    case 7:cp_ethernet_address(AP8_MAC, _ethh->ether_dhost);break;
  }
  
  memcpy(p->data(), _ethh, sizeof(click_ether));
  printf("controller reset ap %X\n", i);
  output(0).push(p);
  }
}


void PacketSelectionSerial::push_control(Packet *p_in)
{
  const unsigned char & c = client_ip(p_in);
  printf("switch request ack ip: %X.\n", c);
  if(c == CLIENT1_IP_SUFFIX)
  {
      printf("PacketSelection: set client %X state to IDLE\n", c);
      state[c-CLIENT1_IP_SUFFIX] = IDLE;
  }
  else
  {
      printf("PacketSelection: I didn't find the client you ack\n");
  }
  
  printf("switch request ack.\n");
  p_in -> kill();
}

void PacketSelectionSerial::push_status(Packet *p_in)
{
  //printf("In push status.\n");
  unsigned char a = status_ap(p_in) - 1;
  // printf("ap id: %x, score: %x\n", ap_id(p_in), ap_score(p_in));
  // printf("next_score_id[a]: %x\n", next_score_id[a]);
  //since the score are minus, we minus again
  score[a][next_score_id[a]] = - status_score(p_in);
  next_score_id[a] = (next_score_id[a] + 1)%n_compare;
  // able to change state

  static unsigned int tmp_counter = 0;


  if(state[0] == IDLE)
  {
      tmp_counter++;
      if(!(tmp_counter%print_interval))
      {
        printf("ap id: %X\n", status_ap(p_in));
        // printf("current mac: %X\n", status_mac(p_in));
        printf("current score: %d\n", status_score(p_in));
        printf("current noise: %d\n", status_noise(p_in));
        printf("current rx_rate: %d\n", status_rxrate(p_in));
        printf("current tx_rate: %d\n", status_txrate(p_in));
      }
      // printf("state idle\n");
      unsigned char best_ap = find_best_ap();

      // WGTT
      if(interval>0)
      {
        best_ap = output_port[0];
        if(!(tmp_counter%interval))
        {
            best_ap = (best_ap + 1)% N_AP;
            printf("prepare manually switch to ap %X\n", best_ap+1);
        }
      }

      if(best_ap != output_port[0])
      {
        // send message
        // send_meg(best_ap)
        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = CLIENT1_IP_SUFFIX;
        control_content[1] = best_ap;
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        //ether part
        switch(output_port[0])
        {
          case 0:cp_ethernet_address(AP1_MAC, _ethh->ether_dhost);break;
          case 1:cp_ethernet_address(AP2_MAC, _ethh->ether_dhost);break;
          case 2:cp_ethernet_address(AP3_MAC, _ethh->ether_dhost);break;
          case 3:cp_ethernet_address(AP4_MAC, _ethh->ether_dhost);break;
          case 4:cp_ethernet_address(AP5_MAC, _ethh->ether_dhost);break;
          case 5:cp_ethernet_address(AP6_MAC, _ethh->ether_dhost);break;
          case 6:cp_ethernet_address(AP7_MAC, _ethh->ether_dhost);break;
          case 7:cp_ethernet_address(AP8_MAC, _ethh->ether_dhost);break;
        }
        memcpy(p->data(), _ethh, sizeof(click_ether));


        printf("controller issu switch to ap %X\n", best_ap+1);
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
  
  // unsigned char potential = (current+1)%2;
  bool switch_to_left = true, switch_to_right = true;
  int j;

  // find potential, can only be left 1 or right one
  if(current==0)
    switch_to_left = false;
  else if(current == N_AP-1)
    switch_to_right = false;

  if(switch_to_left)
  {
    for(j=0; j<n_compare; j++)
      if(score[current-1][n_compare-j-1]>=score[current][j])
      {
        switch_to_left = false;
        break;
      }
  }
  if(switch_to_right)
  {
    for(j=0; j<n_compare; j++)
      if(score[current+1][n_compare-j-1]>=score[current][j])
      {
        switch_to_right = false;
        break;
      }
  }
  if(switch_to_left && switch_to_right)
  {
    int sum_left = 0, sum_right = 0;
    for(j=0; j<n_compare; j++)
    {
      sum_left += score[current-1][j];
      sum_right += score[current+1][j];
    }
    if(sum_left <= sum_right)
      switch_to_right = false;
    else
      switch_to_left = false;
  }
  

  if(switch_to_left)
    return current-1;
  else if(switch_to_right)
    return current+1;
  else
    return current;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketSelectionSerial)
ELEMENT_MT_SAFE(PacketSelectionSerial)
