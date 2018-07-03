
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o
all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(shell cd .. ; pwd)/include 
#-Werror


#
# Enumerate files to compile
#
obj-y += obj-alinkgw_sdk.o obj-cyassl.o obj-json-c.o obj-asec.o#阿里功能
obj-y += tai_probe_info.o tai_tpsk.o  tai_wifi_sche.o #无线信息上报 一键配网 无线定时开关
obj-y += tai_security.o  #安全属性
obj-y += tai_wifiswitch.o #wifi开关
obj-y += tai_hash.o tenda_getclients.o tai_clientlist.o#客户端列表
obj-y += tai_attack.o tai_speed.o
obj-y += tai_wifi_info.o tai_wifi_security.o tai_channel.o tai_signal.o tai_qos.o tai_subdev.o
obj-y += tai_auth.o tai_common.o tai_main.o cJSON.o #公共

ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
clean:
	find . -name '*.[od]' -type f -print | xargs rm -f