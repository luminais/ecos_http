/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/vlanTable.h,v 1.1.1.1 2007/08/06 10:04:52 root Exp $
*
* Abstract: Switch core vlan table access header file.
*
* $Author: root $
*
* $Log: vlanTable.h,v $
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.4  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.3  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/

#ifndef _VLANTABLE_H_
#define _VLANTABLE_H_



/* VLAN table access routines 
*/

/* Create vlan 
Return: EEXIST- Speicified vlan already exists.
        ENFILE- Destined slot occupied by another vlan.*/
int vlanTable_create(unsigned int vid, rtl_vlan_param_t * param);

/* Destroy vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_destroy(unsigned int vid);

/* Add a member port
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_addMemberPort(unsigned int vid, unsigned int portNum);

/* Remove a member port 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_removeMemberPort(unsigned int vid, unsigned int portNum);

/* Set a member port list 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setMemberPort(unsigned int vid, unsigned int portList);

/* Set ACL rule 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setAclRule(unsigned int vid, unsigned int inACLStart, unsigned int inACLEnd,
                                unsigned int outACLStart, unsigned int outACLEnd);

/* Get ACL rule 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getAclRule(unsigned int vid, unsigned int *inACLStart_P, unsigned int *inACLEnd_P,
                                unsigned int *outACLStart_P, unsigned int *outACLEnd_P);

/* Set vlan as internal interface 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setInternal(unsigned int vid);

/* Set vlan as external interface 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setExternal(unsigned int vid);

/* Enable hardware routing for this vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_enableHardwareRouting(unsigned int vid);

/* Disable hardware routing for this vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_disableHardwareRouting(unsigned int vid);

/* Set spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setPortStpStatus(unsigned int vid, unsigned int portNum, unsigned int STPStatus);

/* Get spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getPortStpStatus(unsigned int vid, unsigned int portNum, unsigned int *STPStatus_P);

/* Set spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setStpStatus(unsigned int vid, unsigned int STPStatus);

/* Get information 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getInformation(unsigned int vid, rtl_vlan_param_t * param_P);

/* Get hardware information 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getHwInformation(unsigned int vid, rtl_vlan_param_t * param_P);

/* Get vlan id 
Return: ENOENT- Specified slot does not exist.*/
int vlanTable_getVidByIndex(unsigned int eidx, unsigned int * vid_P);

int32 swCore_aclCreate(uint32 idx, rtl_acl_param_t * rule);
int32 swCore_netifCreate(uint32 idx, rtl_netif_param_t * param);

