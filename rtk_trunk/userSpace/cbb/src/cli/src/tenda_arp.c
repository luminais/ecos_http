/*************************************************************************
	> Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
	> File Name: userSpace/cbb/src/cli/src/tenda_arp.c
	> Description: 
	> Author: zhuhuan
	> Mail: zhuhuan_IT@outlook.com
	> Version: 1.0
	> Created Time: Sat 19 Mar 2016 10:17:50 AM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_var.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <shutils.h>
#include <malloc.h>
#include <wlioctl.h>
#include <shutils.h>
#include <wl_utility_rltk.h>
#include <tenda_arp.h>
#include <pi_common.h>
#include <bcmnvram.h>

struct detec *det[TENDA_ARP_MAX_NUM];
struct detec *expired_list;

cyg_mutex_t expired_list_lock;

/* Function declaration */
static inline unsigned int tenda_arp_hash(const unsigned char *mac);
static int tenda_arp_add_node(struct detec **tenda_arp_list, const unsigned char *mac_addr, const in_addr_t *ip_addr, const u_long *expire_time);
static int tenda_arp_delete_node(struct detec **tenda_arp_list, const in_addr_t *ip_addr);
static int tenda_arp_modify_node(struct detec *tenda_arp_list, const in_addr_t *ip_addr);
static char *inet_ntoa_r_tenda(in_addr_t ina, char *buf);
static struct detec *tenda_arp_create_node(const unsigned char *mac_addr, const in_addr_t *ip_addr, const u_long *expire_time);
static int tenda_arp_dump(struct radix_node *rn, void *req);
static int tenda_arp_show_table(void);
static void tenda_arp_show_expired_list(void);
extern int iflib_getifaddr(char *ifname, struct in_addr *ipaddr, struct in_addr *netmask);
extern size_t strlcpy(dest, src, len);
/* End declaration */

void tenda_arp_init(void)
{
	cyg_mutex_init(&expired_list_lock);
	
	memset((char *)det, 0x0, TENDA_ARP_MAX_NUM * sizeof(det[0]));
	expired_list = NULL;
}
/*****************************************************************************
 函 数 名  : tenda_arp_is_wireless_client
 功能描述  : 返回客户端类型
 输入参数  : unsigned char *mac  
 输出参数  : 无
 返 回 值  : 0：有线
 			   1:    2.4G
 			   2:    5G
 			   3:    2.4G_GUEST
 			   4:	   5G_GUEST
 
 修改历史      :
  1.日    期   : 2017年11月13日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int tenda_arp_is_wireless_client(unsigned char *mac)
{
	struct maclist *mac_list;
	int mac_list_size;
	int i = 0;
	int j = 0;
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
#ifdef __CONFIG_GUEST__
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
#endif
					""};

	
	/* buffers and length */
	mac_list_size = sizeof(uint) + MAX_STA_NUM * sizeof(struct ether_addr);
	mac_list = (struct maclist *)malloc(mac_list_size);

	if (NULL == mac_list)
	{
		return 0;
	}
/*lq 没有进行判断，出现死机情况*/
	if(mac == NULL)
	{
		free(mac_list);		
		return 0;
	}
	/* query wl0 for authenticated sta list */

/*lq 修改查找无线客户端的时候通过遍历无线接口查询*/
	for(j = 0;strcmp(ifname[j],"");j++)
	{
		if(INTERFACE_UP != get_interface_state(ifname[j]))
			continue;

		memset(mac_list,0x0,mac_list_size);
		
		if (getwlmaclist(ifname[j], (char *)mac_list))
		{
			free(mac_list);
			return 0;
		}

		/* query sta_info for each STA and output one table row each */
		for (i = 0; i < mac_list->count; i++)
		{
			if(!memcmp(mac_list->ea[i]. octet, mac, ETHER_ADDR_LEN))
			{
			 	free(mac_list);
			 	return j + 1;   
				//根据上面的数组的索引返回
			}
		}
	}

	
	free(mac_list);

	return 0;
}

/*
 * 本函数目前没有使用，用mac地址来替换ip来进行查找
 * mac format: aa:bb:cc:dd:ee:ff
 */
int tenda_arp_get_auth(unsigned char* mac)
{
	unsigned char mac_addr[ETHER_ADDR_LEN] = {0x0};
	struct detec *tmp_node = NULL;
	
	ether_atoe(mac, mac_addr);

	int mac_index = tenda_arp_hash(mac_addr);
	tmp_node = det[mac_index];

	/* walk this list and find out whether the node is exist */
	while(tmp_node)
	{
		/* return if the node is found successfully */
		if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
		{
			return tmp_node->auth;
		}

		tmp_node = tmp_node->next;
	}

    return -1;
}      

