/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/swCore.h,v 1.4 2010/03/02 10:11:43 bradhuang Exp $
*
* Abstract: Switch core header file.
*
* $Author: bradhuang $
*
* $Log: swCore.h,v $
* Revision 1.4  2010/03/02 10:11:43  bradhuang
* update for DIR615 package
*
* Revision 1.3  2010/02/09 04:45:51  pluswang
* for support ipv6 MLD
*
* Revision 1.2  2008/03/26 12:22:47  davidhsu
* Add private buffer management
*
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.5  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.4  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.3  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.2  2004/03/30 11:34:38  yjlou
* *: commit for 80B IC back
*   +: system clock rate definitions have changed.
*   *: fixed the bug of BIST_READY_PATTERN
*   +: clean all ASIC table when init ASIC.
* -: define _L2_MODE_ to support L2 switch mode.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.2  2004/03/16 06:04:04  yjlou
* +: support pure L2 switch mode (for hardware testing)
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/

#ifndef _SWCORE_H
#define _SWCORE_H

#ifndef CONFIG_RTL865XC
	#define CONFIG_RTL865XC 1
#endif

#define SWNIC_DEBUG
#define SWTABLE_DEBUG
#define SWCORE_DEBUG

#if defined(CONFIG_RTL8196C_AP_HCM) || defined(__ECOS)
#undef SUPPORT_IPV6_MLD //mark
#else
#define SUPPORT_IPV6_MLD		1
#endif

#if defined(CONFIG_RTL8196C_KLD)
#if defined(SUPPORT_IPV6_MLD)
#undef SUPPORT_IPV6_MLD
#endif
#endif
/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch core.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_init(int VlanCount);
#ifdef _L2_MODE_
int32 L2_swCore_config( uint8* gmac, uint32 for_RTL8650A );
#endif//_L2_MODE_


#ifdef CONFIG_RTL865XC

#define RTL_REFINE_ACL
#if defined(RTL_REFINE_ACL)
/* ACL Rule type Definition, 
  * sync from linux sdk 3410
  * Suggest that abandon the original acl rule type definition
  */
#define RTL865X_ACL_MAC				0x00
#define RTL865X_ACL_DSTFILTER_IPRANGE 0x01
#define RTL865X_ACL_IP					0x02
#define RTL865X_ACL_ICMP				0x04
#define RTL865X_ACL_IGMP				0x05
#define RTL865X_ACL_TCP					0x06
#define RTL865X_ACL_UDP				0x07
#define RTL865X_ACL_SRCFILTER			0x08
#define RTL865X_ACL_DSTFILTER			0x09
#define RTL865X_ACL_IP_RANGE			0x0A
#define RTL865X_ACL_SRCFILTER_IPRANGE 0x0B
#define RTL865X_ACL_ICMP_IPRANGE		0x0C
#define RTL865X_ACL_IGMP_IPRANGE 		0x0D
#define RTL865X_ACL_TCP_IPRANGE		0x0E
#define RTL865X_ACL_UDP_IPRANGE		0x0F
#if defined(CONFIG_RTL_8197F)
#define RTL865X_ACL_PM				0x10 /* pattern match*/
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL865X_ACL_IPV6			0x12 /* ipv6 mask */
#define RTL865X_ACL_IPV6_RANGE		0x1A
#endif

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
/*	dummy acl type for qos	*/
#define RTL865X_ACL_802D1P				0x1f
#endif

/* re-define old acl rule action, compatibility with older acl rule action define */
#define RTL8651_ACL_MAC				RTL865X_ACL_MAC
#define RTL8651_ACL_IP				RTL865X_ACL_IP
#define RTL8651_ACL_ICMP			RTL865X_ACL_ICMP
#define RTL8651_ACL_IGMP			RTL865X_ACL_IGMP
#define RTL8651_ACL_TCP				RTL865X_ACL_TCP
#define RTL8651_ACL_UDP				RTL865X_ACL_UDP
#define RTL8652_ACL_IP_RANGE			RTL865X_ACL_IP_RANGE
#define RTL8652_ACL_ICMP_IPRANGE		RTL865X_ACL_ICMP_IPRANGE
#define RTL8652_ACL_TCP_IPRANGE			RTL865X_ACL_TCP_IPRANGE
#define RTL8652_ACL_IGMP_IPRANGE		RTL865X_ACL_IGMP_IPRANGE
#define RTL8652_ACL_UDP_IPRANGE			RTL865X_ACL_UDP_IPRANGE
#define RTL8652_ACL_SRCFILTER_IPRANGE 	RTL865X_ACL_SRCFILTER_IPRANGE
#define RTL8652_ACL_DSTFILTER_IPRANGE 	RTL865X_ACL_DSTFILTER_IPRANGE


