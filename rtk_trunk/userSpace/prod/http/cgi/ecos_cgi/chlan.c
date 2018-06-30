#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flash_cgi.h"
#include "chlan.h"

#ifndef VTS_MAX_NUM_1
#define VTS_MAX_NUM_1 16 //VTS_MAX_NUM in natcmd.c
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
			_SET_VALUE(filter_url,filt);
		}
	}
}

void modify_virtual_server(const char *lan_ip)
{
	int which;
	int len;
	char *p=NULL,*q=NULL,*forwardport=NULL,forward_port4[128];
	char forward_port[128]={0};
	char forward[20]={0},forward_port0[48]={0},forward_port1[20]={0},forward_port2[32]={0};	//huang add
	unsigned int lan_ip2[4]={0},forward_ip[4]={0};
	//5-12>192.168.9.10:5-12,tcp,on	
	for(which = 0;which < VTS_MAX_NUM_1;which++)
	{
		
		sprintf(forward,"forward_port%d",which);
		
		strncpy(forward_port, nvram_safe_get(forward), sizeof(forward_port));
		
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
			
			sscanf(lan_ip, "%u.%u.%u.%u", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);
			sscanf(forward_port1, "%u.%u.%u.%u", &forward_ip[0], &forward_ip[1], &forward_ip[2], &forward_ip[3]);
			sprintf(forward_port1,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],forward_ip[3]);
			
			sprintf(forward_port,"%s>%s:%s",forward_port0,forward_port1,forward_port2);
			//printf("==========forward_port-%d = %s===============\n",which,forward_port);
			_SET_VALUE(ADVANCE_PORT_FORWART_RULE(which),forward_port);
			
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
		sscanf(lan_ip, "%u.%u.%u.%u", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);		
		sscanf(dmz_ip, "%u.%u.%u.%u", &dmz_ip2[0], &dmz_ip2[1], &dmz_ip2[2], &dmz_ip2[3]);
		sprintf(dmz_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],dmz_ip2[3]);
		_SET_VALUE("dmz_ipaddr",dmz_ip);	
	} 
}

//;192.168.254.23;22:88:33:44:44:44;1;86400
void modify_dhcp_static_list(char *lan_ip)
{
	char *dhcp_list,dhcp_static_node[20],new_dhcp_ip[32],new_dhcp_list_node[128];
	unsigned int lan_ip2[4],lan_mask2[4],dhcp_ip[4];
	int which;
	int count = 0;
	char *argc[5] = {0}; 
	char *lan_mask;
	
	_GET_VALUE(LAN_NETMASK,lan_mask);

	sscanf(lan_mask, "%u.%u.%u.%u", &lan_mask2[0], &lan_mask2[1], &lan_mask2[2], &lan_mask2[3]);
	sscanf(lan_ip, "%u.%u.%u.%u", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);

	for(which = 0; which<= DHCPD_STATIC_LEASE_NU; which++){
		
		sprintf(dhcp_static_node,"dhcp_static_lease%d",which);
		
		_GET_VALUE(dhcp_static_node,dhcp_list);

		
		count = sscanfArglistConfig(dhcp_list,';' ,  argc, 5);
		if(count != 5 ){
			freeArglistConfig(argc,count);
			continue;
		}
		
		sscanf(argc[1], "%u.%u.%u.%u", &dhcp_ip[0], &dhcp_ip[1], &dhcp_ip[2], &dhcp_ip[3]);
		if( lan_mask2[3] == 0 ){
			
			sprintf(new_dhcp_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],dhcp_ip[3]);

			memset(new_dhcp_list_node,0,sizeof(new_dhcp_list_node));
			sprintf(new_dhcp_list_node,"%s;%s;%s;%s;%s",argc[0],new_dhcp_ip,argc[2],argc[3],argc[4]);
			dhcp_list = new_dhcp_list_node;
			
			_SET_VALUE(ADVANCE_STATIC_IP_MAPPING_RULE(which),dhcp_list);
		}else{
		 	_SET_VALUE(ADVANCE_STATIC_IP_MAPPING_RULE(which),"");
		}
		
		freeArglistConfig(argc,count);
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

void get_address_pool(const char *lan_ip ,const  char *lan_mask,char* start_ip,char* end_ip )
{
	unsigned int lan_ip2[4],lan_mask2[4],old_start[4],old_end[4];
	char *p_old_start = NULL ;
	char *p_old_end = NULL ;
	
	_GET_VALUE(LAN_DHCP_POOL_START,p_old_start);
	_GET_VALUE(LAN_DHCP_POOL_END,p_old_end);

	sscanf(lan_mask, "%u.%u.%u.%u", &lan_mask2[0], &lan_mask2[1], &lan_mask2[2], &lan_mask2[3]);
	sscanf(lan_ip, "%u.%u.%u.%u", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);
	sscanf(p_old_start, "%u.%u.%u.%u", &old_start[0], &old_start[1], &old_start[2], &old_start[3]);
	sscanf(p_old_end, "%u.%u.%u.%u", &old_end[0], &old_end[1], &old_end[2], &old_end[3]);

	switch(lan_mask2[3]){
		case 0:
			sprintf(start_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],old_start[3]);
	
			sprintf(end_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],old_end[3]);

			break;
			
		case  248:
		case  240:
		case  224:
		case  192:
		case  128:
			if(lan_ip2[3] <= (lan_mask2[3] + (254 - lan_mask2[3])/2 ) ){
				sprintf(start_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],lan_ip2[3]+1);
				sprintf(end_ip,"%u.%u.%u.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],254);
			}else{
				sprintf(start_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],lan_mask2[3]+1);
				sprintf(end_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],lan_ip2[3]-1);

			}
			break;
			
		case  252:
			lan_ip2[3] = lan_ip2[3]==253?254:253;
			sprintf(start_ip,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],lan_ip2[3]);
			strcpy(end_ip,start_ip);
			break;
		default:
			strcpy(start_ip,"");
			strcpy(end_ip,start_ip);
			break;
	}
	
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

			sscanf(lan_ip, "%u.%u.%u.%u", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);		
			sscanf(filt0, "%u.%u.%u.%u", &filter_ip[0], &filter_ip[1], &filter_ip[2], &filter_ip[3]);
			sprintf(filt0,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],filter_ip[3]);
			sscanf(filt1, "%u.%u.%u.%u", &filter_ip[0], &filter_ip[1], &filter_ip[2], &filter_ip[3]);
			sprintf(filt1,"%u.%u.%u.%u",lan_ip2[0], lan_ip2[1], lan_ip2[2],filter_ip[3]);
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

