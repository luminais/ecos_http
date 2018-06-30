
var language_sel='language:';
var language_sc='简体中文';
var language_en='English';
/***********	menu.htm	************/
var menu_site ='Site contents';
var menu_wizard ='Setup Wizard';
var menu_opmode ='Operating Mode';
var menu_wireless='Wireless';
var menu_basic='Basic Settings';
var menu_advance='Advanced Settings';
var menu_security='Security';
var menu_8021xCert='802.1x Cert Install';
var menu_accessControl='Access Control';
var menu_wds='WDS Settings';
var menu_mesh='Mesh Settings';
var menu_siteSurvey='Site Survey';
var menu_wps='WPS';
var menu_schedule='Schedule';
var menu_tcpip='TCP/IP Settings';
var menu_lan='LAN Interface';
var menu_wan='WAN Interface';
var menu_fireWall='Firewall';
var menu_portFilter='Port Filtering';
var menu_ipFilter='IP Filtering';
var menu_macFilter='MAC Filtering';
var menu_portFw='Port Forwarding';
var menu_urlFilter='URL Filtering';
var menu_dmz='DMZ';
var menu_vlan='VLAN';
var menu_qos='QOS';
var menu_routeSetup='Routing Setup';
var menu_management='Management';
var menu_status='Status';
var menu_statistics='Statistics';
var menu_ddns='DDNS';
var menu_timeZone='Time Zone Setting';
var menu_dos='Denial-of-Service';
var menu_log='Log';
var menu_updateFm='Upgrade Firmware';
var menu_setting='Save/Reload Settings';
var menu_psw='Password';
var menu_logout='Logout';
/***********	opmode.htm	************/
var opmode_header='Operating Mode';
var opmode_explain='You can setup different modes for the LAN and WLAN interfaces for NAT and bridging functions.';
var opmode_radio_gw=' Gateway: ';
var opmode_radio_gw_explain='In this mode, the device connects to the internet via an ADSL/Cable Modem. NAT is enabled and PCs on  LAN ports share the same IP Address to the ISP via the WAN port. The connection type can be setup on the WAN page using PPPOE, DHCP client, PPTP client, L2TP client, or static IP.';
var opmode_radio_br=' Bridge: ';
var opmode_radio_br_explain='In this mode, all ethernet ports and wireless interfaces are bridged together and the NAT function is disabled. All WAN related functions, including the firewall, are not supported.';
var opmode_radio_wisp='Wireless ISP:';
var opmode_radio_wisp_explain='In this mode, all ethernet ports are bridged together and the wireless client will connect to the ISP access point. NAT is enabled and PCs on Ethernet ports share the same IP to the ISP via the wireless LAN. You can connect to the ISP’s AP on the Site-Survey page.  The connection type can be setup on the WAN page using PPPOE, DHCP client, PPTP client, L2TP client, or static IP.';
var opmode_radio_wisp_wanif='WAN Interface: ';
var opmode_apply='Apply Change';
var opmode_reset='Reset';



/***********	wlan_schedule.htm	************/
var wlan_schedule_header = 'Wireless Schedule';
var wlan_schedule_explain = 'This page allows you setup the wireless schedule rule. Do not forget to configure the system time before enabling this feature.';
var wlan_schedule_enable = 'Enable Wireless Schedule';
var wlan_schedule_everyday = 'Everday';
var wlan_schedule_sun = 'Sun';
var wlan_schedule_mon = 'Mon';
var wlan_schedule_tue= 'Tue';
var wlan_schedule_wed = 'Wed';
var wlan_schedule_thu = 'Thu';
var wlan_schedule_fri = 'Fri';
var wlan_schedule_sat = 'Sat';
var wlan_schedule_24Hours = '24 Hours';
var wlan_schedule_from = 'From';
var wlan_schedule_to = 'To';
var wlan_schedule_save = 'Apply Changes';
var wlan_schedule_reset = 'Reset';
var wlan_schedule_days = 'Days';
var wlan_schedule_time = 'Time';
var wlan_schedule_time_check = 'Please set the Days!';


/***********	wlwps.htm	************/
var wlwps_header = 'Wi-Fi Protected Setup';
var wlwps_header_explain = 'This page allows you to change the settings for WPS (Wi-Fi Protected Setup). Using this feature allows a wireless client to automically syncronize its settings and easily and securely connect to the Access Point.';
var wlwps_wps_disable = 'Disable WPS';
var wlwps_wps_save = 'Apply Changes';
var wlwps_wps_reset = 'Reset';
var wlwps_status = 'WPS Status:';
var wlwps_status_conn = 'Configured';
var wlwps_status_unconn = 'UnConfigured';
var wlwps_status_reset = 'Reset to UnConfigured';
var wlwps_runon = 'WPS Run on:';
var wlwps_runon_root = 'Root AP';
var wlwps_runon_rpt = 'Repeater Client';
var wlwps_lockdown_state = 'Auto-lock-down state';
var wlwps_self_pinnum = 'Self-PIN Number:';
var wlwps_unlockautolockdown = 'Unlock';
var wlwps_lockdown_state_locked = 'Locked';
var wlwps_lockdown_state_unlocked = 'Unlocked';
var wlwps_pbc_title = 'Push Button Configuration:';
var wlwps_pbc_start_button = 'Start PBC';
var wlwps_stopwsc_title = 'STOP WSC';
var wlwps_stopwsc_button = 'Stop WSC';
var wlwps_pin_number_title = 'Client PIN Number:';
var wlwps_pin_start_button = 'Start PIN';
var wlwps_keyinfo = 'Current Key Info:';
var wlwps_authentication = 'Authentication';
var wlwps_authentication_open = 'Open';
var wlwps_authentication_wpa_psk = 'WPA PSK';
var wlwps_authentication_wep_share = 'WEP Shared';
var wlwps_authentication_wpa_enterprise = 'WPA Enterprise';
var wlwps_authentication_wpa2_enterprise = 'WPA2 Enterprise';
var wlwps_authentication_wpa2_psk = 'WPA2 PSK';
var wlwps_authentication_wpa2mixed_psk = 'WPA2-Mixed PSK';
var wlwps_encryption = 'Encryption';
var wlwps_encryption_none= 'None';
var wlwps_encryption_wep = 'WEP';
var wlwps_encryption_tkip = 'TKIP';
var wlwps_encryption_aes = 'AES';
var wlwps_encryption_tkip_aes = 'TKIP+AES';
var wlwps_key = 'Key';
var wlwps_pin_conn = 'PIN Configuration:';
var wlwps_assign_ssid = 'Assign SSID of Registrar:';
var wlsps_assign_mac = 'Assign Mac of Registrar:';
var wlwps_wpa_save_pininvalid = 'Invalid PIN length! The device PIN is usually four or eight digits long.';
var wlwps_wpa_save_pinnum = 'Invalid PIN! The device PIN must be numeric digits.';
var wlwps_wpa_save_pinchecksum = 'Invalid PIN! Checksum error.';
var wlwps_pinstart_pinlen = 'Invalid Enrollee PIN length! The device PIN is usually four or eight digits long.';
var wlwps_pinstart_pinnum = 'Invalid Enrollee PIN! Enrollee PIN must be numeric digits.';
var wlwps_pinstart_pinchecksum = 'Checksum failed! Use PIN anyway? ';

var warn_msg1='WPS was disabled automatically because wireless mode setting could not be supported. ' + 'You need go to the Wireless/Basic page to modify settings to enable WPS.';
var warn_msg2='WPS was disabled automatically because Radius Authentication could not be supported. ' + 'You need go to the Wireless/Security page to modify settings to enable WPS.';
var warn_msg3="PIN number was generated. Click the \'Apply Changes\' button to make the change effective.";

/***********	wlsurvey.htm	************/
var wlsurvey_onewlan_header = 'Wireless Site Survey</p>';
var wlsurvey_morewlan_header = 'Wireless Site Survey -wlan';
var wlsurvey_header_explain = 'This page provides a tool to scan for wireless networks. If an Access Point or IBSS is found, you could choose to connect to it manually when client mode is enabled.';
var wlsurvey_site_survey = 'Site Survey';
var wlsurvey_chan_next = '<input type="button" value="  Next>>" id="next" onClick="saveClickSSID()">';
var wlsurvey_encryption = 'Encryption:';
var wlsurvey_encryption_no = 'NONE';
var wlsurvey_encryption_wep = 'WEP';
var wlsurvey_encryption_wpa = 'WPA';
var wlsurvey_encryption_wpa2= 'WPA2';

var wlsurvey_keytype = 'Key Type:';
var wlsurvey_keytype_open = 'Open';
var wlsurvey_keytype_shared = 'Shared';
var wlsurvey_keytype_both = 'Both';

var wlsurvey_keylen = 'Key Length:';
var wlsurvey_keylen_64 = '64-bit';
var wlsurvey_keylen_128 = '128-bit';

var wlsurvey_keyfmt = 'Key Format:';
var wlsurvey_keyfmt_ascii = 'ASCII';
var wlsurvey_keyfmt_hex = 'Hex';

var wlsurvey_keyset = 'Key Setting';

var wlsurvey_authmode = 'Authentication Mode:';
var wlsurvey_authmode_enter_radius = 'Enterprise (RADIUS)';
var wlsurvey_authmode_enter_server = 'Enterprise (AS Server)';
var wlsurvey_authmode_personal = 'Personal (Pre-Shared Key)';

