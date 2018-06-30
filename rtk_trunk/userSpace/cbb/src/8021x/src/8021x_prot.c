/*
 *roy +++
 */

#include <8021x.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/sockio.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/md5.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <bcmdhcpd/client/dhcpc_ecos_osl.h>
#include <rc.h>


/*******************************************************\
 全局数据
\*******************************************************/
int x8021_conn_successflag;
extern int x8021_running;
extern int x8021_shutdown;

extern int ifconfig(char *ifname, int flags, char *addr, char *netmask);

unsigned char mainEthMcast8021x[6]={0x01,0x80,0xc2,0x00,0x00,0x03};
//unsigned char mainEthMcast8021x[6]={0xff,0xff,0xff,0xff,0xff,0xff};
extern int X8021_write(int fd, unsigned char *dst_hwaddr, char *data, int len);

/* Digests a string and prints the result.
*/
void MD5String (unsigned char *string,unsigned int len,unsigned char *result)
{
	MD5_CTX context;
	MD5Init(&context);
	MD5Update(&context, string, len);
	MD5Final (result, &context);
}

void GetEapStartPacket(struct x8021_ifc *config)
{
  	char tempbuf[60];
  	int  len = 0;
	EAP* packet = (EAP*)tempbuf;

	memset(tempbuf, 0x00, 60);

	packet->Version = 1;
	packet->PktType = 1;
	packet->Len1 = 0;
  	len = sizeof(* packet);

	X8021_write(config->dev_fd, mainEthMcast8021x, tempbuf, len);
	
	X8021_DBG("Start");
	return;
}

void GetEapLogoffPacket(struct x8021_ifc *config)
{
  	char tempbuf[60];
  	int  len = 0;
	EAP* packet = (EAP*)tempbuf;

	memset(tempbuf, 0x00, 60);

	packet->Version = 1;
	packet->PktType = 2;//logoff
	packet->Len1 = 0;
  	len = sizeof(* packet);
	
	X8021_write(config->dev_fd, mainEthMcast8021x, tempbuf, len);
	
	X8021_DBG("EAPOL logoff");
	memset(mainEthMcast8021x,0,sizeof(mainEthMcast8021x));
	mainEthMcast8021x[0]=0x01;
	mainEthMcast8021x[1]=0x80;
	mainEthMcast8021x[2]=0xc2;
	mainEthMcast8021x[3]=0x00;
	mainEthMcast8021x[4]=0x00;
	mainEthMcast8021x[5]=0x03;
	return;
}

void GetEapIdPacket(struct x8021_ifc *config,int id)
{
	char tempbuf[200];
  	int  len = 0;
	int usrlen = strlen(config->user_name);
	EAP* packet = (EAP*)tempbuf;
	memset(tempbuf, 0x00, 200);

	packet->Version = 1;
	packet->PktType = 0;
	packet->Len1 = htons(5+usrlen);
	packet->Code = 2;
	packet->Id = (unsigned char)id;
	packet->Len2 = htons(5+usrlen);
  	packet->EapType = 1;
	memcpy(&packet->EapType+1, config->user_name, usrlen);

	len = sizeof(* packet) + usrlen;

	X8021_write(config->dev_fd, mainEthMcast8021x, tempbuf, len);

	X8021_DBG("Response Identity");
	
	return;
}

