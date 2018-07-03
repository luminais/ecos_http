#include <pkgconf/system.h>
#include <stdio.h>
#include <network.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef CONFIG_NET_STACK_FREEBSD
#include <net/if_var.h>
#else
#include <net/if.h>
#endif

#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>

#ifndef __SYSINFO__
#define __SYSINFO__
struct sysinfo {
        long uptime;                    /* Seconds since boot */
        unsigned long loads[3];         /* 1, 5, and 15 minute load averages */
        unsigned long totalram;         /* Total usable main memory size */
        unsigned long freeram;          /* Available memory size */
        unsigned long sharedram;        /* Amount of shared memory */
        unsigned long bufferram;        /* Memory used by buffers */
        unsigned long totalswap;        /* Total swap space size */
        unsigned long freeswap;         /* swap space still available */
        unsigned short procs;           /* Number of current processes */
        unsigned short pad;                     /* Padding needed for m68k */
        unsigned long totalhigh;        /* Total high memory size */
        unsigned long freehigh;         /* Available high memory size */
        unsigned int mem_unit;          /* Memory unit size in bytes */
        char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding: libc5 uses this.. */
};
extern int sysinfo (struct sysinfo *__info);
#endif
