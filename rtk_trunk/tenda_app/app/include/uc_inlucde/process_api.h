#ifndef __PROCESS_API_H__
#define __PROCESS_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>

#include "uc_m_sys_types.h"
#include "uc_m_login_types.h"
#include "uc_m_wifi_types.h"
#include "uc_m_safe_check_types.h"
#include "uc_m_wan_types.h"
#include "uc_m_rub_net_types.h"
#include "uc_m_ol_host_types.h"
#include "uc_m_hand_qos_types.h"
#include "uc_m_cloud_info_types.h"
#include "uc_m_energy_types.h"
#include "uc_m_dev_types.h"
#include "uc_m_parent_control_types.h"
#include  "uc_ol_upgrade_types.h"

/*统一错误结构体*/
typedef struct _failure_msg {
	int code;
	unsigned char message[0];
} failure_msg_t;

/*此为注册和取消注册宏,后续版本会让其不可见*/
enum {
	eSTAT_NORMAL,
	eSTAT_UNREGISTER
};

#define SN_MAX				18
#define MAC_MAX				18
#define VER_MAX				32
#define POLICY_VER_MAX             64
#define COMPANY_LEN			128
#define PRODUCT_LEN			129
#define PASSWD_MAX_LEN      128
#define COMPLETE_RET               10

#define MAX_UPGRADE_DIR_LEN	256

/*此结构体为ucloud同内部实现进程的交互命令结构体,
后续版本会让其不可见*/
typedef struct api_cmd_s {
	unsigned char		src;			/*命令的来源*/
	unsigned char		module;			/*模块*/
	unsigned char		cmd;			/*命令id*/
	unsigned char		seq;			/*命令的序列号，部分情况为0*/
	unsigned char		status;			/*成功or失败or其他*/
	unsigned char		ack;			/*标记该命令是实现进程返回给终端的*/
	unsigned char		pad[5];			/*保留*/
	unsigned int		session_id;		/*保存session的id*/
	unsigned int		rcv_time;		/*收到命令的时间*/
	unsigned int		len;			/*data 的长度*/
	char				data[0];		/*具体的结构体*/
} api_cmd_t;

/*收到检测内存命令时需回复内存是否足够,
如内存足够则该enough_memory置1,否则置0*/
struct mem_state_t {
	int  enough_memory;				
};

/*放置镜像的路径,path指向动态分配的路径的字符串, len表示字符串长度*/
struct download_path_t {
	char *path;
	int   len;
};

/*上行测速成功后通知内部实现进程,单位为kb/s,
如该值为0说明测速失败*/
struct speed{
	unsigned int value;
};

//注册命令时通用的回调函数
typedef int (command_cb_fn_t)(api_cmd_t *cmd, unsigned int data_len, void *data, void *privdata);
typedef int (mem_check_cb_t)(struct mem_state_t *memory_state, int img_size);


 //注册发送完成，处理后续工作的回调函数
 typedef void (complete_cb_t) (api_cmd_t *cmd, int len, int ret);

 //delay this function work after config data 
typedef void (timer_delay_cb) (void);

//设置路由器基础数据
typedef struct cloud_basic_info {	
		int  enable;						//云管理是否开启
		char sn[SN_MAX];					//设置sn,可为空
		char mac[MAC_MAX];					//设置mac,必填
		char ver[VER_MAX]; 					//设置软件版本,必填
		char policy_ver[POLICY_VER_MAX];                 //设置策略软件版本，支持策略升级时，必填
		char company[COMPANY_LEN];			//设置公司名称,必填
		char product[PRODUCT_LEN];			//设置设备型号,必填
		char ver_passwd[PASSWD_MAX_LEN];	//设置版本密码,必填
		command_cb_fn_t	*sn_set_cb;			//注册设置sn命令的回调函数
		void			*sn_set_priv;		//注册设置sn命令的私有数据
} cloud_basic_info_t;

//开启升级功能时需要注册的数据
typedef struct upgrade_info{
		char upgrade_dir[MAX_UPGRADE_DIR_LEN];		//设置升级镜像地址,必填
		mem_check_cb_t		*memory_check_cb;	//注册内存检测命令的回调函数,必填
		void				      *memory_check_priv;	//私有数据
		command_cb_fn_t		*begin_upgrade_cb;	//注册开始升级命令的回调函数,必填
		void				  *begin_upgrade_priv;	        //私有数据
		command_cb_fn_t      *begin_policy_upgrade_cb;       //注册策略升级命令的回调函数，支持在线策略升级时，必填
		void				   *begin_policy_upgrade_priv;
}upgrade_info_t;

