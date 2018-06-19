/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tenda_getclients.c
	Description: show all clients like DHCP clients, wl clients and static access clients.
	Author: Lvliang;
	Version : 1.0
	Date: 2015.4.28
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang        2015.4.28   1.0             learn from xuwu(ecos1.0)
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <typedefs.h>
#if 1
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <net/ethernet.h>
#include <wlioctl.h>
#endif
#include <router_net.h>
#include "tenda_client.h"


#ifdef TPI_ALL_CLIENT_DEBUG_ON
#define DB_ERROR(format, args...)	printf( "Error:[%s](%d)->" format, __FUNCTION__, __LINE__, ##args)

/* if you want to debug ,open the note below */
#define DB_PRINT(format, args...)	printf( "Debug:[%s](%d)->" format, __FUNCTION__, __LINE__, ##args)
#else
#define DB_PRINT(format, args...)
#define DB_ERROR(format, args...)	
#endif



#if 1

typedef struct mac_ip_tbl{
	struct ether_addr hw_addr;
	struct	in_addr ip_addr;
}arp_tbl_enty;
#if 0
#define MAX_OUTLINE_CLIENT_NUM		20

extern int get_wifi_outline_clients(struct maclist * maclist, int max_size);

extern int ether_atoe(const char *a, unsigned char *e) ;

/* 1: mathed, otherwise not match */
static int wl_outline_matched(PTPI_ALL_CLIENTS all_clt, struct maclist *mac_list, int list_num)
{
	int i = 0;
	struct ether_addr ea;
	for (i = 0; i < mac_list->count; i++)
	{
		ether_atoe(all_clt->mac, &ea);
		if (!memcmp((void *)&ea, (void *)&mac_list->ea[i], ETHER_ADDR_LEN))
		{
			return 1;
		}
	}
	return 0;
}

/* 获取离线或在线客户端列表 */
int get_wl_outline_clients(OUT PTPI_ALL_CLIENTS clients_list, IN int max_client_num)
{
	int i;
	struct maclist *maclist = NULL;
	PTPI_ALL_CLIENTS tmp_clt = NULL;
	int rcnt_list_num = 0;

	int size = sizeof(maclist->count) + sizeof(struct ether_addr) * MAX_OUTLINE_CLIENT_NUM;
	maclist = (struct ether_addr *)malloc(size); 
	
	if (NULL == maclist)
		return 0;

	memset(maclist, 0, size);

	if ((rcnt_list_num = (get_wifi_outline_clients(maclist, size)))  < 0)
	{
		free(maclist);
		return 0;
	}

	
	if (rcnt_list_num > MAX_OUTLINE_CLIENT_NUM)
	{
		printf("Unsupported list num, error!\n");
	}

	tmp_clt = clients_list;
	for (i = 0; i < max_client_num; i++)
	{
		if (IN_USE == tmp_clt->in_use)
		{
			if (wl_outline_matched(tmp_clt, maclist, rcnt_list_num))
			{
				//printf("set %s(%s) to wireless\n", tmp_clt->ip, tmp_clt->mac);
				tmp_clt->l2type = L2_TYPE_WIRELESS;
			}
		}
		else 
		{
			break;
		}
		tmp_clt++;
	}

	free(maclist);	
}
#endif

#if 1
extern struct lease_t *dhcpd_lease_dump(char *ifname);
extern char *ether_etoa(const unsigned char *e, char *a);
TPI_RET tpi_dhcps_get_clients(PTPI_DHCPS_CLIENTS plist)
{
	struct lease_t * pclients,*client;
	int count = 0/*, i*/;
	
	if(!plist){
		return TPI_RET_NULL_POINTER;
	}

	pclients = dhcpd_lease_dump(SYS_lan_if);

	if(!pclients){
		return TPI_RET_ERROR;
	}

	client = pclients;
	
	do{
	
		plist->clients[count].bound = client->flag? STATIC_BOUND: AUTO_ALLOCATION;
		plist->clients[count].l2type = WIRED;	/* wireless clients need develop */
		plist->clients[count].expiry = client->expiry - time(0);
		
		ether_etoa(client->mac, plist->clients[count].mac);

		strncpy(plist->clients[count].ipaddr, inet_ntoa(client->ipaddr), sizeof(plist->clients[count].ipaddr));

		strncpy(plist->clients[count].hostname, client->hostname, sizeof(plist->clients[count].hostname));

		count++;
		
		/* Is it the last one? */
		if(client->last)
			break;
		
		client++;
	}while(1);

	plist->total = count;
	free(pclients);	

	return TPI_RET_OK;
	
}



