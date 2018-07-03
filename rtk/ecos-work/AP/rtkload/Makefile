
include $(ECOS_WORK_DIR)/.config
include $(USER_CONFIG)
-include $(ECOS_INSTALL_DIR)/include/pkgconf/ecos.mak

DOQUIET = false
#DOQUIET = true 
#KERNEL_ROOT = /home/john/RealTek/linux-2.5/linux
KERNEL_ROOT = .
#ifeq ($(wildcard $(DIR_USERS)/goahead-2.1.1/LINUX/cvimg),)
CVIMG=./cvimg
#else
#CVIMG=$(DIR_USERS)/goahead-2.1.1/LINUX/cvimg
#endif

# ---- shouldn't need to modify below this line.

LDSCRIPT = ld.script

#COPTIONS = -DROM_MEMORY -DCOMPRESSED_KERNEL -D__KERNEL__
COPTIONS = -DCOMPRESSED_KERNEL

ifeq ($(CONFIG_RTL_8197F),y)
CACHE_FN=cache-mips32
else
CACHE_FN=cache
endif

#SOURCES = vsprintf.c string.c ctype.c prom_printf.c misc.c hfload.c start.S read_memory.c
ifdef BZ2_COMPRESS
	SOURCES = misc.c hfload.c start.S $(CACHE_FN).c
else
ifdef GZIP_COMPRESS
	SOURCES = string.c ctype.c misc.c hfload.c start.S read_memory.c $(CACHE_FN).c
else
	SOURCES = string.c ctype.c misc.c hfload.c start.S read_memory.c $(CACHE_FN).c LzmaDecode.c
endif	
endif

LOADER_FILES = hfload.o read_memory.o
#LOADER_FILES = hfload.o
ifdef BZ2_COMPRESS
	SUPPORT_FILES = misc.o $(CACHE_FN).o
else
ifdef GZIP_COMPRESS
	SUPPORT_FILES = vsprintf.o prom_printf.o string.o ctype.o misc.o $(CACHE_FN).o	
else
ifeq ($(strip $(DOQUIET)),true)
    SUPPORT_FILES = string.o ctype.o misc.o $(CACHE_FN).o LzmaDecode.o 
else
    SUPPORT_FILES = vsprintf.o prom_printf.o string.o ctype.o misc.o $(CACHE_FN).o LzmaDecode.o  
endif
endif	
endif

#----------------------------------------------------------------------------
# Separate C++ flags out from C flags.
ACTUAL_CFLAGS = $(ECOS_GLOBAL_CFLAGS)
ACTUAL_CFLAGS := $(subst -fno-rtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -frtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -Woverloaded-virtual,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -fvtable-gc,,$(ACTUAL_CFLAGS))

ACTUAL_CXXFLAGS = $(subst -Wstrict-prototypes,,$(ECOS_GLOBAL_CFLAGS))

CFLAGS  = $(ACTUAL_CFLAGS) -D__ECOS
ASFLAGS = $(ACTUAL_CFLAGS) -D__ECOS
CXXFLAGS = $(ACTUAL_CXXFLAGS) -D__ECOS

#----------------------------------------------------------------------------

#ifeq ($(CONFIG_RTL_8196D),y)
#MARCH = 5281
#else
#MARCH = 4181
#endif

#CFLAGS = -march=$(MARCH) -Os -g -fno-pic -mno-abicalls
CFLAGS += -fno-pic -mno-abicalls
ifeq ($(strip $(DOQUIET)),true)
CFLAGS  += $(WARNINGS)  -D__DO_QUIET__
endif
CFLAGS += -DEMBEDDED -I./ -I$(ECOS_INSTALL_DIR)/include $(COPTIONS) -G 0
ifeq ($(CONFIG_RTL_8197F),y)
CFLAGS += -D__LITTLE_ENDIAN__ -DCONFIG_RTL_8197F
endif
#CFLAGS += -DEMBEDDED -I./ -I$(KERNEL_ROOT)/include/linux -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/lib $(COPTIONS) -G 0
#CFLAGS += -I$(KERNEL_ROOT)/arch/rlx/bsp -I$(KERNEL_ROOT)/arch/rlx/include -I$(KERNEL_ROOT)/arch/rlx/include/asm/mach-generic 
#ASFLAGS = -g -DEMBEDDED -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/rlx/include
#CFLAGS +=  -I$(KERNEL_ROOT)/arch/mips/include -I$(KERNEL_ROOT)/arch/mips/include/asm/mach-generic 
#ASFLAGS = -g -DEMBEDDED -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/mips/include
#ASFLAGS = -march=$(MARCH) -g -fno-pic -mno-abicalls -DEMBEDDED -I./ -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/rlx/include
ASFLAGS += -fno-pic -mno-abicalls -DEMBEDDED -I./ -I$(ECOS_INSTALL_DIR)/include

CFLAGS += -include $(ECOS_INSTALL_DIR)/include/pkgconf/system.h -include $(ECOS_INSTALL_DIR)/include/cyg/hal/bspcpu.h
ASFLAGS += -include $(ECOS_INSTALL_DIR)/include/pkgconf/system.h -include $(ECOS_INSTALL_DIR)/include/cyg/hal/bspcpu.h

LDFLAGS=-static -nostdlib

