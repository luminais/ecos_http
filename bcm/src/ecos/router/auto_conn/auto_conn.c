#include <stdio.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include <net/if_var.h>

#include <sys/mbuf.h>
#include <net/route.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/param.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/sockio.h>
#include <autoconf.h>
#include  "../rc/rc.h"
#include  "auto_conn.h"

extern int tenda_auto_conn_vif_extender;
extern int AutoConnDebugLevel;

cyg_mbox 		auto_conn_state_mbox;
cyg_handle_t 	auto_conn_state_mbox_handle;

int
set_auto_conn_state(int recv)
{
	if (auto_conn_state_mbox_handle && cyg_mbox_tryput(auto_conn_state_mbox_handle, (void *)recv))
		return 0;
	else
		return -1;
}

int
get_auto_conn_state(void)
{
	return ((int)cyg_mbox_get(auto_conn_state_mbox_handle) & 0xffff);
}

int 
crypt_decrypt_rc4(unsigned char *inbuf, int len, struct ether_header *eh)
{
	rc4_ks_t key;
	unsigned char *outbuf = NULL;
	char key_str[KEY_STR_LEN+1] = {0};
	//char data[256] = {0};

	if(!inbuf || len < 0)
	{
		printf("%s : Bad parameter.\n", __FUNCTION__);
		return -1;
	}

	outbuf = (unsigned char*)malloc(len+1);
	if(NULL == outbuf )
	{
		printf("%s : No memory.\n", __FUNCTION__);
		return -1;
	}
	
	#if 1
	memcpy(outbuf, inbuf, len);
	snprintf(key_str, KEY_STR_LEN, "Tenda_%02x%02x%02x%02x%02x%02x", 
		eh->ether_dhost[0]&0xff, eh->ether_dhost[1]&0xff, eh->ether_dhost[2]&0xff,
		eh->ether_dhost[3]&0xff, eh->ether_dhost[4]&0xff, eh->ether_dhost[5]&0xff);
	#else
	memcpy(outbuf, inbuf, len);
	memcpy(key_str, "123456789012", KEY_STR_LEN);
	#endif
	
	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###key_str=%s, before encrypt:%02x\n", key_str, *inbuf);
	prepare_key(key_str, KEY_STR_LEN, &key);
	rc4(outbuf, len, &key);

	memcpy(inbuf, outbuf, len);
	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###after encrypt:%02x\n", *inbuf);

	free(outbuf);
	return len;
}


int 
judge_extend_req(struct mbuf *rcv_mbuf, struct ether_header *eh)
{
	
	auto_conn_pkt *recv_buf = NULL;
	char *buf[32] = {0};
	
	if(rcv_mbuf == NULL)
	{
		printf("###func=%s,recv mbuf error!\n", __FUNCTION__);
		return;
	}

	memcpy(buf, rcv_mbuf->m_data, rcv_mbuf->m_len);
	crypt_decrypt_rc4(buf, rcv_mbuf->m_len, eh);

	recv_buf = (auto_conn_pkt*)buf;

	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s, recv req=%08x\n", 
		__FUNCTION__, ntohl(recv_buf->rnd));
	if(0x1234 == ntohl(recv_buf->rnd))
		return 1;
	
	return 0;
}


time_t first_recv_tm = 0;
static int recv_pkt_times = 0;
struct ether_header recv_pkt_header[10];
void 
recv_req_pkt(struct mbuf *m_recv, struct ether_header *eh)
{
	static int first_doing_flag = 1;
	int recv_flag = 0;
	
	if(ntohs(eh->ether_type) == 0x1234 && 1 == judge_extend_req(m_recv, eh))
	{
		first_recv_tm = time(NULL);

		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,recv config packet!   ", __FUNCTION__);
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "src_mac=%02x:%02x:%02x:%02x:%02x:%02x\t", 
			(eh->ether_shost)[0], (eh->ether_shost)[1], (eh->ether_shost)[2], 
			(eh->ether_shost)[3], (eh->ether_shost)[4], (eh->ether_shost)[5]);
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "dst_mac=%02x:%02x:%02x:%02x:%02x:%02x\n", 
			(eh->ether_dhost)[0], (eh->ether_dhost)[1], (eh->ether_dhost)[2], 
			(eh->ether_dhost)[3], (eh->ether_dhost)[4], (eh->ether_dhost)[5]);

		{
			int i;
			for(i=0;i<10;i++)
			{
				if(recv_pkt_header[i].ether_type != 0x1234)
				{
					memcpy(&(recv_pkt_header[i]), eh, sizeof(struct ether_header));
					recv_flag = i;
					break;
				}
				else if(memcmp(&(recv_pkt_header[i].ether_shost), eh->ether_shost, 6) == 0)
				{
					recv_flag = i;
					break;
				}
			}

			if(i == 10)
			{
				recv_flag = recv_pkt_times%10;
				memcpy(&(recv_pkt_header[recv_flag]), eh, sizeof(struct ether_header));
				recv_pkt_times++;
			}
		}
		set_auto_conn_state(recv_flag);
	}
	else
	{
		if(first_doing_flag == 1)
		{
			first_recv_tm = time(NULL);
			first_doing_flag = 0;
		}
		else if(time(NULL) - first_recv_tm > 60*3)
		{
			set_auto_conn_state(TURN_OFF_VIR_IF);
			first_doing_flag = 1;
		}
	}
}

