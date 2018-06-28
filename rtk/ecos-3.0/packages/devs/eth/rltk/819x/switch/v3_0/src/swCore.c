/*
 * Abstract: Switch core driver source code.
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *		All rights reserved.
 *
 */
 
#ifdef __KERNEL__
#include <linux/autoconf.h>
#include <linux/delay.h>
#endif

#ifdef __ECOS
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_switch.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#endif

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "phy.h"
#include "swTable.h"

#include "rtl865xC_tblAsicDrv.h"
#include "rtl8651_layer2.h"
#include "swNic_poll.h"
#ifdef CONFIG_RTL8196_RTL8366
#include "RTL8366RB_DRIVER/gpio.h"
#include "RTL8366RB_DRIVER/rtl8366rb_apiBasic.h"
#endif

#include "swCore.h"
#include <switch/rtl865x_fdb_api.h>
#include <switch/rtl865x_arp_api.h>
#include <switch/rtl865x_netif.h>

#ifdef CONFIG_RTL8198
#define CONFIG_RTL8198_REVISION_B 1
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)
extern int rtl_vlan_support_enable;
#endif

extern int savOP_MODE_value;

#if defined(CONFIG_RTL_8198_NFBI_BOARD)
 #define RTL8198_NFBI_PORT5_GMII 1  //undef it ,if test with MII mode
#define PORT5_PHYID 0x10
#endif

#if defined CONFIG_RTL_MLD_SNOOPING
/*for support ipv6 multicast snooping*/
void setACL_trap_ipv6_mcast(int index);
void setACL_fwd_ipv6_mcast(int index);
#endif


#if 1//defined (CONFIG_RTL_MLD_SNOOPING) || defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
void EasyACLRule(int index, int action);
#endif

#define BICOLOR_LED 1

#define RTL8651_ETHER_AUTO_100FULL	0x00
#define RTL8651_ETHER_AUTO_100HALF	0x01
#define RTL8651_ETHER_AUTO_10FULL		0x02
#define RTL8651_ETHER_AUTO_10HALF	0x03
#define RTL8651_ETHER_AUTO_1000FULL	0x08
#define RTL8651_ETHER_AUTO_1000HALF	0x09

#define tick_Delay10ms(x) { int i=x; while(i--) __delay(5000); }
// CONFIG_RTL_819XD
#define REG32_ANDOR(x,y,z)   (REG32(x)=(REG32(x)& (y))|(z))

#ifndef __ECOS
static int32 miiPhyAddress = -1;
#endif
extern unsigned int wan_port; 
extern const uint32 cPVCR[6][4];

#ifdef CONFIG_RTL8196C_ETH_IOT
extern uint32 port_link_sts, port_linkpartner_eee;
#endif

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
extern uint32 port_link_sts2;
#endif

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
#ifdef CONFIG_RTL8198
int eee_enabled = 0;
#else
int eee_enabled = 1;
#endif
#ifdef __KERNEL__
static void qos_init(void);
#endif
void eee_phy_enable(void);
void eee_phy_disable(void);
void eee_phy_enable_by_port(int port);
#endif

extern int bonding_type;

#ifdef CONFIG_RTL_8197D_DYN_THR
int32 rtl819x_setQosThreshold(uint32 old_sts, uint32 new_sts)
{
	int32 i, link_up_ports=0;
	uint32 j = new_sts;

	if (old_sts == new_sts)
		return SUCCESS;

	for (i=0; i<5; i++) {
		if ((j & 0x1) == 1)
			link_up_ports++;
		j = j >> 1;
	}

	if (link_up_ports <= DYN_THR_LINK_UP_PORTS) {
		WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_AGG_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_AGG_fcOFF << S_DSC_FCOFF_OFFSET));
		WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_AGG_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_AGG_sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	}
	else {
		WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_DEF_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_DEF_fcOFF << S_DSC_FCOFF_OFFSET));
		WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_DEF_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_DEF_sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	}

	return SUCCESS;
}

unsigned int rtl865x_getPhysicalPortLinkStatus(void)
{
	unsigned int port_num=0;
	unsigned int linkPortMask=0;

#if defined(CONFIG_RTL_8367R_SUPPORT)
	for(port_num=PHY0;port_num<=PHY4;port_num++)
	{
		extern int rtk_port_phyStatus_get(int port_num, int *pLinkStatus, int *pSpeed, int *pDuplex);
		int status, speed, duplex;

		if ((rtk_port_phyStatus_get(port_num, &status, &speed, &duplex)) == 0) {
			if (status == 1) {
				linkPortMask |= 1<<port_num;
			}
		}
	}
#else	
	for(port_num=0;port_num<=RTL8651_PHY_NUMBER;port_num++)
	{
		if((READ_MEM32(PSRP0+(port_num<<2))&PortStatusLinkUp)!=0)
		{
			linkPortMask |= 1<<port_num;
		}
	}
#endif
	return linkPortMask;
}
#endif

#ifdef CONFIG_RTK_VOIP_865xC_QOS
#if !defined(RTL865XC_MNQUEUE_OUTPUTQUEUE) && !defined(RTL865XC_QOS_OUTPUTQUEUE)
typedef struct  {
 	uint32  ifg;					// default: Bandwidth Control Include/exclude Preamble & IFG 
	uint32	gap;					// default: Per Queue Physical Length Gap = 20 
	uint32	drop;					// default: Descriptor Run Out Threshold = 500 

	uint32	systemSBFCOFF;				//*System shared buffer flow control turn off threshold
	uint32	systemSBFCON;				//*System shared buffer flow control turn on threshold

	uint32	systemFCOFF;				// system flow control turn off threshold 
	uint32	systemFCON;				// system flow control turn on threshold 

	uint32	portFCOFF;				// port base flow control turn off threshold 
	uint32	portFCON;				// port base flow control turn on threshold 	

	uint32	queueDescFCOFF;				// Queue-Descriptor=Based Flow Control turn off Threshold  
	uint32	queueDescFCON;				// Queue-Descriptor=Based Flow Control turn on Threshold  

	uint32	queuePktFCOFF;				// Queue-Packet=Based Flow Control turn off Threshold  
	uint32	queuePktFCON;				// Queue-Packet=Based Flow Control turn on Threshold  
}	rtl865xC_outputQueuePara_t;
#endif
#endif

#define GIGA_P0_PHYID	0x16  //for P0 8211C
/*
	0: UTP [embedded PHY]
	1: others [external PHY]
 */
#ifdef __ECOS
#ifdef CONFIG_RTL_8196C
static int ExtP0GigaPhyMode=0;
#endif
#else
#ifdef CONFIG_RTL8197B_PANA
static int ExtP0GigaPhyMode=1;
#else
static int ExtP0GigaPhyMode=0;
#endif
#endif

/*
	0x00: RGMII mode,
	0x01: GMII/MII MAC auto mode
	0x02: GMII/MII PHY auto mode
	0x10: RGMII, force mode   //8196B Mode, because sw MDC/MDIO fail
	0x11: GMII/MII mac, force mode
	0x12: GMII/MII phy,  force mode
 */
#ifdef __KERNEL__
static int SW_P0Mode=0x12;		//when ExtP0GigaPhyMode=1, this setting is valid
#endif

//------------------------------------------------------------------------
#ifndef __ECOS
unsigned int read_gpio_hw_setting(void)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE) || defined(CONFIG_RTL865X_CMO)
	return 0;
#endif

	unsigned int tmp, b2;

	/* 
		read GPIO PORT F3 to determine the LED mode, 0 LED scan mode, 1 LED matrix mode
		PEFGHCNR_REG: Port EFGH control, b.31-b.24 port H, b.23-b.16 port G, b.15-b.8 port F, b.7-b.0 port E 
		GPIOF[7]= HCI_RDY[0]
		GPIOF[6]= HCI_CS[0]
		GPIOF[5]= HCI_RDY[2]
		GPIOF[4]= HCI_RDY[1]
		GPIOF[3]= HCI_INT#  <<=========== 0 LED scan mode, 1 LED matrix mode
		GPIOF[2]= HCI_CS[2]
		GPIOF[1]= reserved
		GPIOF[0]= reserved
	*/
	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG) & (~(0x8<<8) ); //set (GP2)=(F3)= gpio
	REG32(PEFGHPTYPE_REG) = REG32(PEFGHPTYPE_REG) & (~(0x8<<8) );  //change to GPIO mode
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) & (~(0x8<<8) );  //0 input, 1 output, set  inpur
	tmp=REG32(PEFGHDAT_REG);
	b2 = (tmp&(0x08<<8))>>11;
	//printk(">> ................GPIO val=%08x.\n",b2); 
	return b2;	
}
#endif

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) 
{
    struct { uint32 _content[8]; } entry;
    uint32 idx;
    
    bzero(&entry, sizeof(entry));
    for (idx=0; idx<count; idx++)// Write into hardware
        swTable_addEntry(type, idx, &entry);

}

int32 rtl8651_clearAsicAllTable(void) 
{

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	REG32(MEMCR) = 0;
	REG32(MEMCR) = 0x7f;
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
#else
	_rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_VLAN_TABLE, RTL865XC_VLAN_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, RTL8651_ACLTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);	

	#if !defined(CONFIG_RTL8196C_REVISION_B) && !defined(CONFIG_RTL8198)
	// SERVER_PORT_TABLE, ALG_TABLE and L4_ICMP_TABLE are removed
	_rtl8651_clearSpecifiedAsicTable(TYPE_SERVER_PORT_TABLE, RTL8651_SERVERPORTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ALG_TABLE, RTL8651_ALGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_ICMP_TABLE, RTL8651_ICMPTBL_SIZE);
	#endif
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER
	rtl865x_reInitEventMgr();
#endif

 #ifdef CONFIG_RTL_LAYERED_DRIVER_L4
	rtl865x_nat_reinit();
#endif

 #ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	rtl865x_reinitRouteTable();
 	rtl865x_reinitNxtHopTable();
	rtl865x_reinitIpTable();
	rtl865x_reinitPppTable();
	rtl865x_arp_reinit();
 #endif
 #if defined (CONFIG_RTL_HARDWARE_MULTICAST)
 	rtl865x_reinitMulticast();
 #endif
  #ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_layer2_reinit();
 #endif

 #ifdef CONFIG_RTL_LAYERED_DRIVER
 	rtl865x_reinitNetifTable();
 	rtl8651_setAsicOperationLayer(4);
 #endif

	return 0;
}
#endif

#define CPUSSR 		0xb8019004		/* bit 31~16: reserved */
#define _MAGIC_FORM_BOOT		0x00007DE8

void FullAndSemiReset(void)
{
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	REG32(SIRR) |= FULL_RST;
	mdelay(300);

	//REG32(SYS_CLK_MAG) |= CM_PROTECT;  //protect bit=1
	REG32(SYS_CLK_MAG) &= ~CM_ACTIVE_SWCORE;  //active_swcore=0
	mdelay(300);
	REG32(SYS_CLK_MAG) |= CM_ACTIVE_SWCORE;  //active_swcore=1
	//REG32(SYS_CLK_MAG) &= ~CM_PROTECT;   //protect bit=0
	mdelay(50);
#else
	/* Perform full-reset for sw-core. */ 
	REG32(SIRR) |= FULL_RST;
	tick_Delay10ms(50);
#endif
	/* Enable TRXRDY */
	REG32(SIRR) |= TRXRDY;
}

#ifndef __ECOS
int32 rtl865xC_setAsicEthernetMIIMode(uint32 port, uint32 mode)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( mode != LINK_RGMII && mode != LINK_MII_MAC && mode != LINK_MII_PHY )
		return FAILED;

	if ( port == 0 )
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P0GMIICR, ( READ_MEM32( P0GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	else
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	return SUCCESS;

}

int32 rtl865xC_setAsicEthernetRGMIITiming(uint32 port, uint32 Tcomp, uint32 Rcomp)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( Tcomp < RGMII_TCOMP_0NS || Tcomp > RGMII_TCOMP_7NS || Rcomp < RGMII_RCOMP_0NS || Rcomp > RGMII_RCOMP_2DOT5NS )
		return FAILED;
	
	if ( port == 0 )
	{
		WRITE_MEM32(P0GMIICR, ( ( ( READ_MEM32(P0GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}
	else
	{
		WRITE_MEM32(P5GMIICR, ( ( ( READ_MEM32(P5GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetMII(uint32 phyAddress, int32 mode, int32 enabled)
{
	/* Input validation */
	if ( phyAddress < 0 || phyAddress > 31 )
		return FAILED;
	if ( mode != P5_LINK_RGMII && mode != P5_LINK_MII_MAC && mode != P5_LINK_MII_PHY )
		return FAILED;
	
	/* Configure driver level information about mii port 5 */
	if ( enabled )
	{
		if ( miiPhyAddress >= 0 && miiPhyAddress != phyAddress )
			return FAILED;

		miiPhyAddress = phyAddress;
	}
	else
	{
		miiPhyAddress = -1;
	}

	/* MII port MAC interface mode configuration */
	WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << P5_LINK_OFFSET ) );

	return SUCCESS;
}
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
extern int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA);
extern int RTL8367R_init(void);
extern int rtk_vlan_init(void);
#if defined(CONFIG_RTL_VLAN_SUPPORT)
extern int rtk_filter_igrAcl_init(void);
#endif

// for linking issue of \linux-2.6.30\net\rtl\fastpath\9xD\filter.S 
#if !defined(CONFIG_RTL_IGMP_SNOOPING)
int igmp_delete_init_netlink(void) { return 0; }
#endif

	#define REG_IOCFG_GPIO		0x00000018
	
	/* define GPIO port */
	enum GPIO_PORT
	{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
	};

void init_8367r(void)
{	
	int ret, i;

#ifdef CONFIG_RTL_8881A
	REG32(PIN_MUX_SEL2) |=  (3<<12); // reg_iocfg_fcs1, set to GPIO mode
	REG32(PABCD_CNR) &= (~(0x00004000)); //set GPIO pin, A1
	REG32(PABCD_DIR) |= (0x00004000); //output pin

	for (i=0; i<3; i++) 
	{
		// for 8367r h/w reset pin
		{
		REG32(PABCD_DAT) &= (~(0x00004000)); 
		mdelay(1000);
		REG32(PABCD_DAT) |= (0x00004000); 
		mdelay(1000);
		}
		// set to GPIO mode
		REG32(PIN_MUX_SEL)=0xc;// ((REG32(PIN_MUX_SEL)&(~(7<<2))|0x3<<2));

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~(0x00006000)));	//set GPIO pin, F5 and F6
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | ((0x00006000))); //output pin

		//smi_init(GPIO_PORT_A, 3, 2);
		smi_init(GPIO_PORT_F, 5, 6);
		mdelay(100); // 1000);
		ret = RTL8367R_init();
		if (ret == 0)
			break;
	}
#else	

	for (i=0; i<3; i++) {
		// for 8367r h/w reset pin
		REG32(PABCD_DAT) &= ~(0x00000002); 
		mdelay(1000);

		REG32(PABCD_DAT) |= (0x00000002); 
		mdelay(1000);

		// set to GPIO mode
		REG32(PIN_MUX_SEL) |= (REG_IOCFG_GPIO);

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~(0x00006000)));	//set GPIO pin, F5 and F6
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | ((0x00006000))); //output pin

		smi_init(GPIO_PORT_F, 5, 6);
		
		ret = RTL8367R_init();
		if (ret == 0) 
			break;
	}
#endif
	// 8367_VLAN
	 rtk_vlan_init();

	// clear mib counter
	rtk_stat_global_reset();

	RTL8367R_vlan_set(2);

	RTL8367R_cpu_tag(1);
#if defined(CONFIG_RTL_VLAN_SUPPORT)
	rtk_filter_igrAcl_init();
#endif
}
#endif

#if defined(CONFIG_RTL_8881A) && defined(CONFIG_RTL_8211E_SUPPORT)
int init_p0(void)
{
	#define GPIOA3				3
	#define PIN_MUX_SEL3		0xb800004c

	REG32(PCRP0) |=  (0x6 << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset;
	
	REG32(MACCR) |= (1<<12);
	REG32(P0GMIICR) |= ((1<<4) | (3<<0));
	REG32(P0GMIICR) |=(Conf_done);
	REG32(PITCR) |= (1<<0);

	// 0xb8000040 bit [4:2] Configure P0 mdc/mdio PAD as P0-MDIO (3'b100)
	// 0xb8000040 bit [9:7] Configure P0 PAD as P0-MII (3'b000)
	REG32(PIN_MUX_SEL) = (REG32(PIN_MUX_SEL) & ~((7<<7) | (7<<2))) | (4<<2);

	// Reset 8211 ==> GPIOA3, JTAG reset pin, pin_mux3 (0xb800004c) bit7:4 (reg_iocfg_jtag_trstn) = GPIO (4'b0110)
	REG32(PIN_MUX_SEL3) |= (6<<4);
	REG32(PABCD_CNR) &= ~(1<<GPIOA3);
	REG32(PABCD_DIR) |= (1<<GPIOA3);
	REG32(PABCD_DAT) &= ~(1<<GPIOA3);
	mdelay(500);
	REG32(PABCD_DAT) |= (1<<GPIOA3);
	mdelay(300);

	REG32(PITCR) |= (1<<0);
	REG32(P0GMIICR) |=(Conf_done);

	return 0;
}
#endif


int32 rtl8651_getAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 *rData)
{
	uint32 status;
	
	WRITE_MEM32( MDCIOCR, COMMAND_READ | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) );

#ifdef RTL865X_TEST
	status = READ_MEM32( MDCIOSR );
#else

#if defined(CONFIG_RTL8198)
	if (REG32(REVR) == BSP_RTL8198_REVISION_A) 
		mdelay(10);   
#elif defined(CONFIG_RTL8196C_REVISION_B)
	if (REG32(REVR) == RTL8196C_REVISION_A)
		mdelay(10);	//wei add, for 8196C revision A. mdio data read will delay 1 mdc clock.	
#endif

	do { status = READ_MEM32( MDCIOSR ); } while ( ( status & STATUS ) != 0 );
#endif

	status &= 0xffff;
	*rData = status;

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 wData)
{
	WRITE_MEM32( MDCIOCR, COMMAND_WRITE | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) | wData );

#ifdef RTL865X_TEST
#else
	while( ( READ_MEM32( MDCIOSR ) & STATUS ) != 0 );		/* wait until command complete */
#endif

	return SUCCESS;
}

int32 rtl8651_restartAsicEthernetPHYNway(uint32 port, uint32 phyid)
{
	uint32 statCtrlReg0;

	/* read current PHY reg 0 */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	/* enable 'restart Nway' bit */
	statCtrlReg0 |= RESTART_AUTONEGO;

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;
}

void tenda_reset_ports(void)
{
    uint32 statCtrlReg0, i = 0;

    for(i = 0; i < 5; i++){
    	rtl8651_getAsicEthernetPHYReg( i, 0, &statCtrlReg0 );
    	statCtrlReg0 |= RESTART_AUTONEGO;
    	rtl8651_setAsicEthernetPHYReg( i, 0, statCtrlReg0 );
    }
}


#ifndef __ECOS
int32 rtl8651_setAsicEthernetPHY(uint32 port, int8 autoNegotiation, uint32 advCapability, uint32 speed, int8 fullDuplex, 
	uint32 phyId, uint32 isGPHY) 
{
	uint32 statCtrlReg0, statCtrlReg4, statCtrlReg9;

	/* ====================
		Arrange PHY reg 0
	   ==================== */

	/* Read PHY reg 0 (control register) first */
	rtl8651_getAsicEthernetPHYReg(phyId, 0, &statCtrlReg0);

	if ( autoNegotiation == TRUE )	
	{
		statCtrlReg0 |= ENABLE_AUTONEGO;
	}
	else
	{
		statCtrlReg0 &= ~ENABLE_AUTONEGO;

		/* Clear speed & duplex setting */
		if ( isGPHY )
			statCtrlReg0 &= ~SPEED_SELECT_1000M;
		statCtrlReg0 &= ~SPEED_SELECT_100M;
		statCtrlReg0 &= ~SELECT_FULL_DUPLEX;

		if ( speed == 1 )	/* 100Mbps, assume 10Mbps by default */
			statCtrlReg0 |= SPEED_SELECT_100M;

		if ( fullDuplex == TRUE )
			statCtrlReg0 |= SELECT_FULL_DUPLEX;
	}

	/* =============================================================
		Arrange PHY reg 4, if GPHY, also need to arrange PHY reg 9.
	   ============================================================= */
	rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg4 );

	/* Clear all capability */
	statCtrlReg4 &= ~CAP_100BASE_MASK;

	if ( isGPHY )
	{
		rtl8651_getAsicEthernetPHYReg( phyId, 9, &statCtrlReg9 );

		/* Clear all 1000BASE capability */
		statCtrlReg9 &= ~ADVCAP_1000BASE_MASK;
	}
	else
	{
		statCtrlReg9 = 0;
	}
	
	if ( advCapability == RTL8651_ETHER_AUTO_1000FULL )
	{
		statCtrlReg9 = statCtrlReg9 | CAPABLE_1000BASE_TX_FD | CAPABLE_1000BASE_TX_HD;
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_1000HALF )
	{
		statCtrlReg9 = statCtrlReg9 | CAPABLE_1000BASE_TX_HD;
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_100FULL )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_100HALF )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_10FULL )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_10HALF )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_10BASE_TX_HD;
	}
	else
	{
//		RTL_WARN(RTL_MSG_GENERIC, "Invalid advertisement capability!");
		return FAILED;
	}

	/* ===============================
		Set PHY reg 4.
		Set PHY reg 9 if necessary.
	   =============================== */
	rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg4 );

	if ( isGPHY )
	{
		rtl8651_setAsicEthernetPHYReg( phyId, 9, statCtrlReg9 );
	}

	/* =================
		Set PHY reg 0.
	   ================= */
	rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg0 );

	/* =======================================================
		Restart Nway.
		If 'Nway enable' is FALSE, ASIC won't execute Nway.
	   ======================================================= */
	rtl8651_restartAsicEthernetPHYNway(port, phyId);

	return SUCCESS;
}

