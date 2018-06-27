
include $(ECOS_WORK_DIR)/.config

PREFIX?=$(notdir $(shell pwd))_
#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

ifeq ($(CONFIG_APMIB),y)
EXTRA_CFLAGS += -I../network -I../apmib
endif

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

SOURCES = net_api.c set_mac_address.c inet_net_pton.c icmpquery.c ifconfig.c iwlib.c iwpriv.c net_stat.c 
ifeq ($(CONFIG_PING6),y)
SOURCES += ping6.c 
endif

ifeq ($(CONFIG_IPV6_NDP),y) # ipv6 arp like command
SOURCES += ndp.c 
endif

ifeq ($(CONFIG_STATIC_ROUTE),y)
SOURCES += route.c linkaddr.c
EXTRA_CFLAGS += -DNEW_STRUCT_ROUTE
endif
ifeq ($(CONFIG_IPV6_ROUTE6),y)
SOURCES += route_ipv6.c 
endif

#ifeq ($(CONFIG_FIREWALL),y)
#SOURCES +=set_fw.c
#endif

ifeq ($(CONFIG_BRIDGE),y)
SOURCES += bridge.c
endif

#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))

all: depend $(OBJS) $(PREFIX_OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(PREFIX_OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(PREFIX)%.o: %.o
	@cp $^ $@

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) $(PREFIX_OBJS) *.o *~ 

depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
