/*
	E-link protocol implement, 
	All will be done in function: proto_elink_main_loop (Finite state mechine).

	Wangxinyu.yy@gmail.com 
	2016.8.8
*/

#include <stdio.h>
#include <string.h>
//#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>

#include "proto.h"
#include "elink_log_debug.h"
#include "extra.h"
#include "cJSON.h"
//#include "timerlib.h"
#include "list.h"
//#include "base64_2.h"
//#include "./aes_cbc/aes.h"
#include "aes.h"
#include "sys_timer.h"
#include "bcmnvram.h"
#include "elink_common.h"
//#include "clog.h"
int g_log_level = LOG_LEVEL_HIGHTST;


struct elink_ack
{
	unsigned int sequence;
    unsigned int  time_id; // A time stamp     
    struct list_head list;
    
};

struct list_head gWaitAckList;

#define MAX_RECEIVE_BUF_LENGTH (5 * 2024)

/*
	Global parameters. 
*/
int          gDeviceState;
unsigned int gDeviceSeq;
int 		 gDeviceSock;
unsigned char gDeviceAesKey[32] = "1234567890abcdef";
unsigned char gDeviceAesIV[32] = {0};
//unsigned char gDeviceRecvBuf[MAX_RECEIVE_BUF_LENGTH]; /* Receive action will do only in main thread. */
int 		 gDeviceKeepAliveSeq = 0;


#define CHECK_JSON_VALUE(tmp) if(NULL == tmp) \
								{log_msg(LOG_LEVEL_ERROR, "JSON msg error.%s:%d\n", __FUNCTION__, __LINE__); goto bad_msg;}
#define MIN(a,b) ((a)<(b)?(a):(b))

char *stateStr[]={
    "Reset status",
	"Initialize status.",
    "Connect status.",
    "Nogotiation key status.",
    "DH exchange key status",
    "Register status",
    "Config status"  
};


int  print_package(unsigned char *buf, int len)
{
	int i = 0;
	log_debug("Begin*************\n");
	for(i = 0; i < len; i++)
	{
		log_debug("%02X ", buf[i]);
		if(0 == (i+1)%16)
			log_debug("\n");
	}

	log_debug("\nEND****************\n");

    return i;
}


static inline int get_sequence()
{
	return ++gDeviceSeq;
}

static inline int current_sequence()
{
	return gDeviceSeq;
}

static inline int shift_sequence()
{
	return gDeviceSeq+=10000;
}


static inline int get_device_state()
{
	return gDeviceState;
}

int is_connect_gateway()
{
	if(get_device_state() == DEVICE_STATE_CONFIG )
		return 1;
	return 0;
}

int set_device_state(int state)
{
	gDeviceState = state;

	//log_debug("E-link Device Enter : %s\n", stateStr[gDeviceState]);

	/*
	    Enter Regist state , Begin to keep alive.
	*/
	if(DEVICE_STATE_REG == gDeviceState)
	{
		/*已经在线程初始化的地方起了心跳报文定时器,时序要求严格可以将发心跳包的定时器
		放到这里*/
		//wakeup_keep_alive_thread();
	}
	/*
	    Enter Config state ,And Switch to bridge Mode.
	    Bugs, maybe loss package during switch to bridge mode.
	*/
	/*路由器切换到桥模式*/
	if(DEVICE_STATE_CONFIG == gDeviceState )
	{
		/*函数暂时注释,后续需要打开实现*/
		//extra_skip_quick();
		//extra_enter_conf_state();
	}
    
	return 0;
}




void reset_device_state(void *arg)
{
	TRACE;
    set_device_state(DEVICE_STATE_RESET);
    shift_sequence();
}

void real_sleep(int secs)
{
	cyg_thread_delay(secs * 100);
}



int add_to_wait_ack_list(unsigned int seq, long sec)
{
	//每发一个心跳包就添加定时器
	//timeout((timeout_fun *)reset_device_state, NULL, 17 * (DO_TIMER_MIN_TIME));

	struct elink_ack *node;

	node = malloc(sizeof(*node));
    if(NULL == node)
    {
		return -1;
    }
	node->sequence = seq;
    /* Maybe failed to add timer. */
    node->time_id = (unsigned int)cyg_current_time();

    list_add(&node->list, &gWaitAckList);
    

    return 0;
}




void del_from_wait_ack_list(unsigned int seq)
{
	int i = 0;
	struct elink_ack *node , *next_node;
    
	log_debug("%s : seq = %d\n", __FUNCTION__, seq);
    show_wait_ack_list();
    list_for_each_entry_safe(node,next_node,&gWaitAckList,list)
    {
		//if(seq >= node->sequence)
        {
        	log_debug("Delete sequence=%d, timer_id=%d\n", node->sequence, node->time_id);

            list_del(&node->list);
            free(node);
            //break;
        }
    }
}

void clear_wait_ack_list()
{
	struct elink_ack *node , *next_node;
        
    list_for_each_entry_safe(node,next_node,&gWaitAckList,list)
    {
    	log_debug("Delete sequence=%d, timer_id=%d\n", node->sequence, node->time_id);
        list_del(&node->list);
        free(node);
    }
}


void show_wait_ack_list(void)
{
	struct elink_ack *node;
	
	TRACE;
    
    list_for_each_entry(node,&gWaitAckList,list)
    {
    	log_debug("SHOW : sequence=%d, timer_id=%d\n", node->sequence, node->time_id);
    }

}


int free_wait_ack_list(void)
{
	struct elink_ack *node;
    struct elink_ack *tmp;

    list_for_each_entry_safe(node,tmp, &gWaitAckList,list)
	{
		/* Clear timer. */
		//timer_rem(node->time_id, NULL);
        
        list_del(&node->list);
    }
    
    INIT_LIST_HEAD(&gWaitAckList);
    
	return 0;
}


