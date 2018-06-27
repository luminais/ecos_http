/*
 *      Routines to handle MIB operation
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: apmib.c,v 1.16 2009/09/03 05:04:41 keith_huang Exp $
 *
 */
#include <cyg/kernel/kapi.h>
// include file
#include <stdio.h>
#include <stdlib.h>
//#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "apmib.h"
#include "mibtbl.h"


// MAC address filtering
typedef struct _filter {
	struct _filter *prev, *next;
	char val[1];
} FILTER_T, *FILTER_Tp;

typedef struct _linkChain {
	FILTER_Tp pUsedList, pFreeList;
	int size, num, usedNum, compareLen, realsize;
	char *buf;
} LINKCHAIN_T, *LINKCHAIN_Tp;


// macro to remove a link list entry
#define REMOVE_LINK_LIST(entry) { \
	if ( entry ) { \
		if ( entry->prev ) \
			entry->prev->next = entry->next; \
		if ( entry->next ) \
			entry->next->prev = entry->prev; \
	} \
}

// macro to add a link list entry
#define ADD_LINK_LIST(list, entry) { \
	if ( list == NULL ) { \
		list = entry; \
		list->prev = list->next = entry; \
	} \
	else { \
		entry->prev = list; \
		entry->next = list->next; \
		list->next = entry; \
		entry->next->prev = entry; \
	} \
}

// local routine declaration
int rtk_flash_read(char *buf, int offset, int len);
int rtk_flash_write(char *buf, int offset, int len);
#ifndef MIB_TLV
static int init_linkchain(LINKCHAIN_Tp pLinkChain, int size, int num);
static int add_linkchain(LINKCHAIN_Tp pLinkChain, char *val);
static int delete_linkchain(LINKCHAIN_Tp pLinkChain, char *val);
static void delete_all_linkchain(LINKCHAIN_Tp pLinkChain);
static int get_linkchain(LINKCHAIN_Tp pLinkChain, char *val, int index);
#endif

#ifdef COMPRESS_MIB_SETTING
unsigned int mib_compress_write(CONFIG_DATA_T type, unsigned char *data);
int mib_updateDef_compress_write(CONFIG_DATA_T type, char *data, TLV_PARAM_HEADER_T *pheader);
#endif

#ifdef MIB_TLV
int get_tblentry(void *pmib,unsigned int offset,int num,const mib_table_entry_T *mib_tbl,void *val, int index);
int add_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val);
int delete_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val);
int delete_all_tblentry(void *pmib, unsigned int offset, int num,const mib_table_entry_T *mib_tbl);
int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);

unsigned int mib_tlv_init(mib_table_entry_T *mib_tbl, unsigned char *from_data, void *pfile, unsigned int tlv_content_len);
unsigned int mib_get_setting_len(CONFIG_DATA_T type);
unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len);
void mib_display_data_content(CONFIG_DATA_T type, unsigned char * pdata, unsigned int mib_data_len);
void mib_display_tlv_content(CONFIG_DATA_T type, unsigned char * ptlv, unsigned int mib_tlv_len);
int mib_search_by_id(mib_table_entry_T *mib_tbl, unsigned short mib_id, unsigned char *pmib_num, mib_table_entry_T **ppmib, unsigned int *offset);
#endif

mib_table_entry_T* mib_get_table(CONFIG_DATA_T type);
int mib_write_to_raw(const mib_table_entry_T *mib_tbl, void *data, unsigned char *pfile, unsigned int *idx);

// local & global variable declaration
APMIB_Tp pMib=NULL;
APMIB_Tp pMibDef=NULL;
PARAM_HEADER_T hsHeader, dsHeader, csHeader;
TLV_PARAM_HEADER_T tlvhsHeader, tlvdsHeader, tlvcsHeader;

HW_SETTING_Tp pHwSetting=NULL;
#ifdef KLD_ENABLED
WIZARD_T WizMib;
WIZARD_Tp pWizMib; 
#endif

#if defined(HAVE_TR069)
int wlan_idx=0;	// interface index 
int vwlan_idx=0;	// initially set interface index to root

#ifndef CONFIG_WLANIDX_MUTEX
int wlan_idx_bak=0;
int vwlan_idx_bak=0;
#endif
int wan_idx=0;
#else
static int wlan_idx=0;	// interface index 
static int vwlan_idx=0;	// initially set interface index to root

#ifndef CONFIG_WLANIDX_MUTEX
static int wlan_idx_bak=0;
static int vwlan_idx_bak=0;
#endif
static int wan_idx=0;
#endif

#ifdef MIB_TLV
#else
static LINKCHAIN_T wlanMacChain[NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1];
static LINKCHAIN_T wdsChain[NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1];
static LINKCHAIN_T scheduleRuleChain;

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
static LINKCHAIN_T meshAclChain;
#endif

#ifdef HOME_GATEWAY
static LINKCHAIN_T portFwChain, ipFilterChain, portFilterChain, macFilterChain, triggerPortChain;
static LINKCHAIN_T urlFilterChain;
#ifdef ROUTE_SUPPORT
static LINKCHAIN_T staticRouteChain;
#endif

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
static LINKCHAIN_T qosChain;
#endif
#endif
static LINKCHAIN_T dhcpRsvdIpChain;

#if defined(VLAN_CONFIG_SUPPORTED)
static LINKCHAIN_T vlanConfigChain;
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static LINKCHAIN_T  ipsecTunnelChain;
#endif
#endif

#ifdef TLS_CLIENT
static LINKCHAIN_T  certRootChain;
static LINKCHAIN_T  certUserChain;
#endif

static int apmib_mutex_init = 0;
static cyg_mutex_t apmib_mutex;

#ifdef CONFIG_WLANIDX_MUTEX
static volatile cyg_uint32 rtl_spl_state = 0;
#define RTL_SPL_WLANIDX     0x01
static cyg_mutex_t rtl_splx_mutex;
static volatile cyg_handle_t rtl_splx_thread;
#define MAX_BAK_NUM 		20

static struct Wlidx_bak{
	int wlan_idx_bak;
	int vwlan_idx_bak;	
};

typedef struct {
	struct Wlidx_bak wlidx_bak;
	struct WlidxBakStackNode *next;
} WlidxBakStackNode;

typedef struct{
	WlidxBakStackNode *top;
	int count;
}WlidxBakLinkStack;

WlidxBakLinkStack WlidxBakStack;

int InitWlidxBakStack(WlidxBakLinkStack * S)
{
	WlidxBakStackNode *sNode = NULL;

	if(S==NULL)
	{
		diag_printf("=======error, invalid para S=%p ==========\n",S);
		return 0;
	}
	
	if(S->count != 0)
	{
		diag_printf("========error, S->count is %d========\n", S->count);
		return 0;
	}
#if 1
	sNode = (WlidxBakStackNode *)malloc(sizeof(WlidxBakStackNode));
	if(sNode == NULL)
	{
		printf("alloc memory fo stack failed!!!!!!\n");
		return 0;
	}
	sNode->next = NULL;
	S->top = sNode;
	S->count = 0;
	S->top->wlidx_bak.wlan_idx_bak = cyg_thread_self();
#endif

	return 1;
}

int DestoryWlidxBakStack(WlidxBakLinkStack * S)
{
	WlidxBakStackNode *sNode = S->top;
	if(S==NULL)
	{
		diag_printf("=======error, invalid para S=%p ==========\n",S);
		return 0;
	}
	
#if 1	
	if(S->count != 0 || S->top == NULL )
	{
		diag_printf("=================error, S->count is %d, the S->top is %p=================\n", S->count, S->top);
	}
	else if(S->top->next != NULL || S->top->wlidx_bak.wlan_idx_bak != cyg_thread_self())
		diag_printf("=================error, the S->top->next is %p, S->top->wlidx_bak.wlan_idx_bak is %x, cyg_thread_self() is %x=================\n", S->top->next, S->top->wlidx_bak.wlan_idx_bak, cyg_thread_self());
	else
	{
		S->count = 0;
		if(S->top != NULL)
			free(S->top);
	}
#endif
	
	return 1;
}

int PushWlidxBak(WlidxBakLinkStack * S)
{
	WlidxBakStackNode *sNode;
	if(S==NULL)
	{
		diag_printf("=======error, invalid para S=%p ==========\n",S);
		return 0;
	}
	sNode = (WlidxBakStackNode *)malloc(sizeof(WlidxBakStackNode));

	if(sNode == NULL)
	{
		printf("alloc memory fo stack failed!!!!!!\n");
		return 0;
	}
	
	sNode->wlidx_bak.wlan_idx_bak = wlan_idx;
	sNode->wlidx_bak.vwlan_idx_bak = vwlan_idx;
	sNode->next = S->top;
	S->top = sNode;
	S->count++;

	return 1;
}

int PopWlidxBak(WlidxBakLinkStack * S)
{
	WlidxBakStackNode *sNode = S->top;

	if(S==NULL)
	{
		diag_printf("=======error, invalid para S=%p ==========\n",S);
		return 0;
	}
	
	if(sNode != NULL && sNode->next != NULL)
	{
		wlan_idx = sNode->wlidx_bak.wlan_idx_bak;
		vwlan_idx = sNode->wlidx_bak.vwlan_idx_bak;
		
		S->top = sNode->next;
		S->count--;
		free(sNode);
		sNode = NULL;
		
	}
	else
	{
		diag_printf("No pop, stack is empty!!!!! \n");
		return 0;
	}

	return 1;
	
}
static inline cyg_uint32
rtl_spl_any( cyg_uint32 which )
{
    cyg_uint32 rtl_old_spl = rtl_spl_state;
    if ( cyg_thread_self() != rtl_splx_thread ) {
        while ( !cyg_mutex_lock( &rtl_splx_mutex ) )
            continue;
        rtl_old_spl = 0; // Free when we unlock this context
        CYG_ASSERT( 0 == rtl_splx_thread, "Thread still owned" );
        CYG_ASSERT( 0 == rtl_spl_state, "spl still set" );
        rtl_splx_thread = cyg_thread_self();
	 InitWlidxBakStack(&WlidxBakStack);
    }
    CYG_ASSERT( rtl_splx_mutex.locked, "spl_any: mutex not locked" );
    CYG_ASSERT( (cyg_handle_t)rtl_splx_mutex.owner == cyg_thread_self(),
                "spl_any: mutex not mine" );
    rtl_spl_state |= which;

    return rtl_old_spl;
}
cyg_uint32
rtl_splidx()
{
    //SPLXTRACE;
    return rtl_spl_any( RTL_SPL_WLANIDX );
}

//
// Return to a previous interrupt state/level.
//
void
rtl_splx(cyg_uint32 old_state)
{
    //SPLXTRACE;
    
    CYG_ASSERT( 0 != rtl_spl_state, "No state set" );
    CYG_ASSERT( rtl_splx_mutex.locked, "splx: mutex not locked" );
    CYG_ASSERT( (cyg_handle_t)rtl_splx_mutex.owner == cyg_thread_self(),
                "splx: mutex not mine" );

	rtl_spl_state &= old_state;
	if((old_state !=0) && (old_state!=1))
		diag_printf("=========error %s(%d), rtl_spl_state is %d, old_state is %d====\n", __FUNCTION__, __LINE__, rtl_spl_state, old_state);
    if ( 0 == rtl_spl_state ) {
        rtl_splx_thread = 0;
	 DestoryWlidxBakStack(&WlidxBakStack);
        cyg_mutex_unlock( &rtl_splx_mutex );
    }

}
#endif
int _apmib_set_wlanidx(int idx)
{
	if(idx < 0 || idx >= NUM_WLAN_INTERFACE) {
		printf("invalid wlan idx (%d) to set\n",idx);
		return -1;
	}
	wlan_idx=idx;
	return wlan_idx;
}

int _apmib_get_wlanidx(void)
{
	return wlan_idx;
}

int _apmib_set_vwlanidx(int idx)
{
	if(idx < 0 || idx > NUM_VWLAN_INTERFACE) {
		printf("invalid vwlan idx (%d) to set\n",idx);
                return -1;
        }
	vwlan_idx=idx;
	return vwlan_idx;
}

int _apmib_get_vwlanidx(void)
{
	return vwlan_idx;
}

int _apmib_set_wanidx(int idx)
{
	if(idx<0 || idx>NUM_WAN_INTERFACE)
	{
		printf("invalid wan idx to set\n");
		return -1;
	}
	wan_idx=idx;
	return wan_idx;
}
int _apmib_get_wanidx(void)
{
	return wan_idx;
}

int apmib_sem_lock(void)
{
	if (apmib_mutex_init == 0) {
		cyg_mutex_init(&apmib_mutex);
		apmib_mutex_init = 1;
	}
	
	if (cyg_mutex_lock(&apmib_mutex))
		return 0;
	printf("apmib_sem_lock() failed!\n");
	return -1;
}

int apmib_sem_unlock(void)
{
	cyg_mutex_unlock(&apmib_mutex);
	return 0;	
}

#ifdef CONFIG_WLANIDX_MUTEX
unsigned int _apmib_save_idx()
{
	cyg_uint32 ret;
	struct Wlidx_bak Wlidxbak;
	ret=rtl_splidx();
	PushWlidxBak(&WlidxBakStack);
	return ret;
}
unsigned int _apmib_revert_idx(unsigned int s)
{
	struct Wlidx_bak Wlidxbak;
	PopWlidxBak(&WlidxBakStack);
	rtl_splx(s);
	return 1;
}
#else
int _apmib_save_idx() 
{ 
	wlan_idx_bak=wlan_idx;
	vwlan_idx_bak=vwlan_idx;
	return 1;
}
int _apmib_revert_idx()
{
	wlan_idx=wlan_idx_bak;
	vwlan_idx=vwlan_idx_bak;
	return 1;
}
#endif

#ifdef _LITTLE_ENDIAN_
static int _mib_swap_value(const mib_table_entry_T *mib, void *data)
{
	short *pShort;
	int *pInt;

	switch (mib->type)
	{
	case WORD_T:
		pShort = (short *) data;
		*pShort = WORD_SWAP(*pShort);
		break;
	case DWORD_T:
		pInt = (int *) data;
		*pInt = DWORD_SWAP(*pInt);
		break;
	default:
		break;
	}

	return 0;
}

static int _mibtbl_swap_value(const mib_table_entry_T *mib_tbl, void *data, int offset)
{
	int i, j;
	const mib_table_entry_T *mib;
	int new_offset;

	for (i=0; mib_tbl[i].id; i++)
	{
		mib = &mib_tbl[i];
		new_offset = offset + mib->offset;
		for (j=0; j<(mib->total_size / mib->unit_size); j++)
		{
			if (mib->type >= TABLE_LIST_T)
			{
				if (_mibtbl_swap_value(mib->next_mib_table, data, new_offset) != 0)
				{
					fprintf(stderr, "MIB (%s, %d, %d) Error: swap failed\n",
						mib_tbl[i].name, mib_tbl[i].total_size, mib_tbl[i].unit_size);
					return -1;
				}
			}
			else
			{
				_mib_swap_value(mib, (void *)((unsigned int) data + new_offset));
			}
			new_offset += mib->unit_size;
		}
	}

	return 0;
}

void swap_mib_word_value(APMIB_Tp pMib)
{
	mib_table_entry_T *pmib_tl;

	pmib_tl = mib_get_table(CURRENT_SETTING);
	_mibtbl_swap_value(pmib_tl, pMib, 0);
#ifdef VOIP_SUPPORT
	voip_mibtbl_swap_value(&pMib->voipCfgParam);
#endif
}

void swap_mib_value(void *pMib, CONFIG_DATA_T type)
{
	mib_table_entry_T *pmib_tl;
	pmib_tl = mib_get_table(type);
	_mibtbl_swap_value(pmib_tl, pMib, 0);
}
#endif // _LITTLE_ENDIAN_

////////////////////////////////////////////////////////////////////////////////
char *_apmib_hwconf(void)
{
	int ver;
	char *buff;
	int compress_hw_setting=1;
#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING
	rtk_flash_read((char *)&compHeader, HW_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
	if(strcmp((char *)compHeader.signature, COMP_HS_SIGNATURE) == 0 ) //check whether compress mib data
	{
        zipRate = WORD_SWAP(compHeader.compRate);
		compLen = DWORD_SWAP(compHeader.compLen);
		if ( (compLen > 0) && (compLen <= HW_SETTING_SECTOR_LEN) ) {
			compFile=malloc(compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			rtk_flash_read((char *)compFile, HW_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			memcpy((char *)&tlvhsHeader, expFile, sizeof(tlvhsHeader));
			tlvhsHeader.len=WORD_SWAP(tlvhsHeader.len);
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}

#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *hwMibData;
		unsigned int tlv_content_len = tlvhsHeader.len - 1; // last one is checksum

		hwMibData = malloc(sizeof(HW_SETTING_T)+1); // 1: checksum

		if(hwMibData != NULL)
			memset(hwMibData, 0x00, sizeof(HW_SETTING_T)+1);
	
		pmib_tl = mib_get_table(HW_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(TLV_PARAM_HEADER_T),tlvhsHeader.len);

//mib_display_tlv_content(HW_SETTING, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(TLV_PARAM_HEADER_T), (void*)hwMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			memcpy(&hsHeader,&tlvhsHeader,TAG_LEN);
			sprintf((char *)&hsHeader.signature[TAG_LEN], "%02d", HW_SETTING_VER);
			hsHeader.len = sizeof(HW_SETTING_T)+1;
			hwMibData[hsHeader.len-1]  = CHECKSUM(hwMibData, hsHeader.len-1);

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+hsHeader.len);
			memcpy(expFile, &hsHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), hwMibData, hsHeader.len);

//mib_display_data_content(HW_SETTING, expFile+sizeof(PARAM_HEADER_T), hsHeader.len-1);

			
		}	

		if(hwMibData != NULL)
			free(hwMibData);
		
#endif // #ifdef MIB_TLV

	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read hw setting
	{
		if ( rtk_flash_read((char *)&hsHeader, HW_SETTING_OFFSET, sizeof(hsHeader))==0 ) {
	//		printf("Read hw setting header failed!\n");
			return NULL;
		}
		hsHeader.len=WORD_SWAP(hsHeader.len);
		compress_hw_setting=0;
		//diag_printf("[%s:%d]HW_SETTING_OFFSET=0x%x, hsHeader.len=0x%x\n", __FUNCTION__, __LINE__,HW_SETTING_OFFSET,hsHeader.len);
	}

	if ( sscanf((char *)&hsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

    //diag_printf("\r\n ver = %u\n",ver);
	
    if ( memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN)) // invalid signatur
	{
		printf("Invalid hw setting signature [sig=%c%c]!\n", hsHeader.signature[0],hsHeader.signature[1]);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}
	
	if ( (ver != HW_SETTING_VER) || // version not equal to current
		(hsHeader.len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
		printf("Invalid hw setting version number or data length[ver=%d %d, len=%d %d]!\n", ver,HW_SETTING_VER, hsHeader.len, sizeof(HW_SETTING_T)+1);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}
//	if (ver > HW_SETTING_VER)
//		printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);

	buff = calloc(1, hsHeader.len);
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}

#ifdef COMPRESS_MIB_SETTING
	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);
		free(expFile);
		free(compFile);
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);		
	}
	else // not compress mib data, copy mibdata from flash
