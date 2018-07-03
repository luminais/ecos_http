/*
*/

/* System include files */
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <netinet/icmp6.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>

#include <stddef.h>		/* offsetof */
#include <net/if_arp.h>
//#include <linux/if_ether.h>
#include "apmib.h"
#include "mibtbl.h"
//#include "sysconf.h"
#include "sys_utility.h"

// system/sysconf.h
#define DHCP6S_PID_FILE "/var/run/dhcp6s.pid"
#define DHCP6S_CONF_FILE "/var/dhcp6s.conf"
#define DHCP6C_PID_FILE "/var/run/dhcp6c.pid"
#define DHCP6C_CONF_FILE "/var/dhcp6c.conf"
#define DNSV6_PID_FILE "/var/run/dnsmasq.pid"
#define DNRD_PID_FILE "/var/run/dnrd.pid"
#define DNSV6_CONF_FILE "/var/dnsmasq.conf"
#define DNSV6_RESOLV_FILE "/var/dnsmasq_resolv.conf"
#define RESOLV_CONF_FILE "/etc/resolv.conf"
#define RADVD_CONF_FILE "/etc/radvd.conf"
#define RADVD_PID_FILE "/var/run/radvd.pid"
#define ECMH_PID_FILE   "/var/run/ecmh.pid"
#define DHCP6C_DUID_FILE  "/var/dhcpv6_duid"
#define IPV6_ROUTE_PROC "/proc/net/ipv6_route"
#define IPV6_ADDR_PROC "/proc/net/if_inet6"
#define IPV6_GATEWAY_FILE   "/var/gateway_ipv6"
#define DHCP6PD_CONF_FILE "/var/dhcp6pd.conf"
#define DNSV6_ADDR_FILE "/var/dns6.conf"
//mldproxy
#define PROC_BR_MLDPROXY "/proc/br_mldProxy"
#define PROC_BR_MLDSNOOP "/proc/br_mldsnoop"
#define PROC_BR_MLDVERSION "/proc/br_mldVersion"
#define PROC_BR_MLDQUERY "/proc/br_mldquery"
//#define PROC_MLD_MAX_MEMBERS "/proc/sys/net/ipv6/mld_max_memberships"
#define MLDPROXY_PID_FILE "/var/run/mld_pid"

// TODO: dhcp v6
//void set_dhcp6s(void);
// TODO: dns v6
//void set_dnsv6(void);
//void set_radvd(void);
// TODO: pimv6
//void set_ecmh(void);
// TODO: dhcp v6
//void set_dhcp6c(void);
// TODO: ipv6 wan
//void set_wanv6(void);
//void set_lanv6(void);
void set_basicv6(void);

void set_ipv6(void);

#if 0
struct duid_t{
	uint16 duid_type;
	uint16 hw_type;
	uint8 mac[6];		
};

int checkDnsAddrIsExist(char *dnsAddr, char * dnsFileName)
{
	char  line_buf[128];		
	FILE *fp=NULL;
	if((fp=fopen(dnsFileName, "r"))==NULL)
	{
//		printf("Open file : %s fails!\n",dnsFileName);
		return 0;
	}
	while(fgets(line_buf, 128, fp)) 
	{			
		if(strstr(line_buf, dnsAddr)!=NULL)
		{
			fclose(fp);
			return 1;
		}			
	}
	fclose(fp);
	return 0;
}
void set_dhcp6s()
{
	dhcp6sCfgParam_t dhcp6sCfgParam;
	char tmpStr[256];
	int fh;
	int pid=-1;
	
	if ( !apmib_get(MIB_IPV6_DHCPV6S_PARAM,(void *)&dhcp6sCfgParam)){
		printf("get MIB_IPV6_DHCPV6S_PARAM failed\n");
		return;  
	}
	
	if(!dhcp6sCfgParam.enabled){
		return;
	}
	
	if(isFileExist(DHCP6S_CONF_FILE) == 0) {
		/*create config file*/
		fh = open(DHCP6S_CONF_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
		if (fh < 0) {
			fprintf(stderr, "Create %s file error!\n", DHCP6S_CONF_FILE);
			return;
		}
		printf("create dhcp6s.conf\n");
		
		sprintf(tmpStr, "option domain-name-servers %s;\n", dhcp6sCfgParam.DNSaddr6);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "interface %s {\n", dhcp6sCfgParam.interfaceNameds);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "  address-pool pool1 3600;\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "};\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "pool pool1 {\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "  range %s to %s ;\n", dhcp6sCfgParam.addr6PoolS, dhcp6sCfgParam.addr6PoolE);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "};\n");
		write(fh, tmpStr, strlen(tmpStr));

		close(fh);
	}

	/*start daemon*/
	if(isFileExist(DHCP6S_PID_FILE)) {
		pid=getPid_fromFile(DHCP6S_PID_FILE);
		if(dhcp6sCfgParam.enabled == 1){
			sprintf(tmpStr, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			unlink(DHCP6S_PID_FILE);
			RunSystemCmd(NULL_FILE, "/bin/dhcp6s", dhcp6sCfgParam.interfaceNameds, NULL_STR);
		}
		else 
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			
	}else {
		if(dhcp6sCfgParam.enabled == 1)
			RunSystemCmd(NULL_FILE, "/bin/dhcp6s", dhcp6sCfgParam.interfaceNameds, NULL_STR);
	}
		
	return;
}