void tenda_arp_clear_auth_all(void)
{
    int i;
	struct detec *tmp_node = NULL;
	
    for(i=0; i<TENDA_ARP_SIZE(det); i++)                                                                                                                                                   
    {
    	tmp_node = det[i];

		/* walk this list and clear auth for every node */
		while(tmp_node)
		{
			tmp_node->auth = 0;

			tmp_node = tmp_node->next;
		}                                                                                                                                                          
    }                                                                                                                                                                                                                                                                                                                           
}

//ip to mac
unsigned char *tenda_arp_ip_to_mac(in_addr_t ip)
{
	int i;
	static unsigned char mac_addr[ETHER_ADDR_LEN];
	struct detec *tmp_node = NULL;

	for(i=0; i < TENDA_ARP_SIZE(det); i++)																																					 
	{
		tmp_node = det[i];

		/* walk this list and find out whether the node is exist */
		while(tmp_node)
		{
			/* return if the node is found successfully */
			if(tmp_node->ip == ip)
			{
				memset(mac_addr, 0x0, ETHER_ADDR_LEN);
				memcpy(mac_addr, tmp_node->mac, ETHER_ADDR_LEN);
				
           		return mac_addr;
			}

			tmp_node = tmp_node->next;
		}
	}

	return NULL;
}

/*
 * mac to ip
 * mac format: aa:bb:cc:dd:ee:ff
 */
in_addr_t tenda_arp_mac_to_ip(char *mac)
{

	unsigned char mac_addr[ETHER_ADDR_LEN] = {0x0};
	struct detec *tmp_node = NULL;
	
	ether_atoe(mac, mac_addr);

	int mac_index = tenda_arp_hash(mac_addr);
	tmp_node = det[mac_index];

	/* walk this list and find out whether the node is exist */
	while(tmp_node)
	{
		/* return if the node is found successfully */
		if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
		{
			return tmp_node->ip;
		}

		tmp_node = tmp_node->next;
	}

	return 0;
}

/*
 * mac to hostname
 * mac format: aa:bb:cc:dd:ee:ff
 */
char *tenda_arp_mac_to_hostname(unsigned char *mac)
{
	unsigned char mac_addr[ETHER_ADDR_LEN] = {0x0};
	static char hostname[PI_BUFLEN_64];
	struct detec *tmp_node = NULL;
	
	ether_atoe(mac, mac_addr);
	
	int mac_index = tenda_arp_hash(mac_addr);
	tmp_node = det[mac_index];

	/* walk this list and find out whether the node is exist */
	while(tmp_node)
	{
		/* return if the node is found successfully */
		if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
		{
			memset(hostname, 0x0, PI_BUFLEN_64 * sizeof(char));
			strlcpy(hostname, tmp_node->hostname, sizeof(hostname));
			return hostname;
		}
		
		tmp_node = tmp_node->next;
	}

	/*2017/2/5修改 by lrl 添加获取离线设备的主机名*/
	cyg_mutex_lock(&expired_list_lock);
    tmp_node = expired_list;
	while(tmp_node)
	{
		if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
		{
			memset(hostname, 0x0, PI_BUFLEN_64 * sizeof(char));
			strlcpy(hostname, tmp_node->hostname, sizeof(hostname));
			cyg_mutex_unlock(&expired_list_lock);
			return hostname;
		}
		tmp_node = tmp_node->next;
	}	
	cyg_mutex_unlock(&expired_list_lock);
	//查到不到时返回NULL,  跟静态接入无主机名区分开来
	return NULL;
}

/*
 * mac to flag
 * mac format: aa:bb:cc:dd:ee:ff
 */
int tenda_arp_mac_to_flag(unsigned char *mac)
{
	unsigned char mac_addr[ETHER_ADDR_LEN] = {0x0};
	struct detec *tmp_node = NULL;
	
	ether_atoe(mac, mac_addr);

	int mac_index = tenda_arp_hash(mac_addr);
	tmp_node = det[mac_index];

	/* walk this list and find out whether the node is exist */
	while(tmp_node)
	{
		/* return if the node is found successfully */
		if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
		{
			return tmp_node->flag;
		}

		tmp_node = tmp_node->next;
	} 
	
	return 0;
}

