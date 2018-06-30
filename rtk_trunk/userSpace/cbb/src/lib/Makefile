#
# lib库Makefile
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)


#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS =
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/include


#
# Enumerate files to compile
#
ALL_OBJS = 
ALL_OBJS += base64.o 
ALL_OBJS += blowfish.o 
ALL_OBJS += crypt.o 
ALL_OBJS += ecos_oslib.o 
# ALL_OBJS += ether.o 	#内核net/ethernet.h有定义
ALL_OBJS += ether.o 	
# ALL_OBJS += getaddrinfo.o 	#内核net/common中有
ALL_OBJS += getaddrinfo.o 	
# ALL_OBJS += getproto.o 	#内核net/common中有
ALL_OBJS += getproto.o 
# ALL_OBJS += ifaddrs.o 	#内核net/common中有
ALL_OBJS += ifaddrs.o 
ALL_OBJS += hash_func.o 
ALL_OBJS += hcreate.o 
ALL_OBJS += iflib.o 
ALL_OBJS += inflate.o 
ALL_OBJS += libc2.o 
ALL_OBJS += libloipc.o 
ALL_OBJS += libsocket.o 
ALL_OBJS += ping.o 
ALL_OBJS += syslog.o
# ALL_OBJS += lib_test.o
# ALL_OBJS += temp.o


#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk



