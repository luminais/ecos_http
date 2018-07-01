/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
*
* Program : ip table driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_ip.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include "rtl865x_ip.h"
#include <switch/rtl865x_ip_api.h>
#include <rtl/rtl865x_eventMgr.h>
#include <switch/rtl_glue.h>

static rtl865x_ip_entry_t *rtl865x_ipTable;

#define IP_TABLE_INDEX(entry)	(entry - rtl865x_ipTable)

static int32 _rtl865x_delIp(ipaddr_t extIp);

/*
@func int32 | rtl865x_initIpTable | initialize ip table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int rtl865x_initIpTable(void)
{
	TBL_MEM_ALLOC(rtl865x_ipTable, rtl865x_ip_entry_t, IP_NUMBER);
	memset(rtl865x_ipTable,0,sizeof(rtl865x_ip_entry_t)*IP_NUMBER);
	return SUCCESS;
}

/*
@func int32 | rtl865x_initIpTable | reinitialize ip table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed.
*/
int rtl865x_reinitIpTable(void)
{
	int32 i ;
	
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid)
			_rtl865x_delIp(rtl865x_ipTable[i].extIp);
	}
	
	return SUCCESS;
}

static int32 _rtl865x_addIp(ipaddr_t intIp, ipaddr_t extIp, uint32 ip_type)
{
	int i;
	rtl865x_ip_entry_t *entry = NULL;
	rtl865x_tblAsicDrv_extIntIpParam_t asicIp;
	int32 retval = FAILED;

	if(/*ip_type < IP_TYPE_NAPT || */ip_type > IP_TYPE_LOCALSERVER)
		return TBLDRV_EINVALIDINPUT;

	/*duplicate check*/
	/*found a valid entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 0)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
		else
		{
			if(rtl865x_ipTable[i].extIp == extIp)
				return TBLDRV_EENTRYALREADYEXIST;

			if(ip_type == IP_TYPE_NAPT && rtl865x_ipTable[i].type == IP_TYPE_NAPT && rtl865x_ipTable[i].defNaptIp == 1)
				return TBLDRV_EENTRYALREADYEXIST;
		}
	}

	if(entry == NULL)
		return TBLDRV_ENOFREEBUFFER;


	/*update releated information*/
	entry->valid = 1;
	entry->intIp = intIp;
	entry->extIp = extIp;
	entry->type = ip_type;

	/*from 865xC, this field is invalid...*/
	entry->nexthop = NULL;


	/*add this ip entry to asic*/
	/* Set asic */
	bzero(&asicIp, sizeof(rtl865x_tblAsicDrv_extIntIpParam_t));
	asicIp.extIpAddr	= extIp;
	asicIp.intIpAddr	= intIp;
	asicIp.localPublic	= (ip_type == IP_TYPE_LOCALSERVER)? TRUE: FALSE;
	asicIp.nat			= (ip_type == IP_TYPE_NAT)? TRUE: FALSE;
	asicIp.nhIndex		= 0;

	retval = rtl8651_setAsicExtIntIpTable(IP_TABLE_INDEX(entry), &asicIp);

	if(ip_type == IP_TYPE_NAPT)
		rtl8651_setAsicOperationLayer(4);

	return SUCCESS;

}

static int32 _rtl865x_ipTableIsNull(void)
{
	int i;

	/*found the entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 1)
		{
			return FAILED;
		}
	}

	return SUCCESS;
}

static int32 _rtl865x_delIp(ipaddr_t extIp)
{
	int i;
	rtl865x_ip_entry_t *entry = NULL;

	/*found the entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 1 && rtl865x_ipTable[i].extIp == extIp)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	}

	/*update asic ip table*/
	if(entry!=NULL)
	{
		rtl8651_delAsicExtIntIpTable(IP_TABLE_INDEX(entry));
		memset(entry,0,sizeof(rtl865x_ip_entry_t));
	}
	else
		return TBLDRV_EENTRYNOTFOUND;

	//no entry in ip table, set operation to layer 2
	if(_rtl865x_ipTableIsNull() == SUCCESS)
	{
		#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
			rtl8651_setAsicOperationLayer(3);
		#else
			rtl8651_setAsicOperationLayer(2);
		#endif
	}

	return SUCCESS;

}