#endif
	{
		if ( rtk_flash_read(buff, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
	//		printf("Read hw setting failed!\n");
			free(buff);
	//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
			return NULL;
		}
	}
	if ( !CHECKSUM_OK((unsigned char *)buff, hsHeader.len) ) {
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		free(buff);
		return NULL;
	}
	if(!compress_hw_setting){
#ifdef MIB_TLV
		int index=0;
		unsigned short *wVal=NULL;
		unsigned int *dwVal=NULL;
		mib_table_entry_T * pmib_tl = mib_get_table(HW_SETTING);//hwmib_root_table
		pmib_tl=pmib_tl->next_mib_table;//hwmib_table
		while(pmib_tl[index].id){
			switch(pmib_tl[index].type)
			{
			case WORD_T:
				wVal=buff+pmib_tl[index].offset;
				*wVal=WORD_SWAP(*wVal);
				break;
			case DWORD_T:
				dwVal=buff+pmib_tl[index].offset;
				*dwVal=DWORD_SWAP(*dwVal);
				break;
			default:
				break;
			}
			index++;
		}
#endif
	}
	return buff;
}

////////////////////////////////////////////////////////////////////////////////
char *_apmib_dsconf(void)
{
	int ver;
	char *buff;
#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

    //fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	rtk_flash_read((char*)&compHeader, DEFAULT_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
    //fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);	
	if(strcmp((char *)compHeader.signature, COMP_DS_SIGNATURE) == 0 ) //check whether compress mib data
	{
        zipRate = WORD_SWAP(compHeader.compRate);
		compLen = DWORD_SWAP(compHeader.compLen);
		if ( (compLen > 0) && (compLen <= DEFAULT_SETTING_SECTOR_LEN) ) {
			compFile=malloc(compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			rtk_flash_read((char *)compFile, DEFAULT_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			//printf("expandLen read len: %d\n", expandLen);
			// copy the mib header from expFile
			memcpy((char *)&tlvdsHeader, expFile, sizeof(tlvdsHeader));
			tlvdsHeader.len=DWORD_SWAP(tlvdsHeader.len);
			
			if(tlvdsHeader.len > expandLen)
				return NULL;
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}
#ifdef MIB_TLV
		dsHeader.len=WORD_SWAP(dsHeader.len);

		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *defMibData;
		unsigned int tlv_content_len = tlvdsHeader.len - 1; // last one is checksum

		defMibData = malloc(sizeof(APMIB_T)+1); // 1: checksum

		if(defMibData != NULL)
			memset(defMibData, 0x00, sizeof(APMIB_T)+1);
	
		pmib_tl = mib_get_table(DEFAULT_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(TLV_PARAM_HEADER_T), tlvdsHeader.len);


//mib_display_tlv_content(DEFAULT_SETTING, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(TLV_PARAM_HEADER_T), (void*)defMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
		
			memcpy((char *)&dsHeader,(char *)&tlvdsHeader,TAG_LEN);
			sprintf((char *)&dsHeader.signature[TAG_LEN], "%02d", DEFAULT_SETTING_VER);
			dsHeader.len = sizeof(APMIB_T)+1;
			defMibData[dsHeader.len-1]  = CHECKSUM(defMibData, sizeof(APMIB_T));

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+dsHeader.len);
			memcpy(expFile, &dsHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), defMibData, dsHeader.len);
			
//mib_display_data_content(DEFAULT_SETTING, expFile+sizeof(PARAM_HEADER_T), dsHeader.len-1);
		}	
		else
		{
			COMP_TRACE(stderr,"\r\n ERR!Invalid checksum[%u] or mib_tlv_init() fail! __[%s-%u]",tlv_checksum,__FILE__,__LINE__);
		}
		
		if(defMibData != NULL)
			free(defMibData);
		

#endif // #ifdef MIB_TLV		
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read default s/w mib
	{
        if ( rtk_flash_read((char *)&dsHeader, DEFAULT_SETTING_OFFSET, sizeof(dsHeader))==0 ) {
	//		printf("Read default setting header failed!\n");
			return NULL;
		}
		dsHeader.len=WORD_SWAP(dsHeader.len);
		//diag_printf("[%s:%d]dsHeader.len=%d\n", __FUNCTION__, __LINE__, dsHeader.len);
	}

	if ( sscanf((char *)&dsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

//fprintf(stderr,"\r\n (sizeof(APMIB_T)=%u ",(sizeof(APMIB_T)));
//printf("default setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",	dsHeader.signature[0], dsHeader.signature[1], ver, dsHeader.len);
	if ( memcmp(dsHeader.signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != DEFAULT_SETTING_VER) || // version not equal to current
		(dsHeader.len < (sizeof(APMIB_T)+1)) ) { // length is less than current
		printf("Invalid default setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",	dsHeader.signature[0], dsHeader.signature[1], ver, dsHeader.len);
		printf("Expect [sig=%s, ver=%d, len=%d]!\n", DEFAULT_SETTING_HEADER_TAG, DEFAULT_SETTING_VER, sizeof(APMIB_T)+1);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
//	if (ver > DEFAULT_SETTING_VER)
//		printf("Default setting version is greater than current [f:%d, c:%d]!\n", ver, DEFAULT_SETTING_VER);


	buff = calloc(1, dsHeader.len);
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}


#ifdef COMPRESS_MIB_SETTING
	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
	

		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( rtk_flash_read(buff, DEFAULT_SETTING_OFFSET+sizeof(dsHeader), dsHeader.len)==0 ) {
//		printf("Read default setting failed!\n");
		free(buff);
		return NULL;
	}

	if ( !CHECKSUM_OK((unsigned char *)buff, dsHeader.len) ) {
//		printf("Invalid checksum of current setting!\n");


		free(buff);
		return NULL;
	}
	

	return buff;
}

////////////////////////////////////////////////////////////////////////////////
char *_apmib_csconf(void)
{
	int ver;
	char *buff;
#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	rtk_flash_read((char*)&compHeader, CURRENT_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);	
	if(strcmp((char *)compHeader.signature, COMP_CS_SIGNATURE) == 0 ) //check whether compress mib data
	{
        zipRate = WORD_SWAP(compHeader.compRate);
		compLen = DWORD_SWAP(compHeader.compLen);
        if ( (compLen > 0) && (compLen <= CURRENT_SETTING_SECTOR_LEN) ) {
			compFile=calloc(1,compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=calloc(1,zipRate*compLen);
            if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			rtk_flash_read((char *)compFile, CURRENT_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));
			//fprintf(stderr,"\r\n __[%s-%u] len=%d complen=%d rate=%d\n",__FILE__,__LINE__,compLen+sizeof(COMPRESS_MIB_HEADER_T),compLen,zipRate);

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			//printf("expandLen read len: %d\n", expandLen);

			// copy the mib header from expFile
			memcpy((char *)&tlvcsHeader, expFile, sizeof(tlvcsHeader));
			tlvcsHeader.len=DWORD_SWAP(tlvcsHeader.len);
			if(tlvcsHeader.len > expandLen)
				return NULL;
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}
#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *curMibData;
		unsigned int tlv_content_len = tlvcsHeader.len - 1; // last one is checksum

		curMibData = malloc(sizeof(APMIB_T)+1); // 1: checksum

		if(curMibData != NULL)
			memset(curMibData, 0x00, sizeof(APMIB_T)+1);
	
		pmib_tl = mib_get_table(CURRENT_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(TLV_PARAM_HEADER_T), tlvcsHeader.len);

//mib_display_tlv_content(CURRENT_SETTING, expFile+sizeof(PARAM_HEADER_T), csHeader.len);

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(TLV_PARAM_HEADER_T), (void*)curMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			
			memcpy((char *)&csHeader,(char *)&tlvcsHeader,TAG_LEN);
			sprintf((char *)&csHeader.signature[TAG_LEN], "%02d", CURRENT_SETTING_VER);
			csHeader.len = sizeof(APMIB_T)+1;
			curMibData[csHeader.len-1]  = CHECKSUM(curMibData, csHeader.len-1);

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+csHeader.len);
			memcpy(expFile, &csHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), curMibData, csHeader.len);
			
//mib_display_data_content(CURRENT_SETTING, expFile+sizeof(PARAM_HEADER_T), csHeader.len-1);
		}	

		if(curMibData != NULL)
			free(curMibData);
		
#endif // #ifdef MIB_TLV		
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read current s/w mib
	{
		if ( rtk_flash_read((char *)&csHeader, CURRENT_SETTING_OFFSET, sizeof(csHeader))==0 ) {
			fprintf(stderr,"Read current setting header failed!\n");
			return NULL;
		}
		csHeader.len=WORD_SWAP(csHeader.len);
	}

//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);

    if ( sscanf((char *)&csHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;
//fprintf(stderr,"\r\n __[%s-%u]sizeof(APMIB_T)=%d csHeader.len=%d\n",__FILE__,__LINE__,sizeof(APMIB_T),csHeader.len);

    if ( memcmp(csHeader.signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != CURRENT_SETTING_VER) || // version not equal to current
			(csHeader.len < (sizeof(APMIB_T)+1)) ) { // length is less than current
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

//	if (ver > CURRENT_SETTING_VER)
//		printf("Current setting version is greater than current [f:%d, c:%d]!\n", ver, CURRENT_SETTING_VER);

    buff = calloc(1, csHeader.len);
	if ( buff == 0 ) {
		fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

#ifdef COMPRESS_MIB_SETTING
	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), csHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( rtk_flash_read(buff, CURRENT_SETTING_OFFSET+sizeof(csHeader), csHeader.len)==0 ) {
fprintf(stderr,"Read current setting failed!\n");
		free(buff);
		return NULL;
	}

	if ( !CHECKSUM_OK((unsigned char *)buff, csHeader.len) ) {
fprintf(stderr,"Invalid checksum of current setting!\n");
		free(buff);
		return NULL;
	}
	return buff;
}
////////////////////////////////////////////////////////////////////////////
int _apmib_init_HW(void)
{
	char *buff;

	apmib_sem_lock();
	
	if (pHwSetting != NULL) {
		apmib_sem_unlock();
		return 1;
	}
		
    diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	if ((buff=apmib_hwconf()) == NULL) {
		apmib_sem_unlock();
		return 0;
	}
	pHwSetting = (HW_SETTING_Tp)buff;
	apmib_sem_unlock();
	return 1;
}
////////////////////////////////////////////////////////////////////////////////
int __apmib_init(void)
{
#ifndef MIB_TLV
	int i, j, k;
#endif
	char *buff;

	if ( pMib != NULL )	// has been initialized
		goto linkchain;

	if (pHwSetting == NULL) {
        if ((buff=apmib_hwconf()) == NULL) {
            fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
			return 0;
		}
		pHwSetting = (HW_SETTING_Tp)buff;
	}
	
    if ((buff=apmib_dsconf()) == NULL) {
		free(pHwSetting);
		fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return 0;
	}
    pMibDef = (APMIB_Tp)buff;

    if ((buff=apmib_csconf()) == NULL) {		
		fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		free(pHwSetting);
		free(pMibDef);
		return 0;
	}
    pMib = (APMIB_Tp)buff;

linkchain:
#ifndef MIB_TLV
	for (j=0; j<NUM_WLAN_INTERFACE; j++)
		for (k=0; k<(NUM_VWLAN_INTERFACE+1); k++) // wlan[j][0] is for root
	{
		// initialize MAC access control list
		if ( !init_linkchain(&wlanMacChain[j][k], sizeof(MACFILTER_T), MAX_WLAN_AC_NUM))
			goto done;
		
		for (i=0; i<pMib->wlan[j][k].acNum; i++) {
			if ( !add_linkchain(&wlanMacChain[j][k], (char *)&pMib->wlan[j][k].acAddrArray[i]) )
				goto done;
		}
		wlanMacChain[j][k].compareLen = sizeof(MACFILTER_T) - COMMENT_LEN;

		// initialize WDS list
		if ( !init_linkchain(&wdsChain[j][k], sizeof(WDS_T), MAX_WDS_NUM))
			goto done;
		for (i=0; i<pMib->wlan[j][k].wdsNum; i++) {
			if ( !add_linkchain(&wdsChain[j][k], (char *)&pMib->wlan[j][k].wdsArray[i]) )
				goto done;
		}
		wdsChain[j][k].compareLen = sizeof(WDS_T) - COMMENT_LEN;
		
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	// initialize MAC access control list
	if ( !init_linkchain(&meshAclChain, sizeof(MACFILTER_T), MAX_MESH_ACL_NUM))
		goto done;

	for (i=0; i<pMib->meshAclNum; i++) {
		if ( !add_linkchain(&meshAclChain, (char *)&pMib->meshAclAddrArray[i]) )
			goto done;
	}
	meshAclChain.compareLen = sizeof(MACFILTER_T) - COMMENT_LEN;
#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	// initialize schedule table
	if ( !init_linkchain(&scheduleRuleChain, sizeof(SCHEDULE_T), MAX_SCHEDULE_NUM))
		goto done;
	for (i=0; i<pMib->scheduleRuleNum; i++) {
		if ( !add_linkchain(&scheduleRuleChain, (char *)&pMib->scheduleRuleArray[i]) )
			goto done;
	}
	scheduleRuleChain.compareLen = sizeof(SCHEDULE_T);
	




#ifdef HOME_GATEWAY
	// initialize port forwarding table
	if ( !init_linkchain(&portFwChain, sizeof(PORTFW_T), MAX_FILTER_NUM))
		goto done;

	for (i=0; i<pMib->portFwNum; i++) {
		if ( !add_linkchain(&portFwChain, (char *)&pMib->portFwArray[i]) )
			goto done;
	}
	portFwChain.compareLen = sizeof(PORTFW_T) - COMMENT_LEN;

	// initialize ip-filter table
	if ( !init_linkchain(&ipFilterChain, sizeof(IPFILTER_T), MAX_FILTER_NUM))
		goto done;
	for (i=0; i<pMib->ipFilterNum; i++) {
		if ( !add_linkchain(&ipFilterChain, (char *)&pMib->ipFilterArray[i]) )
			goto done;
	}
	ipFilterChain.compareLen = sizeof(IPFILTER_T) - COMMENT_LEN;

	// initialize port-filter table
	if ( !init_linkchain(&portFilterChain, sizeof(PORTFILTER_T), MAX_FILTER_NUM))
		goto done;
	for (i=0; i<pMib->portFilterNum; i++) {
		if ( !add_linkchain(&portFilterChain, (char *)&pMib->portFilterArray[i]) )
			goto done;
	}
	portFilterChain.compareLen = sizeof(PORTFILTER_T) - COMMENT_LEN;

	// initialize mac-filter table
	if ( !init_linkchain(&macFilterChain, sizeof(MACFILTER_T), MAX_FILTER_NUM))
		goto done;
	for (i=0; i<pMib->macFilterNum; i++) {
		if ( !add_linkchain(&macFilterChain, (char *)&pMib->macFilterArray[i]) )
			goto done;
	}
	macFilterChain.compareLen = sizeof(MACFILTER_T) - COMMENT_LEN;

	// initialize url-filter table
	if ( !init_linkchain(&urlFilterChain, sizeof(URLFILTER_T), MAX_URLFILTER_NUM))
		goto done;
	for (i=0; i<pMib->urlFilterNum; i++) {
		if ( !add_linkchain(&urlFilterChain, (char *)&pMib->urlFilterArray[i]) )
			goto done;
	}
	urlFilterChain.compareLen = sizeof(URLFILTER_T);// - COMMENT_LEN;

	// initialize trigger-port table
	if ( !init_linkchain(&triggerPortChain, sizeof(TRIGGERPORT_T), MAX_FILTER_NUM))
		goto done;
	for (i=0; i<pMib->triggerPortNum; i++) {
		if ( !add_linkchain(&triggerPortChain, (char *)&pMib->triggerPortArray[i]) )
			goto done;
	}
	triggerPortChain.compareLen = 5;	// length of trigger port range + proto type
#ifdef GW_QOS_ENGINE
	// initialize QoS rules table
	if ( !init_linkchain(&qosChain, sizeof(QOS_T), MAX_QOS_RULE_NUM)) {
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
		return 0;
	}
	for (i=0; i<pMib->qosRuleNum; i++) {
		if ( !add_linkchain(&qosChain, (char *)&pMib->qosRuleArray[i]) ) {
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
			return 0;
		}
	}
	qosChain.compareLen =  sizeof(QOS_T);
#endif

#ifdef QOS_BY_BANDWIDTH
	// initialize QoS rules table
	if ( !init_linkchain(&qosChain, sizeof(IPQOS_T), MAX_QOS_RULE_NUM)) {
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
		return 0;
	}
	for (i=0; i<pMib->qosRuleNum; i++) {
		if ( !add_linkchain(&qosChain, (char *)&pMib->qosRuleArray[i]) ) {
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
			return 0;
		}
	}
	qosChain.compareLen =  sizeof(IPQOS_T);
#endif

#ifdef ROUTE_SUPPORT
	// initialize static route table
	if ( !init_linkchain(&staticRouteChain, sizeof(STATICROUTE_T), MAX_ROUTE_NUM))
		goto done;
	for (i=0; i<pMib->staticRouteNum; i++) {
		if ( !add_linkchain(&staticRouteChain, (char *)&pMib->staticRouteArray[i]) )
			goto done;
	}
	staticRouteChain.compareLen = sizeof(STATICROUTE_T) -4 ; // not contain gateway
#endif //ROUTE
#ifdef VPN_SUPPORT
	// initialize port forwarding table
	if ( !init_linkchain(&ipsecTunnelChain, sizeof(IPSECTUNNEL_T), MAX_TUNNEL_NUM)) {
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
		return 0;
	}
	for (i=0; i<pMib->ipsecTunnelNum; i++) {
		if ( !add_linkchain(&ipsecTunnelChain, (char *)&pMib->ipsecTunnelArray[i]) ) {
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
			return 0;
		}
	}
	ipsecTunnelChain.compareLen = 1 ;  // only tunnel id
#endif
#endif // HOME_GATEWAY
#ifdef TLS_CLIENT
	if ( !init_linkchain(&certRootChain, sizeof(CERTROOT_T), MAX_CERTROOT_NUM))
		goto done;
	for (i=0; i<pMib->certRootNum; i++) {
		if ( !add_linkchain(&certRootChain, (char *)&pMib->certRootArray[i]) )
			goto done;
	}
	certRootChain.compareLen = 21 ;  // only comment
	if ( !init_linkchain(&certUserChain, sizeof(CERTUSER_T), MAX_CERTUSER_NUM))
		goto done;
	for (i=0; i<pMib->certUserNum; i++) {
		if ( !add_linkchain(&certUserChain, (char *)&pMib->certUserArray[i]) )
			goto done;
	}
	certUserChain.compareLen = 21 ;  // only comment	
#endif
	init_linkchain(&dhcpRsvdIpChain, sizeof(DHCPRSVDIP_T), MAX_DHCP_RSVD_IP_NUM);
	for (i=0; i<pMib->dhcpRsvdIpNum; i++)
		add_linkchain(&dhcpRsvdIpChain, (char *)&pMib->dhcpRsvdIpArray[i]);	
	dhcpRsvdIpChain.compareLen = 4;

#if defined(VLAN_CONFIG_SUPPORTED)
	init_linkchain(&vlanConfigChain, sizeof(VLAN_CONFIG_T), MAX_IFACE_VLAN_CONFIG);
	for (i=0; i<pMib->VlanConfigNum; i++)
		add_linkchain(&vlanConfigChain, (char *)&pMib->VlanConfigArray[i]);	
	vlanConfigChain.compareLen = sizeof(VLAN_CONFIG_T);

#endif
#endif /*no def MIB_TLV*/

	return 1;

#ifndef MIB_TLV
done:
	free(pMib);
	free(pMibDef);
	free(pHwSetting);
	pMib=NULL;
	pMibDef=NULL;
	pHwSetting=NULL;
	return 0;
#endif
}

int _apmib_init(void)
{
	int ret;
	
	apmib_sem_lock();
	ret = __apmib_init();
	apmib_sem_unlock();
	
#ifdef KLD_ENABLED
		memset(&WizMib, '\0', sizeof(WizMib));
		pWizMib = &WizMib;
		Wizard_load_mib();
#endif
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
int _apmib_reinit(void)
{
#ifndef MIB_TLV
	int i, j;
#endif
	int ret;
	
	apmib_sem_lock();

	if (pMib)
		free(pMib);
	if (pMibDef)
		free(pMibDef);
	if (pHwSetting)
		free(pHwSetting);
	pMib=NULL;
	pMibDef=NULL;
	pHwSetting=NULL;
	
#ifdef MIB_TLV
#else
	for (i=0; i<NUM_WLAN_INTERFACE; i++) 
		for (j=0; j<(NUM_VWLAN_INTERFACE+1); j++) 
	{
		free(wlanMacChain[i][j].buf);
		free(wdsChain[i][j].buf);
	}

#ifdef HOME_GATEWAY
	free(portFwChain.buf);
	free(ipFilterChain.buf);
	free(portFilterChain.buf);
	free(macFilterChain.buf);
	free(urlFilterChain.buf);
	free(triggerPortChain.buf);
#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
    	free(qosChain.buf);
#endif
#ifdef ROUTE_SUPPORT
	free(staticRouteChain.buf);
#endif //ROUTE
#ifdef VPN_SUPPORT
	free(ipsecTunnelChain.buf);
#endif
#endif
#ifdef TLS_CLIENT
	free(certRootChain.buf);
	free(certUserChain.buf);
#endif

	free(dhcpRsvdIpChain.buf);
#if defined(VLAN_CONFIG_SUPPORTED)
	free(vlanConfigChain.buf);
#endif	
#endif

	ret = __apmib_init();
	apmib_sem_unlock();
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
static int search_tbl(int id, mib_table_entry_T *pTbl, int *idx)
{
	int i;
	for (i=0; pTbl[i].id; i++) {
		if ( pTbl[i].id == id ) {
			*idx = i;
			return id;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)

#if !defined(CONFIG_CMJ_GPIO)
static int gpio_2g5g = 5;
#else
static int gpio_2g5g = 0;
#endif

int get_gpio_2g5g(void)
{
	return gpio_2g5g;
}

void set_gpio_2g5g(void)
{
	int val;
	if (REG32(BSP_PABCD_DAT) & (1 << 12))
		gpio_2g5g = 2;
	else
		gpio_2g5g = 5;
}


void select_gpio_2g5g(int *id, int *wlanIdx)
{

	if (get_gpio_2g5g() == 2) {
		if (*wlanIdx == 0)
			*wlanIdx = 1;
		else if (*wlanIdx == 1)
			*wlanIdx = 0;

		if (*id == MIB_REPEATER_SSID1)
			*id = MIB_REPEATER_SSID2;
		else if (*id == MIB_REPEATER_SSID2)
			*id = MIB_REPEATER_SSID1;

		if (*id == MIB_REPEATER_ENABLED1)
			*id = MIB_REPEATER_ENABLED2;
		else if (*id == MIB_REPEATER_ENABLED2)
			*id = MIB_REPEATER_ENABLED1;
	}
}
#endif /* #if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) */

#ifdef CONFIG_APP_TR069
int clone_wlaninfo_get(CONFIG_WLAN_SETTING_T *wlanptr, int rootwlan_idx,int vwlan_idx)
{
	
#if CONFIG_APMIB_SHARED_MEMORY == 1	
		apmib_sem_lock();
#endif
	//printf("clone_wlaninfo_set get wlan mib<%d, %d>\n",rootwlan_idx,vwlan_idx );
	memcpy(wlanptr, &pMib->wlan[rootwlan_idx][vwlan_idx], sizeof(CONFIG_WLAN_SETTING_T));

#if CONFIG_APMIB_SHARED_MEMORY == 1	
			apmib_sem_unlock();
#endif

	return 0;

}
int clone_wlaninfo_set(CONFIG_WLAN_SETTING_T *wlanptr, int rootwlan_idx,int vwlan_idx,int Newrootwlan_idx,int Newvwlan_idx, int ChangeRFBand )
{
	
#if CONFIG_APMIB_SHARED_MEMORY == 1	
		apmib_sem_lock();
#endif

	if(ChangeRFBand==1 && pMib->wlan[rootwlan_idx][vwlan_idx].wlanDisabled==0){
		//printf("set old wlan if to disabled if enabled in orig\n");
		pMib->wlan[rootwlan_idx][vwlan_idx].wlanDisabled=1;
	}
		//printf("clone_wlaninfo_set set wlan mib<%d, %d>\n",Newrootwlan_idx,Newvwlan_idx );
	memcpy(&pMib->wlan[Newrootwlan_idx][Newvwlan_idx], wlanptr, sizeof(CONFIG_WLAN_SETTING_T));

#if CONFIG_APMIB_SHARED_MEMORY == 1	
			apmib_sem_unlock();
#endif

	return 0;

}

#endif

////////////////////////////////////////////////////////////////////////////////
int __apmib_get(int id, void *value)
{
	int i, index;
	void *pMibTbl;
	mib_table_entry_T *pTbl;
	unsigned char ch;
	unsigned short wd;
	unsigned long dwd;
#ifdef MIB_TLV
	//unsigned int offset;
	unsigned int num;
#endif
	int wlanIdx = wlan_idx, vwlanIdx = vwlan_idx;

#ifdef MIB_EXT
	int using_ext=0;
	if(id & MIB_EXT_FLAG_MASK) {
		wlanIdx  = (id & MIB_EXT_INDEX1_MASK) >> MIB_EXT_INDEX1_OFFSET;
		vwlanIdx = (id & MIB_EXT_INDEX2_MASK) >> MIB_EXT_INDEX2_OFFSET;
		/*restore orig id value*/
		id &= ~(MIB_EXT_ALL_MAK);
		using_ext=1;
	}
#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	select_gpio_2g5g(&id, &wlanIdx);
#endif

	if ( search_tbl(id, mib_table, &i) ) {
		pMibTbl = (void *)pMib;
		pTbl = mib_table;
	}
	else if ( search_tbl(id, mib_wlan_table, &i) ) {
		pMibTbl = (void *)&pMib->wlan[wlanIdx][vwlanIdx];
		pTbl = mib_wlan_table;
	}
	else if ( search_tbl(id, hwmib_table, &i) ) {
		pMibTbl = (void *)pHwSetting;
		pTbl = hwmib_table;
	}
	else if ( search_tbl(id, hwmib_wlan_table, &i) ) {
	#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		wlanIdx = 0;
	#endif
		pMibTbl = (void *)&pHwSetting->wlan[wlanIdx];
		pTbl = hwmib_wlan_table;
	}
	else {
		//diag_printf("%s:%d can't find id=%d\n",__FUNCTION__,__LINE__,id);
		return 0;
	}
	
#ifdef MIB_TLV
	if(pTbl[i].type > TABLE_LIST_T)
	{		
		__apmib_get(((pTbl[i].id & MIB_ID_MASK)-1),&num);
		index = (int)( *((unsigned char *)value));
		//printf("get index %d\n",index);
		get_tblentry((void*)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value,index);
		return 1;
	}
#endif

	switch (pTbl[i].type) {
	case BYTE_T:
		
//		*((int *)value) =(int)(*((unsigned char *)(((long)pMibTbl) + pTbl[i].offset)));
		memcpy((char *)&ch, ((char *)pMibTbl) + pTbl[i].offset, 1);		
		*((int *)value) = (int)ch;		
		break;

	case WORD_T:
//		*((int *)value) =(int)(*((unsigned short *)(((long)pMibTbl) + pTbl[i].offset)));
		memcpy((char *)&wd, ((char *)pMibTbl) + pTbl[i].offset, 2);
		*((int *)value) = (int)wd;
		break;

	case STRING_T:
		strcpy( (char *)value, (const char *)(((long)pMibTbl) + pTbl[i].offset) );
		break;

	case BYTE5_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 5);
		break;

	case BYTE6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 6);
		break;

	case BYTE13_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 13);
		break;

	case DWORD_T:
		memcpy((char *)&dwd, ((char *)pMibTbl) + pTbl[i].offset, 4);
		*((int *)value) = (int)dwd;
		break;

	case BYTE_ARRAY_T:
#ifdef VOIP_SUPPORT
		if(id == MIB_VOIP_CFG){
			// rock: do nothing here, use flash voip get xxx to replace
		}
		else
#endif /*VOIP_SUPPORT*/
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;

	case IA_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 4);
		break;

#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
		
	case DNSV6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case DHCPV6S_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case DHCPV6C_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case ADDR6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case ADDRV6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case TUNNEL6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;	
#endif
#endif

#ifndef MIB_TLV
	case WLAC_ARRAY_T:
		index = (int)( *((unsigned char *)value));
		return get_linkchain(&wlanMacChain[wlanIdx][vwlanIdx], (char *)value, index );

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	case MESH_ACL_ARRAY_T:
		index = (int)( *((unsigned char *)value));
		return get_linkchain(&meshAclChain, (char *)value, index );
#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	case WDS_ARRAY_T:
		index = (int)( *((unsigned char *)value));
		return get_linkchain(&wdsChain[wlanIdx][vwlanIdx], (char *)value, index );

	case SCHEDULE_ARRAY_T:
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&scheduleRuleChain, (char *)value, index ); 		


#ifdef HOME_GATEWAY
	case PORTFW_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&portFwChain, (char *)value, index );

	case IPFILTER_ARRAY_T:
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&ipFilterChain, (char *)value, index );

	case PORTFILTER_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&portFilterChain, (char *)value, index );

	case MACFILTER_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&macFilterChain, (char *)value, index );

	case URLFILTER_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&urlFilterChain, (char *)value, index );

	case TRIGGERPORT_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&triggerPortChain, (char *)value, index );

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	case QOS_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&qosChain, (char *)value, index );
#endif
#ifdef ROUTE_SUPPORT
	case STATICROUTE_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&staticRouteChain, (char *)value, index );