/* ACL Rule Action type Definition 
* sync from linux sdk 3410
* Suggest that abandon the original acl rule action type definaition
*/
#define RTL865X_ACL_PERMIT				0x00
#define RTL865X_ACL_REDIRECT_ETHER	0x01
#define RTL865X_ACL_DROP				0x02
#define RTL865X_ACL_TOCPU				0x03
#define RTL865X_ACL_LEGACY_DROP		0x04
#define RTL865X_ACL_DROPCPU_LOG		0x05
#define RTL865X_ACL_MIRROR				0x06
#define RTL865X_ACL_REDIRECT_PPPOE	0x07
#define RTL865X_ACL_DEFAULT_REDIRECT			0x08
#define RTL865X_ACL_MIRROR_KEEP_MATCH		0x09
#define RTL865X_ACL_DROP_RATE_EXCEED_PPS		0x0a
#define RTL865X_ACL_LOG_RATE_EXCEED_PPS		0x0b
#define RTL865X_ACL_DROP_RATE_EXCEED_BPS		0x0c
#define RTL865X_ACL_LOG_RATE_EXCEED_BPS		0x0d
#define RTL865X_ACL_PRIORITY					0x0e
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL865X_ACL_VID                     0x0f
#endif

/* re-define old acl rule action, compatibility with older acl rule action define */
#define RTL8651_ACL_PERMIT			RTL865X_ACL_PERMIT
#define RTL8651_ACL_DROP				RTL865X_ACL_DROP
#define RTL8651_ACL_CPU				RTL865X_ACL_TOCPU
//#define RTL8651_ACL_DROP_LOG			0x04
//#define RTL8651_ACL_DROP_NOTIFY		0x05
//#define RTL8651_ACL_L34_DROP			0x06	/* special for default ACL rule */

/* */
#define ACL_IPV6_TO_CPU			100
#define ACL_MC_3333FF_PERMIT		101
#define ACL_MC_3333_TO_CPU		102
#define ACL_MC_01005E_PERMIT		103

/* For PktOpApp */
#define RTL865X_ACL_ONLY_L2				1 /* Only for L2 switch */
#define RTL865X_ACL_ONLY_L3				2 /* Only for L3 routing (including IP multicast) */
#define RTL865X_ACL_L2_AND_L3			3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL865X_ACL_ONLY_L4				4 /* Only for L4 translation packets */
#define RTL865X_ACL_L3_AND_L4			6 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL865X_ACL_ALL_LAYER			7 /* No operation. Don't apply this rule. */

/* re-define old PktOpApp, compatibility with older acl PktOpApp define */
//#define RTL8651_ACLTBL_BACKWARD_COMPATIBLE	0 /* For backward compatible */
#define RTL865XC_ACLTBL_ALL_LAYER			RTL865X_ACL_ALL_LAYER
#define RTL8651_ACLTBL_ONLY_L2				RTL865X_ACL_ONLY_L2 /* Only for L2 switch */
#define RTL8651_ACLTBL_ONLY_L3				RTL865X_ACL_ONLY_L3 /* Only for L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_L2_AND_L3			RTL865X_ACL_L2_AND_L3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_ONLY_L4				RTL865X_ACL_ONLY_L4 /* Only for L4 translation packets */
#define RTL8651_ACLTBL_L3_AND_L4			RTL865X_ACL_L3_AND_L4 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL8651_ACLTBL_NOOP					RTL865X_ACL_ALL_LAYER /* No operation. Don't apply this rule. */


#else
/* ACL Rule Action type Definition */
#define RTL8651_ACL_PERMIT			0x01
#define RTL8651_ACL_DROP				0x02
#define RTL8651_ACL_CPU				0x03
#define RTL8651_ACL_DROP_LOG			0x04
#define RTL8651_ACL_DROP_NOTIFY		0x05
#define RTL8651_ACL_L34_DROP			0x06	/* special for default ACL rule */

/* */
#define ACL_IPV6_TO_CPU			100
#define ACL_MC_3333FF_PERMIT		101
#define ACL_MC_3333_TO_CPU		102
#define ACL_MC_01005E_PERMIT		103

/* ACL Rule type Definition */
#define RTL8651_ACL_MAC				0x00
#define RTL8651_ACL_IP					0x01
#define RTL8651_ACL_ICMP				0x02
#define RTL8651_ACL_IGMP				0x03
#define RTL8651_ACL_TCP					0x04
#define RTL8651_ACL_UDP				0x05
/* 6-8*/ 
#define RTL8652_ACL_IP_RANGE			0x0A
#define RTL8652_ACL_ICMP_IPRANGE		0x0B
#define RTL8652_ACL_TCP_IPRANGE		0x0C
#define RTL8652_ACL_IGMP_IPRANGE		0x0D
#define RTL8652_ACL_UDP_IPRANGE		0x0E
#define RTL8652_ACL_SRCFILTER_IPRANGE 0x09
#define RTL8652_ACL_DSTFILTER_IPRANGE 0x0F

