/* -*- mode: c; tab-width: 3; c-basic-offset: 3; -*- */

/*-
 * Copyright (c) 2001 Charles Mott <cmott@scientech.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/*
 * Alias_local.h contains the function prototypes for alias.c,
 * alias_db.c, alias_util.c and alias_ftp.c, alias_irc.c (as well
 * as any future add-ons).  It also includes macros, globals and
 * struct definitions shared by more than one alias*.c file.
 *
 * This include file is intended to be used only within the aliasing
 * software.  Outside world interfaces are defined in alias.h
 *
 * This software is placed into the public domain with no restrictions
 * on its distribution.
 *
 * Initial version:  August, 1996  (cjm)    
 *
 * <updated several times by original author and Eivind Eklund>
 */

#ifndef _ALIAS_LOCAL_H_
#define	_ALIAS_LOCAL_H_
#define KLD_ENABLED

#ifndef NULL
#define NULL 0
#endif

/* Macros */

/*
 * The following macro is used to update an
 * internet checksum.  "delta" is a 32-bit
 * accumulation of all the changes to the
 * checksum (adding in new 16-bit words and
 * subtracting out old words), and "cksum"
 * is the checksum value to be updated.
 */
#define	ADJUST_CHECKSUM(acc, cksum) \
	do { \
		acc += cksum; \
		if (acc < 0) { \
			acc = -acc; \
			acc = (acc >> 16) + (acc & 0xffff); \
			acc += acc >> 16; \
			cksum = (u_short) ~acc; \
		} else { \
			acc = (acc >> 16) + (acc & 0xffff); \
			acc += acc >> 16; \
			cksum = (u_short) acc; \
		} \
	} while (0)

/* Globals */

extern int packetAliasMode;


/* Structs */

struct alias_link;    /* Incomplete structure */


/* Prototypes */

/* General utilities */
u_short IpChecksum(struct ip *);
u_short TcpChecksum(struct ip *);
void DifferentialChecksum(u_short *, u_short *, u_short *, int);

/* Internal data access */
struct alias_link *
FindIcmpIn(struct in_addr, struct in_addr, u_short, int);

struct alias_link *
FindIcmpOut(struct in_addr, struct in_addr, u_short, int);

struct alias_link *
FindFragmentIn1(struct in_addr, struct in_addr, u_short);

struct alias_link *
FindFragmentIn2(struct in_addr, struct in_addr, u_short);

struct alias_link *
AddFragmentPtrLink(struct in_addr, u_short);

struct alias_link *
FindFragmentPtr(struct in_addr, u_short);

struct alias_link *
FindProtoIn(struct in_addr, struct in_addr, u_char);

struct alias_link *
FindProtoOut(struct in_addr, struct in_addr, u_char);

struct alias_link *
FindUdpTcpIn (struct in_addr, struct in_addr, u_short, u_short, u_char, int);

struct alias_link *
FindUdpTcpOut(struct in_addr, struct in_addr, u_short, u_short, u_char, int);

struct alias_link *
AddPptp(struct in_addr, struct in_addr, struct in_addr, u_int16_t);

struct alias_link *
FindPptpOutByCallId(struct in_addr, struct in_addr, u_int16_t);

struct alias_link *
FindPptpInByCallId(struct in_addr, struct in_addr, u_int16_t);

struct alias_link *
FindPptpOutByPeerCallId(struct in_addr, struct in_addr, u_int16_t);

struct alias_link *
FindPptpInByPeerCallId(struct in_addr, struct in_addr, u_int16_t);

struct alias_link *
FindRtspOut(struct in_addr, struct in_addr, u_short, u_short, u_char);

struct in_addr
FindOriginalAddress(struct in_addr);

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT) ||defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
struct in_addr
FindOriginalAddressByAliasPort(struct in_addr alias_addr, unsigned short alias_port, unsigned short protocol
#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)&&defined(KLD_ENABLED)
													, unsigned short *int_port
#endif
			 										);
#endif

struct in_addr
FindAliasAddress(struct in_addr);

/* External data access/modification */
int FindNewPortGroup(struct in_addr, struct in_addr,
                     u_short, u_short, u_short, u_char, u_char);
