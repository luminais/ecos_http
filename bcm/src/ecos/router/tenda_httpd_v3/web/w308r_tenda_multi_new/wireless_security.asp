<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<meta http-equiv="Pragma" content="no-cache" />
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body>
	<form method="post" name="security_form" action="/goform/wirelessSetSecurity">
		<input type="hidden" name="wifiEn" id="wifiEn" />
		<input type="hidden" name="wpsmethod" id="wpsmethod" />
		<input type="hidden" id="GO" name="GO" value="wireless_security.asp" />
		<fieldset class="table-field">
			<legend>Wireless Security Setup</legend>
			<div class="control-group">
				<label class="control-label">Select SSID</label>
				<div class="controls">
					<span id="ssid-select-content"><select name="ssidIndex" id="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
					</select></span>
				</div>
			</div>
			<div class="control-group">
				<label class="control-label" for="security_mode">Security Mode</label>
				<div class="controls">
					<select name="security_mode" id="security_mode" size="1" onChange="securityMode()"></select>
				</div>
			</div>
			<div class="control-group" id="div_security_shared_mode">
				<label class="control-label">Encryption type</label>
				<div class="controls">
					<select name="security_shared_mode">
					  <option value="enable">WEP</option>
					</select>
				</div>
			</div>
			<!-- WEP -->
			<fieldset id="div_wep" name="div_wep">
			<!--<div id="div_wep" name="div_wep">-->
				<div class="control-group"> 
					<label class="control-label">Default Key</label>
					<div class="controls">
						<select name="wep_default_key" id="wep_default_key" size="1">
							<option value="1" selected="selected">Key 1</option>
							<option value="2">Key 2</option>
							<option value="3">Key 3</option>
							<option value="4">Key 4</option>
						</select>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label" for="WEP1">WEP Key 1</label>
					<div class="controls">
						<input type="password" name="wep_key_1" id="WEP1" maxlength="26" class="text input-medium"/>
						<select class="input-small" id="WEP1Select" name="WEP1Select"> 
						<option value="1">ASCII</option>
						<option value="0">Hex</option>
						</select>
					</div>
				</div>    
				<div class="control-group"> 
					<label class="control-label">WEP Key 2</label>
					<div class="controls" id="td_wep2">
						<input type="password" name="wep_key_2" id="WEP2" maxlength="26" class="text input-medium"/>
						<select id="WEP2Select" name="WEP2Select" class="input-small">
							<option value="1">ASCII</option>
							<option value="0">Hex</option>
						</select>
					</div>
				</div>
				<div class="control-group"> 
					<label class="control-label">WEP Key 3</label>
					<div id="td_wep3" class="controls">
						<input type="password" name="wep_key_3" id="WEP3" maxlength="26" class="text input-medium" />
						<select id="WEP3Select" name="WEP3Select" class="input-small">
							<option value="1">ASCII</option>
							<option value="0">Hex</option>
						</select>
					</div>
				</div>
				<div class="control-group"> 
					<label class="control-label">WEP Key 4</label>
					<div class="controls" id="td_wep4">
						<input type="password" name="wep_key_4" id="WEP4" maxlength="26" class="text input-medium"/>
						<select id="WEP4Select" name="WEP4Select" class="input-small">
							<option value="1">ASCII</option>
							<option value="0">Hex</option>
						</select>
						<span class="help-block">Default ASCII password : ASCII</span>
					</div>
				</div>
			<!--</div>-->
			</fieldset>
			<!-- WPA -->
			<fieldset id="div_wpa" name="div_wpa" class="table-field">        
				<div class="control-group"> 
					<label class="control-label">WPA Algorithms</label>
					<div class="controls" id="wpa_rule">
						<label class="radio"><input name="cipher" value="aes" type="radio" checked="checked" />AES<span id="tui" style="display:none">(Recommended)</span></label>
						<label class="radio"><input name="cipher" value="tkip" type="radio" />TKIP</label>
						<label class="radio"><input name="cipher" value="tkip+aes" type="radio" />TKIP&amp;AES</label>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">Security Key</label>
					<div class="controls" id="td_wlpwd">
						<input name="passphrase" id="passphrase" type="password" size="28" maxlength="63" class="text" />
						<span class="help-block"><span>Default</span>: 12345678</span>
					</div>
				</div>        
				<div class="control-group" style="display:none">
					<label class="control-label" style="display:none">Key Renewal Interval</label>
					<div class="controls"><input name="keyRenewalInterval" size="4" maxlength="4" /> Seconds</div>
				</div>    
			</fieldset>
			<div id="wl_wan_wps">
			<hr />
			<p class="text-red" style="padding-left:160px;">To configure a wireless security key, disable the WPS below!</p>
			<div class="control-group">
				<label class="control-label" id="wpsWPS_text">WPS Status</label>
				<div class="controls">
					<label id="wps_status" class="radio">Configured</label>
				</div>
			</div>
			<div class="control-group">
				<label class="control-label" id="wpsWPS_text">WPS Settings</label>
				<div class="controls">
					<label class="radio"><input type="radio" name="wpsenable" value="disabled" checked="checked" onClick="onwpsuse(0)" />Disable</label>
					<label class="radio"><input type="radio" name="wpsenable" value="enabled" onClick="onwpsuse(1)" />Enable</label>
				</div>
			</div>
			<div id="wifiuse_wps" class="control-group">
				<label class="control-label">WPS Mode</label>
				<div nowrap="nowrap" class="controls">
					<label class="radio"><input type="radio" name="wpsMode" value="pbc" onClick="onSel(0)" checked="checked" />PBC&nbsp;</label>
					<label class="radio"><input type="radio" name="wpsMode" value="pin" onClick="onSel(1)" />PIN</label>
					<input type="text" name="PIN" id="PIN" size="8" maxlength="8" class="text input-small" style="display:none" />
				</div>	
			</div>
            <div id="device_pin" class="control-group">
				<label class="control-label">AP PIN Code</label>
				<div class="controls" style="margin-top:4px;">
					<span id="pin_value"></span>
				</div>
			</div>
			<table style="width:100%">
				<tr>
				    <td><input class="btn btn-mini fr" type="button" id="rstOOB" name="rstOOB" value='Reset OOB' onClick="resetOOB();" /></td>
				</tr>
			</table>
            <h2 class="legend">Notice</h2>
		<table>
			<tr><td valign="top">WL_WPS_MSG</td>
			</tr>
		</table>
            </div>
		</fieldset>
		<div class="btn-group">
			<input type="button" class="btn" value="OK" onClick="preSubmit(document.security_form)" />
			<input type="button" class="btn last" value="Cancel" onClick="init(document.security_form)" />
		</div>
	</form>
	<div id="save" class="none"></div>

	<!-- Le javascript
  ================================================== -->
  <!-- Placed at the end of the document so the pages load faster -->
	<script src="lang/b28n.js"></script>
	<script>
		var SSID_ASP = ["<% get_wireless_basic("SSID"); %>", "<% get_wireless_basic("SSID1"); %>"];
		
		//handle translate
		(function() {
			B.setTextDomain(["base", "wireless"]);
			B.translate();
		})();
	</script>
	<script src="js/libs/tenda.js"></script>
	<script src="js/gozila.js"></script>
	<script src="js/wireless_security.js"></script>
</body>
</html>