#
# Router Wireless Interface Configuration Utility Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile 282322 2011-09-07 16:45:59Z palter $
#

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -Wall
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS	+= -L$(TOP)/nvram -lnvram -L$(TOP)/shared -lshared


OBJS := wlconf.o

all: wlconf

clean:
	rm -f *.o wlconf

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install wlconf $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlconf

wlconf: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
