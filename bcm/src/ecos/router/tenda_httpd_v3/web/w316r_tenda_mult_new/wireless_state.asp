<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>Tenda Multi-functional Broadband SOHO Router</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body onLoad="init();">
<form name=frmSetup action="" method=post >
	<input type="hidden" id="GO" name="GO" value="wireless_state.asp">
	
	<fieldset>
		<legend>Wireless Connection Status</legend>
		<table  class="content1" id="table1">
			<tr><td>&nbsp;</td></tr>
			<tr><td>The currently connected hosts list:
				<input type=button class="btn" value="Refresh" onclick=refresh("wireless_state.asp")></td>
			</tr>
		</table>
		<table class="table">
			<thead>
				<tr>
					<th align=middle width="20%">NO.</th>
					<th align=middle width="50%">MAC Address</th>
					<th align=middle width="30%">Bandwidth</th>
				</tr>
			</thead>
			<tbody>
				<%get_wireless_station("stationinfo");%>                         
			</tbody>
		</table>
	</fieldset>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "wireless"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var enablewireless = "<%get_wireless_basic("WirelessEnable");%>";

function init(){
	if(enablewireless !== "1"){
		alert(_("This feature can be used only if wireless is enabled."));
		top.mainFrame.location.href = "wireless_basic.asp";
	}
}
</script>
</body>
</html>