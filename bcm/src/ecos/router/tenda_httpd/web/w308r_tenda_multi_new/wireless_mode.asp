<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>Wireless | Mode Setting</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body>
	<form method=post name="wireless_mode" action="/goform/wirelessMode" >
		<input type="hidden" id="wl_mode" name="wl_mode" />
		<fieldset class="table-field">
			<legend>WAN Medium Type</legend>
			<div class="control-group">
				<label class="control-label">WAN Medium Type</label>
				<div class="controls">
					<label class="radio"><input type="radio" name="wlMode" checked="checked" value="0" onClick="onWlMode('ap')" />Wired WAN</label>
					<label class="radio"><input type="radio" name="wlMode" value="1" onClick="onWlMode('sta')" />Wireless WAN</label>
				</div>
			</div>
			<div id="wl_wan_note" style="display:none;">
			<p class="text-red" style=" padding-top:10px; padding-bottom:5px;">Complete below settings to connect to WISP AP.</p>
			<div  class="control-group">
				<label class="control-label">SSID</label>
				<div class="controls"><input type="text" class="text" name="sta_ssid" /></div>
			</div>
			<div  class="control-group" style="display:none">
				<label class="control-label">MAC</label>
				<div class="controls"><input type="text" class="text" name="sta_mac" size="17" maxlength="17" /></div>
			</div>
			<div  class="control-group">
				<label class="control-label">Channel</label>
				<div class="controls">
					<select name="sta_channel">
					<script>
						document.write('<option value=' + 0 + '>' + "Auto Select" + '</option>');
						for(var i=1;i<12; i++) {
							document.write('<option value=' + i + '>' + i + '</option>');
						}
					</script>
					</select>
				</div>
			</div>
			<div  class="control-group">
			    <label class="control-label">Security Mode</label>
			    <div class="controls">
					<select name="sta_security_mode" onChange="onChangeSec()">
						<option value="Disable">Disable</option>
						<option value="0">Open</option>
						<option value="1">Shared</option>
						<option value="psk">WPA-PSK</option>
						<option value="psk2">WPA2-PSK</option>
						<option value="psk psk2">Mixed WPA/WPA2 - PSK</option>
					</select>				
				</div>
			</div>
		
		<!-- WEP -->
		<table id="div_wep" name="div_wep" class="content2" style="display:none">
		  <tbody>
		  <tr> 
			<td class="control-label">Default Key</td>
			<td class="controls">
				<select name="wep_default_key" id="wep_default_key" size="1">
					<option value="1" selected="selected">Key1</option>
					<option value="2">Key2</option>
					<option value="3">Key3</option>
					<option value="4">Key4</option>
				</select>
			</td>
		  </tr>
		  
		  <tr> 
			<td class="control-label">WEP key1</td>
			<td class="controls" id="td_wep1">
				<input name="wep_key_1" id="WEP1" class="text input-medium" type="password"  maxlength="26">
				<select class="input-small" id="WEP1Select" name="WEP1Select" > 
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select>
			</td>
		  </tr>

		  <tr> 
			<td class="control-label">WEP key2</td>
			<td class="controls" id="td_wep2">
				<input name="wep_key_2" id="WEP2" class="text input-medium" type="password"  maxlength="26" >
				<select class="input-small" id="WEP2Select" name="WEP2Select">
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select></td>
		  </tr>
		  <tr> 
			<td class="control-label">WEP key3</td>
			<td class="controls" id="td_wep3">
				<input name="wep_key_3" id="WEP3" type="password" class="text input-medium" maxlength="26">
				<select class="input-small" id="WEP3Select" name="WEP3Select">
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select></td>
		  </tr>
		  <tr> 
			<td class="control-label">WEP key4</td>
			<td class="controls" id="td_wep4">
				<input name="wep_key_4" id="WEP4" type="password" class="text input-medium" maxlength="26">
				<select class="input-small" id="WEP4Select" name="WEP4Select" onChange="setChange(1)">
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select></td>
		  </tr>
		</tbody>
		</table>

		<!--//////////////////////WPA//////////////////////////////////-->
		<fieldset id="div_wpa"  style="display:none">
			<div  id="div_wpa_algorithms" name="div_wpa_algorithms"  class="control-group" style="visibility: visible;"> 
				<label class="control-label">WPA Algorithms</label>
				<div class="controls">
					<label class="radio"><input name="cipher" id="cipher" value="aes" type="radio" />AES</label>
					<label class="radio"><input name="cipher" id="cipher" value="tkip" type="radio" checked="checked" />TKIP</label>
					<label class="radio"><input name="cipher" id="cipher" value="tkip+aes" type="radio" />TKIP&amp;AES</label>
				</div>
			</div>
			<div id="wpa_passphrase" name="wpa_passphrase"  class="control-group" style="visibility: visible;">
				<label class="control-label" for="passphrase">Key</label>
				<div class="controls" id="td_wlpwd">
					<input name="passphrase" id="passphrase" class="text" type="password" maxlength="63" />   
				</div>
			</div>  				
		  </fieldset>
		<!--//////////////////////END WPA//////////////////////////////-->
		
		<div style="width:100%;margin:10px auto; text-align:center">
			<input name="wlSurveyBtn" id="wlSurveyBtn" type="button" class="btn btn-small" onClick="SurveyClose()" value="Close Scan" />
		</div>
		<table id="wdsScanTab" class="table" style="display:none">
			<thead>
				<tr>
				  <th width="10%"><div align="center">Select</div></td>
				  <th width="20%"><div align="center">SSID</div></td>
				  <th width="30%"><p align="center">MAC Address</p></td>
				  <th width="10%"><div align="center">Channel</div></td>
				  <th width="15%"><div align="center">Security</div></td>
				  <th width="15%"><div align="center">Signal Strength</div></td>
				</tr>
			</thead>
			<tbody>
			</tbody>
		</table>
		</div>
		<div class="btn-group">
			<input type="button" class="btn" value="OK" onClick="preSubmit(document.wireless_mode)" />
			<input type="button" class="btn last" value="Cancel" onClick="init(document.wireless_mode)" />
		</div>
	</form>

<script src="lang/b28n.js" type="text/javascript"></script>
<script>
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>",
	ssid000 = "<% get_wireless_basic("SSID"); %>";
	
//handle translate
(function() {
	B.setTextDomain(["base", "wireless"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/libs/tenda.js"></script>
<script type="text/javascript" src="js/gozila.js"></script>
<script type="text/javascript" src="js/wireless_mode.js"></script>
</body>
</html> 