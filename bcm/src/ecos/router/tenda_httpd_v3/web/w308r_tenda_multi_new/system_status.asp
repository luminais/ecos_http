<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css" />
<script type="text/javascript">
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
	ip_change = "<%sysTendaGetStatus("wan","ip-change");%>",
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
		return '00:00:00';
	}
	var s = t%60,
		m = parseInt(t/60)%60,
		h = parseInt(t/3600)%24,
		d = parseInt(t/86400),
		str = '';
	
	if (d > 999) {
		return ("Permanent");
	}
	if (d) {
		str += d + ("Day(s)");
	}
	str += fit2(h) + ':';
	str += fit2(m) + ':';
	str += fit2(s);
	return str;
}
</script>

</head>
<body class="system-status">
<form name="systemStatus" method="POST" action="/goform/SysStatusHandle">
  <input type="hidden" name="CMD" value="WAN_CON">
  <input type="hidden" name="GO" value="system_status.asp">
  <input type="hidden" name="action">
	<fieldset>
		<legend>WAN Status</legend>
		<table id="table1">
			<tr>
				<td class="control-label"><span>Connection Status</span></td>
				<td class="controls">
					<span class="text-red" id="con_stat">-</span>
				</td>
			</tr>
			<tr>
				<td class="control-label">Internet Connection Type</td>
				<td class="controls" id="con_type">-</td>
			</tr>
			<tr>
				<td class="control-label">WAN IP</td>
				<td class="controls" id="wan_ip">-</td>
			</tr>
			<tr>
				<td class="control-label">Subnet Mask</td>
				<td class="controls" id="sub_mask">-</td>
			</tr>
			<tr>
				<td class="control-label">Gateway</td>
				<td class="controls" id="gateWay">-</td>
			</tr>
			<tr>
				<td class="control-label">DNS Server</td>
				<td class="controls" id="dns1">-</td>
			</tr>
			<tr>
				<td class="control-label">Alternate DNS Server</td>
				<td class="controls" id="dns2">-</td>
			</tr>
			<script>
			if ((conTypeIdx==2)||(conTypeIdx==3)||(conTypeIdx==4) ||(conTypeIdx==5)) {
				document.write('<tr><td class="control-label">Connection Time</td>');
				document.write('<td class="controls">');
				document.write(timeStr(conntime));
				document.write("</td></tr>");
			}  
			</script>
		</table>
		<table id="errcheck" style="display:none;margin-top:18px" >
			<tr>
				<td class="control-label">Diagnose Connection Status</td>
				<td class="controls" id="errconext"></td>
			</tr>
		</table>
		<div class="btn-group">
		<script>
			if (conTypeIdx==2) {//dhcp
				 document.write('<input type=button class="btn btn-small" value="Release" onclick=preSubmit(1);>');
				 document.write('<input type=button class="btn btn-small" value="Refresh" onclick=preSubmit(2);>');
			} else if (conTypeIdx == 3) { //pppoe
				if(cableDSL == 0) {//unlink
					document.write('<input type=button class="btn btn-small" value="Connect" onclick=preSubmit(3);>');
					document.write('<input type=button class="btn btn-small" value="Disconnect" disabled="disabled">');
				} else if(cableDSL == 1) {//linked
					document.write('<input type=button class="btn btn-small" value="Connect" disabled="disabled">');
					document.write('<input type=button class="btn btn-small" value="Disconnect" onclick=preSubmit(4);>');
				} else {//linked ||linking
					document.write('<input type=button class="btn btn-small" value="Connect"  disabled="disabled">');
					document.write('<input type=button class="btn btn-small" value="Disconnect" onclick=preSubmit(4);>');
				}
			}
		</script>    
		</div>
	</fieldset>

	<fieldset>
		<legend>System Status</legend>
		<table id="table2">
			<tr>
				<td class="control-label">LAN MAC Address</td>
				<td class="controls" id="lan_mac">-</td>
			</tr>
				<tr>
					<td class="control-label">WAN MAC Address</td>
					<td class="controls" id="wan_mac">-</td>
				</tr>
				<tr>
				<td class="control-label">System Time</td>
				<td class="controls" id="systime">-</td>
			</tr>
			<tr>
				<td class="control-label">Running Time</td>
				<td class="controls" id="uptime">-</td>
			</tr>
			<tr>
				<td class="control-label">Connected Client</td> 
				<td class="controls" id="clients">-</td>
			</tr>
			<tr>
				<td class="control-label">System Version</td> 
				<td class="controls" id="run_code_ver">-</td>
			</tr>
			<tr>
				<td class="control-label">Hardware Version</td> 
				<td class="controls" id="hw_ver">-</td>
			</tr>
		</table>
	</fieldset>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script type="text/javascript">
