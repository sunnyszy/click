#ifndef CLICK_CSISEP_HH
#define CLICK_CSISEP_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/atomic.hh>
#include <fcntl.h>
#include <unistd.h>
CLICK_DECLS


// typedef struct
// {
//     int real;
//     int imag;
// }COMPLEX;

// typedef struct
// {
//     uint64_t tstamp;         /* h/w assigned time stamp */

//     uint8_t    rssi;         /*  rx frame RSSI */
//     uint8_t    rssi_0;       /*  rx frame RSSI [ctl, chain 0] */
//     uint8_t    rssi_1;         rx frame RSSI [ctl, chain 1] 
//     uint8_t    rssi_2;       /*  rx frame RSSI [ctl, chain 2] */

//     uint16_t   payload_len;  /*  payload length (bytes) */
//     uint16_t   csi_len;      /*  csi data length (bytes) */
//     uint16_t   buf_len;      /*  data length in buffer */
// }csi_struct;




class CSISep : public Element { public:

  CSISep() CLICK_COLD;
  ~CSISep() CLICK_COLD;

  const char *class_name() const		{ return "CSISep"; }
  const char *port_count() const		{ return PORTS_1_1X2; }
  const char *processing() const		{ return PUSH; }

  // int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
  
  void push(int, Packet *);
  void fragment(Packet *);
  // int   open_csi_device();
  // void  close_csi_device(int fd);
  // int   read_csi_buf(unsigned char* buf_addr,int fd, int BUFSIZE);
  // void  record_status(unsigned char* buf_addr, int cnt, csi_struct* csi_status);
  // bool is_big_endian();

 private:

  // static const uint32_t CSI_LEN = 280;
  // csi_struct*   csi_status;
  // int         fd;
  // int total_msg_cnt;

  // unsigned char buf_addr[24];
  
  // bool print_flag;
  // bool big_endian_flag;
  char shellcmd[64];
  char buffer[100];
  FILE *file;

};

CLICK_ENDDECLS
#endif
