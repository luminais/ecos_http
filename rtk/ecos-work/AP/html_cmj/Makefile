include $(ECOS_WORK_DIR)/.config

#------------------------------------------------------------------------------
# html files
#------------------------------------------------------------------------------
#Record which files need not include
#------------------------------------------------------------------------------
EXCLUDE_ASP=

COMPWEB =../tools/compweb
ifneq ($(CONFIG_JFFS2_SUPPORT),y)
ifneq ($(CONFIG_IRES_WEB_SUPPORT),y)
WEB_PAGES = webpages.bin
else
WEB_PAGES = webpages.ires
endif
else
WEB_PAGES = webpages.jffs2
GEN_JFFS2_IMG = webpages.jffs2
JFFS2_IMG_WEB_DIR=html.jffs2img
GEN_JFFS2_IMG_TOOL =../tools/mkfs.jffs2
endif
ifneq ($(CONFIG_JFFS2_SUPPORT),y)
ifeq ($(CONFIG_HOME_GATEWAY),y)
CV_SIGNATURE = w6cg
else
CV_SIGNATURE = w6ca
endif
else
CV_SIGNATURE = jw6c
endif
#------------------------------------------------------------------------------
all: $(WEB_PAGES) Makefile
	@echo "Webpages Generated"

clean:
	rm -f web_files web_files.tmp webpages.bin* webpages.jffs2 webpages.ires html.jffs2img.tar
	rm -rf html.jffs2img html.tmp
	find . -name "*.c" | xargs rm -f

#------------------------------------------------------------------------------

webpages.bin:
	rm -rf ./html.tmp web_files.tmp
	find . ! -path '*/.*' $(EXCLUDE_ASP) ! -name "." -name "*.*" > web_files
#	find . -type f $(EXCLUDE_FILES) -name "*.*" > web_files
	mkdir -p ./html.tmp
	cat web_files | xargs tar -c | tar -xC html.tmp/
	find html.tmp/ > web_files.tmp
	$(COMPWEB) signature web_files.tmp webpages.bin $(CV_SIGNATURE)

webpages.jffs2:
	-rm -rf $(JFFS2_IMG_WEB_DIR) $(JFFS2_IMG_WEB_DIR).tar
	find . ! -path '*/.*' $(EXCLUDE_ASP) ! -name "." -name "*.*" > web_files
	-mkdir $(JFFS2_IMG_WEB_DIR) 
	cat web_files | xargs tar -cf $(JFFS2_IMG_WEB_DIR).tar
	tar -xf $(JFFS2_IMG_WEB_DIR).tar -C $(JFFS2_IMG_WEB_DIR)
	$(GEN_JFFS2_IMG_TOOL) --pad=0x44000 -r $(JFFS2_IMG_WEB_DIR) -o $(GEN_JFFS2_IMG) -e 0x1000 -b
	$(COMPWEB) signature $(GEN_JFFS2_IMG) webpages.bin $(CV_SIGNATURE)

webpages.ires:
	find . ! -path '*/.*' $(EXCLUDE_ASP) -name "*.*" > web_files
	$(GENIRES)
	touch $(WEB_PAGES)
##############################################################################
.PHONY: clean all depend
