
#include "common.h"
#include "unistd.h"
#include <netinet/in.h>
#include <net/if.h>
#include "extra.h"
#include "elink_common.h"
//#include "wifi.h"
#include "proto.h"
struct elink_ctx g_elink_ctx;
typedef int (*msg_handler)(cJSON *obj, struct cmd_info *c, unsigned char **rsp_buf);
typedef int (*tail_check)(const char *msg);
typedef int (*add_to_list)(char *msg, int len);

unsigned int _get_retry_interval(unsigned int retry_count);

int elink_cloud_client(void);

int _send_remote_msg(unsigned char *msg, int len);

void _heartbeat_tmot_cb(struct uloop_timeout *timeout);
void _remote_ack_tmot_cb(struct uloop_timeout *timeout);
void _remote_connect_cb(struct uloop_timeout *timeout);
void _remote_disconnect_cb(struct uloop_timeout *timeout);
void _remote_action_cb(struct uloop_timeout *timeout);
int _notify_ap_server(int ishb);


struct uloop_timeout heartbeat_timer = { .cb = _heartbeat_tmot_cb };
struct uloop_timeout remote_ack_timer = { .cb = _remote_ack_tmot_cb };
struct uloop_timeout remote_disconnect_timer = { .cb = _remote_disconnect_cb };
struct uloop_timeout remote_connect_timer = { .cb = _remote_connect_cb };


/*****************************************************************************
 函 数 名  : public_key_init
 功能描述  : 初始化公钥
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2018年5月21日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static void public_key_init()
{
	FILE *fp = NULL;
	int len = 0, cnt = 0;

	if (access(ELINKCC_PEM, F_OK) != 0) {
		DEBUG_INFO("%s doesn't exist, use default pem\n", ELINKCC_PEM);
		g_elink_ctx.keys.public_key = (unsigned char *)strdup(REAL_PUBLIC_KEY);

		fp = fopen(ELINKCC_PEM, "w");

		if (fp) {
			fwrite(REAL_PUBLIC_KEY, 1, strlen(REAL_PUBLIC_KEY), fp);
			fclose(fp);
		} else {
			DEBUG_INFO("open pem error\n");
		}
	} else {
		fp = fopen(ELINKCC_PEM, "r");

		if (fp) {
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			rewind(fp);
			g_elink_ctx.keys.public_key = malloc(len);
			cnt = fread(g_elink_ctx.keys.public_key, 1, len, fp);
			fclose(fp);
		} else {
			DEBUG_INFO("open pem error\n");
		}

		if (cnt != len)
			DEBUG_INFO("read pem error\n");
	}
}


void _dump_list(int type)
{
#if ENABLE_LIST_DEBUG
	struct req_info *p = NULL;
	struct cmd_info *c = NULL;
	struct spp_info *s = NULL;
	int count = 0;

	if (type == 0) {
		list_for_each_entry(p, &g_elink_ctx.req_list, list) {
			count++;
		}
		DEBUG_INFO("%d items in remote req list\n", count);
	} else if (type == 1) {
		list_for_each_entry(c, &g_elink_ctx.cmd_list, list) {
			count++;
		}
		DEBUG_INFO("%d items in local cmd list\n", count);
	} else if (type == 2) {
		list_for_each_entry(s, &g_elink_ctx.spp_list, list) {
			count++;
		}
		DEBUG_INFO("%d items in ssp list\n", count);
	}

#endif
	return;
}

/*****************************************************************************
 函 数 名  : _set_backup_servers
 功能描述  : 配置备用server，该函数在状态机开始的时候
 				会去检测是否使用的是次服务器，如果不是，会发送boot request
 输入参数  : char *server  
             int port      
             int idx       
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年5月21日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int _set_backup_servers(char *server, int port, int idx)
{
	cJSON *root = NULL, *aobj = NULL, *array = NULL;
	cJSON *item = NULL;
	char *content = NULL;
	FILE *fp = NULL;

	root = _config_to_jobj(ELINKCC_CONFIG);

	if (!root)
		root = cJSON_CreateObject();

	if (!root) {
		DEBUG_INFO("failed to create json obj\n");
		return -1;
	}

	item = cJSON_GetObjectItem(root, "Backup");

	if (idx == 0) {
		if (item)
			cJSON_DeleteItemFromObject(root, "Backup");

		cJSON_AddItemToObject(root, "Backup", array = cJSON_CreateArray());
	} else {
		array = item;
	}

	cJSON_AddItemToArray(array, aobj = cJSON_CreateObject());

	cJSON_AddItemToObject(aobj, "ServerAddr", cJSON_CreateString(server));
	cJSON_AddItemToObject(aobj, "ServerPort", cJSON_CreateNumber(port));

	content = cJSON_PrintUnformatted(root);
#if ENABLE_JSON_DEBUG
	DEBUG_INFO("new config: %s\n", content);
#endif

	if (!content) {
		DEBUG_INFO("config is not valid json\n");
		return -1;
	}
	fp = fopen(ELINKCC_CONFIG, "w+");

	if (fp) {
		fputs(content, fp);
		fclose(fp);
	} else {
		DEBUG_INFO("failed to update %s\n", ELINKCC_CONFIG);
	}

	FREE(content);
	cJSON_Delete(root);

	return 0;
}

/*****************************************************************************
 函 数 名  : _set_key_value
 功能描述  : 设置到配置文件中，将备用服务器的信息，以及active获取到ID
 输入参数  : char *key   
             int type    
             char *sval  
             int ival    
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2018年5月21日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static int _set_key_value(char *key, int type, char *sval, int ival)
{
	cJSON *obj = NULL;
	cJSON *item = NULL;
	char *content = NULL;
	FILE *fp = NULL;

	obj = _config_to_jobj(ELINKCC_CONFIG);

	if (!obj)
		obj = cJSON_CreateObject();

	if (!obj) {
		DEBUG_INFO("failed to create json obj\n");
		return -1;
	}

	item = cJSON_GetObjectItem(obj, key);

	if (item) { //modify
		if (type == cJSON_Number) {
			cJSON_SetNumberHelper(item,(double) ival);
		} else if (type == cJSON_String) {
			cJSON_DeleteItemFromObject(obj, key);
			cJSON_AddItemToObject(obj, key, cJSON_CreateString(sval));
		}
	} else { //add
		if (type == cJSON_Number) {
			cJSON_AddItemToObject(obj, key, cJSON_CreateNumber(ival));
		} else if (type == cJSON_String) {
			cJSON_AddItemToObject(obj, key, cJSON_CreateString(sval));
		}
	}

	content = cJSON_PrintUnformatted(obj);

#if ENABLE_JSON_DEBUG
	DEBUG_INFO("new config: %s\n", content);
#endif

	if (!content) {
		DEBUG_INFO("config is not valid json\n");
		return -1;
	}

	fp = fopen(ELINKCC_CONFIG, "w+");

	if (fp) {
		fputs(content, fp);
		fclose(fp);
	} else {
		DEBUG_INFO("failed to update %s\n", ELINKCC_CONFIG);
	}

	FREE(content);
	cJSON_Delete(obj);

	return 0;
}
/*****************************************************************************
 函 数 名  : _send_remote_msg
 功能描述  : client发送到ucloud，添加头部
 输入参数  : unsigned char *msg  
             int len             
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年5月26日
    作    者   : Liquan
    修改内容   : 新生成函数

*****************************************************************************/
int _send_remote_msg(unsigned char *msg, int len)
{
	char *hdrbuf = NULL;
	unsigned char *b64_msg = NULL;
	int b64_len = 0;
	int ret = 0;

	b64_len = _do_b64_cmd(msg, len, &b64_msg, 1);
	FREE(msg);

	if (b64_len == -1) {
		DEBUG_INFO("b64 encode failed.\n");
		cyg_thread_delay(100);
		return -1;
	}

	hdrbuf = (char *)_add_elink_hdr_remote(b64_msg, b64_len);
	FREE(b64_msg);

	if (!hdrbuf) {
		DEBUG_INFO("add hdr failed.\n");
		cyg_thread_delay(100);
		return -1;
	}

	//ret = //send_to_cloud_server();//ustream_write(&g_elink_ctx.remote.stream, hdrbuf, ELINK_MSG_LEN(b64_len), 0);
       ret = socket_send(g_elink_ctx.romote_fd, (void *)hdrbuf, ELINK_MSG_LEN(b64_len));
	FREE(hdrbuf);

	return ret;
}