static int get_dhcp_clients(OUT PTPI_ALL_CLIENTS clients_list, IN int max_client_num)
{
		PTPI_DHCPS_CLIENTS plist = NULL;
		PTPI_ALL_CLIENTS all_tmp;
		int /*i,*/client_count,plist_count;

		/* temp buf to get dhcp lists */
		plist = (PTPI_DHCPS_CLIENTS)malloc(sizeof(TPI_DHCPS_CLIENTS));
	
		if (NULL == plist)
		{
			DB_ERROR("malloc plist failed!\n");
			return -1;
		}
		
		memset(plist, 0x0, sizeof(TPI_DHCPS_CLIENTS));

		if (TPI_RET_OK != tpi_dhcps_get_clients( plist))
		{
			DB_ERROR("get dhcps clients failed!\n");
			free(plist);
			return -1;
		}
		
		all_tmp = clients_list;
		client_count = 0;
		plist_count = 0;
		
		while(client_count< max_client_num && plist_count < plist->total)
		{
			if(IN_USE == all_tmp->in_use)	
			{
				//test dhcp_hostname --add by ll
				while(plist_count < plist->total)
				{
					if(strcmp(all_tmp->mac,plist->clients[plist_count].mac) == 0 &&
						strcmp(all_tmp->ip,plist->clients[plist_count].ipaddr) == 0)
					{
						strcpy(all_tmp->hostname,plist->clients[plist_count].hostname);						
						break;
					}
					plist_count++ ;
				}
				plist_count = 0 ;
			}
			else
			{
					break;
			}

			client_count++;
			all_tmp++;
		}
		
		free(plist);
		
		return plist_count;
}

#endif

TPI_RET tpi_lan_get_status(PTPI_LAN_STATUS status)
{
	if(!status){
		return TPI_RET_ERROR;
	}

	strcpy(status->ifname, SYS_lan_if);
	strcpy(status->ip, inet_ntoa(SYS_lan_ip));
	strcpy(status->netmask, inet_ntoa(SYS_lan_mask));

	return TPI_RET_OK;	
}

TPI_RET tpi_same_net(unsigned int one_ip,unsigned int other_ip,unsigned int net_mask)
{
	if((one_ip&net_mask) == (other_ip&net_mask))
		return 1;
	else
		return 0;
}


static int tenda_dump_arp(struct radix_node *rn, void *req)
{
	struct rtentry *rt = (struct rtentry *)rn;
  	struct sockaddr *dst;
  	struct sockaddr_dl *sdl;

	arp_tbl_enty new_arp_item;
	
	int i;
	

    dst = rt_key(rt);
    sdl = (struct sockaddr_dl *)rt->rt_gateway;
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == (RTF_UP | RTF_LLINFO) && sdl->sdl_alen != 0) 
    {
    	
			memset(&new_arp_item, 0, sizeof(arp_tbl_enty));
			
			/* hw addr */
			memcpy(&(new_arp_item.hw_addr), (struct ether_addr *)LLADDR(sdl), ETHER_ADDR_LEN);
			
			//mac is zero?
			if((new_arp_item.hw_addr.octet[0] | new_arp_item.hw_addr.octet[1] |
				new_arp_item.hw_addr.octet[2] | new_arp_item.hw_addr.octet[3] |
				new_arp_item.hw_addr.octet[4] | new_arp_item.hw_addr.octet[5]) == 0)
				return 0;
			
			/* ip addr */
			memcpy(&(new_arp_item.ip_addr), (struct in_addr *)&(((struct sockaddr_in *)dst)->sin_addr), 
				sizeof(struct in_addr));
				
			//ip is same net with lan
			TPI_LAN_STATUS lan_cfg;
	
			memset(&lan_cfg, 0x0, sizeof(TPI_LAN_STATUS));
#if 1		
			/* get lan_ip and lan_mask through tpi inferface */
			if (TPI_RET_OK == tpi_lan_get_status(&lan_cfg))
			{
				/* judge as the same network segment */
				DB_PRINT("arp ip:%x, lan ip:%s, netmask:%s.\n", new_arp_item.ip_addr.s_addr, lan_cfg.ip, lan_cfg.netmask);
				
				if (1 != tpi_same_net(new_arp_item.ip_addr.s_addr, inet_addr(lan_cfg.ip), inet_addr(lan_cfg.netmask)))
				{
					return 0;
				}
				
				/* the ipaddr is not allowed as the same as GW addr */
				if (new_arp_item.ip_addr.s_addr == inet_addr(lan_cfg.ip))
				{
					printf("Found gw ip!\n");
					return 0;
				}
				/* end */	
			}else{
				return 0;	
			}	
#endif				
			DB_PRINT("begin to update arp to clients list\n");
			
			arp_tbl_enty *arp_tmp = (arp_tbl_enty *) req;
		
			for(i = 0; i< MAX_CLIENT_LIST_NUM; i++)
			{
					if(arp_tmp[i].ip_addr.s_addr == 0)
					{
						memcpy((void *)&arp_tmp[i], (void *)&new_arp_item, sizeof(arp_tbl_enty));	
						break;
					}	
			}
			
			if(i >= MAX_CLIENT_LIST_NUM)
			{
					DB_ERROR("tenda_dump_arp: arp_count reach max!!!\n");
			}	

    }

    return 0;
}