/* For PktOpApp */
#define RTL8651_ACLTBL_BACKWARD_COMPATIBLE	0 /* For backward compatible */
#define RTL865XC_ACLTBL_ALL_LAYER			RTL8651_ACLTBL_BACKWARD_COMPATIBLE
#define RTL8651_ACLTBL_ONLY_L2				1 /* Only for L2 switch */
#define RTL8651_ACLTBL_ONLY_L3				2 /* Only for L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_L2_AND_L3			3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_ONLY_L4				4 /* Only for L4 translation packets */
#define RTL8651_ACLTBL_L3_AND_L4			6 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL8651_ACLTBL_NOOP				7 /* No operation. Don't apply this rule. */
#endif

/* MAC ACL rule Definition */
#define dstMac_				un_ty.MAC._dstMac
#define dstMacMask_			un_ty.MAC._dstMacMask
#define srcMac_				un_ty.MAC._srcMac
#define srcMacMask_			un_ty.MAC._srcMacMask
#define typeLen_				un_ty.MAC._typeLen
#define typeLenMask_			un_ty.MAC._typeLenMask

/* IFSEL ACL rule Definition */
#define gidxSel_				un_ty.IFSEL._gidxSel

/* Common IP ACL Rule Definition */
#define srcIpAddr_				un_ty.L3L4._srcIpAddr
#define srcIpAddrMask_			un_ty.L3L4._srcIpAddrMask
#define srcIpAddrUB_				un_ty.L3L4._srcIpAddr
#define srcIpAddrLB_				un_ty.L3L4._srcIpAddrMask
#define dstIpAddr_				un_ty.L3L4._dstIpAddr
#define dstIpAddrMask_			un_ty.L3L4._dstIpAddrMask
#define dstIpAddrUB_				un_ty.L3L4._dstIpAddr
#define dstIpAddrLB_				un_ty.L3L4._dstIpAddrMask
#define tos_					un_ty.L3L4._tos
#define tosMask_				un_ty.L3L4._tosMask
#if !defined(RTL_REFINE_ACL)
/* IP Rrange */
#define srcIpAddrStart_			un_ty.L3L4._srcIpAddr
#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddrMask
#define dstIpAddrStart_			un_ty.L3L4._dstIpAddr
#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddrMask
#else
/*Hyking:Asic use Addr to srore Upper address
	and use Mask to store Lower address
*/
#define srcIpAddrStart_			un_ty.L3L4._srcIpAddrMask
#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddr
#define dstIpAddrStart_			un_ty.L3L4._dstIpAddrMask
#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddr
#endif

/* IP ACL Rule Definition */
#define ipProto_				un_ty.L3L4.is.ip._proto
#define ipProtoMask_			un_ty.L3L4.is.ip._protoMask
#define ipFlagMask_			un_ty.L3L4.is.ip._flagMask
#if 1 //chhuang: #ifdef RTL8650B
#define ipFOP_      				un_ty.L3L4.is.ip._FOP
#define ipFOM_      				un_ty.L3L4.is.ip._FOM
#define ipHttpFilter_      			un_ty.L3L4.is.ip._httpFilter
#define ipHttpFilterM_			un_ty.L3L4.is.ip._httpFilterM
#define ipIdentSrcDstIp_   		un_ty.L3L4.is.ip._identSrcDstIp
#define ipIdentSrcDstIpM_		un_ty.L3L4.is.ip._identSrcDstIpM
#endif /* RTL8650B */
#define ipFlag_				un_ty.L3L4.is.ip.un._flag
#define ipDF_					un_ty.L3L4.is.ip.un.s._DF
#define ipMF_					un_ty.L3L4.is.ip.un.s._MF

