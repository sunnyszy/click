// -*- c-basic-offset: 4 -*-
/*
 * ipfragmenter.{cc,hh} -- element fragments IP packets
 * Robert Morris, Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2002 International Computer Science Institute
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "csisep.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
CLICK_DECLS

CSISep::CSISep()
{    
    csi_status = (csi_struct*)malloc(sizeof(csi_struct));

    fd = open_csi_device();
    // if (fd < 0)
        // printf("Failed to open the device...");
    // else
        // printf("#Receiving data!\n");
    // big_endian_flag = is_big_endian();
    // total_msg_cnt = 0;

}

CSISep::~CSISep()
{
    close_csi_device(fd);
    free(csi_status);
}

int
CSISep::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (Args(conf, this, errh)
      .read_p("PRINTFLAG", BoolArg(), print_flag)
      .complete() < 0)
    return -1;

  return 0;
}


int CSISep::open_csi_device(){
   int fd;
   fd = open("/dev/CSI_dev",O_RDWR);
    return fd;
}

void CSISep::close_csi_device(int fd){
    close(fd);
    //remove("/dev/CSI_dev");
}

// bool CSISep::is_big_endian()
// {
//     unsigned int a = 0x1;
//     unsigned char b = *(unsigned char *)&a;
//     if ( b == 0)
//     {
//         return true;
//     }
//     return false;
// }

int CSISep::read_csi_buf(unsigned char* buf_addr,int fd, int BUFSIZE){
    int cnt;
    /* listen to the port
     * read when 1, a csi is reported from kernel
     *           2, time out
     */           
    cnt = read(fd,buf_addr,BUFSIZE);
    if(cnt)
        return cnt;
    else
        return 0;
}
void CSISep::record_status(unsigned char* buf_addr, int cnt, csi_struct* csi_status){
    csi_status->rssi_0    = buf_addr[20];
    csi_status->rssi_1    = buf_addr[21];
    csi_status->rssi_2    = buf_addr[22];
}



void
CSISep::fragment(Packet *p_in)
{

    // if(print_flag)
    // {
    //     printf("Enter fragment.\n");
    // }  
    int  cnt;
    /* keep listening to the kernel and waiting for the csi report */
    cnt = read_csi_buf(buf_addr,fd,23);
    // if(print_flag)
    // {
    // printf("READ CSI.\n");
    // }
    if (cnt){

        /* fill the status struct with information about the rx packet */
        // if(print_flag)
        // {
        // printf("before record.\n");
        // }
        record_status(buf_addr, cnt, csi_status);
        // if(print_flag)
        // {
        // printf("before create.\n");
        // }
        WritablePacket *p_csi = Packet::make(1);
        // if(print_flag)
        // {
        // printf("before copy.\n");
        // }
        memcpy(p_csi->data(), &(csi_status->rssi_0), 1);
        // if(print_flag)
        // {
        // printf("before output 1.\n");
        // }
        output(1).push(p_csi);
    }
    // if(print_flag)
    // {
    //     printf("finish output 1.\n");
    // } 

    WritablePacket *p_master = p_in->uniqueify();
    struct click_ip *iph = p_master->ip_header();
    //arp
    if(!iph)
    {
        // if(print_flag)
        // {
        //     printf("This is an arp pkt.\n");
        //     printf("Arp len: %d\n", p_master->length());
        // }   
        if(p_master->length()>CSI_LEN)//if contain CSI
        {
            p_master->take(CSI_LEN);
        }
        // if(print_flag)
        // {
        // printf("Finish up.\n");
        // } 
    }
    // no wonder about ip because ip check will do it for you
    // else//ip
    // {
    //     uint16_t ipLenth = (((iph->ip_len)&0xff00)>>8)+(((iph->ip_len)&0x00ff)<<8);
    //     if(print_flag)
    //         printf("IP len: %d, real_len: %d\n", ipLenth,p_master->length()-14);
    //     if(ipLenth < p_master->length()-14)
    //     {   
    //         if(print_flag)
    //             printf("CSI appended ip\n");
    //         p_master->take(CSI_LEN);
    //     }

    // }
    // if(print_flag)
    // {
    //     printf("Finish ip.\n");
    // } 
    output(0).push(p_master);
        
}

void
CSISep::push(int, Packet *p)
{
	fragment(p);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(CSISep)
ELEMENT_MT_SAFE(CSISep)