ifeq ($(CONFIG_RTL_8197F),y)
LDFLAGS += -EL
endif

ifdef BZ2_COMPRESS
CFLAGS += -DBZ2_COMPRESS
ASFLAGS += -DBZ2_COMPRESS
else
ifndef GZIP_COMPRESS
CFLAGS += -DLZMA_COMPRESS
ASFLAGS += -DLZMA_COMPRESS
endif
endif

START_FILE = start.o


LOAD_START_ADDR=0x80500000
ifeq ($(CONFIG_JFFS2_SUPPORT),y)
ifneq ($(CONFIG_ECOS_IMAGE_OFFSET),)
ECOS_IMAGE_OFFSET=$(subst 0x,,$(CONFIG_ECOS_IMAGE_OFFSET))
FLASH_OFFSET=$(shell echo "ibase=obase=16;14000 + $(ECOS_IMAGE_OFFSET)"|bc)
else
FLASH_OFFSET=54000
endif
else
ifneq ($(CONFIG_ECOS_IMAGE_OFFSET),)
ECOS_IMAGE_OFFSET=$(subst 0x,,$(CONFIG_ECOS_IMAGE_OFFSET))
FLASH_OFFSET=$(ECOS_IMAGE_OFFSET)
else
FLASH_OFFSET=20000
endif
endif

CV_OPTION=signature
#CV_SIGNATURE=klln
CV_SIGNATURE=cs6c
#CV_SIGNATURE=csys

#CROSS_COMPILE ?= rsdk-linux-
#CROSS_COMPILE ?= rsdk-elf-
ifeq ($(CONFIG_RTL_8197F),y)
ifeq ($(CONFIG_RAM_SIZE_16M),y)
LOAD_START_ADDR=0x80a00000
else
LOAD_START_ADDR=0x80500000
endif
CROSS_COMPILE ?= msdk-elf-
else
CROSS_COMPILE ?= mips-elf-
endif

LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip
AS=$(CROSS_COMPILE)as
NM=$(CROSS_COMPILE)nm
OBJCOPY=$(CROSS_COMPILE)objcopy

SEDFLAGS	= s/LOAD_ADDR/$(LOAD_START_ADDR)/;


O_TARGET := rtk
obj-y		:= vmlinux_img.o $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)


all: clean rtk-vmlinux_img $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)

#memload-partial
#	$(CC) -fno-pic -mno-abicalls -Os -fomit-frame-pointer -D__KERNEL__ -c vmlinux_img.c -o vmlinux_img.o
ifeq ($(CONFIG_RTL_8197F),y)
	$(CC) -EL -msoft-float -fno-pic -mno-abicalls -Os -fomit-frame-pointer -c vmlinux_img.c -o vmlinux_img.o
else
	$(CC) -fno-pic -mno-abicalls -Os -fomit-frame-pointer -c vmlinux_img.c -o vmlinux_img.o
endif
	$(OBJCOPY) --add-section .vmlinux=vmlinux_img.gz vmlinux_img.o
	@sed "$(SEDFLAGS)" < ld.script.in > $(LDSCRIPT)
	$(LD) $(LDFLAGS) -G 0 -T $(LDSCRIPT) -o memload-partial $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES) vmlinux_img.o
	$(NM) memload-partial | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > system.map
	
	cp memload-partial memload-full
	$(OBJCOPY) -Obinary memload-full nfjrom
	$(CVIMG) $(CV_OPTION) nfjrom ecos.bin $(LOAD_START_ADDR) $(FLASH_OFFSET) $(CV_SIGNATURE)
#	@./size_chk vmlinux_img $(LOAD_START_ADDR)
	@$(CVIMG) size_chk vmlinux_img $(LOAD_START_ADDR) || exit $$?
ifeq ($(CONFIG_BLK_DEV_INITRD),y)
#	@echo "RAMFS no need to check flash size"
else
ifeq ($(CONFIG_RTL_FLASH_MAPPING_ENABLE),y)
#	@$(CVIMG) flash_size_chk ecos.bin || exit $$?
endif
endif

clean:
	rm -f *.o memload system.map nfjrom memload-partial memload-full vmlinux_img.gz target target.img strip1 ecos.bin vmlinux-stripped  $(LDSCRIPT) vmlinux_img
	
rtk-vmlinux_img: 
	cp $(KERNEL_ROOT)/ecos.elf vmlinux-stripped
	$(STRIP) vmlinux-stripped
	$(OBJCOPY) -Obinary vmlinux-stripped vmlinux_img
	rm -f vmlinux_img.gz
ifdef BZ2_COMPRESS
	cat vmlinux_img | bzip2 -9v > vmlinux_img.gz
else
ifdef GZIP_COMPRESS
	cat vmlinux_img | gzip -9v > vmlinux_img.gz	
else
	./lzma e vmlinux_img vmlinux_img.gz
endif
endif
	$(CVIMG) vmlinuxhdr vmlinux_img.gz vmlinux_img.gz $(KERNEL_ROOT)/ecos.elf

depend:
	rm -f .depend
	$(CC) $(CFLAGS) -MM $(SOURCES) >.depend

bz2:
	@make BZ2_COMPRESS=1

gzip:
	@make GZIP_COMPRESS=1

