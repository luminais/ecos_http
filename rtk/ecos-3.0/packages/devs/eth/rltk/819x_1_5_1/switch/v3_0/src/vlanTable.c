/*
 * Abstract: Switch core vlan table access driver source code.
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *		All rights reserved.
 *
 */
 
//#include <asm/rtl8196.h>

#ifdef __KERNEL__
#include <linux/string.h>
#elif defined(__ECOS)
#include <pkgconf/system.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#endif

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "swCore.h"
#include "vlanTable.h"
#include "swTable.h"

#include "rtl865xC_tblAsicDrv.h"

#include <switch/rtl865x_netif.h>

extern void tableAccessForeword(uint32, uint32, void *);
extern int32 swTable_addEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
extern int32 swTable_modifyEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
extern int32 swTable_forceAddEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
extern int32 swTable_readEntry(uint32 tableType, uint32 eidx, void *entryContent_P);

#if 0
#include <asm/mipsregs.h>

int lx4180_ReadStatus(void)
{
   volatile unsigned int reg;
	reg= read_32bit_cp0_register(CP0_STATUS);
	__asm__ volatile("nop");	// david
   	__asm__ volatile("nop");   
	return reg;

}
void lx4180_WriteStatus(int s)
{
   volatile unsigned int reg=s;
	write_32bit_cp0_register(CP0_STATUS, reg);
	__asm__ volatile("nop");	// david
   	__asm__ volatile("nop");   
	return ;

}
#endif


