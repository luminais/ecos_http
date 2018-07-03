#include "simple_config.h"
#include "../system/sys_init.h"

static int set_sc_led_status(RTK_SC_CTXp pCtx)
{
	if(pCtx->sc_wlan_status == 0 && pCtx->sc_status>=2)
	{
		if(pCtx->sc_led_status == 1)
		{
			//run_clicmd("echo 0 > /proc/gpio");
			pCtx->sc_led_status = 0;
		}
		else if(pCtx->sc_led_status == 0)
		{
			//system("echo 1 > /proc/gpio");
			pCtx->sc_led_status = 1;
		}
	}
	else if(pCtx->sc_wlan_status == 0 && pCtx->sc_status<=2 && pCtx->sc_led_status==1)
	{
		//run_clicmd("echo 0 > /proc/gpio");
		pCtx->sc_led_status = 0;
	}
	else if(pCtx->sc_wlan_status == 1 && pCtx->sc_led_status==0)
	{
		//run_clicmd("echo 1 > /proc/gpio");
		pCtx->sc_led_status = 1;
	}

	return 0;
}
static int init_config(RTK_SC_CTXp pCtx)
{
	
	unsigned char name[128]={0};
	unsigned char value[128]={0};
	unsigned char line[256]={0};
	unsigned char tmp[128];
	unsigned char *p;
	FILE *fp;
	int i, intVal;
	int mode, disabled, sc_enabled;

	strcpy(pCtx->sc_wlan_ifname, SC_WLAN_IF);
	apmib_save_idx();
	SetWlan_idx(pCtx->sc_wlan_ifname);
	apmib_get(MIB_WLAN_SC_SAVE_PROFILE, (void *)&intVal);
	pCtx->sc_save_profile = intVal;
	apmib_get(MIB_WLAN_SC_PIN_ENABLED, (void *)&intVal);
	pCtx->sc_pin_enabled = intVal;
	apmib_revert_idx();
	strcpy(pCtx->sc_mib_prefix, pCtx->sc_wlan_ifname);
	fp=stdout;
	if((stdout=fopen(SC_FILE_OUTPUT,"w"))==NULL)
	{
		fprintf(stderr,"redirect output fail!\n");
		stdout=fp;
		return -1;
	}	
	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
		sprintf(tmp, "%s mib_staconfig", "wlan0-vxd");
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
		sprintf(tmp, "%s mib_staconfig", "wlan1-vxd");
	else
		sprintf(tmp, "%s mib_staconfig", pCtx->sc_wlan_ifname);
	run_clicmd(tmp);
	
	fclose(stdout);
	stdout=fp;	
	
	sprintf(pCtx->sc_config_file, "%s", SC_FILE_OUTPUT);
	fp = fopen(pCtx->sc_config_file, "r");
	if (fp == NULL) {
		printf("read config file [%s] failed!\n", pCtx->sc_config_file);
		return -1;
	}


	while ( fgets(line, 200, fp) ) {
		if (line[0] == '#')
			continue;
		if(strstr(line, "sc")==0)
			continue;
		
		sscanf(line, "%[^:]",tmp);
		p=tmp;
		while(*p == ' ')
			p++;
		strcpy(name, p);
		
		sscanf(line, "%*[^:]:%[^?]",tmp);
		p=tmp;
		while(*p == ' ')
			p++;
		strcpy(value, p);
		if (!strcmp(name, "scDeviceName")) {
			strcpy(pCtx->sc_device_name, value);
		}
		else if (!strcmp(name, "scDeviceType")) {
			pCtx->sc_device_type = atoi(value);
		}
		else if (!strcmp(name, "scPin")) {
			strcpy(pCtx->sc_pin, value);
		}
		else if (!strcmp(name, "scDefaultPin")) {
			strcpy(pCtx->sc_default_pin, value);
		}
		else if (!strcmp(name, "scSyncVxdToRoot")) {
			pCtx->sc_sync_profile = atoi(value);			
		}
		if(pCtx->sc_debug)
			printf("%s:	%s", name, value);
	}

	fclose(fp);
	return 0;

}


int get_file_value(unsigned char *target, unsigned char *value, unsigned char *cmd)
{
	FILE *fp=stdout;
	unsigned char line[200], name[64], tmp[128];
	unsigned char *p;

	if((stdout=fopen(SC_FILE_OUTPUT,"w"))==NULL)
	{
		fprintf(stderr,"redirect output fail!\n");
		stdout=fp;
		return -1;
	}		
	run_clicmd(cmd);
	
	fclose(stdout);
	stdout=fp;	

	if((fp=fopen(SC_FILE_OUTPUT,"r"))==NULL)
	{
		fprintf(stderr,"open sc temp file fail!\n");
		return -1;
	}	
	while ( fgets(line, 200, fp) ) {
		if (line[0] == '#')
			continue;
		if(strstr(line, target)==0)
			continue;
		
		sscanf(line, "%[^:]",tmp);
		p=tmp;
		while(*p == ' ')
			p++;
		strcpy(name, p);
		
		sscanf(line, "%*[^:]:%[^?]",tmp);
		p=tmp;
		while(*p == ' ')
			p++;
		strcpy(value, p);
		value[strlen(value) - 1] = '\0';
	}
	fclose(fp);
}

int get_device_ip_status()
{	
	return SC_IP_STATUS;
}

int Check_Wlan_isConnected(RTK_SC_CTXp pCtx)
{
	FILE *fp;
	int result=0;
	unsigned char buffer[64], tmp[64];

	fp = stdout;
	
	if((stdout=fopen(SC_FILE_OUTPUT,"w"))==NULL)
	{
		fprintf(stderr,"redirect output fail!\n");
		stdout=fp;
		return -1;
	}	

	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
		sprintf(tmp, "%s sta_info", "wlan0-vxd");
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
		sprintf(tmp, "%s sta_info", "wlan1-vxd");
	else
		sprintf(tmp, "%s sta_info", pCtx->sc_wlan_ifname);
	run_clicmd(tmp);
	
	fclose(stdout);
	stdout=fp;	

	fp = fopen ( SC_FILE_OUTPUT, "r" );
	if ( fp != NULL ) {		
		char *strtmp;
		char line[100];
		while (fgets(line, sizeof(line), fp))
		{
			unsigned char *p;
			strtmp = line;
			while(*strtmp == ' ')
				strtmp++;
				
			if(strstr(strtmp,"active") != 0){
				unsigned char str1[10];
						
				//-- STA info table -- (active: 1)
				sscanf(strtmp, "%*[^:]:%[^)]",str1);
						
				p = str1;
				while(*p == ' ')
					p++;										
				if(strcmp(p,"0") == 0){
					result=0;
				}else{
					result=1;		
				}										
				break;
			}
					
		}
		fclose(fp );
				
	}
	return result;
}

int get_sc_status(RTK_SC_CTXp pCtx)
{
	unsigned char buffer[64];
	unsigned char value[64];
	unsigned char *p;

	
	
	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
		sprintf(buffer, "%s mib_staconfig", "wlan0-vxd");
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
		sprintf(buffer, "%s mib_staconfig", "wlan1-vxd");
	else
		sprintf(buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname);
	//run_clicmd(buffer);
	get_file_value("scStatus", value, buffer);
	pCtx->sc_status = atoi(value);

	if((pCtx->sc_status >= 10) && (pCtx->sc_control_ip == 0))
	{
		//sprintf(buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname);
		if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
			sprintf(buffer, "%s mib_staconfig", "wlan0-vxd");
		else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
			sprintf(buffer, "%s mib_staconfig", "wlan1-vxd");
		else
			sprintf(buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname);
		//run_clicmd(buffer);
		get_file_value("scControlIP", value, buffer);
		string_to_hex(value, (unsigned char *)&pCtx->sc_control_ip, 8);
		if(pCtx->sc_debug)
			diag_printf("the control IP is %x, value is %s\n", pCtx->sc_control_ip, value);
	}
	return 0;
}


