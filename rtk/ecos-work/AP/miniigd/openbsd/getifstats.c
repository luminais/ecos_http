/* project: miniupnp
 * website : http://miniupnp.free.fr/
 * author : Ryan Wagoner and Thomas Bernard 
 * (c) 2006 Thomas BERNARD
 * This software is subject to the conditions detailed in
 * the LICENCE file provided in the distribution */
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#if defined(__FreeBSD__)
#include <net/if_var.h>
#endif
#include <net/pfvar.h>
#include <netipx/ipx.h>
#include <kvm.h>
#include <fcntl.h>
#include <nlist.h>
#include <sys/queue.h>
#include <stdio.h>
#include <string.h>

#include "getifstats.h"

#define syslog(x, fmt, args...);

struct nlist list[] = {
	{"_ifnet"},
	{NULL}
};

int getifstats(const char * ifname, struct ifdata * data)
{
#if defined(__FreeBSD__)
	struct ifnethead ifh;
#elif defined(__OpenBSD__) || defined(__NetBSD__)
	struct ifnet_head ifh;
#else
	#error "Dont know if I should use struct ifnethead or struct ifnet_head"
#endif
	struct ifnet ifc;
	struct ifnet *ifp;
	kvm_t *kd;
	ssize_t n;

	kd = kvm_open(NULL, NULL, NULL, O_RDONLY/*O_RDWR*/, NULL);
	if(!kd)
	{
		syslog (LOG_ERR, "kvm_open FAILED");
		return -1;
	}
	if(kvm_nlist(kd, list) < 0)
	{
		syslog(LOG_ERR, "kvm_nlist FAILED");
		kvm_close(kd);
		return -1;
	}
	if(!list[0].n_value)
	{
		syslog(LOG_ERR, "checking n_value FAILED");
		kvm_close(kd);
		return -1;
	}
	n = kvm_read(kd, list[0].n_value, &ifh, sizeof(ifh));
	for(ifp = TAILQ_FIRST(&ifh); ifp; ifp = TAILQ_NEXT(&ifc, if_list))
	{
		n = kvm_read(kd, (u_long)ifp, &ifc, sizeof(ifc));
		if(strcmp(ifname, ifc.if_xname) == 0)
		{
			/* found the right interface */
			data->opackets = ifc.if_data.ifi_opackets;
			data->ipackets = ifc.if_data.ifi_ipackets;
			data->obytes = ifc.if_data.ifi_obytes;
			data->ibytes = ifc.if_data.ifi_ibytes;
			data->baudrate = ifc.if_data.ifi_baudrate;
			kvm_close(kd);
			return 0;	/* ok */
		}
	}
	kvm_close(kd);
	return -1;	/* not found */
}