#if defined(RTL_REFINE_ACL)
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
/* IPv6 ACL Rule Definition */
#define srcIpV6Addr_			un_ty.L3V6._srcIpV6Addr
#define srcIpV6AddrMask_		un_ty.L3V6._srcIpV6AddrMask
#define srcIpV6AddrUB_			un_ty.L3V6._srcIpV6Addr
#define srcIpV6AddrLB_			un_ty.L3V6._srcIpV6AddrMask
#define dstIpV6Addr_			un_ty.L3V6._dstIpV6Addr
#define dstIpV6AddrMask_		un_ty.L3V6._dstIpV6AddrMask
#define dstIpV6AddrUB_			un_ty.L3V6._dstIpV6Addr
#define dstIpV6AddrLB_			un_ty.L3V6._dstIpV6AddrMask
#define ipv6HttpFilter_      	un_ty.L3V6._httpFilter
#define ipv6HttpFilterM_	    un_ty.L3V6._httpFilterM
#define ipv6IdentSrcDstIp_   	un_ty.L3V6._identSrcDstIp
#define ipv6IdentSrcDstIpM_		un_ty.L3V6._identSrcDstIpM
#define ipv6FlowLabel_      	un_ty.L3V6._flowLabel
#define ipv6FlowLabelM_	        un_ty.L3V6._flowLabelMask
#define ipv6TrafficClass_      	un_ty.L3V6._trafficClass
#define ipv6TrafficClassM_	    un_ty.L3V6._trafficClassMask
#define ipv6NextHeader_      	un_ty.L3V6._nextheader
#define ipv6NextHeaderM_	    un_ty.L3V6._nextheaderMask
#endif

#if defined(CONFIG_RTL_8198C)
#define ipv6Invert_      	    un_ty.L3V6._INV
#define ipv6EntryType_     	    un_ty.L3V6._ETY
#define ipv6Combine_	        un_ty.L3V6._comb
#define ipv6IPtunnel_	        un_ty.L3V6._ip_tunnel
#elif defined(CONFIG_RTL_8197F)
#define ipv6Invert_      	    _INV
#define ipv6EntryType_     	    _ETY
#define ipv6Combine_	        _comb
#define ipv6IPtunnel_	        _ip_tunnel
#endif
#endif

/* ICMP ACL Rule Definition */
#define icmpType_				un_ty.L3L4.is.icmp._type
#define icmpTypeMask_			un_ty.L3L4.is.icmp._typeMask	
#define icmpCode_				un_ty.L3L4.is.icmp._code
#define icmpCodeMask_			un_ty.L3L4.is.icmp._codeMask

/* IGMP ACL Rule Definition */
#define igmpType_				un_ty.L3L4.is.igmp._type
#define igmpTypeMask_			un_ty.L3L4.is.igmp._typeMask

/* TCP ACL Rule Definition */
#define tcpl2srcMac_				un_ty.L3L4.is.tcp._l2srcMac		// for srcMac & destPort ACL rule
#define tcpl2srcMacMask_			un_ty.L3L4.is.tcp._l2srcMacMask
#define tcpSrcPortUB_			un_ty.L3L4.is.tcp._srcPortUpperBound
#define tcpSrcPortLB_			un_ty.L3L4.is.tcp._srcPortLowerBound
#define tcpDstPortUB_			un_ty.L3L4.is.tcp._dstPortUpperBound
#define tcpDstPortLB_			un_ty.L3L4.is.tcp._dstPortLowerBound
#define tcpFlagMask_			un_ty.L3L4.is.tcp._flagMask
#define tcpFlag_				un_ty.L3L4.is.tcp.un._flag
#define tcpURG_				un_ty.L3L4.is.tcp.un.s._urg
#define tcpACK_				un_ty.L3L4.is.tcp.un.s._ack
#define tcpPSH_				un_ty.L3L4.is.tcp.un.s._psh
#define tcpRST_				un_ty.L3L4.is.tcp.un.s._rst
#define tcpSYN_				un_ty.L3L4.is.tcp.un.s._syn
#define tcpFIN_				un_ty.L3L4.is.tcp.un.s._fin

/* UDP ACL Rule Definition */
#define udpl2srcMac_				un_ty.L3L4.is.udp._l2srcMac		// for srcMac & destPort ACL rule
#define udpl2srcMacMask_		un_ty.L3L4.is.udp._l2srcMacMask
#define udpSrcPortUB_			un_ty.L3L4.is.udp._srcPortUpperBound
#define udpSrcPortLB_			un_ty.L3L4.is.udp._srcPortLowerBound
#define udpDstPortUB_			un_ty.L3L4.is.udp._dstPortUpperBound
#define udpDstPortLB_			un_ty.L3L4.is.udp._dstPortLowerBound

#if 1 //chhuang: #ifdef RTL8650B
/* Source Filter ACL Rule Definition */
#define srcFilterMac_				un_ty.SRCFILTER._srcMac
#define srcFilterMacMask_		un_ty.SRCFILTER._srcMacMask
#define srcFilterPort_				un_ty.SRCFILTER._srcPort
#define srcFilterPortMask_		un_ty.SRCFILTER._srcPortMask
#define srcFilterVlanIdx_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanId_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanIdxMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterVlanIdMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterIpAddr_			un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrMask_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterIpAddrUB_		un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrLB_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterPortUpperBound_	un_ty.SRCFILTER._srcPortUpperBound
#define srcFilterPortLowerBound_	un_ty.SRCFILTER._srcPortLowerBound
#define srcFilterIgnoreL3L4_		un_ty.SRCFILTER._ignoreL3L4
#define srcFilterIgnoreL4_		un_ty.SRCFILTER._ignoreL4

