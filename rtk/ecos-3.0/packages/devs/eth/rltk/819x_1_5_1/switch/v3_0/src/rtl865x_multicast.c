/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __linux__
#include <linux/config.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#elif defined(__ECOS)
#include <assert.h>
#include <sys/time.h>
#include <pkgconf/system.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif





#include "rtl_types.h"
//#include <net/rtl/rtl_glue.h>
//#include "common/rtl_utils.h"
//#include "common/assert.h"

#if 1//def CONFIG_RTL_LAYERED_ASIC_DRIVER
//#include "rtl865x_asicCom.h"
#include "rtl865x_asicL3.h"
#include "rtl865xC_tblAsicDrv.h"
//#include "rtl865x_tblDrvPatch.h"

#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h" 
#include "common/rtl865x_tblDrvPatch.h"
#endif

#include "asicregs.h"
//#include "AsicDriver/asicTabs.h"

//#include "common/rtl8651_tblDrvProto.h"

#include <rtl/rtl865x_eventMgr.h>

//#include "vlanTable.h"
//#include <net/rtl/rtl865x_netif.h>

//#include "l3Driver/rtl865x_ip.h"

#ifdef RTL865X_TEST
#include <string.h>
#endif

#include <rtl/rtl865x_multicast.h>
#include <rtl/rtl865x_igmpsnooping.h>

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
int32 rtl_initMulticastv6(void);
int32 rtl_reinitMulticastv6(void);
unsigned int _rtl819x_getAddMcastv6OpCnt(void);
unsigned int _rtl819x_getDelMcastv6OpCnt(void);
unsigned int _rtl819x_getForceAddMcastv6OpCnt(void);
static void _rtl819x_arrangeMulticastv6(uint32 entryIndex);
extern int32 rtl_getGroupInfov6(uint32 * groupAddr,struct rtl_groupInfo * groupInfo);
#endif
/********************************************************/
/*			Multicast Related Global Variable			*/
/********************************************************/

static rtl865x_mcast_fwd_descriptor_t *rtl865x_mcastFwdDescPool=NULL;
static mcast_fwd_descriptor_head_t  free_mcast_fwd_descriptor_head;
static struct rtl865x_multicastTable mCastTbl;
static uint32 rtl865x_externalMulticastPortMask = 0;
#if defined(__ECOS)
static struct timer_list rtl865x_mCastSysTimer;	/*igmp timer*/
#endif
//extern int rtl865x_totalExtPortNum;

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
static rtl819x_mcast_fwd_descriptor6_t *rtl819x_mcastFwdDescPool6=NULL;
static mcast_fwd_descriptor_head6_t free_mcast_fwd_descriptor_head6;
static struct rtl819x_multicastv6Table mCastTbl6;
static uint32 rtl819x_externalMulticastPortMask6 = 0;
#if defined(__ECOS)
static struct timer_list rtl819x_mCast6SysTimer;	/*mld timer*/
#endif
#endif


static int32 _rtl865x_initMCastFwdDescPool(void)
{
	int32 i;


	MC_LIST_INIT(&free_mcast_fwd_descriptor_head);

	TBL_MEM_ALLOC(rtl865x_mcastFwdDescPool, rtl865x_mcast_fwd_descriptor_t,MAX_MCAST_FWD_DESCRIPTOR_CNT);
	
	if(rtl865x_mcastFwdDescPool!=NULL)
	{
		memset( rtl865x_mcastFwdDescPool, 0, MAX_MCAST_FWD_DESCRIPTOR_CNT * sizeof(rtl865x_mcast_fwd_descriptor_t));	
	}
	else
	{
		return FAILED;
	}
	
	for(i = 0; i<MAX_MCAST_FWD_DESCRIPTOR_CNT;i++)
	{
		MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head, &rtl865x_mcastFwdDescPool[i], next);
	}
	
	return SUCCESS;
}

static rtl865x_mcast_fwd_descriptor_t *_rtl865x_allocMCastFwdDesc(void)
{
	rtl865x_mcast_fwd_descriptor_t *retDesc=NULL;
	retDesc = MC_LIST_FIRST(&free_mcast_fwd_descriptor_head);
	if(retDesc!=NULL)
	{
		MC_LIST_REMOVE(retDesc, next);
		memset(retDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	}
	return retDesc;
}

static int32 _rtl865x_freeMCastFwdDesc(rtl865x_mcast_fwd_descriptor_t *descPtr)
{
	if(descPtr==NULL)
	{
		return SUCCESS;
	}
	memset(descPtr,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head, descPtr, next);
	
	return SUCCESS;
}

static int32 _rtl865x_flushMCastFwdDescChain(mcast_fwd_descriptor_head_t * descChainHead)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc,*nextDesc;
	
	if(descChainHead==NULL)
	{
		return SUCCESS;
	}
	
	curDesc=MC_LIST_FIRST(descChainHead);
	while(curDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		/*remove from the old descriptor chain*/
		MC_LIST_REMOVE(curDesc, next);
		/*return to the free descriptor chain*/
		_rtl865x_freeMCastFwdDesc(curDesc);
		curDesc = nextDesc;
	}

	return SUCCESS;
}



static int32 _rtl865x_mCastFwdDescEnqueue(mcast_fwd_descriptor_head_t * queueHead,
												rtl865x_mcast_fwd_descriptor_t * enqueueDesc)
{

	rtl865x_mcast_fwd_descriptor_t *newDesc;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(enqueueDesc==NULL)
	{
		return SUCCESS;
	}
	
	/*multicast forward descriptor is internal maintained,always alloc new one*/
	newDesc=_rtl865x_allocMCastFwdDesc();
	
	if(newDesc!=NULL)
	{
		memcpy(newDesc, enqueueDesc,sizeof(rtl865x_mcast_fwd_descriptor_t ));
		//memset(&(newDesc->next), 0, sizeof(MC_LIST_ENTRY(rtl865x_mcast_fwd_descriptor_s)));
		newDesc->next.le_next=NULL;
		newDesc->next.le_prev=NULL;
	}
	else
	{
		/*no enough memory*/
		return FAILED;
	}
	

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{

		nextDesc=MC_LIST_NEXT(curDesc, next);
		
		/*merge two descriptor*/
	//	if((strcmp(curDesc->netifName,newDesc->netifName)==0) && (curDesc->vid==newDesc->vid))
		if(strcmp(curDesc->netifName,newDesc->netifName)==0)
		{	
			
			if(newDesc->descPortMask==0)
			{
				newDesc->descPortMask=curDesc->descPortMask;
			}
			MC_LIST_REMOVE(curDesc, next);
			_rtl865x_freeMCastFwdDesc(curDesc);
			
		}
	}

	/*not matched descriptor is found*/
	MC_LIST_INSERT_HEAD(queueHead, newDesc, next);

	return SUCCESS;
	
}


static int32 _rtl865x_mergeMCastFwdDescChain(mcast_fwd_descriptor_head_t * targetChainHead ,
													rtl865x_mcast_fwd_descriptor_t *srcChain)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc;

	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		
		_rtl865x_mCastFwdDescEnqueue(targetChainHead, curDesc);
		
	}
	
	return SUCCESS;
}




static int32 _rtl865x_initMCastEntryPool(void)
{
	int32 index;
	rtl865x_tblDrv_mCast_t *multiCast_t;
	
	TBL_MEM_ALLOC(multiCast_t, rtl865x_tblDrv_mCast_t ,MAX_MCAST_TABLE_ENTRY_CNT);
	TAILQ_INIT(&mCastTbl.freeList.freeMultiCast);
	for(index=0; index<MAX_MCAST_TABLE_ENTRY_CNT; index++)
	{
		memset( &multiCast_t[index], 0, sizeof(rtl865x_tblDrv_mCast_t));
		TAILQ_INSERT_HEAD(&mCastTbl.freeList.freeMultiCast, &multiCast_t[index], nextMCast);
	}

	TBL_MEM_ALLOC(multiCast_t, rtl865x_tblDrv_mCast_t, RTL8651_MULTICASTTBL_SIZE);
	memset(multiCast_t, 0,RTL8651_MULTICASTTBL_SIZE* sizeof(rtl865x_tblDrv_mCast_t));
	mCastTbl.inuseList.mCastTbl = (void *)multiCast_t;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{
		TAILQ_INIT(&mCastTbl.inuseList.mCastTbl[index]);
	}

	return SUCCESS;
}

static rtl865x_tblDrv_mCast_t * _rtl865x_allocMCastEntry(uint32 hashIndex)
{
	rtl865x_tblDrv_mCast_t *newEntry;
	newEntry=TAILQ_FIRST(&mCastTbl.freeList.freeMultiCast);
	if (newEntry == NULL)
	{
		return NULL;
	}		
	
	TAILQ_REMOVE(&mCastTbl.freeList.freeMultiCast, newEntry, nextMCast);

	
	/*initialize it*/
	if(MC_LIST_FIRST(&newEntry->fwdDescChain)!=NULL)
	{
		_rtl865x_flushMCastFwdDescChain(&newEntry->fwdDescChain);
	}
	MC_LIST_INIT(&newEntry->fwdDescChain);
	
	memset(newEntry, 0, sizeof(rtl865x_tblDrv_mCast_t));

	TAILQ_INSERT_TAIL(&mCastTbl.inuseList.mCastTbl[hashIndex], newEntry, nextMCast);
	
	return newEntry;
}

static int32 _rtl865x_flushMCastEntry(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	_rtl865x_flushMCastFwdDescChain(&mCastEntry->fwdDescChain);
	
	memset(mCastEntry, 0, sizeof(rtl865x_tblDrv_mCast_t));
	return SUCCESS;
}

static int32 _rtl865x_freeMCastEntry(rtl865x_tblDrv_mCast_t * mCastEntry, uint32 hashIndex)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[hashIndex], mCastEntry, nextMCast);
	_rtl865x_flushMCastEntry(mCastEntry);
	TAILQ_INSERT_HEAD(&mCastTbl.freeList.freeMultiCast, mCastEntry, nextMCast);
	return SUCCESS;
}


static uint32 _rtl865x_doMCastEntrySrcVlanPortFilter(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc,*nextDesc;
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	for(curDesc=MC_LIST_FIRST(&mCastEntry->fwdDescChain);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next);
		{
			curDesc->fwdPortMask=curDesc->fwdPortMask & (~(1<<mCastEntry->port));
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl865x_freeMCastFwdDesc(curDesc);

			}
		}
		
	}

	return SUCCESS;
}

 
static uint32 rtl865x_genMCastEntryAsicFwdMask(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 asicFwdPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(curDesc->toCpu==0)
		{
			asicFwdPortMask|=(curDesc->fwdPortMask & ((1<<RTL8651_MAC_NUMBER)-1));
		}
		else
		{
			asicFwdPortMask|=( 0x01<<RTL8651_MAC_NUMBER);
		}
	}
	asicFwdPortMask = asicFwdPortMask & (~(1<<mCastEntry->port)); 
	return asicFwdPortMask;
}

static uint16 rtl865x_genMCastEntryCpuFlag(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint16 cpuFlag=FALSE;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}

	if(mCastEntry->cpuHold==TRUE)
	{
		cpuFlag=TRUE;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(	(curDesc->toCpu==TRUE)	||
			(memcmp(curDesc->netifName, RTL_WLAN_NAME,4)==0)	)
		{
			cpuFlag=TRUE;
		}
	}
	
	return cpuFlag;
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
/*for linux bridge level igmp snooping usage*/
static uint32 rtl865x_getMCastEntryDescPortMask(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 descPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		descPortMask=descPortMask | curDesc->descPortMask;
	}
	
	return descPortMask;
}

#endif
/*=======================================
  * Multicast Table APIs
  *=======================================*/
#define RTL865X_MULTICASE_TABLE_APIs

static void  _rtl865x_setASICMulticastPortStatus(void) {
	uint32 index;

	for (index=0; index<RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER; index++) {
		rtl8651_setAsicMulticastPortInternal(index, (rtl865x_externalMulticastPortMask&(1<<index))?FALSE:TRUE);
	}
}

void rtl865x_arrangeMulticastPortStatus(void) {

	rtl865x_externalMulticastPortMask=rtl865x_getExternalPortMask();
	_rtl865x_setASICMulticastPortStatus();
}

