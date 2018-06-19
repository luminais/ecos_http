<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>TENDA 多功能宽带SOHO路由器</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var enablewireless = "<%get_wireless_basic("WirelessEnable");%>";

function init(){
	if(enablewireless !== "1"){
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href = "wireless_basic.asp";
	}
}
</script>
</head>
<body onLoad="init();">
<form name=frmSetup action="" method=post >
	<input type="hidden" id="GO" name="GO" value="wireless_state.asp">
	
	<fieldset>
		<legend>无线客户端</legend>
		<table  class="content1" id="table1">
			<tr><td>本页显示无线路由器的连接信息。</td></tr>
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