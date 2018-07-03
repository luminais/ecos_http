/*
 * Stream Control structure defintions.
 *
 * Copyright (C) 2010, Tenda Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Tenda Corporation.
 *
 * $Id: tc.h,v 1.0 2010/08/20   Exp $
 * $author: Stanley	$
 */

#ifndef	__TC_H__
#define	__TC_H__
#if defined(__FreeBSD__) && defined(KERNEL) && !defined(_KERNEL)
#define _KERNEL

#endif

#if defined(__sgi) && (IRIX > 602)
# include <sys/ptimers.h>
#endif
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#ifndef __ECOS
# include <sys/file.h>
#endif
#if defined(__NetBSD__) && (NetBSD >= 199905) && !defined(IPFILTER_LKM) && \
	defined(_KERNEL)
# include "opt_ipfilter_log.h"
#endif
#if !defined(_KERNEL) && !defined(KERNEL)
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
#endif
#if (defined(KERNEL) || defined(_KERNEL)) && (__FreeBSD_version >= 220000) && \
	!defined(__ECOS)
# include <sys/filio.h>
# include <sys/fcntl.h>
#else
# include <sys/ioctl.h>
#endif
#ifndef __ECOS
# include <sys/fcntl.h>
#endif
#ifndef linux
# include <sys/protosw.h>
#endif
#include <sys/socket.h>
#if defined(_KERNEL) && !defined(linux) && !defined(__ECOS)
# include <sys/systm.h>
#endif
#if !defined(__SVR4) && !defined(__svr4__)
# ifndef linux
#  include <sys/mbuf.h>
# endif
#else
# include <sys/filio.h>
# include <sys/byteorder.h>
# ifdef _KERNEL
#  include <sys/dditypes.h>
# endif
# include <sys/stream.h>
# include <sys/kmem.h>
#endif
#if __FreeBSD_version >= 300000
# include <sys/queue.h>
#endif
#include <net/if.h>
#if __FreeBSD_version >= 300000
# include <net/if_var.h>
# if defined(_KERNEL) && !defined(IPFILTER_LKM) && !defined(__ECOS)
#  include "opt_ipfilter.h"
# endif
#endif
#ifdef sun
# include <net/af.h>
#endif
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#ifdef __sgi
# ifdef IFF_DRVRLOCK     /* IRIX6 */
#include <sys/hashing.h>
#include <netinet/in_var.h>
# endif
#endif

#ifdef RFC1825
# include <vpn/md5.h>
# include <vpn/ipsec.h>
extern struct ifnet vpnif;
#endif

#ifndef linux
# include <netinet/ip_var.h>
# include <netinet/tcp_fsm.h>
#endif
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "netinet/ip_compat.h"
#include <netinet/tcpip.h>
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "netinet/ip_frag.h"
#include "netinet/ip_state.h"
#include "netinet/ip_proxy.h"

#ifndef	MIN
# define	MIN(a,b)	(((a)<(b))?(a):(b))
#endif

#include "autoconf.h"

#define TC_CLIENT_NUMBER 	255
#define TC_RULE_NUMBER_MAX 20
#define CONFIG_TC_INTERVAL 105

#define CONFIG_TC_RATE_INTERVAL 1000

#ifdef __CONFIG_STREAM_STATISTIC__
#define STREAM_CLIENT_NUMBER TC_CLIENT_NUMBER
#define CONFIG_STREAM_INTERVAL 100
#define INBOUND NAT_INBOUND
#define OUTBOUND NAT_OUTBOUND
#endif


typedef struct tc_ip_rate{
	unsigned int max_down_rate;
	unsigned int real_down_rate;  //execpt down rate, it is can channage base on real date stream
	unsigned int min_down_rate;
	unsigned int max_up_rate;
	unsigned int real_up_rate;	 //execpt up rate, it is can channage base on real date stream
	unsigned int min_up_rate;
	unsigned int is_down_limit_len; //add zliang
#ifdef __CONFIG_TC_WINDOWS__
	unsigned int win_up;//上传窗口大小
	unsigned int win_down;
	unsigned int down_drop_len;
	unsigned int up_drop_len;
	unsigned int pre_down_drop_len;
	unsigned int pre_up_drop_len;
#endif// __CONFIG_TC_WINDOWS__
	unsigned int down_packet_len;
	unsigned int up_packet_len;
	char is_up_limit_flag;
	char is_down_limit_flag;
	char mac[20];//add hqw
}tc_ip_rate_t;

typedef struct tc_ip_index{
	tc_ip_rate_t *rule;
}tc_ip_index_t;

#define nt rule
#define ip_mac nt->mac
#define max_down nt->max_down_rate
#define expe_down nt->real_down_rate
#define min_down nt->min_down_rate
#define max_up nt->max_up_rate
#define expe_up nt->real_up_rate
#define min_up nt->min_up_rate
#define down_len nt->down_packet_len
#define up_len nt->up_packet_len
#ifdef __CONFIG_TC_WINDOWS__
#define win_up nt->win_up
#define win_down nt->win_down
#define d_up_len nt->up_drop_len
#define d_down_len  nt->down_drop_len
#define pre_d_up_len  nt->pre_up_drop_len
#define pre_d_down_len  nt->pre_down_drop_len
#endif //__CONFIG_TC_WINDOWS__

