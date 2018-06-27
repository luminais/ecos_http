/* project: miniUPnP
 * http://miniupnp.free.fr/
 * author: Thomas Bernard
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided in this distribution. */
/* $Id: daemonize.c,v 1.1.1.1 2007-08-06 10:04:43 root Exp $ */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "daemonize.h"

int
daemonize(void)
{
	int pid, i;
	pid = fork();
	if(pid<0)
	{
		perror("fork()");
		exit(1);
	}
	else if(pid>0)
	{
		exit(0);
	}
	
	pid = setsid(); /* obtain a new process group */
	if(pid<0)
	{
		perror("setsid()");
		exit(1);
	}
	
	for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */

	i = open("/dev/null",O_RDWR); /* open stdin */
	dup(i); /* stdout */
	dup(i); /* stderr */

	umask(027);
	chdir("/");
	/* TODO : signals
	 * some signals are allready redirected in miniupnpd */
	
	return pid;
}

