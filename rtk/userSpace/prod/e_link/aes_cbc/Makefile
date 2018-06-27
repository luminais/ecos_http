MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../../.. ; pwd)
endif

CURR_DIR=$(shell pwd)

all: $(MODULE)

include $(APP_DIR)/rules.mk
include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config
include $(ECOS_WORK_DIR)/.config

PATH:=$(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/mips-elf/bin:$(PATH)
export PATH

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS =
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/include
#EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/nvram
#EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk/
#EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared/
EXTRA_CFLAGS += -I$(CURR_DIR)/
EXTRA_CFLAGS += -I$(CURR_DIR)/../src
EXTRA_CFLAGS += -I$(CURR_DIR)/../timer



ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

# CFLAGS += $(EXTRA_CFLAGS) 
# CXXFLAGS += $(EXTRA_CFLAGS) 

#
# Enumerate files to compile
#
ALL_OBJS = aes_cbc.o aes_core.o test_aes_cbc.o


#
# Leave rule to make
# 

#$(MODULE): $(ALL_OBJS:.o=.d)
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
