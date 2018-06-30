
var language_sel='语言选择:';
var language_sc='简体中文';
var language_en='English';
/***********	menu.htm	************/
var menu_site ='网站内容';
var menu_wizard ='设置向导';
var menu_opmode ='模式设置';
var menu_wireless='无线';
var menu_basic='基本设置';
var menu_advance='高级设置';
var menu_security='安全';
var menu_8021xCert='802.1x 证书安装';
var menu_accessControl='访问控制';
var menu_wds='WDS设置';
var menu_mesh='网格设置';
var menu_siteSurvey='站点扫描';
var menu_wps='WPS';
var menu_schedule='时间表';
var menu_tcpip='TCP/IP 设置';
var menu_lan='局域网设置';
var menu_wan='广域网设置';
var menu_fireWall='防火墙';
var menu_portFilter='端口过滤';
var menu_ipFilter='IP地址过滤';
var menu_macFilter='MAC地址过滤';
var menu_portFw='端口转发';
var menu_urlFilter='URL过滤';
var menu_dmz='隔离区';
var menu_vlan='虚拟局域网';
var menu_qos='服务质量控制';
var menu_routeSetup='路由设置';
var menu_management='管理';
var menu_status='状态';
var menu_statistics='统计信息';
var menu_ddns='动态域名服务';
var menu_timeZone='时区设置';
var menu_dos='拒绝服务攻击';
var menu_log='日志记录';
var menu_updateFm='升级固件';
var menu_setting='保存/加载设置';
var menu_psw='设置密码';
var menu_logout='注销';
/***********	opmode.htm	************/
var opmode_header='模式设置';
var opmode_explain='你可以对LAN和WAN设置NAT/bridge功能';
var opmode_radio_gw='网关:';
var opmode_radio_gw_explain= '在该模式下，设备通过ADSL或者调制解调器连接到因特网。NAT是使能的，并且局域网端口的主机通过广域网端口共享同一个IP连接到因特网的服务提供商。在广域网页面下可以设置不同的连接类型，类型包括了PPPOE，DHCP客户端，PPTP客户端，L2TP客户端，静态IP。';
var opmode_radio_br='桥接:';
var opmode_radio_br_explain= '在该模式下，所有的以太网端口和无线接口桥接在一起，且NAT功能是不使能的。该模式同样不支持所有与广域网端口有关的功能及防火墙';
var opmode_radio_wisp='无线网络服务提供商:';
var opmode_radio_wisp_explain= '在该模式下，所有的以太网端口桥接在一起，无线客户端连接到网络服务提供商的接入点。NAT是使能的，且以太网端口的主机通过无线局域网共享同一个IP连接到网络服务提供商。在网络扫描页面下可以连接到网络服务提供商的接入点。在广域网页面下可以设置不同的连接类型，类型包括了PPPOE，DHCP客户端，PPTP客户端，L2TP客户端，静态IP。';
var opmode_radio_wisp_wanif='WAN 接口 : ';
var opmode_apply='设置';
var opmode_reset='重置';

/***********	wlan_schedule.htm	************/
var wlan_schedule_header = 'wlan时间表';
var wlan_schedule_explain = '你可以设置wlan时间表，配置系统时间时需打开wlan时间表';
var wlan_schedule_enable = '启动wlan时间表';
var wlan_schedule_everyday = '每天';
var wlan_schedule_sun = '星期天';
var wlan_schedule_mon = '星期一';
var wlan_schedule_tue= '星期二';
var wlan_schedule_wed = '星期三';
var wlan_schedule_thu = '星期四';
var wlan_schedule_fri = '星期五';
var wlan_schedule_sat = '星期六';
var wlan_schedule_24Hours = '24小时';
var wlan_schedule_from = '从';
var wlan_schedule_to = '到';
var wlan_schedule_save = '设置';
var wlan_schedule_reset = '重置';
var wlan_schedule_days = '天';
var wlan_schedule_time = '时间';
var wlan_schedule_time_check = '请设置天!';


/***********	wlwps.htm	************/
var wlwps_header = 'Wi-Fi保护设置';
var wlwps_header_explain = '你可以改变WPS(Wi-Fi保护)的设置. 在这种性能下能够使你的无线网络同步它的设置，并且快速的连接到AP.';
var wlwps_wps_disable = '禁止WPS';
var wlwps_wps_save = '设置';
var wlwps_wps_reset = '重置';
var wlwps_status = 'WPS状态:';
var wlwps_status_conn = '配置';
var wlwps_status_unconn = '未配置';
var wlwps_runon = 'WPS接口:';
var wlwps_runon_root = '主接口';
var wlwps_runon_rpt = '虚接口';
var wlwps_status_reset = '重置到未配置';
var wlwps_lockdown_state = '自动锁定状态';
var wlwps_self_pinnum = '自己的PIN :';
var wlwps_unlockautolockdown = '解锁';
var wlwps_lockdown_state_locked = '锁定';
var wlwps_lockdown_state_unlocked = '未锁定';
var wlwps_pbc_title = '按键配置:';
var wlwps_pbc_start_button = '开始配置';
var wlwps_stopwsc_title = '停止WSC';
var wlwps_stopwsc_button = '停止WSC';
var wlwps_pin_number_title = '客户PIN值:';
var wlwps_pin_start_button = '开始PIN';
var wlwps_keyinfo = '当前密匙信息:';
var wlwps_authentication = '认证';
var wlwps_authentication_open = '打开';
var wlwps_authentication_wpa_psk = 'WPA PSK';
var wlwps_authentication_wep_share = 'WEP共享';
var wlwps_authentication_wpa_enterprise = '企业级WPA模式';
var wlwps_authentication_wpa2_enterprise = '企业级WPA2模式';
var wlwps_authentication_wpa2_psk = 'WPA2 PSK';
var wlwps_authentication_wpa2mixed_psk = 'WPA2混杂模式 PSK';
var wlwps_encryption = '加密';
var wlwps_encryption_none= '无';
var wlwps_encryption_wep = 'WEP';
var wlwps_encryption_tkip = 'TKIP';
var wlwps_encryption_aes = 'AES';
var wlwps_encryption_tkip_aes = 'TKIP+AES';
var wlwps_key = '密匙';
var wlwps_pin_conn = 'PIN配置:';
var wlwps_assign_ssid = '分配的SSID:';
var wlsps_assign_mac = '分配的MAC:';
var wlwps_wpa_save_pininvalid = '无效的PIN长度! 设备PIN通常是4或8位长.';
var wlwps_wpa_save_pinnum = '无效的PIN! 设备PIN必须为数字.';
var wlwps_wpa_save_pinchecksum = '无效的PIN，校验和错误.';
var wlwps_pinstart_pinlen = '无效的初始PIN长度! 设备PIN通常是4或8位长.';
var wlwps_pinstart_pinnum = '无效的初始PIN! 初始PIN必须是数字.';
var wlwps_pinstart_pinchecksum = '校验和错误! 必须使用PIN? ';

var warn_msg1='因为无线模式设置不被支持,因此WPS被禁用. ' + '必须去无线网络/基本配置见面去改变配置从而启用WPS.';
var warn_msg2='因为Radius认证不被支持,因此WPS被禁用.' +'必须去无线网络/安全性界面去改变设置从而启用WPS.';
var warn_msg3 = 'PIN值被创建.必须单击设置按钮使其生效';

/***********	wlsurvey.htm	************/
var wlsurvey_onewlan_header = '无线网络扫描</p>';
var wlsurvey_morewlan_header = '无线网络扫描 -wlan';
var wlsurvey_header_explain = '提供一些工具扫描信道. 当用户模式打开后，你可以手动的选择接入AP.';
var wlsurvey_site_survey = '扫描';
var wlsurvey_chan_next = '<input type="button" value="  前进>>" id="next" onClick="saveClickSSID()">';
var wlsurvey_encryption = '加密:';
var wlsurvey_encryption_no = '无';
var wlsurvey_encryption_wep = 'WEP';
var wlsurvey_encryption_wpa = 'WPA';
var wlsurvey_encryption_wpa2= 'WPA2';
var wlsurvey_keytype = '密匙类型:';
var wlsurvey_keytype_open = '开放';
var wlsurvey_keytype_shared = '共享';
var wlsurvey_keytype_both = 'Both';
var wlsurvey_keylen = '密匙长度:';
var wlsurvey_keylen_64 = '64比特';
var wlsurvey_keylen_128 = '128比特';
var wlsurvey_keyfmt = '密匙格式:';
var wlsurvey_keyfmt_ascii = 'ASCII';
var wlsurvey_keyfmt_hex = 'Hex';
var wlsurvey_keyset = '密匙设置';
var wlsurvey_authmode = '验证模式:';
var wlsurvey_authmode_enter_radius = '企业级 (RADIUS)';
var wlsurvey_authmode_enter_server = '企业级(作为服务器)';
var wlsurvey_authmode_personal = '个人 (共享密匙)';
var wlsurvey_wpacip = 'WPA密钥套件:';
var wlsurvey_wpacip_tkip = 'TKIP';
var wlsurvey_wpacip_aes = 'AES';
var wlsurvey_wp2chip = 'WPA2密钥套件:';
var wlsurvey_wp2chip_tkip = 'TKIP';
var wlsurvey_wp2chip_aes = 'AES';
var wlsurvey_preshared_keyfmt = '预共享密匙格式:';
var wlsurvey_preshared_keyfmt_passphrase = '密码';
var wlsurvey_preshared_keyfmt_hex = 'HEX (64字符)';
var wlsurvey_preshared_key = '预共享密匙:';
var wlsurvey_eaptype = 'EAP类型:';
var wlsurvey_eaptype_md5 = 'MD5';
var wlsurvey_eaptype_tls = 'TLS';
var wlsurvey_eaptype_peap = 'PEAP';
var wlsurvey_intunn_type = '内部隧道类型:';
var wlsurvey_intunn_type_MSCHAPV2 = 'MSCHAPV2';
var wlsurvey_eap_userid = 'EAP用户ID:';
var wlsurvey_radius_passwd = 'RADIUS用户密码';
var wlsurvey_radius_name = 'RADIUS用户名:';
var wlsurvey_user_keypasswd = '用户密钥密码(如果存在):';


