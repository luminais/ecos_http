
include $(ECOS_WORK_DIR)/.config

#------------------------------------------------------------------------------

# User Management switch
UMSW	= -DUSER_MANAGEMENT_SUPPORT

# TLS client mode support
#TLS= -DTLS_CLIENT

# WLAN support
#-DWLAN_EASY_CONFIG
#WLANSUPP= -DWLAN_WPA -DWLAN_WDS -DWLAN_8185AG -DWLAN_WPA2 $(TLS)
WLANSUPP=
ifeq ($(CONFIG_RTL_WDS_SUPPORT),y)
WLANSUPP += -DCONFIG_WLAN_WDS_SUPPORT
endif

# The following two must be defined. Otherwise, there are some ASP Errors.
WLANSUPP += -DWIFI_SIMPLE_CONFIG -DUNIVERSAL_REPEATER

ifeq ($(CONFIG_RTL_CLIENT_MODE_SUPPORT),y)
WLANSUPP += -DCONFIG_WLAN_CLIENT_MODE
#ifeq ($(CONFIG_RTL_REPEATER_MODE_SUPPORT),y)
#WLANSUPP += -DUNIVERSAL_REPEATER
#endif
endif

# Digest Access switch
DASW	= -DDIGEST_ACCESS_SUPPORT


#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =

EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS  -D__NO_FCNTL=1 -I.. $(WLANSUPP) $(UMSW) $(DASW) $(SSLSW) $(IFMODSW) -DDOWN_SIZE

#EXTRA_CFLAGS += -DHOME_GATEWAY

EXTRA_CFLAGS += -I../system -I../network -I../apmib

ifeq ($(CONFIG_IPV6_ENABLE),y)
EXTRA_CFLAGS += -DCONFIG_IPV6
endif
ifeq ($(CONFIG_RTL_8881A),y)
EXTRA_CFLAGS += -DCONFIG_RTL_8812_SUPPORT
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_SELECTABLE
endif
ifeq ($(CONFIG_RTL_8197F),y)
EXTRA_CFLAGS += -D_LITTLE_ENDIAN_
else
EXTRA_CFLAGS += -D_BIG_ENDIAN_
endif

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

ifeq ($(CONFIG_MIPS16),y)
CFLAGS += -mips16
endif

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS for tools
#------------------------------------------------------------------------------
TOOLS_CFLAGS = $(COMMON_CFLAGS) 
TOOLS_CFLAGS += -DCONFIG_RTL_819X

ifeq ($(CONFIG_RTL_8196C),y)
TOOLS_CFLAGS += -DCONFIG_RTL_8196C
endif

ifeq ($(CONFIG_RTL_8196E),y)
TOOLS_CFLAGS += -DCONFIG_RTL_8196E
endif

ifeq ($(CONFIG_RTL_8196D),y)
TOOLS_CFLAGS += -DCONFIG_RTL_819XD -DCONFIG_RTL_8196D
endif

ifeq ($(CONFIG_RTL_8197DL),y)
TOOLS_CFLAGS += -DCONFIG_RTL_819XD
endif

ifeq ($(CONFIG_RTL_8197D),y)
TOOLS_CFLAGS += -DCONFIG_RTL_819XD
endif

ifeq ($(CONFIG_RTL_8881A),y)
TOOLS_CFLAGS += -DCONFIG_RTL_8881A
endif

ifeq ($(CONFIG_RTL_8197F),y)
TOOLS_CFLAGS += -DCONFIG_RTL_8197F
endif

TOOLS_CFLAGS += $(EXTRA_CFLAGS)

ifeq ($(CONFIG_JFFS2_SUPPORT),y)
TOOLS_CFLAGS += -DJFFS2_SUPPORT
JFFS2_TOOL=mkfs.jffs2
endif
#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

HOST_TOOLS = compweb mgbin $(JFFS2_TOOL) cvcfg-gw
CV_SIGNATURE=w6ca
JFFS2_TOOL_DIR=$(shell pwd)/mtd-utils-1.5.0

#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
DEPEND = ./.depend

SOURCES = $(OBJS:.o=.c)

all: $(HOST_TOOLS) Makefile
	echo "done"

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~ $(HOST_TOOLS) web_files webpages.bin

#------------------------------------------------------------------------------

compweb: compweb.c ../apmib/apmib.h
	gcc $(TOOLS_CFLAGS) -o compweb compweb.c

mgbin: mgbin.c ../apmib/apmib.h
	gcc $(TOOLS_CFLAGS) -o mgbin mgbin.c

mgbin-clean:
	rm -f mgbin.o mgbin

mkfs.jffs2:
	if [ ! -e mkfs.jffs2 ];then \
                make -C  $(JFFS2_TOOL_DIR);\
		cp $(JFFS2_TOOL_DIR)/$(JFFS2_TOOL) ./;\
        fi;

cvcfg-gw: cvcfg.c ../apmib/apmib.h
	gcc $(TOOLS_CFLAGS) -o cvcfg-gw cvcfg.c ../apmib/mibtbl.c	
##############################################################################

depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
