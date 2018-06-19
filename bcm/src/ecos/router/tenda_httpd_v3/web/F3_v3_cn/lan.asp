<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>LAN | LAN Settings</title>
<link rel=stylesheet type="text/css" href="css/screen.css">
</head>

<body class="bg" onLoad="init(document.LanSet);">
<form name="LanSet" method="POST" action="/goform/AdvSetLanip">
<input type=hidden name=GO value=lan.asp>
	<fieldset>
		<legend>LAN Settings</legend>
		<table class="content1">                       
			<tr>
				<td colspan=2 align="left" valign="top">This page is used to set the basic network parameters for LAN.</td>
			</tr>
		</table>
		<table class="content2" id="table1">
			<tr>
				<td class="control-label">LAN MAC Address</td>
				<td class="controls"><%aspTendaGetStatus("sys","lanmac");%></td>
			</tr>
			<tr>
				<td class="control-label">IP Address</td>
				<td class="controls"><input class=text maxlength=15 name=LANIP size=15 ></td>
			</tr>
			<tr>
				<td class="control-label">Subnet Mask</td>
				<td class="controls"><input class=text maxlength=15 name=LANMASK size=15></td>
			</tr>
		</table>
	</fieldset>
	<div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.LanSet)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.LanSet)" />
	</div>
</form>

<script src="lang/b28n.js" type="text/javascript"></script>
<script>
	B.setTextDomain(["base", "advanced"]);
</script>
<script language=javascript src="js/gozila.js"></script>
<script language=JavaScript>
//handle translate
B.translate()

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
	if (!verifyIP2(f.LANIP, _("IP address"))) return ;
	
	//增加了支持子网划分，和网络聚集判断
	if ( !tenda_ipMskChk(f.LANMASK, _("subnet mask"), f.LANIP)) return ;	
	
	if(!same_net_with_lan(f)){
		str += _("Router's IP Address has been changed! Please update related settings!");
	}

	form2Cfg(f);
	
	f.LANIP.value=clearInvalidIpstr(f.LANIP.value);
	f.LANMASK.value=clearInvalidIpstr(f.LANMASK.value);
	str += _("You have changed the IP address of this router, please click on OK to confirm this change. The router will reboot. After reboot, if the webpage doesn't update automatically, please renew your computer's connection to the router and then use the new IP address to login.");
	if (window.confirm(str)) {
		f.submit();				
	} else {
		return;
	}
}

</script>
</body>
</html>