/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/swTable.c,v 1.1.1.1 2007/08/06 10:04:52 root Exp $
*
* Abstract: Switch core table access driver source code.
*
* $Author: root $
*
* $Log: swTable.c,v $
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.5  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.4  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:25  alva
* initial import for add TFTP server
*
* Revision 1.3  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.2  2004/03/30 11:25:11  yjlou
* *: table_entry_length() is fixed to 8 words (in swTable_readEntry() and tableAccessForeword() ).
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:55  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:06  danwu
* no message
*
* ---------------------------------------------------------------
*/
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#endif


#include "swTable.h"
#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "rtl865xC_tblAsicDrv.h"
#include "rtl8651_layer2.h"
//#include <assert.h>
#include <switch/rtl865x_fdb_api.h>
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)//defined (CONFIG_RTL_LAYERED_DRIVER_L3)
#include "rtl865x_asicL3.h"
#endif
#include <switch/rtl865x_arp_api.h>
#include "l3Driver/rtl865x_ip.h"



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/* RTL_STATIC_INLINE */ void tableAccessForeword(uint32, uint32, void *);

static uint32 _rtl8651_asicTableSize[] =
{
        2 /*TYPE_L2_SWITCH_TABLE*/,
        1 /*TYPE_ARP_TABLE*/,
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	 3 /*TYPE_L3_ROUTING_TABLE*/,
#else
	 2 /*TYPE_L3_ROUTING_TABLE*/,
#endif
        3 /*TYPE_MULTICAST_TABLE*/,
        5 /*TYPE_NETIF_TABLE*/,
        3 /*TYPE_EXT_INT_IP_TABLE*/,
        3 /*TYPE_VLAN_TABLE*/,
        3 /*TYPE_VLAN1_TABLE*/,          
    4 /*TYPE_SERVER_PORT_TABLE*/,
    3 /*TYPE_L4_TCP_UDP_TABLE*/,
    3 /*TYPE_L4_ICMP_TABLE*/,
    1 /*TYPE_PPPOE_TABLE*/,
#if defined(CONFIG_RTL_8197F)
    11 /*TYPE_ACL_RULE_TABLE*/,
#else
    8 /*TYPE_ACL_RULE_TABLE*/,
#endif
    1 /*TYPE_NEXT_HOP_TABLE*/,
    3 /*TYPE_RATE_LIMIT_TABLE*/,
    1 /*TYPE_ALG_TABLE*/,
#if defined(CONFIG_RTL_8197F)  
   9 /*TYPE_DS_LITE_TABLE*/,	
   6 /*TYPE_6RD_TABLE*/,
   6 /*TYPE_L3_V6_ROUTING_TABLE*/,
   1 /*TYPE_NEXT_HOP_V6_TABLE*/,
   3 /*TYPE_ARP_V6_TABLE*/,
   9 /*TYPE_MULTICAST_V6_TABLE*/,
#endif
};

int32 swTable_addEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    REG32(SWTCR0) = REG32(SWTCR0) | EN_STOP_TLU;
    while ((REG32(SWTCR0) & STOP_TLU_READY) == 0);

    tableAccessForeword(tableType, eidx, entryContent_P);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_ADD;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );

    REG32(SWTCR0) = REG32(SWTCR0) & ~EN_STOP_TLU;

    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )
        return ECOLLISION;
    else
        return 0;
}





int32 swTable_modifyEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    REG32(SWTCR0) = REG32(SWTCR0) | EN_STOP_TLU;
    while ((REG32(SWTCR0) & STOP_TLU_READY) == 0);

    tableAccessForeword(tableType, eidx, entryContent_P);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_MODIFY;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );

    REG32(SWTCR0) = REG32(SWTCR0) & ~EN_STOP_TLU;

    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )
        return EEMPTY;
    else
        return 0;
}





int32 swTable_forceAddEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    REG32(SWTCR0) = REG32(SWTCR0) | EN_STOP_TLU;
    while ((REG32(SWTCR0) & STOP_TLU_READY) == 0);

    tableAccessForeword(tableType, eidx, entryContent_P);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_FORCE;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );

    REG32(SWTCR0) = REG32(SWTCR0) & ~EN_STOP_TLU;

    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )
        return 0;
        
    /* There might be something wrong */
    ASSERT_CSP( 0 );

}





int32 swTable_readEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    uint32 *    entryAddr;

    REG32(SWTCR0) = REG32(SWTCR0) | EN_STOP_TLU;
    while ((REG32(SWTCR0) & STOP_TLU_READY) == 0);

    ASSERT_CSP(entryContent_P);
    
    entryAddr = (uint32 *) (table_access_addr_base(tableType) + eidx * TABLE_ENTRY_DISTANCE);
    
    /* Wait for command ready */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Read registers according to entry width of each table */
    *((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
    *((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
    *((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
    *((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
    *((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
    *((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
    *((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
    *((uint32 *)entryContent_P + 0) = *(entryAddr + 0);

    REG32(SWTCR0) = REG32(SWTCR0) & ~EN_STOP_TLU;

    return 0;
}




//RTL_STATIC_INLINE void tableAccessForeword(uint32 tableType, uint32 eidx,     void *entryContent_P)
void tableAccessForeword(uint32 tableType, uint32 eidx,     void *entryContent_P)
{
    ASSERT_CSP(entryContent_P);

    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Write registers according to entry width of each table */
    REG32(TCR7) = *((uint32 *)entryContent_P + 7);
    REG32(TCR6) = *((uint32 *)entryContent_P + 6);
    REG32(TCR5) = *((uint32 *)entryContent_P + 5);
    REG32(TCR4) = *((uint32 *)entryContent_P + 4);
    REG32(TCR3) = *((uint32 *)entryContent_P + 3);
    REG32(TCR2) = *((uint32 *)entryContent_P + 2);
    REG32(TCR1) = *((uint32 *)entryContent_P + 1);
    REG32(TCR0) = *(uint32 *)entryContent_P;
	
#if defined(CONFIG_RTL_8197F)
	if( _rtl8651_asicTableSize[tableType]>8)
	{	
		REG32(TCR8) = *((uint32 *)entryContent_P + 8);
		REG32(TCR9) = *((uint32 *)entryContent_P + 9);
		REG32(TCR10) = *((uint32 *)entryContent_P + 10);
	}
#endif
    /* Fill address */
    REG32(SWTAA) = table_access_addr_base(tableType) + eidx * TABLE_ENTRY_DISTANCE;
}

#include "rtl865xC_tblAsicDrv.h"

#define RTL_WLAN_NAME "wlan"
#define FDB_STATIC						0x01
#define FDB_DYNAMIC					0x02
static uint8 fidHashTable[]={0x00,0x0f,0xf0,0xff};
rtl865x_tblAsicDrv_l2Param_t __l2buff;

__IRAM_L2_FWD uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr,uint16 fid) {
    return ( macAddr->octet[0] ^ macAddr->octet[1] ^
                    macAddr->octet[2] ^ macAddr->octet[3] ^
                    macAddr->octet[4] ^ macAddr->octet[5] ^fidHashTable[fid]) & 0xFF;
}

int32 rtl8651_setAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p) 
{
	rtl865xc_tblAsic_l2Table_t entry;

	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;
	if(l2p->macAddr.octet[5] != ((row^(fidHashTable[l2p->fid])^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4] ) & 0xff))
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.mac47_40 = l2p->macAddr.octet[0];
	entry.mac39_24 = (l2p->macAddr.octet[1] << 8) | l2p->macAddr.octet[2];
	entry.mac23_8 = (l2p->macAddr.octet[3] << 8) | l2p->macAddr.octet[4];


#if 1 //chhuang: #ifdef CONFIG_RTL8650B
	if( l2p->memberPortMask  > RTL8651_PHYSICALPORTMASK) //this MAC is on extension port
		entry.extMemberPort = (l2p->memberPortMask >>RTL8651_PORT_NUMBER);   
#endif /* CONFIG_RTL8650B */

	entry.memberPort = l2p->memberPortMask & RTL8651_PHYSICALPORTMASK;
	entry.toCPU = l2p->cpu==TRUE? 1: 0;
	entry.isStatic = l2p->isStatic==TRUE? 1: 0;
	entry.nxtHostFlag = l2p->nhFlag==TRUE? 1: 0;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	entry.agingTime = ( l2p->ageSec > 300 )? 0x03: ( l2p->ageSec <= 300 && l2p->ageSec > 150 )? 0x02: (l2p->ageSec <= 150 && l2p->ageSec > 0 )? 0x01: 0x00;
	
	entry.srcBlock = (l2p->srcBlk==TRUE)? 1: 0;
	entry.fid=l2p->fid;
	entry.auth=l2p->auth;
	return swTable_addEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}

int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p) {
	rtl865xc_tblAsic_l2Table_t   entry;
 
	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;

	swTable_readEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);

	if(entry.agingTime == 0 && entry.isStatic == 0 &&entry.auth==0)
		return FAILED;
	l2p->macAddr.octet[0] = entry.mac47_40;
	l2p->macAddr.octet[1] = entry.mac39_24 >> 8;
	l2p->macAddr.octet[2] = entry.mac39_24 & 0xff;
	l2p->macAddr.octet[3] = entry.mac23_8 >> 8;
	l2p->macAddr.octet[4] = entry.mac23_8 & 0xff;
	l2p->macAddr.octet[5] = row ^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4]  ^(fidHashTable[entry.fid]);
	l2p->cpu = entry.toCPU==1? TRUE: FALSE;
	l2p->srcBlk = entry.srcBlock==1? TRUE: FALSE;
	l2p->nhFlag = entry.nxtHostFlag==1? TRUE: FALSE;
	l2p->isStatic = entry.isStatic==1? TRUE: FALSE;
	l2p->memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	l2p->ageSec = entry.agingTime * 150;

	l2p->fid=entry.fid;
	l2p->auth=entry.auth;
	return SUCCESS;
}

int32 rtl8651_lookupL2table(uint16 fid, ether_addr_t * macAddr, int flags)
{
	uint32 hash0, way0;
		
	hash0 = rtl8651_filterDbIndex(macAddr, fid);

	for(way0=0; way0<RTL8651_L2TBL_COLUMN; way0++) {
		if (rtl8651_getAsicL2Table(hash0, way0, &__l2buff)!=SUCCESS ||
			memcmp(&__l2buff.macAddr, macAddr, 6)!= 0)
			continue;
		
		return SUCCESS;			
	}
	return FAILED;
}

int32 rtl8651_delAsicL2Table(uint32 row, uint32 column) {
	rtl8651_tblAsic_l2Table_t entry;

	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;

	bzero(&entry, sizeof(entry));
	return swTable_forceAddEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}

int32 _rtl8651_delAsicEntry(uint32 tableType, uint32 startEidx, uint32 endEidx)
{
	uint32 eidx = startEidx;

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), 0);
		}
	}
#else
	WRITE_MEM32(TCR0, 0);
	WRITE_MEM32(TCR1, 0);
	WRITE_MEM32(TCR2, 0);
	WRITE_MEM32(TCR3, 0);
	WRITE_MEM32(TCR4, 0);
	WRITE_MEM32(TCR5, 0);
	WRITE_MEM32(TCR6, 0);
	WRITE_MEM32(TCR7, 0);
#if defined(CONFIG_RTL_8197F)
    if(_rtl8651_asicTableSize[tableType]>8)
    {
    	WRITE_MEM32(TCR8, 0);
    	WRITE_MEM32(TCR9, 0);
    	WRITE_MEM32(TCR10, 0);
    }
#endif
#endif

	while (eidx <= endEidx) {
		WRITE_MEM32(SWTAA, (uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);//Fill address

		WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command

		while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

		if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
			return FAILED;

		++eidx;
	}
	return SUCCESS;
}


static rtl865x_tblAsicDrv_l2Param_t *lr_fdb_lookup(uint32 vfid, ether_addr_t *mac,  uint32 flags, uint32 *way)
{
	uint32 hash0, way0;

	hash0 = rtl8651_filterDbIndex(mac, vfid);
	for(way0=0; way0<RTL8651_L2TBL_COLUMN; way0++) {
		if (rtl8651_getAsicL2Table(hash0, way0, &__l2buff)!=SUCCESS ||
			memcmp(&__l2buff.macAddr, mac, 6)!= 0)
			continue;
		if (((flags&FDB_STATIC) && __l2buff.isStatic) ||
			((flags&FDB_DYNAMIC) && !__l2buff.isStatic)) {
			ASSERT_CSP(way);
			*way = way0;
			return &__l2buff;
		}
	} return (rtl865x_tblAsicDrv_l2Param_t *)0;
}

int32 rtl8651_delFilterDatabaseEntry(uint16 fid, ether_addr_t * macAddr)
{
	int32 res = FAILED;
	uint32 way, hash;

	if (__l2buff.memberPortMask == EXT_PORT_HWLOOKUP) 
		return SUCCESS;

	if (lr_fdb_lookup(fid, macAddr, FDB_DYNAMIC, &way)) {
		res = SUCCESS;
		hash=rtl8651_filterDbIndex(macAddr, fid);
			rtl8651_delAsicL2Table(hash, way); 
	} 
	return res;
}

int32 rtl8651_setPortToNetif(uint32 port, uint32 netifidx)
{
	uint16 offset;

	if(port>= RTL8651_PORT_NUMBER+3 || netifidx > 8)
		return FAILED;
	offset = (port * 3);
	WRITE_MEM32(PLITIMR,((READ_MEM32(PLITIMR) & (~(0x7 << offset))) | ((netifidx & 0x7)<< offset)));
	return SUCCESS;
}


#if !defined(CONFIG_RTL_LAYERED_DRIVER_L2)
void update_hw_l2table(const char *srcName,const unsigned char *addr)	
{
	ether_addr_t *macAddr;
	macAddr = (ether_addr_t *)(addr);

	if ((memcmp(srcName, RTL_WLAN_NAME, 4) ==0) &&
			rtl8651_lookupL2table(0, macAddr, FDB_DYNAMIC) == 0) 
		rtl8651_delFilterDatabaseEntry(0, macAddr); 	
}
#endif

/*=========================================
  * ASIC DRIVER API: INTERFACE TABLE
  *=========================================*/
  
/*
@func int32		| rtl865xC_setNetDecisionPolicy	| Set Interface Multilayer-Decision-Base Control 
@parm uint32 | policy | Possible values: NETIF_VLAN_BASED / NETIF_PORT_BASED / NETIF_MAC_BASED
@rvalue SUCCESS	| 	Success
@comm
RTL865xC supports Multilayer-Decision-Base for interface lookup.
 */
int32 rtl865xC_setNetDecisionPolicy( enum ENUM_NETDEC_POLICY policy )
{
	if ( policy == NETIF_PORT_BASED )
		WRITE_MEM32( SWTCR0, ( READ_MEM32( SWTCR0 ) & ~LIMDBC_MASK ) | LIMDBC_PORT );
	else if ( policy == NETIF_MAC_BASED )
		WRITE_MEM32( SWTCR0, ( READ_MEM32( SWTCR0 ) & ~LIMDBC_MASK ) | LIMDBC_MAC );
	else
		WRITE_MEM32( SWTCR0, ( READ_MEM32( SWTCR0 ) & ~LIMDBC_MASK ) | LIMDBC_VLAN );

	return SUCCESS;
}

/*
@func int32		| rtl8651_setAsicNetInterface	| Set ASIC Interface Table 
@parm uint32 | idx | Table index. Specific RTL865XC_NETIFTBL_SIZE to auto-search.
@parm rtl865x_tblAsicDrv_intfParam_t* | intfp | pointer to interface structure to add
@rvalue SUCCESS	| 	Success
@rvalue FAILED	| 	Failed
@comm
To read an interface entry, we provide two ways:
1. given the index which we want to force set
2. leave the index with RTL865XC_NETIFTBL_SIZE, we will search the whole table to find out existed entry or empty entry.
 */
 #ifdef RTL865X_TEST
 #define FULL_CAP
 #endif
int32 rtl8651_setAsicNetInterface( uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp )
{
	rtl865xc_tblAsic_netifTable_t entry;
	uint32 i;
 
	if(intfp == NULL)
		return FAILED;

	if ( idx==RTL865XC_NETIFTBL_SIZE )
	{
		/* User does not specific idx, we shall find out idx first. */
#ifdef FULL_CAP
		/* search Interface table to see if exists */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{
			swTable_readEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
	 		if ( entry.valid && entry.vid==intfp->vid )
	 		{
	 			idx = i;
				goto exist;
			}
		}	
		/* Not existed, find an empty entry */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{
			swTable_readEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if ( !entry.valid )
				break;
		}
		if ( i>=RTL865XC_NETIFTBL_SIZE )
			return FAILED; /* no empty entry */
		idx = i;
#else
		/* search Interface table to see if exists */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{	
			/* Since FPGA only has entry 0,1,6,7, we ignore null entry. */
			if (i>1&&(i<(RTL865XC_NETIFTBL_SIZE-2))) continue;
			
			swTable_readEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if (entry.valid)
				if (entry.vid==intfp->vid)
		 		{
		 			idx = i;
					goto exist;
				}
		}	
		/* Not existed, find an empty entry */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{	
			/* Since FPGA only has entry 0,1,6,7, we ignore null entry. */
			if (i>1&&(i<(RTL865XC_NETIFTBL_SIZE-2))) continue;

			swTable_readEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if (!entry.valid)
			{ 
				break;
			}
		}
		if ( i>=RTL865XC_NETIFTBL_SIZE )
			return FAILED; /* no empty entry */
		idx = i;
#endif
	}
	
exist:
	ASSERT_CSP(( idx < RTL865XC_NETIFTBL_SIZE ));

#ifdef CONFIG_HARDWARE_NAT_DEBUG
/*2007-12-19*/
	rtlglue_printf("%s:%d,idx is %d ,intfp->vid is %d\n",__FUNCTION__,__LINE__,idx, intfp->vid);
#endif

	memset(&entry,0,sizeof(entry));
	entry.valid = intfp->valid;
	entry.vid = intfp->vid;
	entry.mac47_19 = (intfp->macAddr.octet[0]<<21) | (intfp->macAddr.octet[1]<<13) | (intfp->macAddr.octet[2]<<5) |
					 (intfp->macAddr.octet[3]>>3);
	entry.mac18_0 = (intfp->macAddr.octet[3]<<16) | (intfp->macAddr.octet[4]<<8) | (intfp->macAddr.octet[5]);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	entry.inACLStartH = (intfp->inAclStart >>1)&0x7f;
	entry.inACLStartL = intfp->inAclStart&0x1;
	entry.inACLEnd	  = intfp->inAclEnd;
	entry.outACLStart = intfp->outAclStart;
	entry.outACLEnd   = intfp->outAclEnd;

#ifdef CONFIG_RTL_8198C_FPGA_TEST
	entry.enHWRouteV6 = intfp->enableRouteV6;
#else
	entry.enHWRouteV6 = (rtl8651_getAsicOperationLayer()>2)?	(intfp->enableRouteV6==TRUE? 1: 0):0;
#endif

	entry.enHWRoute   = (rtl8651_getAsicOperationLayer()>2)?	(intfp->enableRoute==TRUE? 1: 0):0;

	switch(intfp->macAddrNumber) {
		case 0:
		case 1:
			entry.macMaskL = 1;
			entry.macMaskH = 3;
		break;
		case 2:
			entry.macMaskL = 0;
			entry.macMaskH = 3;
		break;
		case 4:
			entry.macMaskL = 0;
			entry.macMaskH = 2;
		break;
		case 8:
			entry.macMaskL = 0;
			entry.macMaskH = 0;
			break;
		default:
			return FAILED;//Not permitted macNumber value
	}
	entry.mtu	= intfp->mtu;
	entry.mtuV6 = intfp->mtuV6;
#else
	entry.inACLStartH = (intfp->inAclStart >>2)&0x1f;
	entry.inACLStartL = intfp->inAclStart&0x3;
	entry.inACLEnd = intfp->inAclEnd;
	entry.outACLStart= intfp->outAclStart;
	entry.outACLEnd = intfp->outAclEnd;

	entry.enHWRoute = (rtl8651_getAsicOperationLayer()>2)?	(intfp->enableRoute==TRUE? 1: 0):0;

	switch(intfp->macAddrNumber) {
		case 0:
		case 1:
		    entry.macMask = 7;
		break;
		case 2:
		    entry.macMask = 6;
		break;
		case 4:
		    entry.macMask = 4;
		break;
		case 8:
		    entry.macMask = 0;
			break;
		default:
		    return FAILED;//Not permitted macNumber value
	}
	entry.mtuH = intfp->mtu >>3;
	entry.mtuL = intfp->mtu & 0x7;
#endif
	return swTable_forceAddEntry(TYPE_NETINTERFACE_TABLE, idx, &entry);
}


/*
@func int32		| rtl865x_delNetInterfaceByVid	| Delete ASIC Interface Table according to Vlan ID
@parm uint16 | vid | vlan id .
@rvalue SUCCESS	| 	Success
@rvalue FAILED	| 	Failed
@comm
 */
int32 rtl865x_delNetInterfaceByVid(uint16 vid)
{
	rtl865xc_tblAsic_netifTable_t entry;
	uint32 i,netIfIdx;
	int32 retVal = FAILED;

	netIfIdx = RTL865XC_NETIFTBL_SIZE;

	if(vid < 1 || vid > 4095)
		return FAILED;

	/*search...*/
	for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
	{
		swTable_readEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
	 	if ( entry.valid && entry.vid==vid )
	 	{
	 		netIfIdx = i;
			break;
		}
	}

	if(netIfIdx < RTL865XC_NETIFTBL_SIZE)
	{
		memset(&entry,0,sizeof(entry));
		retVal = swTable_forceAddEntry(TYPE_NETINTERFACE_TABLE, netIfIdx, &entry);		
	}

	return retVal;
	
}


static int32 rtl8651_operationlayer=0;
int32 rtl8651_setAsicAgingFunction(int8 l2Enable, int8 l4Enable)
{
	WRITE_MEM32( TEACR, (READ_MEM32(TEACR) & ~0x3) |(l2Enable == TRUE? 0x0: 0x1) | (l4Enable == TRUE? 0x0 : 0x2));
	return SUCCESS;
}
int32 rtl8651_setAsicOperationLayer(uint32 layer) 
{
	if(layer<1 || layer>4)
		return FAILED;
	/*   for bridge mode ip multicast patch
		When  in bridge mode,
		only one vlan(8) is available,
		if rtl8651_operationlayer is set less than 3,
		(pelase refer to rtl8651_setAsicVlan() )
		the "enable routing bit" of VLAN table will be set to 0 according to rtl8651_operationlayer.
		On the one hand, multicast data is flooded in vlan(8) by hardware, 
		on the other hand,it will be also trapped to cpu.
		In romedriver process,
		it will do _rtl8651_l2PhysicalPortRelay() in _rtl8651_l2switch(),
		and results in same multicast packet being flooded twice: 
		one is by hareware, the other is by romedriver.
		so the minimum  rtl8651_operationlayer will be set 3.
	*/
	#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST)
	/*if hardware multicast is enable the minimium asic operation layer must be above layer 3  */
	if(layer<4)
	{
		layer=4;
	}
	#endif
	if(layer == 1) {
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L2|EN_L3|EN_L4));
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_IN_ACL));
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_OUT_ACL));
	}else{
		/*
		 * Egress acl check should never enable, becuase of some hardware bug 
		 * reported by alpha    2007/12/05
		 */
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_IN_ACL));
		/*WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_OUT_ACL));*/

		if(layer == 2) {
			WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_L2));
			WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L3|EN_L4));
		}
		else
		{	// options for L3/L4 enable
			//WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)|FRAG2CPU); //IP fragment packet need to send to CPU when multilayer is enabled
			/*
			  * Only for RTL8650B:
			  * It is no need to trap fragment packets to CPU in pure rouitng mode while ACL is enabled, hence we should 
			  * turn this bit (FRAG2CPU) off.
			  * NOTE: if we do this, we should also turn ENFRAG2ACLPT on.     
			  *														-chhuang, 7/30/2004
			  */
			   /*
			  *    FRAG2CPU bit should be in ALECR register, not MSCR.
			  */
			//WRITE_MEM32(MSCR,READ_MEM32(MSCR) & ~FRAG2CPU);
			WRITE_MEM32(ALECR,READ_MEM32(ALECR) & ~FRAG2CPU);
		
			if(layer == 3) {
				WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_L2|EN_L3));
				WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L4));
			}
			else {	// layer 4
				WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_L2|EN_L3|EN_L4));
			}
		}
	}
	if(layer == 1)
		rtl8651_setAsicAgingFunction(FALSE, FALSE);
	else if (layer == 2 || layer == 3)
		rtl8651_setAsicAgingFunction(TRUE, FALSE);
	else
		rtl8651_setAsicAgingFunction(TRUE, TRUE);
	rtl8651_operationlayer	=layer;
	return SUCCESS;
}