int _is_same_server(void)
{
	if (IS_EMPTY_STRING(g_elink_ctx.server.addr) ||
		IS_EMPTY_STRING(g_elink_ctx.server.bssaddr))
		return 1;

	if (_strcmp(g_elink_ctx.server.addr, g_elink_ctx.server.bssaddr) ||
		(g_elink_ctx.server.port != g_elink_ctx.server.bssport)) {
		return 0;
	}

	return 1;
}


void _reconnect_server(void)
{
	g_elink_ctx.curr_srv = NULL;
	DEBUG_INFO("reconnect server\n");
	uloop_timeout_set(&remote_disconnect_timer, ELINKCC_100MS_INTERVAL);
	uloop_timeout_set(&remote_connect_timer, ELINKCC_200MS_INTERVAL);
}

int _parse_boot_resp(cJSON *obj)
{
	cJSON *bizplat = NULL, *item = NULL;
	char *serverip = NULL, *backup = NULL;
	int result = 1, serverport = 0, backport = 0, i = 0, j = 0, chg = 0;
	unsigned char msg[128] = {0};
	int msg_len = 0;

	_get_int_json_value(obj, "Result", cJSON_Number, &result);

	if (0 == result) {

		g_elink_ctx.is_bss = 0;
		bizplat = cJSON_GetObjectItem(obj, "BizPlat");

		if (!bizplat) {
			DEBUG_INFO("failed to get BizPlat\n");
			return -1;
		}

		if (cJSON_IsArray(bizplat)) {

			json_array_foreach(bizplat, i, item) {
				if (i == 0) {

					_get_string_json_value(item, "ServerAddr", &serverip);
					_get_int_json_value(item, "ServerPort", cJSON_Number, &serverport);

					if (!IS_EMPTY_STRING(serverip) && IS_VALID_PORT(serverport)) {
						_set_key_value("ServerAddr", cJSON_String, serverip, 0);
						_set_key_value("ServerPort", cJSON_Number, NULL, serverport);
					}

					if (!IS_EMPTY_STRING(serverip) &&
						(!g_elink_ctx.server.addr || (_strcmp(g_elink_ctx.server.addr, serverip) != 0))) {
						FREE(g_elink_ctx.server.addr);
						g_elink_ctx.server.addr = strdup(serverip);
						chg = 1;
					}

					if (IS_VALID_PORT(serverport) && g_elink_ctx.server.port != serverport) {
						g_elink_ctx.server.port = serverport;
						chg = 1;
					}

					FREE(serverip);
				} else {

					_get_string_json_value(item, "ServerAddr", &backup);
					_get_int_json_value(item, "ServerPort", cJSON_Number, &backport);

					if (!IS_EMPTY_STRING(backup) && IS_VALID_PORT(backport)) {
						_set_backup_servers(backup, backport, j);

						FREE(g_elink_ctx.server.backup[j].addr);
						g_elink_ctx.server.backup[j].addr = strdup(backup);
						g_elink_ctx.server.backup[j].port = backport;
						g_elink_ctx.server.backup[j].used = 0;
						j++;
					}

					FREE(backup);
				}
			}

			/* save to flash */
			msg_len = sprintf(msg, "{\"type\":\"cfg\",\"mac\":\"%s\",\"sequence\":%d,"
							   "\"set\":{\"ctrlcommand\":\"save\"}}", g_elink_ctx.ap.mac, ++g_elink_ctx.sequence);

			if (msg_len > 0) {
				DEBUG_INFO("(local) -> %s (%d)\n", msg, msg_len);
				
			}

		} else {
			DEBUG_INFO("invalid BizPlat in Boot Ack\n");
		}
	}

	if (-2 == result) {
		uloop_timeout_set(&remote_disconnect_timer, ELINKCC_100MS_INTERVAL);
		DEBUG_INFO("reconnect the server after 150 minutes\n");
		uloop_timeout_set(&remote_connect_timer, ELINKCC_150M_INTERVAL);
	} else if (0 == result) {
		if ((0 == chg) && (!g_elink_ctx.is_bootstrap)) {
			uloop_timeout_set(&remote_disconnect_timer, ELINKCC_100MS_INTERVAL);
			DEBUG_INFO("reconnect ability server after 150 minutes\n");

			/* connect ability server after timeout */
			g_elink_ctx.curr_srv = NULL; //_get_server_addr();

			uloop_timeout_set(&remote_connect_timer, ELINKCC_150M_INTERVAL);
		} else if (g_elink_ctx.server.addr && !_is_same_server()) {
			g_elink_ctx.is_bootstrap = 0;
			_reconnect_server();
		} else if (_is_same_server()) {
			g_elink_ctx.is_bootstrap = 0;
			DEBUG_INFO("is same server\n");
		}
	} else {
		_remote_ack_tmot_cb(NULL);
	}


	return result;
}


int _parse_challenge_resp(cJSON *obj)
{
	int result = 1;

	_get_int_json_value(obj, "Result", cJSON_Number, &result);

	//0, -1, -5
	if (0 == result) {
		FREE(g_elink_ctx.keys.challenge_code);
		_get_string_json_value(obj, "ChallengeCode", &g_elink_ctx.keys.challenge_code);
	} else {
		_remote_ack_tmot_cb(NULL);
	}

	return result;

}


int _parse_activate_resp(cJSON *obj)
{
	int result = 1;
	unsigned char msg[1024] = {0};
	int msg_len = 0;

	_get_int_json_value(obj, "Result", cJSON_Number, &result);

	if (0 == result) {
		FREE(g_elink_ctx.dev.token);
		_get_string_json_value(obj, "DevToken", &g_elink_ctx.dev.token);
		_set_key_value("DevToken", cJSON_String, g_elink_ctx.dev.token, 0);

		FREE(g_elink_ctx.dev.id);
		_get_string_json_value(obj, "DevID", &g_elink_ctx.dev.id);
		_set_key_value("DevID", cJSON_String, g_elink_ctx.dev.id, 0);

		if (IS_EMPTY_STRING(g_elink_ctx.dev.id) || IS_EMPTY_STRING(g_elink_ctx.dev.token)) {

			_remote_ack_tmot_cb(NULL);
			return 1;

		} else {
			/* save to flash */
			msg_len = sprintf(msg, "{\"type\":\"cfg\",\"mac\":\"%s\",\"sequence\":%d,"
							   "\"set\":{\"ctrlcommand\":\"save\"}}", g_elink_ctx.ap.mac, ++g_elink_ctx.sequence);

			if (msg_len > 0) {
				DEBUG_INFO("(local) -> %s (%d)\n", msg, msg_len);
				
			}
		}

	} else if (-2 == result) {
		uloop_timeout_set(&remote_disconnect_timer, ELINKCC_100MS_INTERVAL);
		DEBUG_INFO("reconnect the server after 150 minutes\n");
		uloop_timeout_set(&remote_connect_timer, ELINKCC_150M_INTERVAL);
	} else {
		_remote_ack_tmot_cb(NULL);
	}

	/* send GetChallengeCode again before Connect */
	FREE(g_elink_ctx.keys.challenge_code);

	return result;
}

