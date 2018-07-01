/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : route table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_route.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/

#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include "../rtl_types.h"
#include "../rtl865xC_tblAsicDrv.h"
#include <switch/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"
#include "common/rtl865x_eventMgr.h"
#include <switch/rtl_glue.h>
#include <rtl/rtl865x_eventMgr.h>
#include "rtl865x_ppp_local.h"
#include "rtl865x_route.h"
#include "rtl865x_ip.h"
#include "rtl865x_nexthop.h"
#include "rtl865x_arp.h"
#include "l2Driver/rtl865x_fdb.h"
#include <switch/rtl865x_fdb_api.h>

#define LOOPBACK(x)	(((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))
#define BADCLASS(x)	(((x) & htonl(0xf0000000)) == htonl(0xf0000000))

#if defined(CONFIG_RTL_HARDWARE_NAT)
extern int gHwNatEnabled;
#endif
static rtl865x_route_t *rtl865x_route_freeHead;
static rtl865x_route_t *rtl865x_route_inusedHead;
extern unsigned long rtl_getWanIfpIp(char *name);

static rtl865x_route_t* _rtl865x_getDefaultRoute(void)
{
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1)&&(rt->asicIdx == RT_ASIC_ENTRY_NUM -1))
			return rt;
		rt = rt->next;
	}

	return NULL;
}

static rtl865x_route_t* _rtl865x_getRouteEntry(ipaddr_t ipAddr, ipaddr_t ipMask)
{
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1)&&((rt->ipAddr&rt->ipMask)==(ipAddr&ipMask))&&(rt->ipMask==ipMask))
			return rt;
		rt = rt->next;
	}	
	return NULL;
}


