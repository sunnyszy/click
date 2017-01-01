/*
 element for 802.11r client, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "rclientcontrol.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ip.h>
#include <math.h>

 CLICK_DECLS

int
RClientControl::configure(Vector<String> &conf, ErrorHandler *errh)
{
  int i, tmp_start[4],tmp_id;
  for(i=0;i<MAX_N_AP;i++)
  {
    rssi[i] = -127;//quite large
  }
  if (Args(conf, this, errh)
      .read_p("IDENTITY", IntArg(), tmp_id)
      .read_p("FIRSTSTART1", IntArg(), tmp_start[0])
      .read_p("FIRSTSTART2", IntArg(), tmp_start[1])
      .read_p("FIRSTSTART3", IntArg(), tmp_start[2])
      .read_p("FIRSTSTART4", IntArg(), tmp_start[3])
      .complete() < 0)
    return -1;

  identity = tmp_id;
  current_ap = tmp_start[identity-1] - 1;
  state = IDLE;


  _ethh = new click_ether;
  _ethh->ether_type = htons(CONTROL_SUFFIX+ETHER_PROTO_BASE);
  
  switch(identity)
  {
    case 1: cp_ethernet_address(CLIENT1_MAC, _ethh->ether_shost);break;
  }
  cp_ethernet_address(CONTROLLER_IN_MAC, _ethh->ether_dhost);
  syslog (LOG_DEBUG, "RClientControl: finish configure, ready to start\n");
  return 0;
}

void
RClientControl::push(int port, Packet *p_in)
{
  switch(port)
  {
    case 0: push_updata(p_in);break;
    case 1: push_control(p_in);break;
    case 2: push_downdata(p_in);break;
    case 3: push_80211(p_in);break;
  }
}


void RClientControl::push_control(Packet*p_in)
{
  const unsigned char & t = r_control_type(p_in);
  const unsigned char & c = r_control_client(p_in);
  const unsigned char & ori = r_control_ori(p_in);
  const unsigned char & tar = r_control_tar(p_in);

  if(state == ANT && t == 0x09)
  {
    state = INACTIVE;
    control_content[0] = 0x0a;
    control_content[1] = c;
    control_content[2] = ori;
    control_content[3] = tar;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), _ethh, sizeof(click_ether));

    syslog (LOG_DEBUG, "client send reas for client %d, ap_ori %d, ap_tar %d\n", c+1, ori+1, tar+1);
    output(0).push(p);
  }
  else if(state == INACTIVE && t == 0x0d)
  {
    state = IDLE;
    syslog (LOG_DEBUG, "client finish reas for client %d, ap_ori %d, ap_tar %d\n", c+1, ori+1, tar+1);
  }
  p_in -> kill();
  
}

void RClientControl::push_downdata(Packet*p_in)
{
  WritablePacket *p = p_in->uniqueify();
  //printf("RClientControl: ether type: %u\n", r_ether_type_suffix(p));
  if(r_ether_type_suffix(p) == 0x03)
  {
    memcpy(p->data()+13, &ether_type_ip_suffix, 1);
  }  

  if(state == INACTIVE)
    p -> kill();
  else
    output(1).push(p);
  
}

void RClientControl::push_updata(Packet*p_in)
{
  if(state == INACTIVE)
    p_in -> kill();
  else
    output(0).push(p_in);
}
void RClientControl::push_80211(Packet*p_in)
{
  //TODO: filter and parse from click or here?
  const char & rssi_this = status_score(p_in);
  const unsigned char ap = status_ap(p_in) - 1;
  static unsigned char c;
  switch(status_mac(p_in))
  {
    case CLIENT1_MAC_SUFFIX: c = 1;
    case CLIENT2_MAC_SUFFIX: c = 2;
    case CLIENT3_MAC_SUFFIX: c = 3;
    case CLIENT4_MAC_SUFFIX: c = 4;
  }
  syslog (LOG_DEBUG, "RClientControl: receive becon, ap %d, client %d, rssi %d\n",
      ap+1, c, rssi_this);
  if(c == identity)
    rssi[ap] = rssi_this;
  // if IDLE, considering switching
  if(c == identity && state == IDLE && rssi[current_ap] < -67)
  {
    syslog (LOG_DEBUG, "Considering to switch\n");
    unsigned i;
    unsigned max_id;
    char max_rssi = -127;
    // find max rssi
    for(i=0;i<MAX_N_AP;i++)
      if(rssi[i] > max_rssi)
      {
        max_rssi = rssi[i];
        max_id = i;
      }
    if(max_id == current_ap)
      return;
    
    control_content[0] = 0x04;
    control_content[1] = identity-1;
    control_content[2] = current_ap;
    control_content[3] = max_id;

    WritablePacket *p = Packet::make(sizeof(click_ether)+4);
    // // data part
    memcpy(p->data()+sizeof(click_ether), &control_content, 4);
  
    memcpy(p->data(), _ethh, sizeof(click_ether));

    syslog (LOG_DEBUG, "client send ant req for client %d, ap_ori %d, ap_tar %d\n", identity, current_ap+1, max_id+1);
    current_ap = max_id;
    state = ANT;
    output(0).push(p);
  }

}


CLICK_ENDDECLS
EXPORT_ELEMENT(RClientControl)
ELEMENT_MT_SAFE(RClientControl)
