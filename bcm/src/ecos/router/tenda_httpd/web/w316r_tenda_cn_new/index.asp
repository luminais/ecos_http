<!DOCTYPE html>
<html class="index-html">
<head>
	<meta charset="utf-8" />
	<meta http-equiv="Pragma" content="no-cache">
	<title>首页</title>
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
				<legend>上网设置</legend>
				<div class="control-group">
					<label class="control-label">上网方式</label>
					<div class="controls">
						<label class="radio" for="adsl">
							<input type="radio" hidefocus="true" value="2" name="net_type" checked="checked" id="adsl"/>ADSL拨号
						</label>
						<label class="radio" for="atuo">
							<input type="radio" hidefocus="true" value="0" name="net_type" id="atuo" />自动获取
						</label>
					</div>
				</div>
				<div id="ppoe_set">
					<div class="control-group">
						<label for="PUN" class="control-label">宽带用户名</label>
						<div class="controls">
							<input type="text" name='PUN' class="text" id="PUN" maxlength="128" />
						</div>
					</div>
					<div class="control-group">
						<label for="PPW" class="control-label">宽带密码</label>
						<div class="controls">
							<input type="password" name='PPW' class="text" id="PPW" maxlength="128"/>
						</div>
					</div>
				</div>
				<p>其他上网类型请点击"<a href="advance.asp" class="text-red">高级设置</a>"</p>
			</fieldset>
			
			<fieldset>
				<legend>无线加密</legend>
				<div class="control-group">
					<label for="wirelesspassword" class="control-label">无线密码</label>
					<div class="controls">
						<input type="password" name="wirelesspassword" class="text" id="wirelesspassword" maxlength="63"/>
						<span class="help-block">默认无线密码（12345678）</span>
					</div>
				</div>
			</fieldset>
			<div id="submitOperation" class="btn-group">
				<a href="#" class="btn btn-link" id="submit_ok">确 定</a>
				<a href="#" class="btn btn-link" id="submit_cancel">取 消</a>
			</div>
		</form>
	</div>

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
</script>
<script src="js/libs/tenda.js" type="text/javascript"></script>
<script src="js/gozila.js" type="text/javascript"></script>
<script src="js/index.js" type="text/javascript"></script>
</body>
</html>