#endif

#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&ipsecTunnelChain, (char *)value, index );
#endif

#endif /*HOME_GATEWAY*/
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&certRootChain, (char *)value, index );
	case CERTUSER_ARRAY_T:
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&certUserChain, (char *)value, index ); 		
#endif
	case DHCPRSVDIP_ARRY_T:	
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&dhcpRsvdIpChain, (char *)value, index );

#if defined(VLAN_CONFIG_SUPPORTED)
	case VLANCONFIG_ARRAY_T:	
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&vlanConfigChain, (char *)value, index ); 		
#endif 	
#endif /*no def MIB_TLV*/
	default:
		printf("%s:%d can't find type %d\n",__FUNCTION__,__LINE__,pTbl[i].type);
		return 0;
	}

	return 1;
}

int _apmib_get(int id, void *value)
{
	int ret;
	
	apmib_sem_lock();
	ret = __apmib_get(id, value);
	apmib_sem_unlock();
	return ret;
}
#ifdef MIB_EXT

/*if idx==-1 to backup orig usage*/
/*so idx should between -1 and max idx*/
/* return value 0-- invalid 1 valid */
int ext_idx_valid_check(int idx1, int idx2)
{
	if(idx1 < -1 || idx1 > MIB_EXT_INDEX1_MAX)
		return 0;
	if(idx2 < -1 || idx2 > MIB_EXT_INDEX2_MAX)
		return 0;
	return 1;
}

int _apmib_get_ext(int id, void *value, int idx1, int idx2)
{
	if(!ext_idx_valid_check(idx1,idx2))
		return -1;
	/*idx1 idx2 == -1. is orig usage */
	if(-1 == idx1 && -1 == idx2)
	{
		/*back compatible. especially for global idx case*/
		return _apmib_get(id,value);
	}
	
	if(-1 != idx1)
		id |= (idx1<<MIB_EXT_INDEX1_OFFSET) ;

	if(-1 != idx2)
		id |= (idx2 << MIB_EXT_INDEX2_OFFSET);
	
	id |= MIB_EXT_FLAG_MASK;
	return _apmib_get(id,value);
}


int _apmib_set_ext(int id, void *value, int idx1, int idx2)
{
	int wan_old_type;
	if(!ext_idx_valid_check(idx1,idx2))
		return -1;
	
	/*idx1 idx2 == -1. is orig usage */
	if(-1 == idx1 && -1 == idx2)
	{
		/*when set WAN_DHCP with flash cmd, it should set the WAN_OLD_DHCP first*/
		if(id==MIB_WAN_DHCP)
		{			
			_apmib_get(MIB_WAN_DHCP,(void *)&wan_old_type);
			//printf("%s:%d##wan_old_type=%d\n",__FUNCTION__,__LINE__,wan_old_type);
			_apmib_set(MIB_WAN_OLD_DHCP, (void *)&wan_old_type);
		}		
		
		/*back compatible. especially for global idx case*/
		return _apmib_set(id,value);
	}
	
	if(-1 != idx1)
		id |= (idx1<<MIB_EXT_INDEX1_OFFSET) ;

	if(-1 != idx2)
		id |= (idx2 << MIB_EXT_INDEX2_OFFSET);
	
	id |= MIB_EXT_FLAG_MASK;
	return _apmib_set(id,value);
}

int _apmib_getDef_ext(int id, void *value, int idx1, int idx2)
{
	int ret;
	APMIB_Tp saveMib;
	saveMib = pMib;
	pMib = pMibDef;
	ret = _apmib_get_ext(id, value,idx1,idx2);
	pMib = saveMib;
	return ret;
}

int _apmib_setDef_ext(int id, void *value, int idx1, int idx2)
{
	int ret;
	APMIB_Tp saveMib;
	saveMib = pMib;
	pMib = pMibDef;
	ret = _apmib_set_ext(id, value,idx1,idx2);
	pMib = saveMib;
	return ret;
}

#endif
////////////////////////////////////////////////////////////////////////////////
int _apmib_getDef(int id, void *value)
{
	int ret;
	APMIB_Tp saveMib;

	apmib_sem_lock();
	saveMib = pMib;
	pMib = pMibDef;
	ret = __apmib_get(id, value);
	pMib = saveMib;
	apmib_sem_unlock();
	return ret;
}


////////////////////////////////////////////////////////////////////////////////
int __apmib_set(int id, void *value)
{
	int i=0, ret=1;
	void *pMibTbl=NULL;
	mib_table_entry_T *pTbl=NULL;
	unsigned char ch=0;
	unsigned short wd=0;
	unsigned long dwd=0;
	unsigned char* tmp=NULL;
	int max_chan_num=MAX_2G_CHANNEL_NUM_MIB;
#ifdef MIB_TLV
	//unsigned int offset=0;
	unsigned int mib_num_id=0;
	unsigned int num=0;
	unsigned int id_orig=0;
#endif
	int wlanIdx = wlan_idx, vwlanIdx = vwlan_idx;
#ifdef MIB_EXT
	int using_ext=0;
#endif
#if defined(MIB_MOD_TBL_ENTRY)
	unsigned int mod_tbl=0;
#endif

#ifdef MIB_EXT
	if(id & MIB_EXT_FLAG_MASK) {
		wlanIdx  = (id & MIB_EXT_INDEX1_MASK) >> MIB_EXT_INDEX1_OFFSET;
		vwlanIdx = (id & MIB_EXT_INDEX2_MASK) >> MIB_EXT_INDEX2_OFFSET;
		/*restore orig id value*/
		id &= ~(MIB_EXT_ALL_MAK);
		using_ext=1;
	}
#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	select_gpio_2g5g(&id, &wlanIdx);
#endif

#ifdef MIB_TLV
	id_orig = id;
	if( id_orig & MIB_ADD_TBL_ENTRY)
	{
		id=((id_orig & MIB_ID_MASK)-1 )|(MIB_TABLE_LIST);
		mib_num_id=(id_orig & MIB_ID_MASK)-2;

	}
	else if(id_orig & MIB_DEL_TBL_ENTRY)
	{
#if defined(MIB_MOD_TBL_ENTRY)
			if (id_orig & MIB_MOD_TBL_ENTRY) {
				id_orig &= ~MIB_MOD_TBL_ENTRY;
				id = id_orig;
				mod_tbl = 1;
			}
#endif
		id=((id_orig & MIB_ID_MASK)-2 )|(MIB_TABLE_LIST);
		mib_num_id=(id_orig & MIB_ID_MASK)-3;	
	}
	else if(id_orig & MIB_DELALL_TBL_ENTRY)
	{
		id=((id_orig & MIB_ID_MASK)-3 )|(MIB_TABLE_LIST);
		mib_num_id=(id_orig & MIB_ID_MASK)-4;	
	}
#else

	if (id == MIB_WLAN_AC_ADDR_ADD) {
		ret = add_linkchain(&wlanMacChain[wlanIdx][vwlanIdx], (char *)value);
		if ( ret )
			pMib->wlan[wlanIdx][vwlanIdx].acNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_WLAN_AC_ADDR_DEL) {
		ret = delete_linkchain(&wlanMacChain[wlanIdx][vwlanIdx], (char *)value);
		if ( ret )
			pMib->wlan[wlanIdx][vwlanIdx].acNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_WLAN_AC_ADDR_DELALL) {
		delete_all_linkchain(&wlanMacChain[wlanIdx][vwlanIdx]);
		pMib->wlan[wlanIdx][vwlanIdx].acNum = 0;
		return 1;
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	if (id == MIB_WLAN_MESH_ACL_ADDR_ADD) {
		ret = add_linkchain(&meshAclChain, (char *)value);
		if ( ret )
			pMib->meshAclNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_WLAN_MESH_ACL_ADDR_DEL) {
		ret = delete_linkchain(&meshAclChain, (char *)value);
		if ( ret )
			pMib->meshAclNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_WLAN_MESH_ACL_ADDR_DELALL) {
		delete_all_linkchain(&meshAclChain);
		pMib->meshAclNum = 0;
		return 1;
	}
#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	if (id == MIB_WLAN_WDS_ADD) {
		ret = add_linkchain(&wdsChain[wlanIdx][vwlanIdx], (char *)value);
		if ( ret )
			pMib->wlan[wlanIdx][vwlanIdx].wdsNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_WLAN_WDS_DEL) {
		ret = delete_linkchain(&wdsChain[wlanIdx][vwlanIdx], (char *)value);
		if ( ret )
			pMib->wlan[wlanIdx][vwlanIdx].wdsNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_WLAN_WDS_DELALL) {
		delete_all_linkchain(&wdsChain[wlanIdx][vwlanIdx]);
		pMib->wlan[wlanIdx][vwlanIdx].wdsNum = 0;
		return 1;
	}
//Schedule Mib
	if (id == MIB_SCHEDULE_ADD) {
		ret = add_linkchain(&scheduleRuleChain, (char *)value);
		if ( ret )
			pMib->scheduleRuleNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_SCHEDULE_DEL) {
		ret = delete_linkchain(&scheduleRuleChain, (char *)value);
		if ( ret )
			pMib->scheduleRuleNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_SCHEDULE_DELALL) {
		delete_all_linkchain(&scheduleRuleChain);
		pMib->scheduleRuleNum = 0;
		return 1;
	}

#if defined(VLAN_CONFIG_SUPPORTED)
	if (id == MIB_VLANCONFIG_ADD || id == MIB_VLANCONFIG_DEL) {
		int entryNum=0,i;
		VLAN_CONFIG_T entry;
		VLAN_CONFIG_Tp entry_new;
		entry_new = (VLAN_CONFIG_Tp)value;
		__apmib_get(MIB_VLANCONFIG_TBL_NUM, (void *)&entryNum);
		for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
			__apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry);
			if(!strcmp(entry.netIface, entry_new->netIface)){
				update_linkchain(VLANCONFIG_ARRAY_T, &entry, entry_new, sizeof(VLAN_CONFIG_T));
				break;
			}
		}
		return 1;
	}