int _parse_connect_resp(cJSON *obj)
{
	int result = 1, cnt = 0;

	_get_int_json_value(obj, "Result", cJSON_Number, &result);

	FREE(g_elink_ctx.keys.server_key);

	if (0 == result) {
		cnt = _get_string_json_value(obj, "SKey", (char **)&g_elink_ctx.keys.server_key);

		if (cnt != CKEY_SKEY_LEN) {
			_remote_ack_tmot_cb(NULL);
		}
	} else if (-2 == result) {
		FREE(g_elink_ctx.dev.token);
		FREE(g_elink_ctx.dev.id);

		/* send Activate */
		_notify_ap_server(0);
	} else if (-3 == result) {
		uloop_timeout_set(&remote_disconnect_timer, ELINKCC_100MS_INTERVAL);
		DEBUG_INFO("reconnect the server after 150 minutes\n");
		uloop_timeout_set(&remote_connect_timer, ELINKCC_150M_INTERVAL);
	} else {
		_remote_ack_tmot_cb(NULL);
	}

	return result;
}

int _parse_heartbeat_resp(cJSON *obj)
{
	int result = -1;

	uloop_timeout_cancel(&remote_ack_timer);

	_get_int_json_value(obj, "Result", cJSON_Number, &result);

	if (0 != result)
		_remote_ack_tmot_cb(NULL);

	return result;
}

int _parse_request_connect_bss(cJSON *obj)
{
	char *server = NULL, *port = NULL, *id = NULL;
	char outbuf[128] = {0};
	unsigned char *en_msg = NULL;
	int en_len = 0, outlen = 0, ret = 0, pval = 0;

	if (_get_string_json_value(obj, "ID", &id) <= 0) {
		DEBUG_INFO("%d: invalid ID: %s\n", __LINE__, id);
		goto done;
	}

	if (_get_string_json_value(obj, "Server", &server) < 0) {
		DEBUG_INFO("%d: invalid Server: %s\n", __LINE__, server);
		goto done;
	}

	if (_get_string_json_value(obj, "Port", &port) < 0) {
		DEBUG_INFO("%d: invalid Port: %s\n", __LINE__, port);
		goto done;
	}

	pval = atoi(port);

	outlen = sprintf(outbuf, "{\"Result\":0,\"Ack\":\"RequestConnectBSS\",\"ID\":\"%s\"}", id);

	if (outlen > 0) {
		DEBUG_INFO("(remote) -> %s (%d)\n", outbuf, outlen);

		if (_do_aes_ecb_crypt((unsigned char *)outbuf, outlen, &en_msg, &en_len, (unsigned char *)g_elink_ctx.keys.server_key, 1) < 0)
			goto done;

		_send_remote_msg(en_msg, en_len);
	} else {
		DEBUG_INFO("%d: failed to generate msg\n", __LINE__);
	}

	if (IS_VALID_PORT(pval) && !IS_EMPTY_STRING(server)) {
		_set_key_value("BSSAddr", cJSON_String, server, 0);
		_set_key_value("BSSPort", cJSON_Number, NULL, pval);

		FREE(g_elink_ctx.server.bssaddr);
		g_elink_ctx.server.bssaddr = server;
		g_elink_ctx.server.bssport = pval;
	} else {
		FREE(server);
	}

	g_elink_ctx.curr_srv = NULL;
	g_elink_ctx.is_bss = 1;
	uloop_timeout_set(&remote_ack_timer, ELINKCC_30S_INTERVAL);

done:
	FREE(id);
	FREE(port);

	return ret;
}

int return_Parameter_to_remote(cJSON*send_root,char*ID)
{
	char *jstr = NULL;
	unsigned char* buf= NULL;
	unsigned int buflen = 0;
	unsigned char *b64_msg = NULL;
	unsigned char *en_msg = NULL;
	int b64_len = 0, en_len = 0;


	jstr = cJSON_Print(send_root);
	b64_len = _do_b64_cmd((unsigned char *)jstr, strlen(jstr), &b64_msg, 1);

	if (b64_len > 0) {
		buf = (char*)malloc(b64_len + 256);
		buflen = sprintf(buf, "{\"Result\":%d,\"Ack\":\"SetPluginParams\",\"return_Parameter\":\"%s\",\"ID\":\"%s\"}",
						  0, b64_msg, ID);

		if (buflen > 0) {

			DEBUG_INFO("(remote) -> %s (%d)\n", buf, buflen);

			if (_do_aes_ecb_crypt(buf, buflen, &en_msg, &en_len, g_elink_ctx.keys.server_key, 1) < 0)
				goto bad_msg;

			if (en_len > 0) {
				_send_remote_msg(en_msg, en_len);
			} else {
				DEBUG_INFO("%s: RSA encrypt failed.\n", __func__);
			}

		} else {
			DEBUG_INFO("%s: sprintf failed\n", __func__);
		}
	}

bad_msg:
	FREE(b64_msg);
	FREE(jstr);
	FREE(buf);
}

int process_data_cfg(char *buff,char*ID)
{
	int ret = 0;
	cJSON *send_root = NULL;


	process_all_data_cfg(buff,1);

	send_root = cJSON_CreateObject();
	cJSON_AddStringToObject(send_root, "type", "ack");
	cJSON_AddNumberToObject(send_root, "sequence", 1234);
	cJSON_AddStringToObject(send_root, "mac", g_elink_ctx.ap.mac);
	
	return_Parameter_to_remote(send_root,ID);

	if(send_root)
		cJSON_Delete(send_root);

	return ret;
}

int send_get_status_to_cloud(cJSON *get_info,char*ID)
{
	int i = 0;
	int ret = 0;
	int get_array_len = 0;
	cJSON *get_status = NULL;
	cJSON *get_name   = NULL;
	cJSON *send_root = NULL;
	cJSON *send_status = NULL;
	
	send_root = cJSON_CreateObject();
	send_status = cJSON_CreateObject();
	
   	cJSON_AddStringToObject(send_root, "type", "status");
	cJSON_AddNumberToObject(send_root, "sequence", 1234);
	cJSON_AddStringToObject(send_root, "mac", g_elink_ctx.ap.mac);
	cJSON_AddItemToObject(send_root, "status", send_status);
	get_array_len = cJSON_GetArraySize(get_info);
	for(i; i < get_array_len; i++)
	{
		get_status = cJSON_GetArrayItem(get_info,i);
		get_name = cJSON_GetObjectItem(get_status,"name");
		get_route_status_info(send_status,get_name->valuestring);
		DEBUG_INFO(" get status info :  %s \n", get_name->valuestring);
	}

	return_Parameter_to_remote(send_root,ID);
	if(send_root){
	    cJSON_Delete(send_root);
	}

    return ret;
}


