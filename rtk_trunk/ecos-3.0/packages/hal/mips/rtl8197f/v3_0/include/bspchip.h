/*
 * Realtek Semiconductor Corp.
 *
 * bsp/bspchip.h:
 *   bsp chip address and IRQ mapping file
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#ifndef _BSPCHIP_H_
#define _BSPCHIP_H_

#define BSP_HZ 100

/*
 * Register access macro
 */
#define REG32(reg)	(*(volatile unsigned int   *)((unsigned int)reg))
#define REG16(reg)	(*(volatile unsigned short *)((unsigned int)reg))
#define REG08(reg)	(*(volatile unsigned char  *)((unsigned int)reg))
#define REG8(reg)   (*(volatile unsigned char  *)((unsigned int)reg))

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)

/*
 * IRQ Controller
 */
#define BSP_IRQ_ICTL_BASE  0
#define BSP_IRQ_ICTL_NUM   64

#define BSP_IRQ_CPU_BASE   (BSP_IRQ_ICTL_BASE + BSP_IRQ_ICTL_NUM)
#define BSP_IRQ_CPU_NUM    5


#define BSP_BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */


/*
 * IRQ Mapping
 */
#ifdef CONFIG_TOCPU_MASK
#define BSP_GIMR_TOCPU_MASK      (BSP_WLAN_MAC_IE|BSP_SW_IE|BSP_TC0_IE)
#define BSP_TC0_IRQ   (BSP_IRQ_CPU_BASE + 4)  //7
#define BSP_SW_IRQ    (BSP_IRQ_CPU_BASE + 1) //15
#define BSP_WLAN_MAC_IRQ        (BSP_IRQ_CPU_BASE + 3)  //29
#else
#define BSP_GIMR_TOCPU_MASK      (0x0)
#define BSP_TC0_IRQ   7
#define BSP_SW_IRQ        15
#define BSP_WLAN_MAC_IRQ         29
#endif

#define BSP_GIMR_TOCPU_MASK2    (0x0)
#ifdef CONFIG_RTL_819X_INTERNAL_TIMER
#define BSP_TC0_IRQ		7
#define BSP_SITIMER_IRQ		(32+15)
//#define BSP_SITIMER_IRQ		(BSP_IRQ_CPU_BASE + 4)	//GIMR2, 15
#else
#endif
#define BSP_UART0_IRQ           9
//#define BSP_UART0_IRQ           (BSP_IRQ_CPU_BASE + 0) //9
#define BSP_UART1_IRQ           13
#define BSP_SWCORE_IRQ	 BSP_SW_IRQ
#define BSP_GPIO_ABCD_IRQ       16
#define BSP_GPIO_EFGH_IRQ       17
#define BSP_NFBI_IRQ            18
#define BSP_PCM_IRQ             19
#define BSP_SECURITY_IRQ        20
#define BSP_PCIE_IRQ		    21
//#define BSP_PCIE_IRQ		(BSP_IRQ_CPU_BASE + 2) //21
#define BSP_PCIE2_IRQ           22
#define BSP_GDMA_IRQ            23
#define BSP_CPU_WAKEUP_IRQ	27

/*-------------------------------------------------------------------------*/

/*
 * UART
 */
#define BSP_UART0_BAUD		38400
#ifdef CONFIG_RTK_FPGA
#define BSP_UART0_FREQ		(BSP_CPU0_FREQ/8) // switch
//#define BSP_UART0_FREQ		(BSP_CPU0_FREQ*12/10) // USB OTG, pcie
#else
#define BSP_UART0_FREQ		(100000000) // 100MHz
#endif
#define BSP_UART0_BAUD_DIVISOR  ((BSP_UART0_FREQ >> 4) / BSP_UART0_BAUD)

#define BSP_UART0_BASE		0xB8147000
#define BSP_UART0_MAP_BASE	0x18147000
#define BSP_UART0_MAPSIZE	0x100
#define BSP_UART0_RBR       (BSP_UART0_BASE + 0x024)
#define BSP_UART0_THR       (BSP_UART0_BASE + 0x024)
#define BSP_UART0_DLL       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_IER       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_DLM       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_IIR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_FCR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_LCR       (BSP_UART0_BASE + 0x00C)
#define BSP_UART0_MCR       (BSP_UART0_BASE + 0x010)
#define BSP_UART0_LSR       (BSP_UART0_BASE + 0x014)
#define BSP_UART0_MSR       (BSP_UART0_BASE + 0x018)
#define BSP_UART0_SCR       (BSP_UART0_BASE + 0x01C)
#define BSP_UART0_STSR      (BSP_UART0_BASE + 0x020)
#define BSP_UART0_MISCR    (BSP_UART0_BASE + 0x028)
#define BSP_UART0_TXPLSR   (BSP_UART0_BASE + 0x02C)