#if defined(VLAN_CONFIG_SUPPORTED)
	if (id == MIB_VLANCONFIG_DELALL) {
		int entryNum=0,i;
		VLAN_CONFIG_T entry;
		VLAN_CONFIG_T entry_new;
		__apmib_get(MIB_VLANCONFIG_TBL_NUM, (void *)&entryNum);

		for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
			__apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry);
			memcpy(&entry_new, &entry, sizeof(VLAN_CONFIG_T));
			entry_new.enabled=0;
			update_linkchain(VLANCONFIG_ARRAY_T, &entry, &entry_new, sizeof(VLAN_CONFIG_T));
		}
#endif		
		return 1;
	}

#endif

#ifdef HOME_GATEWAY
	if (id == MIB_PORTFW_ADD) {
		ret = add_linkchain(&portFwChain, (char *)value);
		if ( ret )
			pMib->portFwNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_PORTFW_DEL) {
		ret = delete_linkchain(&portFwChain, (char *)value);
		if ( ret )
			pMib->portFwNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_PORTFW_DELALL) {
		delete_all_linkchain(&portFwChain);
		pMib->portFwNum = 0;
		return 1;
	}

	if (id == MIB_IPFILTER_ADD) {
		ret = add_linkchain(&ipFilterChain, (char *)value);
		if ( ret )
			pMib->ipFilterNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_IPFILTER_DEL) {
		ret = delete_linkchain(&ipFilterChain, (char *)value);
		if ( ret )
			pMib->ipFilterNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_IPFILTER_DELALL) {
		delete_all_linkchain(&ipFilterChain);
		pMib->ipFilterNum = 0;
		return 1;
	}

	if (id == MIB_PORTFILTER_ADD) {
		ret = add_linkchain(&portFilterChain, (char *)value);
		if ( ret )
			pMib->portFilterNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_PORTFILTER_DEL) {
		ret = delete_linkchain(&portFilterChain, (char *)value);
		if ( ret )
			pMib->portFilterNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_PORTFILTER_DELALL) {
		delete_all_linkchain(&portFilterChain);
		pMib->portFilterNum = 0;
		return 1;
	}

	if (id == MIB_MACFILTER_ADD) {
		ret = add_linkchain(&macFilterChain, (char *)value);
		if ( ret )
			pMib->macFilterNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_MACFILTER_DEL) {
		ret = delete_linkchain(&macFilterChain, (char *)value);
		if ( ret )
			pMib->macFilterNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_MACFILTER_DELALL) {
		delete_all_linkchain(&macFilterChain);
		pMib->macFilterNum = 0;
		return 1;
	}

	if (id == MIB_URLFILTER_ADD) {
		ret = add_linkchain(&urlFilterChain, (char *)value);
		if ( ret )
			pMib->urlFilterNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_URLFILTER_DEL) {
		ret = delete_linkchain(&urlFilterChain, (char *)value);
		if ( ret )
			pMib->urlFilterNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_URLFILTER_DELALL) {
		delete_all_linkchain(&urlFilterChain);
		pMib->urlFilterNum = 0;
		return 1;
	}

	if (id == MIB_TRIGGERPORT_ADD) {
		ret = add_linkchain(&triggerPortChain, (char *)value);
		if ( ret )
			pMib->triggerPortNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_TRIGGERPORT_DEL) {
		ret = delete_linkchain(&triggerPortChain, (char *)value);
		if ( ret )
			pMib->triggerPortNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_TRIGGERPORT_DELALL) {
		delete_all_linkchain(&triggerPortChain);
		pMib->triggerPortNum = 0;
		return 1;
	}

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	if (id == MIB_QOS_ADD) {
		ret = add_linkchain(&qosChain, (char *)value);
		if ( ret )
			pMib->qosRuleNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_QOS_DEL) {
		ret = delete_linkchain(&qosChain, (char *)value);
		if ( ret )
			pMib->qosRuleNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_QOS_DELALL) {
		delete_all_linkchain(&qosChain);
		pMib->qosRuleNum = 0;
		return 1;
	}
#endif
#ifdef ROUTE_SUPPORT
	if (id == MIB_STATICROUTE_ADD) {
		ret = add_linkchain(&staticRouteChain, (char *)value);
		if ( ret )
			pMib->staticRouteNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_STATICROUTE_DEL) {
		ret = delete_linkchain(&staticRouteChain, (char *)value);
		if ( ret )
			pMib->staticRouteNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_STATICROUTE_DELALL) {
		delete_all_linkchain(&staticRouteChain);
		pMib->staticRouteNum = 0;
		return 1;
	}
#endif //ROUTE
#endif

	if (id == MIB_DHCPRSVDIP_DEL) {
		ret = delete_linkchain(&dhcpRsvdIpChain, (char *)value);
		if ( ret )
			pMib->dhcpRsvdIpNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_DHCPRSVDIP_DELALL) {
		delete_all_linkchain(&dhcpRsvdIpChain);
		pMib->dhcpRsvdIpNum = 0;
		return 1;
	}
	if (id == MIB_DHCPRSVDIP_ADD) {
		ret = add_linkchain(&dhcpRsvdIpChain, (char *)value);
		if ( ret )
			pMib->dhcpRsvdIpNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	if (id == MIB_IPSECTUNNEL_ADD) {
		ret = add_linkchain(&ipsecTunnelChain, (char *)value);
		if ( ret )
			pMib->ipsecTunnelNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_IPSECTUNNEL_DEL) {
		ret = delete_linkchain(&ipsecTunnelChain, (char *)value);
		if ( ret )
			pMib->ipsecTunnelNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_IPSECTUNNEL_DELALL) {
		delete_all_linkchain(&ipsecTunnelChain);
		pMib->ipsecTunnelNum= 0;
		return 1;
	}
#endif
#endif
#ifdef TLS_CLIENT
	if (id == MIB_CERTROOT_ADD) {
		ret = add_linkchain(&certRootChain, (char *)value);
		if ( ret )
			pMib->certRootNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_CERTROOT_DEL) {
		ret = delete_linkchain(&certRootChain, (char *)value);
		if ( ret )
			pMib->certRootNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_CERTROOT_DELALL) {
		delete_all_linkchain(&certRootChain);
		pMib->certRootNum= 0;
		return 1;
	}
	if (id == MIB_CERTUSER_ADD) {
		ret = add_linkchain(&certUserChain, (char *)value);
		if ( ret )
			pMib->certUserNum++;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_CERTUSER_DEL) {
		ret = delete_linkchain(&certUserChain, (char *)value);
		if ( ret )
			pMib->certUserNum--;
		else
			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}
	if (id == MIB_CERTUSER_DELALL) {
		delete_all_linkchain(&certUserChain);
		pMib->certUserNum= 0;
		return 1;
	}	
#endif
#endif /*MIB_TLV*/

	if ( search_tbl(id, mib_table, &i) ) {
		pMibTbl = (void *)pMib;
		pTbl = mib_table;
	}
	else if ( search_tbl(id, mib_wlan_table, &i) ) {
		pMibTbl = (void *)&pMib->wlan[wlanIdx][vwlanIdx];
		pTbl = mib_wlan_table;
	}
	else if ( search_tbl(id, hwmib_table, &i) ) {
		pMibTbl = (void *)pHwSetting;
		pTbl = hwmib_table;
	}
	else if ( search_tbl(id, hwmib_wlan_table, &i) ) {
	#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		wlanIdx = 0;
	#endif	
		pMibTbl = (void *)&pHwSetting->wlan[wlanIdx];			
		pTbl = hwmib_wlan_table;
	}
	else {
		printf("id not found\n");
		return 0;	
	}

#ifdef MIB_TLV
	if(pTbl[i].type > TABLE_LIST_T)
	{		
		__apmib_get(mib_num_id,&num);
		if(id_orig & MIB_ADD_TBL_ENTRY)
		{
			ret= add_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value);
			if(ret)
				num++;
		}
		else if(id_orig & MIB_DEL_TBL_ENTRY)
		{
#if defined(MIB_MOD_TBL_ENTRY)
			if (mod_tbl == 1) {
				ret= mod_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value);
			}
			else 
#endif
			{
				ret= delete_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value);
				if(ret)
					num--;
			}
		}
		else if(id_orig & MIB_DELALL_TBL_ENTRY)
		{
			ret= delete_all_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i]);
			if(ret)
				num=0;
		}

		if(ret)
			__apmib_set(mib_num_id,&num);	
		//else
//			printf("%s:%d \n",__FUNCTION__,__LINE__);
		//printf("num %d ret %d\n",num,ret);
		return ret;
	}
#endif

	switch (pTbl[i].type) {
	case BYTE_T:		
//		*((unsigned char *)(((long)pMibTbl) + pTbl[i].offset)) = (unsigned char)(*((int *)value));
		ch = (unsigned char)(*((int *)value));
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, &ch, 1);
		break;

	case WORD_T:
//		*((unsigned short *)(((long)pMibTbl) + pTbl[i].offset)) = (unsigned short)(*((int *)value));
		wd = (unsigned short)(*((int *)value));
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, &wd, 2);
		break;

	case STRING_T:
		if (value && strlen(value)+1 > pTbl[i].size )
		{
			printf("%s:%d \n",__FUNCTION__,__LINE__);
			return 0;
		}
		if (value==NULL || strlen(value)==0)
			*((char *)(((long)pMibTbl) + pTbl[i].offset)) = '\0';
		else
			strcpy((char *)(((long)pMibTbl) + pTbl[i].offset), (char *)value);
		break;

	case BYTE5_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 5);
		break;

	case BYTE6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 6);
		break;

	case BYTE13_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 13);
		break;

	case DWORD_T:
		dwd = (unsigned long)(*((int *)value));
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, &dwd, 4);
		break;
	case BYTE_ARRAY_T:
		tmp = (unsigned char*) value;
#ifdef VPN_SUPPORT
		if(id == MIB_IPSEC_RSA_FILE){
                        memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, MAX_RSA_FILE_LEN);
		}
		else
#endif

#ifdef VOIP_SUPPORT
		if(id == MIB_VOIP_CFG){
			printf("apimb: mib_set MIB_VOIP_CFG\n");

			memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, pTbl[i].size);

		}
		else
#endif /*VOIP_SUPPORT*/

		{
#if 1// defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
                        if((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM))
				max_chan_num = MAX_2G_CHANNEL_NUM_MIB;
			else if((id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM)
#if defined(CONFIG_WLAN_HAL_8814AE)
					||(id >= MIB_HW_TX_POWER_5G_HT40_1S_C &&  id <=MIB_HW_TX_POWER_5G_HT40_1S_D)
#endif
			)
				max_chan_num = MAX_5G_CHANNEL_NUM_MIB;         

#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
			if(((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_A)) 
				|| ((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_B)) )
				max_chan_num = MAX_2G_CHANNEL_NUM_MIB;

			if(((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_A)) 
				|| ((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_B)) )
				max_chan_num = MAX_5G_DIFF_NUM;
#endif

			if(tmp[0]==2){
				if(tmp[3] == 0xff){ // set one channel value
					memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset + (long)tmp[1] -1), (unsigned char *)(tmp+2), 1);
				}
			}else{
					memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value+1, max_chan_num);
				}		
		}
		break;
	case IA_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  4);
		break;
#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(radvdCfgParam_t));
		break;

	case DNSV6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(dnsv6CfgParam_t));
		break;
	case DHCPV6S_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(dhcp6sCfgParam_t));
		break;
	case DHCPV6C_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(dhcp6cCfgParam_t));
		break;
	case ADDR6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(addrIPv6CfgParam_t));
		break;
	case ADDRV6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(addr6CfgParam_t));
		break;
	case TUNNEL6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(tunnelCfgParam_t));
		break;		
#endif
#endif
	case WLAC_ARRAY_T:
	
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
	case MESH_ACL_ARRAY_T:
#endif

	case WDS_ARRAY_T:
	case SCHEDULE_ARRAY_T:
#ifdef HOME_GATEWAY
	case PORTFW_ARRAY_T:
	case IPFILTER_ARRAY_T:
	case PORTFILTER_ARRAY_T:
	case MACFILTER_ARRAY_T:
	case URLFILTER_ARRAY_T:
	case TRIGGERPORT_ARRAY_T:

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	case QOS_ARRAY_T:
#endif
#ifdef ROUTE_SUPPORT
	case STATICROUTE_ARRAY_T:
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
#endif
#endif
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
	case CERTUSER_ARRAY_T:
#endif
	case DHCPRSVDIP_ARRY_T:		
#if defined(VLAN_CONFIG_SUPPORTED)	
	case VLANCONFIG_ARRAY_T:		
#endif
#ifdef WLAN_PROFILE
	case PROFILE_ARRAY_T:		
#endif

			printf("%s:%d \n",__FUNCTION__,__LINE__);
		return 0;
	default:
		break;
	}
	return 1;
}

int _apmib_set(int id, void *value)
{
	int ret;
	
	apmib_sem_lock();
	ret = __apmib_set(id, value);
	apmib_sem_unlock();
	return ret;
}
////////////////////////////////////////////////////////////////////////////////
int _apmib_setDef(int id, void *value)
{
	int ret;
	APMIB_Tp saveMib;

	apmib_sem_lock();
	saveMib = pMib;
	pMib = pMibDef;
	ret = __apmib_set(id, value);
	pMib = saveMib;
	apmib_sem_unlock();
	return ret;
}



////////////////////////////////////////////////////////////////////////////////
/* Update current used MIB into flash in current setting area
 */
