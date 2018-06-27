#
# ecos router bcmcrypto module for AP Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.1 2010-08-20 10:21:25 Exp $
#

include $(TOPDIR)/.config

# sys kernel config
include $(BSPDIR)/.config

CONFIG_WL_CONF := $(CONFIG_WL_AP)

# Include generic make file
include ../bcmcrypto_generic.mk


#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(OBJS)
endif
