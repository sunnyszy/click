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
        strcpy(ifname, "wlan0");
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
            WritablePacket *p_csi = Packet::make(sizeof(my_test_struct)*N_CLIENT);

            
            output(1).push(p_csi);
            for (i = 0; i < N_CLIENT; i += sizeof(struct iwinfo_assoclist_entry))
            {
                    e = (struct iwinfo_assoclist_entry *) &buf[i];
                    memcpy(p_csi->data()+j, &(e->mac[5]), sizeof(uint8_t));
                    j += sizeof(uint8_t);
                    memcpy(p_csi->data()+j, &(e->signal), sizeof(int8_t));
                    j += sizeof(int8_t);
                    memcpy(p_csi->data()+j, &(e->noise), sizeof(int8_t));
                    j += sizeof(int8_t);
                    memcpy(p_csi->data()+j, &((e->rx_rate).rate), sizeof(uint32_t));
                    j += sizeof(uint32_t);
                    memcpy(p_csi->data()+j, &((e->tx_rate).rate), sizeof(uint32_t));
                    j += sizeof(uint32_t);

                    printf("RateSignal: %d\n", e->signal);
                    printf("RateNoise: %d\n", e->noise);
                    printf("RateRXRaw: %d\n", e->rx_rate);
                    printf("RateTXRaw: %d\n", e->tx_rate);
            }    
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
ELEMENT_MT_SAFE(CSISep)