//开启测速功能时需要注册的数据
typedef struct speed_test_info{
		command_cb_fn_t		*up_speed_set_cb;		//注册设置上行速度命令的回调函数,必填
		void				*up_speed_set_priv;		//私有数据
}speed_test_info_t;


/******************************************************************
 * Function: 		uc_api_lib_init
 * Description:	初始化函数,创建新线程连接ucloud,
 *				初始化相关数据结构,此函数在进程
 *				起始处调用一次即可
 *		
 * Input:
 * Output:
 * Return: 		0:初始化成功
 *  				-1:初始化失败 
 ******************************************************************/
int uc_api_lib_init(void);
/******************************************************************
 * Function: 		uc_api_lib_destroy
 * Description:	清空数据,摧毁新建线程,此函数在进程
 *				结束或不需要ucloud功能时调用
 *		
 * Input:
 * Output:
 * Return: 		0:清数据成功
 * 		   		-1:清数据失败
 ******************************************************************/
int uc_api_lib_connect(void);
int uc_api_lib_commit(int fd);
int uc_api_lib_destroy(void);
/******************************************************************
 * Function: 		uc_api_set_basic_info
 * Description:	设置基础数据函数
 *		
 * Input:			 云管理开关,mac,软件版本等
 * Output:		
 * Return: 		0:设置基础数据成功
 * 		   		-1:设置基础数据失败
 ******************************************************************/
int uc_api_set_basic_info(cloud_basic_info_t *data);
/******************************************************************
 * Function: 		uc_api_enable_upgrade
 * Description:	开启在线升级功能
 *		
 * Input:			镜像放置位置,
 				内存检测命令的执行函数指针,
 				镜像下载完毕开始升级的执行函数指针
 * Output:		
 * Return: 		0:开启在线升级功能成功
 * 		   		-1:开启在线升级功能失败
 ******************************************************************/
int uc_api_enable_upgrade(upgrade_info_t *data);
/******************************************************************
 * Function: 		uc_api_enable_speed_test
 * Description:	开启带宽测速功能
 *		
 * Input:			上行速度测试成功后的设置函数
 * Output:		
 * Return: 		0:开启带宽测速功能成功
 * 		   		-1:开启带宽测速功能失败
 ******************************************************************/
int uc_api_enable_speed_test(speed_test_info_t *data);


#if 1/*M_SYS模块定义的数据结构和命令*/
typedef int (basic_info_fn_t) (const api_cmd_t *cmd, 
			sys_basic_info_t *basic, void *privdata);
typedef int (advance_info_fn_t) (const api_cmd_t *cmd, 
					sys_advance_info_t *advance, void *privdata);
typedef int (sys_reboot_fn_t) (const api_cmd_t *cmd, void *privdata);
typedef int (sys_reset_fn_t) (const api_cmd_t *cmd, void *privdata);
typedef int (set_time_zone_fn_t) (const api_cmd_t *cmd, 
					sys_time_zone_t *time, void *privdata);

typedef struct m_sys_regist_s{
	basic_info_fn_t		*m_sys_basic_info_get_cb;
	void				*basic_info_privdata;
	advance_info_fn_t	*m_sys_advance_info_get_cb;
	void				*advance_info_privdata;
	sys_reset_fn_t		*m_sys_reset_cb;
	void				*reset_privdata;
	sys_reboot_fn_t		*m_sys_reboot_cb;
	void				*reboot_privdata;
	set_time_zone_fn_t		*m_sys_set_time_zone_cb;
	void				*time_zone_privdata;
}m_sys_regist_t;
#endif


#if 1/*M_LOGIN模块定义的数据结构和命令*/
typedef int (login_fn_t) (const api_cmd_t *cmd,
					m_login_t *login_info, void *privdata);
typedef int (up_pwd_fn_t) (const api_cmd_t *cmd, 
					m_login_update_pwd_t *update_info, void *privdata);
#ifdef CONFIG_APP_COSTDOWN
typedef int (get_pwd_sta_fn_t) (const api_cmd_t *cmd, 
					pwd_sta_t *sta, void *privdata);
