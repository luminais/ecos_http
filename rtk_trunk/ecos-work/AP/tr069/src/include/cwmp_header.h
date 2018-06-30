#ifndef __CWMP_HEADER_H
#define __CWMP_HEADER_H
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

#endif
