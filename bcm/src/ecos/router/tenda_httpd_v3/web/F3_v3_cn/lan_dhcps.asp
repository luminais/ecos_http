<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>DHCP | Server</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body onLoad="init()">
<form name="LANDhcpsSet" method="POST" action="/goform/DhcpSetSer">
<input type="hidden" name="GO"  value="lan_dhcps.asp">
<input type="hidden" name="dhcpEn">
	<fieldset class="table-field">
		<legend>DHCP Server</legend>
		<div class="control-group"> 
			<label class="control-label">DHCP Server</label> 
			<div class="controls">
				<label class="checkbox"><input type="checkbox" name="DHEN" value=1 />Enable</label>
			</div>
		</div>
		<div class="control-group"> 
			<label class="control-label">IP Pool Start Address</label> 
			<div class="controls"><span id="start-ip-prefix"></span>
				<input name="dips" class="text input-medium"  SIZE="3" />
			</div>
		</div>
		<div class="control-group"> 
			<label class="control-label">IP Pool End Address</label> 
			<div class="controls"><span id="end-ip-prefix"></span>
				<input name="dipe" class="text input-medium"  SIZE=3 />
			</div>
		</div>
		<div class="control-group"> 
		  <label class="control-label">Lease Time</label> 
		  <div class="controls">
			<select NAME=DHLT SIZE=1>
				<option VALUE="3600">One hour</option>
				<option VALUE="7200">Two hours</option>
				<option VALUE="10800">Three hours</option>
				<option VALUE="86400">One day</option>
				<option VALUE="172800">Two days</option>
				<option VALUE="604800">One week</option>
			</select>
		  </div>
		</div>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.LANDhcpsSet)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.LANDhcpsSet)" />
	</div>
</form>
<div id="save" class="none"></div>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "advanced"]);
	B.translate();
})();
</script>
<script src="js/gozila.js"></script>
<script>
var def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>",
	LANIP = def_LANIP,
	netip = LANIP.replace(/\.\d{1,3}$/,".");

function init(){
	document.getElementById("start-ip-prefix").innerHTML = netip;
	document.getElementById("end-ip-prefix").innerHTML = netip;
	document.LANDhcpsSet.DHEN.checked = <%aspTendaGetStatus("lan","dhcps");%>;//是否启用：0：关闭；1：启用；
	document.LANDhcpsSet.DHLT.value = "<%aspTendaGetStatus("lan","lease_time");%>";//过期时间：3600：一小时；7200：二小时； 10800：三小时； 86400：一天； 172800：两天； 604800：一周
	document.LANDhcpsSet.dips.value = +(("<%aspTendaGetStatus("lan","dhcps_start");%>").split("."))[3];//IP池开始地址
	document.LANDhcpsSet.dipe.value = +(("<%aspTendaGetStatus("lan","dhcps_end");%>").split("."))[3];//IP池结束地址
}

function preSubmit(f) {

	var loc = "/goform/DhcpSetSer?GO=lan_dhcps.asp";

	if (!rangeCheck(f.dips,1,254,_("IP pool start address"))){
		return ;
	}
	if (!rangeCheck(f.dipe,1,254,_("IP pool end address"))) {
		return ;
	}
   
	if (Number(f.dips.value)>Number(f.dipe.value)) {
		alert(_("Please enter an IP pool start address that is smaller than the IP pool end address."));
		return ;
	}
	if(f.DHEN.checked) {
		loc += "&dhcpEn=1";
	} else {
		loc += "&dhcpEn=0";
	}	
	loc += "&dips=" + netip + parseInt(f.dips.value, 10);
	loc += "&dipe=" + netip + parseInt(f.dipe.value, 10);
	loc += "&DHLT=" + f.DHLT.value;		
	if (confirm(_("The router will reboot automatically!"))) {		
	window.location = loc;
		//showSaveMassage();
	}
}
</script>
</body>
</html>