extern void show_network_tables2(int (*mtenda_routing_entry)(struct radix_node *, void *),void *pr);

static int get_arp_clients(OUT PTPI_ALL_CLIENTS clients_list, IN int max_client_num)
{
	int i,client_count,arp_idx,arp_num;
	
	PTPI_ALL_CLIENTS all_tmp;
	
	int list_size = (MAX_CLIENT_LIST_NUM) * sizeof(arp_tbl_enty);
	arp_tbl_enty *arp_tmp_buf = (arp_tbl_enty *)malloc(list_size);
	
	if (NULL == arp_tmp_buf)
	{
		//DB_ERROR("malloc arp_tmp_buf failed!\n");
		return -1;
	}
	
	memset(arp_tmp_buf, 0x0, list_size);
	
	show_network_tables2(tenda_dump_arp, (void *)arp_tmp_buf);
	
	for(i = 0; i< MAX_CLIENT_LIST_NUM; i++)
	{
			if(arp_tmp_buf[i].ip_addr.s_addr == 0)
			{
				break;
			}	
	}
	
	arp_num = (i >= MAX_CLIENT_LIST_NUM)? (i-1):i;
	arp_idx = 0;
	client_count = 0;
	all_tmp = clients_list;
	
	while(client_count < max_client_num && arp_idx < arp_num)
	{
			
			if(NOT_IN_USE == all_tmp->in_use){
				//no in dhcp client list, may be static ip
					all_tmp->in_use = IN_USE;
					strcpy(all_tmp->mac,ether_ntoa((struct ether_addr *)&arp_tmp_buf[arp_idx].hw_addr));
					inet_ntoa_r(arp_tmp_buf[arp_idx].ip_addr,all_tmp->ip);
					strcpy(all_tmp->hostname,"");//no hostname
					all_tmp->l2type = L2_TYPE_WIRED;//defautl
					arp_idx++;
		
			}
			all_tmp++;
			client_count++;					
	}
	
	if(client_count >= max_client_num)
	{
			DB_ERROR("get_arp_clients: client_count reach max!!!\n");
	}
	
	free(arp_tmp_buf);

	return arp_idx;
}

static int dump_all_client(PTPI_ALL_CLIENTS clients_list,int max_client_num)
{
	int i = 0;
	
	DB_PRINT("no.\t\tip\t\tmac\t\t\ttype\t\t\thostname\n");

	for (i = 0; i < max_client_num; i++)
	{
		if(IN_USE == clients_list[i].in_use)
		{
			DB_PRINT("%d\t\t%s\t\t%s\t\t\t%d\t\t\t%s\n",
					i+1, clients_list[i].ip, clients_list[i].mac, clients_list[i].l2type, clients_list[i].hostname);
		}else{
			break;	
		}
	}
	
	return i;

}

#endif
/*
	TPI Interface to get all client lists	
args:
	clients_list         the PTPI_ALL_CLIENTS  pointer
	max_len              max list length
	
return:
	OK :  return 0
	Failed: return -1
*/
int tpi_get_all_clients(OUT PTPI_ALL_CLIENTS clients_list, IN int max_client_num)
{
	int i;
	PTPI_ALL_CLIENTS tmp;

	//init
	tmp = clients_list;
	for(i = 0; i< max_client_num; i++)
	{
		tmp->in_use = NOT_IN_USE;
		tmp++;
	}

	//dont change the next order:
	get_arp_clients(clients_list,max_client_num);
	
	get_dhcp_clients(clients_list,max_client_num);
	
	//get_wl_outline_clients(clients_list, max_client_num);
	
	return dump_all_client(clients_list,max_client_num);

}

