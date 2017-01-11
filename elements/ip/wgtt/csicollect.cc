/*
 sample csi, rssi from TP-Link router
 Input: data pkt
 Output: port 0: csi pkt, port 1: rssi pkt
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "csicollect.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>

CLICK_DECLS

CSICollect::CSICollect()
{
#ifdef __mips__
    openlog("APControl_CSICollect", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);
    syslog (LOG_DEBUG, "finish init\n");
    // total_msg_cnt = 0;
    sample_counter = 0;
#endif 

}

CSICollect::~CSICollect()
{
#ifdef __mips__ 
    iwinfo_finish();
#endif
}

int
CSICollect::configure(Vector<String> &conf, ErrorHandler *errh)
{
    int wlan_port;
    int tmp_len;
    if (Args(conf, this, errh)
      .read_p("SAMPLERATE", IntArg(), sample_rate)
      .read_p("WLANPORT", IntArg(), wlan_port)
      .read_p("PACKETLENGTH", IntArg(), tmp_len)
      .complete() < 0)
    return -1;

#ifdef __mips__ 
    packet_length = tmp_len;
    if(wlan_port == 0)
        strcpy(ifname, "wlan0");
    else if(wlan_port == 1)
        strcpy(ifname, "wlan1");
    else
        syslog (LOG_DEBUG, "Invalid wlan_port argument\n");
    iw = iwinfo_backend(ifname);
    if (!iw)
        syslog (LOG_DEBUG, "Can not connect to backend iwinfo\n");
#endif
    syslog (LOG_DEBUG, "Finish configure, ready to start\n");
    return 0;
}

void
CSICollect::fragment(Packet *p_in)
{
#ifdef __mips__ 
    int i,j;
    sample_counter ++;
    if(sample_counter>sample_rate)
    {
        sample_counter = 0;

        if(!(iw->assoclist(ifname, buf, &len)))
        //     // syslog (LOG_DEBUG, "CSICollect: can not find associlist\n");
        // else if (len <= 0)
        //     // syslog (LOG_DEBUG, "CSICollect: associ number < 0\n");
        // else if (len)
        {
            // WritablePacket *p_csi = Packet::make(sizeof(my_test_struct)*N_CLIENT);
            
            for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry))
            {
                //one pkt per client
                WritablePacket *p_csi = Packet::make(11);
                j=0;
                e = (struct iwinfo_assoclist_entry *) &buf[i];
                uint8_t & mac = e->mac[5];
                int8_t & signal = e->signal;
                int8_t & noise = e->noise;
                uint32_t & rx_rate = (e->rx_rate).rate;
                uint32_t & tx_rate = (e->tx_rate).rate;

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
                output(1).push(p_csi); 
            }   
            
        }
    }
   
    if(p_in->length() > packet_length)
    {
        p_in->pull(p_in->length() - 140);
        output(0).push(p_in);
    }
    else
        p_in -> kill();
    
#endif 
}

void
CSICollect::push(int, Packet *p)
{
#ifdef __mips__ 
    // syslog (LOG_DEBUG, "CSICollect: in push\n");
	fragment(p);
#endif
}


CLICK_ENDDECLS
EXPORT_ELEMENT(CSICollect)