/*
@func int32	| rtl865x_addMulticastExternalPort	| API to add a hardware multicast external port.
@parm  uint32 | extPort	| External port number to be added. 
@rvalue SUCCESS	|Add hardware multicast external port successfully.
@rvalue FAILED	|Add hardware multicast external port failed.
*/
int32 rtl865x_addMulticastExternalPort(uint32 extPort)
{
	rtl865x_externalMulticastPortMask |= (1<<extPort);
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_delMulticastExternalPort	| API to delete a hardware multicast external port.
@parm  uint32 | extPort	| External port number to be deleted.
@rvalue SUCCESS	|Delete external port successfully.
@rvalue FAILED	|Delete external port failed.
*/
int32 rtl865x_delMulticastExternalPort(uint32 extPort)
{
	rtl865x_externalMulticastPortMask &= ~(1<<extPort);
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_setMulticastExternalPortMask	| API to set hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be set.
@rvalue SUCCESS	|Set external port mask successfully.
@rvalue FAILED	|Set external port mask failed.
*/
int32 rtl865x_setMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask =extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_addMulticastExternalPortMask	| API to add hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be added.
@rvalue SUCCESS	|Add external port mask successfully.
@rvalue FAILED	|Add external port mask failed.
*/
int32 rtl865x_addMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask|= extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_delMulticastExternalPortMask	|  API to delete hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be deleted.
@rvalue SUCCESS	|Delete external port mask successfully.
@rvalue FAILED	|Delete external port mask failed.
*/
int32 rtl865x_delMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask &= ~extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

int32 rtl865x_getMulticastExternalPortMask(void)
{
	return rtl865x_externalMulticastPortMask ;
}

static inline void _rtl865x_patchPppoeWeak(rtl865x_tblDrv_mCast_t *mCast_t)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	uint32 netifType;
	/* patch: keep cache in software if one vlan's interface is pppoe */
	MC_LIST_FOREACH(curDesc, &(mCast_t->fwdDescChain), next)
	{
		//if(rtl865x_getNetifType(curDesc->netifName, &netifType)==SUCCESS)
		{
			/*how about pptp,l2tp?*/
			if(netifType==IF_PPPOE)
			{
				mCast_t->flag |= RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
				return;
			}
		}
		
	}

	mCast_t->flag &= ~RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
}
#if 0
static int _rtl865x_checkMulticastEntryEqual(rtl865x_tblDrv_mCast_t * mCastEntry1, rtl865x_tblDrv_mCast_t * mCastEntry2)
{
	if((mCastEntry1==NULL) && (mCastEntry2==NULL))
	{
		return TRUE;
	}
	
	if((mCastEntry1==NULL) && (mCastEntry2!=NULL))
	{
		return FALSE;
	}

	if((mCastEntry1!=NULL) && (mCastEntry2==NULL))
	{
		return FALSE;
	}
	
	if(mCastEntry1->sip!=mCastEntry2->sip)
	{
		return FALSE;
	}

	if(mCastEntry1->dip!=mCastEntry2->dip)
	{
		return FALSE;
	}

	if(mCastEntry1->svid!=mCastEntry2->svid)
	{
		return FALSE;
	}
	
	if(mCastEntry1->port!=mCastEntry2->port)
	{
		return FALSE;
	}

	if(mCastEntry1->mbr!=mCastEntry2->mbr)
	{
		return FALSE;
	}
	
	if(mCastEntry1->cpu!=mCastEntry2->cpu)
	{
		return FALSE;
	}
	
	if(mCastEntry1->extIp!=mCastEntry2->extIp)
	{
		return FALSE;
	}

	if(mCastEntry1->flag!=mCastEntry2->flag)
	{
		return FALSE;
	}

	
	if(mCastEntry1->inAsic!=mCastEntry2->inAsic)
	{
		return FALSE;
	}			

	return TRUE;
}
#endif
#if 1//def CONFIG_PROC_FS
static unsigned int mcastAddOpCnt=0;
unsigned int _rtl865x_getAddMcastOpCnt(void)
{
	return mcastAddOpCnt;
}

static unsigned int mcastDelOpCnt=0;
unsigned int _rtl865x_getDelMcastOpCnt(void)
{
	return mcastDelOpCnt;
}
static unsigned int mcastForceAddOpCnt=0;
unsigned int _rtl865x_getForceAddMcastOpCnt(void)
{
	return mcastForceAddOpCnt;
}

#endif
/* re-select Multicast entry to ASIC for the index ""entryIndex */
static void _rtl865x_arrangeMulticast(uint32 entryIndex)
{
	rtl865x_tblAsicDrv_multiCastParam_t asic_mcast;
	rtl865x_tblDrv_mCast_t *mCast_t=NULL;
	rtl865x_tblDrv_mCast_t *select_t=NULL;
	rtl865x_tblDrv_mCast_t *swapOutEntry=NULL;
	int32 retval;
	
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entryIndex], nextMCast) 
	{
		if ((mCast_t->cpu == 0) && !(mCast_t->flag & RTL865X_MULTICAST_PPPOEPATCH_CPUBIT)) 
		{ /* Ignore cpu=1 */

			if(mCast_t->inAsic==TRUE)
			{
				if(swapOutEntry==NULL)
				{
					swapOutEntry=mCast_t;
				}
				else
				{
					/*impossible, two flow in one asic entry*/
					swapOutEntry->inAsic=FALSE;
					mCast_t->inAsic = FALSE;
				}
			}
		
			if (select_t) 
			{


				if ((mCast_t->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
				{
					/*replace unknown multicast*/
					select_t = mCast_t;
				}
				else
				{
					/*select the heavy load*/
					if ((mCast_t->count) > (select_t->count))
					{
						select_t = mCast_t;
					}
				}
				
			}
			else 
			{
				select_t = mCast_t;
			}
			
			
		}
		else
		{
			mCast_t->inAsic = FALSE;	/* reset "inAsic" bit */
		} 


	}
	
	if(select_t && swapOutEntry)
	{
		if ((swapOutEntry->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
		{
			/*replace unknown multicast*/
			select_t = swapOutEntry;
		}
		else
		{
			if((select_t->count <= (swapOutEntry->count+RTL865X_HW_MCAST_SWAP_GAP)))
				select_t = swapOutEntry;
		}
	}	
	

	
	/*
	if(swapOutEntry)
	{
		diag_printf("%s:%d,swapOutEntry->count:%d,swapOutEntry->dip is 0x%x,swapOutEntry->mbr is 0x%x\n",__FUNCTION__,__LINE__,swapOutEntry->count,swapOutEntry->dip,swapOutEntry->mbr);

	}
	
	if (select_t) 
	{
		diag_printf("%s:%d,select_t->count:%d,select_t->dip is 0x%x,select_t->mbr is 0x%x\n",__FUNCTION__,__LINE__,select_t->count,select_t->dip,select_t->mbr);
	}
	*/
	if (select_t) 
	{
		if((swapOutEntry==NULL) ||(select_t==swapOutEntry))
		{
			select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
			bzero(&asic_mcast, sizeof(rtl865x_tblAsicDrv_multiCastParam_t));
			memcpy(&asic_mcast, select_t, (uint32)&(((rtl865x_tblDrv_mCast_t *)0)->extIp));
			if (select_t->extIp)
			{
			
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
				int32 ipIdx;
				if(rtl865x_getIpIdxByExtIp(select_t->extIp, &ipIdx)==SUCCESS)
				{
					asic_mcast.extIdx=(uint16)ipIdx;
				}
#else
				asic_mcast.extIdx=0;
#endif
		
			}
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
			asic_mcast.idx=(uint16)entryIndex;
	#endif
	#if defined(CONFIG_RTL_819X_SWCORE)
			retval = rtl8651_setAsicIpMulticastTable(&asic_mcast);
	#endif
			
#if 1//def CONFIG_PROC_FS
			mcastAddOpCnt++;
#endif
//			ASSERT_CSP(retval == SUCCESS);
			if(retval==SUCCESS)
			{
				select_t->inAsic = TRUE;
			}
			else
			{			
				diag_printf("%s,%d. setAsicIpMulticastTable fail\n",__FUNCTION__,__LINE__);
				select_t->inAsic = FALSE;
				#if defined(CONFIG_RTL_819X_SWCORE)
				rtl8651_delAsicIpMulticastTable(entryIndex);
				#endif
#if 1//def CONFIG_PROC_FS
				mcastDelOpCnt++;
#endif
			}
				
			ASSERT_CSP(retval == SUCCESS);
			TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
			TAILQ_INSERT_HEAD(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
		}
		else/*(swapOutEntry!=NULL) && (select_t!=swapOutEntry)*/
		{
			
			/*disable swap and only explicit joined mulicast flow can replace unknown multicast flow*/
			if(1)
			{
				/*don't forget to set swapOutEntry's inAsic flag*/
				swapOutEntry->inAsic=FALSE;
				
				select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
				bzero(&asic_mcast, sizeof(rtl865x_tblAsicDrv_multiCastParam_t));
				memcpy(&asic_mcast, select_t, (uint32)&(((rtl865x_tblDrv_mCast_t *)0)->extIp));

				if (select_t->extIp)
				{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3		
					int32 ipIdx;
					if(rtl865x_getIpIdxByExtIp(select_t->extIp, &ipIdx)==SUCCESS)
					{
						asic_mcast.extIdx=(uint16)ipIdx;
					}
#else
					asic_mcast.extIdx=0;
#endif
				}
				
				#if defined(CONFIG_RTL_819X_SWCORE)
				retval = rtl8651_setAsicIpMulticastTable(&asic_mcast);
				#endif
				//printk("asic_mcast:dip:%x,[%s]:[%d].\n",asic_mcast.dip,__FUNCTION__,__LINE__);
#if 1//def CONFIG_PROC_FS
				mcastAddOpCnt++;
#endif
//				ASSERT_CSP(retval == SUCCESS);
				if(retval==SUCCESS)
				{
					select_t->inAsic = TRUE;
				}
				else
				{
					diag_printf("%s,%d. setAsicIpMulticastTable fail\n",__FUNCTION__,__LINE__);
					select_t->inAsic = FALSE;
					#if defined(CONFIG_RTL_819X_SWCORE)
					rtl8651_delAsicIpMulticastTable(entryIndex);
					#endif
#if 1//def CONFIG_PROC_FS
					mcastDelOpCnt++;
#endif
				}
				
				TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
				TAILQ_INSERT_HEAD(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);

			}
			#if 0
			else
			{	
				//printk("swapOutEntry:%d,select:%d,[%s]:[%d].\n",swapOutEntry->unKnownMCast,select_t->unKnownMCast,__FUNCTION__,__LINE__);
				if(swapOutEntry->inAsic == FALSE)
				{
					/*maybe something is wrong, we remove the asic entry*/
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt++;
#endif
				}
				
			}	
			#endif
			
		}
		
	}
	else 	
	{
		if(swapOutEntry!=NULL)
		{
			swapOutEntry->inAsic=FALSE;
		}
		#if defined(CONFIG_RTL_819X_SWCORE)
		rtl8651_delAsicIpMulticastTable(entryIndex);
		#endif
#if 1//def CONFIG_PROC_FS
		mcastDelOpCnt++;
#endif
	}
}


static void _rtl865x_mCastEntryReclaim(void)
{
	uint32 index;
	uint32 freeCnt=0;
	uint32 asicFwdPortMask=0;
	uint32 needReArrange=FALSE;
	rtl865x_tblDrv_mCast_t *curMCastEntry, *nextMCastEntry;

	/*free unused software forward entry*/
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]);
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if((curMCastEntry->inAsic==FALSE)  && (curMCastEntry->count==0))
			{
				_rtl865x_freeMCastEntry(curMCastEntry, index);
				freeCnt++;
			}
			curMCastEntry = nextMCastEntry;
		}
		
	}

	if(freeCnt>0)
	{
		return;
	}
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]);
		needReArrange=FALSE;
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if(curMCastEntry->inAsic)
			{
				asicFwdPortMask=rtl865x_genMCastEntryAsicFwdMask(curMCastEntry);
				if(asicFwdPortMask==0) 
				{
					_rtl865x_freeMCastEntry(curMCastEntry, index);
					needReArrange=TRUE;
				}
			}
			curMCastEntry = nextMCastEntry;
		}
		
		if(needReArrange==TRUE)
		{
			_rtl865x_arrangeMulticast(index);
		}
	}

	return;
}
/*
@func rtl865x_tblDrv_mCast_t *	| rtl865x_findMCastEntry	|  API to find a hardware multicast entry.
@parm  ipaddr_t 	| mAddr	| Multicast stream destination group address. 
@parm  ipaddr_t	|  sip	| Multicast stream source ip address.
@parm  uint16		| svid	| Multicast stream input vlan index.
@parm  uint16 	| sport	| Multicast stream input port number.
*/
rtl865x_tblDrv_mCast_t *rtl865x_findMCastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport)
{
	rtl865x_tblDrv_mCast_t *mCast_t;
	#if defined(CONFIG_RTL_819X_SWCORE)
	uint32 entry = rtl8651_ipMulticastTableIndex(sip, mAddr);
	#else
	uint32 entry = 0;
	#endif
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast) {
		if (mCast_t->dip==mAddr && mCast_t->sip==sip && mCast_t->svid==svid && mCast_t->port==sport)
		{
			if (mCast_t->inAsic == FALSE) 
			{
				mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
				mCast_t->count ++;
			}

			return mCast_t;
		}
	}
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	for(entry=RTL8651_IPMULTICASTTBL_SIZE; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast) {
			if (mCast_t->dip==mAddr && mCast_t->sip==sip && mCast_t->svid==svid && mCast_t->port==sport)
			{
				if (mCast_t->inAsic == FALSE) 
				{
					mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					mCast_t->count ++;
				}

				return mCast_t;
			}
					
		}
	}