int tenda_arp_ip_to_flag(in_addr_t ip)
{
	int i;
	struct detec *tmp_node = NULL;
	
    for(i=0; i < TENDA_ARP_SIZE(det); i++)                                                                                                                                                   
    {
   		tmp_node = det[i];

		/* walk this list and find out whether the node is exist */
		while(tmp_node)
		{
			/* return if the node is found successfully */
			if(tmp_node->ip == ip)
			{
				return tmp_node->flag;
			}

			tmp_node = tmp_node->next;
		}                                                                                                                                                           
    }
	
	return 0;
}

unsigned int tenda_arp_get_online_client_num(void)
{
	int i, client_num;
	struct detec *tmp_node = NULL;
	
	for(client_num = 0, i = 0; i < TENDA_ARP_SIZE(det); i++)																																					 
	{
		tmp_node = det[i];
		/* walk this list */
		while(tmp_node)
		{
			if(0 == tmp_node->flag)
			{
				++client_num;
			}
			else if((0 != tmp_node->flag) && (tenda_arp_is_wireless_client(tmp_node->mac)))
			{
				++client_num;
			}

			tmp_node = tmp_node->next;
		}
	}
	
	return client_num;
}

int tenda_arp_is_online(in_addr_t ip)
{
	int i;
	struct detec *tmp_node = NULL;
	
    for(i=0; i < TENDA_ARP_SIZE(det); i++)                                                                                                                                                   
    {
    	tmp_node = det[i];

		/* walk this list and find out whether the node is exist */
		while(tmp_node)
		{
			/* return if the node is found successfully */
			if(tmp_node->ip == ip)
			{
				return 1;
			}

			tmp_node = tmp_node->next;
		}
    }

	return -1;
}

int tenda_arp_is_multicast(unsigned char *mac) 		// 地址为全0的判断为非法地址
{
	if(mac == NULL)
		return -1;

	if(	mac[0] == 0 &&
		mac[1] == 0 &&
		mac[2] == 0 &&
		mac[3] == 0 &&
		mac[4] == 0 &&
		mac[5] == 0)
		return 1;
	else
		return 0;
}

static int tenda_arp_check_ip_validity(const in_addr_t lan_addr,const in_addr_t lan_mask,const in_addr_t ip_addr)
{
	unsigned int index = 0;
	if(lan_addr == ip_addr)
		return 0;

	
	index = ntohl((ip_addr & lan_mask)^ip_addr);
	if(index < 1 || index > 254)
		return 0;

	if((lan_addr & lan_mask) == (ip_addr & lan_mask))
	{
		return 1;
	}
	return 0;
}

/*判断添加的IP是否是LAN测的*/
inline int tenda_arp_isnot_lanhost(in_addr_t ip)
{
	struct in_addr ipaddr,ipmask;
	
	if(0 != iflib_getifaddr("eth0", &ipaddr, &ipmask))
	{
		ipaddr.s_addr = inet_addr(nvram_safe_get("lan_ipaddr"));
		ipmask.s_addr = inet_addr(nvram_safe_get("lan_netmask"));
	}
	if(tenda_arp_check_ip_validity(ipaddr.s_addr,ipmask.s_addr,ip))
	{
		return 0;
	}
#ifdef __CONFIG_GUEST__
	if(nvram_match(WLAN24G_GUEST_ENABLE,"1") 
		||nvram_match(WLAN5G_GUEST_ENABLE,"1"))
	{
		ipaddr.s_addr = inet_addr(nvram_safe_get("lan1_ipaddr"));
		ipmask.s_addr = inet_addr(nvram_safe_get("lan1_netmask"));

		if(tenda_arp_check_ip_validity(ipaddr.s_addr,ipmask.s_addr,ip))
		{
			return 0;
		}
	}
#endif
	return 1;
}

static int tenda_arp_check_validity(const unsigned char*mac_addr,const in_addr_t *ip_addr)
{
	char broadcast_mac[ETHER_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff}; 
	if(NULL == mac_addr)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);
		return 1;
	}

	if(0 == memcmp(broadcast_mac, mac_addr, ETHER_ADDR_LEN))
		return 1;

	if(tenda_arp_isnot_lanhost(*ip_addr))
		return 1;
	
	return 0;
}

