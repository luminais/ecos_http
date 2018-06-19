#include <dhcpd.h>
#include <bcmnvram.h>
#include <route_cfg.h>
struct ip_hn ip_entries[100];
static int sum=0;


int count_ip_entries()
{

	char *ifname;
	struct lease_t *lease, *dump;
	int last;

	int dhcp_client_num=0;
	int static_client_num=0;
	int find_dhcpip=0;
	int i,j,k=0;
	char tmp[20];

	struct detec det[255];
	memset(det,0,sizeof(det));
	get_tenda_arp(det);
		
	memset(ip_entries,0,sizeof(ip_entries));
	
	ifname = nvram_safe_get("lan_ifname");
	
	if (!strcmp(ifname,""))
		return 0;

	/* dump lease */

	dump = lease = dhcpd_lease_dump(ifname);
	
	/* Write HTML until last */
	last = 0;
	
	k = 0;

	while (last == 0&& dump) {
		/* Get the last flag for next time qualification */
		last = lease->last;

		if (lease->flag & RESERVED) {
			lease++;
			continue;
		}
		
		if(lease->expiry < (unsigned long) time(0) && !(lease->flag & STATICL)){
			lease++;
			continue;
		}		
		
		for(i=0;i<255;i++){
			
			if(memcmp(det[i].mac,lease->mac,sizeof(det[i].mac)))
				continue;
			
			sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",det[i].mac[0],det[i].mac[1],det[i].mac[2],det[i].mac[3],det[i].mac[4],det[i].mac[5]);
			if(det[i].inuse&&strcmp(tmp,"00:00:00:00:00:00")){
				strncpy(ip_entries[k].hostname, lease->hostname,20);

			//	printf("***ip_entries[%d].hostname=%s\n",k,ip_entries[k].hostname);
				
				ip_entries[k].ipaddr.s_addr= lease->ipaddr.s_addr;
				memcpy(ip_entries[k].mac, lease->mac, sizeof(lease->mac));
				ip_entries[k].iptype= 0;
				/* Advaince to next */
				dhcp_client_num++;	k++;
			}
			break;
		}
		lease++;
	}
	
    for(i=0; i<255; i++)                                                                                                                                                   
    {        
		find_dhcpip=0;
		sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",det[i].mac[0],det[i].mac[1],det[i].mac[2],det[i].mac[3],det[i].mac[4],det[i].mac[5]);
        if( det[i].inuse&&strcmp(tmp,"00:00:00:00:00:00")){
			if(check_oneip(inet_ntoa_tenda(det[i].ip))){
				
				for(j=0;j<dhcp_client_num;j++){
					if(!memcmp(det[i].mac,ip_entries[j].mac,sizeof(det[i].mac))){
						find_dhcpip=1;								
						break;
					}						
					/*if(det[i].ip==ip_entries[j].ipaddr.s_addr){
						find_dhcpip=1;								
						break;
					}*/
				}	
				if(find_dhcpip==0&&k<100){
					if(strcmp(det[i].hostname,"")!=0)
						sprintf(ip_entries[k].hostname, "%s",det[i].hostname);
					else
						sprintf(ip_entries[k].hostname, "%s","");
				//	printf("&&&ip_entries[%d].hostname=%s\n",k,ip_entries[k].hostname);
					
					ip_entries[k].ipaddr.s_addr= det[i].ip;
					memcpy(ip_entries[k].mac, det[i].mac, sizeof(det[i].mac));
					ip_entries[k].iptype= 1;
					static_client_num++;
					k++;
				}
        	}
        }
    }
	sum=k;
	if(sum == 0) 
		return 0;
//	printf("sum=%d,dhcp_client_num=%d,static_client_num=%d\n",k,dhcp_client_num,static_client_num);
	/* Free buffer */
	free(dump);
	return 1;
}

