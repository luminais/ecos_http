var	menuObj = {},
	helpinfos = [
'system_status.asp',
	'<p><em>' + _("Connection Status") + ':</em>' + _("System_status_helpinfo1") + '</p>' +
	'<p><em>' + _("Internet Connection Type") + ':</em>' + _("System_status_helpinfo2") + '</p>' +
	'<p><em>' + _("Connection Time") + ':</em>' + _("System_status_helpinfo3") + '</p>' +
	'<p><em>' + _("System Version") + ':</em>' + _("System_status_helpinfo4") + '</p>',
'wan_connected.asp',
	'<p id="help-pppoe"><em>PPPoE:</em>' + _("Wan_connected_helpinfo3") + '</p>' +
	'<p id="help-static"><em>' + _("Static IP") + ':</em>' + _("Wan_connected_helpinfo1") + '</p>' +
	'<p id="help-dhcp"><em>DHCP:</em>' + _("Wan_connected_helpinfo2") + '</p>' +
	'<p id="help-pptp"><em>PPTP:</em>' + _("Wan_connected_helpinfo4") + '</p>' +
	'<p id="help-l2tp"><em>L2TP:</em>' + _("Wan_connected_helpinfo5") + '</p>' +
	'<p>' + _("Contact your ISP for help if you are not sure about which Internet connection type to use.") + '</p>',
'lan.asp',
	'<p>' + _("Lan_helpinfo") + '</p>',
'mac_clone.asp',
	'<p>' + _("mac_clone_helpinfo1") + '</p>' +
	'<p><em>' + _("MAC Address") + ':</em>' + _("mac_clone_helpinfo2") + '</p>' +
	'<p><em>' + _("Restore Default MAC") + ':</em>' + _("mac_clone_helpinfo3") + '</p>' +
	'<p><em>' + _("Clone MAC Address") + ':</em>' + _("mac_clone_helpinfo4") + '</p>',
'wan_dns.asp',
	'<p>' + _("Wan_dns_helpinfo1") + '</p>' +
	'<p>' + _("Wan_dns_helpinfo2") + '</p>',
'net_tc.asp',
	'<p>' + _("Net_tc_helpinfo1") + '</p>' +
	'<p><em>' + _("Upload/Download") + ':</em>' + _("Net_tc_helpinfo2") + '</p>' +
	'<p><em>' + _("Bandwidth Range") + ':</em>' + _("Net_tc_helpinfo3") + '</p>' +
	'<p><em>' + _("Note") + ':</em>' + _("Net_tc_helpinfo4") + '</p>' +
	'<p>1Mbps=128KB/s </p>' +
	'<p>10Mbps=1280KB/s</p>',
'sys_iptAccount.asp',
	'<p>' + _("Traffic statistics allows you to see at a glance how much traffic each device in your network is using.") + '</p>',
'wan_speed.asp',
	'<p>' + _("Wan_speed_helpinfo1") + '</p>',
'wireless_mode.asp',
	'<p><em>' + _("Wired WAN Mode") + '：</em>' + _("wireless_mode_helpinfo1") + '</p>' +
	'<p><em>' + _("Wireless WAN Mode") + '：</em>' + _("wireless_mode_helpinfo2") + '</p>' +
	'<p>' + _("To select an Internet connection type, go to Internet Connection Setup") + '.</p>',
'wireless_basic.asp',
	'<p>' + _("wireless_basic_helpinfo1") + '</p>' +
	'<p><em>SSID:</em>' + _("wireless_basic_helpinfo2") + '</p>' +
	'<p><em>' + _("SSID Broadcast") + ':</em>' + _("wireless_basic_helpinfo3") + '</p>' +
	'<p><em>' + _("Channel") + ':</em>' + _("wireless_basic_helpinfo4")+ '</p>' +
	'<p><em>' + _("Extension Channel") + ':</em>' + _("wireless_basic_helpinfo5")+ '</p>',
'wireless_security.asp',
  '<p>' + _("Wireless_secrity_helpinfo1") + '</p>' +
	'<p><em>' + _("WEP Key") + ':</em>' + _("Wireless_secrity_helpinfo2") + '</p>' +
	'<p><em>'+ _("WPA/WPA2-Personal") + ':</em>' + _("Wireless_secrity_helpinfo") + '</p>' +
	'<p><em>' + _("Security Key") + ':</em>' + _("Wireless_secrity_helpinfo3") + '</p>',			   
'wireless_filter.asp',
	'<p>' + _("Wireless_filter_helpinfo") + '</p>',
'wireless_state.asp',
	'<p>' + _("Wireless_state_helpinfo1") + '</p>' +
	'<p><em>' + _("Bandwidth") + ':</em>' + _("Wireless_state_helpinfo2") + '</p>',
'lan_dhcps.asp',
	'<p>' + _("Lan_dncps_helpinfo1") + '</p>' +
	'<p><em>' + _("IP Address Pool") + ':</em>' + _("Lan_dncps_helpinfo2") + '</p>' +
	'<p><em>' + _("Lease Time") + ':</em>' + _("Lan_dncps_helpinfo3") + '</p>',
'lan_dhcp_clients.asp',
	'<p>' + _("lan_dhcp_clients_helpinfo1") + '</p>' +
	'<p>' + _("lan_dhcp_clients_helpinfo2") + '</p>',
'nat_virtualportseg.asp',
	'<p>' + _("NAT_Portseg_Help_Inf5") + '</p>' +
	'<p><em>' + _("Start Port-End Port") + ':</em>' + _("NAT_Portseg_Help_Inf1") + '</p>' +
	'<p><em>' + _("Enable") + ':</em>' + _("NAT_Portseg_Help_Inf2") + '</p>' +
	'<p><em>' + _("Delete") + ':</em>' + _("NAT_Portseg_Help_Inf3") + '</p>' +
	'<p><em>' + _("Add to") + ':</em>' + _("NAT_Portseg_Help_Inf4") + '</p>',
'nat_dmz.asp',
	'<p>' + _("DMZ_Help_Inf") + '</p>' +
	'<p><em>' + _("DMZ Host IP Address") + ':</em>' + _("DMZ_Help_Inf1") + '</p>',
'upnp_config.asp',
	'<p>' + _("UPNP_Help_Inf") + '</p>',
'firewall_clientfilter.asp',
	'<p>' + _("Firewall_clientfilter_Help_Inf1") + '</p>' +
	'<p>' + _("Firewall_clientfilter_Help_Inf2") + '</p>',
'firewall_mac.asp',
	'<p>' + _("Firewall_MAC_Help_Inf1") + '</p>' +
	'<p>' + _("Firewall_MAC_Help_Inf2") + '</p>',
'firewall_urlfilter.asp'  ,
	'<p>' + _("Firewall_URLFilter_Help_Inf1") + '</p>' +
	'<p>' + _("Firewall_URLFilter_Help_Inf2") + '</p>' +
	'<p>' + _("Firewall_URLFilter_Help_Inf3") + '</p>',
'system_remote.asp',
	'<p>' + _("remote_Help_Inf1") + '</p>' +
	'<p><em>' + _("Port") + ':</em>' + _("remote_Help_Inf2") + '</p>' +
	'<p><em>' + _("IP Address") + ':</em>' + _("remote_Help_Inf3") + '</p>',
'routing_table.asp',
	'<p>' + _("routing_table_Help_Inf1") + '</p>',
'routing_static.asp',
	'<p><em>' + _("Static Routing") + ':</em>' + _("routing_static_Help_Inf1") + '</p>' +
	'<p>' + _("routing_static_Help_Inf2") + '</p>',
'system_hostname.asp',
	'<p>' + _("system_hostname_Help_Inf1") + '</p>' +
	'<p><em>' + _("Note") + ':</em>' + _("system_hostname_Help_Inf2") + '</p>',
'ddns_config.asp',
	'<p>' + _("ddns_config_Help_Inf1") + '</p>' +
  '<p><em>' + _("DDNS Service") + ':</em>' + _("ddns_config_Help_Inf2") + '</p>',
'system_backup.asp',
	'<p><em>' + _("Backup") + ':</em>' + _("system_backup_Help_Inf") + '</p>' +
	'<p><em>' + _("Restore") + ':</em>' + _("system_backup_Help_Inf1") + '</p>',
'system_restore.asp',
	'<p>' + _("system_restore_Help_Inf1") +'</p>' +
	'<p><em>' + _("Default Password") + ':</em>admin</p>' +
	'<p><em>' + _("Default IP Address") + ':</em>192.168.0.1</p>' +
	'<p><em>' + _("Default Subnet Mask") + ':</em>255.255.255.0</p>',
'system_upgrade.asp',
	'<p>' + _("system_upgrade_Help_Inf1") + '</p>' +
	'<p>' + _("system_upgrade_Help_Inf2") + '</p>' +
	'<p>' + _("system_upgrade_Help_Inf3") + '</p>',
'system_reboot.asp',
	'<p>' + _("system_reboot_Help_Inf") + '</p>',
'system_password.asp',
	'<p>' + _("system_password_Help_Inf1") + '</p>' +
	'<p>' + _("system_password_Help_Inf2") + '</p>' +
	'<p><em>' + _("Old Password") + ':</em>' + _("system_password_Help_Inf3") + '</p>' +
	'<p><em>' + _("New Password") + ':</em>' + _("system_password_Help_Inf4") + '</p>' +
	'<p><em>' + _("Confirm New Password") + ':</em>' + _("system_password_Help_Inf5") + '</p>',
'system_log.asp',
	'<p>' + _("system_log_Help_Inf1") + '</p>'
];

