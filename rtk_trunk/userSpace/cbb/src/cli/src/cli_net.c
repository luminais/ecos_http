/*************************************************************************
	> Copyright (C) 2015, Tenda Tech. Co., All Rights Reserved.
	> File Name: cli_net.c
	> Description: 
	> Author: zhuhuan
	> Mail: zhuhuan_IT@outlook.com
	> Version: 1.0
	> Created Time: Wed 23 Dec 2015 03:13:29 PM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <nvram.h>
#include <stdlib.h>
#include <cyg/kernel/kapi.h>
// #include <typedefs.h>
#include <cyg/infra/cyg_type.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pi_common.h>

#ifdef RTL819X
#include <pkgconf/mlt_mips_ram.h>
#endif

/* Functions declaration */
extern void cyg_kmem_print_stats(void);
extern void cyg_fd_thread_dump(cyg_handle_t *thread_id_list);
extern char * inet_ntoa_r(struct in_addr ina, char *buf);

externC cyg_uint8 * mem_real_region_top(cyg_uint8 *regionend) __attribute__((weak));
cyg_uint8 *	 mem_real_region_top(cyg_uint8 *regionend)
{
	unsigned long mem;

	/* Figure out memory size by finding aliases */
	for (mem = (1 << 20); mem < (128 << 20); mem <<= 1) {
		if (*(unsigned long *)((unsigned long)(mem_real_region_top) + mem) == 
		    *(unsigned long *)(mem_real_region_top))
			break;
	}
	/* Ignoring the last page when ddr size is 128M. Cached
	 * accesses to last page is causing the processor to prefetch
	 * using address above 128M stepping out of the ddr address
	 * space.
	 */
	if (mem == (128 << 20))
		mem -= 0x1000;
	return (cyg_uint8 *)(0x80000000u + mem);
}

#ifdef RTL819X
/* MBUF and memory show command */
int mbuf_cmd(int argc, char *argv[])
{
	struct mallinfo mem_info;

	cyg_kmem_print_stats();

	mem_info = mallinfo();
	printf("\nMemory system:"
		"\n   Total %d bytes"    
		"\n   Heap %d bytes, Free %d bytes, Max %d bytes\n",
		CYGMEM_REGION_ram_SIZE,
		mem_info.arena, mem_info.fordblks, mem_info.maxfree);
	return 0;
}
#endif

static int  show_thread_fd(void)
{
	cyg_handle_t t_handle = 0;
	cyg_uint16 id = 0;
	int more;
	cyg_handle_t fd_thread_list[CYGNUM_FILEIO_NFD] = {0};
	int i, j;
	
	/* Get thread id list */
	cyg_fd_thread_dump(fd_thread_list);

	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	do {
		cyg_thread_info info;

		more = cyg_thread_get_next(&t_handle, &id);
		if (t_handle == 0)
			break;

		cyg_thread_get_info(t_handle, id, &info);
		if (info.name == NULL)
			info.name = "(null thread)";

		printf("\r\n%-22s:", info.name);
		for (i = 0, j = 0; i < CYGNUM_FILEIO_NFD; i++) {
			if (fd_thread_list[i] == t_handle) {
				printf(" [%d]", i);
				j++;
				if ((j%10) == 0)
					printf("\r\n%-22s:", "");
			}
		}
	}
	while (more == true);

	printf("\r\n");
	return 0;
}

//add by z10312 规避方法解决概率性mbuf内存耗尽网络不通问题
int whether_times_network_support_run()
{
	
	cyg_handle_t t_handle = 0;
	cyg_uint16 id = 0;
	int more;
	int Network_alarm_support_run = 0;
	int Network_support_run = 0;
	
	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	
	do {
		cyg_thread_info info;
		char *state_string;

		more = cyg_thread_get_next(&t_handle, &id);
		if (t_handle == 0)
			break;
		
		cyg_thread_get_info(t_handle, id, &info);
		
		if (info.name && (info.state == 0))
		{
			if (!strcmp(info.name, "Network alarm support"))
			{
				Network_alarm_support_run = 1;
			}
			else if (!strcmp(info.name, "Network support") )
			{
				Network_support_run = 1;
			}
		}
		
	}
	while (more == true);

	if ((Network_alarm_support_run == 1) && (Network_support_run == 1))
	{
		return 1;
	}
	else 
	{
		return 0;
	}

}