void check_wait_ack_list()
{
	int i = 0;
	struct elink_ack *node , *next_node;
    unsigned int tm = (unsigned int)cyg_current_time();
    
	log_debug("%s : %d\n", __FUNCTION__, __LINE__);
    show_wait_ack_list();
    list_for_each_entry_safe(node,next_node,&gWaitAckList,list)
    {
		if((tm -  node->time_id) > DELAY_TMAX_WAIT_ACK*100)
        {
        	log_debug("Timeout ACK sequence=%d, timer_id=%d, Reset status.\n", node->sequence, node->time_id);
            list_del(&node->list);
            free(node);
            reset_device_state(NULL);
            break;
        }
    }
}



int aes128_encrypt(char *data, int len)
{
    int en_len = 0;
      
	if(len%16 != 0)
	{
		log_msg(LOG_LEVEL_ERROR,"%s : len must exact division By 16 \n", __FUNCTION__);
		return -1;
    }

    char *encode = NULL;
    encode = (char *)malloc(len);
    if(NULL == encode)
    {
        return -1;
    }
    
    memset(encode, 0x0, len);
    
    //Real encryp here.
    log_debug("before encode : %s\n", data);
 #if 0   
    aes_encrypt_wrap(encode, len, data, len, &en_len);
#else
    AES_KEY aesKey;
    memset(&aesKey, 0x0, sizeof(aesKey));
    memset(gDeviceAesIV, 0x0, sizeof(gDeviceAesIV));
    
    AES_set_encrypt_key(gDeviceAesKey,128, &aesKey);
    AES_cbc_encrypt(data, encode,
		     len, &aesKey,
		     gDeviceAesIV, AES_ENCRYPT);
#endif
    memcpy(data, encode, len);
    
    free(encode);
	return 0;
}

int aes128_decrypt(char *data, int len)
{
    char *decode = NULL;
    int de_len;
    
	if(len%16 != 0)
	{
		log_msg(LOG_LEVEL_ERROR,"%s : len must exact division By 16 \n", __FUNCTION__);
		return -1;
    }

    //Real encryp here.
    
    decode = (char *)malloc(len);
    if(NULL == decode)
    {
        return -1;
    }
    memset(decode, 0x0, len);
    //Real encryp here.

    
    log_debug("decrypt len = %d", len);
  #if 0  
    aes_decrypt_wrap(decode, len, data, len, &de_len);
  #else
      AES_KEY aesKey;
      memset(&aesKey, 0x0, sizeof(aesKey));
      memset(gDeviceAesIV, 0x0, sizeof(gDeviceAesIV));
      
      AES_set_decrypt_key(gDeviceAesKey,128, &aesKey);
      AES_cbc_encrypt(data, decode,
               len, &aesKey,
               gDeviceAesIV, AES_DECRYPT);
  #endif

    memcpy(data, decode, len);
    log_debug("after decode: %s", data);
    free(decode);
    
	return 0;
}


/*
	Input:
		type :  
		sequence:
		msg : contain root 
			like : { "wxy":123} , and we will just use it's child, "wxy":123.
	Return:
		negative (-1) stand for error.
		0 is success.

*/
int send_msg_to_gateway(char *type, unsigned int sequence, cJSON *msg)
{
	int dateLen = 0;
	int ret = 0;
    char mac[32] = {0};
	
    char *buff = NULL;
    struct ELINK_HEADER *packet;
 
	cJSON *root = NULL;
	char *out = NULL;
	int outLen = 0;

	/* Just use it child, we have new root below. */
    if(msg)
	    msg = msg->child;

	extra_get_wan_mac(mac, sizeof(mac));

	root = cJSON_CreateObject();
	if(NULL == root)
    {    
    	log_error("cJSON_CreateObject error."LOG_TRACE_STRING);
        return -1;
    }
    /* Add common json msg. */
	cJSON_AddStringToObject(root, "type", type);
	cJSON_AddNumberToObject(root, "sequence", sequence);
	cJSON_AddStringToObject(root, "mac", mac);

	/* add special msg. */
	if(msg)
  	  cJSON_AddItemToArray(root, msg);

	out = cJSON_Print(root);
	outLen = strlen(out);

    if(root)
		cJSON_Delete(root);
    
	buff = malloc(outLen + ELINK_HEADER_LENGTH + 16);
    if(NULL == buff)
    {
    	free(out);
		return -1;
    }
	memset(buff, 0x0, outLen + ELINK_HEADER_LENGTH + 16);
    
    /* Store msg. */
    packet = (struct ELINK_HEADER *)buff;
	memcpy(packet->data, out, outLen);
        
	/* Encapsulate Magic*/
	packet->magic = htonl(ELINK_HEADER_MAGIC);
	/* Encapsulate Length */
	dateLen = MOD_16_INTGER(outLen);
	packet->length = htonl(dateLen);

    log_msg(LOG_LEVEL_TRACE, "We will send:\n%s, len=%d(%d)\n", packet->data, outLen, dateLen);     

    // AES encrypt.
    if(get_device_state() == DEVICE_STATE_REG
        || get_device_state() == DEVICE_STATE_CONFIG)
    {
        log_debug("In %d status\n", get_device_state());
	    aes128_encrypt(packet->data, dateLen);
    }
    
	ret = socket_send(gDeviceSock, (void *)packet, dateLen + ELINK_HEADER_LENGTH);
    log_debug("socket_send ret=%d\n", ret);

	free(buff);
    free(out);
    
	return ret;
}




