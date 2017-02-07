/*
 Controller program, generate simple data and switch control
 Input: trigger packet
 Output: control & data
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "seqgen.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <math.h>

CLICK_DECLS

SeqGen::SeqGen()
{
  openlog("SeqGen", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);

  _ethh = new click_ether;
  _ethh->ether_type = htons(CONTROL_SUFFIX+ETHER_PROTO_BASE);
  cp_ethernet_address(CONTROLLER_IN_MAC, _ethh->ether_shost);
  output_port = 3;
  counter = 0;

  syslog (LOG_DEBUG, "Init finish, ready to start\n");
}

int SeqGen::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (Args(conf, this, errh)
      .read_p("SWITCHINTERVAL", IntArg(), interval)
      .complete() < 0)
    return -1;

  syslog (LOG_DEBUG, "Finish configure. Switch interval: %d\n", interval);
  return 0;
}

void SeqGen::push(int port, Packet *p_in)
{
  // here is a small bug, I can not put the reset function in the initial function
  static unsigned char lock = 0;
  if(!lock)
  { 
    lock++;
    reset_ap();
  }

  // syslog (LOG_DEBUG, "pkt_type: %x\n", pkt_type(p_in));
  push_status(p_in);

}

void SeqGen::reset_ap()
{
  control_content[0] = RESET_CONTENT;
  control_content[1] = RESET_CONTENT;
  for(int i=0;i<MAX_N_AP;i++){ 
  WritablePacket *p = Packet::make(sizeof(click_ether)+2);
  // // data part
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

  syslog (LOG_DEBUG, "Controller reset ap %X\n", i+1);
  output(1).push(p);
  }
}

void SeqGen::push_status(Packet *p_in)
{
  const unsigned char a = status_ap(p_in) - 1;
  unsigned char c = 0;

  

  p_in -> kill();
  // syslog (LOG_DEBUG, "state idle\n");
  unsigned char best_ap;
  // syslog (LOG_DEBUG, "current_state: %d, time_lock: %d\n", state[c], time_lock[c]);
  // WGTT
  counter++;
  
  if(!(counter%interval))
  {
    best_ap = output_port;
    best_ap = 7 - best_ap; //only switch between 3,4
    syslog (LOG_DEBUG, "prepare manually switch to ap %X\n", best_ap+1);
  
    // send_meg(best_ap)
    WritablePacket *p = Packet::make(sizeof(click_ether)+2);
    // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
    // // data part
    control_content[0] = CLIENT1_IP_SUFFIX + c;
    control_content[1] = best_ap;
    memcpy(p->data()+sizeof(click_ether), &control_content, 2);
    //ether part
    switch(output_port)
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

    syslog (LOG_DEBUG, "issu switch. for client: %d to ap: %d\n", c+1, best_ap+1);
    output_port = best_ap;

    output(1).push(p);
  }

  //data packet
  WritablePacket *p_data = Packet::make(1);
  memcpy(p_data->data(), &counter, 1);
  
    //syslog (LOG_DEBUG, "issu switch. for client: %d to ap: %d\n", c+1, best_ap+1);
    //output_port = best_ap;
  output(0).push(p_data);

}



CLICK_ENDDECLS
EXPORT_ELEMENT(SeqGen)
ELEMENT_MT_SAFE(SeqGen)