int32 rtl8651_setAsicFlowControlRegister(uint32 port, uint32 enable, uint32 phyid)
{
	uint32 statCtrlReg4;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4 );

	if ( enable && ( statCtrlReg4 & CAPABLE_PAUSE ) == 0 )
	{
		statCtrlReg4 |= CAPABLE_PAUSE;		
	}
	else if ( enable == 0 && ( statCtrlReg4 & CAPABLE_PAUSE ) )
	{
		statCtrlReg4 &= ~CAPABLE_PAUSE;
	}
	else
		return SUCCESS;	/* The configuration does not change. Do nothing. */

	rtl8651_setAsicEthernetPHYReg( phyid, 4, statCtrlReg4 );
	
	/* restart N-way. */
	rtl8651_restartAsicEthernetPHYNway(port, phyid);

	return SUCCESS;
}
#endif

#ifdef CONFIG_RTK_VOIP_865xC_QOS

void rtl865xC_set_DSCP(int SIP, int RTP) 
{
	//printk("enter rtl865xC_set_DSCP \n");
	//clear dscp priority assignment, otherwise pkt with dscp value 0 will be assign priority 1		
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);	         
	rtl8651_setAsicDscpPriority(RTP, PRI7);
	rtl8651_setAsicDscpPriority(SIP, PRI7);
	//printk("left rtl865xC_set_DSCP \n"); 
} 

//0 disable, 1 enable
void Enable_VOIP_QoS(int enable){
	/*
	effect throughput
	#ifdef CONFIG_RTK_VOIP_DRIVERS_WAN_PORT_0	
		REG32(FCCR0) &=0xfffffffd;//disable Qeueu 1 flow control
		REG32(FCCR0) |=0x20;	  //enable Qeueu 5 flow control
	#endif
	
	#ifdef CONFIG_FIX_WAN_TO_4	
		REG32(FCCR1) &=0xfffffffd;
		REG32(FCCR1) |=0x20;
	#endif
	*/
	if(enable)
	{
		REG32(FCCR1) &=0xfffdffff;//disable CPU Qeueu 1 flow control	
		REG32(PSRP6_RW) &= 0xffffffbf;
		printk("Enable QoS\n");
	}
	else
	{	REG32(FCCR1) |=0x20000;
		REG32(PSRP6_RW) |=0x40;	
		printk("Disable QoS\n");
	}
}

int32 rtl865x_QoS_mechanism()
{
	rtl865xC_outputQueuePara_t	outputQueuePara = {
	1, 	// default: Bandwidth Control Include/exclude Preamble & IFG 
	20, 	// default: Per Queue Physical Length Gap = 20 
	500, 	// default: Descriptor Run Out Threshold = 500 
	
	0xd4,   //default: System shared buffer flow control turn off threshold = 0x78 
	0xf8,	//default: System shared buffer flow control turn on threshold = 0x88 

	0x14a, 	//default: system flow control turn off threshold = 0x14a
	0x158,	//default: system flow control turn on threshold = 0x158
	
	0xf8,   //default: port base flow control turn off threshold = 0xf8
	0x108,  //default: port base flow control turn on threshold = 0x108
	
	0x14, 	//Queue-Descriptor=Based Flow Control turn off Threshold =0x14 
	0x21, 	//Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 
	
	0x03, 	// Queue-Packet=Based Flow Control turn off Threshold = 0x03 
	0x05	// Queue-Packet=Based Flow Control turn on Threshold =0x05 
	};
  	
    	REG32(QNUMCR) = P0QNum_6 | P1QNum_6 | P2QNum_6 | P3QNum_6 | P4QNum_6| P6QNum_6;

	printk("Qos threshold Start \n");
	/* Set the threshold value for qos sytem */
	int32 retval;
	int32	i,j;
	retval = rtl8651_setAsicSystemBasedFlowControlRegister(outputQueuePara.systemSBFCON, outputQueuePara.systemSBFCOFF, outputQueuePara.systemFCON, outputQueuePara.systemFCOFF, outputQueuePara.drop);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set System Base Flow Control Para Error.\n");
		return retval;
	}
	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		retval = rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(0, i, outputQueuePara.queueDescFCON, outputQueuePara.queueDescFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Descriptor Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(PHY0+j, i, outputQueuePara.queueDescFCON, outputQueuePara.queueDescFCOFF);


		retval = rtl8651_setAsicQueuePacketBasedFlowControlRegister(0, i, outputQueuePara.queuePktFCON, outputQueuePara.queuePktFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Packet Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueuePacketBasedFlowControlRegister(PHY0+j, i, outputQueuePara.queuePktFCON, outputQueuePara.queuePktFCOFF);

	}
// add for CPU port
	REG32(0xbb804570)= 0x140021;
	REG32(0xbb804574)= 0x140021;
	REG32(0xbb804578)= 0x140021;
	
	REG32(0xbb8045c4) = 0x038005;
	REG32(0xbb8045c8) = 0x038005;
	REG32(0xbb8045cc) = 0x038005;
		
	retval = rtl8651_setAsicPortBasedFlowControlRegister(0, outputQueuePara.portFCON, outputQueuePara.portFCOFF);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Port Base Flow Control Para Error.\n");
		return retval;
	}
	for(j=1;j<=CPU;j++)
		rtl8651_setAsicPortBasedFlowControlRegister(PHY0+j, outputQueuePara.portFCON, outputQueuePara.portFCOFF);
// add for CPU port	
	REG32(0xbb804524)= 0xf80108;
	retval = rtl8651_setAsicPerQueuePhysicalLengthGapRegister(outputQueuePara.gap);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Queue Physical Lenght Gap Reg Error.\n");
		return retval;
	}

	//clear dscp priority assignment, otherwise pkt with dscp value 0 will be assign priority 1		
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);			

	WRITE_MEM32(QRR,QRST);	

	//for 802.1Q mapping => Priority 1Q == absolutely priority 
	WRITE_MEM32(LPTM8021Q,0xFAC688);
	
	//For CPU port: Priority to Qid	
	WRITE_MEM32(CPUQIDMCR0,0x55432001);
	
	// open all port Queue 5 flow control
	WRITE_MEM32(FCCR0, 0x3f3f3f3f);
	WRITE_MEM32(FCCR1, 0x3f3f3f3f);
	
	printk("Qos threshold End \n");

	
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
extern int L2_table_disabled;
#endif

#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
void Set_GPHYWB(unsigned int phyid, unsigned int page, unsigned int reg, unsigned int mask, unsigned int val)
{
	unsigned int data=0;
	unsigned int wphyid=0;	//start
	unsigned int wphyid_end=1;   //end
	
	if(phyid==999)
	{	wphyid=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	wphyid=phyid;
		wphyid_end=phyid+1;
	}

	for(; wphyid<wphyid_end; wphyid++)
	{
		//change page 
		if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}

		if(mask != 0)
		{
			rtl8651_getAsicEthernetPHYReg( wphyid, reg, &data);
			data = data&mask;
		}
		rtl8651_setAsicEthernetPHYReg( wphyid, reg, data|val  );


		//switch to page 0
		rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
	}
}
#endif

#ifdef CONFIG_RTL8196C_REVISION_B

#ifdef CONFIG_RTL8196C_ETH_IOT
void set_gray_code_by_port(int port)
{
	uint32 val;
	
	rtl8651_setAsicEthernetPHYReg( 4, 31, 1  );
	
	rtl8651_getAsicEthernetPHYReg( 4, 20, &val  );		
	rtl8651_setAsicEthernetPHYReg( 4, 20, val + (0x1 << port)  );

	rtl8651_setAsicEthernetPHYReg( port, 31, 1  );

	rtl8651_setAsicEthernetPHYReg( port, 19,  0x5400 );
	if (port<4) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5440 );
	if (port<3) rtl8651_setAsicEthernetPHYReg( port, 19,  0x54c0 );
	if (port<2) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5480 );
	if (port<1) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5580 );

	rtl8651_setAsicEthernetPHYReg( 4, 20, 0xb20  );
	rtl8651_setAsicEthernetPHYReg( port, 31, 0  );
	
	rtl8651_setAsicEthernetPHYReg( 4, 31, 0  );
}
#endif

void Setting_RTL8196C_PHY(void)
{
	int i=0;
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
	  #=========ADC Bias Current =========================
	  #RG1X_P4~0 [12:10] = Reg_pi_fix [2:0], 5 ->7
	  phywb all 1 17 12-10 0x7      
	*/
	Set_GPHYWB(999, 1, 17, 0xffff-(7<<10), 0x7<<10);

	/*
	  #=========patch for eee============================
	  #1. page4¡Breg24¡Glpi_rx_ti_timer_cnt change to f3     
	  phywb all 4 24 7-0 0xf3

	  #2. page4¡Breg16¡Grg_txqt_ps_sel change to 1    
	  phywb all 4 16 3 1  
	*/
	Set_GPHYWB(999, 4, 24, 0xff00, 0xf3);
	Set_GPHYWB(999, 4, 16, 0xffff-(1<<3), 1<<3);
	/*	
	  #=========patch for IOL Tx amp.=====================      
	  #<a>modify 100M DAC current default value:
	  #Port#0~#4(per port control)
	  #Page1,Reg19,bit[13:11]: 
	  #original value 200uA(3b'100),--> change to 205u(3b'000)   => change to 205u(3b010)

	  phywb all 1 19 13-11 0x2

	  #<b>modify bandgap voltage default value:  
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg23,bit[8:6],                            

	  #original value 1.312V(3b'110),-->change to 1.212V(3b'100).       

	  phywb all 1 23 8-6 0x4

	  #<c>modify TX CS cap default value:                                   
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg18,bit[5:3],                                               

	  #original value Reserved bits(3b'000),-->change to 600fF(3b'011). =>change to 750fF(3b'110) 
	  phywb all 1 18 5-3 0x6
	*/

	Set_GPHYWB(999, 1, 19, 0xffff-(7<<11), 0x2<<11);
	Set_GPHYWB(999, 1, 23, 0xffff-(7<<6)  , 0x4<<6);
	Set_GPHYWB(999, 1, 18, 0xffff-(7<<3), 0x6<<3);
	

	/* 20100223 from Maxod: 100M half duplex enhancement */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_RXIPG_MASK) | 0x05;

	/* fix the link down / link up issue with SmartBit 3101B when DUT(8196c) set to Auto-negotiation 
	    and SmartBit force to 100M Full-duplex */
 	REG32(MACCR)= (REG32(MACCR) & ~SELIPG_MASK) | SELIPG_11;

	/*20100222 from Anson:Switch Corner test pass setting*/
	 /*
		REG21 default=0x2c5
		After snr_ub(page0 reg21.7-4) = 3 and snr_lb(page0 reg21.3-0)=2 ,REG21=0x232
		REG22 default=0x5b85
		After adtune_lb(page0 reg22.6-4)=4 (10uA) ,REG21=0x5b45

		REG0 default=0x1100
		restart AN
		page0 reg0.9 =1 , ,REG0=0x1300
	*/
	//rtl8651_setAsicEthernetPHYReg( i, 0x15, 0x232 );
	//Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0x32<<0);
	// test 96C to 96C restart AN 100 times, result is pass ==> page0 reg21.14(disable the equlizar)=1      
#ifdef CONFIG_RTL8196C_ETH_IOT
	// enable "equalizer reset", i.e. page 0 , reg21, bit14= 0
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x0032);
#else
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x4032);
#endif

	//rtl8651_setAsicEthernetPHYReg( i, 0x16, 0x5b45 );
	//Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x4<<4);
	Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x5<<4);
	//rtl8651_setAsicEthernetPHYReg( i, 0x0, 0x1300 );						
	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 0x1<<9);
	
	/*20100225 from Anson:Switch Force cailibration
	#change calibration update method for patch first pkt no update impedance
	phywb all 1 29 1 0
	#--------------Patch for impedance update fail cause rx crc error with long calbe-------- 
	#Froce cailibration
	phywb all 1 29 2 1
	#Force impedance value = 0x8888
	phywb all 1 28 15-0 0x8888			 	
	#-----------------------------------------------------------------------------------------
	#Select clock (ckt125[4]) edge trigger mlt3[1:0] = negative for patch four corner fail issue(only tx timing)
	phywb all 1 17 2-1 0x3
	*/
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<1), 0x0<<1);
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<2), 0x1<<2);
	//Set_GPHYWB(999, 1, 28, 0xffff-(0xffff), 0x8888);
	Set_GPHYWB(999, 1, 17, 0xffff-(3<<1), 0x3<<1);
		 		 
	/*20100222 from Yozen:AOI TEST pass setting*/
	Set_GPHYWB(999, 1, 18, 0xffff-(0xffff), 0x9004);				
		
	// for "DSP recovery fail when link partner = force 100F"
	Set_GPHYWB(999, 4, 26, 0xffff-(0xfff<<4), 0xff8<<4);

	// fix unlink IOT issue
	Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);
		
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
	
#ifdef CONFIG_RTL8196C_ETH_IOT
	for(i=0; i<5; i++) {
		set_gray_code_by_port(i);
	}
#endif
	printk("  Set 8196C PHY OK\n");

}
#endif

#ifdef CONFIG_RTL8198