int _apmib_update(CONFIG_DATA_T type)
{
	int i, len;
#ifndef MIB_TLV
	int j, k;
#endif
#ifdef _LITTLE_ENDIAN_
	unsigned char *pdata;
#endif
	unsigned char checksum;
	unsigned char *data, *buff;
#ifdef MIB_TLV
	unsigned char *pfile = NULL;
	unsigned char *mib_tlv_data = NULL;
	unsigned int tlv_content_len = 0;
	unsigned int mib_tlv_max_len = 0;
#endif

	apmib_sem_lock();
	//diag_printf("[%s:%d]type=%d\n",__FUNCTION__, __LINE__, type);

	if (type & HW_SETTING) {
#ifdef _LITTLE_ENDIAN_
		//diag_printf("[%s:%d]hsHeader.len=%d\n", __FUNCTION__, __LINE__, hsHeader.len);
        data=malloc(hsHeader.len);
        if(data == NULL)
        {    
            printf("malloc failed\n");
            return -1;
        }    
        pdata=data;
        memcpy(data,pHwSetting, hsHeader.len);     
        swap_mib_value((HW_SETTING_Tp)data,HW_SETTING);
#else
		data = (unsigned char *)pHwSetting;
#endif
		checksum = CHECKSUM(data, hsHeader.len-1);
		data[hsHeader.len-1] = checksum;
#if 0
#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
		mib_tlv_max_len = mib_get_setting_len(type)*4;
//fprintf(stderr,"\r\n mib_tlv_max_len = %p, __[%s-%u]",mib_tlv_max_len,__FILE__,__LINE__);

		pfile = malloc(mib_tlv_max_len);
		tlv_content_len = 0;
		
//mib_display_data_content(HW_SETTING, data, hsHeader.len);		

		if(pfile != NULL && mib_tlv_save(type, (void*)data, pfile, &tlv_content_len) == 1)
		{

			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}
					
			free(pfile);
			pfile = NULL;

		}
		
		if(mib_tlv_data != NULL)
		{
			hsHeader.len = tlv_content_len+1;
			data = mib_tlv_data;
			data[tlv_content_len] = CHECKSUM(data, tlv_content_len);
//mib_display_tlv_content(HW_SETTING, data, hsHeader.len);		
		}

#endif // #ifdef MIB_TLV		
		if( mib_compress_write(type, data) == 1)
		{

		}	
		else
#endif //#ifedf COMPRESS_MIB_SETTING
#endif
		
		buff = malloc(hsHeader.len+sizeof(hsHeader));
		if (buff == NULL) {
			printf("allocate HS buffer failed!\n");	
			apmib_sem_unlock();
			return -1;
		}
        hsHeader.len = WORD_SWAP(hsHeader.len); //for endian
		memcpy(buff, &hsHeader, sizeof(hsHeader));
        hsHeader.len = WORD_SWAP(hsHeader.len);
		memcpy(buff+sizeof(hsHeader), data, hsHeader.len);
		if ( rtk_flash_write((char *)buff, HW_SETTING_OFFSET, (hsHeader.len+sizeof(hsHeader)))==0 ) {
			printf("write hs MIB failed!\n");
			free(buff);
			apmib_sem_unlock();
#if 0
#ifdef MIB_TLV
			if(mib_tlv_data)
				free(mib_tlv_data);
			
			if(pfile)
				free(pfile);
#endif
#endif
#ifdef _LITTLE_ENDIAN_
			free(pdata);
#endif
			return 0;
		}
		free(buff);
#ifdef _LITTLE_ENDIAN_
		free(pdata);
#endif
	}


	if ((type & CURRENT_SETTING) || (type & DEFAULT_SETTING)) {
		
#ifndef MIB_TLV
		for (j=0; j<NUM_WLAN_INTERFACE; j++) 
			for (k=0; k<(NUM_VWLAN_INTERFACE+1); k++)
		{
			memset( pMib->wlan[j][k].acAddrArray, '\0', MAX_WLAN_AC_NUM*sizeof(MACFILTER_T) );
			for (i=0; i<pMib->wlan[j][k].acNum; i++) {
				get_linkchain(&wlanMacChain[j][k], (void *)&pMib->wlan[j][k].acAddrArray[i], i+1);
			}
			memset( pMib->wlan[j][k].wdsArray, '\0', MAX_WDS_NUM*sizeof(WDS_T) );
			for (i=0; i<pMib->wlan[j][k].wdsNum; i++) {
				get_linkchain(&wdsChain[j][k], (void *)&pMib->wlan[j][k].wdsArray[i], i+1);
			}
		}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
		memset( pMib->meshAclAddrArray, '\0', MAX_MESH_ACL_NUM*sizeof(MACFILTER_T) );
		for (i=0; i<pMib->meshAclNum; i++) {
			get_linkchain(&meshAclChain, (void *)&pMib->meshAclAddrArray[i], i+1);
		}
#endif

		memset( pMib->scheduleRuleArray, '\0', MAX_SCHEDULE_NUM*sizeof(SCHEDULE_T) );
		for (i=0; i<pMib->scheduleRuleNum; i++) {
			get_linkchain(&scheduleRuleChain, (void *)&pMib->scheduleRuleArray[i], i+1);
		}
#if	1//def HOME_GATEWAY
		memset( pMib->portFwArray, '\0', MAX_FILTER_NUM*sizeof(PORTFW_T) );
		for (i=0; i<pMib->portFwNum; i++) {
			get_linkchain(&portFwChain, (void *)&pMib->portFwArray[i], i+1);
		}

		memset( pMib->ipFilterArray, '\0', MAX_FILTER_NUM*sizeof(IPFILTER_T) );
		for (i=0; i<pMib->ipFilterNum; i++) {
			get_linkchain(&ipFilterChain, (void *)&pMib->ipFilterArray[i], i+1);
		}
		memset( pMib->portFilterArray, '\0', MAX_FILTER_NUM*sizeof(PORTFILTER_T) );
		for (i=0; i<pMib->portFilterNum; i++) {
			get_linkchain(&portFilterChain, (void *)&pMib->portFilterArray[i], i+1);
		}
		memset( pMib->macFilterArray, '\0', MAX_FILTER_NUM*sizeof(MACFILTER_T) );
		for (i=0; i<pMib->macFilterNum; i++) {
			get_linkchain(&macFilterChain, (void *)&pMib->macFilterArray[i], i+1);
		}
		memset( pMib->urlFilterArray, '\0', MAX_URLFILTER_NUM*sizeof(URLFILTER_T) );
		for (i=0; i<pMib->urlFilterNum; i++) {
			get_linkchain(&urlFilterChain, (void *)&pMib->urlFilterArray[i], i+1);
		}
		memset( pMib->triggerPortArray, '\0', MAX_FILTER_NUM*sizeof(TRIGGERPORT_T) );
		for (i=0; i<pMib->triggerPortNum; i++) {
			get_linkchain(&triggerPortChain, (void *)&pMib->triggerPortArray[i], i+1);
		}
#ifdef GW_QOS_ENGINE
		memset( pMib->qosRuleArray, '\0', MAX_QOS_RULE_NUM*sizeof(QOS_T) );
		for (i=0; i<pMib->qosRuleNum; i++) {
			get_linkchain(&qosChain, (void *)&pMib->qosRuleArray[i], i+1);
		}
#endif

#ifdef QOS_BY_BANDWIDTH
		memset( pMib->qosRuleArray, '\0', MAX_QOS_RULE_NUM*sizeof(IPQOS_T) );
		for (i=0; i<pMib->qosRuleNum; i++) {
			get_linkchain(&qosChain, (void *)&pMib->qosRuleArray[i], i+1);
		}
#endif

#ifdef ROUTE_SUPPORT
		memset( pMib->staticRouteArray, '\0', MAX_ROUTE_NUM*sizeof(STATICROUTE_T) );
		for (i=0; i<pMib->staticRouteNum; i++) {
			get_linkchain(&staticRouteChain, (void *)&pMib->staticRouteArray[i], i+1);
		}
#endif //ROUTE

#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
		memset( pMib->ipsecTunnelArray, '\0', MAX_TUNNEL_NUM*sizeof(IPSECTUNNEL_T) );
		for (i=0; i<pMib->ipsecTunnelNum; i++) {
			get_linkchain(&ipsecTunnelChain, (void *)&pMib->ipsecTunnelArray[i], i+1);
		}
#endif
#endif
#ifdef TLS_CLIENT
		memset( pMib->certRootArray, '\0', MAX_CERTROOT_NUM*sizeof(CERTROOT_T) );
		for (i=0; i<pMib->certRootNum; i++) {
			get_linkchain(&certRootChain, (void *)&pMib->certRootArray[i], i+1);
		}
		memset( pMib->certUserArray, '\0', MAX_CERTUSER_NUM*sizeof(CERTUSER_T) );
		for (i=0; i<pMib->certUserNum; i++) {
			get_linkchain(&certUserChain, (void *)&pMib->certUserArray[i], i+1);
		}		
#endif
		memset(pMib->dhcpRsvdIpArray, '\0', MAX_DHCP_RSVD_IP_NUM*sizeof(DHCPRSVDIP_T));
		for (i=0; i<pMib->dhcpRsvdIpNum; i++) {
			get_linkchain(&dhcpRsvdIpChain, (void *)&pMib->dhcpRsvdIpArray[i], i+1);
		}
#if defined(VLAN_CONFIG_SUPPORTED)		
		memset(pMib->VlanConfigArray, '\0', MAX_IFACE_VLAN_CONFIG*sizeof(VLAN_CONFIG_T));
		for (i=0; i<pMib->VlanConfigNum; i++) {
			get_linkchain(&vlanConfigChain, (void *)&pMib->VlanConfigArray[i], i+1);
		}
#endif	
#endif /*not def MI B_TLV*/
		if (type & CURRENT_SETTING) {
			//diag_printf("[%s:%d]csHeader.len=0x%x\n",__FUNCTION__, __LINE__,csHeader.len);
#ifdef _LITTLE_ENDIAN_
            data=malloc(csHeader.len);
			if(data == NULL)
			{	 
				printf("malloc failed\n");
				return -1;
			}	  
			pdata=data;
			memcpy(data,pMib, csHeader.len);	 
			swap_mib_value((APMIB_Tp)data,CURRENT_SETTING);			
#else
			data = (unsigned char *)pMib;
#endif
			checksum = CHECKSUM(data, csHeader.len-1);
			//diag_printf("[%s:%d]csHeader.len=0x%x, checksum=0x%x\n",__FUNCTION__, __LINE__,csHeader.len,checksum);
			*(data + csHeader.len - 1) = checksum;
			i = CURRENT_SETTING_OFFSET + sizeof(csHeader);
			len = csHeader.len;
		}
		else {
#ifdef _LITTLE_ENDIAN_
            data=malloc(dsHeader.len);
			if(data == NULL)
			{	 
				printf("malloc failed\n");
				return -1;
			}
			pdata=data;
			memcpy(data,pMibDef, dsHeader.len);
			swap_mib_value((APMIB_Tp)data,DEFAULT_SETTING);		
#else
			data = (unsigned char *)pMibDef;
#endif
			checksum = CHECKSUM(data, dsHeader.len-1);
			*(data + dsHeader.len - 1) = checksum;
			i = DEFAULT_SETTING_OFFSET + sizeof(dsHeader);
			len = dsHeader.len;
		}

#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
		mib_tlv_max_len = mib_get_setting_len(type)*4;

		//diag_printf("[%s:%d]mib_tlv_max_len=0x%x\n" ,__FUNCTION__, __LINE__,mib_tlv_max_len);
		//dumpMallInfo();
		
        pfile = malloc(mib_tlv_max_len);
		if(!pfile){
			printf("Malloc %d failed!\n",mib_tlv_max_len);
			apmib_sem_unlock();
#ifdef _LITTLE_ENDIAN_
			free(pdata);
#endif
			return 0;
		}
		tlv_content_len = 0;

//mib_display_data_content(type, data, len);

		if(pfile != NULL && mib_tlv_save(type, (void*)data, pfile, &tlv_content_len) == 1)
		{
			//diag_printf("[%s:%d]tlv_content_len=%d\n",__FUNCTION__, __LINE__,tlv_content_len);
			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}else
			{
				apmib_sem_unlock();
				free(pfile);
#ifdef _LITTLE_ENDIAN_
				free(pdata);
#endif
				printf("Malloc %d failed!\n",tlv_content_len+1);
				return 0;
			}
			free(pfile);
			pfile = NULL;

		}
		
		if(mib_tlv_data != NULL)
		{
			if (type & CURRENT_SETTING)
				tlvcsHeader.len = tlv_content_len+1;
			else
				tlvdsHeader.len = tlv_content_len+1;
			//diag_printf("[%s:%d]tlvcsHeader.len=0x%x\n",__FUNCTION__, __LINE__,tlvcsHeader.len);
			data = mib_tlv_data;
			data[tlv_content_len] = CHECKSUM(data, tlv_content_len);
			//diag_printf("[%s:%d]checksum:0x%x\n", __FUNCTION__, __LINE__, data[tlv_content_len]);
			
//mib_display_tlv_content(type, data, tlv_content_len+1);

		}
#endif // #ifdef MIB_TLV
		if( mib_compress_write(type, data) == 1)
		{
		
		}	
		else
#endif //#ifedf COMPRESS_MIB_SETTING	
		if ( rtk_flash_write((char *)data, i, len)==0 ) 
		{
			printf("Write flash current-setting failed!\n");
			apmib_sem_unlock();
#ifdef MIB_TLV
			if(mib_tlv_data)
				free(mib_tlv_data);
			
			if(pfile)
				free(pfile);
#endif
#ifdef _LITTLE_ENDIAN_
			free(pdata);
#endif
			return 0;
		}
#ifdef MIB_TLV
			/*restore len to APMIB_T structure size*/
			if (type & CURRENT_SETTING)
				csHeader.len = len; 		
			else
				dsHeader.len = len;
#endif
#ifdef _LITTLE_ENDIAN_
		free(pdata);
#endif
		if(data){
			free(data);
			data = NULL;
		}
	}
	apmib_sem_unlock();
/*#ifdef CONFIG_APP_TR069
// send signal to cwmp process to let it know that mib changed
	cwmp_handle_notify();
#endif*/
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/* Update default setting MIB into current setting area
 */
int _apmib_updateDef(void)
{
	unsigned char *data, checksum;
	PARAM_HEADER_T header;
	int i;
#ifdef _LITTLE_ENDIAN_
    unsigned char *pdata;
#endif
#ifdef MIB_TLV
	TLV_PARAM_HEADER_T tlvheader;
	unsigned char *pfile = NULL;
	unsigned char *mib_tlv_data = NULL;
	unsigned int tlv_content_len = 0;
	unsigned int mib_tlv_max_len = 0;
#endif

	apmib_sem_lock();

	memcpy(header.signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN);
	memcpy(&header.signature[TAG_LEN], &dsHeader.signature[TAG_LEN], SIGNATURE_LEN-TAG_LEN);

	header.len = dsHeader.len;
#ifdef _LITTLE_ENDIAN_
    data=malloc(dsHeader.len);
    if(data == NULL)
    {    
        printf("malloc failed\n");
        return -1;
    }    
    pdata=data;
    memcpy(data,pMibDef,dsHeader.len);     
    swap_mib_value((APMIB_Tp)data,DEFAULT_SETTING);
#else
	data = (unsigned char *)pMibDef;
#endif
	checksum = CHECKSUM(data, header.len-1);
	*(data + header.len - 1) = checksum;

	i = CURRENT_SETTING_OFFSET;
#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
	mib_tlv_max_len = mib_get_setting_len(DEFAULT_SETTING)*4;

	pfile = malloc(mib_tlv_max_len);
	tlv_content_len = 0;

//mib_display_data_content(DEFAULT_SETTING, data, sizeof(APMIB_T));	
	if(pfile != NULL && mib_tlv_save(DEFAULT_SETTING, (void*)data, pfile, &tlv_content_len) == 1)
	{

		mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
		if(mib_tlv_data != NULL)
		{
			memcpy(mib_tlv_data, pfile, tlv_content_len);
		}
				
		free(pfile);
	}
	
	if(mib_tlv_data != NULL)
	{
		memcpy(&tlvheader,&header, sizeof(header));
		tlvheader.len = tlv_content_len+1;
		
		data = mib_tlv_data;
		data[tlv_content_len] = CHECKSUM(data, tlv_content_len);

//mib_display_tlv_content(CURRENT_SETTING, data, tlv_content_len+1);			

	}

#endif // #ifdef MIB_TLV

	if(mib_updateDef_compress_write(CURRENT_SETTING, (char *)data, &tlvheader) == 1)
	{
		COMP_TRACE(stderr,"\r\n mib_updateDef_compress_write CURRENT_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
	}
	else
	{
#endif

	if ( rtk_flash_write((char *)&header, i, sizeof(header))==0 ) {
		printf("Write flash current-setting header failed!\n");
		apmib_sem_unlock();
		return 0;
	}
	i += sizeof(header);

	if ( rtk_flash_write((char *)data, i, header.len)==0 ) {
		printf("Write flash current-setting failed!\n");
		apmib_sem_unlock();
		return 0;
	}
#ifdef COMPRESS_MIB_SETTING
	}
#ifdef MIB_TLV	
	if(mib_tlv_data) {
		free(mib_tlv_data);
		mib_tlv_data=NULL;
	}
#endif
#endif
#ifdef _LITTLE_ENDIAN_
	if(pdata){
		free(pdata);
		pdata=NULL;
	}
#endif
	apmib_sem_unlock();
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/* Update MIB into flash current setting area
 */
#ifdef COMPRESS_MIB_SETTING
int _apmib_updateFlash(CONFIG_DATA_T type, char *data, int len, int force, int ver)
{
	int offset;
	/*since COMPRESS MIB no way to keep old or upgrade mib. so only replaced*/
	
	apmib_sem_lock();
	if ( type == HW_SETTING ) {
		offset = HW_SETTING_OFFSET;
	}
	else if ( type == DEFAULT_SETTING ) {
		offset = DEFAULT_SETTING_OFFSET;
	}
	else  {
		offset = CURRENT_SETTING_OFFSET;
	}
	/*no care force and ver.*/
	if(0==rtk_flash_write(data,offset,len)){
		printf("flash write apmib failed");	
		apmib_sem_unlock();
		return 0;
	}	
	apmib_sem_unlock();
	return 1;
}

#else
int _apmib_updateFlash(CONFIG_DATA_T type, char *data, int len, int force, int ver)
{
	unsigned char checksum, checksum1, *ptr=NULL;
	int i, offset=0, curLen, curVer;
	unsigned char *pMibData, *pHdr, tmpBuf[20];

	apmib_sem_lock();

	if ( type == HW_SETTING ) {
		curLen = hsHeader.len - 1;
		pMibData = (unsigned char *)pHwSetting;
		pHdr = (unsigned char *)&hsHeader;
		i = HW_SETTING_OFFSET;
	}
	else if ( type == DEFAULT_SETTING ) {
		curLen = dsHeader.len - 1;
		pMibData = (unsigned char *)pMibDef;
		pHdr = (unsigned char *)&dsHeader;
		i = DEFAULT_SETTING_OFFSET;
	}
	else  {
		curLen = csHeader.len - 1;
		pMibData = (unsigned char *)pMib;
		pHdr = (unsigned char *)&csHeader;
		i = CURRENT_SETTING_OFFSET;
	}

	if (force==2) { // replace by input mib
		((PARAM_HEADER_Tp)pHdr)->len = len + 1;
		sprintf(tmpBuf, "%02d", ver);
		memcpy(&pHdr[TAG_LEN], tmpBuf, SIGNATURE_LEN-TAG_LEN);
		checksum = CHECKSUM(data, len);
		pMibData = data;
		curLen = len;
	}
	else if (force==1) { // update mib but keep not used mib
		sscanf(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN], "%02d", &curVer);
		if ( curVer < ver ) {
			sprintf(tmpBuf, "%02d", ver);
			memcpy(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN],
					tmpBuf, SIGNATURE_LEN-TAG_LEN);
		}
		checksum = CHECKSUM(data, len);
		if (curLen > len) {
			((PARAM_HEADER_Tp)pHdr)->len = curLen + 1;
			ptr = pMibData + len;
			offset = curLen - len;
			checksum1 = CHECKSUM(ptr, offset);
			checksum +=  checksum1;
		}
		else
			((PARAM_HEADER_Tp)pHdr)->len = len + 1;

		curLen = len;
		pMibData = data;
	}
	else { // keep old mib, only update new added portion
		sscanf(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN], "%02d", &curVer);
		if ( curVer < ver ) {
			sprintf(tmpBuf, "%02d", ver);
			memcpy(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN],
					tmpBuf, SIGNATURE_LEN-TAG_LEN);
		}
		if ( len > curLen ) {
			((PARAM_HEADER_Tp)pHdr)->len = len + 1;
			offset = len - curLen;
			checksum = CHECKSUM(pMibData, curLen);
			ptr = data + curLen;
			checksum1 = CHECKSUM(ptr, offset);
			checksum +=  checksum1;
		}
		else
			checksum = CHECKSUM(pMibData, curLen);
	}

	if ( rtk_flash_write((char *)pHdr, i, sizeof(PARAM_HEADER_T))==0 ) {
		printf("Write flash current-setting header failed!\n");
		apmib_sem_unlock();
		return 0;
	}
	i += sizeof(PARAM_HEADER_T);

	if ( rtk_flash_write(pMibData, i, curLen)==0 ) {
		printf("Write flash current-setting failed!\n");
		apmib_sem_unlock();
		return 0;
	}
	i += curLen;

	if (offset > 0) {
		if ( rtk_flash_write((char *)ptr, i, offset)==0 ) {
			printf("Write flash current-setting failed!\n");
			apmib_sem_unlock();
			return 0;
		}
		i += offset;
	}

	if ( rtk_flash_write((char *)&checksum, i, sizeof(checksum))==0 ) {
		printf("Write flash current-setting checksum failed!\n");
		apmib_sem_unlock();
		return 0;
	}

	apmib_sem_unlock();
	return 1;
}
#endif
/////////////////////////////////////////////////////////////////////////////////

#ifdef MIB_TLV
int add_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val)
{
	//printf("num %d count %d \n",num,(mib_tbl->total_size/mib_tbl->unit_size));
	if( num >= (mib_tbl->total_size/mib_tbl->unit_size))
		return 0;
	memcpy((char*)pmib+offset+num*mib_tbl->unit_size,(char*)val,mib_tbl->unit_size);
	return 1;
}

int delete_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val)
{
	int i = 0;
	int total_size = mib_tbl->total_size;
	int unit_size = mib_tbl->unit_size;
	if(num > total_size/unit_size)
		num=total_size/unit_size;
	for(i=0;i<num;i++)
	{
		if(0==memcmp((char*)pmib+offset+i*unit_size,(char*)val,unit_size))
			break;
	}
	/*not found*/
	if(i == num)
	{
		//fprintf(stderr,"not find del entry!!");
		return 0;
	}
	
	if(i==(num-1))
	{
		memset((char*)pmib+offset+i*unit_size,0x0,unit_size);	
	}
	else
	{
		for(;i<num;i++)
		{
			memcpy((char*)pmib+offset+i*unit_size,(char*)pmib+offset+(i+1)*unit_size,unit_size);
		}
	}
	return 1;
}
#if defined(MIB_MOD_TBL_ENTRY) //brucehou
int mod_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val)
{
	int i = 0;
	int total_size = mib_tbl->total_size;
	int unit_size = mib_tbl->unit_size;
	if(num > total_size/unit_size)
		num=total_size/unit_size;
	for(i=0;i<num;i++)
	{
		if(0==memcmp(pmib+offset+i*unit_size,val,unit_size))
			break;
	}

	/*not found*/
	if(i == num)
		return 0;

	memcpy(pmib+offset+i*unit_size,val+unit_size,unit_size);
	return 1;
}
#endif /* #if defined(MIB_MOD_TBL_ENTRY) */


int delete_all_tblentry(void *pmib, unsigned int offset, int num,const mib_table_entry_T *mib_tbl)
{
	memset((char*)pmib+offset,0x0,mib_tbl->total_size);
	return 1;
}
int update_tblentry(void *pmib,unsigned int offset,int num,const mib_table_entry_T *mib_tbl,void *old, void *newone)
{
	int i=0;
	int total_size = mib_tbl->total_size;
	int unit_size = mib_tbl->unit_size;
	if(num > total_size/unit_size)
		return 0;
	for(i=0;i<num;i++)
	{
		if(0==memcmp((char*)pmib+offset+i*unit_size,(char*)old,unit_size))
		{
			break;
		}
	}
	if(i == num)
		return 0;

	/*found*/
	memcpy((char*)pmib+offset+i*unit_size,(char*)newone,unit_size);
	return 1;

}

int get_tblentry(void *pmib,unsigned int offset,int num,const mib_table_entry_T *mib_tbl,void *val, int index)

