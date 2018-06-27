/*utility.c for misc functions*/
#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
//#include "cyg/io/eth/rltk/819x/wlan/ieee802_mib.h"
#endif


#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <time.h>

#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <stdio.h>                     // sprintf().
#include <stdlib.h>
#include <stdarg.h>
#ifdef CONFIG_NET_STACK_FREEBSD
#include <net/if_var.h>
#else
#include <net/if.h>
#endif

#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <net/if_bridge.h>


#include "wl_utility_rltk.h"
#include <sys/socket.h>



/*add fh 仅用于8192cd_hw.c中分配内存，规避从Misc mpool分配内存不足导致死机*/
void * wifi_kmalloc(size_t size,int flag)
{
	 return malloc(size);
}
void wifi_kfree(void *ptr) 
{
	free(ptr);
}




#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  //printf("\033[32m[%s]->[%s]->[%d]\033[0mskfd:%d,ifname:%s,request:0x%X\n",__FILE__,__func__,__LINE__,skfd,ifname,request);
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}
#endif


#if 0 //以后可能会用到
int getWdsInfo(char *interface, char *pInfo)
{

#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd;
    struct iwreq wrq;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
	{
		perror("socket error\n");
		return -1;			
	}
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
	printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);

    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      /* If no wireless name : no wireless extensions */
	close(skfd);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = MAX_WDS_NUM*sizeof(WDS_INFO_T);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETWDSINFO, &wrq) < 0) {
	close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
   }
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
    memset(pInfo, 0, MAX_WDS_NUM*sizeof(WDS_INFO_T)); 
#endif
#endif
    return 0;
}
#endif

int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
	//printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
	return -1 ;
#endif
#endif
    return 0;
}


#if !defined(__CONFIG_WPS_RTK__) && !defined(HAVE_NOWIFI)
int getWlJoinResult(char *interface, unsigned char *res)
{
#ifndef HAVE_NOWIFI
    int skfd;
    struct iwreq wrq;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
	printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
#endif
    return 0;
}
#endif

int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd=0;

    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
	{
		printf("%s(%d) skfd=-1\n",__func__,__LINE__);
		return -1;
	}

    /* Get wireless name */
	//printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );

		printf("%s(%d) no wireless name!\n",__func__,__LINE__);
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);
	
    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0){
    	close( skfd );

		printf("%s(%d)\n",__func__,__LINE__);
		return -1;
	}
    close( skfd );


    if ( result == 0xff )
    	*pStatus = -1;
    else
	*pStatus = (int) result;
#else
	*pStatus = -1;
#endif
#ifdef CONFIG_RTK_MESH 
	// ==== modified by GANTOE for site survey 2008/12/26 ==== 
	return (int)*(char*)wrq.u.data.pointer; 
#else
	return 0;
#endif
#endif
	return 0;
}


int getWlBssInfo(char *interface, bss_info *pInfo)
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
	int skfd=0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1) {
			printf("%s:socket fail\n", __FUNCTION__);
			return -1;
	}
#ifdef ECOS_DBG_STAT
	   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif
	/* Get wireless name */
	//printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
	{
			/* If no wireless name : no wireless extensions */
			// rock: avoid status page error if no wlan interface
			memset(pInfo, 0, sizeof(bss_info));
			//printf("interface=%s\n", interface);
			//printf("%s:iw_get_ext fail\n", __FUNCTION__);
			close( skfd );
#ifdef ECOS_DBG_STAT
				dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

			return 0;
	}

	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length = sizeof(bss_info);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0){
			printf("%s:iw_get_ext2 fail\n", __FUNCTION__);
			close( skfd );
#ifdef ECOS_DBG_STAT
				dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

			return -1;
	}
	close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
	memset(pInfo, 0, sizeof(bss_info));
#endif
#else
	memset(pInfo, 0, sizeof(bss_info));
#endif
#endif
    return 0;
}

/*add by lqz for check wifi password is or not wrong*/
static bss_info tenda_bss = {0};

wlan_mac_state tenda_getWifiStatus()
{
	return tenda_bss.state;
}
/*end*/

int getWlanLink(char* interface)
{
	int connected = 0;

	getWlBssInfo(interface, &tenda_bss);

	if(tenda_bss.state == STATE_CONNECTED)
		connected = 1;
	else
		connected = 0;

	return connected;
}