#define BSP_UART1_BASE		0xB8147400
#define BSP_UART1_MAP_BASE	0x18147400
#define BSP_UART1_RBR       (BSP_UART1_BASE + 0x024)
#define BSP_UART1_THR       (BSP_UART1_BASE + 0x024)
#define BSP_UART1_DLL       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_IER       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_DLM       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_IIR       (BSP_UART1_BASE + 0x008)
#define BSP_UART1_FCR       (BSP_UART1_BASE + 0x008)
   #define BSP_FCR_EN          0x01
   #define BSP_FCR_RXRST       0x02
   #define     BSP_RXRST             0x02
   #define BSP_FCR_TXRST       0x04
   #define     BSP_TXRST             0x04
   #define BSP_FCR_DMA         0x08
   #define BSP_FCR_RTRG        0xC0
   #define     BSP_CHAR_TRIGGER_01   0x00
   #define     BSP_CHAR_TRIGGER_04   0x40
   #define     BSP_CHAR_TRIGGER_08   0x80
   #define     BSP_CHAR_TRIGGER_14   0xC0
#define BSP_UART1_LCR       (BSP_UART1_BASE + 0x00C)
   #define BSP_LCR_WLS0        0x01
   #define     BSP_CHAR_LEN_7        0x00
   #define     BSP_CHAR_LEN_8        0x01
   #define BSP_LCR_STB         0x04
   #define     BSP_ONE_STOP          0x00
   #define     BSP_TWO_STOP          0x04
   #define BSP_LCR_PEN         0x08
   #define     BSP_PARITY_ENABLE     0x01
   #define     BSP_PARITY_DISABLE    0x00
   #define BSP_LCR_EPS         0x30
   #define     BSP_PARITY_ODD        0x00
   #define     BSP_PARITY_EVEN       0x10
   #define     BSP_PARITY_MARK       0x20
   #define     BSP_PARITY_SPACE      0x30
   #define BSP_LCR_BRK         0x40
   #define BSP_LCR_DLAB        0x80
   #define     BSP_DLAB              0x80
#define BSP_UART1_MCR       (BSP_UART1_BASE + 0x010)
#define BSP_UART1_LSR       (BSP_UART1_BASE + 0x014)
   #define BSP_LSR_DR          0x01
   #define     BSP_RxCHAR_AVAIL      0x01
   #define BSP_LSR_OE          0x02
   #define BSP_LSR_PE          0x04
   #define BSP_LSR_FE          0x08
   #define BSP_LSR_BI          0x10
   #define BSP_LSR_THRE        0x20
   #define     BSP_TxCHAR_AVAIL      0x00
   #define     BSP_TxCHAR_EMPTY      0x20
   #define BSP_LSR_TEMT        0x40
   #define BSP_LSR_RFE         0x80
#define BSP_UART1_MSR       (BSP_UART1_BASE + 0x018)
#define BSP_UART1_SCR       (BSP_UART1_BASE + 0x01C)
#define BSP_UART1_STSR      (BSP_UART1_BASE + 0x020)
#define BSP_UART1_MISCR    (BSP_UART1_BASE + 0x028)
#define BSP_UART1_TXPLSR   (BSP_UART1_BASE + 0x02C)

/*
 * Interrupt Controller
 */
 
