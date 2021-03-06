/*
 Parameter of WGTT project, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */
#ifndef _WGTT_H_
#define _WGTT_H_


// #define WGTT_DEBUG
// #ifdef WGTT_DEBUG
// #define DPRINT(fmt, args...) syslog (LOG_DEBUG, "%s: " fmt, __func__ , ## args) 
// #else
// #define DPRINT(fmt, args...) do {} while (0)
// #endif
// #define NDPRINT(fmt, args...) do {} while(0)

// #ifdef WGTT_DEBUG
// #define DASSERT(expr)						\
//     if (!(expr)) {						\
//         syslog("Assertion failed! %s,%s,%s,line=%d\n",	\
//                #expr, __FILE__, __func__, __LINE__);	\
//     }
// #else
// #define DASSERT(expr) do {} while (0)
// #endif

// #ifdef WGTT_DEBUG
// #define DASSERT2(expr1, expr2)						\
//     if (expr1 != expr2) {						\
//         syslog("Assertion failed! %s,%016lx,%s,%016lx,%s,%s,line=%d\n",	\
//                #expr1, expr1, #expr2, expr2, __FILE__, __func__, __LINE__);	\
//     }
// #else
// #define DASSERT2(expr1, expr2) do {} while (0)
// #endif


struct my_test_struct {
  uint8_t mac;
  int8_t signal;
  int8_t noise;
  uint32_t rx_rate;
  uint32_t tx_rate;
  
};

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
#define INACTIVE 2
#define ANT 3

// client 1 ip
#define MAX_N_CLIENT 1
//currently unused
// #define RSSI_THRESHOLD -67

#define CLIENT1_MAC_SUFFIX 0x07
#define CLIENT2_MAC_SUFFIX 0x0b
#define CLIENT3_MAC_SUFFIX 0x0d
#define CLIENT1_MAC "44:c3:06:31:5b:07"
#define CLIENT2_MAC "44:c3:06:31:5b:0b"
#define CLIENT3_MAC "44:c3:06:31:5b:0d"
// ap 
#define MAX_N_AP 3
#define CLIENT1_IP_SUFFIX 135
#define CLIENT2_IP_SUFFIX 136
#define CLIENT3_IP_SUFFIX 137
#define AP1_MAC "70:88:6b:80:60:01"
#define AP2_MAC "70:88:6b:80:60:02"
#define AP3_MAC "70:88:6b:80:60:03"
#define AP4_MAC "70:88:6b:80:60:04"
#define AP5_MAC "70:88:6b:80:60:05"
#define AP6_MAC "70:88:6b:80:60:06"
#define AP7_MAC "70:88:6b:80:60:07"
#define AP8_MAC "70:88:6b:80:60:08"

// controller
#define CONTROLLER_IN_IP_SUFFIX 68

#define CONTROLLER_IN_MAC "70:88:6b:80:60:7d"
#define CONTROLLER_IN_MAC_SUFFIX 0x7d

#define RING_SIZE 256
// unit ms
#define SWITCH_MIN 60
#define K_SWITCH_MIN 1000
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

//for 80211r
#define	r_control_type(p) *(p->data()+14)
#define	r_control_client(p) *(p->data()+15)
#define	r_control_ori(p) *(p->data()+16)
#define	r_control_tar(p) *(p->data()+17)
#define r_src_mac_suffix(p) *(p->data()+11)
#define r_dst_mac_suffix(p) *(p->data()+5)
#define r_ether_type_suffix(p) *(p->data()+13)


#endif