var wlsurvey_use_as_server = '用户本地AS服务器:';
var wlsurvey_as_ser_ipaddr = 'AS 服务器;IP 地址:';
var wlsurvey_back_button = '<<后退  ';
var wlsurvey_conn_button = '连接';
var wlsurvey_wait_explain = '请等待...';
var wlsurvey_inside_nosupport = '内部类型不支持.';
var wlsurvey_eap_nosupport = 'EAP类型不支持.';
var wlsurvey_wrong_method = '错误的方法!';
var wlsurvey_nosupport_method = '错误: 不支持方法ID';
var wlsurvey_nosupport_wpa2 = '错误: 不支持wpa2密码套件 ';
var wlsurvey_nosupport_wpasuit = '错误: 不支持wpa密码套件 ';
var wlsurvey_nosupport_encry = '错误: 不支持加密 ';
var wlsurvey_tbl_ssid = 'SSID';
var wlsurvey_tbl_bssid = 'BSSID';
var wlsurvey_tbl_chan = '信道';
var wlsurvey_tbl_type = '类型';
var wlsurvey_tbl_ency = '加密算法';
var wlsurvey_tbl_signal = '信号强度';
var wlsurvey_tbl_select = '选择';
var wlsurvey_tbl_macaddr = 'MAC地址';
var wlsurvey_tbl_meshid = 'Mesh ID';
var wlsurvey_tbl_none = '无';

var wlsurvey_read_site_error = '扫描状态读取失败!';
var wlsurvey_get_modemib_error = '获取MIB_WLAN_MODE的MIB值失败!';
var wlsurvey_get_bssinfo_error = '获取bssinfo失败!';



/***********	wlwds.htm	************/
var wlwds_onelan_header = 'WDS设置</p>';
var wlwds_morelan_header = 'WDS设置 -wlan';
var wlwds_header_explain = '无线分布式系统通过无线介质与其他AP相联.为了实现这个功能,你必须将这些AP设置在同一信道上，为其他你想通信的AP设置MAC地址，最后代开WDS.';
var wlwds_enable = '打开WDS';
var wlwds_mac_addr = 'MAC地址:';
var wlwds_data_rate = '数据速率:';
var wlwds_rate_auto = '自动';
var wlwds_comment = '注释:';
var wlwds_apply = '设置';
var wlwds_reset = '重置';
var wlwds_set_secu = '设置安全性';
var wlwd_show_stat = '显示统计值';
var wlwds_wdsap_list = '当前WDS AP列表:';
var wlwds_delete_select = '删除所选';
var wlwds_delete_all = '删除全部';
var wlwds_fmwlan_txrate = '数据速率(Mbps)';
var wlwds_fmwlan_select = '选择';
var wlwds_fmwlan_comment = '注释';
var wlwds_macaddr_nocomplete = '输入的MAC地址不完整，MAC地址必须是12个16进制的长度.';
var wlwds_macaddr_invalid = '无效的MAC地址,MAC地址必须是16进制.';
var wlwds_delete_chick = '确定删除所选的入口?';
var wlwds_delete_allchick = '确定删除所有的入口?';

/***********	wlactrl.htm	************/
var wlactrl_onelan_header = '无线接入控制</p>';
var wlactrl_morelan_header = '无线接入控制 -wlan';
var wlactrl_header_explain = '如果你选择允许流出的, 只有那些MAC地址在接入控制列表中的用户才能接入到你的AP。当选择拒绝列出的，在列表中的用户不能接入AP。';
var wlactrl_accmode = '无线接入控制模式:';
var wlactrl_accmode_diable = '禁用';
var wlactrl_accmode_allowlist = '允许列出的';
var wlactrl_accmode_denylist = '拒绝列出的';
var wlactrl_macaddr = 'MAC地址:';
var wlactrl_comment = '注释: ';
var wlactrl_apply = '设置';
var wlactrl_reset = '重置';
var wlactrl_accctrl_list = '当前接入控制列表:';
var wlactrl_delete_select_btn = '删除选择的';
var wlactrl_delete_all_btn = '删除所有';
var wlactrl_fmwlan_macaddr = 'MAC地址';
var wlactrl_fmwlan_select = '选择';
var wlactrl_apply_explain = '如果ACL允许列表打开; WPS2.0将不起作用';
var wlactrl_apply_mac_short = '输入的MAC地址不完整，MAC地址必须是12个16进制的长度. ';
var wlactrl_apply_mac_invalid = '无效的MAC地址,MAC地址必须是16进制.';
var wlactrl_delete_result = '删除所有的入口将会导致用户不能接入AP，确定?';
var wlactrl_delete_select = '确定删除所选的入口?';
var wlactrl_delete_all = '确定删除所有的入口?';

/***********	firewall	************/
var firewall_proto = '协议:';
var firewall_proto_both = 'Both';
var firewall_proto_tcp = 'TCP';
var firewall_proto_udp = 'UDP';
var firewall_add_rule = '增加规则';
var firewall_apply = '设置';
var firewall_reset = '重置';
var firewall_filtertbl = '当前过滤表:';
var firewall_delete_select = '删除所选';
var firewall_delete_all = '删除所有';

var firewall_delete_selectconfm = '确定删除所选的入口?';
var firewall_delete_allconfm = '确定删除所有的入口?';
var firewall_ipaddr_invalid = '无效的IP地址';
var firewall_port_notdecimal = '无效的端口号! 必须为0-9的十进制数.';
var firewall_port_toobig = '无效的端口号! 必须设置为1-65535的值.';
var firewall_port_rangeinvalid = '无效的端口范围! 第1个端口值必须小于第2个.';


var firewall_local_ipaddr = '本地IP地址:';
var firewall_port_range = '端口范围: ';
var firewall_comm = '注释:';
var firewall_ip_invalid_range = '无效的ip地址范围\n结束地址必须大于或者等于起始地址。';


var firewall_tbl_proto = '协议';
var firewall_tbl_comm = '注释';
var firewall_tbl_select = '选择';
var firewall_tbl_localipaddr = '本地IP地址';
var firewall_portrange = '端口范围';

/***********	portfilter.htm	************/
var portfilter_header = '端口过滤';
var portfilter_header_explain = ' 表中的条目用来限制一些从本地网络通过网关发往Internet的特定的数据包,使用这种过滤规则能够保护或者限制本地网络.';
var portfiletr_enable = '打开端口过滤';
var portfilter_noport = '必须为过滤表向设置端口范围';


/***********	ipfilter.htm	************/
var ipfilter_header = 'IP过滤';
var ipfilter_header_explain = ' 表中的条目用来限制一些从本地网络通过网关发往Internet的特定的数据包,使用这种过滤规则能够保护或者限制本地网络.';
var ipfilter_enable = '打开IP过滤';

/***********	Macfilter.htm	************/
var macfilter_header = 'MAC过滤';
var macfilter_header_explain =  '表中的条目用来限制一些从本地网络通过网关发往Internet的特定的数据包,使用这种过滤规则能够保护或者限制本地网络.';
var macfilter_enable = '打开MAC过滤';
var macfilter_macaddr = 'MAC地址: ';
var macfilter_macaddr_nocomplete = '输入的MAC地址不完整，MAC地址必须是12位16进制';
var macfilter_macaddr_nohex = '无效的MAC地址,MAC地址必须为16进制的数字.';
var macfilter_filterlist_macaddr = 'MAC地址';

/***********	Portfw.htm	************/
var portfw_header = '端口转发';
var portfw_header_explain = '表中的条目允许你将一些公共网络服务设置在NAT防火墙后一个内网设备.只有当你希望在私有网络设置类似web或mail服务器时，才需要进行配置.';
var portfw_enable = '打开端口转发';
var portfw_ipaddr = 'IP地址:';
var portfw_apply_port_empty = '端口范围不能为空! 必须将端口的值设置为1-65535.';
var portfw_tbl = '当前的端口转发表:';