/*
---------------------------
GISR 
Bit	Bit Name
31	GISR2_IP
30	DW_I2C_0_IP
29	WLAN_MAC_IP
28	USB0_WAKE_IP
27	CPU_WAKE_IP
26	I2S_IP
25	USB1_WAKE_IP
24	EFUSE_CTRL_IP
23	DW_SSI_0_IP
22	DHC_NAND_IP
21	PCIE0_IP
20	SECURITY_IP
19	PCM_IP
18	NFBI_IP
17	GPIO_EFGH_IP
16	GPIO_ABCD_IP
15	SW_IP
14	SD30_IP
13	USB_H_IP
12	USB_O_IP
11	DW_APB_TIMER_IP
10	TC3_IP
9	DW_UART_0_IP
8	DW_GDMA_IP
7	TC0_IP
6	
5	
4	
3	
2	POK33V_L_IP
1	OTG_CTRL_IP
0	NAND_CTRL_IP
----------------------------
GISR2
31	LX2_S_BTRDY_IP
30	LX1_S_BTRDY_IP
29	LX0_S_BTRDY_IP
28	LX2_BTRDY_IP
27	LX1_BTRDY_IP
26	LX0_BTRDY_IP
25	LX2_BFRAME_IP
24	LX1_BFRAME_IP
23	LX0_BFRAME_IP
22	
21	
20	
19	
18	DPI_DLL_IP
17	RXI300_IP
16	RXI310_SPIC_IP
15	CPU_SI_TIMER_IP
14	CPU_SI_PC_IP
13	
12	
11	
10	SWR_DDR_OVER_LOAD_IP
9	Spi_flashecc_IP
8	Spi_nand_IP
7	DW_UART_2_IP
6	DW_UART_1_IP
5	DW_SSI_1_IP
4	
3	DW_I2C_1_IP
2	
1	TC2_IP
0	TC1_IP
---------------------------------
IRR0 IP#[07,06,05,04, 03,02,01,00]
IRR1 IP#[15,14,13,12, 11,10,09,08]
IRR2 IP#[23,22,21,20, 19,18,17,16]
IRR3 IP#[31,30,29,28, 27,26,25,24]


*/

#define SYSTEM_REG_BASE 	0xB8000000
#define BSP_HW_STRAP		(SYSTEM_REG_BASE + 0x8)
#define BSP_BOND_OPTION		(SYSTEM_REG_BASE + 0xC)

#define BSP_GIMR            0xB8003000
    #define BSP_WLAN_MAC_IE     (1 << 29)
    #define BSP_PCIE_IE               (1 << 21)
    #define BSP_SW_IE                 (1 << 15)
    #define BSP_UART0_IE            (1 << 9)
    #define BSP_TC0_IE                (1 << 7)
#define BSP_GISR            0xB8003004
#define BSP_IRR0            0xB8003008
#define BSP_IRR1            0xB800300C
#define BSP_IRR2            0xB8003010
#define BSP_IRR3            0xB8003014

#define BSP_GIMR2           0xB8003020
    #define BSP_CPU_SI_TIMER_IE     (1 << 15)
    #define BSP_CPU_SI_PC_IE           (1 << 14)
#define BSP_GISR2           0xB8003024
#define BSP_IRR4            0xB8003028
#define BSP_IRR5            0xB800302C
#define BSP_IRR6            0xB8003030
#define BSP_IRR7            0xB8003034

#define IRR_IPtoCPU(a7,a6,a5,a4,a3,a2,a1,a0) ((a7<<28)|(a6<<24)|(a5<<20)|(a4<<16)|(a3<<12)|(a2<<8)|(a1<<4)|(a0<<0))

#ifdef CONFIG_TOCPU_MASK
#define BSP_IRR0_SETTING    IRR_IPtoCPU(7,0,0,0, 0,0,0,0)   //IP [7~0]
#define BSP_IRR1_SETTING    IRR_IPtoCPU(4,0,0,0, 0,0,2,0) //IP [15~8]
#define BSP_IRR2_SETTING    IRR_IPtoCPU(0,0,2,0, 0,0,0,0) //IP [23~16]
#define BSP_IRR3_SETTING    IRR_IPtoCPU(0,0,6,0, 0,0,0,0) //IP [31~24]
#else
#define BSP_IRR0_SETTING    IRR_IPtoCPU(2,0,0,0, 0,0,0,0)   //IP [7~0]
#define BSP_IRR1_SETTING    IRR_IPtoCPU(2,0,0,0, 0,0,2,0)	//IP [15~8]
#define BSP_IRR2_SETTING    IRR_IPtoCPU(0,0,2,0, 0,0,0,0)	//IP [23~16]
#define BSP_IRR3_SETTING    IRR_IPtoCPU(0,0,2,0, 0,0,0,0)	//IP [31~24]
#endif

#define BSP_IRR4_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)
#ifdef CONFIG_RTL_819X_INTERNAL_TIMER
#define BSP_IRR5_SETTING    IRR_IPtoCPU(2,0,0,0, 0,0,0,0)
#else
#define BSP_IRR5_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)
#endif
#define BSP_IRR6_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)
#define BSP_IRR7_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)

/*
 * Timer/Counter
 */
