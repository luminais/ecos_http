#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nvram.h"


struct nvram_tuple tenda_nvram_defaults[] = {
	/*************2.4G基本参数*****************/
	{ "wl0_ssid", "Tenda", 0},
	{ "wl0_hwaddr", "0", 0},
	{ "wl0_encode", "UTF-8", 0},
	{ "wl0_radio", "1", 0},
	{ "wl0_closed", "0", 0},
	{ "wl0_akm", "", 0},
	{ "wl0_wep", "disabled", 0},
	{ "wl0_wpa_psk", "", 0},
	{ "wl0_crypto", "", 0},
	{ "wl0_nettype", "bgn", 0},					/*网络模式*/
	{ "wl0_channel", "0", 0},
	{ "wl0_bandside", "upper", 0},				/*扩展频宽*/
	{ "wl0_bandwidth", "auto", 0},					/*频宽*/
	{ "wl0_ctv_power", "high", 0},
	{ "wl0_country_pwr_power", "78,92,100", 0},
	{ "wl0_offset_power", "0", 0},//功率调节，当前实际应该减去的dbm的两倍
	{ "wl0_mode", "ap", 0},
	{ "wl0_ifname", "wlan1", 0},

	/*************2.4G次接口基本参数*****************/
	{ "wl0.1_radio", "1", 0},
	{ "wl0.1_hwaddr", "0", 0},
	{ "wl0.1_ssid", "Tenda_repeater", 0},
	{ "wl0.1_encode", "UTF-8", 0},
	{ "wl0.1_closed", "0", 0},
	{ "wl0.1_akm", "", 0},
	{ "wl0.1_wep", "disabled", 0},
	{ "wl0.1_wpa_psk", "", 0},
	{ "wl0.1_crypto", "", 0},
	{ "wl0.1_mode", "client", 0},
	{ "wl0.1_ifname", "wlan1-vxd0", 0},

	/*************2.4G 访客网络基本参数*****************/
	{ "wl0.2_ssid", "Tenda_VIP", 0},
	{ "wl0.2_hwaddr", "0", 0},
	{ "wl0.2_nettype", "bgn", 0},					/*网络模式*/
	{ "wl0.2_hwaddr", "0", 0},
	{ "wl0.2_radio", "0", 0},
	{ "wl0.2_closed", "0", 0},
	{ "wl0.2_akm", "", 0},
	{ "wl0.2_wep", "disabled", 0},
	{ "wl0.2_wpa_psk", "", 0},
	{ "wl0.2_crypto", "", 0},
	{ "wl0.2_ifname", "wlan1-va0", 0},
	/*************5G基本参数*****************/
	{ "wl1_ssid", "Tenda", 0},
	{ "wl1_hwaddr", "0", 0},
	{ "wl1_encode", "UTF-8", 0},
	{ "wl1_radio", "1", 0},
	{ "wl1_closed", "0", 0},
	{ "wl1_akm", "", 0},
	{ "wl1_wep", "disabled", 0},
	{ "wl1_wpa_psk", "", 0},
	{ "wl1_crypto", "", 0},
	{ "wl1_nettype", "ac", 0},					/*网络模式*/
	{ "wl1_channel", "0", 0},
	{ "wl1_bandside", "upper", 0},				/*扩展频宽*/
	{ "wl1_bandwidth", "auto", 0},					/*频宽*/
	{ "wl1_ctv_power", "high", 0},
	{ "wl1_country_pwr_power", "92,100", 0},
	{ "wl1_offset_power", "0", 0},//功率调节，当前实际应该减去的dbm的两倍
	{ "wl1_mode", "ap", 0},
	{ "wl1_ifname", "wlan0", 0},
	/*************5G次接口基本参数*****************/
	{ "wl1.1_radio", "1", 0},
	{ "wl1.1_hwaddr", "1", 0},
	{ "wl1.1_ssid", "Tenda_repeater", 0},
	{ "wl1.1_encode", "UTF-8", 0},
	{ "wl1.1_closed", "0", 0},
	{ "wl1.1_akm", "", 0},
	{ "wl1.1_wep", "disabled", 0},
	{ "wl1.1_wpa_psk", "", 0},
	{ "wl1.1_crypto", "", 0},
	{ "wl1.1_mode", "client", 0},
	{ "wl1.1_ifname", "wlan0-vxd0", 0},
	/*************5G 访客网络基本参数*****************/
	{ "wl1.2_ssid", "Tenda_VIP_5G", 0},
	{ "wl1.2_hwaddr", "0", 0},
	{ "wl1.2_nettype", "ac", 0},					/*网络模式*/
	{ "wl1.2_hwaddr", "0", 0},
	{ "wl1.2_radio", "0", 0},
	{ "wl1.2_closed", "0", 0},
	{ "wl1.2_akm", "", 0},
	{ "wl1.2_wep", "disabled", 0},
	{ "wl1.2_wpa_psk", "", 0},
	{ "wl1.2_crypto", "", 0},
	{ "wl1.2_ifname", "wlan0-va0", 0},
	/******************wlan 公共参数******************/
	{ "wl_guest_effective_time","28800", 0},
	{ "wl_guest_down_speed_limit","0", 0},	
	{ "wl_maclist", "", 0},
	{ "wl_macmode", "deny", 0},
	{ "wl_guest_down_speed_limit","0",0},
	/**************wps相关的参数*****************/
	{ "wps_aplockdown_cap", "1", 0},
	{ "wps_config_method", "0x284", 0},
	{ "wps_device_name", "Tenda", 0},
	{ "wps_device_pin", "64274959", 0},
	{ "wps_mfstring", "Tenda", 0},
	{ "wps_mode", "disabled", 0},
	{ "wps_modelname", "Tenda", 0},
	{ "wps_modelnum", "123456", 0},
	{ "wps_proc_status", "0", 0},
	{ "wps_random_ssid_prefix", "Tenda", 0},
	{ "wps_restart", "0", 0},
	{ "wps_sta_pin", "00000000", 0},
	{ "wps_timeout_enable", "0", 0},
	{ "wps_version2", "enabled", 0},
	{ "wps_wer_mode", "allow", 0},
	{ "xtalfreq", "20000", 0},
	{ "wps_configure_state", "1", 0},