static int32 _rtl865x_synRouteToAsic(rtl865x_route_t *rt_t)
{
	int32 ret = FAILED;
	rtl865x_tblAsicDrv_routingParam_t asic_t;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_tblAsicDrv_vlanParam_t vlan;
	rtl865x_netif_local_t *dstNetif = NULL;
	uint32 columIdx,fid;
	int32 pppIdx = 0;
    #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
    uint16 vid = 0;
    #endif
	bzero(&asic_t, sizeof(rtl865x_tblAsicDrv_routingParam_t));

	if(rt_t == NULL)
	{
		diag_printf("%s(%d):NULL!!!!!!!!!!!!!!!!\n",__FUNCTION__,__LINE__);		
		return FAILED;
	}
	/*common information*/
	asic_t.ipAddr	= rt_t->ipAddr;
	asic_t.ipMask	= rt_t->ipMask;
	asic_t.ipAddr = rt_t->ipAddr;
	asic_t.ipMask = rt_t->ipMask;
	/*if the dstNetif is attach on another interface, the netifIdx should the master interface's index*/
	if(rt_t->dstNetif->is_slave == 1)
	{
		//diag_printf("========%s(%d), ip(0x%x),mask(0x%x),netif(%s)\n",__FUNCTION__,__LINE__,rt_t->ipAddr,rt_t->ipMask,rt_t->dstNetif->name);
		dstNetif = _rtl865x_getNetifByName(rt_t->dstNetif->name);
		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	}
	else
		dstNetif = rt_t->dstNetif;

	if(dstNetif == NULL){
			diag_printf("%s(%d) BUG!!!!!!\n",__FUNCTION__,__LINE__);			
			return FAILED;
	}
	asic_t.vidx = dstNetif->asicIdx;
	asic_t.internal = rt_t->dstNetif->is_wan? 0 : 1;
	asic_t.DMZFlag = rt_t->dstNetif->dmz? 1 : 0;
	asic_t.process = rt_t->process;
	
	switch (rt_t->process)
	{
	case RT_PPPOE:
		ret = rtl865x_getPppIdx(rt_t->un.pppoe.pppInfo, &pppIdx);
		
		asic_t.pppoeIdx = pppIdx;
		/*
		*if process==RT_PPPOE, the mac address of pppoe server is add in pppoe module,
		*so, we read the FDB information directly....
		*/
		memset(&vlan, 0, sizeof(rtl865x_tblAsicDrv_vlanParam_t));
        #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
        vid = rt_t->dstNetif->vid;
        rtl8651_findAsicVlanIndexByVid(&vid);
        rtl8651_getAsicVlan(vid, &vlan);
        #else
		rtl8651_getAsicVlan(rt_t->dstNetif->vid, &vlan);
        #endif
		ret = rtl865x_Lookup_fdb_entry(vlan.fid, (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_DYNAMIC, &columIdx,&asic_l2);

		if(ret != SUCCESS)
			diag_printf("can't get l2 entry by mac.....\n");


		/*FIXME_hyking: update mac/fdb table reference count*/
		asic_t.nextHopRow = rtl8651_filterDbIndex( rt_t->un.pppoe.macInfo, vlan.fid);
		asic_t.nextHopColumn = columIdx;
		//diag_printf("----%s[%d], Row is %d, Column is %d, mac is 0x%2x%2x%2x%2x%2x%2x----\n", __FUNCTION__, __LINE__, asic_t.nextHopRow, asic_t.nextHopColumn,
			//((ether_addr_t *)(rt_t->un.pppoe.macInfo))->octet[0], ((ether_addr_t *)(rt_t->un.pppoe.macInfo))->octet[1], 
			//((ether_addr_t *)(rt_t->un.pppoe.macInfo))->octet[2], ((ether_addr_t *)(rt_t->un.pppoe.macInfo))->octet[3],
			//((ether_addr_t *)(rt_t->un.pppoe.macInfo))->octet[4], ((ether_addr_t *)(rt_t->un.pppoe.macInfo))->octet[5]);
		break;

	case RT_L2:
		/*
		* NOTE:this type not used now...
		* if we want to use it, please add FDB entry to sure this L2 entry in both software FDB table and Asic L2 table.
		*/
		memset(&vlan, 0, sizeof(rtl865x_tblAsicDrv_vlanParam_t));
        #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
        vid = rt_t->dstNetif->vid;
        rtl8651_findAsicVlanIndexByVid(&vid);
        rtl8651_getAsicVlan(vid, &vlan);
        #else
		rtl8651_getAsicVlan(rt_t->dstNetif->vid, &vlan);
        #endif
		ret = rtl865x_Lookup_fdb_entry(vlan.fid, (ether_addr_t *)rt_t->un.direct.macInfo, FDB_STATIC, &columIdx,&asic_l2);
		if(ret != SUCCESS)
			diag_printf("can't get l2 entry by mac.....\n");

		/*FIXME_hyking: update mac/fdb table reference count*/
		asic_t.nextHopRow = rtl8651_filterDbIndex(rt_t->un.direct.macInfo, vlan.fid);
		asic_t.nextHopColumn = columIdx;
		break;

	case RT_ARP:
		/*FIXME_hyking: update arp table reference count??*/
		asic_t.arpStart	= rt_t->un.arp.arpsta;
		asic_t.arpEnd	= rt_t->un.arp.arpend;	
		asic_t.arpIpIdx	= rt_t->un.arp.arpIpIdx;		
		break;
		
	case RT_CPU:
	case RT_DROP:
		/*do nothing*/
		break;
		
	case RT_NEXTHOP:
 		asic_t.nhStart		 = rt_t->un.nxthop.nxtHopSta;
		asic_t.nhNum	 	 = rt_t->un.nxthop.nxtHopEnd - rt_t->un.nxthop.nxtHopSta + 1;
		asic_t.nhNxt	 	 = asic_t.nhStart;
		asic_t.ipDomain      = rt_t->un.nxthop.ipDomain;
		asic_t.nhAlgo	 	 = rt_t->un.nxthop.nhalog;	

		break;
		
	default:
		diag_printf("Process_Type(%d) is not support!\n", rt_t->process);
	}

	if(rt_t->asicIdx > RT_ASIC_ENTRY_NUM-1)
	{
		diag_printf("BUG!! %s(%d)....", __FUNCTION__,__LINE__);
		return FAILED;
	}
	
	ret = rtl8651_setAsicRouting(rt_t->asicIdx, &asic_t);

	return ret;
	
}

static int32 _rtl865x_updateDefaultRoute(rtl865x_route_t *rt, int32 action)
{
	int32 i;
	rtl865x_route_t *entry;
	int32 retval = FAILED;

	entry = rt;
	if(entry == NULL)
		return TBLDRV_EINVALIDINPUT;
	
	/*delete nexthop which is add by default route*/
	if(rt->process == RT_NEXTHOP)
		for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd && entry->un.nxthop.nxtHopEnd != 0; i++)
		{
			retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
		}

	entry->un.nxthop.nxtHopSta = 0;
	entry->un.nxthop.nxtHopEnd = 0;
	switch(action)
	{
		case RT_DEFAULT_RT_NEXTHOP_CPU:
			retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)entry, entry->dstNetif, 0,entry->srcIp);
			break;

		case RT_DEFAULT_RT_NEXTHOP_NORMAL:
			{
				rt->process = RT_NEXTHOP;	
				switch(rt->dstNetif->if_type)
				{
					case IF_ETHER:
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, rt->nextHop,entry->srcIp);
						break;
					case IF_PPPOE:
						{
							rtl865x_ppp_t *pppoe;
							
							pppoe = rtl865x_getPppByNetifName(rt->dstNetif->name);

							if(pppoe != NULL)
							{
								/*got pppoe session*/
								retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, pppoe->sessionId,entry->srcIp);
							}
							else
								/*nexthop's action is to CPU*/
								retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, 0,entry->srcIp);
						}
						break;
				}
			}
			break;
	}
	retval = _rtl865x_synRouteToAsic(entry);

	return retval;
}

