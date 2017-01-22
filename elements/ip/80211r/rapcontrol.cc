/*
 element for 802.11r ap, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "rapcontrol.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ip.h>
#include <math.h>

 CLICK_DECLS

int
RAPControl::configure(Vector<String> &conf, ErrorHandler *errh)
{
  int i,tmp_id;

  openlog("RAPControl", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);

  _ethh = new click_ether[MAX_N_CLIENT+1];//first MAX_N_CLIENT: ap->client; ap->controller
  if (Args(conf, this, errh)
      .read_p("IDENTITY", IntArg(), tmp_id)
      .read_p("FIRSTSTART1", IntArg(), tmp_start[0])
      .read_p("FIRSTSTART2", IntArg(), tmp_start[1])
      .read_p("FIRSTSTART3", IntArg(), tmp_start[2])
      .read_p("FIRSTSTART4", IntArg(), tmp_start[3])
      .complete() < 0)
    return -1;

  //down control pkt
  identity = tmp_id;
  for(i=0;i<MAX_N_CLIENT;i++)
  {
    first_start[i] = tmp_start[i];
    state[i] = (identity==tmp_start[i])? IDLE:INACTIVE;
    _ethh[i].ether_type = htons(ETHER_PROTO_BASE+CONTROL_SUFFIX);
    switch(identity-1)
    {
      case 0:cp_ethernet_address(AP1_MAC, _ethh[i].ether_shost);break;
      case 1:cp_ethernet_address(AP2_MAC, _ethh[i].ether_shost);break;
      case 2:cp_ethernet_address(AP3_MAC, _ethh[i].ether_shost);break;
      case 3:cp_ethernet_address(AP4_MAC, _ethh[i].ether_shost);break;
      case 4:cp_ethernet_address(AP5_MAC, _ethh[i].ether_shost);break;
      case 5:cp_ethernet_address(AP6_MAC, _ethh[i].ether_shost);break;
      case 6:cp_ethernet_address(AP7_MAC, _ethh[i].ether_shost);break;
      case 7:cp_ethernet_address(AP8_MAC, _ethh[i].ether_shost);break;
    }
    switch(i)
    {
      case 0:cp_ethernet_address(CLIENT1_MAC, _ethh[i].ether_dhost);break;
    }
  }

  _ethh[MAX_N_CLIENT].ether_type = htons(ETHER_PROTO_BASE+DATA_SUFFIX);
  switch(identity-1)
  {
    case 0:cp_ethernet_address(AP1_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 1:cp_ethernet_address(AP2_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 2:cp_ethernet_address(AP3_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 3:cp_ethernet_address(AP4_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 4:cp_ethernet_address(AP5_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 5:cp_ethernet_address(AP6_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 6:cp_ethernet_address(AP7_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
    case 7:cp_ethernet_address(AP8_MAC, _ethh[MAX_N_CLIENT].ether_shost);break;
  }
  switch(i)
  {
    case 0:cp_ethernet_address(CONTROLLER_IN_MAC, _ethh[MAX_N_CLIENT].ether_dhost);break;
  }

  syslog (LOG_DEBUG, "finish configure, ready to start\n");


  return 0;
}

void
RAPControl::push(int port, Packet *p_in)
{
  switch(port)
  {
    case 0: push_up_control(p_in);break;
    case 1: push_up_data(p_in);break;
    case 2: push_down_control(p_in);break;
    case 3: push_down_data(p_in);break;
  }

}


void RAPControl::push_up_control(Packet*p_in)
{
  int i;
  const unsigned char & t = r_control_type(p_in);
  const unsigned char & c = r_control_client(p_in);
  const unsigned char & ori = r_control_ori(p_in);
  const unsigned char & tar = r_control_tar(p_in);
  syslog (LOG_DEBUG, "AP %d receive up control: state %d, type %X, client %d, ap_ori %d, ap_tar %d\n", identity, state[c], t, c+1, ori+1, tar+1);
  //if(state[c] == IDLE && t == 0x04 && ori == identity - 1)
  if(t == 0x04)
  {
    control_content[0] = 0x05;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(4);
    // // data part
    memcpy(p->data(), &control_content, 4);

    syslog (LOG_DEBUG, "ap %d pass ant req for client %d, ap_ori %d, ap_tar %d\n", identity, c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(t == 0x0a) //(state[c] == INACTIVE && t == 0x0a && tar == identity - 1)
  {
    control_content[0] = 0x0b;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(4);
    // // data part
    memcpy(p->data(), &control_content, 4);

    syslog (LOG_DEBUG, "ap %d pass reas for client %d, ap_ori %d, ap_tar %d\n", identity, c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(t == 0xff)
  {

    for(i=0;i<MAX_N_CLIENT;i++)
    {
      state[i] = (identity==tmp_start[i])? IDLE:INACTIVE;
    }

    control_content[0] = 0xff;
    control_content[1] = 0xff;
    control_content[2] = 0xff;
    control_content[3] = 0xff;

    WritablePacket *p = Packet::make(4);
    // // data part
    memcpy(p->data(), &control_content, 4);

    syslog (LOG_DEBUG, "ap %d pass reset\n", identity);
    output(0).push(p);


  }
  p_in -> kill();
  
}

void RAPControl::push_down_control(Packet*p_in)
{
  int i;
  const unsigned char & t = r_control_type(p_in);
  const unsigned char & c = r_control_client(p_in);
  const unsigned char & ori = r_control_ori(p_in);
  const unsigned char & tar = r_control_tar(p_in);
  syslog (LOG_DEBUG, "AP %d receive down control: state %d, type %X, client %d, ap_ori %d, ap_tar %d\n", identity, state[c], t, c+1, ori+1, tar+1);

  if(state[c] == INACTIVE && t == 0x06 && tar == identity-1)
  {
    control_content[0] = 0x07;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(4);
    // // data part
    memcpy(p->data(), &control_content, 4);

    syslog (LOG_DEBUG, "ap %d send ant ack for client %d, ap_ori %d, ap_tar %d\n", identity, c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(state[c] == IDLE && t == 0x08 && ori == identity-1)
  {
    state[c] = INACTIVE;
    control_content[0] = 0x09;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), &(_ethh[c]), sizeof(click_ether));
    syslog (LOG_DEBUG, "ap %d pass ant ack for client %d, ap_ori %d, ap_tar %d\n", identity, c+1, ori+1, tar+1);
    output(2).push(p);
  }
  else if(state[c] == INACTIVE && t == 0x0c && tar == identity-1)
  {
    state[c] = IDLE;
    control_content[0] = 0x0d;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), &(_ethh[c]), sizeof(click_ether));
    syslog (LOG_DEBUG, "ap %d pass reas ack for client %d, ap_ori %d, ap_tar %d\n", identity, c+1, ori+1, tar+1);
    output(2).push(p);
  }
  else if(t == 0xff)
  {

    for(i=0;i<MAX_N_CLIENT;i++)
    {
      state[i] = (identity==tmp_start[i])? IDLE:INACTIVE;
    }

    control_content[0] = 0xff;
    control_content[1] = 0xff;
    control_content[2] = 0xff;
    control_content[3] = 0xff;

    syslog (LOG_DEBUG, "ap %d perform reset from controller\n", identity);

  }
  p_in -> kill();
  
}

void RAPControl::push_down_data(Packet*p_in)
{
  static unsigned char c;
  switch(r_dst_mac_suffix(p_in))
  {
    case CLIENT1_MAC_SUFFIX: c = 0;break;
    case CLIENT2_MAC_SUFFIX: c = 1;break;
    case CLIENT3_MAC_SUFFIX: c = 2;break;
    case CLIENT4_MAC_SUFFIX: c = 3;break;
  }
  if(state[c] == INACTIVE)
    p_in -> kill();
  else
    output(2).push(p_in);
}

void RAPControl::push_up_data(Packet*p_in)
{
  static unsigned char c;
  switch(r_src_mac_suffix(p_in))
  {
    case CLIENT1_MAC_SUFFIX: c = 0;break;
    case CLIENT2_MAC_SUFFIX: c = 1;break;
    case CLIENT3_MAC_SUFFIX: c = 2;break;
    case CLIENT4_MAC_SUFFIX: c = 3;break;
  }
  if(state[c] == INACTIVE)
    p_in -> kill();
  else
  {
    WritablePacket *p = p_in->uniqueify();
    p->push(sizeof(click_ether));
    memcpy(p->data(), &(_ethh[MAX_N_CLIENT]), sizeof(click_ether));
    output(1).push(p);
  }
}



CLICK_ENDDECLS
EXPORT_ELEMENT(RAPControl)
ELEMENT_MT_SAFE(RAPControl)
