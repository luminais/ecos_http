<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>TENDA 11N宽带路由器</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language=JavaScript src="js/gozila.js"></script>
<script language=JavaScript>
def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>";
var LANIP=def_LANIP,
	netip=LANIP.replace(/\.\d{1,3}$/,".");
function init(){
	document.LANDhcpsSet.DHEN.checked = <%aspTendaGetStatus("lan","dhcps");%>;//是否启用：0：关闭；1：启用；
	document.LANDhcpsSet.DHLT.value = "<%aspTendaGetStatus("lan","lease_time");%>";//过期时间：3600：一小时；7200：二小时； 10800：三小时； 86400：一天； 172800：两天； 604800：一周

	document.LANDhcpsSet.dips.value = (("<%aspTendaGetStatus("lan","dhcps_start");%>").split("."))[3];//IP池开始地址
	document.LANDhcpsSet.dipe.value = (("<%aspTendaGetStatus("lan","dhcps_end");%>").split("."))[3];//IP池结束地址
}

function preSubmit(f) {

	var loc = "/goform/DhcpSetSer?GO=lan_dhcps.asp";

	if (!rangeCheck(f.dips,1,254,"IP池开始地址")){
		return ;
	}
	if (!rangeCheck(f.dipe,1,254,"IP池终止地址")) {
		return ;
	}
   
   	if (Number(f.dips.value)>Number(f.dipe.value)) {
      alert("IP池开始地址不能大于结束地址 !!!");
      return ;
   	}
	if(f.DHEN.checked)
	{
		loc += "&dhcpEn=1";
	}
	else
	{
		loc += "&dhcpEn=0";
	}	
	loc += "&dips=" + netip + f.dips.value;
	loc += "&dipe=" + netip + f.dipe.value;
	loc += "&DHLT=" + f.DHLT.value;		
	window.location = loc;
	showSaveMassage();
}
</script>
</head>
<body onLoad="init()">
<form name="LANDhcpsSet" method="POST" action="/goform/DhcpSetSer">
<input type="hidden" name="GO"  value="lan_dhcps.asp">
<input type="hidden" name="dhcpEn">
	<fieldset>
		<h2 class="legend">DHCP服务器</h2>
		<div class="control-group"> 
			<label class="control-label">DHCP服务器</label> 
			<div class="controls">
				<label class="checkbox"><input type="checkbox" name="DHEN" value=1 />启用</label>
			</div>
		</div>
		<div class="control-group"> 
			<label class="control-label">IP池开始地址</label> 
			<div class="controls"><script>document.write(netip);</script>
				<input name="dips" class="text input-medium"  SIZE="3" />
			</div>
		</div>
		<div class="control-group"> 
			<label class="control-label">IP池结束地址</label> 
			<div class="controls"><script>document.write(netip);</script>
				<input name="dipe" class="text input-medium"  SIZE=3 />
			</div>
		</div>
		<div class="control-group"> 
		  <label class="control-label">过期时间</label> 
		  <div class="controls">
			<select NAME=DHLT SIZE=1>
				<option VALUE="3600">一小时</option>
				<option VALUE="7200">二小时</option>
				<option VALUE="10800">三小时</option>
				<option VALUE="86400">一天</option>
				<option VALUE="172800">两天</option>
				<option VALUE="604800">一周</option>
			</select>
		  </div>
		</div>
	</fieldset>
    <script>tbl_tail_save("document.LANDhcpsSet");</script>
</form>
<div id="save" class="none"></div>
</body>
</html>