/* 
 * Function: tenda_arp_hash
 * Description: mac hash solution of tenda_arp
 * Paramater: 
  	mac_addr: mac address of this node, formate --> char mac[ETHER_ADDR_LEN]
 * Return value:
 	index of det
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
static inline unsigned int tenda_arp_hash(const unsigned char *mac)
{    
	unsigned int hash_val = 0, i;
	
	for(i = 0; i < ETHER_ADDR_LEN; ++i)
		hash_val = 31 * hash_val + mac[i];
	
	return (hash_val % TENDA_ARP_SIZE(det));
}

/* 
 * Function: tenda_arp_update_node
 * Description: routine of updating the table of tenda_arp
 * Paramater: 
  	mac_addr: mac address of this node
  	ip_addr:ip address of this node
  	action:what to do with this node, add, delete or modify
  	expire_time:expire time of this node need adding or updating
 * Return value:
 	-1: error occurs
 	1: updated successfully
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
int tenda_arp_update_node(unsigned char *mac,in_addr_t ip,int action, u_long rmx_expire)
{
	unsigned int tenda_arp_index;
	
	if(ip == 0 || mac == NULL)
		return -1;

	if(tenda_arp_is_multicast(mac))
		return -1;

	tenda_arp_index = tenda_arp_hash(mac);

	switch(action)
	{
		case ADD_ITEM:
			return tenda_arp_add_node(&det[tenda_arp_index], mac, &ip, &rmx_expire);
			break ;
		case DELETE_ITEM:
			return tenda_arp_delete_node(&det[tenda_arp_index], &ip);
			break ;
		case MODIFY_ITEM:
			return tenda_arp_modify_node(det[tenda_arp_index], &ip);
			break ;
		default:
			break ;
	}

	return -1;
}

/* 
 * Function: tenda_arp_add_node
 * Description: add tenda_arp node to the tenda_arp table if this node isn't exist, or update it
 * Paramater: 
  	tenda_arp_list: list of tenda_arp table that this node will be added to or updated
  	mac_addr: point of mac address of this node
  	ip_addr:point of ip address of this node
  	expire_time: point of expire time of this node need adding or updating
 * Return value:
 	-1: error occurs
 	1: added or updated successfully
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
 /*lq在arp更新的时候更新家长控制和流控客户端*/
#ifdef __CONFIG_TC__
extern int update_stream_control(in_addr_t cur_ip,char* cur_mac);
extern void update_parent_ctl_ip(in_addr_t cur_ip, char* cur_mac);
#endif


static int tenda_arp_add_node(struct detec **tenda_arp_list, const unsigned char *mac_addr, const in_addr_t *ip_addr, const u_long *expire_time)
{
	time_t time_now;
	struct detec *new_arp_node;
	
	if(NULL == tenda_arp_list || NULL == ip_addr || NULL == expire_time)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);
		return -1;
	}

	/* init variables */
	time_now = time(0);
	new_arp_node = NULL;

	/* this is first node in this list */
	if(NULL == *tenda_arp_list)
	{
		/* this ip address should not be the one of router itself's */
		if(tenda_arp_check_validity(mac_addr,ip_addr))
		{
			return -1;
		}

		new_arp_node = tenda_arp_create_node(mac_addr, ip_addr, expire_time);

		if(NULL != new_arp_node)
		{
			printf("ADD: %s --- %s Success (new) !!!\n", tenda_arp_inet_ntoa(new_arp_node->ip), inet_mactoa(new_arp_node->mac));
		#ifdef __CONFIG_TC__
			update_stream_control(*ip_addr,mac_addr);
			update_parent_ctl_ip(*ip_addr,mac_addr);
		#endif
			/* chain the new node, in the way of prepending */
			new_arp_node->next = *tenda_arp_list;
			*tenda_arp_list = new_arp_node;

			return 1;
		}
	}
	else
	{
		/* walk this list and find out whether the node has existed aready */
		struct detec *tmp_node = *tenda_arp_list;
		while(tmp_node)
		{
			/* break if the node has existed aready */
			if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
			{
				break ;
			}

			tmp_node = tmp_node->next;
		}

		if(NULL == tmp_node)	/* this is a new node which is needed adding to the list */
		{
			/* this ip address should not be the one of router itself's */
			/* this ip address should not be the one from wan port */
			if(tenda_arp_check_validity(mac_addr,ip_addr))
			{
				return -1;
			}
			
			
			new_arp_node = tenda_arp_create_node(mac_addr, ip_addr, expire_time);

			if(NULL != new_arp_node)
			{
				printf("***** conflict occurs! *****\n");
				printf("ADD: %s --- %s Success (new) !!!\n", tenda_arp_inet_ntoa(new_arp_node->ip), inet_mactoa(new_arp_node->mac));
				/* chain the new node, in the way of prepending */
			#ifdef __CONFIG_TC__
				update_stream_control(*ip_addr,mac_addr);
				update_parent_ctl_ip(*ip_addr,mac_addr);
			#endif
				new_arp_node->next = *tenda_arp_list;
				*tenda_arp_list = new_arp_node;

				return 1;
			}
		}
		else	/* this node has existed aready */
		{
			/* update it's update_time if ip address is the same */
			if(tmp_node->ip == *ip_addr)
			{
				   tmp_node->update_time = time_now;
		                 /* 如果是无线设备，则检查是否在线，如果不在线，则从arp链表中移除 */
		              if(0 != tmp_node->flag && 0 == tenda_arp_is_wireless_client(tmp_node->mac))
		              {
		                 int index = tenda_arp_hash(tmp_node->mac);
		                 tenda_arp_delete_node(&det[index], &tmp_node->ip);
		              }else{
			      	    tmp_node->flag =tenda_arp_is_wireless_client(tmp_node->mac);
		              }
				return 1;
			}

			/* this ip address should not be the one of router itself's */
			/* this ip address should not be the one from wan port */
			if(tenda_arp_check_validity(mac_addr,ip_addr))
			{
				return -1;
			}
		
			/* update if it's ip has changed and this node is newer */
			if(*expire_time > tmp_node->rmx_expire)
			{
				printf("ip changed:%s --> ", tenda_arp_inet_ntoa(tmp_node->ip));
				printf("%s\n", tenda_arp_inet_ntoa(*ip_addr));
			#ifdef __CONFIG_TC__
				update_stream_control(*ip_addr,mac_addr);
				update_parent_ctl_ip(*ip_addr,mac_addr);
			#endif
				tmp_node->update_time = time_now;
				tmp_node->rmx_expire = *expire_time;
				tmp_node->ip = *ip_addr;
				tmp_node->flag =tenda_arp_is_wireless_client(tmp_node->mac);
				return 1;
			}
		}
	}

	return -1;
}