#else
typedef int (get_pwd_sta_fn_t) (const api_cmd_t *cmd, 
					login_common_ack_t **ack_info, void *privdata);
#endif
typedef struct m_login_regist_s{
	login_fn_t			*m_login_login_info_cb;
	void				*login_info_privdata;
	up_pwd_fn_t			*m_login_update_info_cb;
	void				*update_info_privdata;
	get_pwd_sta_fn_t    *m_login_get_pwd_sta_cb;
	void                          *get_pwd_sta_privdata;
}m_login_regist_t;
#endif


#if 1/*M_WIFI模块定义的数据结构和命令*/
typedef int (wifi_basic_info_get_fn_t) (const api_cmd_t *cmd, 
					wifi_basic_t *basic_info, void *privdata);
typedef int (wifi_basic_info_set_fn_t) (const api_cmd_t *cmd, 
					wifi_basic_t *basic_info, void *privdata);
typedef int (wifi_guest_info_get_fn_t) (const api_cmd_t *cmd, 
					guest_info_t *guest_info, void *privdata);
typedef int (wifi_guest_info_set_fn_t) (const api_cmd_t *cmd, 
					guest_info_t *guest_info, void *privdata);
typedef int (wifi_channel_get_fn_t) (const api_cmd_t *cmd, 
					wifi_channel_info_t *chan, void *privdata);
typedef int (wifi_channel_set_fn_t) (const api_cmd_t *cmd, void *privdata);
typedef int (wifi_power_get_fn_t) (const api_cmd_t *cmd, 
					wifi_power_t *power, void *privdata);
typedef int (wifi_power_set_fn_t) (const api_cmd_t *cmd, 
					wifi_power_t *power, void *privdata);

typedef struct m_wifi_regist_s{
	wifi_basic_info_get_fn_t	*m_wifi_basic_info_get_cb;
	void				*wifi_basic_info_get_privdata;
	wifi_basic_info_set_fn_t	*m_wifi_basic_info_set_cb;
	void				*wifi_basic_info_set_privdata;
	wifi_guest_info_get_fn_t	*m_wifi_guest_info_get_cb;
	void				*wifi_guest_info_get_privdata;
	wifi_guest_info_set_fn_t	*m_wifi_guest_info_set_cb;
	void				*wifi_guest_info_set_privdata;
	wifi_channel_get_fn_t		*m_wifi_channel_get_cb;
	void				*wifi_channel_get_privdata;
	wifi_channel_set_fn_t		*m_wifi_channel_set_cb;
	void				*wifi_channel_set_privdata;
	wifi_power_get_fn_t			*m_wifi_power_get_cb;
	void				*wifi_power_get_privdata;
	wifi_power_set_fn_t			*m_wifi_power_set_cb;
	void				*wifi_power_set_privdata;
}m_wifi_regist_t;
#endif

#if 1
typedef int (guard_info_get_fn_t)(const api_cmd_t *cmd, guard_info_t *info, void *private);
typedef int (guard_info_set_fn_t)(const api_cmd_t *cmd,const guard_info_t *info, void *private);

typedef int (safe_check_fn_t) (const api_cmd_t *cmd, 
                                       safe_check_info_t *info, void *privdata);
typedef int (dns_optimize_fn_t) (const api_cmd_t *cmd, void *privdata);


typedef struct m_safe_check_regist_s{
       safe_check_fn_t         *m_safe_check_get;
       void            *m_safe_check_get_privdata;
       dns_optimize_fn_t               *m_dns_opt_set;
       void            *m_dns_opt_set_privdata;
	   guard_info_get_fn_t	*m_guard_info_get;
	   void *m_guard_info_get_private;
	   guard_info_set_fn_t	*m_guard_info_set;
	   void *m_guard_info_set_private;
}m_safe_check_regist_t;
#endif

#if 1/*M_WAN模块定义的数据结构和命令*/
typedef int (get_rate_cb_fn_t) (const api_cmd_t *cmd, 
					wan_rate_info_t *rate_info, void *privdata);
typedef int (basic_set_cb_fn_t) (const api_cmd_t *cmd, 
					wan_basic_info_t *basic_info, void *privdata);
typedef int (basic_get_cb_fn_t) (const api_cmd_t *cmd, 
					wan_basic_info_t *basic_info, void *privdata);
typedef int (detect_type_cb_fn_t) (const api_cmd_t *cmd, 
					wan_detecttype_info_t *type_info, void *privdata);