#endif	
	return (rtl865x_tblDrv_mCast_t *)NULL;	
}


#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)

int rtl865x_findEmptyCamEntry(void)
{
	int index=-1;

	for(index=RTL8651_IPMULTICASTTBL_SIZE; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{

		if(TAILQ_EMPTY(&mCastTbl.inuseList.mCastTbl[index]))
		{
			return index;
		}
	}
	
	return -1;
}
#endif

/*
@func int32	| rtl865x_addMulticastEntry	|  API to add a hardwawre multicast forwarding entry.
@parm  ipaddr_t 	| mAddr	| Multicast flow Destination group address. 
@parm  ipaddr_t 	| sip	| Multicast flow source ip address. 
@parm  uint16 	| svid	| Multicast flow input vlan index. 
@parm  uint16		| sport	| Multicast flow input port number. 
@parm  rtl865x_mcast_fwd_descriptor_t *	| newFwdDescChain	| Multicast flow forwarding descriptor chain to be added. 
@parm  int32 	| flushOldChain	| Flag to indicate to flush old mulicast forwarding descriptor chain or not. 1 is to flush old chain, and 0 is not to. 
@parm  ipaddr_t 	| extIp	| External source ip address used when forward multicast data from lan to wan. 
@parm  int8	| toCpu	| Cpu forwarding flag, 1 is to forward multicast data by cpu,and  0 is not.
@parm  int8	| flag	| For future usage, set to 0 at present.
@rvalue SUCCESS	|Add hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Add hardware multicast forwarding entry failed.
*/
int32 rtl865x_addMulticastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport, 
									rtl865x_mcast_fwd_descriptor_t * newFwdDescChain, 
									int32 flushOldChain, ipaddr_t extIp, char cpuHold, uint8 flag)
{

	rtl865x_tblDrv_mCast_t *mCast_t;
	#if defined(CONFIG_RTL_819X_SWCORE)
	uint32 hashIndex = rtl8651_ipMulticastTableIndex(sip, mAddr);
	#else
	uint32 hashIndex = 0;
	#endif
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	uint32 emptyCamIndex=-1; 
	#endif
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	struct rtl_groupInfo groupInfo;
	#endif
	/*windows xp upnp:239.255.255.0*/
	if(mAddr==0xEFFFFFFA)
	{
		return FAILED;
	}
#if 0
	/*reserved multicast address 224.0.0.x*/
	if((mAddr & 0xFFFFFF00) == 0xE0000000)
	{
		return FAILED;
	}
#endif	

#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t=rtl865x_findMCastEntry(mAddr, sip, svid, sport);
	if(mCast_t==NULL)
	{
		/*table entry collided*/
		if(!TAILQ_EMPTY(&mCastTbl.inuseList.mCastTbl[hashIndex]))
		{
			emptyCamIndex=rtl865x_findEmptyCamEntry();
			if(emptyCamIndex!=-1)
			{
				hashIndex=emptyCamIndex;
			}
		}
	}
	else
	{
		hashIndex=mCast_t->hashIndex;
	}
#else
	/*try to match hash line*/
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[hashIndex], nextMCast) 
	{
		if (mCast_t->sip==sip && mCast_t->dip==mAddr && mCast_t->svid==svid && mCast_t->port==sport)
			break;
	}
#endif	
	
	if (mCast_t == NULL) 
	{
		mCast_t=_rtl865x_allocMCastEntry(hashIndex);
		if (mCast_t == NULL)
		{
			_rtl865x_mCastEntryReclaim();
			mCast_t=_rtl865x_allocMCastEntry(hashIndex);
			if(mCast_t == NULL)
			{
				return FAILED;
			}
		}
		mCast_t->sip			= sip;
		mCast_t->dip			= mAddr;
		mCast_t->svid		= svid;
		mCast_t->port		= sport;
		mCast_t->mbr		= 0;
		mCast_t->count		= 0;
		//mCast_t->maxPPS		= 0;
		
		
		mCast_t->inAsic		= FALSE;
	}
	
	if(flushOldChain)
	{
		_rtl865x_flushMCastFwdDescChain(&mCast_t->fwdDescChain);
		
	}
	
	_rtl865x_mergeMCastFwdDescChain(&mCast_t->fwdDescChain,newFwdDescChain);
	_rtl865x_doMCastEntrySrcVlanPortFilter(mCast_t);
	
	mCast_t->mbr			= rtl865x_genMCastEntryAsicFwdMask(mCast_t);
	mCast_t->extIp			= extIp;

	mCast_t->age			= RTL865X_MULTICAST_TABLE_AGE;
#if 0
	mCast_t->cpu			= (toCpu==TRUE? 1: 0);
#else
	mCast_t->cpuHold			= cpuHold;
	mCast_t->cpu 			= rtl865x_genMCastEntryCpuFlag(mCast_t);
#endif	
	mCast_t->flag			= flag;
	
	if (extIp)
		mCast_t->flag |= RTL865X_MULTICAST_EXTIP_SET;
	else
		mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	#if defined(CONFIG_RTL_819X_SWCORE)
	rtl_getGroupInfo(mAddr, &groupInfo);
	#endif
	if(groupInfo.ownerMask==0)
	{
		mCast_t->unKnownMCast=TRUE;
	}
	else
	{
		mCast_t->unKnownMCast=FALSE;
	}
	#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t->hashIndex=hashIndex;
#endif
	_rtl865x_patchPppoeWeak(mCast_t);
	_rtl865x_arrangeMulticast(hashIndex);
	return SUCCESS;	
}


/*
@func int32	| rtl865x_delMulticastEntry	|  API to delete multicast forwarding entry related with a certain group address.
@parm  ipaddr_t 	| mcast_addr	| Group address to be mached in deleting hardware multicast forwarding entry. 
@rvalue SUCCESS	|Delete hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Delete hardware multicast forwarding entry failed.
*/
int32 rtl865x_delMulticastEntry(ipaddr_t mcast_addr)
{

	rtl865x_tblDrv_mCast_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	uint32 deleteFlag=FALSE;

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		deleteFlag=FALSE;
		mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if (!mcast_addr || mCastEntry->dip == mcast_addr) 
			{
				deleteFlag=TRUE;
				_rtl865x_freeMCastEntry(mCastEntry, entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		
		if(deleteFlag==TRUE)
		{
			_rtl865x_arrangeMulticast(entry);
		}
	}

	return SUCCESS;
}

#if 0
/*the following function maybe used in future*/

int32 rtl865x_addMulticastFwdDesc(ipaddr_t mcast_addr, rtl865x_mcast_fwd_descriptor_t * newFwdDesc)
{

	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 entry, matchedIdx = 0;
	uint32 oldFwdPortMask,newFwdPortMask;
	if(newFwdDesc==NULL)
	{
		return SUCCESS;
	}

	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast)
		{
			if (mCast_t->dip != mcast_addr)
				continue;

			oldFwdPortMask=mCast_t->mbr;

			_rtl865x_mergeMCastFwdDescChain(&mCast_t->fwdDescChain,newFwdDesc);
			_rtl865x_doMCastEntrySrcVlanPortFilter(mCast_t);
			
			mCast_t->mbr 		= rtl865x_genMCastEntryFwdMask(mCast_t);
			newFwdPortMask		= mCast_t->mbr ;
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
			if (mCast_t->flag & RTL865X_MULTICAST_UPLOADONLY)
			{	/* remove upload term*/
				if(oldFwdPortMask!=newFwdPortMask)
				{
					mCast_t->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
					/* we assume multicast member will NEVER in External interface, so we remove
					     external ip now */
					mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
					mCast_t->extIp= 0;
				}
			}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */

			_rtl865x_patchPppoeWeak(mCast_t);
			_rtl865x_arrangeMulticast(entry);
			matchedIdx = entry;
		}
	}

	if (matchedIdx) 
	{
		return SUCCESS;
	}
	return FAILED;
}

int32 rtl865x_delMulticastFwdDesc(ipaddr_t mcast_addr,  rtl865x_mcast_fwd_descriptor_t * deadFwdDesc)
{

	uint32 index;
	rtl865x_tblDrv_mCast_t  *mCastEntry, *nextMCastEntry;
	uint32 oldFwdPortMask,newFwdPortMask;
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{

		for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
		{
			nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
			
			if ((mcast_addr) && (mCastEntry->dip != mcast_addr))
			{
				continue;
			}
			
			oldFwdPortMask=mCastEntry->mbr;
		
			_rtl865x_subMCastFwdDescChain(&mCastEntry->fwdDescChain, deadFwdDesc);
			
			mCastEntry->mbr=rtl865x_genMCastEntryFwdMask(mCastEntry);
			newFwdPortMask=mCastEntry->mbr; 	
			if (mCastEntry->mbr == 0)
			{
				/*to-do:unknown multicast hardware blocking*/
				_rtl865x_freeMCastEntry(mCastEntry, index);
				mCastEntry=NULL;
				_rtl865x_arrangeMulticast(index);
			}
			else
			{
			
				_rtl865x_patchPppoeWeak(mCastEntry);
			}
			
		}
			
		_rtl865x_arrangeMulticast(index);
	}

	return SUCCESS;
}

int32 rtl865x_delMulticastUpStream(ipaddr_t mcast_addr, ipaddr_t sip, uint16 svid, uint16 sport)
{
	uint32 index;
	rtl865x_tblDrv_mCast_t *mCast_t;
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[index], nextMCast) 
		{
			if ((!mcast_addr || mCast_t->dip == mcast_addr) && 
				(!sip || mCast_t->sip==sip) && 
				(!svid || mCast_t->svid==svid) && 
				mCast_t->port==sport)
			{
				_rtl865x_freeMCastEntry(mCast_t, index);
				_rtl865x_arrangeMulticast(index);
				return SUCCESS;
			}
		}
	}
	return FAILED;
}

int32 rtl865x_delMulticastByVid(uint32 vid)
{
	uint16 sport;
	uint32 sportMask;
	rtl865x_mcast_fwd_descriptor_t vlanFwdDesc;
	memset(&vlanFwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	
	/* delete all upstream related to vid */
	sport = 0;
	sportMask=rtl865x_getVlanPortMask(vid);
	while (sportMask) 
	{
		if (sportMask & 1)
		{
			rtl865x_delMulticastUpStream(0, 0, vid, sport);
		}
		
		sportMask = sportMask >> 1;
		sport ++;
	}
	
	/* delete all downstream related to vid*/
	vlanFwdDesc.vid=vid;
	vlanFwdDesc.fwdPortMask=rtl865x_getVlanPortMask(vid);
	rtl865x_delMulticastFwdDesc(0, &vlanFwdDesc);

	return FAILED;
}

int32 rtl865x_delMulticastByPort(uint32 port)
{

	rtl865x_mcast_fwd_descriptor_t portFwdDesc;
	memset(&portFwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	
	/* delete all upstream related to this port */
	rtl865x_delMulticastUpStream(0, 0, 0, port);

	/* delete all downstream related to this port*/
	portFwdDesc.vid=0;
	portFwdDesc.fwdPortMask=1<<port;
	rtl865x_delMulticastFwdDesc(0, &portFwdDesc);

	return SUCCESS;
}

int32 rtl865x_setMGroupAttribute(ipaddr_t groupIp, int8 toCpu)
{
	uint32 index;
	rtl865x_tblDrv_mCast_t *mCast_t;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[index], nextMCast) 
		{
			if (mCast_t->dip == groupIp)
			{
				mCast_t->cpu = (toCpu==TRUE? 1: 0);
			}
		}
		_rtl865x_arrangeMulticast(index);
	}
	return SUCCESS;
}


static int32 _rtl865x_subMCastFwdDescChain(mcast_fwd_descriptor_head_t * targetChainHead,rtl865x_mcast_fwd_descriptor_t *srcChain)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc;
	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		_rtl865x_mCastFwdDescDequeue(targetChainHead, curDesc);
	}

	return SUCCESS;
}

static int32 _rtl865x_mCastFwdDescDequeue(mcast_fwd_descriptor_head_t * queueHead,rtl865x_mcast_fwd_descriptor_t * dequeueDesc)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(dequeueDesc==NULL)
	{
		return FAILED;
	}

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		if((strcmp(curDesc->netifName,dequeueDesc->netifName)==0) ||
			((dequeueDesc->vid==0 ) ||(curDesc->vid==dequeueDesc->vid)))
		{
			curDesc->fwdPortMask &= (~dequeueDesc->fwdPortMask);
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old descriptor chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl865x_freeMCastFwdDesc(curDesc);

			}

			return SUCCESS;
		}
	}

	/*never reach here*/
	return SUCCESS;
}

