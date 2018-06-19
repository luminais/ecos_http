<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>Tenda Multi-functional Broadband SOHO Router</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body>
<form name=frmSetup action="" method=post >
	<input type="hidden" id="GO" name="GO" value="wireless_state.asp">
	
	<fieldset>
		<legend>Wireless Connection Status</legend>
		<div class="control-group">
			<label class="control-label">Select SSID</label>
			<div class="controls">
				<span id="ssid-select-content"><select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
				</select></span>
			</div>		
		</div>
		<p>The currently connected hosts list:
			<input type="button" class="btn" value="Refresh" onClick="refresh('wireless_state.asp')" />
		</p>
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

<!-- Le javascript
================================================== -->
<!-- Placed at the end of the document so the pages load faster -->
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
	var enablewireless = "<%get_wireless_basic("WirelessEnable");%>",
		ssidlist = "<% get_wireless_basic("SsidList"); %>",
		Cur_ssid_index = "<% get_wireless_basic("Cur_wl_unit"); %>",
		SSID = new Array(2);

	SSID[0] = "<% get_wireless_basic("SSID"); %>";
	SSID[1] = "<% get_wireless_basic("SSID1"); %>";


	function init(){
		if(enablewireless !== "1"){
			alert(_("This feature can be used only if wireless is enabled."));
			top.mainFrame.location.href = "wireless_basic.asp";
		} else {
				UpdateMBSSIDList();
		}
	}
	function UpdateMBSSIDList() {
		var defaultSelected = false;
		var selected = false;
		var optionStr = '<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">';
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
		window.location = loc;
	}
	
	window.onload = init;
</script>
</body>
</html>