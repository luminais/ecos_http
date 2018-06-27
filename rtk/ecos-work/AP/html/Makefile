include $(ECOS_WORK_DIR)/.config

GENIRES=./genires.pl
#------------------------------------------------------------------------------
# html files
#------------------------------------------------------------------------------
#Record which files need not include
EXCLUDE_ASP = ! -name genires.pl  ! -name FILE.mak ! -name *.c ! -name *.exe
EXCLUDE_ASP += ! -name snmp.htm #! -name ip_qos.htm ! -name route.htm ! -name util_qos.js ! -name syslog.htm

ifeq ($(CONFIG_ECOS_AP),y)
EXCLUDE_ASP += ! -name ddns.htm
EXCLUDE_ASP += ! -name snmp.htm
EXCLUDE_ASP += ! -name dmz.htm
EXCLUDE_ASP += ! -name macfilter.htm
#EXCLUDE_ASP += ! -name status.htm
EXCLUDE_ASP += ! -name dos.htm
#EXCLUDE_ASP += ! -name syscmd.htm
EXCLUDE_ASP += ! -name tcpipwan.htm
EXCLUDE_ASP += ! -name opmode.htm
EXCLUDE_ASP += ! -name ipfilter.htm
EXCLUDE_ASP += ! -name ip_qos.htm
EXCLUDE_ASP += ! -name portfilter.htm
EXCLUDE_ASP += ! -name urlfilter.htm
EXCLUDE_ASP += ! -name portfw.htm
EXCLUDE_ASP += ! -name radvd.htm
EXCLUDE_ASP += ! -name route.htm
EXCLUDE_ASP += ! -name vlan.htm
EXCLUDE_ASP += ! -name routetbl.htm
EXCLUDE_ASP += ! -name vlan_wan.htm	
endif
ifneq ($(CONFIG_RTL_WDS_SUPPORT),y)
EXCLUDE_ASP += ! -name wlwds.htm ! -name wlwdsenp.htm ! -name wlwdstbl.htm
endif
ifneq ($(CONFIG_RTL_MESH_SUPPORT),y)
EXCLUDE_ASP += ! -name wlmesh.htm ! -name wlmeshACL.htm ! -name wlmeshinfo.htm
endif
EXCLUDE_ASP += ! -name wlmeshproxy.htm
ifneq ($(CONFIG_WLAN_SCHEDULE),y)
EXCLUDE_ASP += ! -name wlan_schedule.htm
endif
ifneq ($(CONFIG_RTL_8881A_SELECTIVE_MODE_SUPPORT),y)
EXCLUDE_ASP += ! -name wlbandmode.htm
endif
ifneq ($(CONFIG_FIREWALL),y)
EXCLUDE_ASP += ! -name ipfilter.htm ! -name portfilter.htm ! -name macfilter.htm
EXCLUDE_ASP += ! -name portfw.htm ! -name dmz.htm ! -name urlfilter.htm
endif
ifneq ($(CONFIG_QOS),y)
EXCLUDE_ASP += ! -name ip_qos.htm
endif
ifneq ($(CONFIG_SYSLOG),y)
EXCLUDE_ASP += ! -name syslog.htm
endif
ifneq ($(CONFIG_MULTI_LANGUAGE),y)
EXCLUDE_ASP += ! -name language_sc.js
endif
ifneq ($(CONFIG_IPV6_ENABLE),y)
EXCLUDE_ASP += ! -name ipv6_basic.htm ! -name radvd.htm 
endif
ifneq ($(CONFIG_TR069),y)
EXCLUDE_ASP += ! -name share.js ! -name tr069config.htm
endif
ifneq ($(CONFIG_HTTP_FILE_SERVER_SUPPORT),y)
EXCLUDE_ASP += ! -name http_files.htm
EXCLUDE_ASP += ! -name usb_conninfo.htm
EXCLUDE_ASP += ! -name usb_fileinfo.htm
endif
#------------------------------------------------------------------------------
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
