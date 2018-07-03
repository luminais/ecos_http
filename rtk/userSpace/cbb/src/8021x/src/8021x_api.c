
#include <8021x.h>
#include <8021x_ecos.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//roy +++
int X8021_write(int fd, unsigned char *dst_hwaddr, char *data, int len)
{
	int result = 0,datalen;
	struct ether_header *eh;

	datalen = (len+14)<60?60:(len+14);
	
	char *buf = (char *)malloc(datalen);

	if(buf == NULL){
		diag_printf("X8021_write: malloc failed\n");
		return -1;
	}
	
	memset(buf,0,datalen);
	eh = (struct ether_header *)buf;

	memcpy(eh->ether_dhost,dst_hwaddr,sizeof(eh->ether_dhost));
	eh->ether_type = ntohs(0x888e);

	memcpy(&buf[14],data,len);

	if(fd >0) result = write(fd, buf, datalen);

	if (result <= 0) {
		diag_printf("X8021_write: raw send failed\n");
	}

	return result;
}	
//+++

