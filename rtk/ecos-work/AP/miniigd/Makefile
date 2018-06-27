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

EXTRA_CFLAGS += -I../common -I../system
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

EVENTING_ENABLED=1
NATPMP_ENABLED=1

GW=1
ifeq ($(GW),1)
CFLAGS += -DCONFIG_RTL8186_GW
endif

ifeq ($(RTL8186_TR),1)
	CFLAGS		+= -DSUPPORT_HNAP
endif

ifeq ($(RTL8186_KB),1)
	CFLAGS          += -DSUPPORT_HNAP
	CFLAGS          += -DCONFIG_RTL8186_KB
endif

ifeq ($(RTL865X_GRC),1)
	CFLAGS          += -DSUPPORT_HNAP
	CFLAGS          += -DCONFIG_RTL8186_GR
endif
ifeq ($(EVENTING_ENABLED),1)
CFLAGS          += -DENABLE_EVENTS
endif

ifeq ($(NATPMP_ENABLED),1)
CFLAGS          += -DENABLE_NATPMP
endif

# For mini_upnp stand alone
CFLAGS		+= -DSTAND_ALONE

ifdef CONFIG_APP_MULTIPPPOE
        CFLAGS += -D MULTI_PPPOE
endif
#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

SOURCES = getifaddr.c miniigd.c minixml.c natpmp.c upnpdescgen.c upnpevents.c upnpglobalvars.c miniigdhttp.c upnpredirect.c upnpreplyparse.c miniigdsoap.c upnppermissions.c\
		picsdesc_xml.c picsdesc_skl.c
SOURCES += ipfw/getifstats.c ipfw/ipfwrdr.c

#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))

all:BUILT_TIME depend  $(OBJS) $(PREFIX_OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(COMMON_LIB) $(PREFIX_OBJS)

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time
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
	rm picsdesc_* built_time
picsdesc_xml.c picsdesc_skl.c:DESC_FILE
DESC_FILE:
ifeq ($(CONFIG_KLD),y)
	./bin2c.pl picsdesc_xml < picsdesc-kld.xml > ./picsdesc_xml.c
	./bin2c.pl picsdesc_skl < picsdesc-kld.skl > ./picsdesc_skl.c
else
	./bin2c.pl picsdesc_xml < picsdesc.xml > ./picsdesc_xml.c
	./bin2c.pl picsdesc_skl < picsdesc.skl > ./picsdesc_skl.c
endif
depend:$(SOURCES) 
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend


