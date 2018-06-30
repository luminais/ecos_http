#
# PPP module Makefile 
# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co.
#
# @ Id: Makefile,v 1.0 2016/01/11 @
#	@ author: zhuhuan @
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o


all: $(MODULE)


#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = .
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/include

#
# Enumerate files to compile
#
ALL_OBJS = md4.o

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk