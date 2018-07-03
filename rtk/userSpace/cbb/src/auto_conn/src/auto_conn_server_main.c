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
#include  "auto_conn_common.h"
#include  "wifi.h"


extern int tenda_auto_conn_vif_extender;
extern int AutoConnDebugLevel;
extern int device_name_24G; 
extern int device_name_5G;



cyg_mbox 		auto_conn_state_mbox;
cyg_handle_t 	auto_conn_state_mbox_handle;

#include "cJSON.h"

/*****************************************************************************
 函 数 名  : add_item
 功能描述  : 新增单条cjson数据
 输入参数  : char *name  nvram名称
             cJSON *root
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
void add_item(char *name, cJSON *root)
{
	if (!name)
	{
		return;
	}

	cJSON *obj = NULL;
	cJSON_AddStringToObject(root, name, nvram_safe_get(name));
}

/*****************************************************************************
 函 数 名  : make_cjson_data
 功能描述  : 根据需求组织需要的数据发送到客户端
 输入参数  : cJSON *root
 输出参数  : 无
 返 回 值  : cJSON

 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
cJSON *make_cjson_data(cJSON *root)
{
	//上网设置
	add_item(WAN0_PPPOE_USERNAME, root);//max_len 128
	add_item(WAN0_PPPOE_PASSWD, root);//max_len 128
	add_item(WAN0_PPPOE_SERVICE, root);//max_len 64
	add_item(WAN0_PROTO, root);//max_len 6
	//无线设置
	cJSON_AddStringToObject(root,WLAN24G_WORK_MODE ,"ap");//max_len 3
	add_item(WLAN24G_SSID, root);//max_len 32
	add_item(WLAN24G_PASSWD, root);//max_len 64
	add_item(WLAN24G_AKM, root);//max_len 9
	
	cJSON_AddStringToObject(root,WLAN5G_WORK_MODE ,"ap");//max_len 3
	add_item(WLAN5G_SSID, root);//max_len 32
	add_item(WLAN5G_PASSWD, root);//max_len 64
	add_item(WLAN5G_AKM, root);//max_len 9
		
#if 0 //会议决策去掉同步登陆密码2017/3/25
	//其他设置
	if (nvram_match("http_defaultpwd1", "1"))
	{
		add_item("http_passwd", root);//max_len 32
	}
#endif
	return root;
}

int set_auto_conn_state(int recv)
{
	if (recv < 0)
	{
		return -1;
	}

	if (auto_conn_state_mbox_handle && cyg_mbox_tryput(auto_conn_state_mbox_handle, (void *)(recv + 1)))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
static int first_doing_flag = 1;
int get_auto_conn_state(void)
{
	int recv;

	if (!router_is_doing_status())
	{
		if (0 == first_doing_flag)
		{
			first_doing_flag = 1;
		}
	
		recv = (int)cyg_mbox_get(auto_conn_state_mbox_handle) & 0xffff;
		return recv - 1;
	}
	else
	{
		//使用doing 状态使用cyg_mbox_timed_get代替cyg_mbox_get，作为超时关闭虚拟接口
		recv = (int)cyg_mbox_timed_get(auto_conn_state_mbox_handle, cyg_current_time() + 10* 100) & 0xffff;

		if (recv)
		{
			return recv - 1;
		}
		
		else
		{	
			if (0 == first_doing_flag)
			{
				set_auto_conn_state(TURN_OFF_VIR_IF);
			}
			first_doing_flag = 0;
			return (FLUSH_TIME);
		}
	}
}


int judge_extend_req(struct mbuf *rcv_mbuf, struct ether_header *eh)
{

	auto_conn_pkt *recv_buf = NULL;
	char *buf[32] = {0};

	if (rcv_mbuf == NULL)
	{
		printf("###func=%s,recv mbuf error!\n", __FUNCTION__);
		return 0;
	}

	memcpy(buf, rcv_mbuf->m_data, rcv_mbuf->m_len);
	crypt_decrypt_rc4(buf, rcv_mbuf->m_len, eh);

	recv_buf = (auto_conn_pkt *)buf;

	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s, recv req=%08x\n",
	                   __FUNCTION__, ntohl(recv_buf->rnd));

	if (0x1234 == ntohl(recv_buf->rnd))
	{
		return 1;
	}
	else if (0x1235 == ntohl(recv_buf->rnd))
	{
		return 2;
	}
	else
	{
		return 0;
	}
}

static int recv_pkt_times = 0;
struct recv_info
{
	struct ether_header recv_pkt_header;
	int rnd;
};
struct recv_info recv_pkt_info[10];
void recv_req_pkt(struct mbuf *m_recv, struct ether_header *eh)
{	
	int recv_flag = 0;
	int conf_type = judge_extend_req(m_recv, eh);
	if (ntohs(eh->ether_type) == 0x1234 && 0 != conf_type)
	{
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,recv config packet!   ", __FUNCTION__);
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "src_mac=%02x:%02x:%02x:%02x:%02x:%02x\t",
		                   (eh->ether_shost)[0], (eh->ether_shost)[1], (eh->ether_shost)[2],
		                   (eh->ether_shost)[3], (eh->ether_shost)[4], (eh->ether_shost)[5]);
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "dst_mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
		                   (eh->ether_dhost)[0], (eh->ether_dhost)[1], (eh->ether_dhost)[2],
		                   (eh->ether_dhost)[3], (eh->ether_dhost)[4], (eh->ether_dhost)[5]);

		{
			int i;

			for (i = 0; i < 10; i++)
			{
				if (recv_pkt_info[i].recv_pkt_header.ether_type != 0x1234)
				{
					memcpy(&(recv_pkt_info[i].recv_pkt_header), eh, sizeof(struct ether_header));

					if (1 == conf_type)
					{
						recv_pkt_info[i].rnd = 0x1234;
					}
					else if (2 == conf_type)
					{
						recv_pkt_info[i].rnd = 0x1235;
					}

					recv_flag = i;
					break;
				}
				else if (memcmp(&(recv_pkt_info[i].recv_pkt_header.ether_shost), eh->ether_shost, 6) == 0)
				{
					recv_flag = i;
					break;
				}
			}

			if (i == 10)
			{
				recv_flag = recv_pkt_times % 10;
				memcpy(&(recv_pkt_info[recv_flag].recv_pkt_header), eh, sizeof(struct ether_header));

				if (1 == conf_type)
				{
					recv_pkt_info[recv_flag].rnd = 0x1234;
				}
				else if (2 == conf_type)
				{
					recv_pkt_info[recv_flag].rnd = 0x1235;
				}

				recv_pkt_times++;
			}
		}
	 set_auto_conn_state(recv_flag);
	}
}

int make_24G_wifi_info(struct transmit_wifi_info *config_24G_info)
{

	char *wl_conf_crypto = NULL, *wl_conf_akm = NULL, *wl_conf_wep = NULL;
	char wl_akm_crypto[PI_BUFLEN_32] = {0};
	char wl_channel[8] = {0};
	unsigned int wl_channel_num = 0;
	int akm_index = 0;
	/*wireless encrypt type*/
	wl_conf_akm = nvram_recv_safe_get(WLAN24G_AKM);
	wl_conf_crypto = nvram_recv_safe_get(WLAN24G_CRYPTO);
	wl_conf_wep = nvram_recv_safe_get(WLAN24G_WEP );
	if (strcmp(wl_conf_akm, "") == 0)
	{
		config_24G_info->enTpye = htonl(NONE);
	}	
	else if (strcmp(wl_conf_akm, "psk") == 0)
	{
		if (strcmp(wl_conf_crypto, "aes") == 0)
		{
			config_24G_info->enTpye = htonl(WPA_AES);
		}
		else if (strcmp(wl_conf_crypto, "tkip") == 0)
		{
			config_24G_info->enTpye = htonl(WPA_TKIP);
		}
	}
	else if (strcmp(wl_conf_akm, "psk2") == 0)
	{
		if (strcmp(wl_conf_crypto, "aes") == 0)
		{
			config_24G_info->enTpye = htonl(WPA2_AES);
		}
		else if (strcmp(wl_conf_crypto, "tkip") == 0)
		{
			config_24G_info->enTpye = htonl(WPA2_TKIP);
		}
	}
	else if (strcmp(wl_conf_akm, "psk psk2") == 0)
	{
		if (strcmp(wl_conf_crypto, "aes") == 0)
		{
			config_24G_info->enTpye = htonl(WPA_WPA2_AES);
		}
		else if (strcmp(wl_conf_crypto, "tkip") == 0)
		{
			config_24G_info->enTpye = htonl(WPA_WPA2_TKIP);
		}
		else if (strcmp(wl_conf_crypto, "tkip+aes") == 0)
		{
			config_24G_info->enTpye = htonl(WPA_WPA2_AES_TKIP);
		}
	}

	/*copy ssid & password*/
	strcpy(config_24G_info->ssid, nvram_recv_safe_get(WLAN24G_SSID));
	strcpy(config_24G_info->passwd, nvram_recv_safe_get(WLAN24G_PASSWD));


	return sizeof(config_24G_info->ssid) + sizeof(config_24G_info->enTpye) + sizeof(config_24G_info->channel) + strlen(config_24G_info->passwd) + 1; //contain '\0'

}