/* Destination Filter ACL Rule Definition */
#define dstFilterMac_				un_ty.DSTFILTER._dstMac
#define dstFilterMacMask_		un_ty.DSTFILTER._dstMacMask
#define dstFilterVlanIdx_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdxMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterVlanId_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterIpAddr_			un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrMask_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortUpperBound_	un_ty.DSTFILTER._dstPortUpperBound
#define dstFilterIpAddrUB_		un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrLB_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortLowerBound_	un_ty.DSTFILTER._dstPortLowerBound
#define dstFilterIgnoreL3L4_		un_ty.DSTFILTER._ignoreL3L4
#define dstFilterIgnoreL4_		un_ty.DSTFILTER._ignoreL4
#endif /* RTL8650B */

#if defined(RTL_REFINE_ACL)
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
#define vlanTagPri_			un_ty.VLANTAG.vlanTagPri
#endif

#if defined(CONFIG_RTL_8197F)
#define pmSPUB_				    un_ty.PM._spub
#define pmSPLB_				    un_ty.PM._splb
#define pmDPUB_		            un_ty.PM._dpub
#define pmDPLB_				    un_ty.PM._dplb
#define pmProtocol_				un_ty.PM._protocol
#define pmSpecialop_		    un_ty.PM._specialop
#define pmSPA_		            un_ty.PM._spa
#define pmSPAM_				    un_ty.PM._spam
#define pmPPPCTL_    		    un_ty.PM._pppctl
#define pmPPPCTLM_	            un_ty.PM._pppctlm
#define pmPPPCTLOR_				un_ty.PM._pppctlor
#define pmOffset0_				un_ty.PM._offset0
#define pmPattern0_				un_ty.PM._pattern0
#define pmPatternMask0_		    un_ty.PM._pm0
#define pmOR0_				    un_ty.PM._or0
#define pmOffset1_				un_ty.PM._offset1
#define pmPattern1_				un_ty.PM._pattern1
#define pmPatternMask1_		    un_ty.PM._pm1
#define pmOR1_				    un_ty.PM._or1
#define pmOffset2_				un_ty.PM._offset2
#define pmPattern2_				un_ty.PM._pattern2
#define pmPatternMask2_		    un_ty.PM._pm2
#define pmOR2_				    un_ty.PM._or2
#define pmOffset3_				un_ty.PM._offset3
#define pmPattern3_				un_ty.PM._pattern3
#define pmPatternMask3_		    un_ty.PM._pm3
#define pmOR3_				    un_ty.PM._or3
#define pmOffset4_				un_ty.PM._offset4
#define pmPattern4_				un_ty.PM._pattern4
#define pmPatternMask4_		    un_ty.PM._pm4
#define pmOR4_				    un_ty.PM._or4
#define pmOffset5_				un_ty.PM._offset5
#define pmPattern5_				un_ty.PM._pattern5
#define pmPatternMask5_		    un_ty.PM._pm5
#define pmOR5_				    un_ty.PM._or5
#define pmOffset6_				un_ty.PM._offset6
#define pmPattern6_				un_ty.PM._pattern6
#define pmPatternMask6_		    un_ty.PM._pm6
#define pmOR6_				    un_ty.PM._or6
#define pmOffset7_				un_ty.PM._offset7
#define pmPattern7_				un_ty.PM._pattern7
#define pmPatternMask7_		    un_ty.PM._pm7
#define pmOR7_				    un_ty.PM._or7
#endif

//for compile error
#define pktOpApp 				pktOpApp_
#endif

