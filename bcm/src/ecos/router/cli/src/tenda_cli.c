
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include <iflib.h>
#include <ecos_oslib.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_var.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <cli.h>

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmparams.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <netconf.h>
#include <nvparse.h>
#include <wlutils.h>



char *inet_ntoa_r_tenda(in_addr_t ina, char *buf);
char *inet_ntoa_tenda(in_addr_t ina);
extern unsigned char *lookmac(in_addr_t ip);
extern in_addr_t lookip(int *mac);

struct detec{
        char inuse;
        char auth;
        unsigned char mac[6];
        in_addr_t ip;
	char hostname[64];
	int flag;
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	time_t time; //sta第一次接入路由器的时间
	time_t interval_time;//sntp更新前的时间间隔
	time_t update_time; //最近一次更新时间
#endif
};
extern struct detec det[255]={0};

#define MAX_STA_COUNT	256
#define NVRAM_BUFSIZE	100

int isinwireless_client(unsigned char *mac,in_addr_t ip)
{
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *name=NULL;
	struct maclist *mac_list;
	int mac_list_size;
	struct maclist *mac_list2;
	int mac_list_size2;
	int i;

	char wl_unit[4]={'\0'};

//	wl_unit = nvram_get("wl_unit");

	strcpy(wl_unit,"0");
	snprintf(prefix, sizeof(prefix), "wl%s_", wl_unit);

	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if(!strcmp(name,""))
		return 0;
	
	/* buffers and length */
	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);

	if (!mac_list){
		return 0;
	}

	/* query wl0 for authenticated sta list */
#ifndef __NetBSD__
	strcpy((char*)mac_list, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size)) {
		free(mac_list);
		return 0;
	}
#else
	/* NetBSD TBD... */
	mac_list->count=0;
#endif

	/* query sta_info for each STA and output one table row each */
	for (i = 0; i < mac_list->count; i++) {
		if(!memcmp(mac_list->ea[i].octet,mac,sizeof(det[i].mac))){
		 	free(mac_list);
		 	return 1;
		}
	}
	
	free(mac_list);
	
	strcpy(wl_unit,"0.1");
	
	snprintf(prefix, sizeof(prefix), "wl%s_", wl_unit);

	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if(!strcmp(name,""))
		return 0;

	/* buffers and length */
	mac_list_size2 = sizeof(mac_list2->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list2 = malloc(mac_list_size2);

	if (!mac_list2){
		return 0;
	}

	/* query wl0.1 for authenticated sta list */
#ifndef __NetBSD__
	strcpy((char*)mac_list2, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, mac_list2, mac_list_size2)) {
		free(mac_list2);
		return 0;
	}
#else
	/* NetBSD TBD... */
	mac_list2->count=0;
#endif

	/* query sta_info for each STA and output one table row each */
	for (i = 0; i < mac_list2->count; i++) {
		if(!memcmp(mac_list2->ea[i].octet,mac,sizeof(det[i].mac))){
		 	free(mac_list2);
		 	return 1;
		}
	}
	
	free(mac_list2);

	return 0;
}



int count_client_num()
{
	int i = 0;
	int j = 0;
	for(i=0; i<255; i++)                                                                                                                                                                                                                       
	{     
		if(det[i].ip==0)
		{
			return j;
			
		}
		if(det[i].inuse==1)
				j++;
		
	}
	return i;
}
int check_ifauth(/*unsigned char* mac,*/ in_addr_t ip)
{
        int i;
        for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
            if( 1 == det[i].inuse &&det[i].ip==ip )                                                                                                  
            {          
            	   //diag_printf("auth is 1\n");
                    return det[i].auth;                                                                                                                                    
            }                                                                                                                                                              
        }                                                                                                                                                                      
        return -1;                                                                                                                                                             
}      
void clear_auth()
{
        int i;
        for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
            if( 1 == det[i].inuse)                                                                                                  
            {                                                                                                                                                              
                    det[i].auth = 0;                                                                                                                                    
            }                                                                                                                                                              
        }                                                                                                                                                                                                                                                                                                                           
}
//ip to mac
unsigned char *lookmac(in_addr_t ip)
{
	static unsigned char rtn[6]={0};
	memset(rtn,0,6);
	int i;
    for(i=0; i<255; i++)                                                                                                                                                   
    {   
        if( 1 == det[i].inuse&& det[i].ip==ip)                                                                                                  
        {
    		memcpy(rtn, det[i].mac, sizeof(rtn));
            return rtn;                                                                                                                             
        }                                                                                                                                                              
    }
	return NULL;
}