static const unsigned int p[]={
//###################### PHY parameter patch ################################
0x1f, 0x0005, //Page 5
0x13, 0x0003, //Page 5 ########## EMA =3#############
//0x01, 0x0700; #Page 5 Reg 1 = 0x0700, NEC ON	(20100112)
0x05,0x8B82,//Page 5 Reg 5 = 0x8B82, Fix 100M re-link fail issue (20100110)
0x06,0x05CB,//Page 5 Reg 6 = 0x05CB, Fix 100M re-link fail issue (20100110)
0x1f,0x0002,//Page 2
0x04,0x80C2,//Page 2 Reg 4 0x80C2, Fix 100M re-link fail issue (20100110)
0x05,0x0938,//Page 2 Reg 5 0x0938, Disable 10M standby mode (20100112)

0x1F,0x0003,//Page 3
0x12,0xC4D2,//Page 3 Reg 18 = 0xC4D2, GAIN upper bond=24
0x0D,0x0207,//Page 3 Reg 13 = 0x0207 (20100112)
0x01,0x3554, //#Page 3 Reg  1 = 0x3554 (20100423)
0x02,0x63E8, //#Page 3 Reg  2 = 0x63E8 (20100423)
0x03,0x99C2, //#Page 3 Reg  3 = 0x99C2 (20100423)
0x04,0x0113, //#Page 3 Reg  4 = 0x0113 (20100423)

0x1f,0x0001,//Page 1
0x07,0x267E,//Page 1 Reg  7 = 0x267E, Channel Gain offset (20100111)
0x1C,0xE5F7,//Page 1 Reg 28 = 0xE5F7, Cable length offset (20100111)
0x1B,0x0424,//Page 1 Reg 27 = 0x0424, SD threshold (20100111)

//#Add by Gary for Channel Estimation fine tune 20100430            
//0x1f,0x0002, //# change to Page 1 (Global)
//0x08,0x0574, //# Page1 Reg8 (CG_INITIAL_MASTER)
//0x09,0x2724, //# Page1 Reg9 (CB0_INITIAL_GIGA) 
//0x1f,0x0003, //# change to Page 3 (Global)
//0x1a,0x06f6, //# Page3 Reg26 (CG_INITIAL_SLAVE)

//#Add by Gary for Channel Estimation fine tune 20100430            
//#Page1 Reg8 (CG_INITIAL_MASTER)
//0x1f, 0x0005,
//0x05, 0x83dd,
//0x06, 0x0574,
//#Page1 Reg9 (CB0_INITIAL_GIGA)
//0x1f, 0x0005,
//0x05, 0x83e0,
//0x06, 0x2724,
//#Page3 Reg26 (CG_INITIAL_SLAVE)
//0x1f, 0x0005,
//0x05, 0x843d,
//0x06, 0x06f6 ,

//#NC FIFO
0x1f,0x0007,//ExtPage
0x1e,0x0042,//ExtPage 66
0x18,0x0000,//Page 66 Reg 24 = 0x0000, NC FIFO (20100111)
0x1e,0x002D,//ExtPage 45
0x18,0xF010,//Page 45 Reg 24 = 0xF010, Enable Giga Down Shift to 100M (20100118)

0x1e,0x002c, //#ExtPage 44
0x18,0x008B, //#Page 44 Reg 24 = 0x008B, Enable deglitch circuit (20100426)


//############################ EEE giga patch ################################

//0x1f 0x0007;   
0x1e,0x0028,
0x16,0xf640,//phywb $phyID 40 22 15-0 0xF640

0x1e,0x0021,	
0x19,0x2929,//phywb $phyID 33 25 15-0 0x2929

0x1a,0x1005,//phywb $phyID 33 26 15-0 0x1005

0x1e,0x0020,	
0x17,0x000a,//phywb $phyID 32 23 15-0 0x000a

0x1b,0x2f4a,//Disable EEE PHY mode
0x15,0x0100,//EEE ability, Disable EEEP

0x1e,0x0040,//
0x1a,0x5110,//	phywb $phyID 64 26 15-0 0x5110
0x18,0x0000,// programable mode

0x1e,0x0041,//
0x15,0x0e02,//phywb $phyID 65 21 15-0 0x0e02

0x16,0x2185,//phywb $phyID 65 22 15-0 0x2185
0x17,0x000c,//phywb $phyID 65 23 15-0 0x000c
0x1c,0x0008,//phywb $phyID 65 28 15-0 0x0008
0x1e,0x0042,//
0x15,0x0d00,//phywb $phyID 66 21 15-0 0x0d00


#if 1
//############################ EEE Run code patch #################################
//###proc 67R_ram_code_20100211_inrx_uc_98_1
//###proc ram_code_0223_uc {} {
0x1f,0x0005,
0x05,0xfff6,
0x06,0x0080,

//### force MDI/MDIX
0x1f, 0x0007,
0x1e, 0x002d,
0x18, 0xf030,

//### pcs nctl patch code (0423)
0x1f, 0x0007,  
0x1e, 0x0023,    
0x16, 0x0005,    

//### startpoint
 0x15, 0x005c,
0x19, 0x0068,
0x15, 0x0082,
0x19, 0x000a,
0x15, 0x00a1,
0x19, 0x0081,
0x15 ,0x00af,
0x19, 0x0080,
0x15, 0x00d4,
0x19, 0x0000,
0x15, 0x00e4,
0x19, 0x0081,
0x15, 0x00e7,
0x19, 0x0080,
0x15, 0x010d,
0x19, 0x0083,
0x15, 0x0118,
0x19, 0x0083,
0x15, 0x0120,
0x19, 0x0082,
0x15, 0x019c,
0x19, 0x0081,
0x15, 0x01a4,
0x19, 0x0080,
0x15, 0x01cd,
0x19, 0x0000,
0x15, 0x01dd,
0x19, 0x0081,
0x15, 0x01e0,
0x19, 0x0080,
//### endpoint

0x16, 0x0000,    
//### end of pcs nctl patch code  


//inrx
0x1f, 0x0007,
0x1e, 0x0040,
0x18, 0x0004,
0x1f,0x0000,
0x17,0x2160,
0x1f,0x0007,
0x1e,0x0040,

//### startpoint
0x18,0x0004,
0x19,0x4000,
0x18,0x0014,
0x19,0x7f00,
0x18,0x0024,
0x19,0x0000,
0x18,0x0034,
0x19,0x0100,
0x18,0x0044,
0x19,0xe000,
0x18,0x0054,
0x19,0x0000,
0x18,0x0064,
0x19,0x0000,
0x18,0x0074,
0x19,0x0000,
0x18,0x0084,
0x19,0x0400,
0x18,0x0094,
0x19,0x8000,
0x18,0x00a4,
0x19,0x7f00,
0x18,0x00b4,
0x19,0x4000,
0x18,0x00c4,
0x19,0x2000,
0x18,0x00d4,
0x19,0x0100,
0x18,0x00e4,
0x19,0x8400,
0x18,0x00f4,
0x19,0x7a00,
0x18,0x0104,
0x19,0x4000,
0x18,0x0114,
0x19,0x3f00,
0x18,0x0124,
0x19,0x0100,
0x18,0x0134,
0x19,0x7800,
0x18,0x0144,
0x19,0x0000,
0x18,0x0154,
0x19,0x0000,
0x18,0x0164,
0x19,0x0000,
0x18,0x0174,
0x19,0x0400,
0x18,0x0184,
0x19,0x8000,
0x18,0x0194,
0x19,0x7f00,
0x18,0x01a4,
0x19,0x8300,
0x18,0x01b4,
0x19,0x8300,
0x18,0x01c4,
0x19,0xe200,
0x18,0x01d4,
0x19,0x0a00,
0x18,0x01e4,
0x19,0x8800,
0x18,0x01f4,
0x19,0x0300,
0x18,0x0204,
0x19,0xe100,
0x18,0x0214,
0x19,0x4600,
0x18,0x0224,
0x19,0x4000,
0x18,0x0234,
0x19,0x7f00,
0x18,0x0244,
0x19,0x0000,
0x18,0x0254,
0x19,0x0100,
0x18,0x0264,
0x19,0x4000,
0x18,0x0274,
0x19,0x3e00,
0x18,0x0284,
0x19,0x0000,
0x18,0x0294,
0x19,0xe000,
0x18,0x02a4,
0x19,0x1200,
0x18,0x02b4,
0x19,0x8000,
0x18,0x02c4,
0x19,0x7f00,
0x18,0x02d4,
0x19,0x8900,
0x18,0x02e4,
0x19,0x8300,
0x18,0x02f4,
0x19,0xe000,
0x18,0x0304,
0x19,0x0000,
0x18,0x0314,
0x19,0x4000,
0x18,0x0324,
0x19,0x7f00,
0x18,0x0334,
0x19,0x0000,
0x18,0x0344,
0x19,0x2000,
0x18,0x0354,
0x19,0x4000,
0x18,0x0364,
0x19,0x3e00,
0x18,0x0374,
0x19,0xfd00,
0x18,0x0384,
0x19,0x0000,
0x18,0x0394,
0x19,0x1200,
0x18,0x03a4,
0x19,0xab00,
0x18,0x03b4,
0x19,0x0c00,
0x18,0x03c4,
0x19,0x0600,
0x18,0x03d4,
0x19,0xa000,
0x18,0x03e4,
0x19,0x3d00,
0x18,0x03f4,
0x19,0xfb00,
0x18,0x0404,
0x19,0xe000,
0x18,0x0414,
0x19,0x0000,
0x18,0x0424,
0x19,0x4000,
0x18,0x0434,
0x19,0x7f00,
0x18,0x0444,
0x19,0x0000,
0x18,0x0454,
0x19,0x0100,
0x18,0x0464,
0x19,0x4000,
0x18,0x0474,
0x19,0xc600,
0x18,0x0484,
0x19,0xff00,
0x18,0x0494,
0x19,0x0000,
0x18,0x04a4,
0x19,0x1000,
0x18,0x04b4,
0x19,0x0200,
0x18,0x04c4,
0x19,0x7f00,
0x18,0x04d4,
0x19,0x4000,
0x18,0x04e4,
0x19,0x7f00,
0x18,0x04f4,
0x19,0x0200,
0x18,0x0504,
0x19,0x0200,
0x18,0x0514,
0x19,0x5200,
0x18,0x0524,
0x19,0xc400,
0x18,0x0534,
0x19,0x7400,
0x18,0x0544,
0x19,0x0000,
0x18,0x0554,
0x19,0x1000,
0x18,0x0564,
0x19,0xbc00,
0x18,0x0574,
0x19,0x0600,
0x18,0x0584,
0x19,0xfe00,
0x18,0x0594,
0x19,0x4000,
0x18,0x05a4,
0x19,0x7f00,
0x18,0x05b4,
0x19,0x0000,
0x18,0x05c4,
0x19,0x0a00,
0x18,0x05d4,
0x19,0x5200,
0x18,0x05e4,
0x19,0xe400,
0x18,0x05f4,
0x19,0x3c00,
0x18,0x0604,
0x19,0x0000,
0x18,0x0614,
0x19,0x1000,
0x18,0x0624,
0x19,0x8a00,
0x18,0x0634,
0x19,0x7f00,
0x18,0x0644,
0x19,0x4000,
0x18,0x0654,
0x19,0x7f00,
0x18,0x0664,
0x19,0x0100,
0x18,0x0674,
0x19,0x2000,
0x18,0x0684,
0x19,0x0000,
0x18,0x0694,
//### 0x2219:0x4600 =>0xe600 #0309
0x19,0xe600,
0x18,0x06a4,
//### 0x2219:0xfc00 =>0xff00 #0309
0x19,0xff00,
0x18,0x06b4,
0x19,0x0000,
0x18,0x06c4,
0x19,0x5000,
0x18,0x06d4,
0x19,0x9d00,
0x18,0x06e4,
0x19,0xff00,
0x18,0x06f4,
0x19,0x4000,
0x18,0x0704,
0x19,0x7f00,
0x18,0x0714,
0x19,0x0000,
0x18,0x0724,
0x19,0x2000,
0x18,0x0734,
0x19,0x0000,
0x18,0x0744,
0x19,0xe600,
0x18,0x0754,
0x19,0xff00,
0x18,0x0764,
0x19,0x0000,
0x18,0x0774,
0x19,0x5000,
0x18,0x0784,
0x19,0x8500,
0x18,0x0794,
0x19,0x7f00,
0x18,0x07a4,
0x19,0xac00,
0x18,0x07b4,
0x19,0x0800,
0x18,0x07c4,
0x19,0xfc00,
0x18,0x07d4,
0x19,0x4000,
0x18,0x07e4,
0x19,0x7f00,
0x18,0x07f4,
0x19,0x0400,
0x18,0x0804,
0x19,0x0200,
0x18,0x0814,
0x19,0x0000,
0x18,0x0824,
0x19,0xff00,
0x18,0x0834,
0x19,0x7f00,
0x18,0x0844,
0x19,0x0000,
0x18,0x0854,
0x19,0x4200,
0x18,0x0864,
0x19,0x0500,
0x18,0x0874,
0x19,0x9000,
0x18,0x0884,
0x19,0x8000,
0x18,0x0894,
0x19,0x7d00,
0x18,0x08a4,
0x19,0x8c00,
0x18,0x08b4,
0x19,0x8300,
0x18,0x08c4,
0x19,0xe000,
0x18,0x08d4,
0x19,0x0000,
0x18,0x08e4,
0x19,0x4000,
0x18,0x08f4,
0x19,0x0400,
0x18,0x0904,
0x19,0xff00,
0x18,0x0914,
0x19,0x0500,
0x18,0x0924,
0x19,0x8500,
0x18,0x0934,
0x19,0x8c00,
0x18,0x0944,
0x19,0xfa00,
0x18,0x0954,
0x19,0xe000,
0x18,0x0964,
0x19,0x0000,
0x18,0x0974,
0x19,0x4000,
0x18,0x0984,
0x19,0x5f00,
0x18,0x0994,
0x19,0x0400,
0x18,0x09a4,
0x19,0x0000,
0x18,0x09b4,
0x19,0xfe00,
0x18,0x09c4,
0x19,0x7300,
0x18,0x09d4,
0x19,0x0d00,
0x18,0x09e4,
0x19,0x0300,
0x18,0x09f4,
0x19,0x4000,
0x18,0x0a04,
0x19,0x2000,
0x18,0x0a14,
0x19,0x0000,
0x18,0x0a24,
0x19,0x0400,
0x18,0x0a34,
0x19,0xda00,
0x18,0x0a44,
0x19,0x0600,
0x18,0x0a54,
0x19,0x7d00,
0x18,0x0a64,
0x19,0x4000,
0x18,0x0a74,
0x19,0x5f00,
0x18,0x0a84,
0x19,0x0400,
0x18,0x0a94,
0x19,0x0000,
0x18,0x0aa4,
0x19,0x0000,
0x18,0x0ab4,
0x19,0x7300,
0x18,0x0ac4,
0x19,0x0d00,
0x18,0x0ad4,
0x19,0x0300,
0x18,0x0ae4,
0x19,0x0400,
0x18,0x0af4,
0x19,0xce00,
0x18,0x0b04,
0x19,0x0900,
0x18,0x0b14,
0x19,0x9d00,
0x18,0x0b24,
0x19,0x0800,
0x18,0x0b34,
0x19,0x9000,
0x18,0x0b44,
0x19,0x0700,
0x18,0x0b54,
0x19,0x7900,
0x18,0x0b64,
0x19,0x4000,
0x18,0x0b74,
0x19,0x7f00,
0x18,0x0b84,
0x19,0x0400,
0x18,0x0b94,
0x19,0x0000,
0x18,0x0ba4,
0x19,0x0000,
0x18,0x0bb4,
0x19,0x0400,
0x18,0x0bc4,
0x19,0x7300,
0x18,0x0bd4,
0x19,0x0d00,
0x18,0x0be4,
0x19,0x0100,
0x18,0x0bf4,
0x19,0x0900,
0x18,0x0c04,
0x19,0x8e00,
0x18,0x0c14,
0x19,0x0800,
0x18,0x0c24,
0x19,0x7d00,
0x18,0x0c34,
0x19,0x4000,
0x18,0x0c44,
0x19,0x7f00,
0x18,0x0c54,
0x19,0x0000,
0x18,0x0c64,
0x19,0x0000,
0x18,0x0c74,
0x19,0x0200,
0x18,0x0c84,
0x19,0x0000,
0x18,0x0c94,
0x19,0x7000,
0x18,0x0ca4,
0x19,0x0c00,
0x18,0x0cb4,
0x19,0x0100,
0x18,0x0cc4,
0x19,0x0900,
0x18,0x0cd4,
0x19,0x7f00,
0x18,0x0ce4,
0x19,0x4000,
0x18,0x0cf4,
0x19,0x7f00,
0x18,0x0d04,
0x19,0x3400,
0x18,0x0d14,
0x19,0x8300,
0x18,0x0d24,
0x19,0x8200,
0x18,0x0d34,
0x19,0x0000,
0x18,0x0d44,
0x19,0x7000,
0x18,0x0d54,
0x19,0x0d00,
0x18,0x0d64,
0x19,0x0100,
0x18,0x0d74,
0x19,0x0f00,
0x18,0x0d84,
0x19,0x7f00,
0x18,0x0d94,
0x19,0x9a00,
0x18,0x0da4,
0x19,0x7d00,
0x18,0x0db4,
0x19,0x4000,
0x18,0x0dc4,
0x19,0x7f00,
0x18,0x0dd4,
0x19,0x1400,
0x18,0x0de4,
0x19,0x0000,
0x18,0x0df4,
0x19,0x8200,
0x18,0x0e04,
0x19,0x0000,
0x18,0x0e14,
0x19,0x7000,
0x18,0x0e24,
0x19,0x0f00,
0x18,0x0e34,
0x19,0x0100,
0x18,0x0e44,
0x19,0x9b00,
0x18,0x0e54,
0x19,0x7f00,
0x18,0x0e64,
0x19,0x4000,
0x18,0x0e74,
0x19,0x1f00,
0x18,0x0e84,
0x19,0x0200,
0x18,0x0e94,
0x19,0x0600,
0x18,0x0ea4,
0x19,0x7100,
0x18,0x0eb4,
0x19,0x1d00,
0x18,0x0ec4,
0x19,0x0100,
0x18,0x0ed4,
0x19,0x4000,
0x18,0x0ee4,
0x19,0x1f00,
0x18,0x0ef4,
0x19,0x0200,
0x18,0x0f04,
0x19,0x0600,
0x18,0x0f14,
0x19,0x7100,
0x18,0x0f24,
0x19,0x0d00,
0x18,0x0f34,
0x19,0x0100,
0x18,0x0f44,
0x19,0x4000,
0x18,0x0f54,
0x19,0x1f00,
0x18,0x0f64,
0x19,0x0200,
0x18,0x0f74,
0x19,0x0600,
0x18,0x0f84,
0x19,0x7100,
0x18,0x0f94,
0x19,0x0d00,
0x18,0x0fa4,
0x19,0x0100,
0x18,0x0fb4,
0x19,0x4000,
0x18,0x0fc4,
0x19,0x1f00,
0x18,0x0fd4,
0x19,0x0200,
0x18,0x0fe4,
0x19,0x0600,
0x18,0x0ff4,
0x19,0x7100,
0x18,0x1004,
0x19,0x0d00,
0x18,0x1014,
0x19,0x0100,
0x18,0x1024,
0x19,0x4000,
0x18,0x1034,
0x19,0x1f00,
0x18,0x1044,
0x19,0x0200,
0x18,0x1054,
0x19,0x0600,
0x18,0x1064,
0x19,0x7100,
0x18,0x1074,
0x19,0x0d00,
0x18,0x1084,
0x19,0x0100,
0x18,0x1094,
0x19,0x4000,
0x18,0x10a4,
0x19,0x1f00,
0x18,0x10b4,
0x19,0x0200,
0x18,0x10c4,
0x19,0x0600,
0x18,0x10d4,
0x19,0x7100,
0x18,0x10e4,
0x19,0x0d00,
0x18,0x10f4,
0x19,0x0100,
0x18,0x1104,
0x19,0x4000,
0x18,0x1114,
0x19,0x7f00,
0x18,0x1124,
0x19,0x0400,
0x18,0x1134,
0x19,0x9000,
0x18,0x1144,
0x19,0x0200,
0x18,0x1154,
0x19,0x0600,
0x18,0x1164,
0x19,0x7300,
0x18,0x1174,
0x19,0x0d00,
0x18,0x1184,
0x19,0x0100,
0x18,0x1194,
0x19,0x0b00,
0x18,0x11a4,
0x19,0x9500,
0x18,0x11b4,
0x19,0x9400,
0x18,0x11c4,
0x19,0x0400,
0x18,0x11d4,
0x19,0x4000,
0x18,0x11e4,
0x19,0x4000,
0x18,0x11f4,
0x19,0x0500,
0x18,0x1204,
0x19,0x8000,
0x18,0x1214,
0x19,0x7800,
0x18,0x1224,
0x19,0x4000,
0x18,0x1234,
0x19,0x7f00,
0x18,0x1244,
0x19,0x0400,
0x18,0x1254,
0x19,0x0000,
0x18,0x1264,
0x19,0x0200,
0x18,0x1274,
0x19,0x0000,
0x18,0x1284,
0x19,0x7000,
0x18,0x1294,
0x19,0x0f00,
0x18,0x12a4,
0x19,0x0100,
0x18,0x12b4,
0x19,0x9b00,
0x18,0x12c4,
0x19,0x7f00,
0x18,0x12d4,
0x19,0xe100,
0x18,0x12e4,
0x19,0x1000,
0x18,0x12f4,
0x19,0x4000,
0x18,0x1304,
0x19,0x7f00,
0x18,0x1314,
0x19,0x0500,
0x18,0x1324,
0x19,0x0000,
0x18,0x1334,
0x19,0x0000,
0x18,0x1344,
0x19,0x0600,
0x18,0x1354,
0x19,0x7300,
0x18,0x1364,
0x19,0x0d00,
0x18,0x1374,
0x19,0x0100,
0x18,0x1384,
0x19,0x0400,
0x18,0x1394,
0x19,0x0600,
0x18,0x13a4,
0x19,0x4000,
0x18,0x13b4,
0x19,0x4000,
0x18,0x13c4,
0x19,0x0400,
0x18,0x13d4,
0x19,0xe000,
0x18,0x13e4,
0x19,0x7d00,
0x18,0x13f4,
0x19,0x0500,
0x18,0x1404,
0x19,0x7800,
0x18,0x1414,
0x19,0x4000,
0x18,0x1424,
0x19,0x4000,
0x18,0x1434,
0x19,0x0400,
0x18,0x1444,
0x19,0xe000,
0x18,0x1454,
0x19,0x9700,
0x18,0x1464,
0x19,0x4000,
0x18,0x1474,
0x19,0x7f00,
0x18,0x1484,
0x19,0x0000,
0x18,0x1494,
0x19,0x0100,
0x18,0x14a4,
0x19,0x4400,
0x18,0x14b4,
0x19,0x0000,
0x18,0x14c4,
0x19,0x0000,
0x18,0x14d4,
0x19,0x0000,
0x18,0x14e4,
0x19,0x4000,
0x18,0x14f4,
0x19,0x8000,
0x18,0x1504,
0x19,0x7f00,
0x18,0x1514,
0x19,0x4000,
0x18,0x1524,
0x19,0x3f00,
0x18,0x1534,
0x19,0x0400,
0x18,0x1544,
0x19,0x5000,
0x18,0x1554,
0x19,0xf800,
0x18,0x1564,
0x19,0x0000,
0x18,0x1574,
0x19,0xe000,
0x18,0x1584,
0x19,0x4000,
0x18,0x1594,
0x19,0x8000,
0x18,0x15a4,
0x19,0x7f00,
0x18,0x15b4,
0x19,0x8900,
0x18,0x15c4,
0x19,0x8300,
0x18,0x15d4,
0x19,0xe000,
0x18,0x15e4,
0x19,0x0000,
0x18,0x15f4,
0x19,0x4000,
0x18,0x1604,
0x19,0x7f00,
0x18,0x1614,
0x19,0x0200,
0x18,0x1624,
0x19,0x1000,
0x18,0x1634,
0x19,0x0000,
0x18,0x1644,
0x19,0xfc00,
0x18,0x1654,
0x19,0xfd00,
0x18,0x1664,
0x19,0x0000,
0x18,0x1674,
0x19,0x4000,
0x18,0x1684,
0x19,0xbc00,
0x18,0x1694,
0x19,0x0e00,
0x18,0x16a4,
0x19,0xfe00,
0x18,0x16b4,
0x19,0x8a00,
0x18,0x16c4,
0x19,0x8300,
0x18,0x16d4,
0x19,0xe000,
0x18,0x16e4,
0x19,0x0000,
0x18,0x16f4,
0x19,0x4000,
0x18,0x1704,
0x19,0x7f00,
0x18,0x1714,
0x19,0x0100,
0x18,0x1724,
0x19,0xff00,
0x18,0x1734,
0x19,0x0000,
0x18,0x1744,
//### 0x2219 : 0xfc00 ##0309
0x19,0xfc00,
0x18,0x1754,
//### 0x2219 : 0xff00 ##0309
0x19,0xff00,
0x18,0x1764,
0x19,0x0000,
0x18,0x1774,
0x19,0x4000,
0x18,0x1784,
0x19,0x9d00,
0x18,0x1794,
0x19,0xff00,
0x18,0x17a4,
0x19,0x4000,
0x18,0x17b4,
0x19,0x7f00,
0x18,0x17c4,
0x19,0x0000,
0x18,0x17d4,
0x19,0xff00,
0x18,0x17e4,
0x19,0x0000,
0x18,0x17f4,
0x19,0xfc00,
0x18,0x1804,
0x19,0xff00,
0x18,0x1814,
0x19,0x0000,
0x18,0x1824,
0x19,0x4000,
0x18,0x1834,
0x19,0x8900,
0x18,0x1844,
0x19,0x8300,
0x18,0x1854,
0x19,0xe000,
0x18,0x1864,
0x19,0x0000,
0x18,0x1874,
0x19,0xac00,
0x18,0x1884,
0x19,0x0800,
0x18,0x1894,
0x19,0xfa00,
0x18,0x18a4,
0x19,0x4000,
0x18,0x18b4,
0x19,0x7f00,
0x18,0x18c4,
0x19,0x0400,
0x18,0x18d4,
0x19,0x0200,
0x18,0x18e4,
0x19,0x0000,
0x18,0x18f4,
0x19,0xfd00,
0x18,0x1904,
0x19,0x7f00,
0x18,0x1914,
0x19,0x0000,
0x18,0x1924,
0x19,0x4000,
0x18,0x1934,
0x19,0x0500,
0x18,0x1944,
0x19,0x9000,
0x18,0x1954,
0x19,0x8000,
0x18,0x1964,
0x19,0x7d00,
0x18,0x1974,
0x19,0x8c00,
0x18,0x1984,
0x19,0x8300,
0x18,0x1994,
0x19,0xe000,
0x18,0x19a4,
0x19,0x0000,
0x18,0x19b4,
0x19,0x4000,
0x18,0x19c4,
0x19,0x0400,
0x18,0x19d4,
0x19,0xff00,
0x18,0x19e4,
0x19,0x0500,
0x18,0x19f4,
0x19,0x8500,
0x18,0x1a04,
0x19,0x8c00,
0x18,0x1a14,
0x19,0xfa00,
0x18,0x1a24,
0x19,0xe000,
0x18,0x1a34,
0x19,0x0000,
0x18,0x1a44,
0x19,0x4000,
0x18,0x1a54,
0x19,0x5f00,
0x18,0x1a64,
0x19,0x0400,
0x18,0x1a74,
0x19,0x0000,
0x18,0x1a84,
0x19,0xfc00,
0x18,0x1a94,
0x19,0x7300,
0x18,0x1aa4,
0x19,0x0d00,
0x18,0x1ab4,
0x19,0x0100,
0x18,0x1ac4,
0x19,0x4000,
0x18,0x1ad4,
0x19,0x2000,
0x18,0x1ae4,
0x19,0x0000,
0x18,0x1af4,
0x19,0x0400,
0x18,0x1b04,
0x19,0xda00,
0x18,0x1b14,
0x19,0x0600,
0x18,0x1b24,
0x19,0x7d00,
0x18,0x1b34,
0x19,0x4000,
0x18,0x1b44,
0x19,0x5f00,
0x18,0x1b54,
0x19,0x0400,
0x18,0x1b64,
0x19,0x0000,
0x18,0x1b74,
0x19,0x0000,
0x18,0x1b84,
0x19,0x7300,
0x18,0x1b94,
0x19,0x0d00,
0x18,0x1ba4,
0x19,0x0100,
0x18,0x1bb4,
0x19,0x0400,
0x18,0x1bc4,
0x19,0xce00,
0x18,0x1bd4,
0x19,0x0800,
0x18,0x1be4,
0x19,0x9200,
0x18,0x1bf4,
0x19,0x0900,
0x18,0x1c04,
0x19,0x9b00,
0x18,0x1c14,
0x19,0x0700,
0x18,0x1c24,
0x19,0x7900,
0x18,0x1c34,
0x19,0x4000,
0x18,0x1c44,
0x19,0x7f00,
0x18,0x1c54,
0x19,0x0400,
0x18,0x1c64,
0x19,0x0000,
0x18,0x1c74,
0x19,0x0000,
0x18,0x1c84,
0x19,0x0400,
0x18,0x1c94,
0x19,0x7300,
0x18,0x1ca4,
0x19,0x0d00,
0x18,0x1cb4,
0x19,0x0100,
0x18,0x1cc4,
0x19,0x0900,
0x18,0x1cd4,
0x19,0x8e00,
0x18,0x1ce4,
0x19,0x0800,
0x18,0x1cf4,
0x19,0x7d00,
0x18,0x1d04,
0x19,0x4000,
0x18,0x1d14,
0x19,0x7f00,
0x18,0x1d24,
0x19,0x0000,
0x18,0x1d34,
0x19,0x0000,
0x18,0x1d44,
0x19,0x0000,
0x18,0x1d54,
0x19,0x0000,
0x18,0x1d64,
0x19,0x7000,
0x18,0x1d74,
0x19,0x0c00,
0x18,0x1d84,
0x19,0x0100,
0x18,0x1d94,
0x19,0x0900,
0x18,0x1da4,
0x19,0x7f00,
0x18,0x1db4,
0x19,0x4000,
0x18,0x1dc4,
0x19,0x7f00,
0x18,0x1dd4,
0x19,0x0400,
0x18,0x1de4,
0x19,0x0000,
0x18,0x1df4,
0x19,0x0000,
0x18,0x1e04,
0x19,0x0000,
0x18,0x1e14,
0x19,0x7000,
0x18,0x1e24,
0x19,0x0d00,
0x18,0x1e34,
0x19,0x0100,
0x18,0x1e44,
0x19,0x0b00,
0x18,0x1e54,
0x19,0x7f00,
0x18,0x1e64,
0x19,0x9a00,
0x18,0x1e74,
0x19,0x7f00,
0x18,0x1e84,
0x19,0x4000,
0x18,0x1e94,
0x19,0x7f00,
0x18,0x1ea4,
0x19,0x0400,
0x18,0x1eb4,
0x19,0x0000,
0x18,0x1ec4,
0x19,0x0000,
0x18,0x1ed4,
0x19,0x0000,
0x18,0x1ee4,
0x19,0x7100,
0x18,0x1ef4,
0x19,0x0d00,
0x18,0x1f04,
0x19,0x0100,
0x18,0x1f14,
0x19,0x9400,
0x18,0x1f24,
0x19,0x7f00,
0x18,0x1f34,
0x19,0x4000,
0x18,0x1f44,
0x19,0x7f00,
0x18,0x1f54,
0x19,0x0500,
0x18,0x1f64,
0x19,0x0000,
0x18,0x1f74,
0x19,0x0000,
0x18,0x1f84,
0x19,0x0000,
0x18,0x1f94,
0x19,0x7300,
0x18,0x1fa4,
0x19,0x0d00,
0x18,0x1fb4,
0x19,0x0100,
0x18,0x1fc4,
0x19,0x0500,
0x18,0x1fd4,
0x19,0x8800,
0x18,0x1fe4,
0x19,0x0400,
0x18,0x1ff4,
0x19,0x7d00,
0x18,0x2004,
0x19,0x4000,
0x18,0x2014,
0x19,0x4000,
0x18,0x2024,
0x19,0x0400,
0x18,0x2034,
0x19,0xe100,
0x18,0x2044,
0x19,0x8a00,
0x18,0x2054,
0x19,0x4000,
0x18,0x2064,
0x19,0x4000,
0x18,0x2074,
0x19,0x0400,
0x18,0x2084,
0x19,0xe100,
0x18,0x2094,
0x19,0xa400,
0x18,0x20a4,
0x19,0x4000,
0x18,0x20b4,
0x19,0x7f00,
0x18,0x20c4,
0x19,0x0000,
0x18,0x20d4,
0x19,0x0100,
0x18,0x20e4,
0x19,0x4000,
0x18,0x20f4,
0x19,0x3e00,
0x18,0x2104,
0x19,0x0000,
0x18,0x2114,
0x19,0xe000,
0x18,0x2124,
0x19,0x1200,
0x18,0x2134,
0x19,0x8000,
0x18,0x2144,
0x19,0x7f00,
0x18,0x2154,
0x19,0x8900,
0x18,0x2164,
0x19,0x8300,
0x18,0x2174,
0x19,0xe000,
0x18,0x2184,
0x19,0x0000,
0x18,0x2194,
0x19,0x4000,
0x18,0x21a4,
0x19,0x7f00,
0x18,0x21b4,
0x19,0x0000,
0x18,0x21c4,
0x19,0x2000,
0x18,0x21d4,
0x19,0x4000,
0x18,0x21e4,
0x19,0x3e00,
0x18,0x21f4,
0x19,0xff00,
0x18,0x2204,
0x19,0x0000,
0x18,0x2214,
0x19,0x1200,
0x18,0x2224,
0x19,0x8000,
0x18,0x2234,
0x19,0x7f00,
0x18,0x2244,
0x19,0x8600,
0x18,0x2254,
0x19,0x8500,
0x18,0x2264,
0x19,0x8900,
0x18,0x2274,
0x19,0xfd00,
0x18,0x2284,
0x19,0xe000,
0x18,0x2294,
0x19,0x0000,
0x18,0x22a4,
0x19,0x9500,
0x18,0x22b4,
0x19,0x0400,
0x18,0x22c4,
0x19,0x4000,
0x18,0x22d4,
0x19,0x4000,
0x18,0x22e4,
0x19,0x1000,
0x18,0x22f4,
0x19,0x4000,
0x18,0x2304,
0x19,0x3f00,
0x18,0x2314,
0x19,0x0200,
0x18,0x2324,
0x19,0x4000,
0x18,0x2334,
0x19,0x3700,
0x18,0x2344,
0x19,0x7f00,
0x18,0x2354,
0x19,0x0000,
0x18,0x2364,
0x19,0x0200,
0x18,0x2374,
0x19,0x0200,
0x18,0x2384,
0x19,0x9000,
0x18,0x2394,
0x19,0x8000,
0x18,0x23a4,
0x19,0x7d00,
0x18,0x23b4,
0x19,0x8900,
0x18,0x23c4,
0x19,0x8300,
0x18,0x23d4,
0x19,0xe000,
0x18,0x23e4,
0x19,0x0000,
0x18,0x23f4,
0x19,0x4000,
0x18,0x2404,
0x19,0x0400,
0x18,0x2414,
0x19,0xff00,
0x18,0x2424,
0x19,0x0200,
0x18,0x2434,
0x19,0x8500,
0x18,0x2444,
0x19,0x8900,
0x18,0x2454,
0x19,0xfa00,
0x18,0x2464,
0x19,0xe000,
0x18,0x2474,
0x19,0x0000,
0x18,0x2484,
0x19,0x4000,
0x18,0x2494,
0x19,0x7f00,
0x18,0x24a4,
0x19,0x0000,
0x18,0x24b4,
0x19,0x0000,
0x18,0x24c4,
0x19,0x4000,
0x18,0x24d4,
0x19,0x3700,
0x18,0x24e4,
0x19,0x7300,
0x18,0x24f4,
0x19,0x0500,
0x18,0x2504,
0x19,0x0200,
0x18,0x2514,
0x19,0x0100,
0x18,0x2524,
0x19,0xd800,
0x18,0x2534,
0x19,0x0400,
0x18,0x2544,
0x19,0x7d00,
0x18,0x2554,
0x19,0x4000,
0x18,0x2564,
0x19,0x7f00,
0x18,0x2574,
0x19,0x0000,
0x18,0x2584,
0x19,0x0000,
0x18,0x2594,
0x19,0x4000,
0x18,0x25a4,
0x19,0x0000,
0x18,0x25b4,
0x19,0x7200,
0x18,0x25c4,
0x19,0x0400,
0x18,0x25d4,
0x19,0x0000,
0x18,0x25e4,
0x19,0x0800,
0x18,0x25f4,
0x19,0x7f00,
0x18,0x2604,
0x19,0x4000,
0x18,0x2614,
0x19,0x7f00,
0x18,0x2624,
0x19,0x0000,
0x18,0x2634,
0x19,0x0000,
0x18,0x2644,
0x19,0xc000,
0x18,0x2654,
0x19,0x0000,
0x18,0x2664,
0x19,0x7200,
0x18,0x2674,
0x19,0x0500,
0x18,0x2684,
0x19,0x0000,
0x18,0x2694,
0x19,0x0400,
0x18,0x26a4,
0x19,0xeb00,
0x18,0x26b4,
0x19,0x8400,
0x18,0x26c4,
0x19,0x7d00,
0x18,0x26d4,
0x19,0x4000,
0x18,0x26e4,
0x19,0x7f00,
0x18,0x26f4,
0x19,0x0000,
0x18,0x2704,
0x19,0x0000,
0x18,0x2714,
0x19,0x4000,
0x18,0x2724,
0x19,0x0000,
0x18,0x2734,
0x19,0x7200,
0x18,0x2744,
0x19,0x0700,
0x18,0x2754,
0x19,0x0000,
0x18,0x2764,
0x19,0x0400,
0x18,0x2774,
0x19,0xde00,
0x18,0x2784,
0x19,0x9b00,
0x18,0x2794,
0x19,0x7d00,
0x18,0x27a4,
0x19,0x4000,
0x18,0x27b4,
0x19,0x7f00,
0x18,0x27c4,
0x19,0x0000,
0x18,0x27d4,
0x19,0x9000,
0x18,0x27e4,
0x19,0x4000,
0x18,0x27f4,
0x19,0x0400,
0x18,0x2804,
0x19,0x7300,
0x18,0x2814,
0x19,0x1500,
0x18,0x2824,
0x19,0x0000,
0x18,0x2834,
0x19,0x0400,
0x18,0x2844,
0x19,0xd100,
0x18,0x2854,
0x19,0x9400,
0x18,0x2864,
0x19,0x9200,
0x18,0x2874,
0x19,0x8000,
0x18,0x2884,
0x19,0x7b00,
0x18,0x2894,
0x19,0x4000,
0x18,0x28a4,
0x19,0x7f00,
0x18,0x28b4,
0x19,0x0000,
0x18,0x28c4,
0x19,0x0000,
0x18,0x28d4,
0x19,0x4000,
0x18,0x28e4,
0x19,0x0000,
0x18,0x28f4,
0x19,0x7200,
0x18,0x2904,
0x19,0x0700,
0x18,0x2914,
0x19,0x0000,
0x18,0x2924,
0x19,0x0400,
0x18,0x2934,
0x19,0xc200,
0x18,0x2944,
0x19,0x9b00,
0x18,0x2954,
0x19,0x7d00,
0x18,0x2964,
0x19,0xe200,
0x18,0x2974,
0x19,0x7a00,
0x18,0x2984,
0x19,0x4000,
0x18,0x2994,
0x19,0x7f00,
0x18,0x29a4,
0x19,0x0000,
0x18,0x29b4,
0x19,0x0000,
0x18,0x29c4,
0x19,0x4000,
0x18,0x29d4,
0x19,0x3700,
0x18,0x29e4,
0x19,0x7300,
0x18,0x29f4,
0x19,0x0500,
0x18,0x2a04,
0x19,0x0000,
0x18,0x2a14,
0x19,0x0100,
0x18,0x2a24,
0x19,0x0300,
0x18,0x2a34,
0x19,0xe200,
0x18,0x2a44,
0x19,0x2a00,
0x18,0x2a54,
0x19,0x0200,
0x18,0x2a64,
0x19,0x7b00,
0x18,0x2a74,
0x19,0xe200,
0x18,0x2a84,
0x19,0x4800,
//### endpoint

0x1f,0x0000,
0x17,0x2100,
0x1f,0x0007,
0x1e,0x0040,
0x18,0x0000,
//### end of inrx dspctl patch code

//### inrx eyesch patch code
0x1f,0x0007,
0x1e,0x0042,
0x15,0x0f00,
0x1f,0x0000,
0x17,0x2160,
0x1f,0x0001,
0x10,0xf25e,
0x1f,0x0007,
0x1e,0x0042,

//### startpoint
0x15,0x0f00,
0x16,0x7408,
0x15,0x0e00,
0x15,0x0f00,
0x15,0x0f01,
0x16,0x4000,
0x15,0x0e01,
0x15,0x0f01,
0x15,0x0f02,
0x16,0x9400,
0x15,0x0e02,
0x15,0x0f02,
0x15,0x0f03,
0x16,0x7408,
0x15,0x0e03,
0x15,0x0f03,
0x15,0x0f04,
0x16,0x4008,
0x15,0x0e04,
0x15,0x0f04,
0x15,0x0f05,
0x16,0x9400,
0x15,0x0e05,
0x15,0x0f05,
0x15,0x0f06,
0x16,0x0803,
0x15,0x0e06,
0x15,0x0f06,
//### endpoint

0x1f, 0x0001,
0x10, 0xf05e,
0x1f, 0x0007,
0x1e, 0x0042,
0x15,0x0d00,
0x15,0x0100,
0x1f,0x0000,
0x17,0x2100,
//### end of inrx eyesch patch code
  	

//### release MDI/MDIX force mode
 0x1f, 0x0007,
 0x1e, 0x002d,
 0x18, 0xf010,



//### uc patch code (0423)
0x1f,0x0005,
//### startpoint
5,0x8000,
6,0xeeff,
6,0xfc8b,
6,0xeeff,
6,0xfda0,
6,0x0280,
6,0x33f7,
6,0x00e0,
6,0xfff7,
6,0xa080,
6,0x02ae,
6,0xf602,
6,0x8398,
6,0x0201,
6,0x4802,
6,0x015b,
6,0x0280,
6,0x98e0,
6,0x8b8c,
6,0xe18b,
6,0x8d1e,
6,0x01e1,
6,0x8b8e,
6,0x1e01,
6,0xa000,
6,0xe4ae,
6,0xd8ee,
6,0xe144,
6,0x77ee,
6,0xe145,
6,0x65ee,
6,0x8b85,
6,0x42ee,
6,0x8ae8,
6,0x6eee,
6,0x8605,
6,0x00ee,
6,0x8606,
6,0x00ee,
6,0x8607,
6,0x02ee,
6,0x8608,
6,0x00ee,
6,0x8609,
6,0x00ee,
6,0x860c,
6,0x00d4,
6,0x07f7,
6,0xe48b,
6,0x96e5,
6,0x8b97,
6,0xd408,
6,0x02e4,
6,0x8b94,
6,0xe58b,
6,0x95d1,
6,0x00bf,
6,0x83ca,
6,0x0229,
6,0x59bf,
6,0x8b88,
6,0xec00,
6,0x19a9,
6,0x8b90,
6,0xf9ee,
6,0xfff6,
6,0x00ee,
6,0xfff7,
6,0xfce0,
6,0xe140,
6,0xe1e1,
6,0x41f7,
6,0x2ff6,
6,0x28e4,
6,0xe140,
6,0xe5e1,
6,0x4104,
6,0xf8e0,
6,0x8b8e,
6,0xad20,
6,0x17f6,
6,0x20e4,
6,0x8b8e,
6,0x0225,
6,0xda02,
6,0x26b4,
6,0x0281,
6,0x5602,
6,0x0234,
6,0x0282,
6,0xca02,
6,0x8350,
6,0xe08b,
6,0x8ead,
6,0x2305,
6,0xf623,
6,0xe48b,
6,0x8ee0,
6,0x8b8e,
6,0xad24,
6,0x08f6,
6,0x24e4,
6,0x8b8e,
6,0x0227,
6,0x7de0,
6,0x8b8e,
6,0xad26,
6,0x0bf6,
6,0x26e4,
6,0x8b8e,
6,0x0205,
6,0x6e02,
6,0x1c9a,
6,0x0280,
6,0xe802,
6,0x8142,
6,0xfc04,
6,0xf8e0,
6,0x8b83,
6,0xad23,
6,0x21e0,
6,0xe022,
6,0xe1e0,
6,0x23ad,
6,0x2920,
6,0xe08b,
6,0x83ad,
6,0x2106,
6,0xe18b,
6,0x84ad,
6,0x283c,
6,0xe08b,
6,0x85ad,
6,0x2106,
6,0xe18b,
6,0x84ad,
6,0x2930,
6,0xbf31,
6,0x4402,
6,0x28e8,
6,0xae28,
6,0xee8a,
6,0xe200,
6,0xee8a,
6,0xe300,
6,0xee8a,
6,0xe400,
6,0xee8a,
6,0xe500,
6,0xee8b,
6,0x7200,
6,0xe08b,
6,0x83ad,
6,0x2108,
6,0xe08b,
6,0x84f6,
6,0x20e4,
6,0x8b84,
6,0xbf31,
6,0x4702,
6,0x28e8,
6,0xfc04,
6,0xf8e0,
6,0xe038,
6,0xe1e0,
6,0x39ac,
6,0x2e08,
6,0xeee0,
6,0x8e36,
6,0xeee0,
6,0x8f20,
6,0xfc04,
6,0xf8fa,
6,0xef69,
6,0xe08b,
6,0x85ad,
6,0x2139,
6,0xe0e0,
6,0x22e1,
6,0xe023,
6,0x58c0,
6,0x5902,
6,0x1e01,
6,0xe18b,
6,0x721f,
6,0x109e,
6,0x26e4,
6,0x8b72,
6,0xad21,
6,0x1de1,
6,0x8b84,
6,0xf729,
6,0xe58b,
6,0x84ac,
6,0x270d,
6,0xac26,
6,0x0502,
6,0x04a2,
6,0xae0d,
6,0x0281,
6,0xf9ae,
6,0x0802,
6,0x8225,
6,0xae03,
6,0x0281,
6,0x9eef,
6,0x96fe,
6,0xfc04,
6,0xd100,
6,0xbf83,
6,0xca02,
6,0x2959,
6,0xd103,
6,0xbf31,
6,0x6b02,
6,0x2959,
6,0xd100,
6,0xbf31,
6,0x6e02,
6,0x2959,
6,0xd100,
6,0xbf83,
6,0xd002,
6,0x2959,
6,0xd10f,
6,0xbf31,
6,0x6202,
6,0x2959,
6,0xd101,
6,0xbf31,
6,0x6502,
6,0x2959,
6,0xd101,
6,0xbf31,
6,0x6802,
6,0x2959,
6,0xe08b,
6,0x85ad,
6,0x240e,
6,0xd100,
6,0xbf31,
6,0x77ad,
6,0x2503,
6,0xbf31,
6,0x7a02,
6,0x2959,
6,0xe08b,
6,0x83ad,
6,0x2208,
6,0xd100,
6,0xbf31,
6,0x5c02,
6,0x2959,
6,0x04d1,
6,0x02bf,
6,0x316b,
6,0x0229,
6,0x59d1,
6,0x06bf,
6,0x3162,
6,0x0229,
6,0x59d1,
6,0x07bf,
6,0x3165,
6,0x0229,
6,0x59d1,
6,0x07bf,
6,0x3168,
6,0x0229,
6,0x59d1,
6,0x01bf,
6,0x83ca,
6,0x0229,
6,0x5902,
6,0x828a,
6,0x04d1,
6,0x02bf,
6,0x316b,
6,0x0229,
6,0x59d1,
6,0x01bf,
6,0x83ca,
6,0x0229,
6,0x59d0,
6,0x1102,
6,0x2804,
6,0x5903,
6,0xef01,
6,0xd100,
6,0xa000,
6,0x02d1,
6,0x01bf,
6,0x316e,
6,0x0229,
6,0x59d1,
6,0x11ad,
6,0x2002,
6,0x0c11,
6,0xad21,
6,0x020c,
6,0x12bf,
6,0x83d0,
6,0x0229,
6,0x5902,
6,0x8261,
6,0x04e0,
6,0x8b85,
6,0xad24,
6,0x22d4,
6,0x0001,
6,0xbf83,
6,0xc102,
6,0x2959,
6,0xbf83,
6,0xc402,
6,0x2997,
6,0xe08a,
6,0xe81b,
6,0x10aa,
6,0x04d1,
6,0x01ae,
6,0x02d1,
6,0x00bf,
6,0x83c7,
6,0x0229,
6,0x5904,
6,0xf8e0,
6,0x8b85,
6,0xad24,
6,0x37e0,
6,0xeaca,
6,0xe1ea,
6,0xcbad,
6,0x292e,
6,0xe0ea,
6,0xcce1,
6,0xeacd,
6,0xad29,
6,0x25e0,
6,0xe008,
6,0xe1e0,
6,0x09ad,
6,0x201c,
6,0xe0e0,
6,0x0ae1,
6,0xe00b,
6,0xad20,
6,0x13d1,
6,0x03bf,
6,0x3177,
6,0xe08b,
6,0x85ad,
6,0x2505,
6,0xd101,
6,0xbf31,
6,0x7a02,
6,0x2959,
6,0xfc04,
6,0xf8f9,
6,0xe08b,
6,0x85ad,
6,0x257b,
6,0xe0e0,
6,0x22e1,
6,0xe023,
6,0xe2e0,
6,0x36e3,
6,0xe037,
6,0x5ac4,
6,0x0d01,
6,0x5802,
6,0x1e20,
6,0xe386,
6,0x05ac,
6,0x314f,
6,0xac3a,
6,0x05ac,
6,0x3e23,
6,0xae56,
6,0xad37,
6,0x53e0,
6,0x8606,
6,0x10e4,
6,0x8606,
6,0xe186,
6,0x071b,
6,0x109e,
6,0x02ae,
6,0x43d1,
6,0x00bf,
6,0x83cd,
6,0x0229,
6,0x59ee,
6,0x8606,
6,0x00ae,
6,0x35ad,
6,0x3619,
6,0xe086,
6,0x08e1,
6,0x8609,
6,0xa403,
6,0xb802,
6,0xae26,
6,0x14e4,
6,0x8608,
6,0xe586,
6,0x09a4,
6,0x03b8,
6,0x1bae,
6,0x0dee,
6,0x8608,
6,0x00ee,
6,0x8609,
6,0x00ae,
6,0x0fac,
6,0x390c,
6,0xd101,
6,0xbf83,
6,0xcd02,
6,0x2959,
6,0xee86,
6,0x0600,
6,0xe686,
6,0x05fd,
6,0xfc04,
6,0xf8f9,
6,0xe08b,
6,0x85ad,
6,0x263d,
6,0xe0e0,
6,0x36e1,
6,0xe037,
6,0xe186,
6,0x0c1f,
6,0x109e,
6,0x30e4,
6,0x860c,
6,0xad21,
6,0x2ad0,
6,0x0b02,
6,0x2804,
6,0x5882,
6,0x7882,
6,0x9f1f,
6,0xe0e0,
6,0x00e1,
6,0xe001,
6,0xf727,
6,0xe4e0,
6,0x00e5,
6,0xe001,
6,0xe2e0,
6,0x20e3,
6,0xe021,
6,0xad30,
6,0xf7f6,
6,0x27e4,
6,0xe000,
6,0xe5e0,
6,0x01fd,
6,0xfc04,
6,0xf8fa,
6,0xef69,
6,0xe08b,
6,0x86ac,
6,0x201a,
6,0xbf83,
6,0xd3d0,
6,0x6c02,
6,0x28bb,

0x06, 0xe0e0,
0x06, 0xe4e1,
0x06, 0xe0e5,
0x06, 0x5806,
0x06, 0x68c0,
0x06, 0xd1d2,
0x06, 0xe4e0,
0x06, 0xe4e5,
0x06, 0xe0e5,


6,0xef96,
6,0xfefc,
6,0x04a0,
6,0xe0ea,
6,0xf0e0,
6,0x7c55,
6,0xe232,
6,0x11e2,
6,0x3288,
6,0xe200,
6,0x70e4,
6,0x2625,
6,0x0807,

6,0x2640,
6,0x7227,
6,0x267e,
6,0x2804,
6,0xb729,
6,0x2576,

 0x06, 0x2a68,
 0x06,0xe52b,
 0x06, 0xad00,
 0x06, 0x2cdb,
 0x06, 0xf02d,
 0x06, 0x67bb,
 0x06, 0x2e7b,
 0x06, 0x0f2f,
 0x06, 0x7365,
 0x06, 0x31ac,
 0x06, 0xcc32,
 0x06, 0x2300,
 0x06, 0x332d,
 0x06, 0x1734,
 0x06, 0x7f52,
 0x06, 0x3510,
 0x06, 0x0036,
 0x06, 0x1000,
 0x06, 0x3710,
 0x06, 0x0038,
 0x06, 0x7fce,
 0x06, 0x3ce5,
 0x06, 0xf73d,
 0x06, 0x3da4,
 0x06, 0x6530,
 0x06, 0x3e67,
 0x06, 0x0053,
 0x06, 0x69d2,
 0x06, 0x0f6a,
 0x06, 0x012c,
 0x06, 0x6c2b,
 0x06, 0x136e,
 0x06, 0xe100,
 0x06, 0x6f12,
 0x06, 0xf771,
 0x06, 0x006b,
 0x06, 0x7306,
 0x06, 0xeb74,
 0x06, 0x94c7,
 0x06, 0x7698,
 0x06, 0x0a77,
 0x06, 0x5000,
 0x06, 0x788a,
 0x06, 0x1579,
 0x06, 0x7f6f,
 0x06, 0x7a06,
 0x06, 0xa600,
//### endpoint
                                  
//#clear the stack                  
0x05,0x8bf0,
0x06,0x0000,
0x06,0x0000,
0x06,0x0000,
0x06,0x0000,
0x06,0x0000,
0x06,0x0000,
0x06,0x0000,
0x06,0x0000,
0x01,0x0701,
0x00,0x0405,
0x1f,0x0000,
//### end of uc patch code
//#Enable negear EEE Nway ability autooff
0x1f,0x0005,
0x05,0x8b84,
0x06,0x0026,
0x1f,0x0000,

//#Add by Gary for Channel Estimation fine tune 20100430
//#Page1 Reg8 (CG_INITIAL_MASTER)
0x1f, 0x0005,
0x05, 0x83dd,
0x06, 0x0574,
//#Page1 Reg9 (CB0_INITIAL_GIGA)
0x1f, 0x0005,
0x05, 0x83e0,
0x06, 0x2724,
//#Page3 Reg26 (CG_INITIAL_SLAVE)
0x1f, 0x0005,
0x05, 0x843d,
0x06, 0x06f6 ,

0x1f, 0x0000,
#endif
};