/*
	Input:
		timeout :  max time to wait ( seconds )
		buff:		  
		buffLen : 
		
	Return:
		negative (-1) stand for error.
		success return data recived Length.

*/

int recv_msg_from_gateway(int timeout, char *buff, int buffLen)
{
	TRACE;
    int ret = 0;
    struct ELINK_HEADER *head;
    int dataLen = 0;
    char *data;

    head = (struct ELINK_HEADER *)buff;
    data = head->data;

    ret = socket_select(gDeviceSock, timeout);
	if(ret <= 0)
    {
    	log_debug(LOG_TRACE_STRING);
		return ret;
    }

	log_debug("%s : Data come in.\n", __FUNCTION__);
	/* Receive header. */
	ret = socket_receive_full(gDeviceSock, head, ELINK_HEADER_LENGTH);    
	if(ret < 0)
    {
    	log_error(LOG_TRACE_STRING);
		return -1;
    }
	/* Fast check. */
    dataLen = ntohl(head->length);

	if(ntohl(ELINK_HEADER_MAGIC) != head->magic )
    {
    	log_error(LOG_TRACE_STRING);
		return -1;
    }
    TRACE;
    log_debug("rec_data len :%d\n", dataLen);
    
    if(dataLen > buffLen)
    {
        log_error("%s : buff is too small (%d), need length=%d.\n", 
        				__FUNCTION__, buffLen, dataLen);
        return -1;
    }  
	/* Receive data. */
	ret = socket_receive_full(gDeviceSock, buff, dataLen);    
	if(ret < 0)
    {
    	log_error(LOG_TRACE_STRING);
		return -1;
    }

    /* AES decrypt. */
    if(get_device_state() == DEVICE_STATE_REG
        || get_device_state() == DEVICE_STATE_CONFIG)
    {
	    ret = aes128_decrypt(buff, dataLen);
        if(ret < 0){
            log_debug("decrypt error . \n");
        }
    }
    TRACE;
   // memcpy(buff, head->data, MIN(dataLen, buffLen));

	log_debug("receive data: %s\n", buff);
   
	return dataLen;
}


int set_reg_config(char *buff)
{
    int ret = -1;
    cJSON *root = NULL;
	cJSON *type = NULL;
	cJSON *seq	= NULL;
    cJSON *status = NULL;
    cJSON *set = NULL;

    root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
    type = cJSON_GetObjectItem(root, "type");
    CHECK_JSON_VALUE(type);
    seq = cJSON_GetObjectItem(root, "sequence");
    CHECK_JSON_VALUE(seq);  
    status = cJSON_GetObjectItem(root,"status");
    CHECK_JSON_VALUE(status);  
    set = cJSON_GetObjectItem(root,"set");
    CHECK_JSON_VALUE(set);
    
    
    
    ret = 0;

    
bad_msg:
    if(root)
    {
        cJSON_Delete(root);
    }
    return ret;
}



int deal_data_ack(int seq)
{
    int ret = 0;
    
    del_from_wait_ack_list(seq);

    return ret;    
}



void print_cjson(cJSON *root)
{
    char *out = NULL;
    out = cJSON_Print(root);

    log_debug("out ==== %s\n", out);
    
    free(out);
}



char *cJSON_type[] = 
{
	"cJSON_False",
	"cJSON_True",
	"cJSON_NULL",
	"cJSON_Number",
	"cJSON_String",
	"cJSON_Array",
	"cJSON_Object",
	NULL

};

int deal_data_cfg(char *buff,int active)
{
	int ret = 0;
	cJSON *root   = NULL;
	cJSON *type   = NULL;
	cJSON *seq	  = NULL;
	cJSON *set_ap    = NULL;

	set_ap_info set_ap_cfg_info[8] = {0};
	set_wifi_info set_wifi_cfg_info[8] = {0};

	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
	type = cJSON_GetObjectItem(root, "type");
	CHECK_JSON_VALUE(type);
	seq = cJSON_GetObjectItem(root, "sequence");
	CHECK_JSON_VALUE(seq);  
	set_ap = cJSON_GetObjectItem(root,"set");
	CHECK_JSON_VALUE(set_ap);

	process_all_data_cfg(buff,active);
bad_msg:
    if(root)
        cJSON_Delete(root);

    ret = send_msg_to_gateway(ELINK_MESSAGE_TYPE_ACK, seq->valueint, NULL);

    return ret;
}



int deal_deassociation(char *buff)
{
	int ret = 0;
	cJSON *root   = NULL;
	cJSON *type   = NULL;
	cJSON *seq	  = NULL;
	cJSON *set_ap    = NULL;

	set_ap_info set_ap_cfg_info[8] = {0};
	set_wifi_info set_wifi_cfg_info[8] = {0};

	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
	type = cJSON_GetObjectItem(root, "type");
	CHECK_JSON_VALUE(type);
	seq = cJSON_GetObjectItem(root, "sequence");
	CHECK_JSON_VALUE(seq);  
	set_ap = cJSON_GetObjectItem(root,"set");
	CHECK_JSON_VALUE(set_ap);

	exec_deassociation(set_ap);
bad_msg:
    if(root)
        cJSON_Delete(root);

    ret = send_msg_to_gateway(ELINK_MESSAGE_TYPE_ACK, seq->valueint, NULL);

    return ret;
}