#ifndef ROUTER_BUFFER_SIZE
#define ROUTER_BUFFER_SIZE 512
#endif
/////////////////////////////////////////////////////////////////////////////

#if 0
int check_wlan_downup(char wlanIndex)
{
	int sock=0,flags=0;
	struct	ifreq	ifr={0};
	snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),"wlan%d",wlanIndex);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		diag_printf("socket error!\n");
		return -1;
	}
	if (ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) 
	{		
		//diag_printf("SIOCGIFFLAGS error!\n");
		close(sock);
		return (-1);
	}
	close(sock);
	if((ifr.ifr_flags & IFF_UP) != 0)
	{
		return 1;
	}	
	return 0;
}
#endif


/////////////////////////////////////////////////////////////////////////////
int getWlStaNum( char *interface, int *num )
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd=0;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
	//printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

      return -1;
	}
    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0){
    	 close( skfd );
#ifdef ECOS_DBG_STAT
			 dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    *num  = (int)staNum;

    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
    *num = 0 ;
#endif
#endif
    return 0;
}


int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
#ifndef HAVE_NOWIFI
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */

    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);
    memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

		return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
#endif
    return 0;
}


/************************************************************
Function:	 getwlmaclist  
Description: 获取无线客户端mac地址
Input:                              
	接口名称，需要填充的mac列表数据结构
Output:                                   
	无
Return:                                    
	
Others:                                  
************************************************************/

int getwlmaclist(char *ifname , char *maclist)
{
	struct wlmaclist wlmaclist;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	int i;
	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 )
	{
 		return 1;
	}
	if ( getWlStaInfo(ifname,  (WLAN_STA_INFO_Tp)buff ) < 0 ) 
	{
 		free(buff);
		return 1;
	}
	wlmaclist.count = 0;
	for (i=1; i<=MAX_STA_NUM; i++)
	{
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
		{
			memcpy( wlmaclist.ea[i-1].octet ,pInfo->addr,sizeof(pInfo->addr));
			wlmaclist.count++;
		}
	}
	memcpy(maclist,&wlmaclist,sizeof(wlmaclist.count) + wlmaclist.count * sizeof(struct ether_addr));
	free(buff);
	return 0;
}

int getBridgeInfo(char *ifname, WLAN_STA_INFO_Tp *bridge_info)
{
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	int i;
	int wl_rssi = 0;

	if(bridge_info == NULL)
		return 0;
	
	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 )
	{
 		return 0;
	}
	if ( getWlStaInfo(ifname,  (WLAN_STA_INFO_Tp)buff ) < 0 ) 
	{
 		free(buff);
		return 0;
	}
	
	for (i=1; i<=MAX_STA_NUM; i++)
	{
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
		{
			memcpy(bridge_info, pInfo, sizeof(WLAN_STA_INFO_T));
			
			break;
		}
	}
	
	free(buff);

	if(i > MAX_STA_NUM)
		return 0;
	
	return 1;
}


// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
#ifdef CONFIG_RTK_MESH 
int setWlJoinMesh (char *interface, unsigned char* MeshId, int MeshId_Len, int Channel, int offset, int Reset) 
{ 
    int skfd; 
    struct iwreq wrq; 
    struct 
    {
        unsigned char *meshid;
        int meshid_len, channel, offset, reset; 
    }mesh_identifier; 
  
    skfd = socket(AF_INET, SOCK_DGRAM, 0); 
    
    /* Get wireless name */ 
	printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
    /* If no wireless name : no wireless extensions */ 
        return -1; 

    mesh_identifier.meshid = MeshId; 
    mesh_identifier.meshid_len = MeshId_Len; 
    mesh_identifier.channel = Channel;
    mesh_identifier.offset = offset;    
    mesh_identifier.reset = Reset;
    wrq.u.data.pointer = (caddr_t)&mesh_identifier; 
    wrq.u.data.length = sizeof(mesh_identifier); 

    if (iw_get_ext(skfd, interface, SIOCJOINMESH, &wrq) < 0) 
        return -1; 

    close( skfd ); 

    return 0; 
} 
// This function might be removed when the mesh peerlink precedure has been completed

#endif 