void set_dnsv6()
{
	dnsv6CfgParam_t dnsCfgParam;
	int pid = -1;
	int fh, dnsMode;
	char tmpStr[128];
	char tmpBuff[32];
	char tmpChar;
	char addr[256];
	char cmdBuffer[256];
	char dns[64], dnstmp[64];
	
	char def_wan_ifname[]="eth1";
	
//	char hostfile[]="/var/dnsmasq_hostfile";
	FILE *fp=NULL;

	system("rm -f /var/dnsmasq.conf 2> /dev/null");		
	system("rm -f /var/dnsmasq_resolv.conf 2> /dev/null");
//	system("rm -f /var/dnsmasq_hostfile 2> /dev/null");
	if ( !apmib_get(MIB_IPV6_DNSV6_PARAM,(void *)&dnsCfgParam))
	{
		printf("get MIB_IPV6_DNSV6_PARAM failed\n");
		return;  
	}
	
	if(!isFileExist(DNSV6_CONF_FILE))
	{
		/*create config file*/
		fh = open(DNSV6_CONF_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
		if (fh < 0) 
		{
			fprintf(stderr, "Create %s file error!\n", DNSV6_CONF_FILE);
			return;
		}

		printf("create dnsmasq.conf\n");

		apmib_get(MIB_ELAN_MAC_ADDR,  (void *)tmpBuff);
		if(!memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", 6))
		apmib_get(MIB_HW_NIC0_ADDR,  (void *)tmpBuff);
		sprintf(cmdBuffer, "%02x%02x%02x%02x%02x%02x", (unsigned char)tmpBuff[0], (unsigned char)tmpBuff[1], 
		(unsigned char)tmpBuff[2], (unsigned char)tmpBuff[3], (unsigned char)tmpBuff[4], (unsigned char)tmpBuff[5]);

		tmpChar=cmdBuffer[1];

		switch(tmpChar) 
		{
			case '0':
			case '1':
			case '4':
			case '5':
			case '8':
			case '9':
			case 'c':
			case 'd':
			tmpChar = (char)((int)tmpChar+2);
			break;
			default:
			break;
		}
		sprintf(addr, "Fe80::%c%c%c%c:%c%cFF:FE%c%c:%c%c%c%c", 
		cmdBuffer[0], tmpChar, cmdBuffer[2], cmdBuffer[3],
		cmdBuffer[4], cmdBuffer[5],
		cmdBuffer[6], cmdBuffer[7],
		cmdBuffer[8],cmdBuffer[9],cmdBuffer[10],cmdBuffer[11]
		);

		sprintf(tmpStr, "domain-needed\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "bogus-priv\n");
		write(fh, tmpStr, strlen(tmpStr));	
//		sprintf(tmpStr, "addn-hosts=%s\n", hostfile);
//		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "address=/%s/%s\n",dnsCfgParam.routerName,addr);
		write(fh, tmpStr, strlen(tmpStr));

//		close(fh);	
	}	
	
	if(!apmib_get(MIB_IPV6_DNS_AUTO,  (void *)&dnsMode))
	{
		printf("get MIB_IPV6_DNS_AUTO failed\n");
		return; 
	}	
	if(dnsMode==0)  //Set DNS Manually 
	{
		addr6CfgParam_t addr6_dns;
		
		if(!apmib_get(MIB_IPV6_ADDR_DNS_PARAM,  (void *)&addr6_dns))
		{
			printf("get MIB_IPV6_ADDR_DNS_PARAM failed\n");
			return;
		}
		snprintf(dns, sizeof(dns), "nameserver %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", 
		addr6_dns.addrIPv6[0], addr6_dns.addrIPv6[1], addr6_dns.addrIPv6[2], addr6_dns.addrIPv6[3], 
		addr6_dns.addrIPv6[4], addr6_dns.addrIPv6[5], addr6_dns.addrIPv6[6], addr6_dns.addrIPv6[7]);

		if(strstr(dns, "0000:0000:0000:0000:0000:0000:0000:0000")==NULL)	
		{
			if(isFileExist(DNSV6_RESOLV_FILE))
			{
				if(!checkDnsAddrIsExist(dns, DNSV6_RESOLV_FILE))
					write_line_to_file(DNSV6_RESOLV_FILE, 2, dns);
			}
			else
				write_line_to_file(DNSV6_RESOLV_FILE, 1, dns);
		}			
		if(isFileExist(DNSV6_RESOLV_FILE))
		{			
			sprintf(tmpStr, "resolv-file=%s\n", DNSV6_RESOLV_FILE);
			write(fh, tmpStr, strlen(tmpStr));
		}	
	}
	else  //Attain DNS Automatically 
	{
		sprintf(tmpStr, "resolv-file=%s\n", DNSV6_ADDR_FILE);
		write(fh, tmpStr, strlen(tmpStr));
	}
	
	close(fh);		
//	system("echo fe80::287:42ff:fe16:9541 rlx.realsil >>/var/dnsmasq_hostfile");
	if(isFileExist(DNSV6_PID_FILE)) 
	{
		pid=getPid_fromFile(DNSV6_PID_FILE);
		if(pid>0)
		{
			sprintf(tmpStr, "kill -9 %d", pid);
			system(tmpStr);
			unlink(DNSV6_PID_FILE);
		}
		if(dnsCfgParam.enabled == 1) 
		{
#if 0
			if(isFileExist(DNRD_PID_FILE)){
			pid=getPid_fromFile(DNRD_PID_FILE);
			sprintf(tmpStr, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			unlink(DNRD_PID_FILE);				
			}
#endif
			sprintf(tmpStr, "dnsmasq -C /var/dnsmasq.conf -O %s", def_wan_ifname);
			system(tmpStr);
		}
	} 
	else
	{
		if(dnsCfgParam.enabled == 1) 
		{
#if 0
			if(isFileExist(DNRD_PID_FILE)) {
			pid=getPid_fromFile(DNRD_PID_FILE);
			sprintf(tmpStr, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			unlink(DNRD_PID_FILE);			
			}
#endif
			sprintf(tmpStr, "dnsmasq -C /var/dnsmasq.conf -O %s", def_wan_ifname);
			system(tmpStr);
		}
	}
	return;
}
#endif
#if 0
void set_radvd()
{
	radvdCfgParam_t radvdCfgParam;
	int fh;
	char tmpStr[256];
	char tmpBuf[256];
	unsigned short tmpNum[8];
	int dnsMode;
	FILE *fp=NULL;

	if ( !apmib_get(MIB_IPV6_RADVD_PARAM,(void *)&radvdCfgParam)){
		printf("get MIB_IPV6_RADVD_PARAM failed\n");
		return;  
	}

	if(!isFileExist(RADVD_CONF_FILE)){
		/*create config file*/
		fh = open(RADVD_CONF_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
		if (fh < 0) {
			fprintf(stderr, "Create %s file error!\n", RADVD_CONF_FILE);
			return;
		}
		printf("create radvd.conf\n");
		sprintf(tmpStr, "interface %s\n", radvdCfgParam.interface.Name);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "{\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvSendAdvert on;\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "MaxRtrAdvInterval %d;\n", radvdCfgParam.interface.MaxRtrAdvInterval);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "MinRtrAdvInterval %d;\n", radvdCfgParam.interface.MinRtrAdvInterval);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "MinDelayBetweenRAs %d;\n", radvdCfgParam.interface.MinDelayBetweenRAs);
		write(fh, tmpStr, strlen(tmpStr));
		if(radvdCfgParam.interface.AdvManagedFlag > 0) {
			sprintf(tmpStr, "AdvManagedFlag on;\n");
			write(fh, tmpStr, strlen(tmpStr));			
		}
		if(radvdCfgParam.interface.AdvOtherConfigFlag > 0){
			sprintf(tmpStr, "AdvOtherConfigFlag on;\n");
			write(fh, tmpStr, strlen(tmpStr));	
		}
		sprintf(tmpStr, "AdvLinkMTU %d;\n", radvdCfgParam.interface.AdvLinkMTU);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvReachableTime %u;\n", radvdCfgParam.interface.AdvReachableTime);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvRetransTimer %u;\n", radvdCfgParam.interface.AdvRetransTimer);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvCurHopLimit %d;\n", radvdCfgParam.interface.AdvCurHopLimit);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvDefaultLifetime %d;\n", radvdCfgParam.interface.AdvDefaultLifetime);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvDefaultPreference %s;\n", radvdCfgParam.interface.AdvDefaultPreference);
		write(fh, tmpStr, strlen(tmpStr));
		if(radvdCfgParam.interface.AdvSourceLLAddress > 0) {
			sprintf(tmpStr, "AdvSourceLLAddress on;\n");
			write(fh, tmpStr, strlen(tmpStr));			
		}		
		if(radvdCfgParam.interface.UnicastOnly > 0){
			sprintf(tmpStr, "UnicastOnly on;\n");
			write(fh, tmpStr, strlen(tmpStr));	
		}
		

		/*prefix 1*/
		if(radvdCfgParam.interface.prefix[0].enabled > 0){
			memcpy(tmpNum,radvdCfgParam.interface.prefix[0].Prefix, sizeof(radvdCfgParam.interface.prefix[0].Prefix));
			sprintf(tmpBuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", tmpNum[0], tmpNum[1], 
				tmpNum[2], tmpNum[3], tmpNum[4], tmpNum[5],tmpNum[6],tmpNum[7]);
			strcat(tmpBuf, "\0");
			sprintf(tmpStr, "prefix %s/%d\n", tmpBuf, radvdCfgParam.interface.prefix[0].PrefixLen);			
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "{\n");
			write(fh, tmpStr, strlen(tmpStr));
			if(radvdCfgParam.interface.prefix[0].AdvOnLinkFlag > 0){
				sprintf(tmpStr, "AdvOnLink on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			if(radvdCfgParam.interface.prefix[0].AdvAutonomousFlag > 0){
				sprintf(tmpStr, "AdvAutonomous on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			sprintf(tmpStr, "AdvValidLifetime %u;\n", radvdCfgParam.interface.prefix[0].AdvValidLifetime);
			write(fh, tmpStr, strlen(tmpStr));					
			sprintf(tmpStr, "AdvPreferredLifetime %u;\n", radvdCfgParam.interface.prefix[0].AdvPreferredLifetime);
			write(fh, tmpStr, strlen(tmpStr));	
			if(radvdCfgParam.interface.prefix[0].AdvRouterAddr > 0){
				sprintf(tmpStr, "AdvRouterAddr on;\n");
				write(fh, tmpStr, strlen(tmpStr));						
			}
			if(radvdCfgParam.interface.prefix[0].if6to4[0]){
				sprintf(tmpStr, "Base6to4Interface %s;\n", radvdCfgParam.interface.prefix[0].if6to4);
				write(fh, tmpStr, strlen(tmpStr));						
			}
			sprintf(tmpStr, "};\n");
			write(fh, tmpStr, strlen(tmpStr));						
		}

		/*prefix 2*/
		if(radvdCfgParam.interface.prefix[1].enabled > 0){
			memcpy(tmpNum,radvdCfgParam.interface.prefix[1].Prefix, sizeof(radvdCfgParam.interface.prefix[1].Prefix));
			sprintf(tmpBuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", tmpNum[0], tmpNum[1], 
				tmpNum[2], tmpNum[3], tmpNum[4], tmpNum[5],tmpNum[6],tmpNum[7]);
			strcat(tmpBuf, "\0");
			sprintf(tmpStr, "prefix %s/%d\n", tmpBuf, radvdCfgParam.interface.prefix[1].PrefixLen);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "{\n");
			write(fh, tmpStr, strlen(tmpStr));
			if(radvdCfgParam.interface.prefix[1].AdvOnLinkFlag > 0){
				sprintf(tmpStr, "AdvOnLink on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			if(radvdCfgParam.interface.prefix[1].AdvAutonomousFlag > 0){
				sprintf(tmpStr, "AdvAutonomous on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			sprintf(tmpStr, "AdvValidLifetime %u;\n", radvdCfgParam.interface.prefix[1].AdvValidLifetime);
			write(fh, tmpStr, strlen(tmpStr));					
			sprintf(tmpStr, "AdvPreferredLifetime %u;\n", radvdCfgParam.interface.prefix[1].AdvPreferredLifetime);
			write(fh, tmpStr, strlen(tmpStr));	
			if(radvdCfgParam.interface.prefix[1].AdvRouterAddr > 0){
				sprintf(tmpStr, "AdvRouterAddr on;\n");
				write(fh, tmpStr, strlen(tmpStr));						
			}
			if(radvdCfgParam.interface.prefix[1].if6to4[0]){
				sprintf(tmpStr, "Base6to4Interface %s;\n", radvdCfgParam.interface.prefix[1].if6to4);
				write(fh, tmpStr, strlen(tmpStr));						
			}
			sprintf(tmpStr, "};\n");
			write(fh, tmpStr, strlen(tmpStr));						
		}
#ifdef SUPPORT_RDNSS_OPTION
		//add RDNSS 
		apmib_get(MIB_IPV6_DNS_AUTO, (void *)&dnsMode);		
		if(dnsMode==0)  //Set DNS Manually 
		{
			addr6CfgParam_t addr6_dns;
			
			apmib_get(MIB_IPV6_ADDR_DNS_PARAM,  (void *)&addr6_dns);
				
			snprintf(tmpBuf, sizeof(tmpBuf), "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", 
			addr6_dns.addrIPv6[0], addr6_dns.addrIPv6[1], addr6_dns.addrIPv6[2], addr6_dns.addrIPv6[3], 
			addr6_dns.addrIPv6[4], addr6_dns.addrIPv6[5], addr6_dns.addrIPv6[6], addr6_dns.addrIPv6[7]);

			//strcpy(tmpBuf, "2001:0000:0000:0000:0000:0000:0000:0045");
			if(strstr(tmpBuf, "0000:0000:0000:0000:0000:0000:0000:0000")==NULL)	
			{
				//add RDNSS 
				sprintf(tmpStr, "RDNSS %s\n",tmpBuf);
				write(fh, tmpStr, strlen(tmpStr));
				sprintf(tmpStr, "{\n");
				write(fh, tmpStr, strlen(tmpStr));
				sprintf(tmpStr, "AdvRDNSSLifetime %d;\n", radvdCfgParam.interface.MaxRtrAdvInterval);
				write(fh, tmpStr, strlen(tmpStr));
				sprintf(tmpStr, "};\n");
				write(fh, tmpStr, strlen(tmpStr));
			}			
		}
		else
		{
			if(isFileExist(DNSV6_ADDR_FILE))
			{
				if((fp=fopen("/var/dns6.conf","r"))!=NULL)
				{				
					memset(tmpStr, 0, sizeof(tmpStr));
					while(fgets(tmpBuf, sizeof(tmpBuf), fp))
					{
						tmpBuf[strlen(tmpBuf)-1]=0;
						strcat(tmpStr, tmpBuf+strlen("nameserver")+1);
						strcat(tmpStr, " ");					
					}
					if(strlen(tmpStr>1)==0)
					{
						tmpStr[strlen(tmpStr)-1]=0;				
						sprintf(tmpBuf, "RDNSS %s\n",tmpStr);
						write(fh, tmpBuf, strlen(tmpBuf));
						sprintf(tmpBuf, "{\n");
						write(fh, tmpBuf, strlen(tmpStr));
						sprintf(tmpBuf, "AdvRDNSSLifetime %d;\n", radvdCfgParam.interface.MaxRtrAdvInterval);
						write(fh, tmpBuf, strlen(tmpBuf));
						sprintf(tmpBuf, "};\n");
						write(fh, tmpBuf, strlen(tmpBuf));	
					}
					fclose(fp);	
				}
			}
		}
#endif

#ifdef SUPPORT_DNSSL_OPTION
		//add DNSSL
		memset(tmpBuf, 0, sizeof(tmpBuf));
		apmib_get(MIB_DOMAIN_NAME, (void *)tmpBuf);	
		if(strlen(tmpBuf)>0)
		{
			sprintf(tmpStr, "DNSSL %s.com %s.com.cn\n", tmpBuf, tmpBuf);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "{\n");
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "AdvDNSSLLifetime %d;\n", radvdCfgParam.interface.MaxRtrAdvInterval);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "};\n");
			write(fh, tmpStr, strlen(tmpStr));		
		}
#endif

		sprintf(tmpStr, "};\n");
		write(fh, tmpStr, strlen(tmpStr));		
		close(fh);		
	}
	
	if(isFileExist(RADVD_PID_FILE)){
		if(radvdCfgParam.enabled == 1) {
			system("killall radvd 2> /dev/null");			
			system("rm -f /var/run/radvd.pid 2> /dev/null");		
			unlink(DNRD_PID_FILE);						
			system("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
			system("radvd -C /var/radvd.conf");
				
		} else {	
			system("killall radvd 2> /dev/null");		
			system("rm -f /var/run/radvd.pid 2> /dev/null");			
		}
	} else{
		if(radvdCfgParam.enabled == 1) {
			system("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
			system("radvd -C /var/radvd.conf");		
		}		
	}
	
	return;
}
#endif
#if 0
void set_ecmh()
{	
		
	if(isFileExist(ECMH_PID_FILE)){
		system("killall ecmh 2> /dev/null");		
	}
	
	system("ecmh");		
	
	return;
}

void start_mldproxy(char *wan_iface, char *lan_iface)
{
	int intValue=0;
	int opmode=-1;
	apmib_get(MIB_MLD_PROXY_DISABLED, (void *)&intValue);
	RunSystemCmd(NULL_FILE, "killall", "-9", "mldproxy", NULL_STR);
	apmib_get(MIB_OP_MODE,(void *)&opmode);
	
	if(intValue==0) {
		//RunSystemCmd(NULL_FILE, "mldproxy", wan_iface, lan_iface, NULL_STR);
		if(opmode==GATEWAY_MODE){
			RunSystemCmd(NULL_FILE, "mldproxy", wan_iface, lan_iface, NULL_STR);
		}	
		else if(opmode==WISP_MODE){
			//RunSystemCmd(NULL_FILE, "mldproxy", "wlan0", lan_iface, NULL_STR);
		}	
		else if(opmode==BRIDGE_MODE){
		}
				
	}
	
}
#endif
int rtr_accept_ra(char* ifname,int enable)
{
	int s;
	struct in6_ndireq nd;

	if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
		printf("socket fail\n");
		return -1;
		/* NOTREACHED */
	}
	bzero(&nd, sizeof(nd));
	strncpy(nd.ifname, ifname, sizeof(nd.ifname));
	if (ioctl(s, SIOCGIFINFO_IN6, (caddr_t)&nd) < 0) {
		printf("ioctl(SIOCGIFINFO_IN6) fail\n");
		close(s);
		return -1;
		/* NOTREACHED */
	}
//diag_printf("%s:%d flags=%x\n",__FUNCTION__,__LINE__,nd.ndi.flags);
	if(enable)
		nd.ndi.flags |=ND6_IFF_ACCEPT_RTADV;
	else
		nd.ndi.flags &=~(ND6_IFF_ACCEPT_RTADV);
	//diag_printf("%s:%d flags=%x\n",__FUNCTION__,__LINE__,nd.ndi.flags);

	if (ioctl(s, SIOCSIFINFO_FLAGS, (caddr_t)&nd) < 0) 
		printf("ioctl(SIOCSRTRFLUSH_IN6) fail!\n");

	close(s);
}

void set_basicv6() 
{
	addrIPv6CfgParam_t addrIPv6CfgParam;
	char tmpStr[256];
	#ifdef __ECOS
	int i, B_len, b_len;
	unsigned int mask;
	#endif
	
	if ( !apmib_get(MIB_IPV6_ADDR_PARAM,(void *)&addrIPv6CfgParam)){
		printf("get MIB_IPV6_ADDR_PARAM failed\n");
		return;        
	}
	#ifdef __ECOS
	//if(addrIPv6CfgParam.enabled == 1) 
	{

		/*	
		 * IPv6 Address for LAN 
		 */
		 if(addrIPv6CfgParam.addrIPv6[0][0] != 0)
		 {
			memset(tmpStr,0,sizeof(tmpStr));
			sprintf(tmpStr,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d", 
							addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],
							addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
							addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],
							addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
							addrIPv6CfgParam.prefix_len[0]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_LAN_INTERFACE, "inet6", tmpStr, NULL_STR);

			//lan should not accept ra
			rtr_accept_ra(_IPV6_LAN_INTERFACE,0);
		 }
		//printf("the cmd for ipv6 is %s\n", tmpStr);
		
#ifdef HAVE_RADVD_SHELL 
		/* Address for LAN Anycast */
		B_len = addrIPv6CfgParam.prefix_len[0]/16;
		b_len = addrIPv6CfgParam.prefix_len[0]%16;

		mask = 0;
		for (i=0;i<b_len;i++) mask |= 1 << 15-i;

		memset(tmpStr,0,sizeof(tmpStr));
		for (i=0;i<B_len;i++)
			sprintf(tmpStr + strlen(tmpStr),"%04x:",addrIPv6CfgParam.addrIPv6[0][i]);

		if (mask) 	sprintf(tmpStr + strlen(tmpStr),"%04x",addrIPv6CfgParam.addrIPv6[0][i] & mask);
		else 		tmpStr[strlen(tmpStr)-1]=0;  /* Remove ':' */

		if(B_len < 8) sprintf(tmpStr + strlen(tmpStr),"::");

		sprintf(tmpStr+strlen(tmpStr),"\/%d",addrIPv6CfgParam.prefix_len[0]);
		RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_LAN_INTERFACE, "inet6", tmpStr, "anycast", NULL_STR);
		//printf("\nB_len=%d, b_len=%d, make=0x%04x\n",B_len, b_len, mask);
		//printf("ipv6 anycast %s\n", tmpStr);
#endif
		
		/*	
		 * IPv6 Address for WAN 
		 */
		 if(addrIPv6CfgParam.addrIPv6[1][0] != 0)
		 {
			memset(tmpStr,0,sizeof(tmpStr));
			sprintf(tmpStr,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d", 
							addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],
							addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
							addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],
							addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
							addrIPv6CfgParam.prefix_len[1]);
			RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_WAN_INTERFACE, "inet6", tmpStr, NULL_STR);
			//printf("the cmd for ipv6 is %s\n", tmpStr);
			rtr_accept_ra(_IPV6_WAN_INTERFACE,1);
		 }

#if 0
		/* Address for WAN Anycast */
		B_len = addrIPv6CfgParam.prefix_len[1]/16;
		b_len = addrIPv6CfgParam.prefix_len[1]%16;

		mask = 0;
		for (i=0;i<b_len;i++) mask |= 1 << 15-i;

		memset(tmpStr,0,sizeof(tmpStr));
		for (i=0;i<B_len;i++)
			sprintf(tmpStr + strlen(tmpStr),"%04x:",addrIPv6CfgParam.addrIPv6[1][i]);

		if (mask) 	sprintf(tmpStr + strlen(tmpStr),"%04x",addrIPv6CfgParam.addrIPv6[1][i] & mask);
		else 		tmpStr[strlen(tmpStr)-1]=0; /* Remove ':' */

		if(B_len < 8) sprintf(tmpStr + strlen(tmpStr),"::");

		sprintf(tmpStr+strlen(tmpStr),"\/%d",addrIPv6CfgParam.prefix_len[1]);
		RunSystemCmd(NULL_FILE, "ifconfig", _IPV6_WAN_INTERFACE, "inet6", tmpStr, "anycast", NULL_STR);
		//printf("\nB_len=%d, b_len=%d, make=0x%04x\n",B_len, b_len, mask);
		//printf("ipv6 anycast %s\n", tmpStr);
#endif
	}
	#else
	//if(addrIPv6CfgParam.enabled == 1) 
	{
		/*
		/bin/ifconfig br0 $ADDR1/$PREFIX1
        /bin/ifconfig eth1 $ADDR2/$PREFIX2
        */
		sprintf(tmpStr,"/bin/ifconfig br0 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
			addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],
			addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
			addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],
			addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
			addrIPv6CfgParam.prefix_len[0]);
		//RunSystemCmd(NULL_FILE, tmpStr, NULL_STR);
		system(tmpStr);
		//printf("the cmd for ipv6 is %s\n", tmpStr);
		
		sprintf(tmpStr,"/bin/ifconfig eth1 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
			addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],
			addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
			addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],
			addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
			addrIPv6CfgParam.prefix_len[1]);
		//RunSystemCmd(NULL_FILE, tmpStr, NULL_STR);
		system(tmpStr);
		//printf("the cmd for ipv6 is %s\n", tmpStr);
	}
	#endif
}

