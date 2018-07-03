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
EXTRA_CFLAGS += -I../apmib
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S
SOURCES = ip6fw.c 
#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
all:depend $(OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~

depend: $(SOURCES)
	if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