#if 0 //该函数将来可能有用
int getMiscData(char *interface, struct _misc_data_ *pData)
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
    int skfd;
    struct iwreq wrq;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
	{
		perror("socket error\n");
		return -1;
	}
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
	printf("\033[32m[%s]->[%s]->[%d]\033[0m\n",__FILE__,__func__,__LINE__);

    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
	/* If no wireless name : no wireless extensions */
	close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pData;
    wrq.u.data.length = sizeof(struct _misc_data_);

    if (iw_get_ext(skfd, interface, SIOCGMISCDATA, &wrq) < 0) {
	close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
    }

    close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
    memset(pData, 0, sizeof(struct _misc_data_)); 
#endif
#else
    memset(pData, 0, sizeof(struct _misc_data_)); 
#endif
#endif
    return 0;
}
#endif

/************************************************************
Function:	 createWlSiteSurveyList  (原博通中的函数名称为wds_enr_create_aplist)        
Description: 调用getWlSiteSurveyRequest()和getWlSiteSurveyResult()。
			 并构造页面显示需要的数据
Input:                              
	页面需要的数据结结构的数组，数组长度不小于64
Output:                                   
	无
Return:                                    
	扫描到的信号数量
Others:                                  
************************************************************/
int createWlSiteSurveyList(wlan_sscan_list_info_t * list,char*ifname)
{
	SS_STATUS_Tp pStatus=NULL;//存放扫描结果
	BssDscr *pBss = NULL;//临时存放单条扫描结果
	int result_count = 0;
	int wpa_exist = 0;
	char mac[18] = {0};

	if (pStatus==NULL)
	{
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL )
		{
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}
	pStatus->number = 0; // 初始化扫描结果个数
	if ( getWlSiteSurveyResult(ifname,pStatus) < 0 )
	{
		printf("Read site-survey status failed!");
	}
	result_count = pStatus->number;
	printf("WlSiteSurvey number:%d\n",result_count);

	//组织返回页面需要的数据
	int i;
	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++)
	{
		pBss = &pStatus->bssdb[i];

		list[i].used = TRUE;
		
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);
		memcpy(list[i].bssid,mac,strlen(mac));
	//	printf("====list[i].bssid:%s==%s [%d]\n",list[i].bssid, __FUNCTION__, __LINE__);
		strncpy(list[i].ssid,pBss->bdSsIdBuf,pBss->bdSsId.Length);
		list[i].ssid[pBss->bdSsId.Length] = '\0';
#if 0
		if(1 == is_gb2312_code(list[i].ssid))
		{
			strncpy(list[i].encode,"GB2312",strlen("GB2312"));
		       printf("========%s=======%s [%d]\n",list[i].ssid, __FUNCTION__, __LINE__);
		}else
#endif
		{
			strncpy(list[i].encode,"UTF-8",strlen("UTF-8"));
		}
	
		list[i].channel = pBss->ChannelNumber;

		list[i].SignalStrength = pBss->rssi - 100;
		//转换加密标识

		if ((pBss->bdCap & cPrivacy) == 0)
			sprintf(list[i].SecurityMode, "NONE");
		else 
		{
			if (pBss->bdTstamp[0] == 0)
				sprintf(list[i].SecurityMode, "UNKNOW");//wep加密，不支持，设置成UNKNOW
			else 
			{
				wpa_exist = 0;
				if (pBss->bdTstamp[0] & 0x0000ffff) 
				{
					memset(list[i].SecurityMode,0x0,32);
					sprintf(list[i].SecurityMode, "WPA");

					wpa_exist = 1;

					if (((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x2)
					{
						memset(list[i].SecurityMode,0x0,32);
						sprintf(list[i].SecurityMode, "UNKNOW");//WPA企业级加密不支持，设置成UNKNOW
					}

					if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x5)
						strcat(list[i].SecurityMode,"/AESTKIP");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x4)
						strcat(list[i].SecurityMode,"/AES");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x1)
						strcat(list[i].SecurityMode,"/TKIP");
				}
				if (pBss->bdTstamp[0] & 0xffff0000) 
				{
					memset(list[i].SecurityMode,0x0,32);
					if (wpa_exist)
						sprintf(list[i].SecurityMode, "WPAWPA2");
					else
						sprintf(list[i].SecurityMode, "WPA2");

					if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x2)
					{
						memset(list[i].SecurityMode,0x0,32);
						sprintf(list[i].SecurityMode, "UNKNOW");//WPA企业级加密不支持，设置成UNKNOW
					}
					
					if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x5)
						strcat(list[i].SecurityMode,"/AESTKIP");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x4)
						strcat(list[i].SecurityMode,"/AES");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x1)
						strcat(list[i].SecurityMode,"/TKIP");
				}
			}
		}
	}
	
	free(pStatus);
	pStatus = NULL;
	return result_count;
	
}
/*****************************************************************************
 函 数 名  : wl_channel_scan_app
 功能描述  : 用于app中WIFI信道质量的功能，在get的时候，需要执行以下几个命令
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : 段靖铖
    修改内容   : 新生成函数


*****************************************************************************/
void wl_channel_scan_app()
{
	//RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "scanCIE","");
	//sleep(3);//等待扫描结束，更新结果
	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "get_ch_score","");
	sleep(5);
}