{
	if(index > num)
		return 0;
	memcpy((char*)val,(char*)pmib+offset+(index-1)*mib_tbl->unit_size,mib_tbl->unit_size);
	return 1;
}


#else
///////////////////////////////////////////////////////////////////////////////
static int init_linkchain(LINKCHAIN_Tp pLinkChain, int size, int num)
{
	FILTER_Tp entry;
	int offset=sizeof(FILTER_Tp)*2;
	char *pBuf;
	int i;

	pLinkChain->realsize = size;

	if (size%4)
		size = (size/4+1)*4;

	pBuf = calloc(num, size+offset);
	if ( pBuf == NULL )
		return 0;

	pLinkChain->buf = pBuf;
	pLinkChain->pUsedList = NULL;
	pLinkChain->pFreeList = NULL;
	entry = (FILTER_Tp)pBuf;

	ADD_LINK_LIST(pLinkChain->pFreeList, entry);
	for (i=1; i<num; i++) {
		entry = (FILTER_Tp)&pBuf[i*(size+offset)];
		ADD_LINK_LIST(pLinkChain->pFreeList, entry);
	}

	pLinkChain->size = size;
	pLinkChain->num = num;
	pLinkChain->usedNum = 0;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
static int add_linkchain(LINKCHAIN_Tp pLinkChain, char *val)
{
	FILTER_Tp entry;

	// get a free entry
	entry = pLinkChain->pFreeList;
	if (entry == NULL)
		return 0;

	if (entry->next==pLinkChain->pFreeList)
		pLinkChain->pFreeList = NULL;
	else
		pLinkChain->pFreeList = entry->next;

	REMOVE_LINK_LIST(entry);

	// copy content
	memcpy(entry->val, val, pLinkChain->realsize);

	// add to used list
	if (pLinkChain->pUsedList == NULL) {
		ADD_LINK_LIST(pLinkChain->pUsedList, entry);
	}
	else {
		ADD_LINK_LIST(pLinkChain->pUsedList->prev, entry);
	}
	pLinkChain->usedNum++;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
static int delete_linkchain(LINKCHAIN_Tp pLinkChain, char *val)
{
	FILTER_Tp curEntry=pLinkChain->pUsedList;

	while (curEntry != NULL) {
		if ( !memcmp(curEntry->val,(unsigned char *)val,pLinkChain->compareLen) ) {
				if (curEntry == pLinkChain->pUsedList) {
					if ( pLinkChain->pUsedList->next != pLinkChain->pUsedList )
						pLinkChain->pUsedList = pLinkChain->pUsedList->next;
					else
						pLinkChain->pUsedList = NULL;
				}
				REMOVE_LINK_LIST(curEntry);
				ADD_LINK_LIST(pLinkChain->pFreeList, curEntry);
				pLinkChain->usedNum--;
				return 1;
		}
		if ( curEntry->next == pLinkChain->pUsedList )
		{
			return 0;
		}
		curEntry = curEntry->next;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
static void delete_all_linkchain(LINKCHAIN_Tp pLinkChain)
{
	FILTER_Tp curEntry;

	if (pLinkChain->pUsedList==NULL)
		return;

	// search for matched mac address
	while (pLinkChain->pUsedList) {
		curEntry = pLinkChain->pUsedList;
		if (pLinkChain->pUsedList->next != pLinkChain->pUsedList)
			pLinkChain->pUsedList = pLinkChain->pUsedList->next;
		else
			pLinkChain->pUsedList = NULL;

		REMOVE_LINK_LIST(curEntry);
		ADD_LINK_LIST(pLinkChain->pFreeList, curEntry);
		pLinkChain->usedNum--;
	}
}

///////////////////////////////////////////////////////////////////////////////
static int get_linkchain(LINKCHAIN_Tp pLinkChain, char *val, int index)
{
	FILTER_Tp curEntry=pLinkChain->pUsedList;

	if ( curEntry == NULL || index > pLinkChain->usedNum)
 		return 0;

	while (--index > 0)
        	curEntry = curEntry->next;
	
	memcpy( (unsigned char *)val, curEntry->val, pLinkChain->realsize);

	return 1;
}

int _update_linkchain(int fmt, void *Entry_old, void *Entry_new, int type_size)
{
	LINKCHAIN_Tp pLinkChain=NULL;
	FILTER_Tp curEntry;
	void *entry;
	int i; 
	int entry_cmp;
#ifdef HOME_GATEWAY
	 	if(fmt==PORTFW_ARRAY_T){
			pLinkChain = &portFwChain;	 
		}else if(fmt == IPFILTER_ARRAY_T){
			pLinkChain = &ipFilterChain;
		}else if(fmt == PORTFILTER_ARRAY_T){
			pLinkChain = &portFilterChain;
		}else if(fmt == MACFILTER_ARRAY_T){
			pLinkChain = &macFilterChain;
		}else if(fmt == URLFILTER_ARRAY_T){
			pLinkChain = &urlFilterChain;
		}else	if(fmt==TRIGGERPORT_ARRAY_T){
				pLinkChain = &triggerPortChain;
		}else if(fmt==DHCPRSVDIP_ARRY_T){
			pLinkChain = &dhcpRsvdIpChain;			
		}
#ifdef ROUTE_SUPPORT				
		if(fmt==STATICROUTE_ARRAY_T){
		 	pLinkChain = &staticRouteChain;
		}
#endif			
#else
		 if(fmt==DHCPRSVDIP_ARRY_T){
			pLinkChain = &dhcpRsvdIpChain;	
		}		
#endif			

#if defined(VLAN_CONFIG_SUPPORTED)
		 if(fmt==VLANCONFIG_ARRAY_T){
		 	pLinkChain = &vlanConfigChain;	
		}
#endif	
	curEntry = pLinkChain->pUsedList;
	for(i=0;i<pLinkChain->usedNum;i++){
		entry = curEntry->val;
		entry_cmp=memcmp(entry, Entry_old, type_size );
		if(entry_cmp ==0){
		//	fprintf(stderr,"find the entry to update!\n");
			memcpy(entry, Entry_new, type_size);
			break;
		}
		curEntry = curEntry->next;
	}
	return 1;
}
#endif

#ifdef COMPRESS_MIB_SETTING

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length if match_length is greater than this */
static unsigned char *text_buf;	/* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
#define LZSS_TYPE	unsigned short
#define NIL			N	/* index for root of binary search trees */
struct lzss_buffer {
	unsigned char	text_buf[N + F - 1];
	LZSS_TYPE	lson[N + 1];
	LZSS_TYPE	rson[N + 257];
	LZSS_TYPE	dad[N + 1];
};
static LZSS_TYPE		match_position, match_length;  /* of longest match.  These are set by the InsertNode() procedure. */
static LZSS_TYPE		*lson, *rson, *dad;  /* left & right children & parents -- These constitute binary search trees. */

void InsertNode(LZSS_TYPE r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	LZSS_TYPE  i, p;
	int cmp;
	unsigned char  *key;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;
	match_length = 0;
	while(1) {
		if (cmp >= 0) {
			if (rson[p] != NIL)
				p = rson[p];
			else {
				rson[p] = r;
				dad[r] = p;
				return;
			}
		} else {
			if (lson[p] != NIL)
				p = lson[p];
			else {
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)
				break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)
				break;
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++)
		rson[i] = NIL;
	for (i = 0; i < N; i++)
		dad[i] = NIL;
}

void DeleteNode(LZSS_TYPE p)  /* deletes node p from tree */
{
	LZSS_TYPE  q;
	
	if (dad[p] == NIL)
		return;  /* not in tree */
	if (rson[p] == NIL)
		q = lson[p];
	else if (lson[p] == NIL)
		q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {
				q = rson[q];
			} while (rson[q] != NIL);
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;
	dad[p] = NIL;
}
int Encode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)
{
	LZSS_TYPE  i, len, r, s, last_match_length, code_buf_ptr;
	unsigned char c;
	unsigned char  code_buf[17], mask;
	unsigned int ulPos=0;
	int enIdx=0;

	struct lzss_buffer *lzssbuf;

	if (0 != (lzssbuf = malloc(sizeof(struct lzss_buffer)))) {
		memset(lzssbuf, 0, sizeof(struct lzss_buffer));
		text_buf = lzssbuf->text_buf;
		rson = lzssbuf->rson;
		lson = lzssbuf->lson;
		dad = lzssbuf->dad;
	} else {
		return 0;
	}

	InitTree();  /* initialize trees */
	code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
		code_buf[0] works as eight flags, "1" representing that the unit
		is an unencoded letter (1 byte), "0" a position-and-length pair
		(2 bytes).  Thus, eight units require at most 16 bytes of code. */
	code_buf_ptr = mask = 1;
	s = 0;
	r = N - F;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';  /* Clear the buffer with
		any character that will appear often. */

	for (len = 0; (len < F) && ulPos < inLen; len++)
		text_buf[r + len] = ucInput[ulPos++];  /* Read F bytes into the last F bytes of the buffer */
	
	//if ((textsize = len) == 0) return;  /* text of size zero */
	if (len == 0) {
		enIdx = 0;
		goto finished;
	}
	
	for (i = 1; i <= F; i++)
		InsertNode(r - i);  /* Insert the F strings,
		each of which begins with one or more 'space' characters.  Note
		the order in which these strings are inserted.  This way,
		degenerate trees will be less likely to occur. */
	InsertNode(r);  /* Finally, insert the whole string just read.  The
		global variables match_length and match_position are set. */
	do {
		if (match_length > len) match_length = len;  /* match_length
			may be spuriously long near the end of text. */
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one byte. */
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		} else {
			code_buf[code_buf_ptr++] = (unsigned char) match_position;
			code_buf[code_buf_ptr++] = (unsigned char)
				(((match_position >> 4) & 0xf0)
			  | (match_length - (THRESHOLD + 1)));  /* Send position and
					length pair. Note match_length > THRESHOLD. */
		}
		if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
			for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
				ucOutput[enIdx++]=code_buf[i];
			//codesize += code_buf_ptr;
			code_buf[0] = 0;  code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;

		for (i = 0; i< last_match_length && 
			ulPos < inLen; i++){
			c = ucInput[ulPos++];
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1)
				text_buf[s + N] = c;  /* If the position is near the end of buffer, extend the buffer to make string comparison easier. */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
			InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
		}
		
		while (i++ < last_match_length) {	/* After the end of text, */
			DeleteNode(s);					/* no need to read, but */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		/* buffer may not be empty. */
		}
	} while (len > 0);	/* until length of string to be processed is zero */
	if (code_buf_ptr > 1) {		/* Send remaining code. */
		for (i = 0; i < code_buf_ptr; i++) 
			ucOutput[enIdx++]=code_buf[i];
		//codesize += code_buf_ptr;
	}
finished:
	free(lzssbuf);
	return enIdx;
}

int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)	/* Just the reverse of Encode(). */
{
	int  i, j, k, r, c;
	unsigned int  flags;
	unsigned int ulPos=0;
	unsigned int ulExpLen=0;

	if ((text_buf = malloc( N + F - 1 )) == 0) {
		//fprintf(stderr, "fail to get mem %s:%d\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';
	r = N - F;
	flags = 0;
	while(1) {
		if (((flags >>= 1) & 256) == 0) {
			c = ucInput[ulPos++];
			if (ulPos>inLen)
				break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			c = ucInput[ulPos++];
			if ( ulPos > inLen )
				break;
			ucOutput[ulExpLen++] = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			i = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			j = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				ucOutput[ulExpLen++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}

	free(text_buf);
	return ulExpLen;
}

unsigned int mib_get_flash_offset(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return HW_SETTING_OFFSET;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_OFFSET;			
		case CURRENT_SETTING:
			return CURRENT_SETTING_OFFSET;
		default:
			return 0;
	}
	
}

unsigned int mib_get_real_len(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return HW_SETTING_SECTOR_LEN;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_SECTOR_LEN;			
		case CURRENT_SETTING:
			return CURRENT_SETTING_SECTOR_LEN;
		default:			
			return 0;
	}
	
}

unsigned int mib_get_setting_len(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return sizeof(HW_SETTING_T);
		case DEFAULT_SETTING:
		case CURRENT_SETTING:
			return sizeof(APMIB_T);
		default:			
			return 0;
	}
	
}
TLV_PARAM_HEADER_T* mib_get_header(CONFIG_DATA_T type)
{
	
	switch(type)
	{
		case HW_SETTING:
			return &tlvhsHeader;
		case DEFAULT_SETTING:
			return &tlvdsHeader;
		case CURRENT_SETTING:
			return &tlvcsHeader;
		default :
			return NULL;
		
	}

}
unsigned int mib_compress_write(CONFIG_DATA_T type, unsigned char *data)
{
	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = 0;
	unsigned int compLen;
	unsigned int real_size = 0;
	TLV_PARAM_HEADER_T *pheader;
	int dst;

	//diag_printf("[%s:%d]\n" ,__FUNCTION__, __LINE__);
	//dumpMallInfo();

	dst = mib_get_flash_offset(type);
	//real_size = mib_get_flash_offset(type);
	real_size = CURRENT_SETTING_SECTOR_LEN;
	pheader = mib_get_header(type);
	expLen = pheader->len+sizeof(TLV_PARAM_HEADER_T);
	//diag_printf("[%s:%d]real_size=0x%x, expLen=0x%x\n" ,__FUNCTION__, __LINE__,real_size,expLen);

	if( (compPtr = malloc(real_size)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",real_size,__FILE__,__LINE__);
		goto mib_compress_write_FAIL;
	}
	if( (expPtr = malloc(expLen)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",expLen,__FILE__,__LINE__);
		goto mib_compress_write_FAIL;
	}
	//diag_printf("[%s:%d]compPtr=%p, expPtr=%p\n", __FUNCTION__, __LINE__,compPtr,expPtr);
	if(compPtr != NULL && expPtr!= NULL)
	{
		pContent = &expPtr[sizeof(TLV_PARAM_HEADER_T)];	// point to start of MIB data 
	
		//diag_printf("[%s:%d]pheader->len=0x%x\n", __FUNCTION__, __LINE__,pheader->len);
		switch(type)
		{
			case HW_SETTING:
				memcpy(pContent, data, WORD_SWAP(pheader->len));
				pheader->len=WORD_SWAP(pheader->len);
				memcpy(expPtr, pheader, sizeof(TLV_PARAM_HEADER_T));
				pheader->len=WORD_SWAP(pheader->len);
				//diag_printf("[%s:%d]pheader->len=0x%x\n", __FUNCTION__, __LINE__,pheader->len);
				break;
			case DEFAULT_SETTING:
			case CURRENT_SETTING:
				memcpy(pContent, data, (pheader->len));
				pheader->len=DWORD_SWAP(pheader->len);
				memcpy(expPtr, pheader, sizeof(TLV_PARAM_HEADER_T));
				pheader->len=DWORD_SWAP(pheader->len);
				//diag_printf("[%s:%d]pheader->len=0x%x\n", __FUNCTION__, __LINE__,pheader->len);
				break;
				break;
			default :
				goto mib_compress_write_FAIL;
		
		}
		compLen = Encode(expPtr, expLen, compPtr+sizeof(COMPRESS_MIB_HEADER_T));

		if(type == HW_SETTING)
			sprintf((char *)compHeader.signature,"%s",COMP_HS_SIGNATURE);
		else if(type == DEFAULT_SETTING)
			sprintf((char *)compHeader.signature,"%s",COMP_DS_SIGNATURE);
		else
			sprintf((char *)compHeader.signature,"%s",COMP_CS_SIGNATURE);

		//diag_printf("[%s:%d] rate=%d, compLen=%d\n", (expLen/compLen)+1, compLen);
		//sleep(3);
		compHeader.compRate = WORD_SWAP((expLen/compLen)+1);
		compHeader.compLen = DWORD_SWAP(compLen);
		memcpy(compPtr, &compHeader, sizeof(COMPRESS_MIB_HEADER_T));

		if ( rtk_flash_write((char *)compPtr, dst, compLen+sizeof(COMPRESS_MIB_HEADER_T))==0 ) {
			COMP_TRACE(stderr,"Write flash compress setting[%u] failed![%s-%u]\n",type,__FILE__,__LINE__);
			goto mib_compress_write_FAIL;
		}
		else
		{
			COMP_TRACE(stderr,"\r\n Compress [%u] to [%u]. Compress rate=%u, __[%s-%u]",expLen,compLen,compHeader.compRate ,__FILE__,__LINE__);

			COMP_TRACE(stderr,"\r\n Flash write to 0x%x len=%d\n",dst,compLen+sizeof(COMPRESS_MIB_HEADER_T));

			COMP_TRACE(stderr,"\r\n");
			if(expPtr)
				free(expPtr);
			if(compPtr)
				free(compPtr);
			return 1;
		}
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);			
	}
mib_compress_write_FAIL:
	if(expPtr)
		free(expPtr);
	if(compPtr)
		free(compPtr);
	return 0;
}

int mib_updateDef_compress_write(CONFIG_DATA_T type, char *data, TLV_PARAM_HEADER_T *pheader)
{
	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader={0};
	unsigned char *expPtr=NULL, *compPtr=NULL;
	unsigned int expLen = 0;
	unsigned int compLen=0;
	unsigned int real_size = 0;
	//int zipRate=0;
	char *pcomp_sig=NULL;
	int dst = mib_get_flash_offset(type);

	if(dst == 0)
	{
		printf("\r\n ERR!! no flash offset! __[%s-%u]\n",__FILE__,__LINE__);
		goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;
	}
	
	switch(type)
	{
		case HW_SETTING:
			pcomp_sig = COMP_HS_SIGNATURE;
			break;
		case DEFAULT_SETTING:
			pcomp_sig = COMP_DS_SIGNATURE;
			break;
		case CURRENT_SETTING:
			pcomp_sig = COMP_CS_SIGNATURE;
			break;
		default:
			printf("\r\n ERR!! no type match __[%s-%u]\n",__FILE__,__LINE__);
			goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;

	}
	expLen = pheader->len+sizeof(TLV_PARAM_HEADER_T);
	if(expLen == 0)
	{
		printf("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;
	}
	real_size = mib_get_real_len(type);
	if(real_size == 0)
	{
		printf("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;
	}
	
	if( (compPtr = malloc(real_size)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",real_size,__FILE__,__LINE__);
		goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;
	}

	if( (expPtr = malloc(expLen)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",expLen,__FILE__,__LINE__);
		goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;
	}
	
	
	//int status;
	pContent = &expPtr[sizeof(TLV_PARAM_HEADER_T)];	// point to start of MIB data 

	memcpy(pContent, data, pheader->len);
	pheader->len=DWORD_SWAP(pheader->len);
	memcpy(expPtr, pheader, sizeof(TLV_PARAM_HEADER_T));
	pheader->len=DWORD_SWAP(pheader->len);

	compLen = Encode(expPtr, expLen, compPtr+sizeof(COMPRESS_MIB_HEADER_T));
	sprintf((char *)compHeader.signature,"%s",pcomp_sig);
	compHeader.compRate = WORD_SWAP((expLen/compLen)+1);
	compHeader.compLen = DWORD_SWAP(compLen);
	memcpy(compPtr, &compHeader, sizeof(COMPRESS_MIB_HEADER_T));
	if ( rtk_flash_write((char *)compPtr, dst, compLen+sizeof(COMPRESS_MIB_HEADER_T))==0 )
//		if ( write(fh, (const void *)compPtr, compLen+sizeof(COMPRESS_MIB_HEADER_T))!=compLen+sizeof(COMPRESS_MIB_HEADER_T) ) 
	{
		printf("Write flash compress [%u] setting failed![%s-%u]\n",type,__FILE__,__LINE__);			
		goto MIB_UPDATEDEF_COMPRESS_WRITE_FAIL;
	}
	
	if(compPtr != NULL)
		free(compPtr);
	if(expPtr != NULL)
		free(expPtr);

	return 1;
			
	
MIB_UPDATEDEF_COMPRESS_WRITE_FAIL:
	if(compPtr != NULL)
		free(compPtr);
	if(expPtr != NULL)
		free(expPtr);
	return 0;
}
#endif //#ifdef COMPRESS_MIB_SETTING

int save_cs_to_file(void)
{
	char *buf, *ptr=NULL;
	PARAM_HEADER_Tp pHeader;
	//unsigned char checksum;
	int len, fh;
	char tmpBuf[100];

#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
	TLV_PARAM_HEADER_Tp ptlvheader;
	int tlv_content_len,compLen;
	unsigned char *pCompptr;
	COMPRESS_MIB_HEADER_Tp pcompHeader;
	char *pbuf;
#endif
#endif

#ifdef MIB_TLV
	len=mib_get_setting_len(CURRENT_SETTING)*4;
#else
	len = csHeader.len;
#endif

#ifdef MIB_TLV
	len += sizeof(TLV_PARAM_HEADER_T);
#else
	len += sizeof(PARAM_HEADER_T);
#endif
	buf = malloc(len);
	if ( buf == NULL ) {
		strcpy(tmpBuf, "Allocate buffer failed!");
		fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
		return 0;
	}
	bzero(buf,len);
//	fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
#ifdef __mips__
#ifndef JFFS2_SUPPORT	
	unlink("/web/config.dat");
	fh = fopen("/web/config.dat", "w");
#else
	fh = fopen("/var/config.dat", "w");
#endif
#else
	fh = fopen("../web/config.dat", O_RDWR|O_CREAT|O_TRUNC);
#endif
	if (fh == -1) {
		printf("Create config file error!\n");
		free(buf);
		fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
		return 0;
	}


#ifdef MIB_TLV
	ptlvheader=(TLV_PARAM_HEADER_Tp)buf;
	pbuf=malloc(csHeader.len);
    if(pbuf) {
        memcpy(pbuf, pMib, csHeader.len-1);
    } else {
        printf("%s %d malloc failed\n",__FUNCTION__,__LINE__);
        return 0;
    }
#else	
	pHeader = (PARAM_HEADER_Tp)buf;	
	len = pHeader->len = csHeader.len;
	memcpy(&buf[sizeof(PARAM_HEADER_T)], pMib, len-1);
#endif

#ifdef _LITTLE_ENDIAN_
#ifdef VOIP_SUPPORT
	// rock: need swap here 
	// 1. write to share space (ex: save setting to config file)
	// 2. read from share space (ex: import config file) 
	pHeader->len  = DWORD_SWAP(pHeader->len);
#else
	//pHeader->len  = WORD_SWAP(pHeader->len);
#endif
#ifdef MIB_TLV
	swap_mib_word_value(pbuf);
#else
	swap_mib_word_value((APMIB_Tp)&buf[sizeof(PARAM_HEADER_T)]);
#endif
#endif

#ifdef MIB_TLV
	memcpy(ptlvheader->signature, tlvcsHeader.signature, SIGNATURE_LEN);
	ptr = (char *)&buf[sizeof(TLV_PARAM_HEADER_T)];
#else	
	memcpy(pHeader->signature, csHeader.signature, SIGNATURE_LEN);
	ptr = (char *)&buf[sizeof(PARAM_HEADER_T)];
#endif

	
#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
	//fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);

	tlv_content_len=0;
	if(mib_tlv_save(CURRENT_SETTING, (void*)pbuf, (unsigned char *)ptr, (unsigned int *)&tlv_content_len) == 1){
		if(tlv_content_len >= (mib_get_setting_len(CURRENT_SETTING)*4)){
			printf("TLV Data len is too long");
			close(fh);
			free(buf);
			free(pbuf);
			fprintf(stderr,"%s %d tlv_content_len 0x%x len 0x%x\n",__FUNCTION__,__LINE__,tlv_content_len,len);
			return 0;
		}
		ptr[tlv_content_len] = CHECKSUM((unsigned char *)ptr, tlv_content_len);	
		ptlvheader->len=tlv_content_len+1; /*add checksum*/
		ptlvheader->len=DWORD_SWAP(ptlvheader->len);
	}

	/*compress*/
	pCompptr = malloc((WEB_PAGE_OFFSET-CURRENT_SETTING_OFFSET)+sizeof(COMPRESS_MIB_HEADER_T));
	if(NULL == pCompptr){
			printf("malloc for Compress buffer failed!! \n");
			close(fh);
			free(buf);
			free(pbuf);
			fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
			return 0;
	}
	compLen = Encode((unsigned char *)buf, DWORD_SWAP(ptlvheader->len)+sizeof(TLV_PARAM_HEADER_T), pCompptr+sizeof(COMPRESS_MIB_HEADER_T));
	pcompHeader=(COMPRESS_MIB_HEADER_Tp)pCompptr;
	memcpy(pcompHeader->signature,COMP_CS_SIGNATURE,COMP_SIGNATURE_LEN);
	unsigned int length_before_compress = DWORD_SWAP(ptlvheader->len)+sizeof(TLV_PARAM_HEADER_T);
	pcompHeader->compRate = WORD_SWAP((length_before_compress/compLen)+1);
	//pcompHeader->compRate = WORD_SWAP((ptlvheader->len/compLen)+1);
	pcompHeader->compLen = DWORD_SWAP(compLen);
#endif	
#endif

#ifdef MIB_TLV
	//fprintf(stderr,"%s %d compLen %d\n",__FUNCTION__,__LINE__,compLen);

	if ( fwrite(pCompptr,1, compLen+sizeof(COMPRESS_MIB_HEADER_T),fh) != compLen+sizeof(COMPRESS_MIB_HEADER_T)) 
	{
		printf("Write config file error!\n");
		close(fh);
		free(pCompptr);
		free(buf);
		free(pbuf);
		fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
		return 0;
	}
	//fprintf(stderr,"%s %d compLen %d\n",__FUNCTION__,__LINE__,compLen);
#else
	checksum = CHECKSUM(ptr, len-1);
	buf[sizeof(PARAM_HEADER_T)+len-1] = checksum;

	ptr = &buf[sizeof(PARAM_HEADER_T)];
	ENCODE_DATA(ptr, len);


	if ( fwrite(buf,1, len+sizeof(PARAM_HEADER_T),fh) != len+sizeof(PARAM_HEADER_T)) {
		printf("Write config file error!\n");
		close(fh);
		free(buf);
		return 0;
	}
#endif

	
	//fprintf(stderr,"%s %d compLen %d\n",__FUNCTION__,__LINE__,compLen);
fclose(fh);
//sync();

#ifdef MIB_TLV	
	if(pCompptr) {
		free(pCompptr);
		pCompptr=NULL;
	}
	free(pbuf);
#endif	
	free(buf);

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	web_voip_saveConfig();
#endif

	return 1;
}
//#endif // WEBS


#ifdef MIB_TLV

int mib_search_by_id(mib_table_entry_T *mib_tbl, unsigned short mib_id, unsigned char *pmib_num, mib_table_entry_T **ppmib, unsigned int *offset)
{
	int i=0;
	mib_table_entry_T *mib=NULL;
	unsigned short mib_num=0;
	
	memcpy(&mib_num, pmib_num, 1);
	
//printf("\r\n search mib_id=%u, offset=%u, mib_num=%u",mib_id,*offset, mib_num);
	
	
	for (i=0; mib_tbl[i].id; i++)
	{
//printf("\r\n mib_tbl[%u].mib_name=%s",i, mib_tbl[i].mib_name);
		mib = &mib_tbl[i];
		
		if(mib_id == mib_tbl[i].id)
		{
			*offset += mib->offset + mib->unit_size*mib_num;
			*ppmib = mib;
//printf("\r\n !! FIND at %s TBL !!",mib_tbl[i].mib_name);
			return 1;
		}
		else
		{
			if(mib_tbl[i].type >= TABLE_LIST_T)
			{
				
				if((mib->total_size%mib->unit_size) == 0 && mib_num < (mib->total_size /mib->unit_size))
				{					
					*offset += mib->offset + mib->unit_size*mib_num;
//printf("\r\n >> Entry %s TBL >>",mib->mib_name);
					if(mib_search_by_id(mib->next_mib_table, mib_id, pmib_num+1, ppmib, offset) == 1)
					{
						return 1;
					}
					else
					{
//printf("\r\n << Leave %s TBL <<",mib->mib_name);
						*offset -= mib->offset + mib->unit_size*mib_num;
					}
				}
				
			}
		}
	}
	return 0;
}

static int mib_init_value(unsigned char *ptlv_data_value, unsigned short tlv_len, const mib_table_entry_T *mib_tbl, void *data)
{
	unsigned int vInt;
	unsigned short vShort;
	unsigned char *pChar;
	
	
	
#if 0
int j=0;
fprintf(stderr,"\r\n mib_tbl->type = %u",mib_tbl->type);
fprintf(stderr,"\r\n %s = ",mib_tbl->name);
for(j=0; j<tlv_len; j++)
	fprintf(stderr,"0x%x_", *(ptlv_data_value+j));
fprintf(stderr,"\r\n");
#endif
			
	switch (mib_tbl->type)
	{
		case BYTE_T:
		case BYTE_ARRAY_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, tlv_len);
						
			break;
			
		case WORD_T:
			pChar = (unsigned char *) data;
			memcpy(&vShort, ptlv_data_value, sizeof(vShort));
			vShort = WORD_SWAP(vShort);
			memcpy(data, &vShort, sizeof(vShort));
			break;
			
		case DWORD_T:
			pChar = (unsigned char *) data;
			memcpy(&vInt, ptlv_data_value, sizeof(vInt));
			vInt = DWORD_SWAP(vInt);
			memcpy(data, &vInt, sizeof(vInt));
			break;
			
		case STRING_T:
			pChar = (unsigned char *) data;
			strncpy((char *)pChar, (char *)ptlv_data_value, mib_tbl->total_size);
			break;
			
		case IA_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, mib_tbl->total_size);
			
			break;
			
		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, mib_tbl->total_size); // avoid alignment issue
			
			break;
			
#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
		case RADVDPREFIX_T:
		case DNSV6_T:
		case DHCPV6S_T:
		case DHCPV6C_T:
		case ADDR6_T:
		case ADDRV6_T:
		case TUNNEL6_T:
			memcpy(data, ptlv_data_value, mib_tbl->total_size);
			break;		
#endif
#endif
		default :
			fprintf(stderr,"\r\n ERR!no mib_name[%s] type[%u]. __[%s-%u]",mib_tbl->name, mib_tbl->type,__FILE__,__LINE__);			
			return 0;
		
		
	}
		
	return 1;
}

unsigned short find_same_tag_times(unsigned char *pdata_array, unsigned short data_size)
{
	unsigned short first_tlv_tag;
	unsigned short tlv_tag;	
	unsigned short tlv_len;
	unsigned short times=0;
	unsigned char *idx = pdata_array;
	int i=0;
	
	memcpy(&first_tlv_tag, idx, sizeof(first_tlv_tag));
	
	while(i<data_size)
	{
		memcpy(&tlv_tag, idx+i, sizeof(tlv_tag));
		i+=sizeof(tlv_tag);			
		
		if(tlv_tag == first_tlv_tag)
			times++;
		
		memcpy(&tlv_len, idx+i, sizeof(tlv_len));
		i+=sizeof(tlv_len);
		tlv_len = WORD_SWAP(tlv_len);
		i+=tlv_len;
		
	}
	
	return times;
}

unsigned int mib_tlv_init_from(mib_table_entry_T *mib_root_tbl, unsigned char *pdata_array, void *pfile, unsigned int data_size, unsigned int *pmib_root_offset)
{	
	unsigned char *idx=NULL;		
	int i=0;
	//int j;
	unsigned short tlv_tag=0;		
	unsigned short tlv_len=0;	
	unsigned short tlv_num=0;	
	//unsigned char tlv_data_value[1000]={0};	
	unsigned char *ptlv_data_value=NULL;		
	//unsigned int offset=0;		
	unsigned char mib_num[10]={0};	
	unsigned char *pmib_num = mib_num;	
	memset(mib_num, 0x00, sizeof(mib_num));		


	idx=pdata_array;	
	while(i<data_size)	
	{		
		memcpy(&tlv_tag, idx+i, sizeof(tlv_tag));
		tlv_tag = WORD_SWAP(tlv_tag);
		i+=sizeof(tlv_tag);					

		memcpy(&tlv_len, idx+i, sizeof(tlv_len));
		tlv_len = WORD_SWAP(tlv_len);
		i+=sizeof(tlv_len);



		if((tlv_tag & MIB_TABLE_LIST) == 0) // NO member		
		{
			mib_table_entry_T *mib_tbl=NULL;
			unsigned int mib_offset=0;

			if((ptlv_data_value=malloc(tlv_len)) == NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR! malloc fail. tlv_tag=%p, tlv_len=%u __[%s-%u]",tlv_tag, tlv_len, __FILE__,__LINE__);
				return 0;
			}
			memcpy(ptlv_data_value, idx+i, tlv_len);
			i+=tlv_len;

			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &mib_tbl, &mib_offset) != 1)			
			{
				//fprintf(stderr,"\r\n Can't find mib_id=%u",tlv_tag);
			}
			else
			{
//fprintf(stderr,"\r\n %u+%u",mib_offset,*pmib_root_offset );
				mib_offset += *pmib_root_offset;
				if(mib_tbl != NULL)
				{
				//fprintf(stderr,"\r\n");
					if(mib_init_value(ptlv_data_value, tlv_len, mib_tbl, (void *)((int) pfile + mib_offset)) != 1)
					{
						fprintf(stderr,"\r\n Assign mib_name[%s] fail!", mib_tbl->name);
						fprintf(stderr,"\r\n mibtbl->id (%08x) unitsize (%d) totoal size (%d) mibtbl->nextbl %p",mib_tbl->id,mib_tbl->unit_size,mib_tbl->total_size,mib_tbl->next_mib_table);
					}					
				}
			}
			if(ptlv_data_value != NULL)
			{
				free(ptlv_data_value);
				ptlv_data_value=NULL;
			}
		}
		else // have member		
		{			
			int j=0;
			mib_table_entry_T *pmib_tbl;
			unsigned int mib_offset=0;

			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &pmib_tbl, &mib_offset) != 1)			
			{
				//fprintf(stderr,"\r\n Can't find mib_id=%u",tlv_tag);		
				i+=tlv_len;	
			}
			else
			{
				if((ptlv_data_value=malloc(tlv_len)) == NULL)
				{
					COMP_TRACE(stderr,"\r\n ERR! malloc fail. tlv_tag=%p, tlv_len=%u __[%s-%u]",tlv_tag, tlv_len, __FILE__,__LINE__);
					return 0;
				}
				
				memcpy(ptlv_data_value, idx+i, tlv_len);
				tlv_num = find_same_tag_times(ptlv_data_value, tlv_len);
//fprintf(stderr,"\r\n tlv_num=%u __[%s-%u]", tlv_num,  __FILE__,__LINE__);
				if(tlv_num != 0)
					tlv_len = tlv_len/tlv_num;

				while(j<tlv_num)
				{
//fprintf(stderr,"\r\n TREE_NODE %s[%u] ENTRY",pmib_tbl->name,j);
					memcpy(ptlv_data_value, idx+i+(tlv_len*j), tlv_len);
					//printf("\r\n %u<%u/%u",j,pmib_tbl->total_size, pmib_tbl->unit_size );
					if( j < (pmib_tbl->total_size / pmib_tbl->unit_size))
					{
						unsigned int mib_tlb_offset=0;
//fprintf(stderr,"\r\n  __[%s-%u]",  __FILE__,__LINE__);
						//printf("\r\n %u+%u+%u*%u",mib_offset,*pmib_root_offset, j,pmib_tbl->unit_size );
						mib_tlb_offset = mib_offset + *pmib_root_offset+j*(pmib_tbl->unit_size);
						//printf("\r\n TREE_NODE name =%s[%u] mib_tbl_offset is %u",pmib_tbl->mib_name,  j, mib_tlb_offset);
						//printf("\r\n tlv_len=%u __[%s-%u]", tlv_len,  __FILE__,__LINE__);
						mib_tlv_init_from(pmib_tbl->next_mib_table, ptlv_data_value, pfile, tlv_len, &mib_tlb_offset);
					}
//fprintf(stderr,"\r\n TREE_NODE %s[%u] LEAVE",pmib_tbl->name,j);					
					j++;
				}
				if(ptlv_data_value != NULL)
				{
					free(ptlv_data_value);
					ptlv_data_value=NULL;
				}

				i+=tlv_len*tlv_num;				
			}
		}
	}
	return 1;
}

