
include .config
ifdef MODEL_NAME
ifeq ($(MODEL_NAME),97f-8812br-16M-gw)
	prefix=97F
endif
ifeq ($(MODEL_NAME),97f-8812br-mp)
	prefix=97F
endif
ifeq ($(MODEL_NAME),96e-88e-gw)
	prefix=96E
endif
ifeq ($(MODEL_NAME),96e-92e-gw)
	prefix=96E
endif
ifeq ($(MODEL_NAME),96e-92e-ipv6-gw)
	prefix=96E_ipv6
endif
ifeq ($(MODEL_NAME),97d-8367r-92e-8812-gw)
	prefix=97D_8367
endif
ifeq ($(MODEL_NAME),97dl-92e-8812-gw)
	prefix=97DL
endif
ifeq ($(MODEL_NAME),8881a-88e-gw)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),8881a-92e-gw)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),8881ab-8367rb-92e-gw)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),8881a-CMJ-gw)
ifeq ($(CONFIG_CUTE_MAHJONG_RTK_UI),y)
	prefix=8881A
else
	prefix=8881A-CMJ
endif
endif
ifeq ($(MODEL_NAME),8881a-XDG-gw)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),96eu-88e-gw)
	prefix=96EU
endif
ifeq ($(MODEL_NAME),8881a-92e-ipv6-gw)
	prefix=8881A_ipv6
endif
ifeq ($(MODEL_NAME),8881a-92e-ipv6-tr069-middleEast-gw)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),96e-88e-mp)
	prefix=96E
endif
ifeq ($(MODEL_NAME),96e-92e-mp)
	prefix=96E
endif
ifeq ($(MODEL_NAME),96d-8194-gw)
	prefix=96D
endif
ifeq ($(MODEL_NAME),96e-8194-gw)
	prefix=96E
endif
ifeq ($(MODEL_NAME),97d-8367r-92e-8812-mp)
	prefix=97D_8367
endif
ifeq ($(MODEL_NAME),97dl-92e-8812-mp)
	prefix=97DL
endif
ifeq ($(MODEL_NAME),8881a-88e-mp)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),8881a-92e-mp)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),8881ab-8367rb-92e-mp)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),8881a-CMJ-mp)
	prefix=8881A-CMJ
endif
ifeq ($(MODEL_NAME),8881a-XDG-mp)
	prefix=8881A
endif
ifeq ($(MODEL_NAME),96eu-88e-mp)
	prefix=96EU
endif
endif

ifeq ($(CONFIG_MIPS16),y)
	MAKE += MIPS=mips16_on PREFIX=$(prefix)
else
	MAKE +=  MIPS=mips16_off PREFIX=$(prefix)
endif

# sanity check
ifndef MODEL_NAME
$(error MODEL_NAME is not defined)
endif

export ECOS_TOPDIR=$(shell pwd)/..
export ECOS_WORK_DIR=$(shell pwd)
ifeq ($(CONFIG_SSL),y)
INCLUDE_PATH=-I$(ECOS_WORK_DIR)/AP/openssl-0.9.8o/include/
else
INCLUDE_PATH=-I$(ECOS_WORK_DIR)/AP/wsc/portingssl/crypto/include/
endif
export INCLUDE_PATH
ECOS_REPOSITORY:=$(ECOS_TOPDIR)/ecos-3.0/packages
export ECOS_REPOSITORY