unsigned int default_val[]={

999,0x1f,0x0002,

2,0x11,0x7e00,

3,0x1f,0x0002,
3,0x17,0xff00,
3,0x18,0x0005,
3,0x19,0x0005,
3,0x1a,0x0005,
3,0x1b,0x0005,
3,0x1c,0x0005,

4,0x1f,0x0002,
4,0x13,0x00aa,
4,0x14,0x00aa,
4,0x15,0x00aa,
4,0x16,0x00aa,
4,0x17,0x00aa,
4,0x18,0x0f0a,
4,0x19,0x50ab,
4,0x1a,0x0000,
4,0x1b,0x0f0f,

999,0x1f,0x0000,
};
  
void Setting_RTL8198_GPHY(void)
{
	int i=0, port =0, len=0;
    
	for(i=0; i<5; i++)
        REG32(PCRP0+i*4) |= (EnForceMode);

	if (REG32(REVR) == BSP_RTL8198_REVISION_A) 
	
	{
		/*
		  #Access command format: phywb {all: phyID=0,1,2,3,4} {page} {RegAddr} {Bit location} {Bit value}
		  # when writing page 72: must do "mdcmdio_cmd write $phyID 31 0x7" first, then write "mdcmdio_cmd write $phyID 30 $PageNum"
		  phywb all 72 21 15-0 0x7092
		  phywb all 72 22 15-0 0x7092
		  phywb all 72 23 15-0 0x7092
		  phywb all 72 24 15-0 0x7092
		  phywb all 72 25 15-0 0x7092
		  phywb all 72 26 15-0 0x7092
		*/

		Set_GPHYWB(999, 72, 21, 0, 0x7092);
		Set_GPHYWB(999, 72, 22, 0, 0x7092);
		Set_GPHYWB(999, 72, 23, 0, 0x7092);
		Set_GPHYWB(999, 72, 24, 0, 0x7092);
		Set_GPHYWB(999, 72, 25, 0, 0x7092);	
		Set_GPHYWB(999, 72, 26, 0, 0x7092);	

		/*
		  set PageNum 2; #All of GPHY register in the Page#2
		  #Array format = {{PhyID List1}  {RegAddr1 RegData1 RegAddr2 RegData2}, ...}

		  set AFE_Reg 
		  {{0 1 2 3 4}  { 4 0x80c8 6  0x0678 7  0x3620}
			  {2}      {11 0x0063 12 0xeb65 13 0x51d1 14 0x5dcb 17  0x7e00}
			  {3}      {23 0x0000 24 0x0000 26 0x0000}
			  {4}      {19 0x0000 20 0x0000 21 0x0000 22 0x0000 23  0x0000 24 0x0f0a 25  0x5050    26 0x0000        27 0x000f}}


		*/

	       //phyid=all
		Set_GPHYWB(999, 2,  0, 0, 0x0000);  
		Set_GPHYWB(999, 2,  1, 0, 0x065a);  
		Set_GPHYWB(999, 2,  2, 0, 0x8c01);  
		Set_GPHYWB(999, 2,  3, 0, 0x0428);  
		Set_GPHYWB(999, 2,  4, 0, 0x80c8);  
		Set_GPHYWB(999, 2,  5, 0, 0x0978);  
		Set_GPHYWB(999, 2,  6, 0, 0x0678);
		Set_GPHYWB(999, 2,  7, 0, 0x3620);	
		Set_GPHYWB(999, 2,  8, 0, 0x0000);  
		Set_GPHYWB(999, 2,  9, 0, 0x0007);  
		Set_GPHYWB(999, 2,  10, 0, 0x0000);  

	       //phyid=2
		Set_GPHYWB( 2,   2, 11, 0, 0x0063);	
		Set_GPHYWB( 2,   2, 12, 0, 0xeb65);	
		Set_GPHYWB( 2,   2, 13, 0, 0x51d1);	
		Set_GPHYWB( 2,   2, 14, 0, 0x5dcb);	
		Set_GPHYWB( 2,   2, 15, 0, 0x3044);	
		Set_GPHYWB( 2,   2, 16, 0, 0x1000);		
		Set_GPHYWB( 2,   2, 17, 0, 0x7e00);		
		Set_GPHYWB( 2,   2, 18, 0, 0x0000);	
	
	       //phyid=3
		Set_GPHYWB( 3,   2, 19, 0, 0x3d22);	
		Set_GPHYWB( 3,   2, 20, 0, 0x2000);	
	   	Set_GPHYWB( 3,   2, 21, 0, 0x6040);	
		Set_GPHYWB( 3,   2, 22, 0, 0x0000);		   
		Set_GPHYWB( 3,   2, 23, 0, 0xff00);	
		Set_GPHYWB( 3,   2, 24, 0, 0x0005);	
		Set_GPHYWB( 3,   2, 25, 0, 0x0005);		
		Set_GPHYWB( 3,   2, 26, 0, 0x0005);	
		Set_GPHYWB( 3,   2, 27, 0, 0x0005);	
		Set_GPHYWB( 3,   2, 28, 0, 0x0005);		

	       //phyid=4
		Set_GPHYWB( 4,   2, 19, 0, 0x00aa);	
		Set_GPHYWB( 4,   2, 20, 0, 0x00aa);	
		Set_GPHYWB( 4,   2, 21, 0, 0x00aa);	
		Set_GPHYWB( 4,   2, 22, 0, 0x00aa);	
		Set_GPHYWB( 4,   2, 23, 0, 0x00aa);	
		Set_GPHYWB( 4,   2, 24, 0, 0x0f0a);	
		Set_GPHYWB( 4,   2, 25, 0, 0x5050);	
		Set_GPHYWB( 4,   2, 26, 0, 0x0000);	
		Set_GPHYWB( 4,   2, 27, 0, 0x0f0f);			

/*
#=========== INRX Para. =================================                                                                 

phywb all 0 21 0x1006
           #dfse_mode[15:14]=3(full), Fine tune aagc_lvl_fnet[10:0]
           phywb all 1 12 15-0 0xdbf0
           
           #cb0_i_giga[12:0]
           phywb all 1 9  15-0 0x2576
           phywb all 1 7  15-0 0x287E
           phywb all 1 10 15-0 0x68E5
           phywb all 1 29 15-0 0x3DA4
           phywb all 1 28 15-0 0xE7F7
           phywb all 1 20 15-0 0x7F52
           phywb all 1 24 15-0 0x7FCE
           phywb all 1 8  15-0 0x04B7
           phywb all 1 6  15-0 0x4072
           phywb all 1 16 15-0 0xF05E
           phywb all 1 27 15-0 0xB414
*/
           
		Set_GPHYWB( 999,   1, 12, 0, 0xdbf0);	

		Set_GPHYWB( 999,   1, 9, 0, 0x2576);	
		Set_GPHYWB( 999,   1, 7, 0, 0x287E);	
		Set_GPHYWB( 999,   1, 10, 0, 0x68E5);	
		Set_GPHYWB( 999,   1, 29, 0, 0x3DA4);		
		Set_GPHYWB( 999,   1, 28, 0, 0xE7F7);	
		Set_GPHYWB( 999,   1, 20, 0, 0x7F52);	
		Set_GPHYWB( 999,   1, 24, 0, 0x7FCE);	
		Set_GPHYWB( 999,   1, 8, 0, 0x04B7);		
		Set_GPHYWB( 999,   1, 6, 0, 0x4072);	
		Set_GPHYWB( 999,   1, 16, 0, 0xF05E);	
		Set_GPHYWB( 999,   1, 27, 0, 0xB414);		

/*
           #=========== Cable Test =================================

	  phywb all 3 26 15-0 0x06A6
	  phywb all 3 16 15-0 0xF05E
	  phywb all 3 19 15-0 0x06EB
	  phywb all 3 18 15-0 0xF4D2
	  phywb all 3 14 15-0 0xE120
	  phywb all 3 0  15-0 0x7C00

	  phywb all 3 2  15-0 0x5FD0
	  phywb all 3 13 15-0 0x0207
	 
	  #disable jabber detect         
	   phywb all 0 16 15-0 0x05EF

	  #Patch for EEE GMII issue
	  phywb all 32 26 15-0 0x0103
	  phywb all 32 22 15-0 0x0004
*/
		Set_GPHYWB( 999,   3, 26, 0, 0x06A6);	 
		Set_GPHYWB( 999,   3, 16, 0, 0xF05E);	
		Set_GPHYWB( 999,   3, 19, 0, 0x06EB);	
		Set_GPHYWB( 999,   3, 18, 0, 0xF4D2);	
		Set_GPHYWB( 999,   3, 14, 0, 0xE120);	
		Set_GPHYWB( 999,   3, 00, 0, 0x7C00);	
	
		Set_GPHYWB( 999,   3, 02, 0, 0x5FD0);	
		Set_GPHYWB( 999,   3, 13, 0, 0x0207);	
		
		Set_GPHYWB( 999,   0, 16, 0, 0x05EF);	
	
		Set_GPHYWB( 999,   3, 26, 0, 0x0103);	
		Set_GPHYWB( 999,   3, 22, 0, 0x0004);	

/*	
	disable aldps_en, for power measurement
	hywb all 44 21 15-0 0x0350
*/
		Set_GPHYWB( 999,   44, 21, 0, 0x0350);	
	}
	else
	{
		len=sizeof(default_val)/sizeof(unsigned int);
		for(i=0;i<len;i=i+3)
		{	
		
			if(default_val[i]==999)
			{
				for(port=0; port<5; port++)
					rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
			}
			else
			{
				port=default_val[i];
				rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
					//__delay(delay_cnt);
			}
		}
		len=sizeof(p)/sizeof(unsigned int);
		for(port=0; port<5; port++)
		{
			for(i=0;i<len;i=i+2)
			{	
				rtl8651_setAsicEthernetPHYReg(port, p[i], p[i+1]);
				//__delay(delay_cnt);
			}
		}
		Set_GPHYWB( 999,   5, 5, 0, 0x8b84);		
		Set_GPHYWB( 999,   5, 6, 0, 0x0006);		
		Set_GPHYWB( 999,   2, 8, 0, 0x0020);

#ifdef CONFIG_RTL_8198_ESD
		Set_GPHYWB(999, 44, 27, 0xffff-(0xf<<12), 0x4<<12);	
#endif		
	}


	for(i=0; i<5; i++)
            REG32(PCRP0+i*4) &= ~(EnForceMode);
	
	printk("==Set GPHY Parameter OK\n");
}

