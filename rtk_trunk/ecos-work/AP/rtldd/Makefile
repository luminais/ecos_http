include $(ECOS_WORK_DIR)/.config

PREFIX?=$(notdir $(shell pwd))_
TARGET = rtldd
SUPPORT_DYN = y
SUPPORT_TZO = y
#SUPPORT_ORAY = y

#CFLAGS is from top Makefile
#CFLAGS += --save-temps
ifeq ($(CONFIG_MIPS16),y)
CFLAGS += -mips16
endif

ifeq ($(SUPPORT_DYN),y)
DYN_OBJ = ./dyndns/dyndns.o
CFLAGS += -DSUPPORT_DYN
else
DYN_OBJ = 
endif

ifeq ($(SUPPORT_TZO),y) 
TZO_OBJ = ./tzo/tzo.o
CFLAGS += -DSUPPORT_TZO
else
TZO_OBJ =
endif

ifeq ($(SUPPORT_ORAY),y)
#ORAY_OBJ = ./oray/oray.o ./oray/oray_util.o ./oray/1x_hmac.o ./oray/1x_md5c.o 
ORAY_OBJ = ./oray/oray.o ./oray/base64.o ./oray/blowfish.o ./oray/generate.o ./oray/md5.o ./oray/phglobal.o ./oray/phsocket.o ./oray/phupdate.o
CFLAGS += -DSUPPORT_ORAY
else
ORAY_OBJ = 
endif

OBJS = rtldd.o rtlddutil.o rtlconnection.o

OBJS += $(DYN_OBJ) $(TZO_OBJ) $(ORAY_OBJ)
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))

all: $(TARGET)

$(PREFIX)%.o: %.o
	@cp $^ $@

clean:
	find ./ -name "*.o" | xargs rm -f
	rm -f $(TARGET)

$(TARGET): $(OBJS) $(PREFIX_OBJS)
	$(AR) rcs $(APP_LIB) $(PREFIX_OBJS)