static int  show_threads(int argc, char* argv[])
{
	cyg_handle_t t_handle = 0;
	cyg_uint16 id = 0;
	int more;
	int stack_usage;
	
	printf("id    state    Pri(Set) Name                   StackBase"
		    "   Size   usage\n\r");
	
	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	do {
		cyg_thread_info info;
		char *state_string;

		more = cyg_thread_get_next(&t_handle, &id);
		if (t_handle == 0)
			break;
		
		cyg_thread_get_info(t_handle, id, &info);
		if (info.name == NULL)
			info.name = "(null thread)";
		
		/*
		 * Translate the state into a string.
		 */
		if (info.state == 0)
			state_string = "RUN";
		else if (info.state & 0x04)
			state_string = "SUSP";
		else {
			switch (info.state & 0x1b) {
			case 0x01:
				state_string = "SLEEP";
				break;
			case 0x02:
				state_string = "CNTSLEEP";
				break;
			case 0x08:
				state_string = "CREATE";
				break;
			case 0x10:
				state_string = "EXIT";
				break;
			default:
				state_string = "????";
				break;
			}
		}
		
			
//roy modify
		stack_usage = cyg_thread_measure_stack_usage(t_handle);


		if (argc == 3)
		{
			
			if (atoi(argv[2]) == id)
			{
				
				/*
				* Now generate the row.
				*/
				printf("%04d  %-8s %-2d ( %-2d) ", id, state_string, info.cur_pri, info.set_pri);


				/*show_stack_usage((char *)info.stack_base,
				             (char *)(info.stack_base+info.stack_size));*/
				printf("%-22s 0x%08x  %-6d %-6d",
				info.name, info.stack_base, info.stack_size,
				stack_usage);
				printf("\r\n");
			
				unsigned int i = 0, j = 0;
				unsigned int * stack_pptr;
				stack_pptr = (unsigned int *)info.stack_base;
				
				cyg_scheduler_lock();
				while (i < (info.stack_size /4))
				{

					if (stack_pptr[i] != 0xDEADBEEF) 
					{
						j++;
						printf("[<%08x>] ",   stack_pptr[i]);
						if (j && (j % 5 == 0))
							printf ("\n");
					}
					i++;
				}			
				cyg_scheduler_unlock();
				
				printf("\r\n");
			}
			else 
			{
				continue;
			}
		}
		else
		{
			
			/*
			* Now generate the row.
			*/
			printf("%04d  %-8s %-2d ( %-2d) ", id, state_string, info.cur_pri, info.set_pri);


			/*show_stack_usage((char *)info.stack_base,
			                     (char *)(info.stack_base+info.stack_size));*/
			printf("%-22s 0x%08x  %-6d %-6d",
			info.name, info.stack_base, info.stack_size,
			stack_usage);
			printf("\r\n");
		}

		
			
	}
	while (more == true);

	return 0;
}

#define	SIN(a)	((struct sockaddr_in *)a)

static char *	ether_ntoa_r(const struct ether_addr *addr, char *buf)
{
	unsigned char *p = (unsigned char *)addr->octet;

	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", p[0], p[1], p[2], p[3], p[4], p[5]);
	return buf;
}

int thread_cmd(int argc, char* argv[])
{
	/* Get the thread id from the hidden control */
	cyg_handle_t t_handle = 0;
	cyg_uint16 id;

	/* Show all threads */
	if (argc == 1 || !strcmp(argv[1], "show")) {
		show_threads(argc, argv);
		return 0;
	}

	if (!strcmp(argv[1], "fd")) {
		show_thread_fd();
		return 0;
	}

	if(argc < 4)
	{
		printf("Parameters error!\n");
		goto usage;
	}
		
	/* Show a specific thread with id */
	if (sscanf(argv[1], "%04hx", &id) == 0 ||
	    (t_handle = cyg_thread_find(id)) == 0) {
		printf("thread id not exist: %s\n", argv[1]);
		goto usage;
	}

	/* Set thread priority */
	if (!strcmp(argv[2], "pri")) {
		cyg_priority_t pri;

		sscanf(argv[3], "%d", &pri);
		cyg_thread_set_priority(t_handle, pri);
		return 0;
	}

	/* If there is a state field, change the thread state */
	if (!strcmp(argv[2], "state")) {
		if (strcmp(argv[3], "run") == 0)
			cyg_thread_resume(t_handle);
		if (strcmp(argv[3], "suspend") == 0)
			cyg_thread_suspend(t_handle);
		if (strcmp(argv[3], "release") == 0)
			cyg_thread_release(t_handle);
		if (strcmp(argv[3], "kill") == 0)
			cyg_thread_kill( t_handle );
		
		return 0;
	}

usage:
	printf("thread <show>\n");
	printf("thread fd\n");
	printf("thread id pri pri-val\n");
	printf("thread id pri run/suspend/release\n");
	return 0;
}