int get_client_rssi_info(char *buff)
{
	int ret = 0;
	cJSON *root   = NULL;
	cJSON *type   = NULL;
	cJSON *seq	  = NULL;
	cJSON *set_ap    = NULL;
	cJSON *get_info   = NULL;
	cJSON *output_root   = NULL;
	cJSON *output_info   = NULL;
	char *test = NULL;
	set_ap_info set_ap_cfg_info[8] = {0};
	set_wifi_info set_wifi_cfg_info[8] = {0};

	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
	type = cJSON_GetObjectItem(root, "type");
	CHECK_JSON_VALUE(type);
	seq = cJSON_GetObjectItem(root, "sequence");
	CHECK_JSON_VALUE(seq);  
	get_info = cJSON_GetObjectItem(root,"get");
	CHECK_JSON_VALUE(get_info);
	
	output_root = cJSON_CreateObject();
	cJSON_AddItemToObject(output_root, "rssiinfo",output_info = cJSON_CreateArray());
	get_sta_rssi_info(get_info,output_info);

	test = cJSON_Print(output_root);
	printf("=======%s========%s [%d]\n",test, __FUNCTION__, __LINE__);
	free(test);
	ret = send_msg_to_gateway(ELINE_MESSAGE_TYPE_RSSIINFO, seq->valueint, output_root);
bad_msg:
	if(root)
		cJSON_Delete(root);
	if(output_root){
	    free(output_root);
	}
    	return ret;
}

int send_get_status_data(cJSON *get_info, unsigned int seq)
{
	int ret = 0;
	cJSON *send_root = NULL;
	cJSON *send_status = NULL;
	cJSON *get_status = NULL;
	cJSON *get_name   = NULL;
	int get_array_len = 0;
	int i = 0;

	send_root = cJSON_CreateObject();
	send_status = cJSON_CreateObject();

	cJSON_AddItemToObject(send_root, "status", send_status);

	get_array_len = cJSON_GetArraySize(get_info);
	for(i; i < get_array_len; i++)
	{
		get_status = cJSON_GetArrayItem(get_info,i);
		get_name = cJSON_GetObjectItem(get_status,"name");
		get_route_status_info(send_status,get_name->valuestring);
		DEBUG_INFO(" get status info :  %s \n", get_name->valuestring);
	}

	ret = send_msg_to_gateway(ELINE_MESSAGE_TYPE_STATUS, seq, send_root);
	log_debug("send status info success");
	if(send_root){
	    free(send_root);
	}

	return ret;
}

int deal_data_get_status(char *buff)
{
    int ret = 0;
    cJSON *root = NULL;
    cJSON *seq  = NULL;
    cJSON *get_info   = NULL;


    get_status_info wifi_status_info[10] = {0};
        
    root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
    seq = cJSON_GetObjectItem(root, "sequence");
	CHECK_JSON_VALUE(seq);
    get_info = cJSON_GetObjectItem(root,"get");
    CHECK_JSON_VALUE(get_info);

    //buid send status info;

    send_get_status_data(get_info, seq->valueint);
    
bad_msg:
    if(root)
        cJSON_Delete(root);

    return ret;
}





int parse_config_status_data(char *buff)
{ 
    int ret = -1;
    cJSON *root = NULL;
	cJSON *type = NULL;
	cJSON *seq	= NULL;
    
	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
	type = cJSON_GetObjectItem(root, "type");
	CHECK_JSON_VALUE(type);
	seq = cJSON_GetObjectItem(root, "sequence");
	CHECK_JSON_VALUE(seq);	

	clear_wait_ack_list();

    if(!strcmp(ELINK_MESSAGE_TYPE_ACK, type->valuestring))
    {
    	//parse keepalive ack or dev_report ack.
    	log_debug("receive keepalive ack or dev_report ack. \n");
        deal_data_ack(seq->valueint);    	
        ret = 0;
        // If ack seq == keep alive seq, change it.
        clear_wait_ack_list();
    }
    else if(!strcmp(ELINE_MESSAGE_TYPE_CFG, type->valuestring))
    {
    	// send cfg info , call extra, send ack;
    	log_debug("receive cfg info ,call extra. \n");
    	//call extra , create json node;
		if(nvram_match(SYSCONFIG_WORKMODE, "bridge"))
			deal_data_cfg(buff,1);  
		else
		{
			deal_data_cfg(buff,0);
			extra_enter_conf_state();
		}		
        ret = 0;
        // Wake up device Report thread.
        //wakeup_dev_report_thread();
    }
    else if(!strcmp(ELINE_MESSAGE_TYPE_GET_STATUS, type->valuestring))
    {
        // send get_status ,send status;
        log_debug("receive get_status ack\n");
        //call extra , create json node;
        //send data;
        deal_data_get_status(buff); 
        //tdSyslog(LOG_SYSTEM, "%s", "e-link send status info success.");
        ret = 0;
    }
    else if(!strcmp(ELINE_MESSAGE_TYPE_DEASSOC, type->valuestring))
    {
        log_debug("receive deassociation \n");

        deal_deassociation(buff); 
        ret = 0;
    }
    else if(!strcmp(ELINE_MESSAGE_TYPE_GETRSSIINFO, type->valuestring))
    {
        log_debug("receive getrssiinfo \n");

        get_client_rssi_info(buff); 
        ret = 0;
    }
    else
    {
        log_debug("data type false. type : %s\n", type->valuestring);
    }

bad_msg:
	if(root)
		cJSON_Delete(root);  
    
    return ret;
}




int deal_config_status_msg()
{
	int ret = 0;
	char buff[MAX_RECEIVE_BUF_LENGTH] = {0};

    ret = recv_msg_from_gateway(2, buff,sizeof(buff)-1);
    if(ret == 0)
    {
		//time out. No message coming.
		log_debug("recv config data timeout . \n");
		return 0;
    }
	else if(ret < 0)
	{
		//error.
		log_error("recv config data error. \n");
		return -1;
	}
	
    ret = parse_config_status_data(buff);

	return ret;
}


