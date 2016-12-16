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
#define MAX_N_CLIENT 4

// ap 
#define MAX_N_AP 8

// controller
#define CONTROLLER_IN_IP_SUFFIX 68

#define CONTROLLER_IN_MAC "70:88:6b:80:60:7d"
#define CONTROLLER_IN_MAC_SUFFIX 0x7d

#define RING_SIZE 256
// 1000 ms
#define SWITCH_MIN 1000
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

unsigned char CLIENT_IP_SUFFIX[MAX_N_CLIENT] = {135, 136, 137, 138};
unsigned short SERVER_PORT[MAX_N_CLIENT] = {5201, 5202, 5203, 5204};
unsigned char AP_IP_SUFFIX[MAX_N_AP] = {1, 2, 3, 4, 5, 6, 7, 8};
char *AP_MAC[MAX_N_AP] = {"70:88:6b:80:60:01", "70:88:6b:80:60:02", "70:88:6b:80:60:03"
	"70:88:6b:80:60:04", "70:88:6b:80:60:05", "70:88:6b:80:60:06", "70:88:6b:80:60:07"
	"70:88:6b:80:60:08"};
//made up last two client mac 
unsigned char CLIENT_MAC_SUFFIX[MAX_N_CLIENT] = {0x07, 0xac, 0x01, 0x02};


#endif /* !_WGTT_H_ */
