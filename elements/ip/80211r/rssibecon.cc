/*
 element for 802.11r becon sending, 
 Created by Zhenyu Song: sunnyszy@gmail.com
 */

#include <click/config.h>
#include "rssibecon.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>

CLICK_DECLS

RSSIBecon::RSSIBecon()
{
#ifdef __mips__
    openlog("RSSIBecon", LOG_PERROR | LOG_CONS | LOG_NDELAY, 0);
    syslog (LOG_DEBUG, "finish init\n");
#endif 

}

RSSIBecon::~RSSIBecon()
{
#ifdef __mips__ 
    iwinfo_finish();
#endif
}

int
RSSIBecon::configure(Vector<String> &conf, ErrorHandler *errh)
{
    int wlan_port;
    if (Args(conf, this, errh)
      .read_p("WLANPORT", IntArg(), wlan_port)
      .complete() < 0)
    return -1;

#ifdef __mips__ 
    strcpy(ifname, "wlan0");
    
    iw = iwinfo_backend(ifname);
    if (!iw)
        syslog (LOG_DEBUG, "can not connect to backend iwinfo\n");
#endif
    syslog (LOG_DEBUG, "finish configure, ready to start\n");
    return 0;
}

void
RSSIBecon::fragment(Packet *p_in)
{
#ifdef __mips__ 
    int i,j;
    // syslog (LOG_DEBUG, "in fragment\n");
    if(!(iw->assoclist(ifname, buf, &len)))
    //     // syslog (LOG_DEBUG, "can not find associlist\n");
    // else if (len <= 0)
    //     // syslog (LOG_DEBUG, "associ number < 0\n");
    // else if (len)
    {
        // syslog (LOG_DEBUG, "prepare to send, len: %d\n", len);
        // WritablePacket *p_csi = Packet::make(sizeof(my_test_struct)*N_CLIENT);
        
        // for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry))
        if(len>0)
        {
            //one pkt per client
            WritablePacket *p_csi = Packet::make(11);
            // syslog (LOG_DEBUG, "creating pkt\n");
            j=0;
            e = (struct iwinfo_assoclist_entry *) &buf[i];
            uint8_t & mac = e->mac[5];
            int8_t & signal = e->signal;
            int8_t & noise = e->noise;
            uint32_t & rx_rate = (e->rx_rate).rate;
            uint32_t & tx_rate = (e->tx_rate).rate;
            // syslog (LOG_DEBUG, "parsing\n");
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
            // syslog (LOG_DEBUG, "copying\n");
            output(0).push(p_csi); 
        }   
    }
    // else
    //     syslog (LOG_DEBUG, "can not find associlist\n");
    // if (len <= 0)
    //     syslog (LOG_DEBUG, "associ number < 0\n");
#endif    
    p_in -> kill();
}

void
RSSIBecon::push(int, Packet *p)
{
    // syslog (LOG_DEBUG, "enter push\n");
#ifdef __mips__ 
    // syslog (LOG_DEBUG, "in push\n");
	fragment(p);
#endif
}


CLICK_ENDDECLS
EXPORT_ELEMENT(RSSIBecon)