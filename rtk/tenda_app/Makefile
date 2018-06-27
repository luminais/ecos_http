MODULE = built-in.o
include $(ECOS_WORK_DIR)/.config
all: $(MODULE)

include $(USER_DIR)/tenda_rules.mk
include $(USER_CONFIG)

# 添加编译工具链路径
PATH := $(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/mips-elf/bin:$(PATH)
export PATH

export ECOS_INSTALL_DIR=$(ECOS_WORK_DIR)/$(MODEL_NAME)_install

# 包含编译链的公共前缀变量ECOS_COMMAND_PREFIX，链接参数ECOS_GLOBAL_LDFLAGS，编译参数ECOS_GLOBAL_CFLAGS
include $(ECOS_INSTALL_DIR)/include/pkgconf/ecos.mak

# bintuils
CROSS_COMPILE = $(ECOS_COMMAND_PREFIX)

export CC = $(CROSS_COMPILE)gcc
export CXX = $(CROSS_COMPILE)gcc
export AR = $(CROSS_COMPILE)ar
export LD = $(CROSS_COMPILE)ld
export CPP = $(CROSS_COMPILE)gcc -E
# Separate C++ flags out from C flags.
ACTUAL_CFLAGS = $(ECOS_GLOBAL_CFLAGS)
ACTUAL_CFLAGS := $(subst -fno-rtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -frtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -Woverloaded-virtual,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -fvtable-gc,,$(ACTUAL_CFLAGS))

ACTUAL_CXXFLAGS = $(subst -Wstrict-prototypes,,$(ECOS_GLOBAL_CFLAGS))

export CFLAGS  = $(ACTUAL_CFLAGS) -D__ECOS
export CXXFLAGS = $(ACTUAL_CXXFLAGS) -D__ECOS

CFLAGS += -I$(ECOS_INSTALL_DIR)/include -include pkgconf/system.h
CXXFLAGS += -I$(ECOS_INSTALL_DIR)/include -include pkgconf/system.h

COMMON_CFLAGS = 
COMMON_CFLAGS += -DREALTEK

# 下面这行是根据REAKTEK ecos-work下的.config生成编译时的CONFIG_宏，但考虑到
# tenda app这边没使用REALTEK的配置宏，故注释掉，需要可自行加上
# COMMON_CFLAGS += $(shell cat $(ECOS_WORK_DIR)/.config | awk -F "=" '{if(match($$0, /^CONFIG_/)) {gsub(/^CONFIG_/, ""); if($$2=="y") printf "-D%s\n", $$1; else printf "-D%s=%s\n", $$1, $$2}}')

ifeq ($(CONFIG_MIPS16),y)
COMMON_CFLAGS += -mips16
endif

export TENDA_APP_DIR=$(shell pwd)

COMMON_CFLAGS += -include cyg/kernel/kapi.h

#链接tenda_app下面的FLAGS
TDAPP_LDFLAGS = 

CFLAGS += $(COMMON_CFLAGS)
CXXFLAGS += $(COMMON_CFLAGS)

obj-y = app
#when use costdown version use it
ifeq ($(CONFIG_APP_COSTDOWN),y)
obj-y += ucloud_costdown
export CONFIG_APP_COSTDOWN
else
obj-y += ucloud
endif

ifeq ($(CONFIG_GUEST),y)
export CONFIG_GUEST
endif
#用于控制APP是否隐藏信号强度功能的宏
ifeq ($(CONFIG_APP_SIGNAL_STRENGTH_HIDE),y)
export CONFIG_APP_SIGNAL_STRENGTH_HIDE
endif

SUBDIRS = $(obj-y)

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(LD) -r $(ENDIAN) $(TDAPP_LDFLAGS) -EL -o $@ $(SUB_OBJS)

clean:
	find .  -path "*ucloud*" -prune -o -type f -name '*.[od]' -print0 | xargs -0 rm -f 
.PHONY:	clean