void GetFragmentAddr(struct alias_link *, struct in_addr *);
void SetFragmentAddr(struct alias_link *, struct in_addr);
void GetFragmentPtr(struct alias_link *, char **);
void SetFragmentPtr(struct alias_link *, char *);
void SetStateIn(struct alias_link *, int);
void SetStateOut(struct alias_link *, int);
int GetStateIn(struct alias_link *);
int GetStateOut(struct alias_link *);
struct in_addr GetOriginalAddress(struct alias_link *);
struct in_addr GetDestAddress(struct alias_link *);
struct in_addr GetAliasAddress(struct alias_link *);
struct in_addr GetDefaultAliasAddress(void);
void SetDefaultAliasAddress(struct in_addr);
u_short GetOriginalPort(struct alias_link *);
u_short GetAliasPort(struct alias_link *);
u_short GetDestPort(struct alias_link *link);
struct in_addr GetProxyAddress(struct alias_link *);
void SetProxyAddress(struct alias_link *, struct in_addr);
u_short GetProxyPort(struct alias_link *);
void SetProxyPort(struct alias_link *, u_short);
void SetAckModified(struct alias_link *);
int GetAckModified(struct alias_link *);
int GetDeltaAckIn(struct ip *, struct alias_link *);
int GetDeltaSeqOut(struct ip *, struct alias_link *);
void AddSeq(struct ip *, struct alias_link *, int);
void SetExpire(struct alias_link *, int);
void ClearCheckNewLink(void);
void SetLastLineCrlfTermed(struct alias_link *, int);
int GetLastLineCrlfTermed(struct alias_link *);
#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)	//jwj20120716: add dir and assured bit
void SetDestCallId(struct alias_link *, u_int16_t, u_int8_t);
#else
void SetDestCallId(struct alias_link *, u_int16_t);
#endif
#ifndef NO_FW_PUNCH
void PunchFWHole(struct alias_link *);
#endif


/* Housekeeping function */
void HouseKeeping(void);

/* Tcp specfic routines */
/*lint -save -library Suppress flexelint warnings */

/* FTP routines */
void AliasHandleFtpOut(struct ip *, struct alias_link *, int);

/* IRC routines */
void AliasHandleIrcOut(struct ip *, struct alias_link *, int);

/* RTSP routines */
void AliasHandleRtspOut(struct ip *, struct alias_link *, int);

/* PPTP routines */
void AliasHandlePptpOut(struct ip *, struct alias_link *);
void AliasHandlePptpIn(struct ip *, struct alias_link *);
int AliasHandlePptpGreOut(struct ip *);
int AliasHandlePptpGreIn(struct ip *);

/* NetBIOS routines */
int AliasHandleUdpNbt(struct ip *, struct alias_link *, struct in_addr *, u_short);
int AliasHandleUdpNbtNS(struct ip *, struct alias_link *, struct in_addr *, u_short *, struct in_addr *, u_short *);

/* CUSeeMe routines */
void AliasHandleCUSeeMeOut(struct ip *, struct alias_link *);
void AliasHandleCUSeeMeIn(struct ip *, struct in_addr);

/* Transparent proxy routines */
int ProxyCheck(struct ip *, struct in_addr *, u_short *);
void ProxyModify(struct alias_link *, struct ip *, int, int);

/* sip routines */
int AliasHandleSip(struct ip *pip, struct alias_link *link);


void rtl_printAliaslink(void);

enum alias_tcp_state {
	ALIAS_TCP_STATE_NOT_CONNECTED,
	ALIAS_TCP_STATE_CONNECTED,
	ALIAS_TCP_STATE_DISCONNECTED
};

/*lint -restore */

//#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)	//jwj20120716: add dir and assured bit
#define INBOUND_DIR		1
#define OUTBOUND_DIR		2
#define  UNKNOWN_DIR		3


#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif
#define NETBIOS_NS_PORT_NUMBER 137
#define NETBIOS_DGM_PORT_NUMBER 138
#define FTP_CONTROL_PORT_NUMBER 21
#define IRC_CONTROL_PORT_NUMBER_1 6667
#define IRC_CONTROL_PORT_NUMBER_2 6668
#define CUSEEME_PORT_NUMBER 7648
#define RTSP_CONTROL_PORT_NUMBER_1 554
#define RTSP_CONTROL_PORT_NUMBER_2 7070
#define PPTP_CONTROL_PORT_NUMBER 1723
#define L2TP_CONTROL_PORT_NUMBER	1701
#define TFTP_CONTROL_PORT_NUMBER 69
#define SIP_CONTROL_PORT_NUMBER 5060
#define DNS_PORT_NUMBER         53
#define H323_Q931_CONTROL_PORT_NUMBER 1720
#define H323_RAS_CONTROL_PORT_NUMBER 1719