#if 0
void set_dhcp6c()
{
		
		char tmpStr[256];
		int fh;
		int val;
		FILE *fp = NULL;
		int pid=-1;
		struct duid_t dhcp6c_duid;
		uint16 len;
		char filename[64];
		char pidname[64];
		dhcp6cCfgParam_t dhcp6cCfgParam;
 		/*for test use fixed duid of 0003000100e04c8196c9*/
		if(isFileExist(DHCP6C_DUID_FILE) == 0){
			/*create config file*/
			fp=fopen(DHCP6C_DUID_FILE,"w+");
			if(fp==NULL){
				fprintf(stderr, "Create %s file error!\n", DHCP6C_DUID_FILE);
				return;
			}
			
			dhcp6c_duid.duid_type=3;
			dhcp6c_duid.hw_type=1;
			dhcp6c_duid.mac[0]=0x00;
			dhcp6c_duid.mac[1]=0xe0;
			dhcp6c_duid.mac[2]=0x4c;
			dhcp6c_duid.mac[3]=0x81;
			dhcp6c_duid.mac[4]=0x96;
			dhcp6c_duid.mac[5]=0xc9;
			len=sizeof(dhcp6c_duid);
			if ((fwrite(&len, sizeof(len), 1, fp)) != 1) {
				fprintf(stderr, "write %s file error!\n", DHCP6C_DUID_FILE);
			}
			else if(fwrite(&dhcp6c_duid,sizeof(dhcp6c_duid),1,fp)!=1)
				fprintf(stderr, "write %s file error!\n", DHCP6C_DUID_FILE);
			
			fclose(fp);
		}

		if ( !apmib_get(MIB_IPV6_DHCPV6C_PARAM,(void *)&dhcp6cCfgParam)){
			fprintf(stderr, "get mib %d error!\n", MIB_IPV6_DHCPV6C_PARAM);
			return;
		}	

		if(dhcp6cCfgParam.enabled){
			sprintf(filename,DHCP6C_CONF_FILE);
			if(isFileExist(filename) == 0)
			/*create config file*/
				fh = open(filename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
			else
				fh = open(filename, O_RDWR|O_TRUNC, S_IRWXO|S_IRWXG);	
		
			if (fh < 0){
				fprintf(stderr, "Create %s file error!\n", filename);
				return;
			}

			if(!apmib_get(MIB_IPV6_LINK_TYPE,&val)){	
				fprintf(stderr, "get mib %d error!\n", MIB_IPV6_LINK_TYPE);
				close(fh);
				return;			
			}
				
			if(val == IPV6_LINKTYPE_IP)
				sprintf(dhcp6cCfgParam.ifName,"eth1");
			else
				sprintf(dhcp6cCfgParam.ifName,"ppp0");
					
			sprintf(tmpStr, "interface %s {\n",dhcp6cCfgParam.ifName);				
			write(fh, tmpStr, strlen(tmpStr));
					
			sprintf(tmpStr, "	send ia-pd %d;\n",100);
			write(fh, tmpStr, strlen(tmpStr));
				
			sprintf(tmpStr, "	send ia-na %d;\n",101);
			write(fh, tmpStr, strlen(tmpStr));
				
			/*dns*/
			sprintf(tmpStr, "	request domain-name-servers;\n",101);
			write(fh, tmpStr, strlen(tmpStr));
					
			sprintf(tmpStr, "};\n\n");
			write(fh, tmpStr, strlen(tmpStr));

			sprintf(tmpStr, "id-assoc pd %d {\n",100);
			write(fh, tmpStr, strlen(tmpStr));					
			sprintf(tmpStr, "		prefix-interface br0 {\n");
			write(fh, tmpStr, strlen(tmpStr));				
			sprintf(tmpStr, "			sla-id %d;\n",dhcp6cCfgParam.dhcp6pd.sla_id);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "			sla-len %d;\n",dhcp6cCfgParam.dhcp6pd.sla_len);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "		};\n");
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "};\n\n");
			write(fh, tmpStr, strlen(tmpStr));					
				
			/*ia-na*/
			sprintf(tmpStr, "id-assoc na %d {\n",101);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "};\n\n");
			write(fh, tmpStr, strlen(tmpStr));	

			close(fh);
		}

		sprintf(pidname,DHCP6C_PID_FILE);
		if(isFileExist(pidname)){
			pid=getPid_fromFile(pidname);
			if(pid>0){
				sprintf(tmpStr, "%d", pid);
				RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			}
			unlink(pidname);
		}	
		/*start daemon*/
		sprintf(tmpStr, "dhcp6c -c %s -p %s %s ", DHCP6C_CONF_FILE,DHCP6C_PID_FILE,dhcp6cCfgParam.ifName);
		/*Use system() instead of RunSystemCmd() to avoid stderr closing, 
		process itself will redirect stderr when it wants to run as deamon() */
		system(tmpStr);
						
		return;
}

