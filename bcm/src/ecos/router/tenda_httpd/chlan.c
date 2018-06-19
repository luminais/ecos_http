#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flash_cgi.h"
#include "chlan.h"
#include "route_cfg.h"

#ifndef VTS_MAX_NUM_1
#define VTS_MAX_NUM_1 10 //VTS_MAX_NUM in natcmd.c
#endif
#ifndef DHCPD_STATIC_LEASE_NU
#define	DHCPD_STATIC_LEASE_NU 19
#endif


char *setFilterStr(char *lan_ip,char *filt);

void modify_filter_client(const char *lan_ip)
{
	char filter_client[20],*filt,filt2[512];

	int which;

	for(which=0;which<= 9;which++){
		sprintf(filter_client,"filter_client%d",which);
		_GET_VALUE(filter_client,filt);
		snprintf(filt2,sizeof(filt2)-1,filt);
#if 1		
		if(strlen(filt2) > 5)
		{
			filt = setFilterStr(lan_ip,filt2);
			if(filt){
				//printf("====filtcli_%d = %s\n",which,filt);
				_SET_VALUE(filter_client,filt);
			}
		}
#else
		diag_printf("====filtcli_%d = \n",which);
#endif
	}
}

void modify_filter_url(const char *lan_ip)
{
	char filter_url[20],*filt,filt2[512];

	int which;

	for(which=0;which<= 9;which++){
		sprintf(filter_url,"filter_url%d",which);
		_GET_VALUE(filter_url,filt);
		snprintf(filt2,sizeof(filt2)-1,filt);
		if(strlen(filt2) > 5)
		{
			filt = setFilterStr(lan_ip,filt2);
			//printf("====filturl_%d = %s\n",which,filt);
			_SET_VALUE(filter_url,filt);
		}
	}
}

void modify_virtual_server(const char *lan_ip)
{
	int which;
	int len;
	char *p,*q,*forward_port,*forwardport,forward_port4[128];
	char forward[20],forward_port0[20],forward_port1[20],forward_port2[32],forward_port3[64];	//huang add
	unsigned int lan_ip2[4],forward_ip[4];
//5-12>192.168.9.10:5-12,tcp,on	
	for(which = 0;which < VTS_MAX_NUM_1;which++)
	{
		sprintf(forward,"forward_port%d",which);
		_GET_VALUE(forward,forward_port);
		strcpy(forward_port4,forward_port);
		forwardport = forward_port4;
		if(strlen(forward_port4) > 5)
		{
			p = strchr(forwardport,':');
			if(p){
				q = p+1;
				*p = '\0';
				strcpy(forward_port2,q);
				p = strchr(forwardport,'>');
				if(p){
					q = p+1;
					*p = '\0';
					strcpy(forward_port1,q);
				}
				strcpy(forward_port0,forwardport);
			}

			sscanf(lan_ip, "%d.%d.%d.%d", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);		
			sscanf(forward_port1, "%d.%d.%d.%d", &forward_ip[0], &forward_ip[1], &forward_ip[2], &forward_ip[3]);
			sprintf(forward_port1,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],forward_ip[3]);

			sprintf(forward_port3,"%s>%s:%s",forward_port0,forward_port1,forward_port2);
			strcpy(forward_port,forward_port3);
			//printf("==========forward_port-%d = %s===============\n",which,forward_port);
			_SET_VALUE(_FW_FORWARD_PORT(which),forward_port);
		}
	}
}

//clean dmz
void modify_dmz(const char *lan_ip)
{
	char *en,*ip_addr,dmz_ip[20];
	unsigned int lan_ip2[4],dmz_ip2[4];
	
	_GET_VALUE("dmz_ipaddr_en",en);
	if(atoi(en) == 1){
		_GET_VALUE("dmz_ipaddr",ip_addr);	
		strcpy(dmz_ip, ip_addr);
		sscanf(lan_ip, "%d.%d.%d.%d", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);		
		sscanf(dmz_ip, "%d.%d.%d.%d", &dmz_ip2[0], &dmz_ip2[1], &dmz_ip2[2], &dmz_ip2[3]);
		sprintf(dmz_ip,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],dmz_ip2[3]);
		_SET_VALUE("dmz_ipaddr",dmz_ip);	
	} 
}

