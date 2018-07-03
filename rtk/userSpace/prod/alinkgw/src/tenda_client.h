/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_clientlist.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.27
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.4.28   		1.0          learn from xuwu(ecos1.0)
************************************************************/
#include <stdio.h>
#include <stdlib.h>


#define IN
#define OUT

#define NOT_IN_USE          0
#define IN_USE              1

#define CLIENT_TBL_ADD      1
#define CLIENT_TBL_DEL      2

#define L2_TYPE_WIRED       0
#define L2_TYPE_WIRELESS    1


#define MAX_CLIENT_LIST_NUM   128


#define TPI_BUFLEN_64       64    //!< buffer length 64




#define TPI_MACLEN	6					/* MAC长度 */
#define TPI_MAC_STRING_LEN	18			/* MAC字符串长度 */
#define TPI_IP_STRING_LEN	16			/* IP字符串长度 */
#define TPI_IFNAME_LEN		16			/* 接口名称长度 */


typedef struct tpi_all_clients{
        int in_use;                                //now in use
        unsigned char mac[TPI_MAC_STRING_LEN];     //mac address
        unsigned char ip[TPI_IP_STRING_LEN];       //ip address
		unsigned char hostname[TPI_BUFLEN_64];     //hostname
		int l2type;                                //layer2 type: wired or wireless 
		
}TPI_ALL_CLIENTS, *PTPI_ALL_CLIENTS;


typedef struct tpi_lan_status{
	char ifname[TPI_IFNAME_LEN];	
	//char mac[TPI_MAC_STRING_LEN];
	char ip[TPI_IP_STRING_LEN];
	char netmask[TPI_IP_STRING_LEN];
}TPI_LAN_STATUS, *PTPI_LAN_STATUS;


/* 接口函数返回值类型 */
typedef enum tpi_ret{
	TPI_RET_OK = 0,						/* 成功 */
	TPI_RET_APP_RUNNING = 1,			/* 模块正在运行 */
	TPI_RET_APP_DEAD = 2,				/* 模块已经退出 */
	TPI_RET_NULL_POINTER = 1001,		/* 空指针错误 */
	TPI_RET_INVALID_PARAM = 1002,		/* 非法参数 */
	TPI_RET_ERROR = 0xff				/* 失败 */
}TPI_RET;



//dhcp clients
/* 绑定类型 */
#define MAX_DHCPS_CLIENTS  253

typedef enum tpi_dhcps_bound_type{
	AUTO_ALLOCATION,								/* 动态分配 */
	STATIC_BOUND									/* 静态绑定 */
}TPI_DHCPS_BOUND_TYPE;

/* 接口介质类型 */
typedef enum tpi_l2type{
	WIRED,
	WIRELESS
}TPI_L2TYPE;


struct lease_t {
	unsigned char	last;
	unsigned char	flag;
	unsigned char	mac[6];
	unsigned int 	ipaddr;
	unsigned int	expiry;
	unsigned char	hostname[64];
};


/* 客户端列表元素 */
typedef struct tpi_dhcps_client {
	unsigned char 	bound;			/*绑定类型*/
	TPI_L2TYPE l2type;						/* 介质类型 */
	char	mac[TPI_MAC_STRING_LEN];			/*MAC地址*/
	char    ipaddr[TPI_IP_STRING_LEN];			/*IP地址*/
	unsigned int	expiry;						/*租约时间*/
	unsigned char	hostname[TPI_BUFLEN_64];	/*主机名*/
}TPI_DHCPS_CLIENT, *PTPI_DHCPS_CLIENT;


/* 客户端列表 */
typedef struct tpi_dhcps_clients{
	int total;
	TPI_DHCPS_CLIENT clients[MAX_DHCPS_CLIENTS];
}TPI_DHCPS_CLIENTS, *PTPI_DHCPS_CLIENTS;