#ifdef PORT5_RGMII_GMII
unsigned int ExtP5GigaPhyMode=0;
void ProbeP5GigaPHYChip(void)	
{
	unsigned int uid,tmp;
	unsigned int i;

	//printk("In Setting port5 \r\n");

	REG32(0xB8000010)=0x01FFFCB9;

	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);


	/* Read */
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID,0x10,0x01FE);
	rtl8651_getAsicEthernetPHYReg(GIGA_P5_PHYID, 0, &tmp );

	//printk("Read port5 phyReg0= 0x%x \r\n",tmp);

	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 2, &tmp );
	//printk("Read port5 UPChipID= 0x%x \r\n",tmp);
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 3, &tmp );
	//printk("Read port5 downChipID= 0x%x \r\n",tmp);
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{	
		//printk("Find Port5   have 8211 PHY Chip! \r\n");
		ExtP5GigaPhyMode=1;
		//return 1;
	}	
	else
	{	
		//printk("NO Find Port5 8211 PHY Chip! \r\n");
		//ExtP5GigaPhyMode=0;
		//return 1;
	}	
	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

}
#endif
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
unsigned int Get_P0_PhyMode(void)
{
/*
	00: External  phy
	01: embedded phy
	10: olt
	11: deb_sel
*/
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf	

	unsigned int v=REG32(HW_STRAP);
	unsigned int mode=GET_BITVAL(v, 6, RANG1) *2 + GET_BITVAL(v, 7, RANG1);


	return (mode&3);


}

//---------------------------------------------------------------------------------------

unsigned int Get_P0_MiiMode(void)
{
/*
	0: MII-PHY
	1: MII-MAC
	2: GMII-MAC
	3: RGMII
*/
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf	
	
	unsigned int v=REG32(HW_STRAP);
	unsigned int mode=GET_BITVAL(v, 27, RANG2);


	return mode;
	
	
}
//---------------------------------------------------------------------------------------
unsigned int Get_P0_RxDelay(void)
{
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf	
	
	unsigned int v=REG32(HW_STRAP);
	unsigned int val=GET_BITVAL(v, 29, RANG3);
	return val;

}
unsigned int Get_P0_TxDelay(void)
{
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf	
	
	unsigned int v=REG32(HW_STRAP);
	unsigned int val=GET_BITVAL(v, 17, RANG1);
	return val;

}
#endif
//====================================================================

#ifdef CONFIG_RTL_819XD
int Setting_RTL8196D_PHY(void)
{
	int i;
	
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	// only do the PHY setting in this revision IC, no need for the new one.
	if (REG32(REVR) == 0x8197C000) {

		Set_GPHYWB(999, 0, 21, 0, 0x0232);

		/* purpose: to avoid 100M N-way link fail issue Set_p="1" */
		Set_GPHYWB(999, 0, 22, 0, 0x5bd5);

	 	/* purpose: to adjust AOI waveform */
		Set_GPHYWB(999, 1, 18, 0, 0x9004);

		/* purpose: to enhance ethernet 100Mhz output voltage about 1.0(v) */
		Set_GPHYWB(999, 1, 19, 0, 0x5400);

		Set_GPHYWB(999, 1, 25, 0, 0x00d0);  //enable pwdn10rx at pwr saving enable snr threshold = 18dB

		Set_GPHYWB(999, 4, 16, 0, 0x737f);// enable EEE, fine tune EEE parameter
		Set_GPHYWB(999, 4, 24, 0, 0xc0f3);	//change EEE wake idle to 10us
		Set_GPHYWB(999, 4, 25, 0, 0x0730);	 // turn off tx/rx pwr at LPI state
	}

	// fix unlink IOT issue
	Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);

	/* fine tune port on/off threshold to 160/148 */
	REG32(PBFCR0) = 0x009400A0;
	REG32(PBFCR1) = 0x009400A0;
	REG32(PBFCR2) = 0x009400A0;
	REG32(PBFCR3) = 0x009400A0;
	REG32(PBFCR4) = 0x009400A0;
	REG32(PBFCR6) = 0x009400A0;

	/* modify egress leaky bucket parameter, default inaccuracy is 5~10%, the new one is 1~2% after modification */
	REG32(ELBPCR) = 0x0000400B;
	REG32(ELBTTCR) = 0x000000C0;

	/* 100M half duplex enhancement */
	/* fix Smartbit Half duplex backpressure IOT issue */
 	REG32(MACCR)= (REG32(MACCR) & ~(CF_RXIPG_MASK | SELIPG_MASK)) | (0x05 | SELIPG_11);

	/* enlarge "Flow control DSC tolerance" from 24 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);

	/* default enable MAC EEE */
	REG32(EEECR) = 0x28739ce7;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
 
	return 0;
}
#endif