//mac to ip

in_addr_t lookip(int *mac)
{
	 int i,j=0,temp=1;
	 in_addr_t a=0;
        for(i=0; i<255; i++)                                                                                                                                                   
        {   
        	temp=1;
             for(j=0;j<6;j++)
             {	
             	if(det[i].mac[j] != mac[j])
             	{
             		temp=0;
					break;
             	}
             }
			 if(temp)
			 {
			 	//printf("ok,i=%d\n",i);
			 	a=det[i].ip;
				return a;
			 }
        }
	return a;
}
/*
in_addr_t lookip(unsigned char *mac)
{
	 int i;
	 printf("%02x:%02x:%02x:%02x:%02x:%02x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        for(i=0; i<255; i++)                                                                                                                                                   
        {       	
                if( 1 == det[i].inuse && memcmp(det[i].mac, mac, sizeof(det[i].mac)) == 0)                                                                                                  
                {     
                		printf("%s\n",inet_ntoa_tenda(det[i].ip));
                        return det[i].ip;                                                                                                                                    
                } 
        }
	return 0;
}
*/

//mac to hostname

char *newlookhostname(int *mac)
{
	 int i,temp=1,j=0;
	 //printf("%02x:%02x:%02x:%02x:%02x:%02x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	 static unsigned char rtn[64]={0};	 
	 memset(rtn,0,64);
        for(i=0; i<255; i++)                                                                                                                                                   
        {
        		temp=1;
                 for(j=0;j<6;j++)
             	{	
	             	if(det[i].mac[j] != mac[j])
	             	{
	             		temp=0;
						break;
	             	}
	             }
				 if(temp)
				 {
				 	//printf("ok,i=%d,%s\n",i,det[i].hostname);
				 	memcpy(rtn, det[i].hostname, sizeof(rtn));
					return rtn;
				 }                                                                                                                                                                                                                                                                                                                          
        }
	return rtn;
}


char *lookhostname(unsigned char *mac)
{
	int i;
	static unsigned char rtn[64]={0};
	memset(rtn,0,64);
    for(i=0; i<255; i++)                                                                                                                                                   
    {                                                                                                                                                                      
            if( 1 == det[i].inuse && memcmp(det[i].mac, mac, sizeof(det[i].mac)) == 0)                                                                                                  
            {                                                                                                                                                              
                    memcpy(rtn, det[i].hostname, sizeof(rtn));
					return rtn;
            }                                                                                                                                                              
    }
	strcpy(rtn,"");
	return rtn;
}

char *mac_look_hostname(char *mac)
{
	int i;
	static unsigned char rtn[64]={0};
	char mac_list[20] = {0};
	int k = 0;
	memset(rtn,0,64);
	for(i=0; i<255; i++)                                                                                                                                                   
	{    
		if(1 == det[i].inuse)
		{
			memset(mac_list,0,20);
			sprintf(mac_list,"%02x:%02x:%02x:%02x:%02x:%02x",det[i].mac[0],det[i].mac[1],det[i].mac[2],det[i].mac[3],det[i].mac[4],det[i].mac[5]);
			for(k=0;k<strlen(mac_list);k++)
			{
				//将MAC地址转换成大写
				if(mac_list[k] >= 'a' && mac_list[k] <= 'z')
					mac_list[k] -= 32;
			}
	        if(memcmp(mac_list, mac, 20) == 0)                                                                                                  
	        {                                                                                                                                                              
	            memcpy(rtn, det[i].hostname, sizeof(rtn));					
				return rtn;
	        }
		}
	}
	strcpy(rtn,"");
	return rtn;
}