/* 
 * Function: tenda_arp_delete_node
 * Description: delete tenda_arp node from the tenda_arp table if this node is exist
 * Paramater: 
  	tenda_arp_list: list of tenda_arp table from which this node will be deleted
  	ip_addr:ip address of this node
 * Return value:
 	-1: error occurs
 	1: deleted successfully
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
static int tenda_arp_delete_node(struct detec **tenda_arp_list, const in_addr_t *ip_addr)
{
	if(NULL == tenda_arp_list || NULL == ip_addr)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);
		return -1;
	}

	/* check whether this list is NULL or not */
	if(NULL == *tenda_arp_list)
	{
		printf("fun=%s; line=%d; msg: error-->this node is not found!\n", __func__, __LINE__);
		return -1;
	}

	/* walk this list and find out whether the node is exist */
	struct detec *tmp_node = *tenda_arp_list;
	struct detec *previous_node = NULL;
	while(tmp_node)
	{
		/* delete it if the node has existed */
		if(tmp_node->ip == *ip_addr)
		{
			printf("REMOVE: %s --- %s remove to the expired list!\n", tenda_arp_inet_ntoa(tmp_node->ip), inet_mactoa(tmp_node->mac));

			/* remove this node from tenda_arp table */
			if(NULL == previous_node)
			{
				*tenda_arp_list = tmp_node->next;
			}
			else
			{
				previous_node->next = tmp_node->next;
			}

			/* move this node to the expired_list, in the way of prepending */
			cyg_mutex_lock(&expired_list_lock);
			tmp_node->next = expired_list;
			expired_list = tmp_node;
			cyg_mutex_unlock(&expired_list_lock);

			return 1;
		}

		previous_node = tmp_node;
		tmp_node = tmp_node->next;
	}

	printf("fun=%s; line=%d; msg: error-->this node is not found!\n", __func__, __LINE__);
	return -1;
}

