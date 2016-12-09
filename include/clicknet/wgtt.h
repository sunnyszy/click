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
#define N_CLIENT 1
#define CLIENT1_IP_SUFFIX 135
#define CLIENT2_IP_SUFFIX 136

// ap 
#define MAX_N_AP 8
#define N_AP 3
#define AP1_IP_SUFFIX 1
#define AP2_IP_SUFFIX 2
#define AP3_IP_SUFFIX 3
#define AP4_IP_SUFFIX 4
#define AP5_IP_SUFFIX 5
#define AP6_IP_SUFFIX 6
#define AP7_IP_SUFFIX 7
#define AP8_IP_SUFFIX 8

// controller
#define CONTROLLER_IN_IP_SUFFIX 68
#define AP1_MAC "70:88:6b:80:60:01"
#define AP2_MAC "70:88:6b:80:60:02"
#define AP3_MAC "70:88:6b:80:60:03"
#define AP4_MAC "70:88:6b:80:60:04"
#define AP5_MAC "70:88:6b:80:60:05"
#define AP6_MAC "70:88:6b:80:60:06"
#define AP7_MAC "70:88:6b:80:60:07"
#define AP8_MAC "70:88:6b:80:60:08"

#define CONTROLLER_IN_MAC "70:88:6b:80:60:7d"
#define CONTROLLER_IN_MAC_SUFFIX 0x7d

#define RING_SIZE 256


#define RESET_CONTENT 0xff


#define pkt_type(p) *(p->data()+13)
#define client_ip(p) *(p->data()+14)
#define status_ap(p) *(p->data()+11)

#define status_mac(p) *((unsigned char *)(p->data()+14))
#define status_score(p) *((char *)(p->data()+15))
#define status_noise(p) *((char *)(p->data()+16))
#define status_rxrate(p) *((int *)(p->data()+17))
#define status_txrate(p) *((int *)(p->data()+21))
#define ip_id(p) *(p->data()+20)


#define start_ap(p) *(p->data()+15)
#define start_seq(p) *(p->data()+15)
#define queue_seq(p) *(p->data()+14)
// #define seq(p) *(p->data()+20)


#endif /* !_WGTT_H_ */