#ifdef CONFIG_RTL_8196E
int Setting_RTL8196E_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	// write page1, reg16, bit[15:13] Iq Current 110:175uA (default 100: 125uA)
	Set_GPHYWB(999, 1, 16, 0xffff-(0x7<<13), 0x6<<13);

	// disable power saving mode in A-cut only
	if (REG32(REVR) == 0x8196e000)
		Set_GPHYWB(999, 0, 0x18, 0xffff-(1<<15), 0<<15);

	/* B-cut and later,
	    just increase a little power in long RJ45 cable case for Green Ethernet feature.
	 */
	else 
	{
		// adtune_lb setting
		Set_GPHYWB(999, 0, 22, 0xffff-(0x7<<4), 0x4<<4);
		//Setting SNR lb and hb
		Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0xc2<<0);
		//auto bais current
		Set_GPHYWB(999, 1, 19, 0xffff-(0x1<<0), 0x0<<0);
		Set_GPHYWB(999, 0, 22, 0xffff-(0x1<<3), 0x0<<3);
	}

	// fix Ethernet IOT issue
	if ( ((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES)  &&
		((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES1)  &&
		((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES2)  &&
		((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES3)  )
	{
		Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);
		Set_GPHYWB(999, 0, 17, 0xffff-(0xf<<8), 0xe<<8);
	}
	
	/* 100M half duplex enhancement */
	/* fix Smartbit Half duplex backpressure IOT issue */
 	REG32(MACCR)= (REG32(MACCR) & ~(CF_RXIPG_MASK | SELIPG_MASK)) | (0x05 | SELIPG_11);
	
	/* enlarge "Flow control DSC tolerance" from 24 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);
	
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
	printk("  Set 8196E PHY OK\n");
	return 0;
}
#endif

#if defined(CONFIG_RTL_8881A)
int Setting_RTL8881A_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);
#if 0
	// Page1, Reg23, Vref[7:6] 10
	Set_GPHYWB(999, 1, 23, 0xffff-(3<<6), 2<<6);

	// Page1, Reg16, cf1[6:4] 110->001
	Set_GPHYWB(999, 1, 16, 0xffff-((7<<4)), ((1<<4)));

        Set_GPHYWB(999,1, 29, 0xFFFE, 0x0);
		mdelay(1);
		Set_GPHYWB(999,0, 0, 0xffff, 0x200);
		mdelay(1);
        Set_GPHYWB(999,1, 29, 0xffff, 0x1);
	    mdelay(1);	
		Set_GPHYWB(999,0, 21, 0xff00, (1<<1|1<<4|1<<5));
		mdelay(1);
#else
if(((REG32(0xb8000000)&0xff)>0))
{
	#define PLL2 0xb8000058
	REG32(PLL2 )|=(5<<6);//20120906:Set RBG_L[8:6]=101 from RDC suggestion
	Set_GPHYWB(999, 1, 23, 0xffff-(3<<6), 2<<6);//SwitchPatch #2 Page1,Reg.23[7:6] = 10
    mdelay(1);
	Set_GPHYWB(999, 1, 16, 0xffff-(7<<4), 1<<4);//SwitchPatch #3 Page1,Reg.16[6:4] = 001
    mdelay(1);
	Set_GPHYWB(999, 4, 16, 0, 0x4077);//SwitchPatch #5 Page4,Reg.16 = 0x4077
    mdelay(1);
	Set_GPHYWB(999, 4, 24, 0, 0xc0f3);//SwitchPatch #6 Page4,Reg.24 = 0xc0f3
    mdelay(1);
	Set_GPHYWB(999, 4, 25, 0, 0x7630);//SwitchPatch #7 Page4,Reg.25 = 0x7630
    mdelay(1);
	REG32( 0xBB804160)=0x0 ;//SwitchPatch #8 BB804160 = 0x0E739CE7 ,Set EEE MAC register
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<4), 2<<4);//Patch #9 Page0,Reg.21[7:4]=0010  ,Set Green ethernet
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<0), 1<<0);//Patch #10 Page0,Reg.21[3:0]=0001  ,Set Green ethernet
    mdelay(1);
	Set_GPHYWB(999, 1, 19, 0xffff-(3<<3), 1<<3);//Patch #11 Page1,Reg.19[4:3]=01  ,....corner internetl SWR Pass 1.12V
    mdelay(1);
	Set_GPHYWB(999, 1, 17, 0xffff-(1<<14), 1<<14);//Patch #12 Page1,Reg.17[14]=1  ,Fix Low SNR issue.
    mdelay(1);
	Set_GPHYWB(999, 1, 16, 0xffff-(3<<12), 0<<0);//Patch #13 Page1,Reg.16[13:12]=00  ,IOL For 10M MAU
    mdelay(1);
	Set_GPHYWB(999, 0, 17, 0xffff-(15<<8), 0xE<<8);
	mdelay(1);	
}
else
{
	Set_GPHYWB(999, 1, 23, 0xffff-(3<<6), 2<<6);//SwitchPatch #2 Page1,Reg.23[7:6] = 10
    mdelay(1);
	Set_GPHYWB(999, 1, 16, 0xffff-(7<<4), 1<<4);//SwitchPatch #3 Page1,Reg.16[6:4] = 001
    mdelay(1);
	Set_GPHYWB(999, 1, 29, 0xffff-(1<<0), 0<<0);//SwitchPatch #4 Page1,Reg.29[0] = 0 //A cut don't need it
	mdelay(1);
	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 1<<9);//SwitchPatch #4 Page0,Reg.0[9] = 1 //A cut don't need it
	mdelay(1);
	Set_GPHYWB(999, 1, 29, 0xffff-(1<<0), 1<<0);//SwitchPatch #4 Page1.Reg.29[0] =1 //A cut don't need it
	mdelay(1);
	Set_GPHYWB(999, 4, 16, 0, 0x7377);//SwitchPatch #5 Page4,Reg.16 = 0x7377
    mdelay(1);
	Set_GPHYWB(999, 4, 24, 0, 0xc0f3);//SwitchPatch #6 Page4,Reg.24 = 0xc0f3
    mdelay(1);
	Set_GPHYWB(999, 4, 25, 0, 0x7630);//SwitchPatch #7 Page4,Reg.25 = 0x7630
    mdelay(1);
	REG32( 0xBB804160)=0x0E739CE7 ;//SwitchPatch #8 BB804160 = 0x0E739CE7 ,Set EEE MAC register
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<4), 3<<4);//Patch #9 Page0,Reg.21[7:4]=0011  ,Set Green ethernet
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<0), 2<<0);//Patch #10 Page0,Reg.21[3:0]=0010  ,Set Green ethernet
	mdelay(1);
	Set_GPHYWB(999, 0, 17, 0xffff-(15<<8), 0xE<<8);
	mdelay(1);
}
#endif		
    // refine for B-cut and later
	{
	unsigned int val= REG32(0xb80000dc);
	if ((val & 0x3) != 0x3)
		REG32(0xb80000dc) |= 0x3; // wlanmac_control, set active_wlanmac_sys and active_wlanmac_lx for RF revision ID

	if ((REG32(0xb86400f0) & 0x0000f000) >= 0x00001000)
		Set_GPHYWB(999, 1, 16, 0xffff-(7<<4), 4<<4); // fix 100M IOL overshoot issue

	REG32(0xb80000dc) = val; // restore value.
	}
    
	/* enlarge "Flow control DSC tolerance" from 24 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);
    
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

int32 swCore_init(int VlanCount)
{
	int port;
	
	if (VlanCount == -2) {
		rtl8651_clearAsicAllTable();
		return 0;
	}
	
	if (VlanCount == -1) {
	    #ifdef CONFIG_RTL_LAYERED_DRIVER
 		rtl865x_reinitNetifTable();
		#else
		_rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
		#endif
		return 0;			
	}

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
	port_link_sts2 = 0;
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
	port_link_sts = 0;
	port_linkpartner_eee = 0;
#endif

#ifdef CONFIG_RTL865X_HW_TABLES
    
	/*
	 * 8211 Giga port compatibility issue:
	 * after test Chariot (3D+3U) for one to two minutes, then issue reboot in console,
	 * sometimes the 8211 Giga port will not work (LAN PC shows 100Mbps, 
	 *      Target's LED is incorrect, ping 192.168.1.254 is failed)
	 * this issue is easy to happen when the LAN PC's Gigabit Ethernet is Marvell's.
	 * 
	 * do the external PHY reset and restart auto-negotiation to fix it.
	 */
#ifndef CONFIG_RTL8196B
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID, 0, (PHY_RESET | ENABLE_AUTONEGO));		
	rtl8651_restartAsicEthernetPHYNway(5, GIGA_P5_PHYID);		
#endif

	/*
	 * after switch operation mode from Gateway mode to Bridge/WISP mode,
	 * ping br0(192.168.1.254) with 32 bytes is okay, 
	 * but ping br0(192.168.1.254) with 65500 bytes is failed.
	 * the fragmented packets sent from 192.168.1.254 have no Ethernet header and IP header,
	 * they are started with ping payload, like:
	 *      0000   75 76 77 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d  uvwabcdefghijklm
	 *      0010   6e 6f 70 71 72 73 74 75 76 77 61 62 63 64 65 66  nopqrstuvwabcdef
	 *      0020   67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76  ghijklmnopqrstuv
	 *      0030   77 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  wabcdefghijklmno
	 *      0040   70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68  pqrstuvwabcdefgh
	 *      0050   69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 61  ijklmnopqrstuvwa
	 *      0060   62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71  bcdefghijklmnopq
	 *
	 * after do the switch semi-reset, the issue is fixed
	 */
	/* Perform the semi-reset */
#ifndef CONFIG_RTL8196B
	REG32(SIRR) |= SEMI_RST;
	tick_Delay10ms(10);        

#endif
#else
	/* Full reset and semreset */
	FullAndSemiReset();

#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
		Setting_RTL8196C_PHY();
#elif defined(CONFIG_RTL_8881A)
	Setting_RTL8881A_PHY();
#elif defined(CONFIG_RTL_819XD)
	Setting_RTL8196D_PHY();
#elif defined(CONFIG_RTL_8196E)
	Setting_RTL8196E_PHY();
#elif defined(CONFIG_RTL8198)
#if 0//def PORT5_RGMII_GMII
	ProbeP5GigaPHYChip();
#endif
	Setting_RTL8198_GPHY();
#endif

	rtl8651_clearAsicAllTable();

#endif /* CONFIG_RTL865X_HW_TABLES */
    
#ifdef CONFIG_RTL865X_PANAHOST
        rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_MII_MAC); //Port 5 MII MAC mode
#elif !defined(CONFIG_RTL8196B)
	rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_RGMII);
#endif

	/*
		# According to Hardware SD: David & Maxod,			
		Set Port5_GMII Configuration Register.
		- RGMII Output Timing compensation control : 0 ns
		- RGMII Input Timing compensation control : 0 ns
	*/
#ifdef CONFIG_RTL865X_PANAHOST
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, (1<<4), RGMII_RCOMP_0NS);
		rtl8651_setAsicEthernetMII(GIGA_P5_PHYID, LINK_MII_MAC, TRUE);
#elif !defined(CONFIG_RTL8196B)
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);
		rtl8651_setAsicEthernetMII(GIGA_P5_PHYID, P5_LINK_RGMII, TRUE);
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
	init_8367r();
#endif


	/*
	 * do not modify PVCR0 ~ PVCR3 (vid) and MSCR (EN_L2/EN_L3/EN_L4) when CONFIG_RTL865X_HW_TABLES
	 * is defined.
	 * they will be set by rtl865x_lightRomeConfig().
	 */
#ifndef CONFIG_RTL865X_HW_TABLES
#ifdef CONFIG_RTL8196_RTL8366
	{
	extern int savOP_MODE_value;
	int32 retval = 0;
	int iport;
	rtl8366rb_vlanConfig_t vlancfg_8366;
		if(savOP_MODE_value==2)
		{
			/* for lan */
			memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
			vlancfg_8366.fid = 0;
			vlancfg_8366.mbrmsk = (RTL8366RB_LANPORT&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
			vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
			vlancfg_8366.vid = RTL_LANVLANID;
			retval = rtl8366rb_setVlan(&vlancfg_8366);
			/*need set pvid??*/
			for(iport=0;iport<8;iport++)
				if  ((1<<iport)&vlancfg_8366.mbrmsk)
				{
					retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
				}
		
			/* for wan */
			iport=0;
			memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
			retval = 0;

			vlancfg_8366.fid = 0;
			vlancfg_8366.mbrmsk = (RTL8366RB_WANPORT&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
			vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
			vlancfg_8366.vid = RTL_WANVLANID;
			retval = rtl8366rb_setVlan(&vlancfg_8366);
			/*need set pvid??*/
			for(iport=0;iport<8;iport++)
				if  ((1<<iport)&vlancfg_8366.mbrmsk)
				{
					retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
				}
		}
		else
		{
			{
                        memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
                        vlancfg_8366.fid = 0;
                        vlancfg_8366.mbrmsk = ((RTL8366RB_WANPORT|RTL8366RB_LANPORT)&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
                        vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
                        vlancfg_8366.vid = RTL_LANVLANID;
                        retval = rtl8366rb_setVlan(&vlancfg_8366);
                        /*need set pvid??*/
                        for(iport=0;iport<8;iport++)
                                if  ((1<<iport)&vlancfg_8366.mbrmsk)
                                {
                                        retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
                                }
                	}
	                {
	                        /* for wan clear it */
	                        int iport;
	                        rtl8366rb_vlanConfig_t vlancfg_8366;

	                        vlancfg_8366.fid = 0;
	                        vlancfg_8366.mbrmsk = 0;
	                        vlancfg_8366.untagmsk = 0;
	                        vlancfg_8366.vid = RTL_WANVLANID;
	                        retval = rtl8366rb_setVlan(&vlancfg_8366);
	                }
			
		}
	}

	REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;

#elif defined(CONFIG_RTL8196B_GW_MP) || defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
	REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;

#else
	if(VlanCount==1)    
	{
		//wei add, original all lan use VID=9
		/* Set PVID of all ports to 9 */
		REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
	}
	else if(VlanCount==2)
	{
		REG32(PVCR0) = cPVCR[wan_port][0];
		REG32(PVCR1) = cPVCR[wan_port][1];
		REG32(PVCR2) = cPVCR[wan_port][2];
		REG32(PVCR3) = cPVCR[wan_port][3];
	}

#if defined(CONFIG_RTK_GUEST_ZONE) 
	else
	{
#if defined(CONFIG_RTL8196C_KLD)		
		REG32(PVCR0) = (LAN3_VID << 16) | LAN4_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN2_VID;
		REG32(PVCR2) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
#else
		REG32(PVCR0) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR1) = (LAN3_VID << 16) | LAN2_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN4_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
#endif		
	}
#endif

/*
	8196B VLAN enabled:
	Physical port 	4		3		2		1		0		[Power]
				Vlan1	Vlan2	Vlan3	Vlan4	WAN
	8196C VLAN enabled:
	Physical port 	0		1		2		3		4		[Power]
				Vlan1	Vlan2	Vlan3	Vlan4	WAN
	===================================================
	8196B-KLD Guest Zone enabled:
	Physical port 	0		1		2		3		4		[Power]
				LAN1	LAN2	LAN3	LAN4	WAN
	8196C-KLD Guest Zone enabled:
	Physical port 	0		1		2		3		4		[Power]
				LAN1	LAN2	LAN3	LAN4	WAN
 */
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	else
	{
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
		REG32(PVCR0) = (LAN2_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN4_VID << 16) | LAN3_VID;
		REG32(PVCR2) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;		
#else
		REG32(PVCR0) = (LAN4_VID << 16) | WAN_VID;
		REG32(PVCR1) = (LAN2_VID << 16) | LAN3_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;		
#endif		
	}
#endif	
#endif
#endif

#ifdef __ECOS
#ifdef CONFIG_POCKET_ROUTER_SUPPORT
	REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
#endif
#endif

#ifdef CONFIG_RTL8197B_PANA
		REG32(PCRP1) = EnForceMode & (~ForceLink) & (~EnablePHYIf);
		REG32(PCRP2) = EnForceMode & (~ForceLink) & (~EnablePHYIf);
		REG32(PCRP3) = (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		REG32(PCRP4) = EnForceMode & (~ForceLink) & (~EnablePHYIf);

#elif defined(CONFIG_RTL_8198_NFBI_BOARD)
    //WRITE_MEM32(PIN_MUX_SEL_2, 0); //for led control
        REG32(PCRP0) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
        REG32(PCRP1) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
        REG32(PCRP2) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
        REG32(PCRP3) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
        REG32(PCRP4) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
        REG32(PCRP5) &= (0xFFFFFFFF-(0x00000000|MacSwReset));

        REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
        REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
        REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
        REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
        REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;

        //port5 STP forwarding?
        REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface
        rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_PHY); //port 5 MII PHY mode
        REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1

#if defined(RTL8198_NFBI_PORT5_GMII) //GMII mode
           #define GMII_PIN_MUX 0xc0
	   #define PIN_MUX_SEL 0xb8000040
           REG32(PIN_MUX_SEL)= REG32(PIN_MUX_SEL)&(~(GMII_PIN_MUX));
        REG32(PCRP5) = 0 | (PORT5_PHYID<<ExtPHYID_OFFSET) |
                        EnForceMode| ForceLink|ForceSpeed1000M|ForceDuplex |
                        MIIcfg_RXER | EnablePHYIf | MacSwReset;
#else //MII mode*/
        REG32(PCRP5) = 0 | (PORT5_PHYID<<ExtPHYID_OFFSET) |
                        EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex |
                        MIIcfg_RXER | EnablePHYIf | MacSwReset;
#endif

#elif defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)

		#if defined(CONFIG_RTL_8367R_SUPPORT)
		REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (1<<15) );
		REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
		#else
		#if defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_GPIO)
		 ;
		#else
		REG32(PIN_MUX_SEL2)= 0;
		#endif
		#endif

		REG32(PCRP1) = REG32(PCRP1) & (0xFFFFFFFF-(MacSwReset));
		REG32(PCRP2) = REG32(PCRP2) & (0xFFFFFFFF-(MacSwReset));
		REG32(PCRP3) = REG32(PCRP3) & (0xFFFFFFFF-(MacSwReset));
		REG32(PCRP4) = REG32(PCRP4) & (0xFFFFFFFF-(MacSwReset));

		REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
		REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
		REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
		REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
#ifdef PORT5_RGMII_GMII
		if(ExtP5GigaPhyMode)
		{
			REG32(PCRP5) &= (0x83FFFFFF-(0x00000000|MacSwReset));
			REG32(PCRP5) = REG32(PCRP5) | (GIGA_P5_PHYID << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset | 1<<20;
			REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done ;//| P5txdely;
		}
#endif

#elif defined(CONFIG_RTL8196C)

#if !defined(CONFIG_POCKET_ROUTER_SUPPORT) && !defined (CONFIG_RTL_8196CS) && !defined(CONFIG_RTL_ETH_PORT4_ONLY)
		REG32(PCRP1) = REG32(PCRP1) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
		REG32(PCRP2) = REG32(PCRP2) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
		REG32(PCRP3) = REG32(PCRP3) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
		REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
		REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | EnablePHYIf|MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
		REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | EnablePHYIf|MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
#endif
		REG32(PCRP4) = REG32(PCRP4) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);
		REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | EnablePHYIf|MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);

#else
		//REG32(PCRP0) = (0 << ExtPHYID_OFFSET) | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP1) = (1 << ExtPHYID_OFFSET) | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP2) = (2 << ExtPHYID_OFFSET) | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP3) = (3 << ExtPHYID_OFFSET) | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP4) = (4 << ExtPHYID_OFFSET) | IPMSTP_PortST_MASK | EnablePHYIf;
#endif


#if defined(CONFIG_RTL_8881A) && defined(CONFIG_RTL_8211E_SUPPORT)
	init_p0();

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) //wei add
	{
	unsigned int P0phymode=1;
	//diag_printf("P0phymode=%02x, %s phy\n", P0phymode,   (P0phymode==0) ? "external" : "embedded");

	#if defined(CONFIG_RTL_8367R_SUPPORT)
	P0phymode = 3;
	#endif

	if (bonding_type == BOND_8196ES) {
		// reduce power consumption in 8196ES
		for (port=1; port<5; port++) {
			REG32(PCRP0+port*4) |= (EnForceMode);		
			rtl8651_getAsicEthernetPHYReg(port, 0, &P0phymode);
			P0phymode |= POWER_DOWN;
			rtl8651_setAsicEthernetPHYReg(port, 0, P0phymode);				
		}			
		
		P0phymode=Get_P0_PhyMode();
	}

	if(P0phymode==1)  //embedded phy
	{
#ifdef CONFIG_FPGA_ROMCODE	
		REG32_ANDOR(0xbb804050, ~(0x1f<<0), 0x11<<0); 
		REG32(PCRP0) |=  (0<< ExtPHYID_OFFSET) |  EnablePHYIf | MacSwReset;	//emabedded
#else
		REG32(PCRP0) |=  (0 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;	//emabedded
#endif
	}
	else //external phy
	{
		REG32(PCRP0) |=  (0x06 << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset;	//external
			
		unsigned int P0miimode=Get_P0_MiiMode();
		//unsigned int P0miimode=1;
		//const unsigned char *miimodename[]={ "MII-PHY", "MII-MAC", "GMII-MAC", "RGMII" };
		//panic_printk("P0miimode=%02x, %s\n", P0miimode, miimodename[P0miimode] );
		#if defined(CONFIG_RTL_8367R_SUPPORT)
		P0miimode = 3;
		#endif
		
		if(P0miimode==0)       		REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_PHY<<23); 
		else if(P0miimode==1)     	REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_MAC<<23);  
		else if(P0miimode==2)     	REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_MAC<<23);  //GMII
		else if(P0miimode==3)     	REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_RGMII<<23);   
		
		 if(P0miimode==3) 
		 {
			unsigned int P0txdly=Get_P0_TxDelay();
			unsigned int P0rxdly=Get_P0_RxDelay();	
			
			#if defined(CONFIG_RTL_8367R_SUPPORT)
			P0txdly = 1;
			P0rxdly = 5;
			#endif

			REG32_ANDOR(P0GMIICR, ~((1<<4)|(3<<0)) , (P0txdly<<4) | (P0rxdly<<0) );			
		}
		 
		#if defined(CONFIG_RTL_8367R_SUPPORT)
		#define MACCR1					(0x058+SWMACCR_BASE)	/* Embedded PHY ID Register */
		#define CFG_TX_CPUC_TAG                      (1<<26)                  /* Enable Tx CPU tag */
		#define CFG_CPUC_TAG                      	(1<<25)                  /* Enable CPU tag */
		#define PORT0_ROUTER_MODE               (1 << 0)                /* 1: enable Port0 as router mode,  0: normal mode */

		REG32(P0GMIICR) |= (CFG_CPUC_TAG|CFG_TX_CPUC_TAG);

		// enable port 0 router mode
		REG32(MACCR1) |= PORT0_ROUTER_MODE;        	
		#endif


		if(P0miimode==0)       	 	 REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ;
#ifdef CONFIG_FPGA_ROMCODE	
		else if(P0miimode==1)     	 REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed10M |ForceDuplex) ; 
#else
		else if(P0miimode==1)     	 REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ; 
#endif	
		else if(P0miimode==2)     	 REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex );
		else if(P0miimode==3)     	 REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex );

		REG32(PITCR) |= (1<<0);   //00: embedded , 01L GMII/MII/RGMII

		if((P0miimode==2)  ||(P0miimode==3)) {
			REG32(MACCR) |= (1<<12);   //giga link
		}
		REG32(P0GMIICR) |=(Conf_done);
	}   
	}
#else // CONFIG_RTL_819XD

		if(ExtP0GigaPhyMode==0)
		{
#if defined(CONFIG_RTL8198)
			REG32(PCRP0) = REG32(PCRP0) & (0xFFFFFFFF-(MacSwReset));
			REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
#elif defined(CONFIG_RTL8196C)
#if !defined(CONFIG_POCKET_ROUTER_SUPPORT) && !defined (CONFIG_RTL_8196CS) && !defined(CONFIG_RTL_ETH_PORT4_ONLY)
			REG32(PCRP0) = REG32(PCRP0) & (0xFFFFFFFF-(0x00400000|MacSwReset));
			TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
			REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset;
			TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
#endif
#else		
			//REG32(PITCR) = REG32(PITCR) | (0<<0);  //default, embedded PHY
			REG32(PCRP0) = (0 << ExtPHYID_OFFSET) | EnablePHYIf;
#endif
		}
		
#ifndef __ECOS
		else if(ExtP0GigaPhyMode==1)
		{
			//unsigned int temp;

			REG32(PITCR) = REG32(PITCR) | (1<<0);
			rtl865xC_setAsicEthernetRGMIITiming(0, (0<<4), RGMII_RCOMP_2DOT5NS); //wei add, for 8652 demo board	
			//rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_7NS, RGMII_RCOMP_0NS); //wei add, for 8652 demo board	
		
			//8211C auto learning
			//rtl8651_getAsicEthernetPHYReg(GIGA_P0_PHYID, 3, &temp);
			
			if( (SW_P0Mode&0x0f)==0)  // RGMII mode
			{
				rtl865xC_setAsicEthernetMIIMode(0, LINK_RGMII); //wei add for 8652 demo board
			}
			else if( (SW_P0Mode&0x0f)==1)  //MII MAC  mode
			{
				rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_MAC); //wei add for 8652 demo board
			}
			else if( (SW_P0Mode&0x0f)==2)  //MII PHY mode
			{
				rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_PHY); //wei add for 8652 demo board
			}		

			if( (SW_P0Mode&0xf0)==0)  
			{
				REG32(PCRP0) = (GIGA_P0_PHYID<< ExtPHYID_OFFSET) | EnablePHYIf;
			}
			else if( (SW_P0Mode&0xf0)==0x10)  //GMII / MII mode
			{			
				REG32(PCRP0) = 0 | (GIGA_P0_PHYID<<ExtPHYID_OFFSET) |
							EnForceMode| ForceLink|ForceSpeed10M |ForceDuplex |
							MIIcfg_RXER | EnablePHYIf   ;			
			}						
		}	
#endif

#ifdef CONFIG_RTL865X_PANAHOST
		REG32(PCRP5) = 0 | (GIGA_P5_PHYID<<ExtPHYID_OFFSET) |
							EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex |
							MIIcfg_RXER | 				
							AcptMaxLen_16K|EnablePHYIf;
#elif !defined(CONFIG_RTL8196B)
		REG32(PCRP5) = (GIGA_P5_PHYID<<ExtPHYID_OFFSET)|IPMSTP_PortST_MASK |EnablePHYIf;
#endif

	/* Set forwarding status */
#ifdef CONFIG_RTL8197B_PANA
	REG32(PCRP0) = (REG32(PCRP0) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP1) = (REG32(PCRP1) & ~STP_PortST_MASK) | STP_PortST_DISABLE;
	REG32(PCRP2) = (REG32(PCRP2) & ~STP_PortST_MASK) | STP_PortST_DISABLE;
	REG32(PCRP3) = (REG32(PCRP3) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP4) = (REG32(PCRP4) & ~STP_PortST_MASK) | STP_PortST_DISABLE;