/* ACL access parameters */
typedef struct {
	union {
		/* MAC ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _typeLen, _typeLenMask;
		} MAC; 
		/* IFSEL ACL rule */
		struct {
			uint8 _gidxSel;
		} IFSEL; 
		/* IP Group ACL rule */
		struct {
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint8 _tos, _tosMask;
			union {
				/* IP ACL rle */
				struct {
					uint8 _proto, _protoMask, _flagMask;// flag & flagMask only last 3-bit is meaning ful
#if 1 //chhuang: #ifdef RTL8650B
					uint32 _FOP:1, _FOM:1, _httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
#endif /* RTL8650B */
					union {
						uint8 _flag;
						struct {
							uint8 pend1:5,
								 pend2:1,
								 _DF:1,	//don't fragment flag
								 _MF:1;	//more fragments flag
						} s;
					} un;							
				} ip; 
				/* ICMP ACL rule */
				struct {
					uint8 _type, _typeMask, _code, _codeMask;
				} icmp; 
				/* IGMP ACL rule */
				struct {
					uint8 _type, _typeMask;
				} igmp; 
				/* TCP ACL rule */
				struct {
					ether_addr_t _l2srcMac, _l2srcMacMask;	// for srcMac & destPort ACL rule
					uint8 _flagMask;
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;
					union {
						uint8 _flag;
						struct {
							uint8 _pend:2,
								  _urg:1, //urgent bit
								  _ack:1, //ack bit
								  _psh:1, //push bit
								  _rst:1, //reset bit
								  _syn:1, //sync bit
								  _fin:1; //fin bit
						}s;
					}un;					
				}tcp; 
				/* UDP ACL rule */
				struct {
					ether_addr_t _l2srcMac, _l2srcMacMask;	// for srcMac & destPort ACL rule
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;										
				}udp; 
			}is;
		}L3L4;
		
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		struct
		{		 
			inv6_addr_t _srcIpV6Addr, _srcIpV6AddrMask;
			inv6_addr_t _dstIpV6Addr, _dstIpV6AddrMask;
			uint32 _INV:1,_ETY:1, _comb:1,_ip_tunnel:1 ,_httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
			uint32 _flowLabel, _flowLabelMask;
			uint8 _trafficClass, _trafficClassMask;
			uint8 _nextheader, _nextheaderMask;
		}L3V6;
#endif
 
#if 1 //chhuang: #ifdef RTL8650B
		/* Source filter ACL rule */
		struct {
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _srcPort, _srcPortMask;
			uint16 _srcVlanIdx, _srcVlanIdxMask;
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			uint16 _srcPortUpperBound, _srcPortLowerBound;
			uint32 _ignoreL3L4:1, //L2 rule
				  	 _ignoreL4:1; //L3 rule
		} SRCFILTER;
		/* Destination filter ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			uint16 _vlanIdx, _vlanIdxMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint16 _dstPortUpperBound, _dstPortLowerBound;
			uint32 _ignoreL4:1, //L3 rule
				   _ignoreL3L4:1; //L2 rule
		} DSTFILTER;
#endif /* RTL8650B */

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT) ||defined(CONFIG_RTL_MULTIPLE_WAN)  ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
		struct {
			uint8	vlanTagPri;
		} VLANTAG;
#endif
	
#if defined(CONFIG_RTL_8197F)
		/* Pattern Match ACL rule */
		struct {
			uint16 _spub, _splb ,_dpub, _dplb ;
			uint16 _protocol, _specialop ,_spa, _spam ;
			uint16	_pppctl ,_pppctlm, _pppctlor ;
			uint16 _offset0, _pattern0 ,_pm0, _or0 ;
			uint16 _offset1, _pattern1 ,_pm1, _or1 ;
			uint16 _offset2, _pattern2 ,_pm2, _or2 ;
			uint16 _offset3, _pattern3 ,_pm3, _or3 ;
			uint16 _offset4, _pattern4 ,_pm4, _or4 ;
			uint16 _offset5, _pattern5 ,_pm5, _or5 ;
			uint16 _offset6, _pattern6 ,_pm6, _or6 ;
			uint16 _offset7, _pattern7 ,_pm7, _or7 ;
		} PM;
#endif

	}un_ty;
#if defined(RTL_REFINE_ACL)
	uint32	ruleType_:5,
			actionType_:4,
			pktOpApp_:3,
			priority_:3,
			direction_:2,
#if defined(CONFIG_RTL_HW_QOS_SUPPORT) ||defined(CONFIG_RTL_MULTIPLE_WAN)  ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
			upDown_:1,//0: uplink acl rule for hw qos; 1: downlink acl rule for hw qos
#endif
#if defined(CONFIG_RTL_8197F)
			_INV:1,_ETY:1, _comb:1,_ip_tunnel:1,
#endif
			nexthopIdx_:5, /* Index of nexthop table (NOT L2 table) */	/* used as network interface index for 865xC qos system */
			ratelimtIdx_:4; /* Index of rate limit table */	/* used as outputQueue index for 865xC qos system */

	uint32	netifIdx_:3, /*for redirect*/
			pppoeIdx_:3, /*for redirect*/
			L2Idx_:10, /* Index of L2 table */
			inv_flag:8, /*mainly for iptables-->acl rule, when iptables rule has invert netif flag, this acl rule is added to other netifs*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			aclIdx:8;	/* aisc entry idx */
#else
			aclIdx:7;	/* aisc entry idx */
#endif

#if defined(CONFIG_RTL_8197F)
	uint32 aclvid_:12, /* vid if ActionType =1111 */
			aclfid_:2, /*  fid if ActionType =1111 */
			ipfrag_apply_:1;