void set_wanv6()
{
		char tmpStr[256];
		char gateway[64];
		addr6CfgParam_t	addr6_wan;
		addr6CfgParam_t addr6_gw;
		int val;
		
		if(!apmib_get(MIB_IPV6_LINK_TYPE,&val)){	
			fprintf(stderr, "get mib %d error!\n", MIB_IPV6_LINK_TYPE);
			return;			
		}

		if(val == IPV6_LINKTYPE_PPP)
			return;

		/*disable proc of forwarding to enable RA process in kernel*/
		sprintf(tmpStr,"echo 0 > /proc/sys/net/ipv6/conf/eth1/forwarding 2> /dev/null");
		system(tmpStr);			

		if(!apmib_get(MIB_IPV6_ORIGIN_TYPE,&val)){	
			fprintf(stderr, "get mib %d error!\n", MIB_IPV6_ORIGIN_TYPE);
			return;			
		}

		switch(val){
			case IPV6_ORIGIN_AUTO:
				break;
	
			case IPV6_ORIGIN_DHCP:
				set_dhcp6c();									
				break;
	
			case IPV6_ORIGIN_STATIC:
				/*ifconfig ipv6 address*/
				if ( !apmib_get(MIB_IPV6_ADDR_WAN_PARAM,(void *)&addr6_wan)){
					fprintf(stderr, "get mib %d error!\n", MIB_IPV6_ADDR_WAN_PARAM);
					return ;        
				}

				sprintf(tmpStr,"ifconfig eth1 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
					addr6_wan.addrIPv6[0],addr6_wan.addrIPv6[1],addr6_wan.addrIPv6[2],
					addr6_wan.addrIPv6[3],addr6_wan.addrIPv6[4],addr6_wan.addrIPv6[5],
					addr6_wan.addrIPv6[6],addr6_wan.addrIPv6[7],addr6_wan.prefix_len);
				system(tmpStr);

				if ( !apmib_get(MIB_IPV6_ADDR_GW_PARAM,(void *)&addr6_gw)){
					fprintf(stderr, "get mib %d error!\n", MIB_IPV6_ADDR_GW_PARAM);
					return ;
				}
				/*route -A inet6 add 3ffe:501:ffff::/64 gw fe80::0200:00ff:fe00:a0a0 dev br0*/
				sprintf(tmpStr,"route -A inet6 del default dev eth1 2> /dev/null");
				system(tmpStr);

				sprintf(gateway,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
					addr6_gw.addrIPv6[0],addr6_gw.addrIPv6[1],addr6_gw.addrIPv6[2],
					addr6_gw.addrIPv6[3],addr6_gw.addrIPv6[4],addr6_gw.addrIPv6[5],
					addr6_gw.addrIPv6[6],addr6_gw.addrIPv6[7]);
				sprintf(tmpStr,"route -A inet6 add default gw %s dev eth1",gateway);
				system(tmpStr);
				break;
				
			default:
				break;
		}	

		return;			
	
}