static int32 _rtl865x_arrangeRoute(rtl865x_route_t *start_rt, int32 start_idx)
{
	int32 count;
	rtl865x_route_t *rt = NULL;

	rt = start_rt;
	count = 0;
	while(rt)
	{		
		if(rt->valid)
		{
			/*if the rule is default route...*/
			if(rt->ipMask == 0)
				rt->asicIdx = RT_ASIC_ENTRY_NUM-1;
			else
			{
				/* entry number more than asic table's capacity*/
				/* entry index=RT_ASIC_ENTRY_NUM-1 is reserved for default route*/
				if((start_idx + count > RT_ASIC_ENTRY_NUM-2))
					break;
				
				/*delete old asic entry firstly...*/
				if(start_idx+count < rt->asicIdx && rt->asicIdx < RT_ASIC_ENTRY_NUM-1)
					rtl8651_delAsicRouting(rt->asicIdx);
				
				rt->asicIdx = start_idx+count;
				_rtl865x_synRouteToAsic(rt);
			}			
		}

		/*next entry*/
		rt= rt->next;
		count++;
	}

	
	/*more route entry need to add?*/
	if(rt)
	{
		/*not enough asic table entry! have to update default route's action TOCPU*/
		rt = _rtl865x_getDefaultRoute();
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_CPU);		
	}
	else
	{
		rt = _rtl865x_getDefaultRoute();
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_NORMAL);
	}
	
	return SUCCESS;
}

static int32 _rtl865x_addRouteToInusedList(rtl865x_route_t *rt)
{
	int32 retval = FAILED;
	int32 start_idx = 0;
	rtl865x_route_t *entry,*fore_rt,*start_rt;

	fore_rt = NULL;
	entry = rtl865x_route_inusedHead;

	/*always set 0x0f when init..., this value would be reset in arrange route*/
	rt->asicIdx = 0x0f;
	rt->next = NULL;
		
	/*find the right position...*/
	while(entry)
	{
		if(entry->valid == 1)
		{
			if(entry->ipMask < rt->ipMask)
			{
				break;
			}
		}
		fore_rt = entry;
		entry = entry->next;
	}

	/*insert this rule after insert_entry*/
	if(fore_rt)
	{
		rt->next = fore_rt->next;
		fore_rt->next = rt;
		start_idx = fore_rt->asicIdx+1;
		start_rt = rt;
	}
	else		
	{
		/*insert head...*/
		rt->next = rtl865x_route_inusedHead;		
		rtl865x_route_inusedHead = rt;
		
		start_idx = 0;
		start_rt = rtl865x_route_inusedHead;
	}	

	retval = _rtl865x_arrangeRoute(start_rt, start_idx);
	
	//_rtl865x_route_print();
	return retval;
	
}

static int32 _rtl865x_delRouteFromInusedList(rtl865x_route_t * rt)
{
	int32 retval,start_idx;
	rtl865x_route_t *fore_rt = NULL,*entry = NULL,*start_rt = NULL;

	entry = rtl865x_route_inusedHead;
	while(entry)
	{
		if(entry == rt)
			break;

		fore_rt = entry;		
		entry = entry->next;
	}

	/*fore_rt == NULL means delete list head*/
	if(fore_rt == NULL)
	{
		rtl865x_route_inusedHead = rtl865x_route_inusedHead->next;
		start_rt = rtl865x_route_inusedHead;
		start_idx = 0;
	}
	else
	{
		fore_rt->next = rt->next;
		start_rt = fore_rt->next;
		start_idx = fore_rt->asicIdx + 1;
	}

	/*delete route from asic*/
	if(rt->asicIdx < RT_ASIC_ENTRY_NUM)
		rtl8651_delAsicRouting(rt->asicIdx);
	
	retval = _rtl865x_arrangeRoute(start_rt, start_idx);
	rt->asicIdx = 0x0f;
	
	//_rtl865x_route_print();
	
	return retval;	
	
}


