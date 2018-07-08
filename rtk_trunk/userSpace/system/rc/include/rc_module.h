#ifndef __RC_MODULE_H__
#define __RC_MODULE_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __LIST_H__
#include "list.h"
#endif

#ifndef __SYS_OPTION_H__
#include "sys_option.h"
#endif

#define RC "rc"

#define RC_MODULE_1S_TIME 100

typedef enum
{
	RC_TYPE_NONE = 1,
    RC_TYPE_OP,
    RC_TYPE_DEBUG_ID,   
    RC_TYPE_WLAN_IFNAME,   
    RC_TYPE_STRING_INFO,
} RC_TYPE_ID;

/*这里表示RC线程接收消息的各个模块名*/
typedef enum
{
    RC_DBG_MODULE = 0,
	RC_LAN_MODULE,
	RC_TC_MODULE,
	RC_WAN_MODULE,
	RC_WIFI_MODULE,
#ifdef __CONFIG_GET_MANUFACTURER__
	RC_MANUFACTURER_MODULE,
#endif
	RC_FIREWALL_MODULE,
	RC_HTTP_MODULE,
	RC_UPNP_MODULE,
	RC_DNSMASQ_MODULE,
	RC_SNTP_MODULE,
	RC_DHCP_SERVER_MODULE,
	RC_WAN_SURF_CHECK_MODULE,
	RC_WAN_MODE_CHECK_MODULE,
#ifdef __CONFIG_AUTO_CONN_CLIENT__
	RC_AUTO_CONN_CLIENT_MODULE,
#endif
#ifdef __CONFIG_AUTO_CONN_SERVER__
	RC_AUTO_CONN_SERVER_MODULE,
#endif
	RC_APCLIENT_DHCPC_MODULE,
	RC_REBOOT_CHECK_MODULE,
	RC_WIFI_SWITCH_SCHED_MODULE,
	RC_SYSTOOLS_MODULE,
#ifdef __CONFIG_WPS_RTK__
	RC_WPS_MODULE,
#endif
#ifdef __CONFIG_DDNS__
	RC_DDNS_MODULE,
#endif
#ifdef __CONFIG_TENDA_APP__
	RC_UCLOUD_MODULE,
#endif
#ifdef __CONFIG_LED__
	RC_LED_MODULE,
#endif
#ifdef __CONFIG_IPTV__
	RC_IGMP_MODULE,
#endif
#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
	RC_SWITCH_LED_MODULE,
#endif
	RC_LOGIN_KEEP_MODULE,
    RC_MAX_MODULE_NUM,
} RC_MODULE_ID;

/*这里表示各个模块关联的类型*/
typedef enum
{
    INTENT_NONE = 0,
    INTENT_PREV = 1,
    INTENT_NEXT = 2,
} INTENT_TYPE;

typedef enum
{
    MODULE_CALLBACK_GET = 0,/*读取模块是否执行完毕*/
    MODULE_CALLBACK_REINIT,/*将模块执行状态标志位置回*/
} MODULE_CALLBACK_TYPE;

#include <cyg/hal/hal_tables.h>
typedef struct rc_module_node {
    PIU8 id; /* 模块id，唯一，0-255 */
    PI8 name[PI_BUFLEN_32]; /* 模块名称  */
    PIU8 ops_num; /* 依赖该模块消息的模块个数 */
    RET_INFO (*init)(); /* 模块初始化函数，系统启动时调用*/
	MODULE_WORK_STATUS (*callbak)(MODULE_CALLBACK_TYPE type); /*用于回调函数用*/
    struct list_node ops_head; /* 依赖该模块的操作 */ 
} CYG_HAL_TABLE_TYPE rc_module_node_t;

/* 消息处理时，向模块传递的参数，若有新参数，需要在此增加 */
typedef struct rc_modules_common_struct
{
    PIU8 op; /* 模块的操作动作，启动、关闭、重启*/
    PIU8 debug_id; /* 用于开启DEBUG,该字段只可在RC_DBG_MODULE中使用，由RC_DBG_MODULE该模块统一开启各个模块的DEBUG信息*/
    PI8   wlan_ifname[PI_BUFLEN_16];
    PI8   string_info[PI_BUFLEN_128];//用于用户可自定义扩展的消息   
} RC_MODULES_COMMON_STRUCT;

/*模块的处理函数链节点*/
struct rc_msg_ops
{
    struct list_node list;
    RC_MODULE_ID intent_module_id; /* 依赖模块id */
    INTENT_TYPE type;/*关联的类型*/
    RET_INFO (*ops)(RC_MODULES_COMMON_STRUCT *);/* 消息处理函数 */
};

#define RC_MODULE_REGISTER(id, name, init, callback)  \
rc_module_node_t rc_module_node_##id CYG_HAL_TABLE_ENTRY(rc_module_tab) = { \
	id, \
	name, \
	0, \
	init, \
	callback \
};/*向rc_module_tab数据段中插入节点*/

extern rc_module_node_t __rc_module_tab__[],__rc_module_tab_end__;

#define RC_MODULE_LIST_SCAN(rc_module) for(rc_module = __rc_module_tab__; rc_module != &__rc_module_tab_end__; rc_module++)

#define RC_MODULE_ID_UNINRAND(id) (id < RC_DBG_MODULE || id >= RC_MAX_MODULE_NUM)

void rc_callbak(PIU8 module_id,PIU32 module_delay,PIU32 module_extra_delay);
extern RC_TYPE_ID rc_msg_get_option_type(PI8 *op_name);
extern RET_INFO rc_msg_2_tlv(PI8 *msg,PI8 *tlv_msg);
extern void rc_module_msg_handle(PIU8 module_id,RC_MODULES_COMMON_STRUCT *var);
extern RET_INFO rc_msg_2_var(MODULE_MSG_OPTION *option,RC_MODULES_COMMON_STRUCT *var);
extern rc_module_node_t * rc_find_module_index(RC_MODULE_ID id);
extern RET_INFO rc_rcv_msg_handle(PIU8 module_id,PI8 *msg);
extern RET_INFO rc_register_module_msg_opses(struct rc_msg_ops *reg,RC_MODULE_ID id,PIU8 n);
extern RET_INFO rc_module_init();
#endif/*__RC_MODULE_H__*/