extern void get_channel(char *buf);
extern int 
send_ether_packet(struct ifnet *send_ifnet, struct ether_header *eh_rcv_pkt, char *msg, int len);
int send_conf_pkt(struct ifnet *send_ifnet, struct ether_header *eh_rcv_pkt)
{
	struct sockaddr dst;
	struct ether_header *eh;

	char sndBuf[1024] = {0};
	auto_conn_pkt *tmp = (auto_conn_pkt *)sndBuf;
	struct wifi_info info;
	char *data;
	int wifiLen = 0;
	int sndLen = 0;
	char *wl_conf_mode = NULL, *wl_conf_crypto = NULL, *wl_conf_akm = NULL, *wl_conf_wep = NULL;
	char wl_channel[8] = {0};
	unsigned int wl_channel_num = 0;

reply:	
	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###%s : Reply \n", __FUNCTION__);	

	/*
	* Set destination to send and tell
	* the ether_output() to do raw send
	* without routing for us.
	*/
	memset(&dst, 0, sizeof(dst));
	dst.sa_len = 16;
	dst.sa_family = AF_UNSPEC;

	eh = (struct ether_header *)dst.sa_data;
	eh->ether_type = ntohs(0x1234);

	//memcpy(eh->ether_dhost, "\xff\xff\xff\xff\xff\xff", sizeof(eh->ether_dhost));
	memcpy(eh->ether_dhost, eh_rcv_pkt->ether_shost, 6);

	/*
	*generate a packet
	*/
	memset(&info, 0x0, sizeof(info));

	/*wireless encrypt type*/
	wl_conf_mode = nvram_recv_safe_get("wl0_mode");
	if(strcmp(wl_conf_mode, "ap") == 0)
	{
		wl_conf_akm = nvram_recv_safe_get("wl0_akm");
		wl_conf_crypto = nvram_recv_safe_get("wl0_crypto");
		wl_conf_wep = nvram_recv_safe_get("wl0_wep");

	}
	else
	{
		wl_conf_akm = nvram_recv_safe_get("wl0.1_akm");
		wl_conf_crypto = nvram_recv_safe_get("wl0.1_crypto");
		wl_conf_wep = nvram_recv_safe_get("wl0.1_wep");
	}
	
	if(strcmp(wl_conf_akm, "") == 0)
	{
		info.enTpye = htonl(NONE);
	}
	else if(strcmp(wl_conf_akm, "psk") == 0)
	{
		if(strcmp(wl_conf_crypto, "aes") == 0)
		{
			info.enTpye = htonl(WPA_AES);
		}
		else if(strcmp(wl_conf_crypto, "tkip") == 0)
		{
			info.enTpye = htonl(WPA_TKIP);
		}
	}
	else if(strcmp(wl_conf_akm, "psk2") == 0)
	{
		if(strcmp(wl_conf_crypto, "aes") == 0)
		{
			info.enTpye = htonl(WPA2_AES);
		}
		else if(strcmp(wl_conf_crypto, "tkip") == 0)
		{
			info.enTpye = htonl(WPA2_TKIP);
		}
	}
	else if(strcmp(wl_conf_akm, "psk psk2") == 0)
	{
		if(strcmp(wl_conf_crypto, "aes") == 0)
		{
			info.enTpye = htonl(WPA_WPA2_AES);
		}
		else if(strcmp(wl_conf_crypto, "tkip") == 0)
		{
			info.enTpye = htonl(WPA_WPA2_TKIP);
		}
		else if(strcmp(wl_conf_crypto, "tkip+aes") == 0)
		{
			info.enTpye = htonl(WPA_WPA2_AES_TKIP);
		}
	}
	else if(strcmp(wl_conf_wep, "enabled") == 0)
	{
		info.enTpye = htonl(WEP);
	}
	
	/*channel*/
	get_channel(wl_channel);
	wl_channel_num = atoi(wl_channel);
	if(wl_channel_num < 14)
		info.channel = htonl(wl_channel_num);
	else
		info.channel = htonl(0);

	/*copy ssid & password*/
	if(strcmp(wl_conf_mode, "ap") == 0)
	{
		strcpy(info.ssid, nvram_recv_safe_get("wl0_ssid"));
		strcpy(info.passwd, nvram_recv_safe_get("wl0_wpa_psk"));
	}
	else
	{
		strcpy(info.ssid, nvram_recv_safe_get("wl0.1_ssid"));
		strcpy(info.passwd, nvram_recv_safe_get("wl0.1_wpa_psk"));
	}
	wifiLen = sizeof(info.ssid) + sizeof(info.enTpye) + sizeof(info.channel) + strlen(info.passwd) + 1; //contain '\0'

	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,ssid=%s, password=%s, passType=%d, channel=%d---\n",
		__FUNCTION__, info.ssid, info.passwd, ntohl(info.enTpye), ntohl(info.channel));
	
	tmp->rnd = htonl(0x1234);
	tmp->ack = htonl(0x4321);
	data = (char *)&tmp->data; 

	memcpy(data, &info, wifiLen);
	sndLen = sizeof(auto_conn_pkt) - 4 + wifiLen ;

	tmp->dataLen = wifiLen;	
	tmp->dataLen = htonl(tmp->dataLen);

	crypt_decrypt_rc4(tmp, sndLen, eh);	//rc4 encrypt
	send_ether_packet(send_ifnet, eh, sndBuf, sndLen);
	
	return 1;
}

