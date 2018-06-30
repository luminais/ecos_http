#
# Web Pages Makefile for eCos
#
# Copyright (C) 2010 Broadcom Corporation
#
# $Id: Makefile,v 1.8 2010-08-12 02:36:58 Exp $
#

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../.. ; pwd)
	VENDOR:= broadcom
	SRCBASE=$(shell cd ../../../.. ; pwd)
include ../.config
else
include $(TOPDIR)/.config
endif

#
# install mogrified files internally
#

#
# Rules
#
all: $(TOPDIR)/prod/www/$(VENDOR)/pages.mk install $(VENDOR).tar

$(TOPDIR)/prod/www/$(VENDOR)/pages.mk:
	$(MAKE) -C $(TOPDIR)/prod/www/$(VENDOR) pages.mk

# Retrieve $(PAGES)
include $(TOPDIR)/prod/www/$(VENDOR)/pages.mk

VPAGES = $(addprefix $(VENDOR)/, $(PAGES))

.INTERMEDIATE: $(PAGES)
.INTERMEDIATE: $(VPAGES)

#
# tar the pages
#
$(VENDOR).tar: $(PAGES)
	@tar cvf $@ $^

$(PAGES): $(VPAGES)

$(PAGES): $(foreach page,$(PAGES),$(VENDOR)/$(page))
ifeq ($(CONFIG_LIBZ),y)
	@gzip -c9 $(VENDOR)/$@ > $@
else
	@cp $(VENDOR)/$@ $@
endif

$(VPAGES): $(foreach page,$(VPAGES),$(SRCBASE)/router/www/$(page))
	@cp $(SRCBASE)/router/www/$@ ./$@

install: $(VPAGES)
	@echo "Install..."

#
# clean rules
#
clean: extra-clean
	$(MAKE) -C $(TOPDIR)/prod/www/$(VENDOR) clean

extra-clean:
	rm -rf $(VENDOR).tar
	rm -rf $(PAGES) $(VPAGES)
	rm -f $(VENDOR)/*.gz
