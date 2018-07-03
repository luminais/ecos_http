/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_handle_module.h
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年12月21日
  最近修改   :
  功能描述   : 

  功能描述   : 接口头文件

  修改历史   :
  1.日    期   : 2016年12月21日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/
#ifndef __CGI_HANDLE_MODULE_H__
#define __CGI_HANDLE_MODULE_H__
#include "webs.h"
#include "cgi_common.h"
#include "cJSON.h"



/**************************************app get*************************************/
void app_get_wifi_timer(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_sys_basic_info(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_sys_advance_info(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_delogin_pwd(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_safe_grade(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_wl_power(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_wl_channel_grade(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_wifi_push_info(cJSON *recv_root,cJSON *send_root, void *info);
#ifdef __CONFIG_GUEST__
void app_get_wifi_guest_info(cJSON *recv_root,cJSON *send_root, void *info);
#endif
void app_get_ucloud_basic_info(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_ucloud_info_manage_en(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_ucloud_info_try_clear_acc(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_ol_host_info(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_rub_net_black_list(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_strange_host_info(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_hand_qos_info(cJSON *recv_root,cJSON *send_root,void *info);
void app_get_hand_qos_max_uplimit(cJSON *recv_root,cJSON *send_root,void *info);
void  app_get_upgrade_path(cJSON *recv_root,cJSON *send_root,void *info);
void  app_get_upgrade_memory_state (cJSON *recv_root,cJSON *send_root,void *info);

void app_get_wifi_basic_info(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_dev_remark(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_wan_basic(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_wan_rate(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_wan_detect(cJSON *recv_root,cJSON *send_root, void *info);
void app_get_mac_filter_mode(cJSON *recv_root,cJSON *send_root, void *info);
#ifdef __CONFIG_LED__
void app_get_led_status(cJSON *recv_root,cJSON *send_root, void *info);
#endif

/**********************************************************************************/



/**************************************app set*************************************/
void app_set_wifi_timer(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_reboot(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_reset(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_login_pwd(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wifi_timer(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wl_power_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wl_power(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wl_channel_grade(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_ucloud_info_sn(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_ucloud_info_manage_en(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_ucloud_info_clear_account(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_rub_net_add_blacklist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_rub_net_delete_blacklist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_rub_net_add_to_trustlist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_rub_net_delete_from_trustlist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_hand_qos_info (cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info);

void app_set_wifi_basic_info(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wifi_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
#ifdef __CONFIG_GUEST__
void app_set_wifi_guest_info(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wifi_guest_info_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
#endif
void app_set_dev_remark(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wan_basic(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wan_basic_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_wizard_succeed(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_macfilter_mode(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
void app_set_rub_net_flush_blacklist(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
#ifdef __CONFIG_LED__
void app_set_led_status(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info);
#endif

/**********************************************************************************/


#endif __CGI_HANDLE_MODULE_H__

