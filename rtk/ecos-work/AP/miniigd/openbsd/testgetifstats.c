#include <stdio.h>
#include "getifstats.h"

int main(int argc, char * * argv)
{
	int r;
	struct ifdata data;
	printf("usage: %s if_name\n", argv[0]);
	if(argc<2)
		return -1;
	r = getifstats(argv[1], &data);
	if(r<0)
		printf("getifstats() failed\n");
	else
	{
		printf("ipackets = %10lu   opackets = %10lu\n",
		       data.ipackets, data.opackets);
		printf("ibytes =   %10lu   obytes =   %10lu\n",
		       data.ibytes, data.obytes);
		printf("baudrate = %10lu\n", data.baudrate);
	}
	return 0;
}

