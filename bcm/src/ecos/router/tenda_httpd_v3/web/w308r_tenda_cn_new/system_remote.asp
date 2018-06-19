<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Administrator Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var def_RMEN = "<%getfirewall("wan","wanweben");%>";//1:启用;0:关闭
var def_RMPORT = "<%getfirewall("wan","webport");%>";//端口
var def_RMIP = "<%getfirewall("wan","rmanip");%>";//IP 地址:开始IP1;结束IP1p
var lan_ip = "<%aspTendaGetStatus("lan", "lanip");%>";
var lan_ipmask = "<%aspTendaGetStatus("lan", "lanmask");%>";
function init(f){
	var en = parseInt(def_RMEN);
	f.RMEN.checked = en;
	onSwitch();
    f.RMsIP1.value = def_RMIP;
	f.RMPORT.value = def_RMPORT;
}

function preSubmit(f) {    
	f.RMsIP1.value = clearInvalidIpstr(f.RMsIP1.value);
	f.RMPORT.value = clearInvalidIpstr(f.RMPORT.value);    
	var sip1 = f.RMsIP1.value;
	var port = f.RMPORT.value;
	var loc = "/goform/SafeWanWebMan?GO=system_remote.asp";
	
	if(f.RMEN.checked) {	
		if (!rangeCheck(f.RMPORT,1,65535,"端口")) return;
		loc += "&RMEN=1&port=" + port;
		if (!verifyIP0(f.RMsIP1,"IP地址")) return ;
		if(sip1 == "") {
			sip1 = "0.0.0.0";
		}
		if (ipCheck(lan_ip,sip1,lan_ipmask)){
			alert(f.RMsIP1.value+ "不能与LAN　IP同网段!");
			f.RMsIP1.value="0.0.0.0";
			return ;
		}
		loc += "&IP=" + sip1;		
	} else {
		loc += "&RMEN=0";
	}
	window.location = loc;
}

function onSwitch(){
	if(document.frmSetup.RMEN.checked){
		document.getElementById("RMPORT").disabled = false;	
		document.getElementById("ipTab").style.display = "";
	}else {
		document.getElementById("ipTab").style.display = "none";
		document.getElementById("RMPORT").disabled = true;
	}
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name="frmSetup" method="POST" action="/goform/SafeWanWebMan">
	<input type="hidden" id="GO" value="system_remote.asp">
	<fieldset>
		<h2 class="legend">远程WEB管理</h2>
		<div class="control-group" id="login_tip">
			<div class="controls">
				<label class="checkbox" for="RMEN">
					<input type="checkbox" id="RMEN" onClick="onSwitch()">启用
				</label>
			</div>
		</div>
		<table class="content2" id="ipTab">                
			<tr>
			  <td class="control-label">端口</td>
			  <td class="controls"><input class="text" id="RMPORT" size="5" maxlength="5"></td>
			</tr>
			<tr>
			  <td class="control-label">IP地址</td>
			  <td class="controls"><input class="text" id="RMsIP1" size="15" maxlength="15"></td>
			</tr>
		</table>
	</fieldset>
<script>tbl_tail_save("document.frmSetup");</script>
</form>
</body>
</html>