int send_dev_reg_info_request()
{
    TRACE;
	int ret = -1;
	cJSON *root = NULL;
	cJSON *data = NULL;
    cJSON *child = NULL;

    char vendor_data[32] = {0};
    char model_data[8]   = {0};
    char url_data[32]    = {0};
    char wl_type[8]      = {0};
	char swversion[32]      = {0};
	char hdversion[32]      = {0};

    unsigned int cur_sequence = 0;
	/*
         	"data":
	{
		"vendor":	"tenda",
		"model":	"AC9",
		"swversion": "V15.03.4.18",     #软件版本号
		"hdversion" : "V1.0",    #硬件版本号
		"url":		"xxxx",  # 是给各厂商做自己产品介绍的链接地址
		"wireless":	"yes",  #标识该设备来自有线还是无线："yes","no"
	}
	*/
	root = cJSON_CreateObject();
    data = cJSON_CreateObject();   //no free
    cJSON_AddItemToObject(root, "data", data);
	/*以下wxtra函数暂时注释,协议跑起来的时候要开放*/

    extra_get_dev_reg_vendor(vendor_data);
    extra_get_dev_reg_model(model_data);
    extra_get_dev_reg_url(url_data);
	extra_get_soft_version(swversion);
	extra_get_hd_version(hdversion);
	
    //extra_get_dev_reg_wireless(char * wireless);  //无需打开
    
    cJSON_AddStringToObject(data, "vendor", vendor_data);
    cJSON_AddStringToObject(data, "model", model_data);
    cJSON_AddStringToObject(data, "swversion", swversion);
    cJSON_AddStringToObject(data, "hdversion", hdversion);
    cJSON_AddStringToObject(data, "url", url_data);
	cJSON_AddStringToObject(data, "wireless", "yes");

    cur_sequence = get_sequence();
    ret = send_msg_to_gateway(ELINK_MESSAGE_TYPE_DEV_REGISTER, cur_sequence, root);
    
    add_to_wait_ack_list(cur_sequence,DELAY_T3);
	if(root)
    {    
    	/* Just free this node (root). */
		free(root);
	}

	return ret;
}

int wait_dh_reply(int timeout)
{
	TRACE;
    int ret = -1;
	char buff[MAX_RECEIVE_BUF_LENGTH] = {0};
    int buffLen = 0;
    char decode_dh_key[128] = {0};
    char decode_dh_p[128] = {0};
    char decode_dh_g[128] = {0};
    
    ret = buffLen = recv_msg_from_gateway(timeout, buff, sizeof(buff) -1);
    if(ret <= 0)
    {
    	log_error(LOG_TRACE_STRING);
		return -1;
    }

	log_debug("wait_dh_reply:%s\n", buff);
	
    /* Parse Json Data. 
	   Example:
	   {
			"type":		"dh",
			"sequence":	number,		#每次使用后+1，家庭网关回应组网设备的sequence与组网设备发给家庭网关的sequence相同
			"mac":		"mac",			#WAN port默认MAC，格式："00112233ABCD"
			
			"data":
			{
				"dh_key":	"DH key",	#BASE64编码后的DH public key
				"dh_p":		"DH_P",#BASE64编码后的DH P值
				"dh_g":		"DH_G",#BASE64编码后的DH G值，DH的G值使用十进制的2或者5
			},
		}
    */
	cJSON *root;
    cJSON *type;
	cJSON *seq;
    cJSON *data;

	cJSON *dh_key;
	cJSON *dh_p;
	cJSON *dh_g;
	/* For parse check, set "ret = -1;" 
	   Attention : macro "CHECK_JSON_VALUE"
    */
    ret = -1;
    root = cJSON_Parse(buff);
    CHECK_JSON_VALUE(root);
    type = cJSON_GetObjectItem(root, "type");
    CHECK_JSON_VALUE(type);
    seq = cJSON_GetObjectItem(root, "sequence");
    CHECK_JSON_VALUE(seq);
    
    data = cJSON_GetObjectItem(root, "data");
    CHECK_JSON_VALUE(data);

    dh_key = cJSON_GetObjectItem(data,"dh_key");
    CHECK_JSON_VALUE(dh_key);

    ret = base64_decode(dh_key->valuestring,decode_dh_key,sizeof(decode_dh_key));    
    log_debug("decode64 dh_key : ret=%d \n", ret);
    print_package(decode_dh_key,16);
   
    ret = 0;

	log_debug("wait_nogo_key_type: type=%s,seq=%d\n", 
        					type->valuestring, seq->valueint);
    
    /* Check Type */
    if(0 != strcmp(ELINK_MESSAGE_TYPE_DH_ALGO , type->valuestring))
    {
    	TRACE;
		ret = -1;
    }
    
	/* Check sequence. */
    if(current_sequence() != seq->valueint)
    {
    	TRACE;
		ret = -1;
    }
    
    /* Check Others. */
  
    // Call DH ALGO 
    //Calculate AES key
    //copy to  gDeviceAesKey
    DH_calculate_wrap(gDeviceAesKey, 16,decode_dh_key,16);
    
bad_msg:

	if(root)
		cJSON_Delete(root);
	
	return ret;
}




int dh_get_key(char *pubkey, int pubkeyLen, char *prime, int primeLen)
{
	int ret = -1;
    
    if(NULL == pubkey || NULL == prime)
    {
        return ret;
    }

    ret = DH_init_wrap(pubkey, pubkeyLen, prime, primeLen);	
	if(ret < 0)
    {
		log_debug("DH_init_wrap error. \n");
        return -1;
    }
    return ret;
}