menuObj["advance"] 	= ["system_status.asp;" + _("Status"), "wan_connected.asp;" + _("Internet Connection Setup"),
		"mac_clone.asp;" + _("MAC Clone"), "wan_speed.asp;" + _("WAN Speed"), "wireless_mode.asp;" + _("WAN Medium Type"), "lan.asp;" + _("LAN Settings"),
		"wan_dns.asp;" + _("DNS Settings"), "lan_dhcps.asp;" + _("DHCP Server"),
		"lan_dhcp_clients.asp;" + _("DHCP Client List")];
		
menuObj["wireless"] = ["wireless_basic.asp;" + _("Wireless Basic Settings"),
		"wireless_security.asp;" + _("Wireless Security"),
		"wireless_filter.asp;" + _("Access Control"), "wireless_state.asp;" + _("Wireless Connection Status")];
		
menuObj["bandwidth"] = ["net_tc.asp;" + _("Bandwidth Control"), "sys_iptAccount.asp;" + _("Traffic Statistics")];

menuObj["specific"] = ["nat_virtualportseg.asp;" + _("Port Range Forwarding"), "nat_dmz.asp;" + _("DMZ Host"),
		"ddns_config.asp;DDNS", "upnp_config.asp;" + _("UPNP Settings"), "routing_static.asp;" + _("Static Routing"),
		"routing_table.asp;" + _("Routing Table")];
		
menuObj["behave"] 	= ["firewall_urlfilter.asp;" + _("URL Filter Settings"),
		"firewall_mac.asp;" + _("MAC Address Filter Settings"),
		"firewall_clientfilter.asp;" + _("Client Filter Settings")];
		
menuObj["system"] 	= ["system_reboot.asp;" + _("Reboot"),
		"system_restore.asp;" + _(" Restore To Factory Default"),
		"system_backup.asp;" + _("Backup/Restore"), "system_log.asp;" + _("Syslog"),
		"system_remote.asp;" + _("Remote Web Management"),
		"system_hostname.asp;" + _("Time Settings"), "system_password.asp;" + _("Change Password"),
		"system_upgrade.asp;" + _("Upgrade")];