/* STATIC VARIABLE DECLARATIONS
 */



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
#if 1//!defined(__ECOS) || defined(CONFIG_RTL_VLAN_SUPPORT)||defined (CONFIG_RTL_MLD_SNOOPING) 
int32 swCore_aclCreate(uint32 idx, rtl_acl_param_t * rule)
{
	acl_table_t    entry;

	rule->aclIdx = idx;
	memset(&entry, 0, sizeof(entry));

	switch(rule->ruleType_) {
	case RTL8651_ACL_MAC: /* Etnernet type rule: 0x0000 */
		 entry.is.ETHERNET.dMacP47_32 = rule->dstMac_.octet[0]<<8 	| rule->dstMac_.octet[1];
		 entry.is.ETHERNET.dMacP31_16 = rule->dstMac_.octet[2]<<8 	| rule->dstMac_.octet[3];
		 entry.is.ETHERNET.dMacP15_0   = rule->dstMac_.octet[4]<<8 	| rule->dstMac_.octet[5];
	 	 entry.is.ETHERNET.dMacM47_32 = rule->dstMacMask_.octet[0]<<8 | rule->dstMacMask_.octet[1];
		 entry.is.ETHERNET.dMacM31_16 = rule->dstMacMask_.octet[2]<<8 | rule->dstMacMask_.octet[3];
		 entry.is.ETHERNET.dMacM15_0   = rule->dstMacMask_.octet[4]<<8 | rule->dstMacMask_.octet[5];
		 entry.is.ETHERNET.sMacP47_32  = rule->srcMac_.octet[0]<<8 	| rule->srcMac_.octet[1];
		 entry.is.ETHERNET.sMacP31_16  = rule->srcMac_.octet[2]<<8 	| rule->srcMac_.octet[3];
		 entry.is.ETHERNET.sMacP15_0    = rule->srcMac_.octet[4]<<8 	| rule->srcMac_.octet[5];
		 entry.is.ETHERNET.sMacM47_32  = rule->srcMacMask_.octet[0]<<8 | rule->srcMacMask_.octet[1];
		 entry.is.ETHERNET.sMacM31_16  = rule->srcMacMask_.octet[2]<<8 | rule->srcMacMask_.octet[3];
		 entry.is.ETHERNET.sMacM15_0    = rule->srcMacMask_.octet[4]<<8 | rule->srcMacMask_.octet[5];
		 entry.is.ETHERNET.ethTypeP       = rule->typeLen_;
		 entry.is.ETHERNET.ethTypeM       = rule->typeLenMask_;
		 entry.ruleType = 0x0;
		 #if defined(RTL_REFINE_ACL)
		 entry.ruleType = rule->ruleType_;
		 #endif
		 break;
		 
	case RTL8651_ACL_IP: /* IP Rule Type: 0x0010 */
	case RTL8652_ACL_IP_RANGE:
		 entry.is.L3L4.is.IP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IP.IPProtoP = rule->ipProto_;
		 entry.is.L3L4.is.IP.IPProtoM = rule->ipProtoMask_;
		 entry.is.L3L4.is.IP.IPFlagP = rule->ipFlag_;
		 entry.is.L3L4.is.IP.IPFlagM = rule->ipFlagMask_;
 		 entry.is.L3L4.is.IP.FOP = rule->ipFOP_;
		 entry.is.L3L4.is.IP.FOM = rule->ipFOM_;
		 entry.is.L3L4.is.IP.HTTPP = entry.is.L3L4.is.IP.HTTPM = rule->ipHttpFilter_;
		 entry.is.L3L4.is.IP.identSDIPP = entry.is.L3L4.is.IP.identSDIPM = rule->ipIdentSrcDstIp_;
		 if (rule->ruleType_==RTL8651_ACL_IP)
			 entry.ruleType = 0x2;
		else
			 entry.ruleType = 0xa;
		//diag_printf("entry.ruleType:%x,rule->ipProto_:%x,rule->ipProtoMask_:%x[%s]:[%d].\n",entry.ruleType,rule->ipProto_,rule->ipProtoMask_,__FUNCTION__,__LINE__);	
		 goto l3l4_shared;
	
	case RTL8651_ACL_ICMP:
	case RTL8652_ACL_ICMP_IPRANGE:
		 entry.is.L3L4.is.ICMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.ICMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.ICMP.ICMPTypeP = rule->icmpType_;
		 entry.is.L3L4.is.ICMP.ICMPTypeM = rule->icmpTypeMask_;
		 entry.is.L3L4.is.ICMP.ICMPCodeP = rule->icmpCode_;
		 entry.is.L3L4.is.ICMP.ICMPCodeM = rule->icmpCodeMask_;
 		 if (rule->ruleType_==RTL8651_ACL_ICMP)
			 entry.ruleType = 0x4;
		 else
		 	entry.ruleType=0xc;
		 goto l3l4_shared;
	
	case RTL8651_ACL_IGMP:
	case RTL8652_ACL_IGMP_IPRANGE:
		 entry.is.L3L4.is.IGMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IGMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IGMP.IGMPTypeP = rule->igmpType_;
		 entry.is.L3L4.is.IGMP.IGMPTypeM = rule->igmpTypeMask_; 
  		 if (rule->ruleType_==RTL8651_ACL_IGMP)
			 entry.ruleType = 0x5;
   		 else
			entry.ruleType=0xd;
 		 goto l3l4_shared;
		 
	case RTL8651_ACL_TCP:
	case RTL8652_ACL_TCP_IPRANGE:
		 entry.is.L3L4.is.TCP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.TCP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.TCP.TCPFlagP = rule->tcpFlag_;
		 entry.is.L3L4.is.TCP.TCPFlagM = rule->tcpFlagMask_;
		 entry.is.L3L4.is.TCP.TCPSPUB = rule->tcpSrcPortUB_;
		 entry.is.L3L4.is.TCP.TCPSPLB = rule->tcpSrcPortLB_;
		 entry.is.L3L4.is.TCP.TCPDPUB = rule->tcpDstPortUB_;
		 entry.is.L3L4.is.TCP.TCPDPLB = rule->tcpDstPortLB_;
 		 if (rule->ruleType_==RTL8651_ACL_TCP)
			 entry.ruleType = 0x6;
 		else
			entry.ruleType=0xe;
         goto l3l4_shared;

	case RTL8651_ACL_UDP:
	case RTL8652_ACL_UDP_IPRANGE:
		 entry.is.L3L4.is.UDP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.UDP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.UDP.UDPSPUB = rule->udpSrcPortUB_;
		 entry.is.L3L4.is.UDP.UDPSPLB = rule->udpSrcPortLB_;
		 entry.is.L3L4.is.UDP.UDPDPUB = rule->udpDstPortUB_;
		 entry.is.L3L4.is.UDP.UDPDPLB = rule->udpDstPortLB_;
  		 if (rule->ruleType_==RTL8651_ACL_UDP)
			 entry.ruleType = 0x7;
		 else
		 	entry.ruleType=0xf;

l3l4_shared:
		 #if defined(RTL_REFINE_ACL)
		 entry.ruleType = rule->ruleType_;
		 #endif
		 entry.is.L3L4.sIPP = rule->srcIpAddr_;
		 entry.is.L3L4.sIPM = rule->srcIpAddrMask_;
		 entry.is.L3L4.dIPP = rule->dstIpAddr_;
		 entry.is.L3L4.dIPM = rule->dstIpAddrMask_;
		 //diag_printf("rule->dstIpAddr_:%x,rule->dstIpAddrMask_:%x\n",rule->dstIpAddr_,rule->dstIpAddrMask_);
		 break;
		 
#if defined(RTL_REFINE_ACL)		 
	 case RTL865X_ACL_SRCFILTER:	 /* TCP Rule Type: 0x0008 */
	 case RTL865X_ACL_SRCFILTER_IPRANGE:			 /* 0x000B */
		  rule->srcFilterMac_.octet[0] = rule->srcFilterMac_.octet[0] & rule->srcFilterMacMask_.octet[0];
		  rule->srcFilterMac_.octet[1] = rule->srcFilterMac_.octet[1] & rule->srcFilterMacMask_.octet[1];
		  rule->srcFilterMac_.octet[2] = rule->srcFilterMac_.octet[2] & rule->srcFilterMacMask_.octet[2];
		  rule->srcFilterMac_.octet[3] = rule->srcFilterMac_.octet[3] & rule->srcFilterMacMask_.octet[3];
		  rule->srcFilterMac_.octet[4] = rule->srcFilterMac_.octet[4] & rule->srcFilterMacMask_.octet[4];
		  rule->srcFilterMac_.octet[5] = rule->srcFilterMac_.octet[5] & rule->srcFilterMacMask_.octet[5];

		  entry.is.SRC_FILTER.sMacP47_32 = rule->srcFilterMac_.octet[0]<<8 | rule->srcFilterMac_.octet[1];
		  entry.is.SRC_FILTER.sMacP31_16 = rule->srcFilterMac_.octet[2]<<8 | rule->srcFilterMac_.octet[3];
		  entry.is.SRC_FILTER.sMacP15_0 = rule->srcFilterMac_.octet[4]<<8 | rule->srcFilterMac_.octet[5];
		  entry.is.SRC_FILTER.sMacM3_0 =rule->srcFilterMacMask_.octet[5] &0xf;

		  rule->srcFilterVlanId_ = rule->srcFilterVlanId_ & rule->srcFilterVlanIdMask_;
		  entry.is.SRC_FILTER.spaP = rule->srcFilterPort_;
		  entry.is.SRC_FILTER.sVidP = rule->srcFilterVlanId_;
		  entry.is.SRC_FILTER.sVidM = rule->srcFilterVlanIdMask_;
		  if(rule->srcFilterIgnoreL3L4_)
			 entry.is.SRC_FILTER.protoType = 1;
		  else if(rule->srcFilterIgnoreL4_)
			 entry.is.SRC_FILTER.protoType = 2;
		  else
			 entry.is.SRC_FILTER.protoType = 0;

		  entry.is.SRC_FILTER.sIPP = rule->srcFilterIpAddr_;
		  entry.is.SRC_FILTER.sIPM = rule->srcFilterIpAddrMask_;
		  entry.is.SRC_FILTER.SPORTUB = rule->srcFilterPortUpperBound_;
		  entry.is.SRC_FILTER.SPORTLB = rule->srcFilterPortLowerBound_;

		  entry.ruleType = rule->ruleType_;
		  break;

	 case RTL865X_ACL_DSTFILTER:	 /* TCP Rule Type: 0x0009 */
	 case RTL865X_ACL_DSTFILTER_IPRANGE:			 /* 0x0001 */
		  entry.is.DST_FILTER.dMacP47_32 = rule->dstFilterMac_.octet[0]<<8 | rule->dstFilterMac_.octet[1];
		  entry.is.DST_FILTER.dMacP31_16 = rule->dstFilterMac_.octet[2]<<8 | rule->dstFilterMac_.octet[3];
		  entry.is.DST_FILTER.dMacP15_0 = rule->dstFilterMac_.octet[4]<<8 | rule->dstFilterMac_.octet[5];
		  entry.is.DST_FILTER.dMacM3_0 =  rule->dstFilterMacMask_.octet[5]&0xf;
		  entry.is.DST_FILTER.vidP = rule->dstFilterVlanIdx_;
		  entry.is.DST_FILTER.vidM = rule->dstFilterVlanIdxMask_;
		  if(rule->dstFilterIgnoreL3L4_)
			 entry.is.DST_FILTER.protoType = 1;
		  else if(rule->dstFilterIgnoreL4_)
			 entry.is.DST_FILTER.protoType = 2;
		  else
			 entry.is.DST_FILTER.protoType = 0;
		  entry.is.DST_FILTER.dIPP = rule->dstFilterIpAddr_;
		  entry.is.DST_FILTER.dIPM = rule->dstFilterIpAddrMask_;
		  entry.is.DST_FILTER.DPORTUB = rule->dstFilterPortUpperBound_;
		  entry.is.DST_FILTER.DPORTLB = rule->dstFilterPortLowerBound_;

			  entry.ruleType = rule->ruleType_;
		  break;


#if defined(CONFIG_RTL_8197F)
	 case RTL865X_ACL_IPV6: 		 /* IP Rule Type: 0x0012 */
	 case RTL865X_ACL_IPV6_RANGE:					 /* 0x001A */
		 entry.ruleType = rule->ruleType_;
		 entry.ruleType1 = rule->ruleType_>>4;
		 if(rule->ipv6EntryType_==1) //first entry
		 {
			 entry.is.L3V6.is.entry0.sip_addr31_0	= rule->srcIpV6Addr_.v6_addr32[3];
			 entry.is.L3V6.is.entry0.sip_addr63_32	= rule->srcIpV6Addr_.v6_addr32[2];
			 entry.is.L3V6.is.entry0.sip_addr95_64	= rule->srcIpV6Addr_.v6_addr32[1];
			 entry.is.L3V6.is.entry0.sip_addr127_96 = rule->srcIpV6Addr_.v6_addr32[0];
			 
			 entry.is.L3V6.is.entry0.sip_mask31_0	= rule->srcIpV6AddrMask_.v6_addr32[3];
			 entry.is.L3V6.is.entry0.sip_mask63_32	= rule->srcIpV6AddrMask_.v6_addr32[2];
			 entry.is.L3V6.is.entry0.sip_mask95_64	= rule->srcIpV6AddrMask_.v6_addr32[1];

			 entry.is_d.ipv6.is.entry0.sip_mask119_96	 = rule->srcIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
			 entry.is_d.ipv6.is.entry0.sip_mask127_120	 = rule->srcIpV6AddrMask_.v6_addr32[0]>>24;
			 entry.is_d.ipv6.is.entry0.flowLabel		 = rule->ipv6FlowLabel_;
			 entry.is_d.ipv6.is.entry0.flowLabelM3_0	 = rule->ipv6FlowLabelM_&0xF;
			 entry.is_d.ipv6.is.entry0.flowLabelM19_4	 = rule->ipv6FlowLabelM_>>4;	  
		 }
		 else
		 {
			 entry.is.L3V6.is.entry1.dip_addr31_0	= rule->dstIpV6Addr_.v6_addr32[3];
			 entry.is.L3V6.is.entry1.dip_addr63_32	= rule->dstIpV6Addr_.v6_addr32[2];
			 entry.is.L3V6.is.entry1.dip_addr95_64	= rule->dstIpV6Addr_.v6_addr32[1];
			 entry.is.L3V6.is.entry1.dip_addr127_96 = rule->dstIpV6Addr_.v6_addr32[0];
			 
			 entry.is.L3V6.is.entry1.dip_mask31_0	= rule->dstIpV6AddrMask_.v6_addr32[3];
			 entry.is.L3V6.is.entry1.dip_mask63_32	= rule->dstIpV6AddrMask_.v6_addr32[2];
			 entry.is.L3V6.is.entry1.dip_mask95_64	= rule->dstIpV6AddrMask_.v6_addr32[1];
			 
			 entry.is_d.ipv6.is.entry1.dip_mask119_96	 = rule->dstIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
			 entry.is_d.ipv6.is.entry1.dip_mask127_120	 = rule->dstIpV6AddrMask_.v6_addr32[0]>>24;
			 entry.is_d.ipv6.is.entry1.trafficClass 	 = rule->ipv6TrafficClass_; 
			 entry.is_d.ipv6.is.entry1.trafficClassM	 = rule->ipv6TrafficClassM_; 
			 entry.is_d.ipv6.is.entry1.nextHeader		 = rule->ipv6NextHeader_; 

			 entry.is_d.ipv6.is.entry1.nextHeaderM		 = rule->ipv6NextHeaderM_; 
			 entry.is_d.ipv6.is.entry1.HTTPP	   = entry.is_d.ipv6.is.entry1.HTTPM = rule->ipv6HttpFilter_;
			 entry.is_d.ipv6.is.entry1.identSDIPP  = entry.is_d.ipv6.is.entry1.identSDIPM  = rule->ipv6IdentSrcDstIp_; 
		 }

		 entry.inv		 = rule->ipv6Invert_;
		 entry.ipv6ETY0  = rule->ipv6EntryType_;
		 entry.comb 	 = rule->ipv6Combine_;
		 entry.ip_tunnel = rule->ipv6IPtunnel_;
		 break;
#elif defined(CONFIG_RTL_8198C)
	 case RTL865X_ACL_IPV6: 		 /* IP Rule Type: 0x0012 */
	 case RTL865X_ACL_IPV6_RANGE:					 /* 0x001A */
		 entry.ruleType = rule->ruleType_;
		 entry.ruleType1 = rule->ruleType_>>4;
		 if(rule->ipv6EntryType_==1)//first entry
		 {
			 entry.is.L3V6.is.entry0.sip_addr31_0	= rule->srcIpV6Addr_.v6_addr32[3];
			 entry.is.L3V6.is.entry0.sip_addr63_32	= rule->srcIpV6Addr_.v6_addr32[2];
			 entry.is.L3V6.is.entry0.sip_addr95_64	= rule->srcIpV6Addr_.v6_addr32[1];
			 entry.is.L3V6.is.entry0.sip_addr127_96 = rule->srcIpV6Addr_.v6_addr32[0];
			 
			 entry.is.L3V6.is.entry0.sip_mask31_0	= rule->srcIpV6AddrMask_.v6_addr32[3];
			 entry.is.L3V6.is.entry0.sip_mask63_32	= rule->srcIpV6AddrMask_.v6_addr32[2];
			 entry.is.L3V6.is.entry0.sip_mask95_64	= rule->srcIpV6AddrMask_.v6_addr32[1];

			 entry.ipv6.is.entry0.sip_mask119_96	= rule->srcIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
			 entry.ipv6.is.entry0.sip_mask127_120	= rule->srcIpV6AddrMask_.v6_addr32[0]>>24;
			 entry.ipv6.is.entry0.flowLabel 		= rule->ipv6FlowLabel_;
			 entry.ipv6.is.entry0.flowLabelM3_0 	= rule->ipv6FlowLabelM_&0xF;
			 entry.ipv6.is.entry0.flowLabelM19_4	= rule->ipv6FlowLabelM_>>4; 	 
		 }
		 else
		 {
			 entry.is.L3V6.is.entry1.dip_addr31_0	= rule->dstIpV6Addr_.v6_addr32[3];
			 entry.is.L3V6.is.entry1.dip_addr63_32	= rule->dstIpV6Addr_.v6_addr32[2];
			 entry.is.L3V6.is.entry1.dip_addr95_64	= rule->dstIpV6Addr_.v6_addr32[1];
			 entry.is.L3V6.is.entry1.dip_addr127_96 = rule->dstIpV6Addr_.v6_addr32[0];
			 
			 entry.is.L3V6.is.entry1.dip_mask31_0	= rule->dstIpV6AddrMask_.v6_addr32[3];
			 entry.is.L3V6.is.entry1.dip_mask63_32	= rule->dstIpV6AddrMask_.v6_addr32[2];
			 entry.is.L3V6.is.entry1.dip_mask95_64	= rule->dstIpV6AddrMask_.v6_addr32[1];
			 
			 entry.ipv6.is.entry1.dip_mask119_96	= rule->dstIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
			 entry.ipv6.is.entry1.dip_mask127_120	= rule->dstIpV6AddrMask_.v6_addr32[0]>>24;
			 entry.ipv6.is.entry1.trafficClass		= rule->ipv6TrafficClass_; 
			 entry.ipv6.is.entry1.trafficClassM 	= rule->ipv6TrafficClassM_; 
			 entry.ipv6.is.entry1.nextHeader		= rule->ipv6NextHeader_; 

			 entry.ipv6.is.entry1.nextHeaderM		= rule->ipv6NextHeaderM_; 
			 entry.ipv6.is.entry1.HTTPP 	  = entry.ipv6.is.entry1.HTTPM = rule->ipv6HttpFilter_;
			 entry.ipv6.is.entry1.identSDIPP  = entry.ipv6.is.entry1.identSDIPM  = rule->ipv6IdentSrcDstIp_; 
		 }

		 entry.inv		 = rule->ipv6Invert_;
		 entry.ipv6ETY0  = rule->ipv6EntryType_;
		 entry.comb 	 = rule->ipv6Combine_;
		 entry.ip_tunnel = rule->ipv6IPtunnel_;
		 break;
#endif
#if defined(CONFIG_RTL_8197F)
		 case RTL865X_ACL_PM:			 /* IP Rule Type: 0x0012 */
			 entry.ruleType = rule->ruleType_;
			 entry.ruleType1 = rule->ruleType_>>4;
			 if(rule->ipv6EntryType_==1) //first entry
			 {
				 entry.is.PM_U.is.entry0.spub = rule->un_ty.PM._spub;
				 entry.is.PM_U.is.entry0.splb = rule->un_ty.PM._splb;
				 entry.is.PM_U.is.entry0.dpub = rule->un_ty.PM._dpub;
				 entry.is.PM_U.is.entry0.dplb = rule->un_ty.PM._dplb;
				 entry.is.PM_U.is.entry0.protocol = rule->un_ty.PM._protocol;
				 
				 entry.is.PM_U.is.entry0.offset0 = rule->un_ty.PM._offset0;
				 entry.is.PM_U.is.entry0.pattern0 = rule->un_ty.PM._pattern0;
				 entry.is.PM_U.is.entry0.pm0_15_1 = rule->un_ty.PM._pm0>>1;
				 entry.is.PM_U.is.entry0.pm0_0 = rule->un_ty.PM._pm0 & 0x1;
				 entry.is.PM_U.is.entry0.or0 = rule->un_ty.PM._or0;
					 
				 entry.is.PM_U.is.entry0.offset1 = rule->un_ty.PM._offset1;
				 entry.is.PM_U.is.entry0.pattern1_8_0 = rule->un_ty.PM._pattern1&0x1ff;
				 entry.is.PM_U.is.entry0.pattern1_15_9 = rule->un_ty.PM._pattern1>>9; 
				 entry.is.PM_U.is.entry0.pm1 = rule->un_ty.PM._pm1;
				 entry.is.PM_U.is.entry0.or1 = rule->un_ty.PM._or1;
				 
				 entry.is.PM_U.is.entry0.offset2 = rule->un_ty.PM._offset2;
				 entry.is.PM_U.is.entry0.pattern2_0 = rule->un_ty.PM._pattern2&0x1;
				 entry.is.PM_U.is.entry0.pattern2_15_1 = rule->un_ty.PM._pattern2>>1;
				 entry.is.PM_U.is.entry0.pm2 = rule->un_ty.PM._pm2;
				 entry.is.PM_U.is.entry0.or2 = rule->un_ty.PM._or2;
				 
				 entry.is_d.PM_D.is.entry0.offset3 = rule->un_ty.PM._offset3;
				 entry.is_d.PM_D.is.entry0.pattern3 = rule->un_ty.PM._pattern3;
				 entry.is_d.PM_D.is.entry0.pm3_8_0 = rule->un_ty.PM._pm3&0x1ff;
				 entry.is_d.PM_D.is.entry0.pm3_15_9 = rule->un_ty.PM._pm3>>9;
				 entry.is_d.PM_D.is.entry0.or3 = rule->un_ty.PM._or3;
			 }
			 else
			 {
				 entry.is.PM_U.is.entry1.offset4 = rule->un_ty.PM._offset4;
				 entry.is.PM_U.is.entry1.pattern4 = rule->un_ty.PM._pattern4;
				 entry.is.PM_U.is.entry1.pm4_8_0 = rule->un_ty.PM._pm4&0x1ff;
				 entry.is.PM_U.is.entry1.pm4_15_9 = rule->un_ty.PM._pm4>>9; 		 
				 entry.is.PM_U.is.entry1.or4 = rule->un_ty.PM._or4;
				 
				 entry.is.PM_U.is.entry1.offset5 = rule->un_ty.PM._offset5;
				 entry.is.PM_U.is.entry1.pattern5 = rule->un_ty.PM._pattern5;
				 entry.is.PM_U.is.entry1.pm5_0 = rule->un_ty.PM._pm5&0x1;
				 entry.is.PM_U.is.entry1.pm5_15_1 = rule->un_ty.PM._pm5>>1;
				 entry.is.PM_U.is.entry1.or5 = rule->un_ty.PM._or5; 
				 
				 entry.is.PM_U.is.entry1.offset6 = rule->un_ty.PM._offset6; 
				 entry.is.PM_U.is.entry1.pattern6_8_0 = rule->un_ty.PM._pattern6&0x1ff; 
				 entry.is.PM_U.is.entry1.pattern6_15_9 = rule->un_ty.PM._pattern6>>9;
				 entry.is.PM_U.is.entry1.pm6 = rule->un_ty.PM._pm6; 
				 entry.is.PM_U.is.entry1.or6 = rule->un_ty.PM._or6;
				 
				 entry.is.PM_U.is.entry1.offset7 = rule->un_ty.PM._offset7;
				 entry.is.PM_U.is.entry1.pattern7_0 = rule->un_ty.PM._pattern7&0x1;
				 entry.is.PM_U.is.entry1.pattern7_15_1 = rule->un_ty.PM._pattern7>>1;
				 entry.is.PM_U.is.entry1.pm7 = rule->un_ty.PM._pm7;
				 entry.is.PM_U.is.entry1.or7 = rule->un_ty.PM._or7;
				 
				 entry.is.PM_U.is.entry1.specialop = rule->un_ty.PM._specialop;
				 entry.is.PM_U.is.entry1.spa = rule->un_ty.PM._spa;
				 entry.is.PM_U.is.entry1.spam = rule->un_ty.PM._spam;
				 entry.is.PM_U.is.entry1.pppctl = rule->un_ty.PM._pppctl;
				 entry.is.PM_U.is.entry1.pppctlm = rule->un_ty.PM._pppctlm;
				 entry.is.PM_U.is.entry1.pppctl_or = rule->un_ty.PM._pppctlor;
			 }
	 
			 entry.inv = rule->ipv6Invert_;
			 entry.ipv6ETY0 = rule->ipv6EntryType_;
			 entry.comb = rule->ipv6Combine_;
			 entry.ip_tunnel = rule->ipv6IPtunnel_;
			 entry.is_d.PM_D.is.entry1.ipfrag_apply = rule->ipfrag_apply_;
			 break;
#endif
#endif
	default: return FAILED; /* Unknown rule type */
	
	}
	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	/*   when comb = 1, will continue to check down next ACL rule, 
		the last one of the rule set must set comb = 0, 
		all ACL rule match will do the last one's action 
	*/
	entry.comb		= rule->ipv6Combine_;
	entry.inv		= rule->ipv6Invert_;
	entry.ip_tunnel = rule->ipv6IPtunnel_;
#endif
#if defined(CONFIG_RTL_8197F)
	entry.is_d.PM_D.is.entry1.ipfrag_apply = rule->ipfrag_apply_;
#endif

	switch(rule->actionType_) {
#if defined(RTL_REFINE_ACL)
	diag_printf("%s %d entry.actionType=0x%x \n", __FUNCTION__, __LINE__, entry.actionType);
	case RTL865X_ACL_PERMIT:
	case RTL865X_ACL_REDIRECT_ETHER:
	case RTL865X_ACL_DROP:
	case RTL865X_ACL_TOCPU:
	case RTL865X_ACL_LEGACY_DROP:
	case RTL865X_ACL_DROPCPU_LOG:
	case RTL865X_ACL_MIRROR:
	case RTL865X_ACL_REDIRECT_PPPOE:
	case RTL865X_ACL_MIRROR_KEEP_MATCH:
		 entry.nextHop = rule->L2Idx_;
		 entry.vid = rule->netifIdx_;
		 entry.PPPoEIndex = rule->pppoeIdx_;
		 break;

	case RTL865X_ACL_DEFAULT_REDIRECT:
		entry.nextHop = rule->nexthopIdx_;
		break;

	case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_PRIORITY:
		entry.nextHop = rule->priority_;
		break;
	#if defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_VID:		 
		entry.nextHop = (rule->aclvid_ & 0x3ff);
		entry.vid = (rule->aclvid_ & 0xc00)>>10;
		entry.vid |= (rule->aclfid_&0x1)<<2;
		entry.PPPoEIndex = (rule->aclfid_&0x2)>>1;
		break;
	#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_VID:
		entry.nextHop = (rule->aclvid_ & 0x3ff);
		entry.vid = (rule->aclvid_ & 0xc00)>>10;
		break;
	#endif
#else
	case RTL8651_ACL_PERMIT:			entry.actionType = 0x00;
		 goto _common_action;
	case RTL8651_ACL_DROP:			entry.actionType = 0x02;
		 goto _common_action;
	case RTL8651_ACL_CPU:		 	 	entry.actionType = 0x03;
		 goto _common_action;
	case RTL8651_ACL_DROP_LOG: /* fall thru */
	case RTL8651_ACL_DROP_NOTIFY: entry.actionType = 0x05;
		goto _common_action;

_common_action:
		/* handle pktOpApp */
		if ( rule->pktOpApp == RTL865XC_ACLTBL_ALL_LAYER )
			entry.pktOpApp = 0x7;
		else if ( rule->pktOpApp == RTL8651_ACLTBL_NOOP )
			entry.pktOpApp = 0;
		else
 			entry.pktOpApp = rule->pktOpApp;
 		break;
#endif
	}
#if defined(RTL_REFINE_ACL)
	entry.actionType = rule->actionType_;
	entry.pktOpApp = rule->pktOpApp_;
#endif
    /* Write into hardware */
    if ( swTable_forceAddEntry(TYPE_ACL_RULE_TABLE, idx, &entry) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

#if 1//defined(CONFIG_RTL_VLAN_SUPPORT)
int rtl8651_getAsicAclRule(uint32 index, rtl_acl_param_t *rule)
{
	acl_table_t    entry;
	if((index >= RTL8651_ACLTBL_SIZE) || rule == NULL)
		return FAILED;

	bzero(rule, sizeof(rtl_acl_param_t));
	swTable_readEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	
#if	defined(RTL_REFINE_ACL) && (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F))
	rule->ruleType_  = entry.ruleType  ;
	rule->ruleType_ |= entry.ruleType1<<4;
	switch(rule->ruleType_) {
#else	 
	switch(entry.ruleType) {
#endif
	case RTL8651_ACL_MAC: /* Ethernet rule type */
		 rule->dstMac_.octet[0]     = entry.is.ETHERNET.dMacP47_32 >> 8;
		 rule->dstMac_.octet[1]     = entry.is.ETHERNET.dMacP47_32 & 0xff;
		 rule->dstMac_.octet[2]     = entry.is.ETHERNET.dMacP31_16 >> 8;
	 	 rule->dstMac_.octet[3]     = entry.is.ETHERNET.dMacP31_16 & 0xff;
		 rule->dstMac_.octet[4]     = entry.is.ETHERNET.dMacP15_0 >> 8;
		 rule->dstMac_.octet[5]     = entry.is.ETHERNET.dMacP15_0 & 0xff;
		 rule->dstMacMask_.octet[0] = entry.is.ETHERNET.dMacM47_32 >> 8;
		 rule->dstMacMask_.octet[1] = entry.is.ETHERNET.dMacM47_32 & 0xff;
		 rule->dstMacMask_.octet[2] = entry.is.ETHERNET.dMacM31_16 >> 8;
		 rule->dstMacMask_.octet[3] = entry.is.ETHERNET.dMacM31_16 & 0xff;
	 	 rule->dstMacMask_.octet[4] = entry.is.ETHERNET.dMacM15_0 >> 8;
		 rule->dstMacMask_.octet[5] = entry.is.ETHERNET.dMacM15_0 & 0xff;
	 	 rule->srcMac_.octet[0]     = entry.is.ETHERNET.sMacP47_32 >> 8;
		 rule->srcMac_.octet[1]     = entry.is.ETHERNET.sMacP47_32 & 0xff;
		 rule->srcMac_.octet[2]     = entry.is.ETHERNET.sMacP31_16 >> 8;
		 rule->srcMac_.octet[3]     = entry.is.ETHERNET.sMacP31_16 & 0xff;
		 rule->srcMac_.octet[4]     = entry.is.ETHERNET.sMacP15_0 >> 8;
		 rule->srcMac_.octet[5]     = entry.is.ETHERNET.sMacP15_0 & 0xff;
		 rule->srcMacMask_.octet[0] = entry.is.ETHERNET.sMacM47_32 >> 8;
		 rule->srcMacMask_.octet[1] = entry.is.ETHERNET.sMacM47_32 & 0xff;
		 rule->srcMacMask_.octet[2] = entry.is.ETHERNET.sMacM31_16 >> 8;
		 rule->srcMacMask_.octet[3] = entry.is.ETHERNET.sMacM31_16 & 0xff;
		 rule->srcMacMask_.octet[4] = entry.is.ETHERNET.sMacM15_0 >> 8;
		 rule->srcMacMask_.octet[5] = entry.is.ETHERNET.sMacM15_0 & 0xff;
		 rule->typeLen_             = entry.is.ETHERNET.ethTypeP;
		 rule->typeLenMask_         = entry.is.ETHERNET.ethTypeM;
		 rule->ruleType_            = entry.ruleType;
		 break;
	case RTL865X_ACL_IP: /* IP mask rule type */
	case RTL865X_ACL_IP_RANGE: /* IP range rule type*/
		 rule->tos_         = entry.is.L3L4.is.IP.IPTOSP;
		 rule->tosMask_     = entry.is.L3L4.is.IP.IPTOSM;
		 rule->ipProto_     = entry.is.L3L4.is.IP.IPProtoP;
		 rule->ipProtoMask_ = entry.is.L3L4.is.IP.IPProtoM;
		 rule->ipFlag_      = entry.is.L3L4.is.IP.IPFlagP;
		 rule->ipFlagMask_  = entry.is.L3L4.is.IP.IPFlagM;
 		 rule->ipFOP_ = entry.is.L3L4.is.IP.FOP;
		 rule->ipFOM_ = entry.is.L3L4.is.IP.FOM;
		 rule->ipHttpFilterM_ = entry.is.L3L4.is.IP.HTTPM;
		 rule->ipHttpFilter_  = entry.is.L3L4.is.IP.HTTPP;
		 rule->ipIdentSrcDstIp_ = entry.is.L3L4.is.IP.identSDIPM;
		 rule->ruleType_= entry.ruleType;
		 goto l3l4_shared;
		 
	 #if defined(RTL_REFINE_ACL)	 
	 case RTL865X_ACL_ICMP: /* ICMP  (ip is mask) rule type */
	 case RTL865X_ACL_ICMP_IPRANGE: /* ICMP (ip is	range) rule type */
		  rule->tos_ = entry.is.L3L4.is.ICMP.IPTOSP;
		  rule->tosMask_ = entry.is.L3L4.is.ICMP.IPTOSM;
		  rule->icmpType_ = entry.is.L3L4.is.ICMP.ICMPTypeP;
		  rule->icmpTypeMask_ = entry.is.L3L4.is.ICMP.ICMPTypeM;
		  rule->icmpCode_ = entry.is.L3L4.is.ICMP.ICMPCodeP;
		  rule->icmpCodeMask_ = entry.is.L3L4.is.ICMP.ICMPCodeM;
		  rule->ruleType_ = entry.ruleType;
		  goto l3l4_shared;
	 
	 case RTL865X_ACL_IGMP: /* IGMP (ip is mask) rule type */
	 case RTL865X_ACL_IGMP_IPRANGE: /* IGMP (ip is range) rule type */
		  rule->tos_ = entry.is.L3L4.is.IGMP.IPTOSP;
		  rule->tosMask_ = entry.is.L3L4.is.IGMP.IPTOSM;
		  rule->igmpType_ = entry.is.L3L4.is.IGMP.IGMPTypeP;
		  rule->igmpTypeMask_ = entry.is.L3L4.is.IGMP.IGMPTypeM;
		  rule->ruleType_ = entry.ruleType;
		  goto l3l4_shared;
	 
	 case RTL865X_ACL_TCP: /* TCP rule type */
	 case RTL865X_ACL_TCP_IPRANGE:
		  rule->tos_ = entry.is.L3L4.is.TCP.IPTOSP;
		  rule->tosMask_ = entry.is.L3L4.is.TCP.IPTOSM;
		  rule->tcpFlag_ = entry.is.L3L4.is.TCP.TCPFlagP;
		  rule->tcpFlagMask_ = entry.is.L3L4.is.TCP.TCPFlagM;
		  rule->tcpSrcPortUB_ = entry.is.L3L4.is.TCP.TCPSPUB;
		  rule->tcpSrcPortLB_ = entry.is.L3L4.is.TCP.TCPSPLB;
		  rule->tcpDstPortUB_ = entry.is.L3L4.is.TCP.TCPDPUB;
		  rule->tcpDstPortLB_ = entry.is.L3L4.is.TCP.TCPDPLB;
		  rule->ruleType_ = entry.ruleType;
		  goto l3l4_shared;
	 
	 case RTL865X_ACL_UDP: /* UDP rule type */
	 case RTL865X_ACL_UDP_IPRANGE:
		  rule->tos_ = entry.is.L3L4.is.UDP.IPTOSP;
		  rule->tosMask_ = entry.is.L3L4.is.UDP.IPTOSM;
		  rule->udpSrcPortUB_ = entry.is.L3L4.is.UDP.UDPSPUB;
		  rule->udpSrcPortLB_ = entry.is.L3L4.is.UDP.UDPSPLB;
		  rule->udpDstPortUB_ = entry.is.L3L4.is.UDP.UDPDPUB;
		  rule->udpDstPortLB_ = entry.is.L3L4.is.UDP.UDPDPLB;
		  rule->ruleType_ = entry.ruleType;
	#endif	 
	l3l4_shared:
		rule->srcIpAddr_ = entry.is.L3L4.sIPP;
		rule->srcIpAddrMask_ = entry.is.L3L4.sIPM;
		rule->dstIpAddr_ = entry.is.L3L4.dIPP;
		rule->dstIpAddrMask_ = entry.is.L3L4.dIPM;
		break;
#if defined(RTL_REFINE_ACL)
	case RTL865X_ACL_SRCFILTER: /* Source Filter */
	case RTL865X_ACL_SRCFILTER_IPRANGE:
		 rule->srcFilterMac_.octet[0]	  = entry.is.SRC_FILTER.sMacP47_32 >> 8;
		 rule->srcFilterMac_.octet[1]	  = entry.is.SRC_FILTER.sMacP47_32 & 0xff;
		 rule->srcFilterMac_.octet[2]	  = entry.is.SRC_FILTER.sMacP31_16 >> 8;
		 rule->srcFilterMac_.octet[3]	  = entry.is.SRC_FILTER.sMacP31_16 & 0xff;
		 rule->srcFilterMac_.octet[4]	  = entry.is.SRC_FILTER.sMacP15_0 >> 8;
		 rule->srcFilterMac_.octet[5]	  = entry.is.SRC_FILTER.sMacP15_0 & 0xff;
		 if ( entry.is.SRC_FILTER.sMacM3_0&0x8)
		{
			 rule->srcFilterMacMask_.octet[0] = 0xff;
			 rule->srcFilterMacMask_.octet[1] = 0xff;
			 rule->srcFilterMacMask_.octet[2] = 0xff;
			 rule->srcFilterMacMask_.octet[3] = 0xff;
			 rule->srcFilterMacMask_.octet[4] = 0xff;
			 rule->srcFilterMacMask_.octet[5] = 0xF0|entry.is.SRC_FILTER.sMacM3_0;
		}
		 else
		{
			 rule->srcFilterMacMask_.octet[0] = 0x0;
			 rule->srcFilterMacMask_.octet[1] = 0x0;
			 rule->srcFilterMacMask_.octet[2] = 0x0;
			 rule->srcFilterMacMask_.octet[3] = 0x0;
			 rule->srcFilterMacMask_.octet[4] = 0x0;
			 rule->srcFilterMacMask_.octet[5] = entry.is.SRC_FILTER.sMacM3_0;
		}

		 rule->srcFilterPort_ = entry.is.SRC_FILTER.spaP;
		 rule->srcFilterVlanIdx_ = entry.is.SRC_FILTER.sVidP;
		 rule->srcFilterVlanIdxMask_ = entry.is.SRC_FILTER.sVidM;
		 if(entry.is.SRC_FILTER.protoType == 2) rule->srcFilterIgnoreL4_ = 1;
		 else if(entry.is.SRC_FILTER.protoType == 1) rule->srcFilterIgnoreL3L4_ = 1;
		 rule->srcFilterIpAddr_ = entry.is.SRC_FILTER.sIPP;
		 rule->srcFilterIpAddrMask_ = entry.is.SRC_FILTER.sIPM;
		 rule->srcFilterPortUpperBound_ = entry.is.SRC_FILTER.SPORTUB;
		 rule->srcFilterPortLowerBound_ = entry.is.SRC_FILTER.SPORTLB;
		 rule->ruleType_ = entry.ruleType;
		 break;

	case RTL865X_ACL_DSTFILTER: /* Destination Filter */
	case RTL865X_ACL_DSTFILTER_IPRANGE: /* Destination Filter(IP range) */
		 rule->dstFilterMac_.octet[0]	  = entry.is.DST_FILTER.dMacP47_32 >> 8;
		 rule->dstFilterMac_.octet[1]	  = entry.is.DST_FILTER.dMacP47_32 & 0xff;
		 rule->dstFilterMac_.octet[2]	  = entry.is.DST_FILTER.dMacP31_16 >> 8;
		 rule->dstFilterMac_.octet[3]	  = entry.is.DST_FILTER.dMacP31_16 & 0xff;
		 rule->dstFilterMac_.octet[4]	  = entry.is.DST_FILTER.dMacP15_0 >> 8;
		 rule->dstFilterMac_.octet[5]	  = entry.is.DST_FILTER.dMacP15_0 & 0xff;
		 if ( entry.is.DST_FILTER.dMacM3_0&0x8)
		{
			 rule->dstFilterMacMask_.octet[0] = 0xff;
			 rule->dstFilterMacMask_.octet[1] = 0xff;
			 rule->dstFilterMacMask_.octet[2] = 0xff;
			 rule->dstFilterMacMask_.octet[3] = 0xff;
			 rule->dstFilterMacMask_.octet[4] = 0xff;
			 rule->dstFilterMacMask_.octet[5] = 0xF0|entry.is.DST_FILTER.dMacM3_0;
		}
		 else
		{
			 rule->dstFilterMacMask_.octet[0] = 0x0;
			 rule->dstFilterMacMask_.octet[1] = 0x0;
			 rule->dstFilterMacMask_.octet[2] = 0x0;
			 rule->dstFilterMacMask_.octet[3] = 0x0;
			 rule->dstFilterMacMask_.octet[4] = 0x0;
			 rule->dstFilterMacMask_.octet[5] = entry.is.DST_FILTER.dMacM3_0;
		}


		 rule->dstFilterVlanIdx_ = entry.is.DST_FILTER.vidP;
		 rule->dstFilterVlanIdxMask_ = entry.is.DST_FILTER.vidM;
		 if(entry.is.DST_FILTER.protoType == 1) rule->dstFilterIgnoreL3L4_ = 1;
		 else if(entry.is.DST_FILTER.protoType == 2) rule->dstFilterIgnoreL4_ = 1;
		 rule->dstFilterIpAddr_ = entry.is.DST_FILTER.dIPP;
		 rule->dstFilterIpAddrMask_ = entry.is.DST_FILTER.dIPM;
		 rule->dstFilterPortUpperBound_ = entry.is.DST_FILTER.DPORTUB;
		 rule->dstFilterPortLowerBound_ = entry.is.DST_FILTER.DPORTLB;
		 rule->ruleType_ = entry.ruleType;
		 break;

#if	defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_IPV6: /* IP Rule Type: 0x0010 */
	case RTL865X_ACL_IPV6_RANGE:

		if(entry.ipv6ETY0)//first entry
		{
			rule->srcIpV6Addr_.v6_addr32[3] = entry.is.L3V6.is.entry0.sip_addr31_0;
			rule->srcIpV6Addr_.v6_addr32[2] = entry.is.L3V6.is.entry0.sip_addr63_32;
			rule->srcIpV6Addr_.v6_addr32[1] = entry.is.L3V6.is.entry0.sip_addr95_64;
			rule->srcIpV6Addr_.v6_addr32[0] = entry.is.L3V6.is.entry0.sip_addr127_96;
			
			rule->srcIpV6AddrMask_.v6_addr32[3] = entry.is.L3V6.is.entry0.sip_mask31_0;
			rule->srcIpV6AddrMask_.v6_addr32[2] = entry.is.L3V6.is.entry0.sip_mask63_32;
			rule->srcIpV6AddrMask_.v6_addr32[1] = entry.is.L3V6.is.entry0.sip_mask95_64;

			rule->srcIpV6AddrMask_.v6_addr32[0] =  entry.is_d.ipv6.is.entry0.sip_mask119_96;
			rule->srcIpV6AddrMask_.v6_addr32[0] |= entry.is_d.ipv6.is.entry0.sip_mask127_120 <<24;
			rule->ipv6FlowLabel_  =  entry.is_d.ipv6.is.entry0.flowLabel;
			rule->ipv6FlowLabelM_ =  entry.is_d.ipv6.is.entry0.flowLabelM3_0;
			rule->ipv6FlowLabelM_ |= entry.is_d.ipv6.is.entry0.flowLabelM19_4<<4;
		}
		else
		{
			rule->dstIpV6Addr_.v6_addr32[3] = entry.is.L3V6.is.entry1.dip_addr31_0;
			rule->dstIpV6Addr_.v6_addr32[2] = entry.is.L3V6.is.entry1.dip_addr63_32;
			rule->dstIpV6Addr_.v6_addr32[1] = entry.is.L3V6.is.entry1.dip_addr95_64;
			rule->dstIpV6Addr_.v6_addr32[0] = entry.is.L3V6.is.entry1.dip_addr127_96;
	  
			rule->dstIpV6AddrMask_.v6_addr32[3] = entry.is.L3V6.is.entry1.dip_mask31_0;
			//entry.is.L3V6.is.entry1.dip_mask63_32  = rule->dstIpV6AddrMask_.v6_addr32[2];
			//entry.is.L3V6.is.entry1.dip_mask95_64  = rule->dstIpV6AddrMask_.v6_addr32[1];
			rule->dstIpV6AddrMask_.v6_addr32[2] = entry.is.L3V6.is.entry1.dip_mask63_32;
			rule->dstIpV6AddrMask_.v6_addr32[1] = entry.is.L3V6.is.entry1.dip_mask95_64;
			
			rule->dstIpV6AddrMask_.v6_addr32[0] = entry.is_d.ipv6.is.entry1.dip_mask119_96;
			rule->dstIpV6AddrMask_.v6_addr32[0] |= entry.is_d.ipv6.is.entry1.dip_mask127_120<<24;
			rule->ipv6TrafficClass_  = entry.is_d.ipv6.is.entry1.trafficClass; 
			rule->ipv6TrafficClassM_ = entry.is_d.ipv6.is.entry1.trafficClassM; 
			rule->ipv6NextHeader_	 = entry.is_d.ipv6.is.entry1.nextHeader; 
			rule->ipv6NextHeaderM_	 = entry.is_d.ipv6.is.entry1.nextHeaderM; 
			rule->ipv6HttpFilter_	 = entry.is_d.ipv6.is.entry1.HTTPP;
			rule->ipv6HttpFilterM_	  = entry.is_d.ipv6.is.entry1.HTTPM;
			rule->ipv6IdentSrcDstIp_ = entry.is_d.ipv6.is.entry1.identSDIPP;
			rule->ipv6IdentSrcDstIpM_ = entry.is_d.ipv6.is.entry1.identSDIPM;
		}

		rule->ipv6Invert_	 = entry.inv;
		rule->ipv6EntryType_ = entry.ipv6ETY0;
		rule->ipv6Combine_	= entry.comb ;
		rule->ipv6IPtunnel_ = entry.ip_tunnel;
		break;
#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_IPV6: /* IP Rule Type: 0x0010 */
	case RTL865X_ACL_IPV6_RANGE:

		if(entry.ipv6ETY0)//first entry
		{
			rule->srcIpV6Addr_.v6_addr32[3] = entry.is.L3V6.is.entry0.sip_addr31_0;
			rule->srcIpV6Addr_.v6_addr32[2] = entry.is.L3V6.is.entry0.sip_addr63_32;
			rule->srcIpV6Addr_.v6_addr32[1] = entry.is.L3V6.is.entry0.sip_addr95_64;
			rule->srcIpV6Addr_.v6_addr32[0] = entry.is.L3V6.is.entry0.sip_addr127_96;
			
			rule->srcIpV6AddrMask_.v6_addr32[3] = entry.is.L3V6.is.entry0.sip_mask31_0;
			rule->srcIpV6AddrMask_.v6_addr32[2] = entry.is.L3V6.is.entry0.sip_mask63_32;
			rule->srcIpV6AddrMask_.v6_addr32[1] = entry.is.L3V6.is.entry0.sip_mask95_64;

			rule->srcIpV6AddrMask_.v6_addr32[0] =  entry.ipv6.is.entry0.sip_mask119_96;
			rule->srcIpV6AddrMask_.v6_addr32[0] |= entry.ipv6.is.entry0.sip_mask127_120 <<24;
			rule->ipv6FlowLabel_  =  entry.ipv6.is.entry0.flowLabel;
			rule->ipv6FlowLabelM_ =  entry.ipv6.is.entry0.flowLabelM3_0;
			rule->ipv6FlowLabelM_ |= entry.ipv6.is.entry0.flowLabelM19_4<<4;
		}
		else
		{
			rule->dstIpV6Addr_.v6_addr32[3] = entry.is.L3V6.is.entry1.dip_addr31_0;
			rule->dstIpV6Addr_.v6_addr32[2] = entry.is.L3V6.is.entry1.dip_addr63_32;
			rule->dstIpV6Addr_.v6_addr32[1] = entry.is.L3V6.is.entry1.dip_addr95_64;
			rule->dstIpV6Addr_.v6_addr32[0] = entry.is.L3V6.is.entry1.dip_addr127_96;
	  
			rule->dstIpV6AddrMask_.v6_addr32[3] = entry.is.L3V6.is.entry1.dip_mask31_0;
			//entry.is.L3V6.is.entry1.dip_mask63_32  = rule->dstIpV6AddrMask_.v6_addr32[2];
			//entry.is.L3V6.is.entry1.dip_mask95_64  = rule->dstIpV6AddrMask_.v6_addr32[1];
			rule->dstIpV6AddrMask_.v6_addr32[2] = entry.is.L3V6.is.entry1.dip_mask63_32;
			rule->dstIpV6AddrMask_.v6_addr32[1] = entry.is.L3V6.is.entry1.dip_mask95_64;
			
			rule->dstIpV6AddrMask_.v6_addr32[0] = entry.ipv6.is.entry1.dip_mask119_96;
			rule->dstIpV6AddrMask_.v6_addr32[0] |= entry.ipv6.is.entry1.dip_mask127_120<<24;
			rule->ipv6TrafficClass_  = entry.ipv6.is.entry1.trafficClass; 
			rule->ipv6TrafficClassM_ = entry.ipv6.is.entry1.trafficClassM; 
			rule->ipv6NextHeader_	 = entry.ipv6.is.entry1.nextHeader; 
			rule->ipv6NextHeaderM_	 = entry.ipv6.is.entry1.nextHeaderM; 
			rule->ipv6HttpFilter_	 = entry.ipv6.is.entry1.HTTPP;
			rule->ipv6HttpFilterM_	  = entry.ipv6.is.entry1.HTTPM;
			rule->ipv6IdentSrcDstIp_ = entry.ipv6.is.entry1.identSDIPP;
			rule->ipv6IdentSrcDstIpM_ = entry.ipv6.is.entry1.identSDIPM;
		}

		rule->ipv6Invert_	 = entry.inv;
		rule->ipv6EntryType_ = entry.ipv6ETY0;
		rule->ipv6Combine_	= entry.comb ;
		rule->ipv6IPtunnel_ = entry.ip_tunnel;
		break;
#endif
		
#if	defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_PM: /* acl pattern match */
		if(entry.ipv6ETY0)//first entry
		{
			rule->pmSPUB_ = entry.is.PM_U.is.entry0.spub;
			rule->pmSPLB_ = entry.is.PM_U.is.entry0.splb;
			rule->pmDPUB_ = entry.is.PM_U.is.entry0.dpub;
			rule->pmDPLB_ = entry.is.PM_U.is.entry0.dplb;
			rule->pmProtocol_ = entry.is.PM_U.is.entry0.protocol;			 

			rule->pmOffset0_  = entry.is.PM_U.is.entry0.offset0;
			rule->pmPattern0_ = entry.is.PM_U.is.entry0.pattern0;
			rule->pmPatternMask0_ = ((entry.is.PM_U.is.entry0.pm0_15_1<<1 )
										| entry.is.PM_U.is.entry0.pm0_0);
			rule->pmOR0_ =entry.is.PM_U.is.entry0.or0 ; 	  

			rule->pmOffset1_	  = entry.is.PM_U.is.entry0.offset1;
			rule->pmPattern1_ = ((entry.is.PM_U.is.entry0.pattern1_8_0)
								|(entry.is.PM_U.is.entry0.pattern1_15_9<<9));
			rule->pmPatternMask1_ = entry.is.PM_U.is.entry0.pm1;
			rule->pmOR1_		  = entry.is.PM_U.is.entry0.or1;

			rule->pmOffset2_	  = entry.is.PM_U.is.entry0.offset2;
			rule->pmPattern2_ = (entry.is.PM_U.is.entry0.pattern2_0
							  |( entry.is.PM_U.is.entry0.pattern2_15_1<<1));
			rule->pmPatternMask2_ = entry.is.PM_U.is.entry0.pm2;
			rule->pmOR2_		  = entry.is.PM_U.is.entry0.or2;		

			rule->pmOffset3_  = entry.is_d.PM_D.is.entry0.offset3;
			rule->pmPattern3_ = entry.is_d.PM_D.is.entry0.pattern3;
			rule->pmPatternMask3_ = (entry.is_d.PM_D.is.entry0.pm3_8_0
								   |(entry.is_d.PM_D.is.entry0.pm3_15_9<<9));
			rule->pmOR3_ = entry.is_d.PM_D.is.entry0.or3;
		}
		else
		{
			rule->pmOffset4_	  = entry.is.PM_U.is.entry1.offset4;
			rule->pmPattern4_	  = entry.is.PM_U.is.entry1.pattern4;
			rule->pmPatternMask4_ = (entry.is.PM_U.is.entry1.pm4_8_0
								  | (entry.is.PM_U.is.entry1.pm4_15_9<<9));

			rule->pmOR4_		  = entry.is.PM_U.is.entry1.or4;
			
			rule->pmOffset5_ = entry.is.PM_U.is.entry1.offset5;
			rule->pmPattern5_ = entry.is.PM_U.is.entry1.pattern5;
			rule->pmPatternMask5_ = (entry.is.PM_U.is.entry1.pm5_0
									|(entry.is.PM_U.is.entry1.pm5_15_1<<1));
			rule->pmOR5_ = entry.is.PM_U.is.entry1.or5;

			rule->pmOffset6_ = entry.is.PM_U.is.entry1.offset6;
			rule->pmPattern6_ = (entry.is.PM_U.is.entry1.pattern6_8_0
							   |(entry.is.PM_U.is.entry1.pattern6_15_9<<9));
			rule->pmPatternMask6_ = entry.is.PM_U.is.entry1.pm6;
			rule->pmOR6_ = entry.is.PM_U.is.entry1.or6;

			rule->pmOffset7_  = entry.is.PM_U.is.entry1.offset7;
			rule->pmPattern7_ = (entry.is.PM_U.is.entry1.pattern7_0 
							   |(entry.is.PM_U.is.entry1.pattern7_15_1<<1));
			rule->pmPatternMask7_ = entry.is.PM_U.is.entry1.pm7;
			rule->pmOR7_ = entry.is.PM_U.is.entry1.or7;

			rule->pmSpecialop_ = entry.is.PM_U.is.entry1.specialop;
			rule->pmSPA_ = entry.is.PM_U.is.entry1.spa;
			rule->pmSPAM_ = entry.is.PM_U.is.entry1.spam;
			rule->pmPPPCTL_ = entry.is.PM_U.is.entry1.pppctl;
			rule->pmPPPCTLM_ = entry.is.PM_U.is.entry1.pppctlm;
			rule->pmPPPCTLOR_ = entry.is.PM_U.is.entry1.pppctl_or;
		}
		rule->ipv6IPtunnel_ = entry.ip_tunnel;
		rule->ipfrag_apply_ = entry.is_d.PM_D.is.entry1.ipfrag_apply;
		rule->ipv6EntryType_ = entry.ipv6ETY0;
		break;
#endif
#endif
	default: return FAILED; /* Unknown rule type */

	}
	
#if defined(RTL_REFINE_ACL)	
#if defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8198C)
	rule->ipv6Combine_	= entry.comb;
	rule->ipv6Invert_	 = entry.inv;
	rule->ipv6IPtunnel_	= entry.ip_tunnel;		
#endif

#if defined(CONFIG_RTL_8197F)
	rule->ipfrag_apply_ = entry.is_d.PM_D.is.entry1.ipfrag_apply;
#endif
	switch(entry.actionType) {

	case RTL865X_ACL_PERMIT:
	case RTL865X_ACL_REDIRECT_ETHER:
	case RTL865X_ACL_DROP:
	case RTL865X_ACL_TOCPU:
	case RTL865X_ACL_LEGACY_DROP:
	case RTL865X_ACL_DROPCPU_LOG:
	case RTL865X_ACL_MIRROR:
	case RTL865X_ACL_REDIRECT_PPPOE:
	case RTL865X_ACL_MIRROR_KEEP_MATCH:
		rule->L2Idx_ = entry.nextHop ;
		rule->netifIdx_	=  entry.vid;
		rule->pppoeIdx_	= entry.PPPoEIndex;
		 break;

	case RTL865X_ACL_DEFAULT_REDIRECT:
		rule->nexthopIdx_ =	entry.nextHop;
		break;

	case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_PRIORITY:
		rule->priority_	= entry.nextHop;
		break;

#if	defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_VID:
		rule->aclvid_ =	(entry.nextHop | (entry.vid	<<10)) & 0xfff;
		rule->aclfid_ =	((entry.PPPoEIndex << 1)| (entry.vid >>2)) & 0x3;
		break;

#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_VID:
		rule->aclvid_ =	(entry.nextHop | (entry.vid	<<10)) & 0xfff;
		break;
#endif

	}	
#endif
	rule->aclIdx = index;
	rule->actionType_ = entry.actionType;
	rule->pktOpApp = entry.pktOpApp;

	return SUCCESS;
}
#endif
#endif

int32 swCore_netifCreate(uint32 idx, rtl_netif_param_t * param)
{
    netif_table_t    entryContent;
		
#if 0    
    uint32	temp,temp2;    

    // disable interrupt
    // I don't know the reason but if you want to use "-O" flag, must disalbe interrupt before swTable_readEntry();
    temp = lx4180_ReadStatus();
    if (0 != (temp&0x1)) {
	    temp2 = temp&0xfffffffe;
	    lx4180_WriteStatus(temp2);
    }
#endif		

    ASSERT_CSP(param);

    swTable_readEntry(TYPE_NETINTERFACE_TABLE, idx, &entryContent);

#if 0
    // restore status register
    if (0 != (temp&0x1)) {
	    lx4180_WriteStatus(temp);
    }
#endif
		
    if ( entryContent.valid )
    {
       //return EEXIST;
       return 17;
    }

    bzero( (void *) &entryContent, sizeof(entryContent) );
    entryContent.valid = param->valid;
    entryContent.vid = param->vid;

    entryContent.mac47_19 = ((param->gMac.mac47_32 << 13) | (param->gMac.mac31_16 >> 3)) & 0x1FFFFFFF;
    entryContent.mac18_0 = ((param->gMac.mac31_16 << 16) | param->gMac.mac15_0) & 0x7FFFF;
#if defined(CONFIG_RTL_8197F)
	entryContent.inACLStartH = (param->inAclStart >>1)&0x7f;
	entryContent.inACLStartL = param->inAclStart&0x1;
	entryContent.inACLEnd    = param->inAclEnd;
	entryContent.outACLStart = param->outAclStart;
	entryContent.outACLEnd   = param->outAclEnd;

	entryContent.enHWRouteV6 = 0;

	entryContent.enHWRoute   = (rtl8651_getAsicOperationLayer()>2)?	(param->enableRoute==TRUE? 1: 0):0;

	switch(param->macAddrNumber) {
		case 0:
		case 1:
		    entryContent.macMaskL = 1;
		    entryContent.macMaskH = 3;
		break;
		case 2:
		    entryContent.macMaskL = 0;
		    entryContent.macMaskH = 3;
		break;
		case 4:
		    entryContent.macMaskL = 0;
		    entryContent.macMaskH = 2;
		break;
		case 8:
		    entryContent.macMaskL = 0;
		    entryContent.macMaskH = 0;
			break;
		default:
		    return FAILED;//Not permitted macNumber value
	}
	entryContent.mtu   = param->mtu;
	//entryContent.mtuV6 = intfp->mtuV6;
#else
    entryContent.inACLStartH = (param->inAclStart >> 2) & 0x1f;
    entryContent.inACLStartL = param->inAclStart & 0x3;
    entryContent.inACLEnd = param->inAclEnd;
    entryContent.outACLStart = param->outAclStart;
    entryContent.outACLEnd = param->outAclEnd;
    entryContent.enHWRoute = param->enableRoute;

    entryContent.macMask = 8 - (param->macAddrNumber & 0x7);

    entryContent.mtuH = param->mtu >> 3;
    entryContent.mtuL = param->mtu & 0x7;
#endif
    /* Write into hardware */
    if ( swTable_addEntry(TYPE_NETINTERFACE_TABLE, idx, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}



#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
extern int32 rtl8651_getAsicVlan(uint16 vid, rtl865x_tblAsicDrv_vlanParam_t *vlanp); 
int rtl8651_findAsicVlanIndexByVid(uint16 *vid)
{
	int i;
	rtl865x_tblAsicDrv_vlanParam_t vlan;

	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		if (rtl8651_getAsicVlan((uint16)i, &vlan) == SUCCESS){
			if(*vid == vlan.vid){
				*vid = i;
				return SUCCESS;
			}
		}
	}

	return FAILED;
}

static int rtl8651_getAsicVlanIndex(vlan_table_t *entry, uint32 *vid)
{
	int i;
	int ret = FAILED;
	rtl865x_tblAsicDrv_vlanParam_t vlan;

	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		if ((rtl8651_getAsicVlan((uint16)i, &vlan) == SUCCESS) && (entry->vid == vlan.vid)){
			if((entry->memberPort != vlan.memberPortMask) || (entry->egressUntag != vlan.untagPortMask) ||(entry->fid != vlan.fid)){
				*vid = i;
				return SUCCESS;
			}else{
				return FAILED;
			}
		}
	}

	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		if ( rtl8651_getAsicVlan((uint16)i, &vlan ) == FAILED )
			break;
	}

	if(i == RTL865XC_VLANTBL_SIZE){
		ret = FAILED;	//vlan table is full
	}else{
		*vid = i;
		ret = SUCCESS;
	}
    
	return ret;
}
#endif

int32 vlanTable_create(uint32 vid, rtl_vlan_param_t * param)
{
    vlan_table_t    entryContent;    
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
    int flag = FAILED;
#endif
#if 0		
    uint32	temp,temp2;    

    // disable interrupt
    // I don't know the reason but if you want to use "-O" flag, must disalbe interrupt before swTable_readEntry();
    temp = lx4180_ReadStatus();
    if (0 != (temp&0x1)) {
	    temp2 = temp&0xfffffffe;
	    lx4180_WriteStatus(temp2);
    }
#endif
		
    ASSERT_CSP(param);
    swTable_readEntry(TYPE_VLAN_TABLE, vid, &entryContent);

#if 0		
    // restore status register
    if (0 != (temp&0x1)) {
	    lx4180_WriteStatus(temp);
    }
#endif		

    bzero( (void *) &entryContent, sizeof(entryContent) );

#if 1
#define RTL8651_MAC_NUMBER				6
#define RTL8651_PORT_NUMBER				RTL8651_MAC_NUMBER
#define RTL8651_PHYSICALPORTMASK			((1<<RTL8651_MAC_NUMBER)-1)

    if(param->memberPort > RTL8651_PHYSICALPORTMASK )
        entryContent.extMemberPort = param->memberPort >> RTL8651_PORT_NUMBER;
    if(param->egressUntag > RTL8651_PHYSICALPORTMASK )
        entryContent.extEgressUntag = param->egressUntag >> RTL8651_PORT_NUMBER;	
    entryContent.memberPort = param->memberPort & RTL8651_PHYSICALPORTMASK;
    entryContent.egressUntag = param->egressUntag & RTL8651_PHYSICALPORTMASK;
#else
    entryContent.memberPort = param->memberPort & ALL_PORT_MASK;
    entryContent.egressUntag = param->egressUntag;
#endif
    entryContent.fid = param->fid;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
    entryContent.vid = vid & 0xfff;
    flag = rtl8651_getAsicVlanIndex(&entryContent, &vid);
    if(flag == FAILED)
        return FAILED;
#endif

    /* Write into hardware */
    if ( swTable_addEntry(TYPE_VLAN_TABLE, vid, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}






int32 vlanTable_destroy(uint32 vid)
{
    vlan_table_t    entryContent;

#if 0		
    uint32	temp,temp2;
    
    // disable interrupt
    // I don't know the reason but if you want to use "-O" flag, must disalbe interrupt before swTable_readEntry();
    temp = lx4180_ReadStatus();

    if (0 != (temp&0x1)) {
	    temp2 = temp&0xfffffffe;
	    lx4180_WriteStatus(temp2);
    }
#endif		

    swTable_readEntry(TYPE_VLAN_TABLE, vid, &entryContent);

#if 0		
    // restore status register
    if (0 != (temp&0x1)) {
	    lx4180_WriteStatus(temp);
    }
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
    int flag = FAILED;
    flag = rtl8651_findAsicVlanIndexByVid((uint16 *)&vid);
    if(flag == FAILED)
        return FAILED;
#endif
    
    bzero(&entryContent, sizeof(vlan_table_t));
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}






int32 vlanTable_setStpStatusOfAllPorts(uint32 vid, uint32 STPStatus)
{
    return 0;
}
#if defined (CONFIG_RTL_IGMP_SNOOPING)

int32 vlanPortmask_get(uint32 vid)
{
	vlan_table_t    entryContent;
	unsigned int 	vlanPortmask=0;
	uint16 index = vid;
	
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	rtl8651_findAsicVlanIndexByVid(&index);
#endif
	swTable_readEntry(TYPE_VLAN_TABLE, index, &entryContent);
	vlanPortmask =entryContent.memberPort;
	
	return vlanPortmask;
}
#endif