//mac to flag

int lookflag(unsigned char *mac)
{
	 int i,a=0;
        for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
                if( 1 == det[i].inuse && memcmp(det[i].mac, mac, sizeof(det[i].mac)) == 0)                                                                                                  
                {                                                                                                                                                              
                        a = det[i].flag; 
						return a;
                }                                                                                                                                                              
        }
	return a;
}


int iplookflag(in_addr_t ip)
{
	 int i,a=0;
        for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
                if( 1 == det[i].inuse&& det[i].ip==ip)                                                                                                  
                {                                                                                                                                                              
                        a = det[i].flag; 
						return a;
                }                                                                                                                                                              
        }
	return 0;
}



int lookentry(in_addr_t ip)
{
	int i;
        for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
            if( 1 == det[i].inuse&&det[i].ip==ip)                                                                                                  
            {   
            	   //printf("in the entry\n");
                    return 1;                                                                                                                                    
            }                                                                                                                                                              
        }
	//printf("no entry\n");
	return -1;
}

int ismulticast(unsigned char *mac)
{
	if(mac == NULL)
		return -1;

	if(	mac[0] == 0 &&
		mac[1] == 0 &&
		mac[2] == 0 &&
		mac[3] == 0 &&
		mac[4] == 0 &&
		mac[5] == 0)
		return 1;
	else
		return 0;
}
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
/*
	归一化tenda_arp 表项更新
	add : 由匹配MAC 改为匹配MAC + IP
	del :  由匹配IP  改为匹配MAC + IP
*/
int update_arptable(unsigned char *mac, in_addr_t ip, int peration)
{
	int i,j=0;
	time_t now;
	if(ip == 0 || mac == NULL)
		return -1;

	if(ismulticast(mac))
		return -1;
	now = time(0);
	
	if( 1 == peration )//add sta
	{
		for(i=0; i<255; i++)
		{
			if(memcmp(det[i].mac, mac, sizeof(det[i].mac)) == 0 && det[i].ip == ip)
			{
				/*find sta , not update*/
				det[i].update_time = now;
				return 1;
			}
		}
		for(i=0; i<255; i++) 
		{
			 if(0==det[i].inuse)
			 {
				det[i].inuse = 1;
				 memcpy(det[i].mac, mac, sizeof(det[i].mac));
				 det[i].ip = ip;
				 memset(det[i].hostname,0,sizeof(det[i].hostname));
				 if(isinwireless_client(det[i].mac,det[i].ip)==1)
					det[i].flag=1;
				else
					det[i].flag=0;
				det[i].time = now;
				det[i].interval_time = 0;
				det[i].update_time = now;
				printf("ADD %s -- %s Success (new) (%d) !!!\n",inet_ntoa_tenda(ip),inet_mactoa(mac),i+1);
				return 1;
			 }
		}
		 return -1;
	}
	else  if( 2 == peration )//del sta
	{
		for(i=0; i<255; i++)
		{
			 if(memcmp(det[i].mac,mac,sizeof(det[i].mac)) == 0 && det[i].ip == ip)
			 {
			 	if(now - det[i].update_time < 30)
			 	{
			 		printf("%s[%s] has updated, return. now:%u last:%u\n", inet_ntoa_tenda(ip), inet_mactoa(mac), now ,det[i].update_time);
					return 1;
			 	}
				printf("DEL %s[%s]. now:%u last:%u\n", inet_ntoa_tenda(ip), inet_mactoa(mac), now ,det[i].update_time);
				memset(det[i].mac, 0, sizeof(det[i].mac));
				det[i].ip = 0;
				strcpy(det[i].hostname,"");
				det[i].flag = 0;
				det[i].inuse = 0;
				det[i].auth = 0;
				det[i].time = 0;
				det[i].interval_time = 0;
				det[i].update_time = 0;

				return 1;
			 }
		}
		 return -1;
	}
	else  if( 3 == peration )
	{
		for(i=0; i<255; i++)
		{
			 if(1==det[i].inuse && !memcmp(det[i].mac,mac,sizeof(det[i].mac)) && det[i].ip==ip)
			 {
				det[i].auth = 1;
				return 1;
			 }
		}
		return -1;
	}
	
	 return -1;
}
#else

