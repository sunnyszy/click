/*
 element for 802.11r ap, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "rcontrollercontrol.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ip.h>
#include <math.h>

 CLICK_DECLS

int
RControlerControl::configure(Vector<String> &conf, ErrorHandler *errh)
{
  int i;
  openlog("RControlerControl", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);
  _ethh = new click_ether[MAX_N_AP*2];
  if (Args(conf, this, errh)
      .read_p("FIRSTSTART1", IntArg(), tmp_start[0])
      .read_p("FIRSTSTART2", IntArg(), tmp_start[1])
      .read_p("FIRSTSTART3", IntArg(), tmp_start[2])
      .read_p("FIRSTSTART4", IntArg(), tmp_start[3])
      .complete() < 0)
    return -1;

  for(i=0;i<MAX_N_CLIENT;i++)
  {
    outport[i] = tmp_start[i] - 1;
  }


  for(i=0;i<MAX_N_AP;i++)//control pkt
  {
    _ethh[i].ether_type = htons(ETHER_PROTO_BASE+CONTROL_SUFFIX);
    cp_ethernet_address(CONTROLLER_IN_MAC, _ethh[i].ether_shost);
    switch(i)
    {
            case 0:cp_ethernet_address(AP1_MAC, _ethh[i].ether_dhost);break;
            case 1:cp_ethernet_address(AP2_MAC, _ethh[i].ether_dhost);break;
            case 2:cp_ethernet_address(AP3_MAC, _ethh[i].ether_dhost);break;
            case 3:cp_ethernet_address(AP4_MAC, _ethh[i].ether_dhost);break;
            case 4:cp_ethernet_address(AP5_MAC, _ethh[i].ether_dhost);break;
            case 5:cp_ethernet_address(AP6_MAC, _ethh[i].ether_dhost);break;
            case 6:cp_ethernet_address(AP7_MAC, _ethh[i].ether_dhost);break;
            case 7:cp_ethernet_address(AP8_MAC, _ethh[i].ether_dhost);break;
    }
  }

  for(i=0;i<MAX_N_AP;i++)//data pkt
  {
    _ethh[i+MAX_N_AP].ether_type = htons(ETHER_PROTO_BASE+DATA_SUFFIX);
    cp_ethernet_address(CONTROLLER_IN_MAC, _ethh[i+MAX_N_AP].ether_shost);
    switch(i)
    {
            case 0:cp_ethernet_address(AP1_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 1:cp_ethernet_address(AP2_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 2:cp_ethernet_address(AP3_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 3:cp_ethernet_address(AP4_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 4:cp_ethernet_address(AP5_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 5:cp_ethernet_address(AP6_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 6:cp_ethernet_address(AP7_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
            case 7:cp_ethernet_address(AP8_MAC, _ethh[i+MAX_N_AP].ether_dhost);break;
    }
  }
  syslog (LOG_DEBUG, "finish configure, ready to start\n");

  return 0;
}

void
RControlerControl::push(int port, Packet *p_in)
{
  switch(port)
  {
    case 0: push_up_control(p_in);break;
    default: push_down_data(p_in, port);break;
  }

}


void RControlerControl::push_up_control(Packet*p_in)
{
  int i;
  const unsigned char & t = r_control_type(p_in);
  const unsigned char & c = r_control_client(p_in);
  const unsigned char & ori = r_control_ori(p_in);
  const unsigned char & tar = r_control_tar(p_in);

  if(t == 0x05)
  {
    control_content[0] = 0x06;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), &(_ethh[tar]), sizeof(click_ether));

    syslog (LOG_DEBUG, "controller pass ant req for client %d, ap_ori %d, ap_tar %d\n", c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(t == 0x07)
  {
    control_content[0] = 0x08;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), &(_ethh[ori]), sizeof(click_ether));

    syslog (LOG_DEBUG, "controller pass ant ack for client %d, ap_ori %d, ap_tar %d\n", c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(t == 0x0b)
  {
    control_content[0] = 0x0c;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), &(_ethh[tar]), sizeof(click_ether));

    outport[c] = tar;
    syslog (LOG_DEBUG, "controller ack reas for client %d, ap_ori %d, ap_tar %d\n", c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(t == 0xff)
  {

    for(i=0;i<MAX_N_CLIENT;i++)
    {
      outport[i] = tmp_start[i] - 1;
    }
    control_content[0] = 0xff;
    control_content[1] = 0xff;
    control_content[2] = 0xff;
    control_content[3] = 0xff;
    for(i=0;i<MAX_N_AP;i++)
    {
      WritablePacket *p = Packet::make(sizeof(click_ether)+4);
      // // data part
      memcpy(p->data()+sizeof(click_ether), &control_content, 4);
    
      memcpy(p->data(), &(_ethh[i]), sizeof(click_ether));

      syslog (LOG_DEBUG, "controller reset and broadcast reset\n");
      output(0).push(p);
    }

  }
  p_in -> kill();
  
}

void RControlerControl::push_down_data(Packet*p_in, int port)
{
    WritablePacket *p = p_in->uniqueify();
    p->push(sizeof(click_ether));
    
    //eth
    memcpy(p->data(), &(_ethh[outport[port-1]+MAX_N_AP]), sizeof(click_ether));
    //syslog (LOG_DEBUG, "port: %d\n", port);
    //syslog (LOG_DEBUG, "outport: %d\n", outport[port-1]);
    output(0).push(p);
}



CLICK_ENDDECLS
EXPORT_ELEMENT(RControlerControl)
ELEMENT_MT_SAFE(RControlerControl)