ifeq ($(CONFIG_RTL_8197F),y)
PATH:=$(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/msdk-elf/bin:$(PATH)
else
PATH:=$(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/mips-elf/bin:$(PATH)
endif
export PATH

#------------------------------------------------------------------------------
# define TARGET_NAME
#------------------------------------------------------------------------------
ifeq ($(CONFIG_RTL_8196C),y)
TARGET_NAME=rtl8196c
endif

ifeq ($(CONFIG_RTL_8196E),y)
TARGET_NAME=rtl8196e
endif

ifeq ($(CONFIG_RTL_8196D),y)
TARGET_NAME=rtl819xd
endif

ifeq ($(CONFIG_RTL_8197D),y)
TARGET_NAME=rtl819xd
endif

ifeq ($(CONFIG_RTL_8197DL),y)
TARGET_NAME=rtl819xd
endif

ifeq ($(CONFIG_RTL_8881A),y)
TARGET_NAME=rtl8881a
endif

ifeq ($(CONFIG_RTL_8197F),y)
TARGET_NAME=rtl8197f
endif

#------------------------------------------------------------------------------
# select wlan driver
#------------------------------------------------------------------------------
ifndef WLAN_DRIVER_VERSION

WLAN_DRIVER_VERSION=v3_3
#ifeq ($(CONFIG_RTL_8196C),y)
#ifeq ($(CONFIG_RTL_92C_SUPPORT),y)
#WLAN_DRIVER_VERSION=v3_1
#endif
#endif

#ifeq ($(CONFIG_RTL_8196E),y)
#ifeq ($(CONFIG_RTL_88E_SUPPORT),y)
#WLAN_DRIVER_VERSION=v3_1
#endif
#ifeq ($(CONFIG_RTL_92E_SUPPORT),y)
#WLAN_DRIVER_VERSION=v3_2
#endif
#endif


endif

ifeq ($(CONFIG_NET_STACK_OPENBSD),y)
ifeq ($(CONFIG_NET_STACK_FREEBSD),y)
$(error You should just select either OpenBSD or FreeBSD network stack)
endif
endif


ifndef CONFIG_RAM_SIZE_4M
ifndef CONFIG_RAM_SIZE_8M
ifndef CONFIG_RAM_SIZE_16M
ifndef CONFIG_RAM_SIZE_32M
ifndef CONFIG_RAM_SIZE_64M
$(error You MUST specify the memory size used)
endif
endif
endif
endif
endif

ifeq ($(CONFIG_PPPOE),y)
PPP_SUPPORT=y
export PPP_SUPPORT
else
ifeq ($(CONFIG_L2TP),y)
PPP_SUPPORT=y
export PPP_SUPPORT
else
ifeq ($(CONFIG_PPTP),y)
PPP_SUPPORT=y
export PPP_SUPPORT
endif
endif
endif

ifeq ($(CONFIG_RTL_8196C),y)
defconfig_txt=config-gw-96c.txt
defconfig_dat=config-gw-96c.dat
else
ifeq ($(CONFIG_RTL_8196E),y)
ifeq ($(CONFIG_RTL_ULINKER),y)
defconfig_txt=config-gw-96eu-88e.txt
defconfig_dat=config-gw-96eu-88e.dat
else
ifeq ($(CONFIG_HOME_GATEWAY),y)
defconfig_txt=config-gw-96e-88e.txt
defconfig_dat=config-gw-96e-88e.dat
else
defconfig_txt=config-ap-96e-88e.txt
defconfig_dat=config-ap-96e-88e.dat
endif
endif
ifeq ($(CONFIG_SLOT_0_8192EE),y)
defconfig_txt=config-gw-96e-92e.txt
defconfig_dat=config-gw-96e-92e.dat
endif
else
ifeq ($(CONFIG_RTL_8196D),y)
ifeq ($(CONFIG_SLOT_0_8194AE),y)
defconfig_txt=config-gw-96d-8194.txt
defconfig_dat=config-gw-96d-8194.dat
else
defconfig_txt=config-gw-96d-88e.txt
defconfig_dat=config-gw-96d-88e.dat
endif
endif
ifeq ($(CONFIG_RTL_8197DL),y)
defconfig_txt=config-gw-97dl-8812-92e.txt
defconfig_dat=config-gw-97dl-8812-92e.dat
endif
ifeq ($(CONFIG_RTL_8367R),y)
defconfig_txt=config-gw-97dl-8812-92e.txt
defconfig_dat=config-gw-97d-8367r-8812-92e.dat
endif
endif
endif

ifeq ($(CONFIG_RTL_8881A),y)
defconfig_txt=config-gw-8881a.txt
defconfig_dat=config-gw-8881a.dat
ifeq ($(CONFIG_SLOT_0_8192EE),y)
defconfig_txt=config-gw-8881a-92e.txt
defconfig_dat=config-gw-8881a-92e.dat
endif
ifeq ($(CONFIG_SLOT_0_88E),y)
defconfig_txt=config-gw-8881a-88e.txt
defconfig_dat=config-gw-8881a-88e.dat
endif

ifeq ($(CONFIG_RTL_8881A_SELECTIVE_MODE_SUPPORT),y)
defconfig_txt=config-gw-8881a-selective.txt
defconfig_dat=config-gw-8881a-selective.dat
endif

ifeq ($(CONFIG_CUTE_MAHJONG),y)
defconfig_txt=config-gw-8881a-CMJ.txt
ifeq ($(CONFIG_CUTE_MAHJONG_RTK_UI),y)
defconfig_txt=config-gw-8881a-CMJ-RTK.txt
endif
defconfig_dat=config-gw-8881a-CMJ.dat
endif

ifeq ($(MODEL_NAME),8881a-XDG-gw)
defconfig_txt=config-gw-8881a-selective.txt
defconfig_dat=config-gw-8881a-XDG.dat
endif
endif

ifeq ($(CONFIG_RTL_8197F),y)
ifeq ($(CONFIG_HOME_GATEWAY),y)
ifeq ($(CONFIG_SLOT_0_8822BE),y)
defconfig_txt=config-gw-97f-8822be.txt
ifeq ($(CONFIG_RTL_8367R), y)
defconfig_dat=config-gw-97f-8367-8822be.dat
else
defconfig_dat=config-gw-97f-8822be.dat
endif
else
defconfig_txt=config-gw-97f.txt
defconfig_dat=config-gw-97f.dat
endif
else
defconfig_txt=config-ap-97f.txt
defconfig_dat=config-ap-97f.dat
ifeq ($(CONFIG_SLOT_0_8822BE),y)
ifneq ($(CONFIG_RTL_8367R), y)
defconfig_txt=config-ap-97f-8812br.txt
defconfig_dat=config-ap-97f-8812br.dat
endif
endif
endif
endif
WEB_PAGES_BIN = 
ifeq ($(CONFIG_HTTPD),y)
ifeq ($(CONFIG_IRES_WEB_SUPPORT),y)
WEB_PAGES_BIN = 
else
ifeq ($(CONFIG_KLD),y)
WEB_PAGES_BIN = ./AP/kld/html/webpages.bin
else
ifeq ($(CONFIG_CUTE_MAHJONG),y)
ifeq ($(CONFIG_CUTE_MAHJONG_RTK_UI),y)
WEB_PAGES_BIN =./AP/html/webpages.bin
else 
WEB_PAGES_BIN = ./AP/html_cmj/webpages.bin
endif
else
WEB_PAGES_BIN =./AP/html/webpages.bin
endif
endif
endif
endif

ifeq ($(CONFIG_RLX_PROFILING),y)
MIPS16_SUPPORT=no
WATCHDOG_SUPPORT=no
endif

#------------------------------------------------------------------------------
# sanity check
#------------------------------------------------------------------------------
ifndef TARGET_NAME
$(error TARGET_NAME is not defined)
endif

ifndef WLAN_DRIVER_VERSION
$(error WLAN_DRIVER_VERSION is not defined)
endif

#------------------------------------------------------------------------------
.PHONY: help all clean config ecos ecos-clean app app-clean image flash

help:
	@echo "How to build image:"
	@echo "1) cp defconfig-rtl8196e-88e .config"
	@echo "2) make config"
	@echo "3) make ecos && make app-clean && make app && make image"

all:
	make ecos && make app && make image
#    && make flash
	
config:
	make prepare && make configuration

#检查一些必要的或者相互有依赖的宏是否开启
prepare:
ifneq ($(CONFIG_RTL_REPORT_LINK_STATUS),y)
	$(error please enable CONFIG_RTL_REPORT_LINK_STATUS macro)
endif
ifeq ($(CONFIG_ALG),y)
ifneq ($(CONFIG_NAPT),y)
	$(error If you want to enable alg,please enable natd first)
endif
endif

ifeq ($(CONFIG_BRIDGE_VLAN),y)
ifneq ($(CONFIG_VLAN),y)
	$(error If you want to enable bridge vlan,please enable vlan first)
endif
endif

ifeq ($(CONFIG_FASTPATH),y)
ifneq ($(CONFIG_NAPT),y)
	$(error If you want to enable fastpath,please enable natd first)
endif
endif

ifeq ($(CONFIG_HW_NAT),y)
ifneq ($(CONFIG_NAPT),y)
	$(error If you want to enable hw nat,please enable natd first)
endif
endif

ifeq ($(CONFIG_FIREWALL),y)
ifneq ($(CONFIG_NAPT),y)
	$(error If you want to enable firewall,please enable natd first)
endif
endif

ifeq ($(CONFIG_QOS),y)
ifneq ($(CONFIG_FIREWALL),y)
	$(error If you want to enable qos,please enable natd and firewall first)
else
ifneq ($(CONFIG_NAPT),y)
	$(error If you want to enable qos,please enable natd and firewall first)
endif
endif
endif

ifeq ($(CONFIG_MINIIGD),y)
ifneq ($(CONFIG_FIREWALL),y)
	$(error If you want to enable miniigd,please enable natd and firewall first)
else
ifneq ($(CONFIG_NAPT),y)
	$(error If you want to enable miniigd,please enable natd and firewall first)
endif
endif
endif

ifeq ($(CONFIG_IGMPPROXY_KERNELMODE),y)
ifeq ($(CONFIG_IGMPPROXY_USERMODE),y)
	$(error You should just select either igmp proxy kernel mode or user mode)
endif
endif
ifeq ($(CONFIG_AUTH),y)
ifneq ($(CONFIG_IWCONTROL),y)
	$(error If you want to enable 802.1x authentication,please enable iwcontrol tool first)
endif
endif

ifeq ($(CONFIG_IAPP),y)
ifneq ($(CONFIG_IWCONTROL),y)
	$(error If you want to enable IEEE802.11f(IAPP),please enable iwcontrol tool first)
endif
endif

ifeq ($(CONFIG_HTTPD),y)
ifeq ($(CONFIG_IRES_WEB_ADVANCED_SUPPORT),)
ifneq ($(CONFIG_WEB_COMP_TWICE),y)
#	$(error please enable CONFIG_WEB_COMP_TWICE if you dont want CONFIG_IRES_WEB_ADVANCED_SUPPORT)
endif
endif
endif

# 根据ECOS_WORK_DIR目录下.config的配置，把config-options下对应的配置配置到内核配置文件$(MODEL_NAME).ecc
# 最后ecosconfig --config=../$(MODEL_NAME).ecc --prefix=$(ECOS_WORK_DIR)/$(MODEL_NAME)_install tree;生成编译树
# 把生成内核配置文件写这儿看着很冗长，可以参考bcm, 直接拷贝配置好的ecc文件，省去这一步
configuration:
	@if [ ! -d ../gnutools/rsdk-1.5.7/linux/newlib ];then \
		cd ../gnutools/;\
		tar -xvjf rsdk-1.5.7p1-linux-newlib.tar.bz2;\
		cd ../ecos-work;\
	fi;
	@if [ ! -d ../gnutools/msdk-4.9.4/linux/newlib ];then \
		cd ../gnutools/;\
		tar -xvjf msdk-4.9.4-linux-newlib.tar.bz2;\
		cd ../ecos-work;\
	fi;
	rm -rf $(ECOS_WORK_DIR)/$(MODEL_NAME).ecc
	rm -rf $(ECOS_WORK_DIR)/$(MODEL_NAME)_install
	rm -rf $(ECOS_WORK_DIR)/$(MODEL_NAME)_build
	mkdir -p $(ECOS_WORK_DIR)/$(MODEL_NAME)_build
ifeq ($(CONFIG_NET_STACK_OPENBSD),y)
	ecosconfig --config=$(MODEL_NAME).ecc new $(TARGET_NAME) old_net
else
	ecosconfig --config=$(MODEL_NAME).ecc new $(TARGET_NAME) net
endif
	ecosconfig --config=$(MODEL_NAME).ecc version $(WLAN_DRIVER_VERSION) RTLPKG_DEVS_ETH_RLTK_819X_WLAN
ifeq ($(CONFIG_NOFLASH),y)
	ecosconfig --config=$(MODEL_NAME).ecc remove CYGPKG_IO_FLASH
	ecosconfig --config=$(MODEL_NAME).ecc remove CYGPKG_DEVS_FLASH_SPI_RTL819X
endif
	ecosconfig --config=$(MODEL_NAME).ecc remove CYGPKG_NS_DNS
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_FS_RAM

ifeq ($(PPP_SUPPORT),y) 
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_PPP
ifeq ($(CONFIG_PPP_RESERVED_CLUSTER),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ppp_module_reserved_cluster
endif
endif

ifeq ($(CONFIG_USB),y)
	ecosconfig --config=$(MODEL_NAME).ecc add RTLPKG_IO_USB_HOST
ifeq ($(CONFIG_USB_OHCI_HCD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/usb/controller/usb_ohci_hcd_enable
endif
ifeq ($(CONFIG_USB_EHCI_HCD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/usb/controller/usb_ehci_hcd_enable
endif
ifeq ($(CONFIG_USB_OTG_HCD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/usb/controller/usb_otg_hcd_enable
endif
endif

ifeq ($(CONFIG_PPPOE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/pppoe
endif

ifeq ($(PPP_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ppp/cflags_add_chapms
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ppp/ppp_pppd_priority

ifeq ($(CONFIG_DNSPROXY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ppp/dnsproxy_enable
endif

endif

ifeq ($(CONFIG_L2TP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/l2tp
endif

ifeq ($(CONFIG_PPTP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/pptp
endif

ifeq ($(CONFIG_DOUBLE_NAPT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/double_alias
endif

ifeq ($(CONFIG_RTL_NFJROM_MP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/nfjrom_mp_on
endif

ifeq ($(CONFIG_DNS),y)
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_NS_DNS
ifeq ($(CONFIG_MIPS16),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ns/dns/cflags_add_mips16
endif
ifeq ($(CONFIG_RTL_DNS_TRAP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ns/dns/dnstrap_enable
endif
endif
#	Add Athttpd
#	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_ATHTTPD
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_CRC
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_COMPRESS_ZLIB
#	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_IO_FLASH
ifeq ($(CONFIG_JFFS2_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_LINUX_COMPAT
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_FS_JFFS2
endif
#	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_IO_FILEIO_INODE
	ecosconfig --config=$(MODEL_NAME).ecc add CYGPKG_CPULOAD
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/services/compress/zlib/deflate_makes_gzip_off
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/donot_use_rom_monitor
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/donot_route_diag2debugchannel
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/rtc_div_factor_1000
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/use_interrupt_stack
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/interrupt_stack_size_32K
ifeq ($(CONFIG_RTL_CPU_POWER_SAVING),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/cpu_sleep_on
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/cute_mahjong_on
endif
ifeq ($(CONFIG_RTL_8881AM),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/rtl_8881AM_on
endif
ifeq ($(CONFIG_RAM_SIZE_4M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/ram_memory_size_4M
endif
ifeq ($(CONFIG_RAM_SIZE_8M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/ram_memory_size_8M
endif
ifeq ($(CONFIG_RAM_SIZE_16M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/ram_memory_size_16M
endif
ifeq ($(CONFIG_RAM_SIZE_32M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/ram_memory_size_32M
endif
ifeq ($(CONFIG_RAM_SIZE_64M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/ram_memory_size_64M
endif
ifeq ($(CONFIG_RTL_8196CS),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8196cs_support_on
endif
ifeq ($(CONFIG_RTL_8196D),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8196d_support
endif
ifeq ($(CONFIG_RTL_8197D),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8197d_support
endif
ifeq ($(CONFIG_RTL_8197DL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8197dl_support
endif
ifeq ($(CONFIG_RTL_8197F),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/msdk-elf-prefix
endif
ifeq ($(CONFIG_RTL_8197FS),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8197fs_support
endif
ifeq ($(CONFIG_RTL_8197FN),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8197fn_support
endif
ifeq ($(CONFIG_RTL_8197FB),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/8197fb_support
endif
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/dwalloc_on
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/no_imem_support
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/no_dmem_support
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/romeperf_on
#ifeq ($(CONFIG_RTL_NFJROM_MP),y)
	#ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/nfjrom_mp_on
#endif
ifeq ($(CONFIG_RTL_TIMER_ADJUSTMENT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/timer_adjust_on
endif

ifeq ($(CONFIG_RTL_HZ_1000),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/hz_1000
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/rtc_denominator_factor_1000
endif

ifeq ($(CONFIG_DBG_STAT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/dbg_stat_on
endif
ifeq ($(CONFIG_PRINTK),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_printk
endif
ifeq ($(WLAN_DRIVER_VERSION),v3_3)
ifeq ($(CONFIG_RTL_8881A),y)
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_reg_param_v700
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_reg_param_v702b
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_reg_param_v702bsw
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_no_embedded_wifi
endif
ifeq ($(CONFIG_RTL_8197F),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/rtl8197f_support
endif
ifeq ($(CONFIG_SLOT_0_NONE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_none
endif
ifeq ($(CONFIG_SLOT_1_NONE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie1_none
endif
ifeq ($(CONFIG_SLOT_0_8194AE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_8194
endif
ifeq ($(CONFIG_SLOT_0_8822BE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_8822be
endif
ifeq ($(CONFIG_SLOT_0_92C),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_8192c
endif
ifeq ($(CONFIG_SLOT_1_92C),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie1_8192c
endif
ifeq ($(CONFIG_SLOT_0_88E),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_8188e
endif
ifeq ($(CONFIG_SLOT_1_88E),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie1_8188e
endif
ifeq ($(CONFIG_SLOT_0_8192EE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_8192e
endif
ifeq ($(CONFIG_SLOT_1_8192EE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie1_8192e
endif
ifeq ($(CONFIG_SLOT_0_8812),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie0_8812
endif
ifeq ($(CONFIG_SLOT_1_8812),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie1_8812
endif
else
ifeq ($(CONFIG_RTL_88E_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/rtl8188e_support
endif
ifeq ($(CONFIG_RTL_92E_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/rtl8192e_support
endif
ifeq ($(CONFIG_RTL_92C_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/rtl8192c_support
endif
endif
ifeq ($(CONFIG_RLX_PROFILING),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/hal/rlx_profiling_on
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/error/per_thread_errno_off
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/use_mips16_off
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/eth/eth_drv_cflags_add_pg
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/wrapper_cflags_add_pg
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/switch_cflags_add_pg
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/wlan_cflags_add_pg
endif
ifeq ($(CONFIG_NOETH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_eth0
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_eth1
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/brsc_off
endif

ifeq ($(CONFIG_DELAY_REFILL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/delay_refill_on
endif

ifeq ($(CONFIG_RTL_EXCHANGE_PORTMASK),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/exchange_portmask
endif

ifeq ($(CONFIG_RTL_8367R),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_8367r
endif

ifeq ($(CONFIG_RTL_8211E_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_8211E
endif

ifeq ($(CONFIG_RTL_8211F_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_8211F
endif

ifeq ($(CONFIG_RTL_PHY_POWER_CTRL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_phypower_control
endif

ifeq ($(CONFIG_RTL_IGMP_SNOOPING),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/igmpsnoop_enable
ifeq ($(CONFIG_RTL_MLD_SNOOPING),y)	
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/mldsnoop_enable
endif
ifeq ($(CONFIG_RTL_HARDWARE_MULTICAST),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/hwmcast_enable
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/hw_common_enable
else
ifeq ($(CONFIG_HW_NAT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/hw_common_enable
endif
endif
	
endif
ifeq ($(CONFIG_RTL_CUSTOM_PASSTHRU),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/passthru_peth0
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/passthru_pwlan0
endif

ifeq ($(CONFIG_RTL_11W_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/11w_support_on
endif

ifeq ($(CONFIG_TENDA_WLAN_STEER),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/tenda_wlan_steer_on
endif

ifeq ($(CONFIG_TENDA_WLAN_DBG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/tenda_wlan_debug_on
endif

ifeq ($(CONFIG_IWCONTROL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/iwcontrol_support_on
endif
ifeq ($(CONFIG_RTL_HS2_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/hs2_support_on
endif

ifeq ($(CONFIG_NOWIFI),y)
ifeq ($(WLAN_DRIVER_VERSION),v3_3)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/nowifi
ifeq ($(CONFIG_RTL_8881A),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_no_embedded_wifi
endif
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_wlan0
endif
endif
ifeq ($(CONFIG_PCIE_POWER_SAVING),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/pcie_power_saving_on
endif
ifeq ($(CONFIG_WPS),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/wps_support_on
endif
ifeq ($(CONFIG_PACP_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/monitor_mode_on
endif
ifeq ($(CONFIG_RTL_DFS_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/dfs_support_on
endif
ifeq ($(CONFIG_WLAN_PROFILE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/wlan_profile_support_on
endif
ifeq ($(CONFIG_RTL_CLIENT_MODE_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/client_mode_support_on
ifeq ($(CONFIG_RTL_REPEATER_MODE_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/repeater_mode_support_on
endif
ifeq ($(CONFIG_RTL_802_1X_CLIENT_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8021x_client_support_on
endif
ifeq ($(CONFIG_RTL_SUPPORT_MULTI_PROFILE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/multi_profile_on
endif
ifeq ($(CONFIG_RTL_MULTI_CLONE_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/multi_macclone_on
endif
endif

ifeq ($(CONFIG_RTL_AC2G_256QAM),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/ac2g_256qam_support_on
endif
ifeq ($(CONFIG_RTL_WDS_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/wds_support_on
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/wds_num_8
endif
ifeq ($(CONFIG_RTL_MESH_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/mesh_support_on
endif
ifeq ($(CONFIG_RTL_VAP_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/vap_support_on
ifeq ($(CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/mbssid_num_4
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/multi_repeater_mode_support_on
endif
endif
ifeq ($(CONFIG_DISABLE_PHYDM_DEBUG_FUNCTION),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_phydm_debug_support
endif
ifeq ($(CONFIG_RTL_SIMPLE_CONFIG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/simple_config_on
endif
ifeq ($(CONFIG_RTL_SIMPLE_CONFIG_USE_WPS_BUTTON),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/simple_config_use_wps_button_on
endif

#Slot 0 setting
ifeq ($(CONFIG_SLOT_0_EXT_PA),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_ext_highpwr_pa
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot0_ext_highpwr_pa
endif

ifeq ($(CONFIG_SLOT_0_RFE_TYPE_0),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_0
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_1),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_1
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_2),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_2
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_3),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_3
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_4),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_4
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_5),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_5
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_6),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_6
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_7),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_7
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_8),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_8
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_9),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_9
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_10),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_10
endif
ifeq ($(CONFIG_SLOT_0_RFE_TYPE_13),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_rfe_type_13
endif
ifeq ($(CONFIG_SLOT_0_TX_BEAMFORMING),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_tx_beamforming
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot0_tx_beamforming
endif
ifeq ($(CONFIG_SLOT_0_ANT_SWITCH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_ant_switch
#@G bind to slot 0 as linux Kconfig said
ifeq ($(CONFIG_NO_2G_DIVERSITY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/slot0_ant_switch_type_no
endif
ifeq ($(CONFIG_2G_CGCS_RX_DIVERSITY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/slot0_ant_switch_type_rx
endif
ifeq ($(CONFIG_2G_CG_TRX_DIVERSITY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/slot0_ant_switch_type_trx
endif
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot0_ant_switch
endif
ifeq ($(CONFIG_SLOT_0_EXT_LNA),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_ext_highpwr_lna
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot0_ext_highpwr_lna
endif
ifeq ($(CONFIG_SLOT_0_PA_RTC5634),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot0_pa_rtc5634
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot0_pa_rtc5634
endif
#Slot 1 setting
ifeq ($(CONFIG_SLOT_1_EXT_PA),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot1_ext_highpwr_pa
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot1_ext_highpwr_pa
endif
ifeq ($(CONFIG_SLOT_1_TX_BEAMFORMING),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot1_tx_beamforming
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot1_tx_beamforming
endif
ifeq ($(CONFIG_SLOT_1_ANT_SWITCH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot1_ant_switch
#5G bind to slot 1 as linux Kconfig said
ifeq ($(CONFIG_NO_5G_DIVERSITY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/slot1_ant_switch_type_no
endif
ifeq ($(CONFIG_5G_CGCS_RX_DIVERSITY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/slot1_ant_switch_type_rx
endif
ifeq ($(CONFIG_5G_CG_TRX_DIVERSITY),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/slot1_ant_switch_type_trx
endif
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot1_ant_switch
endif
ifeq ($(CONFIG_SLOT_1_EXT_LNA),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot1_ext_highpwr_lna
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot1_ext_highpwr_lna
endif
ifeq ($(CONFIG_SLOT_1_PA_RTC5634),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_slot1_pa_rtc5634
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_slot1_pa_rtc5634
endif

#8197F setting 
ifeq ($(CONFIG_SOC_RFE_TYPE_0),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type_int
endif
ifeq ($(CONFIG_SOC_RFE_TYPE_1),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type_ext
endif
ifeq ($(CONFIG_SOC_RFE_TYPE_2),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type2
endif
ifeq ($(CONFIG_SOC_RFE_TYPE_3),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type3
endif
ifeq ($(CONFIG_SOC_RFE_TYPE_4),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type4
endif
ifeq ($(CONFIG_SOC_RFE_TYPE_5),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type5
endif
ifeq ($(CONFIG_SOC_RFE_TYPE_6),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8197f_rfe_type6
endif
#endif 8197F setting

#8881A setting 
ifeq ($(CONFIG_RTL_8881A_SELECTIVE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_8881a_selective
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_8881a_selective
endif

ifeq ($(CONFIG_8881A_EXT_PA),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_pa_type_ext
endif

ifeq ($(CONFIG_8881A_INT_PA),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_pa_type_int
endif
ifeq ($(CONFIG_8881A_INT_PA_RTC5634),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_pa_type_int_5634
endif

ifeq ($(CONFIG_RTL_8881A_ANT_SWITCH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_8881a_ant_switch
ifeq ($(CONFIG_NO_5G_DIVERSITY_8881A),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_ant_switch_type_no
endif
ifeq ($(CONFIG_5G_CGCS_RX_DIVERSITY_8881A),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_ant_switch_type_rx
endif
ifeq ($(CONFIG_5G_CG_TRX_DIVERSITY_8881A),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8881a_ant_switch_type_trx
endif
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/disable_8881a_ant_switch
endif

#8812 PA type
ifeq ($(CONFIG_PA_SKYWORKS_5022),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8812_ext_pa_type_5022
endif
ifeq ($(CONFIG_PA_RFMD_4501),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8812_ext_pa_type_4501
endif
ifeq ($(CONFIG_PA_SKYWORKS_5023),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/8812_ext_pa_type_5023
endif


#ifeq ($(CONFIG_HIGH_POWER_EXT_PA),y)
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_ext_highpwr_pa
#endif
#ifeq ($(CONFIG_ANT_SWITCH),y)
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_antenna_diversity
#endif
ifeq ($(CONFIG_SYSLOG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/infra/rtk_syslog_on
endif
ifeq ($(CONFIG_REMOTELOG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/infra/rtk_remotelog_on
endif
ifeq ($(CONFIG_DEBUG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/infra/asserts_and_tracing_on
ifeq ($(CONFIG_DEBUG_THREAD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/kernel/checking_threads_stack
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/kernel/checking_all_threads_stack
endif
endif
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/kernel/measure_stack_usage_on
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/kernel/dbg_gdb_thread_support_off
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/debug_on
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/init_eth0_manually
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/init_eth1_manually
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/tftp_support_off
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/dhcpc_support_off
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_1M660K
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_1M896K
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_2M
ifneq ($(CONFIG_NET_MBUF_SHRINK),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_3M928K
endif
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_4M304K
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_4M104K
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_4M4K
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_6M
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_12M
ifeq ($(CONFIG_NET_STACK_OPENBSD),y)
ifeq ($(CONFIG_BRIDGE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/bridge_enable
ifeq ($(CONFIG_NOWIFI),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/brsc_off
endif
ifeq ($(CONFIG_NOETH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/brsc_off
else
ifeq ($(CONFIG_ETH1_BRIDGED),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/eth1_bridged_on
endif
endif
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/brsc_off
endif
endif
ifeq ($(CONFIG_NET_STACK_FREEBSD),y)
ifeq ($(CONFIG_NET_MBUF_SHRINK),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/mbuf_shrink_enable
ifeq ($(CONFIG_NATD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf
else
ifeq ($(CONFIG_RAM_SIZE_4M),y)
else
ifeq ($(CONFIG_RAM_SIZE_8M),y)
#ifeq ($(CONFIG_IPV6_ENABLE),y)
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt_ipv6
#else
ifeq ($(CONFIG_RTL_8881A),y)
ifeq ($(CONFIG_CUTE_MAHJONG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt
else
#8881A Dual 8M
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt_8881a_8m
endif
else
ifeq ($(CONFIG_RTL_8196D),y)
ifeq ($(CONFIG_WLAN_HAL_8814AE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt_8194_8m
endif #CONFIG_WLAN_HAL_8814AE
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt
endif #CONFIG_RTL_8196D
endif #CONFIG_RTL_8881A_8M
#endif #CONFIG_IPV6_ENABLE
ifeq ($(CONFIG_ECOS_AP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_pure_ap
endif
else
ifeq ($(CONFIG_RAM_SIZE_16M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt_2
else
ifeq ($(CONFIG_RAM_SIZE_32M),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt_2
else
ifeq ($(CONFIG_RAM_SIZE_64M),y)
else
endif #64M
endif #32M
endif #16M
endif #8M
endif #4M
endif #CONFIG_NATD
else
ifeq ($(CONFIG_NATD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf
else
#ifeq ($(CONFIG_IPV6_ENABLE),y)
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt_ipv6
#else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/memusage_small_mbuf_kernel_napt
#endif #CONFIG_IPV6_ENABLE
endif
endif
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/net_thread_size_16k
ifeq ($(CONFIG_BRIDGE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/bridge_enable
endif
ifeq ($(CONFIG_SAME_LAN_MAC),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/same_mac_enable
endif
ifeq ($(CONFIG_FIREWALL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fw_enable
endif
ifeq ($(CONFIG_IPV6FIREWALL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/ipv6fw_enable
endif
ifeq ($(CONFIG_QOS),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/qos_enable
endif

ifeq ($(CONFIG_NBSERVER),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/netbios_enable
endif

ifeq ($(CONFIG_IGMPPROXY_KERNELMODE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/igmpproxy_kernelmode
endif

ifeq ($(CONFIG_IGMPPROXY_USERMODE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/igmpproxy_usermode
endif


ifeq ($(CONFIG_NAPT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/napt_enable
endif
ifeq ($(CONFIG_NATD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/natd_enable
endif

ifeq ($(CONFIG_ALG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/alg_enable
endif
ifeq ($(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/static_dhcp_ip_same_with_gw_support
endif
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/drv_deliver_in_inet_enable

ifeq ($(CONFIG_FASTPATH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fastpath_enable
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fastpath_skb_enable
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fastpath_pppoe_enable
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/pppoe_direct_reply
ifeq ($(CONFIG_PPPOE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fastpath_pppoe_enable
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/pppoe_direct_reply
endif

ifeq ($(CONFIG_L2TP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fastpath_l2tp_enable
endif

ifeq ($(CONFIG_PPTP),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/fastpath_pptp_enable
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ppp/ppp_mppc_mppe_support
endif

ifeq ($(CONFIG_BOOTP_COMPAT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/stack_accept_unicast
endif

endif

ifeq ($(CONFIG_HW_NAT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/hw_nat_enable
endif

ifeq ($(CONFIG_HW_NAT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/hw_common_enable
endif

ifeq ($(CONFIG_VLAN),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/vlan_enable
endif
ifeq ($(CONFIG_BRIDGE_VLAN),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/bridge_vlan_enable
endif

ifeq ($(CONFIG_NETSNIPER),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/netsniper_enable
endif

ifeq ($(CONFIG_KLD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/kld_enable
endif

ifeq ($(CONFIG_SPI_FIREWALL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/spi_firewall_enable
endif

ifeq ($(CONFIG_RTL_QOS_RATE_LIMIT_CHECK),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/qos_ratelimit_enable
endif

ifeq ($(CONFIG_RTL_NAT_LOOPBACK),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/nat_loopback
endif

endif
ifeq ($(CONFIG_MIPS16),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/isoinfra/cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/error/cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/time_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/stdlib_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/string_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/i18n_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/setjmp_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/startup_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libc/stdio_cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/language/c/libm/cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/services/memalloc/cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/services/compress/zlib/cflags_add_mips16
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/cflags_add_mips16
ifeq ($(CONFIG_NET_STACK_OPENBSD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/openbsd/cflags_add_mips16
endif
ifeq ($(CONFIG_USB),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/usb/cflags_add_mips16
endif
ifeq ($(CONFIG_NET_STACK_FREEBSD),y)
ifeq ($(CONFIG_IPV6_ENABLE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/cflags_add_mcast_mips16
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/cflags_add_mips16
endif
endif

ifeq ($(PPP_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/ppp/cflags_add_chapms_mips16
endif
endif
ifeq ($(CONFIG_IPV6_ENABLE),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/ipv6_enable
endif
ifeq ($(CONFIG_READYLOGO_CORE_PATCH),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/ipv6_readylogo_core_patch
endif
ifeq ($(CONFIG_FORWARD_IPV6_FRAG_SUPPORT),y)
	 ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/ipv6_forward_frag_support
endif
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/athttpd_root_dir
#	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/athttpd_debug_level
ifeq ($(CONFIG_RTL_8197DL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/fileio/max_open_file_descriptors_256	
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/fileio/max_open_file_descriptors_256	
endif
ifeq ($(CONFIG_MINIIGD),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/max_tcp_socket_limit
endif
ifeq ($(CONFIG_JFFS2_SUPPORT),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/fileio/support_io_fileio_inode
endif
ifeq ($(CONFIG_MEM_CHAIN_API),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/flash/support_io_flash_mem_chain_upgrade
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/net/freebsd/support_io_flash_mem_chain_upgrade
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/services/memalloc/support_io_flash_mem_chain_upgrade
endif

ifeq ($(CONFIG_RTL_8197DL),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/fileio/max_open_files_256
else
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/fileio/max_open_files_256
endif

ifeq ($(CONFIG_RTL_8196E),y)
ifeq ($(CONFIG_RTL_ULINKER),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_ulinker
endif
endif

ifeq ($(CONFIG_SOFT_WATCHDOG),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/watchdog/support_io_softdog
endif

ifeq ($(CONFIG_RTL_REPORT_LINK_STATUS),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/devs/eth/rltk/819x/enable_if_status
endif
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/serial/enable_hw_dev_driver
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/serial/use_tty0_as_console
# ifneq ($(CONFIG_RTL_8197F),y)
	ecosconfig --config=$(MODEL_NAME).ecc import $(ECOS_WORK_DIR)/config-options/io/flash/provide_dev_block_devices
# endif
	ecosconfig --config=$(MODEL_NAME).ecc resolve
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_build; \
	ecosconfig --config=../$(MODEL_NAME).ecc --prefix=$(ECOS_WORK_DIR)/$(MODEL_NAME)_install tree;

#是否是mips16,选择不同的汇编代码编译，fastpath_core.S,fast_l2tp_core.S,fast_pptp_core.S,fast_pppoe_core.S,filter.S,8192cd_profile.S
ecos:
ifeq ($(CONFIG_MIPS16),y)
ifeq ($(CONFIG_FASTPATH),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_on_$(prefix)_fastpath_core.S fastpath_core.S
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_on_$(prefix)_fast_pppoe_core.S fast_pppoe_core.S	
endif
ifeq ($(CONFIG_L2TP),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_on_$(prefix)_fast_l2tp_core.S fast_l2tp_core.S
endif
ifeq ($(CONFIG_PPPOE),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_on_$(prefix)_fast_pppoe_core.S fast_pppoe_core.S
endif
ifeq ($(CONFIG_FASTPATH),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_on_$(prefix)_filter.S filter.S
endif
ifeq ($(CONFIG_PPTP),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_on_$(prefix)_fast_pptp_core.S fast_pptp_core.S
endif
ifeq ($(CONFIG_RTL_SIMPLE_CONFIG),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/devs/eth/rltk/819x/wlan/v3_3/src/;cp mips16_on_$(prefix)_8192cd_profile.S 8192cd_profile.S
endif
else
ifeq ($(CONFIG_FASTPATH),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_off_$(prefix)_fastpath_core.S fastpath_core.S
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_off_$(prefix)_fast_pppoe_core.S fast_pppoe_core.S	
endif
ifeq ($(CONFIG_L2TP),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_off_$(prefix)_fast_l2tp_core.S fast_l2tp_core.S
endif
ifeq ($(CONFIG_PPPOE),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_off_$(prefix)_fast_pppoe_core.S fast_pppoe_core.S
endif
ifeq ($(CONFIG_FASTPATH),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_off_$(prefix)_filter.S filter.S
endif
ifeq ($(CONFIG_PPTP),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/net/bsd_tcpip/v3_0/src/sys/netinet/fastpath/;cp mips16_off_$(prefix)_fast_pptp_core.S fast_pptp_core.S
endif
ifeq ($(CONFIG_RTL_SIMPLE_CONFIG),y)
	cd $(ECOS_TOPDIR)/ecos-3.0/packages/devs/eth/rltk/819x/wlan/v3_3/src/;cp mips16_off_$(prefix)_8192cd_profile.S 8192cd_profile.S
endif
endif
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_build; make

ecos-clean:
	@if [ -d "$(ECOS_WORK_DIR)/$(MODEL_NAME)_build" ];then \
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_build && make clean; \
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/lib && rm -rf *; \
	mv $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/include/pkgconf $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/; \
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/include && rm -rf *;	\
	mv $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/pkgconf $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/include; \
	fi;



app:
	$(MAKE) -C AP
# app:
# ifeq ($(CONFIG_MIPS16),y)
	# make -C AP MIPS=mips16_on PREFIX=$(prefix)
# else
	# make -C AP MIPS=mips16_off PREFIX=$(prefix)
# endif


app-clean:
	make -C AP clean
	
flash:
	$(MAKE) -C ../userSpace/cbb/src/mkFlashImage

image:
	make -C AP rtkload
	./AP/tools/mgbin -c -o fw-$(MODEL_NAME).bin $(WEB_PAGES_BIN) ./AP/rtkload/ecos.bin
# llm add, 添加以前bcm的头，但是换掉MAGIC为RTK0
	chmod +x trx
# lq add，添加中英文版本互斥
#ifeq ($(CONFIG_WEB_VERSION),"cn")
#lrl add 不同产品添加魔术字互斥
ifndef CONFIG_FW_ID
	./trx -f 0x8000 -o bcmhead_upgrade_file.bin fw-$(MODEL_NAME).bin
else
	./trx -f $(CONFIG_FW_ID) -o bcmhead_upgrade_file.bin fw-$(MODEL_NAME).bin
endif
ifeq ($(CONFIG_TENDA_ATE_REALTEK),y)
	sed 's/HDR0/RTK0/' bcmhead_upgrade_file.bin > RTK0_MAGIC_ATE_TEST_upgradefile.bin
else
	sed 's/HDR0/RTK0/' bcmhead_upgrade_file.bin > RTK0_MAGIC_upgradefile.bin
endif
ifeq ($(CONFIG_RTL_NFJROM_MP),y)
	cp ./AP/rtkload/nfjrom ./$(MODEL_NAME)-nfjrom
	@if [ -d ../image ];then \
	rm ../image/*; \
	cp ./AP/rtkload/nfjrom ../image/$(MODEL_NAME)-nfjrom;\
	fi;
endif

#
# short-cut to build and clean one application
#
%:
	make -C $* all
%_build:
	cd AP && $(MAKE) $(@:_build=)

%-build:
	cd AP && $(MAKE) $(@:-build=)

%_clean:
	cd AP && $(MAKE) $(@:_clean=)-clean

%-clean:
	@dir=$*;	\
	if  echo $* | grep "/" ; then	\
		ENTER_DIR=$${dir%%/*};	\
		MAKE_DIR=$${dir#*/};	\
		cd $$ENTER_DIR && $(MAKE) $${MAKE_DIR}-clean; \
	else	\
		$(MAKE) -C $$dir clean;	\
	fi
	
%-only:
	@dir=$*;	\
	if  echo $* | grep "/" ; then	\
		ENTER_DIR=$${dir%%/*};	\
		MAKE_DIR=$${dir#*/};	\
		cd $$ENTER_DIR && $(MAKE) $${MAKE_DIR}-only; \
	else	\
		$(MAKE) -C $$dir;	\
	fi
	
	
clean:
	@if [ -d "$(ECOS_WORK_DIR)/$(MODEL_NAME)_build" ];then \
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_build && make clean; \
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/lib && rm -rf *; \
	mv $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/include/pkgconf $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/; \
	cd $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/include && rm -rf *; \
	mv $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/pkgconf $(ECOS_WORK_DIR)/$(MODEL_NAME)_install/include; \
	fi;
	make -C AP clean;
	@if [ -d "$(ECOS_WORK_DIR)/$(MODEL_NAME)_build" ];then \
	make -C AP rtkload-clean; \
	fi; 
# llm add
#	@rm -f $(ECOS_WORK_DIR)/*.ecc; 
	@rm -f $(ECOS_WORK_DIR)/*.bin;
	@rm -f $(ECOS_WORK_DIR)/*.dat;

#------------------------------------------------------------------------------
redboot:
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-romram.ecc
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-romram_install
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-romram_build
	mkdir -p $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-romram_build
	ecosconfig --config=$@-$(MODEL_NAME)-romram.ecc new $(TARGET_NAME) redboot
	ecosconfig --config=$@-$(MODEL_NAME)-romram.ecc import $(ECOS_REPOSITORY)/hal/mips/$(TARGET_NAME)/v3_0/misc/redboot_ROMRAM.ecm
	cd $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-romram_build; \
	ecosconfig --config=../$@-$(MODEL_NAME)-romram.ecc --prefix=$(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-romram_install tree; \
	make
	
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-ram.ecc
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-ram_install
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-ram_build
	mkdir -p $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-ram_build
	ecosconfig --config=$@-$(MODEL_NAME)-ram.ecc new $(TARGET_NAME) redboot
	ecosconfig --config=$@-$(MODEL_NAME)-ram.ecc import $(ECOS_REPOSITORY)/hal/mips/$(TARGET_NAME)/v3_0/misc/redboot_RAM.ecm
	cd $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-ram_build; \
	ecosconfig --config=../$@-$(MODEL_NAME)-ram.ecc --prefix=$(ECOS_WORK_DIR)/$@-$(MODEL_NAME)-ram_install tree; \
	make

#------------------------------------------------------------------------------
default:
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME).ecc
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	mkdir -p $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	ecosconfig --config=$@-$(MODEL_NAME).ecc new $(TARGET_NAME) default
	cd $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build; \
	ecosconfig --config=../$@-$(MODEL_NAME).ecc --prefix=$(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install tree; \
	make

openbsd:
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME).ecc
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	mkdir -p $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	ecosconfig --config=$@-$(MODEL_NAME).ecc new $(TARGET_NAME) old_net
	cd $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build; \
	ecosconfig --config=../$@-$(MODEL_NAME).ecc --prefix=$(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install tree; \
	make

freebsd:
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME).ecc
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	mkdir -p $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	ecosconfig --config=$@-$(MODEL_NAME).ecc new $(TARGET_NAME) net
	cd $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build; \
	ecosconfig --config=../$@-$(MODEL_NAME).ecc --prefix=$(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install tree; \
	make

lwip:
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME).ecc
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install
	rm -rf $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	mkdir -p $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build
	ecosconfig --config=$@-$(MODEL_NAME).ecc new $(TARGET_NAME) lwip_eth
	cd $(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_build; \
	ecosconfig --config=../$@-$(MODEL_NAME).ecc --prefix=$(ECOS_WORK_DIR)/$@-$(MODEL_NAME)_install tree; \
	make

include Kconfig.mk

.PHONY: config prepare configuration

trx:
	cd ../image && trx -o ecos.trx fw-$(MODEL_NAME).bin;
	./AP/tools/td_trx ../image/ecos.trx

        