unsigned int mib_tlv_init(mib_table_entry_T *mib_tbl, unsigned char *from_data, void *pfile, unsigned int tlv_content_len)
{
	unsigned int mib_offset = 0;

	if(mib_tbl == NULL || from_data == NULL || pfile == NULL || tlv_content_len == 0)
		return 0;

	if(mib_tlv_init_from(mib_tbl, from_data, pfile, tlv_content_len, &mib_offset) == 1) {

		return 1;
	}	
	else {
		
		return 0;
	}	

}

extern void dumpMallInfo();

unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len)
{
	mib_table_entry_T *pmib_tl = NULL;

	//diag_printf("[%s:%d]addr tlv_content_len = %p\n", __FUNCTION__, __LINE__, tlv_content_len);				


	if(mib_tlvfile == NULL)
	{
		return 0;
	}
	//diag_printf("[%s:%d]mib_tlvfile = 0x%x\n", __FUNCTION__, __LINE__, mib_tlvfile);		
	
	pmib_tl = mib_get_table(type);

	if(pmib_tl==0)
	{
		return 0;
	}
//fprintf(stderr,"\r\n mib_data = %p, __[%s-%u]",mib_data,__FILE__,__LINE__);					
//fprintf(stderr,"\r\n mib_tlvfile = %p, __[%s-%u]",mib_tlvfile,__FILE__,__LINE__);	
//fprintf(stderr,"\r\n pmib_tl->name=%s, __[%s-%u]",pmib_tl->name,__FILE__,__LINE__);	
//fprintf(stderr,"\r\n tlv_content_len = %p, __[%s-%u]",tlv_content_len,__FILE__,__LINE__);
	mib_write_to_raw(pmib_tl, (void *)((int) mib_data), mib_tlvfile, tlv_content_len);
	
	return 1;

}

void mib_display_data_content(CONFIG_DATA_T type, unsigned char * pdata, unsigned int mib_data_len)
{
	int kk;
	fprintf(stderr,"\r\n type=%u, mibdata_len = %u",type, mib_data_len);
	fprintf(stderr,"\r\n pdata=");
	for(kk=0; kk< mib_data_len; kk++)
	{
		fprintf(stderr,"0x%02x_", *(pdata+kk));
		if( (kk+1)%10 == 0) fprintf(stderr,"\r\n");
	}
	fprintf(stderr,"0x%02x_",CHECKSUM(pdata, mib_data_len));
}