typedef struct m_wan_regist_s{
	get_rate_cb_fn_t	*m_wan_rate_get_cb;
	void				*m_wan_rate_get_privdata;
	basic_set_cb_fn_t	*m_wan_basic_set_cb;
	void				*m_wan_basic_set_privdata;
	basic_get_cb_fn_t	*m_wan_basic_get_cb;
	void				*m_wan_basic_get_privdata;
	detect_type_cb_fn_t	*m_wan_detect_type_cb;
	void				*m_wan_detect_type_privdata;
}m_wan_regist_t;


#endif

#if 1
typedef int (access_user_fn_t) (const api_cmd_t *cmd, \
					access_user_t *user_info, void *privdata);
typedef int (rub_net_get_fn_t) (const api_cmd_t *cmd, \
					rub_net_t *rub_net_info, void *privdata);
typedef int (rub_net_set_fn_t) (const api_cmd_t *cmd, \
					rub_net_t *rub_net_info, void *privdata);
typedef int (history_list_get_fn_t) (const api_cmd_t *cmd,\
					rub_network_history_t *list_info, void *privdata);
typedef int (history_list_clear_fn_t) (const api_cmd_t *cmd, void *privdata);
#ifdef CONFIG_APP_COSTDOWN
typedef int (black_list_get_fn_t) (const api_cmd_t *cmd, \
					black_list_t **black_list, int len, void *privdata);
#else
typedef int (black_list_get_fn_t) (const api_cmd_t *cmd, \
					black_list_t **black_list,void *privdata);
#endif
typedef int (black_list_flush_fn_t)(void*);
typedef int (mf_mode_get_fn_t)(mac_filter_mode_t *, void *);
typedef int (mf_mode_set_fn_t)(mac_filter_mode_t *, void *);
typedef int (white_list_add_fn_t)(access_list_set_t *, void *);
typedef int (white_list_del_fn_t)(access_list_set_t *, void *);
typedef int (white_list_get_fn_t)(white_list_t *, void *);
typedef int (white_list_flush_fn_t)(void*);
typedef int (black_list_add_fn_t)(access_list_set_t *, void *);
typedef int (black_list_del_fn_t)(access_list_set_t *, void *);

typedef struct m_rub_net_regist_s {
	access_user_fn_t  	*m_access_user_set;
	void		*m_access_user_set_privdata;
	rub_net_get_fn_t  		*m_rub_net_get;
	void		*m_rub_net_get_privdata;
	rub_net_set_fn_t  	*m_rub_net_set;
	void		*m_rub_net_set_privdata;
	history_list_get_fn_t  		*m_history_list_get;
	void		*m_history_list_get_privdata;
	history_list_clear_fn_t  	*m_history_list_clear;
	void		*m_history_list_clear_privdata;
	black_list_get_fn_t *m_black_list_get;
	void		*m_black_list_get_privdata;
	black_list_flush_fn_t *m_black_list_flush;
	void 		*m_black_list_flush_privdata;
	mf_mode_get_fn_t *m_macfilter_mode_get;
	void 		*m_macfilter_mode_get_privdata;
	mf_mode_set_fn_t *m_macfilter_mode_set;
	void		*m_macfilter_mode_set_privdata;
	white_list_add_fn_t	*m_white_list_add;
	void		*m_white_list_add_privdata;
	white_list_del_fn_t	*m_white_list_del;
	void 		*m_white_list_del_privdata;
	white_list_get_fn_t	*m_white_list_get;
	void		*m_white_list_get_privdata;
	white_list_flush_fn_t	*m_white_list_flush;
	void		*m_white_list_flush_privdata;
	black_list_add_fn_t	*m_black_list_add;
	void		*m_black_list_add_privdata;
	black_list_del_fn_t	*m_black_list_del;
	void		*m_black_list_del_privdata;
}m_rub_net_regist_t;
#endif

#if 1/*M_OL_HOST?￡?é?¨ò?μ?êy?Y?á11oí?üá?*/
typedef int (ol_hosts_get_cb_fn_t)(const api_cmd_t *cmd,
				 ol_host_common_ack_t **ol_hosts_get_info, void *privdata);


typedef struct m_ol_host_regist_s{
	ol_hosts_get_cb_fn_t      *m_ol_host_get_cb;
	void				      *m_ol_host_get_privdata;

}m_ol_host_regist_t;

