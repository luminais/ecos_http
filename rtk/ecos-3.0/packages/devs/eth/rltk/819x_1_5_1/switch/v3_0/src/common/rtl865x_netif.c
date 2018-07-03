/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
*
* Program : network interface driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_netif.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include <switch/rtl865x_ip_api.h>
#include <rtl/rtl865x_eventMgr.h>
#include <switch/rtl_glue.h>
#include <switch/rtl865x_netif.h>
#include "rtl865x_netif_local.h"
#include <rtl/rtl865x_eventMgr.h> /*call back function....*/

static rtl865x_netif_local_t *netifTbl;

static int (*rtl_get_drv_netifName_by_psName)(const char *psName,char *netifName);

static int32 _rtl865x_delNetif(char *name);

#if defined(CONFIG_RTL_HARDWARE_NAT)
/*****************************************************************************
 函 数 名  :  dump_netifTbl
 功能描述  : 调试硬件转发
 输入参数  : 
 			      
 输出参数  : 无
 
 返 回 值  : 

 修改历史      :
  1.日    期   : 2016年12月29日
    作    者   : 林玲珑
    修改内容   : 新生成函数

*****************************************************************************/
void dump_netifTbl(void)
{


	if (!netifTbl)
		return;
	
	int i;

	
	for(i = 0; i < NETIF_NUMBER; i++)
	{

		printf ("vid=%d, mtu=%d, macAddrNumber=%d, inAclStart=%d,inAclEnd=%d,outAclStart=%d, outAclEnd=%d  enableRoute=%d\n",  
			netifTbl[i].vid, netifTbl[i].mtu, netifTbl[i].macAddrNumber,
			netifTbl[i].inAclStart, netifTbl[i].inAclEnd, netifTbl[i].outAclStart, netifTbl[i].outAclEnd, netifTbl[i].enableRoute);
			
		printf (" mac[%2x:%2x:%2x:%2x:%2x:%2x]  name=%s=\n",
			netifTbl[i].macAddr.octet[0], netifTbl[i].macAddr.octet[1],
			netifTbl[i].macAddr.octet[2], netifTbl[i].macAddr.octet[3],netifTbl[i].macAddr.octet[4],netifTbl[i].macAddr.octet[5],
			netifTbl[i].name);
		
		printf (" valid=%d  if_type=%d refCnt=%d  asicIdx=%d is_wan=%d is_defaultWan=%d  is_slave=%d\n", 
			netifTbl[i].valid, netifTbl[i].if_type,netifTbl[i].refCnt,netifTbl[i].asicIdx,
			netifTbl[i].is_wan,netifTbl[i].is_defaultWan,netifTbl[i].dmz, netifTbl[i].is_slave);
			
	}


}
#endif

static int32 _rtl865x_setAsicNetif(rtl865x_netif_local_t *entry)
{
	int32 retval = FAILED;
	rtl865x_tblAsicDrv_intfParam_t asicEntry;

	if(entry->is_slave == 1)
		return retval;
	
	memset(&asicEntry,0,sizeof(rtl865x_tblAsicDrv_intfParam_t));
	asicEntry.enableRoute = entry->enableRoute;
	asicEntry.inAclStart = entry->inAclStart;
	asicEntry.inAclEnd = entry->inAclEnd;
	asicEntry.outAclStart = entry->outAclStart;
	asicEntry.outAclEnd = entry->outAclEnd;
	//asicEntry.macAddr = entry->macAddr;
	memcpy(asicEntry.macAddr.octet,entry->macAddr.octet, 6);
	asicEntry.macAddrNumber = entry->macAddrNumber;
	asicEntry.mtu = entry->mtu;
	asicEntry.vid = entry->vid;
	asicEntry.valid = entry->valid;
#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	asicEntry.mtuV6 		= entry->mtuV6;
	asicEntry.enableRouteV6 = entry->enableRouteV6;
#endif

	retval = rtl8651_setAsicNetInterface( entry->asicIdx, &asicEntry);
	return retval;

}

rtl865x_netif_local_t *_rtl865x_getSWNetifByName(char *name)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;

	if(name == NULL)
		return NULL;

	for(i = 0; i < NETIF_NUMBER; i++)
	{
		//printk("%s:%d,i(%d),valid(%d),ifname(%s),strlen of name(%d), netifTbl(0x%p),netifTblName(%s)\n",__FUNCTION__,__LINE__,i,netifTbl[i].valid,name,strlen(name),&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && strlen(name) == strlen(netifTbl[i].name) && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			netif = &netifTbl[i];
			break;
		}
	}

	return netif;
}

