include ../../../../.config
SHELL = /bin/sh

CFLAGS          += $(GLOBAL_CFLAGS)
CXXFLAGS        += $(GLOBAL_CXXFLAGS)

srcdir = .
top_srcdir = .
cur_dir = $(shell pwd)

prefix = $(shell pwd)/..
exec_prefix = ${prefix}

CROSS_COMPILE ?= mips-elf-
###CROSS_COMPILE = ./../../../../gnutools/mips-elf/bin/mips-elf-
#CC = $(CROSS_COMPILE)gcc
#CXX = $(CROSS_COMPILE)g++
#LD = $(CROSS_COMPILE)gcc
#AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
#STRIP = $(CROSS_COMPILE)strip
#SSTRIP = $(CROSS_COMPILE)sstrip

LN_S = ln -s
PACKAGE = soapcpp2
install_sh = $(cur_dir)/install-sh
#platform = LINUX

SUBDIRS = soapcpp2 #libcwmp

subdir = .
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
RECURSIVE_TARGETS = all-recursive install-recursive

all:
	$(MAKE) all-recursive

$(RECURSIVE_TARGETS):
	@set fnord $$MAKEFLAGS; amf=$$2; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "==========="; \
	  echo "Making $$target in $$subdir"; \
	    local_target="$$target"; \
	  echo $$amf; \
	  echo "==========="; \
	  (cd $$subdir && $(MAKE) $$local_target) || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \

install: install-recursive

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: $(RECURSIVE_TARGETS) all all-am clean install