#elif defined(CONFIG_RTL_8198C)
	uint32 aclvid_:12; /* vid if ActionType =1111 */
#endif

#else
	uint32	ruleType_:4;
	uint32	actionType_:4;
#if 1	/* RTL8650B */
	uint32  	pktOpApp:3;
#endif	/* RTL8650B */
	uint32	isEgressRateLimitRule_:1;
	uint32	naptProcessType:4;
	uint32	naptProcessDirection:2;
	uint32	matchType_;
	
	uint16  	dsid; /* 2004/1/19 orlando */
	uint16	priority:3;
	uint32	dvid_:3;
	uint32	priority_:1;
	uint32	nextHop_:10;
	uint32  	pppoeIdx_:3;
	uint32	isIPRange_:1;			/* support IP Range ACL */
	uint32	isRateLimitCounter_:1;	/* support Rate Limit Counter Mode */
#if 1 //chhuang: #ifdef RTL8650B
	uint16  	nhIndex; //index of next hop table
	uint16  	rlIndex; //index of rate limit table
#endif /* RTL8650B */
	uint32	aclIdx;
#endif
} rtl_acl_param_t;


/* Netif access parameters */
typedef struct {
    macaddr_t       gMac;
    uint16          macAddrNumber;
    uint16          vid;
    uint32          inAclStart, inAclEnd, outAclStart, outAclEnd;
    uint32          mtu;
    uint32          enableRoute:1,
                    valid:1;
} rtl_netif_param_t;
#endif


/* VLAN service
*/

#define RTL_STP_DISABLE 0
#define RTL_STP_BLOCK   1
#define RTL_STP_LEARN   2
#define RTL_STP_FORWARD 3


#ifdef CONFIG_RTL865XC
/* VLAN access parameters */
typedef struct {
    uint32          memberPort;
    uint32          egressUntag;
    uint32          fid: 2;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
    uint32          vid:12;
#endif
	
} rtl_vlan_param_t;
#endif