int send_dh_request()
{
	TRACE;
	int ret = -1;
	cJSON *root = NULL;
	cJSON *data = NULL;
    cJSON *child = NULL;

    char dh_key[128] = {0};
    char dh_p[64] = {0};
    char dh_g[64] = "\x05";
    char dh_key_base64[256] = {0};
    char dh_p_base64[128] = {0};
    char dh_g_base64[128] = {0};

    ret = dh_get_key(dh_key, sizeof(dh_key), dh_p, sizeof(dh_p));
    if(ret < 0)
    {
        log_error("Error. dh_get_key.\n");
        return -1;
    }
    base64_encode(dh_key, 16, dh_key_base64, sizeof(dh_key_base64));
    log_debug("dh_key_encode64 : %s \n", dh_key_base64);
    
    base64_encode(dh_p, 16, dh_p_base64, sizeof(dh_p_base64));
    log_debug("dh_p_encode64 : %s\n", dh_p_base64);
    
    base64_encode(dh_g, 1, dh_g_base64, sizeof(dh_g_base64));
    log_debug("dh_g_encode64 : %s\n", dh_g_base64);
    

    /* {
		
			"data":
			{
				"dh_key":	"DH key",	#BASE64编码后的DH public key
				"dh_p":		"DH_P",#BASE64编码后的DH P值
				"dh_g":		"DH_G",#BASE64编码后的DH G值，DH的G值使用十进制的2或者5
			},
		}
	*/
	root = cJSON_CreateObject();
    data = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", data);
    
    cJSON_AddStringToObject(data, "dh_key", dh_key_base64);
    cJSON_AddStringToObject(data, "dh_p", dh_p_base64);
    cJSON_AddStringToObject(data, "dh_g", dh_g_base64);

    ret = send_msg_to_gateway(ELINK_MESSAGE_TYPE_DH_ALGO, get_sequence(), root);
    
	if(root)
    {    
    	/* Just free this node (root). */
		free(root);
	}
    
	return ret;
}



int send_nogo_key_type()
{
	int ret = -1;
	cJSON *root = NULL;
	cJSON *array = NULL;
    cJSON *child = NULL;

    /* {
	    	"keymodelist":
			[
				{"keymode":"dh"}
			] 
		}
	*/
	root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "keymodelist", array);
    
    child = cJSON_CreateObject();
    cJSON_AddStringToObject(child, "keymode", "dh");
    cJSON_AddItemToArray(array, child);
	cJSON_AddStringToObject(root, "version", ELINK_VERSION);

    ret = send_msg_to_gateway(ELINK_MESSAGE_TYPE_KEY_NEG_REQ, get_sequence(), root);

	if(root)
    {    
    	/* Just free this node (root). */
		free(root);
	}
    
	return 0;
}



int wait_nogo_key_type(int timeout)
{
	TRACE;
    int ret = -1;
	char buff[MAX_RECEIVE_BUF_LENGTH] = {0};
    int buffLen = 0;
    
    ret = buffLen = recv_msg_from_gateway(timeout, buff, sizeof(buff) -1);
	TRACE;

    if(ret <= 0)
    {
    	log_error(LOG_TRACE_STRING);
		return -1;
    }
	
    /* Parse Json Data. 
	   Example:
	   	{
			"type":		"keyngack",
			"sequence":	number,	
			"mac":		"mac",		
			
			"keymode":	"dh",		#"ecdh" or "dh"
		}
    */
	cJSON *root;
    cJSON *type;
	cJSON *seq;
    cJSON *keymode;

	/* For parse check, set "ret = -1;" 
	   Attention : macro "CHECK_JSON_VALUE"
    */
    ret = -1;
    root = cJSON_Parse(buff);	
    CHECK_JSON_VALUE(root);
    type = cJSON_GetObjectItem(root, "type");
    CHECK_JSON_VALUE(type);
    seq = cJSON_GetObjectItem(root, "sequence");
    CHECK_JSON_VALUE(seq);
    keymode = cJSON_GetObjectItem(root, "keymode");
    CHECK_JSON_VALUE(keymode);

    ret = 0;

	log_debug("wait_nogo_key_type: type=%s,seq=%d,keymode=%s\n", 
        					type->valuestring, seq->valueint, keymode->valuestring);
    
    /* Check Type */
    if(0 != strcmp(ELINK_MESSAGE_TYPE_KEY_NEG_ACK , type->valuestring))
    {
		ret = -1;
    }
    
	/* Check sequence. */
    if(current_sequence() != seq->valueint)
    {
		ret = -1;
    }
    
    /* Check Key mode */
    if(0 != strcmp(ELINK_MESSAGE_TYPE_DH_ALGO , keymode->valuestring))
    {
		ret = -1;
    }
    
bad_msg:

	if(root)
		cJSON_Delete(root);
	
	return ret;
}




struct elink_client clinets_new[20];
struct elink_client clinets_old[20];
int client_number = 0;