	/********************wifi定时相关参数************/
	{ "wl_wifictl_enable", "0", 0}, 		 
	{ "wl_wifictl_day_round", "1111111", 0},
	{ "wl_wifictl_time_interval", "00:00-07:00", 0},
	{ "wl_time_week", "01111100", 0},
	{ "wl_wifictl_schedule_offtime_list1", "UTC+08:00", 0},
	{ "wl_wifictl_schedule_ontime_list1", "UTC+08:00", 0},
	{ "wl_wifictl_schedule_listnum", "0", 0},
	
	/******************app相关的参数*****************/
	{ "uc_manage_en","1",0},  /* 云管理开关 */ 
	{ "ucloud_need_clear_acc","0",0}, /* 云账号是否需要清空 */ 

	/*****************其他配置参数*******************/
	{"sys_workmode","route",0},
	{ "country_code", "CN", 0},
	{ "auto_conn_extend_rssi", "20", 0}, /*自动桥接\自动快速设置信号强度阈值*/
	
	{ "td_dig_sel", "44", 0},/* 抗干扰接收灵敏度调整范围的上限，44=0x2c*/	
	
	{ "td_auto_antiInterferance", "1", 0},/*是否开启干扰自适应*/	
	
	{ "adaptivity_enable", "1", 0},
	{ "antswctl2g", "0x1", 0},
	{ "auto_clone_flag", "yes", 0},	
	{ "autofw_port0", "", 0},
	{ "boardflags2", "0x1000", 0},
	{ "boardnum", "396", 0},
	{ "boardpwrctl", "0xb00", 0},
	{ "boardtype", "0x058e", 0},
	{ "boot_wait", "off", 0},
	{ "br0_ifnames", "eth0", 0},
	{ "btn2_status", "WIFI_SWITCH", 0},
	{ "c8021x_en", "", 0},
	{ "c8021x_pswd", "", 0},
	{ "c8021x_user", "", 0},
	{ "clkfreq", "300,150,75", 0},
	{ "clnway", "1", 0},
	{ "config_index", "0", 0},
	{ "console_loglevel", "1", 0},
	{ "ddns_enable", "0", 0},
	{ "ddns_set1", "", 0},
	{ "device_remark_list1", "", 0},
	{ "device_remark_list10", "", 0},
	{ "device_remark_list11", "", 0},
	{ "device_remark_list12", "", 0},
	{ "device_remark_list13", "", 0},
	{ "device_remark_list14", "", 0},
	{ "device_remark_list15", "", 0},
	{ "device_remark_list16", "", 0},
	{ "device_remark_list17", "", 0},
	{ "device_remark_list18", "", 0},
	{ "device_remark_list19", "", 0},
	{ "device_remark_list2", "", 0},
	{ "device_remark_list20", "", 0},
	{ "device_remark_list3", "", 0},
	{ "device_remark_list4", "", 0},
	{ "device_remark_list5", "", 0},
	{ "device_remark_list6", "", 0},
	{ "device_remark_list7", "", 0},
	{ "device_remark_list8", "", 0},
	{ "device_remark_list9", "", 0},
	{ "device_remark_num", "0", 0},
	{ "dhcp_domain", "wan", 0},
	{ "dhcp_end", "192.168.0.200", 0},
	{ "dhcp_start", "192.168.0.100", 0},
	{ "dhcp_static_lease0", "", 0},
	{ "dhcp_static_lease1", "", 0},
	{ "dhcp_static_lease10", "", 0},
	{ "dhcp_static_lease11", "", 0},
	{ "dhcp_static_lease12", "", 0},
	{ "dhcp_static_lease13", "", 0},
	{ "dhcp_static_lease14", "", 0},
	{ "dhcp_static_lease15", "", 0},
	{ "dhcp_static_lease16", "", 0},
	{ "dhcp_static_lease17", "", 0},
	{ "dhcp_static_lease18", "", 0},
	{ "dhcp_static_lease19", "", 0},
	{ "dhcp_static_lease2", "", 0},
	{ "dhcp_static_lease3", "", 0},
	{ "dhcp_static_lease4", "", 0},
	{ "dhcp_static_lease5", "", 0},
	{ "dhcp_static_lease6", "", 0},
	{ "dhcp_static_lease7", "", 0},
	{ "dhcp_static_lease8", "", 0},
	{ "dhcp_static_lease9", "", 0},
	{ "dhcp_static_list", "", 0},
	{ "dhcp_wan0_mtu", "1500", 0},
	{ "dhcp_wins", "wan", 0},
	{ "dmz_ipaddr", "", 0},
	{ "dmz_ipaddr_en", "0", 0},
	{ "ecos_name", "ecos", 0},
	{ "err_check", "0", 0},
	{ "et0mdcport", "0", 0},
	{ "et0phyaddr", "30", 0},
	{ "ezc_enable", "1", 0},
	{ "ezc_version", "2", 0},
	{ "filter_client_cur_nu", "1", 0},
	{ "filter_client_mode", "disable", 0},
	{ "filter_client0", "", 0},
	{ "filter_client1", "", 0},
	{ "filter_client2", "", 0},
	{ "filter_client3", "", 0},
	{ "filter_client4", "", 0},
	{ "filter_client5", "", 0},
	{ "filter_client6", "", 0},
	{ "filter_client7", "", 0},
	{ "filter_client8", "", 0},
	{ "filter_client9", "", 0},
	{ "filter_mac_cur_nu", "1", 0},
	{ "filter_mac_loc", "0", 0},
	{ "filter_mac_mode", "deny", 0},
	{ "filter_mac_num", "0", 0},
	{ "filter_mac0", "", 0},
	{ "filter_mac1", "", 0},
	{ "filter_mac10", "", 0},
	{ "filter_mac11", "", 0},
	{ "filter_mac12", "", 0},
	{ "filter_mac13", "", 0},
	{ "filter_mac14", "", 0},
	{ "filter_mac15", "", 0},
	{ "filter_mac16", "", 0},
	{ "filter_mac17", "", 0},
	{ "filter_mac18", "", 0},
	{ "filter_mac19", "", 0},
	{ "filter_mac2", "", 0},
	{ "filter_mac3", "", 0},
	{ "filter_mac4", "", 0},
	{ "filter_mac5", "", 0},
	{ "filter_mac6", "", 0},
	{ "filter_mac7", "", 0},
	{ "filter_mac8", "", 0},
	{ "filter_mac9", "", 0},
	{ "filter_url_cur_nu", "1", 0},
	{ "filter_url_mode", "disable", 0},
	{ "filter_url0", "", 0},
	{ "filter_url1", "", 0},
	{ "filter_url10", "", 0},
	{ "filter_url11", "", 0},
	{ "filter_url12", "", 0},
	{ "filter_url13", "", 0},
	{ "filter_url14", "", 0},
	{ "filter_url15", "", 0},
	{ "filter_url2", "", 0},
	{ "filter_url3", "", 0},
	{ "filter_url4", "", 0},
	{ "filter_url5", "", 0},
	{ "filter_url6", "", 0},
	{ "filter_url7", "", 0},
	{ "filter_url8", "", 0},
	{ "filter_url9", "", 0},
	{ "forward_port_list", "", 0},
	{ "forward_port0", "", 0},
	{ "forward_port1", "", 0},
	{ "forward_port2", "", 0},
	{ "forward_port3", "", 0},
	{ "forward_port4", "", 0},
	{ "forward_port5", "", 0},
	{ "forward_port6", "", 0},
	{ "forward_port7", "", 0},
	{ "forward_port8", "", 0},
	{ "forward_port9", "", 0},
	{ "fw_disable", "0", 0},
	{ "fw_info", "V5.110.27.21", 0},
	{ "gpio20", "wps_button", 0},
	{ "gpio6", "sys_led", 0},
	{ "gpio7", "wps_led", 0},
	{ "hacker_att", "1", 0},
	{ "http_defaultpwd", "", 0},
	{ "http_defaultpwd1", "0", 0},
	{ "http_lanport", "80", 0},
	{ "http_passwd", "", 0},
	{ "http_passwd_tip", "", 0},
	{ "http_username", "admin", 0},
	{ "http_wanport", "", 0},
	{ "is_default", "1", 0},
	{ "is_modified", "0", 0},
	{ "lan_br", "bridge0", 0},//add by z10312 新增平台获取桥接口配置 16-0113
	{ "lan_dhcp", "0", 0},
	{ "lan_dns", "192.168.0.1", 0},
	{ "lan_domain", "", 0},
	{ "lan_gateway", "192.168.0.1", 0},
	{ "lan_hwnames", "", 0},
	{ "lan_ifname", "eth0", 0},
	{ "lan_ifnames", "eth0 wlan1 wlan0", 0},
	{ "lan_ipaddr", "192.168.0.1", 0},
	{ "lan_lease", "86400", 0},
	{ "lan_netmask", "255.255.255.0", 0},
	{ "lan_proto", "dhcp", 0},
	{ "lan_route", "", 0},
	{ "lan_stp", "1", 0},
	{ "lan_wins", "", 0},
	{ "lan_wps_oob", "disabled", 0},
	{ "lan_wps_reg", "enabled", 0},
	{ "lan0_pppoe_ifname", "ppp1", 0},
	{ "lan1_br", "bridge1", 0},
	{ "lan1_ifnames", "wlan1-va0 wlan0-va0", 0},
	{ "lan1_dhcp", "0", 0},
	{ "lan1_domain", "", 0},
	{ "lan1_gateway", "192.168.10.1", 0},
	{ "lan1_dns", "192.168.10.1", 0},
	{ "lan1_hwnames", "", 0},
	{ "lan1_ifname", "bridge1", 0},
	{ "lan1_ipaddr", "192.168.10.1", 0},
	{ "lan1_lease", "86400", 0},
	{ "lan1_netmask", "255.255.255.0", 0},
	{ "lan1_proto", "dhcp", 0},
	{ "lan1_route", "", 0},
	{ "lan1_stp", "1", 0},
	{ "lan1_wins", "", 0},
	{ "lan1_wps_oob", "enabled", 0},
	{ "lan1_wps_reg", "enabled", 0},
	{ "dhcp1_end", "192.168.10.200", 0},
	{ "dhcp1_start", "192.168.10.100", 0},
	{ "landevs", "eth0 wlan0", 0},
	{ "log_ipaddr", "", 0},
	{ "log_level", "0", 0},
	{ "mode_need_switch", "yes", 0},
	{ "nat_type", "sym", 0},
	{ "never_prompt_pppoe", "0", 0},
	{ "never_prompt_wlpwd", "0", 0},
	{ "ntp_server", "129.6.15.28 129.6.15.29 18.145.0.30 211.138.200.208", 0},
	{ "nvram_changed", "0", 0},
	{ "os_date", "Jan 13 2016", 0},
	{ "os_language", "en", 0},
	{ "os_name", "linux", 0},
	{ "os_server", "", 0},
	{ "os_version", "5.110.27.21", 0},
	{ "ping_dis_wan", "0", 0},
	{ "pppoe_index", "0", 0},
	{ "pppoe_wan0_mtu", "1480", 0},
	{ "qosList0", "", 0},
	{ "qosList1", "", 0},
	{ "qosList10", "", 0},
	{ "qosList12", "", 0},
	{ "qosList13", "", 0},
	{ "qosList14", "", 0},
	{ "qosList15", "", 0},
	{ "qosList16", "", 0},
	{ "qosList17", "", 0},
	{ "qosList18", "", 0},
	{ "qosList19", "", 0},
	{ "qosList2", "", 0},
	{ "qosList20", "", 0},
	{ "qosList21", "", 0},
	{ "qosList22", "", 0},
	{ "qosList23", "", 0},
	{ "qosList24", "", 0},
	{ "qosList25", "", 0},
	{ "qosList26", "", 0},
	{ "qosList27", "", 0},
	{ "qosList28", "", 0},
	{ "qosList29", "", 0},
	{ "qosList3", "", 0},
	{ "qosList30", "", 0},
	{ "qosList31", "", 0},
	{ "qosList32", "", 0},
	{ "qosList33", "", 0},
	{ "qosList34", "", 0},
	{ "qosList35", "", 0},
	{ "qosList36", "", 0},
	{ "qosList37", "", 0},
	{ "qosList38", "", 0},
	{ "qosList39", "", 0},
	{ "qosList4", "", 0},
	{ "qosList5", "", 0},
	{ "qosList6", "", 0},
	{ "qosList7", "", 0},
	{ "qosList8", "", 0},
	{ "qosList9", "", 0},
	{ "reboot_enable", "enable", 0},
	{ "reboot_max_rx_bytes", "3000", 0},
	{ "reboot_max_tx_bytes", "3000", 0},
	{ "reboot_time", "3:00-5:00", 0},
	{ "reboot_type", "1", 0},
	{ "remark_0", "", 0},
	{ "remark_1", "", 0},
	{ "remark_10", "", 0},
	{ "remark_11", "", 0},
	{ "remark_12", "", 0},
	{ "remark_13", "", 0},
	{ "remark_14", "", 0},
	{ "remark_15", "", 0},
	{ "remark_16", "", 0},
	{ "remark_17", "", 0},
	{ "remark_18", "", 0},
	{ "remark_19", "", 0},
	{ "remark_2", "", 0},
	{ "remark_3", "", 0},
	{ "remark_4", "", 0},
	{ "remark_5", "", 0},
	{ "remark_6", "", 0},
	{ "remark_7", "", 0},
	{ "remark_8", "", 0},
	{ "remark_9", "", 0},
	{ "remark_loc", "0", 0},
	{ "remark_num", "0", 0},
	{ "reset_gpio", "20", 0},
	{ "restart_time", "3:00-5:00", 0},
	{ "restore_defaults", "0", 0},
	{ "restore_quick_set", "1", 0},
	{ "rltk_wisp_wlan_id", "0", 0},
	{ "rltk_wlan_ack_timeout", "0", 0},
	{ "rltk_wlan_aggregation", "1", 0},
	{ "rltk_wlan_block_relay", "0", 0},
	{ "rltk_wlan_control_sideband", "1", 0},
	{ "rltk_wlan_iapp_disable", "0", 0},
	{ "rltk_wlan_ldpc_enable", "1", 0},
	{ "rltk_wlan_macclone_enable", "0", 0},
	{ "rltk_wlan_network_type", "0", 0},
	{ "rltk_wlan_preamble", "0", 0},
	{ "rltk_wlan_protection_disable", "1", 0},
	{ "rltk_wlan_short_gi", "1", 0},
	{ "rltk_wlan_sta_num", "0", 0},
	{ "rltk_wlan_stbc_enable", "0", 0},//BSPLJF++ 170224 在复杂环境下，关闭stbc功能，支持stbc的网卡会更倾向于用更高的速率向路由器发包
	{ "rltk_wlan_tx_beamforming", "1", 0},  //默认开启
	{ "rltk_wlan_wmm_enable", "1", 0},
	{ "rltk_wlan_wpa_group_rekey_time", "86400", 0},
	{ "rm_web_en", "0", 0},
	{ "rm_web_ip", "0.0.0.0", 0},
	{ "rm_web_port", "8080", 0},
	{ "sdram_config", "0x103", 0},
	{ "sdram_init", "0x0000", 0},
	{ "sdram_refresh", "0x0000", 0},
	{ "static_wan0_mtu", "1500", 0},
	{ "stats_server", "", 0},
	{ "tc_0", "", 0},
	{ "tc_1", "", 0},
	{ "tc_10", "", 0},
	{ "tc_11", "", 0},
	{ "tc_12", "", 0},
	{ "tc_13", "", 0},
	{ "tc_14", "", 0},
	{ "tc_15", "", 0},
	{ "tc_16", "", 0},
	{ "tc_17", "", 0},
	{ "tc_18", "", 0},
	{ "tc_19", "", 0},
	{ "tc_2", "", 0},
	{ "tc_3", "", 0},
	{ "tc_4", "", 0},
	{ "tc_5", "", 0},
	{ "tc_6", "", 0},
	{ "tc_7", "", 0},
	{ "tc_8", "", 0},
	{ "tc_9", "", 0},
	{ "tc_enable", "1", 0},
	{ "tc_isp_downrate", "1024", 0},
	{ "tc_isp_uprate", "1024", 0},
	{ "tc_loc", "0", 0},
	{ "tc_num", "0", 0},
	{ "tc_stream_stat_en", "0", 0},
	{ "td_adaptivity_mode", "0", 0},//Intel网卡性能兼容性调整开关
	{ "te_version", "en", 0},
	{ "time_mode", "0", 0},
	{ "time_zone", "57", 0},
	{ "timer_interval", "3600", 0},
	{ "tnvt2sn_n0ndnn_nr", "", 0},
	{ "upnp_enable", "1", 0},
	{ "ure_disable", "1", 0},
	{ "vlan1hwname", "et0", 0},
	{ "vlan2hwname", "et0", 0},
	{ "vlan2ports", "0 5*", 0},
	{ "wait_time", "20", 0},
	{ "wan_domain", "www.tendawifi.com", 0},
	{ "wan_ifnames", "eth1", 0},
	{ "wan_mode_check_verdict", "", 0},
	{ "wan_primary", "1", 0},
	{ "wan_speed", "0", 0},
	{ "wan0_check", "1", 0},
	{ "wan0_connect", "Disconnected", 0},
	{ "wan0_desc", "Default Connection", 0},
	{ "wan0_dns", "", 0},
	{ "wan0_dns_fix", "0", 0},
	{ "wan0_domain", "", 0},
	{ "wan0_gateway", "0.0.0.0", 0},
	{ "wan0_hostname", "Tenda", 0},
	{ "wan0_ifname", "eth1", 0},
	{ "wan0_ifnames", "eth1", 0},
	{ "wan0_ipaddr", "0.0.0.0", 0},
	{ "wan0_l2tp_demand", "0", 0},
	{ "wan0_l2tp_dns", "", 0},
	{ "wan0_l2tp_gateway", "0.0.0.0", 0},
	{ "wan0_l2tp_idletime", "60", 0},
	{ "wan0_l2tp_ifname", "ppp0", 0},
	{ "wan0_l2tp_ipaddr", "0.0.0.0", 0},
	{ "wan0_l2tp_keepalive", "1", 0},
	{ "wan0_l2tp_mru", "1452", 0},
	{ "wan0_l2tp_mtu", "1452", 0},
	{ "wan0_l2tp_netmask", "0.0.0.0", 0},
	{ "wan0_l2tp_passwd", "", 0},
	{ "wan0_l2tp_server_name", "", 0},
	{ "wan0_l2tp_static", "0", 0},
	{ "wan0_l2tp_username", "", 0},
	{ "wan0_macclone_mode", "default", 0},
	{ "wan0_mtu", "1500", 0},
	{ "wan0_netmask", "0.0.0.0", 0},
	{ "wan0_pppoe_ac", "", 0},
	{ "wan0_pppoe_demand", "0", 0},
	{ "wan0_pppoe_et", "0", 0},
	{ "wan0_pppoe_idletime", "60", 0},
	{ "wan0_pppoe_ifname", "ppp0", 0},
	{ "wan0_pppoe_keepalive", "1", 0},
	{ "wan0_pppoe_mru", "1480", 0},
	{ "wan0_pppoe_mtu", "1480", 0},
	{ "wan0_pppoe_passwd", "", 0},
	{ "wan0_pppoe_prio_xkjs", "0", 0},
	{ "wan0_pppoe_service", "", 0},
	{ "wan0_pppoe_st", "0", 0},
	{ "wan0_pppoe_username", "", 0},
	{ "wan0_pptp_demand", "0", 0},
	{ "wan0_pptp_dns", "", 0},
	{ "wan0_pptp_gateway", "0.0.0.0", 0},
	{ "wan0_pptp_idletime", "60", 0},
	{ "wan0_pptp_ifname", "ppp0", 0},
	{ "wan0_pptp_ipaddr", "0.0.0.0", 0},
	{ "wan0_pptp_keepalive", "1", 0},
	{ "wan0_pptp_mppe_en", "1", 0},
	{ "wan0_pptp_mru", "1452", 0},
	{ "wan0_pptp_mtu", "1452", 0},
	{ "wan0_pptp_netmask", "0.0.0.0", 0},
	{ "wan0_pptp_passwd", "", 0},
	{ "wan0_pptp_server_name", "", 0},
	{ "wan0_pptp_static", "0", 0},
	{ "wan0_pptp_username", "", 0},
	{ "wan0_primary", "1", 0},
	{ "wan0_proto", "dhcp", 0},
	{ "wan0_proto_index", "pppoe", 0},
	{ "wan0_route", "", 0},
	{ "wan0_tmp_pppoe_passwd", "", 0},
	{ "wan0_tmp_pppoe_username", "", 0},
	{ "wan0_unit", "0", 0},
	{ "wan0_wins", "", 0},
	{ "wan1_pppoe_ifname", "ppp2", 0},
	{ "wandevs", "eth1", 0},
	{ "watchdog", "45000", 0},
	{ "white_list0", "", 0},
	{ "white_list1", "", 0},
	{ "white_list10", "", 0},
	{ "white_list11", "", 0},
	{ "white_list12", "", 0},
	{ "white_list13", "", 0},
	{ "white_list14", "", 0},
	{ "white_list15", "", 0},
	{ "white_list16", "", 0},
	{ "white_list17", "", 0},
	{ "white_list18", "", 0},
	{ "white_list19", "", 0},
	{ "white_list2", "", 0},
	{ "white_list3", "", 0},
	{ "white_list4", "", 0},
	{ "white_list5", "", 0},
	{ "white_list6", "", 0},
	{ "white_list7", "", 0},
	{ "white_list8", "", 0},
	{ "white_list9", "", 0},
            /*归一化，家长控制*/
	{"parentCtl_client_num", "0", 0},
	{"parentCtl_client_list1", "", 0},
	{"parentCtl_client_list2", "", 0},
	{"parentCtl_client_list3", "", 0},
	{"parentCtl_client_list4", "", 0},
	{"parentCtl_client_list5", "", 0},
	{"parentCtl_client_list6", "", 0},
	{"parentCtl_client_list7", "", 0},
	{"parentCtl_client_list8", "", 0},
	{"parentCtl_client_list9", "", 0},
	{"parentCtl_client_list10", "", 0},
	{"parentCtl_time", "19:00-21:00", 0},
	{"parentCtl_date", "00000110", 0},
	{"parentCtl_urlFilter_mode", "disable", 0},
	{"parentCtl_urlFilter_list", "", 0},
	{ "pingwan_enable", "1", 0}, //ping wan add by zgd
	{"led_time","00:00-07:00",0},		//LED定时关闭时间段
	{"led_ctl_type","0",0},		//LED控制类型	0:常开 1:常关  2:定时关闭
	{"iptv_enable","0",0},		//IPTV开关
	{"iptv_portName","lan1",0},		//IPTV 口
	{ 0, 0, 0 }
};


