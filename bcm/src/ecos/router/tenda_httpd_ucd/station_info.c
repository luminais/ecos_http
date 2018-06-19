#include <stdlib.h>
#include <stdio.h>
 #include <unistd.h>

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmparams.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <netconf.h>
#include <nvparse.h>
#include <wlutils.h>


#include "route_cfg.h"
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"
#include "../tc/tc.h"

#define MAX_STA_COUNT	256
#define NVRAM_BUFSIZE	100

static void wl_list_station(webs_t wp, char *name, struct ether_addr *ea, int index, int is_2t2r,char_t  *type);
extern int get_wireless_station(int eid, webs_t wp, int argc, char_t **argv);


int get_wireless_station(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *type;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *name=NULL;
	struct maclist *mac_list;
	int mac_list_size;
	int i;

	int mode = 0;
	char wlif[64];
	wlc_rev_info_t rev;
	int is_2t2r = 0;

	char *wl_unit;
	
	if (ejArgs(argc, argv, T("%s"), &type) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}

	/*if(  0 != strcmp(type, "stationinfo")){
		return websWrite(wp, T("Insufficient args\n"));

	}*/

	snprintf(prefix, sizeof(prefix), "wl%d_", mode);

	name = nvram_safe_get(strcat_r(prefix, "ifname",wlif));
	wl_ioctl(name, WLC_GET_REVINFO, &rev, sizeof(rev));
	
	if (rev.chipnum == BCM5357_CHIP_ID) {
			is_2t2r = 1;
	}
	_GET_VALUE(WLN0_UNIT, wl_unit);
	
	snprintf(prefix, sizeof(prefix), "wl%s_", wl_unit);

	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	/* buffers and length */
	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);

	if (!mac_list){
		return 0;
	}
if(0 == strcmp(type, "macinfo") || 0 == strcmp(type, "stationinfo")){
	/* query wl0 for authenticated sta list */
	#ifndef __NetBSD__
		strcpy((char*)mac_list, "authe_sta_list");
	/*strcpy((char*)mac_list, "sta_info");*/
		if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size)) {
			free(mac_list);
			return 0;
		}
	#else
	/* NetBSD TBD... */
		mac_list->count=0;
	#endif
	/* query sta_info for each STA and output one table row each */
		for (i = 0; i < mac_list->count; i++) {
			wl_list_station(wp, name, &mac_list->ea[i], i,is_2t2r,type);
		}
	}
	else if(0 == strcmp(type,"macinfo2")){
		if(0==strcmp(name,"eth1"))
			name = "wl0.1";
		else 
			name = "eth1";
		#ifndef __NetBSD__
			strcpy((char*)mac_list, "authe_sta_list");
	/*strcpy((char*)mac_list, "sta_info");*/
			if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size)) {
				free(mac_list);
				return 0;
			}
		#else
	/* NetBSD TBD... */
			mac_list->count=0;
		#endif
		for (i = 0; i < mac_list->count; i++) {
		wl_list_station(wp, name, &mac_list->ea[i], i,is_2t2r,type);
		}	
	}
	
	free(mac_list);

	return 0;
}


/* Output one row of the HTML authenticated STA list table */
static void
wl_list_station(webs_t wp, char *name, struct ether_addr *ea, int index, int is_2t2r,char_t  *type)
{
	char buf[sizeof(sta_info_t)];
	char macbuf[32];
//	int rates_i;
	//assert(sizeof (buf) >= sizeof (sta_info_t));
	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (unsigned char *)ea, ETHER_ADDR_LEN);

	if (!wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf))) {
//		char ea_str[ETHER_ADDR_STR_LEN];
		sta_info_t *sta = (sta_info_t *)buf;
//		uint32 f = sta->flags;

		memset(macbuf,0,sizeof(macbuf));
		sprintf(macbuf,"%02X:%02X:%02X:%02X:%02X:%02X",
				sta->ea.octet[0],  sta->ea.octet[1],
				sta->ea.octet[2],  sta->ea.octet[3],
				sta->ea.octet[4],  sta->ea.octet[5]);
#if 0
		
		diag_printf("station hadware address:%s\n",	macbuf);
		diag_printf("version of this struct: %d\n", sta->ver);
		diag_printf("length in bytes of this structure: %d\n", sta->len);
		diag_printf("station's advertised capabilities: %d\n", sta->cap);
		diag_printf("flags defined below: %d\n", (sta->flags&WL_STA_N_CAP));
		diag_printf("time since data pkt rx'd from sta: %d\n", sta->idle);
		diag_printf("rx_decrypt_failures=%d,         rx_decrypt_succeeds=%d\n", sta->rx_decrypt_failures, sta->rx_decrypt_succeeds);
		diag_printf("rx_mcast_pkts=%d,               rx_ucast_pkts=%d\n", sta->rx_mcast_pkts, sta->rx_ucast_pkts);
		diag_printf("tx_rate=%d,                          rx_rate=%d\n", sta->tx_rate, sta->rx_rate);
		diag_printf("tx_pkts=%d,                          tx_failures=%d\n", sta->tx_pkts, sta->tx_failures);
		diag_printf("count=%d,                   \n", sta->rateset.count);
		diag_printf("sta->rateset.rates:   ");
		for(rates_i=0; rates_i<16; rates_i++)
			diag_printf("%u, ",sta->rateset.rates[rates_i]);
		diag_printf("\n");
		diag_printf("listen_interval_inms=%d\n\n\n", sta->listen_interval_inms);
#endif	
		if(strcmp(type, "stationinfo")==0){
			websWrite(wp,T("<tr><td width=20%% align=middle>%d</td>"),index+1);
			websWrite(wp,T("<td width=50%% align=middle>%s</td>"),macbuf);//mac
			//websWrite(wp,T("<td width=30%% align=middle>%s</td></tr>"),((sta->flags&WL_STA_N_CAP)!=WL_STA_N_CAP)? "20M":"40M");//PSM
			if(is_2t2r)/*2T2R*/
			websWrite(wp,T("<td width=30%% align=middle>%s</td></tr>"),((sta->tx_rate/1000) <= 130)? "20M":"40M");//PSM
			else	/*5356,1T1R*/
			websWrite(wp,T("<td width=30%% align=middle>%s</td></tr>"),((sta->tx_rate/1000) <= 65)? "20M":"40M");//PSM
		}
		/*ldm add*/
		 if(strcmp(type, "macinfo")==0)
		{
			if(index>0)
				websWrite(wp,T("%s"),",");
			websWrite(wp,T("%c"),0x27);
			websWrite(wp,T("%s"),macbuf);
			websWrite(wp,T("%c"),0x27);
		}
		 if(strcmp(type, "macinfo2")==0)
		{
			if(index>0)
				websWrite(wp,T("%s"),",");
			websWrite(wp,T("%c"),0x27);
			websWrite(wp,T("%s"),macbuf);
			websWrite(wp,T("%c"),0x27);
		}
	}
}