int is_valid_control_pkt(unsigned char *nonce, unsigned char *digest, unsigned char *key, unsigned int key_len, RTK_SC_CTXp pCtx)
{
	unsigned char buffer[256];
	struct sockaddr hw_addr;
	unsigned char md5_digest[16];
	int len=0;
	int i=0;
	memset(buffer,0x0,256);
	MD5_CTX md5;

	memcpy(buffer, nonce, SC_NONCE_LEN);
	len+=SC_NONCE_LEN;
/*
	printf("the nonce is ");
	for(i=0; i<len; i++)
		printf("%02x", buffer[i]);
	printf("\n");
*/
	memcpy(buffer+len, key, key_len);
	len+=key_len;
/*
	printf("the buf is ");
	for(i=0; i<len; i++)
		printf("%02x", buffer[i]);
	printf("\n");
*/
	wlan_MD5_Init(&md5);
	wlan_MD5_Update(&md5,buffer,len);
	wlan_MD5_Final(md5_digest,&md5);

/*
	printf("the digest is ");
	for(i=0; i<16; i++)
		printf("%02x", md5_digest[i]);
	printf("\n");
*/
	if(memcmp(digest, md5_digest, 16) == 0)
		return 1;
	else
		return 0;
}

int get_random_ssid(RTK_SC_CTXp pCtx)
{
	int i=0;
	srand( (unsigned)time( NULL ) );
	for(i=0; i<16; i++)
	{
		sprintf(&(pCtx->sc_ssid[i]), "%d", (rand()%10));
	}
	pCtx->sc_ssid[16] = '\0';
	return 0;
}

int send_connect_ack(RTK_SC_CTXp pCtx)
{
	int i=0;
	struct ack_msg ack_msg;
	unsigned int intaddr;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	unsigned char *p;
	int sockfd_ack;
	struct sockaddr_in control_addr;
	int addr_len;
	struct ifreq interface;
	unsigned char ifname[16];

	memset(&ack_msg, 0, sizeof(struct ack_msg));
	
	pCtx->sc_ip_status = get_device_ip_status();

	if(pCtx->sc_ip_status != SC_DHCP_GETTING_IP)
	{
		if(1)//if(is_bridge_if(pCtx))
		{
			getInAddr("eth0", IP_ADDR, (void *)&intaddr);
			strcpy(ifname, "eth0");
		}
		else
		{
			getInAddr(pCtx->sc_wlan_ifname, IP_ADDR, (void *)&intaddr);
		}
	}
	else
	{
		intaddr = 0;
		if(pCtx->sc_send_ack == SC_SUCCESS_IP)
			return 0;
	}

	if ((sockfd_ack = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(1);
	}

	#if 0
	strncpy(interface.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ);
	if (setsockopt(sockfd_ack, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface)) < 0) {
		perror("setsockopt - SO_BINDTODEVICE");
	}
	#endif
	

	bzero(&control_addr,sizeof(struct sockaddr_in)); 
	control_addr.sin_family = AF_INET;         		
	control_addr.sin_port = htons(ACK_DEST_PORT);     	
	control_addr.sin_addr.s_addr = htonl(pCtx->sc_control_ip);


	if ( getInAddr(pCtx->sc_wlan_ifname, HW_ADDR, (void *)&hwaddr ) )
	{
		pMacAddr = (unsigned char *)hwaddr.sa_data;
		for(i=0; i<6; i++)
			ack_msg.smac[i] = pMacAddr[i];
	}
	
	ack_msg.flag = SC_RSP_ACK;
	ack_msg.length = htons(sizeof(struct ack_msg)-3);
	ack_msg.device_type = htons(0);
	ack_msg.device_ip = intaddr;
	memset(ack_msg.device_name, 0, 64);
	strcpy(ack_msg.device_name, pCtx->sc_device_name);
	ack_msg.pin_enabled = pCtx->sc_pin_enabled;
	
	if(pCtx->sc_debug == 2)
	{
		p=&ack_msg;
		diag_printf("the ack from application is:\n");
		for(i=0; i<sizeof(struct ack_msg); i++)
			diag_printf("%02x", p[i]);
		diag_printf("\n");
	}
	
	addr_len = sizeof(struct sockaddr);
	for(i=0; i<SC_ACK_ROUND;i++)
	{
		sendto(sockfd_ack,(unsigned char *)&ack_msg,sizeof(struct ack_msg),0,(struct sockaddr *)&control_addr,addr_len);
	}

	close(sockfd_ack);
	
	return 0;
	
}