#ifndef CONFIG_RTL865XC
#define CONFIG_RTL865XC 1
#endif
#ifdef CONFIG_RTL865XC
/* Hardware bit allocation of VLAN table 
*/
#if defined(CONFIG_RTL_8197F)
typedef struct {
#ifndef _LITTLE_ENDIAN
	 /* word 0 */
#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && !defined(CONFIG_RTL_8198C)
	uint32	vid:12;
#else
	uint32	reserved1:12;
#endif
	uint32	fid:2;
	uint32     extEgressUntag  : 3;
	uint32     egressUntag : 6;
	uint32     extMemberPort   : 3;
	uint32     memberPort  : 6;
#else /*_LITTLE_ENDIAN*/
	/* word 0 */
	
	uint32     memberPort  : 6;
	uint32     extMemberPort   : 3;
	uint32     egressUntag : 6;
	uint32     extEgressUntag  : 3;
	uint32	fid:2;
	uint32	reserved1:12;

#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} vlan_table_t;


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          mac18_0:19;
    uint32          vid		 : 12;
    uint32          valid       : 1;	
    /* word 1 */
#if defined(CONFIG_RTL_8198C)
    uint32          inACLStartL:1;
    uint32          enHWRouteV6    : 1;	
#else    
    uint32         inACLStartL:2;	
#endif
    uint32         enHWRoute : 1;	
    uint32         mac47_19:29;

    /* word 2 */
#if defined(CONFIG_RTL_8198C)
    uint32         macMaskL    :1;
    uint32         outACLEnd   :8;
    uint32         outACLStart :8;
    uint32         inACLEnd    :8;	
    uint32         inACLStartH :7;	
#else
    uint32         mtuL       : 3;
    uint32         macMask :3;	
    uint32         outACLEnd : 7;	
    uint32         outACLStart : 7;	
    uint32         inACLEnd : 7;	
    uint32         inACLStartH: 5;	
#endif

    /* word 3 */
#if defined(CONFIG_RTL_8198C)
    uint32         mtuV6       :15;
    uint32         mtu         :15;
    uint32         macMaskH    :2;
#else
    uint32          reserv10   : 20;
    uint32          mtuH       : 12;
#endif

#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32          valid       : 1;	
    uint32          vid		 : 12;
    uint32          mac18_0:19;

    /* word 1 */
    uint32         mac47_19:29;
    uint32          enHWRoute      : 1;	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32          enHWRouteV6    : 1;	
    uint32          inACLStartL:1;
#else
    uint32          inACLStartL:2;	
#endif


    /* word 2 */
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32         inACLStartH :7;	
    uint32         inACLEnd    :8;	
    uint32         outACLStart :8;
    uint32         outACLEnd   :8;
    uint32         macMaskL    :1;
#else
    uint32         inACLStartH : 5;	
    uint32         inACLEnd : 7;	
    uint32         outACLStart : 7;
    uint32         outACLEnd : 7;	
    uint32         macMask :3;
    uint32         mtuL       : 3;
#endif

    /* word 3 */
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32         macMaskH    :2;
    uint32         mtu         :15;
    uint32         mtuV6       :15;
#else
    uint32          mtuH       : 12;
    uint32          reserv10   : 20;
#endif

#endif /*_LITTLE_ENDIAN*/
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} netif_table_t;
#else
typedef struct {
	 /* word 0 */
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	unsigned int	vid:12;
#else
	unsigned int	reserved1:12;
#endif
	 
	unsigned int	fid:2;
	unsigned int     extEgressUntag  : 3;
	unsigned int     egressUntag : 6;
	unsigned int     extMemberPort   : 3;
	unsigned int     memberPort  : 6;

    /* word 1 */
    unsigned int          reservw1;
    /* word 2 */
    unsigned int          reservw2;
    /* word 3 */
    unsigned int          reservw3;
    /* word 4 */
    unsigned int          reservw4;
    /* word 5 */
    unsigned int          reservw5;
    /* word 6 */
    unsigned int          reservw6;
    /* word 7 */
    unsigned int          reservw7;
} vlan_table_t;

typedef struct {
    /* word 0 */
    unsigned int          mac18_0:19;
    unsigned int          vid		 : 12;
    unsigned int          valid       : 1;	
    /* word 1 */
    unsigned int         inACLStartL:2;	
    unsigned int         enHWRoute : 1;	
    unsigned int         mac47_19:29;

    /* word 2 */
    unsigned int         mtuL       : 3;
    unsigned int         macMask :3;	
    unsigned int         outACLEnd : 7;	
    unsigned int         outACLStart : 7;	
    unsigned int         inACLEnd : 7;	
    unsigned int         inACLStartH: 5;	
    /* word 3 */
    unsigned int          reserv10   : 20;
    unsigned int          mtuH       : 12;

    /* word 4 */
    unsigned int          reservw4;
    /* word 5 */
    unsigned int          reservw5;
    /* word 6 */
    unsigned int          reservw6;
    /* word 7 */
    unsigned int          reservw7;
} netif_table_t;
#endif

