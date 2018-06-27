#ifndef ELINK_COMMON_HEAD_
#define ELINK_COMMON_HEAD_
typedef struct get_status_info
{
    char wifi_info[32];

}get_status_info;


 struct get_staus
 {
 	char *status;
	int(*ops)(cJSON*);
 };

typedef struct elink_msg_module{
	unsigned char id;
	char msg[256];
}ELINK_MSG_MODULE;

#define MAX_MODULE_NUM 16

 struct config_route
 {
 	char *key;
	int(*ops)(cJSON*,ELINK_MSG_MODULE*);
 };
 
#define DEBUG_INFO(fmt, ...)	do{/*printf(fmt,##__VA_ARGS__);/*cyg_thread_delay(100);*/}while(0)	

#define CHECK_JSON_VALUE(tmp) if(NULL == tmp) \
								{printf( "JSON msg error.%s:%d\n", __FUNCTION__, __LINE__); goto bad_msg;}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

typedef struct set_ap_info
{
    int  ap_apidx;
	char ap_enable[16];
    char ap_ssid[64];
    char ap_key[64];
    char ap_auth[32];
    char ap_encrypt[32];
    char ap_mode[16];
    int  ap_channel;
}set_ap_info;

typedef struct set_wifi_info
{
    char wifiswitch[16];
    char ledswitch[16];
    char timerswith[16];
    char timerwday[16];
    char timertime[16];
    char timerenable[16];
}set_wifi_info;

struct roaming_ctx {
	int roaming_enable;      //enable
	int threshold_rssi;        //rssi 阀值
	int report_interval;       //时间间隔
	int start_time;               //开始检测时间
	int start_rssi;                //强于该值
};

typedef void (*roaming_timeout_handler)(struct roaming_timeout *t);
struct roaming_timeout
{
	roaming_timeout_handler cb;
	unsigned long long  exprie_time;
};
#endif /* ELINK_COMMON_HEAD_ */
