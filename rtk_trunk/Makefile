############################################################################
#
# Realtek Semiconductor Corp.
#
# Makefile -- Top level dist makefile.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 12, 2008
#

#如果没有.config文件，那么规则合并后，将先执行make menuconfig，再make all
#ifneq (.config,$(wildcard .config))
# TARGET := menuconfig
ifeq ($(REALTEK_MP_ENABLE),y)
CONFIG_TENDA_ATE_REALTEK=y
export CONFIG_TENDA_ATE_REALTEK
endif

TOP_DIR = $(shell pwd)
CHECK_USRT_DIR = $(TOP_DIR)/userSpace/.config
USER_DIR = $(TOP_DIR)/userSpace
USER_CONFIG = $(USER_DIR)/.config
USER_CONFIG_D = $(USER_DIR)/.config.d
export USER_CONFIG
export USER_CONFIG_D
export TOP_DIR USER_DIR CONFIG_WEB_VERSION CONFIG_TENDA_APP CONFIG_FW_ID CONFIG_Tenda_DEFAULT_PROFILE
include $(TOP_DIR)/.config
include $(USER_CONFIG)
ifeq ($(CONFIG_BOARD_rtl8196d),y)
ECC_NAME = 96d-8194-gw
WIFI_DRIVER_VERSION = 819x_1_4_3
WIFI_CHIP = 8814A
else ifeq ($(CONFIG_MODEL),rtl8196e-8194)
ECC_NAME = 96e-8194-gw
WIFI_DRIVER_VERSION = 819x_1_4_3
WIFI_CHIP = 8814A
else ifeq ($(CONFIG_BOARD_rtl8197f),y)
ECC_NAME = 97f-8812br-16M-gw
WIFI_DRIVER_VERSION = 819x_1_5_1
WIFI_CHIP = 8197F
WIFI_CHIP_5G = 8822B
else
ECC_NAME = 96e-92e-gw
WIFI_DRIVER_VERSION = 819x_1_4_3
WIFI_CHIP = 8192E
endif

DIR_WIFI_DRIVER = $(TOP_DIR)/ecos-3.0/packages/devs/eth/rltk/$(WIFI_DRIVER_VERSION)/wlan/v3_3/src/rtl8192cd
# llm add,上传.config，那么根据是否有内核配置文件判断是否make config。首次编译将不出图形化界面
ifneq (ecos-work/$(ECC_NAME).ecc, $(wildcard ecos-work/$(ECC_NAME).ecc))
TARGET := config
#
# DIR_ROOT Used when .config not Exist
#
DIR_ROOT = $(shell pwd)
DIR_ECOS = $(DIR_ROOT)/ecos-work
export DIR_ROOT DIR_ECOS 
else


#
# variables
#
DIR_ROOT = $(shell pwd)
DIR_RSDK = $(DIR_ROOT)/$(CONFIG_RSDKDIR)

DIR_BOARD = $(DIR_ROOT)/target
DIR_IMAGE = $(DIR_ROOT)/target/image
DIR_ECOS = $(DIR_ROOT)/ecos-work

MAKE = PATH=$(DIR_RSDK)/bin:$(PATH); make

export DIR_ROOT DIR_RSDK DIR_IMAGE DIR_BOARD 
export DIR_ECOS 

ERROR :=
ifneq ($(CONFIG_RSDKDIR),$(wildcard $(CONFIG_RSDKDIR)))
ERROR += '$(CONFIG_RSDKDIR) does not exist'
endif

ifneq ($(CONFIG_ECOSDIR),$(wildcard $(CONFIG_ECOSDIR)))
ERROR += '$(CONFIG_ECOSDIR) does not exist'
endif

ifeq ($(ERROR),)
# for ramfs or MP build, rootfs must be ready before kernel
TARGET := all
else
TARGET := error
endif

endif


ECOSDB_DIR = $(TOP_DIR)/ecos-3.0/packages/ecos_db
ECOSDB_FILE = $(TOP_DIR)/ecos-3.0/packages/ecos.db

ifeq ($(CONFIG_BOARD_rtl8196d),y)
		ecos_db=$(ECOSDB_DIR)/8196d_ecos.db