#endif
#if	defined (CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
extern int rtl_vlan_support_enable ;
extern unsigned int rtl_getNicFwdPortMaskofBr0(unsigned int brFwdPortMask);
#endif
static int32 rtl865x_multicastCallbackFn(void *param)
{
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	uint32 index;
	uint32 oldDescPortMask,newDescPortMask;/*for device decriptor forwarding usage*/
	
	uint32 oldAsicFwdPortMask,newAsicFwdPortMask;/*for physical port forwarding usage*/
	uint32 oldCpuFlag,newCpuFlag;
	#if	defined (CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
	uint32 newNicFwdPortMask=0;
	uint32 oldNicFwdPortMask=0;
	#endif
	rtl_multicastEventContext_t mcastEventContext;

	rtl865x_mcast_fwd_descriptor_t newFwdDesc;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	rtl865x_tblDrv_mCast_t  *mCastEntry,*nextMCastEntry;
	struct rtl_multicastDeviceInfo_s bridgeMCastDev;

	struct rtl_groupInfo groupInfo;
	int32 retVal=FAILED;

	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	/*check device name's validity*/
	if(strlen(mcastEventContext.devName)==0)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
	diag_printf("%s:%d,mcastEventContext.devName is %s, mcastEventContext.groupAddr is 0x%x,mcastEventContext.sourceAdd is 0x%x,mcastEventContext.portMask is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName, mcastEventContext.groupAddr[0], mcastEventContext.sourceAddr[0], mcastEventContext.portMask);
	#endif
//	if(mcastEventContext.groupAddr==0)
//		return EVENT_CONTINUE_EXECUTE;
	/*case 1:this is multicast event from bridge(br0) */
	/*sync wlan and ethernet*/
	//hyking:[Fix me] the RTL_BR_NAME...
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0 || memcmp(mcastEventContext.devName,RTL_BR1_NAME,3)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
#endif
	{

		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if (/*(mcastEventContext.groupAddr!=0) && */(mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}
				#if defined(CONFIG_RTL_819X_SWCORE)
				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				#endif
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
				#if	defined (CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
				if(rtl_vlan_support_enable){
					oldNicFwdPortMask=mCastEntry->mbr;
					oldCpuFlag=mCastEntry->cpu;
				}	
				#endif
				oldDescPortMask=rtl865x_getMCastEntryDescPortMask( mCastEntry);	
				
				/*sync with control plane*/
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				#if	defined (CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
				if(rtl_vlan_support_enable)
				{
					#if NBRIDGE > 0
					newNicFwdPortMask=rtl_getNicFwdPortMaskofBr0(multicastFwdInfo.fwdPortMask);
					#endif
					/*update/replace old forward descriptor*/
					//diag_printf("newNicFwdPortMask:%x,oldNicFwdPortMask:%x,bridgeMCastDev.swPortMask:%x,[%s]:[%d].\n",newNicFwdPortMask,oldNicFwdPortMask,bridgeMCastDev.swPortMask,__FUNCTION__,__LINE__);
				
					newFwdDesc.fwdPortMask=newNicFwdPortMask;
					newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
					_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
					mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
					mCastEntry->cpu		= rtl865x_genMCastEntryCpuFlag(mCastEntry);
					newCpuFlag			=mCastEntry->cpu;
					newNicFwdPortMask = mCastEntry->mbr;
				}	
				
				#else
				if(retVal!=SUCCESS)
				{
					continue;
				}
				
				#endif
				retVal= rtl_getIgmpSnoopingModuleDevInfo(mcastEventContext.moduleIndex, &bridgeMCastDev);
				if(retVal!=SUCCESS)
				{
					continue;
				}
				
				newDescPortMask=multicastFwdInfo.fwdPortMask;
				
				//diag_printf("oldCpuFlag:%x,newCpuFlag:%x,newNicFwdPortMask:%x,oldNicFwdPortMask:%x,bridgeMCastDev.swPortMask:%x,[%s]:[%d].\n",oldCpuFlag,newCpuFlag,newNicFwdPortMask,oldNicFwdPortMask,bridgeMCastDev.swPortMask,__FUNCTION__,__LINE__);
				
				#ifndef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
				if(	(oldDescPortMask != newDescPortMask) &&
					(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
						(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
				
				{
					/*this multicast entry should be re-generated at linux protocol stack bridge level*/
					
					_rtl865x_freeMCastEntry(mCastEntry, index);
					
					_rtl865x_arrangeMulticast(index);
				}
				#else
				if(rtl_vlan_support_enable==0)
				{
					if(	(oldDescPortMask != newDescPortMask) &&
					(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
						(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
					{
						/*this multicast entry should be re-generated at linux protocol stack bridge level*/
					
						_rtl865x_freeMCastEntry(mCastEntry, index);
						
						_rtl865x_arrangeMulticast(index);
					}	
				}
				else
				{
					if((oldCpuFlag != newCpuFlag)||(newNicFwdPortMask!=oldNicFwdPortMask))
					{
					/*this multicast entry should be re-generated at linux protocol stack bridge level*/
						//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
						
						if(newNicFwdPortMask==0)
							
							_rtl865x_freeMCastEntry(mCastEntry, index);
						
						_rtl865x_arrangeMulticast(index);
					}	
				}
				#endif
			}
		}
		
		return EVENT_CONTINUE_EXECUTE;
	}
			
	/*case 2:this is multicast event from ethernet (eth0)*/
	/*update ethernet forwarding port mask*/
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
   if(memcmp(mcastEventContext.devName,"eth*",4)==0 || memcmp(mcastEventContext.devName,RTL_PS_ETH_NAME_ETH2,4)==0)
#else
	if(memcmp(mcastEventContext.devName,"eth*",4)==0)
#endif
	{
		#if 0//def CONFIG_RTL865X_MUTLICAST_DEBUG
		diag_printf("%s:%d,multicast event from ethernet (%s),mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
		#endif
		
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if (/*(mcastEventContext.groupAddr!=0) && */(mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}
				
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);

				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;
				
				/*sync with control plane*/
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);

				newFwdDesc.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<mCastEntry->port));
				newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
			
				/*update/replace old forward descriptor*/
				
				_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu		= rtl865x_genMCastEntryCpuFlag(mCastEntry);
				
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
				
				#if 0//def CONFIG_RTL865X_MUTLICAST_DEBUG
				diag_printf("%s:%d,oldAsicFwdPortMask is %d,oldCpuFlag:%d,newAsicFwdPortMask is %d,newCpuFlag:%d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,oldCpuFlag,newAsicFwdPortMask,newCpuFlag);
				#endif
				
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
						     external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
						mCastEntry->extIp= 0;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */
				#if defined(CONFIG_RTL_819X_SWCORE)
				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				#endif
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
	
				
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl865x_patchPppoeWeak(mCastEntry);
					
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{

						_rtl865x_freeMCastEntry(mCastEntry, index);
					}
					
					_rtl865x_arrangeMulticast(index);
				}
			}

				
			
		}
	}
	#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_multicastUnRegisterEvent(void)
{
#if 1
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_multicastCallbackFn;
	rtl865x_unRegisterEvent(&eventParam);
#endif
	return SUCCESS;
}

static int32 _rtl865x_multicastRegisterEvent(void)
{
#if 1
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_multicastCallbackFn;
	rtl865x_registerEvent(&eventParam);
#endif
	return SUCCESS;
}


void _rtl865x_timeUpdateMulticast(uint32 secPassed)
{

	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	uint32 entry;
	uint32 needReArrange=FALSE;
	uint32 hashLineCnt=0;
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		hashLineCnt=0;
		needReArrange=FALSE;
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == TRUE)
			{
				/* Entry is in the ASIC */
				if (mCast_t->age <= secPassed) 
				{
					if(mCast_t->mbr==0)
					{
						_rtl865x_freeMCastEntry(mCast_t, entry);
						needReArrange=TRUE;
					}
					else
					{
						mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					}
				}
				else
				{
					mCast_t->age -= secPassed;
				}
			}
			else 
			{
				//printk("------------mCast_t->count:%d,[%s]:[%d].\n",mCast_t->count,__FUNCTION__,__LINE__);
			
				//mCast_t->count=0;
			
				/* Entry is not in the ASIC */
				if (mCast_t->age <= secPassed)
				{ /* aging out */
					_rtl865x_freeMCastEntry(mCast_t, entry);
				}
				else 
				{
					mCast_t->age -= secPassed;
				}
			}
			
			/*won't count multicast entry forwarded by cpu*/
			if(mCast_t->cpu==0)
			{
				
				hashLineCnt++;
				//printk("------------hashLineCnt:%d,[%s]:[%d].\n",hashLineCnt,__FUNCTION__,__LINE__);
				if(hashLineCnt>=2)
				{
					needReArrange=TRUE;
				}
			}
		
			//mCast_t->count = 0;
			mCast_t = nextMCast_t;
		}
		
		if(needReArrange==TRUE)
		{
			//printk("------------entry:%d,hashLineCnt:%d,[%s]:[%d].\n",entry,hashLineCnt,__FUNCTION__,__LINE__);
			_rtl865x_arrangeMulticast(entry);
		}
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == FALSE)
				mCast_t->count=0;
			
			mCast_t = nextMCast_t;
		}
		
	}
}

#if defined(__ECOS)
static void _rtl865x_mCastSysTimerExpired(uint32 expireDada)
{

	_rtl865x_timeUpdateMulticast(1);
	mod_timer(&rtl865x_mCastSysTimer, jiffies+HZ);
	
}

static void _rtl865x_initMCastSysTimer(void)
{
#ifdef __KERNEL__	

	init_timer(&rtl865x_mCastSysTimer);
	rtl865x_mCastSysTimer.data=rtl865x_mCastSysTimer.expires;
	rtl865x_mCastSysTimer.expires=jiffies+HZ;
	rtl865x_mCastSysTimer.function=(void*)_rtl865x_mCastSysTimerExpired;
	add_timer(&rtl865x_mCastSysTimer);
#elif defined(__ECOS)
	init_timer(&rtl865x_mCastSysTimer);
	rtl865x_mCastSysTimer.data = (unsigned long)0;
	rtl865x_mCastSysTimer.function=(void*)_rtl865x_mCastSysTimerExpired;
	//BUG_ON(timer_pending(&igmpSysTimer));
	mod_timer(&rtl865x_mCastSysTimer, jiffies+HZ);
#endif
}

static void _rtl865x_destroyMCastSysTimer(void)
{
	 del_timer_sync(&rtl865x_mCastSysTimer);
}

#endif

/*
@func int32	| rtl865x_initMulticast	|  Init hardware ip multicast module.
@parm  rtl865x_mCastConfig_t *	| mCastConfigPtr	| Pointer of hardware multicast configuration. 
@rvalue SUCCESS	|Initialize successfully.
@rvalue FAILED	|Initialize failed.
*/
int32 rtl865x_initMulticast(rtl865x_mCastConfig_t * mCastConfigPtr)
{
	_rtl865x_multicastUnRegisterEvent();
	_rtl865x_initMCastEntryPool();
	_rtl865x_initMCastFwdDescPool();
	
	rtl865x_setMulticastExternalPortMask(0);
	if(mCastConfigPtr!=NULL)
	{
		rtl865x_setMulticastExternalPortMask(mCastConfigPtr->externalPortMask);
	}
	#if defined(__ECOS)	
	_rtl865x_initMCastSysTimer();
	#endif
	rtl8651_setAsicOperationLayer(3) ;
	rtl8651_setAsicMulticastMTU(1522); 
	rtl8651_setAsicMulticastEnable(TRUE);
	rtl865x_setAsicMulticastAging(TRUE);
	_rtl865x_multicastRegisterEvent();
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
	rtl_initMulticastv6();
#endif
	return SUCCESS;
}

/*
@func int32	| rtl865x_reinitMulticast	|  Re-init hardware ip multicast module.
@rvalue SUCCESS	|Re-initialize successfully.
@rvalue FAILED	|Re-initialize failed.
*/
int32 rtl865x_reinitMulticast(void)
{
	_rtl865x_multicastUnRegisterEvent();
	/*delete all multicast entry*/
	rtl8651_setAsicMulticastEnable(FALSE);
	rtl865x_delMulticastEntry(0);
	
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl865x_destroyMCastSysTimer();
	_rtl865x_initMCastSysTimer();
	#endif
	
	/*regfster twice won't cause any side-effect, 
	because event management module will handle duplicate event issue*/
	rtl8651_setAsicMulticastMTU(1522); 
	rtl8651_setAsicMulticastEnable(TRUE);
	rtl865x_setAsicMulticastAging(TRUE);
	_rtl865x_multicastRegisterEvent();
	return SUCCESS;
}	



#if 1
#if 0

