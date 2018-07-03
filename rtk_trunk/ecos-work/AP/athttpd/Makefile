include $(ECOS_WORK_DIR)/.config
include $(ECOS_AP_DIR)/config.mk

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_TR069),y)
EXTRA_CFLAGS += -DCONFIG_TR069
endif
ifeq ($(CONFIG_APMIB),y)
EXTRA_CFLAGS += -I../network -I../apmib
endif
ifeq ($(CONFIG_HTTPD_WITH_SSL), y)
SSL_LIB_PATH := ./../openssl-0.9.8o
EXTRA_CFLAGS += -DSERVER_SSL -I$(SSL_LIB_PATH)/include/
endif
ifeq ($(CONFIG_WEB_CSRF_SECURITY_SUPPORT),y)
EXTRA_CFLAGS += -DCSRF_SECURITY_PATCH
endif
EXTRA_CFLAGS += -I../common -I../system
ifeq ($(CONFIG_IPV6_ENABLE),y)
EXTRA_CFLAGS += -DCONFIG_IPV6
endif
ifeq ($(CONFIG_HTTPD_IPV6_SUPPORT),y)
EXTRA_CFLAGS += -DATHTTPD_IPV6
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_SELECTABLE
ifeq ($(CONFIG_CUTE_MAHJONG_RTK_UI),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_RTK_UI
endif
endif
ifeq ($(CONFIG_RTL_XDG),y)
EXTRA_CFLAGS += -DCONFIG_RTL_XDG
endif
ifeq ($(CONFIG_RTL_8881A),y)
EXTRA_CFLAGS += -DCONFIG_RTL_8812_SUPPORT
endif

ifeq ($(CONFIG_RTL_DNS_TRAP),y)
EXTRA_CFLAGS += -DCONFIG_RTL_DNS_TRAP
endif
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

LOCAL_OBJS-y =
#------------------------------------------------------------------------------
# LOCAL_OBJS-ys
#------------------------------------------------------------------------------
LOCAL_OBJS-y =
#LOCAL_OBJS-y += asp.o asp_form.o auth.o cgi.o fmget.o forms.o handler.o http.o jim-aio.o jim.o socket.o
LOCAL_OBJS-y += asp.o asp_form.o auth.o cgi.o fmget.o forms.o handler.o http.o socket.o
LOCAL_OBJS-$(CONFIG_RTLDD) += fmddns.o
LOCAL_OBJS-y += fmmgmt.o
LOCAL_OBJS-y += utility.o
LOCAL_OBJS-y += multipart.o
LOCAL_OBJS-y += fmtcpip.o
LOCAL_OBJS-$(CONFIG_HTTP_FILE_SERVER_SUPPORT) += fmhttp_files.o http_files.o

#-----------------------------------------------------------------------------
# web added by winfred_wang
#-----------------------------------------------------------------------------
#LOCAL_OBJS-y += fmpasswd.o
LOCAL_OBJS-$(CONFIG_SYSLOG) += fmsyslog.o
LOCAL_OBJS-y += fmdoscfg.o
#LOCAL_OBJS-y += fmlogout.o
ifeq ($(CONFIG_NOWIFI),y)
else
LOCAL_OBJS-y += fmwlan.o
endif
LOCAL_OBJS-$(CONFIG_FIREWALL) += fmfwall.o
LOCAL_OBJS-$(CONFIG_IPV6_ENABLE) += fmipv6.o
LOCAL_OBJS-$(CONFIG_ROUTED) += fmroute.o
LOCAL_OBJS-$(CONFIG_NETSNIPER_WANTYPE) += netsniper_wantype.o
LOCAL_OBJS-$(CONFIG_TR069) += fmtr069.o

PREFIX_OBJS=$(join $(dir $(LOCAL_OBJS-y)),$(addprefix $(PREFIX),$(notdir $(LOCAL_OBJS-y))))
obj-y += $(PREFIX_OBJS)
include $(ECOS_AP_DIR)/rules.mk