#endif

#if 1/*M_HAND_QOS?￡?é?¨ò?μ?êy?Y?á11oí?üá?*/
typedef int (hand_qos_get_fn_t) (const api_cmd_t *cmd,hand_qos_get_param_t *param,  \
					 hand_qos_common_ack_t **info, void *privdata);

typedef int (hand_qos_set_fn_t) (const api_cmd_t *cmd, \
					const hand_qos_set_info_t *set_info, void *privdata);

typedef int (hand_qos_get_genable_fn_t) (const api_cmd_t *cmd, \
					hand_qos_common_ack_t *info, void *privdata);

typedef int (hand_qos_set_genable_fn_t) (const api_cmd_t *cmd, \
					hand_qos_global_en_t *info, void *privdata);
					
typedef int (hand_qos_max_uplimit_fn_t) (const api_cmd_t *cmd, \
					hand_qos_max_uplimit_t *info, void *privdata);

typedef struct m_hand_qos_regist_s{
	hand_qos_get_fn_t              *m_hand_qos_get_cb;
	void				          *m_hand_qos_get_privdata;
	hand_qos_set_fn_t             *m_hand_qos_set_cb;
	void				          *m_hand_qos_set_privdata;
	hand_qos_get_genable_fn_t   *m_hand_qos_get_genable_cb;
	void                                   *m_hand_qos_get_genable_privdata; 
	hand_qos_set_genable_fn_t   *m_hand_qos_set_genable_cb;
	void                                   *m_hand_qos_set_genable_privdata; 
	hand_qos_max_uplimit_fn_t   *m_hand_qos_max_uplimit_cb;
	void						*m_hand_qos_max_uplimit_privdata; 

}m_hand_qos_regist_t;

#endif
#if 1
typedef int (cloud_info_manage_en_get_fn_t) (const api_cmd_t *cmd,  \
					m_cloud_info_manage_en_t *manage_en, void *privdata);
typedef int (cloud_info_manage_en_set_fn_t) (const api_cmd_t *cmd,  \
					m_cloud_info_manage_en_t *manage_en, void *privdata);
typedef int (cloud_info_clear_account_ack_fn_t) (const api_cmd_t *cmd,  \
					void *privdata);
typedef struct m_cloud_info_regist_s{
	cloud_info_manage_en_get_fn_t              *cloud_info_manage_en_get_cb;
	void				          *cloud_info_manage_en_get_privdata;
	cloud_info_manage_en_set_fn_t             *cloud_info_manage_en_set_cb;
	void				          *cloud_info_manage_en_set_privdata;
	cloud_info_clear_account_ack_fn_t             *cloud_info_clear_account_ack_cb;
	void				          *cloud_info_clear_account_ack_privdata;

}m_cloud_info_regist_t;
#endif

#if 1 /*M_ENERGY*/
typedef int (led_get_fn_t) (const api_cmd_t *cmd, \
					m_energy_led_t *led_info, void *privdata);
typedef int (led_set_fn_t) (const api_cmd_t *cmd,  \
					m_energy_led_t *led_info, void *privdata);
typedef int (wl_timer_get_fn_t) (const api_cmd_t *cmd,\
					m_energy_wireless_timer_t *timer_info, void *privdata);
typedef int (wl_timer_set_fn_t) (const api_cmd_t *cmd,  \
					m_energy_wireless_timer_t *timer_info, void *privdata);
typedef int (energy_mode_get_fn_t) (const api_cmd_t *cmd, \
					m_energy_mode_t *led_info, void *privdata);
typedef int (energy_mode_set_fn_t) (const api_cmd_t *cmd,  \
					m_energy_mode_t *led_info, void *privdata);
typedef struct m_energy_regist_s{
	led_get_fn_t                 *m_led_get_cb;
	void				    *m_led_get_privdata;
	led_set_fn_t                 *m_led_set_cb;
	void				    *m_led_set_privdata;
	wl_timer_get_fn_t   	    *m_wl_timer_get_cb;
	void                              *m_wl_timer_get_privdata; 
	wl_timer_set_fn_t   	    *m_wl_timer_set_cb;
	void                              *m_wl_timer_set_privdata; 
	energy_mode_get_fn_t   	    *m_energy_mode_get_cb;
	void                              *m_energy_mode_get_privdata; 
	energy_mode_set_fn_t   	    *m_energy_mode_set_cb;
	void                              *m_energy_mode_set_privdata; 

}m_energy_regist_t;
#endif