var wlsurvey_wpacip = 'WPA Cipher Suite:';
var wlsurvey_wpacip_tkip = 'TKIP';
var wlsurvey_wpacip_aes = 'AES';
var wlsurvey_wp2chip = 'WPA2 Cipher Suite:';
var wlsurvey_wp2chip_tkip = 'TKIP';
var wlsurvey_wp2chip_aes = 'AES';

var wlsurvey_preshared_keyfmt = 'Pre-Shared Key Format:';
var wlsurvey_preshared_keyfmt_passphrase = 'Passphrase';
var wlsurvey_preshared_keyfmt_hex = 'HEX (64 characters)';
var wlsurvey_preshared_key = 'Pre-Shared Key:';

var wlsurvey_eaptype = 'EAP Type:';
var wlsurvey_eaptype_md5 = 'MD5';
var wlsurvey_eaptype_tls = 'TLS';
var wlsurvey_eaptype_peap = 'PEAP';

var wlsurvey_intunn_type = 'Inside Tunnel Type:';
var wlsurvey_intunn_type_MSCHAPV2 = 'MSCHAPV2';

var wlsurvey_eap_userid = 'EAP User ID:';
var wlsurvey_radius_passwd = 'RADIUS User Password';
var wlsurvey_radius_name = 'RADIUS User Name:';
var wlsurvey_user_keypasswd = 'User Key Password (if any):';


var wlsurvey_use_as_server = 'Use Local AS Server:';
var wlsurvey_as_ser_ipaddr = 'AS Server IP Address:';
var wlsurvey_back_button = '<<Back  ';
var wlsurvey_conn_button = 'Connect';
var wlsurvey_wait_explain = 'Please wait...';
var wlsurvey_inside_nosupport = 'This inside type is not supported.';
var wlsurvey_eap_nosupport = 'This EAP type is not supported.';
var wlsurvey_wrong_method = 'Wrong method!';
var wlsurvey_nosupport_method = 'Error: not supported method id';
var wlsurvey_nosupport_wpa2 = 'Error: not supported WPA2 cipher suite ';
var wlsurvey_nosupport_wpasuit = 'Error: not supported WPA cipher suite ';
var wlsurvey_nosupport_encry = 'Error: not supported encryption';

var wlsurvey_tbl_ssid = 'SSID';
var wlsurvey_tbl_bssid = 'BSSID';
var wlsurvey_tbl_chan = 'Channel';
var wlsurvey_tbl_type = 'Type';
var wlsurvey_tbl_ency = 'Encrypt';
var wlsurvey_tbl_signal = 'Signal';
var wlsurvey_tbl_select = 'Select';
var wlsurvey_tbl_macaddr = 'MAC Adddress';
var wlsurvey_tbl_meshid = 'Mesh ID';
var wlsurvey_tbl_none = 'None';

var wlsurvey_read_site_error = 'Read site-survey status failed!';
var wlsurvey_get_modemib_error = 'Get MIB_WLAN_MODE MIB failed!';
var wlsurvey_get_bssinfo_error = 'Get bssinfo failed!';


/***********	wlwds.htm	************/
var wlwds_onelan_header = 'WDS Settings</p>';
var wlwds_morelan_header = 'WDS Settings -wlan';
var wlwds_header_explain = 'Wireless Distribution System uses the wireless media to communicate with other APs, as Ethernet does. To do this, you must set these APs to the same channel and set the MAC address of other APs that you want to communicate with in the table, and then enable WDS.';
var wlwds_enable = 'Enable WDS';
var wlwds_mac_addr = 'MAC Address:';
var wlwds_data_rate = 'Data Rate:';
var wlwds_rate_auto = 'Auto';
var wlwds_comment = 'Comment:';
var wlwds_apply = 'Apply Changes';
var wlwds_reset = 'Reset';
var wlwds_set_secu = 'Set Security';
var wlwd_show_stat = 'Show Statistics';
var wlwds_wdsap_list = 'Current WDS AP List:';
var wlwds_delete_select = 'Delete Selected';
var wlwds_delete_all = 'Delete All';
var wlwds_fmwlan_txrate = 'Tx Rate (Mbps)';
var wlwds_fmwlan_select = 'Select';
var wlwds_fmwlan_comment = 'Comment';
var wlwds_macaddr_nocomplete = 'Input MAC address is not complete. It should be 12 digits in hex.';
var wlwds_macaddr_invalid = 'Invalid MAC address. It should be in hex numbers (0-9 and a-f).';
var wlwds_delete_chick = 'Do you really want to delete the selected entry?';
var wlwds_delete_allchick = 'Do you really want to delete all entries?';

/***********	wlactrl.htm	************/
var wlactrl_onelan_header = 'Wireless Access Control</p>';
var wlactrl_morelan_header = 'Wireless Access Control -wlan';
var wlactrl_header_explain = 'If you choose Allowed Listed, only those clients whose wireless MAC addresses are in the access control list will be able to connect to your Access Point. When Deny Listed is selected, these wireless clients on the list will not be able to connect to the Access Point.';
var wlactrl_accmode = 'Wireless Access Control Mode:';
var wlactrl_accmode_diable = 'Disable';
var wlactrl_accmode_allowlist = 'Allow Listed';
var wlactrl_accmode_denylist = 'Deny Listed';
var wlactrl_macaddr = 'MAC Address:';
var wlactrl_comment = 'Comment: ';
var wlactrl_apply = 'Apply Changes';
var wlactrl_reset = 'Reset';
var wlactrl_accctrl_list = 'Current Access Control List:';
var wlactrl_delete_select_btn = 'Delete Selected';
var wlactrl_delete_all_btn = 'Delete All';
var wlactrl_fmwlan_macaddr = 'MAC Address';
var wlactrl_fmwlan_select = 'Select';
var wlactrl_apply_explain = 'If the ACL allow list is turned on; WPS2.0 will be disabled';
var wlactrl_apply_mac_short = 'Input MAC address is not complete. It should be 12 digits in hex.';
var wlactrl_apply_mac_invalid = 'Invalid MAC address. It should be in hex numbers (0-9 and a-f).';
var wlactrl_delete_result = 'Deleting all entries will cause all clients to be unableto connect to the AP.  Are you sure?';
var wlactrl_delete_select = 'Do you really want to delete the selected entry?';
var wlactrl_delete_all = 'Do you really want to delete all entries?';

/***********	firewall	************/
var firewall_proto = 'Protocol:';
var firewall_proto_both = 'Both';
var firewall_proto_tcp = 'TCP';
var firewall_proto_udp = 'UDP';
var firewall_add_rule = 'Add rule';
var firewall_apply = 'Apply Changes';
var firewall_reset = 'Reset';
var firewall_filtertbl = 'Current Filter Table:';
var firewall_delete_select = 'Delete Selected';
var firewall_delete_all = 'Delete All';

var firewall_delete_selectconfm = 'Do you really want to delete the selected entry?';
var firewall_delete_allconfm = 'Do you really want to delete all entries?';
var firewall_ipaddr_invalid = 'Invalid IP address';
var firewall_port_notdecimal = 'Invalid port number! It should be the decimal numbers (0-9).';
var firewall_port_toobig = 'Invalid port number! You should set a value between 1-65535.';
var firewall_port_rangeinvalid = 'Invalid port range! 1st port value should be less than the 2nd value.';



var firewall_local_ipaddr = 'Loal IP Address:';
var firewall_port_range = 'Port Range: ';
var firewall_comm = 'Comment:';
var firewall_ip_invalid_range = 'Invalid ip address range!\nEnding address should be greater than or equal to starting address.';

var firewall_tbl_proto = 'Protocol';
var firewall_tbl_comm = 'Comment';
var firewall_tbl_select = 'Select';
var firewall_tbl_localipaddr = 'Local IP Address';
var firewall_portrange = 'Port Range';

/***********	portfilter.htm	************/
var portfilter_header = 'Port Filtering';
var portfilter_header_explain = 'Entries in this table are used to restrict certain types of data packets from your local network passing to the Internet through the Gateway. Use of these filters can be helpful in securing or restricting your local network.';
var portfiletr_enable = 'Enable Port Filtering';
var portfilter_noport = 'You should set a port range for adding in an entry.';


/***********	ipfilter.htm	************/
var ipfilter_header = 'IP Filtering';
var ipfilter_header_explain = 'Entries in this table are used to restrict certain types of data packets from your local network passing to the Internet through the Gateway. Use of such filters can be helpful in securing or restricting your local network.';
var ipfilter_enable = 'Enable IP Filtering';

/***********	Macfilter.htm	************/
var macfilter_header = 'MAC Filtering';
var macfilter_header_explain =  'Entries in this table are used to restrict certain types of data packets from your local network passing to the Internet through the Gateway. Use of such filters can be helpful in securing or restricting your local network.';
var macfilter_enable = 'Enable MAC Filtering';
var macfilter_macaddr = 'MAC Address: ';
var macfilter_macaddr_nocomplete = 'Input MAC address is not complete. It should be 12 digits in hex.';
var macfilter_macaddr_nohex = 'Invalid MAC address. It should be in hex numbers (0-9 or a-f).';
var macfilter_filterlist_macaddr = 'MAC Address';