/***********	urlfilter.htm	************/
var urlfilter_header = 'URL过滤';
var urlfilter_header_explain = ' URL过滤用来拒绝LAN用户访问Internet的请求,阻止用户访问那些包含关键字的URL.';
var urlfilter_enable = '打开URL过滤';
var urlfilter_urladdr = 'URL地址: ';
var urlfilter_apply_error = '错误符号: \";\"';
var urlfilter_filterlist_yrladdr = 'URL地址';

/***********	dmz.htm	************/
var dmz_header = 'DMZ';
var dmz_header_explain = 'DMZ可以在不牺牲内网的非授权访问的特性来提供Internet服务.典型的,DMZ主机包含一些必须被Internet公开访问的设备,例如Web(HTTP)服务器,FTP服务器,SMTP(e-mail)服务器和DNS服务器.';
var dmz_enable = '打开DMZ';
var dmz_host_ipaddr = 'DMZ主机IP地址:';

/***********	logout.htm	************/
var logout_header = '注销';
var logout_header_explain = '这个页面用来退出系统.';
var logout_confm = '确定要退出系统 ?';
var logout_apply = '设置';

/***********	password.htm	************/
var password_header = '密码设置';
var password_header_explain = '这个页面用来设置帐号访问AP的web服务器,空的用户名和密码将会取消这种保护.';
var password_user_name = '用户名:';
var password_user_passwd = '新密码:';
var password_user_passwd_confm = '确认密码:';
var password_apply = '设置';
var password_user_empty = '用户帐号是空的.\n取消密码保护?';
var password_passwd_unmatched = '密码不匹配,请在新密码栏和确认密码栏中重新输入相同的密码';
var password_passwd_empty = '密码不能为空,请重新输入.';
var password_user_invalid = '用户名不接受空格符,请重新输入.';
var password_passwd_invalid = '密码不接受空格符,请重新输入.';
var password_reset = '  重置  ';

/***********	saveconf.htm	************/
var saveconf_header = '保存/重新加载配置';
var saveconf_header_explain = '这个页面允许你保存当前的配置到文件中,也可以从文件中加载新的配置.除此之外,你还可以将当前配置恢复成默认配置.';
var saveconf_save_to_file = '保存配置到文件:';
var saveconf_save = '保存...';
var saveconf_load_from_file = '加载配置从文件:';
var saveconf_load = '上传';
var saveconf_reset_to_default = '恢复默认配置:';
var saveconf_reset = '重置';
var saveconf_load_from_file_empty = '请选择一个文件!';
var saveconf_reset_to_default_confm = '确认将当前设置恢复成默认配置?';

/***********	upload.htm	************/
var upload_header = '升级固件';
var upload_header_explain = '这个页面允许你更新AP的固件,请注意,在上传的过程中设备如果断电可能会使系统崩溃.';
var upload_version = '固件版本:';
var upload_file_select = '选择文件:';
var upload_send = '上传';
var upload_reset = '重置';
var upload_up_failed = '更新固件失败!';
var upload_send_file_empty = '文件名不能为空 !';

/***********	syslog.htm	************/
var syslog_header = '系统日志';
var syslog_header_explain = '这个页面被用来设置远程日志服务器和现实系统日志.';
var syslog_enable = '打开日志';
var syslog_sys_enable = ' 整个系统 ';
var syslog_wireless_enable = '无线网络';
var syslog_dos_enable = '拒绝服务';
var syslog_11s_enable = '11s';
var syslog_rtlog_enable = '打开远程日志';
var syslog_local_ipaddr = '日志服务器IP地址:';
var syslog_apply = '设置';
var syslog_refresh = '刷新';
var syslog_clear = ' 清除 ';

/***********	dos.htm	************/
var dos_header = '拒绝服务';
var dos_header_explain = '拒绝服务攻击是由黑客发动的攻击,以阻止合法用户访问互联服务.';
var dos_enable = '启用防止拒绝服务';

var dos_packet_sec = ' 包/秒';
var dos_sysflood_syn = '整个系统泛洪: SYN';
var dos_sysflood_fin = '整个系统泛洪: FIN';
var dos_sysflood_udp = '整个系统泛洪: UDP';
var dos_sysflood_icmp = '整个系统泛洪: ICMP';
var dos_ipflood_syn = '每个源口IP泛洪: SYN';
var dos_ipflood_fin = '每个源口IP泛洪: FIN';
var dos_ipflood_udp = '每个源口IP泛洪: UDP';
var dos_ipflood_icmp = '每个源口IP泛洪: ICMP';
var dos_portscan = 'TCP/UDP端口扫描';
var dos_portscan_low = '低';
var dos_portscan_high = '高';
var dos_portscan_sensitivity = '敏感性';
var dos_icmp_smurf = 'ICMP Smurf';
var dos_ip_land = 'IP Land';
var dos_ip_spoof = 'IP Spoof';
var dos_ip_teardrop = 'IP TearDrop';
var dos_pingofdeath = 'PingOfDeath';
var dos_tcp_scan = 'TCP Scan';
var dos_tcp_synwithdata = 'TCP SynWithData';
var dos_udp_bomb = 'UDP Bomb';
var dos_udp_echochargen = 'UDP EchoChargen';
var dos_select_all = ' 全部选择 ';
var dos_clear_all = '全部清除';
var dos_enable_srcipblocking = '启动源IP阻止';
var dos_block_time = '阻止时间(秒)';
var dos_apply = '设置';

/***********	ntp.htm	************/
var ntp_header = '时区设置';
var ntp_header_explain = '你可以通过与Internet上的时间服务器进行同步来维护系统时间.';
var ntp_curr_time = ' 当前时间 : ';
var ntp_year = '年';
var ntp_month = '月';
var ntp_day = '日';
var ntp_hour = '小时';
var ntp_minute = '分';
var ntp_second = '秒';
var ntp_copy_comptime = '复制计算机时间';
var ntp_time_zone = '时区选择 : ';
var ntp_enable_clientup = '启动NTP客户端更新 ';
var ntp_adjust_daylight = '自动夏令时调整 ';
//var ntp_server = ' SNTP服务器 : ';
var ntp_server = ' NTP服务器 : ';
var ntp_server_north_america1 = '198.123.30.132    - 北美';
var ntp_server_north_america2 = '209.249.181.22   - 北美';
//var ntp_server_Europe1 = '85.12.35.12  - 欧洲';
//var ntp_server_Europe2 = '217.144.143.91   - 欧洲';
var ntp_server_Europe1 = '131.188.3.220  - 欧洲';
var ntp_server_Europe2 = '130.149.17.8   - 欧洲';
var ntp_server_Australia = '223.27.18.137  - 澳洲';
//var ntp_server_asia1 = '133.100.11.8 - 亚太区';
var ntp_server_asia1 = '203.117.180.36 - 亚太区';
var ntp_server_asia2 = '210.72.145.44 - 亚太区';
var ntp_manu_ipset = ' (手动IP设置) ';
var ntp_apply = '设置';
var ntp_reset = '重置';
var ntp_refresh = '刷新';
var ntp_month_invalid = '无效的月份,这些值必须由0-9的数字组成.';
var ntp_time_invalid = '无效的时间!';
var ntp_ip_invalid = '无效的IP地址';
var ntp_servip_invalid = '无效的NTP服务器IP地址!这个值不能为空.';
var ntp_field_check = ' 域不能为空\n';
var ntp_invalid = '无效的 ';
var ntp_num_check = ' 值,这些值必须由0-9的数字组成.';

/***********	ddns.htm	************/
var ddns_header = '动态域名服务器';
var ddns_header_explain = '动态域名服务为你提供一个可变IP地址和有效且不变的互联网域名(URL)的之间的配对.';
var ddns_enable = '启动DDNS ';
var ddns_serv_provider = ' 服务提供商 : ';
var ddns_dyndns = 'DynDNS ';
var ddns_orayddns = 'OrayDDNS ';
var ddns_orayddns = 'TZO ';
var ddns_domain_name = '域名 : ';
var ddns_user_name = ' 用户名/邮箱: ';
var ddns_passwd = ' 密码/密匙: ';
var ddns_note = '注解:';
var ddns_oray_header = '对于Oray DDNS,你可以创建自己的Oray帐号';
var ddns_here = '这里 ';
var ddns_dyn_header = '对于DynDNS,你可以创建自己的DynDNS帐号';
var ddns_tzo_header1 = '对于TZO，你可以获得30天免费试';
var ddns_tzo_header2 = '或在这里管理你的帐号';
var ddns_tzo_header3 = '管理';
var ddns_apply = '设置';
var ddns_reset = '重置';
var ddns_domain_name_empty = '域名不能为空';
var ddns_user_name_empty = '用户名/邮箱不能为空';
var ddns_passwd_empty = '密码/密匙不能为空';

