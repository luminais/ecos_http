#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <net/ethernet.h>

#if defined(CYGPKG_NET_OPENBSD_STACK)
char *ether_ntoa (struct ether_addr *e)
{
	static char a[] = "xx:xx:xx:xx:xx:xx";

	/*
	if (e->ether_addr_octet[0] > 0xFF || e->ether_addr_octet[1] > 0xFF ||
	    e->ether_addr_octet[2] > 0xFF || e->ether_addr_octet[3] > 0xFF ||
	    e->ether_addr_octet[4] > 0xFF || e->ether_addr_octet[5] > 0xFF) {
		errno = EINVAL;
		return (NULL);
	}
	*/
	(void)sprintf(a, "%02x:%02x:%02x:%02x:%02x:%02x",
	    e->ether_addr_octet[0], e->ether_addr_octet[1],
	    e->ether_addr_octet[2], e->ether_addr_octet[3],
	    e->ether_addr_octet[4], e->ether_addr_octet[5]);

	return (a);
}
#elif defined(CYGPKG_NET_FREEBSD_STACK)
char *ether_ntoa(const struct ether_addr *e)
{
	static char a[] = "xx:xx:xx:xx:xx:xx";

	/*
	if (e->ether_addr_octet[0] > 0xFF || e->ether_addr_octet[1] > 0xFF ||
	    e->ether_addr_octet[2] > 0xFF || e->ether_addr_octet[3] > 0xFF ||
	    e->ether_addr_octet[4] > 0xFF || e->ether_addr_octet[5] > 0xFF) {
		errno = EINVAL;
		return (NULL);
	}
	*/
	(void)sprintf(a, "%02x:%02x:%02x:%02x:%02x:%02x",
	    e->octet[0], e->octet[1],
	    e->octet[2], e->octet[3],
	    e->octet[4], e->octet[5]);
	return (a);
}
#endif

static char *
_ether_aton(s, e)
	char *s;
	struct ether_addr *e;
{
	int i;
	long l;
	char *pp;

	while (isspace(*s))
		s++;

	/* expect 6 hex octets separated by ':' or space/NUL if last octet */
	for (i = 0; i < 6; i++) {
		l = strtol(s, &pp, 16);
		if (pp == s || l > 0xFF || l < 0)
			return (NULL);
		if (!(*pp == ':' || (i == 5 && (isspace(*pp) || *pp == '\0'))))
			return (NULL);
#ifdef CONFIG_NET_STACK_FREEBSD
		e->octet[i] = (u_char)l;
#else
		e->ether_addr_octet[i] = (u_char)l;
#endif
		s = pp + 1;
	}

	/* return character after the octets ala strtol(3) */
	return (pp);
}

struct ether_addr *
ether_aton(s)
	char *s;
{
	static struct ether_addr n;

	return (_ether_aton(s, &n) ? &n : NULL);
}