/*****************************************************************************
 函 数 名  : wl_channel_optimize_app
 功能描述  : 用于app中WIFI信道质量的功能，使他全部设置成自动状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void wl_channel_optimize_app()
{
	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "autoch","");
}

/************************************************************
Function:	 delete_all_online_client      
Description: 删除所有无线客户端
Input: ifname    接口名                       
Output:                                   
Return:0 删除失败，1 删除成功                                   
Others:                  lq add                
************************************************************/
int delete_all_online_client(char *ifname)
{
	struct wlmaclist *mac_list = NULL;
	char dev_mac[13] = {0};
	int mac_list_size = 0;
	int i =0;

	if(NULL == ifname)
		return 0;

	mac_list_size = sizeof(uint) + MAX_STA_NUM * sizeof(struct ether_addr);
	mac_list = (struct wlmaclist *)malloc(mac_list_size);
	
	if (getwlmaclist(ifname, mac_list))
	{
		free(mac_list);
		return 0;
	}

	for (i = 0; i < mac_list->count; ++i)
	{
		memset(dev_mac, '\0', 13 * sizeof(char));
		sprintf(dev_mac,"%02X%02X%02X%02X%02X%02X",
			(mac_list->ea[i].octet)[0],(mac_list->ea[i].octet)[1],
			(mac_list->ea[i].octet)[2],(mac_list->ea[i].octet)[3],
			(mac_list->ea[i].octet)[4],(mac_list->ea[i].octet)[5]);
		RunSystemCmd(0, "iwpriv", ifname, "del_sta", dev_mac, "");
	}

	free(mac_list);
	return 1;

}

#ifdef __CONFIG_WL_MAC_FILTER__
/************************************************************
Function:	 wl_access_control      
Description: 将指定无线客户端加入黑名单
Input: ifname   	接口名称，mac地址，操作类型{ADD_ACL,REMOVE_ACL}                      
Output:                                   
Return:0 删除失败，1 删除成功                                   
Others:                  lq add                
************************************************************/
int wl_access_control(char *ifname,char *mac,int type)
{
	char dev_mac[13] = {0};
	char *cur_mac = mac;
	int i =0;
	if(NULL == cur_mac || NULL == ifname)
		return 0;
	
	while(i < 12)
	{
		if('\0' == *cur_mac)
			break;
		
		if(':' == *cur_mac)
		{
			cur_mac++;
			continue;
		}
		
		dev_mac[i] = *cur_mac;
		cur_mac++;
		i++;
	}
	dev_mac[i] = '\0';

	if(ADD_ACL == type)
		RunSystemCmd(0, "iwpriv", ifname, "add_acl_table", dev_mac, "");
	else if(REMOVE_ACL == type)
		RunSystemCmd(0, "iwpriv", ifname, "rm_acl_table", dev_mac, "");
	else
		return 0;

	return 1;

}

