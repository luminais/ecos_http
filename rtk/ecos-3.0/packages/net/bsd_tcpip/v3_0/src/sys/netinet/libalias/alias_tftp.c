/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : TFTP ALG PROCESS
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/

#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
//#include <netinet/fastpath/rtl_queue.h>
#include <rtl/rtl_queue.h>
#include <sys/param.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "alias_local.h"
//#include <netinet/fastpath/rtl_alias.h>
#include <rtl/rtl_alias.h>

typedef struct udp_hdr udphdr;

struct tftphdr {
	unsigned short opcode;
};

struct tftp_alg_entry{
	unsigned long			intIp;
	unsigned long			extIp;
	unsigned long			remIp;
	unsigned short		intPort;
	unsigned short 		extPort;
	unsigned short		valid;
	unsigned long 		last_used;	
};

#define TFTP_OPCODE_READ	1
#define TFTP_OPCODE_WRITE	2
#define TFTP_OPCODE_DATA	3
#define TFTP_OPCODE_ACK	4
#define TFTP_OPCODE_ERROR	5

#define  RTL_TFTP_ALG_ENTRY_EXPIER		60
#define  RTL_TFTP_ALG_ENTRY_CHECK		5*hz
#define  RTL_TFTP_ALG_ENTRY_MAX			8

int rtl_tftp_alg_support_enable = 1;
struct tftp_alg_entry tftpAlg[RTL_TFTP_ALG_ENTRY_MAX];

struct callout rtl_tftp_alg_timer;

void rtl_tftpAlgTimerHandle(void)
{
	int i;
	unsigned long now = (unsigned long)jiffies;

	for(i=0; i<RTL_TFTP_ALG_ENTRY_MAX; i++)
	{
		if((tftpAlg[i].valid==1) && ((tftpAlg[i].last_used+RTL_TFTP_ALG_ENTRY_EXPIER*HZ) < now)){
			memset(&tftpAlg[i], 0, sizeof(struct tftp_alg_entry));
		}
	}

	callout_reset(&rtl_tftp_alg_timer, RTL_TFTP_ALG_ENTRY_CHECK, rtl_tftpAlgTimerHandle, 0);
}

int rtl_aliasHandleTftpOut(struct ip* iph, struct alias_link *link)
{
	int i;
	struct tftphdr *tfh = NULL;
	udphdr *udph = NULL;
	struct tftp_alg_entry *entry = NULL;

    #ifdef RTL_NF_ALG_CTL
	ALG_CHECK_ONOFF(alg_type_tftp);
	#endif

	udph = (void *) iph + iph->ip_hl*4;
	tfh = (void*)udph + sizeof(udphdr);
	if(tfh==NULL)
		return -1;

	if((ntohs(tfh->opcode)!=TFTP_OPCODE_READ)&&
	    (ntohs(tfh->opcode)!=TFTP_OPCODE_WRITE))
		return -1;
	
	for(i=0; i<RTL_TFTP_ALG_ENTRY_MAX; i++)
	{
		if(tftpAlg[i].valid == 0){
			entry = &tftpAlg[i];
			break;
		}
	}

	if(entry==NULL)
		return -1;

	entry->intIp 		= link->src_addr.s_addr;
	entry->intPort		= link->src_port;
	entry->extIp 		= link->alias_addr.s_addr;
	entry->extPort	= link->alias_port;
	entry->remIp 		= link->dst_addr.s_addr;
	entry->last_used	= (unsigned long)jiffies;
	entry->valid 		= 1;

	return 0;
}

int rtl_aliasHandleTftpIn(unsigned long *intIp, unsigned short *intPort, 
								      unsigned long extIp, unsigned short extPort,
								      unsigned long remIp)
{
	int i;

    #ifdef RTL_NF_ALG_CTL
	ALG_CHECK_ONOFF(alg_type_tftp);
	#endif
    
	for(i=0; i<RTL_TFTP_ALG_ENTRY_MAX; i++)
	{
		if((tftpAlg[i].valid==1) && (tftpAlg[i].extIp==extIp) &&
		   (tftpAlg[i].extPort==extPort) && (tftpAlg[i].remIp==remIp)){
		    	*intIp 	   = tftpAlg[i].intIp;
			*intPort 	   = tftpAlg[i].intPort;
			return 0;
		}
	}
	
	return -1;
}

void rtl_showTftpAlg(void)
{
	int i;

	for(i=0; i<RTL_TFTP_ALG_ENTRY_MAX; i++)
	{		
		if(tftpAlg[i].valid){
			diag_printf("tftpAlg[%d]: int=0x%08X:%-5u ext=0x%08X:%-5u remIp=0x%08X\n", i, 
				tftpAlg[i].intIp, tftpAlg[i].intPort, tftpAlg[i].extIp, tftpAlg[i].extPort, tftpAlg[i].remIp);
		}
	}
}

void rtl_initTftpAlg(void)
{
	memset(tftpAlg, 0, sizeof(struct tftp_alg_entry)*RTL_TFTP_ALG_ENTRY_MAX);
	/*stoy rtl_tftp_alg_timer  anyway*/
	callout_stop(&rtl_tftp_alg_timer);
	callout_init(&rtl_tftp_alg_timer);
	callout_reset(&rtl_tftp_alg_timer, RTL_TFTP_ALG_ENTRY_CHECK, rtl_tftpAlgTimerHandle, 0);
}

void rtl_UninitTftpAlg(void)
{
	callout_stop(&rtl_tftp_alg_timer);
}