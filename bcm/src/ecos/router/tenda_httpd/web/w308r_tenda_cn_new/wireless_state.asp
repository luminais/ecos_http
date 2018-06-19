<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>TENDA 多功能宽带SOHO路由器</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var enablewireless = "<%get_wireless_basic("WirelessEnable");%>",
	ssidlist = "<% get_wireless_basic("SsidList"); %>",
	Cur_ssid_index = "<% get_wireless_basic("Cur_wl_unit"); %>",
	SSID = new Array(2);
	
SSID[0] = "<% get_wireless_basic("SSID"); %>";
SSID[1] = "<% get_wireless_basic("SSID1"); %>";


function init(){
	if(enablewireless !== "1"){
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href = "wireless_basic.asp";
	} else {
			UpdateMBSSIDList();
	}
}
function UpdateMBSSIDList() {
	var defaultSelected = false,
		selected = false,
		optionStr = '<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">';
	for(var i=0; i<SSID.length; i++){
		if(SSID[i] != "") {
			if (Cur_ssid_index == i) {
				optionStr += '<option selected="selected">' + SSID[i] + '</option>';
			} else {
				optionStr += '<option>' + SSID[i] + '</option>';
			}
		}
	}
	optionStr += '</select>';
	document.getElementById('ssid-select-content').innerHTML = optionStr;
}

/*
 * When user select the different SSID, this function would be called.
 */ 
function selectMBSSIDChanged() {
	var ssid_index;
	
	ssid_index = document.frmSetup.ssidIndex.options.selectedIndex;
	
	var loc = "/goform/onSSIDChange?GO=wireless_state.asp";

	loc += "&ssid_index=" + ssid_index; 
	location = loc;
}
</script>
</head>
<body onLoad="init();">
<form name=frmSetup action="" method=post >
	<input type="hidden" id="GO" name="GO" value="wireless_state.asp">
	
	<fieldset>
		<h2 class="legend">无线客户端</h2>
		<table  class="content1" id="table1">
			<tr><td>选择无线信号名称(SSID)
				<span id="ssid-select-content"><select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
				</select></span></td>
			</tr>
			<tr><td>&nbsp;</td>
			</tr>
			<tr><td>当前连接的主机列表：
				<input type=button class="btn" value="刷 新" onclick=refresh("wireless_state.asp")></td>
			</tr>
		</table>
		<table class="table">
			<thead>
				<tr>
					<th align=middle width="20%">序号</th>
					<th align=middle width="50%">MAC地址</th>
					<th align=middle width="30%">带宽</th>
				</tr>
			</thead>
			<tbody>
				<%get_wireless_station("stationinfo");%>                         
			</tbody>
		</table>
	</fieldset>
</form>
</body>
</html>