int make_5G_wifi_info(struct transmit_wifi_info * config_5G_info)
{

	char *wl_conf_crypto = NULL, *wl_conf_akm = NULL, *wl_conf_wep = NULL;
	char wl_channel[8] = {0};
	unsigned int wl_channel_num = 0;
	wl_conf_akm = nvram_recv_safe_get(WLAN5G_AKM);
	wl_conf_crypto = nvram_recv_safe_get(WLAN5G_CRYPTO);
	wl_conf_wep = nvram_recv_safe_get(WLAN5G_WEP);
	if (strcmp(wl_conf_akm, "") == 0)
	{
		config_5G_info->enTpye = htonl(NONE);
	}
	else if (strcmp(wl_conf_akm, "psk") == 0)
	{
		if (strcmp(wl_conf_crypto, "aes") == 0)
		{
			config_5G_info->enTpye = htonl(WPA_AES);
		}
		else if (strcmp(wl_conf_crypto, "tkip") == 0)
		{
			config_5G_info->enTpye = htonl(WPA_TKIP);
		}
	}
	else if (strcmp(wl_conf_akm, "psk2") == 0)
	{
		if (strcmp(wl_conf_crypto, "aes") == 0)
		{
			config_5G_info->enTpye = htonl(WPA2_AES);
		}
		else if (strcmp(wl_conf_crypto, "tkip") == 0)
		{
			config_5G_info->enTpye = htonl(WPA2_TKIP);
		}
	}
	else if (strcmp(wl_conf_akm, "psk psk2") == 0)
	{
		if (strcmp(wl_conf_crypto, "aes") == 0)
		{
			config_5G_info->enTpye = htonl(WPA_WPA2_AES);
		}
		else if (strcmp(wl_conf_crypto, "tkip") == 0)
		{
			config_5G_info->enTpye = htonl(WPA_WPA2_TKIP);
		}
		else if (strcmp(wl_conf_crypto, "tkip+aes") == 0)
		{
			config_5G_info->enTpye = htonl(WPA_WPA2_AES_TKIP);
		}
	}	
	/*copy ssid & password*/
	strcpy(config_5G_info->ssid, nvram_recv_safe_get(WLAN5G_SSID));
	strcpy(config_5G_info->passwd, nvram_recv_safe_get(WLAN5G_PASSWD));

	return sizeof(config_5G_info->ssid) + sizeof(config_5G_info->enTpye) + sizeof(config_5G_info->channel) + strlen(config_5G_info->passwd) + 1; //contain '\0'

}

