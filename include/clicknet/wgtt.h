/*
 Parameter of WGTT project, 
 Created by Zhenyu Song: sunnyszy@gmail.com
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
#define MAX_N_CLIENT 2

#define CLIENT1_MAC_SUFFIX 0x07
#define CLIENT2_MAC_SUFFIX 0x08
#define CLIENT3_MAC_SUFFIX 0x09
#define CLIENT4_MAC_SUFFIX 0x0a

// ap 
#define MAX_N_AP 2

// controller
#define CONTROLLER_IN_IP_SUFFIX 68

#define CONTROLLER_IN_MAC "70:88:6b:80:60:7d"
#define CONTROLLER_IN_MAC_SUFFIX 0x7d

#define RING_SIZE 256
// 1000 ms
#define SWITCH_MIN 100000
// content encapsulated in mac header
#define RESET_CONTENT 0xff

// mac header indicates the packet type
#define pkt_type(p) *(p->data()+13)
#define client_ip(p) *(p->data()+14)

#define status_ap(p) *(p->data()+11)
// when can use status mac to know client
#define status_mac(p) *((unsigned char *)(p->data()+14))
#define status_score(p) *((char *)(p->data()+15))
#define status_noise(p) *((char *)(p->data()+16))
#define status_rxrate(p) *((int *)(p->data()+17))
#define status_txrate(p) *((int *)(p->data()+21))

// I think we don't need this 
// #define ip_id(p) *(p->data()+20)
// start ap in controll-ap command
#define start_ap(p) *(p->data()+15)
// seq number in ap-ap command
#define start_seq(p) *(p->data()+15)
// seq in data pkt
#define queue_seq(p) *(p->data()+14)
#define data_client(p) *(p->data()+20)

#define CLIENT1_IP_SUFFIX 135
#define AP1_MAC "70:88:6b:80:60:01"
#define AP2_MAC "70:88:6b:80:60:02"
#define AP3_MAC "70:88:6b:80:60:03"
#define AP4_MAC "70:88:6b:80:60:04"
#define AP5_MAC "70:88:6b:80:60:05"
#define AP6_MAC "70:88:6b:80:60:06"
#define AP7_MAC "70:88:6b:80:60:07"
#define AP8_MAC "70:88:6b:80:60:08"

// char *CLIENT_MAC[MAX_N_CLIENT] = {"44:c3:06:31:5b:01", "44:c3:06:31:5b:02", "44:c3:06:31:5b:03",
// 	"44:c3:06:31:5b:04"};

#endif _WGTT_H_