typedef struct {
#ifndef _LITTLE_ENDIAN
   union {
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
            uint16          dMacM15_0;
            uint16          dMacP47_32;
            /* word 2 */
            uint16          dMacM47_32;
            uint16          dMacM31_16;
            /* word 3 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 4 */
            uint16          sMacM15_0;
            uint16          sMacP47_32;
            /* word 5 */
            uint16          sMacM47_32;
            uint16          sMacM31_16;
            /* word 6 */
            uint16          ethTypeM;
            uint16          ethTypeP;
        } ETHERNET;
        struct {
            /* word 0 */
            unsigned int          reserv1     : 24;
            unsigned int          gidxSel     : 8;
            /* word 1~6 */
            unsigned int          reserv2[6];
        } IFSEL;
        struct {
            /* word 0 */
            ipaddr_t        sIPP;
            /* word 1 */
            ipaddr_t        sIPM;
            /* word 2 */
            ipaddr_t        dIPP;
            /* word 3 */
            ipaddr_t        dIPM;
            union {
                struct {
                    /* word 4 */
                    uint8           IPProtoM;
                    uint8           IPProtoP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    unsigned int          reserv0     : 20;
                    unsigned int          identSDIPM  : 1;
                    unsigned int          identSDIPP  : 1;
                    unsigned int          HTTPM       : 1;
                    unsigned int          HTTPP       : 1;
                    unsigned int          FOM         : 1;
                    unsigned int          FOP         : 1;
                    unsigned int          IPFlagM     : 3;
                    unsigned int          IPFlagP     : 3;
                    /* word 6 */
                    unsigned int          reserv1;
                } IP;
                struct {
                    /* word 4 */
                    uint8           ICMPTypeM;
                    uint8           ICMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          reserv0;
                    uint8           ICMPCodeM;
                    uint8           ICMPCodeP;
                    /* word 6 */
                    unsigned int          reserv1;
                } ICMP;
                struct {
                    /* word 4 */
                    uint8           IGMPTypeM;
                    uint8           IGMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5,6 */
                    unsigned int          reserv0[2];
                } IGMP;
                struct {
                    /* word 4 */
                    uint8           TCPFlagM;
                    uint8           TCPFlagP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          TCPSPLB;
                    uint16          TCPSPUB;
                    /* word 6 */
                    uint16          TCPDPLB;
                    uint16          TCPDPUB;
                } TCP;
                struct {
                    /* word 4 */
                    uint16          reserv0;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          UDPSPLB;
                    uint16          UDPSPUB;
                    /* word 6 */
                    uint16          UDPDPLB;
                    uint16          UDPDPUB;
                } UDP;
            } is;
        } L3L4;
#if defined(CONFIG_RTL_8198C)
		struct{
			union{
				struct{
					/* word 0 */
					uint32		  sip_addr31_0;
					/* word 1 */
					uint32		  sip_addr63_32;
					/* word 2 */
					uint32		  sip_addr95_64;
					/* word 3 */
					uint32		  sip_addr127_96;
					/* word 4 */
					uint32		  sip_mask31_0;
					/* word 5 */
					uint32		  sip_mask63_32;
					/* word 6 */
					uint32		  sip_mask95_64;			 
				}entry0;
				struct{
					/* word 0 */
					uint32		  dip_addr31_0;
					/* word 1 */
					uint32		  dip_addr63_32;
					/* word 2 */
					uint32		  dip_addr95_64;
					/* word 3 */
					uint32		  dip_addr127_96;
					/* word 4 */
					uint32		  dip_mask31_0;
					/* word 5 */
					uint32		  dip_mask63_32;
					/* word 6 */
					uint32		  dip_mask95_64;	 
				}entry1;
			}is;
		}L3V6;
#endif        

        struct {
            /* word 0 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 1 */
	     uint16           reserv1:3;
	     uint16		   spaP:9;
             uint16         sMacM3_0:4;
            uint16          sMacP47_32;
	/* word 2 */
	     unsigned int	          reserv3:2;
            unsigned int          sVidM:12;
	     unsigned int          sVidP:12;
	     unsigned int		   reserv2:6;
            /* word 3 */
            unsigned int          reserv5     : 6;
            unsigned int          protoType   : 2;
     	     unsigned int          reserv4        : 24;
		/* word 4 */
            ipaddr_t        sIPP;
            /* word 5 */
            ipaddr_t        sIPM;
            /* word 6 */
            uint16          SPORTLB;
            uint16          SPORTUB;
        } SRC_FILTER;
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
	     uint16 	   vidM:12;	
            uint16          dMacM3_0:4;
            uint16          dMacP47_32;			
            /* word 2 */
	     unsigned int          reserv2:20;
	     unsigned int          vidP:12;			
            /* word 3 */
            unsigned int          reserv4     : 24;
            unsigned int          protoType   : 2;
	     unsigned int          reserv3:6;
            /* word 4 */
            ipaddr_t        dIPP;
            /* word 5 */
            ipaddr_t        dIPM;
            /* word 6 */
            uint16          DPORTLB;
            uint16          DPORTUB;
        } DST_FILTER;

    } is;
    /* word 7 */