int process_data_get_status(char *buff,char*ID)
{
	int ret = 0;
	cJSON *root = NULL;
	cJSON *get_info   = NULL;
	int get_array_len = 0;
	int i = 0;

	get_status_info wifi_status_info[10] = {0};
	    
	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);

	get_info = cJSON_GetObjectItem(root,"get");
	CHECK_JSON_VALUE(get_info);

	send_get_status_to_cloud(get_info,ID);

	bad_msg:
	if(root)
	    cJSON_Delete(root);

    return ret;
}
//{"type":"getrssiinfo","get":{"mac":["683e342671bb","BCA92020D8B7"]}}}
int process_data_getrssiinfo(char *buff,char*ID)
{
	int ret = 0;
	cJSON *root   = NULL;
	cJSON *get_info   = NULL;
	cJSON *output_root   = NULL;
	cJSON *output_info   = NULL;
	    
	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);

	get_info = cJSON_GetObjectItem(root,"get");
	CHECK_JSON_VALUE(get_info);
	output_root = cJSON_CreateObject();
	cJSON_AddStringToObject(output_root, "type", "rssiinfo");
	cJSON_AddNumberToObject(output_root, "sequence", 1234);
	cJSON_AddStringToObject(output_root, "mac", g_elink_ctx.ap.mac);
	cJSON_AddItemToObject(output_root, "rssiinfo",output_info = cJSON_CreateArray());
	get_sta_rssi_info(get_info,output_info);
	return_Parameter_to_remote(output_root,ID);

	/*need process get sta_info and rssi*/
bad_msg:
	if(output_root)
	    cJSON_Delete(output_root);
	if(root)
	    cJSON_Delete(root);

    return ret;
}
//{"type":"deassociation","set":{"mac":["683E342671bb"]}
int process_data_deassociation(char *buff,char*ID)
{
	int ret = 0;
	cJSON *root = NULL;
	cJSON *set_info = NULL;
	cJSON *send_root = NULL;
	
	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);

	set_info = cJSON_GetObjectItem(root,"set");
	CHECK_JSON_VALUE(set_info);
	exec_deassociation(set_info);
	send_root = cJSON_CreateObject();
	CHECK_JSON_VALUE(send_root);
	cJSON_AddStringToObject(send_root, "type", "ack");
	cJSON_AddNumberToObject(send_root, "sequence", 1234);
	cJSON_AddStringToObject(send_root, "mac", g_elink_ctx.ap.mac);
	
	return_Parameter_to_remote(send_root,ID);
bad_msg:
	if(root)
		cJSON_Delete(root);
	if(send_root)
		cJSON_Delete(send_root);

    return ret;
}

int parse_config_data_from_cloud(char *buff,char* ID)
{ 
	int ret = -1;
	cJSON *root = NULL;
	cJSON *type = NULL;
	cJSON *seq	= NULL;

	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
	
	type = cJSON_GetObjectItem(root, "type");
	CHECK_JSON_VALUE(type);
	
	if(!strcmp(ELINE_MESSAGE_TYPE_CFG, type->valuestring))
	{
		DEBUG_INFO("receive cfg info ,call extra. \n");
		process_data_cfg(buff,ID);   
		ret = 0;
	}
	else if(!strcmp(ELINE_MESSAGE_TYPE_GET_STATUS, type->valuestring))
	{

		DEBUG_INFO("receive get_status ack\n");
		process_data_get_status(buff,ID); 
		DEBUG_INFO("process data over\n");
		ret = 0;
	}
	else if(!strcmp(ELINE_MESSAGE_TYPE_GET_STA_RSSI, type->valuestring))
	{

		DEBUG_INFO("receive getrssiinfo ack\n");
		process_data_getrssiinfo(buff,ID); 
		DEBUG_INFO("process data over\n");
		ret = 0;
	}
	else if(!strcmp(ELINE_MESSAGE_TYPE_DEASSOC, type->valuestring))
	{

		DEBUG_INFO("receive deassociation ack\n");
		process_data_deassociation(buff,ID); 
		DEBUG_INFO("process data over\n");
		ret = 0;
	}
	else
	{
		DEBUG_INFO("data type false. type : %s\n", type->valuestring);
	}

	bad_msg:
	if(root)
	cJSON_Delete(root);  

	return ret;
}


int _parse_set_plugin_params(cJSON *obj)
{
	char *name = NULL, *id = NULL, *version = NULL;
	char *param_b64 = NULL;
	unsigned char *param_json = NULL;
	char *new_json = NULL;
	int b64_len = 0, json_len = 0;
	struct spp_info *si = NULL;

	if (_get_string_json_value(obj, "Plugin_Name", &name) <= 0 ||
		_strcmp(name, ELINK_PLUGIN_NAME) != 0) {
		DEBUG_INFO("%d: invalid Plugin_Name: %s\n", __LINE__, name);
		goto done;
	}

	if (_get_string_json_value(obj, "ID", &id) <= 0) {
		DEBUG_INFO("%d: invalid ID: %s\n", __LINE__, id);
		goto done;
	}

	if (_get_string_json_value(obj, "Version", &version) <= 0) {
		DEBUG_INFO("%d: invalid Version: %s\n", __LINE__, version);
		goto done;
	}

	b64_len = _get_string_json_value(obj, "Parameter", &param_b64);

	if (!param_b64) {
		DEBUG_INFO("%d: failed to get Parameter\n", __LINE__);
		goto done;
	}

	json_len = _do_b64_cmd((unsigned char *)param_b64, b64_len, &param_json, 0);

	if (json_len < 0) {
		DEBUG_INFO("%d: failed to decode Parameter\n", __LINE__);
		goto done;
	}

	if (json_len <= 0) {
		DEBUG_INFO("%d: Parameter is null\n", __LINE__);
		goto done;
	}

	/* setup ID/sequence mapping */
	/*get config */
	DEBUG_INFO("(local) -> %s (%d)\n", param_json, json_len);
	
	parse_config_data_from_cloud(param_json,id);
done:
	FREE(id);
	FREE(version);
	FREE(name);
	FREE(param_b64);
	FREE(param_json);

	return 0;
}


int _parse_list_plugin(cJSON *obj, char *method)
{
	char *id = NULL;
	unsigned char outbuf[1024] = {0};
	unsigned char *en_msg = NULL;
	int len = 0, en_len = 0;

	/*
	{"RPCMethod":"ListPlugin","ID":"83c322fa-ada0-11e7-a9d4-f80f41f70c5a"}
	*/

	if (_get_string_json_value(obj, "ID", &id) <= 0) {
		DEBUG_INFO("%d: invalid ID: %s\n", __LINE__, id);
		goto done;
	}

	len = sprintf(outbuf, "{\"Result\":0,\"ID\":\"%s\",\"Ack\":\"%s\","
				   "\"Plugin\":[{\"Plugin_Name\":\"%s\",\"Version\":\"%s\",\"Run\":1}]}",
				   id, method, ELINK_PLUGIN_NAME, g_elink_ctx.ap.swver);

	if (len > 0) {
		DEBUG_INFO("(remote) -> %s (%d)\n", outbuf, len);

		if (_do_aes_ecb_crypt(outbuf, len, &en_msg, &en_len, g_elink_ctx.keys.server_key, 1) < 0)
			goto done;

		_send_remote_msg(en_msg, en_len);
	} else {
		DEBUG_INFO("%d: failed to generate msg\n", __LINE__);
	}

done:
	FREE(id);

	return 0;
}

int _parse_unsupported_method(cJSON *obj, char *method)
{
	char *id = NULL;
	unsigned char outbuf[1024] = {0};
	unsigned char*en_msg = NULL;
	int len = 0, en_len = 0;

	/*
	{"RPCMethod":"XXXX","ID":"83c322fa-ada0-11e7-a9d4-f80f41f70c5a"}
	*/

	if (_get_string_json_value(obj, "ID", &id) <= 0) {
		DEBUG_INFO("%d: invalid ID: %s\n", __LINE__, id);
		goto done;
	}

	len = sprintf(outbuf, "{\"Result\":-4,\"ID\":\"%s\",\"Ack\":\"%s\"}", id, method);

	if (len > 0) {
		DEBUG_INFO("(remote) -> %s (%d)\n", outbuf, len);

		if (_do_aes_ecb_crypt(outbuf, len, &en_msg, &en_len, g_elink_ctx.keys.server_key, 1) < 0)
			goto done;

		_send_remote_msg(en_msg, en_len);
	} else {
		DEBUG_INFO("%d: failed to generate msg\n", __LINE__);
	}

done:
	FREE(id);

	return 0;
}


