# $FreeBSD$

#MAINTAINER	= ru@FreeBSD.org
#MAINTAINER     += ari@suutari.iki.fi
#PROG		= natd
#SRCS		= natd.c icmp.c
#CFLAGS	       += -Wall
#LDADD		= -lalias
#DPADD		= ${LIBALIAS}
#MAN		= natd.8

#.include <bsd.prog.mk>
include $(ECOS_WORK_DIR)/.config

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

PREFIX?=$(notdir $(shell pwd))_
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S
SOURCES = natd.c icmp.c 
#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))
all:depend $(OBJS) $(PREFIX_OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(PREFIX_OBJS)
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(PREFIX)%.o: %.o
	@cp $^ $@

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) $(PREFIX_OBJS) *.o *~

depend: $(SOURCES)
	if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
