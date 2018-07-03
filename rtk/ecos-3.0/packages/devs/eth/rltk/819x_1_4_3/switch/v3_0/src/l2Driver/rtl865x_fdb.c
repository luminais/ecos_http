/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_fdb.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include "rtl865x_fdb.h"
#include <switch/rtl865x_fdb_api.h>
//#include "common/rtl865x_eventMgr.h"
#include <rtl/rtl865x_eventMgr.h>

int32    arpAgingTime = 450;
ether_addr_t cpu_mac = { {0x00, 0x00, 0x0a, 0x00, 0x00, 0x0f} };

int32 rtl865x_getReserveMacAddr(ether_addr_t *macAddr)
{
	memcpy( macAddr, &cpu_mac, sizeof(ether_addr_t));
	return SUCCESS;
}

/*To do list: enable IVL support!!!!!*/
int32 _rtl865x_layer2_patch(void)
{

	int32 retval = 0;
	ether_addr_t mac = { {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} };
	uint32 portmask = 0x5f;

	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_LAN_FID, &mac, FDB_TYPE_TRAPCPU, portmask, TRUE, FALSE);
#if defined (CONFIG_RTL_IVL_SUPPORT)
	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_WAN_FID, &mac, FDB_TYPE_TRAPCPU, portmask, TRUE, FALSE);
#endif

	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_LAN_FID, &cpu_mac, FDB_TYPE_TRAPCPU, 0, TRUE, FALSE);
#if defined (CONFIG_RTL_IVL_SUPPORT)
	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_WAN_FID, &cpu_mac, FDB_TYPE_TRAPCPU, 0, TRUE, FALSE);
#endif

	ASSERT(retval == SUCCESS);
	return SUCCESS;
}

int32 _rtl865x_clearHWL2Table(void)
{
	int i, j;

	for (i = 0; i < RTL8651_L2TBL_ROW; i++)
		for (j = 0; j < RTL8651_L2TBL_COLUMN; j++)
		{
			rtl8651_delAsicL2Table(i, j);
		}

	return SUCCESS;
}

int32 rtl865x_layer2_init(void)
{

	//_rtl865x_fdb_alloc();

	//_rtl865x_fdb_init();

	_rtl865x_layer2_patch();

	return SUCCESS;
}

int32 rtl865x_layer2_reinit(void)
{
	_rtl865x_clearHWL2Table();

	//_rtl865x_fdb_collect();

	//_rtl865x_fdb_alloc();

	//_rtl865x_fdb_init();

	_rtl865x_layer2_patch();

	return SUCCESS;
}

 uint32 rtl865x_getHWL2Index(ether_addr_t * macAddr, uint16 fid)
 {
	return (rtl8651_filterDbIndex(macAddr, fid));
 }

int32 rtl865x_setHWL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p)
{
	return (rtl8651_setAsicL2Table(row, column, l2p));
}

int32 rtl865x_getHWL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p)
{
	return rtl8651_getAsicL2Table(row, column, l2p);
}

int32 rtl865x_refleshHWL2Table(ether_addr_t * macAddr, uint32 flags,uint16 fid)
{

	rtl865x_tblAsicDrv_l2Param_t L2temp, *L2buff;
	uint32 rowIdx, col_num;
	uint32 colIdx = 0;
	uint32 found = FALSE;

	L2buff = &L2temp;
	memset(L2buff, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);

	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) {
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))!=SUCCESS ||
			memcmp(&(L2buff->macAddr), macAddr, 6)!= 0)
			continue;

		if (((flags&FDB_STATIC) && L2buff->isStatic) ||
			((flags&FDB_DYNAMIC) && !L2buff->isStatic)) {
				ASSERT(colIdx);
				col_num = colIdx;
				found = TRUE;
				break;
		}
	}

	if (found == TRUE)
	{
		L2buff->ageSec = 450;
		rtl8651_setAsicL2Table(rowIdx, colIdx, L2buff);
		return SUCCESS;
	}
	else
	{
		return FAILED;
	}
}

int32 rtl_get_hw_fdb_age(uint32 fid,ether_addr_t *mac, uint32 flags)
{
        uint32 rowIdx;
        uint32 colIdx = 0;
        int32 retval = 0;
        rtl865x_tblAsicDrv_l2Param_t L2buff;

        memset(&L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));

        rowIdx = rtl8651_filterDbIndex(mac, fid);

        for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++)
        {
                retval = rtl8651_getAsicL2Table(rowIdx, colIdx, &L2buff);

                if (retval !=SUCCESS ||
                        memcmp(&(L2buff.macAddr), mac, 6)!= 0)
                        continue;
                if (((flags&FDB_DYNAMIC) && !L2buff.isStatic)||((flags&FDB_STATIC) && L2buff.isStatic))
                {
                        retval = L2buff.ageSec;
                        break;
                }

        }

        return retval;
}