/* 
 * Function: tenda_arp_modify_node
 * Description: modify auth of tenda_arp node of the tenda_arp table
 * Paramater: 
  	tenda_arp_list: list of tenda_arp table that this node will be added to or updated
  	ip_addr:ip address of this node
 * Return value:
 	-1: error occurs
 	1: modified successfully
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
static int tenda_arp_modify_node(struct detec *tenda_arp_list, const in_addr_t *ip_addr)
{
	if(NULL == ip_addr)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);
		return -1;
	}

	/* check whether this list is NULL or not */
	if(NULL == tenda_arp_list)
	{
		printf("fun=%s; line=%d; msg: error-->this node is not found!\n", __func__, __LINE__);
		return -1;
	}

	/* walk this list and find out whether the node if exist */
	while(tenda_arp_list)
	{
		/* modify if the node is exist */
		if(tenda_arp_list->ip == *ip_addr)
		{
			tenda_arp_list->auth = 1;
			return 1;
		}

		tenda_arp_list = tenda_arp_list->next;
	}

	printf("fun=%s; line=%d; msg: error-->this node is not found!\n", __func__, __LINE__);
	return -1;
}

/* 
 * Function: tenda_arp_create_node
 * Description: add tenda_arp node to the tenda_arp table if this node isn't exist, or update it
 * Paramater: 
  	mac_addr: point of mac address of this node
  	ip_addr: point of ip address of this node
  	expire_time: point of expire time of this node need adding or updating
 * Return value:
 	NULL: error occurs
 	!NULL: point of new node created successfully
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
static struct detec *tenda_arp_create_node(const unsigned char *mac_addr, const in_addr_t *ip_addr, const u_long *expire_time)
{
	struct detec *tmp_node = NULL;
	struct detec *previous_node = NULL;
	time_t time_now = time(0);

	/* walk the expored_list to find out whether this new node has ever existed */
	cyg_mutex_lock(&expired_list_lock);
	tmp_node = expired_list;
	while(tmp_node)
	{
		if(0 == memcmp(tmp_node->mac, mac_addr, ETHER_ADDR_LEN))
		{
	            if(0 != tmp_node->flag && 0 == tenda_arp_is_wireless_client(tmp_node->mac))
	            {
	                cyg_mutex_unlock(&expired_list_lock);
	                return NULL;
	            }
			if(NULL == previous_node)
			{
				expired_list = tmp_node->next;
			}
			else
			{
				previous_node->next = tmp_node->next;
			}
			break ;
		}

		previous_node = tmp_node;
		tmp_node = tmp_node->next;
	}
	cyg_mutex_unlock(&expired_list_lock);

	if(NULL != tmp_node)
	{
		tmp_node->ip = *ip_addr;
		tmp_node->time = time_now;
		tmp_node->update_time = time_now;
		tmp_node->rmx_expire = *expire_time;
	}
	else
	{
		/* get a room to hold the now node */
		tmp_node = (struct detec *)malloc(sizeof(struct detec));
		if(NULL == tmp_node)
		{
			printf("fun=%s; line=%d; msg: no buffer!\n", __func__, __LINE__);
			return NULL;
		}

		/* get value */
		memset((char *)tmp_node, 0x0, sizeof(struct detec));
		memcpy(tmp_node->mac, mac_addr, ETHER_ADDR_LEN);
		tmp_node->ip = *ip_addr;
		tmp_node->flag = tenda_arp_is_wireless_client(mac_addr);
		tmp_node->time = time_now;
		tmp_node->update_time = time_now;
		tmp_node->interval_time = 0;
		tmp_node->rmx_expire = *expire_time;
		
	}

	return tmp_node;
}

/* 
 * Function: tenda_arp_delete_expired_list
 * Description: delete those nodes, which are in the expired_list
 * Paramater: 
  	NULL
 * Return value:
 	NULL
 * Author: zhuhuan
 * create time: 2016.03.19
 * Version: v1.0
 */
void tenda_arp_delete_expired_list(void)
{
	struct detec *tmp_node = NULL;

	cyg_mutex_lock(&expired_list_lock);
	while(tmp_node = expired_list)
	{
		printf("DELETE: %s --- %s connected time:%u secs\n", tenda_arp_inet_ntoa(tmp_node->ip), inet_mactoa(tmp_node->mac), tmp_node->update_time- tmp_node->time);
		expired_list = expired_list->next;
		free(tmp_node);
	}
	cyg_mutex_unlock(&expired_list_lock);
}

static char *inet_ntoa_r_tenda(in_addr_t ina, char *buf)
{
    unsigned char *ucp = (unsigned char *)&ina;
    sprintf(buf, "%d.%d.%d.%d",
                 ucp[0] & 0xff,
                 ucp[1] & 0xff,
                 ucp[2] & 0xff,
                 ucp[3] & 0xff);
    return buf;
}

//char *tenda_arp_inet_ntoa(in_addr_t ina)
char *tenda_arp_inet_ntoa(in_addr_t ina)
{
    static char buf[sizeof "123.456.789.012"];
    return inet_ntoa_r_tenda(ina, buf);
}