#define BSP_TC_BASE         0xB8003100
#define BSP_TC0DATA         (BSP_TC_BASE + 0x00)
#define BSP_TC1DATA         (BSP_TC_BASE + 0x04)
   #define BSP_TCD_OFFSET      4
#define BSP_TC0CNT          (BSP_TC_BASE + 0x08)
#define BSP_TC1CNT          (BSP_TC_BASE + 0x0C)
#define BSP_TCCNR           (BSP_TC_BASE + 0x10)
   #define BSP_TC0EN           (1 << 31)
   #define BSP_TC0MODE_TIMER   (1 << 30)
   #define BSP_TC1EN           (1 << 29)
   #define BSP_TC1MODE_TIMER   (1 << 28)
#define BSP_TCIR            (BSP_TC_BASE + 0x14)
   #define BSP_TC0IE           (1 << 31)
   #define BSP_TC1IE           (1 << 30)
   #define BSP_TC0IP           (1 << 29)
   #define BSP_TC1IP           (1 << 28)
#define BSP_CDBR            (BSP_TC_BASE + 0x18)
   #define BSP_DIVF_OFFSET     16
#define BSP_WDTCNR          (BSP_TC_BASE + 0x1C)

#define BSP_DIVISOR         200

#if BSP_DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 * Wifi
 */
#define BSP_WLAN_BASE_ADDR      0xB8640000UL
#define BSP_WLAN_CONF_ADDR      0x00000000UL //for compiler's happy

/*
 * PCI-E 
 */
#define BSP_PCIE_RC_CFG		0xb8b00000UL
#define BSP_PCIE_EP_CFG		0xb8b10000UL
#define BSP_PCIE_IO_BASE	0xb8c00000UL
#define BSP_PCIE_IO_SIZE	0x00200000UL
//#define BSP_PCIE_IO_MAPBASE	0x19200000UL
//#define BSP_PCIE_IO_MAPSIZE	0x00200000UL
#define BSP_PCIE_MEM_BASE	0xb9000000UL
#define BSP_PCIE_MEM_SIZE	0x00c00000UL
//#define BSP_PCIE_MEM_MAPBASE	0x19400000UL
//#define BSP_PCIE_MEM_MAPSIZE	0x00c00000UL

#define BSP_PCIE_RC_EXTENDED_REG	0xb8b01000

#define BSP_PCIE0_H_CFG     0xB8B00000
#define BSP_PCIE0_H_EXT     0xB8B01000
#define BSP_PCIE0_H_MDIO    (BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR  (BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR   (BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG   (BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC    (BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE0_D_CFG0    0xB8B10000
#define BSP_PCIE0_D_MEM     0xB9000000

#define BSP_PCIE1_H_CFG   0x00000000 //for compiler's happy
#define BSP_PCIE1_H_PWRCR 0x00000000 //for compiler's happy
#define BSP_PCIE1_D_CFG0  0x00000000 //for compiler's happy

/*
 * USB
 */
#define BSP_USB_USB3_BASE	0xb8000000UL
#define BSP_USB_USB3_SIZE	0x00100000UL
#define BSP_USB_USB3_MAPBASE	0x18000000UL
#define BSP_USB_USB3_MAPSIZE	0x00100000UL
#define BSP_USB_OTG_BASE	0xb8100000UL
#define BSP_USB_OTG_SIZE	0x00040000UL
#define BSP_USB_OTG_MAPBASE	0x18100000UL
#define BSP_USB_OTG_MAPSIZE	0x00040000UL

/*
 * SPIC
 */
#define BSP_SPIC_BASE		0x18143000UL    // 0xB8143000UL
#define BSP_SPIC_SIZE		0x00001000UL

#define BSP_SPIC_AUTO_BASE	0x10000000UL    // 0xB0000000
#define BSP_SPIC_AUTO_SIZE	0x02000000UL

/*
 * PIN MUX
 */