/*
1-->add
2-->del
3-->modify
*/
int update_arptable(unsigned char *mac, in_addr_t ip, int peration)                                                                                                                                 
{                                                                                                                                                                              
        int i,j=0;
		int found=0;
		
		if(ip == 0 || mac == NULL)
			return -1;

		if(ismulticast(mac))
			return -1;
       
        if( 1 == peration )                                                                                                                                                                                                                                
        {      
		//	printf("ADD %s -- %s\n",inet_ntoa_tenda(ip),inet_mactoa(mac));
   	 		for(i=0; i<255; i++)  
	 		{	
				//if(det[i].ip==ip)
				if(memcmp(det[i].mac, mac, sizeof(det[i].mac)) == 0)
				{
					
                    //memcpy(det[i].mac, mac, sizeof(det[i].mac));
                    det[i].ip = ip;
					det[i].inuse = 1;
					if(isinwireless_client(det[i].mac,det[i].ip) == 1)
						det[i].flag=1;
					//printf("ADD %s -- %s Success (exit) (%d)!!!\n",inet_ntoa_tenda(ip),inet_mactoa(mac),i);
					found=1; 
					break;
				//	return 1;
				}
   	 		}
			if(found){
			#if 1
				for(i=0;i<255;i++){
					if(!memcmp(det[i].mac,mac,sizeof(det[i].mac)) && det[i].ip != ip){
						det[i].ip = 0;
						det[i].inuse=0;
						det[i].flag=0;
						memset(det[i].mac,0,sizeof(det[i].mac));
						memset(det[i].hostname,0,sizeof(det[i].hostname));
                     	return 1;
                	}
				}
			#endif
				return 1;
			}		
			
            for(i=0; i<255; i++)                                                                                                                                                                                                                       
            {                                                                                                                                                                                                                                         
                if(0==det[i].inuse /*&& det[i].ip == 0*/)                                                                                                                                                                                                                
                {                                  
                    det[i].inuse = 1;
                    memcpy(det[i].mac, mac, sizeof(det[i].mac));
                    det[i].ip = ip;
					memset(det[i].hostname,0,sizeof(det[i].hostname));
					if(isinwireless_client(det[i].mac,det[i].ip)==1)
						det[i].flag=1;
					else
						det[i].flag=0;	
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
			det[i].time = time(0);
			det[i].interval_time = 0;
		#endif
				//	printf("ADD %s -- %s Success (new) (%d) !!!\n",inet_ntoa_tenda(ip),inet_mactoa(mac),i);

                    return 1;
						
                }
            }
            return -1;
        }
        if( 2 == peration )
        {
			//printf("DEL %s -- %s\n",inet_ntoa_tenda(ip),inet_mactoa(mac));
			#if 1
            for(i=0; i<255; i++)
            {
                if(/*!memcmp(det[i].mac,mac,sizeof(det[i].mac))|| */det[i].ip == ip)
                {	
						memset(det[i].mac, 0, sizeof(det[i].mac));
						det[i].ip = 0;
						strcpy(det[i].hostname,"");
						det[i].flag = 0;
                        det[i].inuse = 0;
		    			det[i].auth = 0;
				#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
					det[i].time = 0;
					det[i].interval_time = 0;
				#endif
					
                        return 1;
                }
            }
			#endif
            return -1;
        }
        if( 3 == peration )
        {
            for(i=0; i<255; i++)
            {
                    if(1==det[i].inuse /*&&!memcmp(det[i].mac,mac,sizeof(det[i].mac)) */&& det[i].ip==ip)
                    {
                            det[i].auth = 1;
                            return 1;
                    }

            }
            return -1;
        }
        return -1;
}
#endif
char *
inet_ntoa_r_tenda(in_addr_t ina, char *buf)
{
    unsigned char *ucp = (unsigned char *)&ina;
    diag_sprintf(buf, "%d.%d.%d.%d",
                 ucp[0] & 0xff,
                 ucp[1] & 0xff,
                 ucp[2] & 0xff,
                 ucp[3] & 0xff);
    return buf;
}

