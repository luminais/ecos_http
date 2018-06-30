
include $(ECOS_WORK_DIR)/.config

PREFIX=wsc_
PREFIX?=$(notdir $(shell pwd))_ 
#------------------------------------------------------------------------------
OPENSSLDIR	= ../portingssl
UPNPDIR		= ../../mini_upnp
IXMLINC		= ./
INCS = -I$(OPENSSLDIR)/crypto/include -I$(UPNPDIR) -I$(IXMLINC)

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG
endif

EXTRA_CFLAGS += -D__linux__ -DRSDK_BUILT -DB_ENDIAN $(INCS)
EXTRA_CFLAGS += -I../../system -I../../apmib -I../../common

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

CFLAGS += -DWPS2DOTX
ifeq ($(CONFIG_KLD),y)
CFLAGS += -DCONFIG_RTL865X_KLD
endif
ifeq ($(CONFIG_RTL_8197F),y)
CFLAGS += -DL_ENDIAN
endif

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

SOURCES = wsc.c txpkt.c rxpkt.c utils.c simplecfg_mini_upnp_main.c wscd_conf.c
#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))

all: BUILT_TIME depend $(OBJS) $(PREFIX_OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(PREFIX_OBJS)

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
	rm -f $(OBJS) $(PREFIX_OBJS) *.o *~ built_time
	rm wscd_conf.c
wscd_conf.c: CONF_FILE
depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi
CONF_FILE:
	./bin2c.pl wscd_conf < wscd.conf > ./wscd_conf.c
-include $(DEPEND)

.PHONY: clean all depend