static int32 _rtl865x_usedNetifInRoute(int8 *ifname)
{

	rtl865x_route_t *rt = NULL;
	rt = rtl865x_route_inusedHead;

	while(rt)
	{	
		if(memcmp(rt->dstNetif->name,ifname,strlen(ifname)) == 0)
			return SUCCESS;
		rt = rt->next;
	}	
	return FAILED;
}

#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_HARDWARE_ROUTE_ACL)
extern int rtl865x_hw_route_acl;

static int rtl865x_addAclForHwRoute(rtl865x_netif_local_t * netIf, ipaddr_t ipAddr, ipaddr_t ipMask)
{


	rtl865x_AclRule_t	rule;
	int32 ret=-1;

	if(rtl865x_hw_route_acl!=1)
	{
		return FAILED;
	}
	
	if((netIf->if_type!=IF_ETHER) || (ipAddr==0))
	{
		return FAILED;
	}
	if(strcmp(netIf->name,RTL_DRV_WAN0_NETIF_NAME)==0)

	{
		return FAILED;
	}
	
#if defined(CONFIG_RTL_MULTIPLE_WAN)
	if(strcmp(netIf->name,RTL_DRV_WAN1_NETIF_NAME)==0)  
	{
		return FAILED;
	}
#endif

	//printk("%s:%d,ifName is %s, ip is 0x%x,ipMask is 0x%x\n",__FUNCTION__,__LINE__,netIf->name,ipAddr,ipMask);		
	ret=rtl865x_regist_aclChain(netIf->name, RTL865X_ACL_ROUTE_USED, RTL865X_ACL_INGRESS);
	//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);		

	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_IP_RANGE;
	rule.actionType_		= RTL865X_ACL_TOCPU;
	rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
	rule.srcIpAddrLB_ 	= 0;
	rule.srcIpAddrUB_ 	= (ipAddr & ipMask) -1;

	rule.dstIpAddrLB_ 	= 0;
	rule.dstIpAddrUB_ 	= 0xFFFFFFFF;
	ret= rtl865x_add_acl(&rule, netIf->name, RTL865X_ACL_ROUTE_USED);

	//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);	


	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_IP_RANGE;
	rule.actionType_		= RTL865X_ACL_TOCPU;
	rule.pktOpApp_		= RTL865X_ACL_ALL_LAYER;
	rule.srcIpAddrLB_	= (ipAddr & ipMask) + (~ipMask)+1;
	rule.srcIpAddrUB_	= 0xFFFFFFFF;

	rule.dstIpAddrLB_	= 0;
	rule.dstIpAddrUB_	= 0xFFFFFFFF;
	ret= rtl865x_add_acl(&rule, netIf->name, RTL865X_ACL_ROUTE_USED);
	
	//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);	

	#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	#else
	rtl865x_reConfigDefaultAcl(netIf->name);
	#endif

	return SUCCESS;
}

static int rtl865x_delAclForHwRoute(rtl865x_netif_local_t * netIf, ipaddr_t ipAddr, ipaddr_t ipMask)
{
	int32 ret=-1;

	if((netIf->if_type!=IF_ETHER) || (ipAddr==0))
	{
		return FAILED;
	}
	
	
	//printk("%s:%d,ifName is %s, ip is 0x%x,ipMask is 0x%x\n",__FUNCTION__,__LINE__,netIf->name,ipAddr,ipMask);	

	ret=rtl865x_unRegist_aclChain(netIf->name, RTL865X_ACL_ROUTE_USED, RTL865X_ACL_INGRESS);
	

	#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	#else
	rtl865x_reConfigDefaultAcl(netIf->name);
	#endif

	return SUCCESS;
}

int rtl865x_reArrangeHwRouteAcl(void)
{
	
	rtl865x_route_t *rt=rtl865x_route_inusedHead;


	while(rt)
	{
		if((rtl865x_hw_route_acl ==1) && (rtl8651_getAsicOperationLayer()==4))
		{
			rtl865x_delAclForHwRoute(rt->dstNetif, rt->ipAddr, rt->ipMask);
			rtl865x_addAclForHwRoute(rt->dstNetif,  rt->ipAddr, rt->ipMask);
		}
		else
		{
			rtl865x_delAclForHwRoute(rt->dstNetif, rt->ipAddr, rt->ipMask);
		}

		
		rt = rt->next;
	}
	return SUCCESS;
}
#endif