rtl865x_netif_local_t *_rtl865x_getNetifByName(char *name)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;

	if(name == NULL)
		return NULL;

	for(i = 0; i < NETIF_NUMBER; i++)
	{
		//printk("i(%d),ifname(%s),netifTbl(0x%p),netifTblName(%s)\n",i,name,&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && strlen(name) == strlen(netifTbl[i].name) && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			if(netifTbl[i].is_slave == 0)
				netif = &netifTbl[i];
			else
			{
				netif = netifTbl[i].master;
			}
			break;
		}
	}
	
	return netif;
}

rtl865x_netif_local_t *_rtl865x_getDefaultWanNetif(void)
{
	int32 i;
	rtl865x_netif_local_t *firstWan, *defNetif;
	firstWan = defNetif = NULL;

	for(i = 0; i < NETIF_NUMBER; i++)
	{
		//printk("i(%d),netifTbl(0x%p)\n",i,&netifTbl[i]);
		if(netifTbl[i].valid == 1 && netifTbl[i].is_wan == 1 && firstWan == NULL)
			firstWan = &netifTbl[i];

		if(netifTbl[i].valid == 1 && netifTbl[i].is_defaultWan == 1)
		{
			defNetif = &netifTbl[i];
			break;
		}
	}

	/*if not found default wan, return wan interface first found*/
	if(defNetif == NULL)
	{
		defNetif = firstWan;
	}

	return defNetif;

}

int32 _rtl865x_setDefaultWanNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getSWNetifByName(name);

	//printk("set default wan interface....(%s)\n",name);
	if(entry)
		entry->is_defaultWan = 1;

	return SUCCESS;
}

int32 _rtl865x_clearDefaultWanNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getSWNetifByName(name);

	//printk("set default wan interface....(%s)\n",name);
	if(entry)
		entry->is_defaultWan = 0;

	return SUCCESS;
}

static int32 _rtl865x_attachMasterNetif(char *slave, char *master)
{
	rtl865x_netif_local_t *slave_netif, *master_netif;

	slave_netif = _rtl865x_getSWNetifByName(slave);
	master_netif = _rtl865x_getNetifByName(master);

	if(slave_netif == NULL || master_netif == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	//printk("===%s(%d),slave(%s),master(%s),slave_netif->master(0x%p)\n",__FUNCTION__,__LINE__,slave,master,slave_netif->master);
	if(slave_netif->master != NULL)
		return TBLDRV_EENTRYALREADYEXIST;

	slave_netif ->master = master_netif;

	return SUCCESS;

}

static int32 _rtl865x_detachMasterNetif(char *slave)
{
	rtl865x_netif_local_t *slave_netif;

	slave_netif = _rtl865x_getSWNetifByName(slave);

	if(slave_netif == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	slave_netif ->master = NULL;

	return SUCCESS;
}

int32 _rtl865x_addNetif(rtl865x_netif_t *netif)
{
	rtl865x_netif_local_t *entry;
	int32 retval = FAILED;
	int32 i;
	
	if(netif == NULL)
		return TBLDRV_EINVALIDINPUT;

	/*duplicate entry....*/
	entry = _rtl865x_getSWNetifByName(netif->name);
	if(entry)
		return TBLDRV_EENTRYALREADYEXIST;

	/*get netif buffer*/
	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid == 0)
			break;
	}


	if(i == NETIF_NUMBER)
		return TBLDRV_ENOFREEBUFFER;
	/*add new entry*/
	entry = &netifTbl[i];

	entry->valid = 1;
	entry->mtu = netif->mtu;
	entry->if_type = netif->type;
	entry->macAddr = netif->macAddr;
	entry->vid = netif->vid;
	entry->is_wan = netif->is_wan;
	entry->dmz = netif->dmz;
	entry->is_slave = netif->is_slave;
	memcpy(entry->name,netif->name,MAX_IFNAMESIZE);
	entry->asicIdx = i;
	entry->enableRoute = netif->enableRoute;
	entry->macAddrNumber = 1;
	#if defined(CONFIG_RTL_LAYERED_DRIVER)
	entry->inAclStart = netif->inAclStart;
	entry->inAclEnd  = netif->inAclEnd;
	entry->outAclStart = netif->outAclStart;
	entry->outAclEnd = netif->outAclEnd;
	#else
	entry->inAclEnd = entry->inAclStart = entry->outAclEnd = entry->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
	#endif
	entry->refCnt = 1;
	entry->master = NULL;
	#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    entry->enableRouteV6 = netif->enableRouteV6;
    entry->mtuV6         = netif->mtuV6;
    #endif

	/*only write master interface into ASIC*/
	if(entry->is_slave == 0)
	{
		retval = _rtl865x_setAsicNetif(entry);
		//if(retval == SUCCESS)
			//rtl865x_referVlan(entry->vid);

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
		/*register 2 ingress chains: system/user*/
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_SYSTEM_USED, RTL865X_ACL_INGRESS);
#if RTL_LAYERED_DRIVER_DEBUG
		printk("register system acl chain, return %d\n",retval);
		_rtl865x_print_freeChainNum();
#endif
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED, RTL865X_ACL_INGRESS);
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED, RTL865X_ACL_EGRESS);

