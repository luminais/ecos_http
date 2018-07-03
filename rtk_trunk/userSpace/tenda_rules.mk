#
# tenda app的编译规则
# CFLAGS：tenda app 总Makefile中定义
# EXTRA_CFLAGS：各模块根据需要自己定义
# llm, 2015-12-24 14:36:58
#

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MD -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MD -c $< -o $@

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(EXTRA_CFLAGS) $(DEBUG_CFLAGS) -MD -c $< -o $@

	
	
ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
