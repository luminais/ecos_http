#	@(#)Makefile	8.1 (Berkeley) 6/5/93
#	$FreeBSD$

#PROG=	route6d
#MAN8=	route6d.8

#CFLAGS+=	-Dss_len=__ss_len -Dss_family=__ss_family -DINET6 \
#	-DHAVE_GETIFADDRS

#.include <bsd.prog.mk>

include $(ECOS_WORK_DIR)/.config

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
SOURCES =  route6d.c
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