/***********	Portfw.htm	************/
var portfw_header = 'Port Forwarding';
var portfw_header_explain = 'Entries in this table allow you to automatically redirect common network services to a specific machine behind the NAT firewall.  These settings are only necessary if you wish to host some sort of server such as a web server or mail server on the private local network behind your Gateway\'s NAT firewall.';//need xiugai
var portfw_enable = 'Enable Port Forwarding';
var portfw_ipaddr = 'IP Address:';
var portfw_apply_port_empty = 'Port range cannot be empty! You should set a value between 1-65535.';
var portfw_tbl = 'Current Port Forwarding Table:';

/***********	urlfilter.htm	************/
var urlfilter_header = 'URL Filtering';
var urlfilter_header_explain = 'The URL filter is used to restrict LAN users access to the internet. Block those URLs which contain keywords listed below.';
var urlfilter_enable = 'Enable URL Filtering';
var urlfilter_urladdr = 'URL Address: ';
var urlfilter_apply_error = 'Error Character: \";\"';
var urlfilter_filterlist_yrladdr = 'URL Address';

/***********	dmz.htm	************/
var dmz_header = 'DMZ';
var dmz_header_explain = 'A Demilitarized Zone is used to provide Internet services without sacrificing unauthorized access to its local private network. Typically, the DMZ host contains devices accessible to Internet traffic, such as Web (HTTP ) servers, FTP servers, SMTP (e-mail) servers, and DNS servers.';
var dmz_enable = 'Enable DMZ';
var dmz_host_ipaddr = 'DMZ Host IP Address:';

/***********	logout.htm	************/
var logout_header = 'Logout';
var logout_header_explain = 'This page is used to  logout.';
var logout_confm = 'Do you want to logout ?';
var logout_apply = 'Apply Changes';

/***********	password.htm	************/
var password_header = 'Password Setup';
var password_header_explain = ' This page is used to setup an account to access the web server of the Access Point. An empty user name and password will disable password protection.';
var password_user_name = 'User Name:';
var password_user_passwd = 'New Password:';
var password_user_passwd_confm = 'Confirm Password:';
var password_apply = 'Apply Changes';
var password_user_empty = 'User account is empty.\nDo you want to disable the password protection?';
var password_passwd_unmatched = 'Password does not match. Please re-type the password in the \'new\' and \'confirmed\' boxes.';
var password_passwd_empty = 'Password cannot be empty. Please try again.';
var password_user_invalid = 'Cannot accept space character in user name. Please try again.';
var password_passwd_invalid = 'Cannot accept space character in password. Please try again.';
var password_reset = '  Reset  ';

/***********	saveconf.htm	************/
var saveconf_header = 'Save/Reload Settings';
var saveconf_header_explain = ' This page allows you to save current settings to a file or reload the settings from a file that was saved previously. You can also reset the current configuration to factory defaults.';
var saveconf_save_to_file = 'Save Settings to File:';
var saveconf_save = 'Save...';
var saveconf_load_from_file = 'Load Settings from File:';
var saveconf_load = 'Upload';
var saveconf_reset_to_default = 'Reset Settings to Default:';
var saveconf_reset = 'Reset';
var saveconf_load_from_file_empty = 'Please select a file!';
var saveconf_reset_to_default_confm = 'Do you really want to reset the current settings to default settings?';

/***********	upload.htm	************/
var upload_header = 'Upgrade Firmware';
var upload_header_explain = ' This page allows you to upgrade the Access Point firmware to the latest version. Please note, do not power off the device during the upload as it may crash the system.';
var upload_version = 'Firmware Version:';
var upload_file_select = 'Select File:';
var upload_send = 'Upload';
var upload_reset = 'Reset';
var upload_up_failed = 'Update firmware failed!';
var upload_send_file_empty = 'File name cannot be empty!';

/***********	syslog.htm	************/
var syslog_header = 'System Log';
var syslog_header_explain = 'This page can be used to set a remote log server and view the system log.';
var syslog_enable = 'Enable Log';
var syslog_sys_enable = 'System All';
var syslog_wireless_enable = 'Wireless';
var syslog_dos_enable = 'DoS';
var syslog_11s_enable = '11s';
var syslog_rtlog_enable = 'Enable Remote Log';
var syslog_local_ipaddr = 'Log Server IP Address:';
var syslog_apply = 'Apply Changes';
var syslog_refresh = 'Refresh';
var syslog_clear = 'Clear';

/***********	dos.htm	************/
var dos_header = 'Denial of Service';
var dos_header_explain = 'A "denial-of-service" (DoS) attack is characterized by an explicit attempt by hackers to prevent legitimate users of a service from using that service.';
var dos_enable = 'Enable DoS Prevention';
var dos_packet_sec = ' Packets/Second';
var dos_sysflood_syn = 'Whole System Flood: SYN';
var dos_sysflood_fin = 'Whole System Flood: FIN';
var dos_sysflood_udp = 'Whole System Flood: UDP';
var dos_sysflood_icmp = 'Whole System Flood: ICMP';
var dos_ipflood_syn = 'Per-Source IP Flood: SYN';
var dos_ipflood_fin = 'Per-Source IP Flood: FIN';
var dos_ipflood_udp = 'Per-Source IP Flood: UDP';
var dos_ipflood_icmp = 'Per-Source IP Flood: ICMP';
var dos_portscan = 'TCP/UDP PortScan';
var dos_portscan_low = 'Low';
var dos_portscan_high = 'High';
var dos_portscan_sensitivity = 'Sensitivity';
var dos_icmp_smurf = 'ICMP Smurf';
var dos_ip_land = 'IP Land';
var dos_ip_spoof = 'IP Spoof';
var dos_ip_teardrop = 'IP TearDrop';
var dos_pingofdeath = 'PingOfDeath';
var dos_tcp_scan = 'TCP Scan';
var dos_tcp_synwithdata = 'TCP SynWithData';
var dos_udp_bomb = 'UDP Bomb';
var dos_udp_echochargen = 'UDP EchoChargen';
var dos_select_all = ' Select ALL ';
var dos_clear_all = 'Clear ALL';
var dos_enable_srcipblocking = 'Enable Source IP Blocking';
var dos_block_time = 'Block time (sec)';
var dos_apply = 'Apply Changes';

/***********	ntp.htm	************/
var ntp_header = 'Time Zone Setting';
var ntp_header_explain = 'You can maintain the system time by synchronizing with a public time server over the Internet.';
var ntp_curr_time = 'Current Time:';
var ntp_year = 'Yr';
var ntp_month = 'Mon';
var ntp_day = 'Day';
var ntp_hour = 'Hr';
var ntp_minute = 'Mn';
var ntp_second = 'Sec';
var ntp_copy_comptime = 'Copy Computer Time';
var ntp_time_zone = 'Time Zone Select:';
var ntp_enable_clientup = 'Enable NTP client Update';
var ntp_adjust_daylight = 'Automatically Adjust for Daylight Saving';
//var ntp_server = ' SNTP server:';
var ntp_server = ' NTP server:';
var ntp_server_north_america1 = '198.123.30.132    - North America';
var ntp_server_north_america2 = '209.249.181.22   - North America';
//var ntp_server_Europe1 = '85.12.35.12  - Europe';
//var ntp_server_Europe2 = '217.144.143.91   - Europe';
var ntp_server_Europe1 = '131.188.3.220  - Europe';
var ntp_server_Europe2 = '130.149.17.8   - Europe';
var ntp_server_Australia = '223.27.18.137  - Australia';
//var ntp_server_asia1 = '133.100.11.8 - Asia Pacific';
var ntp_server_asia1 = '203.117.180.36 - Asia Pacific';
var ntp_server_asia2 = '210.72.145.44 - Asia Pacific';
var ntp_manu_ipset = '(Manual IP Setting)';
var ntp_apply = 'Apply Change';
var ntp_reset = 'Reset';
var ntp_refresh = 'Refresh';
var ntp_month_invalid = 'Invalid month Number. It should be in  number (1-9).';
var ntp_time_invalid = 'Invalid Time value!';
var ntp_ip_invalid = 'Invalid IP address';
var ntp_servip_invalid = 'Invalid NTP Server IP address! It can not be empty.';
var ntp_field_check = 'Field can\'t be empty\n';
var ntp_invalid = 'Invalid';
var ntp_num_check = ' Number. It should be in  numbers (0-9).';

/***********	ddns.htm	************/
var ddns_header = 'Dynamic DNS';
var ddns_header_explain = 'Dynamic DNS is a service that provides you with a valid, unchanging, internet domain name (an URL) to go with a (possibly changing) IP-address.';
var ddns_enable = 'Enable DDNS ';
var ddns_serv_provider = ' Service Provider:';
var ddns_dyndns = 'DynDNS';
var ddns_orayddns = 'OrayDDNS';
var ddns_tzo = 'TZO';
var ddns_domain_name = 'Domain Name:';
var ddns_user_name = ' User Name/Email:';
var ddns_passwd = ' Password/Key:';
var ddns_note = 'Note:';
var ddns_oray_header = 'For Oray DDNS, you can create your Oray account';
var ddns_here = 'here';
var ddns_dyn_header = 'For DynDNS, you can create your DynDNS account';
var ddns_tzo_header1 = 'For TZO, you can have a 30 days free trial';
var ddns_tzo_header2 = 'or manage your TZO account in';
var ddns_tzo_header3 = 'control panel';
var ddns_apply = 'Apply Change';
var ddns_reset = 'Reset';
var ddns_domain_name_empty = 'Domain Name can\'t be empty';
var ddns_user_name_empty = 'User Name/Email can\'t be empty';
var ddns_passwd_empty = 'Password/Key can\'t be empty';