#define BSP_PIN_MUX_SEL0			0xB8000800UL
#define BSP_PIN_MUX_SEL1			0xB8000804UL
#define BSP_PIN_MUX_SEL2			0xB8000808UL
#define BSP_PIN_MUX_SEL3			0xB800080CUL
#define BSP_PIN_MUX_SEL4			0xB8000810UL
#define BSP_PIN_MUX_SEL5			0xB8000814UL
#define BSP_PIN_MUX_SEL6			0xB8000818UL
#define BSP_PIN_MUX_SEL7			0xB800081CUL
#define BSP_PIN_MUX_SEL8			0xB8000820UL
#define BSP_PIN_MUX_SEL9			0xB8000824UL
#define BSP_PIN_MUX_SEL10			0xB8000828UL
#define BSP_PIN_MUX_SEL11			0xB800082CUL
#define BSP_PIN_MUX_SEL12			0xB8000830UL
#define BSP_PIN_MUX_SEL13			0xB8000834UL
#define BSP_PIN_MUX_SEL14			0xB8000838UL
#define BSP_PIN_MUX_SEL15			0xB800083CUL
#define BSP_PIN_MUX_SEL16			0xB8000840UL
#define BSP_PIN_MUX_SEL17			0xB8000844UL
#define BSP_PIN_MUX_SEL18			0xB8000848UL

/* GPIO Register Set */
#define BSP_GPIO_PHYS_BASE	(0x18003500)
#define BSP_GPIO_SIZE		(0x38)
#define BSP_GPIO_BASE	(0xB8003500)
#define BSP_PABCD_CNR	(0x000 + BSP_GPIO_BASE) /* Port ABCD control */
#define BSP_PABCD_PTYPE	(0x004 + BSP_GPIO_BASE) /* Port ABCD type */
#define BSP_PABCD_DIR	(0x008 + BSP_GPIO_BASE) /* Port ABCD direction */
#define BSP_PABCD_DAT	(0x00C + BSP_GPIO_BASE) /* Port ABCD data */
#define BSP_PABCD_ISR	(0x010 + BSP_GPIO_BASE) /* Port ABCD interrupt status */
#define BSP_PAB_IMR	(0x014 + BSP_GPIO_BASE) /* Port AB interrupt mask */
#define BSP_PCD_IMR	(0x018 + BSP_GPIO_BASE) /* Port CD interrupt mask */
#define BSP_PEFGH_CNR	(0x01C + BSP_GPIO_BASE) /* Port ABCD control */
#define BSP_PEFGHP_TYPE	(0x020 + BSP_GPIO_BASE) /* Port ABCD type */
#define BSP_PEFGH_DIR	(0x024 + BSP_GPIO_BASE) /* Port ABCD direction */
#define BSP_PEFGH_DAT	(0x028 + BSP_GPIO_BASE) /* Port ABCD data */
#define BSP_PEFGH_ISR	(0x02C + BSP_GPIO_BASE) /* Port ABCD interrupt status */
#define BSP_PEF_IMR	(0x030 + BSP_GPIO_BASE) /* Port AB interrupt mask */
#define BSP_PGH_IMR	(0x034 + BSP_GPIO_BASE) /* Port CD interrupt mask */


#define BSP_CLK_MANAGE 	0xB8000010
#define BSP_PIN_MUX_SEL		0xB8000040
#define BSP_PIN_MUX_SEL_2	0xB8000044
#define BSP_PIN_MUX_SEL_3	0xB800004C

#if defined(__ECOS)
#define SYS_CLK_MANAGE				(SYSTEM_REG_BASE + 0x10)
#define SYS_ENABLE					(SYSTEM_REG_BASE + 0x50)
#define SYS_PCIE_PHY				(SYSTEM_REG_BASE + 0x100)
#define PCIE_RC_EXTENDED_REG_MDIO	(BSP_PCIE_RC_EXTENDED_REG + 0x00)
#define PCIE_RC_EXTENDED_REG_PWRCR	(BSP_PCIE_RC_EXTENDED_REG + 0x08)
#define PCIE_RC_EXTENDED_REG_IPCFG	(BSP_PCIE_RC_EXTENDED_REG + 0x0c)

// PCIE_RC_EXTENDED_REG_MDIO
#define PCIE_MDIO_DATA_OFFSET 		(16)
#define PCIE_MDIO_REG_OFFSET 		(8)
#define PCIE_MDIO_RDWR_OFFSET 		(0)

/*
 * Bonding Option
 */
#define BSP_BOND_97FB	1
#define BSP_BOND_97FN	2
#define BSP_BOND_97FS	3

#endif
/*
 * Revision
 */
#define BSP_REVR		0xB8000000
#define BSP_RTL8198_REVISION_A	0xC0000000
#define BSP_RTL8198_REVISION_B	0xC0000001
#define BSP_RTL8197D		0x8197C000
#define BSP_RTL8196E		0x8196E000

#endif   /* _BSPCHIP_H */
