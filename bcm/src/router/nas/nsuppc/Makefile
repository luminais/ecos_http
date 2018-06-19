#
# 802.1x Supplicant Standalone x86/Linux Build Makefile
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id: Makefile 241388 2011-02-18 03:33:22Z stakita $
#
TARGET = nsuppc

all: $(TARGET)

SRCBASE	:= ../../..

CFLAGS	+= -I.. -I$(SRCBASE)/include -I$(SRCBASE)/router/shared -Wall
CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
CFLAGS  += -DNAS_WKSP_BUILD_CMD_LINE=0 -DNAS_WKSP_BUILD_NAS_SUPPL=1
CFLAGS  += -DNAS_WKSP_USE_FILE_SYSTEM=1 -DNAS_WKSP_ENCAP_EAPOL_PACKET=0

ifeq ($(CONFIG_BCMWPA2),y)
ADDED_CFLAGS += -DBCMWPA2
endif

CFLAGS	+= -DBCMSUPPL

NASSRC = $(SRCBASE)/router/nas
ENCSRC = $(SRCBASE)/bcmcrypto
EXTSRC = $(SRCBASE)/router/shared
UTLSRC = $(SRCBASE)/shared

# build target
NASOBJS = mppe.o wpa.o nas.o nas_linux.o nas_wl.o nas_wksp.o
ENCOBJS = aes.o aeskeywrap.o rijndael-alg-fst.o passhash.o prf.o md5.o hmac.o rc4.o sha1.o
EXTOBJS = wl.o wl_linux.o linux_timer.o shutils.o
UTLOBJS = bcmwpa.o bcmutils.o

.SUFFIXES : .o .c

$(TARGET): $(NASOBJS) $(ENCOBJS) $(EXTOBJS) $(UTLOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o :: $(NASSRC)/%.c $(TARGET).depend
	$(CC) -c $(CFLAGS) -o $@ $<

%.o :: $(ENCSRC)/%.c $(TARGET).depend
	$(CC) -c $(CFLAGS) -o $@ $<

%.o :: $(EXTSRC)/%.c $(TARGET).depend
	$(CC) -c $(CFLAGS) -o $@ $<

%.o :: $(UTLSRC)/%.c $(TARGET).depend
	$(CC) -c $(CFLAGS) -o $@ $<

# generate dependencies
NASSRCS = ${foreach file,${NASOBJS:.o=.c},$(NASSRC)/${file}}
ENCSRCS = ${foreach file,${ENCOBJS:.o=.c},$(ENCSRC)/${file}}
EXTSRCS = ${foreach file,${EXTOBJS:.o=.c},$(EXTSRC)/${file}}
UTLSRCS = ${foreach file,${UTLOBJS:.o=.c},$(UTLSRC)/${file}}

$(TARGET).depend : Makefile
	$(CC) -M $(CFLAGS) $(NASSRCS) $(ENCSRCS) $(UTLSRCS)> $@

-include $(TARGET).depend

# clean up mess
clean:
	rm -f $(NASOBJS)
	rm -f $(ENCOBJS)
	rm -f $(EXTOBJS)
	rm -f $(UTLOBJS)
	rm -f $(TARGET)
	rm -f $(TARGET).depend