#if defined(CONFIG_RTL_8198C)
		uint32			ip_tunnel	: 1;
		uint32			comb	  : 1;
		uint32			ruleType1 : 1;
		uint32			ipv6ETY0  : 1;
		uint32			inv 	  : 1;
#else
    unsigned int          reserv0     : 5;
#endif
    unsigned int          pktOpApp    : 3;
    unsigned int          PPPoEIndex  : 3;
    unsigned int          vid         : 3;
    unsigned int          nextHop     : 10; //index of l2, next hop, or rate limit tables
    unsigned int          actionType  : 4;
    unsigned int          ruleType    : 4;
#if defined(CONFIG_RTL_8198C)
	struct{
		union{
			struct{
				/*word 8*/
				uint32 sip_mask119_96  :24;
				uint32 reserv1		   :8;
				/*word 9*/
				uint32 flowLabelM3_0   :4;
				uint32 flowLabel	   :20;
				uint32 sip_mask127_120 :8;
				/*word 10*/
				uint32 reserv2		   :16;
				uint32 flowLabelM19_4  :16;
			}entry0;
			struct{
				/*word 8*/
				uint32 dip_mask119_96  :24;
				uint32 reserv1		   :8;
				/*word 9*/
				uint32 nextHeader	   :8;
				uint32 trafficClassM   :8;				  
				uint32 trafficClass    :8;
				uint32 dip_mask127_120 :8;
				/*word 10*/ 			
				uint32 reserv2		   :20;
				uint32 identSDIPM	  :1;
				uint32 identSDIPP	   :1;				   
				uint32 HTTPM		  :1;
				uint32 HTTPP		   :1;				
				uint32 nextHeaderM	   :8;
			}entry1;
		}is;
	}ipv6;
#endif    

#else /* littlen endian*/
    union {
        struct {
            /* word 0 */
            uint16          dMacP15_0;
            uint16          dMacP31_16;
            /* word 1 */
            uint16          dMacP47_32;
            uint16          dMacM15_0;
            /* word 2 */
            uint16          dMacM31_16;
            uint16          dMacM47_32;
            /* word 3 */
            uint16          sMacP15_0;
            uint16          sMacP31_16;
            /* word 4 */
            uint16          sMacP47_32;
            uint16          sMacM15_0;
            /* word 5 */
            uint16          sMacM31_16;
            uint16          sMacM47_32;
            /* word 6 */
            uint16          ethTypeP;
            uint16          ethTypeM;
        } ETHERNET;
        struct {
            /* word 0 */
            uint32          gidxSel     : 8;
            uint32          reserv1     : 24;
            /* word 1~6 */
            uint32          reserv2[6];
        } IFSEL;
        struct {
            /* word 0 */
            ipaddr_t        sIPP;
            /* word 1 */
            ipaddr_t        sIPM;
            /* word 2 */
            ipaddr_t        dIPP;
            /* word 3 */
            ipaddr_t        dIPM;
            union {
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           IPProtoP;
                    uint8           IPProtoM;
                    /* word 5 */
                    uint32          IPFlagP     : 3;
                    uint32          IPFlagM     : 3;
                    uint32          FOP         : 1;
                    uint32          FOM         : 1;
                    uint32          HTTPP       : 1;
                    uint32          HTTPM       : 1;
                    uint32          identSDIPP  : 1;
                    uint32          identSDIPM  : 1;
                    uint32          reserv0     : 20;

                    /* word 6 */
                    uint32          reserv1;
                } IP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           ICMPTypeP;
                    uint8           ICMPTypeM;
                    /* word 5 */
                    uint8           ICMPCodeP;
                    uint8           ICMPCodeM;
                    uint16          reserv0;
                    /* word 6 */
                    uint32          reserv1;
                } ICMP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           IGMPTypeP;
                    uint8           IGMPTypeM;
                    /* word 5,6 */
                    uint32          reserv0[2];
                } IGMP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           TCPFlagP;
                    uint8           TCPFlagM;
                    /* word 5 */
                    uint16          TCPSPUB;
                    uint16          TCPSPLB;
                    /* word 6 */
                    uint16          TCPDPUB;
                    uint16          TCPDPLB;
                } TCP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint16          reserv0;
                    /* word 5 */
                    uint16          UDPSPUB;
                    uint16          UDPSPLB;
                    /* word 6 */
                    uint16          UDPDPUB;
                    uint16          UDPDPLB;
                } UDP;
            } is;
        } L3L4;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        struct{
            union{
                struct{
                    /* word 0 */
                    uint32        sip_addr31_0;
                    /* word 1 */
                    uint32        sip_addr63_32;
                    /* word 2 */
                    uint32        sip_addr95_64;
                    /* word 3 */
                    uint32        sip_addr127_96;
                    /* word 4 */
                    uint32        sip_mask31_0;
                    /* word 5 */
                    uint32        sip_mask63_32;
                    /* word 6 */
                    uint32        sip_mask95_64;             
                }entry0;
                struct{
                    /* word 0 */
                    uint32        dip_addr31_0;
                    /* word 1 */
                    uint32        dip_addr63_32;
                    /* word 2 */
                    uint32        dip_addr95_64;
                    /* word 3 */
                    uint32        dip_addr127_96;
                    /* word 4 */
                    uint32        dip_mask31_0;
                    /* word 5 */
                    uint32        dip_mask63_32;
                    /* word 6 */
                    uint32        dip_mask95_64;     
                }entry1;
            }is;
        }L3V6;