else ifeq ($(CONFIG_BOARD_rtl8197f),y)
		ecos_db=$(ECOSDB_DIR)/8197f_ecos.db
else
		ecos_db=$(ECOSDB_DIR)/8196e_ecos.db
endif

exist_tsfile = $(shell if [ -f $(DIR_ROOT)/tsfile.mk ]; then echo "exist"; else echo "notexist"; fi;)

ifdef tsfile
TARGETS_FILE := $(tsfile)
else
	ifeq ($(exist_tsfile), exist)
include tsfile.mk
	else
	TARGETS_FILE := NULL
	endif
endif

all: ecos_db_detect $(USER_CONFIG_D) $(TARGET)

prepare:
	@if [ ! -e $(DIR_ROOT)/targets/$(TARGETS_FILE) ]; then \
		echo $(DIR_ROOT)/"targets/"$(TARGETS_FILE)"  file error!";  \
	else \
		echo "TARGETS_FILE=$(TARGETS_FILE)" > $(DIR_ROOT)/tsfile.mk ; \
		cp $(DIR_ROOT)/targets/$(TARGETS_FILE)/$(TARGETS_FILE)_main_chip.config $(DIR_ROOT)/.config -fr ; \
		cp $(DIR_ROOT)/targets/$(TARGETS_FILE)/$(TARGETS_FILE)_ecos_work.config $(DIR_ECOS)/.config -fr ; \
		cp $(DIR_ROOT)/targets/$(TARGETS_FILE)/$(TARGETS_FILE)_user.config $(USER_CONFIG) -fr; \
	fi