void _update_ckey(void)
{
	FREE(g_elink_ctx.keys.client_key);
	g_elink_ctx.keys.client_key = _get_random_string(CKEY_SKEY_LEN);
}

void _get_ap_ipaddr(void)
{
	g_elink_ctx.ap.ipaddr = (char*)malloc(32);
	extra_get_gateway_ip(g_elink_ctx.ap.ipaddr);
}


int _gen_boot_msg(unsigned char **buf)
{
	char *result = NULL;
	int cnt = 0;
	_update_ckey();
	result = (char*)malloc(1024 * 2);
	cnt = sprintf(result, "{\"RPCMethod\":\"Boot\",\"DevType\":\"%s\",\"Vendor\":\"%s\","
				   "\"Model\":\"%s\",\"MAC\":\"%s\",\"FirmwareVer\":\"%s\",\"HardwareVer\":\"%s\","
				   "\"bootstrap\":\"%d\",\"CKey\":\"%s\"}",
				   "E-Link AP", g_elink_ctx.ap.vendor, g_elink_ctx.ap.model, g_elink_ctx.ap.mac,
				   g_elink_ctx.ap.swver, g_elink_ctx.ap.hdver, g_elink_ctx.is_bootstrap, g_elink_ctx.keys.client_key);
	*buf = (unsigned char *)result;

	return cnt;
}

int _gen_activate_msg(unsigned char **buf)
{
	char *result = NULL;
	int cnt = 0;

	result = (char*)malloc(1024 * 2);
	if (IS_EMPTY_STRING(g_elink_ctx.keys.challenge_code)) {
		cnt = sprintf(result, "{\"RPCMethod\":\"GetChallengeCode\",\"MAC\":\"%s\"}", g_elink_ctx.ap.mac);
		*buf = (unsigned char *)result;
		return cnt;
	}

	_update_ckey();

	if (IS_EMPTY_STRING(g_elink_ctx.ap.ipaddr))
		_get_ap_ipaddr();

	cnt = sprintf(result, "{\"RPCMethod\":\"Activate\",\"CKey\":\"%s\",\"Vendor\":\"%s\","
				   "\"Model\":\"%s\",\"FirmwareVer\":\"%s\",\"HardwareVer\":\"%s\","
				   "\"MAC\":\"%s\",\"IPAddr\":\"%s\",\"SN\":\"%s\",\"ChallengeCode\":\"%s\",\"LOID\":\"%s\","
				   "\"PlatformID\":\"%s\",\"CloudClientVer\":\"%s\",\"MiddleWareVer\":\"%s\",\"MiddleWareBakVer\":\"%s\"}",
				   g_elink_ctx.keys.client_key, g_elink_ctx.ap.vendor, g_elink_ctx.ap.model,
				   g_elink_ctx.ap.swver, g_elink_ctx.ap.hdver, g_elink_ctx.ap.mac, g_elink_ctx.ap.ipaddr,
				   g_elink_ctx.ap.sn, g_elink_ctx.keys.challenge_code, "", "1.0.122", "", "", "");

	*buf = (unsigned char *)result;

	return cnt;
}

int _gen_connect_msg(unsigned char **buf, char *token, char *id)
{
	char *result = NULL;
	int cnt = 0;

	if (!token || !id)
		return -1;
	result = (char*)malloc(1024 * 2);
	if (IS_EMPTY_STRING(g_elink_ctx.keys.challenge_code)) {
		cnt = sprintf(result, "{\"RPCMethod\":\"GetChallengeCode\",\"MAC\":\"%s\"}", g_elink_ctx.ap.mac);
		*buf = (unsigned char *)result;
		return cnt;
	}

	_update_ckey();

	if (IS_EMPTY_STRING(g_elink_ctx.ap.ipaddr))
		_get_ap_ipaddr();

	//TODO: Flag and upgrade_ID
	cnt = sprintf(result, "{\"RPCMethod\":\"Connect\",\"DevToken\":\"%s\",\"DevID\":\"%s\","
				   "\"CKey\":\"%s\",\"Flag\":\"%d\",\"Vendor\":\"%s\",\"Model\":\"%s\",\"SN\":\"%s\","
				   "\"HardwareVer\":\"%s\",\"FirmwareVer\":\"%s\",\"MAC\":\"%s\",\"IPAddr\":\"%s\","
				   "\"CloudClientVer\":\"%s\",\"MiddleWareVer\":\"%s\",\"MiddleWareBakVer\":\"%s\","
				   "\"upgrade_ID\":\"%s\",\"LOID\":\"%s\",\"PlatformID\":\"%s\",\"UPPER_MAC\":\"%s\",\"ChallengeCode\":\"%s\"}",
				   token, id, g_elink_ctx.keys.client_key, 0,
				   g_elink_ctx.ap.vendor, g_elink_ctx.ap.model, g_elink_ctx.ap.sn,
				   g_elink_ctx.ap.hdver, g_elink_ctx.ap.swver,
				   g_elink_ctx.ap.mac, g_elink_ctx.ap.ipaddr, "1.0.122", "", "", "", "", "",
				   g_elink_ctx.ap.gwmac, g_elink_ctx.keys.challenge_code);

	*buf = (unsigned char *)result;

	return cnt;
}

int _gen_heartbeat_msg(unsigned char **buf)
{
	char *result = NULL;
	int cnt = 0;

	result = (char*)malloc(1024 * 2);
	cnt = sprintf(result, "{\"RPCMethod\":\"Heartbeat\",\"MAC\":\"%s\",\"IPAddr\":\"%s\"}",
				   g_elink_ctx.ap.mac, g_elink_ctx.ap.ipaddr);

	*buf = (unsigned char *)result;
	return cnt;
}


int _notify_ap_server(int ishb)
{
	unsigned char *msg = NULL;
	unsigned char *en_msg = NULL;
	int len = 0, en_len = 0;
	cJSON *obj = NULL;

	if (!ishb) {
		if (g_elink_ctx.is_bss)
			len = _gen_boot_msg(&msg);
		else if (IS_EMPTY_STRING(g_elink_ctx.dev.token) || IS_EMPTY_STRING(g_elink_ctx.dev.id))
			len = _gen_activate_msg(&msg);
		else
			len = _gen_connect_msg(&msg, g_elink_ctx.dev.token, g_elink_ctx.dev.id);
	} else {
		len = _gen_heartbeat_msg(&msg);
	}

	DEBUG_INFO("(remote) -> %s (%d)\n", msg, len);

	if (len == -1) {
		DEBUG_INFO("failed to generate boot/activate/connect msg");
		return len;
	} else {
		obj = cJSON_Parse((char *)msg);

		if (!obj) {
			DEBUG_INFO("Invalid request (Not json format)\n");
			return -1;
		} else {
			cJSON_Delete(obj);
		}
	}

	if (!ishb) {
		if (!g_elink_ctx.is_bss && IS_EMPTY_STRING(g_elink_ctx.keys.challenge_code)) {
			en_len = len;
			en_msg = (unsigned char *)strdup((char *)msg);
		} else {
			en_len = _do_rsa_encrypt(msg, len, &en_msg, g_elink_ctx.keys.public_key);

			if (en_len == 0) {
				DEBUG_INFO("RSA encrypt failed.\n");
				FREE(msg);
				return en_len;
			}
		}

	} else if (_do_aes_ecb_crypt(msg, len, &en_msg, &en_len, g_elink_ctx.keys.server_key, 1) < 0) {
		FREE(msg);
		FREE(en_msg);
		return -1;
	}

	_send_remote_msg(en_msg, en_len);
	uloop_timeout_set(&remote_ack_timer, ELINKCC_30S_INTERVAL);
	FREE(msg);

	return 0;
}