int32 rtl865x_Lookup_fdb_entry(uint32 fid, ether_addr_t *mac, uint32 flags, uint32 *col_num, rtl865x_tblAsicDrv_l2Param_t *L2buff)
{
	uint32 rowIdx;
	uint32 colIdx;
	int32 retval;

	rowIdx = rtl8651_filterDbIndex(mac, fid);
	colIdx = 0;

	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++)
	{
		retval = rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff);

		if (retval !=SUCCESS ||
			memcmp(&(L2buff->macAddr), mac, 6)!= 0)
			continue;
		if (((flags&FDB_STATIC) && L2buff->isStatic) ||
			((flags&FDB_DYNAMIC) && !L2buff->isStatic)) {
			ASSERT(colIdx);
			*col_num = colIdx;
			return SUCCESS;
		}

	}
	return FAILED;
}

/*
@func enum RTL_RESULT | rtl865x_addFdbEntry | Add an MAC address, said Filter Database Entry
@parm uint32 | fid | The filter database index (valid: 0~3)
@parm ether_addr_t * | mac | The MAC address to be added
@parm uint32 | portmask | The portmask of this MAC address belongs to
@parm uint32 | type | fdb entry type
@rvalue SUCCESS | Add success
@rvalue FAILED | General failure
@comm
	Add a Filter Database Entry to L2 Table(1024-Entry)
@devnote
	(under construction)
*/
int32 rtl865x_addFilterDatabaseEntry( uint32 fid, ether_addr_t * mac, uint32 portmask, uint32 type )
{
        int32 retval;
	 int32 s;
        if (type != FDB_TYPE_FWD && type != FDB_TYPE_SRCBLK && type != FDB_TYPE_TRAPCPU)
                return TBLDRV_EINVALIDINPUT; /* Invalid parameter */

        if (fid >= RTL865x_FDB_NUMBER)
                return TBLDRV_EINVALIDINPUT;
        if (mac == (ether_addr_t *)NULL)
                return TBLDRV_EINVALIDINPUT;
		
	 s = splimp();
        retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, fid,  mac, type, portmask, FALSE, FALSE);
	 splx(s);
        return retval;
}

int32 rtl865x_addFilterDatabaseEntryExtension( uint16 fid, rtl865x_filterDbTableEntry_t * L2entry)
{
        int32 retval;
	 int32 s;
        if (L2entry->process != FDB_TYPE_FWD && L2entry->process != FDB_TYPE_SRCBLK && L2entry->process != FDB_TYPE_TRAPCPU)
                return TBLDRV_EINVALIDINPUT; /* Invalid parameter */

        if (fid >= RTL865x_FDB_NUMBER)
                return TBLDRV_EINVALIDINPUT;
        if (&(L2entry->macAddr) == (ether_addr_t *)NULL)
                return TBLDRV_EINVALIDINPUT;

        /*l2 lock*/
	s = splimp();
	retval = _rtl865x_addFilterDatabaseEntry(	L2entry->l2type,
											fid,
											&(L2entry->macAddr),
											L2entry->process,
											L2entry->memberPortMask,
											L2entry->auth,
											L2entry->SrcBlk);

        /*l2 unlock*/
	 splx(s);
        return retval;
}


inline int32 convert_setAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag,int8 fid, int8 auth)
{
	rtl865x_tblAsicDrv_l2Param_t l2;

	memset(&l2,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));

	l2.ageSec				= ageSec;
	l2.cpu				= cpu;
	l2.isStatic				= isStatic; 
	l2.memberPortMask		= mbr;
	l2.nhFlag				= nhFlag;
	l2.srcBlk				= srcBlk;
//#ifdef RTL865XC_LAN_PORT_NUM_RESTRIT
//	if(enable4LanPortNumRestrict == TRUE)
	l2.fid=fid; 
	l2.auth = auth;
//#endif	
	memcpy(&l2.macAddr, mac, 6);
	return rtl8651_setAsicL2Table(row, column, &l2);
}

/*
 * <<RTL8651 version B Bug>>
 * RTL8651 L2 entry bug:
 *		For each L2 entry added by driver table as a static entry, the aging time 
 *		will not be updated by ASIC
 * Bug fixed:
 *		To patch this bug, set the entry is a dynamic entry and turn on the 'nhFlag', 
 *		then the aging time of this entry will be updated and once aging time expired,
 *		it won't be removed by ASIC automatically.
 */