void set_lanv6()
{
	addr6CfgParam_t addr6;
	char tmpBuf[128];
	
	if ( !apmib_get(MIB_IPV6_ADDR_LAN_PARAM,&addr6)){
		fprintf(stderr, "get mib %d error!\n", MIB_IPV6_ADDR_LAN_PARAM);
		return ;        
	}
	sprintf(tmpBuf,"ifconfig br0 add %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d 2> /dev/null",
					addr6.addrIPv6[0],addr6.addrIPv6[1],addr6.addrIPv6[2],
					addr6.addrIPv6[3],addr6.addrIPv6[4],addr6.addrIPv6[5],
					addr6.addrIPv6[6],addr6.addrIPv6[7],addr6.prefix_len);
	system(tmpBuf);
	return;
}
#endif

#ifdef HAVE_RTL_DNSMASQ
#define RTL_DNSMASQ_CONFILE "/etc/dnsmasq.conf"
#define RTL_DNSMASQ_RESOLVFILE "/etc/dnsv6_resolv.conf"

void create_rtl_dnsmasq_config_file()
{
	unsigned char tmpBuff[32];
	unsigned char tmpChar;
	unsigned char cmdBuffer[64];
	unsigned char addr[64];
	apmib_get(MIB_ELAN_MAC_ADDR,  (void *)tmpBuff);
	if(!memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", 6))
		apmib_get(MIB_HW_NIC0_ADDR,  (void *)tmpBuff);
	
	sprintf(cmdBuffer, "%02x%02x%02x%02x%02x%02x", tmpBuff[0], tmpBuff[1], 
	tmpBuff[2], tmpBuff[3], tmpBuff[4], tmpBuff[5]);

	tmpChar=cmdBuffer[1];

	switch(tmpChar) 
	{
		case '0':
		case '1':
		case '4':
		case '5':
		case '8':
		case '9':
		case 'c':
		case 'd':
			tmpChar = (char)((int)tmpChar+2);
			break;
		default:
			break;
	}
	sprintf(addr, "Fe80::%c%c%c%c:%c%cFF:FE%c%c:%c%c%c%c", 
	cmdBuffer[0], tmpChar, cmdBuffer[2], cmdBuffer[3],
	cmdBuffer[4], cmdBuffer[5],cmdBuffer[6], cmdBuffer[7],
	cmdBuffer[8],cmdBuffer[9],cmdBuffer[10],cmdBuffer[11]);

	sprintf(cmdBuffer, "address=/%s/%s\n","realsil.com",addr);
	write_line_to_file(RTL_DNSMASQ_CONFILE, 1, cmdBuffer);

	sprintf(cmdBuffer, "server=fe80::20e:2eff:fec0:748b@eth1\n");
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

	sprintf(cmdBuffer, "server=172.29.17.10@eth1\n");
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

	sprintf(cmdBuffer, "domain-needed\n");
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

	sprintf(cmdBuffer, "bogus-priv\n");
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

	sprintf(cmdBuffer, "strict-order\n");
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

	sprintf(cmdBuffer, "interface=%s\n", "eth0");
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);	

	//sprintf(cmdBuffer, "interface=%s\n", "eth1");
	//write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

	sprintf(cmdBuffer, "resolv-file=%s\n", RTL_DNSMASQ_RESOLVFILE);
	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);

