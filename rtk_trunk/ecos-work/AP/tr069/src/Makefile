#
# Makefile for TR069 on ECOS
#
include ../../../.config
# Bin-utils
CROSS_COMPILE ?= mips-elf-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip
MAKE 	= PATH=$(TOOLCHAINPATH)/bin:$(PATH); make

# Directory
CWMP_ROOT = .
TARGET_DIR = lib
INCLUDE_DIR = include
LIBCWMP_DIR =libcwmp
ADAPTER_DIR =common_adapter_api
GSOAP_DIR = gsoaplib
MATRIXSSL_DIR = matrixssl-1-8-3-open
CWMPCLIENT_DIR = cwmpClient

# CFLAGS
EXTRA_CFLAGS = 
EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS  -D__NO_FCNTL=1 -I../../ $(SSLSW) $(IFMODSW) -DDOWN_SIZE
ifeq ($(CONFIG_TR069),y)
EXTRA_CFLAGS += -mips16
endif
CFLAGS	+= $(GLOBAL_CFLAGS)
CFLAGS 	+= $(EXTRA_CFLAGS) -I.
CXXFLAGS+= $(GLOBAL_CXXFLAGS)
CXXFLAGS+= $(EXTRA_CFLAGS) -I.

# TARGET OBJS
LIBCWMP_LIBRARY =
LIBGSOAP_LIBRARY =
ifeq ($(CONFIG_CWMP_WITH_NO_SSL),y)
LIBCWMP_LIBRARY = libcwmp.a
LIBGSOAP_LIBRARY = libgsoapck.a
endif
ifeq ($(CONFIG_CWMP_WITH_MATRIXSSL),y)
LIBCWMP_LIBRARY = libcwmpmssl.a
LIBGSOAP_LIBRARY = libgsoapmssl.a
LIBMATRIXSSL_LIBRARY = libmatrixsslstatic.a
endif
ifeq ($(CONFIG_CWMP_WITH_OPENSSL),y)
LIBCWMP_LIBRARY = libcwmpssl.a
LIBGSOAP_LIBRARY = libgsoapssl.a
endif
OBJS = 	./$(ADAPTER_DIR)/*.o \
		./$(CWMPCLIENT_DIR)/*.o \
		./$(TARGET_DIR)/*.o
ifeq ($(CONFIG_CWMP_WITH_MATRIXSSL),y)
OBJS += ./matrixssl-1-8-3-open/src/*.o \
		./matrixssl-1-8-3-open/src/os/*.o \
		./matrixssl-1-8-3-open/src/os/ecos/*.o \
		./matrixssl-1-8-3-open/src/crypto/peersec/*.o \
		./matrixssl-1-8-3-open/src/pki/*.o 
endif

# Clean target
CLEAN_OBJ_TARGET = $(wildcard $(TARGET_DIR)/*.o)

# Make
ifeq ($(CONFIG_CWMP_WITH_MATRIXSSL),y)
all:
	@echo "******Making matrix ssl******"
	if [ -d $(MATRIXSSL_DIR) ]; then \
		if [ ! -d lib ]; then mkdir lib; fi; \
		cd $(MATRIXSSL_DIR)/src; \
		$(MAKE); \
	fi
else
all:
endif
	@echo "******Making soap******"
	if [ -d $(GSOAP_DIR) ]; then \
		cd $(GSOAP_DIR); \
		CPPFLAGS +='-DCONFIG_APP_TR069 ' ./configure --prefix=$(CWMP_ROOT); \
		$(MAKE); \
		$(MAKE) install; \
	fi
	@echo "******Making libcwmp******"
	if [ -d $(LIBCWMP_DIR) ]; then \
		if [ ! -d $(TARGET_DIR) ]; then \
			mkdir $(TARGET_DIR);\
		fi; \
		cd $(LIBCWMP_DIR);\
		$(MAKE);\
		$(MAKE) install;\
		cp $(LIBCWMP_LIBRARY) ../$(TARGET_DIR)/;\
	fi
	@echo "******Making cwmpClient******"
	cd cwmpClient;\
	./bin2c.pl cacert_pem < cacert.pem > ./cacert_pem.c;\
	./bin2c.pl client_pem < client.pem > ./client_pem.c;\
	$(MAKE);
	@echo "*****Making common adapter api*****"
	cd $(ADAPTER_DIR);\
	$(MAKE);
	@echo "******Archieve libapp.a******"
	cd $(TARGET_DIR); \
	$(AR) -x $(LIBCWMP_LIBRARY) ;\
	$(AR) -x $(LIBGSOAP_LIBRARY);\
	cd ..;\
	$(AR) rcs $(APP_LIB) $(OBJS)

romfs:
	cp -f $(CWMP_ROOT)/cwmpClient/cwmpClient $(CWMP_ROOT)/../../romfs/bin
	cp -f $(CWMP_ROOT)/cwmpClient/DefaultCwmpNotify.txt $(CWMP_ROOT)/../../romfs/etc
	$(ROMFSINST) -s /var/cwmp_default /etc/cwmp_default
	$(ROMFSINST) -s /var/cwmp_config /etc/cwmp_config 

ifeq ($(CONFIG_CWMP_WITH_MATRIXSSL),y)
clean:
	if [ -d $(MATRIXSSL_DIR) ]; then \
		$(MAKE) -C $(MATRIXSSL_DIR)/src clean; \
		$(MAKE) -C $(MATRIXSSL_DIR)/examples clean; \
		test -z "$(TARGET_DIR)/$(LIBMATRIXSSL_LIBRARY)" || rm -f $(TARGET_DIR)/$(LIBMATRIXSSL_LIBRARY); \
	fi
else
clean:
endif
	test -z "$(CLEAN_OBJ_TARGET)" || rm -f $(CLEAN_OBJ_TARGET)
	if [ -d $(GSOAP_DIR) ]; then \
		test -z "$(TARGET_DIR)/$(LIBGSOAP_LIBRARY)" || rm -f $(TARGET_DIR)/$(LIBGSOAP_LIBRARY); \
		$(MAKE) -C $(GSOAP_DIR) clean; \
	fi
	if [ -d $(LIBCWMP_DIR) ]; then \
		test -z "$(TARGET_DIR)/$(LIBCWMP_LIBRARY)" || rm -f $(TARGET_DIR)/$(LIBCWMP_LIBRARY); \
		$(MAKE) -C $(LIBCWMP_DIR) clean; \
		if [ -d $(TARGET_DIR) ]; then \
			rm -rf $(TARGET_DIR); \
		fi;\
		if [ -d $(INCLUDE_DIR) ]; then \
			rm -rf $(INCLUDE_DIR); \
		fi; \
	fi 
	$(MAKE) -C $(CWMPCLIENT_DIR) clean	
	$(MAKE) -C $(ADAPTER_DIR) clean