#else
	REG32(PCRP0) = (REG32(PCRP0) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
	REG32(PCRP1) = (REG32(PCRP1) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
	REG32(PCRP2) = (REG32(PCRP2) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
	REG32(PCRP3) = (REG32(PCRP3) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
	REG32(PCRP4) = (REG32(PCRP4) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);
#endif

#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)||defined (CONFIG_RTL_HARDWARE_MULTICAST)
	// Add for ip-multicast forward
	REG32(PCRP0) = (REG32(PCRP0) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
	REG32(PCRP1) = (REG32(PCRP1) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;	
	TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
	REG32(PCRP2) = (REG32(PCRP2) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
	REG32(PCRP3) = (REG32(PCRP3) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
	REG32(PCRP4) = (REG32(PCRP4) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);
	#ifndef CONFIG_RTL8196B
	REG32(PCRP5) = (REG32(PCRP5) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	#endif
	REG32(PCRP6) = (REG32(PCRP6) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
#endif
#endif // CONFIG_RTL_819XD || CONFIG_RTL_8196E
	
#if defined CONFIG_HW_MULTICAST_TBL ||defined (CONFIG_RTL_HARDWARE_MULTICAST)
//	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_MCAST | EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU);
	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_MCAST | EN_UNMCAST_TOCPU);
	WRITE_MEM32(CSCR, (READ_MEM32(CSCR) & ~(ALLOW_L2_CHKSUM_ERR|ALLOW_L3_CHKSUM_ERR|ALLOW_L4_CHKSUM_ERR)) | (EN_ETHER_L3_CHKSUM_REC | EN_ETHER_L4_CHKSUM_REC));
	UPDATE_MEM32(ALECR, 1522, MultiCastMTU_MASK, MultiCastMTU_OFFSET);
	WRITE_MEM32(TEACR, READ_MEM32(TEACR) & ~IPMcastAgingDisable);
#endif

	#ifndef CONFIG_RTL865X_HW_TABLES
	/* Enable L2 lookup engine and spanning tree functionality */
	
	#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
		//rtl8651_setAsicOperationLayer(3);
		REG32(MSCR) = EN_L2|EN_L3;
	#else
		
		REG32(MSCR) = EN_L2;
	#endif
	#endif

#if defined(CONFIG_RTL8196C)
	REG32(QNUMCR) = P0QNum_1 | P1QNum_1 | P2QNum_1 | P3QNum_1 | P4QNum_1;
#endif

	#ifdef CONFIG_RTK_VOIP_865xC_QOS
	if (rtl865x_QoS_mechanism() != SUCCESS)
	{
		rtlglue_printf("rtl865x_QoS_mechanism Error.\n");
		return FAILED;
	}
	#endif

#ifndef __ECOS
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	qos_init();
#endif
#endif

	/* Start normal TX and RX */
	REG32(SIRR) |= TRXRDY;

	/* Init PHY LED style */
 /* Init PHY LED style */
#if defined(CONFIG_RTL_8881A)
	/*
		#LED = direct mode
		set mode 0x0
		swwb 0xbb804300 21-20 0x2 19-18 $mode 17-16 $mode 15-14 $mode 13-12 $mode 11-10 $mode 9-8 $mode
	*/ 
	//REG32(PIN_MUX_SEL)= (REG32(PIN_MUX_SEL) & ~( (7<<2) | (7<<7) | (3<<10) |  (1<<15) )) | (4<<2);  //let P0 to mii mode
#if defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_GPIO)
	// rtl_gpio.c will take care of it.
	//REG32(PIN_MUX_SEL2)&=~ ((7<<3) );  //S0-S3, P0
#else
 	REG32(PIN_MUX_SEL2)&=~ ((7<<0) | (7<<3) | (7<<6) | (7<<9) | (3<<12)   );  //S0-S3, P0
#endif

	REG32(LEDCR)  = (2<<20) | (0<<18) | (0<<16) | (0<<14) | (0<<12) | (0<<10) | (0<<8);  //P0-P5
#ifdef CONFIG_RTL_ALP
	/* Set LED BlinkTime filed(Bit 30:28) of Direct Mode LED Configuration Register to 0x000(No blink).To meet external spec request. Leon */
	REG32(SWCORE_BASE+0x4314) &= ~(7<<28);
#endif
#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	/*
		#LED = direct mode
		set mode 0x0
		swwb 0xbb804300 21-20 0x2 19-18 $mode 17-16 $mode 15-14 $mode 13-12 $mode 11-10 $mode 9-8 $mode
	*/	
	#if !defined(CONFIG_RTL_8367R_SUPPORT)
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (3<<3) | (1<<15) );  //let P0 to mii mode
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
	#endif
	
	REG32(LEDCR)  = (2<<20) | (0<<18) | (0<<16) | (0<<14) | (0<<12) | (0<<10) | (0<<8);  //P0-P5
#else
#ifdef BICOLOR_LED
#ifdef CONFIG_RTL8196B
#ifdef CONFIG_RTL8197B_PANA
    REG32(0xb8000030) = REG32(0xb8000030) & (~0x00020000); //bit17, port 3 led
#else
	/*
	 * 0xb8000030: System pin mux selection
	 * 	change to LED-SW mode from DBG mode (chip initial value is DBG mode)
	 */
	//REG32(0xb8000030) = 0x0;
      #if defined(CONFIG_RTL8198)    
	REG32(LEDCR)  = 0x55500;
      #endif
#endif
#else
	if(read_gpio_hw_setting())	// 1 means using matrix mode
	{
	 	REG32(LEDCR)  = 0x155500;
	}

	REG32(TCR0) = 0x000002c2;
	REG32(SWTAA) = PORT5_PHY_CONTROL;
	REG32(SWTACR) = ACTION_START | CMD_FORCE;
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
#endif	
#else
	#if defined(BICOLOR_LED_VENDOR_BXXX)
	REG32(LEDCR) |= 0x00080000;

	REG32(PABCNR) &= ~0xc01f0000; /* set port a-7/6 & port b-4/3/2/1/0 to gpio */
	REG32(PABDIR) |=  0x401f0000; /* set port a-6 & port b-4/3/2/1/0 gpio direction-output */
	REG32(PABDIR) &= ~0x80000000; /* set port a-7 gpio direction-input */
	#else /* BICOLOR_LED_VENDOR_BXXX */
	REG32(LEDCR) = 0x00000000;
	REG32(TCR0) = 0x000002c7;
	REG32(SWTAA) = PORT5_PHY_CONTROL;
	REG32(SWTACR) = ACTION_START | CMD_FORCE;
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
	#endif /* BICOLOR_LED_VENDOR_BXXX */
#endif
#endif // CONFIG_RTL_819XD || CONFIG_RTL_8196E

	#ifdef CONFIG_RTL8196B_TLD
	if ((REG32(CPUSSR) & 0x0000ffff) != _MAGIC_FORM_BOOT) 
	#endif
	{

	/*PHY FlowControl. Default enable*/
	for(port=0;port<MAX_PORT_NUMBER;port++)
	{
#ifndef __ECOS
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
		if (eee_enabled) {
			uint32 reg;			
			eee_phy_enable_by_port(port);

			if (REG32(REVR) == RTL8196C_REVISION_B) {
				// enable EEE MAC
				reg = REG32(EEECR);
				REG32(EEECR) = (reg & ~(0x1f << (port * 5))) | 
					((FRC_P0_EEE_100|EN_P0_TX_EEE|EN_P0_RX_EEE) << (port * 5));
			}
		}
#endif
#endif
		/* Set Flow Control capability. */
	#ifndef CONFIG_RTL8196B
		if (port == MAX_PORT_NUMBER-1)
			rtl8651_setAsicFlowControlRegister(RTL8651_MII_PORTNUMBER, TRUE, GIGA_P5_PHYID);
		else				
	#endif
			rtl8651_restartAsicEthernetPHYNway(port+1, port);				
	}
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	if (eee_enabled == 0) {
		REG32(EEECR) = 0;
	}
#endif	
	}


#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	REG32(EEECR) = 0;
#endif

	#if defined(CONFIG_RTL865X_DIAG_LED)
	/* diagnosis led (gpio-porta-6) on */
	REG32(PABDAT) |=  0x40000000; /* pull high by set portA-0(bit 30) as gpio-output-1, meaning: diag led OFF */
	#endif /* CONFIG_RTL865X_DIAG_LED */

#if !defined(CONFIG_RTL8196B)
	REG32(MDCIOCR) = 0x96181441;	// enable Giga port 8211B LED
#endif

#if  defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_ULINKER) /* disable unused port for saving power *///jwj
	if (bonding_type == BOND_8196EU)
	{
		uint32 statCtrlReg0;
		int32 index;

		for (index=0; index<4; index++) {
			/* read current PHY reg 0 value */
			rtl8651_getAsicEthernetPHYReg( index, 0, &statCtrlReg0 );

			REG32(PCRP0+(index*4)) |= EnForceMode;
			statCtrlReg0 |= POWER_DOWN;

			/* write PHY reg 0 */
			rtl8651_setAsicEthernetPHYReg( index, 0, statCtrlReg0 );              
		}
	}
#endif


#ifdef __ECOS
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1_BRIDGED
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	REG32(FFCR) = IPMltCstCtrl_Enable|EN_UNMCAST_TOCPU|EN_UNUNICAST_TOCPU;
	#else
	REG32(FFCR) = IPMltCstCtrl_TrapToCpu|EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU; // for bridge forwarding
	#endif	
	#else
	REG32(FFCR) = EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU; // for bridge forwarding
	#endif
#else
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	REG32(FFCR) = IPMltCstCtrl_Enable|EN_UNMCAST_TOCPU|EN_UNUNICAST_TOCPU;
	#else
	REG32(FFCR) = IPMltCstCtrl_TrapToCpu|EN_UNMCAST_TOCPU|EN_UNUNICAST_TOCPU;
	#endif
	#else	
	REG32(FFCR) = EN_UNMCAST_TOCPU|EN_UNUNICAST_TOCPU;
	#endif
#endif
#else
//	REG32(FFCR) = EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU; // rx broadcast and unicast packet
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	REG32(FFCR) = IPMltCstCtrl_Enable|EN_UNMCAST_TOCPU|EN_UNUNICAST_TOCPU;
	#else
	REG32(FFCR) = IPMltCstCtrl_TrapToCpu|EN_UNMCAST_TOCPU|EN_UNUNICAST_TOCPU; // rx multicast packet
	#endif
#endif

#ifdef CONFIG_RTL8196_RTL8366
	{
		rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_4NS, RGMII_RCOMP_2NS);
		WRITE_MEM32(PITCR, Port0_TypeCfg_GMII_MII_RGMII);
		WRITE_MEM32(PCRP0, 0xe80367);		/* force mode 1G */
	}
#endif


	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
/*
	Explanation for ACL setting:
			ACL range
	WAN: 	0~0
	LAN:		10~12

	L2_table_disabled=0, EN_IN_ACL = 0, all ACL rules are no used
	L2_table_disabled=1, EN_IN_ACL = 1, 
		ACL #0: RTL8651_ACL_CPU
		ACL #10: RTL8651_ACL_CPU
		ACL #11 ~ 12: don't care because rule #10 will trap all packets to CPU
 */
	EasyACLRule(0, RTL8651_ACL_CPU);	// WAN
	EasyACLRule(10, RTL8651_ACL_CPU); // LAN

	if (L2_table_disabled) {
	  	REG32(MSCR) |= EN_IN_ACL;
	}
	else 
	  	REG32(MSCR) &= (~EN_IN_ACL);
	#endif

	#if defined(CONFIG_RTL_VLAN_SUPPORT)
/*
	Explanation for ACL setting:
			ACL range
	WAN: 	0~0
	LAN:		10~12

	L2_table_disabled=0, EN_IN_ACL = 0, all ACL rules are no used
	L2_table_disabled=1, EN_IN_ACL = 1, 
		ACL #0: RTL8651_ACL_CPU
		ACL #10: RTL8651_ACL_CPU
		ACL #11 ~ 12: don't care because rule #10 will trap all packets to CPU
 */
	EasyACLRule(0, RTL8651_ACL_PERMIT);	// WAN
	if (rtl_vlan_support_enable)
	{
		EasyACLRule(1, RTL8651_ACL_CPU);	// WAN
		EasyACLRule(10, RTL8651_ACL_CPU); // LAN
	}
	else
	{
		EasyACLRule(1, RTL8651_ACL_PERMIT);	// WAN
		EasyACLRule(10, RTL8651_ACL_PERMIT); // LAN
	}
	REG32(MSCR) |= EN_IN_ACL;

	#if 0
	if (rtl_vlan_support_enable) 
	  	REG32(MSCR) |= EN_IN_ACL;
	else 
	  	REG32(MSCR) &= (~EN_IN_ACL);
	#endif
	#endif


	#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
	rtl8651_defineProtocolBasedVLAN( IP6_PASSTHRU_RULEID, 0x0, 0x86DD );
	updateProtocolBasedVLAN();
	#endif
	

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	//enable RTL8196C 10M power saving mode
	{
		int tmp,idx;

		for(idx=0;idx<MAX_PORT_NUMBER;idx++) {
			rtl8651_getAsicEthernetPHYReg( idx, 0x18, (unsigned int *)&tmp );
#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
			tmp |= BIT(15); //enable power saving mode
#else
			tmp &= ~BIT(15); //disable power saving mode
#endif
			rtl8651_setAsicEthernetPHYReg( idx, 0x18, tmp );
		}
	}
	REG32(MPMR) |= PM_MODE_ENABLE_AUTOMATIC_POWER_DOWN;
#ifdef CONFIG_RTL8196C
	#define PIN_MUX_SEL 0xb8000040
	// Configure LED-SIG0/LED-SIG1/LED-SIG2/LED-SIG3/LED-PHASE0/LED-PHASE1/LED-PHASE2/LED-PHASE3 PAD as LED-SW

#if !defined(CONFIG_POCKET_ROUTER_SUPPORT) && !defined (CONFIG_RTL_8196CS) && !defined(CONFIG_RTL_ETH_PORT4_ONLY)
	REG32(PIN_MUX_SEL) &= ~(0xFFFF);		
#endif
#endif
#endif

#if defined (CONFIG_RTL_MLD_SNOOPING)
/*
	Explanation for ACL setting:
			ACL range
	WAN: 	0~1
	LAN:		10~12

	EN_IN_ACL always be 1,
	L2_table_disabled=0, 
		ACL #0: ACL_IPV6_TO_CPU
		ACL #1: RTL8651_ACL_PERMIT
		ACL #10: RTL8651_ACL_PERMIT for DA = 33 33 ff xx xx xx
		ACL #11: RTL8651_ACL_CPU for DA = 33 33 xx xx xx xx
		ACL #12: RTL8651_ACL_PERMIT
	L2_table_disabled=1, 
		ACL #0: RTL8651_ACL_PERMIT for DA = 01 00 5E xx xx xx
		ACL #1: RTL8651_ACL_CPU
		ACL #10: RTL8651_ACL_CPU
		ACL #11 ~ 12: don't care because rule #10 will trap all packets to CPU
 */
	/* add for support ipv6 multicast snooping */
	{
	EasyACLRule(0, ACL_IPV6_TO_CPU);	// WAN
	EasyACLRule(1, RTL8651_ACL_PERMIT);	// WAN

	setACL_fwd_ipv6_mcast(10);			// LAN for hw forward ipv6 multicast (DA=33-33-ff-xx-xx-xx)
	setACL_trap_ipv6_mcast(11);			// LAN for tarp ipv6 multicast (DA=33-33-xx-xx-xx-xx)

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	if (L2_table_disabled) {
		EasyACLRule(10, RTL8651_ACL_CPU); // LAN
		EasyACLRule(0, RTL8651_ACL_CPU);
	}		
	else 
#endif	
		EasyACLRule(12, RTL8651_ACL_PERMIT); // LAN
	
	if(savOP_MODE_value==2) {
		REG32(MSCR) |= EN_IN_ACL;
		REG32(SWTCR1) |= EN_FRAG_TO_ACLPT;
	}
	else
	{
		REG32(MSCR) &= ~EN_IN_ACL;
		REG32(FFCR) &= ~EN_MCAST;
	}
	}
#else
	if(savOP_MODE_value==2) {
		REG32(MSCR) |= EN_IN_ACL;
		REG32(SWTCR1) |= EN_FRAG_TO_ACLPT;
	}
#endif	

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	REG32(VCR0) &= ~EnVlanInF_MASK;
#endif	
	
	
	return 0;
}

void swCore_start(void)
{
	int i;
	uint32 statCtrlReg;

	//REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES;
	REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_128WORDS | MBUF_2048BYTES;	
	REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | LINK_CHANGE_IE | PKTHDR_DESC_RUNOUT_IE_ALL;
	//REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | LINK_CHANGE_IE;

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	REG32(SBFCR2) = (REG32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (0x136 << S_Max_SBuf_FCON_OFFSET) | (0xfc << S_Max_SBuf_FCOFF_OFFSET);
#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	#ifdef CONFIG_RTL_8197D_DYN_THR
	REG32(SBFCR1) =  (REG32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_AGG_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_AGG_fcOFF << S_DSC_FCOFF_OFFSET);
	REG32(SBFCR2) =  (REG32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_AGG_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_AGG_sharedOFF << S_Max_SBuf_FCOFF_OFFSET);
	#endif
#else
	REG32(SBFCR2) = 0xd400f8; // adjust internal buffer threshold
#endif

	REG32(ELBTTCR) = 0x00000400;/*mark_patch for correcting the Leaky bucket value*/	

	if (bonding_type == BOND_8196ES) {
		rtl8651_getAsicEthernetPHYReg(0, 0, &statCtrlReg);
		statCtrlReg &= ~POWER_DOWN;
		rtl8651_setAsicEthernetPHYReg(0, 0, statCtrlReg);
	}
	#if defined(CONFIG_RTL_ULINKER)
	else if  (bonding_type == BOND_8196EU) {
		rtl8651_getAsicEthernetPHYReg(4, 0, &statCtrlReg);
		statCtrlReg &= ~POWER_DOWN;
		rtl8651_setAsicEthernetPHYReg(4, 0, statCtrlReg);
	}
	#endif
    #if defined(CONFIG_RTL_8881A)
    #if defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH)
    else if (bonding_type == BOND_8881AM)
    {
        void rtl_setUnusedPortPowerOff(void);
        rtl_setUnusedPortPowerOff();
        #if defined(CONFIG_RTL_8881A_CUTE_MAHJONG_ESD)
        rtl_startCuteMahjondEsdCheck();
        #endif
    }
    #endif
    #endif
	else {
		for (i=0; i<5; i++) {	
			#if defined(CONFIG_RTL_8367R_SUPPORT)
			extern int rtk_stp_mstpState_set(int msti, int port, int stp_state);
			rtk_stp_mstpState_set(0, i, 3); // 3: STP_STATE_FORWARDING
			#else
			rtl8651_getAsicEthernetPHYReg(i, 0, &statCtrlReg);
			statCtrlReg &= ~POWER_DOWN;
			rtl8651_setAsicEthernetPHYReg(i, 0, statCtrlReg);
			REG32(PCRP0+i*4) &= ~(EnForceMode);		
			#endif
		}
	}
}

void swCore_down(void)
{
	int i;
	uint32 statCtrlReg;
	
	REG32(CPUICR) = 0; 
	REG32(CPUIIMR) = 0;        	
	REG32(SIRR) = 0;

	for (i=0; i<5; i++) {
		#if defined(CONFIG_RTL_8367R_SUPPORT)
		extern int rtk_stp_mstpState_set(int msti, int port, int stp_state);
		rtk_stp_mstpState_set(0 , i, 1); // 1: STP_STATE_BLOCKING
		#else
		REG32(PCRP0+i*4) |= (EnForceMode);		
		rtl8651_getAsicEthernetPHYReg(i, 0, &statCtrlReg);
		statCtrlReg |= POWER_DOWN;
		rtl8651_setAsicEthernetPHYReg(i, 0, statCtrlReg);	
		#endif
        #if defined(CONFIG_RTL_8881A_CUTE_MAHJONG_ESD)
        rtl_stopCuteMahjondEsdCheck(i);
        #endif    
	}	
}

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
#ifndef CONFIG_RTK_VOIP
int32 rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > PHY5))
		return FAILED; 

	if ((fcON > (QG_DSC_FCON_MASK >> QG_DSC_FCON_OFFSET)) || 
		(fcOFF > (QG_DSC_FCOFF_MASK >> QG_DSC_FCOFF_OFFSET)))
		return FAILED; 


	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QDBFCRP0G0+(port*0xC)), (READ_MEM32(QDBFCRP0G0+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET)); 		
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:		
			WRITE_MEM32((QDBFCRP0G1+(port*0xC)), (READ_MEM32(QDBFCRP0G1+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET)); 		
			break;
		case QUEUE5:
			WRITE_MEM32((QDBFCRP0G2+(port*0xC)), (READ_MEM32(QDBFCRP0G2+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET)); 		
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > PHY5))
		return FAILED; 

	if ((fcON > (QG_QLEN_FCON_MASK>> QG_QLEN_FCON_OFFSET)) || 
		(fcOFF > (QG_QLEN_FCOFF_MASK >> QG_QLEN_FCOFF_OFFSET)))
		return FAILED; 

	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QPKTFCRP0G0+(port*0xC)), (READ_MEM32(QPKTFCRP0G0+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET)); 		
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:		
			WRITE_MEM32((QPKTFCRP0G1+(port*0xC)), (READ_MEM32(QPKTFCRP0G1+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET)); 
			break;
		case QUEUE5:
			WRITE_MEM32((QPKTFCRP0G2+(port*0xC)), (READ_MEM32(QPKTFCRP0G2+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET)); 
			break;
		default:
			return FAILED;
	}

	return SUCCESS;
}
int32 rtl8651_setAsicPortBasedFlowControlRegister(enum PORTID port, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((fcON > (P_MaxDSC_FCON_MASK >> P_MaxDSC_FCON_OFFSET)) || 
		(fcOFF > (P_MaxDSC_FCOFF_MASK >> P_MaxDSC_FCOFF_OFFSET)))
		return FAILED; 

	switch (port)
	{
		case PHY0:
			WRITE_MEM32(PBFCR0, (READ_MEM32(PBFCR0) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY1:
			WRITE_MEM32(PBFCR1, (READ_MEM32(PBFCR1) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY2:
			WRITE_MEM32(PBFCR2, (READ_MEM32(PBFCR2) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY3:
			WRITE_MEM32(PBFCR3, (READ_MEM32(PBFCR3) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY4:
			WRITE_MEM32(PBFCR4, (READ_MEM32(PBFCR4) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY5:
			WRITE_MEM32(PBFCR5, (READ_MEM32(PBFCR5) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case CPU:
			WRITE_MEM32(PBFCR6, (READ_MEM32(PBFCR6) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
		default:
			return FAILED;
	}

	return SUCCESS;
}
#endif

#define FC_S_DSC_RUNOUT        500
#define FC_S_DSC_FCON          400
#define FC_S_DSC_FCOFF         280
#define FC_S_MaxSBuf_FCON      310
#define FC_S_MaxSBuf_FCOFF     252
#define FC_P_MaxDSC_FCON        320
#define FC_P_MaxDSC_FCOFF       258

#ifdef __KERNEL__
static void qos_init(void)
{
/* suggested value from David Lu:
set global_thrList  {\
            S_DSC_RUNOUT        500         \
            S_DSC_FCON          400         \ 
            S_DSC_FCOFF         280         \ 
            S_MaxSBuf_FCON      310         \
            S_MaxSBuf_FCOFF     252         \
            Q_P0_EN_FC          0x3F        \
            Q_P1_EN_FC          0x3F        \
            Q_P2_EN_FC          0x3F        \
            Q_P3_EN_FC          0x3F        \
            Q_P4_EN_FC          0x3F        \
            Q_P5_EN_FC          0x3F        \
            Q_P6_EN_FC          0x3F        \
            QLEN_GAP            72          \
            P0QNum              4           \
            P1QNum              4           \
            P2QNum              4           \
            P3QNum              4           \
            P4QNum              4           \
            P5QNum              4           \
            P6QNum              4           \
            };                               
        # -----------------------------------
set port_thrList   {\
            P_MaxDSC_FCON        320        \
            P_MaxDSC_FCOFF       258        \
            QG0_DSC_FCON         42         \
            QG0_DSC_FCOFF        33         \
            QG1_DSC_FCON         42         \
            QG1_DSC_FCOFF        33         \
            QG2_DSC_FCON         42         \
            QG2_DSC_FCOFF        33         \
            QG0_QLEN_FCON        24         \
            QG0_QLEN_FCOFF       20         \
            QG1_QLEN_FCON        24         \
            QG1_QLEN_FCOFF       20         \
            QG2_QLEN_FCON        24         \
            QG2_QLEN_FCOFF       20         \
            } ;

	Flow control turn off/on descriptor threshold:
		Flow Control OFF Threshold = 2 * QG_DSC_FCOFF[4:0]
		Flow Control ON Threshold = 2 * QG_DSC_FCON[4:0]
		so QG_DSC_FCON[4:0] = 42/2 = 21; QG_DSC_FCOFF[4:0] = 33/2 = 16

	Flow control turn off/on packet threshold:
		OFF Threshold = 4 * QG_QLEN_FCOFF[3:0]
		ON Threshold = 4 * QG_QLEN_FCON[3:0]
		so QG_QLEN_FCON[3:0] = 24/4 = 6; QG_QLEN_FCOFF[3:0] = 20/4 = 5
 */
	int i, j;

	/* System Based Flow Control Threshold Register */
	WRITE_MEM32(SBFCR0, (READ_MEM32(SBFCR0) & ~(S_DSC_RUNOUT_MASK)) | (FC_S_DSC_RUNOUT << S_DSC_RUNOUT_OFFSET));
	WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( FC_S_DSC_FCON<< S_DSC_FCON_OFFSET) | (FC_S_DSC_FCOFF << S_DSC_FCOFF_OFFSET));
	WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (FC_S_MaxSBuf_FCON << S_Max_SBuf_FCON_OFFSET) | (FC_S_MaxSBuf_FCOFF << S_Max_SBuf_FCOFF_OFFSET));

    	REG32(QNUMCR) = P0QNum_4 | P1QNum_4 | P2QNum_4 | P3QNum_4 | P4QNum_4| P6QNum_4;
	
	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(0, i, 21, 16);
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(PHY0+j, i, 21, 16);

		rtl8651_setAsicQueuePacketBasedFlowControlRegister(0, i, 6, 5);
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueuePacketBasedFlowControlRegister(PHY0+j, i, 6, 5);
	}
		
	rtl8651_setAsicPortBasedFlowControlRegister(0, FC_P_MaxDSC_FCON, FC_P_MaxDSC_FCOFF);
	for(j=1;j<=CPU;j++)
		rtl8651_setAsicPortBasedFlowControlRegister(PHY0+j, FC_P_MaxDSC_FCON, FC_P_MaxDSC_FCOFF);

	WRITE_MEM32(PQPLGR, (READ_MEM32(PQPLGR) & ~(QLEN_GAP_MASK)) | (0x48 << QLEN_GAP_OFFSET)); 		

	// open all port Queue 5 flow control
	WRITE_MEM32(FCCR0, 0x3f3f3f3f);
	WRITE_MEM32(FCCR1, 0x3f3f3f3f);
}
#endif /*  __KERNEL__ */


// EEE PHY -- Page 4
// register 16
#define P4R16_eee_10_cap                           (1 << 13)	// enable EEE 10M
#define P4R16_eee_nway_en                           (1 << 12)	// enable Next Page exchange in nway for EEE 100M
#define P4R16_tx_quiet_en                            (1 << 9)	// enable ability to turn off pow100tx when TX Quiet state
#define P4R16_rx_quiet_en                            (1 << 8)	// enable ability to turn off pow100rx when RX Quiet state

// register 25
#define P4R25_rg_dacquiet_en                            (1 << 10)	// enable ability to turn off DAC when TX Quiet state
#define P4R25_rg_ldvquiet_en                            (1 << 9)		// enable ability to turn off line driver when TX Quiet state
#define P4R25_rg_eeeprg_rst                            (1 << 6)		// reset for EEE programmable finite state machine
#define P4R25_rg_ckrsel                            		(1 << 5)		// select ckr125 as RX 125MHz clock
#define P4R25_rg_eeeprg_en                            (1 << 4)		// enable EEE programmable finite state machine

static const unsigned short phy_data[]={
	0x5000,  // write, address 0
	0x6000,  // write, address 1
	0x7000,  // write, address 2
	0x4000,  // write, address 3
	0xD36C,  // write, address 4
	0xFFFF,  // write, address 5
	0x5060,  // write, address 6
	0x61C5,  // write, address 7
	0x7000,  // write, address 8
	0x4001,  // write, address 9
	0x5061,  // write, address 10
	0x87F5,  // write, address 11
	0xCE60,  // write, address 12
	0x0026,  // write, address 13
	0x8E03,  // write, address 14
	0xA021,  // write, address 15
	0x300B,  // write, address 16
	0x58A0,  // write, address 17
	0x629C,  // write, address 18
	0x7000,  // write, address 19
	0x4002,  // write, address 20
	0x58A1,  // write, address 21
	0x87EA,  // write, address 22
	0xAE25,  // write, address 23
	0xA018,  // write, address 24
	0x3016,  // write, address 25
	0x6894,  // write, address 26
	0x6094,  // write, address 27
	0x5123,  // write, address 28
	0x63C2,  // write, address 29
	0x5127,  // write, address 30
	0x4003,  // write, address 31
	0x87E0,  // write, address 32
	0x8EF3,  // write, address 33
	0xA10E,  // write, address 34
	0xCC40,  // write, address 35
	0x0007,  // write, address 36
	0xCA40,  // write, address 37
	0xFFE0,  // write, address 38
	0xA202,  // write, address 39
	0x3020,  // write, address 40
	0x7008,  // write, address 41
	0x3020,  // write, address 42
	0xCC44,  // write, address 43
	0xFFF4,  // write, address 44
	0xCC44,  // write, address 45
	0xFFF2,  // write, address 46
	0x3000,  // write, address 47
	0x5220,  // write, address 48
	0x4004,  // write, address 49
	0x3000,  // write, address 50
	0x64A0,  // write, address 51
	0x5429,  // write, address 52
	0x4005,  // write, address 53
	0x87CA,  // write, address 54
	0xCE18,  // write, address 55
	0xFFC8,  // write, address 56
	0xCE64,  // write, address 57
	0xFFD0,  // write, address 58
	0x3036,  // write, address 59
	0x65C0,  // write, address 60
	0x50A9,  // write, address 61
	0x4006,  // write, address 62
	0xA3DB,  // write, address 63
	0x303F,  // write, address 64
};

static int ram_code_done=0;

void set_ram_code(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data)/sizeof(unsigned short);

	if (ram_code_done)
		return;
	
	rtl8651_getAsicEthernetPHYReg( 4, 0x19, &reg );
	
	// turn on rg_eeeprg_rst
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));

	// turn on mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	// begin to write all RAM
	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data[i]);
	}
	
	for(i=0;i<63;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, 0);
	}	
	
	// finish reading all RAM
	// turn off mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);

	// turn off rg_eeeprg_rst, enable EEE programmable finite state machine
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

static const unsigned short phy_data_b[]={
  0x5000,  // write, address 0
  0x6000,  // write, address 1
  0x7000,  // write, address 2
  0x4000,  // write, address 3
  0x8700,  // write, address 4
  0xD344,  // write, address 5
  0xFFFF,  // write, address 6
  0xCA6C,  // write, address 7
  0xFFFD,  // write, address 8
  0x5460,  // write, address 9
  0x61C5,  // write, address 10
  0x7000,  // write, address 11
  0x4001,  // write, address 12
  0x5461,  // write, address 13
  0x4001,  // write, address 14
  0x87F1,  // write, address 15
  0xCE60,  // write, address 16
  0x0026,  // write, address 17
  0x8E03,  // write, address 18
  0xA021,  // write, address 19
  0x300F,  // write, address 20
  0x5CA0,  // write, address 21
  0x629C,  // write, address 22
  0x7000,  // write, address 23
  0x4002,  // write, address 24
  0x5CA1,  // write, address 25
  0x87E6,  // write, address 26
  0xAE25,  // write, address 27
  0xA018,  // write, address 28
  0x301A,  // write, address 29
  0x6E94,  // write, address 30
  0x6694,  // write, address 31
  0x5523,  // write, address 32
  0x63C2,  // write, address 33
  0x5527,  // write, address 34
  0x4003,  // write, address 35
  0x87DC,  // write, address 36
  0x8EF3,  // write, address 37
  0xA10E,  // write, address 38
  0xCC40,  // write, address 39
  0x0007,  // write, address 40
  0xCA40,  // write, address 41
  0xFFDF,  // write, address 42
  0xA202,  // write, address 43
  0x3024,  // write, address 44
  0x7008,  // write, address 45
  0x3024,  // write, address 46
  0xCC44,  // write, address 47
  0xFFF4,  // write, address 48
  0xCC44,  // write, address 49
  0xFFF2,  // write, address 50
  0x3000,  // write, address 51
  0x5620,  // write, address 52
  0x4004,  // write, address 53
  0x3000,  // write, address 54
  0x64A0,  // write, address 55
  0x5429,  // write, address 56
  0x4005,  // write, address 57
  0x87C6,  // write, address 58
  0xCE18,  // write, address 59
  0xFFC4,  // write, address 60
  0xCE64,  // write, address 61
  0xFFCF,  // write, address 62
  0x303A,  // write, address 63
  0x65C0,  // write, address 64
  0x54A9,  // write, address 65
  0x4006,  // write, address 66
  0xA3DB,  // write, address 67
  0x3043,  // write, address 68
};

void set_ram_code_b(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data_b)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	rtl8651_getAsicEthernetPHYReg(4, 0x19, &reg );
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));	
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data_b[i]);
	}
	
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

void eee_phy_enable_by_port(int port)
{
	uint32 reg;

	// change to page 4
	rtl8651_setAsicEthernetPHYReg(port, 31, 4);

	// enable EEE N-way & set turn off power at quiet state
	rtl8651_getAsicEthernetPHYReg( port, 16, &reg );
	reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);

#ifdef CONFIG_RTL8196C_ETH_IOT	
	reg |= P4R16_eee_10_cap;	// enable 10M_EEE also.
#endif	
	rtl8651_setAsicEthernetPHYReg( port, 16, reg );

	// enable EEE turn off DAC and line driver when TX Quiet state
	rtl8651_getAsicEthernetPHYReg( port, 25, &reg );
//	reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
	reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);
	
	rtl8651_setAsicEthernetPHYReg( port, 25, reg );

	rtl8651_setAsicEthernetPHYReg( port, 17, 0xa2a2 );
	rtl8651_setAsicEthernetPHYReg( port, 19, 0xc5c2 );
	rtl8651_setAsicEthernetPHYReg( port, 24, 0xc0f3 );

	if ((REG32(REVR) == RTL8196C_REVISION_A) && (port == 4)) {
		set_ram_code();
	}
	else if ((REG32(REVR) == RTL8196C_REVISION_B) && (port == 4)) {
		set_ram_code_b();
	}
	
	// switch to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
}

#ifdef DBG_EEE
void eee_phy_enable(void)
{
	int i;
	uint32 reg;

	// EEE PHY enable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// enable EEE N-way & set turn off power at quiet state
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// enable EEE turn off DAC and line driver when TX Quiet state
		rtl8651_getAsicEthernetPHYReg( i, 25, &reg );
//		reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
		reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);
		rtl8651_setAsicEthernetPHYReg( i, 25, reg );

		rtl8651_setAsicEthernetPHYReg( i, 17, 0xa2a2 );
		rtl8651_setAsicEthernetPHYReg( i, 19, 0xc5c2 );
		rtl8651_setAsicEthernetPHYReg( i, 24, 0xc0f3 );

		if ((REG32(REVR) == RTL8196C_REVISION_A) && (i == 4)) {
			set_ram_code();
		}
		else if ((REG32(REVR) == RTL8196C_REVISION_B) && (i == 4)) {
			set_ram_code_b();
		}
		
		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1, i);				
	}
}