/*****************************************************************************
 函 数 名  : _heartbeat_tmot_cb
 功能描述  : 心跳处理
 输入参数  : struct uloop_timeout *timeout  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年5月21日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void _heartbeat_tmot_cb(struct uloop_timeout *timeout)
{
	//发送心跳报文
	_notify_ap_server(1);
	//重置心跳报文
	uloop_timeout_set(&heartbeat_timer, ELINKCC_100S_INTERVAL);
}


/*****************************************************************************
 函 数 名  : _remote_ack_tmot_cb
 功能描述  : 重新连接服务器
 输入参数  : struct uloop_timeout *timeout  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年5月21日
    作    者   :
    修改内容   : 新生成函数

*****************************************************************************/
void _remote_ack_tmot_cb(struct uloop_timeout *timeout)
{
	uloop_timeout_set(&remote_disconnect_timer, 0);

	if (g_elink_ctx.retry_count < 5) {
		uloop_timeout_set(&remote_connect_timer, _get_retry_interval(g_elink_ctx.retry_count));
		g_elink_ctx.retry_count++;
	} else {
		g_elink_ctx.retry_count = 0;
		DEBUG_INFO("reconnect the server after 150 minutes\n");
		uloop_timeout_set(&remote_connect_timer, ELINKCC_150M_INTERVAL);
	}
}

int _process_remote_msg(char *data, unsigned int len)
{
	cJSON *obj = NULL;
	unsigned char *buffer = (unsigned char *)data;
	unsigned char *b64_msg = NULL;
	unsigned char *outbuf = NULL;
	int outlen = 0, b64_len = 0, ret = -1;
	char *acktype = NULL, *rpcmethod = NULL;
	unsigned char *aes_key = NULL;

	b64_len = _do_b64_cmd(buffer, len, &b64_msg, 0);

	if (b64_len == -1) {
		DEBUG_INFO("Invalid server request (b64 decode failed)\n");
		_bin_print(buffer, len);
		FREE(b64_msg);
		goto done;
	}

	DEBUG_INFO("after b64:\n");
	_bin_print(b64_msg, (unsigned int)b64_len);
	if (g_elink_ctx.is_bss || !IS_EMPTY_STRING(g_elink_ctx.keys.challenge_code))
		aes_key = g_elink_ctx.keys.server_key ? g_elink_ctx.keys.server_key : g_elink_ctx.keys.client_key;

	DEBUG_INFO("aes decrypt key is %s (s: %s, c: %s)\n", aes_key,
			   g_elink_ctx.keys.server_key, g_elink_ctx.keys.client_key);

	if (aes_key) {
		if (_do_aes_ecb_crypt(b64_msg, b64_len, &outbuf, &outlen, aes_key, 0) < 0) {
			FREE(b64_msg);
			goto done;
		}
	} else {
		outbuf = (unsigned char *)strdup((char *)b64_msg);
		outlen = b64_len;
	}

	DEBUG_INFO("(remote) <- %s (%d)\n", outbuf, outlen);

	FREE(b64_msg);
	obj = cJSON_Parse((char *)outbuf);

	if (obj == NULL) {
		DEBUG_INFO("Invalid server request (Not json format)\n");
		_bin_print(outbuf, outlen);
		FREE(outbuf);
		goto done;
	}

	FREE(outbuf);

	_get_string_json_value(obj, "Ack", &acktype);

	if (!acktype) {
		_get_string_json_value(obj, "RPCMethod", &rpcmethod);

		if (!rpcmethod) {
			int resultack = 0;

			if (_get_int_json_value(obj, "ResultAck", cJSON_Number, &resultack) < 0) {

				DEBUG_INFO("%d: Unsupported server request\n", __LINE__);
				cJSON_Delete(obj);
				return -1;
			} else {
				uloop_timeout_cancel(&remote_ack_timer);
				uloop_timeout_set(&remote_disconnect_timer, ELINKCC_100MS_INTERVAL);
				uloop_timeout_set(&remote_connect_timer, ELINKCC_200MS_INTERVAL);
			}
		} else if (_strcmp(rpcmethod, "SetPluginParams") == 0) {
			ret = _parse_set_plugin_params(obj);
		} else if (_strcmp(rpcmethod, "RequestConnectBSS") == 0) {
			ret = _parse_request_connect_bss(obj);
		} else if (_strcmp(rpcmethod, "ListPlugin") == 0) {
			ret = _parse_list_plugin(obj, rpcmethod);
		} else {
			ret = _parse_unsupported_method(obj, rpcmethod);
		}

		FREE(rpcmethod);
	} else {

		uloop_timeout_cancel(&remote_ack_timer);

		if (_strcmp(acktype, "Boot") == 0)
			ret = _parse_boot_resp(obj);
		else if (_strcmp(acktype, "GetChallengeCode") == 0)
			ret = _parse_challenge_resp(obj);
		else if (_strcmp(acktype, "Activate") == 0)
			ret = _parse_activate_resp(obj);
		else if (_strcmp(acktype, "Connect") == 0)
			ret = _parse_connect_resp(obj);
		else if (_strcmp(acktype, "Heartbeat") == 0)
			ret = _parse_heartbeat_resp(obj);
		else if (_strcmp(acktype, "PluginNotification") == 0)
			; /* do nothing */
		else
			DEBUG_INFO("Unknown Ack type: %s\n", acktype);

		if (ret == 0) {
			if (g_elink_ctx.retry_count > 0)
				g_elink_ctx.retry_count--;

			if ((_strcmp(acktype, "Boot") == 0 && _is_same_server()) ||
				_strcmp(acktype, "Activate") == 0 ||
				_strcmp(acktype, "GetChallengeCode") == 0)
				_notify_ap_server(0);
			else if (_strcmp(acktype, "Connect") == 0)
				uloop_timeout_set(&heartbeat_timer, ELINKCC_100S_INTERVAL);
		}

		FREE(acktype);
	}

	cJSON_Delete(obj);

done:

	if (data) {
		FREE(data);
		data = NULL;
	}

	return ret;
}

unsigned int _get_retry_interval(unsigned int retry_count)
{
	unsigned int interval = 0;
	int min = 0, max = 0;

	/*	0	0-90s
		1	90-180s
		2	180-360s
		3	360-720s
		4	720-1440s */
	min = (retry_count < 1) ? 0 : 90 * (1 << (retry_count - 1));
	max = 90 * (1 << retry_count);

	interval = _get_random_number(min, max);

#if ENABLE_INTERVAL_DEBUG

	DEBUG_INFO("DDBBGG interval was %d, DDBBGG modify interval to 1\n", interval);
	interval = 1;

#endif

	DEBUG_INFO("(bss: %d)retry count %u, next retry is %u seconds later\n", g_elink_ctx.is_bss, retry_count, interval);

	return interval * 1000;
}

