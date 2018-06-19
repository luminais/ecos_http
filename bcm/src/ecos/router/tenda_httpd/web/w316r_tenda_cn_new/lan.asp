<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>LAN | LAN Settings</title>
<link rel=stylesheet type="text/css" href="css/screen.css">
<script language=javascript src="js/libs/tenda.js"></script>
<script language=javascript src="js/gozila.js"></script>
</head>
<body class="bg" onLoad="init(document.LanSet);">
<form name="LanSet" method="POST" action="/goform/AdvSetLanip">
<input type=hidden name=GO value=lan.asp>
	<fieldset>
		<legend>LAN口设置</legend>
		<table class="content1">                       
			<tr>
				<td colspan=2 align="left" valign="top">本页设置LAN口的基本网络参数。</td>
			</tr>
		</table>
		<table class="content2" id="table1">
			<tr>
				<td class="control-label">MAC 地址</td>
				<td class="controls"><%aspTendaGetStatus("sys","lanmac");%></td>
			</tr>
			<tr>
				<td class="control-label">IP地址</td>
				<td class="controls"><input class=text maxlength=15 name=LANIP size=15 ></td>
			</tr>
			<tr>
				<td class="control-label">子网掩码</td>
				<td class="controls"><input class=text maxlength=15 name=LANMASK size=15></td>
			</tr>
		</table>
	</fieldset>
<script>tbl_tail_save("document.LanSet");</script>
</form>

<script language=JavaScript>
addCfg("LANIP",0,"<%aspTendaGetStatus("lan","lanip");%>");
addCfg("LANMASK",1,"<%aspTendaGetStatus("lan","lanmask");%>");
function same_net_with_lan(f){
	var new_mask = f.LANMASK.value.split(".");
	var new_ip = f.LANIP.value.split(".");

	var old_mask = getCfg("LANMASK").split(".");
	var old_ip = getCfg("LANIP").split(".");

	var i_new_mask = new_mask[0]<<24|new_mask[1]<<16|new_mask[2]<<8|new_mask[3];
	var i_new_ip = new_ip[0]<<24|new_ip[1]<<16|new_ip[2]<<8|new_ip[3];

	var i_old_mask = old_mask[0]<<24|old_mask[1]<<16|old_mask[2]<<8|old_mask[3];
	var i_old_ip = old_ip[0]<<24|old_ip[1]<<16|old_ip[2]<<8|old_ip[3];

	if((i_new_mask&i_new_ip) == (i_old_mask&i_old_ip))
		return true;
	else
		return false;
}

function init(f){	
	cfg2Form(f);
}

function preSubmit(f) {  
	var str="";
	if (!verifyIP2(f.LANIP,"IP 地址")) return ;
	//增加了支持子网划分，和网络聚集判断
	if ( !tenda_ipMskChk(f.LANMASK,"子网掩码", f.LANIP)) return ;	
	
	if(!same_net_with_lan(f)){
		str+="路由器IP地址网段已发生改变，请重新配置相关参数。";
	}

	form2Cfg(f);
	
	f.LANIP.value=clearInvalidIpstr(f.LANIP.value);
	f.LANMASK.value=clearInvalidIpstr(f.LANMASK.value);
	str+="如果页面没有自动刷新，请更新您电脑的网络设置，然后用新的IP登录。";
	if (window.confirm(str)) {
		f.submit();				
	} else {
		return;
	}
}

</script>
</body>
</html>