void eee_phy_disable(void)
{
	int i;
	uint32 reg;

	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// disable (EEE N-way & turn off power at quiet state)
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg = reg & 0xECFF;
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1, i);				
	}
}
#endif
#endif

void set_phy_pwr_save(int id, int val)
{
	uint32 reg_val;	
	int i, start, end;
	
	if (id == 99)
		{ start=0; end=4; }
	else if (id <= 4)
		{ start = end = id; }
	else
		return;
	for(i=start; i<=end; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 24, &reg_val);

		if (val == 1)
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val | BIT(15)) );
		else 
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val & (~BIT(15))) );
	}
}

#ifdef DBG_PHY_REG
void phy_op(uint32 op, uint32 id, uint32 page, uint32 reg, uint32 *val)
{
	int i, start, end;
	
	if ((op == 1) && (id == 99))
		{ start=0; end=4; }
	else if (id <= 4)
		{ start = end = id; }
	else
		return;
	
	for(i=start; i<=end; i++)
	{
		//change page 
		if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( i, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( i, 30, page  );
		}
		else if (page)
		{
			rtl8651_setAsicEthernetPHYReg( i, 31, page  );
		}

		if (op == 1) //read
			rtl8651_getAsicEthernetPHYReg( i, reg, val);
		else 
			rtl8651_setAsicEthernetPHYReg( i, reg, *val);

		//switch to page 0
		if (page)
			rtl8651_setAsicEthernetPHYReg( i, 31, 0  );

		//if (op == 0)
		//	rtl8651_restartAsicEthernetPHYNway(i+1, i);				
			
		panic_printk("  PHY: id= %d, page= %d, reg= %d, val= 0x%x.\n", 
			i, page, reg, *val);
	}
}

void phy_read_all(void)
{
	int i, page, reg;
	uint32 val;
	
	for(i=0; i<5; i++)
	{
		for(page=0; page<5; page++) {
			//change page 
			 if (page)
				rtl8651_setAsicEthernetPHYReg(i, 31, page);
		
			for(reg=0; reg<31; reg++) {
				rtl8651_getAsicEthernetPHYReg( i, reg, &val);
				panic_printk("id= %d, page= %d, reg= %02d, val= 0x%04x\n", 
					i, page, reg, val);
			}
			panic_printk("------------------------------------\n");
		}
		//switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0);
	}
}
#endif

#if defined CONFIG_RTL_MLD_SNOOPING 

void setACL_trap_ipv6_mcast(int index)
{
    
        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));

	    printk("setACL_trap_ipv6_mcast\n");

        ap.actionType_ = RTL8651_ACL_CPU;
        ap.ruleType_ = RTL8651_ACL_MAC;
        
        ap.un_ty.MAC._dstMac.octet[0]=0x33;
        ap.un_ty.MAC._dstMac.octet[1]=0x33;
        ap.un_ty.MAC._dstMac.octet[2]=0x00;
        ap.un_ty.MAC._dstMac.octet[3]=0x00;
        ap.un_ty.MAC._dstMac.octet[4]=0x00;
        ap.un_ty.MAC._dstMac.octet[5]=0x00;	
		
        ap.un_ty.MAC._dstMacMask.octet[0]=0xFF;
        ap.un_ty.MAC._dstMacMask.octet[1]=0xFF;
		
        
        ap.un_ty.MAC._typeLenMask = 0;
        ap.un_ty.MAC._typeLen = 0;
        ret = swCore_aclCreate(index, &ap);
        
        if ( ret != 0 ) { 
            printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );

        }    
	
} 


void setACL_fwd_ipv6_mcast(int index)	// ping6_issue
{
        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));

        ap.actionType_ = RTL8651_ACL_PERMIT;
        ap.ruleType_ = RTL8651_ACL_MAC;
	
        ap.un_ty.MAC._dstMac.octet[0]=0x33;
        ap.un_ty.MAC._dstMac.octet[1]=0x33;
        ap.un_ty.MAC._dstMac.octet[2]=0xFF;
		
        ap.un_ty.MAC._dstMacMask.octet[0]=0xFF;
        ap.un_ty.MAC._dstMacMask.octet[1]=0xFF;
        ap.un_ty.MAC._dstMacMask.octet[2]=0xFF;		
	
        ret = swCore_aclCreate(index, &ap);
        
        if ( ret != 0 ) { 
            printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );
        }    	
}  
#endif
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)

void setACL_fwd_ipv4_mcast(int index)	// permit ipv4 multicast packet
{
        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));
		#if 1
		ap.pktOpApp = RTL865XC_ACLTBL_ALL_LAYER;
        ap.actionType_ = RTL8651_ACL_PERMIT;
        ap.ruleType_ = RTL8651_ACL_IP;
	
        ap.dstIpAddr_=0xE0000000;
        ap.dstIpAddrMask_=0xF0000000;
        ap.ipProto_ = 0x11;
		ap.ipProtoMask_ =0xFF;
		
		#endif
        		
        ret = swCore_aclCreate(index, &ap);
        
        if ( ret != 0 ) { 
            printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );
        }    	
}  
#endif
#if 1//defined CONFIG_RTL_MLD_SNOOPING || defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)|| defined(CONFIG_RTL_VLAN_SUPPORT)
void EasyACLRule(int index, int action)
{

        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));		
		
	switch(action)
	{
		case ACL_IPV6_TO_CPU:
       		ap.actionType_ = RTL8651_ACL_CPU;
        	ap.ruleType_ = RTL8651_ACL_MAC;
    		ap.un_ty.MAC._typeLenMask = 0xffff;
		    ap.un_ty.MAC._typeLen = 0x86dd;	
			//diag_printf("index:%d,action:%d,[%s]:[%d].\n",index,action,__FUNCTION__,__LINE__);
			break;			
		default:
		ap.actionType_ = action;
	       		break;
	}
			
		ap.pktOpApp = RTL865XC_ACLTBL_ALL_LAYER;		
		ret = swCore_aclCreate(index, &ap);
        
       if ( ret != 0 ) { 
           printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );
       } 
		
}

void showAclRule(void)
{
	char *actionT[] = {"permit", "redirect to ether", "drop", "to cpu", "legacy drop", 
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match", 
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
					};

	rtl_acl_param_t asic_acl;
	unsigned short index;


	diag_printf("\n%s\n", "ASIC ACL Table:");
	
	for(index=0; index<3; index++)
	{
		rtl8651_getAsicAclRule(index, &asic_acl);
		
		diag_printf("%s\n", "	Wan Netif Acl Rules:");
		switch(asic_acl.ruleType_)
		{
				
			case RTL8651_ACL_MAC:
				diag_printf(" [%d] rule type: %s   rule action: %s\n", index, "Ethernet", actionT[asic_acl.actionType_]);
				diag_printf("\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				diag_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);
				
				diag_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
			break;
			case RTL865X_ACL_IP:
				diag_printf(" [%d] rule type: %s   rule action: %s\n", asic_acl.aclIdx, "IP", actionT[asic_acl.actionType_]);
				diag_printf( "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				diag_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				diag_printf("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				diag_printf("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				diag_printf( "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_,asic_acl.ipMF_);
				break;
			default:
			break;
		}
		diag_printf("pktOpApp: %d\n", asic_acl.pktOpApp);
	}

	for(index=10; index<13; index++)
	{
		rtl8651_getAsicAclRule(index, &asic_acl);
		diag_printf("%s\n", "	Lan Netif Acl Rules:");
		switch(asic_acl.ruleType_)
		{
			case RTL8651_ACL_MAC:
				diag_printf(" [%d] rule type: %s   rule action: %s\n", index, "Ethernet", actionT[asic_acl.actionType_]);
				diag_printf("\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				diag_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);
				
				diag_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
			break;
			case RTL865X_ACL_IP:
				diag_printf(" [%d] rule type: %s   rule action: %s\n", asic_acl.aclIdx, "IP", actionT[asic_acl.actionType_]);
				diag_printf( "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				diag_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				diag_printf("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				diag_printf("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				diag_printf( "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
				break;
			default:
			break;
		}
		diag_printf("pktOpApp: %d\n", asic_acl.pktOpApp);
	}
}
#endif