void _get_server_addr(char **serveraddr, unsigned int *serverport)
{
	int idx = 0;

	DEBUG_INFO("%s: bss %s:%d, abi: %s:%d, retry %d\n", __func__,
			   g_elink_ctx.server.bssaddr, g_elink_ctx.server.bssport,
			   g_elink_ctx.server.addr, g_elink_ctx.server.port, g_elink_ctx.retry_count);

	*serveraddr = NULL;
	*serverport = 0;

	if (g_elink_ctx.is_bss) {
		*serveraddr = g_elink_ctx.server.bssaddr;
		*serverport = g_elink_ctx.server.bssport;
		g_elink_ctx.retry_count = 0;
		g_elink_ctx.retry_cycle = 0;
	} else {
		if (g_elink_ctx.retry_count == 5) {

			for (idx = 0; idx < NUM_OF_BACKUP_SRV; idx++) {
				if (g_elink_ctx.server.backup[idx].port == 0)
					continue;

				if (g_elink_ctx.server.backup[idx].used == 0) {
					*serveraddr = g_elink_ctx.server.backup[idx].addr;
					*serverport = g_elink_ctx.server.backup[idx].port;
					g_elink_ctx.server.backup[idx].used = 1;
					g_elink_ctx.retry_count = 0;
					break;
				}
			}
		} else {
			*serveraddr = g_elink_ctx.server.addr;
			*serverport = g_elink_ctx.server.port;
		}
	}

	return;
}


void _free_elink_config(int freeap)
{
	int i = 0;

	FREE(g_elink_ctx.server.bssaddr);
	FREE(g_elink_ctx.server.addr);

	for (i = 0; i < NUM_OF_BACKUP_SRV; i++) {
		FREE(g_elink_ctx.server.backup[i].addr);
		g_elink_ctx.server.backup[i].used = 0;
	}

	FREE(g_elink_ctx.dev.id);
	FREE(g_elink_ctx.dev.token);

	if (freeap) {
		FREE(g_elink_ctx.ap.mac);
		FREE(g_elink_ctx.ap.vendor);
		FREE(g_elink_ctx.ap.model);
		FREE(g_elink_ctx.ap.swver);
		FREE(g_elink_ctx.ap.hdver);
		FREE(g_elink_ctx.ap.ipaddr);
		FREE(g_elink_ctx.ap.sn);
		FREE(g_elink_ctx.ap.gwmac);
	}
}

int update_ap_info()
{
	char vendor_data[32] = {0};
	char model_data[8]   = {0};
	char url_data[32]    = {0};
	char wl_type[8]      = {0};
	char swversion[32]      = {0};
	char hdversion[32]      = {0};
	char mac[32] = {0};
	char elink_sn[ELINK_SN_LEN + 1] = {0};
	char gateway_mac[32] = {0};

	extra_get_gateway_mac(gateway_mac);
	extra_get_dev_reg_vendor(vendor_data);
	extra_get_dev_reg_model(model_data);
	extra_get_dev_reg_url(url_data);
	extra_get_soft_version(swversion);
	extra_get_hd_version(hdversion);
	extra_get_wan_mac(mac, sizeof(mac));
	extra_get_elink_sn(elink_sn);
	g_elink_ctx.ap.gwmac = strdup(gateway_mac);
	g_elink_ctx.ap.hdver = strdup(hdversion);
	g_elink_ctx.ap.mac = strdup(mac);
	g_elink_ctx.ap.model = strdup(model_data);
	g_elink_ctx.ap.swver = strdup(swversion);
	g_elink_ctx.ap.vendor = strdup(vendor_data);
	g_elink_ctx.ap.sn = strdup(elink_sn);

	_convert_mac(g_elink_ctx.ap.mac);

	if (IS_EMPTY_STRING(g_elink_ctx.ap.vendor)) {
		return ;
	}

	if (IS_EMPTY_STRING(g_elink_ctx.ap.model)) {
		return ;
	}

	if (IS_EMPTY_STRING(g_elink_ctx.ap.swver)) {
		return ;
	}

	if (IS_EMPTY_STRING(g_elink_ctx.ap.hdver)) {
		return ;
	}

	if (IS_EMPTY_STRING(g_elink_ctx.ap.sn) || strlen(g_elink_ctx.ap.sn) < ELINK_SN_LEN) {
		FREE(g_elink_ctx.ap.sn);
		g_elink_ctx.ap.sn = malloc(ELINK_SN_LEN + 1);
		//sprintf(g_elink_ctx.ap.sn, "FFFFFFFFFFFFFFFFFFFFFF%s", g_elink_ctx.ap.mac);
		sprintf(g_elink_ctx.ap.sn, "99DD007001699D08A0F000502B73FB20E0");
		
	}

	if (IS_EMPTY_STRING(g_elink_ctx.ap.ipaddr)) {
	}
}

int _read_elink_config(struct elink_ctx *config)
{
	cJSON *obj = NULL, *bklist = NULL, *item = NULL;
	int i = 0, j = 0;

	_free_elink_config(0);
	update_ap_info();
	obj = _config_to_jobj(ELINKCC_CONFIG);

	if (!obj) {
		config->server.bssport = DEFAULT_BSS_PORT;
		config->server.bssaddr = strdup(DEFAULT_BSS_ADDR);
		g_elink_ctx.is_bootstrap = 1;
		g_elink_ctx.is_bss = 1;

		return -1;
	}

	/*
	{"vendor":"upointech","model":"elinkclt","swver":"1.0","hdver":"0.1",
	"ServerAddr":"192.168.0.6","ServerPort":7777,
	"Backup":[{"ServerAddr":"192.168.0.61","ServerPort":7778},
	          {"ServerAddr":"192.168.0.62","ServerPort":7779}],
	"DevToken":"Nm598EAiC4PlZtGSYHD0HPCNDUXnC9Np",
	"DevID":"Nm598EAiC4PlZtGSYHD0HPCNDUXnC9NpzdmqOCPJudGtwYp2"}
	*/
	_get_string_json_value(obj, "DevToken", &config->dev.token);
	_get_string_json_value(obj, "DevID", &config->dev.id);

	_get_int_json_value(obj, "BSSPort", cJSON_Number, &config->server.bssport);
	_get_string_json_value(obj, "BSSAddr", &config->server.bssaddr);

	if (config->server.bssport == 0)
		config->server.bssport = DEFAULT_BSS_PORT;

	if (IS_EMPTY_STRING(config->server.bssaddr))
		config->server.bssaddr = strdup(DEFAULT_BSS_ADDR);

	_get_int_json_value(obj, "ServerPort", cJSON_Number, &config->server.port);
	_get_string_json_value(obj, "ServerAddr", &config->server.addr);

	if (IS_EMPTY_STRING(config->server.addr) || !IS_VALID_PORT(config->server.port))
		config->is_bss = 1;

	bklist = cJSON_GetObjectItem(obj, "Backup");

	if (cJSON_IsArray(bklist)) {
		json_array_foreach(bklist, i, item) {
			_get_string_json_value(item, "ServerAddr", &config->server.backup[j].addr);
			_get_int_json_value(item, "ServerPort", cJSON_Number, &config->server.backup[j].port);

			if (!IS_EMPTY_STRING(config->server.backup[j].addr) && IS_VALID_PORT(config->server.backup[j].port)) {
				DEBUG_INFO("[%d]: %s:%d\n", j, config->server.backup[j].addr, config->server.backup[j].port);
				j++;
			}

			if (j > (NUM_OF_BACKUP_SRV - 1))
				break;
		}
	}

	cJSON_Delete(obj);

	return 0;

}

void elink_init_ustream(struct elink_ctx *ctx)
{
	memset(&g_elink_ctx, 0, sizeof(struct elink_ctx));

	public_key_init();

	g_elink_ctx.cmd_list.next = &g_elink_ctx.cmd_list;
	g_elink_ctx.cmd_list.prev = &g_elink_ctx.cmd_list;

	g_elink_ctx.req_list.next = &g_elink_ctx.req_list;
	g_elink_ctx.req_list.prev = &g_elink_ctx.req_list;

	g_elink_ctx.spp_list.next = &g_elink_ctx.spp_list;
	g_elink_ctx.spp_list.prev = &g_elink_ctx.spp_list;
}