int send_devs_report(int seq, int len)
{
	int ret = 0;
	cJSON *root = NULL;
	cJSON *dev_array = NULL;	
    cJSON *child = NULL;

    int i = 0;
    char mac_info[32]  = {0};
    char vmac_info[32] = {0};
    int connecttype = 0;
	/*
	"dev":
	[
		{
			"mac":	"mac",	#下挂设备的MAC，格式："00112233ABCD"
			"vmac":	"mac",	#下挂设备在中继设备下的虚拟MAC，格式："00112233ABCD"
			"connecttype":	number	#下挂设备与网关的连接形式(0 有线，1无线)
		}
	] // "dev"
	*/
	root = cJSON_CreateObject();
	dev_array= cJSON_CreateArray();
	cJSON_AddItemToObject(root,"dev",dev_array);

    for(i = 0; i < len; i++)
    {   
        child = cJSON_CreateObject();
        strcpy(mac_info, clinets_old[i].mac);
        strcpy(vmac_info, clinets_old[i].mac);
        connecttype = clinets_old[i].connType;
        //log_debug("%s [%d][mhw]:mac = %s\n", __FUNCTION__, __LINE__, mac_info);
        //log_debug("%s [%d][mhw]:vmac= %s\n", __FUNCTION__, __LINE__, vmac_info);
        
        cJSON_AddStringToObject(child, "mac", mac_info);
    	cJSON_AddStringToObject(child, "vmac", vmac_info);
    	cJSON_AddNumberToObject(child, "connecttype", connecttype);
        cJSON_AddItemToArray(dev_array, child);
    }

	ret = send_msg_to_gateway(ELINE_MESSAGE_TYPE_DEV_REPORT, seq, root);
	log_debug("send dev_status .\n");

	if(root)
    {    
    	/* Just free this node (root). */
		free(root);
	}
	return ret;
}

struct roaming_timeout   *process_roaming_hook = NULL;
int register_roaming_process(struct roaming_timeout *timeout,int time)
{
	timeout->exprie_time = cyg_current_time()/100 + time;
	process_roaming_hook = timeout;
}

int roaming_process_canle()
{
	process_roaming_hook = NULL;
}

void roaming_process()
{
	unsigned long long cur_time = 0;
	roaming_timeout_handler cb = NULL;
	if(process_roaming_hook)
	{
		cur_time = cyg_current_time()/100;
		if(cur_time >= process_roaming_hook->exprie_time)
		{
			cb = process_roaming_hook->cb;
			process_roaming_hook = NULL;
			cb(NULL);
			
		}
	}
			
}


int send_roaming_report(cJSON *root)
{
	int ret = 0;
	int current_seq = get_sequence();
	ret = send_msg_to_gateway(ELINE_MESSAGE_TYPE_CFG, current_seq, root);
	return ret;
}

int dev_status_is_change(int *plen)
{
	/*change :1; nochange :0*/
	int ret = 0;
    int change = 0;
    int i = 0, j = 0;

    #if 1
    memset(clinets_new, 0x0, sizeof(elink_client)*20);
    extra_get_client_mac(clinets_new, plen);
    if(client_number != *plen)
    {
        change = 1;
        goto out;
    }
    
    for(i = 0; i < client_number; i++)
    {      
        change = 1;
        for(j = 0; j < client_number; j++)
        {    
            if(0 == strcmp(clinets_new[i].mac, clinets_old[j].mac))
            { 
                change = 0;
                break;
            }
        }

        if(change)
            break;
    }
 out:   
    memset(clinets_old, 0x0, sizeof(elink_client)*20);
    memcpy(clinets_old, clinets_new, sizeof(elink_client)*20);
    client_number = *plen;
	#endif
	return change;
}

void *proto_elink_keep_alive(void *args)
{
	int ret = 0;
	int current_seq = 0;	

	if( get_device_state() < DEVICE_STATE_REG)
	{
		return NULL;
	}
    
	log_debug("Send keep alive package.\n");
    
	current_seq = get_sequence();

	ret = send_msg_to_gateway(ELINK_MESSAGE_TYPE_KEEP_ALIVE, current_seq,NULL);
    if(ret <= 0)
    {
        log_debug("send keepalive error.\n");
        //some ... to Reset state.
        log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
        reset_device_state(NULL);

		return NULL;
	}
    
	log_debug("send keepalive succee\n");

    add_to_wait_ack_list(current_seq,DELAY_T4);

    return NULL;
}

void *proto_elink_devs_status_check()
{
	int ret = 0;
	int change = 0;
	int current_seq = 0;
    int len = 0;
	
	log_debug("%s : %d\n", __FUNCTION__, __LINE__);
	/*
		How to check devices behand Chnaged ?
		Device report package.
	*/
	log_debug("e-link : Device report package.\n");
	change = dev_status_is_change(&len);
	if(1 == change)
    {
		//send package, add timer in list.
        log_debug("e-link : dev status is change.\n");
		current_seq = get_sequence();
		ret = send_devs_report(current_seq, len);
		//add_to_wait_ack_list(current_seq,DELAY_T5);
	}
    
    return NULL;
}




int proto_elink_init()
{
	srand(time(NULL));

    if(strlen(nvram_safe_get("elink_debug")) > 0)
    {
        g_log_level = atoi(nvram_safe_get("elink_debug"));
    }

    printf("********%s [%d] E-LINK Debug Level : %d ***********************\n", __FUNCTION__, __LINE__, g_log_level);
    
	INIT_LIST_HEAD(&gWaitAckList);
	set_device_state(DEVICE_STATE_INIT);
    
	gDeviceSeq = rand()%100;
    
	return 0;
}

int proto_elink_main_loop_stop()
{
	set_device_state(DEVICE_STATE_EXIT);
    return 0;
}


int process_device_init_state(void *input)
{
	int ret = 0;
	ret = extra_wan_connect_status();
	if(ret == ELINK_WAN_CONNECTED)
	{
		memset(clinets_new, 0x0, sizeof(elink_client)*20);
		memset(clinets_old, 0x0, sizeof(elink_client)*20);
		set_device_state(DEVICE_STATE_CONNECT);
		log_debug("e-link :wan connect success.\n");
	}
	else
	{    
		//reset_device_state(NULL);
		set_device_state(DEVICE_STATE_INIT);
		log_error("e-link : wan connect fail.\n");
		real_sleep(DELAY_T8); 
	}
	return 0;
}