char *
inet_ntoa_tenda(in_addr_t ina)
{
    static char buf[sizeof "123.456.789.012"];
    return inet_ntoa_r_tenda(ina, buf);
}

int check_ip_connected(char *ip)
{
	int i;
	for(i=0;i<255;i++)
	{
		if(strcmp(inet_ntoa_tenda(det[i].ip),ip)==0)
		{
			if(det[i].inuse==1)
				return 1;
			else if(det[i].inuse==0)
				return 0;
		}
			
	}	
	return 0;
}

int check_ip_type(char *ip)
{
	int i;
	for(i=0;i<255;i++)
	{
		if(strcmp(inet_ntoa_tenda(det[i].ip),ip)==0)
		{
			if(det[i].flag==0)
				return 0;
			else
				return 1;
		}	
		
	}	
	return 0;
}
void get_tenda_arp(struct detec *buf)
{

	int i;
	                                                                                                                                                                                                                               
	for(i=0; i<255; i++){     
		buf[i].inuse=det[i].inuse;
		buf[i].auth=det[i].auth;
		buf[i].flag=det[i].flag;
		memcpy(buf[i].mac,det[i].mac,sizeof(det[i].mac));
		memcpy(buf[i].hostname,det[i].hostname,64);
		buf[i].ip=det[i].ip;	
	}
	return;
}

struct machost_t{
		in_addr_t ip;
        unsigned char mac[6];
		char hostname[64];
};
void set_arp_hostname(struct machost_t *buf)
{
	int i,j;

	for(i=0;i<255;i++){
		if(buf[i].ip==0||!strcmp(buf[i].hostname,""))
			continue;
		
		for(j=0;j<255;j++){
			if(det[j].ip==0)
				continue;
			if(!memcmp(det[j].mac,buf[i].mac,sizeof(det[j].mac))){
				strncpy(det[j].hostname,buf[i].hostname,64);
				break;
			}
		}
	}
	return;
}

static int show_det_table(int argc, char* argv[])
{

	int i;
	char tmp[20];
	                                                                                                                                                                                                                               
	for(i=0; i<255; i++)                                                                                                                                                                                                                       
	{     
		if(det[i].ip!=0)
		{
			if(det[i].flag==0)
				strcpy(tmp,"wired");
			else
				strcpy(tmp,"wireless");
			
			diag_printf("NO.%d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X,%s,%s,%s\n",
						i+1,
                        det[i].inuse,det[i].auth,
                        det[i].mac[0]&0XFF,
                        det[i].mac[1]&0XFF,
                        det[i].mac[2]&0XFF,
                        det[i].mac[3]&0XFF,
                        det[i].mac[4]&0XFF,
                        det[i].mac[5]&0XFF,
                        inet_ntoa_tenda(det[i].ip),
                        tmp,
                        det[i].hostname
                        );
		}
	}
	return 0;
}


/* cli node */
CLI_NODE("tenda_arp",	show_det_table,	"tenda_arp command");