/***********	ip_qos.htm	************/
var ip_qos_header = 'QoS';
var ip_qos_header_explain = ' Entries in this table improve your online gaming experience by ensuring that your game traffic is prioritized over other network traffic, such as FTP or Web.';
var ip_qos_enable = 'Enable QoS';
var ip_qos_bandwidth = 'Bandwidth Shaping';
var ip_qos_schedule = 'WFQ';
var ip_qos_auto_upspeed = 'Automatic Uplink Speed';
var ip_qos_manu_upspeed = 'Uplink Speed (Kbps):';
var ip_qos_auto_downspeed = 'Automatic Downlink Speed';
var ip_qos_manu_downspeed = 'Downlink Speed (Kbps):';
var ip_qos_rule_set = 'QoS Rule Setting:';
var ip_qos_addrtype = 'Address Type:';
var ip_qos_addrtype_ip = 'IP';
var ip_qos_addrtype_mac = 'MAC';
var ip_qos_local_ipaddr = 'Local IP Address:';
var ip_qos_proto = 'Protocol:';
var ip_qos_proto_udp = 'udp';
var ip_qos_proto_tcp = 'tcp';
var ip_qos_proto_both = 'both';
var ip_qos_local_port = 'Local Port:(1~65535)';
var ip_qos_macaddr = 'MAC Address:';
var ip_qos_mode = 'Mode:';
var ip_qos_weight = 'Weight';
var ip_qos_upband = 'Uplink Bandwidth (Kbps):';
var ip_qos_downband = 'Downlink Bandwidth (Kbps):';
var ip_qos_apply = 'Apply Changes';
var ip_qos_reset = 'Reset';
var ip_qos_curr_qos = 'Current QoS Rules Table:';
var ip_qos_delete_select_btn = 'Delete Selected';
var ip_qos_delete_all_btn = 'Delete All';

var ip_qos_upspeed_empty = 'Manual Uplink Speed cannot be empty or less then 100 when Automatic Uplink Speed is disabled.';
var ip_qos_downspeed_empty = 'Manual Downlink Speed cannot be empty or less then 100 when Automatic Downlink Speed is disabled.';
var ip_qos_ip_invalid = 'Invalid IP address';
var ip_qos_startip_invalid = 'Invalid start IP address! It should be set within the current subnet.';
var ip_qos_portrange_invalid = 'Invalid port range! It should be 1~65535.';
var ip_qos_macaddr_notcomplete = 'Input MAC address is not complete. It should be 12 digits in hex.';
var ip_qos_macaddr_invalid = 'Invalid MAC address. It should be in hex number (0-9 or a-f),and can not be all zero.';
var ip_qos_band_empty = 'Uplink Bandwidth or Downlink Bandwidth cannot be 0 or empty.';
var ip_qos_band_invalid = 'Invalid input! It should be the decimal number (0-9).';
var ip_qos_band_notint = 'Invalid input! It should be integer numbers';
var ip_qos_weigth_empty = 'Weight cannot be empty.';
var ip_qos_weight_invalid = 'Invalid weight range! It should be int number (1~20).';
var ip_qos_delete_select = 'Do you really want to delete the selected entry?';
var ip_qos_delete_all = 'Do you really want to delete the all entries?';

var ip_qos_tbl_localaddr = 'Local IP Address';
var ip_qos_tbl_macaddr = 'MAC Address';
var ip_qos_tbl_mode = 'Mode';
var ip_qos_tbl_valid = 'Valid';
var ip_qos_tbl_upband = 'Uplink Bandwidth';
var ip_qos_tbl_downband = 'Downlink Bandwidth';
var ip_qos_tbl_select = 'Select';
var ip_qos_restrict_maxband = "Restricted maximum bandwidth";
var ip_qos_quarant_minband = "Guaranteed minimum bandwidth";

/***********	wlbasic.htm	************/
var wlbasic_header='Wireless Basic Setting';
var wlbasic_explain = 'This page is used to configure the parameters for wireless LAN clients which may connect to your Access Point. Here you may change wireless encryption settings as well as wireless network parameters.';
var wlbasic_network_type = 'Network Type:';
var wlbasic_ssid = 'SSID:';
var wlbasic_disabled = 'Disable Wireless LAN Interface';
var wlbasic_country = 'Country:';
var wlbasic_band= 'Band:';
var wlbasic_infrastructure = "Infrastructure";
var wlbasic_adhoc = "Adhoc";
var wlbasic_addprofile = 'Add to Profile';
var wlbasic_channelwidth = 'Channel Width:';
var wlbasic_ctlsideband = 'Control Sideband:';
var wlbasic_ctlsideautomode = 'Auto';
var wlbasic_ctlsidelowermode = 'Lower';
var wlbasic_ctlsideuppermode = 'Upper';
var wlbasic_chnnelnum = 'Channel Number:';
var wlbasic_broadcastssid= 'Broadcast SSID:';
var wlbasic_brossid_enabled = 'Enabled';
var wlbasic_brossid_disabled = 'Disabled';
var wlbasic_wmm ='WMM:';
var wlbasic_wmm_disabled = 	'Disabled';
var wlbasic_wmm_enabled = 	'Enabled';
var wlbasic_data_rate = 'Data Rate:';
var wlbasic_data_rate_auto = "Auto";
var wlbasic_associated_clients = 'Associated Clients:';
var wlbasic_show_associated_clients = 'Show Active Clients';
var wlbasic_enable_mac_clone = 'Enable Mac Clone (Single Ethernet Client)';
var wlbasic_enable_repeater_mode = 'Enable Universal Repeater Mode (Acting as AP and client simultaneouly)';
var wlbasic_extended_ssid = 'SSID of Extended Interface:';
var wlbasic_ssid_note = 'Note: If you want to change the setting for Mode and SSID, you must go to the EasyConfig page to disable EasyConfig first.';
var wlbasic_enable_wl_profile = 'Enable Wireless Profile';
var wlbasic_wl_profile_list = 'Wireless Profile List:';
var wlbasic_apply = 'Apply Changes';
var wlbasic_reset = 'Reset';
var wlbasic_delete_select = 'Delete Selected';
var wlbasic_delete_all = 'DeleteAll';
var wlbasic_enable_wire = 'Do you also want to enable wireless profile?';
var wlbasic_asloenable_wire = 'Do you also want to enable wireless profile?';

var wlbasic_mode = 'Mode:';
var wlbasic_client = 'Client';

/***********	wlstatbl.htm	************/
var wlstatbl_tbl_name = 'Active Wireless Client Table';
var wlstatbl_explain = ' This table shows the MAC address, transmission, reception packet counters and encrypted status for each associated wireless client.';
var wlstatbl_mac = 'MAC Address';
var wlstatbl_mode = 'Mode';
var wlstatbl_tx = 'Tx Packet';
var wlstatbl_rx = 'Rx Packe';
var wlstatbl_tx_rate ='Tx Rate (Mbps)';
var wlstatbl_ps = 'Power Saving';
var wlstatbl_expired_time = 'Expired Time (s)';
var wlstatbl_refresh = 'Refresh';
var wlstatbl_close = 'Close';

/***********	wladvanced.htm	************/
var wladv_vallid_num_alert = 'Invalid value. It should be in decimal numbers (0-9).';
var wladv_fragment_thre_alert = 'Invalid value of Fragment Threshold. Input value should be between 256-2346 in decimal.';
var wladv_rts_thre_alert = 'Invalid value of RTS Threshold. Input value should be between 0-2347 in decimal.';
var wladv_beacon_alert = 'Invalid value of Beacon Interval. Input value should be between 20-1024 in decimal.';
var wladv_header = 'Wireless Advanced Settings';
var wladv_explain = 'These settings are only for more technically advanced users who have a sufficient knowledge about wireless LAN. These settings should not be changed unless you know what effect the changes will have on your Access Point.';
var wladv_frg_thre = 'Fragment Threshold:';
var wladv_rts_thre = 'RTS Threshold:';
var wladv_beacon_interval = 'Beacon Interval:';
var wladv_preamble_type = 'Preamble Type:';
var wladv_preamble_long = 'Long Preamble';
var wladv_preamble_short = 'Short Preamble';
var wladv_iapp = 'IAPP:';
var wladv_iapp_enabled = 'Enabled';
var wladv_iapp_disabled = 'Disabled';
var wladv_protection = 'Protection:';
var wladv_protection_enabled = 'Enabled';
var wladv_protection_disabled = 'Disabled';
var wladv_aggregation = 'Aggregation:';
var wladv_aggregation_enabled = 'Enabled';
var wladv_aggregation_disabled = 'Disabled';
var wladv_short_gi = 'Short GI:';
var wladv_short_gi_enabled = 'Enabled';
var wladv_short_gi_disabled = 'Disabled';
var wladv_wlan_partition = 'WLAN Partition:';
var wladv_wlan_partition_enabled = 'Enabled';
var wladv_wlan_partition_disabled = 'Disabled';
var wladv_stbc = 'STBC:';
var wladv_stbc_enabled = 'Enabled';
var wladv_stbc_disabled = 'Disabled';
var wladv_coexist = '20/40MHz Coexist:';
var wladv_coexist_enabled = 'Enabled';
var wladv_coexist_disabled = 'Disabled';
var wladv_tx_beamform = 'TX Beamforming:';
var wladv_tx_beamform_enabled = 'Enabled';
var wladv_tx_beamform_disabled = 'Disabled';
var wladv_rf_power = 'RF Output Power:';
var wladv_apply = 'Apply Changes';
var wladv_reset = ' Reset ';