//P_WIFI_CURRCET_INFO_STRUCT tpi_wifi_get_curret_info(P_WIFI_CURRCET_INFO_STRUCT cfg)

extern int send_ether_packet(struct ifnet *send_ifnet, struct ether_header *eh_rcv_pkt, char *msg, int len);

int send_conf_pkt(struct ifnet *send_ifnet, struct recv_info *recv_pkt_info)
{
	struct sockaddr dst;
	struct ether_header *eh , *eh_rcv_pkt;

	char sndBuf[CONF_BUF_LEN] = {0};
	auto_conn_pkt *tmp = (auto_conn_pkt *)sndBuf;
	char *data;
	int config_len = 0;
	int sndLen = 0;


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

	eh_rcv_pkt = &(recv_pkt_info->recv_pkt_header);

	//memcpy(eh->ether_dhost, "\xff\xff\xff\xff\xff\xff", sizeof(eh->ether_dhost));
	memcpy(eh->ether_dhost, eh_rcv_pkt->ether_shost, 6);

	/*
	*generate a packet
	*/
	
	if (0x1234 == recv_pkt_info->rnd)
	{	

		tmp->rnd = htonl(0x1234);
		tmp->ack = htonl(0x4321);
		if(device_name_24G == AUTO_CONN_VIF_24G)
		{
			struct transmit_wifi_info wifi_24G_info;
			memset(&wifi_24G_info, 0x0, sizeof(wifi_24G_info));
			config_len = make_24G_wifi_info(&wifi_24G_info);//获得服务器的2.4G WiFibasic
			AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,ssid=%s, password=%s, passType=%d, channel=%d---\n",
							   __FUNCTION__, wifi_24G_info.ssid, wifi_24G_info.passwd, ntohl(wifi_24G_info.enTpye), ntohl(wifi_24G_info.channel));
			data = (char *)&tmp->data;
			memcpy(data, &wifi_24G_info, config_len);
		}
		else
		{
			struct transmit_wifi_info wifi_5G_info;
			memset(&wifi_5G_info, 0x0, sizeof(wifi_5G_info));
			config_len = make_5G_wifi_info(&wifi_5G_info);//获得服务器的5G WiFibasic
			AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,ssid=%s, password=%s, passType=%d, channel=%d---\n",
							   __FUNCTION__, wifi_5G_info.ssid, wifi_5G_info.passwd, ntohl(wifi_5G_info.enTpye), ntohl(wifi_5G_info.channel));
			data = (char *)&tmp->data;
			memcpy(data, &wifi_5G_info, config_len);
		}

	}
	
	else if (0x1235 == recv_pkt_info->rnd)
	{
		int send_count = atoi(nvram_safe_get("auto_sync_count"));
		if(0 == send_count)
		{
			cJSON *root = NULL;
			char *cjson_data;
			root = cJSON_CreateObject();

			if (!root)
			{
				return 1;
			}

			make_cjson_data(root);
			cjson_data = cJSON_Print(root);
			cJSON_Delete(root);

			config_len = strlen(cjson_data);

			AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,cjson:%s---len = %d\n",
			                   __FUNCTION__, cjson_data, config_len);
			tmp->rnd = htonl(0x1235);
			tmp->ack = htonl(0x4321);
			data = (char *)&tmp->data;

			memcpy(data, cjson_data, config_len);
			free(cjson_data);
		}
		else
		{
			AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "func:%s,auto_sync_count:%d\n",__FUNCTION__, send_count);              
			return 1;
		}
	}
	else
	{
		config_len = 0;
	}


	sndLen = sizeof(auto_conn_pkt) - 4 + config_len ;

	if (sndLen > CONF_BUF_LEN)
	{
		printf("sndLen is too long!\n");
		return 1;
	}

	tmp->dataLen = config_len;
	tmp->dataLen = htonl(tmp->dataLen);

	crypt_decrypt_rc4(tmp, sndLen, eh);	//rc4 encrypt
	send_ether_packet(send_ifnet, eh, sndBuf, sndLen);

	//成功同步配置一次之后不在启用配置自动同步
	if (0x1235 == recv_pkt_info->rnd)
	{
		int count = 0;
		char mac[] = "xx:xx:xx:xx:xx:xx";
		char str_count[8] = {0};
		count = atoi(nvram_safe_get("auto_sync_count"));
		printf("auto_sync_count : %d client :%s \n",count,ether_etoa(eh_rcv_pkt->ether_shost,mac));
		count ++;
		sprintf(str_count,"%d",count);
		nvram_set("auto_sync_count", str_count);
		nvram_commit();
	}
	return 1;
}