static rtl865x_ip_entry_t* _rtl865x_getIpEntryByExtIp(ipaddr_t extIp)
{
	int32 i;
	rtl865x_ip_entry_t *entry = NULL;

	for(i = 0; i < IP_NUMBER; i++)
		if(rtl865x_ipTable[i].valid == 1 && rtl865x_ipTable[i].extIp == extIp)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	return entry;

}

/*
@func rtl865x_ip_entry_t* | rtl865x_getIpEntryByIp | get ip entry .
@parm ipaddr_t | extIp | ip address
@rvalue ip_entry | Success.
@rvalue NULL | Failed
*/
rtl865x_ip_entry_t* rtl865x_getIpEntryByIp(ipaddr_t extIp)
{
	return _rtl865x_getIpEntryByExtIp(extIp);
}


/*
@func int32 | rtl865x_getIpIdxByExtIp | get asic idex .
@parm ipaddr_t | extIp | ip address
@parm int32* | idx | index
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
*/
int32 rtl865x_getIpIdxByExtIp(ipaddr_t extIp, int32 *idx)
{
	int32 retval = FAILED;
	rtl865x_ip_entry_t *entry = NULL;

	entry = _rtl865x_getIpEntryByExtIp(extIp);
	if(entry)
	{
		if(idx)
			*idx = IP_TABLE_INDEX(entry);
		retval = SUCCESS;
	}

	return retval;
}

/*
@func int32 | rtl865x_addIp | add ip table entry.
@parm ipaddr_t | intIp | internal ip address
@parm ipaddr_t | extIp | external ip address
@parm uint32 | ip_type | entry type. support IP_TYPE_NAPT/IP_TYPE_NAT/IP_TYPE_LOCALSERVER
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | Invalid input.
@rvalue RTL_EENTRYALREADYEXIST | entry already exist.
@rvalue RTL_ENOFREEBUFFER | not enough buffer in System.
@rvalue FAILED | Failed.
@comm
the extIp is the primary key of the ip table.
*/
int rtl865x_addIp(unsigned long intIp, unsigned long extIp, unsigned int ip_type)
{
	int32 retval = FAILED;
	unsigned long s;
	//rtl_down_interruptible(&ip_sem);
	s = splimp();
	retval = _rtl865x_addIp(intIp, extIp, ip_type);
	//rtl_up(&ip_sem);
	splx(s);
	return retval;
}

/*
@func int32 | rtl865x_delIp | delete ip table entry.
@parm ipaddr_t | extIp | external ip address
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | Invalid input.
@rvalue RTL_EENTRYNOTFOUND | not found.
@rvalue FAILED | Failed.
@comm
the extIp is the primary key of the ip table.
*/
int rtl865x_delIp(unsigned long extIp)
{
	int32 retval = FAILED;
	unsigned long s;
	//rtl_down_interruptible(&ip_sem);
	s = splimp();
	retval = _rtl865x_delIp(extIp);
	//rtl_up(&ip_sem);
	splx(s);
	return retval;

}

int32 rtl865x_getIPIdx(rtl865x_ip_entry_t *entry, int32 *idx)
{
	if(idx)
	{
		*idx =IP_TABLE_INDEX(entry);
		return SUCCESS;
	}
	return FAILED;
}

void rtl865x_asicIpShow(void)
{
	rtl865x_tblAsicDrv_extIntIpParam_t asic_ip;
	int32	i;
	int8 intIpBuf[sizeof"255.255.255.255"];
	int8 extIpBuf[sizeof"255.255.255.255"];

	diag_printf("%s\n", "ASIC IP Table:\n");

	for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
	{
		if (rtl8651_getAsicExtIntIpTable(i,  &asic_ip) == FAILED)
		{
			diag_printf("  [%d] (Invalid)\n", i);
			continue;
		}
		inet_ntoa_r(asic_ip.intIpAddr, intIpBuf);
		inet_ntoa_r(asic_ip.extIpAddr,extIpBuf);			
		diag_printf("  [%d] intip(%-14s) extip(%-14s) type(%s) nhIdx(%d)\n",
					i, intIpBuf,extIpBuf,
					(asic_ip.localPublic==TRUE? "LP" : (asic_ip.nat==TRUE ? "NAT" : "NAPT")), asic_ip.nhIndex);
	}

}

