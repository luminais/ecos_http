#include <stdio.h>

extern char _binary_www_bin_start[];

void tar_base(unsigned long adrs);

void 
tarbase_init()
{
	tar_base((unsigned long)_binary_www_bin_start);
}
