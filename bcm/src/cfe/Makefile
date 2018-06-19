PRODUCT ?=fh450v3
ENVRAM_SIZE:=0x1000
CFE_VERSION := V1.0.0.6
CFE_TIME = `date +%y%m%d`
PATH:=$(PATH):/projects/hnd/tools/linux/hndtools-mipsel-linux-uclibc-4.2.3/bin/
all:
	make clean
	make -C cfe/build/broadcom/bcm947xx/ CFG_GMAC=1 CFG_CFE_VERSION=$(CFE_VERSION) CFG_PRODUCT=$(PRODUCT)
	./tools/nvserial -i cfe/build/broadcom/bcm947xx/compressed/cfez.bin  -o target/$(PRODUCT)/cfez_$(PRODUCT)_$(CFE_VERSION)_$(CFE_TIME).bin -b 0x400 -c $(ENVRAM_SIZE) -s $(PRODUCT) target/$(PRODUCT)/envram_$(PRODUCT).txt 
clean:
	make -C cfe/build/broadcom/bcm947xx/ CFG_GMAC=1 clean

release:
	make all PRODUCT=f3
	make all PRODUCT=f3_en
	make all PRODUCT=fh450v3
	make all PRODUCT=fh456
	make all PRODUCT=fh456_ali
	make all PRODUCT=fh303
	make all PRODUCT=fh307
help:
	@echo ""
	@echo "if you wang to change a product , you must <make clean > at first!"
	@echo "you must have the PRODUCT options , when you make everytime!"
	@echo "make PRODUCT=fh450v3|f3|f3_en|fh456|fh456_ali"
	@echo ""