/*****************************************************************************
 函 数 名  : wl_add_acladdr
 功能描述  : 设置无线访问控制的控制列表
 输入参数  : char *ifname  
             char *mac     
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月29日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int wl_add_acladdr(char *ifname,char *mac)
{
	char dev_mac[13] = {0};
	char *cur_mac = mac;
	char *tmp[32] = {0};
	int i =0;
	if(NULL == cur_mac || NULL == ifname)
		return 0;
	
	while(i < 12)
	{
		if('\0' == *cur_mac)
			break;
		
		if(':' == *cur_mac)
		{
			cur_mac++;
			continue;
		}
		
		dev_mac[i] = *cur_mac;
		cur_mac++;
		i++;
	}
	dev_mac[i] = '\0';
	sprintf(tmp,"acladdr=%s",dev_mac);
	RunSystemCmd(0, "iwpriv", ifname, "set_mib",tmp, "");
	RunSystemCmd(0, "iwpriv", ifname, "add_acl_table", dev_mac, "");
	return 1;

}

/*****************************************************************************
 函 数 名  : Refresh_acl
 功能描述  : 根据黑名单同步更新无线访问控制列表并使之生效
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
void refresh_acl_table()
{
	int acl_count = 0;
	char tmp[100] = {0};
	char mac[64] = {0}, filter_mac[32] = {0}, acl_mac_list[(MACFILTER_ITEM_LEN + 1) * MAX_MACFILTER_LIST_MUM +1] = {0};
	int    filter_mode = 2;
	int    filter_num = 0;
	if(strcmp(nvram_safe_get(ADVANCE_MACFILTER_MODE),"pass") == 0)
	{
		filter_mode = 1;
	}
	
	/* 1、清除原来的数据 */
	RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "clear_acl_table", "");
	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "clear_acl_table", "");
	/*lq 设置无线访问控制模式*/
	memset(tmp,0x0,100);
	sprintf(tmp,"aclmode=%d",filter_mode);
	RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "set_mib",tmp, "");
	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "set_mib",tmp, "");
	memset(tmp,0x0,100);
	nvram_unset(ADVICE_MAC_LIST);

	/* 2、根据黑名单列表更新无线访问控制列表 */
	for (acl_count = 0; acl_count < MAX_MACFILTER_LIST_MUM; acl_count++)
	{
		if(filter_mode == 1)
		{
			sprintf(filter_mac,"%s%d","white_list",acl_count);
		}
		else
		{
			sprintf(filter_mac,"%s%d","filter_mac",acl_count);
		}
	    	strcpy(tmp,nvram_safe_get(filter_mac));
		if(strlen(tmp) > MACFILTER_ITEM_LEN)
		{
			strcpy(mac,strtok(tmp,","));
			if(strlen(acl_mac_list) < MACFILTER_ITEM_LEN)
			{
				strcpy(acl_mac_list,mac);
			}
			else
			{
				sprintf(acl_mac_list,"%s %s",acl_mac_list,mac);
			}
			/*设置无线访问控制列表*/
			
			wl_add_acladdr(TENDA_WLAN5_AP_IFNAME,mac);
			wl_add_acladdr(TENDA_WLAN24_AP_IFNAME,mac);
			filter_num += 1;
		}
	}
	/*更新无线访问控制*/
	sprintf(tmp,"aclnum=%d",filter_num);
	RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "set_mib",tmp, "");
	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "set_mib",tmp, "");
	RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "fflush_acl_list", "");
	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "fflush_acl_list", "");
	nvram_set(ADVICE_MAC_LIST,acl_mac_list);
	nvram_commit();
}
/*获取扫描到的设备的个数*/
int getWlDevProbeNum( char *interface, int *num )
{
	int skfd=0;
	unsigned short devNum;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
	{

		close( skfd );
		return -1;
	}
	wrq.u.data.pointer = (caddr_t)&devNum;
	wrq.u.data.length = sizeof(devNum);

	if (iw_get_ext(skfd, interface, SIOCDEVPROBENUM, &wrq) < 0)
	{
		close( skfd );
		return -1;
	}
	*num  = (int)devNum;
	close( skfd );

	return 0;
}


/*获取扫描到的设备的信息，这里需要注意*/
int getWlDevProbeInfo( char *interface,  WLAN_DEV_PROBE_INFO_Tp pInfo )
{
	int skfd=0;
	int ret = 0 ;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){

		close( skfd );
		return -1;
	}
	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length = sizeof(WLAN_DEV_PROBE_INFO_T) * (MAX_DEV_PROBE_NUM+1);
	memset(pInfo, 0x0, sizeof(WLAN_DEV_PROBE_INFO_T) * (MAX_DEV_PROBE_NUM+1));
	ret = iw_get_ext(skfd, interface, SIOCDEVPROBERES, &wrq);
	if (ret < 0){
		close( skfd );
		return -1;
	}
	close( skfd );

	return 0;
}

#endif

