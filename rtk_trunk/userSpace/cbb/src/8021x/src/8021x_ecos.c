/*
 *roy +++
 */
#include <8021x.h>
#include <8021x_ecos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iflib.h>
#include <ecos_oslib.h>
#include <bcmnvram.h>


int x8021_running = 0;
int x8021_shutdown = 0;

extern int x8021_conn_successflag;

cyg_handle_t c8021x_thread_h = 0;
cyg_thread   c8021x_thread;
#define C8021X_STACK (8192)
static cyg_uint8 c8021x_stack[ C8021X_STACK];
static char c8021x_thread_name[]="8021XC";

static void
X8021_main(struct x8021_ifc *ctx)
{
	/* Enter os independent main loop */
	X8021_mainloop(ctx);
}


void
X8021_start(char *ifname)
{
	struct x8021_ifc *ctx;

	if(x8021_running)
		return ;
	ctx = (struct x8021_ifc *)malloc(sizeof(*ctx));

	if(ctx == NULL) return;
	
	//memset(&ctx,0,sizeof(ctx));
	/* Setup param */
	strcpy(ctx->ifname, ifname);
	strcpy(ctx->user_name,nvram_safe_get("wan0_1x_username"));
	strcpy(ctx->password,nvram_safe_get("wan0_1x_password"));

	ctx->mtu = atoi(nvram_safe_get("wan0_1x_mtu"));
	
	/* Get physical MAC addr */
	if (iflib_getifhwaddr(ifname, ctx->macaddr) != 0){
		free(ctx);
		return ;
	}
	/* Create thread */	
	cyg_thread_create(
		8,
		(cyg_thread_entry_t *)X8021_main,
		(cyg_addrword_t)ctx,
		c8021x_thread_name,
		c8021x_stack,
		C8021X_STACK,
		&c8021x_thread_h,
		&c8021x_thread);
	cyg_thread_resume(c8021x_thread_h);

	/* Make sure param is copied */
	cyg_thread_delay(10);

	return;
}

void
X8021_stop(char *ifname)
{
	int pid;
	
	pid = oslib_getpidbyname(c8021x_thread_name);
	if (pid == 0)
		return;

	/* Send stop signal to dhcpc */
	x8021_shutdown=1;
	x8021_conn_successflag=0;

	/* Wait until thread exit */
	while (oslib_waitpid(pid, NULL) != 0)
		cyg_thread_delay(1);

	return;
}


