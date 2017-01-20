/*
 Controller program, issusing switching between different ap. 
 Input: control/status packet
 Output: control packet
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "packetselectionSerial.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <math.h>

CLICK_DECLS

int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}




PacketSelectionSerial::PacketSelectionSerial()
{
  int i,j,k;
  openlog("PacketSelectionSerial", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);
  score = new int**[MAX_N_CLIENT];
  next_score_id = new unsigned char*[MAX_N_CLIENT];
  for(i=0; i<MAX_N_CLIENT; i++)
  {
    score[i] = new int*[MAX_N_AP];
    next_score_id[i] = new unsigned char[MAX_N_AP];
    for(j=0; j<MAX_N_AP; j++)
    {
      score[i][j] = new int[n_compare];
      next_score_id[i][j] = 0;
      for(k=0; k<n_compare;k++)
      {
        score[i][j][k] = 9999;//a quite small score
       
      }
    }
    state[i] = IDLE;
    time_lock[i] = false;
    last_time[i] = 0;
  }
  tmp_score = new int*[3];
  for(i=0;i<3;i++)
    tmp_score[i] = new int[n_compare];

  _ethh = new click_ether;
  _ethh->ether_type = htons(CONTROL_SUFFIX+ETHER_PROTO_BASE);
  cp_ethernet_address(CONTROLLER_IN_MAC, _ethh->ether_shost);

  syslog (LOG_DEBUG, "Init finish, ready to start\n");
}

int PacketSelectionSerial::configure(Vector<String> &conf, ErrorHandler *errh)
{
  int i,tmp[4];
  if (Args(conf, this, errh)
      .read_p("INTERVAL", IntArg(), interval)
      .read_p("FIRSTSTART1", IntArg(), tmp[0])
      .read_p("FIRSTSTART2", IntArg(), tmp[1])
      .read_p("FIRSTSTART3", IntArg(), tmp[2])
      .read_p("FIRSTSTART4", IntArg(), tmp[3])
      .read_p("PRINTINTERVAL", IntArg(), print_interval)
      .complete() < 0)
    return -1;

  for(i=0; i<MAX_N_CLIENT; i++)
  {
    first_start[i] = tmp[i];
    output_port[i] = first_start[i]-1;
  }

  syslog (LOG_DEBUG, "Finish configure. Switch interval: %d\n", interval);
  return 0;
}

void PacketSelectionSerial::push(int port, Packet *p_in)
{
  // here is a small bug, I can not put the reset function in the initial function
  static unsigned char lock = 0;
  if(!lock)
  { 
    lock++;
    reset_ap();
  }

  // syslog (LOG_DEBUG, "pkt_type: %x\n", pkt_type(p_in));
  switch(pkt_type(p_in))
  {
    case CONTROL_SUFFIX:  push_control(p_in);break;
    case STATUS_SUFFIX:   push_status(p_in);break;
  }

}

void PacketSelectionSerial::reset_ap()
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
  output(0).push(p);
  }
}


void PacketSelectionSerial::push_control(Packet *p_in)
{
  const unsigned char & c = client_ip(p_in);
  
  state[c-CLIENT1_IP_SUFFIX] = IDLE;

  syslog (LOG_DEBUG, "Switch request ack, ip: %d.\n", c);
  p_in -> kill();
}

void PacketSelectionSerial::push_status(Packet *p_in)
{
  const unsigned char a = status_ap(p_in) - 1;
  unsigned char c;
  switch(status_mac(p_in))
  {
    case CLIENT1_MAC_SUFFIX: c = 0;break;
    case CLIENT2_MAC_SUFFIX: c = 1;break;
    case CLIENT3_MAC_SUFFIX: c = 2;break;
    case CLIENT4_MAC_SUFFIX: c = 3;break;
  }
  //since the score are minus, we minus again
  //TODO: smaller is better?
  score[c][a][next_score_id[c][a]] = - status_score(p_in);
  next_score_id[c][a] = (next_score_id[c][a] + 1)%n_compare;
  // able to change state

  static unsigned int tmp_counter = 0;
  tmp_counter++;
  if(!(tmp_counter%print_interval))
  {
    int rx_rate = status_rxrate(p_in);
    int tx_rate = status_txrate(p_in);
    syslog (LOG_DEBUG, "client mac: %X, ap id: %X\n", status_mac(p_in), status_ap(p_in));
    syslog (LOG_DEBUG, "signal: %d, noise: %d\n", status_score(p_in), status_noise(p_in));
    syslog (LOG_DEBUG, "rx_rate: %d.%dMb/s, tx_rate: %d.%d Mb/s\n", 
      rx_rate / 1000, rx_rate / 100, tx_rate / 1000, tx_rate / 100);
  }

  gettimeofday(&tv, NULL);
  double now_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
  if(now_time - last_time[c] > SWITCH_MIN)
    time_lock[c] = false;


  if(state[c] == IDLE && !time_lock[c])
  {
      // syslog (LOG_DEBUG, "state idle\n");
      unsigned char best_ap;
      // syslog (LOG_DEBUG, "current_state: %d, time_lock: %d\n", state[c], time_lock[c]);
      // WGTT
      if(interval>0)
      {
        best_ap = output_port[c];
        if(!(tmp_counter%interval))
        {
            best_ap = (best_ap + 1)% 2;
            syslog (LOG_DEBUG, "prepare manually switch to ap %X\n", best_ap+1);
        }
      }
      else
         best_ap = find_best_ap_global(c);

      if(best_ap != output_port[c])
      {
        // send_meg(best_ap)
        WritablePacket *p = Packet::make(sizeof(click_ether)+2);
        // click_ip *ip = reinterpret_cast<click_ip *>(p->data()+sizeof(click_ether));
        // // data part
        control_content[0] = CLIENT1_IP_SUFFIX + c;
        control_content[1] = best_ap;
        memcpy(p->data()+sizeof(click_ether), &control_content, 2);
        //ether part
        switch(output_port[c])
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
        state[c] = SWITCH_REQ;
        output_port[c] = best_ap;

        // after issue switch, time lock will be turn on, and turned off after 1 s
        time_lock[c] = true;
        gettimeofday(&tv, NULL);
        last_time[c] = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
        output(0).push(p);
      }
  }
  p_in -> kill();

}

// incomplete version, only for 2 ap and 1 client
unsigned char PacketSelectionSerial::find_best_ap_neighbor(unsigned char c)
{
  unsigned char &current = output_port[c];
  bool switch_to_left = true, switch_to_right = true;
  int i,j;
  int num_bigger;
  // copy to tmp: current, sort
  for(i=0; i<n_compare;i++)
    tmp_score[1][i] = score[c][current][i];
  qsort(tmp_score[1], sizeof(tmp_score[1])/sizeof(*tmp_score[1]), sizeof(*tmp_score[1]), comp);
  // for(i=0; i<n_compare;i++)
  //   printf("%dth element: %d\n", i, tmp_score[1][i]);

  // find potential, can only be left 1 or right one
  if(current==0)
    switch_to_left = false;
  else if(current == MAX_N_AP-1)
    switch_to_right = false;

  if(switch_to_left)
  {
    // copy to tmp: left, sort
    for(i=0; i<n_compare;i++)
      tmp_score[0][i] = score[c][current-1][i];
    qsort(tmp_score[0], sizeof(tmp_score[0])/sizeof(*tmp_score[0]), sizeof(*tmp_score[0]), comp);
    
    num_bigger = 0;
    for(j=0; j<n_compare; j++)
      if(tmp_score[0][j] <= tmp_score[1][j])
      {
        num_bigger ++;
      }
    if(num_bigger < MAJOR)
      switch_to_left = false;
  }
  if(switch_to_right)
  {
    //copy to tmp: right, sort
    for(i=0; i<n_compare;i++)
      tmp_score[2][i] = score[c][current+1][i];
    qsort(tmp_score[2], sizeof(tmp_score[2])/sizeof(*tmp_score[2]), sizeof(*tmp_score[2]), comp);
  
    num_bigger = 0;
    for(j=0; j<n_compare; j++)
      if(tmp_score[2][j] <= tmp_score[1][j])
      {
        num_bigger ++;
      }
    if(num_bigger < MAJOR)
      switch_to_right = false;
  }
  if(switch_to_left && switch_to_right)
  {
    num_bigger = 0;
    for(j=0; j<n_compare; j++)
      if(tmp_score[0][j] <= tmp_score[2][j])
      {
        num_bigger ++;
      }
    if(num_bigger < MAJOR)
      switch_to_left = false;
    else
      switch_to_right = false;
  }
  

  if(switch_to_left)
    return current-1;
  else if(switch_to_right)
    return current+1;
  else
    return current;
}

// incomplete version, only for 2 ap and 1 client
unsigned char PacketSelectionSerial::find_best_ap_global(unsigned char c)
{
  int min_id = 0, min_value = 9999;
  unsigned char i, j;
  for(i=0; i<MAX_N_AP;i++)
  {
    int sum = 0;
    for(j=0; j<n_compare;j++)
    {
      sum += score[c][i][j];
    }
    if(sum < min_value)
    {
      min_id = i;
      min_value = sum;
    }
  }
  return min_id;
}


CLICK_ENDDECLS
EXPORT_ELEMENT(PacketSelectionSerial)
ELEMENT_MT_SAFE(PacketSelectionSerial)
