<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<meta http-equiv="refresh" content="5"/>
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css" />
<script>
var cableDSL = "<%sysTendaGetStatus("wan","contstatus");%>",
	subMask = "<%sysTendaGetStatus("wan","wanmask");%>",
	wanIP = "<%sysTendaGetStatus("wan","wanip");%>",
	gateWay = "<%sysTendaGetStatus("wan","gateway");%>",
	dns1 = "<%sysTendaGetStatus("wan","dns1");%>",
	dns2 = "<%sysTendaGetStatus("wan","dns2");%>",
	conntime = "<%sysTendaGetStatus("wan","connetctime");%>",
	conTypeIdx = "<%aspTendaGetStatus("wan","connecttype");%>",
	run_code_ver = "<%aspTendaGetStatus("sys","sysver");%>",
	boot_code_ver = "<%aspTendaGetStatus("sys","bootloadver");%>",
	hw_ver = "<%aspTendaGetStatus("sys","hardwarever");%>",
	clients = "<%aspTendaGetStatus("sys","conclient");%>",
	uptime = "<%aspTendaGetStatus("sys","runtime");%>",
	systime = "<%aspTendaGetStatus("sys","systime");%>",
	lan_mac = "<%aspTendaGetStatus("sys","lanmac");%>",
	wan_mac = "<%aspTendaGetStatus("sys","wanmac");%>",
	err_check = "<%sysTendaGetStatus("pppoe","err_check");%>",
	index = "<%sysTendaGetStatus("pppoe","index");%>",
	message = "<%sysTendaGetStatus("wan","linkstatus");%>",
	lan_ip = "<%aspTendaGetStatus("lan","lanip");%>",
	lan_mask = "<%aspTendaGetStatus("lan","lanmask");%>";
function fit2(n){
	var s = String(n + 100).substr(1, 2);
	return s;
}
function timeStr(t){
	if (t < 0) {
		str = '00:00:00';
		return str;
	}
	var s = t%60,
		m = parseInt(t/60)%60,
		h = parseInt(t/3600)%24,
		d = parseInt(t/86400),
		str = '';
	
	if (d > 999) {
		return '永久';
	}
	if (d) {
		str += d + '天 ';
	}
	str += fit2(h) + ':';
	str += fit2(m) + ':';
	str += fit2(s);
	return str;
}
</script>
</head>
<body>
<form name="systemStatus" method="POST" action="/goform/SysStatusHandle">
  <input type="hidden" name="CMD" value="WAN_CON">
  <input type="hidden" name="GO" value="system_status.asp">
  <input type="hidden" name="action">
	<fieldset>
		<legend>WAN口状态</legend>
		<table id="table1">
			<tr>
				<td class="control-label"><span>连接状态</span></td>
				<td class="controls">
					<span class="text-red" id="con_stat">-</span>
				</td>
			</tr>
			<tr>
				<td class="control-label">连接方式</td>
				<td class="controls" id="con_type">-</td>
			</tr>
			<tr>
				<td class="control-label">WAN IP</td>
				<td class="controls" id="wan_ip">-</script></td>
			</tr>
			<tr>
				<td class="control-label">子网掩码</td>
				<td class="controls" id="sub_mask">-</script></td>
			</tr>
			<tr>
				<td class="control-label">网关</td>
				<td class="controls" id="gateWay">-</script></td>
			</tr>
			<tr>
				<td class="control-label">域名服务器</td>
				<td class="controls" id="dns1">-</td>
			</tr>
			<tr>
				<td class="control-label">备用域名服务器</td>
				<td class="controls" id="dns2">-</td>
			</tr>
			<script>
			if ((conTypeIdx==2)||(conTypeIdx==3)||(conTypeIdx==4) ||(conTypeIdx==5)) {
				document.write('<tr><td class="control-label">连接时间</td>');
				document.write('<td class="controls">');
				document.write(timeStr(conntime));
				document.write("</td></tr>");
			}  
			</script>
		</table>
		<table id="errcheck" style="display:none;margin-top:18px" >
			<tr>
				<td class="control-label">网络连接状态诊断</td>
				<td class="controls" id="errconext"></td>
			</tr>
		</table>
		<div class="btn-group">
		<script>
			if (conTypeIdx==2) {//dhcp
				 document.write('<input type=button class="btn btn-small" value="释  放" onclick=preSubmit(1);>');
				 document.write('<input type=button class="btn btn-small" value="更  新" onclick=preSubmit(2);>');
			} else if (conTypeIdx == 3) { //pppoe
				if(cableDSL == 0) {//unlink
					document.write('<input type=button class="btn btn-small" value="连  接" onclick=preSubmit(3);>');
					document.write('<input type=button class="btn btn-small" value="断  开" disabled="disabled">');
				} else if(cableDSL == 1) {//linked
					document.write('<input type=button class="btn btn-small" value="连  接" disabled="disabled">');
					document.write('<input type=button class="btn btn-small" value="断  开" onclick=preSubmit(4);>');
				} else {//linked ||linking
					document.write('<input type=button class="btn btn-small" value="连  接"  disabled="disabled">');
					document.write('<input type=button class="btn btn-small" value="断  开" onclick=preSubmit(4);>');
				}
			}
		</script>    
		</div>
	</fieldset>

	<fieldset>
		<legend>系统状态</legend>
		<table id="table2">
			<tr>
				<td class="control-label">LAN MAC 地址</td>
				<td class="controls" id="lan_mac">-</script></td>
			</tr>
				<tr>
					<td class="control-label">WAN MAC 地址</td>
					<td class="controls" id="wan_mac">-</script></td>
				</tr>
				<tr>
				<td class="control-label">系统时间</td>
				<td class="controls" id="systime">-</script></td>
			</tr>
			<tr>
				<td class="control-label">运行时间</td>
				<td class="controls" id="uptime">-</script></td>
			</tr>
			<tr>
				<td class="control-label">客户端个数</td> 
				<td class="controls" id="clients">-</script></td>
			</tr>
			<tr>
				<td class="control-label">软件版本号</td> 
				<td class="controls" id="run_code_ver">-</script></td>
			</tr>
			<tr>
				<td class="control-label">硬件版本号</td> 
				<td class="controls" id="hw_ver">-</td>
			</tr>
		</table>
	</fieldset>