void GetEapPwdPacket(struct x8021_ifc *config,int id, char *chap, int chaplen)
{
	char tempbuf[200];
	unsigned char TmpBuf[128];
	
  	int  len = 0;
	EAP* packet = (EAP*)tempbuf;

	MD5_data *md5_data;
	MD5_CTX context;
	unsigned char md5[16];

	int pwdlen = strlen(config->password);
	int userlen = strlen(config->user_name);
	memset(tempbuf, 0x00, 200);

	packet->Version = 1;
	packet->PktType = 0;
	packet->Len1 = htons(22+userlen);
	packet->Code = 2;
	packet->Id = (unsigned char)id;
	packet->Len2 = htons(22+userlen);
	packet->EapType = 4;
// Get MD5 password
	TmpBuf[0] = (unsigned char)id;
	memcpy(TmpBuf + 1, config->password, pwdlen);
	memcpy(TmpBuf + 1 + pwdlen, chap, chaplen);
	TmpBuf[1 + pwdlen + chaplen] = 0;
	
	MD5Init(&context);
    	MD5Update(&context, TmpBuf, strlen(TmpBuf));
	MD5Final(md5, &context);
//
	md5_data = (MD5_data *)&tempbuf[sizeof(* packet)];
	md5_data->value_size = 16;
	memcpy(md5_data->value,md5,16);	
	memcpy(md5_data->extra_data,config->user_name,userlen);
	
	len = sizeof(* packet) + sizeof(*md5_data) + userlen -1;

	X8021_write(config->dev_fd,mainEthMcast8021x, tempbuf, len);

	X8021_DBG("Response MD5-Challenge");
	
	return;
}

void X8021Connect(struct x8021_ifc *config)
{
    	GetEapStartPacket(config);
}

int X8021Initialize(struct x8021_ifc *config)
{
	char *ifname = config->ifname;
	char name[64];

	X8021_DBG("interface %s init", ifname);

	sprintf(name, "%s/%s/%d", X8021_DEVNAME, ifname,ETHERTYPE_PAE);
	config->dev_fd = open(name, O_RDWR);
	if (config->dev_fd <= 0) {
		return -1;
	}

	X8021_DBG("Initialize OK");
	
	return 0;
}

void X8021Terminate(struct x8021_ifc *config) 
{	
	GetEapLogoffPacket(config);
	if(config->dev_fd >0)
	{
		close(config->dev_fd);
	}
	free(config);
}


void X8021Disconnect(struct x8021_ifc *config)
{
	X8021Terminate(config);
}

int X8021_rx(struct x8021_ifc *config)
{
    int  rid;
    char pwd[128];
    unsigned char  pwdlen;
 
    EAP *p = (EAP *)config->rx_buf;

    // diag_printf("X8021_rx : Request Code =%d \r\n", p->Code);

    switch( p->Code ) 
    {
	    case RT_CODE_RT:    // 请求处理
	   			
	        rid = p->Id;

	        //diag_printf("X8021_rx : username=%s, user_id=%s, rid = %d ! \r\n", g_cfg.x8021_param.user_id, user_id, rid );
	        //diag_printf("X8021_rx : EapType = %d \r\n", atoi((char*)p->EapType));

	        if(p->EapType == EAP_USR) // 请求用户名
	        {
	            // diag_printf("X8021_rx : user_id=%s, rid = %d ! \r\n", user_id, rid );
	            X8021_DBG("Request Identity");
	            GetEapIdPacket(config, rid);
	        }
	        else if(p->EapType == EAP_PWD) // 请求密码
	        {

		    X8021_DBG("Request MD5-Challenge");
					
	           pwdlen = config->rx_buf[9]&0XFF;
	           memset(pwd, 0x00, 128);
	           memcpy(pwd, &config->rx_buf[10], pwdlen);
	            //diag_printf("X8021_rx : user_id=%s rid = %d user_pwd=%s! pwd=%s \r\n", user_id, rid, user_pwd, pwd);

	           GetEapPwdPacket(config,rid, pwd, pwdlen);
	        }
	        break;
	    case RT_CODE_SU:    // 成功
	        if( x8021_conn_successflag !=3 ) 
	        {        	
	    		X8021_DBG("Success");
	        	x8021_conn_successflag = 3;
	        }
	        break;
	    case RT_CODE_FL:    // 失败        
	        if( x8021_conn_successflag != -1 ) 
	        {        
	    		X8021_DBG("Failure");
			GetEapLogoffPacket(config);
	        	x8021_conn_successflag = -1;
	        }
	        break;
	    default:
	        diag_printf("X8021_rx : finished! \r\n");
	        break;
    }
	return TRUE;
}


