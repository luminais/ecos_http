
include ../../.config

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

#VER = 0.9.8
#EXTRA_CFLAGS += -DVERSION='"$(VER)"'

EXTRA_CFLAGS += -DCYGPKG_NET_DHCP

EXTRA_CFLAGS += -DCYGOPT_NET_DHCP_OPTION_DHCP_CLIENTID_MAC

ifeq ($(CONFIG_SUPPORT_FOR_RUSSIA_CUSTOMER), y)
EXTRA_CFLAGS += -DSUPPORT_STATIC_ROUTE_OPTION -DSUPPORT_FOR_RUSSIA_CUSTOMER
endif

ifeq ($(CONFIG_TR069),y)
ifneq ($(CONFIG_SUPPORT_FOR_RUSSIA_CUSTOMER), y)
EXTRA_CFLAGS += -DSUPPORT_STATIC_ROUTE_OPTION
endif
endif

ifeq ($(CONFIG_BOOTP_COMPAT), y)
EXTRA_CFLAGS += -DBOOTP_COMPAT
endif

#EXTRA_CFLAGS += -DSEND_ARP_TO_CHECK_IP_CONFLICT

EXTRA_CFLAGS += -DCHECK_RECV_INTERFACE_INDEX
EXTRA_CFLAGS += -DCHECK_SERVER_ALIVE
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

ifeq ($(CONFIG_NETSNIPER_WANTYPE),y)
SOURCES = dhcp_prot.c bootp_support.c ./dhcpplus/des_pc.c ./dhcpplus/dhcpcplus_main.c ./dhcpplus/dhcp_plus.c ./dhcpplus/dhcp_pluslib.c ./dhcpplus/md5c.c
else
SOURCES = dhcp_prot.c bootp_support.c 
endif

#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
all: BUILT_TIME depend $(OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(OBJS)

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time
	
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~ built_time

depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