int32 rtl8651_getAsicOperationLayer(void) 
{
#ifdef RTL865X_DEBUG
	uint32 regValue, layer=0;

	regValue = READ_MEM32(MSCR);
	switch(regValue & (EN_L2|EN_L3|EN_L4)) {
		case 0:
			layer = 1;
		break;
		case EN_L2:
			layer = 2;
		break;
		case (EN_L2|EN_L3):
			layer = 3;
		break;
		case (EN_L2|EN_L3|EN_L4):
			layer = 4;
		break;
		default:
			assert(0);//ASIC should not have such value
	}
	if(layer!=rtl8651_operationlayer){
		rtlglue_printf( "READ_MEM32(MSCR)=0x%08x\n", READ_MEM32(MSCR) );
		rtlglue_printf( "layer=%d, rtl8651_operationlayer=%d\n", layer, rtl8651_operationlayer );
		RTL_BUG( "MSCR layer setting is not the same as rtl8651_operationlayer" );
		return -1;
	}
#endif
	return  rtl8651_operationlayer;
}

#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
int32 rtl8651_setAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.nextHop = (arpp->nextHopRow<<2) | (arpp->nextHopColumn&0x3);
	entry.valid = 1;
	entry.aging=arpp->aging;
	return swTable_forceAddEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_delAsicArp(uint32 index) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if(index >= RTL8651_ARPTBL_SIZE)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return swTable_forceAddEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_getAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;
	//_rtl8651_readAsicEntry(TYPE_ARP_TABLE, index, &entry);
	/*for 8196B patch,read arp table and ip multicast table should stop TLU*/
	swTable_readEntry(TYPE_ARP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;
	arpp->nextHopRow = entry.nextHop>>2;
	arpp->nextHopColumn = entry.nextHop&0x3;
	arpp->aging=entry.aging&0x1f;
	return SUCCESS;
}

int32 rtl8651_setAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp) 
{
	rtl8651_tblAsic_extIpTable_t   entry;
	
	if((index >= RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL) || 
	((extIntIpp->localPublic == TRUE) && (extIntIpp->nat == TRUE))) //Local public IP and NAT property cannot co-exist
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.externalIP = extIntIpp->extIpAddr;
	entry.internalIP = extIntIpp->intIpAddr;
	entry.isLocalPublic = extIntIpp->localPublic==TRUE? 1: 0;
	entry.isOne2One = extIntIpp->nat==TRUE? 1: 0;
    	entry.nextHop = extIntIpp->nhIndex;
	entry.valid = 1;
	return swTable_forceAddEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
}
int32 rtl8651_delAsicExtIntIpTable(uint32 index) 
{
    	rtl8651_tblAsic_extIpTable_t   entry;

	if(index >= RTL8651_IPTABLE_SIZE)
		return FAILED;
        
	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return swTable_forceAddEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
}

int32 rtl8651_getAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp)
{
    	rtl8651_tblAsic_extIpTable_t   entry;
    
	if((index>=RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL))
		return FAILED;
	swTable_readEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;//Entry not found
	extIntIpp->extIpAddr = entry.externalIP;
	extIntIpp->intIpAddr = entry.internalIP;
	extIntIpp->localPublic = entry.isLocalPublic==1? TRUE: FALSE;
	extIntIpp->nat = entry.isOne2One==1? TRUE: FALSE;
    	extIntIpp->nhIndex = entry.nextHop;
    	return SUCCESS;
}

int32 rtl8651_setAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp)
{
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index >= RTL8651_NEXTHOPTBL_SIZE ) || (nextHopp == NULL))
        return FAILED;

	/* for debug
	rtlglue_printf("[%s:%d] rtl8651_setAsicNextHopTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    memset(&entry,0,sizeof(entry));
    entry.nextHop = (nextHopp->nextHopRow <<2) | nextHopp->nextHopColumn;
    entry.PPPoEIndex = nextHopp->pppoeIdx;
    entry.dstVid = nextHopp->dvid;
    entry.IPIndex = nextHopp->extIntIpIdx;
    entry.type = nextHopp->isPppoe==TRUE? 1: 0;
    return swTable_forceAddEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
}

int32 rtl8651_getAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp) 
{
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index>=RTL8651_NEXTHOPTBL_SIZE) || (nextHopp == NULL))
        return FAILED;

    swTable_readEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
    nextHopp->nextHopRow = entry.nextHop>>2;
    nextHopp->nextHopColumn = entry.nextHop&0x3;
    nextHopp->pppoeIdx = entry.PPPoEIndex;
    nextHopp->dvid = entry.dstVid;
    nextHopp->extIntIpIdx = entry.IPIndex;
    nextHopp->isPppoe = entry.type==1? TRUE: FALSE;
    return SUCCESS;
}

int32 rtl8651_setAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) 
{
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL) || (pppoep->sessionId == 0xffff))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.sessionID = pppoep->sessionId;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	entry.ageTime = pppoep->age;
#endif /*CONFIG_RTL865XB*/
	
	return swTable_forceAddEntry(TYPE_PPPOE_TABLE, index, &entry);
}

int32 rtl8651_getAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) 
{
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL))
		return FAILED;

	swTable_readEntry(TYPE_PPPOE_TABLE, index, &entry);
	pppoep->sessionId = entry.sessionID;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	pppoep->age = entry.ageTime;
#endif /*CONFIG_RTL865XB*/

	return SUCCESS;
}

int32 rtl8651_setAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) 
{
	uint32 i, asicMask;
	rtl865xc_tblAsic_l3RouteTable_t entry;
#ifdef FPGA
	if (index==2) index=6;
	if (index==3) index=7;
	if (index>=4 && index <=5) 
		rtlglue_printf("Out of range\n");
#endif	
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	if (routingp->ipMask) {
		for(i=0; i<32; i++)
			if(routingp->ipMask & (1<<i)) break;
		asicMask = 31 - i;
	} else asicMask = 0;
    
	memset(&entry,0,sizeof(entry));
	entry.IPAddr = routingp->ipAddr;
	switch(routingp->process) {
	case 0://PPPoE
		entry.linkTo.PPPoEEntry.PPPoEIndex = routingp->pppoeIdx;
		entry.linkTo.PPPoEEntry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.PPPoEEntry.IPMask = asicMask;
		entry.linkTo.PPPoEEntry.netif = routingp->vidx;
		entry.linkTo.PPPoEEntry.internal=routingp->internal;
		entry.linkTo.PPPoEEntry.isDMZ=routingp->DMZFlag;
		entry.linkTo.PPPoEEntry.process = routingp->process;
		entry.linkTo.PPPoEEntry.valid = 1;

		break;
	case 1://Direct
		entry.linkTo.L2Entry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.L2Entry.IPMask = asicMask;
		entry.linkTo.L2Entry.netif = routingp->vidx;
		entry.linkTo.L2Entry.internal=routingp->internal;
		entry.linkTo.L2Entry.isDMZ=routingp->DMZFlag;
		entry.linkTo.L2Entry.process = routingp->process;
		entry.linkTo.L2Entry.valid = 1;		
		break;
	case 2://arp
		entry.linkTo.ARPEntry.ARPEnd = routingp->arpEnd >> 3;
		entry.linkTo.ARPEntry.ARPStart = routingp->arpStart >> 3;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		entry.linkTo.ARPEntry.isDMZ = routingp->DMZFlag;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;

		entry.linkTo.ARPEntry.ARPIpIdx = routingp->arpIpIdx; /* for RTL8650B C Version Only */
		break;
	case 4://CPU
	case 6://DROP
		/*
		  *   Note:  although the process of this routing entry is CPU/DROP,
		  *             we still have to assign "vid" field for packet decision process use.
		  *                                                                                          - 2005.3.23 -
		  */
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		break;
	case 5://NAPT NextHop
		entry.linkTo.NxtHopEntry.nhStart = routingp->nhStart >> 1;
		switch (routingp->nhNum)
		{
		    case 2: entry.linkTo.NxtHopEntry.nhNum = 0; break;
		    case 4: entry.linkTo.NxtHopEntry.nhNum = 1; break;
		    case 8: entry.linkTo.NxtHopEntry.nhNum = 2; break;
		    case 16: entry.linkTo.NxtHopEntry.nhNum = 3; break;
		    case 32: entry.linkTo.NxtHopEntry.nhNum = 4; break;
		    default: return FAILED;
		}
		entry.linkTo.NxtHopEntry.nhNxt = routingp->nhNxt;
		entry.linkTo.NxtHopEntry.nhAlgo = routingp->nhAlgo;
		entry.linkTo.NxtHopEntry.IPMask = asicMask;
		entry.linkTo.NxtHopEntry.process = routingp->process;
		entry.linkTo.NxtHopEntry.valid = 1;
		entry.linkTo.NxtHopEntry.IPDomain = routingp->ipDomain;
		entry.linkTo.NxtHopEntry.internal = routingp->internal;
		entry.linkTo.NxtHopEntry.isDMZ = routingp->DMZFlag;
		break;
		
	default: 
		return FAILED;
	}
    	return swTable_forceAddEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

int32 rtl8651_delAsicRouting(uint32 index) 
{
	rtl865xc_tblAsic_l3RouteTable_t entry;

	if(index >= RTL8651_ROUTINGTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(entry));
	entry.linkTo.ARPEntry.valid = 0;
	return swTable_forceAddEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

#endif
// define it if needed
#if 1 //def DEBUG_SWITCH_TABLES
//#define rtlglue_printf printk

void rtl865xC_dump_l2(void)
{
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	uint32 row, col, port, m=0;

	rtlglue_printf(">>ASIC L2 Table:\n");

	for(row=0x0; row<RTL8651_L2TBL_ROW; row++)
	{
		for(col=0; col<RTL8651_L2TBL_COLUMN; col++)
		{
			memset((void*)&asic_l2, 0, sizeof(asic_l2));
			if (rtl8651_getAsicL2Table(row, col, &asic_l2) == FAILED)
			{
				continue;
			}

			if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0 &&asic_l2.auth==0)
			{
				continue;
			}

			rtlglue_printf("%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, row, col, 
					asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], 
					asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5],asic_l2.fid
			);

			m++;

			for (port = 0 ; port < RTL8651_PORT_NUMBER + 3 ; port ++)
			{
				if (asic_l2.memberPortMask & (1<<port))
				{
					rtlglue_printf("%d ", port);
				}
			}

			rtlglue_printf(")");
			rtlglue_printf("%s %s %s %s age:%d ",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"", asic_l2.nhFlag?"NH":"", asic_l2.ageSec);

			if (asic_l2.auth)
			{
				rtlglue_printf("AUTH:%d",asic_l2.auth);
			}
			rtlglue_printf("\n");
		}
	}
}