int X8021Receive(struct x8021_ifc *config)
{
    	char buf[256];
	int n, datalen = 0;
	struct timeval tv;
	fd_set fds;
	int maxfd;

	/* Select every one second */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	maxfd = config->dev_fd;
	FD_ZERO(&fds);
	FD_SET(config->dev_fd, &fds);
	maxfd = config->dev_fd;
	
	/* Wait for socket events */
	n = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (n <= 0)
		return n;

	/* Check 8021x packet */
	if (FD_ISSET(config->dev_fd, &fds)) {
		memset(buf,0,sizeof(buf));
		/*pxy rm, don't switch multicase to unicast*/
	//	memset(mainEthMcast8021x,0,sizeof(mainEthMcast8021x));
		datalen = read(config->dev_fd, buf, sizeof(buf));
		datalen = datalen>256?256:datalen;
		/*pxy rm, don't switch multicase to unicast*/
	//	memcpy(mainEthMcast8021x,&buf[6],6);
		memcpy(config->rx_buf,&buf[14],datalen-14);
		X8021_rx(config);
	}

	return datalen;
}


void
X8021_mainloop(struct x8021_ifc *config)
{
	/* Do init */	
	int n;
	int mtu,wan_start_successflag = 1;
	char unit,tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char *value;

	for (unit = 0; unit < 255; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		 if(nvram_match(strcat_r(prefix, "proto", tmp), "8021x"))
		 	break;
	}

	wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
	
	if (X8021Initialize(config) != 0) {
		X8021_DBG("interface %s init failed!", config->ifname);
		goto error;
	}

	x8021_running = 1;
	x8021_shutdown = 0;

	while(1)
	{
				x8021_conn_successflag = 0;
				X8021Connect(config);
listen:					
				n = X8021Receive(config);
				if(wan_start_successflag)//若没有配置IP，并且没有监听到数据则重新发起认证
				{
					if(n <=0)
					{
						if(x8021_shutdown)
							break;
						continue;
					}
				}
				if(x8021_shutdown)
						break;
				if(x8021_conn_successflag == 3)
				{
						// if 802.1x is successful, call wan start
						//mon_snd_cmd( MON_CMD_WAN_INIT );
					if(wan_start_successflag)
					{
						if(2 == atoi(nvram_safe_get("wan0_1x_ardmode")))
						{
							value = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
							nvram_set(strcat_r(prefix, "connect", tmp), "Connecting");
							nvram_set("wan0_1x_bind_ardmode","2");//for wan_stop
							dhcpc_start(wan_ifname, "8021xdhcpc", value);
						}else if(1 == atoi(nvram_safe_get("wan0_1x_ardmode"))){
							nvram_set("wan0_1x_bind_ardmode","1");//for wan_stop
							ifconfig(wan_ifname, IFUP,
								 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
								 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
								 mtu = atoi(nvram_safe_get(strcat_r(prefix, "mtu", tmp)));
							if(mtu > 0)
								ifconfig_mtu(wan_ifname,mtu);
						    #ifdef CONFIG_RTL_HARDWARE_NAT
							#ifndef HAVE_NOETH
							extern int rtl_setWanNetifMtu(int mtu);
							rtl_setWanNetifMtu(mtu);
							#endif
							#endif
							add_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)), NULL , ADD_NS_PUSH);
							wan_up(wan_ifname);
						}
					}
					wan_start_successflag=0;
					x8021_conn_successflag=0;
					goto listen;
					//break;
				}		
				else if(x8021_conn_successflag == -1)
				{
						cyg_thread_delay(300);
						if(x8021_shutdown)
							break;
						continue;
				}
				else
				{
						goto listen;
				}			
								
	}
error:
	X8021Terminate(config);
	x8021_running = 0;
	x8021_conn_successflag=0;
	return;
}