/*
 * Print the status of the interface.  If an address family was
 * specified, show it and it only; otherwise, show them all.
 */
static int if_status(char *ifname, int show_all)
{
	struct ifaddrs *ifaddrs = NULL;
	struct ifaddrs *ifa, *ifn;

	
	if (getifaddrs(&ifaddrs) < 0) {
		printf("%s::getifaddrs() error\n", __func__);
		return -1;
	}

	for (ifa = ifaddrs; ifa != 0; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_LINK) {
			int flags = ifa->ifa_flags;
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
			int type = sdl->sdl_type;
			char addr[20];
			
			/* Check solo shown */
			if (ifname && strcmp(ifname, ifa->ifa_name) != 0)
				continue;
			if (!(flags & IFF_UP) && !ifname && !show_all)
			//if (!ifname && !show_all)
				continue;

			/* Show status */
			printf("%-8s", ifa->ifa_name);

			switch (type) {
			case IFT_ETHER:
				printf("Link encap:Ethernet  ");
				printf("HWaddr %s",
					(char *)ether_ntoa_r((struct ether_addr *)LLADDR(sdl), addr));
				break;
			case IFT_LOOP:
				printf("Link encap:Local Loopback  ");
				break;
			case IFT_PPP:
				printf("Link encap:Point to Point  ");
				break;
			case IFT_PROPVIRTUAL:
				printf("Link encap:Bridge  ");
				break;
			default:
				break;
			}
			printf("\n");

			/* Search for all inet address matched this interface name */
			for (ifn = ifaddrs; ifn != 0; ifn = ifn->ifa_next) {
				if (strcmp(ifn->ifa_name, ifa->ifa_name) == 0 &&
				    ifn->ifa_addr->sa_family == AF_INET) {
					printf("        ");
					printf("inet addr:%s  ",
						inet_ntoa_r(SIN(ifn->ifa_addr)->sin_addr, addr));

					if (flags & IFF_POINTOPOINT) {
						printf("dest addr:%s  ",
						inet_ntoa_r(SIN(ifn->ifa_dstaddr)->sin_addr,
						addr));
					}
					else if (flags & IFF_BROADCAST) {
						printf("Bcast:%s  ",
						inet_ntoa_r(SIN(ifn->ifa_dstaddr)->sin_addr,
							addr));
					}

					if (ifn->ifa_netmask) {
						printf("Mask:%s",
						inet_ntoa_r(SIN(ifn->ifa_netmask)->sin_addr,
						addr));
					}
					printf("\n");
				}
			}

			/* Print flags and other things */
			printf("        ");
			if ((flags & IFF_UP)) printf("UP ");
			if ((flags & IFF_POINTOPOINT)) printf("POINTTOPOINT ");
			if ((flags & IFF_BROADCAST)) printf("BROADCAST ");
			if ((flags & IFF_LOOPBACK)) printf("LOOPBACK ");
			if ((flags & IFF_RUNNING)) printf("RUNNING ");
			if ((flags & IFF_PROMISC)) printf("PROMISC ");
			if ((flags & IFF_MULTICAST)) printf("MULTICAST ");
			if ((flags & IFF_ALLMULTI)) printf("ALLMULTI ");

			printf(" MTU:%ld", ((struct if_data *)ifa->ifa_data)->ifi_mtu);
			printf(" Metric %ld\n", ((struct if_data *)ifa->ifa_data)->ifi_metric);

			/* More statistic value */
			printf("        ");
			printf(" Rx packets:%ld", ((struct if_data *)ifa->ifa_data)->ifi_ipackets);
			printf(" error:%ld", ((struct if_data *)ifa->ifa_data)->ifi_ierrors);
			printf(" drop:%ld", ((struct if_data *)ifa->ifa_data)->ifi_iqdrops);
			printf(" mcast:%ld\n", ((struct if_data *)ifa->ifa_data)->ifi_imcasts);
			printf("        ");
			printf(" Tx packets:%ld", ((struct if_data *)ifa->ifa_data)->ifi_opackets);
			printf(" error:%ld", ((struct if_data *)ifa->ifa_data)->ifi_oerrors);
			printf(" mcast:%ld\n", ((struct if_data *)ifa->ifa_data)->ifi_omcasts);
			printf("        ");
			printf(" Rx bytes:%ld", ((struct if_data *)ifa->ifa_data)->ifi_ibytes);
			printf(" Tx bytes:%ld\n", ((struct if_data *)ifa->ifa_data)->ifi_obytes);
			
			printf("\n");
		}
	}

	freeifaddrs(ifaddrs);
	return 0;
}

