<!DOCTYPE html>
<html class="advance-html">
<head>
<meta charset="utf-8">
<title>Tenda 11N Wireless Router</title>
<link rel="stylesheet" type="text/css" href="css/screen.css" />
</head>
<body class="advance-body">
<div id="main">
	<div class="navbar">
    <div id="menu" class="navbar-inner">
        <a href="#" class="brand"></a>
        <ul id="menu_ul" class="nav">
			<li>
				<a id="system" target="mainFrame" href="system_reboot.asp">
                    <span class="nav-left">&nbsp;</span>
                    <span class="nav-text">Tools</span>
                    <span class="nav-right">&nbsp;</span>
               	</a>
			</li>
			<li>
            	<a  id="behave" target="mainFrame"  href="firewall_urlfilter.asp">
                	<span class="nav-left">&nbsp;</span>
                    <span class="nav-text">Security</span>
                    <span class="nav-right">&nbsp;</span>
                </a>
            </li>
			<li>
            <a id="specific" target="mainFrame"  href="nat_virtualportseg.asp">
                	<span class="nav-left">&nbsp;</span>
                    <span class="nav-text nav-large">Applications</span>
                    <span class="nav-right">&nbsp;</span>
             </a></li>
			<li>
            	<a id="bandwidth" target="mainFrame" href="net_tc.asp">
            		<span class="nav-left">&nbsp;</span>
                    <span class="nav-text nav-mini">QoS</span>
                    <span class="nav-right">&nbsp;</span>
            	</a>
            </li>
			<li><a id="wireless" href="wireless_basic.asp" target="mainFrame" >
            		<span class="nav-left">&nbsp;</span>
                    <span class="nav-text">Wireless</span>
                    <span class="nav-right">&nbsp;</span>
            </a></li>
			<li><a class="active" id="advance" target="mainFrame" href="system_status.asp">
                	<span class="nav-left">&nbsp;</span>
                    <span class="nav-text">Advanced</span>
                    <span class="nav-right">&nbsp;</span>
            </a></li>
			<li><a id="index" href="index.asp">
            	<span class="nav-left">&nbsp;</span>
                <span class="nav-text">Home</span>
                <span class="nav-right">&nbsp;</span>
            </a></li>
       </ul>
    </div>
	</div>
	<div id="main_content" class="container">
		<div id="sub_menu">
			<ul class="nav-list">
				<li><a class="active" target="mainFrame" href="system_status.asp">Status</a></li>
				<li><a target="mainFrame" href="wan_connected.asp">Internet Connection Setup</a></li>
				<li><a target="mainFrame" href="mac_clone.asp">MAC Clone</a></li>
				<li><a target="mainFrame" href="wan_speed.asp">WAN Speed</a></li>
				<li><a target="mainFrame" href="lan.asp" >LAN Settings</a></li>
				<li><a target="mainFrame" href="wan_dns.asp">DNS Settings</a></li>
				<li><a target="mainFrame" href="lan_dhcps.asp">DHCP Server</a></li>
				<li><a target="mainFrame" href="lan_dhcp_clients.asp">DHCP Client List</a></li>
			</ul>
		</div>
		<div id="container">
			<img id="loading" src="img/loading.gif"/>
			<div id="container_main" class="container-main">
				<iframe id="main_iframe" name="mainFrame" frameborder="0" 
				scrolling="auto" framespacing="0" marginwidth="0" marginheight="0" 
				src="system_status.asp"></iframe>
			</div>
			<div id="help">
				<h2 class="help-title">Help</h2>
				<div id="help_text"></div>
			</div>
			<div class="cb"></div>
		</div>
  </div>
</div>
<div class="page-footer"></div>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
var highPower = '<%aspTendaGetStatus("wifi","HighPower");%>';
(function() {
	B.setTextDomain(["base", "config"]);
	var help_info_status = '<p><em>' + _("Connection Status") + ':</em>' + _("System_status_helpinfo1") + '</p>' +
			'<p><em>' + _("Internet Connection Type") + ':</em>' + _("System_status_helpinfo2") + '</p>' +
			'<p><em>' + _("Connection Time") + ':</em>' + _("System_status_helpinfo3") + '</p>' +
			'<p><em>' + _("System Version") + ':</em>' + _("System_status_helpinfo4") + '</p>';
	document.getElementById("help_text").innerHTML = help_info_status;
	B.translate();
})();
</script>
<script language=javascript src="js/libs/tenda.js"></script>
<script language=javascript src="js/config.js"></script>
<script language=javascript src="js/advance.js"></script>
</body>
</html>