# Make `routed` for FreeBSD
# $FreeBSD$
# $FreeBSD$

#PROG=   routed
#SRCS=   if.c input.c main.c output.c parms.c radix.c rdisc.c table.c trace.c
#MAN=   routed.8
#SUBDIR= rtquery
#LDADD+= -lmd
#DPADD+= ${LIBMD}
#COPTS= -g -DDEBUG -W -Wall -Wcast-align -Wcast-qual -Winline -Wpointer-arith -Wnested-externs -Wwrite-strings -Wunused
#.include <bsd.prog.mk>

include $(ECOS_WORK_DIR)/.config

PREFIX ?= $(notdir $(shell pwd))_

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
.SUFFIXES: .o .c .cpp .cxx .s .S
SOURCES =  if.c input.c main.c parms.c output.c radix.c rdisc.c table.c trace.c
#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)

PREFIX_OBJS=$(join $(dir $(OBJS)),  $(addprefix $(PREFIX), $(notdir $(OBJS))) )

all:depend $(OBJS) Makefile $(PREFIX_OBJS)
#	$(AR) rcs $(APP_LIB) $(OBJS)
	$(AR) rcs $(APP_LIB) $(PREFIX_OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(PREFIX)%.o:%.o
	@cp $^ $@

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~

depend: $(SOURCES)
	if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
