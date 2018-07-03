# $FreeBSD$

#PROG=	ipfw
#MAN=	ipfw.8
#CFLAGS+=-Wall

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
SOURCES = ipfw.c heapsort.c
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
