#
# router_shared目录Makefile
# 这里完成路由器默认配置文件编译
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TENDA_APP_DIR)/.config

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -DBCMDBG_ERR
EXTRA_CFLAGS += -I./
EXTRA_CFLAGS += -DBCM47XX -I$(shell cd .. ; pwd)/net_drive/include/

#
# Enumerate files to compile
#
# ALL_OBJS = wl.o wl_ecos.o wlif_utils.o shutils.o nvparse.o defaults.o
ALL_OBJS = shutils.o nvparse.o defaults.o wl.o wl_ecos.o

#
# Leave rule to make
#


init_cfg:
# ifneq ($(CONFIG_A9),y)
	# cp -f $(TENDA_APP_DIR)/config/cfg/$(CONFIG_Tenda_DEFAULT_PROFILE) $(TENDA_APP_DIR)/router_shared/defaults.c
# endif

$(MODULE): init_cfg $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

clean:
	find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f

	
.PHONY: all clean init_cfg

include $(TENDA_APP_DIR)/tenda_rules.mk