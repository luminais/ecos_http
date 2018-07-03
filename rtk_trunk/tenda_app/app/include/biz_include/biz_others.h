#ifndef BIZ_OTHERS_H
#define BIZ_OTHERS_H
#ifdef CONFIG_APP_COSTDOWN
int biz_m_wifi_push_wifi_info(wifi_basic_t *basic_info);
typedef int (push_strange_info_func)(char *data,int data_len);
void biz_m_rub_net_push_strange_host_info(push_strange_info_func *push_func);
#else
void biz_m_wifi_push_wifi_info(void);
void biz_m_rub_net_push_strange_host_info(void);
#endif

#endif