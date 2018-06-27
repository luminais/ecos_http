# 
# 腾达应用目录总Makefile
# llm, 2015-12-24 16:11:47
# 


TARGET = built-in.o

#-----------------------------------------------------------------------------
# REALTEK方案特有的--依赖上层Makefile变量、文件的地方
#-----------------------------------------------------------------------------

include $(ECOS_WORK_DIR)/.config


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
ifeq ($(CONFIG_TENDA_ATE_REALTEK),y)
COMMON_CFLAGS += -DCONFIG_TENDA_ATE_REALTEK
endif

ifeq ($(CONFIG_RTL_8197F),y)
ifeq ($(CONFIG_USE_PCIE_SLOT_0),y)
COMMON_CFLAGS += -DCONFIG_RTL_92D_SUPPORT
#COMMON_CFLAGS += -DFOR_DUAL_BAND
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D
endif
endif

# 下面这行是根据REAKTEK ecos-work下的.config生成编译时的CONFIG_宏，但考虑到
# tenda app这边没使用REALTEK的配置宏，故注释掉，需要可自行加上
# COMMON_CFLAGS += $(shell cat $(ECOS_WORK_DIR)/.config | awk -F "=" '{if(match($$0, /^CONFIG_/)) {gsub(/^CONFIG_/, ""); if($$2=="y") printf "-D%s\n", $$1; else printf "-D%s=%s\n", $$1, $$2}}')

ifeq ($(CONFIG_MIPS16),y)
COMMON_CFLAGS += -mips16
endif

#------------------------------------------------------------------------------
# tenda app需要的
#------------------------------------------------------------------------------
export TENDA_APP_DIR=$(shell pwd)

# bcm lib库的include和src/include都放在这个include里
COMMON_CFLAGS += -I$(shell pwd)/include -include autoconf.h
COMMON_CFLAGS += -I$(shell pwd)/system/common/include -include sys_config.h
COMMON_CFLAGS += -include cyg/kernel/kapi.h

#链接tenda_app下面的FLAGS
ifeq ($(CONFIG_RTL_8197F),y)
export TDAPP_LDFLAGS = -EL
else
TDAPP_LDFLAGS = 
endif
CFLAGS += $(COMMON_CFLAGS)
CFLAGS += -I$(TENDA_APP_DIR)/cbb/include -I$(TENDA_APP_DIR)/system/include -I$(TENDA_APP_DIR)/system/msg/include -I$(TENDA_APP_DIR)/system/rc/include
CFLAGS += -I$(TENDA_APP_DIR)/rc/ -I$(TENDA_APP_DIR)/shard/
CXXFLAGS += $(COMMON_CFLAGS)
DEBUG_CFLAGS = 
ifeq ($(CONFIG_MEMCHECK),y)
DEBUG_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/mem_check/
DEBUG_CFLAGS += -include $(TENDA_APP_DIR)/cbb/src/mem_check/tenda_mem.h
endif
export DEBUG_CFLAGS

#------------------------------------------------------------------------------
# obj-y 需要编译的目录
#------------------------------------------------------------------------------
include .config

obj-y =  
obj-y += system
obj-y += cbb
obj-y += prod
obj-y += net_drive 
obj-y += router_shared
#------------------------------------------------------------------------------


OBJ_DIRS = $(obj-y)
TRX_OBJS = $(join $(OBJ_DIRS), $(patsubst %, /obj-%.o, $(OBJ_DIRS)))


#------------------------------------------------------------------------------
all: $(obj-y) 
	$(LD) -r $(TDAPP_LDFLAGS) -o $(TARGET) $(TRX_OBJS)



#在没有.config的情况下生成.config, 根据.config生成include/autoconf.h文件
#.config.d作用：在修改.config后，make时同步修改autoconf.h
# .config.d: .config
	# echo -e "Do nothing \nFor .config dependencies,llm\n" > .config.d
	# chmod 777 .config.d
	# chmod +x create_autoconf.sh
	# ./create_autoconf.sh .config > include/autoconf.h
	# cp include/autoconf.h router_shared/bcmconfig.h
	
# .config:
	# if [ ! -f .config ]; then	\
		# cp config/F3_normal_en.config .config;	\
		# chmod 777 .config;	\
	# fi

clean:
	for i in ${obj-y} $(lib-y) $(app-y); do \
		make -C $$i clean; \
	done
	rm -f $(TARGET) .config.d


$(obj-y):
	[ -d "$@" ] && $(MAKE) -C $@

#
# Generic rules
#

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
	


.PHONY: all clean 
.PHONY: $(obj-y)

