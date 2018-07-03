CFLAGS		+= $(GLOBAL_CFLAGS)
CXXFLAGS	+= $(GLOBAL_CXXFLAGS)

SOURCES		:= $(patsubst %.o, %.c, $(filter-out %/, $(obj-y)))
__subdir-y	:= $(patsubst %/,%,$(filter %/, $(obj-y)))
subdir-y	+= $(__subdir-y)
obj-y		:= $(patsubst %/, %/$(TARGET), $(obj-y))

CXXSOURCES	:= $(patsubst %.o, %.cxx, $(filter-out %/, $(cxxobj-y)))
CPPSOURCES	+= $(patsubst %.o, %.cpp, $(filter-out %/, $(cppobj-y)))

DEPENDENCY  = $(obj-y:.o=.d) $(cxxobj-y:.o=.d) $(cppobj-y:.o=.d)

.PHONY: all clean $(subdir-y)

$(TARGET): $(subdir-y) $(obj-y) $(cxxobj-y) $(cppobj-y)
	$(LD) -r -o $@ $(GLOBAL_LDFLAGS) $(obj-y) $(cxxobj-y) $(cppobj-y)

$(subdir-y):
	$(MAKE) -C $@


%.i: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MMD -E $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MMD -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MMD -c $< -o $@

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MMD -c $< -o $@

$(PREFIX)%.o: %.o
	@cp $^ $@

clean:
	-for i in $(subdir-y) ; do \
		$(MAKE) -C $$i clean ; \
	done
	rm -f $(DEPENDENCY) 
	rm -f $(obj-y) $(cxxobj-y) $(cppobj-y) $(TARGET)

-include $(DEPENDENCY)
