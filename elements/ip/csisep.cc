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
    if (fd < 0)
        printf("Failed to open the device...");
    else 
        printf("#Receiving data!\n");
    total_msg_cnt = 0;

}

CSISep::~CSISep()
{
    close_csi_device(fd);
    free(csi_status);
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
    
    int  cnt;
    /* keep listening to the kernel and waiting for the csi report */
    cnt = read_csi_buf(buf_addr,fd,24);

    if (cnt){
        total_msg_cnt += 1;

        /* fill the status struct with information about the rx packet */
        record_status(buf_addr, cnt, csi_status);
 
        if(total_msg_cnt%100 == 0)
        {
            printf("rssi : %u %u %u\n", csi_status->rssi_0, csi_status->rssi_1, csi_status->rssi_2);
        }
        WritablePacket *p_csi = Packet::make(1);
        memcpy(p_csi->data(), &(csi_status->rssi_0), 1);
        if (noutputs() == 2)
            output(1).push(p_csi);
        else
            p_csi->kill();
    }

    WritablePacket *p_master = p_in->uniqueify();
    p_master->take(CSI_LEN);
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