//add  zliang
#define down_limit_len  nt->is_down_limit_len

/*0:disable; 1:enable; 2:The IP client exist stream*/
#define up_flag nt->is_up_limit_flag			

/*0:disable; 1:enable; 2:The IP client exist stream*/
#define down_flag nt->is_down_limit_flag		

typedef struct tc_wan_total_rate{
	unsigned int total_down_rate;
	unsigned int total_up_rate;
}tc_wan_total_rate_t;

 #ifdef __CONFIG_TC_STATIC__
typedef struct tc_static{
	unsigned int down_drop_udp;
	unsigned int up_drop_udp;
	unsigned int down_drop_oth;
	unsigned int up_drop_oth;
	unsigned int down_drop_tcp;
	unsigned int up_drop_tcp;
}tc_static_t;
#endif

int stream_control(int nat_dir, int nflags, ip_t *ip/*, fr_info_t *fin*/);
int str2arglist(char *buf, int *list, char c, int max);
void tc_tmr_func(void );
void tc_set_timeout(void);


#ifdef __CONFIG_STREAM_STATISTIC__
#define MAX_BYTES_CELL_NUMBER 4294967295

typedef struct total_steam_statistic{
	unsigned long up_bytes;
	unsigned long down_bytes;
}total_stream_statistic_t;

typedef struct stream_statistic{
	unsigned int up_packets;
	unsigned int down_packets;
	unsigned int up_bytes;
	unsigned int down_bytes;
	unsigned int up_Mbytes;
	unsigned int down_Mbytes;
	unsigned int  up_byte_pers;
	unsigned int  down_byte_pers;
}stream_statistic_t;

//hqw add 2013.10.30网速控制的结构体
typedef struct stream_list{
	char hostname[64];
	char remark[128];
	in_addr_t  ip;
	char  mac[20];
	unsigned int type;
	unsigned int  up_byte_pers;
	unsigned int  down_byte_pers;
	unsigned int  set_pers;
	float		up_limit;
	float		down_limit;
	float		limit;
	int  access;
}stream_list_t;

//lq add 是否限速
struct stream_list_enable
{
	struct stream_list* node;
	struct stream_list_enable* next;
};

/*黑名单列表*/
struct mac_filter_node
{
	char mac[32];
	char remark[128];
	struct mac_filter_node* next;
};
typedef struct tc_rule{
	
	unsigned int startip;    		//起始地址
	unsigned int endip;       		//终止地址
	unsigned int minrate;		//最小速率
	unsigned int maxrate;		//最大速率
	unsigned int direction; 		//0:up上传; 1:down下载
	unsigned int enable;		//0:disable; 1:enable
	unsigned int port;       		//端口
	unsigned int protocol;		//0:TCP&UDP; 1:TCP; 2:UDP
	char mac[20];
}*tc_rule_p;

typedef struct statistic_list_ip_index{
	stream_list_t *index;
}statistic_list_ip_index_t;


typedef struct qosList{
	stream_list_t qosInfo;
	struct qosList *next;
}qos_list_t;


extern statistic_list_ip_index_t stream__ip_iist[STREAM_CLIENT_NUMBER];
//end of hqw

typedef struct statistic_ip_index{
	stream_statistic_t *index;
}statistic_ip_index_t;
#define MAX_CLIENT_NUMBER 	255
#define TIMES 1
#ifdef __CONFIG_GUEST__
#define ip_ubs(type,X) ((type) ? ((stream_ip_per[X][0])/TIMES) : ((guest_stream_ip_per[X][0])/TIMES))
#define ip_dbs(type,X) ((type) ? ((stream_ip_per[X][1])/TIMES) : ((guest_stream_ip_per[X][1])/TIMES))
#define ip_upa(type,X) ((type) ? (stream_ip[X].index->up_packets) : (guest_stream_ip[X].index->up_packets)) 
#define ip_dpa(type,X) ((type) ? (stream_ip[X].index->down_packets) : (guest_stream_ip[X].index->down_packets)) 
#define ip_uby(type,X) ((type) ? (stream_ip[X].index->up_bytes) : (guest_stream_ip[X].index->up_bytes)) 
#define ip_dby(type,X) ((type) ? (stream_ip[X].index->down_bytes) : (guest_stream_ip[X].index->down_bytes)) 
#define ip_uM(type,X)  ((type) ? (stream_ip[X].index->up_Mbytes) : (guest_stream_ip[X].index->up_Mbytes)) 
#define ip_dM(type,X)  ((type) ? (stream_ip[X].index->down_Mbytes) : (guest_stream_ip[X].index->down_Mbytes)) 
#else
#define ip_ubs(type,X) ((stream_ip_per[X][0])/TIMES)
#define ip_dbs(type,X) ((stream_ip_per[X][1])/TIMES)
#define ip_upa(type,X) (stream_ip[X].index->up_packets)
#define ip_dpa(type,X) (stream_ip[X].index->down_packets)
#define ip_uby(type,X) (stream_ip[X].index->up_bytes)
#define ip_dby(type,X) (stream_ip[X].index->down_bytes)
#define ip_uM(type,X)   (stream_ip[X].index->up_Mbytes)
#define ip_dM(type,X)   (stream_ip[X].index->down_Mbytes)
#endif
#endif
#endif	/* __TC_H__ */
