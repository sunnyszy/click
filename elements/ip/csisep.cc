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
#include "iwinfo.h"
CLICK_DECLS

CSISep::CSISep()
{    
    // csi_status = (csi_struct*)malloc(sizeof(csi_struct));
    // print_flag = true;
    // fd = open_csi_device();
    // printf("This is a new version\n");
    // if (fd < 0)
    //     printf("Failed to open the device...\n");
    // else
    //     printf("#Receiving data!\n");
    // big_endian_flag = is_big_endian();
    // if(big_endian_flag)
    //     printf("big endian\n");
    // else
    //     printf("little endian\n");
    sprintf(shellcmd,"iwinfo wlan1 info | grep 'Signal'");
    sample_counter = 0;
    printf("CSISep: finish init\n");
    // total_msg_cnt = 0;

}

CSISep::~CSISep()
{
    // close_csi_device(fd);
    // free(csi_status);
}

int
CSISep::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (Args(conf, this, errh)
      .read_p("SAMPLERATE", IntArg(), sample_rate)
      .complete() < 0)
    return -1;

  return 0;
}


// int CSISep::open_csi_device(){
//    int fd;
//    fd = open("/dev/CSI_dev",O_RDWR);
//     return fd;
// }

// void CSISep::close_csi_device(int fd){
//     close(fd);
//     //remove("/dev/CSI_dev");
// }

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

// int CSISep::read_csi_buf(unsigned char* buf_addr,int fd, int BUFSIZE){
//     int cnt;
//     /* listen to the port
//      * read when 1, a csi is reported from kernel
//      *           2, time out
//      */           
//     cnt = read(fd,buf_addr,BUFSIZE);
//     if(cnt)
//         return cnt;
//     else
//         return 0;
// }
// void CSISep::record_status(unsigned char* buf_addr, int cnt, csi_struct* csi_status){
//     csi_status->rssi_0    = buf_addr[20];
//     csi_status->rssi_1    = buf_addr[21];
//     csi_status->rssi_2    = buf_addr[22];
// }

int CSISep::get_rssi()
{
    printf("In get_rssi\n");
    int i, rv = 0, len;
    const struct iwinfo_ops *iw;
    char buf[IWINFO_BUFSIZE];
    struct iwinfo_assoclist_entry *e;
    const char ifname[6] = "wlan1"; 

    iw = iwinfo_backend(ifname);
    

    if (!iw)
    {
        rv = 1;
    }
    else
    {
        if (iw->assoclist(ifname, buf, &len))
        {
            rv = 2;
        }
        else if (len <= 0)
        {
            rv = 3;
        }
        else
        {
            for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry))
            {
                e = (struct iwinfo_assoclist_entry *) &buf[i];

                printf("RateSignal: %d\n", e->signal);
                printf("RateNoise: %d\n", e->noise);
                printf("RateRXRaw: %d\n", e->rx_rate);
                printf("RateTXRaw: %d\n", e->tx_rate);
            }
            rv = 0;
        }
    }

    iwinfo_finish();

    return rv;
}

void
CSISep::fragment(Packet *p_in)
{
    get_rssi();
    sample_counter ++;
    if(sample_counter>sample_rate)
    {
        sample_counter = 0;

    if(NULL == (file = popen(shellcmd,"r")))     
    {    
        // printf("execute command failed!");         
    }
    else
    {
        fgets(buffer, 100, file);
        char * p = strchr(buffer, ':');
        // printf("the string buffer: %s\n", p+3);
        uint8_t length = atoi(p+3);
        // printf("The Signal length %hu\n", length);
        if(length>0)
        {
            WritablePacket *p_csi = Packet::make(1);
            memcpy(p_csi->data(), &length, 1);
            output(1).push(p_csi);
        }
        pclose(file);
        
            
    }
    }
    
    // printf("CSISep: out fragment\n");
    output(0).push(p_in);
        
}

void
CSISep::push(int, Packet *p)
{
    // printf("CSISep: in push\n");
	fragment(p);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(CSISep)
ELEMENT_LIBS(-L/Volumes/BasicWrt/openwrt_linksys/staging_dir/target-arm_cortex-a9+vfpv3_uClibc-0.9.33.2_eabi/usr/lib -liwinfo)