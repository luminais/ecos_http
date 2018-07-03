#
# Leaf Makefile
#
MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -DBCMDRIVER
EXTRA_CFLAGS +=-I../
#
# Enumerate files to compile
#
VPATH = $(SRCBASE)/shared/nvram
#
# Enumerate files to compile
#
VPATH += $(SRCBASE)/router/shared

ALL_OBJS = nvram.o nvram_rw.o
#tenda add
ALL_OBJS += envram_rw.o

#
# Leave rule to make
#
$(MODULE):  $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