int set_profile_to_flash(RTK_SC_CTXp pCtx, unsigned int sync_to_vxd)
{
	unsigned char buffer[128]={0}, cmd_buffer[128]={0};
	unsigned char value[128];
	unsigned char tmp[128];
	unsigned char *p;
	unsigned int security_type=0;
	unsigned int wsc_auth=0, wsc_enc=0;
	unsigned char wsc_psk[65] = {0};
	int intVal=0;
	apmib_save_idx();
		

	security_type = pCtx->sc_status - 10;
	
	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
	{
		sprintf(cmd_buffer, "%s mib_staconfig", "wlan0-vxd");
		if(sync_to_vxd == 1)
			SetWlan_idx("wlan0");
		else
			SetWlan_idx(pCtx->sc_wlan_ifname);
	}
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
	{
		sprintf(cmd_buffer, "%s mib_staconfig", "wlan1-vxd");
		if(sync_to_vxd == 1)
			SetWlan_idx("wlan1");
		else
			SetWlan_idx(pCtx->sc_wlan_ifname);
	}
	else
	{
		sprintf(cmd_buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname);
		SetWlan_idx(pCtx->sc_wlan_ifname);
	}

	//sprintf(cmd_buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname, SC_SECURITY_FILE);
	get_file_value("scPassword", value, cmd_buffer);
	strcpy(pCtx->sc_passwd, value);
	if(pCtx->sc_debug)
		diag_printf("the password is %s\n", pCtx->sc_passwd);
	apmib_set(MIB_WLAN_SC_PASSWD, pCtx->sc_passwd);
	memset(value, 0, 128);
	memset(pCtx->sc_ssid, 0, 32);
	//sprintf(cmd_buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname, SC_SECURITY_FILE);
	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
		sprintf(cmd_buffer, "%s mib_staconfig", "wlan0-vxd");
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
		sprintf(cmd_buffer, "%s mib_staconfig", "wlan1-vxd");
	else
		sprintf(cmd_buffer, "%s mib_staconfig", pCtx->sc_wlan_ifname);
	get_file_value("scSSID", value, cmd_buffer);
	diag_printf("save profile to flash, the ssid is %s\n", value);
	strcpy(pCtx->sc_ssid, value);
	 
	if(pCtx->sc_debug)
		diag_printf("the SSID is %s, the length is %d\n", pCtx->sc_ssid, strlen(pCtx->sc_ssid));

	apmib_set(MIB_WLAN_SSID, pCtx->sc_ssid);

	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd"))
	{
		apmib_set(MIB_REPEATER_SSID1, pCtx->sc_ssid);
	}
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd"))
	{
		apmib_set(MIB_REPEATER_SSID2, pCtx->sc_ssid);
	}

#if defined(CONFIG_APP_WSC)
	apmib_set(MIB_WLAN_WSC_SSID, pCtx->sc_ssid);
#endif
	
	if(pCtx->sc_debug)
		diag_printf("the security type is %d\n", security_type);
	
	switch(security_type)
	{
		case 0:
			intVal = ENCRYPT_DISABLED;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			wsc_enc = 0;
			wsc_auth= WSC_AUTH_OPEN;
			break;
		case 1:
			intVal = ENCRYPT_WEP;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WEP64;
			apmib_set(MIB_WLAN_WEP, (void *)&intVal);
			sprintf(value, "%02x%02x%02x%02x%02x", pCtx->sc_passwd[0], pCtx->sc_passwd[1], pCtx->sc_passwd[2], pCtx->sc_passwd[3],pCtx->sc_passwd[4]);
			value[10] = 0;
			apmib_set(MIB_WLAN_WEP64_KEY1, value);
			intVal = KEY_ASCII;
			apmib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal);
			intVal = AUTH_BOTH;
			apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&intVal);
			break;
		case 2:
			intVal = ENCRYPT_WEP;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WEP64;
			apmib_set(MIB_WLAN_WEP, (void *)&intVal);
			apmib_set(MIB_WLAN_WEP64_KEY1, pCtx->sc_passwd);
			intVal = KEY_HEX;
			apmib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal);
			intVal = AUTH_BOTH;
			apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&intVal);	
			break;
		case 3:
			intVal = ENCRYPT_WEP;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WEP128;
			apmib_set(MIB_WLAN_WEP, (void *)&intVal);
			sprintf(value, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
				pCtx->sc_passwd[0], pCtx->sc_passwd[1], pCtx->sc_passwd[2], pCtx->sc_passwd[3],pCtx->sc_passwd[4], pCtx->sc_passwd[5], pCtx->sc_passwd[6],
				pCtx->sc_passwd[7], pCtx->sc_passwd[8],pCtx->sc_passwd[9], pCtx->sc_passwd[10], pCtx->sc_passwd[11],pCtx->sc_passwd[12]
				);
			value[26] = 0;
			apmib_set(MIB_WLAN_WEP128_KEY1, value);
			intVal = KEY_ASCII;
			apmib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal);
			intVal = AUTH_BOTH;
			apmib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal);	
			break;
		case 4:
			intVal = ENCRYPT_WEP;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WEP128;
			apmib_set(MIB_WLAN_WEP, (void *)&intVal);
			apmib_set(MIB_WLAN_WEP128_KEY1, pCtx->sc_passwd);
			intVal = KEY_HEX;
			apmib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal);
			intVal = AUTH_BOTH;
			apmib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal);	
			break;
		case 5:
			intVal = ENCRYPT_WPA2;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WPA_AUTH_PSK;
			apmib_set(MIB_WLAN_WPA_AUTH, (void *)&intVal);
			intVal = WPA_CIPHER_AES;
			apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
			intVal = KEY_ASCII;
			apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&intVal);
			apmib_set(MIB_WLAN_WPA_PSK, pCtx->sc_passwd);
			wsc_enc = WSC_ENCRYPT_AES;
			wsc_auth = WSC_AUTH_WPA2PSK;
			break;
		case 6:
			intVal = ENCRYPT_WPA2;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WPA_AUTH_PSK;
			apmib_set(MIB_WLAN_WPA_AUTH, (void *)&intVal);
			intVal = WPA_CIPHER_TKIP;
			apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
			intVal = KEY_ASCII;
			apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&intVal);
			apmib_set(MIB_WLAN_WPA_PSK, pCtx->sc_passwd);
			wsc_enc = WSC_ENCRYPT_TKIP;
			wsc_auth = WSC_AUTH_WPA2PSK;
			break;
		case 7:
			intVal= ENCRYPT_WPA;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WPA_AUTH_PSK;
			apmib_set(MIB_WLAN_WPA_AUTH, (void *)&intVal);
			intVal = WPA_CIPHER_AES;
			apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
			intVal= KEY_ASCII;
			apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&intVal);
			apmib_set(MIB_WLAN_WPA_PSK, pCtx->sc_passwd);
			wsc_enc = WSC_ENCRYPT_AES;
			wsc_auth = WSC_AUTH_WPAPSK;
			break;
		case 8:
			intVal= ENCRYPT_WPA;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			intVal = WPA_AUTH_PSK;
			apmib_set(MIB_WLAN_WPA_AUTH, (void *)&intVal);
			intVal = WPA_CIPHER_TKIP;
			apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
			intVal = KEY_ASCII;
			apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&intVal);
			apmib_set(MIB_WLAN_WPA_PSK, pCtx->sc_passwd);
			wsc_enc = WSC_ENCRYPT_TKIP;
			wsc_auth = WSC_AUTH_WPAPSK;
			break;
		default:
			intVal= ENCRYPT_WPA;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&intVal);
			break;
	}
	intVal = 2;
	apmib_set(MIB_WLAN_SC_SAVE_PROFILE, (void *)&intVal);
#if defined(CONFIG_APP_WSC)
	apmib_set(MIB_WLAN_WSC_ENC, &wsc_enc);
	apmib_set(MIB_WLAN_WSC_AUTH, &wsc_auth);
	if(wsc_enc >= 2)
	{
		apmib_set(MIB_WLAN_WSC_PSK, pCtx->sc_passwd);
	}
#endif

	pCtx->sc_save_profile = 2;
	
	apmib_update(CURRENT_SETTING);
	apmib_revert_idx();
	return 1;
	
}