static int32 _rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop, int8 * ifName,ipaddr_t srcIp)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_route_t *rt = NULL,*tmp_rt = NULL;
	int32 idx;
	int32 netSize = 0, usedArpCnt = 0;
	int32 retval = FAILED;
	//diag_printf("%s:%d,ifName is %s, ip is 0x%x,ipMask is 0x%x, nexthop is 0x%x\n",__FUNCTION__,__LINE__,ifName,ipAddr,ipMask, nextHop);		
	/*para check*/
	if(ifName == NULL)
		netif = _rtl865x_getDefaultWanNetif();
	else	
		netif = _rtl865x_getSWNetifByName(ifName);
	
	if(netif == NULL)
		return TBLDRV_EINVALIDINPUT;


	if(netif->if_type == IF_NONE)
		return TBLDRV_ENOLLTYPESPECIFY;
	idx = 0;
	for(idx = 0; idx < 32; idx++)
		if((1<<idx) & ipMask)
			break;

	netSize = 1<<idx;
	if(netSize > RT_MAX_ARP_SIZE)
		return TBLDRV_EINVALIDINPUT;

	/*
	*duplicate entry check:
	*	in Driver system, default route is always exist.
	*	so if ipMask == 0, it's means that default route should be update...
	*/
	if(ipMask != 0 && (rt = _rtl865x_getRouteEntry(ipAddr, ipMask)) != NULL)
	{
		//rt->ref_count++;
		return TBLDRV_EENTRYALREADYEXIST;
	}

	/*add default route: just update the default route becase the default route always exist!*/
	if(ipMask == 0)
	{
		rt = _rtl865x_getDefaultRoute();
		/*deference rt's orginal netif*/
		if(rt && rt->dstNetif)
			rtl865x_deReferNetif(rt->dstNetif->name);
	}

	/*allocate a new buffer for adding entry*/
	if(rt == NULL)
	{
		rt = rtl865x_route_freeHead;

		if(rt)
		{
			rtl865x_route_freeHead = rt->next;
		}
	}
	
	if(rt == NULL)
	{
		/*no buffer, default route should be TOCPU?*/
		return TBLDRV_ENOFREEBUFFER;
	}
		
	/*common information*/
	rt->ipAddr 	= ipAddr & ipMask;
	rt->ipMask 	= ipMask;
	rt->nextHop	= nextHop;
	rt->srcIp		= srcIp;
	rt->dstNetif 	= netif;	
	
	/*don't specify the nexthop ip address, it's means that:
	* all packet match this route entry should be forward by network interface with arp
	*/

	if(nextHop == 0 && ipMask != 0)
	{
		switch(netif->if_type)
		{
			case IF_ETHER:
				
				rt->process = RT_ARP;
				tmp_rt = rtl865x_route_inusedHead;
				while(tmp_rt)
				{
					if(tmp_rt->valid && tmp_rt->process == RT_ARP && tmp_rt->dstNetif == netif)
						usedArpCnt += tmp_rt->un.arp.arpend - tmp_rt->un.arp.arpend + 1;
					
					tmp_rt = tmp_rt->next;
				}
				if((usedArpCnt + netSize) > RT_MAX_ARP_SIZE)
				{
					diag_printf("!!!!ERROR!!!usedArp(%d),netsize(%d)\n",usedArpCnt,netSize);
					goto addFailed;
				}
				/*allocate arp entry for this route rule...*/
				
				retval = rtl865x_arp_tbl_alloc(rt);
				if( retval != SUCCESS)
				{
					diag_printf("error!!can't allocate arp for this route entry....retval(%d)\n",retval);
					goto addFailed;
				}

				rt->un.arp.arpIpIdx = 0; /*FIXME_hyking[this field is invalid, right?]*/
				
				break;

			case IF_PPPOE:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = RT_PPPOE;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if(ppp == NULL)
					{
						diag_printf("error!!can't get pppoe session information by interface name(%s)\n",netif->name);
						goto addFailed;
					}

					rt->un.pppoe.macInfo = &ppp->server_mac;
					rt->un.pppoe.pppInfo = ppp;
					
					/*update reference...*/
					rtl865x_referPpp(ppp->sessionId);
				}
				break;
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = RT_L2;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if(ppp == NULL)
					{
						/*printk("Warning!!CAn't get pptp/l2tp session information by interface name(%s)\n",netif->name);*/
						goto addFailed;
					}

					rt->un.direct.macInfo = &ppp->server_mac;
				}
				break;

			default:
				diag_printf("lltype(%d) is not support now....\n",netif->if_type);
				goto addFailed;
		}
		
	}
	else
	{
		/*if default is valid, delete nexthop firstly...*/
		if(rt->valid == 1 && rt->process == RT_NEXTHOP)
              {
                        int i;  
                        for ( i = rt->un.nxthop.nxtHopSta; i <= rt->un.nxthop.nxtHopEnd; i++)
                        {       
                                retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
                        }       
              }
		
		/*use nexthop type*/
		rt->process = RT_NEXTHOP;			
		switch(netif->if_type)
		{
			case IF_ETHER:
				retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, nextHop,srcIp);
				break;
			case IF_PPPOE:
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *pppoe;
					
					pppoe = rtl865x_getPppByNetifName(netif->name);

					if(pppoe != NULL)
					{
						/*got pppoe session*/
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, pppoe->sessionId,srcIp);
					}
					else
						/*nexthop's action is to CPU*/
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, 0,srcIp);
				}
				break;

			default:
				retval = FAILED;
				break;
				
		}
		
		if(retval != SUCCESS)
		{
			diag_printf("error!!add nexthop error! retval (%d)\n",retval);
			goto addFailed;	
		}		
		rt->un.nxthop.nhalog = RT_ALOG_SIP; /* use per-source IP */
		rt->un.nxthop.ipDomain = RT_DOMAIN_16_1;		
	}

	rt->valid		= 1;
	rt->ref_count	= 1;
	/*update reference....*/
	rtl865x_referNetif(netif->name);
	if(ipMask == 0)
		_rtl865x_setDefaultWanNetif(netif->name);

	/**/
	if(rt->asicIdx == RT_ASIC_ENTRY_NUM-1)
	{
		retval = _rtl865x_synRouteToAsic(rt);
	}
	else
	{		
		/*insert the adding route to inused list*/
		retval = _rtl865x_addRouteToInusedList(rt);		
	}

	/*if route is add, please enable Routing for the releated netif*/
	retval = rtl865x_enableNetifRouting(netif);
	