void tenda_arp_get_all(struct detec *buf)
{

	int i, j;
	struct detec *tmp_node = NULL;
	                                                                                                                                                                                                                               
	for(j = 0, i = 0; i < TENDA_ARP_SIZE(det); i++)
	{
		tmp_node = det[i];

		/* walk this list and find out whether the node is exist */
		while(tmp_node)
		{
       		buf[j].auth = tmp_node->auth;
			buf[j].flag = tmp_node->flag;
			buf[j].ip = tmp_node->ip;
			memcpy(buf[j].mac, tmp_node->mac, ETHER_ADDR_LEN);
			strlcpy(buf[j++].hostname, tmp_node->hostname, PI_BUFLEN_64);

			tmp_node = tmp_node->next;
		}
	}
	
	return;
}

struct machost_t{
		in_addr_t ip;
        unsigned char mac[ETHER_ADDR_LEN];
		char hostname[PI_BUFLEN_64];
		struct machost_t *next
};

void tenda_arp_set_hostname(struct machost_t *machost)
{

	int tenda_arp_mac_index;
	struct detec *tmp_node = NULL;

	for( ; machost; machost = machost->next)
	{
		tenda_arp_mac_index = tenda_arp_hash(machost->mac);
		tmp_node = det[tenda_arp_mac_index];

		/* walk this list and find out whether the node is exist */
		while(tmp_node)
		{
			/* return if the node is found successfully */
			if(tmp_node->ip == machost->ip && 0 != strcmp("", machost->hostname))
			{
				if(0 != strcmp(tmp_node->hostname, machost->hostname))
				{
					printf("%s: mac=%s, ip=%s, host name=%s\n", __func__, inet_mactoa(tmp_node->mac), tenda_arp_inet_ntoa(tmp_node->ip), machost->hostname);
					strlcpy(tmp_node->hostname, machost->hostname, PI_BUFLEN_64);
				}
			}

			tmp_node = tmp_node->next;
		}
	}
	
	return ;
}

int tenda_arp_update_all()
{	
	show_network_tables2(tenda_arp_dump,NULL);
	return 0;
}

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
#define	rt_expire rt_rmx.rmx_expire
#endif

static int tenda_arp_dump(struct radix_node *rn, void *req)
{
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst;
    struct sockaddr_dl *sdl;
	char ip[20] = {0};
	char mac[20] = {0};
    dst = rt_key(rt);
	char *lan_ifname;
	
    sdl = (struct sockaddr_dl *)rt->rt_gateway;
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == (RTF_UP | RTF_LLINFO) && sdl->sdl_alen == ETHER_ADDR_LEN) 
    {	
    	
			strcpy(ip,inet_ntoa(((struct sockaddr_in *)dst)->sin_addr));
			strcpy(mac,ether_ntoa((struct ether_addr *)LLADDR(sdl)));
			if(strcmp(mac, "00:00:00:00:00:00") == 0)
			{
				return 0;
			}
	
			tenda_arp_update_node(LLADDR(sdl), ((struct sockaddr_in *)dst)->sin_addr.s_addr, 1, rt->rt_expire);
update:
			lan_ifname = nvram_safe_get("lan_ifname");
			if (strcmp(lan_ifname, "") != 0) 
				copyHostname2TendaArp(lan_ifname);
#ifdef __CONFIG_GUEST__
			lan_ifname = nvram_safe_get("lan1_ifname");
			if (strcmp(lan_ifname, "") != 0) 
				copyHostname2TendaArp(lan_ifname);
#endif
				
    }
    return 0;
}