#if RTL_LAYERED_DRIVER_DEBUG
		printk("register user acl chain, return %d\n",retval);
		_rtl865x_print_freeChainNum();
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
#endif
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
	}


	return SUCCESS;
}

static int32 _rtl865x_delNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	int32 retval = FAILED;

	/*FIXME:hyking, get swNetif entry.....*/
	entry = _rtl865x_getSWNetifByName(name);
	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	if(entry->refCnt > 1)
	{
		diag_printf("name(%s),refcnt(%d)\n",name,entry->refCnt);
		return TBLDRV_EREFERENCEDBYOTHER;
	}

	if(entry->is_slave == 0)
	{
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
		retval = _rtl865x_unRegister_all_aclChain(name);
#endif

		retval = rtl865x_delNetInterfaceByVid(entry->vid);
		if(retval == SUCCESS)
		{
			//rtl865x_deReferVlan(entry->vid);

			/*flush acl*/
			#if 0
			do_eventAction(EV_DEL_NETIF, (void*)entry);
			#else
			rtl865x_raiseEvent(EVENT_DEL_NETIF, (void*)entry);
			#endif
		}

		/*now delete all slave interface whose master is the deleting master interface*/
		{
			int32 i ;

			for(i = 0; i < NETIF_NUMBER; i++)
			{
				if(netifTbl[i].valid == 1 && netifTbl[i].is_slave == 1 && netifTbl[i].master == entry)
					netifTbl[i].master = NULL;
			}
		}
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
#if RTL_LAYERED_DRIVER_DEBUG
		printk("unregist all acl chain, return %d\n",retval);
		_rtl865x_print_freeChainNum();
#endif
#endif
	}

	//entry->valid = 0;
	memset(entry,0,sizeof(rtl865x_netif_local_t));
	retval = SUCCESS;

	return retval;
}

static int32 _rtl865x_referNetif(char *ifName)
{
	rtl865x_netif_local_t *entry;
	if(ifName == NULL)
		return FAILED;

	entry = _rtl865x_getSWNetifByName(ifName);
	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	entry->refCnt++;
	return SUCCESS;
}

static int32 _rtl865x_deReferNetif(char *ifName)
{
	rtl865x_netif_local_t *entry;
	if(ifName == NULL)
		return FAILED;

	entry = _rtl865x_getSWNetifByName(ifName);
	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	entry->refCnt--;

	return SUCCESS;
}

static int32 _rtl865x_setNetifVid(char *name, uint16 vid)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL || vid < 1 ||vid > 4095)
		return TBLDRV_EINVALIDINPUT;

	entry = _rtl865x_getSWNetifByName(name);

	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	//if(entry->vid > 0 && entry->vid <= 4095)
		//rtl865x_deReferVlan(entry->vid);

	entry->vid = vid;

	/*update asic table*/
	if (entry->is_slave)
		return SUCCESS;
	else
		return _rtl865x_setAsicNetif(entry);
}


static int32 _rtl865x_setNetifType(char *name, uint32 ifType)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL || ifType <= IF_NONE ||ifType > IF_L2TP)
		return TBLDRV_EINVALIDINPUT;

	entry = _rtl865x_getSWNetifByName(name);

	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	entry->if_type = ifType;

	return SUCCESS;
}

int32 _rtl865x_setNetifMac(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;

	if(netif == NULL)
		return TBLDRV_EINVALIDINPUT;
	entry = _rtl865x_getNetifByName(netif->name);

	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	entry->macAddr = netif->macAddr;

	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);

	return retval;

}