/*
@func int32		| rtl8651_getAsicMulticastSpanningTreePortState 	| Retrieve Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | pointer to memory to store the port state
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
In RTL865xC platform, Multicast spanning tree configuration is gotten by this API.
@xref  <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState)
{
	uint32 reg;
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER || portState == NULL )
		return FAILED;

	reg = ( READ_MEM32( PCRP0 + offset ) & IPMSTP_PortST_MASK );

	switch(reg)
	{
		case IPMSTP_PortST_DISABLE:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case IPMSTP_PortST_BLOCKING:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case IPMSTP_PortST_LEARNING:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case IPMSTP_PortST_FORWARDING:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}
#else
extern int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState);

#endif
int32 rtl_dumpSwMulticastInfo(void)
{
	uint32 mCastMtu=0;
	uint32 mCastEnable=FALSE;
	uint32 index;
	int8 isInternal;
	uint32 portStatus;
	uint32 internalPortMask=0;
	uint32 externalPortMask=0;
	int32 ret=FAILED;
	
	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
    rtl819x_tblDrv_mCastv6_t *mCastv6_t, *nextMCastv6_t;
    rtl819x_mcast_fwd_descriptor6_t *curDescv6, *nextDescv6;
#endif
	uint32 entry;
	uint32 cnt;
	diag_printf("----------------------------------------------------\n");
	diag_printf("Asic Operation Layer :%d\n", rtl8651_getAsicOperationLayer());
	
	ret=rtl8651_getAsicMulticastEnable(&mCastEnable);
	if(ret==SUCCESS)
	{
		diag_printf("Asic Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		diag_printf("Read Asic Multicast Table Enable Bit Error\n");
	}
	
	ret=rtl8651_getAsicMulticastMTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		diag_printf("Asic Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		diag_printf("Read Asic Multicast MTU Error\n");
	}
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
	ret=rtl819x_getAsicMulticastv6Enable(&mCastEnable);
	if(ret==SUCCESS)
	{
		diag_printf("Asic IPV6 Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		diag_printf("Read Asic IPV6 Multicast Table Enable Bit Error\n");
	}
	ret=rtl819x_getAsicMulticastv6MTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		diag_printf("Asic IPV6 Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		diag_printf("Read Asic IPV6 Multicast MTU Error\n");
	}
#endif	
	for (index=0; index<RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER; index++)
	{
		ret=rtl8651_getAsicMulticastPortInternal(index, &isInternal);
		if(ret==SUCCESS)
		{
			if(isInternal==TRUE)
			{
				internalPortMask |= 1<<index;
			}
			else
			{
				externalPortMask |= 1<<index;
			}
		}
	
	}

	diag_printf("Internal Port Mask:0x%x\nExternal Port Mask:0x%x\n", internalPortMask,externalPortMask);
	diag_printf("----------------------------------------------------\n");
	diag_printf("Multicast STP State:\n");
	for (index=0; index<RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER; index++)
	{
		ret= rtl8651_getAsicMulticastSpanningTreePortState(index, &portStatus);
		if(ret==SUCCESS)
		{
			diag_printf("port[%d]:",index);
			if(portStatus==RTL8651_PORTSTA_DISABLED)
			{
				diag_printf("disabled\n");
			}
			else if(portStatus==RTL8651_PORTSTA_BLOCKING)
			{
				diag_printf("blocking\n");
			}
			else if(portStatus==RTL8651_PORTSTA_LEARNING)
			{
				diag_printf("learning\n");
			}
			else if(portStatus==RTL8651_PORTSTA_FORWARDING)
			{
				diag_printf("forwarding\n");
			}
		}
		
	}
	diag_printf("----------------------------------------------------\n");
	diag_printf("Software Multicast Table:\n");
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			diag_printf("\t[%2d]  dip:%d.%d.%d.%d, sip:%d.%d.%d.%d, mbr:0x%x, svid:%d, spa:%d, \n", entry,
				mCast_t->dip>>24, (mCast_t->dip&0x00ff0000)>>16, (mCast_t->dip&0x0000ff00)>>8, (mCast_t->dip&0xff), 
				mCast_t->sip>>24, (mCast_t->sip&0x00ff0000)>>16, (mCast_t->sip&0x0000ff00)>>8, (mCast_t->sip&0xff),
				mCast_t->mbr,mCast_t->svid, mCast_t->port);
			diag_printf("\t      extIP:0x%x,age:%d, cpu:%d, count:%d, inAsic:%d, (%s)\n", 
				mCast_t->extIp,mCast_t->age, mCast_t->cpu,mCast_t->count,mCast_t->inAsic,mCast_t->unKnownMCast?"UnknownMCast":"KnownMCast");
			
			cnt=0;
			curDesc=MC_LIST_FIRST(&mCast_t->fwdDescChain);
			while(curDesc)
			{
				nextDesc=MC_LIST_NEXT(curDesc, next );
				diag_printf("\t      netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDesc->netifName,curDesc->descPortMask,curDesc->toCpu,curDesc->fwdPortMask);
				curDesc = nextDesc;
			}
			
			diag_printf("\n");
			mCast_t = nextMCast_t;
		}
		
	}

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
		diag_printf("----------------------------------------------------\n");
		diag_printf("IPV6 Software Multicast Table:\n");
		for (entry=0; entry< RTL819X_MULTICAST6TBL_SIZE; entry++) {
			mCastv6_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
			while (mCastv6_t) {
				/*save the next entry first*/
				nextMCastv6_t=TAILQ_NEXT(mCastv6_t, nextMCast); 		
				diag_printf("\t[%2d]  dip:%08x:%08x:%08x:%08x\n\t	   sip:%08x:%08x:%08x:%08x\n\t		mbr:0x%x, svid:%d, spa:%d, ",
					entry,
					mCastv6_t->dip[0],mCastv6_t->dip[1],
					mCastv6_t->dip[2],mCastv6_t->dip[3],
					mCastv6_t->sip[0],mCastv6_t->sip[1],
					mCastv6_t->sip[2],mCastv6_t->sip[3],
	
					mCastv6_t->mbr,
					mCastv6_t->svid,
					mCastv6_t->port);
				
				diag_printf("age:%d, cpu:%d, count:%d, inAsic:%d, (%s)\n", 
					mCastv6_t->age, mCastv6_t->cpu,mCastv6_t->count,mCastv6_t->inAsic,mCastv6_t->unKnownMCast?"UnknownMCast":"KnownMCast");
				cnt=0;
				curDescv6=MC_LIST_FIRST(&mCastv6_t->fwdDescChain);
				while(curDescv6)
				{
					nextDescv6=MC_LIST_NEXT(curDescv6, next );
					diag_printf("\t	  netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDescv6->netifName,curDescv6->descPortMask,curDescv6->toCpu,curDescv6->fwdPortMask);
					curDescv6 = nextDescv6;
				}
					
				diag_printf("\n");
				mCastv6_t = nextMCastv6_t;
			}
				
		}
#endif

	return SUCCESS;
}
#endif
static int32 rtl865x_hw_mcast_read(void)
{
	int len=0;
	rtl865x_tblAsicDrv_multiCastParam_t asic;
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
	rtl819x_tblAsicDrv_multiCastv6Param_t asic6;
#endif
	uint32 entry;

	#if 1
	diag_printf("%s\n", "ASIC Multicast Table:");
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				#if 0 
				rtlglue_printf("\t[%d]  (INVALID)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
					asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
					asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
					asic.mbr);
				rtlglue_printf("\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
					asic.age, asic.cpu);
				#endif
				continue;
			}
			else
			{
				diag_printf("\t[%d]  (OK)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
				diag_printf("\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
			}
	}
	diag_printf("\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
	#else
	len = sprintf(page, "%s\n", "ASIC Multicast Table:");

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				len +=sprintf(page+len,"\t[%d]  (Invalid Entry)\n", entry);
				continue;
			}
			len += sprintf(page+len, "\t[%d]  dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			len +=sprintf(page+len,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
	}
	#endif
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
	diag_printf("----------------------------------------------------\n");
	diag_printf("%s\n", "ASIC IPV6 Multicast Table:");
	for(entry=0; entry<RTL819X_MULTICAST6TBL_SIZE; entry++)
	{
		if(rtl819x_getAsicIpMulticastv6Table(entry,&asic6) != SUCCESS){
			continue;
		}
		else
		{	
			diag_printf("\t[%d]  (OK)dip:%08x:%08x:%08x:%08x\n",
				entry,
				asic6.dip.v6_addr32[0],asic6.dip.v6_addr32[1],
				asic6.dip.v6_addr32[2],asic6.dip.v6_addr32[3]);
			diag_printf("\t		  sip:%08x:%08x:%08x:%08x\n",
				asic6.sip.v6_addr32[0],asic6.sip.v6_addr32[1],
				asic6.sip.v6_addr32[2],asic6.sip.v6_addr32[3]);
					
			diag_printf("\t		  spa:%d, age:%d, cpu:%d, mbr(0x%x)\n", asic6.port, asic6.age, asic6.cpu, asic6.mbr);
		}
	}
	diag_printf("\n\t TotalOpCntv6:AddMcastOpCntv6:%d\tDelMcastOpCntv6:%d\tForceAddMcastOpCntv6:%d\t \n", _rtl819x_getAddMcastv6OpCnt(),_rtl819x_getDelMcastv6OpCnt(),_rtl819x_getForceAddMcastv6OpCnt());
#endif
	
	return len;
}

void dump_multicast_table(void)
{
	rtl_dumpSwMulticastInfo();
	rtl865x_hw_mcast_read();
}

int rtl865x_genVirtualMCastFwdDescriptor(unsigned int forceToCpu, uint32 fwdPortMask, rtl865x_mcast_fwd_descriptor_t *fwdDescriptor)
{
	
	if(fwdDescriptor==NULL)
	{
		return FAILED;
	}
	
	memset(fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	fwdDescriptor->toCpu=forceToCpu;
	fwdDescriptor->fwdPortMask=fwdPortMask;
	return SUCCESS;

}

int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr)
{
	rtl865x_mcast_fwd_descriptor_t fwdDescriptor;
	rtl865x_tblDrv_mCast_t * existMCastEntry=NULL;
	existMCastEntry=rtl865x_findMCastEntry(destIpAddr, srcIpAddr, (uint16)srcVlanId, (uint16)srcPort);
	if(existMCastEntry!=NULL)
	{
		if(existMCastEntry->mbr==0)
		{
			return SUCCESS;
		}
	}
	memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	rtl865x_addMulticastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort, &fwdDescriptor, TRUE, 0, 0, 0);
	return SUCCESS;
}

/*
@func int32	| rtl865x_flushHWMulticastEntry	|  API to delete all multicast 
forwarding entry
@rvalue SUCCESS	|Delete hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Delete hardware multicast forwarding entry failed.
*/

