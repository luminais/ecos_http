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


#ifdef INET6
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <network.h>
#include <sys/types.h>
#include <stdlib.h>
#include <tpi_dhcp6s.h>
#endif
#include <dhcp6.h>

#define DHCP6S_CONFFILE "/var/dhcp6s.conf"
#define STACK_SIZE 65535

#define AUTO_DHCP6S 1
#define MANU_DHCP6S 0
#define AUTO_DNS 1
#define MANU_DNS 0
#define DHCP6S_OPTION_DOMAIN 	"option domain-name-servers"	/* need space*/
#define DHCP6S_DOMAIN_SEARCH	"option domain-name "
static cyg_uint8 dhcp6s_stack[STACK_SIZE];
cyg_handle_t dhcp6s_thread_h = 0;
cyg_thread   dhcp6s_thread;
extern P_DHCP6S_CFG dhcp6s_cfg;
extern struct dhcp6_if *dhcp6_if_dhcp6c;

extern int dhcp6s_run;

void dhcp6s_start();

extern int dhcp6s_main();
static int init_dhcp6s_cfg()
{
	FILE *fp;
	char str[128]={0};

	fp=fopen(DHCP6S_CONFFILE,"w+");
	if(!fp)
	{
		return 1;
	}
	fprintf(fp,"%s %s %s ;\n",DHCP6S_OPTION_DOMAIN, dhcp6s_cfg->pri_dns,dhcp6s_cfg->sec_dns);
	if(strlen(dhcp6s_cfg->domain)>0)
	fprintf(fp, "%s \"%s\";\n", DHCP6S_DOMAIN_SEARCH,dhcp6s_cfg->domain);
	else
	fprintf(fp, "%s \"%s\";\n", DHCP6S_DOMAIN_SEARCH,"");

	printf("dhcp6s_cfg->dhcp6s_mode = %d \n",dhcp6s_cfg->dhcp6s_mode);
	if(1==dhcp6s_cfg->dhcp6s_mode)
	{
		fprintf(fp, "%s\n", "interface br0 {");
		fprintf(fp, "\t%s\n", "address-pool mynetwork 3600;");
		fprintf(fp, "%s\n", "};");
		fprintf(fp, "%s\n", "pool mynetwork {");
		sprintf(str,"range %s to %s;",dhcp6s_cfg->start_addr,dhcp6s_cfg->end_addr);
		fprintf(fp,"%s\n",str);
		fprintf(fp,"%s\n","};");
	}
	fclose(fp);
}

void dhcp6s_start()
{
	long int i=1;
	printf("dhcp6s_run = %d \n",dhcp6s_run);
	if(dhcp6s_run==1)
	{
		printf("dhcp6s is running \n");
		return ;
	}
	else
		dhcp6s_run=1;

	init_dhcp6s_cfg();

	while(i<1000000)
	{
		i++;
	}
	
	cyg_thread_create(
	23,
	(cyg_thread_entry_t *)dhcp6s_main,
	0,
	"dhcp6s",
	dhcp6s_stack,
	sizeof(dhcp6s_stack),
	&dhcp6s_thread_h,
	&dhcp6s_thread);
	cyg_thread_resume(dhcp6s_thread_h);
}


extern struct dhcp6_if *dhcp6_if_dhcp6c;

extern int loipc(char *cmd, int sport, int dport, char *resp, int *rlen);


void dhcp6s_stop( void )
{	
	int pid;
	pid = oslib_getpidbyname("dhcp6s");
	
	if (loipc("STOP", 0, DHCP6S_PORT, 0, 0) < 0)
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
	{
		return; 
	}
	dhcp6s_run = 0;
	free(dhcp6_if_dhcp6c);
	dhcp6_if_dhcp6c=NULL;
}