/***********	ip_qos.htm	************/
var ip_qos_header = 'QoS';
var ip_qos_header_explain = ' 表中的条目用于改善你的在线游戏体验,确保你的游戏流量的优先级高于其他网络流量.';
var ip_qos_enable = '启动QoS';
var ip_qos_bandwidth = '带宽限速';
var ip_qos_schedule = '加权队列调度';
var ip_qos_auto_upspeed = '自动上行速度';
var ip_qos_manu_upspeed = '手动上行速度(Kbps):';
var ip_qos_auto_downspeed = '自动下行速度';
var ip_qos_manu_downspeed = '手动下行速度(Kbps):';
var ip_qos_rule_set = 'QoS规则设定:';
var ip_qos_addrtype = '地址类型:';
var ip_qos_addrtype_ip = 'IP';
var ip_qos_addrtype_mac = 'MAC';
var ip_qos_local_ipaddr = '本地IP地址:';
var ip_qos_proto = '协议:';
var ip_qos_proto_udp = 'udp';
var ip_qos_proto_tcp = 'tcp';
var ip_qos_proto_both = 'both';
var ip_qos_local_port = '本地端口:(1~65535)';
var ip_qos_macaddr = 'MAC地址:';
var ip_qos_mode = '模式:';
var ip_qos_weight = '权重';
var ip_qos_upband = '上行带宽(Kbps):';
var ip_qos_downband = '下行带宽(Kbps):';
var ip_qos_apply = '设置';
var ip_qos_reset = '重置';
var ip_qos_curr_qos = '当前QoS规则表:';
var ip_qos_delete_select_btn = '删除所选';
var ip_qos_delete_all_btn = '删除所有';

var ip_qos_upspeed_empty = '自动上行速度关闭时，手动上行速度不能为空且不能比100小.';
var ip_qos_downspeed_empty = '自动下行速度关闭时，手动下行速度不能为空且不能比100小.';
var ip_qos_ip_invalid = '无效的IP地址';
var ip_qos_startip_invalid = '无效的起始IP地址,IP地址必须在当前子网的范围内.';
var ip_qos_portrange_invalid = '无效的端口范围,端口的范围为1-65535.';
var ip_qos_macaddr_notcomplete = '输入的MAC地址不完整,MAC地址必须为12个16进制长度';
var ip_qos_macaddr_invalid = '无效的MAC地址,MAC地址由16进制数组成(0-9或a-f)且不能为全零.';
var ip_qos_band_empty = '上行带宽或下行带宽不能为0或为空.';
var ip_qos_band_invalid = '无效的输入!必须为0-9的十进制数.';
var ip_qos_band_notint = '无效的输入! 必须为整数。';
var ip_qos_weight_empty = '权重值不能为空.';
var ip_qos_weight_invalid = '无效的权重范围!权重的范围为1-20的整数.';
var ip_qos_delete_select = '确定删除所选表项?';
var ip_qos_delete_all = '确定删除所有表项?';

var ip_qos_tbl_localaddr = '本地IP地址';
var ip_qos_tbl_macaddr = 'MAC地址';
var ip_qos_tbl_mode = '模式';
var ip_qos_tbl_valid ='有效';
var ip_qos_tbl_upband = '上行带宽';
var ip_qos_tbl_downband = '下行带宽';
var ip_qos_tbl_select = '选择';
var ip_qos_restrict_maxband = "限制最大带宽";
var ip_qos_quarant_minband = "保护最小带宽";

/***********	wlbasic.htm	************/
var wlbasic_header='无线基本参数设置';
var wlbasic_explain = '本页面可以配置连接到接入点的无线客户端参数。你可以设置无线网络参数与加密参数';
var wlbasic_network_type = '网络类型:';
var wlbasic_ssid = '网络服务标识:';
var wlbasic_disabled = '禁用无线网络接口';
var wlbasic_country = '国家:';
var wlbasic_band= '频段:';
var wlbasic_infrastructure = "基础模式";
var wlbasic_adhoc = "自组网模式";
var wlbasic_addprofile = '增加至配置';
var wlbasic_channelwidth = '信道带宽:';
var wlbasic_ctlsideband = '控制边带:';
var wlbasic_ctlsideautomode = '自动';
var wlbasic_ctlsidelowermode = '低';
var wlbasic_ctlsideuppermode = '高';
var wlbasic_chnnelnum = '信道编号:';
var wlbasic_broadcastssid= '广播网络服务标识';
var wlbasic_brossid_enabled = '启用';
var wlbasic_brossid_disabled = '禁用';
var wlbasic_wmm ='无线多媒体:';
var wlbasic_wmm_disabled = 	'禁用';
var wlbasic_wmm_enabled = 	'启用';
var wlbasic_data_rate = '速率:';
var wlbasic_data_rate_auto = "自动";
var wlbasic_associated_clients = '已连接的客户端:';
var wlbasic_show_associated_clients = '显示活跃的客户端';
var wlbasic_enable_mac_clone = '启用MAC地址克隆 (单一以太网客户端)';
var wlbasic_enable_repeater_mode = '启用通用中继模式(同时作为接入点和客户端)';
var wlbasic_extended_ssid = '扩展接口的网络服务标识:';
var wlbasic_ssid_note = '注意:如果需要更改模式和网络服务标识设置，请务必首先在快速配置页面禁用快速配置';
var wlbasic_enable_wl_profile = '启用无线配置';
var wlbasic_wl_profile_list = '无线配置清单:';
var wlbasic_apply = '设置';
var wlbasic_reset = '重置';
var wlbasic_delete_select = '删除所选';
var wlbasic_delete_all = '删除全部';
var wlbasic_enable_wire = '确定增加至无线配置文件?';
var wlbasic_asloenable_wire = '确定增加至无线配置文件?';

var wlbasic_mode = '模式:';
var wlbasic_client = '客户端';


/***********	wlstatbl.htm	************/
var wlstatbl_tbl_name = '活跃的无线客户端列表';
var wlstatbl_explain = '列表显示每个已连接无线客户端的MAC地址、传输与接收分组计数值、加密状态。';
var wlstatbl_mac = 'MAC 地址';
var wlstatbl_mode = '模式';
var wlstatbl_tx = '传输分组数';
var wlstatbl_rx = '接收分组数';
var wlstatbl_tx_rate ='传输速率 (Mbps)';
var wlstatbl_ps = '节能';
var wlstatbl_expired_time = '过期时间(s)';
var wlstatbl_refresh = '刷新';
var wlstatbl_close = '关闭';

/***********	wladvanced.htm	************/
var wladv_vallid_num_alert = '无效数字，必须是0-9的十进制数';
var wladv_fragment_thre_alert = '无效分段阈值，输入值必须是256-2346的十进制数';
var wladv_rts_thre_alert = '无效RTS阈值，输入值必须是0-2347的十进制数';
var wladv_beacon_alert = '无效信标间隔，输入值必须是20-1024的十进制数';
var wladv_header = '无线高级设置';
var wladv_explain = ' 高级设置适用于对无线局域网有充分认识的高级用户。除非了解设置的更改对接入点的作用，请不要更改设置';
var wladv_frg_thre = '分段阈值:';
var wladv_rts_thre = 'RTS 阈值:';
var wladv_beacon_interval = '信标间隔:';
var wladv_preamble_type = '报文类型:';
var wladv_preamble_long = '长报文';
var wladv_preamble_short = '短报文';
var wladv_iapp = '接入点内部协议:';
var wladv_iapp_enabled = '启用';
var wladv_iapp_disabled = '禁用';
var wladv_protection = '保护机制:';
var wladv_protection_enabled = '启用';
var wladv_protection_disabled = '禁用';
var wladv_aggregation = '帧聚合:';
var wladv_aggregation_enabled = '启用';
var wladv_aggregation_disabled = '禁用';
var wladv_short_gi = '短防护时间间隔:';
var wladv_short_gi_enabled = '启用';
var wladv_short_gi_disabled = '禁用';
var wladv_wlan_partition = '无线局域网划分:';
var wladv_wlan_partition_enabled = '启用';
var wladv_wlan_partition_disabled = '禁用';
var wladv_stbc = '空时分组编码:';
var wladv_stbc_enabled = '启用';
var wladv_stbc_disabled = '禁用';
var wladv_coexist = '20/40MHz 共存:';
var wladv_coexist_enabled = '启用';
var wladv_coexist_disabled = '禁用';
var wladv_tx_beamform = '传输波束形成:';
var wladv_tx_beamform_enabled = '启用';
var wladv_tx_beamform_disabled = '禁用';
var wladv_rf_power = '射频输出功率:';
var wladv_apply = '设置';
var wladv_reset = ' 重置 ';

