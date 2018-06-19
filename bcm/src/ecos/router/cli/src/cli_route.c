/*
 * Network protocol CLI route debug commands.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_route.c,v 1.2 2010-07-26 09:07:51 Exp $
 *
 */
 //roy +++2010/09/06

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <cli.h>

typedef void pr_fun(char *fmt, ...);
extern void show_network_tables(pr_fun *);
extern int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
extern int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
extern void route_flush(char *ifname);

static int route_cmd(int argc, char* argv[])
{
	char *ifname=NULL;

	if(argc == 1){
		show_network_tables(&printf);
	}else if(argc == 3){
	//flush
		ifname = argv[3];
		route_flush(ifname);
	}else if(argc >= 5){
		
		if (argc > 5)
			ifname = argv[5];
		
		 if ( !strcmp(argv[0], "add" ))
			route_add(ifname, 1,argv[2], argv[3], argv[4]);
		else if ( !strcmp(argv[0], "del" ))
			route_del(ifname, 1,argv[2], argv[3], argv[4]);
		else
			printf("route add/del/flush\n\r");
	}else{
		printf("route add/del/flush\n\r");
	}

	return 0;	
}

/* cli node */
CLI_NODE("route",	route_cmd,	"route command");

 //roy+++
 