void mib_display_tlv_content(CONFIG_DATA_T type, unsigned char * ptlv, unsigned int mib_tlv_len)
{
	int kk;
	fprintf(stderr,"\r\n type=%u, tlv_content_len = %u",type, mib_tlv_len);			
	fprintf(stderr,"\r\n tlv_content=");
	for(kk=0; kk< mib_tlv_len; kk++)
	{
		fprintf(stderr,"0x%02x_", *(ptlv+kk));
		if( (kk+1)%10 == 0) fprintf(stderr,"\r\n");
	}
}

int mib_write_to_raw(const mib_table_entry_T *mib_tbl, void *data, unsigned char *pfile, unsigned int *idx)
{	
	//diag_printf("[%s:%d]---start---\n", __FUNCTION__, __LINE__);
	//dumpMallInfo();
	//diag_printf("[%s:%d]*idx=%d\n", __FUNCTION__, __LINE__,*idx);
	unsigned short tlv_tag=0;	
	unsigned short tlv_len=0;	
	unsigned short tlv_num=0;		
	int i, j;
	//int k;

#if 0
fprintf(stderr,"\r\n > mib_tbl->name=%s, __[%s-%u]",mib_tbl->name,__FILE__,__LINE__);	
#endif

//fprintf(stderr,"\r\n idx = %p, __[%s-%u]",idx,__FILE__,__LINE__);		
//fprintf(stderr,"\r\n data = %p, __[%s-%u]",data,__FILE__,__LINE__);					
//fprintf(stderr,"\r\n pfile = %p, __[%s-%u]",pfile,__FILE__,__LINE__);	


	//diag_printf("[%s:%d]mib_tbl->type=%d\n", __FUNCTION__, __LINE__, mib_tbl->type);
	if(mib_tbl->type >= TABLE_LIST_T)	
	{

		const mib_table_entry_T *mib = mib_tbl->next_mib_table;		
		unsigned int offset=0;				

		for(i=0 ; mib[i].id ; i++)		
		{	
			//diag_printf("[%s:%d] %d:%s\n", __FUNCTION__, __LINE__, i, mib[i].name);
			const mib_table_entry_T *pmib = &mib[i];			
#if 0
fprintf(stderr,"\r\n Turn mib[i].mib_name=%s __[%s-%u]",mib[i].name,__FILE__,__LINE__);
#endif
			if(mib[i].type < TABLE_LIST_T)			
			{				
				mib_write_to_raw(pmib, (void *)((int) data + offset), pfile, idx);				
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
//printf("\r\n offset + %u __[%s-%u]",pmib->unit_size,__FILE__,__LINE__);				
				//offset += pmib->size;
				offset += pmib->total_size;
			}
			else
			{				
				unsigned int ori_idx = 0;				
				unsigned char *ptlv_len = NULL;								
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
				if((pmib->total_size%pmib->unit_size) == 0)					
					tlv_num = pmib->total_size/pmib->unit_size;
				
				tlv_tag = (pmib->id);
				tlv_tag = WORD_SWAP(tlv_tag);
				memcpy(pfile+*idx, &tlv_tag, 2);
				*idx+=2;	
				
				tlv_len = WORD_SWAP(tlv_len);				
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
				memcpy(pfile+*idx, &tlv_len, 2);				
				ptlv_len = (unsigned char *)(pfile+*idx);				
				*idx+=2;								
				tlv_num = pmib->total_size/pmib->unit_size;				
				ori_idx = *idx;
//printf("\r\n -- ptlv_len[0x%x] *idx[%u] ori_idx[%u] tlv_num[%u]<< ",ptlv_len, *idx,ori_idx,tlv_num);

				for(j=0 ; j<tlv_num ; j++)				
				{
				
					mib_write_to_raw(pmib, (void *)((int) data + offset), pfile, idx);
					offset += pmib->unit_size;					
				}											
				tlv_len = (*idx-ori_idx);	
				tlv_len = WORD_SWAP(tlv_len); // for endian
				memcpy(ptlv_len, &tlv_len, 2);
			}								
		}		
	}
	else
	{	
		unsigned char *pChar = (unsigned char *) data;		
		//unsigned short mib_value;

		tlv_tag = (mib_tbl->id);
		tlv_tag = WORD_SWAP(tlv_tag); // for endian
		memcpy(pfile+*idx, &tlv_tag, 2);		
		*idx+=2;				

		//tlv_len = (mib_tbl->size);
        tlv_len = (mib_tbl->total_size);
		tlv_len = WORD_SWAP(tlv_len); // for endian
		memcpy(pfile+*idx, &tlv_len, 2);

		*idx+=2;	

		memcpy(pfile+*idx, pChar, WORD_SWAP(tlv_len));				
		*idx+=WORD_SWAP(tlv_len);
		
#if 0 // TLV_DEBUG
fprintf(stderr,"\r\n >>mib_tbl->name=%s",mib_tbl->name);	
fprintf(stderr,"\r\n >>>tlv_tag=0x%x, tlv_len=0x%x, tlv_value = ",tlv_tag,tlv_len);
for(k=0; k< tlv_len; k++)
{	
fprintf(stderr,"0x%02x_", *(pChar+k));	
if( (k+1)%10 == 0) fprintf(stderr,"\r\n");
}
fprintf(stderr,"\r\n");
#endif

#if 0
fprintf(stderr,"\r\n pFile=");
for(k=0; k< *idx; k++)
{	
fprintf(stderr,"0x%02x_", *(pfile+k));	
if( (k+1)%10 == 0) fprintf(stderr,"\r\n");
}
#endif			
	}	
	return 1;
}

// NOT allow the same id in different mib table
int mibtbl_check_add(mib_table_entry_T *root, mib_table_entry_T *mib_tbls[])
{
	int i, cnt;

	cnt = 0;
	for (i=0; root[i].id; i++)
	{
		if (root[i].type >= TABLE_LIST_T)
			cnt += mibtbl_check_add(root[i].next_mib_table, &mib_tbls[cnt]);
		else
			mib_tbls[cnt++] = &root[i];
	}

	return cnt;
}

static int
mibtbl_check_id(const void *p1, const void *p2)
{
	int res;
	mib_table_entry_T *mib1 = * (mib_table_entry_T * const *) p1;
	mib_table_entry_T *mib2 = * (mib_table_entry_T * const *) p2;

//	if (mib1->id == mib2->id)
	if (mib1->id == mib2->id && mib1->name != mib2->name)
		fprintf(stderr, "MIB Error: %s detect duplicate id in %s\n",
			mib1->name, mib2->name);

	return mib1->id - mib2->id;
}

#define MAX_MIBTBL_CHECK 1024
int mibtbl_check(void)
{
	int i;
	int cnt;
	static mib_table_entry_T *mib_tbls[MAX_MIBTBL_CHECK];
	mib_table_entry_T *pmib_tl_hw;
	mib_table_entry_T *pmib_tl;

	pmib_tl_hw = mib_get_table(HW_SETTING);
	pmib_tl = mib_get_table(CURRENT_SETTING);

	cnt = mibtbl_check_add(pmib_tl_hw, mib_tbls);
	cnt += mibtbl_check_add(pmib_tl, &mib_tbls[cnt]);

	if (cnt >= MAX_MIBTBL_CHECK)
	{
		fprintf(stderr, "MAX_MIBTBL_CHECK is smaller than MIB TBL\n");
		return -1;
	}

	qsort(mib_tbls, cnt, sizeof(mib_table_entry_T *), mibtbl_check_id);
	return 0;	
}

#endif
#ifdef KLD_ENABLED
void Wizard_load_mib(void)
{
	unsigned char buffer[128]={0};
	unsigned int Value=0;
	
//	fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
//diag_printf("*************** Wizard_load_mib ***********\n");
	pWizMib ->wizardWanConnMode = 0; //0:Auto

	pWizMib ->wizardSettingChanged = 0;
	//DHCP_T dhcp;
	apmib_get( MIB_IP_ADDR,  (void *)buffer); 
	memcpy(pWizMib->ipAddr, buffer, 4);
	
	apmib_get( MIB_SUBNET_MASK,  (void *)buffer);
	memcpy(pWizMib->subnetMask, buffer, 4);
	
	apmib_get( MIB_DHCP, (void *)&Value);
	pWizMib->dhcp = (unsigned char)Value;
	
	apmib_get( MIB_DHCP_CLIENT_START,  (void *)buffer);
	memcpy(pWizMib->dhcpClientStart, buffer, 4);
	
	apmib_get( MIB_DHCP_CLIENT_END,  (void *)buffer);
	memcpy(pWizMib->dhcpClientEnd, buffer, 4);
	
	
	apmib_get( MIB_HOST_NAME, (void *)&buffer); 
	sprintf(pWizMib->hostName, "%s", buffer);
	
	apmib_get(MIB_WAN_MAC_ADDR,  (void *)buffer);
	memcpy(pWizMib->wanMacAddr, buffer, 6);
	
	apmib_get( MIB_WAN_DHCP, (void *)&Value);
	pWizMib->wanDhcp = (unsigned char)Value;
	
	apmib_get( MIB_WAN_IP_ADDR,  (void *)buffer);
	memcpy(pWizMib->wanIpAddr, buffer, 4);
	
	apmib_get( MIB_WAN_SUBNET_MASK,  (void *)buffer);
	memcpy(pWizMib->wanSubnetMask, buffer, 4);
	
	apmib_get( MIB_WAN_DEFAULT_GATEWAY,  (void *)buffer);
	memcpy(pWizMib->wanDefaultGateway, buffer, 4);
	
	
	apmib_get( MIB_WAN_IP_DYNAMIC, (void *)&Value);
	pWizMib->wanIPMode = (unsigned char)Value;
	
	apmib_get( MIB_PPPOE_WAN_IP_DYNAMIC, (void *)&Value);
	pWizMib->pppoewanIPMode = (unsigned char)Value;
	
	apmib_get( MIB_PPP_SERVICE_NAME,  (void *)buffer);
	sprintf(pWizMib->pppServiceName, "%s", buffer);
	
	apmib_get( MIB_PPP_USER_NAME,  (void *)buffer);
	sprintf(pWizMib->pppUserName, "%s", buffer);
	
	apmib_get( MIB_PPP_PASSWORD,  (void *)buffer);
	sprintf(pWizMib->pppPassword, "%s", buffer);
	
	apmib_get( MIB_PPPOE_IP,  (void *)buffer);
	memcpy(pWizMib->pppoeipAddr, buffer, 4);
	
	
	apmib_get( MIB_PPTP_WAN_IP_DYNAMIC, (void *)&Value);
	pWizMib->pptpwanIPMode = (unsigned char)Value;
	
	apmib_get( MIB_PPTP_IP_ADDR,  (void *)buffer);
	memcpy(pWizMib->pptpIpAddr, buffer, 4);
	
	
	apmib_get( MIB_PPTP_SUBNET_MASK,  (void *)buffer);
	memcpy(pWizMib->pptpSubnetMask, buffer, 4);
	
	apmib_get( MIB_PPTP_GATEWAY,  (void *)buffer);
	memcpy(pWizMib->pptpGateway, buffer, 4);
	
	apmib_get( MIB_PPTP_SERVER_IP_ADDR,  (void *)buffer);
	sprintf(pWizMib->pptpServerIpAddr, "%s", buffer);
	
	apmib_get( MIB_PPTP_USER_NAME,  (void *)buffer);
	sprintf(pWizMib->pptpUserName, "%s", buffer);
	
	apmib_get( MIB_PPTP_PASSWORD,  (void *)buffer);
	sprintf(pWizMib->pptpPassword, "%s", buffer);
	
	
	
	apmib_get( MIB_L2TP_WAN_IP_DYNAMIC, (void *)&Value);
	pWizMib->l2tpwanIPMode = (unsigned char)Value;
	
	apmib_get( MIB_L2TP_USER_NAME,  (void *)buffer);
	sprintf(pWizMib->l2tpUserName, "%s", buffer);
	
	apmib_get( MIB_L2TP_PASSWORD,  (void *)buffer);
	sprintf(pWizMib->l2tpPassword, "%s", buffer);
	
	apmib_get( MIB_L2TP_IP_ADDR,  (void *)buffer);
	memcpy(pWizMib->l2tpIpAddr, buffer, 4);
	
	apmib_get( MIB_L2TP_SUBNET_MASK,  (void *)buffer);
	memcpy(pWizMib->l2tpSubnetMask, buffer, 4);
	
	apmib_get( MIB_L2TP_GATEWAY,  (void *)buffer);
	memcpy(pWizMib->l2tpGateway, buffer, 4);
	
	apmib_get( MIB_L2TP_SERVER_IP_ADDR,  (void *)buffer);
	sprintf(pWizMib->l2tpServerIpAddr, "%s", buffer);
	
	
	apmib_get(MIB_USER_PASSWORD, (void *)buffer);
	sprintf(pWizMib->adminPassword, "%s", buffer);

	apmib_get( MIB_NTP_ENABLED, (void *)&Value);
	pWizMib->ntp_enabled = (unsigned char)Value;
		
	apmib_get( MIB_NTP_TIMEZONE,  (void *)buffer);
	sprintf(pWizMib->ntpTimeZone, "%s", buffer);
	
	apmib_get( MIB_NTP_SERVER_IP1,  (void *)&Value);
	memcpy(&(pWizMib->ntpServerIp1),&Value,sizeof(unsigned int));

	apmib_get( MIB_NTP_TIMEZONE_IDX, (void *)&Value);
	pWizMib->ntpTimeZoneIdx = (unsigned char)Value;
	
	apmib_get( MIB_DNS1,  (void *)buffer);
	memcpy(pWizMib->dns1, buffer, 4);
	
	apmib_get( MIB_DNS2,  (void *)buffer);
	memcpy(pWizMib->dns2, buffer, 4);
	
	apmib_get( MIB_DNS3,  (void *)buffer);
	memcpy(pWizMib->dns3, buffer, 4);
	
	apmib_get( MIB_WAN_DNS_MODE, (void *)&Value);
	pWizMib->dnsMode = Value;
	
	apmib_get( MIB_WLAN_SSID,  (void *)buffer);
	sprintf(pWizMib->ssid, "%s", buffer);
	
	apmib_get( MIB_WLAN_CHANNEL,  (void *)&Value);
	pWizMib->channel = (unsigned char)Value;
	
	apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&Value);
	pWizMib->wlanDisabled = (unsigned char)Value;
}
void Wizard_Update_mib(void)
{
	int value=0;
	apmib_set(MIB_USER_PASSWORD, (void *)pWizMib->adminPassword); 
	apmib_set(MIB_NTP_TIMEZONE, (void *)pWizMib->ntpTimeZone);
	apmib_set(MIB_NTP_SERVER_IP1, (void *)&(pWizMib->ntpServerIp1));
	value = pWizMib->ntp_enabled;
	apmib_set( MIB_NTP_ENABLED, (void *)&value);
	value = pWizMib->ntpTimeZoneIdx;
	apmib_set(MIB_NTP_TIMEZONE_IDX, (void *)&value);	
	apmib_set( MIB_IP_ADDR, (void *)pWizMib->ipAddr);
	apmib_set(MIB_SUBNET_MASK, (void *)pWizMib->subnetMask);
					
	value = pWizMib->dhcp;
	apmib_set(MIB_DHCP, (void *)&value);
	if(pWizMib->dhcp==DHCP_SERVER){
		apmib_set(MIB_DHCP_CLIENT_START, (void *)pWizMib->dhcpClientStart);
		apmib_set(MIB_DHCP_CLIENT_END, (void *)pWizMib->dhcpClientEnd);
	}
	value = pWizMib->wanDhcp;
	apmib_set(MIB_WAN_DHCP, (void *)&value);
	value = pWizMib->wanIPMode;
	apmib_set(MIB_WAN_IP_DYNAMIC, (void *)&value);
	if(pWizMib->wanDhcp == DHCP_CLIENT){		
		apmib_set(MIB_HOST_NAME, (void *)pWizMib->hostName);
		apmib_set(MIB_WAN_MAC_ADDR, (void *)pWizMib->wanMacAddr);		
	}else if(pWizMib->wanDhcp == PPPOE){
		value = pWizMib->pppoewanIPMode;
		apmib_set(MIB_PPPOE_WAN_IP_DYNAMIC, (void *)&value);
		apmib_set(MIB_PPP_SERVICE_NAME, (void *)pWizMib->pppServiceName);
		if(pWizMib->pppoewanIPMode==1)//static  ip
			apmib_set(MIB_PPPOE_IP, (void *)pWizMib->pppoeipAddr);
		apmib_set(MIB_PPP_USER_NAME, (void *)pWizMib->pppUserName);	
		apmib_set(MIB_PPP_PASSWORD, (void *)pWizMib->pppPassword);		
	}else if(pWizMib->wanDhcp == PPTP){
		value = pWizMib->pptpwanIPMode;
		apmib_set(MIB_PPTP_WAN_IP_DYNAMIC, (void *)&value);
		apmib_set(MIB_PPTP_USER_NAME, (void *)pWizMib->pptpUserName);
		apmib_set(MIB_PPTP_PASSWORD, (void *)pWizMib->pptpPassword);
		apmib_set(MIB_PPTP_IP_ADDR, (void *)pWizMib->pptpIpAddr);
		apmib_set(MIB_PPTP_SERVER_IP_ADDR, (void *)pWizMib->pptpServerIpAddr);
		apmib_set(MIB_PPTP_SUBNET_MASK, (void *)pWizMib->pptpSubnetMask);
		apmib_set(MIB_PPTP_GATEWAY, (void *)pWizMib->pptpGateway);		
	}else if(pWizMib->wanDhcp == L2TP){
		value = pWizMib->l2tpwanIPMode;
		apmib_set(MIB_L2TP_WAN_IP_DYNAMIC, (void *)&value);
		apmib_set(MIB_L2TP_USER_NAME, (void *)pWizMib->l2tpUserName);
		apmib_set(MIB_L2TP_PASSWORD, (void *)pWizMib->l2tpPassword);
		apmib_set(MIB_L2TP_SERVER_IP_ADDR, (void *)pWizMib->l2tpServerIpAddr);
		apmib_set( MIB_L2TP_IP_ADDR,  (void *)pWizMib->l2tpIpAddr);	
		apmib_set( MIB_L2TP_SUBNET_MASK,  (void *)pWizMib->l2tpSubnetMask);	
		apmib_set( MIB_L2TP_GATEWAY,  (void *)pWizMib->l2tpGateway);
	}
	else{ //wan fixed ip mode	
		apmib_set(MIB_WAN_IP_ADDR, (void *)pWizMib->wanIpAddr);
		apmib_set(MIB_WAN_SUBNET_MASK, (void *)pWizMib->wanSubnetMask);
		apmib_set(MIB_WAN_DEFAULT_GATEWAY, (void *)pWizMib->wanDefaultGateway);
		if(pWizMib->dns1[0] != '\0')
		apmib_set(MIB_DNS1, (void *)pWizMib->dns1);
		if(pWizMib->dns2[0] != '\0')
		apmib_set(MIB_DNS2, (void *)pWizMib->dns2);
		if(pWizMib->dns3[0] != '\0')
		apmib_set(MIB_DNS3, (void *)pWizMib->dns3);
	}
	value= pWizMib->wlanDisabled;
	apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&value);
	value= pWizMib->channel;
	apmib_set( MIB_WLAN_CHANNEL, (void *)&(value));
	apmib_set(MIB_WLAN_SSID, (void *)pWizMib->ssid);
	
	apmib_set( MIB_WAN_DNS_MODE, (void *)&(pWizMib->dnsMode));	
}

#endif