#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_HARDWARE_ROUTE_ACL)
		rtl865x_delAclForHwRoute(netif,  ipAddr, ipMask);
		if(rtl8651_getAsicOperationLayer()==4)
		{
			rtl865x_addAclForHwRoute(netif,  ipAddr, ipMask);
		}
#endif
	return retval;
	
addFailed:
	if(rt->asicIdx == RT_ASIC_ENTRY_NUM -1)
	{
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_CPU);
	}
	else
	{
		/*free this route entry and return error code...*/	
		memset(rt,0,sizeof(rtl865x_route_t));
		rt->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = rt;
	}
	return retval;
	
}

static int32 _rtl865x_delRoute( ipaddr_t ipAddr, ipaddr_t ipMask )
{
	rtl865x_route_t *entry;
	int32 i;
	int32 retval = 0;

	entry = _rtl865x_getRouteEntry(ipAddr, ipMask);

	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	//diag_printf("%s:%d,ip is 0x%x,ipMask is 0x%x, nexthop is 0x%x\n",__FUNCTION__,__LINE__, entry->ipAddr, entry->ipMask, entry->nextHop);		

	if(entry->asicIdx == RT_ASIC_ENTRY_NUM-1)
	{
		/*if default route
		* 1. reset default route
		* 2. reset entry->netif...
		*/
		rtl865x_netif_local_t *netif = NULL;
		_rtl865x_clearDefaultWanNetif(entry->dstNetif->name);

		netif = _rtl865x_getDefaultWanNetif();
		if(netif==NULL)
		{
			return TBLDRV_EINVNETIFNAME;
		}
		
		if(netif != entry->dstNetif)
		{
			rtl865x_deReferNetif(entry->dstNetif->name);
			entry->dstNetif = netif;
			rtl865x_referNetif(netif->name);
		}
		
		retval = _rtl865x_updateDefaultRoute(entry, RT_DEFAULT_RT_NEXTHOP_CPU);
	}
	else		
	{
		/*not default route*/
		switch(entry->process)
		{
			case RT_PPPOE:
				{
					rtl865x_ppp_t *ppp = entry->un.pppoe.pppInfo;
					if(ppp)
						rtl865x_deReferPpp(ppp->sessionId);
				}
				break;
			case RT_L2:
				/*
				* NOTE:this type not used now...
				* if we want to use it, please DELETE FDB entry to sure this L2 entry is deleted both software FDB table and Asic L2 table.
				*/
				break;
			case RT_ARP:
				/*free arp*/
				retval = rtl865x_arp_tbl_free(entry);
				if( retval != SUCCESS)
				{
					diag_printf("======error!!can't FREE arp entry for this route entry....retval(%d)\n",retval);					
				}
				break;				

			case RT_CPU:
			case RT_DROP:
				/*do nothing*/
				
				break;

			case RT_NEXTHOP:
				/*delete nexthop which is add by l3*/
				for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd; i++)
				{
					retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
				}				
				break;
		}

		/*FIXME_hyking: update netif reference count*/
		rtl865x_deReferNetif(entry->dstNetif->name);

		//printk("%s:%d,ifName is %s, ip is 0x%x,ipMask is 0x%x\n",__FUNCTION__,__LINE__,entry->dstNetif->name,ipAddr,ipMask);		
		#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_HARDWARE_ROUTE_ACL)
			rtl865x_delAclForHwRoute(entry->dstNetif,  ipAddr, ipMask);
		#endif
		/*remove from inused list...*/		
		_rtl865x_delRouteFromInusedList(entry);

		if(_rtl865x_usedNetifInRoute(entry->dstNetif->name) == FAILED)
			rtl865x_disableNetifRouting(entry->dstNetif);

		/*add to free list*/
		memset(entry,0,sizeof(rtl865x_route_t));
		entry->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = entry;

		retval = SUCCESS;		
	}	
	
	//_rtl865x_route_print();
	return retval;
	
}

