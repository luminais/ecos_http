<!DOCTYPE html>
<html class="index-html">
<head>
	<meta charset="utf-8" />
	<meta http-equiv="Pragma" content="no-cache">
	<title>Home</title>
	<link rel="stylesheet" href="css/screen.css" />
</head>
<body class="index-body">
	<div class="navbar">
		<div class="navbar-inner">
			<a class="brand" href="index.asp"></a>
		</div>
	</div>
	
	<div class="container">
		<form name="basicset" action="/goform/WizardHandle" method="post">
			<input type=hidden name=MACC />
			<input type="hidden" name="GO" value="advance.asp">
			<input type="hidden" id="v12_time" name="v12_time">
			<input type="hidden" name="WANT1" id="WANT1">
			<fieldset>
				<legend>Internet Connection Setup</legend>
				<div class="control-group">
					<label class="control-label">Internet Connection Type</label>
					<div class="controls">
						<label class="radio" for="adsl">
							<input type="radio" hidefocus="true" value="2" name="net_type" checked="checked" id="adsl"/>PPPoE
						</label>
						<label class="radio" for="atuo">
							<input type="radio" hidefocus="true" value="0" name="net_type" id="atuo" />DHCP
						</label>
					</div>
				</div>
				<div id="ppoe_set">
					<div class="control-group">
						<label for="PUN" class="control-label">PPPoE Username</label>
						<div class="controls">
							<input type="text" name='PUN' class="text" id="PUN" maxlength="128" />
						</div>
					</div>
					<div class="control-group">
						<label for="PPW" class="control-label">PPPoE Password</label>
						<div class="controls">
							<input type="password" name='PPW' class="text" id="PPW" maxlength="128"/>
						</div>
					</div>
				</div>
				<p><span>For other connection types, click</span>&nbsp;"<a href="advance.asp" class="text-red">Advanced</a>"</p>
			</fieldset>
			
			<fieldset>
				<legend>Wireless Security Setup</legend>
				<div class="control-group">
					<label for="wirelesspassword" class="control-label">Security Key</label>
					<div class="controls">
						<input type="password" name="wirelesspassword" class="text" id="wirelesspassword" maxlength="63"/>
						
					</div>
				</div>
			</fieldset>
			<div id="submitOperation" class="btn-group">
				<a href="#" class="btn btn-link" id="submit_ok">OK</a>
				<a href="#" class="btn btn-link" id="submit_cancel">Cancel</a>
			</div>
		</form>
	</div>

<script src="lang/b28n.js" type="text/javascript"></script>
<script>
var def_PUN = '<%aspTendaGetStatus("ppoe","userid");%>',
	def_PPW = '<%aspTendaGetStatus("ppoe","pwd");%>',
	def_WANT = '<%aspTendaGetStatus("wan","connecttype");%>',
	def_wirelesspassword = "<%get_wireless_password("wireless","wirelesspassword");%>",
	cln_MAC = "<%aspTendaGetStatus("sys","manmac");%>",
	def_MAC = "<%aspTendaGetStatus("sys","wanmac");%>",
	fac_MAC = "<%aspTendaGetStatus("sys","fmac");%>",
	wl_MAC_list = new Array(<%get_wireless_station("macinfo");%>),
	cloneway="<%aspTendaGetStatus("sys","clnway");%>",
	config_num="<%aspTendaGetStatus("sys","config_num");%>",
	def_wps = '<%get_wireless_password("wireless","wpsen");%>',
	ssid000 = "<% get_wireless_basic("SSID"); %>";

	//handle translate
	(function() {
		B.setTextDomain("base");
		B.translate();
	})();
</script>
<script src="js/libs/tenda.js" type="text/javascript"></script>
<script src="js/gozila.js" type="text/javascript"></script>
<script src="js/index.js" type="text/javascript"></script>
</body>
</html>