/***********	wlsecutity.htm wlsecutity_all.htm	************/
var wlsec_validate_note = "Note: if you choose [Enterprise (AS Server)] and apply changes here, \nthis wlan interface and its virtual interfaces will use the current AS setting.\nDo you want to continue?";
var wlsec_header = 'Wireless Security Setup';
var wlsec_explain = ' This page allows you setup wireless security. Using WEP or WPA Encryption Keys will help prevent unauthorized access to your wireless network.';
var wlsec_select_ssid = 'Select SSID:';
var wlsec_psk= 'PSK';
var wlsec_pre_shared = 'Pre-Shared Key';
var wlsec_tkip = 'TKIP';
var wlsec_aes = 'AES';
var wlsec_apply = 'Apply Changes';
var wlsec_reset = 'Reset';
var wlsec_inside_type_alert = "This inside type is not supported.";
var wlsec_eap_alert = 'This EAP type is not supported.';
var wlsec_wapi_remote_ca_install_alert = "Please make sure that wapi certs from the remote AS have already been installed on webpage [WAPI] -> [Certificate Install].";
var wlsec_wapi_local_ca_install_alert = "Please make sure that wapi certs from the local AS have already been installed at webpage [WAPI] -> [Certificate Install].";
var wlsec_wapi_wrong_select_alert = "Wrong wapi cert selected index.";
var wlsec_wapi_key_length_alert = "the wapi key should be at least 8 characters and no more than 32 characters";
var wlsec_wapi_key_hex_alert = "The Hex Mode WAPI key length should be 64 Hex number";
var wlsec_wapi_invalid_key_alert = "Invalid key value. It should be in hex numbers (0-9 or a-f).";
var wlsec_wep_confirm = "if WEP is turned on, WPS2.0 will be disabled";
var wlsec_wpa_confirm = "under WPA only or TKIP only, WPS2 daemon will be disabled";
var wlsec_wpa2_empty_alert = "WPA2 Cipher Suite Cannot be empty.";
var wlsec_wpa_empty_alert = "WPA Cipher Suite Cannot be empty.";
var wlsec_tkip_confirm = "if TKIP only; WPS2.0 will be disabled";
var wlsec_encryption =  'Encryption:';
var wlsec_disabled = 'Disabled';
var wlsec_wpa_mix = 'WPA-Mixed';
var wlsec_802_1x = '802.1x Authentication:';
var wlsec_auth = 'Authentication:';
var wlsec_auth_open_sys = 'Open System';
var wlsec_auth_shared_key = 'Shared Key';
var wlsec_auth_auto = 'Auto';
var wlsec_key_length = 'Key Length:';
var wlsec_key_hex = 'HEX';
var wlsec_key_ascii = 'ASCII';
var wlsec_encryption_key = 'Encryption Key:';
var wlsec_auth_mode = 'Authentication Mode:';
var wlsec_auth_enterprise_mode = 'Enterprise (RADIUS)';
var wlsec_auth_enterprise_ap_mode = 'Enterprise (AS Server)';
var wlsec_auth_personal_mode = 'Personal (Pre-Shared Key)';
var wlsec_wpa_suite = 'WPA Cipher Suite:';
var wlsec_wpa2_suite = 'WPA2 Cipher Suite:';
var wlsec_wep_key_format = 'Key Format:';
var wlsec_pre_key_format = 'Pre-Shared Key Format:';
var wlsec_pre_key = 'Pre-Shared Key:';
var wlsec_passpharse = 'Passphrase';
var wlsec_key_hex64 = 'HEX (64 characters)';
var wlsec_key_64bit = '64 Bits';
var wlsec_key_128bit = '128 Bits';
var wlsec_radius_server_ip = "RADIUS&nbsp;Server&nbsp;IP&nbsp;Address:";
var wlsec_radius_server_port = 'RADIUS&nbsp;Server&nbsp;Port:';
var wlsec_radius_server_password = 'RADIUS&nbsp;Server&nbsp;Password:';
var wlsec_eap_type = 'EAP Type:';
var wlsec_inside_tunnel_type = 'Inside Tunnel Type:';
var wlsec_eap_user_id = 'EAP User ID:';
var wlsec_radius_user = 'RADIUS User Name:';
var wlsec_radius_user_password = 'RADIUS User Password:';
var wlsec_user_key_password = 'User Key Password (if any):';
var wlsec_use_local_as = 'Use Local AS Server:';
var wlsec_as_ip = 'AS&nbsp;Server&nbsp;IP&nbsp;Address:';
var wlsec_select_wapi_ca = 'Select WAPI certificate:';
var wlsec_use_ca_from_as = 'Use Cert from Remote AS';
var wlsec_adhoc_wep = 'Adhoc mode don\'t support WEP encryption with N band or AC band!';

/***********	tcpipwan.htm  tcpiplan.htm************/
var tcpip_check_ip_msg = 'Invalid IP address';
var tcpip_check_server_ip_msg = 'Invalid server IP address';
var tcpip_check_dns_ip_msg1 = 'Invalid DNS1 address';
var tcpip_check_dns_ip_msg2 = 'Invalid DNS2 address';
var tcpip_check_dns_ip_msg3 = 'Invalid DNS3 address';
var tcpip_check_size_msg = "Invalid MTU size! You should set a value between";
var tcpip_check_user_name_msg = "user name cannot be empty!";
var tcpip_check_password_msg = "password cannot be empty";
var tcpip_check_invalid_time_msg = "Invalid idle time value! You should set a value between";
var tcpip_pppoecontype_alert = "wrong pppoeConnType";
var tcpip_pptpontype_alert = "wrong pptpConnType";
var tcpip_l2tpcontype_alert = "wrong l2tpConnType";
var tcpip_pppcontype_alert = "wrong pppConnType";
var tcpip_browser_alert = 'Error! Your browser must have CSS support!';
var tcpip_wan_header = 'WAN Interface Setup';
var tcpip_wan_explain = 'This page is used to configure the parameters for Internet network which connects to the WAN port of your Access Point. Here you may change the access method to static IP, DHCP, PPPoE, PPTP or L2TP by click the item value of WAN Access type.';
var tcpip_wan_auto_dns = 'Attain DNS Automatically';
var tcpip_wan_manually_dns =  'Set DNS Manually';
var tcpip_wan_conn_time = '&nbsp;(1-1000 minutes)';
var tcpip_wan_max_mtu_size = 'bytes';
var tcpip_wan_conn = 'Connect';
var tcpip_wan_disconn = 'Disconnect';
var tcpip_wan_continuous = 'Continuous';
var tcpip_wan_on_demand = 'Connect on Demand';
var tcpip_wan_manual = 'Manual';
var tcpip_wan_access_type = 'WAN Access Type:';
var tcpip_wan_type_static_ip = 'Static IP';
var tcpip_wan_type_client = "DHCP Client";
var tcpip_wan_type_pppoe_henan = "PPPoE+(henan CNCMAX broadband)";
var tcpip_wan_type_pppoe_nanchang = "Dynamic PPPoE(nanchang ChinaNetClient)";
var tcpip_wan_type_pppoe_other1 = "PPPoE_other1(hunan and hubei ChinaNetClient)";
var tcpip_wan_type_pppoe_other2 = "PPPoE_other2(hunan and hubei ChinaNetClient)";
var tcpip_wan_type_dhcp_plus = "DHCP+(henan region)";
var tcpip_wan_ip = "IP Address:";
var tcpip_wan_mask = 'Subnet Mask:';
var tcpip_wan_default_gateway = 'Default Gateway:';
var tcpip_wan_mtu_size = 'MTU Size:';
var tcpip_wan_host = 'Host Name:';
var tcpip_wan_user = 'User Name:';
var tcpip_wan_password = 'Password:';
var tcpip_wan_server_ac = 'Service Name(AC):';
var tcpip_wan_conn_type = 'Connection Type:';
var tcpip_wan_idle_time = 'Idle Time:';
var tcpip_wan_server_ip = 'Server IP Address:';
var tcpip_wan_clone_mac = 'Clone MAC Address:';
var tcpip_wan_enable_tr069_dhcp = '&nbsp;&nbsp;Enable DHCP For TR069';
var tcpip_wan_enable_upnp = '&nbsp;&nbsp;Enable uPNP';
var tcpip_wan_enable_igmp_proxy = '&nbsp;&nbsp;Enable IGMP Proxy';
var tcpip_wan_enable_ping = '&nbsp;&nbsp;Enable Ping Access on WAN';
var tcpip_wan_enable_webserver = '&nbsp;&nbsp;Enable Web Server Access on WAN';
var tcpip_wan_enable_ipsec = '&nbsp;&nbsp;Enable IPsec pass through on VPN connection';
var tcpip_wan_enable_pptp = '&nbsp;&nbsp;Enable PPTP pass through on VPN connection';
var tcpip_wan_enable_l2tp = '&nbsp;&nbsp;Enable L2TP pass through on VPN connection';
var tcpip_wan_enable_ipv6 = '&nbsp;&nbsp;Enable IPv6 pass through on VPN connection';
var tcpip_wan_enable_netsniper = '&nbsp;&nbsp;Enable bypass netsniper';
var tcpip_wan_dynamic_ip = 'Dynamic IP (DHCP)';
var tcpip_wan_static_ip = 'Static IP';
var tcpip_wan_attain_by_DN = 'Attain Server By Domain Name';
var tcpip_wan_attain_by_Ip = 'Attain Server By Ip Address';
var tcpip_apply = 'Apply Changes';
var tcpip_reset = 'Reset';
var tcpip_lan_wrong_dhcp_field = "Wrong dhcp field!";
var tcpip_lan_start_ip = 'Invalid DHCP client start address!';
var tcpip_lan_end_ip = 'Invalid DHCP client End address!';
var tcpip_lan_ip_alert = '\nIt should be located in the same subnet of the current IP address.';
var tcpip_lan_invalid_rang = 'Invalid DHCP client address range!\nEnding address should be greater than starting address.';
var tcpip_lan_invalid_rang_value = "Invalid value. It should be in range (1 ~ 10080).";
var tcpip_lan_invalid_dhcp_type = "Load invalid dhcp type!";
var tcpip_lan_header = 'LAN Interface Setup';
var tcpip_lan_explain = ' This page is used to configure the parameters for the local area network that connects to the LAN port of your Access Point. Here you may change the settings for IP addresss, subnet mask, DHCP, etc..';
var tcpip_lan_ip = "IP Address:";
var tcpip_lan_mask = 'Subnet Mask:';
var tcpip_lan_default_gateway = 'Default Gateway:';
var tcpip_lan_dhcp = 'DHCP:';
var tcpip_lan_dhcp_disabled = 'Disabled';
var tcpip_lan_dhcp_client = 'Client';
var tcpip_lan_dhcp_server = 'Server';
var tcpip_lan_dhcp_auto = 'Auto';
var tcpip_lan_dhcp_rang = 'DHCP Client Range:';
var tcpip_lan_dhcp_time = 'DHCP Lease Time:';
var tcpip_minutes = 'minutes';
var tcpip_lan_staicdhcp = 'Static DHCP:';
var tcpip_staticdhcp = "Set Static DHCP";
var tcpip_domain = "Domain Name:";
var tcpip_netbios = "NetBIOS Name:";
var tcpip_802_1d = "802.1d Spanning Tree:";
var tcpip_802_1d_enable = 'Enabled';
var tcpip_802_1d_disabled = 'Disabled';
var tcpip_show_client = 'Show Client';