rtl865x_route_t* _rtl85x_getRouteEntry(ipaddr_t dst)
{
	rtl865x_route_t *tmpRtEntry = NULL;
	rtl865x_route_t *rt=rtl865x_route_inusedHead;
	uint32 mask;

	mask = 0;
	while(rt)
	{
		if (rt->valid == 1 && rt->ipAddr == (rt->ipMask & dst) && mask <= rt->ipMask) {
			mask = rt->ipMask;
			tmpRtEntry = rt;
		}
		rt = rt->next;
	}
	return tmpRtEntry;
}

/*
@func int32 | rtl865x_addRoute |add a route entry.
@parm ipaddr_t | ipAddr | ip address.
@parm ipaddr_t | ipMask | ip mask.
@parm ipaddr_t | nextHop | the route's next hop.
@parm int8* | ifName | destination network interface. 
@parm ipaddr_t | srcIp |source IP
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EINVALIDINPUT | invalid input.
@rvalue RTL_ENOLLTYPESPECIFY | network interface's link type is not specified.
@rvalue RTL_EENTRYALREADYEXIST | route entry is already exist.
@rvalue RTL_ENOFREEBUFFER | not enough memory in system.
@comm
	if ifName=NULL, it means the destionation network interface of route entry with ip/ipMask/nextHop is default wan.
*/
int32 rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop,int8 * ifName,ipaddr_t srcIp)
{
	int32 retval = 0;
	unsigned int s;	
	
	//printk("========%s(%d), ip(0x%x),mask(0x%x),ifname(%s),nxthop(0x%x)\n",__FUNCTION__,__LINE__,ipAddr,ipMask,ifName,nextHop);
	s = splimp();
	retval = _rtl865x_addRoute(ipAddr,ipMask,nextHop,ifName,srcIp);		
	splx(s);	
	//printk("========%s(%d), ip(0x%x),mask(0x%x),ifname(%s),nxthop(0x%x),retval(%d)\n",__FUNCTION__,__LINE__,ipAddr,ipMask,ifName,nextHop,retval);
	//_rtl865x_route_print();
	return retval;
}
/*
@func int32 | rtl865x_delRoute |delete a route entry.
@parm ipaddr_t | ipAddr | ipAddress.
@parm ipaddr_t | ipMask | ipMask.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EENTRYNOTFOUND | not found the entry.
@comm	
*/
int32 rtl865x_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask)
{

	int32 retval = 0;
	unsigned int s;	
	//printk("========%s(%d), ip(0x%x),mask(0x%x)\n",__FUNCTION__,__LINE__,ipAddr,ipMask);
	s = splimp();
	retval = _rtl865x_delRoute(ipAddr,ipMask);
	splx(s);	
	//printk("==================================retval(%d)\n",retval);
	return retval;

}

