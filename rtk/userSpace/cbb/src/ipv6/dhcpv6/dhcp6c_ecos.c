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


#ifdef INET6
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <network.h>
#include <sys/types.h>
#include <stdlib.h>
#include "network6.h"
#endif


extern struct dhcp6_if *dhcp6_if_dhcp6s;
#define DHCP6C_CONFFILE "/var/dhcp6c.conf"
extern P_DHCP6C_CFG dhcp6c_cfg;
#define STACK_SIZE 65535
#define DHCP6C_STATEFULL 1
#define DHCP6C_STATELESS 0  
static cyg_uint8 dhcp6c_stack[STACK_SIZE];
cyg_handle_t dhcp6c_thread_hs = 0;
cyg_thread   dhcp6c_thread;

extern P_DHCP6C_CFG dhcp6c_cfg;
extern DHCP6C_STATUS dhcp6c_status;


extern int loipc(char *cmd, int sport, int dport, char *resp, int *rlen);
extern void free_resources6s(struct dhcp6_if *);

extern void dhcp6c_mains();
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

void dhcp6c_starts()
{
	
	
	init_dhcp6c_cfg();
	cyg_thread_create(
	25,
	(cyg_thread_entry_t *)dhcp6c_mains,
	0,
	"dhcp6c",
	dhcp6c_stack,
	sizeof(dhcp6c_stack),
	&dhcp6c_thread_hs,
	&dhcp6c_thread);
	cyg_thread_resume(dhcp6c_thread_hs);
	return ;
}
extern struct dhcp6_if *dhcp6_if;

void dhcp6c_stops(void)
{
	int pid;
	pid = oslib_getpidbyname("dhcp6c");
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
	}
	else
		return ;
	
	free_resources6s(dhcp6_if_dhcp6s);
	free(dhcp6_if_dhcp6s);
	dhcp6_if_dhcp6s=NULL;
	
}