/***********	wlsecutity.htm wlsecutity_all.htm	************/
var wlsec_validate_note = "注意:如果你已选择了[企业(身份认证服务器)]且更改了这里，\n 无线局域网接口及它的虚拟接口将使用目前身份认证服务器的设置，你想继续吗? ";
var wlsec_header = '无线安全设置';
var wlsec_explain = "本页面可以设置无线安全参数。使用加密密钥使能WEP或者WPA可以阻止任何未经授权的用户连接到无线网络";
var wlsec_select_ssid = '选择网络服务标识:';
var wlsec_psk= 'PSK';
var wlsec_pre_shared = '预共享';
var wlsec_tkip = 'TKIP';
var wlsec_aes = 'AES';
var wlsec_apply = '设置';
var wlsec_reset = '重置';
var wlsec_inside_type_alert = "该内部类型未被支持.";
var wlsec_eap_alert = '该EAP类型未被支持.';
var wlsec_wapi_remote_ca_install_alert = '请确定从远程身份认证服务器获取的WAPI证书已经安装在[WAPI] ->[证书安装]页面.'; 
var wlsec_wapi_local_ca_install_alert = '请确定从本地身份认证服务器获取的WAPI证书已经安装在[WAPI] ->[证书安装]页面.'; 
var wlsec_wapi_wrong_select_alert = "wapi证书索引选择错误.";
var wlsec_wapi_key_length_alert = "wapi密钥最少8个字符且不多于32个字符";
var wlsec_wapi_key_hex_alert = "十六进制的WAPI密钥必须是64个十六进制数.";
var wlsec_wapi_invalid_key_alert = "无效密钥值.必须是十六进制数(0-9 or a-f).";
var wlsec_wep_confirm = "如果WEP使能，WPS2.0将被禁用.";
var wlsec_wpa_confirm = "在只使能WPA或TKIP情况下，WPS2 的守护进程将被禁用";
var wlsec_wpa2_empty_alert = "WPA2 密钥套件不能为空.";
var wlsec_wpa_empty_alert = "WPA2 密钥套件不能为空.";
var wlsec_tkip_confirm = "只使能TKIP情况下，WPS2 的守护进程将被禁用";
var wlsec_encryption =  '加密方式:';
var wlsec_disabled = '禁用';
var wlsec_wpa_mix = 'WPA-混合';
var wlsec_802_1x = '802.1x 认证:';
var wlsec_auth = '认证:';
var wlsec_auth_open_sys = '开放系统';
var wlsec_auth_shared_key = '共享密钥';
var wlsec_auth_auto = '自动';
var wlsec_key_length = '密钥长度:';
var wlsec_key_hex = '十六进制';
var wlsec_key_ascii = 'ASCII';
var wlsec_encryption_key = '加密密钥:';
var wlsec_auth_mode = '认证模式:';
var wlsec_auth_enterprise_mode = '企业 (RADIUS)';
var wlsec_auth_enterprise_ap_mode = '企业 (身份认证服务器)';
var wlsec_auth_personal_mode = '个人 (预共享密钥)';
var wlsec_wpa_suite = 'WPA 密钥套件:';
var wlsec_wpa2_suite = 'WPA2 密钥套件:';
var wlsec_wep_key_format = '密钥格式:';
var wlsec_pre_key_format = '预共享密钥格式:';
var wlsec_pre_key = '预共享密钥:';
var wlsec_passpharse = '口令';
var wlsec_key_hex64 = '十六进制 (64 字符)';
var wlsec_key_64bit = '64 比特';
var wlsec_key_128bit = '128 比特';
var wlsec_radius_server_ip = "RADIUS&nbsp;服务器&nbsp;IP&nbsp;地址:";
var wlsec_radius_server_port = 'RADIUS&nbsp;服务器&nbsp;端口:';
var wlsec_radius_server_password = 'RADIUS&nbsp;服务器&nbsp;密码:';
var wlsec_eap_type = 'EAP 类型:';
var wlsec_inside_tunnel_type = '内部隧道类型:';
var wlsec_eap_user_id = 'EAP 用户标识:';
var wlsec_radius_user = 'RADIUS 用户名称:';
var wlsec_radius_user_password = 'RADIUS 用户密码:';
var wlsec_user_key_password = '用户密钥密码 (if any):';
var wlsec_use_local_as = '使用本地身份认证服务器:';
var wlsec_as_ip = '身份认证服务器&nbsp;IP&nbsp;地址:';
var wlsec_select_wapi_ca = '选择WAPI证书:';
var wlsec_use_ca_from_as = '使用从远程认证服务器获取的证书';
var wlsec_adhoc_wep = '在N频段或AC频段下，Adhoc网络不支持WEP加密!';

/***********	tcpipwan.htm  tcpiplan.htm************/
var tcpip_check_ip_msg = '无效的IP地址';
var tcpip_check_server_ip_msg = '无效的服务器IP地址';
var tcpip_check_dns_ip_msg1 = '无效的DNS1地址';
var tcpip_check_dns_ip_msg2 = '无效的DNS2地址';
var tcpip_check_dns_ip_msg3 = '无效的DNS3地址';
var tcpip_check_size_msg = "无效的MTU长度，必须设置为";
var tcpip_check_user_name_msg = "用户名不能为空!";
var tcpip_check_password_msg = "密码不能为空";
var tcpip_check_invalid_time_msg = "无效空闲时间值，必须设置为";
var tcpip_pppoecontype_alert = "错误的PPPOE连接类型";
var tcpip_pptpontype_alert = "错误的PPTP连接类型";
var tcpip_l2tpcontype_alert = "错误的L2TP连接类型";
var tcpip_pppcontype_alert = "错误的PPP连接类型";
var tcpip_browser_alert = '错误!您的浏览器必须支持CSS!';
var tcpip_wan_header = '广域网络接口设置';
var tcpip_wan_explain = '该页面可以设置因特网的网络参数，因特网是通过接入点的WAN接口连接的。通过改变WAN接入类型可以改变为静态IP、DHCP、PPPOE、PPTP或者L2TP等不同的接入方式。'; 