/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanCreate
 * --------------------------------------------------------------------
 * FUNCTION: This service creates a vlan.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
    EEXIST: Speicified vlan already exists.
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanCreate(uint32 vid, rtl_vlan_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanDestroy
 * --------------------------------------------------------------------
 * FUNCTION: This service destroys a vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanDestroy(uint32 vid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service sets port based vlan id.
 * INPUT   :
		portNum: Port number.
		pvid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPVid(uint32 portNum, uint32 pvid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port based vlan id.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : 
		pvid_P: Pointer to a variable to hold the PVid.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPVid(uint32 portNum, uint32 *pvid_P);

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetSTPStatusOfAllPorts
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status.
 * INPUT   :
		vid: Vlan ID.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetSTPStatusOfAllPorts(uint32 vid, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
 * OUTPUT  : 
		STPStatus_P: Pointer to a variable to hold the spanning tree 
		        status of the specified port.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 *STPStatus_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of the specified vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetInformation(uint32 vid, rtl_vlan_param_t * param_P);


/* Layer 2 service
*/

/* L2 forwarding table access parameters 
*/
typedef struct {
    macaddr_t       mac;
    uint16          isStatic    : 1;
    uint16          hPriority   : 1;
    uint16          toCPU       : 1;
    uint16          srcBlock    : 1;
    uint16          nxtHostFlag : 1;
    uint16          reserv0     : 11;
    uint32          memberPort;
    uint32          agingTime;
} rtl_l2_param_t;


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrAdd
 * --------------------------------------------------------------------
 * FUNCTION: This service adds the static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENFILE: Cannot allocate slot.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrAdd(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrRemove
 * --------------------------------------------------------------------
 * FUNCTION: This service removes the specified static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified MAC address does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrRemove(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   :
        entryIndex: Index of entry.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
        EEMPTY: Specified entry is empty.
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformation(uint32 entryIndex, rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformationByMac
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   : None.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformationByMac(rtl_l2_param_t * param_P);


/* Counter service 
*/

typedef struct {
    uint32  etherStatsOctets;
    uint32  etherStatsDropEvents;
    uint32  etherStatsCRCAlignErrors;
    uint32  etherStatsFragments;
    uint32  etherStatsJabbers;
    uint32  ifInUcastPkts;
    uint32  etherStatsMulticastPkts;
    uint32  etherStatsBroadcastPkts;
    uint32  etherStatsUndersizePkts;
    uint32  etherStatsPkts64Octets;
    uint32  etherStatsPkts65to127Octets;
    uint32  etherStatsPkts128to255Octets;
    uint32  etherStatsPkts256to511Octets;
    uint32  etherStatsPkts512to1023Octets;
    uint32  etherStatsPkts1024to1518Octets;
    uint32  etherStatsOversizepkts;
    uint32  dot3ControlInUnknownOpcodes;
    uint32  dot3InPauseFrames;
} rtl_ingress_counter_t;
typedef struct {
    uint32  ifOutOctets;
    uint32  ifOutUcastPkts;
    uint32  ifOutMulticastPkts;
    uint32  ifOutBroadcastPkts;
    uint32  dot3StatsLateCollisions;
    uint32  dot3StatsDeferredTransmissions;
    uint32  etherStatsCollisions;
    uint32  dot3StatsMultipleCollisionFrames;
    uint32  dot3OutPauseFrames;
} rtl_egress_counter_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : None.
 * OUTPUT  : 
        portList_P: List of member ports.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetMemberPort(uint32 *portList_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterSetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : 
        portList: List of member ports.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterSetMemberPort(uint32 portList);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetIngress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the ingress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the ingress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetIngress(rtl_ingress_counter_t *counters_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetEgress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the egress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the egress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetEgress(rtl_egress_counter_t *counters_P);


/* Port service 
*/

#define RTL_PORT_100M_FD        (1 << 0)
#define RTL_PORT_100M_HD        (1 << 1)
#define RTL_PORT_10M_FD         (1 << 2)
#define RTL_PORT_10M_HD         (1 << 3)

typedef struct {
    uint8   capableFlowCtrl : 1;
    uint8   capable100MFull : 1;
    uint8   capable100MHalf : 1;
    uint8   capable10MFull  : 1;
    uint8   capable10MHalf  : 1;
    uint8   reserv0         : 3;
} rtl_auto_nego_ability_t;
    
typedef struct {
    uint8   enAutoNego          : 1;
    uint8   enSpeed100M         : 1;
    uint8   enFullDuplex        : 1;
    uint8   enLoopback          : 1;
    uint8   linkEstablished     : 1;
    uint8   autoNegoCompleted   : 1;
    uint8   remoteFault         : 1;
    uint8   reserv0             : 1;
    rtl_auto_nego_ability_t   autoNegoAbility;
    rtl_auto_nego_ability_t   linkPartnerAutoNegoAbility;
    uint32  speedDuplex;
} rtl_port_status_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetSpeedDuplex
 * --------------------------------------------------------------------
 * FUNCTION: This service sets speed and duplex mode of specified port.
 * INPUT   :
		portNum: Port number.
		speedDuplex: Speed and duplex mode. Valid values are 
		    RTL_PORT_100M_FD, RTL_PORT_100M_HD, RTL_PORT_10M_FD and 
		    RTL_PORT_10M_HD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetSpeedDuplex(uint32 portNum, uint32 speedDuplex);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetAutoNegociationAbility
 * --------------------------------------------------------------------
 * FUNCTION: This service sets auto negociation pause, speed and duplex 
        mode capability of specified port.
 * INPUT   :
		portNum: Port number.
		anAbility_P: Pointer to the data structure which specifies the auto 
		    negociation abilities.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetAutoNegociationAbility(uint32 portNum, rtl_auto_nego_ability_t *anAbility_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portEnableAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service enables auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portEnableAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portDisableAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service disables auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portDisableAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portRestartAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service restarts auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portRestartAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to loopback mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portResetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to normal mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portResetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portGetStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port status of specified port.
 * INPUT   : 
		portNum: Port number.
 * OUTPUT  : 
    portStatus_P: Pointer to an area to hold the port status.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portGetStatus(uint32 portNum, rtl_port_status_t *portStatus_P);

extern void swCore_start(void);
extern void swCore_down(void);

#define swCore_vlanCreate vlanTable_create
#define swCore_vlanDestroy vlanTable_destroy
#define swCore_vlanSetPortSTPStatus vlanTable_setPortStpStatus
#define swCore_vlanSetSTPStatusOfAllPorts vlanTable_setStpStatusOfAllPorts
#define swCore_vlanGetPortSTPStatus vlanTable_getSTPStatus
#define swCore_vlanGetInformation vlanTable_getInformation

extern unsigned int ProbeWanPort(int probe);
extern void FullAndSemiReset(void);
int32 rtl8651_restartAsicEthernetPHYNway(uint32 port, uint32 phyid);
void set_phy_pwr_save(int id, int val);
void EasyACLRule(int index, int action);
int32 swCore_aclCreate(uint32 idx, rtl_acl_param_t * rule);

#ifdef CONFIG_RTL_8197D_DYN_THR	
extern int32 rtl819x_setQosThreshold(uint32 old_sts, uint32 new_sts);
extern unsigned int rtl865x_getPhysicalPortLinkStatus(void);
#endif

#endif /* _SWCORE_H */

