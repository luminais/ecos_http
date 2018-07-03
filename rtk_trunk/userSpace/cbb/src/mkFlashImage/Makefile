# We need nvram.txt and envram.txt path
TOPDIR=$(shell cd ../../../.. ; pwd)
USEDIR=$(shell cd ../../.. ; pwd)
include $(USEDIR)/.config
#echo $TOPDIR

EXEC = mkFlashImage

CFE_PATH       = $(TOPDIR)/targets/$(CONFIG_BOOT_FILE)/boot.bin
IMAGE_PATH     = $(TOPDIR)/ecos-work/RTK0_MAGIC_upgradefile.bin 
OUT_FLASH_PATH = $(TOPDIR)/ecos-work/


.PHONY:all clean mkFlashImage

all:$(EXEC) flash_image

mkFlashImage:
	cp $(USEDIR)/config/cfg/$(CONFIG_Tenda_DEFAULT_PROFILE) ./
	gcc -g -o $@ mkFlashImage.c $(CONFIG_Tenda_DEFAULT_PROFILE) -I$(USEDIR)/include -I$(USEDIR)/cbb/include -I$(USEDIR)/cbb/src/nvram

flash_image:
	./mkFlashImage --cfe $(CFE_PATH) --image $(IMAGE_PATH) --out $(OUT_FLASH_PATH)


help:
	@echo "make mkFlashImage                   # Create binary command."
	@echo "make flash_image                    # Create Flash image."

clean:
	rm $(EXEC)
	