#if 1/*M_DEV*/
typedef int (dev_nickname_get_fn_t) (const api_cmd_t *cmd,dev_name_t *param,  \
					 nickname_info_t *info, void *privdata);

typedef int (dev_nickname_set_fn_t) (const api_cmd_t *cmd, \
					nickname_info_t *set_info, void *privdata);

typedef struct m_dev_nickname_regist_s{
	dev_nickname_get_fn_t              *m_dev_nickname_get_cb;
	void				          *m_dev_nickname_get_privdata;
	dev_nickname_set_fn_t             *m_dev_nickname_set_cb;
	void				          *m_dev_nickname_set_privdata;
}m_dev_nickname_regist_t;

#endif
#if 1/*M_PARENT_CONTROL*/
typedef int (parent_control_get_fn_t) (const api_cmd_t *cmd, \
		pc_get_param_t* param, parent_control_common_ack_t **info, void *privdata);
typedef int (parent_control_set_fn_t) (const api_cmd_t *cmd, \
					const pc_set_param_t *param, void *privdata);
typedef int (parent_control_get_type_fn_t) (const api_cmd_t *cmd, \
					parent_control_common_ack_t *param, void *privdata);

typedef struct m_parent_control_regist_s{
	parent_control_get_fn_t             *m_parent_control_get_cb;
	void				          *m_parent_control_get_privdata;
	parent_control_set_fn_t             *m_parent_control_set_cb;
	void				          *m_parent_control_set_privdata;
	parent_control_get_type_fn_t  		*m_parent_control_get_type_cb;
	void				          *m_parent_control_get_type_privdata;
}m_parent_control_regist_t;

#endif
int uc_api_enable_m_sys(m_sys_regist_t *data);
int uc_api_enable_m_login(m_login_regist_t *data);
int uc_api_enable_m_wifi(m_wifi_regist_t *data);
int uc_api_enable_m_safe_check(m_safe_check_regist_t *data);
int uc_api_enable_m_wan(m_wan_regist_t * data);
int uc_api_enable_m_rub_net(m_rub_net_regist_t *data);
int uc_api_enable_m_ol_host(m_ol_host_regist_t * data);
int uc_api_enable_m_hand_qos(m_hand_qos_regist_t * data);
int uc_api_enable_m_energy(m_energy_regist_t *data);
int uc_api_enable_m_dev_nickname(m_dev_nickname_regist_t *data);

/******************************************************************
 * Function: 		uc_api_m_rub_net_push_strange_host_info
 * Description:	发送陌生主机上线的信息
 *		
 * Input:			rub_strange_host_info_t
 * Output:		
 * Return: 		0:发送数据成功
 * 		   		-1:发送数据失败
 ******************************************************************/
int uc_api_m_rub_net_push_strange_host_info(rub_strange_host_info_t *info);
int uc_api_enable_m_cloud_info(m_cloud_info_regist_t * data); 
/******************************************************************
 * Function: 		uc_api_m_cloud_info_send_clear_account
 * Description:	发送清空路由器绑定云账号信息，该接口发送消息后，若
 *			命令执行成功，ucloud会回调注册实现函数cloud_info_clear_account_ack_fn_t
 *		
 * Input:			无
 * Output:		
 * Return: 		0:发送数据成功
 * 		   		-1:发送数据失败
 ******************************************************************/
int uc_api_m_cloud_info_send_clear_account(); 

/******************************************************************
 * Function: 		uc_api_m_wifi_push_wifi_info
 * Description:	上报wifi基础信息修改的情况到服务器
 *		
 * Input:			wifi_basic_t
 * Output:		
 * Return: 		0:发送数据成功
 * 		   		-1:发送数据失败
 ******************************************************************/
int uc_api_m_wifi_push_wifi_info(wifi_basic_t *info);
/* Macro api */
int uc_api_enable_m_parent_control(m_parent_control_regist_t * data); 

#ifdef ECOS
/******************************************************************
 * Function: 		uc_api_m_cloud_info_restart_connect_svr
 * Description:	重启连接云服务器
 *		
 * Input:			无
 * Output:		
 * Return: 		0:发送数据成功
 * 		   		-1:发送数据失败
 ******************************************************************/

int uc_api_m_cloud_info_restart_connect_svr(void);
#endif
#endif