//extern void init_array();
extern struct ifnet *ifunit2(const char *name);
extern void auto_conn_vif_control(unsigned int on);

void auto_conn_route_main()
{
	int recv_state = 0;
	struct ifnet *send_ifnet_24G = NULL;
	struct ifnet *send_ifnet_5G = NULL;
	cyg_thread_delay(300);
	

#if defined(RTL819X)
	send_ifnet_24G = ifunit2(TENDA_WLAN24_VIRTUAL_IFNAME);
	send_ifnet_5G = ifunit2(TENDA_WLAN5_VIRTUAL_IFNAME);
	if (send_ifnet_24G == NULL || send_ifnet_24G == 0 || send_ifnet_5G == NULL || send_ifnet_5G == 0 )
	{
		printf("###func=%s,look up error!!!\n", __FUNCTION__);
		return;
	}
#endif

	router_set_undo_status();
	/* Init event box */
	cyg_mbox_create(&auto_conn_state_mbox_handle, &auto_conn_state_mbox);

	while (1)
	{	

		recv_state = get_auto_conn_state();
		AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###func=%s,recv_conf_type=%d---\n",
		                   __FUNCTION__, recv_state);

		switch (recv_state)
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
				{
					if(device_name_24G == AUTO_CONN_VIF_24G)
					{
						send_conf_pkt(send_ifnet_24G, &(recv_pkt_info[recv_state]));
					}
					if(device_name_5G == AUTO_CONN_VIF_5G)
					{
						send_conf_pkt(send_ifnet_5G, &(recv_pkt_info[recv_state]));
					}
				}
				break;

			case TURN_OFF_VIR_IF:	//turn off vir if
				if (!router_is_undo_status())
				{
					auto_conn_vif_control(0);
				}

				//init_array();
				router_set_undo_status();
				break;

			case TURN_ON_VIR_IF:	//turn on vir if
				if(!router_is_doing_status())
				{
					auto_conn_vif_control(1);
				}
				router_set_doing_status();
				break;
			case FLUSH_TIME:
				break;
			
			case AUTO_CONNECT_SERVER_STOP:
				cyg_thread_exit();

			default:
				break;
		}
	}
}

void auto_conn_route_stop()
{
	set_auto_conn_state(AUTO_CONNECT_SERVER_STOP);
}

#if defined(BCM535X)
static cyg_handle_t auto_conn_route_handle;
static cyg_thread 	auto_conn_route_thread;
static char 		auto_conn_route_stack[1024 * 64];

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
#endif