int rtl865x_flushHWMulticastEntry(void)
{

	rtl865x_tblDrv_mCast_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		
		mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if(mCastEntry->dip)
			{
				
				_rtl865x_freeMCastEntry(mCastEntry, entry);
				_rtl865x_arrangeMulticast(entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		


		
	}

	return SUCCESS;
}

int rtl865x_getMCastHashMethod(unsigned int *hashMethod)
{
	if(hashMethod==NULL)
	{
		return -1;
	}

	#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD)    
	*hashMethod = (REG32(FFCR) & 0x60)>>5;
	#else
	*hashMethod=0;
	#endif
	return 0;
}

int rtl865x_setMCastHashMethod(unsigned int hashMethod)
{
    #if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD)    
    uint32 	  oldHashMethod = 0;
	uint32 	  currHashMethod = 0;
 	oldHashMethod = (REG32(FFCR) & 0x60)>>5;
    currHashMethod = hashMethod; 
	if(currHashMethod >3)
	{
		return -1;
	}
				
	currHashMethod=currHashMethod&0x3;
				
    if (oldHashMethod != currHashMethod) /* set FFCR Register bit 5~6 and flush multicast table */
    {
        REG32(FFCR) = (REG32(FFCR) & 0xFFFFFF9F)|(currHashMethod << 5);
        /* exclude 0->1 and 1->0 */
        if(!((currHashMethod == HASH_METHOD_SIP_DIP0 && oldHashMethod == HASH_METHOD_SIP_DIP1) || 
            (currHashMethod == HASH_METHOD_SIP_DIP1 && oldHashMethod == HASH_METHOD_SIP_DIP0)))
        {

            rtl865x_flushHWMulticastEntry();   
                   

        }
            
    }
    return 0;
#else
	return 0;
#endif	
}
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_MLD_SNOOPING)
static int32 _rtl819x_initMCastv6FwdDescPool(void)
{
	int32 i;


	MC_LIST_INIT(&free_mcast_fwd_descriptor_head6);

	TBL_MEM_ALLOC(rtl819x_mcastFwdDescPool6, rtl819x_mcast_fwd_descriptor6_t,MAX_MCASTV6_FWD_DESCRIPTOR_CNT);
	
	if(rtl819x_mcastFwdDescPool6!=NULL)
	{
		memset( rtl819x_mcastFwdDescPool6, 0, MAX_MCASTV6_FWD_DESCRIPTOR_CNT * sizeof(rtl819x_mcast_fwd_descriptor6_t));	
	}
	else
	{
		return FAILED;
	}
	
	for(i = 0; i<MAX_MCASTV6_FWD_DESCRIPTOR_CNT;i++)
	{
		MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head6, &rtl819x_mcastFwdDescPool6[i], next);
	}
	
	return SUCCESS;
}
static rtl819x_mcast_fwd_descriptor6_t *_rtl819x_allocMCastv6FwdDesc(void)
{
	rtl819x_mcast_fwd_descriptor6_t *retDesc=NULL;
	retDesc = MC_LIST_FIRST(&free_mcast_fwd_descriptor_head6);
	if(retDesc!=NULL)
	{
		MC_LIST_REMOVE(retDesc, next);
		memset(retDesc,0,sizeof(rtl819x_mcast_fwd_descriptor6_t));
	}
	return retDesc;
}
static int32 _rtl819x_freeMCastv6FwdDesc(rtl819x_mcast_fwd_descriptor6_t *descPtr)
{
	if(descPtr==NULL)
	{
		return SUCCESS;
	}
	memset(descPtr,0,sizeof(rtl819x_mcast_fwd_descriptor6_t));
	MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head6, descPtr, next);
	
	return SUCCESS;
}
static int32 _rtl819x_flushMCastv6FwdDescChain(mcast_fwd_descriptor_head6_t * descChainHead)
{
	rtl819x_mcast_fwd_descriptor6_t * curDesc,*nextDesc;
	
	if(descChainHead==NULL)
	{
		return SUCCESS;
	}
	
	curDesc=MC_LIST_FIRST(descChainHead);
	while(curDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		/*remove from the old descriptor chain*/
		MC_LIST_REMOVE(curDesc, next);
		/*return to the free descriptor chain*/
		_rtl819x_freeMCastv6FwdDesc(curDesc);
		curDesc = nextDesc;
	}

	return SUCCESS;
}
static int32 _rtl819x_mCastv6FwdDescEnqueue(mcast_fwd_descriptor_head6_t * queueHead,
												rtl819x_mcast_fwd_descriptor6_t * enqueueDesc)
{
	rtl819x_mcast_fwd_descriptor6_t *newDesc;
	rtl819x_mcast_fwd_descriptor6_t *curDesc,*nextDesc;
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(enqueueDesc==NULL)
	{
		return SUCCESS;
	}
	
	/*multicast forward descriptor is internal maintained,always alloc new one*/
	newDesc=_rtl819x_allocMCastv6FwdDesc();
	
	if(newDesc!=NULL)
	{
		memcpy(newDesc, enqueueDesc,sizeof(rtl819x_mcast_fwd_descriptor6_t));
		memset(&(newDesc->next), 0, sizeof(MC_LIST_ENTRY(rtl819x_mcast_fwd_descriptor6_t)));
		newDesc->next.le_next=NULL;
		newDesc->next.le_prev=NULL;
	}
	else
	{
		/*no enough memory*/
		return FAILED;
	}
	

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{

		nextDesc=MC_LIST_NEXT(curDesc, next);
		
		/*merge two descriptor*/
		if((strcmp(curDesc->netifName,newDesc->netifName)==0) && (curDesc->vid==newDesc->vid))
		if(strcmp(curDesc->netifName,newDesc->netifName)==0)
		{	
			if(newDesc->descPortMask==0)
			{
				newDesc->descPortMask=curDesc->descPortMask;
			}
			MC_LIST_REMOVE(curDesc, next);
			_rtl819x_freeMCastv6FwdDesc(curDesc);
			
		}
	}

	/*not matched descriptor is found*/
	MC_LIST_INSERT_HEAD(queueHead, newDesc, next);

	return SUCCESS;
	
}
static int32 _rtl819x_mergeMCastv6FwdDescChain(mcast_fwd_descriptor_head6_t * targetChainHead ,
													rtl819x_mcast_fwd_descriptor6_t *srcChain)
{
	rtl819x_mcast_fwd_descriptor6_t *curDesc;

	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		
		_rtl819x_mCastv6FwdDescEnqueue(targetChainHead, curDesc);
		
	}
	
	return SUCCESS;
}
static int32 _rtl819x_initMCastv6EntryPool(void)
{
	int32 index;
	rtl819x_tblDrv_mCastv6_t *multiCast_t;
	
	TBL_MEM_ALLOC(multiCast_t, rtl819x_tblDrv_mCastv6_t ,MAX_MCASTV6_TABLE_ENTRY_CNT);
	TAILQ_INIT(&mCastTbl6.freeList.freeMultiCast);
	for(index=0; index<MAX_MCASTV6_TABLE_ENTRY_CNT; index++)
	{
		memset( &multiCast_t[index], 0, sizeof(rtl819x_tblDrv_mCastv6_t));
		TAILQ_INSERT_HEAD(&mCastTbl6.freeList.freeMultiCast, &multiCast_t[index], nextMCast);
	}

	TBL_MEM_ALLOC(multiCast_t, rtl819x_tblDrv_mCastv6_t, RTL819X_MULTICAST6TBL_SIZE);
	memset(multiCast_t, 0,RTL819X_MULTICAST6TBL_SIZE* sizeof(rtl819x_tblDrv_mCastv6_t));
	mCastTbl6.inuseList.mCastTbl = (void *)multiCast_t;

	for(index=0; index<RTL819X_MULTICAST6TBL_SIZE; index++)
	{
		TAILQ_INIT(&mCastTbl6.inuseList.mCastTbl[index]);
	}
	return SUCCESS;
}
static rtl819x_tblDrv_mCastv6_t * _rtl819x_allocMCastv6Entry(uint32 hashIndex)
{
	rtl819x_tblDrv_mCastv6_t *newEntry;
	newEntry=TAILQ_FIRST(&mCastTbl6.freeList.freeMultiCast);
	if (newEntry == NULL)
	{
		return NULL;
	}		
	
	TAILQ_REMOVE(&mCastTbl6.freeList.freeMultiCast, newEntry, nextMCast);

	
	/*initialize it*/
	if(MC_LIST_FIRST(&newEntry->fwdDescChain)!=NULL)
	{
		_rtl819x_flushMCastv6FwdDescChain(&newEntry->fwdDescChain);
	}
	MC_LIST_INIT(&newEntry->fwdDescChain);
	
	memset(newEntry, 0, sizeof(rtl819x_tblDrv_mCastv6_t));

	TAILQ_INSERT_TAIL(&mCastTbl6.inuseList.mCastTbl[hashIndex], newEntry, nextMCast);
	
	return newEntry;
}
static int32 _rtl819x_flushMCastv6Entry(rtl819x_tblDrv_mCastv6_t *mCastEntry)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	_rtl819x_flushMCastv6FwdDescChain(&mCastEntry->fwdDescChain);
	
	memset(mCastEntry, 0, sizeof(rtl819x_tblDrv_mCastv6_t));
	return SUCCESS;
}
static int32 _rtl819x_freeMCastv6Entry(rtl819x_tblDrv_mCastv6_t * mCastEntry, uint32 hashIndex)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	TAILQ_REMOVE(&mCastTbl6.inuseList.mCastTbl[hashIndex], mCastEntry, nextMCast);
	_rtl819x_flushMCastv6Entry(mCastEntry);
	TAILQ_INSERT_HEAD(&mCastTbl6.freeList.freeMultiCast, mCastEntry, nextMCast);
	return SUCCESS;
}
static uint32 _rtl819x_doMCastv6EntrySrcVlanPortFilter(rtl819x_tblDrv_mCastv6_t *mCastEntry)
{
	rtl819x_mcast_fwd_descriptor6_t * curDesc,*nextDesc;
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	for(curDesc=MC_LIST_FIRST(&mCastEntry->fwdDescChain);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next);
		{
			curDesc->fwdPortMask=curDesc->fwdPortMask & (~(1<<mCastEntry->port));
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl819x_freeMCastv6FwdDesc(curDesc);

			}
		}
		
	}

	return SUCCESS;
}
static uint32 rtl819x_genMCastv6EntryAsicFwdMask(rtl819x_tblDrv_mCastv6_t *mCastEntry)
{
	uint32 asicFwdPortMask=0;
	rtl819x_mcast_fwd_descriptor6_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(curDesc->toCpu==0)
		{
			asicFwdPortMask|=(curDesc->fwdPortMask & ((1<<RTL8651_MAC_NUMBER)-1));
		}
		else
		{
			asicFwdPortMask|=( 0x01<<RTL8651_MAC_NUMBER);
		}
	}
	asicFwdPortMask = asicFwdPortMask & (~(1<<mCastEntry->port)); 
	return asicFwdPortMask;
}
static uint16 rtl819x_genMCastv6EntryCpuFlag(rtl819x_tblDrv_mCastv6_t *mCastEntry)
{
	uint16 cpuFlag=FALSE;
	rtl819x_mcast_fwd_descriptor6_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}

	if(mCastEntry->cpuHold==TRUE)
	{
		cpuFlag=TRUE;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(	(curDesc->toCpu==TRUE)	||
			(memcmp(curDesc->netifName, RTL_WLAN_NAME,4)==0)	)
		{
			cpuFlag=TRUE;
		}
	}
	
	return cpuFlag;
}
static void  _rtl819x_setASICMulticastv6PortStatus(void) {
	uint32 index;
	for (index=0; index<RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER; index++) {
		rtl8651_setAsicMulticastPortInternal(index, (rtl865x_externalMulticastPortMask&(1<<index))?FALSE:TRUE);
	}
}
int32 rtl819x_addMulticastv6ExternalPort(uint32 extPort)
{
	rtl819x_externalMulticastPortMask6 |= (1<<extPort);
	_rtl819x_setASICMulticastv6PortStatus();
	return SUCCESS;
}
int32 rtl819x_delMulticastv6ExternalPort(uint32 extPort)
{
	rtl819x_externalMulticastPortMask6 &= ~(1<<extPort);
	_rtl819x_setASICMulticastv6PortStatus();
	return SUCCESS;
}
int32 rtl819x_setMulticastv6ExternalPortMask(uint32 extPortMask)
{
	rtl819x_externalMulticastPortMask6 =extPortMask;
	_rtl819x_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl819x_addMulticastv6ExternalPortMask(uint32 extPortMask)
{
	rtl819x_externalMulticastPortMask6|= extPortMask;
	_rtl819x_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl819x_delMulticastv6ExternalPortMask(uint32 extPortMask)
{
	rtl819x_externalMulticastPortMask6 &= ~extPortMask;
	_rtl819x_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl819x_getMulticastv6ExternalPortMask(void)
{
	return rtl819x_externalMulticastPortMask6 ;
}
static inline void _rtl819x_patchPppoeWeakv6(rtl819x_tblDrv_mCastv6_t *mCast_t)
{
	rtl819x_mcast_fwd_descriptor6_t * curDesc;
	uint32 netifType = 0;
	/* patch: keep cache in software if one vlan's interface is pppoe */
	MC_LIST_FOREACH(curDesc, &(mCast_t->fwdDescChain), next)
	{
		//if(rtl865x_getNetifType(curDesc->netifName, &netifType)==SUCCESS)
		{
			/*how about pptp,l2tp?*/
			if(netifType==IF_PPPOE)
			{
				mCast_t->flag |= RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
				return;
			}
		}
		
	}

	mCast_t->flag &= ~RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
}
uint32 rtl819x_getMCastv6EntryDescPortMask(rtl819x_tblDrv_mCastv6_t *mCastEntry)
{
	uint32 descPortMask=0;
	rtl819x_mcast_fwd_descriptor6_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		descPortMask=descPortMask | curDesc->descPortMask;
	}
	
	return descPortMask;
}

static unsigned int mcastAddOpCnt6=0;
static unsigned int mcastDelOpCnt6=0;
static unsigned int mcastForceAddOpCnt6=0;
unsigned int _rtl819x_getAddMcastv6OpCnt(void)
{
	return mcastAddOpCnt6;
}

unsigned int _rtl819x_getDelMcastv6OpCnt(void)
{
	return mcastDelOpCnt6;
}

unsigned int _rtl819x_getForceAddMcastv6OpCnt(void)
{
	return mcastForceAddOpCnt6;
}
static int is_ip6_addr_equal(unsigned int *addr1, unsigned int *addr2)
{
	if(addr1[0]==addr2[0]&&
	   addr1[1]==addr2[1]&&
	   addr1[2]==addr2[2]&&
	   addr1[3]==addr2[3])
	   return 1;
	else
	   return 0;
}
int32 rtl819x_flushHWMulticastv6Entry(void)
{
	rtl819x_tblDrv_mCastv6_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	inv6_addr_t ip0;
	memset(&ip0,0,sizeof(inv6_addr_t));
	for(entry=0; entry<RTL819X_MULTICAST6TBL_SIZE; entry++) 
	{
		mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if(!is_ip6_addr_equal(ip0.v6_addr32,mCastEntry->dip))
			{
				_rtl819x_freeMCastv6Entry(mCastEntry, entry);
				_rtl819x_arrangeMulticastv6(entry);
			}
			mCastEntry = nextMCastEntry;
		}	
	}
	return SUCCESS;
}
int rtl819x_getMCastv6HashMethod(unsigned int *hashMethod)
{
	return rtl819x_getAsicMCastv6HashMethod(hashMethod);
}
int rtl819x_setMCastv6HashMethod(unsigned int hashMethod)
{
	uint32	  oldHashMethod = 0;
	rtl819x_getAsicMCastv6HashMethod(&oldHashMethod);
	if(hashMethod >3)
	{
		return -1;
	}
	hashMethod &= 3;
					
	if (oldHashMethod != hashMethod) /* set IPV6CR Register bit 17~18 and flush multicastv6 table */
	{
		rtl819x_setAsicMCastv6HashMethod(hashMethod);
		/* exclude 0->1 and 1->0 */
		if(!((hashMethod == HASH_METHOD_SIP_DIP0 && oldHashMethod == HASH_METHOD_SIP_DIP1) || 
			(hashMethod == HASH_METHOD_SIP_DIP1 && oldHashMethod == HASH_METHOD_SIP_DIP0)))
		{
			rtl819x_flushHWMulticastv6Entry();   	
		}
				
	}
	return 0;
}
static void _rtl819x_arrangeMulticastv6(uint32 entryIndex)
{
	rtl819x_tblAsicDrv_multiCastv6Param_t asic_mcast;
	rtl819x_tblDrv_mCastv6_t *mCast_t=NULL;
	rtl819x_tblDrv_mCastv6_t *select_t=NULL;
	rtl819x_tblDrv_mCastv6_t *swapOutEntry=NULL;
	int32 retval;
	int32 hashMethod=0;
	rtl819x_getMCastv6HashMethod(&hashMethod);
	TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[entryIndex], nextMCast) 
	{
		if ((mCast_t->cpu == 0) && !(mCast_t->flag & RTL865X_MULTICAST_PPPOEPATCH_CPUBIT)) 
		{ /* Ignore cpu=1 */

			if(mCast_t->inAsic==TRUE)
			{
				if(swapOutEntry==NULL)
				{
					swapOutEntry=mCast_t;
				}
				else
				{
					/*impossible, two flow in one asic entry*/
					swapOutEntry->inAsic=FALSE;
					mCast_t->inAsic = FALSE;
				}
			}
		
			if (select_t) 
			{
				if ((mCast_t->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
				{
					/*replace unknown multicast*/
					select_t = mCast_t;
				}
				else
				{
					/*select the heavy load*/
					if ((mCast_t->count) > (select_t->count))
					{
						select_t = mCast_t;
					}
				}
				
			}
			else 
			{
				select_t = mCast_t;
			}
			
			
		}
		else
		{
			mCast_t->inAsic = FALSE;	/* reset "inAsic" bit */
		} 
	}
	
	if(select_t && swapOutEntry)
	{
		if ((swapOutEntry->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
		{
			/*replace unknown multicast*/
			select_t = swapOutEntry;
		}
		else
		{
			if((select_t->count <= (swapOutEntry->count+RTL865X_HW_MCAST_SWAP_GAP)))
				select_t = swapOutEntry;
		}
	}	
	
	if (select_t) 
	{
		if((swapOutEntry==NULL) ||(select_t==swapOutEntry))
		{
			select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
			bzero(&asic_mcast, sizeof(rtl819x_tblAsicDrv_multiCastv6Param_t));
//			memcpy(&asic_mcast, select_t, (uint32)&(((rtl819x_tblDrv_mCastv6_t *)0)->six_rd_idx));
			memcpy(&asic_mcast, select_t, sizeof(rtl819x_tblAsicDrv_multiCastv6Param_t));
#if defined(CONFIG_RTL_819X_SWCORE)			
			retval = rtl819x_setAsicIpMulticastv6Table(hashMethod,&asic_mcast);
#endif
			
#if 1//def CONFIG_PROC_FS
			mcastAddOpCnt6++;
#endif
			ASSERT_CSP(retval == SUCCESS);
			if(retval==SUCCESS)
			{
				select_t->inAsic = TRUE;
			}
			else
			{
				select_t->inAsic = FALSE;
				#if defined(CONFIG_RTL_819X_SWCORE)
				rtl819x_delAsicIpMulticastv6Table(entryIndex);
				#endif
#if 1//def CONFIG_PROC_FS
				mcastDelOpCnt6++;
#endif
			}
				
			ASSERT_CSP(retval == SUCCESS);
			TAILQ_REMOVE(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
			TAILQ_INSERT_HEAD(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
		}
		else/*(swapOutEntry!=NULL) && (select_t!=swapOutEntry)*/
		{
			
			/*disable swap and only explicit joined mulicast flow can replace unknown multicast flow*/
			if(1)
			{
				/*don't forget to set swapOutEntry's inAsic flag*/
				swapOutEntry->inAsic=FALSE;
				
				select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
				bzero(&asic_mcast, sizeof(rtl819x_tblAsicDrv_multiCastv6Param_t));
				//memcpy(&asic_mcast, select_t, (uint32)&(((rtl819x_tblAsicDrv_multiCastv6Param_t *)0)->six_rd_idx));
				memcpy(&asic_mcast, select_t, sizeof(rtl819x_tblAsicDrv_multiCastv6Param_t));
#if defined(CONFIG_RTL_819X_SWCORE)
				retval = rtl819x_setAsicIpMulticastv6Table(hashMethod,&asic_mcast);
#endif
				//printk("asic_mcast:dip:%x,[%s]:[%d].\n",asic_mcast.dip,__FUNCTION__,__LINE__);
#if 1//def CONFIG_PROC_FS
				mcastAddOpCnt6++;
				
#endif
				ASSERT_CSP(retval == SUCCESS);
				if(retval==SUCCESS)
				{
					select_t->inAsic = TRUE;
				}
				else
				{
					select_t->inAsic = FALSE;		
#if defined(CONFIG_RTL_819X_SWCORE)
					rtl819x_delAsicIpMulticastv6Table(entryIndex);
#endif
#if 1//def CONFIG_PROC_FS
					mcastDelOpCnt6++;
#endif
				}
				
				TAILQ_REMOVE(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
				TAILQ_INSERT_HEAD(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);

			}
			#if 0
			else
			{	
				//printk("swapOutEntry:%d,select:%d,[%s]:[%d].\n",swapOutEntry->unKnownMCast,select_t->unKnownMCast,__FUNCTION__,__LINE__);
				if(swapOutEntry->inAsic == FALSE)
				{
					/*maybe something is wrong, we remove the asic entry*/
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt6++;
#endif
				}
				
			}	
			#endif
			
		}
		
	}
	else 	
	{
		if(swapOutEntry!=NULL)
		{
			swapOutEntry->inAsic=FALSE;
		}
		
#if defined(CONFIG_RTL_819X_SWCORE)
		rtl819x_delAsicIpMulticastv6Table(entryIndex);
#endif
#if 1//def CONFIG_PROC_FS
		mcastDelOpCnt6++;
#endif
	}
}
static void _rtl819x_mCastv6EntryReclaim(void)
{
	uint32 index;
	uint32 freeCnt=0;
	uint32 asicFwdPortMask=0;
	uint32 needReArrange=FALSE;
	rtl819x_tblDrv_mCastv6_t *curMCastEntry, *nextMCastEntry;

	/*free unused software forward entry*/
	for(index=0; index<RTL819X_MULTICAST6TBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]);
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if((curMCastEntry->inAsic==FALSE)  && (curMCastEntry->count==0))
			{
				_rtl819x_freeMCastv6Entry(curMCastEntry, index);
				freeCnt++;
			}
			curMCastEntry = nextMCastEntry;
		}
		
	}

	if(freeCnt>0)
	{
		return;
	}
	
	for(index=0; index<RTL819X_MULTICAST6TBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]);
		needReArrange=FALSE;
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if(curMCastEntry->inAsic)
			{
				asicFwdPortMask=rtl819x_genMCastv6EntryAsicFwdMask(curMCastEntry);
				if(asicFwdPortMask==0) 
				{
					_rtl819x_freeMCastv6Entry(curMCastEntry,index);
					needReArrange=TRUE;
				}
			}
			curMCastEntry = nextMCastEntry;
		}
		
		if(needReArrange==TRUE)
		{
			_rtl819x_arrangeMulticastv6(index);
		}
	}

	return;
}
rtl819x_tblDrv_mCastv6_t *rtl819x_findMCastv6Entry(unsigned int *dip, unsigned int *sip, uint16 svid, uint16 sport)
{
	rtl819x_tblDrv_mCastv6_t *mCast_t;
	uint32 entry=0;
	uint32 hashMethod=0;
	inv6_addr_t srcAddr, dstAddr;
	
	memcpy(srcAddr.v6_addr32, sip, 4*sizeof(unsigned int));
	memcpy(dstAddr.v6_addr32, dip, 4*sizeof(unsigned int));
	
	rtl819x_getMCastv6HashMethod(&hashMethod);
	entry = rtl819x_ipMulticastv6TableIndex(hashMethod,srcAddr,dstAddr);
	
	TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[entry], nextMCast) {
		if (is_ip6_addr_equal(mCast_t->dip,dip) && is_ip6_addr_equal(mCast_t->sip,sip) && mCast_t->port==sport)
		{
			if (mCast_t->inAsic == FALSE) 
			{
				mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
				mCast_t->count ++;
			}
			return mCast_t;
		}
	}

#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	for(entry=RTL8651_IPMULTICASTTBL_SIZE; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[entry], nextMCast) {
			if (is_ip6_addr_equal(mCast_t->dip,dip) && is_ip6_addr_equal(mCast_t->sip,sip) && mCast_t->port==sport)
			{
				if (mCast_t->inAsic == FALSE) 
				{
					mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					mCast_t->count ++;
				}
				return mCast_t;
			}
					
		}
	}
