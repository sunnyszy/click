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
#ifdef __arm__
    printf("CSISep: finish init\n");
    // total_msg_cnt = 0;
    sample_counter = 0;
#endif 

}

CSISep::~CSISep()
{
#ifdef __arm__ 
    iwinfo_finish();
#endif
}

int
CSISep::configure(Vector<String> &conf, ErrorHandler *errh)
{
    int wlan_port;
    if (Args(conf, this, errh)
      .read_p("SAMPLERATE", IntArg(), sample_rate)
      .read_p("WLANPORT", IntArg(), wlan_port)
      .complete() < 0)
    return -1;

#ifdef __arm__ 
    if(wlan_port == 0)
        strcpy(ifname, "wlan0");
    else if(wlan_port == 1)
        strcpy(ifname, "wlan1");
    else
        printf("Invalid wlan_port argument\n");
    iw = iwinfo_backend(ifname);
    if (!iw)
        printf("CSISep: can not connect to backend iwinfo\n");
#endif
    printf("CSISep: finish configure, ready to start\n");
    return 0;
}

void
CSISep::fragment(Packet *p_in)
{
#ifdef __arm__ 
    int i, j=0;
    sample_counter ++;
    if(sample_counter>sample_rate)
    {
        sample_counter = 0;

        if (iw->assoclist(ifname, buf, &len))
            printf("CSISep: can not find associlist\n");
        else if (len <= 0)
            printf("CSISep: associ number < 0\n");
        else
        {
            // WritablePacket *p_csi = Packet::make(sizeof(my_test_struct)*N_CLIENT);
            WritablePacket *p_csi = Packet::make(11*N_CLIENT);
            for (i = 0; i < N_CLIENT; i += sizeof(struct iwinfo_assoclist_entry))
            {
                e = (struct iwinfo_assoclist_entry *) &buf[i];
                uint8_t & mac = e->mac[5];
                int8_t & signal = e->signal;
                int8_t & noise = e->noise;
                uint32_t & rx_rate = (e->rx_rate).rate;
                uint32_t & tx_rate = (e->tx_rate).rate;

                // printf("rx_rate: %d\n", rx_rate);
                // printf("rx_rate: %X\n", rx_rate);
                // printf("tx_rate: %d\n", tx_rate);
                memcpy(p_csi->data()+j, &(mac), 1);
                j += 1;
                memcpy(p_csi->data()+j, &(signal), 1);
                j += 1;
                memcpy(p_csi->data()+j, &(noise), 1);
                j += 1;
                memcpy(p_csi->data()+j, &(rx_rate), 4);
                j += 4;
                memcpy(p_csi->data()+j, &(tx_rate), 4);
                j += 4;
                // printf("RateSignal: %d\n", e->signal);
                // printf("RateNoise: %d\n", e->noise);
                // printf("RateRXRaw: %d\n", (e->rx_rate).rate);
                // printf("RateTXRaw: %d\n", (e->tx_rate).rate);
            }   
            output(1).push(p_csi); 
        }
    }
#endif    
    output(0).push(p_in);

}

void
CSISep::push(int, Packet *p)
{
#ifdef __arm__ 
    // printf("CSISep: in push\n");
	fragment(p);
#endif
}


CLICK_ENDDECLS
EXPORT_ELEMENT(CSISep)
// #ifdef __arm__ 
//     ELEMENT_LIBS(-L/Volumes/BasicWrt/openwrt_linksys/staging_dir/target-arm_cortex-a9+vfpv3_uClibc-0.9.33.2_eabi/usr/lib -liwinfo)
// #endif