var tcpip_l2tp_server_domain_name="Invalid Domain Name! Please enter characters in A(a)~Z(z) or 0-9 without spacing."
/***********	dhcptbl.htm ************/
var dhcp_header = 'Active DHCP Client Table';
var dhcp_explain = ' This table shows the assigned IP address, MAC address and time expired for each DHCP leased client.';
var dhcp_ip = 'IP Address';
var dhcp_mac = 'MAC Address';
var dhcp_time = 'Time Expired(s)';
var dhcp_refresh = 'Refresh';
var dhcp_close = 'Close';

/***********	util_gw.htm ************/
var util_gw_wps_warn1 = 'The SSID has been configured by WPS. Any change of the setting ';
var util_gw_wps_warn2 = 'AP Mode has been configured by WPS. Any change of the setting ';
var util_gw_wps_warn3 = 'The security setting has been configured by WPS. Any change of the setting ' ;
var util_gw_wps_warn4 = 'The WPA Enterprise Authentication cannot be supported by WPS. ';
var util_gw_wps_warn5 = 'The 802.1x Authentication cannot be supported by WPS. ';
var util_gw_wps_warn6 = 'WDS mode cannot be supported by WPS. ';
var util_gw_wps_warn7 = 'Adhoc Client mode cannot be supported by WPS. ';
var util_gw_wps_cause_disconn = 'May cause stations to be disconnected. ';
var util_gw_wps_want_to = 'Are you sure you want to continue with the new setting?';
var util_gw_wps_cause_disabled = 'Using this configuration will cause WPS to be disabled. ';
var util_gw_wps_ecrypt_11n = 'Invalid Encryption Mode! WPA or WPA2, Cipher suite AES should be used for 802.11n band.';
var util_gw_wps_ecrypt_basic = 'The Encryption Mode is not suitable for the 802.11n band, please modify wlan encrypt settings or it will not function properly.';
var util_gw_wps_ecrypt_confirm = 'Are you sure you want to continue with this encryption mode for the 11n band? This may cause some  performance loss!';
var util_gw_ssid_hidden_alert = "if hiddenSSID is turned on; WPS2.0 will be disabled";
var util_gw_ssid_empty = 'SSID cannot be empty';
var util_gw_preshared_key_length_alert =  'Pre-Shared Key value should be 64 characters.';
var util_gw_preshared_key_alert = "Invalid Pre-Shared Key value. It should be in hex number (0-9 or a-f).";
var util_gw_preshared_key_min_alert = 'Pre-Shared Key value should be set at least 8 characters.';
var util_gw_preshared_key_max_alert = 'Pre-Shared Key value should be less than 64 characters.';
var util_gw_decimal_rang = 'It should be a decimal number between 1-65535.';
var util_gw_invalid_radius_port = 'Invalid port number of RADIUS Server! ';
var util_gw_empty_radius_port = "RADIUS Server port number cannot be empty! ";
var util_gw_invalid_radius_ip = 'Invalid RADIUS Server IP address';
var util_gw_mask_empty = 'Subnet mask cannot be empty! ';
var util_gw_ip_format = 'It should be filled with four 3-digit numbers, i.e., xxx.xxx.xxx.xxx.';
var util_gw_mask_rang = '\nIt should be the number of 0, 128, 192, 224, 240, 248, 252 or 254';
var util_gw_mask_rang1 = '\nIt should be the number of 128, 192, 224, 240, 248, 252 or 254';
var util_gw_mask_invalid1 = 'Invalid subnet mask in 1st digit.';
var util_gw_mask_invalid2 = 'Invalid subnet mask in 2nd digit.';
var util_gw_mask_invalid3 = 'Invalid subnet mask in 3rd digit.';
var util_gw_mask_invalid4 = 'Invalid subnet mask in 4th digit.';
var util_gw_mask_invalid = 'Invalid subnet mask value. ';
var util_gw_decimal_value_rang = "It should be the decimal number (0-9).";
var util_gw_invalid_degw_ip = 'Invalid default gateway address';
var util_gw_invalid_gw_ip = 'Invalid gateway address!';
var util_gw_locat_subnet = '\nIt should be located in the same subnet of current IP address.';
var util_gw_mac_complete = 'Input MAC address is not complete. ';
var util_gw_mac_empty = 'Input MAC address is empty! ';
var util_gw_mac_zero = 'Input MAC address can not be all zero! ';
var util_gw_mac_ff = 'Input MAC address can not be all FF! ';
var util_gw_12hex = 'It should be 12 digits in hex.';
var util_gw_invalid_mac = 'Invalid MAC address. ';
var util_gw_hex_rang = 'It should be in hex number (0-9 or a-f).';
var util_gw_ip_empty = 'IP address cannot be empty! ';
var util_gw_invalid_value = ' value. ';
var util_gw_should_be = 'It should be ';
var util_gw_check_ppp_rang1 = ' range in 1st digit. ';
var util_gw_check_ppp_rang2 = ' range in 2nd digit. ';
var util_gw_check_ppp_rang3 = ' range in 3rd digit. ';
var util_gw_check_ppp_rang4 = ' range in 4th digit. ';
var util_gw_check_ppp_rang5 = ' range in 5th digit. ';
var util_gw_invalid_key_length = 'Invalid length of Key ';
var util_gw_char = ' characters.';
var util_gw_invalid_wep_key_value = 'Invalid length of WEP Key value. ';
var util_gw_invalid_key_value = 'Invalid key value. ';
var util_gw_invalid_ip = 'Invalid IP address';
var util_gw_ipaddr_empty = 'IP address cannot be empty! It should be filled with four 3-digit numbers, i.e., xxx.xxx.xxx.xxx.';
var util_gw_ipaddr_nodecimal = ' value. It should be decimal numbers (0-9).';
var util_gw_ipaddr_1strange = ' range in 1st digit. It should be 1-223 except 127.';
var util_gw_ipaddr_2ndrange = ' range in 2nd digit. It should be 0-255.';
var util_gw_ipaddr_3rdrange = ' range in 3rd digit. It should be 0-255.';
var util_gw_ipaddr_4thrange = ' range in 4th digit. It should be 1-254.';