int connect_cloud_sever()
{
	int sock = 0;
	int ret = 0;
	sock = socket_init();
	if(sock < 0)
	{
		return 0;
	}
	ret = socket_connect(sock, g_elink_ctx.curr_srv, g_elink_ctx.curr_port);
	if(ret < 0)
	{
		socket_close(sock);
		return 0;
	}
	return sock;
}

int elink_cloud_client(void)
{
	char portstr[8] = {0};
	int cloudclient_fd = UNKNOWN_FD, i = 0;

	if (!g_elink_ctx.curr_srv)
		_get_server_addr(&g_elink_ctx.curr_srv, &g_elink_ctx.curr_port);

	DEBUG_INFO("current server %s:%d\n", g_elink_ctx.curr_srv, g_elink_ctx.curr_port);

	if (IS_EMPTY_STRING(g_elink_ctx.curr_srv) || (g_elink_ctx.curr_port == 0)) {
		g_elink_ctx.retry_cycle++;
		uloop_timeout_set(&remote_disconnect_timer, 0);

		if (g_elink_ctx.retry_cycle % 3 == 0) {
			DEBUG_INFO("re-connect bss server\n");
			g_elink_ctx.is_bss = 1;
			/* re-connect bss to get new ability server */
			uloop_timeout_set(&remote_connect_timer, ELINKCC_200MS_INTERVAL);

		} else {
			DEBUG_INFO("re-connect ability server after %d seconds\n", ELINKCC_150M_INTERVAL / 1000);
			g_elink_ctx.retry_count = 0;

			for (i = 0; i < NUM_OF_BACKUP_SRV; i++)
				g_elink_ctx.server.backup[i].used = 0;

			uloop_timeout_set(&remote_connect_timer, ELINKCC_150M_INTERVAL);
		}

		return 0;
	}

	sprintf(portstr, "%u", g_elink_ctx.curr_port);
	cloudclient_fd = connect_cloud_sever();

	if (cloudclient_fd > 0) {
		DEBUG_INFO("elink cloudclient fd: %d\n", cloudclient_fd);
		g_elink_ctx.romote_fd = cloudclient_fd;
		_notify_ap_server(0);
	}else
	{
		DEBUG_INFO("create socket failed: %m\n", errno);
		if(!gpi_wan_get_connect())
		{
			uloop_timeout_set(&remote_connect_timer, ELINKCC_100MS_INTERVAL * 20);
		}else
		{
			if (g_elink_ctx.retry_count == 5)
				g_elink_ctx.retry_count = 0;
			uloop_timeout_set(&remote_connect_timer, _get_retry_interval(g_elink_ctx.retry_count++));
		}
	}

	return 0;
}

int elink_notification(cJSON *obj)
{
	int len = 0, b64len = 0, en_len = 0;
	unsigned char *b64buf = NULL, *en_msg = NULL;
	char *jstr = NULL, *time = NULL, *id = NULL;
	unsigned char outbuf[1024 * 4] = {0};
	if (IS_EMPTY_STRING(g_elink_ctx.keys.server_key)) {
		DEBUG_INFO("ap server not connected, ignore this dev_report msg\n");
		return 0;
	}


	jstr = cJSON_PrintUnformatted(obj);

	if (!jstr) {
		DEBUG_INFO("%s: invalid json obj\n", __func__);
		return -1;
	}

	b64len = _do_b64_cmd((unsigned char *)jstr, strlen(jstr), &b64buf, 1);
	_get_sys_time(&time);
	_get_uuid_string(&id);

	if (b64len > 0 && time) {

		len = sprintf(outbuf, "{\"RPCMethod\":\"PluginNotification\",\"ID\":\"%s\","
					   "\"Plugin_Name\":\"%s\",\"Version\":\"%s\",\"Time\":\"%s\",\"Message\":\"%s\"}",
					   id, ELINK_PLUGIN_NAME, g_elink_ctx.ap.swver, time, b64buf);

	} else {
		DEBUG_INFO("%d: failed to generate PluginNotification msg\n", __LINE__);
	}

	if (len > 0) {
		DEBUG_INFO("(remote) -> %s (%d)\n", outbuf, len);

		if (_do_aes_ecb_crypt(outbuf, len, &en_msg, &en_len, g_elink_ctx.keys.server_key, 1) < 0)
			goto done;

		_send_remote_msg(en_msg, en_len);
		uloop_timeout_set(&remote_ack_timer, ELINKCC_30S_INTERVAL);
	} else {
		DEBUG_INFO("%d: failed to generate msg\n", __LINE__);
	}

done:

	FREE(jstr);
	FREE(time);
	FREE(id);
	FREE(b64buf);
	return 0;
}

void _remote_connect_cb(struct uloop_timeout *timeout)
{
	DEBUG_INFO("%s: %d\n", __func__, __LINE__);
	elink_cloud_client();
}

void _remote_disconnect_cb(struct uloop_timeout *timeout)
{
	struct req_info *p = NULL, *q = NULL;
	struct cmd_info *c = NULL, *d = NULL;
	struct spp_info *e = NULL, *f = NULL;

	list_for_each_entry_safe(c, d, &g_elink_ctx.cmd_list, list) {
		list_del(&c->list);
		FREE(c->content);
		FREE(c);
	}

	list_for_each_entry_safe(p, q, &g_elink_ctx.req_list, list) {
		list_del(&p->list);
		FREE(p->raw);
		FREE(p);
	}

	list_for_each_entry_safe(e, f, &g_elink_ctx.spp_list, list) {
		list_del(&e->list);
		uloop_timeout_cancel(&e->timer);
		FREE(e->id);
		FREE(e);
	}

	if (g_elink_ctx.romote_fd > 0) {
		DEBUG_INFO("%s: close tcp fd %d\n", __func__, g_elink_ctx.romote_fd);
		socket_close(g_elink_ctx.romote_fd);
		g_elink_ctx.romote_fd = UNKNOWN_FD;
	}

	//wait free other info
	FREE(g_elink_ctx.keys.server_key);
	FREE(g_elink_ctx.keys.client_key);
	FREE(g_elink_ctx.keys.challenge_code);
	
	uloop_timeout_cancel(&remote_ack_timer);
	uloop_timeout_cancel(&heartbeat_timer);
}

int elinksdk_main_loop()
{

	/*初始化配置信息*/
	elink_init_ustream(&g_elink_ctx);

	/*从配置文件中读取配置信息*/
	_read_elink_config(&g_elink_ctx);

	/*连接远端服务器*/
	elink_cloud_client();
	/*状态转换*/
	uloop_run();

	uloop_done();

	return 0;
}


static int elinksdk_thread_running = 0;
static char elinksdk_daemon_stack[1024*100];
static cyg_handle_t elinksdk_daemon_handle;
static cyg_thread elinksdk_daemon_thread;
int elinksdk_main()
{
    char *value = NULL;
    
    if(elinksdk_thread_running == 0)
    {
        cyg_thread_create( 8,
                           (cyg_thread_entry_t *)elinksdk_main_loop,
                           0,
                           "ElinkSdk",
                           &elinksdk_daemon_stack,
                           sizeof(elinksdk_daemon_stack),
                           &elinksdk_daemon_handle,
                           &elinksdk_daemon_thread);
        cyg_thread_resume(elinksdk_daemon_handle);
        elinksdk_thread_running = 1;
    }
    return 0;
}