var tcpip_wan_auto_dns = '自动获取DNS';
var tcpip_wan_manually_dns =  '手动设置DNS';
var tcpip_wan_conn_time = '&nbsp;(1-1000 分钟)';
var tcpip_wan_max_mtu_size = '字节';
var tcpip_wan_conn = '连接';
var tcpip_wan_disconn = '断开连接';
var tcpip_wan_continuous = '连续';
var tcpip_wan_on_demand = '需要时连接';
var tcpip_wan_manual = '手动';
var tcpip_wan_access_type = 'WAN 接入类型:';
var tcpip_wan_type_static_ip = '静态 IP';
var tcpip_wan_type_client = "DHCP 客户端";
var tcpip_wan_type_henan = "PPPoE+(河南宽带我世界)";
var tcpip_wan_type_nanchang = "动态PPPoE(江西南昌星空极速)";
var tcpip_wan_type_other1 = "PPPoE_other1(湖南、湖北等地区星空极速)";
var tcpip_wan_type_other2 = "PPPoE_other2(湖南、湖北等地区星空极速)";
var tcpip_wan_type_dhcp_plus = "DHCP+(河南地区)";
var tcpip_wan_ip = "IP 地址:";
var tcpip_wan_mask = '子网掩码:';
var tcpip_wan_default_gateway = '默认网关:';
var tcpip_wan_mtu_size = 'MTU 长度:';
var tcpip_wan_host = '主机名称:';
var tcpip_wan_user = '用户名称:';
var tcpip_wan_password = '密码:';
var tcpip_wan_server_ac = '服务器名称(AC):';
var tcpip_wan_conn_type = '连接时间:';
var tcpip_wan_idle_time = '空闲时间:';
var tcpip_wan_server_ip = '服务器IP地址:';
var tcpip_wan_clone_mac = '克隆MAC地址:';
var tcpip_wan_enable_upnp = '&nbsp;&nbsp;启用 uPNP';
var tcpip_wan_enable_igmp_proxy = '&nbsp;&nbsp;启用IGMP代理';
var tcpip_wan_enable_ping = '&nbsp;&nbsp;在WAN上启用Ping';
var tcpip_wan_enable_webserver = '&nbsp;&nbsp;在WAN上启用Web服务器';
var tcpip_wan_enable_ipsec = '&nbsp;&nbsp;VPN连接使能IPsec通过';
var tcpip_wan_enable_pptp = '&nbsp;&nbsp;VPN连接使能PPTP通过';
var tcpip_wan_enable_l2tp = '&nbsp;&nbsp;VPN连接使能L2TP通过';
var tcpip_wan_enable_ipv6 = '&nbsp;&nbsp;VPN连接使能IPV6通过';
var tcpip_wan_enable_netsniper = '&nbsp;&nbsp;启用防网络尖兵';
var tcpip_wan_dynamic_ip = '动态IP (DHCP)';
var tcpip_wan_static_ip = '静态IP';
var tcpip_wan_attain_by_DN = '通过域名访问服务器';
var tcpip_wan_attain_by_Ip = '通过IP访问服务器';
var tcpip_apply = '设置';
var tcpip_reset = '重置';
var tcpip_lan_wrong_dhcp_field = "错误的dhcp域!";
var tcpip_lan_start_ip = '无效的DHCP客户端起始地址';
var tcpip_lan_end_ip = '无效的DHCP客户端结束地址';
var tcpip_lan_ip_alert = '\n必须是在子网内的地址。';
var tcpip_lan_invalid_rang = '无效的客户端地址范围\n结束地址必须大于起始地址。';
var tcpip_lan_invalid_rang_value = "无效值. 必须在 (1 ~ 10080)范围内.";
var tcpip_lan_invalid_dhcp_type = "载入错误的dhcp类型!";
var tcpip_lan_header = '局域网接口设置';
var tcpip_lan_explain = '该页面可以配置连接到接入点LAN端口的局域网网络参数，配置IP地址，子网掩码，DHCP等参数';
var tcpip_lan_ip = "IP地址:";
var tcpip_lan_mask = '子网掩码:';
var tcpip_lan_default_gateway = '默认网关:';
var tcpip_lan_dhcp = 'DHCP:';
var tcpip_lan_dhcp_disabled = '禁用';
var tcpip_lan_dhcp_client = '客户端';
var tcpip_lan_dhcp_server = '服务器';
var tcpip_lan_dhcp_auto = '自动';
var tcpip_lan_dhcp_rang = 'DHCP 客户端范围:';
var tcpip_lan_dhcp_time = 'DHCP 租约时间:';
var tcpip_minutes = '分钟';
var tcpip_lan_staicdhcp = '静态 DHCP:';
var tcpip_staticdhcp = "设置 DHCP";
var tcpip_domain = "域名名称:";
var tcpip_netbios = "NetBIOS 名称:";
var tcpip_802_1d = "802.1d 生成树:";
var tcpip_802_1d_enable = '启用';
var tcpip_802_1d_disabled = '禁用';
var tcpip_show_client = '显示客户端';
var tcpip_l2tp_server_domain_name = '非法的服务器域名！正确的域名由字母或数字组成且不含有空格';
/***********	dhcptbl.htm ************/
var dhcp_header = '活跃的DHCP客户端列表';
var dhcp_explain = '该表显示了每个已租约的DHCP客户的分配的IP地址、MAC地址、租约时间期限。';
var dhcp_ip = 'IP 地址';
var dhcp_mac = 'MAC 地址';
var dhcp_time = '期限时间(s)';
var dhcp_refresh = '刷新';
var dhcp_close = '关闭';
/***********	util_gw.js ************/
var util_gw_wps_warn1 = 'WPS已经配置了网络服务标识。设置的任何改变';
var util_gw_wps_warn2 = 'WPS已经配置了接入点模式。设置的任何改变';
var util_gw_wps_warn3 = 'WPS已经配置了安全设置。设置的任何改变';
var util_gw_wps_warn4 = 'WPS不支持WPA企业认证。';
var util_gw_wps_warn5 = 'WPS不支持802.1x认证。';
var util_gw_wps_warn6 = 'WPS不支持无线分布式系统模式。';
var util_gw_wps_warn7 = 'WPS不支持自组织网络模式。 ';
var util_gw_wps_cause_disconn = '会导致工作站断开连接。';
var util_gw_wps_want_to = '确定继续更改新的设置?';
var util_gw_wps_cause_disabled = '使用该配置将会导致禁用。 ';
var util_gw_wps_ecrypt_11n = '无效的加密模式! WPA or WPA2, AES密码套件 必须使用802.11n的频段。';
var util_gw_wps_ecrypt_basic = '加密模式不适用于802.11n的频段。 请更改无线局域网络的加密设置，否则工作将会不正确。';
var util_gw_wps_ecrypt_confirm = '确定继续把该加密模式应用在802.11n的频段上? 用户使用无线局域网络不会得到良好性能!';
var util_gw_ssid_hidden_alert = "如果开启隐藏网络服务标识; WPS2.0将会被禁用。";
var util_gw_ssid_empty = '网络服务标识不能为空。';
var util_gw_preshared_key_length_alert =  '预共享密钥必须是64个字符。';
var util_gw_preshared_key_alert = "预共享密钥值无效。必须是十六进制数(0-9 or a-f)。";
var util_gw_preshared_key_min_alert = '预共享密钥值设置值最少设置为8个字符。';
var util_gw_preshared_key_max_alert = '预共享密钥值设置不能超过64个字符。';
var util_gw_decimal_rang = '必须是1-65535的十进制数。';
var util_gw_invalid_radius_port = 'RADIUS服务器端口值无效! ';
var util_gw_empty_radius_port = "RADIUS服务器端口值不能为空! ";
var util_gw_invalid_radius_ip = 'RADIUS服务器IP地址无效';
var util_gw_mask_empty = '子网掩码不能为空! ';
var util_gw_ip_format = '必须填充为4个数字，如 xxx.xxx.xxx.xxx.';
var util_gw_mask_rang = '\n必须是这些数字: 0, 128, 192, 224, 240, 248, 252 or 254';
var util_gw_mask_rang1 = '\n必须是这些数字: 128, 192, 224, 240, 248, 252 or 254';
var util_gw_mask_invalid1 = '子网掩码的第一个数字无效。';
var util_gw_mask_invalid2 = '子网掩码的第二个数字无效。';
var util_gw_mask_invalid3 = '子网掩码的第三个数字无效。';
var util_gw_mask_invalid4 = '子网掩码的第四个数字无效。';
var util_gw_mask_invalid = '子网掩码值无效。';
var util_gw_decimal_value_rang = "必须是十进制数 (0-9).";
var util_gw_invalid_degw_ip = '默认网关地址无效';
var util_gw_invalid_gw_ip = '网关地址无效!';
var util_gw_locat_subnet = '\n必须是同一子网内的当前IP地址。';
var util_gw_mac_complete = '输入的MAC地址不完全。 ';
var util_gw_mac_empty = '输入的MAC地址为空！ ';
var util_gw_mac_zero = '输入的MAC地址不能为全0! ';
var util_gw_mac_ff = '输入的MAC地址不能为全F! ';
var util_gw_12hex = '必须是12个十六进制数。';
var util_gw_invalid_mac = 'MAC地址无效。 ';
var util_gw_hex_rang = '必须是十六进制数(0-9 or a-f).';
var util_gw_ip_empty = 'IP地址不能为空! ';
var util_gw_invalid_value = ' 值。 ';
var util_gw_should_be = '必须是 ';
var util_gw_check_ppp_rang1 = ' 值域的第一个数字。';
var util_gw_check_ppp_rang2 = ' 值域的第二个数字。';
var util_gw_check_ppp_rang3 = ' 值域的第三个数字。';
var util_gw_check_ppp_rang4 = ' 值域的第四个数字。';
var util_gw_check_ppp_rang5 = ' 值域的第五个数字。';
var util_gw_invalid_key_length = '密钥长度值无效 ';
var util_gw_char = ' 字符。';
var util_gw_invalid_wep_key_value = 'WEP密钥长度值无效。';
var util_gw_invalid_key_value = '密钥值无效。 ';
var util_gw_invalid_ip = '无效的IP地址。';
var util_gw_ipaddr_empty = 'IP地址不能为空! IP地址是xxx.xxx.xxx.xxx形式的4位数.';
var util_gw_ipaddr_nodecimal = '值,必须为0-9的十进制数.';
var util_gw_ipaddr_1strange = '域在第1位,必须为0-255.';
var util_gw_ipaddr_2ndrange = '域在第2位,必须为0-255.';
var util_gw_ipaddr_3rdrange = '域在第3位,必须为0-255.';
var util_gw_ipaddr_4thrange = '域在第4位,必须为1-254.';