var util_gw_array0 = "(GMT-12:00)Eniwetok, Kwajalein";
var util_gw_array1 = "(GMT-11:00)Midway Island, Samoa";
var util_gw_array2 = "(GMT-10:00)Hawaii";
var util_gw_array3 = "(GMT-09:00)Alaska";
var util_gw_array4 = "(GMT-08:00)Pacific Time (US & Canada); Tijuana";
var util_gw_array5 = "(GMT-07:00)Arizona";
var util_gw_array6 = "(GMT-07:00)Mountain Time (US & Canada)";
var util_gw_array7 = "(GMT-06:00)Central Time (US & Canada)";
var util_gw_array8 = "(GMT-06:00)Mexico City, Tegucigalpa";
var util_gw_array9 = "(GMT-06:00)Saskatchewan";
var util_gw_array10 = "(GMT-05:00)Bogota, Lima, Quito";
var util_gw_array11 = "(GMT-05:00)Eastern Time (US & Canada)";
var util_gw_array12 = "(GMT-05:00)Indiana (East)";
var util_gw_array13 = "(GMT-04:00)Atlantic Time (Canada)";
var util_gw_array14 = "(GMT-04:00)Caracas, La Paz";
var util_gw_array15 = "(GMT-04:00)Santiago";
var util_gw_array16 = "(GMT-03:30)Newfoundland";
var util_gw_array17 = "(GMT-03:00)Brasilia";
var util_gw_array18 = "(GMT-03:00)Buenos Aires, Georgetown";
var util_gw_array19 = "(GMT-02:00)Mid-Atlantic";
var util_gw_array20 = "(GMT-01:00)Azores, Cape Verde Is.";
var util_gw_array21 = "(GMT)Casablanca, Monrovia";
var util_gw_array22 = "(GMT)Greenwich Mean Time: Dublin, Edinburgh, Lisbon, London";
var util_gw_array23 = "(GMT+01:00)Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna";
var util_gw_array24 = "(GMT+01:00)Belgrade, Bratislava, Budapest, Ljubljana, Prague";
var util_gw_array25 = "(GMT+01:00)Barcelona, Madrid";
var util_gw_array26 = "(GMT+01:00)Brussels, Copenhagen, Madrid, Paris, Vilnius";
var util_gw_array27 = "(GMT+01:00)Paris";
var util_gw_array28 = "(GMT+01:00)Sarajevo, Skopje, Sofija, Warsaw, Zagreb";
var util_gw_array29 = "(GMT+02:00)Athens, Istanbul, Minsk";
var util_gw_array30 = "(GMT+02:00)Bucharest";
var util_gw_array31 = "(GMT+02:00)Cairo";
var util_gw_array32 = "(GMT+02:00)Harare, Pretoria";
var util_gw_array33 = "(GMT+02:00)Helsinki, Riga, Tallinn";
var util_gw_array34 = "(GMT+02:00)Jerusalem";
var util_gw_array35 = "(GMT+03:00)Baghdad, Kuwait, Riyadh";
var util_gw_array36 = "(GMT+03:00)Moscow, St. Petersburg, Volgograd";
var util_gw_array37 = "(GMT+03:00)Mairobi";
var util_gw_array38 = "(GMT+03:30)Tehran";
var util_gw_array39 = "(GMT+04:00)Abu Dhabi, Muscat";
var util_gw_array40 = "(GMT+04:00)Baku, Tbilisi";
var util_gw_array41 = "(GMT+04:30)Kabul";
var util_gw_array42 = "(GMT+05:00)Ekaterinburg";
var util_gw_array43 = "(GMT+05:00)Islamabad, Karachi, Tashkent";
var util_gw_array44 = "(GMT+05:30)Bombay, Calcutta, Madras, New Delhi";
var util_gw_array45 = "(GMT+06:00)Astana, Almaty, Dhaka";
var util_gw_array46 = "(GMT+06:00)Colombo";
var util_gw_array47 = "(GMT+07:00)Bangkok, Hanoi, Jakarta";
var util_gw_array48 = "(GMT+08:00)Beijing, Chongqing, Hong Kong, Urumqi";
var util_gw_array49 = "(GMT+08:00)Perth";
var util_gw_array50 = "(GMT+08:00)Singapore";
var util_gw_array51 = "(GMT+08:00)Taipei";
var util_gw_array52 = "(GMT+09:00)Osaka, Sapporo, Tokyo";
var util_gw_array53 = "(GMT+09:00)Seoul";
var util_gw_array54 = "(GMT+09:00)Yakutsk";
var util_gw_array55 = "(GMT+09:30)Adelaide";
var util_gw_array56 = "(GMT+09:30)Darwin";
var util_gw_array57 = "(GMT+10:00)Brisbane";
var util_gw_array58 = "(GMT+10:00)Canberra, Melbourne, Sydney";
var util_gw_array59 = "(GMT+10:00)Guam, Port Moresby";
var util_gw_array60 = "(GMT+10:00)Hobart";
var util_gw_array61 = "(GMT+10:00)Vladivostok";
var util_gw_array62 = "(GMT+11:00)Magadan, Solomon Is., New Caledonia";
var util_gw_array63 = "(GMT+12:00)Auckland, Wllington";
var util_gw_array64 = "(GMT+12:00)Fiji, Kamchatka, Marshall Is.";

var util_gw_chanauto = 'Auto';
var uyi_gw_chan_dfsauto = 'Auto(DFS)';
var util_gw_bcast_mcast_mac = 'It should not be a broadcast or multicast mac address!';

/***********	status.htm ************/
var status_ip = 'IP Address';
var status_mac = 'MAC Address';
var status_subnet_mask = 'Subnet Mask';
var status_default_gw = 'Default Gataway';
var status_attain_ip = 'Attain IP Protocol';
var status_ipv6_global_ip = 'Global Address';
var status_ipv6_ll = 'LL Address';
var status_ipv6_link = 'Link Type';
var status_ipv6_conn = 'Connection Type';
var status_header = 'Access Point Status';
var status_explain = ' This page shows the current status and some basic settings of the device.';
var status_wan_config = "WAN Configuration";
var status_ipv6_lan = 'LAN IPv6 Configuration';
var status_ipv6_wan = 'WAN IPv6 Configuration';
var status_sys = 'System';
var status_uptime = 'Uptime';
var status_fw_ver = 'Firmware Version';
var status_build_time = 'Build Time';
var status_wl = 'Wireless ';
var status_config = 'Configuration';
var status_client_mode_inf = "Infrastructure Client";
var status_client_mode_adhoc = "Ad-hoc Client";
var status_ap = 'AP';
var status_wds = 'WDS';
var status_mesh = 'mesh';
var status_mpp = 'MPP';
var status_map = 'MAP';
var status_mp = 'MP';
var status_band = 'Band';
var status_ssid = 'SSID';
var status_channel_num = 'Channel Number';
var status_encrypt = 'Encryption';
var status_bssid = 'BSSID';
var status_assoc_cli = 'Associated Clients';
var status_state = 'State';
var status_vir_ap  = 'Virtual AP';
var status_repater_config = " Repeater Interface Configuration";
var status_tcpip_config = 'TCP/IP Configuration';
var status_dhcp_server = 'DHCP Server';
var status_disabled = 'Disabled';
var status_enabled = 'Enabled';
var status_auto = 'Auto';
var status_unknown = 'Unknown';
var status_dhcp_get_ip = 'Getting IP from DHCP server...';
var status_conn = 'Connected';
var status_disconn = 'Disconneted';
var status_fixed = 'Fixed IP ';
var status_start = 'Started';
var status_idle = 'Idle';
var status_wait_key = 'Waiting for keys';
var status_scan = 'Scanning';
var status_mode = 'Mode';
/***********	stats.htm ************/
var stats_header = 'Statistics';
var stats_explain = 'This page shows the packet counters for transmission and reception pertaining to wireless and Ethernet networks.';
var stats_lan = 'LAN';
var stats_send = 'Sent Packets';
var stats_recv = 'Received Packets';
var stats_repeater = 'Repeater';
var stats_eth = 'Ethernet';
var stats_wan = 'WAN';
var stats_refresh = 'Refresh';




/***********	wlwdstbl.htm ************/
var wlwdstbl_header = 'WDS AP Table';
var wlwdstbl_wlan = "wlan";
var wlwdstbl_explain = 'This table shows the MAC address, transmission, reception packet counters, and state information for each configured WDS AP.';
var wlwdstbl_mac = 'MAC Address';
var wlwdstbl_tx_pkt = 'Tx Packets';
var wlwdstbl_tx_err = 'Tx Errors';
var wlwdstbl_tx_rate = 'Tx Rate (Mbps)';
var wlwdstbl_rx_pkt = 'Rx Packets';
var wlwdstbl_refresh = 'Refresh';
var wlwdstbl_close = 'Close';
/***********	wlwdsenp.htm ************/
var wlwdsenp_hex = 'HEX';
var wlwdsenp_char = 'characters';
var wlwdsenp_header = 'WDS Security Setup';
var wlwdsenp_wlan = 'wlan';
var wlwdsenp_explain = 'This page allows you to setup the wireless security for WDS. When enabled, you must make sure each WDS device has adopted the same encryption algorithm and Key.';
var wlwdsenp_wep_key_format = 'WEP Key Format:';
var wlwdsenp_encrypt = 'Encryption:';
var wlwdsenp_wep_key = 'WEP Key:';
var wlwdsenp_prekey_format = 'Pre-Shared Key Format:';
var wlwdsenp_prekey = 'Pre-Shared Key:';
var wlwdsenp_none = 'None';
var wlwdsenp_pass = 'Passphrase';
var wlwdsenp_bits = 'bits';
var wlwdsenp_apply = "Apply Changes";
var wlwdsenp_reset = 'Reset';
/***********	tcpip_staticdhcp.htm ************/
var tcpip_dhcp_del_select = 'Do you really want to delete the selected entry?';
var tcpip_dhcp_del_all = 'Do you really want to delete all entries?';
var tcpip_dhcp_header = 'Static DHCP Setup';
var tcpip_dhcp_explain = 'This page allows you reserve IP addresses and assign the same IP address to a network device with a specified MAC address each time it requests an IP address. This is similar to having a static IP address except that the device must still request an IP address from the DHCP server. ';
var tcpip_dhcp_st_enabled = 'Enable Static DHCP';
var tcpip_dhcp_comment = 'Comment';
var tcpip_dhcp_list = 'Static DHCP List:';
var tcpip_dhcp_apply = "Apply Changes";
var tcpip_dhcp_reset = 'Reset';
var tcpip_dhcp_delsel = 'Delete Selected';
var tcpip_dhcp_delall = 'Delete All';
var tcpip_dhcp_select = 'Select';