int32 rtl8651_getAsicVlan(uint16 vid, rtl865x_tblAsicDrv_vlanParam_t *vlanp) 
{	
	rtl865xc_tblAsic_vlanTable_t entry;
	if(vlanp == NULL||vid>=RTL865XC_VLANTBL_SIZE)
		return FAILED;

	swTable_readEntry(TYPE_VLAN_TABLE, vid, &entry);	
	if((entry.extMemberPort | entry.memberPort) == 0)
	{
		return FAILED;
	}
	vlanp->memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
	vlanp->untagPortMask = (entry.extEgressUntag<<RTL8651_PORT_NUMBER) |entry.egressUntag;
	vlanp->fid=entry.fid;
    #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
    vlanp->vid = entry.vid;
    #endif
    
	return SUCCESS;
}

void rtl865xC_dump_vlan(void)
{
	int i, j;

	rtlglue_printf(">>ASIC VLAN Table:\n\n");
	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_vlanParam_t vlan;
        
        memset(&vlan, 0x00, sizeof(vlan));
		if ( rtl8651_getAsicVlan((uint16)i, &vlan ) == FAILED )
			continue;
        
        #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
		rtlglue_printf("  Idx[%d] VID[%d] ", i, vlan.vid);
        #else
		rtlglue_printf("  VID[%d] ", i);
        #endif
		rtlglue_printf("\n\tmember ports:");

		for( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
		{
			if ( vlan.memberPortMask & ( 1 << j ) )
				rtlglue_printf("%d ", j);
		}

		rtlglue_printf("\n\tUntag member ports:");				

		for( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
		{
			if( vlan.untagPortMask & ( 1 << j ) )
				rtlglue_printf("%d ", j);
		}

		rtlglue_printf("\n\tFID:\t%d\n",vlan.fid);
	}
}

int32 rtl8651_getAsicNetInterface( uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp )
{
	rtl865xc_tblAsic_netifTable_t entry;
	uint32 i;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	 uint32   macMask;
#endif

	if(intfp == NULL)
		return FAILED;

	intfp->valid=0;

	if ( idx == RTL865XC_NETIFTBL_SIZE )
	{
		/* idx is not specified, we search whole interface table. */
		for( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
		{
#if 1 //def FULL_CAP
#else
			/* Since FPGA only has entry 0,1,6,7, we ignore null entry. */
			if (i>1&&(i<(RTL865XC_NETIFTBL_SIZE-2))) continue;
#endif

			swTable_readEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if ( entry.valid && entry.vid==intfp->vid ){
				goto found;
			}
		}

		/* intfp.vid is not found. */
		return FAILED;
	}
	else
	{
		/* idx is specified, read from ASIC directly. */
		swTable_readEntry(TYPE_NETINTERFACE_TABLE, idx, &entry);
	}

found:
	intfp->valid=entry.valid;
	intfp->vid=entry.vid;
	intfp->macAddr.octet[0] = entry.mac47_19>>21;
	intfp->macAddr.octet[1] = (entry.mac47_19 >>13)&0xff;
	intfp->macAddr.octet[2] = (entry.mac47_19 >>5)&0xff;
	intfp->macAddr.octet[3] = ((entry.mac47_19 &0x3f) <<3) | (entry.mac18_0 >>16);
	intfp->macAddr.octet[4] = (entry.mac18_0 >> 8)&0xff;
	intfp->macAddr.octet[5] = entry.mac18_0 & 0xff;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	intfp->inAclEnd 	= entry.inACLEnd;
	intfp->inAclStart	= (entry.inACLStartH<<1)|entry.inACLStartL;  
	intfp->outAclStart	= entry.outACLStart;
	intfp->outAclEnd	= entry.outACLEnd;
	intfp->enableRoute	= entry.enHWRoute==1? TRUE: FALSE;
	intfp->enableRouteV6= entry.enHWRouteV6==1? TRUE: FALSE;

	macMask = (entry.macMaskH<<1)|entry.macMaskL;
	switch(macMask)
	{
		case 0:
			intfp->macAddrNumber =8;
			break;
		case 6:
			intfp->macAddrNumber =2;
			break;
		case 4:
			intfp->macAddrNumber =4;
			break;
		case 7:
			intfp->macAddrNumber =1;
			break;			
	}  
	intfp->mtu = entry.mtu;
	intfp->mtuV6 = entry.mtuV6;
#else	
	intfp->inAclEnd = entry.inACLEnd;
	intfp->inAclStart= (entry.inACLStartH<<2)|entry.inACLStartL;
	intfp->outAclStart = entry.outACLStart;
	intfp->outAclEnd = entry.outACLEnd;
	intfp->enableRoute = entry.enHWRoute==1? TRUE: FALSE;

	switch(entry.macMask)
	{
		case 0:
			intfp->macAddrNumber =8;
			break;
		case 6:
			intfp->macAddrNumber =2;
			break;
		case 4:
			intfp->macAddrNumber =4;
			break;
		case 7:
			intfp->macAddrNumber =1;
			break;

			
	}
	intfp->mtu = (entry.mtuH <<3)|entry.mtuL;
#endif
	return SUCCESS;
}

#define RTL8651_ACLTBL_ALL_TO_CPU			127  // This rule is always "To CPU"
#define RTL8651_ACLTBL_DROP_ALL				126 //This rule is always "Drop"
#define RTL8651_ACLTBL_PERMIT_ALL			125	// This rule is always "Permit"
#define RTL8651_ACLTBL_PPPOEPASSTHRU		124 //For PPPoE Passthru Only
#define RTL8651_ACLTBL_RESERV_SIZE			4	//this many of ACL rules are reserved for internal use

void rtl865xC_dump_netif(void)
{
	int8	*pst[] = { "DIS/BLK",  "LIS", "LRN", "FWD" };
	uint8 *mac;
	int32 i, j;

	rtlglue_printf(">>ASIC Netif Table:\n\n");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_intfParam_t intf;
		rtl865x_tblAsicDrv_vlanParam_t vlan;

		if ( rtl8651_getAsicNetInterface( i, &intf ) == FAILED )
			continue;

		if ( intf.valid )
		{
			mac = (uint8 *)&intf.macAddr.octet[0];
			rtlglue_printf("[%d]  VID[%d] %x:%x:%x:%x:%x:%x", 
				i, intf.vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			rtlglue_printf("  Routing %s \n",
				intf.enableRoute==TRUE? "enabled": "disabled" );

			rtlglue_printf("      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					rtlglue_printf("permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					rtlglue_printf("all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					rtlglue_printf("drop all,");
			}
			else
				rtlglue_printf("ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			rtlglue_printf("  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					rtlglue_printf("permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					rtlglue_printf("all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					rtlglue_printf("drop all,");
			}
			else
				rtlglue_printf("ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);

			rtlglue_printf("\n      %d MAC Addresses, MTU %d Bytes\n", intf.macAddrNumber, intf.mtu);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			rtlglue_printf(" 	 enableRouteV6:%d,	  mtuv6:%d\n", intf.enableRouteV6, intf.mtuV6);
#endif
            #if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
			rtl8651_findAsicVlanIndexByVid(&intf.vid);
            #endif
			rtl8651_getAsicVlan( intf.vid, &vlan );

			rtlglue_printf("\n      Untag member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
			{
				if ( vlan.untagPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}
			rtlglue_printf("\n      Active member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}
			
			rtlglue_printf("\n      Port state(");

			for ( j = 0; j < RTL8651_PORT_NUMBER + 3; j++ )
			{
				if ( ( vlan.memberPortMask & ( 1 << j ) ) == 0 )
					continue;
				if ((( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET ) > 4 )
					rtlglue_printf("--- ");
				else
					rtlglue_printf("%d:%s ", j, pst[(( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET )]);

			}
			rtlglue_printf(")\n\n");
		}

	}
}
#if 1//defined PROC_DEBUG


//diagnostic
int32 diagnostic_show(void)
{
	int		len;
	uint32	regData, regData1;
	int		port, regIdx;
	uint32	mask, offset;

	rtlglue_printf("Diagnostic Register Info:\n");

	regData = READ_MEM32(GDSR0);
	rtlglue_printf("MaxUsedDescriptor: %d CurUsed Descriptor: %d\n",
		(regData&MaxUsedDsc_MASK)>>MaxUsedDsc_OFFSET,
		(regData&USEDDSC_MASK)>>USEDDSC_OFFSET);
	rtlglue_printf("DescRunOut: %s TotalDescFC: %s ShareBufFC: %s\n",
		(regData&DSCRUNOUT)?"YES":"NO", (regData&TotalDscFctrl_Flag)?"YES":"NO", (regData&SharedBufFCON_Flag)?"YES":"NO");

	for(regIdx = 0; regIdx<2; regIdx++)
	{
		regData = READ_MEM32(GCSR0+(regIdx<<2));

		for(port=0; port<4; port++)
		{
			switch(port)
			{
				case 0:
					mask = P0OQCgst_MASK;
					offset = P0OQCgst_OFFSET;
					break;
				case 1:
					mask = P1OQCgst_MASK;
					offset = P1OQCgst_OFFSET;
					break;
				case 2:
					mask = P2OQCgst_MASK;
					offset = P2OQCgst_OFFSET;
					break;
				default:
					mask = P3OQCgst_MASK;
					offset = P3OQCgst_OFFSET;
					break;
			}
			regData1 = (regData&mask)>>offset;
			if (regData1==0)
				rtlglue_printf("Port%d not congestion\n", port+(regIdx<<2));
			else
			{
				rtlglue_printf("Port%d queue congestion mask 0x%x\n", port+(regIdx<<2), regData1);
			}
		}
	}

	for(port=0;port<=CPU;port++)
	{
		rtlglue_printf("Port%d each queue used descriptor: Queue[0~5]: [ ", port);
		for(regIdx=0; regIdx<3; regIdx++)
		{
			regData = READ_MEM32(P0_DCR0+(port<<4)+(regIdx<<2));
			rtlglue_printf("%d %d ",
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET),
				((regData&Pn_OQDSCR_MASK)>>Pn_ODD_OQDSCR_OFFSET));
		}

		regData = READ_MEM32(P0_DCR3+(port<<4));
		rtlglue_printf("]  Input queue [%d]\n",
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET));
	}

	return 0;
}


int32 rtl8651_getAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) 
{
	uint32 i;
	rtl865xc_tblAsic_l3RouteTable_t entry;
    
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	swTable_readEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	if(entry.linkTo.ARPEntry.valid == 0)
		return FAILED;

	routingp->ipAddr = entry.IPAddr;
	routingp->process = entry.linkTo.ARPEntry.process;
	for(i=0, routingp->ipMask = 0; i<=entry.linkTo.ARPEntry.IPMask; i++)
		routingp->ipMask |= 1<<(31-i);
    
    	routingp->vidx = entry.linkTo.ARPEntry.netif;
	routingp->internal= entry.linkTo.PPPoEEntry.internal;
	switch(routingp->process) {
	case 0://PPPoE
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = entry.linkTo.PPPoEEntry.PPPoEIndex;
		routingp->nextHopRow = entry.linkTo.PPPoEEntry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.PPPoEEntry.nextHop & 0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 1://Direct
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = entry.linkTo.L2Entry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.L2Entry.nextHop&0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 2://Indirect
		routingp->arpEnd = entry.linkTo.ARPEntry.ARPEnd;
		routingp->arpStart = entry.linkTo.ARPEntry.ARPStart;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->arpIpIdx = entry.linkTo.ARPEntry.ARPIpIdx;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 4: /* CPU */
	case 6: /* Drop */
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	case 5:
		routingp->nhStart = (entry.linkTo.NxtHopEntry.nhStart) << 1;
		switch (entry.linkTo.NxtHopEntry.nhNum)
		{
		case 0: routingp->nhNum = 2; break;
		case 1: routingp->nhNum = 4; break;
		case 2: routingp->nhNum = 8; break;
		case 3: routingp->nhNum = 16; break;
		case 4: routingp->nhNum = 32; break;
		default: return FAILED;
		}
		routingp->nhNxt = entry.linkTo.NxtHopEntry.nhNxt;
		routingp->nhAlgo = entry.linkTo.NxtHopEntry.nhAlgo;
		routingp->ipDomain = entry.linkTo.NxtHopEntry.IPDomain;
		routingp->internal= entry.linkTo.NxtHopEntry.internal;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		 
		break;
#endif
	default: return FAILED;
	}
    return SUCCESS;
}

//l3
void l3_show(void)
{
	int len;
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	int8 *strNetType[] = { "WAN", "DMZ", "LAN",  "RLAN"};
	uint32 idx, mask;
	int netIdx;

	rtlglue_printf("%s\n", "ASIC L3 Routing Table:\n");
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++)
	{
		if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED)
		{
			rtlglue_printf("\t[%d]  (Invalid)\n", idx);
			continue;
		}
		if (idx == RTL8651_ROUTINGTBL_SIZE-1)
			mask = 0;
		else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
				mask--;
		netIdx = asic_l3.internal<<1|asic_l3.DMZFlag;
		rtlglue_printf("\t[%d]  %d.%d.%d.%d/%d process(%s) %s \n", idx, (asic_l3.ipAddr>>24),
			((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff),
			mask, str[asic_l3.process],strNetType[netIdx]);

		switch(asic_l3.process)
		{
		case 0x00:	/* PPPoE */
			rtlglue_printf("\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x01:	/* L2 */
			rtlglue_printf("              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x02:	/* ARP */
			rtlglue_printf("             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart<<3, asic_l3.arpEnd<<3, asic_l3.arpIpIdx);
			break;

		case 0x03:	/* Reserved */
			;

		case 0x04:	/* CPU */
			rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x05:	/* NAPT Next Hop */
			rtlglue_printf("              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
				asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
			break;

		case 0x06:	/* DROP */
			rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x07:	/* Reserved */
			/* pass through */
		default:
		;
		}
	}

	//return len;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
static int32 rtl819x_proc_privSkbInfo_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	int cpu_queue_cnt,rxRing_cnt,txRing_cnt,wlan_txRing_cnt,wlan_txRing_cnt1, mbuf_pending_cnt;
	int rx_skb_queue_cnt,poll_buf_cnt;
	unsigned long flags;

	len = sprintf(page, "original eth private buf total(%d)\n",MAX_ETH_SKB_NUM);

	local_irq_save(flags);
	mbuf_pending_cnt = get_mbuf_pending_times();
	cpu_queue_cnt = get_cpu_completion_queue_num();
	rxRing_cnt = get_nic_rxRing_buf();
	txRing_cnt = get_nic_txRing_buf();
#if defined(CONFIG_RTL8192CD)
	wlan_txRing_cnt = get_nic_buf_in_wireless_tx("wlan0");
	wlan_txRing_cnt1 = get_nic_buf_in_wireless_tx("wlan1");
#else
	wlan_txRing_cnt = 0;
	wlan_txRing_cnt1 = 0;
#endif
	rx_skb_queue_cnt = get_buf_in_rx_skb_queue();
	poll_buf_cnt = get_buf_in_poll();
	local_irq_restore(flags);

	len+=sprintf(page + len,"cpu completion cnt(%d)\nnic rxring cnt(%d)\nnic txring cnt(%d)\nwlan0 txring cnt(%d)\nwlan1 txring cnt(%d)\nrx skb queue cnt(%d)\npool buf cnt(%d)\nmbuf pending times(%d)\nsum(%d)\n",
	cpu_queue_cnt,rxRing_cnt,txRing_cnt,wlan_txRing_cnt,wlan_txRing_cnt1, rx_skb_queue_cnt,poll_buf_cnt,mbuf_pending_cnt,
	cpu_queue_cnt+rxRing_cnt+txRing_cnt+wlan_txRing_cnt+wlan_txRing_cnt1+rx_skb_queue_cnt+poll_buf_cnt);

	return len;
}

#endif
 
int32 wan_link_status(int port)
{
#if defined(CONFIG_RTL_8367R_SUPPORT)
    extern int rtk_port_phyStatus_get(int port, int *pLinkStatus, int *pSpeed, int *pDuplex);
    int status, speed, duplex;
    if ((rtk_port_phyStatus_get(port, &status, &speed, &duplex)) == 0) {
	    if (status == 0)
		    return 0;
	    else
            return 1;
    }
#else
    uint32	regData;
	uint32	data0;

	regData = READ_MEM32(PSRP0+((port)<<2));
	data0 = regData & PortStatusLinkUp;
	return data0;
#endif
}

int32 port_status_read( void )
{
	int		len;
	uint32	regData;
	uint32	data0;
	int		port;
	uint32  	regData_PCR;
	
#if defined(CONFIG_RTL_8367R_SUPPORT)
	rtlglue_printf("Dump 8367R Port Status:\n");

	#if defined(CONFIG_RTL_8197F)
	extern int rtk_port_status_read(int port);
	rtk_port_status_read(port);
	#else
	for(port=PHY0;port<=PHY4;port++)
	{
		extern int rtk_port_phyStatus_get(int port, int *pLinkStatus, int *pSpeed, int *pDuplex);
		int status, speed, duplex;

		if ((rtk_port_phyStatus_get(port, &status, &speed, &duplex)) == 0) {
			rtlglue_printf("Port%d ", port);

			if (status == 0) {
				rtlglue_printf("LinkDown\n\n");
				continue;
			}
			else {
				rtlglue_printf("LinkUp | ");
			}

			rtlglue_printf("	%s-Duplex | ", duplex?"Full":"Half");
			
			rtlglue_printf("Speed %s\n\n", speed==PortStatusLinkSpeed100M?"100M":
				(speed==PortStatusLinkSpeed1000M?"1G":
					(speed==PortStatusLinkSpeed10M?"10M":"Unkown")));			
		}
	}
	#endif
#else
	rtlglue_printf("Dump Port Status:\n");

	for(port=PHY0;port<=CPU;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));
		regData_PCR = READ_MEM32(PCRP0+((port)<<2));
		
		if (port==CPU)
			rtlglue_printf("CPUPort ");
		else
			rtlglue_printf("Port%d ", port);
		
		data0 = regData_PCR & EnForceMode;
		if(data0)
		{
			rtlglue_printf("Enforce Mode ");
			data0 = regData_PCR & PollLinkStatus;
			if (data0)
				rtlglue_printf(" | polling LinkUp");
			else
			{
				rtlglue_printf(" | disable Auto-Negotiation\n\n");
			}
		}
		else
		{
			rtlglue_printf("Force Mode disable\n");
		}
		data0 = (regData & (PortEEEStatus_MASK))>>PortEEEStatus_OFFSET;
		rtlglue_printf("EEE Status %0x\n",data0);
		
		regData = READ_MEM32(PSRP0+((port)<<2));
		data0 = regData & PortStatusLinkUp;

		if (data0)
			rtlglue_printf("LinkUp | ");
		else
		{
			rtlglue_printf("LinkDown\n\n");
			continue;
		}
		data0 = regData & PortStatusNWayEnable;
		rtlglue_printf("NWay Mode %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusRXPAUSE;
		rtlglue_printf("	RXPause %s | ", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusTXPAUSE;
		rtlglue_printf("TXPause %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusDuplex;
		rtlglue_printf("	Duplex %s | ", data0?"Enabled":"Disabled");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		rtlglue_printf("Speed %s\n\n", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
	}
#endif
	return len;
}

extern int32 rtl865xC_setAsicEthernetForceModeRegs(uint32 port, uint32 enForceMode, uint32 forceLink, uint32 forceSpeed, uint32 forceDuplex);
extern int32 rtl8651_setAsicEthernetPHYSpeed( uint32 port, uint32 speed );
extern int32 rtl8651_setAsicEthernetPHYDuplex( uint32 port, uint32 duplex );
extern int32 rtl8651_setAsicEthernetPHYAutoNeg( uint32 port, uint32 autoneg);
extern int32 rtl8651_setAsicEthernetPHYAdvCapality(uint32 port, uint32 capality);
extern int32 rtl8651_restartAsicEthernetPHYNway(uint32 port, uint32 phyid);

int32 port_status_write( uint32	port_mask, int 	type )
{
	int 		port;
#if !defined(CONFIG_RTL_8367R_SUPPORT)
	int forceMode = 0;
	int forceLink = 0;
	int forceLinkSpeed = 0;
	int forceDuplex = 0;
	uint32 advCapability = 0;
#endif


#define SPEED10M 	0
#define SPEED100M 	1
#define SPEED1000M 	2

#if defined(CONFIG_RTL_8367R_SUPPORT)
	for(port = 0; port < 5; port++)
	{
		extern int set_8367r_speed_mode(int port, int type);
		if (((1<<port) & port_mask) && (type != HALF_DUPLEX_1000M))
		{
			set_8367r_speed_mode(port, type);
		}
	}
#else
	switch(type)
	{
		case HALF_DUPLEX_10M:
		{
			forceMode=TRUE;
			forceLink=TRUE;
			forceLinkSpeed=SPEED10M;
			forceDuplex=FALSE;
			advCapability=(1<<HALF_DUPLEX_10M);
			break;
		}
		case HALF_DUPLEX_100M:
		{
			forceMode=TRUE;
			forceLink=TRUE;
			forceLinkSpeed=SPEED100M;
			forceDuplex=FALSE;
			advCapability=(1<<HALF_DUPLEX_100M);
			break;
		}
		case HALF_DUPLEX_1000M:
		{
			forceMode=TRUE;
			forceLink=TRUE;
			forceLinkSpeed=SPEED1000M;
			forceDuplex=FALSE;
			advCapability=(1<<HALF_DUPLEX_1000M);
			break;
		}
		case DUPLEX_10M:
		{
			forceMode=TRUE;
			forceLink=TRUE;
			forceLinkSpeed=SPEED10M;
			forceDuplex=TRUE;
			advCapability=(1<<DUPLEX_10M);
			break;
		}
		case DUPLEX_100M:
		{
			forceMode=TRUE;
			forceLink=TRUE;
			forceLinkSpeed=SPEED100M;
			forceDuplex=TRUE;
			advCapability=(1<<DUPLEX_100M);
			break;
		}
		case DUPLEX_1000M:
		{
			forceMode=TRUE;
			forceLink=TRUE;
			forceLinkSpeed=SPEED1000M;
			forceDuplex=TRUE;
			advCapability=(1<<DUPLEX_1000M);
			break;
		}	
		default:
		{
			forceMode=FALSE;
			forceLink=TRUE;
			/*all capality*/
			advCapability=(1<<PORT_AUTO);
		}
	}
#endif

	for(port = 0; port < CPU; port++)
	{
		if((1<<port) & port_mask)
		{
			#if 0
			rtl865xC_setAsicEthernetForceModeRegs(port, forceMode, forceLink, forceLinkSpeed, forceDuplex);

			/*Set PHY Register*/
			rtl8651_setAsicEthernetPHYSpeed(port,forceLinkSpeed);
			rtl8651_setAsicEthernetPHYDuplex(port,forceDuplex);
			rtl8651_setAsicEthernetPHYAutoNeg(port,forceMode?FALSE:TRUE);
			rtl8651_setAsicEthernetPHYAdvCapality(port,advCapability);
			
			rtl8651_restartAsicEthernetPHYNway(port+1,port);
			#endif
		}
	}

	return 1;
}

#ifdef CONFIG_RTL_8367R_SUPPORT
extern uint32 r8367_cpu_port;
extern rtk_stat_port_cntr_t  port_cntrs;
extern int rtk_stat_port_getAll(uint32 port, rtk_stat_port_cntr_t *pPort_cntrs);

void display_8367r_port_stat(uint32 port, rtk_stat_port_cntr_t *pPort_cntrs)
{
	if ( port > 4 )
		rtlglue_printf("\n<CPU port>\n");
	else
		rtlglue_printf("\n<Port: %d>\n", port);

	rtlglue_printf("Rx counters\n");

	rtlglue_printf("   ifInOctets  %llu\n", pPort_cntrs->ifInOctets);
	rtlglue_printf("   etherStatsOctets  %llu\n", pPort_cntrs->etherStatsOctets);
	rtlglue_printf("   ifInUcastPkts  %u\n", pPort_cntrs->ifInUcastPkts);
	rtlglue_printf("   etherStatsMcastPkts  %u\n", pPort_cntrs->etherStatsMcastPkts);
	rtlglue_printf("   etherStatsBcastPkts  %u\n", pPort_cntrs->etherStatsBcastPkts);
	
	rtlglue_printf("   StatsFCSErrors  %u\n", pPort_cntrs->dot3StatsFCSErrors);
	rtlglue_printf("   StatsSymbolErrors  %u\n", pPort_cntrs->dot3StatsSymbolErrors);
	rtlglue_printf("   InPauseFrames  %u\n", pPort_cntrs->dot3InPauseFrames);
	rtlglue_printf("   ControlInUnknownOpcodes  %u\n", pPort_cntrs->dot3ControlInUnknownOpcodes);
	rtlglue_printf("   etherStatsFragments  %u\n", pPort_cntrs->etherStatsFragments);
	rtlglue_printf("   etherStatsJabbers  %u\n", pPort_cntrs->etherStatsJabbers);
	rtlglue_printf("   etherStatsDropEvents  %u\n", pPort_cntrs->etherStatsDropEvents);

	rtlglue_printf("   etherStatsUndersizePkts  %u\n", pPort_cntrs->etherStatsUndersizePkts);
	rtlglue_printf("   etherStatsOversizePkts  %u\n", pPort_cntrs->etherStatsOversizePkts);

	rtlglue_printf("   Len= 64: %u pkts, 65 - 127: %u pkts, 128 - 255: %u pkts\n",
		pPort_cntrs->etherStatsPkts64Octets,
		pPort_cntrs->etherStatsPkts65to127Octets,
		pPort_cntrs->etherStatsPkts128to255Octets);
	rtlglue_printf("       256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n",
		pPort_cntrs->etherStatsPkts256to511Octets,
		pPort_cntrs->etherStatsPkts512to1023Octets,
		pPort_cntrs->etherStatsPkts1024toMaxOctets);

	rtlglue_printf("\nOutput counters\n");

	rtlglue_printf("   ifOutOctets  %llu\n", pPort_cntrs->ifOutOctets);
	rtlglue_printf("   ifOutUcastPkts  %u\n", pPort_cntrs->ifOutUcastPkts);
	rtlglue_printf("   ifOutMulticastPkts  %u\n", pPort_cntrs->ifOutMulticastPkts);
	rtlglue_printf("   ifOutBrocastPkts  %u\n", pPort_cntrs->ifOutBrocastPkts);
	
	rtlglue_printf("   StatsSingleCollisionFrames  %u\n", pPort_cntrs->dot3StatsSingleCollisionFrames);
	rtlglue_printf("   StatsMultipleCollisionFrames  %u\n", pPort_cntrs->dot3StatsMultipleCollisionFrames);
	rtlglue_printf("   StatsDeferredTransmissions  %u\n", pPort_cntrs->dot3StatsDeferredTransmissions);
	rtlglue_printf("   StatsLateCollisions  %u\n", pPort_cntrs->dot3StatsLateCollisions);
	rtlglue_printf("   etherStatsCollisions  %u\n", pPort_cntrs->etherStatsCollisions);
	rtlglue_printf("   StatsExcessiveCollisions  %u\n", pPort_cntrs->dot3StatsExcessiveCollisions);
	rtlglue_printf("   OutPauseFrames  %u\n", pPort_cntrs->dot3OutPauseFrames);
	rtlglue_printf("   dot1dBasePortDelayExceededDiscards  %u\n", pPort_cntrs->dot1dBasePortDelayExceededDiscards);
	rtlglue_printf("   dot1dTpPortInDiscards  %u\n", pPort_cntrs->dot1dTpPortInDiscards);
	rtlglue_printf("   outOampduPkts  %u\n", pPort_cntrs->outOampduPkts);
	rtlglue_printf("   inOampduPkts  %u\n", pPort_cntrs->inOampduPkts);
	rtlglue_printf("   pktgenPkts  %u\n", pPort_cntrs->pktgenPkts);

}

void rtl_get8367rPortStats(void)
{
	uint32	portNum=0xFFFFFFFF;

	for (portNum=0; portNum<7; portNum++) {
		if ((portNum > 4) && (portNum != r8367_cpu_port))	// skip port 6 in 8367R; skip port 5 in 8367RB
			continue;

		rtk_stat_port_getAll(portNum, &port_cntrs);
		display_8367r_port_stat(portNum, &port_cntrs);
	}
}
#endif

int32 rtl8651_getAsicPvid(uint32 port, uint32 *pvid) 
{
	uint16 offset;
	offset=(port*2)&(~0x3);
	if(port>RTL8651_PORT_NUMBER || pvid == NULL)
		return FAILED;
	if((port&0x1))
	{
		*pvid=(((READ_MEM32(PVCR0+offset)>>16)&0xFFF));		
	}
	else
	{
		*pvid=((READ_MEM32(PVCR0+offset)&0xFFF));
	}
	return SUCCESS;
}

int32 rtl8651_setAsicPvid(uint32 port, uint32 pvid)
{
	uint32 regValue,offset;
	
	if(port>RTL8651_PORT_NUMBER || pvid>=RTL865XC_VLAN_NUMBER)
		return FAILED;;
	offset=(port*2)&(~0x3);
	regValue=READ_MEM32(PVCR0+offset);
	if((port&0x1))
	{
		regValue=  ((pvid &0xfff) <<16) | (regValue&~0xFFF0000);
	}
	else
	{	
		regValue =	(pvid &0xfff) | (regValue &~0xFFF);
	}
	WRITE_MEM32(PVCR0+offset,regValue);
	return SUCCESS;
}



int32 pvid_read( void )
{
	uint32 vidp[9];
	int32  i;

	for(i=0; i<=RTL8651_PORT_NUMBER; i++)
	{
		if (rtl8651_getAsicPvid(i, &vidp[i]) != SUCCESS)
		{
			rtlglue_printf("ASIC PVID get failed.\n");
		}
	}
	rtlglue_printf(">> PVID Reg:\n");
	for(i=0; i<=RTL8651_PORT_NUMBER; i++)
		rtlglue_printf("p%d: %d\n", i, vidp[i]);

	

	return 1;
}

int32 pvid_write(uint32 port, uint32 pvid)
{


	
	if(port>(RTL8651_PORT_NUMBER))
	{
		goto errout;
	}

	
	if(pvid>4096)
	{
		goto errout;
	}

	rtl8651_setAsicPvid( port,	pvid);

	return 0;
	errout:
	rtlglue_printf("wrong format\n");



	return (-1);
}

uint32 rtl8651_returnAsicCounter(uint32 offset) 
{
	if(offset & 0x3)
		return 0;
	return  READ_MEM32(MIB_COUNTER_BASE + offset);
}

uint64 rtl865xC_returnAsicCounter64(uint32 offset)
{
	if ( offset & 0x3 )
		return 0;

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8196C)
	return ( READ_MEM32( MIB_COUNTER_BASE + offset ) + ( ( uint64 ) READ_MEM32( MIB_COUNTER_BASE + offset + 4 ) << 22 ) );
#else
	return ( READ_MEM32( MIB_COUNTER_BASE + offset ) + ( ( uint64 ) READ_MEM32( MIB_COUNTER_BASE + offset + 4 ) << 32 ) );
#endif
}

int32 rtl8651_clearAsicCounter(void) 
{
	WRITE_MEM32(MIB_CONTROL, ALL_COUNTER_RESTART_MASK);
#if 0	/* We don't want to read once first. */
	_rtl8651_initialRead();
#endif
#ifdef CONFIG_RTL_8367R_SUPPORT
	rtk_stat_global_reset();
#endif

	return SUCCESS;
}


/*
@func int32 | rtl865xC_dumpAsicDiagCounter | Dump complex counters of all ports (CPU port included).
@rvalue SUCCESS | Finish showing the counters.
@comm
Dump complex counters of all ports.
*/
int32 rtl865xC_dumpAsicDiagCounter(void)
{
	uint32 i;

	for ( i = 0; i <= RTL8651_PORT_NUMBER; i++ )
	{
		uint32 addrOffset_fromP0 = i * MIB_ADDROFFSETBYPORT;
		
		if ( i == RTL8651_PORT_NUMBER )
			rtlglue_printf("<CPU port (extension port included)>\n");
		else
			rtlglue_printf("<Port: %d>\n", i);

		rtlglue_printf("Rx counters\n");
		rtlglue_printf("   Rcv %llu bytes, Drop %u pkts,etherStatsDropEvents %u\n", 
			rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter(OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   CRCAlignErr %u, SymbolErr %u, FragErr %u, JabberErr %u\n",
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
		rtlglue_printf("   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));
		
		rtlglue_printf("Output counters\n");
		rtlglue_printf("   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
			rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   Broadcast %u pkts, Late collision %u, Deferred transmission %u \n",
			rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   Collisions %u Single collision %u Multiple collision %u pause %u\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));
		
	}

	rtlglue_printf("<Whole system counters>\n");
	rtlglue_printf("   CpuEvent %u pkts\n", rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCPUEVENTPKT));

	return SUCCESS;
}

int32 rtl865xC_dumpPortAsicCounter(uint32 portNum)
{
	if((portNum>=0) && (portNum<=RTL8651_PORT_NUMBER))
	{
		
		uint32 addrOffset_fromP0 = portNum * MIB_ADDROFFSETBYPORT;

		if ( portNum == RTL8651_PORT_NUMBER )
			rtlglue_printf("<CPU port (extension port included)>\n");
		else
			rtlglue_printf("<Port: %u>\n", portNum);

		rtlglue_printf("Rx counters\n");
		rtlglue_printf("   Rcv %llu bytes, Drop %u pkts, CRCAlignErr %u, FragErr %u, JabberErr %u\n",
			rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n",
			rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0),
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
		rtlglue_printf("   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));

		rtlglue_printf("Output counters\n");
		rtlglue_printf("   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
			rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   Broadcast %u pkts, Late collision %u, Deferred transmission %u \n",
			rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
		rtlglue_printf("   Collisions %u Single collision %u Multiple collision %u pause %u\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));

	}
	return 1;
}

/*
@func int32 | rtl8651_clearAsicCounter | Clear specified ASIC counter to zero
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the corresponding counter will be reset to zero and read the counter once
	for guarantee following read can get correct value. 
*/
int32 rtl8651_clearAsicSpecifiedCounter(uint32 counterIdx) {

	uint32 reg;
	rtlglue_printf("attention!this function is obsolete, please use new api:rtl8651_resetAsicMIBCounter()  or rtl8651_clearAsicCounter()\n");
	return FAILED;
	switch(counterIdx) {
		case 0:
		reg = READ_MEM32(MIB_CONTROL);
		WRITE_MEM32(MIB_CONTROL, SYS_COUNTER_RESTART);
		WRITE_MEM32(MIB_CONTROL, reg);
		break;
	default:	
		rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range

	}
	_rtl8651_initialRead();
	return SUCCESS;
}

/*
@func int32 | rtl8651_resetAsicCounterMemberPort | Clear the specified counter value and its member port
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the member port of the specified counter will be cleared to null set. 
*/
int32 rtl8651_resetAsicCounterMemberPort(uint32 counterIdx){

	rtlglue_printf("attention!this function is obsolete, please use new api:rtl8651_resetAsicMIBCounter()  or rtl8651_clearAsicCounter()\n");
	return FAILED;
	switch(counterIdx) {
		case 0:
		WRITE_MEM32(MIB_CONTROL, 0x0);
		break;
		default:	
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range

	}
	_rtl8651_initialRead();
	return SUCCESS;
}

/*
@func int32 | rtl8651_addAsicCounterMemberPort | The specified counter value add the specified port port into counter monitor member
@parm	uint32	|	counterIdx |  Specify the counter to add member port
@parm	uint32	|	port |  The added member port
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter and a valid port number, the specified port will be added to the counter coverage. 
*/
int32 rtl8651_addAsicCounterMemberPort(uint32 counterIdx, uint32 port) {
	uint32 reg, portMask;
	rtlglue_printf("attention!this function is obsolete, it shouldn't be used any more\n");
	return FAILED;

	
	if(port <RTL8651_PORT_NUMBER)
		portMask = 1<<(port + PORT_FOR_COUNTING_OFFSET);
	else if (port < RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		portMask = 1<<(port - RTL8651_PORT_NUMBER + EXT_PORT_FOR_COUNTING_OFFSET);
	else
		return FAILED;//Port number out of range
	switch(counterIdx) {
		case 0:
		reg = READ_MEM32(MIB_CONTROL) & 0x3FE00000;
		WRITE_MEM32(MIB_CONTROL, reg|portMask);
		break;
		default:	
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range
	}
	return SUCCESS;
}


/*
@func int32 | rtl8651_resetAsicCounterMemberPort | Clear the specified counter value and its member port
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the corresponding counter will be reset to zero, its member port will be cleared to null set and read the counter once
	for guarantee following read can get correct value. 
*/
int32 rtl8651_delAsicCounterMemberPort(uint32 counterIdx, uint32 port) {
	uint32 reg, portMask;
	rtlglue_printf("attention!this function is obsolete, it shouldn't be used any more\n");
	return FAILED;
	if(port <RTL8651_PORT_NUMBER)
		portMask = 1<<(port + PORT_FOR_COUNTING_OFFSET);
	else if (port < RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		portMask = 1<<(port - RTL8651_PORT_NUMBER + EXT_PORT_FOR_COUNTING_OFFSET);
	else
		return FAILED;//Port number out of range
	switch(counterIdx) {
		case 0:
		reg = READ_MEM32(MIB_CONTROL) & 0x3FE00000;
		WRITE_MEM32(MIB_CONTROL, reg&~portMask);
		break;
		default:	
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range
	}
	return SUCCESS;
}


/*
@func int32 | rtl8651_resetAsicCounterMemberPort | Clear the specified counter value and its member port
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the corresponding counter will be reset to zero, its member port will be cleared to null set and read the counter once
	for guarantee following read can get correct value. 
*/
int32 rtl8651_getAsicCounter(uint32 counterIdx, rtl865x_tblAsicDrv_basicCounterParam_t * basicCounter) {

	rtlglue_printf("attention!this function is obsolete, please use new api:rtl8651_getSimpleAsicMIBCounter()  or rtl8651_getAdvancedMIBCounter()\n");
	return FAILED;
	_rtl8651_initialRead();
	switch(counterIdx) {
		case 0:
				basicCounter->rxBytes = READ_MEM32(MIB_COUNTER_BASE);
				basicCounter->rxPackets = READ_MEM32(MIB_COUNTER_BASE+0x14) +	//Unicast
						READ_MEM32(MIB_COUNTER_BASE+0x18) + 	//Multicast
						READ_MEM32(MIB_COUNTER_BASE+0x1c);	//Broadcast
				basicCounter->rxErrors = READ_MEM32(MIB_COUNTER_BASE+0x8) +	//CRC error and Alignment error
						READ_MEM32(MIB_COUNTER_BASE+0xc) +	//Fragment error
						READ_MEM32(MIB_COUNTER_BASE+010);	//Jabber error
				basicCounter->drops = READ_MEM32(MIB_COUNTER_BASE+0x4);
				basicCounter->cpus = READ_MEM32(MIB_COUNTER_BASE+0x74);
				basicCounter->txBytes = READ_MEM32(MIB_COUNTER_BASE+0x48);
				basicCounter->txPackets = READ_MEM32(MIB_COUNTER_BASE+0x4c) +	//Unicast
						READ_MEM32(MIB_COUNTER_BASE+0x50) +	//Multicast
						READ_MEM32(MIB_COUNTER_BASE+0x54);	//Broadcast
				/*
				basicCounter->mbr = (READ_MEM32(MIB_CONTROL)&PORT_FOR_COUNTING_MASK)>>PORT_FOR_COUNTING_OFFSET | 
								((READ_MEM32(MIB_CONTROL)&EXT_PORT_FOR_COUNTING_MASK)>>EXT_PORT_FOR_COUNTING_OFFSET)<<6;
				*/
		break;
		default:
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range

	}
	return SUCCESS;
}

static int32 rtl865x_proc_mibCounter_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;

	rtl865xC_dumpAsicDiagCounter();

	return len;
}
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
/*=========================================
 * ASIC DRIVER API: Protocol-based VLAN
 *=========================================*/

/*
@func int32 | rtl8651_defineProtocolBasedVLAN | configure user-definable protocol-based VLAN
@parm uint32 | ruleNo        | Valid values: RTL8651_PBV_RULE_USR1 and RTL8651_PBV_RULE_USR2
@parm uint8  | ProtocolType  | 00:ethernetII, 01:RFC-1042, 10: LLC-Other, 11:reserved
@parm uint16 | ProtocolValue | ethernetII:ether type, RFC-1042:ether type, LLC-Other:PDSAP(8),SSAP(8)}
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_defineProtocolBasedVLAN( uint32 ruleNo, uint8 ProtocolType, uint16 ProtocolValue )
{
	ASSERT_CSP( (ruleNo > 0 && ruleNo < RTL8651_PBV_RULE_MAX) );
	
	if ( ruleNo == RTL8651_PBV_RULE_USR1 )
	{
		WRITE_MEM32( PBVCR0, ( ProtocolType << PBVCR_PROTO_TYPE_OFFSET ) |
		                    ( ProtocolValue << PBVCR_PROTO_VALUE_OFFSET ) );
	}
	else if ( ruleNo == RTL8651_PBV_RULE_USR2 )
	{
		WRITE_MEM32( PBVCR1, ( ProtocolType << PBVCR_PROTO_TYPE_OFFSET ) |
		                     ( ProtocolValue << PBVCR_PROTO_VALUE_OFFSET ) );
	}

	return SUCCESS;
}

/*
@func int32 | rtl8651_setProtocolBasedVLAN | set corresponding table index of protocol-based VLAN
@parm uint32 | ruleNo  | rule Number (1~6)
@parm uint8  | port    | 0~4:PHY  5:MII  6~8:ExtPort
@parm uint8  | vlanIdx | VLAN Table index (0~7)
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_setProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8 valid, uint16 vlanId )
{
	uint32 addr;
	uint32 value;

	ASSERT_CSP( (ruleNo > 0 && ruleNo < RTL8651_PBV_RULE_MAX) );
//	assert( vlanId < RTL865XC_VLAN_NUMBER );
//	assert( port < RTL8651_AGGREGATOR_NUMBER );
	ruleNo = ruleNo-1;
	valid = valid ? TRUE : FALSE;
	if ( valid == FALSE )
	{
		vlanId = 0; // clear it for looking pretty.
	}

	if ( port < RTL8651_MAC_NUMBER )
	{
		// Port0 ~ Port9
		addr=PBVR0_0 +(ruleNo*5*4) + ((port/2)*4) ;
		value = ( vlanId<<1 | valid );
		if (port&0x1)
			value =(value <<16) | (0x0000FFFF& READ_MEM32(addr));
		else
			value =value|( 0xFFFF0000& READ_MEM32(addr));		
		WRITE_MEM32(addr,value);
	}
	return SUCCESS;
}

/*
@func int32 | rtl8651_getProtocolBasedVLAN | get corresponding table index of protocol-based VLAN
@parm uint32 | ruleNo  | rule Number (1~6)
@parm uint8* | port    | (output) 0~4:PHY  5:MII  6~8:ExtPort
@parm uint8* | vlanIdx | (output) VLAN Table index (0~7)
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_getProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8* valid, uint32* vlanId )
{
	uint32 value;

	ASSERT_CSP( ruleNo > 0 && ruleNo < RTL8651_PBV_RULE_MAX );
	//assert( port < RTL8651_AGGREGATOR_NUMBER );
	ruleNo=ruleNo-1;
	if ( port < RTL865XC_PORT_NUMBER )
	{
		// Port0 ~ Port9		
		value =  READ_MEM32(PBVR0_0+(ruleNo*5*4) +(port/2)*4 );
		if (port&0x1)
			value =(value & 0xffff0000)>>16;
		else
			value &= 0x0000ffff ;
		if ( valid ) *valid = value & 1;
		if ( vlanId ) *vlanId = value >> 1;
	}
	
	//assert( *vlanId < RTL865XC_VLAN_NUMBER );

	return SUCCESS;
}

#endif

#if 0//defined PHYREG_DEBUG

static int32 rtl865x_proc_mibCounter_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[512];
	char		*strptr;
	char		*cmdptr;
	uint32	portNum=0xFFFFFFFF;

	if(len>512)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';

		strptr=tmpbuf;

		if(strlen(strptr)==0)
		{
			goto errout;
		}

		cmdptr = strsep(&strptr," ");
		if (cmdptr==NULL)
		{
			goto errout;
		}

		/*parse command*/
		if(strncmp(cmdptr, "clear",5) == 0)
		{
			rtl8651_clearAsicCounter();
		}
		else if(strncmp(cmdptr, "dump",4) == 0)
		{
			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}

			if(strncmp(cmdptr, "port",4) != 0)
			{
				goto errout;
			}

			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}
			portNum = simple_strtol(cmdptr, NULL, 0);


			if((portNum>=0) && (portNum<=RTL8651_PORT_NUMBER))
			{
				extern uint32 rtl8651_returnAsicCounter(uint32 offset);
				extern uint64 rtl865xC_returnAsicCounter64(uint32 offset);
				uint32 addrOffset_fromP0 = portNum * MIB_ADDROFFSETBYPORT;

				if ( portNum == RTL8651_PORT_NUMBER )
					rtlglue_printf("<CPU port (extension port included)>\n");
				else
					rtlglue_printf("<Port: %d>\n", portNum);

				rtlglue_printf("Rx counters\n");
				rtlglue_printf("   Rcv %llu bytes, Drop %u pkts, CRCAlignErr %u, FragErr %u, JabberErr %u\n",
					rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
				rtlglue_printf("   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));

				rtlglue_printf("Output counters\n");
				rtlglue_printf("   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
					rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   Broadcast %u pkts, Late collision %u, Deferred transmission %u \n",
					rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   Collisions %u Single collision %u Multiple collision %u pause %u\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));

			}
			else
			{
				goto errout;
			}
		}
		else
		{
			goto errout;
		}

	}
	else
	{
errout:
		rtlglue_printf("error input\n");
	}

	return len;
}


static int32 proc_phyReg_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return 0;
}

static int32 proc_mmd_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return 0;
}

/*
	echo read phy_id device_id reg_addr  > /proc/rtl865x/mmd
	echo write phy_id device_id reg_addr data_for_write > /proc/rtl865x/mmd
 */
static int32 proc_mmd_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 	tmpbuf[64];
	uint32	phyId, regId, regData, devId, dataReadBack;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32 	ret;

	if(len>64)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if (!memcmp(cmd_addr, "read", 4))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			devId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret = mmd_read(phyId, devId, regId, &regData);
			
			if(ret==SUCCESS)
			{
				rtlglue_printf("read phyId(%d), devId(%d), regId(%d), regData:0x%x\n", phyId,devId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			devId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			mmd_write(phyId, devId, regId, regData);

			/* confirm result */
			rtl8651_setAsicEthernetPHYReg( phyId, 13, (devId | 0x4000));
			ret=rtl8651_getAsicEthernetPHYReg(phyId, 14, &dataReadBack);

			if(ret==SUCCESS)
			{
				rtlglue_printf("extWrite phyId(%d), devId(%d), regId(%d), regData:0x%x, regData(read back):0x%x\n", 
					phyId, devId, regId, regData, dataReadBack);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
	}
	else
	{
errout:
		rtlglue_printf("error input!\n");
	}

	return len;
}

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
void getPhyByPortPage(int port, int page)
{
	uint32	regData;
	int reg;

	//change page num
	//rtl8651_setAsicEthernetPHYReg(port, 31, page);
	if (page>=31)
	{
		rtl8651_setAsicEthernetPHYReg( port, 31, 7  );
		rtl8651_setAsicEthernetPHYReg( port, 30, page  );
	}
	else if (page>0)
	{
		rtl8651_setAsicEthernetPHYReg( port, 31, page  );
	}

	for(reg=0;reg<32;reg++)
	{
		rtl8651_getAsicEthernetPHYReg( port, reg, &regData);
		rtlglue_printf("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
	}
	//if(page!=3)
	//{
		rtlglue_printf("------------------------------------------\n");
	//}

	//change back to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );

}
#if 0
static const int _8198_phy_page[] = {	0, 1, 2, 3,     		4, 5, 6, 32,
								  	33, 34, 35, 36,	40, 44, 45, 46,
								  	64, 65, 66, 69,	70, 80, 81, 161 };

int32 phyReg_read( int cmd ,uint32	phyId,uint32 regId,uint32	pageId)
{
	int i;
	uint32	phyId, pageId,regId, regData;
	int32 	ret;
	if (!memcmp(cmd_addr, "read", 4))
	{
		

		ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);
		if(ret==SUCCESS)
		{
			rtlglue_printf("read phyId(%d), regId(%d),regData:0x%x\n", phyId, regId, regData);
		}
		else
		{
			rtlglue_printf("error input!\n");
		}
	}
#ifdef CONFIG_8198_PORT5_RGMII
		else if (!memcmp(cmd_addr, "8370read", 8))
		{
			
			ret = rtl8370_getAsicReg(regId, &regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8370_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("get fail %d\n", ret);
			}
		}
#endif			
#ifdef CONFIG_RTL_8367R_SUPPORT
		else if (!memcmp(cmd_addr, "8367read", 8))
		{
			
			ret = rtl8367b_getAsicReg(regId, &regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8367b_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("get fail %d\n", ret);
			}
		}
#endif		
else if (!memcmp(cmd_addr, "extRead", 7))
		{
			
			//switch page
			if (pageId>=31)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else if (pageId>0)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
			}

			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);

			if(ret==SUCCESS)
			{
				rtlglue_printf("extRead phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId,pageId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}

			//change back to page 0
			rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);

		}
else if (!memcmp(cmd_addr, "snr", 3))
		{
			uint32 	sum;
			for (i=0;i<5;i++)
			{
				if (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp)
				{
					for (j=0, sum=0;j<10;j++)
					{
						rtl8651_getAsicEthernetPHYReg(i, 29, &regData);
						sum += regData;
						mdelay(10);
					}
					sum /= 10;
					//db = -(10 * log(sum/262144));
					//printk("  port %d SNR = %d dB\n", i, db);
					rtlglue_printf("  port %d SUM = %d\n", i, sum);
				}
				else
				{
					rtlglue_printf("  port %d is link down\n", i);
				}
			}
		}
		else if (!memcmp(cmd_addr, "dumpAll", 7))
		{
			int port;
#ifdef CONFIG_RTL_8196C
			int page,reg;
#endif
			for (port=0; port<5; port++)
			{
				rtlglue_printf("==========================================\n");

#ifdef CONFIG_RTL_8196C
				for(page=0;page<4;page++)
				{
					//change page num
					rtl8651_setAsicEthernetPHYReg(port, 31, page);
					for(reg=0;reg<32;reg++)
					{
						rtl8651_getAsicEthernetPHYReg( port, reg, &regData);
						rtlglue_printf("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
					}
					if(page!=3)
					{
						rtlglue_printf("------------------------------------------\n");
					}
					//change back to page 0
					rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
				}
#elif defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
				//set pageNum {0 1 2 3 4 5 6 32 33 34 35 36 40 44 45 46 64 65 66 69 70 80 81 161}
				for (i=0; i<24; i++)
					getPhyByPortPage(port,  _8198_phy_page[i]);
#endif
			}
		}

}

int32 phyReg_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[64];
	uint32	phyId, pageId,regId, regData;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32 	ret;
	int 		i, j;

	if(len>64)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		#if 0
		if (!memcmp(cmd_addr, "read", 4))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("read phyId(%d), regId(%d),regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		#endif
		if (!memcmp(cmd_addr, "write", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("Write phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
#ifdef CONFIG_8198_PORT5_RGMII
		#if 0
		else if (!memcmp(cmd_addr, "8370read", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			ret = rtl8370_getAsicReg(regId, &regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8370_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("get fail %d\n", ret);
			}
		}
		#endif
		else if (!memcmp(cmd_addr, "8370write", 9))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret = rtl8370_setAsicReg(regId, regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8370_setAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("set fail %d\n", ret);
			}
		}
#endif
#ifdef CONFIG_RTL_8367R_SUPPORT
		#if 0
		else if (!memcmp(cmd_addr, "8367read", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			ret = rtl8367b_getAsicReg(regId, &regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8367b_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("get fail %d\n", ret);
			}
		}
		#endif
		else if (!memcmp(cmd_addr, "8367write", 9))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret = rtl8367b_setAsicReg(regId, regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8367b_setAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("set fail %d\n", ret);
			}
		}
#endif
#if 0
		else if (!memcmp(cmd_addr, "extRead", 7))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			//switch page
			if (pageId>=31)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else if (pageId>0)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
			}

			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);

			if(ret==SUCCESS)
			{
				rtlglue_printf("extRead phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId,pageId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}

			//change back to page 0
			rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);

		}
#endif		
		else if (!memcmp(cmd_addr, "extWrite", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			//switch page
			if (pageId>=31)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else if (pageId>0)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
			}

			ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);

			if(ret==SUCCESS)
			{
				rtlglue_printf("extWrite phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId, pageId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}

			//change back to page 0
			rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
		}
		#if 0
		else if (!memcmp(cmd_addr, "snr", 3))
		{
			uint32 	sum;
			for (i=0;i<5;i++)
			{
				if (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp)
				{
					for (j=0, sum=0;j<10;j++)
					{
						rtl8651_getAsicEthernetPHYReg(i, 29, &regData);
						sum += regData;
						mdelay(10);
					}
					sum /= 10;
					//db = -(10 * log(sum/262144));
					//printk("  port %d SNR = %d dB\n", i, db);
					rtlglue_printf("  port %d SUM = %d\n", i, sum);
				}
				else
				{
					rtlglue_printf("  port %d is link down\n", i);
				}
			}
		}
		else if (!memcmp(cmd_addr, "dumpAll", 7))
		{
			int port;
#ifdef CONFIG_RTL_8196C
			int page,reg;
#endif
			for (port=0; port<5; port++)
			{
				rtlglue_printf("==========================================\n");

#ifdef CONFIG_RTL_8196C
				for(page=0;page<4;page++)
				{
					//change page num
					rtl8651_setAsicEthernetPHYReg(port, 31, page);
					for(reg=0;reg<32;reg++)
					{
						rtl8651_getAsicEthernetPHYReg( port, reg, &regData);
						rtlglue_printf("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
					}
					if(page!=3)
					{
						rtlglue_printf("------------------------------------------\n");
					}
					//change back to page 0
					rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
				}
#elif defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
				//set pageNum {0 1 2 3 4 5 6 32 33 34 35 36 40 44 45 46 64 65 66 69 70 80 81 161}
				for (i=0; i<24; i++)
					getPhyByPortPage(port,  _8198_phy_page[i]);
#endif
			}
		}
	#endif	
	}
	else
	{
errout:
		rtlglue_printf("error input!\n");
	}

	return len;
}
#endif
//MACCR--------------------------------------------------------------------------------------------
static int32 mac_config_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	
	rtlglue_printf("MAC Configuration Register Info:\n");
	regData = READ_MEM32(MACCR);
	
	data0 = regData & IPG_SEL;
	rtlglue_printf( "IfgSel:  ");
	if (data0)
		rtlglue_printf("internal counter =352\n");
	else
		rtlglue_printf("internal counter =480\n");
	
	data0 = regData & INFINITE_PAUSE_FRAMES  ;
	rtlglue_printf("INFINITE_PAUSE_FRAMES:  ");
	if (data0)
		rtlglue_printf("Infinite pause frame count\n");
	else
		rtlglue_printf("Maximum of 128 consecutive pause frames\n");

	data0= regData & LONG_TXE;
	rtlglue_printf("LONG_TXE:  ");
	if (data0)
		rtlglue_printf("Carrier-based backpressure\n");
	else
		rtlglue_printf("Collision-based backpressure\n");

	data0= regData & EN_48_DROP;
	rtlglue_printf("EN_48_DROP:  ");
	if (data0)
		rtlglue_printf("Enabled\n");
	else
		rtlglue_printf("Disabled\n");
	
	data0= (regData & SELIPG_MASK)>>SELIPG_OFFSET;
	rtlglue_printf("SELIPG:  ");
	if(data0==0x00)
		rtlglue_printf("7 byte-time\n");
	else if(data0==0x01)
		rtlglue_printf("8 byte-time\n");
	else if(data0==0x10)
		rtlglue_printf("10 byte-time\n");
	else
		rtlglue_printf("12 byte-time\n");
	
	data0= (regData & CF_SYSCLK_SEL_MASK)>>CF_SYSCLK_SEL_OFFSET;
	rtlglue_printf("CF_SYSCLK_SEL:  ");
	if(data0==0x00)
		rtlglue_printf("50MHz\n");
	else if(data0==0x01)
		rtlglue_printf("100MHz\n");
	else
		rtlglue_printf("reserved status\n");

	data0= (regData & CF_FCDSC_MASK)>>CF_FCDSC_OFFSET;
	rtlglue_printf("CF_FCDSC:  %d pages\n",data0);

	data0= (regData & CF_RXIPG_MASK);
	rtlglue_printf("CF_RXIPG:  %d pkts\n",data0);
	
	return len;

}



//FC threshold--------------------------------------------------------------------------
static int32 fc_threshold_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{

	int 	len;
	uint32	regData;
	uint32	data0;
	int port;	
	int group=3;
	len = sprintf(page, "Dump FC threshold Information:\n");

	//SBFCR0
	regData = READ_MEM32(SBFCR0);
	data0 = regData & S_DSC_RUNOUT_MASK;
	len += sprintf(page+len, "S_DSC_RUNOUT:%d\n",data0);
	//SBFCR1
	regData = READ_MEM32(SBFCR1);
	data0 = (regData & SDC_FCOFF_MASK)>>SDC_FCOFF_OFFSET;
	len += sprintf(page+len, "SDC_FCOFF:%d, ",data0);
	data0 = (regData & SDC_FCON_MASK)>>SDC_FCON_OFFSET;
	len += sprintf(page+len, "SDC_FCON:%d\n",data0);
	//SBFCR2
	regData = READ_MEM32(SBFCR2);
	data0 = (regData & S_Max_SBuf_FCOFF_MASK)>>S_Max_SBuf_FCOFF_OFFSET;
	len += sprintf(page+len, "S_MaxSBuf_FCON:%d, ",data0);
	data0 = (regData & S_Max_SBuf_FCON_MASK)>>S_Max_SBuf_FCON_OFFSET;
	len += sprintf(page+len, "S_MaxSBuf_FCOFF:%d\n",data0);
	//FCCR0,FCCR1
	regData = READ_MEM32(FCCR0);
	data0 =(regData & Q_P0_EN_FC_MASK)>>(Q_P0_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P0_EN_FC:%0x, ",data0);	
	data0 =(regData & Q_P1_EN_FC_MASK)>>(Q_P1_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P1_EN_FC:%0x, ",data0);	
	data0 =(regData & Q_P2_EN_FC_MASK)>>(Q_P2_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P2_EN_FC:%0x\n",data0);
	data0 =(regData & Q_P3_EN_FC_MASK)>>(Q_P3_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P3_EN_FC:%0x, ",data0);
	regData = READ_MEM32(FCCR1);
	data0 =(regData & Q_P4_EN_FC_MASK)>>(Q_P4_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P4_EN_FC:%0x, ",data0);	
	regData = READ_MEM32(FCCR0);
	data0 =(regData & Q_P5_EN_FC_MASK)>>(Q_P5_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P5_EN_FC:%0x\n",data0);	
	//PQPLGR
	regData = READ_MEM32(PQPLGR);
	data0 =regData & QLEN_GAP_MASK;
	len += sprintf(page+len, "QLEN_GAP:%d\n",data0);
	//QRR
	regData = READ_MEM32(QRR);
	data0 =regData & QRST;
	len += sprintf(page+len, "QRST:%d\n",data0);
	//IQFCTCR
	regData = READ_MEM32(IQFCTCR);
	data0 =(regData & IQ_DSC_FCON_MASK)>>IQ_DSC_FCON_OFFSET;
	len += sprintf(page+len, "IQ_DSC_FCON:%d, ",data0);
	data0 =(regData & IQ_DSC_FCOFF_MASK)>>IQ_DSC_FCOFF_OFFSET;
	len += sprintf(page+len, "IQ_DSC_FCOFF:%d\n",data0);
	//QNUMCR
	regData = READ_MEM32(QNUMCR);
	len += sprintf(page+len,"The number of output queue for port(0~6) :\n");
	data0=(regData & P0QNum_MASK )>>(P0QNum_OFFSET);
	len += sprintf(page+len, "P0QNum%d, ",data0);
	data0=(regData & P1QNum_MASK )>>(P1QNum_OFFSET);
	len += sprintf(page+len, "P1QNum%d, ",data0);
	data0=(regData & P2QNum_MASK )>>(P2QNum_OFFSET);
	len += sprintf(page+len, "P2QNum%d, ",data0);
	data0=(regData & P3QNum_MASK )>>(P3QNum_OFFSET);
	len += sprintf(page+len, "P3QNum%d, ",data0);
	data0=(regData & P4QNum_MASK )>>(P4QNum_OFFSET);
	len += sprintf(page+len, "P4QNum%d, ",data0);
	data0=(regData & P5QNum_MASK )>>(P5QNum_OFFSET);
	len += sprintf(page+len, "P5QNum%d, ",data0);
	data0=(regData & P6QNum_MASK )>>(P6QNum_OFFSET);
	len += sprintf(page+len, "P6QNum%d\n",data0);
	//per port   
	for(port=PHY0;port<=CPU;port++)
	{
		if (port==CPU)
		len += sprintf(page+len, "\nCPUPort\n");
		else
		len += sprintf(page+len, "\nPort%d\n", port);
		
		regData = READ_MEM32(PBFCR0+((port)<<2));
		data0 = (regData & P_MaxDSC_FCOFF_MASK)>>P_MaxDSC_FCOFF_OFFSET;
		len += sprintf(page+len, "   P_MaxDSC_FCOFF:%d, ",data0);
		data0 = (regData & P_MaxDSC_FCON_MASK)>>P_MaxDSC_FCON_OFFSET;
		len += sprintf(page+len, "P_MaxDSC_FCON:%d\n",data0);
		//if(port<CPU)
		{
			for (group=GR0;group<=GR2;group++)
			{
				len += sprintf(page+len, "   Port%dGroup%d\n",port,group);
				/* QDBFCRP0G0,QDBFCRP0G1,QDBFCRP0G2
				 * QDBFCRP1G0,QDBFCRP1G1,QDBFCRP1G2
				 * QDBFCRP2G0,QDBFCRP2G1,QDBFCRP2G2
				 * QDBFCRP3G0,QDBFCRP3G1,QDBFCRP3G2
				 * QDBFCRP4G0,QDBFCRP4G1,QDBFCRP4G2
				 * QDBFCRP5G0,QDBFCRP5G1,QDBFCRP5G2
				 * - Queue-Descriptor=Based Flow Control Threshold for Port 0 Group 0 */
				regData =READ_MEM32(QDBFCRP0G0+((port)<<2)+((group)<<2));
				//len+=sprintf(page+len,"address:%0x",(QDBFCRP0G0+((port)<<2)+((group)<<2)));
				data0 = (regData & QG_DSC_FCOFF_MASK)>>QG_DSC_FCOFF_OFFSET;
				len += sprintf(page+len, "   QG_DSC_FCOFF:%d, ",data0);
				data0 = (regData & QG_DSC_FCON_MASK)>>QG_DSC_FCON_OFFSET;
				len += sprintf(page+len, "QG_DSC_FCON:%d, ",data0);

				/* QPKTFCRP0G0,QPKTFCRP0G1,QPKTFCRP0G2
				 * QPKTFCRP1G0,QPKTFCRP1G1,QPKTFCRP1G2
				 * QPKTFCRP2G0,QPKTFCRP2G1,QPKTFCRP2G2
				 * QPKTFCRP3G0,QPKTFCRP3G1,QPKTFCRP3G2
				 * QPKTFCRP4G0,QPKTFCRP4G1,QPKTFCRP4G2
				 * QPKTFCRP5G0,QPKTFCRP5G1,QPKTFCRP5G2
				   - Queue-Packet-Based Flow Control Register for Port 0 Group 0 */
				regData =READ_MEM32(QPKTFCRP0G0+((port)<<2)+((group)<<2));
				//len+=sprintf(page+len,"address:%0x",(QPKTFCRP0G0+((port)<<2)+((group)<<2)));
				data0 = (regData & QG_QLEN_FCOFF_MASK)>>QG_QLEN_FCOFF_OFFSET;
				len += sprintf(page+len, "QG_QLEN_FCOFF:%d, ",data0);
				data0 = (regData & QG_QLEN_FCON_MASK)>>QG_QLEN_FCON_OFFSET;
				len += sprintf(page+len, "QG_QLEN_FCON:%d\n",data0);
				
			}	
		}
	}

	return len;
}

static int32 fc_threshold_write(struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#endif
#endif
#endif
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)//defined (CONFIG_RTL_LAYERED_DRIVER_L3)


int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
	return swTable_addEntry(tableType, eidx, entryContent_P);

}
inline unsigned int bitReverse(unsigned int x)
{
  unsigned int y = 0x55555555;
  x = (((x >> 1) & y) | ((x & y) << 1));
  y = 0x33333333;
  x = (((x >> 2) & y) | ((x & y) << 2));
  y = 0x0f0f0f0f;
  x = (((x >> 4) & y) | ((x & y) << 4));
  y = 0x00ff00ff;
  x = (((x >> 8) & y) | ((x & y) << 8));
  return((x >> 16) | (x << 16));
}
/*=========================================
  * ASIC DRIVER API: Multicast Table
  *=========================================*/
static uint32 currHashMethod = 0; /* init hash method as 0 */
uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr)
{
	uint32 idx;
	#if defined CONFIG_RTL_819X

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8881A)
		currHashMethod = ( REG32(FFCR) & 0x00000060) >> 5; 
		
		if (currHashMethod == HASH_METHOD_SIP) 
		{
			idx = srcAddr & 0xff;
		}
		else if (currHashMethod == HASH_METHOD_DIP) 
		{
			idx = dstAddr & 0xff;
		}
		else
		{
        #if 0
			uint32 sip[32],dip[32];
			uint32 hash[8];
			uint32 i;
	
			for(i=0; i<8; i++) {
				hash[i]=0;
			}
	
			for(i=0; i<32; i++) {
				if((srcAddr & (1<<i))!=0) {
					sip[i]=1;
				}
				else	{
					sip[i]=0;
				}
	
				if((dstAddr & (1<<i))!=0) {
					dip[i]=1;
				}
				else {
					dip[i]=0;
				}			
			}
	
			hash[7] = sip[0] ^ sip[8]	^ sip[16] ^ sip[24] ^ dip[7] ^ dip[15] ^ dip[23] ^ dip[31];
			hash[6] = sip[1] ^ sip[9]	^ sip[17] ^ sip[25] ^ dip[6] ^ dip[14] ^ dip[22] ^ dip[30];
			hash[5] = sip[2] ^ sip[10] ^ sip[18] ^ sip[26] ^ dip[5] ^ dip[13] ^ dip[21] ^ dip[29];
			hash[4] = sip[3] ^ sip[11] ^ sip[19] ^ sip[27] ^ dip[4] ^ dip[12] ^ dip[20] ^ dip[28];
			hash[3] = sip[4] ^ sip[12] ^ sip[20] ^ sip[28] ^ dip[3] ^ dip[11] ^ dip[19] ^ dip[27];
			hash[2] = sip[5] ^ sip[13] ^ sip[21] ^ sip[29] ^ dip[2] ^ dip[10] ^ dip[18] ^ dip[26];
			hash[1] = sip[6] ^ sip[14] ^ sip[22] ^ sip[30] ^ dip[1] ^ dip[9]  ^ dip[17] ^ dip[25];
			hash[0] = sip[7] ^ sip[15] ^ sip[23] ^ sip[31] ^ dip[0] ^ dip[8]  ^ dip[16] ^ dip[24];
	
			for(i=0; i<8; i++) {
				hash[i]=hash[i] & (0x01);
			}
	
			idx=0;
			for(i=0; i<8; i++) {
				idx=idx+(hash[i]<<i);
			}
        #else
			uint32 temp;
			uint8 *tempPtr;
			uint32 newSrcAddr = bitReverse(srcAddr);
			
			temp = newSrcAddr^dstAddr;
			tempPtr = (uint8 *)(&temp);
			idx = (uint32)(tempPtr[0]^tempPtr[1]^tempPtr[2]^tempPtr[3]);
        #endif
			
		}

#elif defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD)
    currHashMethod = ( REG32(FFCR) & 0x00000060) >> 5; /* Frame Forwarding Configuration Register bit:5~6 */
    
    if (currHashMethod == HASH_METHOD_SIP) /* hash method:  SIP[6:0] */
    {
        idx = srcAddr & 0x7f;
    }
    else if (currHashMethod == HASH_METHOD_DIP) /* hash method:  DIP[6:0] */
    {
        idx = dstAddr & 0x7f;
    }
    else
    {
        uint32 sip[32],dip[32];
    	uint32 hash[7];
    	uint32 i;

    	for(i=0; i<7; i++) {
    		hash[i]=0;
    	}

    	for(i=0; i<32; i++)	{
    		if((srcAddr & (1<<i))!=0) {
    			sip[i]=1;
    		}
    		else 	{
    			sip[i]=0;
    		}

    		if((dstAddr & (1<<i))!=0) {
    			dip[i]=1;
    		}
    		else {
    			dip[i]=0;
    		}			
    	}

    	hash[0] = sip[0] ^ sip[7]   ^ sip[14] ^ sip[21] ^ sip[28] ^ dip[1] ^ dip[8]   ^ dip[15] ^ dip[22] ^ dip[29];
    	hash[1] = sip[1] ^ sip[8]   ^ sip[15] ^ sip[22] ^ sip[29] ^ dip[2] ^ dip[9]   ^ dip[16] ^ dip[23] ^ dip[30];
    	hash[2] = sip[2] ^ sip[9]   ^ sip[16] ^ sip[23] ^ sip[30] ^ dip[3] ^ dip[10] ^ dip[17] ^ dip[24] ^ dip[31];
    	hash[3] = sip[3] ^ sip[10] ^ sip[17] ^ sip[24] ^ sip[31] ^ dip[4] ^ dip[11] ^ dip[18] ^ dip[25];
    	hash[4] = sip[4] ^ sip[11] ^ sip[18] ^ sip[25]               ^ dip[5] ^ dip[12] ^ dip[19] ^ dip[26];
    	hash[5] = sip[5] ^ sip[12] ^ sip[19] ^ sip[26]               ^ dip[6] ^ dip[13] ^ dip[20] ^ dip[27];
    	hash[6] = sip[6] ^ sip[13] ^ sip[20] ^ sip[27]   ^ dip[0] ^ dip[7] ^ dip[14] ^ dip[21] ^ dip[28];

    	for(i=0; i<7; i++) {
    		hash[i]=hash[i] & (0x01);
    	}

    	idx=0;
    	for(i=0; i<7; i++) {
    		idx=idx+(hash[i]<<i);
    	}
    }
	return idx;
#else
	{
		#if 0
		uint32 sip[32],dip[32];
		uint32 hash[6];
		uint32 i;
		uint32 bitmask;

		for(i=0; i<32; i++)
		{
			bitmask=1<<i;
			if((srcAddr & bitmask)!=0)
			{
				sip[i]=1;
			}
			else
			{
				sip[i]=0;
			}

			if((dstAddr & bitmask)!=0)
			{
				dip[i]=1;
			}
			else
			{
				dip[i]=0;
			}
			
		}

		hash[0] = dip[0]^dip[6]^dip[12]^dip[18]^dip[24]^dip[26]^dip[28]^dip[30]^
	         		sip[23]^sip[5]^sip[11]^sip[17]^sip[31]^sip[25]^sip[27]^sip[29];
		hash[1] = dip[1]^dip[7]^dip[13]^dip[19]^dip[25]^dip[27]^dip[29]^dip[31]^
		         	sip[0]^sip[6]^sip[12]^sip[18]^sip[24]^sip[26]^sip[28]^sip[30];
		hash[2] = dip[2]^dip[8]^dip[14]^dip[20]^sip[1]^sip[7]^sip[13]^sip[19];
		hash[3] = dip[3]^dip[9]^dip[15]^dip[21]^sip[2]^sip[8]^sip[14]^sip[20];
		hash[4] = dip[4]^dip[10]^dip[16]^dip[22]^sip[3]^sip[9]^sip[15]^sip[21];
		hash[5] = dip[5]^dip[11]^dip[17]^dip[23]^sip[4]^sip[10]^sip[16]^sip[22];

		idx=0;
		for(i=0; i<6; i++)
		{
			hash[i]=hash[i] & (0x01);
			idx=idx+(hash[i]<<i);
		}
		#else
		uint32 hashSrcAddr;
		uint32 hashDstAddr;		
		hashDstAddr=((dstAddr ^ (dstAddr>>6) ^ (dstAddr>>12) ^ (dstAddr>>18)) ) ^ 
					(((dstAddr>>24) ^ (dstAddr>>26) ^ (dstAddr>>28) ^ (dstAddr>>30) ) & 0x03);
		
		hashSrcAddr=(	( 	(((srcAddr>>23) ^(srcAddr>>31)) & 0x01) 	^ 
							((srcAddr ^ (srcAddr >>24))<<1) 			^
							(srcAddr>>5) 	^
							(srcAddr>>11) 	^ 
							(srcAddr>>17) 	^ 
							(srcAddr>>25) 	^ 
							(srcAddr>>27) 	^ 
							(srcAddr>>29)	)	& 0x03	
					) 	|
					 (	(	(srcAddr>>1) 	^
					 		(srcAddr>>7) 	^
					 		(srcAddr>>13) 	^
					 		(srcAddr>>19)	)	<<2
					 );
					
					
		idx= (hashDstAddr ^ hashSrcAddr) & 0x3F; 
		#endif
	}
#endif
	#else
	#error "wrong compile flag, not supported IC reversion"
	{
		idx = srcAddr ^ (srcAddr>>8) ^ (srcAddr>>16) ^ (srcAddr>>24) ^ dstAddr ^ (dstAddr>>8) ^ (dstAddr>>16) ^ (dstAddr>>24);
		idx = ((idx >> 2) ^ (idx & 0x3)) & (RTL8651_IPMULTICASTTBL_SIZE-1);
	}
	#endif
	return idx;
}

int32 rtl8651_setAsicIpMulticastTable(rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) {
	uint32 idx;
 	rtl865xc_tblAsic_ipMulticastTable_t entry;
	int16 age;

	if(mCast_t->dip >>28 != 0xe || mCast_t->port >= RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER)//rtl8651_totalExtPortNum
		return FAILED;//Non-IP multicast destination address
	memset(&entry,0,sizeof(entry));
	entry.srcIPAddr 		= mCast_t->sip;
	entry.destIPAddrLsbs 	= mCast_t->dip & 0xfffffff;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	idx = mCast_t->idx;
#else
	idx = rtl8651_ipMulticastTableIndex(mCast_t->sip, mCast_t->dip);
#endif

#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
	entry.srcPort 			= mCast_t->port;
	entry.portList 			= mCast_t->mbr;

#else

	if (mCast_t->port >= RTL8651_PORT_NUMBER) {
		/* extension port */
		entry.srcPortExt = 1;
		entry.srcPort 			= (mCast_t->port-RTL8651_PORT_NUMBER);

	} else {
		entry.srcPortExt = 0;
		entry.srcPort 			= mCast_t->port;

	}

	entry.extPortList 		= mCast_t->mbr >> RTL8651_PORT_NUMBER;
	entry.srcVidH 			= ((mCast_t->svid)>>4) &0xff;
	entry.srcVidL 			= (mCast_t->svid)&0xf;
	entry.portList 			= mCast_t->mbr & (RTL8651_PHYSICALPORTMASK);
#endif
	entry.toCPU 			= 0;
	entry.valid 			= 1;
	entry.extIPIndex 		= mCast_t->extIdx;
	
	entry.ageTime			= 0;
	
	age = (int16)mCast_t->age;
#if 0
	while ( age > 0 ) {
		if ( (++entry.ageTime) == 7)
			break;
		age -= 5;
	}
#endif
	entry.ageTime = 7;

	
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
}

int32 rtl8651_delAsicIpMulticastTable(uint32 index) {
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
}
#if 1
int32 _rtl8651_readAsicEntryStopTLU(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	uint32 *entryAddr;
	uint32 tmp;/* dummy variable, don't remove it */

	ASSERT_CSP(entryContent_P);
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase(tableType) + (eidx<<5 /*RTL8651_ASICTABLE_ENTRY_LENGTH*/) ) ;
		/*(uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);*/
#if 0
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
#endif	

	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		#if 0
		//while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
		#endif	
	}

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
		
#if defined(CONFIG_RTL_8197F)
		if(index<8)
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
		else
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index-8+8192));
#else
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
#endif
		}
	}
#else
	*((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
	*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
	*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
	*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
	*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
	*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
	*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
	*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
	
#if defined(CONFIG_RTL_8197F)
		if(_rtl8651_asicTableSize[tableType]>8)
		{
			*((uint32 *)entryContent_P + 8) = *(entryAddr + 8192);
			*((uint32 *)entryContent_P + 9) = *(entryAddr + 8193);
			*((uint32 *)entryContent_P + 10) = *(entryAddr + 8194);
		}
#endif
#endif


	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}

	/* Dummy read. Must read an un-used table entry to refresh asic latch */
	tmp = *(uint32 *)((uint32) rtl8651_asicTableAccessAddrBase(TYPE_ACL_RULE_TABLE) + 1024 * RTL8651_ASICTABLE_ENTRY_LENGTH);

	return 0;
}
#endif
int32 rtl8651_getAsicIpMulticastTable(uint32 index, rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) 
{	
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	
	if (mCast_t == NULL)
		return FAILED;
   	//_rtl8651_readAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
	_rtl8651_readAsicEntryStopTLU(TYPE_MULTICAST_TABLE, index, &entry);

 	mCast_t->sip	= entry.srcIPAddr;
	if(entry.valid)
	{
 		mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;
	}
	else
	{
		if(entry.destIPAddrLsbs==0)
		{
			mCast_t->dip	= entry.destIPAddrLsbs;
		}
		else
		{
			mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;
		}
	}

#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
	mCast_t->svid = 0;
	mCast_t->port = entry.srcPort;
	mCast_t->mbr = entry.portList;
#else
	mCast_t->svid = (entry.srcVidH<<4) | entry.srcVidL;
#if 1
//#ifdef CONFIG_RTL8650BBASIC
#if 1
	if (entry.srcPortExt) {
		mCast_t->port = entry.srcPort + RTL8651_PORT_NUMBER;
	} else {
		mCast_t->port = entry.srcPort;
	}
#else
	mCast_t->port = entry.srcPortExt<<3 | entry.srcPortH<<1 | entry.srcPortL;
#endif
	mCast_t->mbr = entry.extPortList<<RTL8651_PORT_NUMBER | entry.portList;
#else
	mCast_t->port = entry.srcPortH<<1 | entry.srcPortL;
	mCast_t->mbr = entry.portList;
#endif
#endif
	mCast_t->extIdx = entry.extIPIndex ;
	mCast_t->age	= entry.ageTime * 5;
	mCast_t->cpu = entry.toCPU;
	
	if(entry.valid == 0)
	{
		return FAILED;

	}

	return SUCCESS;
	
}


/*
@func int32		|	rtl8651_setAsicMulticastEnable 	| Enable / disable ASIC IP multicast support.
@parm uint32		|	enable	| TRUE to indicate ASIC IP multicast process is enabled; FALSE to indicate ASIC IP multicast process is disabled.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
We would use this API to enable/disable ASIC IP multicast process.
If it's disabled here, Hardware IP multicast table would be ignored.
If it's enabled here, IP multicast table is used to forwarding IP multicast packets.
 */
int32 rtl8651_setAsicMulticastEnable(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(FFCR, READ_MEM32(FFCR)|EN_MCAST);
	} else
	{
		WRITE_MEM32(FFCR, READ_MEM32(FFCR) & ~EN_MCAST);
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_getAsicMulticastEnable 	| Get the state about ASIC IP multicast support.
@parm uint32*		|	enable	| Pointer to store the state about ASIC IP multicast support.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
We would use this API to get the status of ASIC IP multicast process.
TRUE to indicate ASIC IP multicast process is enabled; FALSE to indicate ASIC IP multicast process is disabled.
*/
int32 rtl8651_getAsicMulticastEnable(uint32 *enable)
{
	if (enable == NULL)
	{
		return FAILED;
	}

	*enable = (READ_MEM32(FFCR) & EN_MCAST) ? TRUE : FALSE;

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastPortInternal 	| Configure internal/external state for each port
@parm uint32		|	port		| Port to set its state.
@parm int8		|	isInternal	| set TRUE to indicate <p port> is internal port; set FALSE to indicate <p port> is external port
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
In RTL865x platform,
user would need to config the Internal/External state for each port to support HW multicast NAPT.
If packet is sent from internal port to external port, and source VLAN member port checking indicates that L34 is needed.
Source IP modification would be applied.
 */
int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal)
{
	if (port >= RTL8651_PORT_NUMBER + RTL8651_EXTPORT_NUMBER)//rtl8651_totalExtPortNum
	{	/* Invalid port */
		return FAILED;
	}

	/*
		RTL865XC : All multicast mode are stored in [SWTCR0 / Switch Table Control Register 0]
	*/
	if (isInternal == TRUE)
	{
		WRITE_MEM32(SWTCR0, READ_MEM32(SWTCR0) | (((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET));
	} else
	{
		WRITE_MEM32(SWTCR0, READ_MEM32(SWTCR0) & ~(((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET));
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_getAsicMulticastPortInternal 	| Get internal/external state for each port
@parm uint32		|	port		| Port to set its state.
@parm int8*		|	isInternal	| Pointer to get port state of <p port>.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To get the port internal / external state for <p port>:
TRUE to indicate <p port> is internal port; FALSE to indicate <p port> is external port
 */
int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal)
{
	if (isInternal == NULL)
	{
		return FAILED;
	}

	if (port >= RTL8651_PORT_NUMBER + RTL8651_EXTPORT_NUMBER)//rtl8651_totalExtPortNum
	{	/* Invalid port */
		return FAILED;
	}

	if (READ_MEM32(SWTCR0) & (((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET))
	{
		*isInternal = TRUE;
	} else
	{
		*isInternal = FALSE;
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastMTU 	| Set MTU for ASIC IP multicast forwarding
@parm uint32		|	mcastMTU	| MTU used by HW IP multicast forwarding.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To set the MTU for ASIC IP multicast forwarding.
Its independent from other packet forwarding because IP multicast would include L2/L3/L4 at one time.
*/
int32 rtl8651_setAsicMulticastMTU(uint32 mcastMTU)
{
	if (mcastMTU & ~(MultiCastMTU_MASK) )
	{	/* multicast MTU overflow */
		return FAILED;
	}

	UPDATE_MEM32(ALECR, mcastMTU, MultiCastMTU_MASK, MultiCastMTU_OFFSET);

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastMTU 	| Get MTU for ASIC IP multicast forwarding
@parm uint32*	|	mcastMTU	| Pointer to get MTU used by HW IP multicast forwarding.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To get the MTU value for ASIC IP multicast forwarding.
Its independent from other packet forwarding because IP multicast would include L2/L3/L4 at one time.
*/
int32 rtl8651_getAsicMulticastMTU(uint32 *mcastMTU)
{
	if (mcastMTU == NULL)
	{
		return FAILED;
	}

	*mcastMTU = GET_MEM32_VAL(ALECR, MultiCastMTU_MASK, MultiCastMTU_OFFSET);

	return SUCCESS;
}

int32 rtl865x_setAsicMulticastAging(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR) & ~IPMcastAgingDisable);
		
	} else
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR)|IPMcastAgingDisable);
	}

	return SUCCESS;
}


int32 rtl8651_getAsicAgingFunction(int8 * l2Enable, int8 * l4Enable) {
	if(l2Enable == NULL || l4Enable == NULL)
		return FAILED;

	*l2Enable = (READ_MEM32(TEACR) & 0x1)? FALSE: TRUE;
	*l4Enable = (READ_MEM32(TEACR) & 0x2)? FALSE: TRUE;
	return SUCCESS;
}

/*
@func int32		| rtl8651_setAsicSpanningEnable 	| Enable/disable ASIC spanning tree support
@parm int8		| spanningTreeEnabled | TRUE to indicate spanning tree is enabled; FALSE to indicate spanning tree is disabled.
@rvalue SUCCESS	| 	Success
@comm
Global switch to enable or disable ASIC spanning tree support.
If ASIC spanning tree support is enabled, further configuration would be refered by ASIC to prcoess packet forwarding / MAC learning.
If ASIC spanning tree support is disabled, all MAC learning and packet forwarding would be done regardless of port state.
Note that the configuration does not take effect for spanning tree BPDU CPU trapping. It is set in <p rtl8651_setAsicResvMcastAddrToCPU()>.
@xref <p rtl8651_setAsicMulticastSpanningTreePortState()>, <p rtl865xC_setAsicSpanningTreePortState()>, <p rtl8651_getAsicMulticastSpanningTreePortState()>, <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_setAsicSpanningEnable(int8 spanningTreeEnabled)
{
	if(spanningTreeEnabled == TRUE)
	{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_STP));
		WRITE_MEM32(RMACR ,READ_MEM32(RMACR)|MADDR00);

	}else
	{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_STP));
		WRITE_MEM32(RMACR, READ_MEM32(RMACR)&~MADDR00);

	}
	return SUCCESS;
}

/*
@func int32		| rtl8651_getAsicSpanningEnable 	| Getting the ASIC spanning tree support status
@parm int8*		| spanningTreeEnabled | The pointer to get the status of ASIC spanning tree configuration status.
@rvalue FAILED	| 	Failed
@rvalue SUCCESS	| 	Success
@comm
Get the ASIC global switch to enable or disable ASIC spanning tree support.
The switch can be set by calling <p rtl8651_setAsicSpanningEnable()>
@xref <p rtl8651_setAsicSpanningEnable()>, <p rtl8651_setAsicMulticastSpanningTreePortState()>, <p rtl865xC_setAsicSpanningTreePortState()>, <p rtl8651_getAsicMulticastSpanningTreePortState()>, <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_getAsicSpanningEnable(int8 *spanningTreeEnabled)
{
	if(spanningTreeEnabled == NULL)
		return FAILED;
	*spanningTreeEnabled = (READ_MEM32(MSCR)&(EN_STP)) == (EN_STP)? TRUE: FALSE;
	return SUCCESS;
}

/*
@func int32		| rtl865xC_setAsicSpanningTreePortState 	| Configure Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | Spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
Config IEEE 802.1D spanning tree port sate into ASIC.
 */
int32 rtl865xC_setAsicSpanningTreePortState(uint32 port, uint32 portState)
{
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER )
		return FAILED;

	switch(portState)
	{
		case RTL8651_PORTSTA_DISABLED:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_DISABLE );
			break;
		case RTL8651_PORTSTA_BLOCKING:
		case RTL8651_PORTSTA_LISTENING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_BLOCKING );
			break;
		case RTL8651_PORTSTA_LEARNING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_LEARNING );
			break;
		case RTL8651_PORTSTA_FORWARDING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_FORWARDING );
			break;
		default:
			return FAILED;
	}

	//TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset, EnForceMode);
	return SUCCESS;
}

/*
@func int32		| rtl865xC_getAsicSpanningTreePortState 	| Retrieve Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | pointer to memory to store the port state
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
Possible spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
 */
int32 rtl865xC_getAsicSpanningTreePortState(uint32 port, uint32 *portState)
{
	uint32 reg;
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER || portState == NULL )
		return FAILED;

	reg = ( READ_MEM32( PCRP0 + offset ) & STP_PortST_MASK );

	switch(reg)
	{
		case STP_PortST_DISABLE:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case STP_PortST_BLOCKING:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case STP_PortST_LEARNING:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case STP_PortST_FORWARDING:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;

}

/*
@func int32		| rtl8651_setAsicMulticastSpanningTreePortState 	| Configure Multicast Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | Spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
In RTL865xC platform, Multicast spanning tree configuration is set by this API.
@xref  <p rtl865xC_setAsicSpanningTreePortState()>
 */
int32 rtl8651_setAsicMulticastSpanningTreePortState(uint32 port, uint32 portState)
{
#if 0	//Note: 96C/98 have remove these bits!!!
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER )
	{
		return FAILED;
	}

	switch(portState)
	{
		case RTL8651_PORTSTA_DISABLED:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_DISABLE );
			break;
		case RTL8651_PORTSTA_BLOCKING:
		case RTL8651_PORTSTA_LISTENING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_BLOCKING );
			break;
		case RTL8651_PORTSTA_LEARNING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_LEARNING );
			break;
		case RTL8651_PORTSTA_FORWARDING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_FORWARDING );
			break;
		default:
			return FAILED;
	}

	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
#endif

	return SUCCESS;
}

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
#if defined (CONFIG_RTL_8197F)
int32 rtl819x_setAsicMulticastv6Enable(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(IPV6CR0, READ_MEM32(IPV6CR0)|EN_MCASTv6);
	} 
	else
	{
		WRITE_MEM32(IPV6CR0, READ_MEM32(IPV6CR0) & ~EN_MCASTv6);
	}
	return SUCCESS;
}

int32 rtl819x_getAsicMulticastv6Enable(uint32 *enable)
{
	if (enable == NULL)
	{
		return FAILED;
	}

	*enable = (READ_MEM32(IPV6CR0) & EN_MCASTv6) ? TRUE : FALSE;

	return SUCCESS;
}

int32 rtl819x_getAsicMulticastv6MTU(uint32 *mcastMTU)
{
	if (mcastMTU == NULL)
	{
		return FAILED;
	}

	*mcastMTU = GET_MEM32_VAL(IPV6CR0, MultiCastv6MTU_MASK, MultiCastv6MTU_OFFSET);

	return SUCCESS;
}
int32 rtl819x_setAsicMulticastv6MTU(uint32 mcastMTU)
{
	if (mcastMTU & ~(MultiCastv6MTU_MASK) )
	{	/* multicast MTU overflow */
		return FAILED;
	}

	UPDATE_MEM32(IPV6CR0, mcastMTU, MultiCastv6MTU_MASK, MultiCastv6MTU_OFFSET);

	return SUCCESS;
}
int32 rtl819x_getAsicMCastv6HashMethod(unsigned int *hashMethod)
{
	if(hashMethod==NULL)
	{
		return FAILED;
	}
	*hashMethod = (READ_MEM32(IPV6CR0) & 0x60000)>>17;
	return SUCCESS;
}
int32 rtl819x_setAsicMCastv6HashMethod(unsigned int hashMethod)//to be checked
{
	uint32	  oldHashMethod = 0;
	oldHashMethod = (READ_MEM32(IPV6CR0) & 0x60000)>>17;
	currHashMethod = hashMethod; 
	if(hashMethod >3)
	{
		return FAILED;
	}
			
	hashMethod=hashMethod&0x3;
					
	if (oldHashMethod != hashMethod) /* set IPV6CR Register bit 17~18 and flush multicastv6 table */
	{
		WRITE_MEM32(IPV6CR0,(READ_MEM32(IPV6CR0) & 0xFFF9FFFF)|(currHashMethod << 17));
	}
	return SUCCESS;
}

int32 rtl819x_setAsicMulticastv6Aging(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR) & ~IPv6McastAgingDisable);
		
	} else
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR)|IPv6McastAgingDisable);
	}

	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: IPv6 Multicast Table
  *=========================================*/