var util_gw_array0 = "(GMT-12:00)埃尼威托克岛, 夸贾林环礁";
var util_gw_array1 = "(GMT-11:00)中途岛, 萨摩亚";
var util_gw_array2 = "(GMT-10:00)夏威夷";
var util_gw_array3 = "(GMT-09:00)阿拉斯加州";
var util_gw_array4 = "(GMT-08:00)太平洋时间(美国和加拿大); 提华纳";
var util_gw_array5 = "(GMT-07:00)亚利桑那州";
var util_gw_array6 = "(GMT-07:00)山地时区(美国和加拿大)";
var util_gw_array7 = "(GMT-06:00)中央时间(美国&加拿大)";
var util_gw_array8 = "(GMT-06:00)墨西哥城, 特古西加尔巴";
var util_gw_array9 = "(GMT-06:00)萨斯喀彻温省";
var util_gw_array10 = "(GMT-05:00)波哥大, 利马, 基多";
var util_gw_array11 = "(GMT-05:00)东部时间(美国&加拿大)";
var util_gw_array12 = "(GMT-05:00)印第安纳州(东)";
var util_gw_array13 = "(GMT-04:00)大西洋时间(加拿大)";
var util_gw_array14 = "(GMT-04:00)加拉加斯,拉巴斯";
var util_gw_array15 = "(GMT-04:00)圣地亚哥";
var util_gw_array16 = "(GMT-03:30)纽芬兰";
var util_gw_array17 = "(GMT-03:00)巴西利亚";
var util_gw_array18 = "(GMT-03:00)布宜诺斯艾利斯,乔治城";
var util_gw_array19 = "(GMT-02:00)大西洋中部";
var util_gw_array20 = "(GMT-01:00)亚速尔群岛,佛得角.";
var util_gw_array21 = "(GMT)卡萨布兰卡,蒙罗维亚";
var util_gw_array22 = "(GMT)格林威治标准时间:都柏林爱丁堡、里斯本、伦敦";
var util_gw_array23 = "(GMT+01:00)阿姆斯特丹、柏林、伯尔尼、罗马、斯德哥尔摩、维也纳";
var util_gw_array24 = "(GMT+01:00)贝尔格莱德,布拉迪斯拉发、布达佩斯、卢布尔雅那,布拉格";
var util_gw_array25 = "(GMT+01:00)巴萨,皇马";
var util_gw_array26 = "(GMT+01:00)布鲁塞尔,哥本哈根,马德里,巴黎,维尔纽斯";
var util_gw_array27 = "(GMT+01:00)巴黎";
var util_gw_array28 = "(GMT+01:00)萨拉热窝,斯科普里,索非亚,华沙,萨格勒布";
var util_gw_array29 = "(GMT+02:00)雅典,伊斯坦布尔,明斯克";
var util_gw_array30 = "(GMT+02:00)布加勒斯特";
var util_gw_array31 = "(GMT+02:00)开罗";
var util_gw_array32 = "(GMT+02:00)哈拉雷,比勒陀利亚";
var util_gw_array33 = "(GMT+02:00)赫尔辛基,包括里加、塔林";
var util_gw_array34 = "(GMT+02:00)耶路撒冷";
var util_gw_array35 = "(GMT+03:00)巴格达,科威特,利雅得";
var util_gw_array36 = "(GMT+03:00)莫斯科、圣彼得堡、伏尔加格勒";
var util_gw_array37 = "(GMT+03:00)内罗比";
var util_gw_array38 = "(GMT+03:30)德黑兰";
var util_gw_array39 = "(GMT+04:00)阿布扎比,马斯喀特";
var util_gw_array40 = "(GMT+04:00)巴库第比利斯";
var util_gw_array41 = "(GMT+04:30)喀布尔";
var util_gw_array42 = "(GMT+05:00)叶卡捷琳堡";
var util_gw_array43 = "(GMT+05:00)伊斯兰堡,巴基斯坦卡拉奇,塔什干";
var util_gw_array44 = "(GMT+05:30)孟买、加尔各答、马德拉斯,新德里";
var util_gw_array45 = "(GMT+06:00)阿斯塔纳、阿拉木图、达卡";
var util_gw_array46 = "(GMT+06:00)科伦坡";
var util_gw_array47 = "(GMT+07:00)曼谷、河内、雅加达";
var util_gw_array48 = "(GMT+08:00)北京、重庆、香港、乌鲁木齐";
var util_gw_array49 = "(GMT+08:00)珀斯";
var util_gw_array50 = "(GMT+08:00)新加坡";
var util_gw_array51 = "(GMT+08:00)台北";
var util_gw_array52 = "(GMT+09:00)大阪,札幌,东京";
var util_gw_array53 = "(GMT+09:00)首尔";
var util_gw_array54 = "(GMT+09:00)雅库茨克";
var util_gw_array55 = "(GMT+09:30)阿德莱德";
var util_gw_array56 = "(GMT+09:30)达尔文";
var util_gw_array57 = "(GMT+10:00)布里斯班";
var util_gw_array58 = "(GMT+10:00)堪培拉、墨尔本、悉尼";
var util_gw_array59 = "(GMT+10:00)关岛,莫尔兹比港";
var util_gw_array60 = "(GMT+10:00)霍巴特";
var util_gw_array61 = "(GMT+10:00)海参崴";
var util_gw_array62 = "(GMT+11:00)马加丹州,所罗门群岛。,新喀里多尼亚";
var util_gw_array63 = "(GMT+12:00)奥克兰, 惠灵顿";
var util_gw_array64 = "(GMT+12:00)斐济, 堪察加半岛, 马绍尔群岛.";

var util_gw_chanauto = '自动';
var uyi_gw_chan_dfsauto = '自动(DFS)';
var util_gw_bcast_mcast_mac= '不能设置广播或多播MAC地址';

/***********	status.htm ************/
var status_ip = 'IP 地址';
var status_mac = 'MAC 地址';
var status_subnet_mask = '子网掩码';
var status_default_gw = '默认网关';
var status_attain_ip = '获取IP协议';
var status_ipv6_global_ip = '全局地址';
var status_ipv6_ll = '链路本地地址';
var status_ipv6_link = '链路类型';
var status_ipv6_conn = '连接类型';
var status_header = '接入点状态';
var status_explain = ' 该页面显示了目前状态和设备的一些基本设置。';
var status_wan_config = '广域网络配置';
var status_ipv6_lan = '局域网络的IPV6配置';
var status_ipv6_wan = '广域网络的IPV6配置';
var status_sys = '系统';
var status_uptime = '更新时间';
var status_fw_ver = '固件版本';
var status_build_time = '构建时间';
var status_wl = '无线';
var status_config = '配置';
var status_client_mode_inf = "基础类型客户端";
var status_client_mode_adhoc = "自组网客户";
var status_ap = '接入点';
var status_wds = '无线分布式系统';
var status_mesh = '网状';
var status_mpp = '网状入口节点';
var status_map = '网状接入点';
var status_mp = '网状节点';
var status_band = '频段';
var status_ssid = '网络服务标识';
var status_channel_num = '信道编号';
var status_encrypt = '加密方式';
var status_bssid = '基本网络服务标识';
var status_assoc_cli = '已连接客户端';
var status_state = '状态';
var status_vir_ap  = '虚拟接入点';
var status_repater_config = " 中继接口配置";
var status_tcpip_config = 'TCP/IP 配置';
var status_dhcp_server = 'DHCP 服务器';
var status_disabled = '禁用';
var status_enabled = '启用';
var status_auto = '自动';
var status_unknown = '未知';
var status_dhcp_get_ip = '从DHCP服务器获取IP...';
var status_conn = '已连接';
var status_disconn = '已断开连接';
var status_fixed = '固定IP';
var status_start = '已启动';
var status_idle = '空闲';
var status_wait_key = '等待密钥';
var status_scan = '扫描';
var status_mode = '模式';

/***********	stats.htm ************/
var stats_header = '统计';
var stats_explain = '该页面显示了以太网与无线网络的传输与接收封包计数';
var stats_lan = '局域网';
var stats_send = "已发送封包数";
var stats_recv = "已接收封包数";
var stats_repeater = '中继';
var stats_eth = '以太';
var stats_wan = '广域网';
var stats_refresh = '刷新';




/***********	wlwdstbl.htm ************/
var wlwdstbl_header = '无线分布式系统接入点列表';
var wlwdstbl_wlan = "无线局域网";
var wlwdstbl_explain = '该表显示了每个已配置无线分布式系统接入点的MAC地址、传输与接收封包计数及状态信息。';
var wlwdstbl_mac = 'MAC地址';
var wlwdstbl_tx_pkt = '传输封包数';
var wlwdstbl_tx_err = '传输错误数';
var wlwdstbl_tx_rate = '传输速率(Mbps)';
var wlwdstbl_rx_pkt = '接收封包数';
var wlwdstbl_refresh = '刷新';
var wlwdstbl_close = '关闭';
/***********	wlwdsenp.htm ************/
var wlwdsenp_hex = '十六进制';
var wlwdsenp_char = '字符';
var wlwdsenp_header = '无线分布式系统安全设置';
var wlwdsenp_wlan = '无线局域网';
var wlwdsenp_explain = '该页面可以配置无线分布系统的安全设置。当安全设置使能时，必须确保无线分布系统内的设备已经采用同样的加密算法和密钥';
var wlwdsenp_wep_key_format = 'WEP密钥格式:';
var wlwdsenp_encrypt = '加密方式:';
var wlwdsenp_wep_key = 'WEP密钥:';
var wlwdsenp_prekey_format = '预共享密钥格式:';
var wlwdsenp_prekey = '预共享密钥:';
var wlwdsenp_none = '无';
var wlwdsenp_pass = '口令';
var wlwdsenp_bits = '比特';
var wlwdsenp_apply = "设置";
var wlwdsenp_reset = '重置';
/***********	tcpip_staticdhcp.htm ************/
var tcpip_dhcp_del_select = '确定删除已选择的表项?';
var tcpip_dhcp_del_all = '确定删除所有表项?';
var tcpip_dhcp_header = '静态DHCP设置';
var tcpip_dhcp_explain = '该页面可以预留IP地址，允许一个拥有特殊MAC地址的网络设备在任何需要得一个IP地址的时候，都可得到同样的IP地址。这相当于一个设备拥有一个静态IP地址，除非设备必须需要从DHCP服务器获取IP地址。';
var tcpip_dhcp_st_enabled = '启用静态DHCP';
var tcpip_dhcp_comment = '描述';
var tcpip_dhcp_list = '静态DHCP列表:';
var tcpip_dhcp_apply = '设置';
var tcpip_dhcp_reset = '重置';
var tcpip_dhcp_delsel = '删除已选';
var tcpip_dhcp_delall = '删除所有';
var tcpip_dhcp_select = '选择';

