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
#ifndef __APPLE__
    strcpy(ifname, "wlan0");
    printf("CSISep: finish init\n");
    // total_msg_cnt = 0;
#endif 

}

CSISep::~CSISep()
{
#ifndef __APPLE__ 
    iwinfo_finish();
#endif
}

int
CSISep::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (Args(conf, this, errh)
      .read_p("SAMPLERATE", IntArg(), sample_rate)
      .complete() < 0)
    return -1;

#ifndef __APPLE__ 

    iw = iwinfo_backend(ifname);
    if (!iw)
        printf("CSISep: can not connect to backend iwinfo\n");
    if (iw->assoclist(ifname, buf, &len))
        printf("CSISep: can not find associlist\n");
    else if (len <= 0)
        printf("CSISep: associ number < 0\n");
#endif
    printf("CSISep: finish configure, ready to start\n");
    return 0;
}

void
CSISep::fragment(Packet *p_in)
{
#ifndef __APPLE__ 
    // int i;
    sample_counter ++;
    if(sample_counter>sample_rate)
    {
        sample_counter = 0;

        if(len>0)
        {
            WritablePacket *p_csi = Packet::make(sizeof(my_test_struct)*len);

            memcpy(p_csi->data(), buf, sizeof(my_test_struct)*len);
            output(1).push(p_csi);
            // for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry))
            // {
            //         e = (struct iwinfo_assoclist_entry *) &buf[i];
            //         printf("RateSignal: %d\n", e->signal);
            //         printf("RateNoise: %d\n", e->noise);
            //         printf("RateRXRaw: %d\n", e->rx_rate);
            //         printf("RateTXRaw: %d\n", e->tx_rate);
            // }    
        }
    }
#endif    
    output(0).push(p_in);

}

void
CSISep::push(int, Packet *p)
{
#ifndef __APPLE__ 
    // printf("CSISep: in push\n");
	fragment(p);
#endif
}


CLICK_ENDDECLS
EXPORT_ELEMENT(CSISep)
ELEMENT_LIBS(-L/Volumes/BasicWrt/openwrt_linksys/staging_dir/target-arm_cortex-a9+vfpv3_uClibc-0.9.33.2_eabi/usr/lib -liwinfo)