//	sprintf(cmdBuffer, "fe80::20e:2eff:fec0:748b www.google.com\n");
//	write_line_to_file("/etc/dnshostfile", 1, cmdBuffer);
	
//	sprintf(cmdBuffer, "addn-hosts=%s\n", "/etc/dnshostfile");
//	write_line_to_file(RTL_DNSMASQ_CONFILE, 2, cmdBuffer);	
}

//void start_rtl_dnsmasq()
void set_dnsv6()
{
	char dnsv6_addr[128];
	//sprintf(dnsv6_addr, "nameserver 2001:da8:8000:1:202:120:2:100\n");
	sprintf(dnsv6_addr, "nameserver 172.29.17.10\n");
	write_line_to_file(RTL_DNSMASQ_RESOLVFILE, 1, dnsv6_addr);	
	
	sprintf(dnsv6_addr, "nameserver fe80::20e:2eff:fec0:748b\n");
	write_line_to_file(RTL_DNSMASQ_RESOLVFILE, 2, dnsv6_addr);
	
	create_rtl_dnsmasq_config_file();
	rtl_dnsmasq_startup();
}
#endif
void set_ipv6()
{
	int val;

#if defined(CONFIG_IPV6)
	printf("Start setting IPv6[IPv6]\n");

	#if 0
	if(!apmib_get(MIB_IPV6_WAN_ENABLE,&val)){		
		fprintf(stderr, "get mib %d error!\n", MIB_IPV6_WAN_ENABLE);
		return ;			
	}
	else if(val==0)
		return;
	
	RunSystemCmd("/proc/sys/net/ipv6/conf/all/forwarding", "echo", "1", NULL_STR);
	#endif
	
	set_basicv6();

// TODO: ipv6 wan
//	set_wanv6();
//	set_lanv6();

// TODO: dhcp v6	
	//printf("Start dhcpv6[IPv6]\n");
//	set_dhcp6s();

// TODO: dns v6	
//	printf("Start dnsv6[IPv6]\n");
#ifdef HAVE_RTL_DNSMASQ
	set_dnsv6();
#endif

	//printf("Start radvd[IPv6]\n");
	//ignored by sen_liu for radvd startup in start_lan_app(), 
	//and to make restart radvd immeditely, set radvdconf in fmipv6.c 
	//set_radvd();

// TODO: pimv6
	//printf("Start ECMH[IPv6]\n");
//	set_ecmh();
	//printf("Start mldproxy[IPv6]\n");
//	start_mldproxy("eth1","br0");
#ifdef HAVE_IPV6FIREWALL
	set_ipv6_firewall();
#endif

#endif

	return;
}