#endif	
	return (rtl819x_tblDrv_mCastv6_t *)NULL;	
}
int rtl819x_addMulticastv6Entry(unsigned int *dip,unsigned int *sip, unsigned short svid, unsigned short sport, 
									rtl819x_mcast_fwd_descriptor6_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag)
{
	rtl819x_tblDrv_mCastv6_t *mCast_t;
	uint32 hashIndex;
	#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	uint32 emptyCamIndex=-1; 
	#endif
	uint32 hashMethod=0;
	struct rtl_groupInfo groupInfo;
	inv6_addr_t srcAddr, dstAddr;
	
	memcpy(srcAddr.v6_addr32, sip, 4*sizeof(unsigned int));
	memcpy(dstAddr.v6_addr32, dip, 4*sizeof(unsigned int));
	
	rtl819x_getMCastv6HashMethod(&hashMethod);
	hashIndex = rtl819x_ipMulticastv6TableIndex(hashMethod,srcAddr,dstAddr);

#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t=rtl8198C_findMCastv6Entry(dip, sip, svid, sport);
	if(mCast_t==NULL)
	{
		/*table entry collided*/
		if(!TAILQ_EMPTY(&mCastTbl6.inuseList.mCastTbl[hashIndex]))
		{
			emptyCamIndex=rtl8198C_findEmptyCamEntryv6();
			if(emptyCamIndex!=-1)
			{
				hashIndex=emptyCamIndex;
			}
		}
	}
	else
	{
		hashIndex=mCast_t->hashIndex;
	}
#else
	/*try to match hash line*/
	TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[hashIndex], nextMCast) 
	{
		if (is_ip6_addr_equal(mCast_t->sip,sip) && is_ip6_addr_equal(mCast_t->dip,dip) && mCast_t->port==sport)
			break;
	}
#endif	
	
	if (mCast_t == NULL) 
	{
		mCast_t=_rtl819x_allocMCastv6Entry(hashIndex);
		if (mCast_t == NULL)
		{
			_rtl819x_mCastv6EntryReclaim();
			mCast_t=_rtl819x_allocMCastv6Entry(hashIndex);
			if(mCast_t == NULL)
			{
				return FAILED;
			}
		}
		
		memcpy(mCast_t->sip, sip, sizeof(unsigned int)*4);
		memcpy(mCast_t->dip, dip, sizeof(unsigned int)*4);

		mCast_t->port		= sport;
		mCast_t->mbr		= 0;
		mCast_t->count		= 0;
		mCast_t->inAsic		= FALSE;
		mCast_t->six_rd_eg  = 0;
		mCast_t->six_rd_idx = 0;
		mCast_t->svid		= svid;	
	}
	
	if(flushOldChain)
	{
		_rtl819x_flushMCastv6FwdDescChain(&mCast_t->fwdDescChain);
		
	}
	
	_rtl819x_mergeMCastv6FwdDescChain(&mCast_t->fwdDescChain,newFwdDescChain);
	_rtl819x_doMCastv6EntrySrcVlanPortFilter(mCast_t);
	
	mCast_t->mbr			= rtl819x_genMCastv6EntryAsicFwdMask(mCast_t);

	mCast_t->age			= RTL865X_MULTICAST_TABLE_AGE;
#if 0
	mCast_t->cpu			= (toCpu==TRUE? 1: 0);
#else
	mCast_t->cpuHold			= cpuHold;
	mCast_t->cpu 			= rtl819x_genMCastv6EntryCpuFlag(mCast_t);
#endif	
	mCast_t->flag			= flag;

	
	if (extIp)
		mCast_t->flag |= RTL865X_MULTICAST_EXTIP_SET;
	else
		mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;

	rtl_getGroupInfov6(dip, &groupInfo);
	if(groupInfo.ownerMask==0)
	{
		mCast_t->unKnownMCast=TRUE;
	}
	else
	{
		mCast_t->unKnownMCast=FALSE;
	}

