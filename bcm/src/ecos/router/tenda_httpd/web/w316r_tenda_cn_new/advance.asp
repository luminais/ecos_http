<!DOCTYPE html>
<html class="advance-html">
<head>
<meta charset="utf-8">
<title>TENDA 11N无线路由器</title>
<link rel="stylesheet" type="text/css" href="css/screen.css" />
</head>
<body class="advance-body">
<div id="main">
	<div class="navbar">
    <div id="menu" class="navbar-inner">
        <a href="#" class="brand"></a>
        <ul id="menu_ul" class="nav">
			<li id="system"><a target="mainFrame" href="system_reboot.asp">系统工具</a></li>
			<li id="behave"><a target="mainFrame" href="firewall_urlfilter.asp">行为管理</a></li>
			<li id="specific"><a target="mainFrame"  href="nat_virtualportseg.asp">特殊应用</a></li>
			<li id="bandwidth"><a target="mainFrame" href="net_tc.asp">带宽控制</a></li>
			<li id="wireless"><a href="wireless_basic.asp" target="mainFrame" >无线设置</a></li>
			<li class="active" id="advance"><a target="mainFrame" href="system_status.asp">高级设置</a></li>
			<li id="index"><a href="index.asp">返回首页</a></li>
       </ul>
    </div>
	</div>
	<div id="main_content" class="container">
		<div id="sub_menu">
			<ul class="nav-list">
				<li><a class="active" target="mainFrame" href="system_status.asp">运行状态</a></li>
				<li><a target="mainFrame" href="wan_connected.asp">上网设置</a></li>
				<li><a target="mainFrame" href="mac_clone.asp">MAC克隆</a></li>
				<li><a target="mainFrame" href="wan_speed.asp">WAN速率控制</a></li>
				<li><a target="mainFrame" href="lan.asp" >LAN口设置</a></li>
				<li><a target="mainFrame" href="wan_dns.asp">DNS设置</a></li>
				<li><a target="mainFrame" href="lan_dhcps.asp">DHCP服务器</a></li>
				<li><a target="mainFrame" href="lan_dhcp_clients.asp">DHCP客户端列表</a></li>
			</ul>
		</div>
		<div id="container">
			<div id="container_main" class="container-main">
				<iframe name="mainFrame" id="main_iframe" src="system_status.asp" frameborder="0" scrolling="auto" marginwidth="0" framespacing="0" marginheight="0"></iframe>
			</div>
			<div id="help">
				<h2 class="help-title">帮助信息</h2>
				<div id="help_text">
					<p>如需返回首页，请点击返回首页</p>
					<p>如需修改上网方式及相关参数，请点击上网设置。</p>
					<p class="sina-weibo"><em>腾达新浪官方微博:</em><a href="http://e.weibo.com/100tenda" target="_blank" title="腾达新浪官方微博"></a></p>
				</div>
			</div>
			<div class="cb"></div>
		</div>
  </div>
</div>
<div class="page-footer"></div>
<script language=javascript src="js/libs/tenda.js"></script>
<script language=javascript src="js/gozila.js"></script>
<script language=javascript src="js/config.js"></script>
<script language=javascript src="js/advance.js"></script>
</body>
</html>