//;192.168.254.23;22:88:33:44:44:44;1;86400
void modify_dhcp_static_list(char *lan_ip)
{
	char *p,*q;
	char *dhcp_list,dhcp_static[20],dhcp_list0[32],dhcp_list1[32],dhcp_list3[64];
	unsigned int lan_ip2[4],dhcp_ip[4];
	int which;

	for(which = 0; which<= DHCPD_STATIC_LEASE_NU; which++){
		sprintf(dhcp_static,"dhcp_static_lease%d",which);
		_GET_VALUE(dhcp_static,dhcp_list);
		//printf("===================dhcp_list = %s\n",dhcp_list);
		strcpy(dhcp_list3,dhcp_list);
		//printf("===================dhcp_list3 = %s\n",dhcp_list3);
		dhcp_list = dhcp_list3+1;
		//printf("===================dhcp_list_2 = %s\n",dhcp_list);
		if(strlen(dhcp_list3) > 5)
		{
			p = strchr(dhcp_list,';');
			//printf("===================p = %s\n",p);
			if(p){
				q = p+1;
				//printf("===================q = %s\n",q);
				*p = '\0';
				strcpy(dhcp_list1,q);
				//printf("===================dhcp_list1 = %s\n",dhcp_list1);
				strcpy(dhcp_list0,dhcp_list);
				//printf("===================dhcp_list0 = %s\n",dhcp_list0);
			}else{
				continue;
			}
			
			sscanf(lan_ip, "%d.%d.%d.%d", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);		
			sscanf(dhcp_list0, "%d.%d.%d.%d", &dhcp_ip[0], &dhcp_ip[1], &dhcp_ip[2], &dhcp_ip[3]);
			sprintf(dhcp_list0,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],dhcp_ip[3]);

			memset(dhcp_list3,0,sizeof(dhcp_list3));
			sprintf(dhcp_list3,";%s;%s",dhcp_list0,dhcp_list1);
			//printf("===================dhcp_list3_2 = %s\n",dhcp_list3);
			dhcp_list = dhcp_list3;
			//printf("==========dhcp_list-%d = %s===============\n",which,dhcp_list);
			_SET_VALUE(LAN0_DHCP_SATIC(which),dhcp_list);
		}
		}
	
}

void modify_filter_virtual_server(char *lan_ip)
{
	modify_filter_client(lan_ip);
	modify_filter_url(lan_ip);
	modify_virtual_server(lan_ip);
	modify_dmz(lan_ip);
	modify_dhcp_static_list(lan_ip);
}
//192.168.2.20-192.168.2.30:www.baidu.com,0-6,0-0,on,123455434
char *setFilterStr(char *lan_ip,char *filt)
{
	char *p,*q1,*q2;
	char filt0[20],filt1[20],filt3[512];
	unsigned int lan_ip2[4],filter_ip[4];
	
	p = strchr(filt,':');
	if(p){
		q1 = p+1;
		*p = '\0';

		p = strchr(filt,'-');
		if(p){
			q2 = p+1;
			*p = '\0';
			strcpy(filt0,filt);
			strcpy(filt1,q2);

			sscanf(lan_ip, "%d.%d.%d.%d", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);		
			sscanf(filt0, "%d.%d.%d.%d", &filter_ip[0], &filter_ip[1], &filter_ip[2], &filter_ip[3]);
			sprintf(filt0,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],filter_ip[3]);
			sscanf(filt1, "%d.%d.%d.%d", &filter_ip[0], &filter_ip[1], &filter_ip[2], &filter_ip[3]);
			sprintf(filt1,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],filter_ip[3]);
			//printf("====filt0 = %s\n====filt1 = %s\n====filt2 = %s\n",filt0,filt1,q1);
			sprintf(filt3,"%s-%s:%s",filt0,filt1,q1);
			strcpy(filt,filt3);
		}else{
			return NULL;
		}		
	}else{
		return NULL;
	}
	return filt;
}