struct nvram_tuple tenda_factory_defaults[] = {
	//以下参数为制作flash中需要修改的参数
	{ "wl_obss_coex", "0", 0},
	{ "wl_nbw_cap", "0", 0},
	{ "wl0_nbw_cap", "0", 0},
	{ "wl_channel", "7", 0},
	{ "wl0_obss_coex", "0", 0},
	{ "wl0_channel", "7", 0},
	{ "restore_quick_set", "0", 0},
	{ "mode_need_switch", "no", 0},
	//以下参数为页面导烧录时临时加进去，根据实际情况去修改
	//{ "extend_pass", "", 0},
	//{ "restore_pppoe_first", "1", 0},
	//{ "wan0_hwaddr_bak", "C8:3A:35:DB:E0:B0", 0},
	//{ "wan0_macclone_mode_bak", "clone", 0},
	//{ "wan0_proto_index", "dhcp", 0},
	//{ "wl_macmode", "deny", 0},
	//{ "wl0_wps_pin", "", 0},
	{ 0, 0, 0 }
};




struct nvram_tuple tenda_envram_defaults[] = {
    { "HW_BOARD_VER", "1", 0},
    { "wl0_ctv_power", "high", 0},
    { "wl1_ctv_power", "high", 0},
    { "wl0_country_pwr_power", "78,92,100", 0},
    { "wl1_country_pwr_power", "92,100", 0},
    { "country_offset_power", "12,6,0", 0},//功率设为最大值23x2，然后根据 低、中、高分别减country_offset_power对应的值 fh add
    { "HW_WLAN0_11N_THER", "28", 0},
	{ "HW_WLAN0_11N_XCAP", "36", 0},	
	{ "HW_WLAN0_REG_DOMAIN", "13", 0},
	{ "HW_WLAN0_TX_POWER_5G_HT40_1S_A", "0000000000000000000000000000000000000000000000000000000000000000000000262626262626262626262626262626262626262626272727272727272727272727272727272727272727272727272727272727272727272727272727272727272727272727272727272727272728282828282828282828282828282828272727272727272727272727272727271b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b14141414141414140f0f0f0f0f0f0f0f00000000000000000000000000000000000000", 0},
	{ "HW_WLAN0_TX_POWER_5G_HT40_1S_B", "00000000000000000000000000000000000000000000000000000000000000000000002828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282828282823232323232323232323232323232323231f1f1f1f1f1f1f1f1b1b1b1b1b1b1b1b00000000000000000000000000000000000000", 0},
	{ "HW_WLAN0_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A", "2424242424242424242424000000", 0},
	{ "HW_WLAN0_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B", "2424242424242424242424010101", 0},
	{ "HW_WLAN0_TX_POWER_DIFF_5G_40BW2S_20BW2S_A", "0202020202020202020202000000", 0},
	{ "HW_WLAN0_TX_POWER_DIFF_5G_40BW2S_20BW2S_B", "0202020202020202020202000000", 0},	
	{ "HW_WLAN1_11N_THER", "36", 0},
	{ "HW_WLAN1_11N_XCAP", "2", 0},
	{ "HW_WLAN1_REG_DOMAIN", "13", 0},
	{ "HW_WLAN1_TX_POWER_CCK_A", "1515151616161616161818181818", 0},
	{ "HW_WLAN1_TX_POWER_CCK_B", "1313131515151515151818181818", 0},
	{ "HW_WLAN1_TX_POWER_DIFF_HT20", "ffffff0000000000000000000000", 0},
	{ "HW_WLAN1_TX_POWER_DIFF_HT40_2S", "0000000000000000000000000000", 0},
	{ "HW_WLAN1_TX_POWER_DIFF_OFDM", "2222222222222222222222222222", 0},
	{ "HW_WLAN1_TX_POWER_HT40_1S_A", "1919191a1a1a1a1a1a1b1b1b1b1b", 0},
	{ "HW_WLAN1_TX_POWER_HT40_1S_B", "1818181919191919191c1c1c1c1c", 0},
    { "HW_WLAN0_RF_TYPE", "a", 0},
    { "HW_WLAN0_LED_TYPE", "7", 0},
    { "HW_WLAN0_11N_TSSI1", "0", 0},
    { "HW_WLAN0_11N_TSSI2", "0", 0},
    { "HW_WLAN0_11N_TRSWITCH", "0", 0},
	{ "HW_WLAN0_11N_TRSWPAPE_C9", "0", 0},
	{ "HW_WLAN0_11N_TRSWPAPE_CC", "0", 0},
	{ "HW_WLAN1_RF_TYPE", "a", 0},
    { "HW_WLAN1_LED_TYPE", "7", 0},
    { "HW_WLAN1_11N_TSSI1", "0", 0},
    { "HW_WLAN1_11N_TSSI2", "0", 0},
    { "HW_WLAN1_11N_TRSWITCH", "0", 0},
	{ "HW_WLAN1_11N_TRSWPAPE_C9", "0", 0},
	{ "HW_WLAN1_11N_TRSWPAPE_CC", "0", 0},
    { "wl0_hwaddr", "00:90:4C:88:88:89", 0},
	{ "wl1_hwaddr", "00:90:4C:88:88:8D", 0},
    { "et0macaddr", "00:90:4C:88:88:88", 0},
    { "BOARD_NAME", "AC6_V3.0", 0},
    { "country_code", "CN", 0},
    { "RFTestFlag", "0", 0},	//RFTestFlag ThroughputTestFlag FinishTestFlag产测防止漏测参数
    { "ThroughputTestFlag", "0", 0},
    { "FinishTestFlag", "0", 0},
    { 0, 0, 0 }
};