#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t->hashIndex=hashIndex;
#endif
	_rtl819x_patchPppoeWeakv6(mCast_t);
	_rtl819x_arrangeMulticastv6(hashIndex);
	return SUCCESS;	
}
int rtl819x_delMulticastv6Entry(unsigned int *groupAddr)
{
	rtl819x_tblDrv_mCastv6_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	uint32 deleteFlag=FALSE;
	for(entry=0; entry<RTL819X_MULTICAST6TBL_SIZE; entry++) 
	{
		deleteFlag=FALSE;
		mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if (is_ip6_addr_equal(groupAddr,mCastEntry->dip)) 
			{
				deleteFlag=TRUE;
				_rtl819x_freeMCastv6Entry(mCastEntry, entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		
		if(deleteFlag==TRUE)
		{
			_rtl819x_arrangeMulticastv6(entry);
		}
	}
	return SUCCESS;
}
static int32 rtl819x_multicastv6CallbackFn(void *param)
{
	uint32 index;
	uint32 oldDescPortMask;
	uint32 newDescPortMask;/*for device decriptor forwarding usage*/
	uint32 oldAsicFwdPortMask,newAsicFwdPortMask;/*for physical port forwarding usage*/
	uint32 oldCpuFlag = 0,newCpuFlag = 0;
		
	rtl_multicastEventContext_t mcastEventContext;
	
	rtl819x_mcast_fwd_descriptor6_t newFwdDesc;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	rtl819x_tblDrv_mCastv6_t *mCastEntry,*nextMCastEntry;
	struct rtl_multicastDeviceInfo_s bridgeMCastDev;
	struct rtl_groupInfo groupInfo;

	int32 retVal=FAILED;
	
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	/*check device name's validity*/
	if(strlen(mcastEventContext.devName)==0)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
	diag_printf("%s:%d,mcastEventContext.devName is %s, mcastEventContext.groupAddr is 0x%x,mcastEventContext.sourceIP is 0x%x,mcastEventContext.portMask is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName, mcastEventContext.groupAddr[0], mcastEventContext.sourceAddr[0], mcastEventContext.portMask);
	#endif

	/*case 1:this is multicast event from bridge(br0) */
	/*sync wlan and ethernet*/
	//hyking:[Fix me] the RTL_BR_NAME...

	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
	{
		for (index=0; index< RTL819X_MULTICAST6TBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				if (!is_ip6_addr_equal(mcastEventContext.groupAddr,mCastEntry->dip))
				{
					continue;
				}
	
				rtl_getGroupInfov6(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
	
				oldDescPortMask=rtl819x_getMCastv6EntryDescPortMask( mCastEntry); 
					
				/*sync with control plane*/
				memset(&newFwdDesc, 0 ,sizeof(rtl819x_mcast_fwd_descriptor6_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
				multicastDataInfo.ipVersion=6;
				memcpy(multicastDataInfo.sourceIp,mCastEntry->sip,sizeof(unsigned int)*4);
				memcpy(multicastDataInfo.groupAddr,mCastEntry->dip,sizeof(unsigned int)*4);
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
					
	
				if(retVal!=SUCCESS)
				{
					continue;
				}
					
				retVal= rtl_getIgmpSnoopingModuleDevInfo(mcastEventContext.moduleIndex, &bridgeMCastDev);
				if(retVal!=SUCCESS)
				{
					continue;
				}
				newDescPortMask=multicastFwdInfo.fwdPortMask;
				if( (oldDescPortMask != newDescPortMask) &&
					(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
						(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
				{
					/*this multicast entry should be re-generated at linux protocol stack bridge level*/
					_rtl819x_freeMCastv6Entry(mCastEntry,index);
					_rtl819x_arrangeMulticastv6(index);
				}
					
			}
		}
			
		return EVENT_CONTINUE_EXECUTE;
	}

	/*case 2:this is multicast event from ethernet (eth0)*/
	/*update ethernet forwarding port mask*/

	if(memcmp(mcastEventContext.devName,"eth*",4)==0)
	{
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
		diag_printf("%s:%d,multicast event from ethernet (%s),mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
	#endif
			
		for (index=0; index< RTL819X_MULTICAST6TBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
#if	0				
				printk("mCastEntry->dip:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
					mCastEntry->dip.v6_addr16[0],mCastEntry->dip.v6_addr16[1],
					mCastEntry->dip.v6_addr16[2],mCastEntry->dip.v6_addr16[3],
					mCastEntry->dip.v6_addr16[4],mCastEntry->dip.v6_addr16[5],
					mCastEntry->dip.v6_addr16[6],mCastEntry->dip.v6_addr16[7]);
				printk("groupAddr:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
					groupAddr.v6_addr16[0],groupAddr.v6_addr16[1],
					groupAddr.v6_addr16[2],groupAddr.v6_addr16[3],
					groupAddr.v6_addr16[4],groupAddr.v6_addr16[5],
					groupAddr.v6_addr16[6],groupAddr.v6_addr16[7]);
#endif
				if (!is_ip6_addr_equal(mCastEntry->dip,mcastEventContext.groupAddr))
				{
					continue;
				}
					
				memset(&newFwdDesc, 0 ,sizeof(rtl819x_mcast_fwd_descriptor6_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
	
				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;
	
				/*sync with control plane*/
				multicastDataInfo.ipVersion=6;
				memcpy(multicastDataInfo.sourceIp,mCastEntry->sip,sizeof(unsigned int)*4);
				memcpy(multicastDataInfo.groupAddr,mCastEntry->dip,sizeof(unsigned int)*4);
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				if (multicastFwdInfo.unknownMCast == TRUE)
					multicastFwdInfo.cpuFlag = TRUE;

				newFwdDesc.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<mCastEntry->port));
				newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
				
				
				/*update/replace old forward descriptor*/
				_rtl819x_mergeMCastv6FwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 		= rtl819x_genMCastv6EntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu 	= rtl819x_genMCastv6EntryCpuFlag(mCastEntry);
					
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
					
			#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
				diag_printf("%s:%d,oldAsicFwdPortMask is %d,newAsicFwdPortMask is %d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,newAsicFwdPortMask);
			#endif
					
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
								external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */
				rtl_getGroupInfov6(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl819x_patchPppoeWeakv6(mCastEntry);
						
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl819x_freeMCastv6Entry(mCastEntry,index);
					}
					_rtl819x_arrangeMulticastv6(index);
				}
			}	
				
		}
	}

	return EVENT_CONTINUE_EXECUTE;
}
static int32 _rtl819x_multicastv6UnRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST6;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl819x_multicastv6CallbackFn;
	rtl865x_unRegisterEvent(&eventParam);
	return SUCCESS;
}
static int32 _rtl819x_multicastv6RegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST6;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl819x_multicastv6CallbackFn;
	rtl865x_registerEvent(&eventParam);
	return SUCCESS;
}
void _rtl819x_timeUpdateMulticastv6(uint32 secPassed)
{
	rtl819x_tblDrv_mCastv6_t *mCast_t, *nextMCast_t;
	uint32 entry;
	uint32 needReArrange=FALSE;
	uint32 hashLineCnt=0;
	//printk("[%s:%d]\n",__FUNCTION__,__LINE__);
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL819X_MULTICAST6TBL_SIZE; entry++) {
		hashLineCnt=0;
		needReArrange=FALSE;
		mCast_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == TRUE)
			{
				/* Entry is in the ASIC */
				if (mCast_t->age <= secPassed) 
				{
					if(mCast_t->mbr==0)
					{
						_rtl819x_freeMCastv6Entry(mCast_t,entry);
						needReArrange=TRUE;
					}
					else
					{
						mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					}
				}
				else
				{
					mCast_t->age -= secPassed;
				}
			}
			else 
			{
				//printk("------------mCast_t->count:%d,[%s]:[%d].\n",mCast_t->count,__FUNCTION__,__LINE__);
			
				//mCast_t->count=0;
			
				/* Entry is not in the ASIC */
				if (mCast_t->age <= secPassed)
				{ /* aging out */
					_rtl819x_freeMCastv6Entry(mCast_t, entry);
				}
				else 
				{
					mCast_t->age -= secPassed;
				}
			}
			
			/*won't count multicast entry forwarded by cpu*/
			if(mCast_t->cpu==0)
			{
				
				hashLineCnt++;
				//printk("------------hashLineCnt:%d,[%s]:[%d].\n",hashLineCnt,__FUNCTION__,__LINE__);
				if(hashLineCnt>=2)
				{
					needReArrange=TRUE;
				}
			}
		
			//mCast_t->count = 0;
			mCast_t = nextMCast_t;
		}
		
		if(needReArrange==TRUE)
		{
			//printk("------------entry:%d,hashLineCnt:%d,[%s]:[%d].\n",entry,hashLineCnt,__FUNCTION__,__LINE__);
			_rtl819x_arrangeMulticastv6(entry);
		}
		mCast_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == FALSE)
				mCast_t->count=0;
			
			mCast_t = nextMCast_t;
		}
		
	}
}
#if defined(__ECOS)
static void _rtl819x_mCastv6SysTimerExpired(uint32 expireDada)
{

	_rtl819x_timeUpdateMulticastv6(1);
	mod_timer(&rtl819x_mCast6SysTimer, jiffies+HZ);
	
}
static void _rtl819x_initMCastv6SysTimer(void)
{
	init_timer(&rtl819x_mCast6SysTimer);
	rtl819x_mCast6SysTimer.data = (unsigned long)0;
	rtl819x_mCast6SysTimer.function=(void*)_rtl819x_mCastv6SysTimerExpired;
	//BUG_ON(timer_pending(&igmpSysTimer));
	mod_timer(&rtl819x_mCast6SysTimer, jiffies+HZ);
}
static void _rtl819x_destroyMCastv6SysTimer(void)
{
	 del_timer_sync(&rtl819x_mCast6SysTimer);
}

#endif
int32 rtl_initMulticastv6(void)
{
	//only set register, other to check
	int mtu;
	_rtl819x_multicastv6UnRegisterEvent();
	_rtl819x_initMCastv6EntryPool();
	_rtl819x_initMCastv6FwdDescPool();
#if defined(__ECOS) 
	_rtl819x_initMCastv6SysTimer();
#endif
	_rtl819x_multicastv6RegisterEvent();
	rtl819x_setAsicMulticastv6MTU(1522);
	rtl819x_setAsicMulticastv6Enable(TRUE);
	rtl819x_setAsicMulticastv6Aging(TRUE);
	return SUCCESS;
}
int32 rtl_reinitMulticastv6(void)
{
	unsigned int ip6addr0[4]= {0};
	_rtl819x_multicastv6UnRegisterEvent();
	/*delete all multicast entry*/
	rtl819x_setAsicMulticastv6Enable(FALSE);
	rtl819x_delMulticastv6Entry(ip6addr0);
#if defined(__ECOS) 
	_rtl819x_destroyMCastv6SysTimer();
	_rtl819x_initMCastv6SysTimer();
#endif
	
	/*regfster twice won't cause any side-effect, 
	because event management module will handle duplicate event issue*/
	rtl819x_setAsicMulticastv6MTU(1522);
	rtl819x_setAsicMulticastv6Enable(TRUE);
	rtl819x_setAsicMulticastv6Aging(TRUE);
	_rtl819x_multicastv6RegisterEvent();

	return SUCCESS;
}
int rtl819x_genVirtualMCastv6FwdDescriptor(unsigned int forceToCpu, unsigned int  fwdPortMask, rtl819x_mcast_fwd_descriptor6_t *fwdDescriptor)
{
	if(fwdDescriptor==NULL)
	{
		return FAILED;
	}
	memset(fwdDescriptor, 0, sizeof(rtl819x_mcast_fwd_descriptor6_t ));
	fwdDescriptor->toCpu=forceToCpu;
	fwdDescriptor->fwdPortMask=fwdPortMask;
	return SUCCESS;
}
int rtl819x_blockMulticastv6Flow(unsigned int srcVlanId,unsigned int srcPort,unsigned int *srcIpAddr,unsigned int *destIpAddr)
{
	rtl819x_mcast_fwd_descriptor6_t fwdDescriptor;
	rtl819x_tblDrv_mCastv6_t * existMCastEntry=NULL;
	existMCastEntry=rtl819x_findMCastv6Entry(destIpAddr, srcIpAddr, (uint16)srcVlanId, (uint16)srcPort);
	if(existMCastEntry!=NULL)
	{
		if(existMCastEntry->mbr==0)
		{
			return SUCCESS;
		}
	}
	memset(&fwdDescriptor, 0, sizeof(rtl819x_mcast_fwd_descriptor6_t ));
	rtl819x_addMulticastv6Entry(destIpAddr,srcIpAddr,(unsigned short)srcVlanId,(unsigned short)srcPort,&fwdDescriptor,TRUE,0,0,0);
	return SUCCESS;
}

#endif