int32 rtl8651_setAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag, int8 fid,int8 auth) 
{
	if(mac->octet[0]&0x1)
	{
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				cpu,
				FALSE,
				mbr,
				ageSec,
				isStatic,	/* No one will be broadcast/multicast source */
				nhFlag,	/* No one will be broadcast/multicast source */
				fid,
				TRUE
		);
	}	
	else {
		int8 dStatic=isStatic/*, dnhFlag=(isStatic==TRUE? TRUE: FALSE)*/;
		int8 dnhFlag = nhFlag;
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				cpu,
				srcBlk,
				mbr,
				ageSec,
				dStatic,
				dnhFlag,
				fid,
				auth
//				FALSE,							/* patch here!! always dynamic entry */
//				(isStatic==TRUE? TRUE: FALSE)	/* patch here!! nhFlag always turned on if static entry*/
		);
	}
}

int32 _rtl865x_addFilterDatabaseEntry(uint16 l2Type, uint16 fid,  ether_addr_t * macAddr, uint32 type, uint32 portMask, uint8 auth, uint8 SrcBlk)
{
	uint32 rowIdx = 0;
	uint32 colIdx = 0;
	uint32 col_num = 0;
	uint32 col_tmp = 0;
	uint16 tmp_age = 0xffff;
	int32   found = FALSE;
	int32   flag = FALSE;
	int32   nexthp_flag = FALSE;
	int32  isStatic = FALSE;
	int32  toCpu = FALSE;
	rtl865x_tblAsicDrv_l2Param_t l2entry_tmp,*L2buff;
	int32 retval = 0;

	L2buff = &l2entry_tmp;
	memset(L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++)
	{
		retval = rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff);
		if ((retval)==SUCCESS)
		{
			/*check whether mac address has been learned  to HW or not*/
			if (memcmp(&(L2buff->macAddr), macAddr, 6)== 0)
			{
				/*the entry has been auto learned*/
				if (L2buff->memberPortMask==portMask)
				{
					/*the entry has been auto learned        */
					found = TRUE;
					col_tmp = colIdx;
				}
				else
				/*portmask is different , it should be overwrited*/
				{
					found = FALSE;
					flag = TRUE;
				}
				break;
			}
			/*no matched entry, try get minimum aging time L2 Asic entry*/
			if (tmp_age> L2buff->ageSec)
			{
				tmp_age = L2buff->ageSec;
				col_num = colIdx;
			}
		}
		else
		{
			/*there is still empty l2 asic entry*/
			flag = TRUE;
			break;
		}
	}

	switch(l2Type) {
	case RTL865x_L2_TYPEI:
		nexthp_flag = FALSE;isStatic = FALSE;
		break;
	case RTL865x_L2_TYPEII:
		nexthp_flag = TRUE; isStatic = TRUE;
		break;
	case RTL865x_L2_TYPEIII:
		nexthp_flag = FALSE;isStatic = TRUE;
		break;
	default: ASSERT(0);
	}

	switch(type) {
	case FDB_TYPE_FWD:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_DSTBLK:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_SRCBLK:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_TRAPCPU:
			toCpu =  TRUE;
			break;
	default: ASSERT(0);
	}

	if (found == FALSE)
	{
		/*portmask is different , so it should overwrite the original asic entry. Or there is empty entry, set it to asic*/
		if(flag == TRUE)
		{
			rtl8651_setAsicL2Table_Patch(
					rowIdx,
					colIdx,
					macAddr,
					toCpu,
					SrcBlk,
					portMask,
					arpAgingTime,
					isStatic,
					nexthp_flag,
					fid,
					auth);
			col_tmp = colIdx;
		}
	}
	/*find the same asic entry, should update the aging time*/
	else
	{
		rtl8651_setAsicL2Table_Patch(
				rowIdx,
				col_tmp,
				macAddr,
				toCpu,
				SrcBlk,
				portMask,
				arpAgingTime,
				isStatic,
				nexthp_flag,
				fid,
				auth);
	}

	return SUCCESS;
}

/*
@func int32 | rtl8651_delFilterDatabaseEntry | Remove a filter database entry.
@parm uint16 | fid | Filter database entry.
@parm ether_addr_t * | macAddr | Pointer to a MAC Address.
@rvalue RTL_EINVALIDFID | Filter database ID.
@rvalue RTL_NULLMACADDR | The specified MAC address is NULL.
@rvalue RTL_EINVALIDINPUT | Invalid input parameter.
*/
int32 rtl865x_delFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr) 
{
	int32 retval;
	int32 s;
	if (fid >= RTL865x_FDB_NUMBER)
		return TBLDRV_EINVALIDINPUT;
	if (macAddr == (ether_addr_t *)NULL)
		return TBLDRV_EINVALIDINPUT;

/*	L2 lock		*/
	s = splimp();

	retval = _rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, macAddr);
	
/*	L2 unlock		*/
	splx(s);

	return retval;
}