/***********	wizard.htm ************/
var wizard_header = '设置安装向导';
var wizard_header_explain = '安装向导将指导您配置为AP,请按照安装向导指示一步一步执行.';
var wizard_welcome = '欢迎来到安装向导.';
var wizard_content_explain = '该向导将引导您通过下列步骤。首先点击下一个.';
var wizard_content1 = '模式设置';
var wizard_content2 = '选择你的时区';
var wizard_content3 = '设置LAN接口';
var wizard_content4 = '设置WAN时区';
var wizard_content5 = '选择无线频率';
var wizard_content6 = '无线局域网设置';
var wizard_content7 = '无线安全性设置';
var wizard_next_btn = '  前进>>';
var wizard_back_btn = '<<后退  ';
var wizard_cancel_btn = '  取消  ';
var wizard_finish_btn = '完成';

var wizard_opmode_invalid = '无效的模式值 ';
var wizard_chanset_wrong = '错误域输入!';
var wizard_wantypeselect = '错误! 你的浏览器必须支持CSS !';
var wizard_weplen_error = '无效的WEP密匙长度';

var wizard_content5_explain = '你可以选择无线频率';
var wizard_wire_band = '无线频率:';

var wizard_basic_header_explain = ' 本页面可以配置连接到接入点的无线客户端参数. ';
var wizard_wlan1_div0_mode = '模式:';
var wizard_chan_auto = '自动';
var wizard_client = '客户端';


var wizard_wpa_tkip = 'WPA (TKIP)';
var wizard_wpa_aes = 'WPA (AES)';
var wizard_wpa2_aes = 'WPA2(AES)';
var wizard_wpa2_mixed = 'WPA2混合';
var wizard_use_cert_from_remote_as0 = ' 用户证书来自远端AS0 ';
var wizard_use_cert_from_remote_as1 = ' 用户证书来自远端AS1 ';

var wizard_5G_basic = '无线5GHz基本设置';
var wizard_5G_sec = '无线5GHz安全设置';
var wizard_2G_basic = '无线2.4GHz基本设置';
var wizard_2G_sec = '无线2.4G安全设置';

/***********	route.htm ************/
var route_header = '路由设置';
var route_header_explain = '这个界面用来设置动态路由协议或设置静态路由表.';
var route_enable = '启用';
var route_disable = '禁用';

var route_apply = '设置';
var route_reset = '重置';
var route_dynamic = '启用动态路由';
var route_nat = 'NAT:';
var route_rip = 'RIP:';
var route_rip1 = 'RIPv1' ;
var route_rip2 = 'RIPv2';
var route_rip6 = 'RIPng';
var route_static = '启用静态路由';
var route_ipaddr = 'IP地址:';
var route_mask = '子网掩码:';
var route_gateway = '网关:';
var route_metric = '跳数:';
var route_interface = '接口:';
var route_lan = '无线局域网';
var route_wan = '无线广域网';

var route_static_tbl = '静态路由表:';
var route_tbl_destip = '目的IP地址';
var route_tbl_mask = '掩码 ';
var route_tbl_gateway = '网关 ';
var route_tbl_metric = '跳数 ';
var route_tbl_inter = '接口 ';
var route_tbl_select = '选择';
var route_deletechick_warn = '确定删除所选入口项?';
var route_deleteall_warn = '确定删除所有入口项?';
var route_deletechick = '删除所选';
var route_deleteall = '删除所有';
var route_showtbl = '显示路由表';

var route_addchick0 = '无效的IP地址 ';
var route_addchick1 = '无效的IP地址! ';
var route_addchick2 = '无效的网关地址! ';
var route_addchick3 = '无效的跳数! 跳数的范围为 1 ~ 15';
var route_checkip1 = 'IP地址不能为空! IP地址为如 xxx.xxx.xxx.xxx所示的4个十进制数.';
var route_checkip2 = '值,必须为十进制数 (0-9).';
var route_checkip3 = '范围在地址第1位,必须为1-223.';
var route_checkip4 = '范围在地址第2位,必须为0-255.';
var route_checkip5 = '范围在地址第3位,必须为0-255.';
var route_checkip6 = '范围在地址第4位,必须为0-255.';
var route_validnum = '无效的值,必须为十进制数(0-9).';
var route_setrip = '在PPP广域网模式下不能禁用NAT!';

/***********	routetbl.htm ************/
var routetbl_header = '路由表';
var routetbl_header_explain = ' 这个表显示所有的路由表项 .';
var routetbl_refresh = '刷新';
var routetbl_close = ' 关闭 ';
var routetbl_dst = '目的IP';
var routetbl_gw = '网关';
var routetbl_mask = '掩码';
var routetbl_flag = '标记';
var routetbl_iface = '接口';
var routetbl_type = '类型';

/***********	vlan.htm ************/
var vlan_header = '虚拟局域网设置';
var vlan_header_explain = '这个界面用来进行虚拟局域网配置,路由器通过创建虚拟局域网来提供细分服务,虚拟局域网用来解决可伸缩性、安全性和网络管理等问题.';
var vlan_apply = '设置';
var vlan_reset = '重置';

var vlan_enable = '启用虚拟局域网';
var vlan_id = '虚拟局域网 ID:';
var vlan_forwardrule = '转发规则:';
var vlan_forwardrulenat = '网络地址转换';
var vlan_forwardrulebridge = '桥接模式';
var vlan_tagtbl = 'Tag表';
var vlan_tagtbl_interface = '接口名';
var vlan_tagtbl_taged = 'taged';
var vlan_tagtbl_untaged = 'unTaged';
var vlan_tagtbl_notin = '不在这个虚拟局域网中';

var vlan_settbl = '当前无线局域网设置';
var vlan_settbl_id = '虚拟局域网ID';
var vlan_settbl_taged = 'taged接口';
var vlan_settbl_untaged = 'untaged接口';
var vlan_settbl_forwardrule = '转发规则';
var vlan_settbl_modify = '修改';
var vlan_settbl_select = '选择';
var vlan_deletechick = '删除所选';
var vlan_deleteall = '删除所有';

var vlan_nettbl = '当前网络接口设置';
var vlan_nettbl_name = '接口名';
var vlan_nettbl_pvid = '端口VLAN标识号';
var vlan_nettbl_defprio = '默认优先级';
var vlan_nettbl_defcfi = '默认Cfi';

var vlan_checkadd1 = '无效的虚拟局域网id,虚拟局域网的id的值必须在1~4094之间';
var vlan_checkadd2 = '这个虚拟局域网必须绑定至少1个接口!!';
var vlan_deletesel = '确定删除所选表项?';
var vlan_deleteall_conf = '确定删除所有表项?';

/***********	CONFIG_CUTE_MAHJONG ************/
var opmode_radio_upgradekit='Upgrade Kit:';
var opmode_radio_upgradekit_explain='In this mode all ethernet ports and wireless interfaces are bridged together and the NAT function is disabled.';
var opmode_radio_gw_explain_cmj='In this mode, the device connects to internet via an ADSL/Cable Modem. NAT is enabled and PCs on LAN ports share the same IP to the ISP via the WAN port. The connection type is DHCP client.';
var opmode_radio_wisp_explain_cmj='In this mode, all ethernet ports are bridged together and the wireless client will connect to the ISP’s Access Point. NAT is enabled and PCs on Ethernet ports share the same IP to the ISP via the wireless LAN. You can connect to the ISP’s AP on the Site-Survey page. The connection type is DHCP client.';
var opmode_radio_auto_explain_cmj='In this mode, the WAN detection function will automatically boot in Gateway or WISP mode.';

var wizard_cmj_content1 = 'Set Operating Mode';
var wizard_cmj_content2 = 'Set Wireless Network Name';
var wizard_cmj_content2_explain = 'Enter the Wireless Network Name of the AP.';
var wizard_cmj_content2_explain2 = 'Wireless Network Name (SSID):';
var wizard_cmj_content3 = 'Select the Wireless Security Mode';

var wizard_sitesurvey_content = 'Wireless Site Survey';
var wizard_sitesurvey_content_explain = 'This page provides a tool to scan for wireless networks. If an Access Point or IBSS is found, you can choose to connect to it manually when client mode is enabled.';

var switch_ui_mode2adv  = '切换至高级模式';
var switch_ui_mode2easy = '切换至精简模式';
/***********	OK_MSG&ERR_MSG ************/
var okmsg_explain = "<body><blockquote><h4>改变配置成功!</h4>你的改变已经保存.路由器必须重启以使改变生效.<br> 你可以立刻重启, 你也可以继续修改稍后再重启.\n";
var okmsg_reboot_now = '立刻重启';
var okmsg_reboot_later = '稍后重启';

var okmsg_btn = '  确定  ';

var okmsg_fw_saveconf = '重新载入配置成功!<br><br>路由器正在重启.<br>在这段时间内不要关闭或重启路由器.<br>';
var okmsg_fw_opmode = '设置操作模式成功!';
var okmsg_fw_passwd = '密码修改成功!<br><br>在这段时间内不要重启或关闭路由器.';

/*********** common msg ************/
var ip_should_in_current_subnet = '无效的IP地址,IP地址必须在当前子网的范围内.';
var ip_should_be_different_from_lanip = '无效的IP地址,IP地址不能与lan ip相同.';

function dw(str)
{
	document.write(str);
}
