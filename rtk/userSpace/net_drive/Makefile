MODULE = obj-$(shell pwd | sed "s/.*\///" ).o
all: $(MODULE)
include $(ECOS_AP_DIR)/rules.mk
include $(TENDA_APP_DIR)/.config
#
# Enumerate subdirectories
#
NETDRV_DIR = $(shell pwd)
export NETDRV_DIR
CFLAGS +=-DCFG_GMAC -DBCMWPA2 -DBCM47XX -D__ECOS -DROUTER_TINY -D_KERNEL -D__INSIDE_NET -DBCMDMA32 -DCYGIMPL_TRACE_SPLX -DTENDA_KDEV_ADD -DCONFIG_PPP_NUM_SESSIONS=3 
CFLAGS += -include autoconf.h
SUBDIRS  =
SUBDIRS += kdev
SUBDIRS += ipdev
SUBDIRS += keapol
#-----------------------------------------------------------
#配置文件还未导入，使用的宏可能会与应用层有些冲突，所以现在现将所有模块编译进去，以方便测试调试
ifneq ($(CONFIG_A9),y)
SUBDIRS += ipfilter
endif
SUBDIRS += kdhcpc
SUBDIRS += kdhcpd
SUBDIRS += pppoe
SUBDIRS += pptp
SUBDIRS += l2tp
SUBDIRS += ppp
SUBDIRS += batchupgrade
#-----------------------------------------------------------
#obj-$(CONFIG_TENDA_NAT)			+= ipfilter/
#obj-$(CONFIG_DHCPC)				+= kdhcpc/
#obj-$(CONFIG_DHCPD) 				+= kdhcpd/
#obj-$(CONFIG_PPPOE) 				+= pppoe/
#obj-$(CONFIG_PPTP)					+= pptp/
#obj-$(CONFIG_L2TP) 				+= l2tp/
#obj-$(CONFIG_TENDA_PPP)			+= ppp/
#obj-$(CONFIG_BATCH_UPGRADE)		+= batchupgrade/

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#

ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): subdirs $(SUB_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $@ $(SUB_OBJS)
endif

# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