uint32 rtl819x_ipMulticastv6TableIndex(uint32 hash_type,inv6_addr_t srcAddr, inv6_addr_t dstAddr)
{
	uint32 idx=0,hash_idx_sip,hash_idx_dip;
	static uint32 sip[128],dip[128];
	uint32 sip_hash[8],dip_hash[8];
	uint32 i,j,offset,src,dst;

    if(hash_type==2)
    {
        idx = srcAddr.v6_addr32[3]&0xFF;
        return idx;
    }
    else if(hash_type==3)
    {
        idx = dstAddr.v6_addr32[3]&0xFF;
        return idx;
    }

	for(i=0; i<8; i++) {
		sip_hash[i]=0;
		dip_hash[i]=0;        
	}

    for(j=0;j<4;j++)
    {
        offset = j*32;
        src = srcAddr.v6_addr32[j];
        dst = dstAddr.v6_addr32[j];
    	for(i=0; i<32; i++)	{
    		if((src& (1<<i))!=0) {
    			sip[i+offset]=1;
    		}
    		else 	{
    			sip[i+offset]=0;
    		}

    		if((dst & (1<<i))!=0) {
    			dip[i+offset]=1;
    		}
    		else {
    			dip[i+offset]=0;
    		}			
    	}
    }

	sip_hash[0] = sip[7] ^ sip[15] ^ sip[23] ^ sip[31] ^ sip[39] ^ sip[47] ^ sip[55] ^ sip[63] ^ sip[71] ^ sip[79] ^ sip[87] ^ sip[95] ^ sip[103]^ sip[111] ^ sip[119] ^ sip[127];
	sip_hash[1] = sip[6] ^ sip[14] ^ sip[22] ^ sip[30] ^ sip[38] ^ sip[46] ^ sip[54] ^ sip[62] ^ sip[70] ^ sip[78] ^ sip[86] ^ sip[94] ^ sip[102]^ sip[110] ^ sip[118] ^ sip[126];
	sip_hash[2] = sip[5] ^ sip[13] ^ sip[21] ^ sip[29] ^ sip[37] ^ sip[45] ^ sip[53] ^ sip[61] ^ sip[69] ^ sip[77] ^ sip[85] ^ sip[93] ^ sip[101]^ sip[109] ^ sip[117] ^ sip[125];
	sip_hash[3] = sip[4] ^ sip[12] ^ sip[20] ^ sip[28] ^ sip[36] ^ sip[44] ^ sip[52] ^ sip[60] ^ sip[68] ^ sip[76] ^ sip[84] ^ sip[92] ^ sip[100]^ sip[108] ^ sip[116] ^ sip[124];
	sip_hash[4] = sip[3] ^ sip[11] ^ sip[19] ^ sip[27] ^ sip[35] ^ sip[43] ^ sip[51] ^ sip[59] ^ sip[67] ^ sip[75] ^ sip[83] ^ sip[91] ^ sip[99] ^ sip[107] ^ sip[115] ^ sip[123];
	sip_hash[5] = sip[2] ^ sip[10] ^ sip[18] ^ sip[26] ^ sip[34] ^ sip[42] ^ sip[50] ^ sip[58] ^ sip[66] ^ sip[74] ^ sip[82] ^ sip[90] ^ sip[98] ^ sip[106] ^ sip[114] ^ sip[122];
	sip_hash[6] = sip[1] ^ sip[9]  ^ sip[17] ^ sip[25] ^ sip[33] ^ sip[41] ^ sip[49] ^ sip[57] ^ sip[65] ^ sip[73] ^ sip[81] ^ sip[89] ^ sip[97] ^ sip[105] ^ sip[113] ^ sip[121];
	sip_hash[7] = sip[0] ^ sip[8]  ^ sip[16] ^ sip[24] ^ sip[32] ^ sip[40] ^ sip[48] ^ sip[56] ^ sip[64] ^ sip[72] ^ sip[80] ^ sip[88] ^ sip[96] ^ sip[104] ^ sip[112] ^ sip[120];

	dip_hash[7] = dip[7] ^ dip[15] ^ dip[23] ^ dip[31] ^ dip[39] ^ dip[47] ^ dip[55] ^ dip[63] ^ dip[71] ^ dip[79] ^ dip[87] ^ dip[95] ^ dip[103]^ dip[111] ^ dip[119] ^ dip[127];
	dip_hash[6] = dip[6] ^ dip[14] ^ dip[22] ^ dip[30] ^ dip[38] ^ dip[46] ^ dip[54] ^ dip[62] ^ dip[70] ^ dip[78] ^ dip[86] ^ dip[94] ^ dip[102]^ dip[110] ^ dip[118] ^ dip[126];
	dip_hash[5] = dip[5] ^ dip[13] ^ dip[21] ^ dip[29] ^ dip[37] ^ dip[45] ^ dip[53] ^ dip[61] ^ dip[69] ^ dip[77] ^ dip[85] ^ dip[93] ^ dip[101]^ dip[109] ^ dip[117] ^ dip[125];
	dip_hash[4] = dip[4] ^ dip[12] ^ dip[20] ^ dip[28] ^ dip[36] ^ dip[44] ^ dip[52] ^ dip[60] ^ dip[68] ^ dip[76] ^ dip[84] ^ dip[92] ^ dip[100]^ dip[108] ^ dip[116] ^ dip[124];
	dip_hash[3] = dip[3] ^ dip[11] ^ dip[19] ^ dip[27] ^ dip[35] ^ dip[43] ^ dip[51] ^ dip[59] ^ dip[67] ^ dip[75] ^ dip[83] ^ dip[91] ^ dip[99] ^ dip[107] ^ dip[115] ^ dip[123];
	dip_hash[2] = dip[2] ^ dip[10] ^ dip[18] ^ dip[26] ^ dip[34] ^ dip[42] ^ dip[50] ^ dip[58] ^ dip[66] ^ dip[74] ^ dip[82] ^ dip[90] ^ dip[98] ^ dip[106] ^ dip[114] ^ dip[122];
	dip_hash[1] = dip[1] ^ dip[9]  ^ dip[17] ^ dip[25] ^ dip[33] ^ dip[41] ^ dip[49] ^ dip[57] ^ dip[65] ^ dip[73] ^ dip[81] ^ dip[89] ^ dip[97] ^ dip[105] ^ dip[113] ^ dip[121];
	dip_hash[0] = dip[0] ^ dip[8]  ^ dip[16] ^ dip[24] ^ dip[32] ^ dip[40] ^ dip[48] ^ dip[56] ^ dip[64] ^ dip[72] ^ dip[80] ^ dip[88] ^ dip[96] ^ dip[104] ^ dip[112] ^ dip[120];

	for(i=0; i<8; i++) {
		sip_hash[i]=sip_hash[i] & (0x01);
		dip_hash[i]=dip_hash[i] & (0x01);
	}

	hash_idx_sip=0;
	for(i=0; i<8; i++) {
		hash_idx_sip=hash_idx_sip+(sip_hash[i]<<i);
	}
	hash_idx_dip=0;
	for(i=0; i<8; i++) {
		hash_idx_dip=hash_idx_dip+(dip_hash[i]<<i);
	}

 	idx=0;
    idx = hash_idx_dip ^ hash_idx_sip;
    
	return idx;
}
int32 rtl819x_setAsicIpMulticastv6Table(uint32 hash_type,rtl819x_tblAsicDrv_multiCastv6Param_t *mCast_t)
{
    uint32 idx;
 	rtl819x_tblAsic_ipMulticastv6Table_t entry;
	
	if(mCast_t == NULL)
		return FAILED;

	if(mCast_t->dip.v6_addr32[0] >>24 != 0xFF || mCast_t->port >= RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER)//rtl8651_totalExtPortNum
	{
        return FAILED;//Non-IP multicast destination address
	}
       
	memset(&entry,0,sizeof(entry));
	entry.sip_addr31_0 		= mCast_t->sip.v6_addr32[3];
	entry.sip_addr63_32 	= mCast_t->sip.v6_addr32[2];
	entry.sip_addr95_64 	= mCast_t->sip.v6_addr32[1];
	entry.sip_addr127_96 	= mCast_t->sip.v6_addr32[0];

	entry.dip_addr31_0 		= mCast_t->dip.v6_addr32[3];
	entry.dip_addr63_32 	= mCast_t->dip.v6_addr32[2];
	entry.dip_addr95_64 	= mCast_t->dip.v6_addr32[1];
	entry.dip_addr123_96 	= mCast_t->dip.v6_addr32[0]&0xFFFFFFF;

    idx=rtl819x_ipMulticastv6TableIndex(hash_type,mCast_t->sip, mCast_t->dip);

	entry.srcPort 	    = mCast_t->port;
	entry.extmbr 		= mCast_t->mbr >> RTL8651_PORT_NUMBER;
	entry.mbr 			= mCast_t->mbr & (RTL8651_PHYSICALPORTMASK);	
	entry.toCPU 		= mCast_t->cpu;
	entry.valid 		= 1;
    entry.six_rd_eg     = mCast_t->six_rd_eg;
    entry.six_rd_idx    = mCast_t->six_rd_idx;
	entry.ageTime		= mCast_t->age;
	////age = (int16)mCast_t->age;
#if 0
	while ( age > 0 ) {
		if ( (++entry.ageTime) == 7)
			break;
		age -= 5;
	}
#endif
	////entry.ageTime = 7;
    /* todo add compare already has data in this entry?*/
#if defined (CONFIG_RTL_8197F)
	entry.destInterface = 0;
#endif
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_V6_TABLE, idx, &entry);
}
int32 rtl819x_delAsicIpMulticastv6Table(uint32 index)
{
	rtl819x_tblAsic_ipMulticastv6Table_t entry;
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_V6_TABLE, index, &entry);
}
int32 rtl819x_getAsicIpMulticastv6Table(uint32 index, rtl819x_tblAsicDrv_multiCastv6Param_t *mCast_t)
{	
	rtl819x_tblAsic_ipMulticastv6Table_t entry;
	
	if (mCast_t == NULL)
	{
        return FAILED;
	}
    
	memset(&entry,0,sizeof(entry));
	_rtl8651_readAsicEntryStopTLU(TYPE_MULTICAST_V6_TABLE, index, &entry);

    mCast_t->sip.v6_addr32[3] = entry.sip_addr31_0;
	mCast_t->sip.v6_addr32[2] = entry.sip_addr63_32;
	mCast_t->sip.v6_addr32[1] = entry.sip_addr95_64;
	mCast_t->sip.v6_addr32[0] = entry.sip_addr127_96;

	mCast_t->dip.v6_addr32[3] = entry.dip_addr31_0;
	mCast_t->dip.v6_addr32[2] = entry.dip_addr63_32;
	mCast_t->dip.v6_addr32[1] = entry.dip_addr95_64;
	mCast_t->dip.v6_addr32[0] = entry.dip_addr123_96;

	mCast_t->port = entry.srcPort;
	mCast_t->mbr  = (entry.extmbr<<RTL8651_PORT_NUMBER) |entry.mbr;	
	mCast_t->age  = entry.ageTime;////*5
	mCast_t->cpu  = entry.toCPU;
    mCast_t->six_rd_eg  = entry.six_rd_eg;
    mCast_t->six_rd_idx = entry.six_rd_idx;
	
	if(entry.valid == 0)
	{
		return FAILED;
	}
	return SUCCESS;
}
#endif
#endif