/***********	wizard.htm ************/
var wizard_header = 'Setup Wizard';
var wizard_header_explain = 'The setup wizard will guide  you to configure the Access Point for the first time. Follow the setup wizard step by step.';
var wizard_welcome = 'Welcome to the Setup Wizard.';
var wizard_content_explain = 'The Wizard will guide you through the following steps. Begin by clicking on  Next.';
var wizard_content1 = 'Setup Operating Mode';
var wizard_content2 = 'Choose your Time Zone';
var wizard_content3 = 'Setup LAN Interface';
var wizard_content4 = 'Setup WAN Interface';
var wizard_content5 = 'Select Wireless Band';
var wizard_content6 = 'Wireless Basic Setting';
var wizard_content7 = 'Wireless Security Setting';
var wizard_next_btn = '  Next>>';
var wizard_back_btn = '<<Back  ';
var wizard_cancel_btn = '  Cancel  ';
var wizard_finish_btn = 'Finished';

var wizard_opmode_invalid = 'Invalid opmode value ';
var wizard_chanset_wrong = 'Wrong field input!';
var wizard_wantypeselect = 'Error! Your browser must have CSS support!';
var wizard_weplen_error = 'Invalid mib value length0';

var wizard_content5_explain = 'You can select the Wireless Band';
var wizard_wire_band = 'Wireless Band:';

var wizard_basic_header_explain = ' This page is used to configure the parameters for wireless LAN clients that may connect to your Access Point.';
var wizard_wlan1_div0_mode = 'Mode:';
var wizard_chan_auto = 'Auto';
var wizard_client = 'Client';

var wizard_wpa_tkip = 'WPA (TKIP)';
var wizard_wpa_aes = 'WPA (AES)';
var wizard_wpa2_aes = 'WPA2(AES)';
var wizard_wpa2_mixed = 'WPA2 Mixed';
var wizard_use_cert_from_remote_as0 = 'Use Cert from Remote AS0';
var wizard_use_cert_from_remote_as1 = 'Use Cert from Remote AS1';

var wizard_5G_basic = 'Wireless 5GHz Basic Settings';
var wizard_5G_sec = 'Wireless 5G Security Setup';
var wizard_2G_basic = 'Wireless 2.4GHz Basic Settings';
var wizard_2G_sec = 'Wireless 2.4G Security Setup';

/***********	route.htm ************/
var route_header = 'Routing Setup';
var route_header_explain = 'This page is used to setup dynamic routing protocol or edit static route entry.';
var route_enable = 'Enabled';
var route_disable = 'Disabled';
var route_apply = 'Apply Changes';
var route_reset = 'Reset';

var route_dynamic = 'Enable Dynamic Route';
var route_nat = 'NAT:';
var route_rip = 'RIP:';
var route_rip1 = 'RIPv1';
var route_rip2 = 'RIPv2';
var route_rip6 = 'RIPng:';
var route_static = 'Enable Static Route';
var route_ipaddr = 'IP Address:';
var route_mask = 'Subnet Mask:';
var route_gateway = 'Gateway:';
var route_metric = 'Metric:';
var route_interface = 'Interface:';
var route_lan = 'LAN';
var route_wan = 'WAN';
var route_showtbl = 'Show Route Table';

var route_static_tbl = 'Static Route Table:';

var route_tbl_destip = 'Destination IP Address';
var route_tbl_mask = 'Netmask';
var route_tbl_gateway = 'Gateway';
var route_tbl_metric = 'Metric';
var route_tbl_inter = 'Interface';
var route_tbl_select = 'Select';

var route_deletechick_warn = 'Do you really want to delete the selected entry?';
var route_deleteall_warn = 'Do you really want to delete all entries?';
var route_deletechick = 'Delete Selected';
var route_deleteall = 'Delete All';

var route_addchick0 = 'Invalid IP address ';
var route_addchick1 = 'Invalid IP address value! ';
var route_addchick2 = 'Invalid Gateway address! ';
var route_addchick3 = 'Invalid metric value! The range is 1 ~ 15';
var route_checkip1 = 'IP address cannot be empty! It should be filled with four 3-digit numbers, i.e., xxx.xxx.xxx.xxx.';
var route_checkip2 = ' value. It should be decimal numbers (0-9).';
var route_checkip3 = ' range in 1st digit. It should be 1-223.';
var route_checkip4 = ' range in 2nd digit. It should be 0-255.';
var route_checkip5 = ' range in 3rd digit. It should be 0-255.';
var route_checkip6 = ' range in 4th digit. It should be 0-255.';
var route_validnum = 'Invalid value. It should be in decimal number (0-9).';
var route_setrip = 'You can not disable NAT in PPP wan type!';

/***********	routetbl.htm ************/
var routetbl_header = 'Routing  Table ';
var routetbl_header_explain = ' This table shows all routing entries:';
var routetbl_refresh = 'Refresh';
var routetbl_close = ' Close ';
var routetbl_dst = 'Destination';
var routetbl_gw = 'Gateway';
var routetbl_mask = 'Genmask';
var routetbl_flag = 'Flags';
var routetbl_iface = 'Interface';
var routetbl_type = 'Type';

/***********	vlan.htm ************/
var vlan_header = 'VLAN Settings';
var vlan_header_explain = 'Entries in below table are used to configure vlan settings. VLANs are created to provide the segmentation services traditionally provided by routers. VLANs address issues such as scalability, security, and network management.';
var vlan_apply = 'Apply Changes';
var vlan_reset = 'Reset';

var vlan_enable = 'Enable VLAN';
var vlan_id = 'VLAN ID:';
var vlan_forwardrule = 'Forward Rule:';
var vlan_forwardrulenat = 'NAT';
var vlan_forwardrulebridge = 'Bridge';
var vlan_tagtbl = 'Tag Table';
var vlan_tagtbl_interface = 'interface name';
var vlan_tagtbl_taged = 'tagged';
var vlan_tagtbl_untaged = 'unTagged';
var vlan_tagtbl_notin = 'not in this vlan';

var vlan_settbl = 'Current VLAN setting table';
var vlan_settbl_id = 'VlanID';
var vlan_settbl_taged = 'tagged interface';
var vlan_settbl_untaged = 'untagged interface';
var vlan_settbl_forwardrule = 'forward rule';
var vlan_settbl_modify = 'modify';
var vlan_settbl_select = 'select';
var vlan_deletechick = 'Delete Selected';
var vlan_deleteall = 'Delete All';

var vlan_nettbl = 'Current net interface setting table';
var vlan_nettbl_name = 'interface name';
var vlan_nettbl_pvid = 'Pvid';
var vlan_nettbl_defprio = 'default Priority';
var vlan_nettbl_defcfi = 'default Cfi';

var vlan_checkadd1 = 'invalid vlan id,must be between 1-4094';
var vlan_checkadd2 = 'At least one interface should bind to this vlan!';
var vlan_deletesel = 'Do you really want to delete the selected entry?';
var vlan_deleteall_conf = 'Do you really want to delete all entries?';


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

var switch_ui_mode2adv  = 'switch to advanced mode';
var switch_ui_mode2easy = 'switch to easy mode';


/***********	OK_MSG ************/
var okmsg_explain = "<body><blockquote><h4>Changed setting successfully!</h4>Your changes have been saved. The router must be rebooted for the changes to take effect.<br> You can reboot now, or you can continue to make other changes and reboot later.\n";
var okmsg_reboot_now = 'Reboot Now';
var okmsg_reboot_later = 'Reboot Later';

var okmsg_btn = '  OK  ';

var okmsg_fw_saveconf = 'Reloaded settings successfully!<br><br>The Router is booting.<br>Do not turn off or reboot the Device during this time.<br>';
var okmsg_fw_opmode = 'Set Operating Mode successfully!';
var okmsg_fw_passwd = 'Changed setting successfully!<br><br>Do not turn off or reboot the Router during this time.';

/*********** common msg ************/
var ip_should_in_current_subnet = 'Invalid IP address! It should be set within the current lan subnet.';
var ip_should_be_different_from_lanip = 'Invalid IP address! It should be different from lan ip address.';

function dw(str)
{
	document.write(str);
}

