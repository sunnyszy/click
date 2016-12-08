/*
 * Definitation of WGTT project, maintained by Zheyu Song: sunnyszy@gmail.com
 */
#ifndef _WGTT_H_
#define _WGTT_H_

// ether type
#define ETHER_PROTO_BASE 0x0800
#define CONTROL_SUFFIX 0x1
#define STATUS_SUFFIX 0x2
#define DATA_SUFFIX 0x3

// ip
#define IP_BASE 0xac110200

// controller state
#define IDLE 0
#define SWITCH_REQ 1

// client 1 ip
#define CLIENT0_IP_SUFFIX 135

// ap 
#define AP0_IP_SUFFIX 0
#define AP1_IP_SUFFIX 1
#define AP2_IP_SUFFIX 2

// controller
#define CONTROLLER_IN_IP_SUFFIX 68
#define AP0_MAC "70:88:6b:80:60:00"
#define AP1_MAC "70:88:6b:80:60:01"
#define AP2_MAC "70:88:6b:80:60:02"
#define CONTROLLER_IN_MAC "70:88:6b:80:60:7d"
#define CONTROLLER_IN_MAC_SUFFIX 0x7d

#define RING_SIZE 256
#define N_CLIENT 1
#define N_AP 3

#define RESET_CONTENT 0xff


#define pkt_type(p) *(p->data()+13)
#define client_ip(p) *(p->data()+14)
#define status_ap(p) *(p->data()+11)

#define ap_score(p) *((char *)(p->data()+15))
#define ap_noise(p) *((char *)(p->data()+16))
// #define ip_id(p) *(p->data()+20)


#define start_ap(p) *(p->data()+15)
#define start_seq(p) *(p->data()+15)
#define queue_seq(p) *(p->data()+14)
// #define seq(p) *(p->data()+20)


#endif /* !_WGTT_H_ */
