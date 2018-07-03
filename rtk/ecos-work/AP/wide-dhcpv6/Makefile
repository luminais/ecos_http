# Copyright (c) 2004 WIDE Project. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

#
# $Id: Makefile,v 1.1 2008-02-02 09:23:31 cary_lee Exp $
# $KAME: Makefile.in,v 1.45 2005/10/16 16:25:38 suz Exp $
#
include ../../.config

PREFIX?=$(notdir $(shell pwd))_
#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif
EXTRA_CFLAGS += -I../common
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S






srcdir=	.

CFLAGS +=	-Os -I$(srcdir) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DYYTEXT_POINTER=1 -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GETIFADDRS=1 -DHAVE_IF_NAMETOINDEX=1 -DHAVE_DAEMON=1 -DHAVE_WARNX=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FCNTL_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_SYSLOG_H=1 -DHAVE_UNISTD_H=1 -DHAVE_IFADDRS_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_STRUCT_TM_TM_ZONE=1 -DHAVE_TM_ZONE=1 -DGETPGRP_VOID=1 -DSETPGRP_VOID=1 -DRETSIGTYPE=void -DHAVE_MKTIME=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_ANSI_FUNC=1 -DHAVE_STDARG_H=1  -DSYSCONFDIR=\"${sysconfdir}\" \
	-DLOCALDBDIR=\"${localdbdir}\"
LIBOBJS= strlcpy$U.o strlcat$U.o 
#LIBOBJS= strlcpy$U.o strlcat$U.o arc4random$U.o
#LIBS=	 -L ../flex-2.5.33/ -lfl 
#CC= 	rsdk-linux-gcc	
#TARGET=	dhcp6c dhcp6s dhcp6relay dhcp6ctl
TARGET=	dhcp6s 
DEPEND = ./.depend

GENSRCS=cfparse.c cftoken.c
SERVOBJS=	dhcp6s.o common.o if.o config.o timer.o lease.o \
	base64.o auth.o dhcp6_ctl.o $(GENSRCS:%.c=%.o)
CLEANFILES+=	y.tab.h
PREFIX_SERVOBJS=$(join $(dir $(SERVOBJS)),$(addprefix $(PREFIX),$(notdir $(SERVOBJS))))
PREFIX_LIBOBJS=$(join $(dir $(LIBOBJS)),$(addprefix $(PREFIX),$(notdir $(LIBOBJS))))
all:	$(TARGET)
dhcp6s:	depend $(SERVOBJS) $(PREFIX_SERVOBJS) $(LIBOBJS) $(PREFIX_LIBOBJS) Makefile
	$(AR) qc $(APP_LIB) $(PREFIX_SERVOBJS) $(PREFIX_LIBOBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(PREFIX)%.o: %.o
	@cp $^ $@


cfparse.c y.tab.h: cfparse.y
	bison -y -d cfparse.y
	mv y.tab.c cfparse.c

cftoken.c: cftoken.l y.tab.h
	flex cftoken.l
	mv lex.yy.c $@	
	cp cftoken_bake.c cftoken.c	

#cftoken.c:cftoken_bake.c 
#	mv cftoken_bake.c cftoken.c	


getaddrinfo.o:	$(srcdir)/missing/getaddrinfo.c
	$(CC) -c $(srcdir)/missing/$*.c
getnameinfo.o:	$(srcdir)/missing/getnameinfo.c
	$(CC) -c $(srcdir)/missing/$*.c
strlcat.o:	$(srcdir)/missing/strlcat.c
	$(CC) -c $(srcdir)/missing/$*.c
strlcpy.o:	$(srcdir)/missing/strlcpy.c
	$(CC) -c $(srcdir)/missing/$*.c
arc4random.o:	$(srcdir)/missing/arc4random.c
	$(CC) $(CFLAGS) -c $(srcdir)/missing/$*.c
getifaddrs.o:	$(srcdir)/missing/getifaddrs.c
	$(CC) -c $(srcdir)/missing/$*.c
daemon.o:	$(srcdir)/missing/daemon.c
	$(CC) -c $(srcdir)/missing/$*.c
warnx.o:	$(srcdir)/missing/warnx.c
	$(CC) -c $(srcdir)/missing/$*.c

$(srcdir)/ianaopts.h: gentab.pl bootp-dhcp-parameters
	expand bootp-dhcp-parameters | perl gentab.pl > ianaopts.h


includes::

clean::
	/bin/rm -f *.o $(TARGET) $(CLEANFILES)
	/bin/rm -rf _install

distclean:: clean
	/bin/rm -f Makefile config.cache config.log config.status .depend


package:
	tar -zcvf wide-dhcpv6.tar.gz $(srcdir)/*.[chyl1-8] $(srcdir)/Makefile.in \
		$(srcdir)/README $(srcdir)/COPYRIGHT $(srcdir)/CHANGES \
		$(srcdir)/configure $(srcdir)/configure.in \
		$(srcdir)/install-sh  $(srcdir)/*.sample \
		$(srcdir)/missing/arc4random.?  $(srcdir)/missing/strlcat.c \
		$(srcdir)/missing/strlcpy.c $(srcdir)/missing/daemon.c \
		$(srcdir)/missing/err.h $(srcdir)/missing/warnx.c \
		$(srcdir)/missing/ifaddrs.h $(srcdir)/missing/getifaddrs.c \
		$(srcdir)/missing/sys/queue.h

depend: $(SERVOBJS)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi


-include $(DEPEND)

.PHONY: clean all depend