/*
@func int32 | rtl865x_getRouteEntry |according the destination ip address, get the matched route entry.
@parm ipaddr_t | dst | destionation ip address.
@parm rtl865x_route_t* | rt | retrun value: route entry pointer
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_getRouteEntry(ipaddr_t dst,rtl865x_route_t *rt)
{
	int32 retval = FAILED;
	rtl865x_route_t *ret_entry = NULL;

	ret_entry = _rtl85x_getRouteEntry(dst);
	if(ret_entry && rt)
	{
		memcpy(rt,ret_entry,sizeof(rtl865x_route_t));		
		retval = SUCCESS;
	}
	return retval;
}

/*
@func int32 | rtl865x_initRouteTable |initialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_initRouteTable(void)
{
	int32 i;
	rtl865x_route_t *rt;
	rtl865x_route_freeHead = NULL;
	rtl865x_route_inusedHead = NULL;

	/*malloc buffer*/
	TBL_MEM_ALLOC(rt, rtl865x_route_t, RT_DRV_ENTRY_NUM);	
	memset(rt,0,sizeof(rtl865x_route_t)*RT_DRV_ENTRY_NUM);
	for(i = 0; i < RT_DRV_ENTRY_NUM; i++)
	{
		rt[i].next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = &rt[i];		
	}	

	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitRouteTable |reinitialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_reinitRouteTable(void)
{
	rtl865x_route_t *rt;
	rt = rtl865x_route_inusedHead;
	while(rt && rt->asicIdx != RT_ASIC_ENTRY_NUM -1)
	{
		_rtl865x_delRoute(rt->ipAddr,rt->ipMask);
		rt = rtl865x_route_inusedHead;
	}

	/*delete the last route*/
	rt = rtl865x_route_inusedHead;
	if(rt)
	{
		/*FIXME_hyking: update netif reference count*/
		rtl865x_deReferNetif(rt->dstNetif->name);
		
		/*remove from inused list...*/		
		_rtl865x_delRouteFromInusedList(rt);

		/*add to free list*/
		memset(rt,0,sizeof(rtl865x_route_t));
		rt->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = rt;
	}
	return SUCCESS;
}

void rtl_asicRouteShow(void)
{
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	int8 *strNetType[] = { "WAN", "DMZ", "LAN",  "RLAN"};
	uint32 idx, mask;
	int netIdx;

	diag_printf("%s\n", "ASIC L3 Routing Table:\n");
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++)
	{
		if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED)
		{
			diag_printf("\t[%d]  (Invalid)\n", idx);
			continue;
		}
		if (idx == RTL8651_ROUTINGTBL_SIZE-1)
			mask = 0;
		else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
				mask--;
		netIdx = asic_l3.internal<<1|asic_l3.DMZFlag;
		diag_printf("\t[%d]  %d.%d.%d.%d/%d process(%s) %s \n", idx, (asic_l3.ipAddr>>24),
			((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff), 
			mask, str[asic_l3.process],strNetType[netIdx]);
		
		switch(asic_l3.process) 
		{
		case 0x00:	/* PPPoE */
			diag_printf("\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;
			
		case 0x01:	/* L2 */
			diag_printf("              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x02:	/* ARP */
			diag_printf("             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart<<3, asic_l3.arpEnd<<3, asic_l3.arpIpIdx);
			break;

		case 0x03:	/* Reserved */
			;

		case 0x04:	/* CPU */
			diag_printf("             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x05:	/* NAPT Next Hop */
			diag_printf("              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
				asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
			break;

		case 0x06:	/* DROP */
			diag_printf("             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x07:	/* Reserved */
			/* pass through */
		default: 
		;
		}
	}

}


int rtl_insertRouteHook(unsigned long ipAddr, unsigned long ipMask, unsigned long nexthop, char *if_name)
{
#if defined(CONFIG_RTL_HARDWARE_NAT)
	int rc = FAILED;
	unsigned long srcIp = 0;

	srcIp = rtl_getWanIfpIp(if_name);

	if ((gHwNatEnabled !=0)&&(!MULTICAST(ipAddr) && !LOOPBACK(ipAddr) && !BADCLASS(ipAddr))){
		rc = rtl865x_addRoute(ntohl(ipAddr), ntohl(ipMask), ntohl(nexthop), if_name, srcIp);	
	}
	return rc;
#endif
	return FAILED;
}

int rtl_delRouteHook(unsigned long ipAddr, unsigned long ipMask)
{
#if defined(CONFIG_RTL_HARDWARE_NAT)
	return rtl865x_delRoute(ntohl(ipAddr), ntohl(ipMask));
#endif
	return FAILED;
}