#endif

        struct {
            /* word 0 */
            uint16          sMacP15_0;
            uint16          sMacP31_16;
            /* word 1 */
            uint16          sMacP47_32;
            uint16          sMacM3_0:4;
	     uint16		   spaP:9;
	     uint16           reserv1:3;
            /* word 2 */
	     uint32		   reserv2:6;
	     uint32          sVidP:12;
            uint32          sVidM:12;
	     uint32	          reserv3:2;
	     
            /* word 3 */
            uint32          reserv4        : 24;
            uint32          protoType   : 2;
            uint32          reserv5     : 6;
            /* word 4 */
            ipaddr_t        sIPP;
            /* word 5 */
            ipaddr_t        sIPM;
            /* word 6 */
            uint16          SPORTUB;
            uint16          SPORTLB;
        } SRC_FILTER;
        struct {
            /* word 0 */
            uint16          dMacP15_0;
            uint16          dMacP31_16;

            /* word 1 */
            uint16          dMacP47_32;			
            uint16          dMacM3_0:4;
	     uint16 	   vidP:12;	
            /* word 2 */
	     uint32          vidM:12;			
	     uint32          reserv2:20;
            /* word 3 */
	     uint32          reserv3:6;
            uint32          protoType   : 2;
            uint32          reserv4     : 24;
            /* word 4 */
            ipaddr_t        dIPP;
            /* word 5 */
            ipaddr_t        dIPM;
            /* word 6 */
            uint16          DPORTUB;
            uint16          DPORTLB;
        } DST_FILTER;
#if defined(CONFIG_RTL_8197F)
		struct {
			union{
				struct{
					/* word 0 */            
					uint32  spub:16; ////sport upper bound
					uint32  splb:16; ////sport lower bound
					/* word 1 */
					uint32  dpub:16; ////dport upper bound
					uint32  dplb:16; ////dport lower bound
                    
					/* word 2 */
					uint32  protocol :2; 
					uint32  rsv      :6;
					uint32  offset0  :7;
					uint32  pattern0 :16;
					uint32  pm0_0    :1;

					/* word 3 */
					uint32  pm0_15_1     :15;
					uint32  or0      :1;
					uint32  offset1  :7;
					uint32  pattern1_8_0 :9; 
            
					/* word 4 */
					uint32  pattern1_15_9 :7; 
					uint32  pm1           :16;        
					uint32  or1          :1;
					uint32  offset2      :7;            
					uint32  pattern2_0    :1;

					/* word 5 */
					uint32  pattern2_15_1 :15;
					uint32  pm2          :16;
					uint32  or2          :1;

					/* word 6 */
					uint32  rsv1:32;             
				} entry0;
				struct{
					/* word 0 */
					uint32  offset4   :7;
					uint32  pattern4  :16;
					uint32  pm4_8_0   :9;

					/* word 1 */
					uint32  pm4_15_9      :7;
					uint32  or4           :1;            
					uint32  offset5  :7; 
					uint32  pattern5 :16; 
					uint32  pm5_0      :1;  

					/* word 2 */
					uint32  pm5_15_1     :15;
					uint32  or5          :1;
					uint32  offset6      :7; 
					uint32  pattern6_8_0 :9; 

					/* word 3 */
					uint32  pattern6_15_9 :7; 
					uint32  pm6           :16;        
					uint32  or6           :1;
					uint32  offset7       :7; 
					uint32  pattern7_0    :1;

					/* word 4 */
					uint32  pattern7_15_1 :15;
					uint32  pm7           :16;  
					uint32  or7           :1;

					/* word 5 */
					uint32  specialop     :1;
					uint32  spa           :9;
					uint32  spam          :9;
					uint32  pppctl        :1;
					uint32  pppctlm       :1;
					uint32  pppctl_or     :1;
					uint32  rsv2          :10;
					/* word 6 */
					uint32        rsv3;     
				}entry1;
			}is;
		}PM_U;//pattern match rule up        