int32 _rtl865x_delFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr)
{
	uint32 rowIdx = 0;
	uint32 colIdx = 0;
	rtl865x_tblAsicDrv_l2Param_t L2temp, *L2buff ;
	rtl865x_filterDbTableEntry_t l2entry_t;
	int32 found = FALSE;

	rowIdx = rtl8651_filterDbIndex(macAddr, fid);

	L2buff = &L2temp;
	memset(L2buff, 0 , sizeof (rtl865x_tblAsicDrv_l2Param_t));
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++)
	{
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))==SUCCESS)
		{
			/*check whether mac address has been learned  to SW or not*/
			if (memcmp(&(L2buff->macAddr), macAddr, 6)== 0)
			{
				/*the entry has been auto learned*/
				found = TRUE;
				break;
			}
		}
	}

	/*
	when a sta from eth0 to wlan0, the layer driver fdb will be deleted, but linux bridge fdb still exist,
	so delete the asic entry anyway to avoid layer connection issue.
	eg:
	first moment .sta is at eth0
	second moment:connect sta to wlan0, the layer driver fdb will be deleted, but linux bridge fdb still there
	third moment:move sta to eth0 again, layer driver fdb won't be created due to linux bridge fdb already exist.
	*/
	if (found == TRUE)
	{
		rtl8651_delAsicL2Table(rowIdx, colIdx);
		memset(&l2entry_t, 0, sizeof(rtl865x_filterDbTableEntry_t));
		memcpy(&(l2entry_t.macAddr), macAddr, sizeof(ether_addr_t));
		rtl865x_raiseEvent(EVENT_DEL_FDB, (void *)(&l2entry_t));
	}

	return SUCCESS;
}


#ifdef CONFIG_RTL865X_SYNC_L2
int32 rtl865x_arrangeFdbEntry(const unsigned char *timeout_addr, int32 *port)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	int32 rowIdx;

	macAddr = (ether_addr_t *)(timeout_addr);
	rowIdx = rtl8651_filterDbIndex(macAddr, RTL_LAN_FID);
	found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	if (found == SUCCESS)
	{
		if(fdbEntry.ageSec == 450)
		{
			return RTL865X_FDBENTRY_450SEC;
		}
		if(fdbEntry.ageSec == 300)
		{
			return RTL865X_FDBENTRY_300SEC;
		}
		if(fdbEntry.ageSec == 150)
		{
			return RTL865X_FDBENTRY_150SEC;
		}
	}
	return FAILED;
}

int32 rtl865x_delLanFDBEntry(uint16 l2Type,  const unsigned char *addr)
{
	int32 ret=FAILED;
	ether_addr_t *macAddr;

	if (addr == NULL)
	{
		return TBLDRV_EINVALIDINPUT;
	}
	macAddr = (ether_addr_t *)(addr);

	ret =_rtl865x_delFilterDatabaseEntry(	l2Type, RTL_LAN_FID, macAddr);

	return ret;
}

#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)
void update_hw_l2table(const char *srcName,const unsigned char *addr)
{

	//int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 ret = 0;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	macAddr = (ether_addr_t *)(addr);

	if (memcmp(srcName, "wlan0", 4) ==0)
	{
		if (rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS)
		{
			if((fdbEntry.memberPortMask & RTL8651_PHYSICALPORTMASK)!=0)
			{
				ret = rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_LAN_FID, macAddr);
			}

		}

#if defined (CONFIG_RTL_IVL_SUPPORT)
		if (rtl865x_Lookup_fdb_entry(RTL_WAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS)
		{
			if((fdbEntry.memberPortMask & RTL8651_PHYSICALPORTMASK)!=0)
			{
				ret = rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_WAN_FID, macAddr);
			}

		}
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
		{
			extern void del_8367r_L2(const unsigned char *addr);
			del_8367r_L2(addr);	
		}	
#endif
	}

}
#endif
int32 rtl865x_addFDBEntry(const unsigned char *addr)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 ret=FAILED;
	int8 port_num = -1;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;

	if (addr == NULL)
		return TBLDRV_EINVALIDINPUT;

	macAddr = (ether_addr_t *)(addr);
	found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	if (found == SUCCESS )
	{
		/*Update asic fdb entry expire time*/
		ret =_rtl865x_addFilterDatabaseEntry(	(fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII,
		        										fdbEntry.fid,
		        										addr,
		        										FDB_TYPE_FWD,
		        										fdbEntry.memberPortMask,
		        										FALSE,
		        										FALSE);
	}
	else
	{
		/* not add */
	}
	return ret;
}


int32 rtl865x_ConvertPortMasktoPortNum(int32 portmask)
{
	int32 i = 0;

	for (i = PHY0; i < EXT3; i++)
	{
		if(((portmask >> i) & 0x01) == 1)
		{
			return i;
		}
	}
	return FAILED;
}
#endif