#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
extern int rtl_fpTimer_update(struct alias_link *link);
extern void rtl_fpAddConnCache(struct alias_link *link, struct ip *iph);
extern void rtl_delConnCache(struct alias_link *link);
extern int rtl_linkAssuredCheck(struct alias_link *link);
extern int rtl_skipFashpathCheck(u_short sport, u_short dport);
extern int get_fast_fw();
#endif
#if defined(CONFIG_RTL_HARDWARE_NAT)
extern int rtl_addHwNatEntry(struct alias_link *link, struct ip *iph);
extern int rtl_delHwNatEntry(struct alias_link *link);
extern int rtl_hwNatTimer_update(struct alias_link *link);
#endif
#if defined(CONFIG_RTL_CONE_NAT_SUPPORT)
extern int rtl_cone_nat_mode;
typedef enum{
	SYMMETRIC_NAT=0,
	FULL_CONE__NAT=1,
	RESTRICTED_CONE_NAT=2,
	PORT_RESTRICTED_CONE_NAT=3,
}NAT_MODE;

typedef struct _rtl_cone_nat_entry_t
{
	unsigned short		course;		/* 1:Out-Bonud 2:In-Bound */	
	unsigned short 		protocol;
	unsigned long			intIp;
	unsigned long			extIp;
	unsigned long			remIp;
	unsigned short		intPort;
	unsigned short 		extPort;
	unsigned short 		remPort;
	unsigned short		valid;
	unsigned long 		last_used;
}rtl_cone_nat_entry_t;
int rtl_addRtlConeNatEntry(struct alias_link *link);
int rtl_getIntIpPortFromConeNatEntry(rtl_cone_nat_entry_t *nat_entry, unsigned long *ip, unsigned short *port);
int rtl_getAliasPortForPortRestrConeNat(unsigned long intIp, unsigned short intPort);
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
extern int rtl_port_forwarding_enabled;
extern unsigned long rtl_getIntIpByPortFromPortFwdEntry(unsigned short port, unsigned short protocol);
#if defined(KLD_ENABLED)
extern int rtl_getIntIpAndPortFromPortFwdEntry(unsigned short mPort, unsigned short protocol, unsigned short *intPort, unsigned long *intIp);
#endif
#endif

#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
extern int rtl_trigger_port_enabled;
int rtl_triggerOut(struct alias_link *link);
unsigned int rtl_triggerDnat(unsigned short dport, unsigned short proto);
#endif

extern void rtl_initL2tpAlg(void);
extern int rtl_tftp_alg_support_enable;
void rtl_initTftpAlg(void);
int rtl_aliasHandleTftpIn(unsigned long *intIp, unsigned short *intPort, 
								      unsigned long extIp, unsigned short extPort,
								      unsigned long remIp);
int rtl_aliasHandleTftpOut(struct ip* iph, struct alias_link *link);
#if defined(CONFIG_RTL_819X)
void rtl_setLinkFragTotalLen(struct alias_link *link, struct ip* pip);
void rtl_incLinkFragSumLen(struct alias_link *link, struct ip* pip);
void rtl_deleteLink(struct alias_link *link);
#endif
#ifdef  ECOS_DBG_STAT
extern int dbg_napt_index;
/*DEBUG type */
#define DBG_TYPE_MAX		5
#define	DBG_TYPE_MALLOC		0
#define DBG_TYPE_FILE		1
#define DBG_TYPE_SOCKET		2
#define DBG_TYPE_MBUF		3
#define DBG_TYPE_SKB		4

/*action type*/
#define	DBG_ACTION_ADD		0
#define DBG_ACTION_DEL		1
#endif

#define RTL_NF_ALG_CTL 1
#ifdef RTL_NF_ALG_CTL
extern int alg_enable(int type);
enum alg_type
{
    alg_type_ftp,
    alg_type_tftp,
    alg_type_rtsp,
    alg_type_pptp,
    alg_type_l2tp,
    //alg_type_ipsec,
    alg_type_sip,
    //alg_type_h323,
    alg_type_end
};

struct alg_entry
{
    char *name;
    int enable;
};

#define ALG_CTL_DEF(type, val)  [alg_type_##type] = {#type, val}
#define ALG_CHECK_ONOFF(type)   \
if (!alg_enable(type))          \
{                               \
    return 0;                   \
}
#endif

#endif /* !_ALIAS_LOCAL_H_ */
