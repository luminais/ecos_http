<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>Wireless mode setting</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body onLoad="init();">
<form method=post name="formSetup" action="/goform/wirelessMode" >
<input type="hidden" name="wds_list" value="1">
	<fieldset>
		<h2 class="legend">Wireless Extender</h2>
		<div class="control-group">
			<label class="control-label">Extender Mode</label>
			<div class="controls">
				<select name="extra_mode" onChange="onWlMode();clear_data()">
            		<option value="none">Disable</option>
					<option value="apclient">Universal Repeater</option>
            		<option value="wisp">WISP Mode</option>
            		<option value="wds">WDS Bridge</option>
            	</select>
			</div>
		</div>
		<div id="wireless_enable">
			<div  class="control-group">
				<label class="control-label">SSID</label>
				<div class="controls"><input type="text" class="text" name="ssid" /></div>
			</div>
			<div class="control-group">
				<label class="control-label">Channel</label>
				<div class="controls"  id="channelWrap">
					<select name="channel">
						<option value=0>Auto select</option>
						<script>
							for(var i=1;i<14; i++) {
								document.write('<option value=' + i + '>' + i + '</option>');
							}
						</script>
					</select>
				</div>
			</div>
			<div class="control-group" style="display:none" id="wds_mac1" name="wds_mac1">
				<label class="control-label">AP MAC Address</label>
				<div class="controls">
					<input type=text name="wds_1" size=20 maxlength=17 class="text" value="">
				</div>
			</div>
			<div class="control-group" style="display:none" id="wds_mac2" name="wds_mac2">
				<label class="control-label">AP MAC Address</label>
				<div class="controls">
					<input type=text name="wds_2" size=20 maxlength=17 class="text" value="">
				</div>
			</div>
			<div  class="control-group">
			    <label class="control-label">Security Mode</label>
			    <div class="controls">
					<select name="security" onChange="onChangeSec()">
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
								<option value="1" selected="selected">Key 1</option>
								<option value="2">Key 2</option>
								<option value="3">Key 3</option>
								<option value="4">Key 4</option>
							</select>
						</td>
					</tr>
		   
					<tr> 
						<td class="control-label">WEP Key 1</td>
						<td class="controls">
							<input name="wep_key_1" id="WEP1" class="text input-medium" type="password"  maxlength="26">
							<select class="input-small" id="WEP1Select" name="WEP1Select" > 
								<option value="1">ASCII</option>
								<option value="0">Hex</option>
							</select>
						</td>
					</tr>

					<tr> 
						<td class="control-label">WEP Key 2</td>
						<td class="controls" id="td_wep2">
							<input name="wep_key_2" id="WEP2" class="text input-medium" type="password"  maxlength="26" >
							<select class="input-small" id="WEP2Select" name="WEP2Select">
								<option value="1">ASCII</option>
								<option value="0">Hex</option>
							</select>
						</td>
					</tr>
					<tr> 
						<td class="control-label">WEP Key 3</td>
						<td class="controls" id="td_wep3">
							<input name="wep_key_3" id="WEP3" type="password" class="text input-medium" maxlength="26">
							<select class="input-small" id="WEP3Select" name="WEP3Select">
								<option value="1">ASCII</option>
								<option value="0">Hex</option>
							</select>
						</td>
					</tr>
					<tr> 
						<td class="control-label">WEP Key 4</td>
						<td class="controls" id="td_wep4">
							<input name="wep_key_4" id="WEP4" type="password" class="text input-medium" maxlength="26">
							<select class="input-small" id="WEP4Select" name="WEP4Select" onChange="setChange(1)">
								<option value="1">ASCII</option>
								<option value="0">Hex</option>
							</select>
						</td>
					</tr>
				</tbody>
			</table>
			
			<!--WPA-->
			<div id="div_wpa" style="display:none">
				<div class="control-group"> 
					<label class="control-label">WPA Algorithms</label>
					<div class="controls">
						<label class="radio"><input name="cipher" id="cipher" value="aes" type="radio" />AES</label>
						<label class="radio"><input name="cipher" id="cipher" value="tkip" type="radio" checked="checked" />TKIP</label>
						<label class="radio"><input name="cipher" id="cipher" value="tkip+aes" type="radio" />TKIP&amp;AES</label>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label" for="passphrase">Security Key</label>
					<div class="controls">
						<input name="passphrase" id="passphrase" class="text" type="password" maxlength="63" />   
					</div>
				</div> 
				<div class="control-group" style="display:none;">
					<label class="control-label" for="passphrase">Key Renewal Interval</label>
					<div class="controls">
						<input name="keyRenewalInterval" id="keyRenewalInterval" class="text" type="text" maxlength="4" />   
					</div>
				</div> 				
			</div>
		  
		  <!--scan-->
		  	<div style="width:100%;margin:10px auto; text-align:center">
				<input name="wlSurveyBtn" id="wlSurveyBtn" type="button" class="btn btn-small" onClick="SurveyClose()" value="Close Scan" />
			</div>
			<table id="wdsScanTab" class="table table-fixed" style="display:none">
				<thead>
					<tr>
				 		<th><div align="center">Select</div></th>
				  		<th width="110"><div align="center">SSID</div></th>
				 		<th width="110"><p align="center">MAC Address</p></th>
				  		<th width="45"><div align="center">Channel</div></th>
				  		<th width="115"><div align="center">Security</div></th>
				  		<th width="50"><div align="center">Signal Strength</div></th>
					</tr>
				</thead>
				<tbody>
				</tbody>
			</table>
		</div>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.formSetup)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.formSetup)" />
	</div>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
var enablewireless='<% get_wireless_basic("WirelessEnable"); %>',
	ssid000 = "<% get_wireless_basic("SSID"); %>";
	
//handle translate
(function() {
	B.setTextDomain(["base", "wireless"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/libs/tenda.js"></script>
<script type="text/javascript" src="js/gozila.js"></script>
<script type="text/javascript" src="js/wireless_extra.js"></script>
</body>
</html>