static int tenda_arp_show_table(void)
{
	int i, j;
	struct detec *tmp_node = NULL;
	char *ifname[] ={
					"wired",
					TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
#ifdef __CONFIG_GUEST__
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
#endif
					""};	
	printf("*********************** tenda arp table ***********************\n");
	                                                                                                                                                                                                                               
	for(i=0, j = 0; i<TENDA_ARP_SIZE(det); i++)
	{
		tmp_node = det[i];
		
		/* walk this list*/
		while(tmp_node)
		{
			printf("NO.%d,%d,%02X:%02X:%02X:%02X:%02X:%02X,%s,%s,%s\n",
					++j,
                    tmp_node->auth,
                    tmp_node->mac[0]&0XFF,
                    tmp_node->mac[1]&0XFF,
                    tmp_node->mac[2]&0XFF,
                    tmp_node->mac[3]&0XFF,
                    tmp_node->mac[4]&0XFF,
                    tmp_node->mac[5]&0XFF,
                    tenda_arp_inet_ntoa(tmp_node->ip),
                    ifname[tmp_node->flag],
                    tmp_node->hostname
                    );

			tmp_node = tmp_node->next;
		}
	}
	
	return 0;
}
/*重新写回arp*/
extern int rtl865x_addArp(unsigned long ip, unsigned char * mac);
int tenda_arp_update_hw_table(void)
{
	int i = 0;
	int j = 0;
	struct detec *tmp_node = NULL;
	                                                                                                                                                                                                                               
	for(i=0, j = 0; i<TENDA_ARP_SIZE(det); i++)
	{
		tmp_node = det[i];
		
		/* walk this list*/
		while(tmp_node)
		{
		      rtl865x_addArp(tmp_node->ip, tmp_node->mac);
			tmp_node = tmp_node->next;
		}
	}
	
	return 0;
}

static void tenda_arp_show_expired_list(void)
{
	int j = 0;
	struct detec *tmp_node = NULL;
	char *ifname[] ={
					"wired",
					TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
#ifdef __CONFIG_GUEST__
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
#endif
					""};	
	printf("*********************** expired list ***********************\n");

    cyg_mutex_lock(&expired_list_lock);
	tmp_node = expired_list;
	while(tmp_node)
	{
		printf("NO.%d,%d,%02X:%02X:%02X:%02X:%02X:%02X,%s,%s,%s\n",
				++j,
                tmp_node->auth,
                tmp_node->mac[0]&0XFF,
                tmp_node->mac[1]&0XFF,
                tmp_node->mac[2]&0XFF,
                tmp_node->mac[3]&0XFF,
                tmp_node->mac[4]&0XFF,
                tmp_node->mac[5]&0XFF,
                tenda_arp_inet_ntoa(tmp_node->ip),
                 ifname[tmp_node->flag],
                tmp_node->hostname
                );

		tmp_node = tmp_node->next;
	}
    cyg_mutex_unlock(&expired_list_lock);

	return ;
}

void tenda_arp_show(int argc, char* argv[])
{
	if(1 == argc)
		tenda_arp_show_table();
	else if(2 == argc && 0 == strcmp("expired_list", argv[1]))
		tenda_arp_show_expired_list();
}

/*
sntp同步后保存时间更新导致的误差,重新计算sta接入时间
*/
void tenda_arp_update_time()
{
	int i = 0;
	time_t now_time = time(0);
	struct detec *tmp_node = NULL;
	
	for(i=0; i<TENDA_ARP_SIZE(det); i++)                                                                                                                                                   
    {
    	tmp_node = det[i];

		/* walk this list*/
		while(tmp_node)
		{
			tmp_node->time = now_time - tmp_node->interval_time;
			 tmp_node->update_time =now_time - tmp_node->interval_time;
			tmp_node = tmp_node->next;
		}
    }

	/* update expired list */
	cyg_mutex_lock(&expired_list_lock);
	tmp_node = expired_list;
	while(tmp_node)
	{
		tmp_node->time += tmp_node->interval_time;
        tmp_node->update_time += tmp_node->interval_time;
        
		
		tmp_node = tmp_node->next;
	}
	cyg_mutex_unlock(&expired_list_lock);
	
	printf("fun=%s; line=%d; msg: update arp client time successfully!\n",__func__, __LINE__);
	
	return ;
}

/*
sntp同步后时间有变化，保存ntp更新前的在线时间
*/
void tenda_arp_set_interval_time()
{
	int i = 0;
	time_t now_time = time(0);
    
	struct detec *tmp_node = NULL;

	for(i=0; i<TENDA_ARP_SIZE(det); i++)                                                                                                                                                   
    {
    	tmp_node = det[i];

		/* walk this list*/
		while(tmp_node)
		{
			tmp_node->interval_time = now_time - tmp_node->time;
			
			tmp_node = tmp_node->next;
		}
    }

	/* set expired list */
	cyg_mutex_lock(&expired_list_lock);
	tmp_node = expired_list;
	while(tmp_node)
	{
		tmp_node->interval_time = now_time - tmp_node->time;
		
		tmp_node = tmp_node->next;
	}
	cyg_mutex_unlock(&expired_list_lock);

	printf("fun=%s; line=%d; msg: set arp client interval_time successfully!\n",__func__, __LINE__);
	
	return ;
}