/*
*	send filter_hostname0~filter_hostname9
*	
*/
void clearHostnameFilter()
{
	int i,j,found=0;
	char strmac1[90];
	
	int k,found2=0;
	char strHost[120];
	char strItem[120];
	char strHost_tail[60];

	for(i=0;i<18;i++){
		if(!strcmp(nvram_safe_get(_FW_FILTER_HOSTNAME(i)),""))
				continue;
		for(j=0,found=0;j<sum;j++){
			memset(strmac1,0,sizeof(strmac1));
			
			sprintf(strmac1,"%02X:%02X:%02X:%02X:%02X:%02X",ip_entries[j].mac[0],ip_entries[j].mac[1],ip_entries[j].mac[2],ip_entries[j].mac[3],ip_entries[j].mac[4],ip_entries[j].mac[5]);
		
			if(!strncmp(nvram_safe_get(_FW_FILTER_HOSTNAME(i)),strmac1,17)){
				found=1;
				break;
			}
		}
		/*old disconnected host*/
		if(!found){
			nvram_set(_FW_FILTER_HOSTNAME(i),"");
		}
		else{
			for(k=0,found2=0;k<18;k++){			
				if(!strncmp(nvram_safe_get(_FW_FILTER_HOSTNAME(i)),nvram_safe_get(_FW_FILTER_MAC(k)),17)){
					found2=1;
					break;
				}
			}
			/*old connected host*/
			if(!found2){
				memset(strHost,0,sizeof(strHost));
				memset(strHost_tail,0,sizeof(strHost_tail));
				memset(strItem,0,sizeof(strItem));
				
				sprintf(strHost,"%s",nvram_safe_get(_FW_FILTER_HOSTNAME(i)));				
				cut_filterhostname_tail(strHost,strHost_tail);
				sprintf(strItem,"%s,0-6,0-0,off,notag,%s",strmac1,strHost_tail);
				nvram_set(_FW_FILTER_HOSTNAME(i),strItem);
			}
		}
	}
/*	for(i=0;i<10;i++){
		if(!strcmp(nvram_get(_FW_FILTER_HOSTNAME(i)),""))
				continue;	
		printf("filter_hostname%d=%s\n",i,nvram_get(_FW_FILTER_HOSTNAME(i)));		
	}*/
	return;
}

/*
*	in: 	C8:9C:DC:54:92:B0,0-6,0-0,on,tag,INVE-20120929FI,192.168.0.100
*	out:	INVE-20120929FI,192.168.0.100
*/
void cut_filterhostname_tail(char *in,char *out)
{
	int  i,ch=',';		
	char *rtn0=NULL,*rtn1=NULL;
	
	for(rtn0=in,i=0;i<5;i++)	
	{		
		rtn1=strchr(rtn0,ch);		
		rtn0=rtn1+1;		
	}
	strcpy(out,rtn0);		
//	printf("tail=%s\n",out);
	return;
}
/*
*	in: 	C8:9C:DC:54:92:B0,0-6,0-0,on,tag,INVE-20120929FI,192.168.0.100
*	out:	C8:9C:DC:54:92:B0,0-6,0-0,on,tag
*/
void cut_filterhostname_head(char *in,char *out)
{
	int  i,ch=',';		
	char *rtn0=NULL,*rtn1=NULL;
	
	for(rtn0=in,i=0;i<5;i++)	
	{		
		rtn1=strchr(rtn0,ch);		
		rtn0=rtn1+1;		
	}
	strncpy(out,in,rtn0-in-1);		
//	printf("head=%s\n",out);
	return;
}
/*
*	if filter_hostname's mac exit in filter_mac web ,
*	then recover filter_hostname
*
*/
void isinMacfilter()
{
	int i,j;
	char strHost[120]={'\0'};
	char strMac[120]={'\0'};
	char strHost_tail[60]={'\0'};

	for(i=0;i<18;i++){

		memset(strHost, 0, sizeof(strHost));
		memset(strHost_tail, 0, sizeof(strHost_tail));
		memset(strMac, 0, sizeof(strMac));
		
		if(!strcmp(nvram_safe_get(_FW_FILTER_HOSTNAME(i)),""))
				continue;
		for(j=0;j<18;j++){
			
			if(!strcmp(nvram_safe_get(_FW_FILTER_MAC(j)),""))
				continue;
			
			if(!strncmp(nvram_safe_get(_FW_FILTER_MAC(j)),nvram_safe_get(_FW_FILTER_HOSTNAME(i)),17)){
				sprintf(strHost,"%s",nvram_safe_get(_FW_FILTER_HOSTNAME(i)));
				sprintf(strMac,"%s",nvram_safe_get(_FW_FILTER_MAC(j)));
				cut_filterhostname_tail(strHost,strHost_tail);
				sprintf(strHost,"%s,%s",strMac,strHost_tail);
				nvram_set(_FW_FILTER_HOSTNAME(i),strHost);

				break;
			}
		}
	}
	return;
}