int sync_vxd_to_root(RTK_SC_CTXp pCtx)
{
	unsigned char buffer[128], ifname[16];
	unsigned int security_type;

	kick_event(1<<13);
	return 0;

	security_type = pCtx->sc_status-10;
	if(strstr(pCtx->sc_wlan_ifname, "wlan0-vxd0"))
		strcpy(ifname, "wlan0");
	else if(strstr(pCtx->sc_wlan_ifname, "wlan1-vxd0"))
		strcpy(ifname, "wlan1");
	sprintf(buffer, "ifconfig %s down", pCtx->sc_wlan_ifname);
	system(buffer);
	sprintf(buffer, "ifconfig %s down", ifname);
	system(buffer);
	sprintf(buffer, "iwpriv %s set_mib ssid=\"%s\"", ifname, pCtx->sc_ssid);
	system(buffer);
	sprintf(buffer, "iwpriv %s set_mib sc_enabled=0", pCtx->sc_wlan_ifname);
	system(buffer);

	switch(security_type)
	{
		case 0:
			sprintf(buffer, "iwpriv %s set_mib encmode=0", ifname);
			system(buffer);

			break;
		case 1:
			sprintf(buffer, "iwpriv %s set_mib encmode=1", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wepkey1=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 2:
			sprintf(buffer, "iwpriv %s set_mib encmode=1", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wepkey1=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 3:
			sprintf(buffer, "iwpriv %s set_mib encmode=5", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wepkey1=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 4:
			sprintf(buffer, "iwpriv %s set_mib encmode=5", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wepkey1=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 5:
			//WPA2 AES
			sprintf(buffer, "iwpriv %s set_mib encmode=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib psk_enable=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wpa2_cipher=8", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib passphrase=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 6:
			sprintf(buffer, "iwpriv %s set_mib encmode=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib psk_enable=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wpa2_cipher=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib passphrase=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 7:
			sprintf(buffer, "iwpriv %s set_mib encmode=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib psk_enable=1", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wpa_cipher=8", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib passphrase=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		case 8:
			sprintf(buffer, "iwpriv %s set_mib encmode=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib psk_enable=1", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib wpa_cipher=2", ifname);
			system(buffer);
			sprintf(buffer, "iwpriv %s set_mib passphrase=\"%s\"", ifname, pCtx->sc_passwd);
			system(buffer);
			break;
		default:
			sprintf(buffer, "iwpriv wlan0 set_mib encmode=0", ifname);
			system(buffer);
			break;
	}
	sprintf(buffer, "ifconfig %s up", ifname);
	system(buffer);
	sprintf(buffer, "ifconfig %s up", pCtx->sc_wlan_ifname);
	system(buffer);
	//don't restart simple config when up interface again.
	pCtx->sc_run_time = 0;
	
	return 0;
}

int simple_config_main(int argc, char *argv[])
{
	int i=0, ret=0, start_sc=0, accept_control=0, on=1, reinit=0, wps_sc_concurrent=0, removed=0;
	int reinit_flag =0;
	int sockfd_scan, sockfd_control;                     				// socket descriptors
	struct sockaddr_in device_addr;     		// my address information
	struct sockaddr_in control_addr;  			// connector¡¦s address information
	int addr_len, numbytes;
	FILE *fp;
	fd_set fds;	
	int max_fd, selret;
	unsigned char buf[256];
	RTK_SC_CTXp pCtx;
	unsigned char *p;
	struct timeval timeout, begin_time, current_time; 
	struct ack_msg ack_msg;
	struct response_msg res_msg;
	int configured=0;
	int link_time=0;

	pCtx = &g_sc_ctx;
	pCtx->sc_debug = 0;
	pCtx->sc_led_enabled = 1;

	if( init_config(pCtx)!= 0)
		exit(1);
	
#if 1	
	if ((sockfd_scan = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(1);
	}

	//ret = setsockopt( sockfd_scan, SOL_SOCKET, SO_BROADCAST|SO_REUSEADDR, &on, sizeof(on) );

	bzero(&device_addr,sizeof(struct sockaddr_in)); 
	device_addr.sin_family = AF_INET;         		// host byte order
	device_addr.sin_port = htons(18864);     	// short, network byte order
	device_addr.sin_addr.s_addr = INADDR_ANY;// automatically fill with my IP

	// bind the socket with the address
	if (bind(sockfd_scan, (struct sockaddr *)&device_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind");
		close(sockfd_scan);
		exit(1);
	}
	
#if 1
	if ((sockfd_control = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(1);
	}
	//ret = setsockopt( sockfd_control, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	bzero(&device_addr,sizeof(struct sockaddr_in)); 
	device_addr.sin_family = AF_INET;         		// host byte order
	device_addr.sin_port = htons(ACK_DEST_PORT);     	// short, network byte order
	device_addr.sin_addr.s_addr = INADDR_ANY;// automatically fill with my IP

	// bind the socket with the address
	if (bind(sockfd_control, (struct sockaddr *)&device_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind");
		close(sockfd_scan);
		exit(1);
	}
#endif	

	addr_len = sizeof(struct sockaddr);

	pCtx->sc_run_time=0;
	pCtx->sc_pbc_duration_time = 0;

	if(pCtx->sc_save_profile !=2 )
	{
		sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
		run_clicmd(buf);
		sleep(1);		
	}
	else
	{
		configured = 1;
		pCtx->sc_config_success = 1;
		sprintf(buf, "iwpriv %s set_mib sc_status=0", pCtx->sc_wlan_ifname);
		run_clicmd(buf);
	}

	pCtx->sc_ip_status = get_device_ip_status();

	while(1) {
#ifdef HAVE_SYSTEM_REINIT
		if(sc_quitting==1)
		{
			close(sockfd_scan);
			close(sockfd_control);
			break;
		}
#endif
		pCtx->sc_run_time++;
		pCtx->sc_wlan_status = Check_Wlan_isConnected(pCtx);
		get_sc_status(pCtx);		
		if((pCtx->sc_wlan_status == 0) && (pCtx->sc_status == 0))
		{
			sprintf(buf, "ifconfig %s down", pCtx->sc_wlan_ifname);
			run_clicmd(buf);
			sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
			run_clicmd(buf);
			sprintf(buf, "iwpriv %s set_mib sc_status=0", pCtx->sc_wlan_ifname);
			run_clicmd(buf);
			sprintf(buf, "ifconfig %s up", pCtx->sc_wlan_ifname);
			run_clicmd(buf);

			pCtx->sc_save_profile = 0;
			if(pCtx->sc_debug)
				diag_printf("Simple Config enter monitor mode\n");
			sleep(2);
		}
		
		if(1)//(pCtx->sc_wlan_status == 1)
		{
			if(pCtx->sc_led_enabled)
			{
				timeout.tv_sec = 0;
				timeout.tv_usec = 250000;
			}
			else
			{
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
			}
			max_fd = 0;
			FD_ZERO(&fds);
			FD_SET(sockfd_scan, &fds);
			FD_SET(sockfd_control, &fds);

			max_fd = (sockfd_control > sockfd_scan) ? sockfd_control : sockfd_scan;
			selret = select(max_fd+1, &fds, NULL, NULL, &timeout);
			
			if (selret && FD_ISSET(sockfd_scan, &fds)) 
			{
				struct scan_msg *pMsg;
				unsigned int intaddr;
				struct sockaddr hwaddr;
				unsigned char *pMacAddr;
				
				memset(buf, 0, 256);
				if (((numbytes = recvfrom(sockfd_scan, buf, 256, 0,
					(struct sockaddr *)&control_addr, &addr_len)) == -1) && (pCtx->sc_wlan_status == 1)) {
					fprintf(stderr,"Receive scan packet failed!!!\n");
					close(sockfd_scan);
					exit(1);
				}
				else if(pCtx->sc_wlan_status == 1)
				{
					control_addr.sin_port = htons(ACK_DEST_PORT); 
					pMsg = (struct scan_msg *)buf;
					switch(pMsg->flag)
					{
						case SC_SUCCESS_ACK:
							if(pCtx->sc_send_ack == SC_SUCCESS_ACK)
							{
								if(pCtx->sc_ip_status == SC_DHCP_GETTING_IP)
								{
									if(pCtx->sc_debug)
										diag_printf("receive config success ack, but the device haven't get IP now!\n");
									pCtx->sc_send_ack = SC_SUCCESS_IP;
								}
							}
#if 0
							if((pCtx->sc_send_ack == SC_SUCCESS_ACK) || (pCtx->sc_send_ack == SC_SUCCESS_IP))
							{
								if(pCtx->sc_ip_status != SC_DHCP_GETTING_IP)
								{
									if(pCtx->sc_debug)
										diag_printf("receive config success ack\n");
									//if(pCtx->sc_save_profile == 0)
										pCtx->sc_save_profile = 1;
									pCtx->sc_send_ack = 0;
								}
							}
#endif
							break;
						case SC_SCAN:
							if(pCtx->sc_save_profile != 2)
							{
								if(pCtx->sc_debug == 3)
								{
									diag_printf("receive scan message, don't send reply before the setting is saved to flash.\n");
								}
								break;
							}
							if(pCtx->sc_debug == 2)
								diag_printf("receive scan from %s\n ", inet_ntoa( control_addr.sin_addr));
							pCtx->sc_ip_status = get_device_ip_status();
							if(pCtx->sc_ip_status != SC_DHCP_GETTING_IP)
							{
								if(1)//(is_bridge_if(pCtx))
									getInAddr("eth0", IP_ADDR, (void *)&intaddr);
								else
									getInAddr(pCtx->sc_wlan_ifname, IP_ADDR, (void *)&intaddr);
							}
							else
							{
								intaddr = 0;
								break;
							}

							if ( getInAddr(pCtx->sc_wlan_ifname, HW_ADDR, (void *)&hwaddr ) )
							{
								pMacAddr = (unsigned char *)hwaddr.sa_data;
								for(i=0; i<6; i++)
									ack_msg.smac[i] = pMacAddr[i];
							}

							ack_msg.flag = SC_RSP_SCAN;
							ack_msg.length = htons(sizeof(struct ack_msg)-3);
							ack_msg.status = 1;
							if(pMsg->sec_level != 0)
							{
								if(is_valid_control_pkt(pMsg->nonce, pMsg->digest, pCtx->sc_default_pin, 8, pCtx) == 0)
								{
									ack_msg.status = 0;
									diag_printf("the scan info is invalid!\n");
								}
							}
							ack_msg.device_type = htons(0);
							ack_msg.device_ip = intaddr;
							memset(ack_msg.device_name, 0, 64);
							strcpy(ack_msg.device_name, pCtx->sc_device_name);
							ack_msg.pin_enabled = pCtx->sc_pin_enabled;
							if(pCtx->sc_debug == 2)
							{
								p=&ack_msg;
								diag_printf("the response packet for scan is:\n",removed);
								for(i=0; i<sizeof(struct ack_msg); i++)
									diag_printf("%02x", p[i]);
								diag_printf("\n");
							}
#if 1
							// STA is not removed when receive scan_message, then no reply.
							if ( !removed && res_msg.flag == SC_RSP_DEL )
							{
								if(pCtx->sc_debug == 2)
									diag_printf("receive remove success ack, but not removed really!\n");
								break;
							}
#endif
							for(i=0; i<SC_ACK_ROUND;i++)
							{
								sendto(sockfd_scan,(unsigned char *)&ack_msg,sizeof(struct ack_msg),0,(struct sockaddr *)&control_addr,addr_len);
							}
							break;
						default:
							diag_printf("invalid request\n");
							break;
					}
				}
				else
				{
					if(pCtx->sc_debug == 2)
						diag_printf("receive scan info when wlan is disconnect. this packets is queued by socket\n");
				}
			}
			
			if (selret && FD_ISSET(sockfd_control, &fds) ) 
			{
				//receive the command from the client
				unsigned char status;
				struct request_msg *pMsg;

				memset(buf, 0, 256);
				if ((numbytes = recvfrom(sockfd_control, buf, 256, 0,
					(struct sockaddr *)&control_addr, &addr_len)) == -1  && (pCtx->sc_wlan_status == 1)) {
					fprintf(stderr,"Receive control packet failed!!!\n");
					close(sockfd_control);
					
					exit(1);
				}
				else if(pCtx->sc_wlan_status == 1)
				{
					control_addr.sin_port = htons(ACK_DEST_PORT); 
					pMsg = (struct request_msg *)buf;
					pMsg->length = ntohs(pMsg->length);
					if(accept_control == 1)
						memset(&res_msg, 0, sizeof(struct response_msg));
					
					if(pCtx->sc_debug == 2)
					{
						diag_printf("the receiv control buf is ", buf);
						for(i=0; i<numbytes; i++)
							diag_printf("%02x", buf[i]);
						diag_printf("\n");
						diag_printf("the flag is %d, the length is %d\n",pMsg->flag, pMsg->length);
					}
					switch(pMsg->flag)
					{
						case SC_SAVE_PROFILE:
							if(accept_control == 1)
							{
								if(pCtx->sc_debug)
									diag_printf("receive info to save profile!\n");
								gettimeofday(&begin_time, NULL);
								res_msg.flag = SC_RSP_SAVE;
								res_msg.status = 1;
								if(pMsg->sec_level != 0)
								{
									if(is_valid_control_pkt(pMsg->nonce, pMsg->digest1, pCtx->sc_default_pin, 8, pCtx) == 0)
									{
										res_msg.status = 0;
										diag_printf("the control info is invalid!\n");
									}
									else if(pCtx->sc_pin_enabled == 1)
									{
										if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_pin, 8, pCtx) == 0)
										{
											res_msg.status = 0;
											if(pCtx->sc_debug)
												diag_printf("the control info is invalid!\n");
										}
									}
									else if(pCtx->sc_pin_enabled == 0)
									{
										if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_default_pin, 8, pCtx) == 0)
										{
											res_msg.status = 0;
											if(pCtx->sc_debug)
												diag_printf("the control info is invalid!\n");
										}
									}
								}
								if(res_msg.status != 0)
								{
									
									if(pCtx->sc_debug)
										diag_printf("save profile to flash\n");
									accept_control = 0;

									pCtx->sc_send_ack = SC_SAVE_PROFILE;
								}
							}
							break;
						case SC_DEL_PROFILE:
							if(pCtx->sc_save_profile != 2)
								break;
							
							if(accept_control == 1)
							{
								if(pCtx->sc_debug)
									diag_printf("receive info to remove device!\n");
								gettimeofday(&begin_time, NULL);
								res_msg.status = 1;
								res_msg.flag = SC_RSP_DEL;
								if(pMsg->sec_level != 0)
								{
									if(is_valid_control_pkt(pMsg->nonce, pMsg->digest1, pCtx->sc_default_pin, 8, pCtx) == 0)
									{
										res_msg.status = 0;
										if(pCtx->sc_debug)
											diag_printf("the control info delete is invalid!\n");
									}
									else if(pCtx->sc_pin_enabled == 1)
									{
										if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_pin, 8, pCtx) == 0)
										{
											res_msg.status = 0;
											if(pCtx->sc_debug)
												diag_printf("the control info delete is invalid!\n");
										}
									}
									else if(pCtx->sc_pin_enabled == 0)
									{
										if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_default_pin, 8, pCtx) == 0)
										{
										res_msg.status = 0;
										if(pCtx->sc_debug)
												diag_printf("the control info delete is invalid!\n");
										}
									}
								}
								if(res_msg.status != 0)
								{
								
									accept_control = 0;
									pCtx->sc_send_ack = SC_DEL_PROFILE;
									removed = 0;
									if(pCtx->sc_sync_profile && strstr(pCtx->sc_wlan_ifname, "vxd"))
										reinit = SC_REINIT_SYSTEM;
									else
										reinit = SC_REINIT_WLAN;
								}
								
							}
							
							break;
						case SC_RENAME:
							if(pCtx->sc_save_profile != 2)
								break;
							if(accept_control == 1)
							{
								if(pCtx->sc_debug)
									diag_printf("receive info to rename device\n");
								gettimeofday(&begin_time, NULL);
								res_msg.status = 1;
								res_msg.flag = SC_RSP_RENAME;
								if(pMsg->sec_level != 0)
								{
									if(is_valid_control_pkt(pMsg->nonce, pMsg->digest1, pCtx->sc_default_pin, 8, pCtx) == 0)
									{
										res_msg.status = 0;
										diag_printf("the control info rename is invalid!\n");
									}
									else if(pCtx->sc_pin_enabled == 1)
									{
										if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_pin, 8, pCtx) == 0)
										{
											res_msg.status = 0;
											if(pCtx->sc_debug)
												diag_printf("the control info rename is invalid!%d\n", __LINE__);
										}
									}
									else if(pCtx->sc_pin_enabled == 0)
									{
										if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_default_pin, 8, pCtx) == 0)
										{
										res_msg.status = 0;
											if(pCtx->sc_debug)
												diag_printf("the control info rename is invalid!%d\n", __LINE__);
										}
									}
								}
								if(res_msg.status != 0)
								{
									accept_control = 0;
									
									strcpy(pCtx->sc_device_name, pMsg->device_name);
									pCtx->sc_send_ack = SC_RENAME;
									if(pCtx->sc_debug)
										diag_printf("the new device name is %s\n", pCtx->sc_device_name);
								}
							}
							break;
						case SC_SUCCESS_ACK:
							if((pCtx->sc_send_ack == SC_SUCCESS_ACK) || (pCtx->sc_send_ack == SC_SUCCESS_IP))
							{
								if(pCtx->sc_ip_status != SC_DHCP_GETTING_IP)
								{
									if(pCtx->sc_debug)
										diag_printf("receive connect success ack and can control device now!\n");
									pCtx->sc_save_profile = 1;
									pCtx->sc_send_ack = 0;
									pCtx->sc_config_success = 1;									
									break;
								}
							}
							if(pMsg->sec_level != 0)
							{
								if(is_valid_control_pkt(pMsg->nonce, pMsg->digest1, pCtx->sc_default_pin, 8, pCtx) == 0)
								{
									res_msg.status = 0;
									diag_printf("the control info success is invalid!\n");
								}
								else if(pCtx->sc_pin_enabled == 1)
								{
									if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_pin, 8, pCtx) == 0)
									{
										res_msg.status = 0;
										if(pCtx->sc_debug)
											diag_printf("the control info success is invalid!\n");
									}
								}
								else if(pCtx->sc_pin_enabled == 0)
								{
									if(is_valid_control_pkt(pMsg->nonce, pMsg->digest2, pCtx->sc_default_pin, 8, pCtx) == 0)
									{
									res_msg.status = 0;
										if(pCtx->sc_debug)
											diag_printf("the control info success is invalid!\n");
									}
								}
							}
							
							if(res_msg.status != 0)
							{
							
								if(pCtx->sc_debug == 2)
									diag_printf("receive ack type is %d, response ack type is %d\n", pMsg->device_name[0],  pCtx->sc_send_ack);
								if(pMsg->device_name[0] == pCtx->sc_send_ack)
								{
									pCtx->sc_config_success = 1;
									if(pCtx->sc_debug)
										diag_printf("receive control success ack, type is %d\n", pCtx->sc_send_ack);
#if 0
									if(reinit != 0) // for remove
									{
										sprintf(buf, "ifconfig %s down", pCtx->sc_wlan_ifname);
										run_clicmd(buf);
										if(pCtx->sc_debug == 2)
											diag_printf("down interface for reinit system!\n");
									}
#endif
								}
							}
							
							break;							
						default:
							//if(pCtx->sc_debug)
							//	diag_printf("invalid request\n");
							res_msg.flag = SC_RSP_INVALID;
							status = 0;
							break;
					}

					// when receive rename/remove pkt, but not receive rename/remove success ack
					// or
					// config success ack(DUT got ip)
					
					if((pCtx->sc_send_ack && (pCtx->sc_config_success==0)) || (res_msg.status== 0)) 
					{					
						res_msg.length = htons(sizeof(struct response_msg)-3);
						p= &res_msg;
						
						if(pCtx->sc_debug == 2)
						{
							diag_printf("response----");
							for(i=0; i<sizeof(struct response_msg); i++)
								diag_printf("%02x", p[i]);
							diag_printf("\n");
						}
						for(i=0; i<SC_ACK_ROUND;i++)
						{
							sendto(sockfd_control,(unsigned char *)&res_msg,sizeof(struct response_msg),0,(struct sockaddr *)&control_addr,addr_len);
						}
					}

				}
				else
				{
					if(pCtx->sc_debug == 2)
						diag_printf("receive control info when wlan is disconnect. this packets is queued by socket\n");
				}
			}
			else
			{
				//try to receive all packets from socket and then receive anothor control device info.
				accept_control = 1;			
				
			}
			if(accept_control == 1) 
			{				
				if(pCtx->sc_send_ack == SC_RENAME || pCtx->sc_send_ack==SC_DEL_PROFILE)
				{
					gettimeofday(&current_time, NULL);
					//diag_printf("=============%s-%d during time = %d\n",__FUNCTION__,__LINE__,current_time.tv_sec-begin_time.tv_sec);
					if((current_time.tv_sec-begin_time.tv_sec)>=3 && pCtx->sc_config_success != 1)
					{
						pCtx->sc_config_success = 1;
						if(pCtx->sc_debug)
							diag_printf("auto do control command after receive control info enough time.\n");
					}
				}	
				if(pCtx->sc_config_success == 1)
				{
					switch(pCtx->sc_send_ack)
					{
						case SC_SAVE_PROFILE:
							set_profile_to_flash(pCtx, 0);
							if(strstr(pCtx->sc_wlan_ifname, "vxd") && (pCtx->sc_sync_profile))
							{
								set_profile_to_flash(pCtx, 1);
							}
							break;								
						case SC_DEL_PROFILE:
							pCtx->sc_ip_status = get_device_ip_status();
							if(pCtx->sc_ip_status != SC_DHCP_STATIC_IP)
							{
								pCtx->sc_ip_status = SC_DHCP_GETTING_IP;
								//sprintf(buf, "echo 0 > %s", SC_IP_STATUS_FILE);
								//run_clicmd(buf);
								if(pCtx->sc_debug)
									diag_printf("set sc ip status to getting IP!\n");
							}
							get_random_ssid(pCtx);
							apmib_save_idx();
							SetWlan_idx(pCtx->sc_wlan_ifname);							
							apmib_set(MIB_WLAN_SSID, pCtx->sc_ssid); // set root ssid or vxd ssid		
							if (strstr(pCtx->sc_wlan_ifname, "vxd")){
								// set vxd ssid of UI
								if (apmib_get_wlanidx() == 0)
									apmib_set( MIB_REPEATER_SSID1, pCtx->sc_ssid);
								else
									apmib_set( MIB_REPEATER_SSID2, pCtx->sc_ssid);
							}
							i = ENCRYPT_DISABLED;
							apmib_set(MIB_WLAN_ENCRYPT, (void *)&i);
							i = 0;
							apmib_set(MIB_WLAN_SC_SAVE_PROFILE, (void *)&i);							
							if(pCtx->sc_sync_profile && strstr(pCtx->sc_wlan_ifname, "vxd"))
							{							
								// change interface to root
								diag_printf("[### delete ]: sync vxd setting to root interface now\n");
								if (strstr(pCtx->sc_wlan_ifname, "wlan0"))
									SetWlan_idx("wlan0");
								else if (strstr(pCtx->sc_wlan_ifname, "wlan1"))
									SetWlan_idx("wlan1");
								
								apmib_set(MIB_WLAN_SSID,"RTK 11n AP");						
								i = ENCRYPT_DISABLED;
								apmib_set(MIB_WLAN_ENCRYPT,(void *)&i);			
							}
							apmib_update(CURRENT_SETTING);
							apmib_revert_idx();
							
							removed = 1;							
							if ( pCtx->sc_debug == 2 )
								diag_printf("accept_control=1, then Remove_end.\n");
							break;
						case SC_RENAME:
							apmib_save_idx();
							SetWlan_idx(pCtx->sc_wlan_ifname);
							apmib_set(MIB_SC_DEVICE_NAME, (void *)pCtx->sc_device_name);
							apmib_update(CURRENT_SETTING);
							apmib_revert_idx();
							sprintf(buf, "iwpriv %s set_mib sc_device_name=\"%s\"", pCtx->sc_wlan_ifname, pCtx->sc_device_name);
							run_clicmd(buf);
							break;
						
					}
					
					if(reinit == SC_REINIT_SYSTEM)
					{
						reinit = 0;
						close(sockfd_scan);
						close(sockfd_control);
						// reboot=> simple config will restart.
						run_clicmd("reboot");
					}
					else if(reinit == SC_REINIT_WLAN)
					{
#if 0
						// if not suspend dhcpc and then resume
						// the first allocated ip will not change
						// then if DUT connected to LAN2, DUT will not be controlled by LAN2 smart phone.
						reinit_flag = 1;
						pCtx->sc_save_profile = 0;
						configured = 0;
						pCtx->sc_ip_status = get_device_ip_status();
						
						if(pCtx->sc_ip_status == SC_DHCP_GETTING_IP || pCtx->sc_ip_status == SC_DHCP_GOT_IP)
						{							
							//close(sockfd_scan);
							//close(sockfd_control);
							//run_clicmd("killall udhcpc");
							if(landhcpc_started && landhcpc_running)
							{
								diag_printf("================> dhcpc cyg_thread_suspend\n");
								cyg_thread_suspend(landhcpc_thread);
								landhcpc_running=0;
							}
						}
#endif
						sprintf(buf, "ifconfig %s down", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "iwpriv %s set_mib sc_pin_enabled=%d", pCtx->sc_wlan_ifname, pCtx->sc_pin_enabled);
						run_clicmd(buf);
						sprintf(buf, "iwpriv %s set_mib ssid=\"%s\"", pCtx->sc_wlan_ifname, pCtx->sc_ssid);
						run_clicmd(buf);
						sprintf(buf, "ifconfig %s up", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						pCtx->sc_status = 0;							
#if 0
						if(pCtx->sc_ip_status == SC_DHCP_GETTING_IP || pCtx->sc_ip_status == SC_DHCP_GOT_IP)
						{								
							//run_clicmd("udhcpc -i br0 -p /etc/udhcpc/udhcpc-br0.pid -s /usr/share/udhcpc/br0.sh &");
							if(!landhcpc_started)
							{
								diag_printf("dhcpc_startup\n");
								landhcpc_startup("eth0",0);
							}else
							{
								if(!landhcpc_running)
								{
									diag_printf("================> dhcpc cyg_thread_resume \n");
									cyg_thread_resume(landhcpc_thread);
									landhcpc_running=1;
									dhcpc_reconnect(1);
								}
							}	
						}
						sleep(2);
#endif
					}

					pCtx->sc_send_ack = 0;
					pCtx->sc_config_success= 0;					
					reinit = 0;
				}

			}
		}
		else
		{
			sleep(1);
		}
	
		if( pCtx->sc_wlan_status== 0 && pCtx->sc_send_ack != SC_DEL_PROFILE ) //wlan  not connected
		{
			pCtx->sc_linked_time= 0;
			pCtx->sc_send_ack = 0;
			pCtx->sc_control_ip = 0;
			link_time = 0;			
			if(pCtx->sc_status >= 10)
			{
				//connect successful by simple config but hasn't save setting to flash, wait more time for disconnect abnormal
				i = 10;
				while(i-- && !Check_Wlan_isConnected(pCtx))
				{
					sleep(1);
				}
				pCtx->sc_wlan_status = Check_Wlan_isConnected(pCtx);
				if(pCtx->sc_wlan_status==0 && reinit==0 && i==0)
				{
					sprintf(buf, "iwpriv %s set_mib sc_status=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					if(pCtx->sc_debug) 
						diag_printf("disconnect after Simple Config success, try to restart simple config\n");
				}
			}
		}
		else if ( pCtx->sc_wlan_status== 1 )
		{		
			if(pCtx->sc_save_profile == 0 && configured == 1 )
			{
				pCtx->sc_save_profile = 2;
			}
			if(link_time++==80)
			{			
				if(pCtx->sc_status ==1 && pCtx->sc_send_ack == 0)
				{
					if(pCtx->sc_debug)
						diag_printf("disable simple config for it has connect to target AP for enough time\n");
					sprintf(buf, "iwpriv %s set_mib sc_status=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib sc_enabled=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					pCtx->sc_save_profile = 2;
				}
			}
			if(pCtx->sc_status >= 10)
			{
				pCtx->sc_linked_time++;
				if(pCtx->sc_linked_time == 1)
				{
					reinit= 0;
					//pCtx->sc_save_profile = 0;
					pCtx->sc_send_ack = SC_SUCCESS_ACK;
					pCtx->sc_config_success = 0;
					if(pCtx->sc_debug)
						diag_printf("try to send driver connect successful ack\n");
					gettimeofday(&begin_time, NULL);
#if 0
					pCtx->sc_ip_status = get_device_ip_status();
					if(pCtx->sc_ip_status != SC_DHCP_STATIC_IP)
					{
						run_clicmd("killall udhcpc");
						run_clicmd("udhcpc -i br0 -p /etc/udhcpc/udhcpc-br0.pid -s /usr/share/udhcpc/br0.sh");
					}
#endif
				}

				// write to flash if possible
				if(pCtx->sc_save_profile==1)
				{					
					set_profile_to_flash(pCtx, 0);
					if(pCtx->sc_sync_profile && strstr(pCtx->sc_wlan_ifname, "vxd"))
					{						
						diag_printf("[### save ]: sync vxd setting to root interface now\n");
						set_profile_to_flash(pCtx, 1);
						reinit = 0;
						close(sockfd_scan);
						close(sockfd_control);
						run_clicmd("reboot");
						//if(pCtx->sc_sync_profile == 2)
						//	sync_vxd_to_root(pCtx);
					}
					pCtx->sc_save_profile = 2;
					sprintf(buf, "iwpriv %s set_mib sc_status=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib sc_enabled=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
				}
				if((pCtx->sc_send_ack == SC_SUCCESS_ACK) || (pCtx->sc_send_ack == SC_SUCCESS_IP))
				{
					if(pCtx->sc_linked_time>=60)
					{
						gettimeofday(&current_time, NULL);
						if((current_time.tv_sec-begin_time.tv_sec)>=60)
						{
							pCtx->sc_save_profile = 1;
							pCtx->sc_send_ack = 0;
							pCtx->sc_config_success = 1;
							if(pCtx->sc_debug) 
								diag_printf("auto save config info to flash after connect success for enough time.\n");
						}
						else
							send_connect_ack(pCtx);
					}
					else
						send_connect_ack(pCtx);
				}
			}
			
			pCtx->sc_pbc_duration_time = 0;
		}

		if(pCtx->sc_led_enabled)
			set_sc_led_status(pCtx);
#if defined(SIMPLE_CONFIG_PBC_SUPPORT)			
		if(1)//(g_sc_connect_status == 0)
		{
			ret = wps_button_pressed();//get_gpio_status();
			if(ret == 1)
			{
			
				if(pCtx->sc_debug)
					diag_printf("GPIO is Pressed\n");
				//run_clicmd("echo 0 > /proc/gpio");
				pCtx->sc_pbc_pressed_time++;
				pCtx->sc_pbc_duration_time = 0;
			}
			if((ret == 0) &&(pCtx->sc_pbc_pressed_time>0))
			{
#if defined(CONFIG_SIMPLE_CONFIG_PBC_SUPPORT)
				if(pCtx->sc_pbc_pressed_time>=3)
					wps_sc_concurrent = 1;
				else 
					wps_sc_concurrent = 0;
#endif
				pCtx->sc_pbc_pressed_time = 0;
				//if(start_sc ==0)
				{
					
					if(pCtx->sc_debug)
						diag_printf("start RTK simple config now\n");
					pCtx->sc_linked_time = 0; //for LED
					pCtx->sc_pbc_duration_time = 1;
					
					pCtx->sc_ip_status = get_device_ip_status();
					if(pCtx->sc_debug)
						diag_printf("the ip status is %d\n", pCtx->sc_ip_status);
					if(pCtx->sc_ip_status != SC_DHCP_STATIC_IP)
					{
						pCtx->sc_ip_status = SC_DHCP_GETTING_IP;
					}
					get_random_ssid(pCtx);
					sprintf(buf, "ifconfig %s down", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib ssid=\"%s\"", pCtx->sc_wlan_ifname, pCtx->sc_ssid);
					run_clicmd(buf);
					//sprintf(buf, "iwpriv %s set_mib encmode=0", pCtx->sc_wlan_ifname);
					//run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib sc_duration_time=120", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib sc_pin_enabled=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "iwpriv %s set_mib sc_status=0", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					sprintf(buf, "ifconfig %s up", pCtx->sc_wlan_ifname);
					run_clicmd(buf);
					pCtx->sc_status = 0;
					pCtx->sc_save_profile = 0;
					configured = 0;
					//run_clicmd("echo 1 > /proc/gpio");
					sleep(1);
					//run_clicmd("echo 0 > /proc/gpio");
					sleep(1);
					//run_clicmd("echo 1 > /proc/gpio");
					sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
					run_clicmd(buf);				
				}
			}

			if(pCtx->sc_status != 0)
			{
				if(pCtx->sc_pbc_duration_time > 0)
				{
					pCtx->sc_pbc_duration_time++;
					if(pCtx->sc_pbc_duration_time > 120)
					{
						pCtx->sc_pbc_duration_time = 0;
						sprintf(buf, "ifconfig %s down", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "iwpriv %s set_mib sc_enabled=0", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "iwpriv %s set_mib sc_duration_time=-1", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "iwpriv %s set_mib sc_pin_enabled=1", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "ifconfig %s up", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sleep(2);
						sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
					}
				}
	
			}
#if defined(WPS_SC_CONCURRENT)
			if(pCtx->sc_wps_support == 1)
			{
				if(start_sc == 1)
				{
					pCtx->sc_wps_duration_time = 0;
					if(pCtx->sc_pbc_duration_time > MAX_SC_TIME)
					{
						sprintf(buf, "iwpriv %s set_mib sc_enabled=0", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						sprintf(buf, "wscd -sig_pbc %s", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						pCtx->sc_wps_duration_time = 1;
						pCtx->sc_pbc_duration_time = 0;
					}
				}
				else if(pCtx->sc_wps_duration_time>0)
				{
					printf("it is doing WPS now!!!\n");
					pCtx->sc_wps_duration_time++;
					if(pCtx->sc_wps_duration_time > MAX_SC_TIME)
					{
						run_clicmd("echo 1 > /tmp/wscd_cancel");	
						sprintf(buf, "iwpriv %s set_mib sc_enabled=1", pCtx->sc_wlan_ifname);
						run_clicmd(buf);
						pCtx->sc_wps_duration_time = 0;
						pCtx->sc_pbc_duration_time = 1;
					}
				}
			}
#endif
		}
		
#if defined(CONFIG_SIMPLE_CONFIG_PBC_SUPPORT)
		if(wps_sc_concurrent == 1)
		{
			if(pCtx->sc_status >=2 && pCtx->sc_status <=5)
			{
				//system("echo 1 > /tmp/wscd_cancel");
       			extern cyg_flag_t wsc_flag;
				cyg_flag_setbits(&wsc_flag, 0x80);	
				wps_sc_concurrent = 0;
				sleep(2);
				sprintf(buf, "ifconfig %s down", pCtx->sc_wlan_ifname);
				run_clicmd(buf);
				sprintf(buf, "ifconfig %s up", pCtx->sc_wlan_ifname);
				run_clicmd(buf);
			}
		}
#endif
#if 0
		if(pCtx->sc_status == 4)
		{
			//run_clicmd("echo 1 > /tmp/wscd_cancel");	
		}
		
		if((pCtx->sc_linked_time >=10) && (pCtx->sc_pbc_pressed_time == 0))
		{
			//run_clicmd("echo 1 > /proc/gpio");
		}
		else if(start_sc == 1) 
		{
			if(pCtx->sc_pbc_pressed_time == 0)
			{
				if((pCtx->sc_run_time%2) == 0)
					run_clicmd("echo 1 > /proc/gpio");
				else
					run_clicmd("echo 0 > /proc/gpio");
			}
			
		}
		else
			run_clicmd("echo 0 > /proc/gpio");
#endif
#endif
	}
#endif

	close(sockfd_scan);
	close(sockfd_control);
#ifdef HAVE_SYSTEM_REINIT
	sc_quitting = 0;
#endif
	return 0;
}

int simple_config_start(unsigned char *argv)
{

#if 0
	if(!strcmp(argv[0],"start")) 
		simple_config_cmd_enable=1;
	else
	{
		diag_printf("invalid input!\n should be 'simple_config start'\n");
		return(-1);
	}
#endif
#if 1//def HAVE_SYSTEM_REINIT
	if(sc_run_flag == 1)
	{
		diag_printf("simple config thread is running!\n");
		return -1;
	}
	
	sc_run_flag = 1;
#endif

	memcpy(SC_WLAN_IF, argv, strlen(argv));
	{		
		cyg_thread_create(SIMPLE_CONFIG_THREAD_PRIORITY,
		simple_config_main,
		0,
		"simple_config",
		simple_config_stack,
		sizeof(simple_config_stack),
		&simple_config_thread,
		&simple_config_thread_object);
		
		diag_printf("Starting simple config thread\n");
		cyg_thread_resume(simple_config_thread);
		return(0);
	}
	
}

#ifdef HAVE_SYSTEM_REINIT
void kill_simple_config()
{
	if(sc_run_flag){
		sc_quitting = 1;
		while(sc_quitting){
			cyg_thread_delay(8);
		}
		simple_config_exit();
		sc_run_flag = 0;
	}
	
}
void simple_config_cleanup()
{

	
}
void simple_config_exit()
{
	cyg_thread_kill(simple_config_thread);
	cyg_thread_delete(simple_config_thread);
}
#endif