</form>
<script type="text/javascript">
function preSubmit(idx) {   
	var f=document.systemStatus;
		f.action.value=idx;
		f.submit() ;
}
(function (window) {
"use strict"
var conType = ["静态 IP", "自动获取", "ADSL拨号", "PPTP", "L2TP"],
	state	= ["禁用", "启用"],
	conStat	= ["未连接", "连接中...", "已连接"],
	dynHtml = {
		con_stat: conStat[cableDSL],
		con_type: conType[conTypeIdx-1],
		wan_ip: wanIP,
		sub_mask: subMask,
		gateWay: gateWay,
		dns1: dns1,
		dns2: dns2,
		lan_mac: lan_mac,
		wan_mac: wan_mac,
		systime: systime,
		uptime: timeStr(uptime),
		clients: clients,
		run_code_ver: run_code_ver,
		hw_ver: hw_ver
	};

function initHtml() {
	var id;
	for (id in dynHtml) {
		document.getElementById(id).innerHTML = dynHtml[id];
	}
}
function initIframe() {
	try {
		//reset this iframe height by call parent frame function
		window.parent.reinitIframe();
	} catch (ex){ 
		setTimeout("initIframe()", 100);
	}
}
window.onload = function () {
	initHtml();
	if(message == 0) {
		document.getElementById("errcheck").style.display="";
		document.getElementById("errconext").innerHTML='<span class="text-red">检测到WAN口网线未连接，请检查并连接好您的WAN口网线。</span>';
	} else if (conTypeIdx == 3) {	
		if(cableDSL==1 || cableDSL==2) {
			if(err_check == "5") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML=
						'<span class="text-red">网络运营商远端无响应，请确认不接路由器时是否可以正常上网，如不能，请联系当地网络运营商解决。</span>' 
			} else if(err_check=="7") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML = 
						'<span class="text-red"> 正在诊断您输入的宽带用户名和宽带密码是否正确，请稍等，整个过程约1-5分钟。</span>';	
			} else if(err_check=="2") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML = 
						'<span class="text-red">用户名密码验证失败，请确认您的宽带用户名与宽带密码并重新输入。</span>';
			} else if(err_check=="3") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML= '<span class="text-success"> 网络连接成功。</span>';
			}
		}
	} else if(conTypeIdx == 2) {	
		if(err_check == "11") {
        if(lan_mask=="255.255.255.0") {
				var m,
					ipnum = lan_ip.split("."),
					ipval = parseInt(ipnum[2]);
				m = (parseInt(ipnum[2]) + 1)%256;
				var con = confirm("检测到IP地址冲突，LAN口IP地址将自动修改为"+ipnum[0]+"."+ipnum[1]+"."+m+"."+ipnum[3]+"，请以"+ipnum[0]+"."+ipnum[1]+"."+m+"."+ipnum[3]+"重新登录界面。");
				if(con == true) {
					var new_ip = ipnum[0]+"."+ipnum[1]+"."+m+"."+ipnum[3],
						loc = "/goform/AdvSetLanip?GO=lan.asp" + "&LANIP=" + new_ip + "&LANMASK=" + lan_mask;
					window.location = loc;
				}
			}
		 }
	}
	initIframe();
}
})(window);
</script>
</body>
</span>