/*C8:9C:DC:54:92:B0,0-6,0-0,on,tag,INVE-20120929FI,192.168.0.100*/
void printf_ip_entries()
{
	int i,j,k=0,found,first=0;
//	printf("***************************************\n");
	char str[120]={'\0'},strmac1[120]={'\0'};
	char strHost[120]={'\0'};
	char strHost_head[60]={'\0'};
	char strHost_tail[60]={'\0'};

	for(i=0;i<sum;i++){
		
		memset(strHost_head, 0, sizeof(strHost_head));
		memset(strHost_tail, 0, sizeof(strHost_tail));
		memset(str, 0, sizeof(str));
		memset(strmac1, 0, sizeof(strmac1));
		memset(strHost, 0, sizeof(strHost));
		
		found=0;
	//	printf("ip_entries[%d].hostname=%s\n",i,ip_entries[i].hostname);
	//	printf("ip_entries[%d].ipaddr=%s\n",i,inet_ntoa(ip_entries[i].ipaddr));
	//	printf("ip_entries[%d].mac=%02X:%02X:%02X:%02X:%02X:%02X\n",i,ip_entries[i].mac[0],ip_entries[i].mac[1],ip_entries[i].mac[2],ip_entries[i].mac[3],ip_entries[i].mac[4],ip_entries[i].mac[5]);
	//	printf("ip_entries[%d].iptype=%s\n",i,ip_entries[i].iptype==0?"dhcp":"static");
		sprintf(strmac1,"%02X:%02X:%02X:%02X:%02X:%02X",ip_entries[i].mac[0],ip_entries[i].mac[1],ip_entries[i].mac[2],ip_entries[i].mac[3],ip_entries[i].mac[4],ip_entries[i].mac[5]);
		for(j=0;j<18;j++){
			
			if(!strcmp(nvram_safe_get(_FW_FILTER_HOSTNAME(j)),"")){
				if(first==0){
					k=j; first=1;
				}
				continue;
			}
			if(!strncmp(nvram_safe_get(_FW_FILTER_HOSTNAME(j)),strmac1,17)){
				found=1;
				break;
			}
		}
		if(!found){
			sprintf(str,"%02X:%02X:%02X:%02X:%02X:%02X,0-6,0-0,off,notag,%s,%s",ip_entries[i].mac[0],ip_entries[i].mac[1],ip_entries[i].mac[2],ip_entries[i].mac[3],ip_entries[i].mac[4],ip_entries[i].mac[5],ip_entries[i].hostname,inet_ntoa(ip_entries[i].ipaddr));
			nvram_set(_FW_FILTER_HOSTNAME(k),str);
		}
		else{
			sprintf(strHost,"%s",nvram_safe_get(_FW_FILTER_HOSTNAME(j)));
			cut_filterhostname_head(strHost,strHost_head);
			sprintf(strHost_tail,"%s,%s",ip_entries[i].hostname,inet_ntoa(ip_entries[i].ipaddr));
			sprintf(str,"%s,%s",strHost_head,strHost_tail);
			nvram_set(_FW_FILTER_HOSTNAME(j),str);
		}
	}
	isinMacfilter();
	clearHostnameFilter();

	return;
}


/*end add*/