int32 _rtl865x_setNetifMtu(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(netif->name);

	if(entry == NULL)
		return TBLDRV_EENTRYNOTFOUND;

	entry->mtu = netif->mtu;

	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);

	return retval;

}

int32 _rtl865x_getNetifIdxByVid(uint16 vid)
{
	int32 i;
	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid == 1 && netifTbl[i].vid == vid)
			break;
	}

	if(i == NETIF_NUMBER)
		return -1;

	return i;
}

int32 _rtl865x_getNetifIdxByName(uint8 *name)
{
	int32 i;
	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid == 1 && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
			break;
	}

	if(i == NETIF_NUMBER)
		return -1;

	return i;
}

int32 _rtl865x_getNetifIdxByNameExt(uint8 *name)
{
	int32 i;
	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid == 1 && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			if (netifTbl[i].is_slave==TRUE)
			{
				if(netifTbl[i].master)
					return _rtl865x_getNetifIdxByNameExt(netifTbl[i].master->name);
				else
					return -1;
			}
			else
				return i;
		}
	}

	return -1;
}

int32 rtl865x_attachMasterNetif(char *slave, char *master)
{
	return _rtl865x_attachMasterNetif(slave, master);
}

int32 rtl865x_detachMasterNetif(char *slave)
{
	return _rtl865x_detachMasterNetif(slave);
}

/*
@func int32 | rtl865x_addNetif |add network interface.
@parm rtl865x_netif_t* | netif | network interface
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | netif is NULL
@rvalue RTL_EENTRYALREADYEXIST | netif is already exist
@rvalue RTL_ENOFREEBUFFER | no netif to used
@rvalue FAILED | Failed
@comm
	rtl865x_netif_t: please refer in header file.
*/

int32 rtl865x_addNetif(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned int s;
	
	s = splimp();
	retval = _rtl865x_addNetif(netif);
	splx(s);
	return retval;
}

/*
@func int32 | rtl865x_delNetif |delete network interface.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue RTL_EREFERENCEDBYOTHER | netif is referenced by onter table entry
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_delNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned int s;
	
	s = splimp();
	retval = _rtl865x_delNetif(ifName);
	splx(s);
	return retval;
}
/*
@func int32 | rtl865x_referNetif |reference network interface entry.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
when other table entry refer network interface table entry, please call this API.
*/
int32 rtl865x_referNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned int s;
	
	s = splimp();
	retval = _rtl865x_referNetif(ifName);
	splx(s);
	return retval;
}

/*
@func int32 | rtl865x_deReferNetif |dereference network interface.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
this API should be called after rtl865x_referNetif.
*/
int32 rtl865x_deReferNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned int s;
	
	s = splimp();
	retval = _rtl865x_deReferNetif(ifName);
	splx(s);
	return retval;
}

/*
@func int32 | rtl865x_setNetifVid |mapping network interface with vlan.
@parm char* | name | network interface name
@parm uint16 | vid | vlan id
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifVid(char *name, uint16 vid)
{
	int32 ret;
	unsigned int s;
	
	s = splimp();
	ret = _rtl865x_setNetifVid(name,vid);
	splx(s);
	return ret;
}

int32 rtl865x_setPortToNetif(char *name,uint32 port)
{
	int32 ret;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
		return FAILED;

	ret = rtl8651_setPortToNetif(port, entry->asicIdx);
	return ret;
}

/*
@func int32 | rtl865x_setNetifType |config network interface type.
@parm char* | ifName | network interface name
@parm uint32 | ifType | interface type. IF_ETHER/IF_PPPOE/IF_PPTP/IF_L2TP allowed.
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifType(char *name, uint32 ifType)
{
	int32 ret;
	unsigned int s;
	
	s = splimp();
	ret = _rtl865x_setNetifType(name,ifType);
	splx(s);
	return ret;
}

/*
@func int32 | rtl865x_setNetifMac |config network interface Mac address.
@parm rtl865x_netif_t* | netif | netif name&MAC address
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifMac(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned int s;
	
	s = splimp();
	retval = _rtl865x_setNetifMac(netif);
	splx(s);
	return retval;
}

/*
@func int32 | rtl865x_setNetifMtu |config network interface MTU.
@parm rtl865x_netif_t* | netif | netif name & MTU
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifMtu(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned int s;
	
	s = splimp();
	retval = _rtl865x_setNetifMtu(netif);
	splx(s);
	return retval;
}


/*
@func int32 | rtl865x_initNetifTable | initialize network interface table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initNetifTable(void)
{
	TBL_MEM_ALLOC(netifTbl, rtl865x_netif_local_t, NETIF_NUMBER);
	memset(netifTbl,0,sizeof(rtl865x_netif_local_t)*NETIF_NUMBER);
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	/*init reserved acl in function init_acl...*/
#else
	//_rtl865x_confReservedAcl();
