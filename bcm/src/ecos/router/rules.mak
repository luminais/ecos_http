XCC           = $(CROSS_PREFIX)gcc
XCXX          = $(XCC)
XLD           = $(CROSS_PREFIX)ld
XOC           = $(CROSS_PREFIX)objcopy
XNM           = $(CROSS_PREFIX)nm
XOD           = $(CROSS_PREFIX)objdump
XAR           = $(CROSS_PREFIX)ar

# RULES
%.o: %.o.gz
	gzip -d -c $< > $*.o

%.o: %.c
	$(XCC) -c -o $*.o $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) $<

%.o: %.cxx
	$(XCXX) -c -o $*.o $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) $<

%.o: %.C
	$(XCXX) -c -o $*.o $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) $<

%.o: %.cc
	$(XCXX) -c -o $*.o $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) $<

%.o: %.S
	$(XCXX) -c -o $*.o $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) -D__GNU_ASM__ $<

%.d: %.o.gz
	@echo "$@ : $<" > $@
	gzip -d -c $< > $*.o
	
%.d : %.c
	$(XCXX) -c  -o $(@:.d=.o) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)
	
%.d : %.cxx
	$(XCXX) -c  -o $(@:.d=.o) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)
	
%.d : %.C
	$(XCXX) -c  -o $(@:.d=.o) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)
	
%.d : %.cc
	$(XCXX) -c  -o $(@:.d=.o) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)
	
%.d : %.S
	$(XCXX) -c  -o $(@:.d=.o) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) -D__GNU_ASM__ $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)

%.i: %.c
	$(XCC) -E -o $*.i $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) $< 
		
%.bin: %
	$(XOC) -O binary $(@:.bin=) $@

%.map: %
	$(XNM) $(@:.map=) | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > $@

%.dis: %
	$(XOD) -S --show-raw-insn $(@:.dis=) > $@

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