//handle translate
(function() {
	B.setTextDomain(["base", "advanced"]);
	B.translate();
})();
</script>
<script language=JavaScript src="js/gozila.js"></script>
<script type="text/javascript">

function preSubmit(idx) {   
	var f=document.systemStatus;
		f.action.value=idx;
		f.submit() ;
}
(function (window) {
"use strict"
var conType = [_("Static IP"), "DHCP", "PPPoE", "PPTP", "L2TP"],
	state	= [_("Disable"), _("Enable")],
	conStat	= [_("Disconnected"), _("Connecting"), _("Connected")],
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
window.onload = function () {
	var timer = setTimeout(function() {
		window.location.reload(true);
	}, 5000);
	
	initHtml();
	if(message == 0) {
		document.getElementById("errcheck").style.display="";
		document.getElementById("errconext").innerHTML='<span class="text-red">' + _("Please check hardware connection of the WAN port.") + '</span>';
	} else if (conTypeIdx == 3) {	
		if(cableDSL==1 || cableDSL==2) {
			if(err_check == "5") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML=
						'<span class="text-red">' + _("No response from your Internet Service Provider(ISP), please consult your ISP.") + '</span>'; 	
			} else if(err_check=="2") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML = 
						'<span class="text-red">' + _("Authentication failed! Please check PPPoE Username and PPPoE Password.") + '</span>';
			} else if(err_check=="3") {
				document.getElementById("errcheck").style.display="";
				document.getElementById("errconext").innerHTML= '<span class="text-success">' + _("Connected to Internet successfully!") + '</span>';
			}
		}
	} else if(conTypeIdx == 2) {	
		if(err_check == "11") {
			var con = confirm(_("IP conflict!The LAN IP address will be changed into %s automatically and it will be the new login IP address.",[ip_change]));
			if(con == true) {
				clearTimeout(timer);
				var loc = "/goform/AdvSetLanip?GO=lan.asp" + "&LANIP=" + ip_change + "&LANMASK=" + lan_mask;
				window.location = loc;
			}
/*
        	if(lan_mask=="255.255.255.0") {
				var m,
					ipnum = lan_ip.split("."),
					ipval = parseInt(ipnum[2]);
				//m = (parseInt(ipnum[2]) + 1)%256;
				m = (parseInt(net_leader.split(".")[2]) + parseInt(255 - third_mask.split(".")[2]) + 1)%256;
				var ipmodify = ipnum[0]+"."+ipnum[1]+"."+m+"."+ipnum[3];
				var con = confirm(_("IP conflict!The LAN IP address will be changed into %s automatically and it will be the new login IP address.",[ipmodify]));
				if(con == true) {
					var new_ip = ipnum[0]+"."+ipnum[1]+"."+m+"."+ipnum[3],
						loc = "/goform/AdvSetLanip?GO=lan.asp" + "&LANIP=" + new_ip + "&LANMASK=" + lan_mask;
					window.location = loc;
				}
			}
*/
		 }
	}
	
	try {
		//reset this iframe height by call parent frame function
		window.parent.reinitIframe();
	}  catch (ex){ 
		setTimeout("window.parent.reinitIframe()",100);
	}
	
}
})(window);
</script>
</body>
</html>