int process_device_connect_state(void *input)
{
	int ret = 0;
	char gateWayIp[32] = {0};
	ret = extra_get_gateway_ip(gateWayIp);
	log_debug("e-link :wan ip = %s [ret=%d]\n", gateWayIp, ret);
	if(ret < 0)
	{
		//Get ip failed.
		log_error("extra_get_gateway_ip get ip failed.\n");
		set_device_state(DEVICE_STATE_INIT);
		real_sleep(DELAY_T1); 
		return 0;
	}

	gDeviceSock = socket_init();
	if(gDeviceSock < 0)
	{
		// socket create error.
		log_error("socket create error.\n");
		set_device_state(DEVICE_STATE_INIT);
		real_sleep(DELAY_T1); 
		return 0;
	}
	ret = socket_connect(gDeviceSock, gateWayIp, GATE_WAY_PORT);
	if(ret < 0)
	{
		log_error("e-link :Connect to gateway error.\n");
		socket_close(gDeviceSock);
		set_device_state(DEVICE_STATE_INIT);
		real_sleep(DELAY_T1); 
		return 0;
	}
	set_device_state(DEVICE_STATE_NOGO_KEY);
	return 0;
}

int process_device_nogokey_state(void *input)
{
	int ret = 0;
		/* Send nogotiation key exchange type. */
        ret = send_nogo_key_type();
        if(ret < 0)
        {
		// error.
		log_error("Send nogotiation key error.\n");
		log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
		reset_device_state(NULL);
		return 0;
        }

        ret = wait_nogo_key_type(DELAY_T9);
        if(ret < 0)
        {
		// error.
		log_error("Wait nogotiation key error.\n");
		log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
		reset_device_state(NULL);
             return 0;
        }
        log_msg( LOG_LEVEL_DEBUG, "nogotiation key type success.\n"); 
        set_device_state(DEVICE_STATE_DH);
	return 0;
        //tdSyslog(LOG_SYSTEM, "%s", "e-link nogo key success.");

}

int process_device_dh_state(void *input)
{
	int ret = 0;
	/* Send DH request. */
	ret = send_dh_request();
	if(ret < 0)
	{
		// error.
		log_error("Send nogotiation key error.\n");
		log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
		reset_device_state(NULL);
		return 0;
	}

	ret = wait_dh_reply(DELAY_T2);
	if(ret < 0)
	{
		// error.
		log_error("Wait nogotiation DH error.\n");
		log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
		reset_device_state(NULL);
	       return 0;
	}
	log_msg( LOG_LEVEL_DEBUG, "DH type success.\n"); 
	//aes_init(gDeviceAesKey);

	set_device_state(DEVICE_STATE_REG);
	return 0;
	//tdSyslog(LOG_SYSTEM, "%s", "e-link DH success.");

}

int process_device_reg_state(void *input)
{
	int ret = 0;
	/* send device information*/ 
	ret = send_dev_reg_info_request();
	if(ret < 0)
	{
		//error
		log_error("Send device information error.\n");
		log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
		reset_device_state(NULL);
		return 0;
	}

	log_msg( LOG_LEVEL_DEBUG, "Send device regist information success.\n");
	set_device_state(DEVICE_STATE_CONFIG);
	return 0;
	//tdSyslog(LOG_SYSTEM, "%s", "e-link reg dev success.");
}

int process_device_config_state(void *input)
{
	int ret = 0;
	static int config_status_times = 0;
	/* Main state. */
	log_debug("Main state deal message.\n");
	ret = deal_config_status_msg();
	
	if(ret < 0)
	{
		//error
		log_error("deal_config_state_msgn error \n");
		log_debug("func:%s line:%d set device reset\n",__func__,__LINE__);
		reset_device_state(NULL);
	}
	else
	{
		if(config_status_times == 5)
		{
			extra_enter_conf_state();
		}
	    roaming_process();
	    config_status_times++;

	    if(0 == config_status_times%6 ) // Send keep alive every 10 s.
	    {
	        proto_elink_keep_alive(NULL);
	        proto_elink_devs_status_check();
	    }
	    check_wait_ack_list();
	}
	return 0;
}

int process_device_exit_state(void *input)
{
	int ret = 0;
	if(gDeviceSock > 0)
	{
	    socket_close(gDeviceSock);
	    gDeviceSock = -1;
	}
	/* Somethings need to do here. Like free sequence list. */
	show_wait_ack_list();
	free_wait_ack_list();
	extra_enter_reset_state();
	if(DEVICE_STATE_EXIT ==get_device_state())
	{
	    log_debug("%s [%d] Elink force EXIT.\n", __FUNCTION__, __LINE__);
	    return 1;
	}
	/* Re-to Init state, clear and delay jobs . */
	set_device_state(DEVICE_STATE_INIT);
	real_sleep(DELAY_T1);    
	return 0;
}

static device_state_process_func device_state_machine[DEVICE_STATE_EXIT+1] = 
{
	process_device_init_state,
	process_device_connect_state,
	process_device_nogokey_state,
	process_device_dh_state,
	process_device_reg_state,
	process_device_config_state,
	process_device_exit_state,
	process_device_exit_state,
};

int proto_elink_main_loop()
{
	char gateWayIp[32] = {0};
    int ret = 0;
    int config_status_times = 0;

    log_msg( LOG_LEVEL_DEBUG,"e-link Start. Build %s -%s.\n", __DATE__, __TIME__);
    
    while(1)
    {
         if(device_state_machine[get_device_state()](NULL))
		 	break;
    }   

	return 0;
}

