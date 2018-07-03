
include ../../.config

PREFIX?=$(notdir $(shell pwd))_
#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------

LOG_FACILITY = LOG_DAEMON
PATH_RADVD_CONF = /etc/radvd.conf
PATH_RADVD_LOG = /var/radvd.log
PATH_RADVD_PID = /var/radvd.pid
VERSION = 0.9.1
FREEBSD_CC_VER = 300000

LEX = flex
LEX_OUTPUT_ROOT = lex.yy
YACC = bison -y
YFLAGS = -d

LIBS = -lcrypt

DEFS = -DPATH_RADVD_CONF=\"$(PATH_RADVD_CONF)\" \
        -DPATH_RADVD_LOG=\"$(PATH_RADVD_LOG)\" -DLOG_FACILITY=$(LOG_FACILITY) \
        -DPATH_RADVD_PID=\"$(PATH_RADVD_PID)\" \
        -DVERSION=\"$(VERSION)\" -DINET6=1

COMMON_SRC = radvd_log.c socket.c recv.c radvd_util.c
radvdconf_SOURCES = radvdconf.c
SOURCES = $(COMMON_SRC) $(radvdconf_SOURCES) radvd.c timer.c send.c process.c radvd_interface.c \
        device.c device-common.c gram.c scanner.c

TWO_SPEC_SRC=gram.y scanner.l

radvdump_SOURCES = $(COMMON_SRC) radvdump.c


EXTRA_CFLAGS += -O2 -fno-strict-aliasing -pipe $(DEFS)

EXTRA_CFLAGS += -D__FreeBSD_cc_version=$(FREEBSD_CC_VER) -D_KERNEL

#------------------------------------------------------------------------------
DEPEND = ./.depend

.SUFFIXES: .o .c .cpp .cxx .s .S

OBJS = $(SOURCES:.c=.o)
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))
all: BUILT_TIME depend  $(OBJS) $(PREFIX_OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(PREFIX_OBJS)

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time
	
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $< $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(LIBS)

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(LIBS)

$(PREFIX)%.o: %.o
	@cp $^ $@

.l.c:
	$(LEX) $(AM_LFLAGS) $(LFLAGS) $< && mv $(LEX_OUTPUT_ROOT).c $@

.y.c:
	$(YACC) $(AM_YFLAGS) $(YFLAGS) $< && mv y.tab.c $*.c
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; else mv y.tab.h $*.h; fi; \
	else :; fi
gram.h: gram.c

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) $(PREFIX_OBJS) *~ built_time

depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
