#
#                                                                         
# 用途：由.config文件创建autoconf.h文件, llm                              
#                                                                         


#!/bin/bash 

INPUT=$1

echo "#ifndef _AUTOCONF_H_"
echo "#define _AUTOCONF_H_"

awk  -F '=' '{										\
	if(match($1, /^CONFIG_/))						\
	{												\
		if(match($2, /^y$/))						\
			printf "#define __%s__ 1\n", $1;		\
		else										\
			printf "#define __%s__ %s\n", $1, $2;	\
	}												\
	else if(match($0, /^#.?CONFIG_.*is not set/))	\
	{												\
		split($0, array, " ");						\
		printf "#undef __%s__\n", array[2];			\
	}												\
	else											\
		printf "// %s \n", $0						\
	}' $INPUT

echo "#endif"
