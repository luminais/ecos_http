#
# ecos router Stream Control module Makefile
# 
# Copyright (C) 2010, Tenda Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.0 2010/13/56 Exp $
#	$author: stanley$
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)


include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config

EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include -I$(TENDA_APP_DIR)/cbb/src/tc -I$(TENDA_APP_DIR)/include -I$(TENDA_APP_DIR)/router_shared  -I./include -I. -I$(APP_DIR)/tenda_paltform/shared_common -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -include autoconf.h
EXTRA_CFLAGS += -I./crypto/include -I./crypto/include/openssl
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/../ecos-3.0/packages/services/gfx/mw/v3_0/include -I$(TENDA_APP_DIR)/../ecos-3.0/packages/io/common/v3_0/include

EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -D_KERNEL -DCYGIMPL_TRACE_SPLX -DBSD=200108 -DTENDA_KDEV_ADD -DRTL_819X

EXTRA_CFLAGS +=  -DOPENSSL_FIPS -D__linux__ -DRSDK_BUILT -DOPENSSL_NO_SPEED -DOPENSSL_THREADS -D_REENTRANT \
		-DDSO_DLFCN -DHAVE_DLFCN_H -DOPENSSL_NO_KRB5 -DB_ENDIAN -DTERMIO 

SRCPATH = ./crypto/ ./crypto/aes ./crypto/bn ./crypto/des ./crypto/dh ./crypto/evp ./crypto/hmac ./crypto/md4  ./crypto/rand ./crypto/rand ./crypto/sha
VPATH = $(SRCPATH)

obj-y	= mem.o mem_clr.o bn_add.o bn_asm.o bn_const.o bn_ctx.o \
		bn_div.o bn_exp.o bn_gcd.o bn_lib.o bn_mod.o bn_mont.o \
		bn_mul.o bn_prime.o bn_rand.o bn_recp.o bn_shift.o bn_sqr.o \
		bn_word.o dh_check.o dh_gen.o dh_key.o dh_lib.o digest.o \
		m_sha1.o hmac.o md_rand.o rand_lib.o sha1dgst.o sha256.o \
		aes_cbc.o fips_aes_core.o \
		md4_dgst.o md4_one.o \
		des_enc.o ecb_enc.o set_key.o des_old.o 		 

ALL_OBJS := $(obj-y)
		
#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif

clean:
	-find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f
	-rm -f $(MODULE)
	
.PHONY: clean