extern void init_array();
extern struct ifnet *ifunit2(const char *name);
extern void auto_conn_vif_control(unsigned int on);
void auto_conn_route_main()
{
	int recv_state = 0;
	struct ifnet *send_ifnet = NULL;
	
	cyg_thread_delay(300);
	
	send_ifnet = ifunit2("br0");
	if (send_ifnet == NULL || send_ifnet == 0)
	{
		printf("###func=%s,look up error!!!\n", __FUNCTION__);
		return;	
	}
	
	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,ifindex=%d, if_name=%s, if_xname=%s---\n",
		__FUNCTION__, send_ifnet->if_index, send_ifnet->if_name, send_ifnet->if_xname);

	set_undo_status();
	/* Init event box */
	cyg_mbox_create(&auto_conn_state_mbox_handle, &auto_conn_state_mbox);
	
	while(1)
	{
		recv_state = get_auto_conn_state();
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,recv_conf_type=%d---\n", 
			__FUNCTION__, recv_state);
		
		switch(recv_state)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				send_conf_pkt(send_ifnet, &(recv_pkt_header[recv_state]));
				break;
			case TURN_OFF_VIR_IF:	//turn off vir if
				if(!is_undo_status())
					auto_conn_vif_control(0);
				init_array();
				set_undo_status();
				break;
			case TURN_ON_VIR_IF:	//turn on vir if
				if(!is_doing_status())
					auto_conn_vif_control(1);
				set_doing_status();
				first_recv_tm = time(NULL);
				break;
			case FLUSH_TIME:
				first_recv_tm = time(NULL);
				break;
			default:
				break;
		}
	}
}


static cyg_handle_t auto_conn_route_handle;
static cyg_thread 	auto_conn_route_thread;
static char 		auto_conn_route_stack[1024*64];

void auto_conn_route_start()
{
	cyg_thread_create(
		8,
		(cyg_thread_entry_t *)auto_conn_route_main,
		0,
		"auto_conn_route",
		(void *)&auto_conn_route_stack[0],
		sizeof(auto_conn_route_stack),
		&auto_conn_route_handle,
		&auto_conn_route_thread);
	
	cyg_thread_resume(auto_conn_route_handle);
}