int ifconfig_cmd(int argc, char *argv[])
{
	char *ifname = NULL;
	char *address = NULL;
	char *netmask = "255.255.255.0";
	int show_all = 0;
	int flags = IFF_UP;
	
	/* Skip program name */
	--argc;
	++argv;
	if (*argv) {
		/* Parse argument */
		for (; *argv; ++argv) {

			
			if (!strcmp(*argv, "-a")) {
				show_all = 1;
			}
			else if (ifname == NULL) {
				ifname = *argv;
			}
			else if (!strcmp(*argv, "down")) {
				flags = 0;
			}
			else if (address == NULL) {
				address = *argv;
			}
			else if (!strcmp(*argv, "netmask")) {
				if (!address)
					goto usage;
				argv++;
				if (!*argv)
					goto usage;

				netmask = *argv;
			}
			else {
				goto usage;
			}
		}
	}

	if (!address && flags)
		return if_status(ifname, show_all);
	
	/* Config ipaddress */
	if (!ifname)
		goto usage;
	
	//add by z10312 支持cli ifconfig interface down相关接口,以便于bsp调试 16-0125
	if (!flags) 
		return tenda_ifconfig(ifname, flags, 0, 0);

	
	return tenda_ifconfig(ifname, flags, address, netmask); 
 	
usage:
	printf("Usage: ifconfig [-a] [ifname] [address] [netmask mask] [up/down]");
	return -1;
}


extern void cyg_kmem_print_stats(void);

#if 0	//llm remove,冗余代码
/* MBUF and memory show command */
static int
net_mbuf_cmd(int argc, char *argv[])
{
	struct mallinfo mem_info;
	static char *mem_top = 0;

	cyg_kmem_print_stats();

	if (mem_top == NULL)
		mem_top = HAL_MEM_REAL_REGION_TOP(mem_top);

	mem_info = mallinfo();
	printf("\nMemory system:"
		"\n   Total %d bytes"    
		"\n   Heap %d bytes, Free %d bytes, Max %d bytes\n",
		(((int)mem_top) & ~0x80000000),
		mem_info.arena, mem_info.fordblks, mem_info.maxfree);

	return 0;
}
#endif

//roy +++
int net_mbuf_free_mem(void)
{
	struct mallinfo mem_info;

	mem_info = mallinfo();

	return mem_info.fordblks;
}
//


/************************************************************
Function:	 if_TX_RX_status               
Description:  从ifname中获取上传下载数据包的大小

Input:         

Output: 	    

Return:     

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int if_TX_RX_status(char *ifname,u_long * tx,u_long * rx)
{
	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifa, *ifn;

	if (getifaddrs(&ifaddrs) < 0) {
		printf("%s::getifaddrs() error\n", __func__);
		return -1;
	}

	for (ifa = ifaddrs; ifa != 0; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr->sa_family == AF_LINK) 
		{
			int flags = ifa->ifa_flags;

			/* Check solo shown */
			if (ifname && strcmp(ifname, ifa->ifa_name) != 0)
				continue;
			if (!(flags & IFF_UP) && !ifname)
				continue;

			//(*rx) = ((struct if_data *)ifa->ifa_data)->ifi_ipackets;
			//(*tx) = ((struct if_data *)ifa->ifa_data)->ifi_opackets;
			(*rx) = ((struct if_data *)ifa->ifa_data)->ifi_ibytes;
			(*tx) = ((struct if_data *)ifa->ifa_data)->ifi_obytes;
		}
	}

	freeifaddrs(ifaddrs);
	return 0;
}