ecos_db_detect:
	cp $(DIR_ROOT)/targets/$(TARGETS_FILE)/radio/$(WIFI_CHIP)/* $(DIR_WIFI_DRIVER)/WlanHAL/Data/$(WIFI_CHIP) -fr;\
	cp $(DIR_ROOT)/targets/$(TARGETS_FILE)/radio/$(WIFI_CHIP_5G)/* $(DIR_WIFI_DRIVER)/WlanHAL/Data/$(WIFI_CHIP_5G) -fr;\
	cp $(ecos_db) $(ECOSDB_FILE)

error:
	@echo
	@echo "=== NOTICE ===" 
	@echo
	@for X in $(ERROR) ; do \
		echo ERROR: $$X; \
	done
	@echo
	@echo "Please run 'make config' to reconfigure"
	@echo

#
# 0. target selection
#
.PHONY: config menuconfig ui-sync oem

# 目标config和menuconfig的区别：
# menuconfig通过图形界面配置自动生成.config文件,而config的.config由用户自己手动修改

config: 
	@if [ ! -d gnutools/rsdk-1.5.7/linux/newlib ];then \
		cd gnutools/;\
		tar -xvjf rsdk-1.5.7p1-linux-newlib.tar.bz2;\
		cd ..;\
	fi;
	@if [ ! -d gnutools/msdk-4.9.4/linux/newlib ];then \
                cd gnutools/;\
                tar -xvjf msdk-4.9.4-linux-newlib.tar.bz2;\
                cd ..;\
        fi;
	@chmod u+x config/genconfig
	@chmod u+x config/setconfig
	@chmod u+x config/boardparse
	@config/genconfig > Kconfig
	#@config/mconf Kconfig
	#@config/setconfig defaults
	#@make menuconfig2
	#@config/setconfig final	
	@$(MAKE) -C $(DIR_ECOS) config


$(USER_CONFIG_D): $(USER_CONFIG)
	echo -e "Do nothing \nFor $(USER_CONFIG) dependencies,llm\n" > $(USER_CONFIG_D)
	chmod 777 $(USER_CONFIG_D)
	chmod a+x $(USER_DIR)/create_autoconf.sh
	$(USER_DIR)/create_autoconf.sh $(USER_CONFIG) > $(USER_DIR)/include/autoconf.h
ifeq ($(CHECK_USRT_DIR), $(wildcard $(CHECK_USRT_DIR)))
	cp $(USER_DIR)/include/autoconf.h $(USER_DIR)/router_shared/bcmconfig.h
	cp -f $(USER_DIR)/config/cfg/$(CONFIG_Tenda_DEFAULT_PROFILE) $(USER_DIR)/cbb/src/nvram/nvram_default.c
endif
	
$(USER_CONFIG):
	@echo $(USER_CONFIG) not found!
	error
	
menuconfig:
	@if [ ! -d gnutools/rsdk-1.5.7/linux/newlib ];then \
		cd gnutools/;\
		tar -xvjf rsdk-1.5.7p1-linux-newlib.tar.bz2;\
		cd ..;\
	fi;
	@if [ ! -d gnutools/msdk-4.9.4/linux/newlib ];then \
                cd gnutools/;\
                tar -xvjf msdk-4.9.4-linux-newlib.tar.bz2;\
                cd ..;\
        fi;
	@chmod u+x config/genconfig
	@chmod u+x config/setconfig
	@chmod u+x config/boardparse
	@config/genconfig > Kconfig
	@config/mconf Kconfig
	@config/setconfig defaults
	@make menuconfig2
	@$(MAKE) -C $(DIR_ECOS) config	

ui-sync:ui-sync-rm
	@echo $(USER_DIR)/prod/http/web/$(CONFIG_WEB_DIR)
	cp -r $(USER_DIR)/prod/http/web/$(CONFIG_WEB_DIR)_src $(USER_DIR)/prod/http/web/OEM_UI
	find $(USER_DIR)/prod/http/web/OEM_UI -name "*.svn" -type d | xargs rm -rf
	svn add $(USER_DIR)/prod/http/web/OEM_UI
	svn ci $(USER_DIR)/prod/http/web/OEM_UI -m "[OEM][make ui-sync][backups $(CONFIG_WEB_DIR)_src directory]"
	svn up
.IGNORE:ui-sync-rm
ui-sync-rm:
	svn rm $(USER_DIR)/prod/http/web/OEM_UI	
	svn ci $(USER_DIR)/prod/http/web/OEM_UI -m "[OEM][make ui-sync][rm OEM_UI directory]"
oem:
	$(MAKE) CONFIG_WEB_DIR="OEM_UI"
	
# force reload .config for DIR_LINUX, DIR_USERS, ...
menuconfig2:
	@if egrep "^CONFIG_MCONF_ECOS=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_ECOS) menuconfig; \
	fi
	@config/setconfig final

.PHONY:ecos_menuconfig
ecos_menuconfig:
	$(MAKE) -C $(DIR_ECOS) menuconfig

.PHONY:all
all:
	$(MAKE) -C $(DIR_ECOS) all
ifneq ($(CONFIG_TENDA_ATE_REALTEK),y)
#	objdump -S ./ecos-work/AP/appimg > ./Exception
endif

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
	
	
.PHONY:ecos
ecos:
	$(MAKE) -C $(DIR_ECOS) ecos

.PHONY:ecos-clean
ecos-clean:
	$(MAKE) -C $(DIR_ECOS) ecos-clean

.PHONY:app
app:
	$(MAKE) -C $(DIR_ECOS) app

.PHONY:app-clean
app-clean:
	$(MAKE) -C $(DIR_ECOS) app-clean

.PHONY:flash
flash:
	$(MAKE) -C $(DIR_ECOS) flash
	
.PHONY:image
image:
	$(MAKE) -C $(DIR_ECOS) image

.PHONY:mk
mk:
	make app-clean && make app && make image

.PHONY: clean
clean:
	$(MAKE) -C $(DIR_ECOS) clean
	rm -rf ./Exception

trx:
	$(MAKE) -C $(DIR_ECOS) trx

boot_menuconfig:
	cd bootcode_rtl8196d && make menuconfig
boot_clean:
	cd bootcode_rtl8196d && make clean
boot_build:boot_clean
	cd bootcode_rtl8196d && make
	cp bootcode_rtl8196d/boot/Output/boot.bin $(DIR_ECOS)
	cp bootcode_rtl8196d/boot/Output/boot $(DIR_ECOS)

boot_build_tiny:boot_clean
	cd bootcode_rtl8196d && make TENDA_BOOT_TINY=1
	cp bootcode_rtl8196d/boot/Output/boot.bin $(DIR_ECOS)
	cp bootcode_rtl8196d/boot/Output/boot $(DIR_ECOS)

    
    