#endif       
    } is;
    /* word 7 */

    uint32          ruleType    : 4;
    uint32          actionType  : 4;
    uint32          nextHop     : 10; //index of l2, next hop, or rate limit tables
    uint32          vid         : 3;
    uint32          PPPoEIndex  : 3;
    uint32          pktOpApp    : 3;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32          inv       : 1;
    uint32          ipv6ETY0  : 1;
    uint32          ruleType1 : 1;
    uint32          comb      : 1;
    uint32          ip_tunnel : 1;
#else
    uint32          reserv0     : 5;
#endif

#if defined(CONFIG_RTL_8198C)
    struct{
        union{
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 sip_mask119_96  :24;
                /*word 9*/
                uint32 sip_mask127_120 :8;
                uint32 flowLabel       :20;
                uint32 flowLabelM3_0   :4;
                /*word 10*/
                uint32 flowLabelM19_4  :16;
                uint32 reserv2         :16;
            }entry0;
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 dip_mask119_96  :24;
                /*word 9*/
                uint32 dip_mask127_120 :8;
                uint32 trafficClass    :8;
                uint32 trafficClassM   :8;                
                uint32 nextHeader      :8;
                /*word 10*/             
                uint32 nextHeaderM     :8;
                uint32 HTTPP           :1;
                uint32 HTTPM          :1;
                uint32 identSDIPP      :1;                
                uint32 identSDIPM     :1;
                uint32 reserv2         :20;
            }entry1;
        }is;
    }ipv6;
#endif

#if defined(CONFIG_RTL_8197F)
    union {
    struct{
        union{
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 sip_mask119_96  :24;
                /*word 9*/
                uint32 sip_mask127_120 :8;
                uint32 flowLabel       :20;
                uint32 flowLabelM3_0   :4;
                /*word 10*/
                uint32 flowLabelM19_4  :16;
                uint32 reserv2         :16;
            }entry0;
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 dip_mask119_96  :24;
                /*word 9*/
                uint32 dip_mask127_120 :8;
                uint32 trafficClass    :8;
                uint32 trafficClassM   :8;                
                uint32 nextHeader      :8;
                /*word 10*/             
                uint32 nextHeaderM     :8;
                uint32 HTTPP           :1;
                uint32 HTTPM          :1;
                uint32 identSDIPP      :1;                
                uint32 identSDIPM     :1;
                uint32 reserv2         :20;
            }entry1;
            }is;
        }ipv6;
    struct {
        union{
            struct{
                /* word 8 */            
                uint32 offset3  :7;
                uint32 pattern3 :16;
                uint32 pm3_8_0  :9;

                /* word 9 */
                uint32 pm3_15_9 :7;
                uint32 or3      :1;
                uint32 rsv      :24;                    

                /* word 10 */
                uint32 rsv1  ;
            }entry0;
            struct{
                /*word 8*/
                uint32 nexthopaddr2 :1;
                uint32 ipfrag_apply :1; ////todo maybe better coding style here.
                uint32 rsv          :30;
                /*word 9*/
                uint32 rsv1;                    

                /*word 10*/
                uint32 rsv2;        
            }entry1;
        }is;
    }PM_D;//pattern match rule down part
    } is_d;
#endif

#endif

} acl_table_t;

#endif /* RTL865XC */


#endif /*_VLANTABLE_H_*/