#endif

	rtl_get_drv_netifName_by_psName = NULL;

	return SUCCESS;
}

int32 rtl865x_config_callback_for_get_drv_netifName(int (*fun)(const char *psName,char *netifName))
{
	rtl_get_drv_netifName_by_psName = fun;
	return SUCCESS;
}

int32 rtl865x_get_drvNetifName_by_psName(const char *psName,char *netifName)
{
	if(strlen(psName) >= MAX_IFNAMESIZE)
		return FAILED;

	if(rtl_get_drv_netifName_by_psName)
		rtl_get_drv_netifName_by_psName(psName,netifName);
	else
	{
		memcpy(netifName,psName,MAX_IFNAMESIZE);
	}

	return SUCCESS;
}

/*
@func int32 | rtl865x_enableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_enableNetifRouting(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;

	if(netif == NULL)
		return TBLDRV_EINVALIDINPUT;
	if(netif ->enableRoute == 1)
		return SUCCESS;

	netif->enableRoute = 1;
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

/*
@func int32 | rtl865x_disableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_disableNetifRouting(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;

	if(netif == NULL)
		return TBLDRV_EINVALIDINPUT;

	if(netif ->enableRoute == 0)
		return SUCCESS;

	netif->enableRoute = 0;
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

/*
@func int32 | rtl865x_disableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_reinitNetifTable(void)
{
	int32 i;
	unsigned int s;
    
	s = splimp();
	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid)
		{
			_rtl865x_delNetif(netifTbl[i].name);
		}
	}
	splx(s);
	return SUCCESS;
}

void rtl865x_swNetifShow(void)
{
	int8	*pst[] = { "DIS/BLK",  "LIS", "LRN", "FWD" };
	uint8 *mac;
	int32 i, j;

	diag_printf("%s\n", "SW Network Interface Table:");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_vlanParam_t vlan;
		rtl865x_netif_local_t *netif;

		netif = &netifTbl[i];
		if (netif->valid == 0)
			continue;
		
		printk("netif[%d]\n", i);
		mac = (uint8 *)&netif->macAddr.octet[0];
		diag_printf("[%d]  %s VID[%d] %x:%x:%x:%x:%x:%x", 
			i, netif->name, netif->vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		diag_printf("  Routing %s \n",
			netif->enableRoute==TRUE? "enabled": "disabled" );

		diag_printf("      ingress ");
		diag_printf("ACL %d-%d, ", netif->inAclStart, netif->inAclEnd);

		diag_printf("  egress ");
		diag_printf("ACL %d-%d, ", netif->outAclStart, netif->outAclEnd);

		diag_printf("\n      %d MAC Addresses, MTU %d Bytes\n", netif->macAddrNumber, netif->mtu);
        #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
        uint16 vid = netif->vid;
        rtl8651_findAsicVlanIndexByVid(&vid);
        rtl8651_getAsicVlan(vid, &vlan);
        #else
		rtl8651_getAsicVlan( netif->vid, &vlan );
        #endif

		diag_printf("\n      Untag member ports:");

		for ( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
		{
			if ( vlan.untagPortMask & ( 1 << j ) )
				diag_printf("%d ", j);
		}
		diag_printf("\n      Active member ports:");

		for ( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
		{
			if ( vlan.memberPortMask & ( 1 << j ) )
				diag_printf("%d ", j);
		}

		#if 0
		diag_printf("\n      Port state(");

		for ( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
		{
			if ( ( vlan.memberPortMask & ( 1 << j ) ) == 0 )
				continue;
			if ((( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET ) > 4 )
				diag_printf("--- ");
			else
				diag_printf("%d:%s ", j, pst[(( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET )]);

		}
		diag_printf(")\n\n");
		#else
		diag_printf("\n\n");
		#endif

	}
}

#if 0
extern int rtl_vlan_support_enable;
int32 rtl865x_reConfigDefaultAcl(char *ifName)
{
	rtl865x_AclRule_t	rule;
	int ret=FAILED;

	unsigned int s;
	s = splimp();

#if defined (CONFIG_RTL_VLAN_SUPPORT)
		if(rtl_vlan_support_enable==0)
		{
			/*del old default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_del_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);

			/*add new default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_add_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);
		}
		else
		{
			/*del old default to cpu acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_TOCPU;
			ret=_rtl865x_del_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);

			/*add new default to cpu acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_TOCPU;
			ret=_rtl865x_add_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);
		}
#else
		{
			/*del old default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_del_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);

			/*add new default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_add_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);
		}
#endif
		splx(s);

		return SUCCESS;
}
#endif
#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_NAT_ACL)
extern int rtl865x_nat_acl;
static int rtl865x_addNatAclToCpu(rtl865x_netif_local_t * netIf)
{
	rtl865x_AclRule_t	rule;
	int32 ret=-1;

	if(rtl865x_nat_acl!=0)
	{
		return FAILED;
	}
	
	if(netIf->if_type!=IF_ETHER)
	{
		return FAILED;
	}

	ret=rtl865x_regist_aclChain(netIf->name, RTL865X_ACL_NAT_USED, RTL865X_ACL_INGRESS);

	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_MAC;
	rule.actionType_		= RTL865X_ACL_TOCPU;
	rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
	
	rule.dstMac_.octet[0]=netIf->macAddr.octet[0];
	rule.dstMac_.octet[1]=netIf->macAddr.octet[1];
	rule.dstMac_.octet[2]=netIf->macAddr.octet[2];
	rule.dstMac_.octet[3]=netIf->macAddr.octet[3];
	rule.dstMac_.octet[4]=netIf->macAddr.octet[4];
	rule.dstMac_.octet[5]=netIf->macAddr.octet[5];
		
	rule.dstMacMask_.octet[0]=0xFF;
	rule.dstMacMask_.octet[1]=0xFF;
	rule.dstMacMask_.octet[2]=0xFF;
	rule.dstMacMask_.octet[3]=0xFF;
	rule.dstMacMask_.octet[4]=0xFF;
	rule.dstMacMask_.octet[5]=0xFF;

	
	ret= rtl865x_add_acl(&rule, netIf->name, RTL865X_ACL_NAT_USED);

	//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);	

	#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	#else
	rtl865x_reConfigDefaultAcl(netIf->name);
	#endif

	return SUCCESS;
}

static int rtl865x_delNatAclToCpu(rtl865x_netif_local_t * netIf)
{
	int32 ret=-1;

	if(netIf->if_type!=IF_ETHER)
	{
		return FAILED;
	}
	
	
	//printk("%s:%d,ifName is %s, ip is 0x%x,ipMask is 0x%x\n",__FUNCTION__,__LINE__,netIf->name,ipAddr,ipMask);	

	ret=rtl865x_unRegist_aclChain(netIf->name, RTL865X_ACL_NAT_USED, RTL865X_ACL_INGRESS);
	

	#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	#else
	rtl865x_reConfigDefaultAcl(netIf->name);
	#endif

	return SUCCESS;
}

int rtl865x_reArrangeNatAcl(void)
{
	int i;
	
	/*get netif buffer*/
	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid == 0)
		{
			continue;
		}

		if((rtl865x_nat_acl ==1) && (rtl8651_getAsicOperationLayer()==4))
		{
			rtl865x_delNatAclToCpu(&netifTbl[i]);
		}
		else
		{
			rtl865x_delNatAclToCpu(&netifTbl[i]);
			rtl865x_addNatAclToCpu(&netifTbl[i]);
		}
		
	}
	
	return SUCCESS;
}
#endif
/*=====================================
*acl releated function
*======================================*/
int32 _rtl865x_setACLToNetif(char *name, uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx)
{
	rtl865x_netif_local_t *netif = NULL;
	int32 i;

    if (name == NULL)
		return FAILED;
    
	for(i = 0 ; i < NETIF_NUMBER; i++)
	{
		netif = &netifTbl[i];
		if((netif->valid == 0) || (netif->is_slave == 1) || (strlen(name) != strlen(netifTbl[i].name)) ||
            (memcmp(netifTbl[i].name,name,strlen(name)) != 0))
			continue;

		netif->inAclStart = start_ingressAclIdx;
		netif->inAclEnd = end_ingressAclIdx;
		netif->outAclStart = start_egressAclIdx;
		netif->outAclEnd = end_egressAclIdx;
		_rtl865x_setAsicNetif(netif);
	}
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	rtl865x_setDefACLForNetDecisionMiss(start_ingressAclIdx,end_ingressAclIdx,start_egressAclIdx,end_egressAclIdx);
#endif

	return SUCCESS;
}

