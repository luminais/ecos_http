#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <bcmnvram.h>
#include <netdb.h>
#define DHCP6C_PORT 4497

#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <network.h>
#include <sys/types.h>
#include <stdlib.h>
#include "network6.h"

#define DHCP6C_CONFFILE "/var/dhcp6c.conf"
#define STACK_SIZE 20480
#define DHCP6C_STATEFULL 1
#define DHCP6C_STATELESS 0

static cyg_uint8 dhcp6c_stack[STACK_SIZE];
cyg_handle_t dhcp6c_thread_h = 0;
cyg_thread   dhcp6c_thread;
extern int dhcp6c_run;
P_DHCP6C_CFG dhcp6c_cfg = NULL;
extern DHCP6C_STATUS dhcp6c_status;
extern struct dhcp6_if *dhcp6_if_dhcp6c;

extern int loipc(char *cmd, int sport, int dport, char *resp, int *rlen);
extern void free_resources(struct dhcp6_if *);

extern void dhcp6c_main();
static int init_dhcp6c_cfg()
{
	FILE *fp;
	strcpy(dhcp6c_status.ifname,dhcp6c_cfg->ifname);


	printf("dhcp6c_cfg->stless = %d dhcp6c_cfg->gpd = %d\n",
		dhcp6c_cfg->stless,dhcp6c_cfg->gpd);

	fp=fopen(DHCP6C_CONFFILE,"w+");
	if(!fp)
	{
		return 1;
	}
	fprintf(fp,"interface %s {\n",dhcp6c_cfg->ifname);
	fprintf(fp,"\trequest domain-name-servers;\n");
	fprintf(fp,"\trequest domain-name;\n");
	if(dhcp6c_cfg->stless==0)

	{
		fprintf(fp,"\tsend ia-na %d;\n",dhcp6c_cfg->iaid);
	}



	if(dhcp6c_cfg->gpd==1)
		fprintf(fp,"\tsend ia-pd %d;\n",dhcp6c_cfg->iapd);

	fprintf(fp,"%s","};\n");

	if(dhcp6c_cfg->gpd==1)
	{
		fprintf(fp,"id-assoc pd %d {\n",dhcp6c_cfg->iapd);
		fprintf(fp,"\tprefix-interface br0 {\n");
		fprintf(fp,"\tsla-id %d;\n",dhcp6c_cfg->slid);
		fprintf(fp,"%s", "\t};\n");
		fprintf(fp,"%s", "};\n");
	}


	if(dhcp6c_cfg->stless==0)
	{
		fprintf(fp,"id-assoc na %d {\n",dhcp6c_cfg->iaid);
		fprintf(fp,"%s","};");
	}
	fclose(fp);
	return 0;
}

void dhcp6c_start()
{
	if(dhcp6c_run == 1)
	{
		return ;
	}
	else
	{
		dhcp6c_run = 1;
	}

	init_dhcp6c_cfg();
	cyg_thread_create(
	7,
	(cyg_thread_entry_t *)dhcp6c_main,
	0,
	"dhcp6c",
	dhcp6c_stack,
	sizeof(dhcp6c_stack),
	&dhcp6c_thread_h,
	&dhcp6c_thread);
	cyg_thread_resume(dhcp6c_thread_h);

	/* Make sure param is copied */
	cyg_thread_delay(10);
	return ;
}
extern struct dhcp6_if *dhcp6_if;

void dhcp6c_stop(void)
{
	int pid;


	pid = oslib_getpidbyname("dhcp6c");
	if(pid==0)
		return;
	printf("will send dhcp6c stop signal \n");
	if (loipc("STOP", 0, DHCP6C_PORT, 0, 0) < 0)
	{
		return;
	}
	if (pid)
	{
		while (oslib_waitpid(pid, NULL) != 0)
		{
			cyg_thread_delay(1);
		}
		printf("************* will free interface \n");

	}
	else
		return ;


}

