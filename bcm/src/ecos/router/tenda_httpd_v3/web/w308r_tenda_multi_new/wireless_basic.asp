<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<meta http-equiv="Pragma" content="no-cache" />
<title>Wireless Basic Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body>
	<form method=post name="wireless_basic" action="/goform/wirelessBasic" >
	<!--<input type="hidden" id="rebootTag" name="rebootTag">-->
	<input type="hidden" id="GO" name="GO" value="wireless_basic.asp">
	<input type="hidden" name="bssid_num" value="1">
		<fieldset class="table-field">
			<legend>Wireless Basic Settings</legend>
			<div class="control-group">
				<label for="enablewireless" class="control-label">Enable Wireless</label>
				<div class="controls">
					<input type="hidden" name="enablewirelessEx" />
					<input type="checkbox" name="enablewireless"  value="1" id="enablewireless" onClick="onenablewirelesschange()">
				</div>
			</div>
			
			<div id="divwieless" style="clear:both"><!-- 为了消除ie7bug 添加clear:both属性 -->
				<div class="control-group"> 
					<label class="control-label">Primary SSID</label>
					<div class="controls"><input type="text" name=ssid id=ssid size=20 maxlength=32 class="text" /></div>
				</div>
				<div class="control-group"> 
					<label class="control-label">Secondary SSID</label>
					<div class="controls"><input type="text" name=mssid_1 id=mssid_1 size=20 maxlength=32 class="text" value="" /></div>
				</div>
				<div class="control-group">
					<label class="control-label">Network Mode</label>
					<div class="controls">
						<select name="wirelessmode" id="wirelessmode" size="1" onChange="wirelessModeChange()">
							<option value=0>11b/g mixed mode</option>
							<option value=1>11b mode</option>
							<option value=4>11g mode</option>
							<option value=9>11b/g/n mixed mode</option>
						</select>
					</div>
				</div>      
				<div class="control-group"> 
					<label class="control-label">SSID Broadcast</label>
					<div class="controls">
						<label class="radio"><input type=radio name=broadcastssid value="0" checked />Enable</label>
						<label class="radio"><input type=radio name=broadcastssid value="1" />Disable</label>
					</div>
				</div>
				<div class="control-group"> 
					<label class="control-label">AP Isolation</label>
					<div class="controls">
						<label class="radio"><input type=radio name=ap_isolate value="1" checked />Enable</label>
						<label class="radio"><input type=radio name=ap_isolate value="0" />Disable</label>
					</div>
				</div>
				<div id="div_11b_channel" name="div_11b_channel" class="control-group">
					<label class="control-label">Channel</label>
					<div class="controls">
						<div id="11b"></div>
					</div>
				</div>
				
				<div id="div_11n" class="content2" style="display:none">
					<div class="control-group">
						<label class="control-label">Channel Bandwidth</label>
						<div class="controls">
							<label class="radio"><input type="radio" name="n_bandwidth" value="0" onClick="Channel_BandWidth_onClick()" checked />20&nbsp;</label>
							<label class="radio"><input type="radio" name="n_bandwidth" value="1" onClick="Channel_BandWidth_onClick()" />40</label>
							<label class="radio"><input type="radio" name="n_bandwidth" value="2" onClick="Channel_BandWidth_onClick()" />20/40</label>			
						</div>
					</div>
					<div class="control-group">
						<label class="control-label">Extension Channel</label>
						<div class="controls">
							<select id="n_extcha" name="n_extcha" size="1"></select>
						</div>
					</div>
				</div>
				
				<div id="div_wmm">
					<div class="control-group"> 
						<label class="control-label">WMM Capable</label>
						<div class="controls">
							<label class="radio"><input type=radio name=wmm_capable value="on" onClick="wmm_capable_enable_switch()" checked />Enable</label>
							<label class="radio"><input type=radio name=wmm_capable value="off" onClick="wmm_capable_enable_switch()" />Disable</label>
						</div>
					</div>
					<div id="div_apsd_capable" class="control-group">
						<label class="control-label">APSD Capable</label>
						<div class="controls">
							<label class="radio"><input type=radio name=apsd_capable value="on" />Enable</label>
							<label class="radio"><input type=radio name=apsd_capable value="off" checked />Disable</label>
						</div>
					</div>
				</div>
			</div>
		</fieldset>
		<div class="btn-group">
			<input type="button" class="btn" value="OK" onClick="preSubmit(document.wireless_basic)" />
			<input type="button" class="btn last" value="Cancel" onClick="init(document.wireless_basic)" />
		</div>
	</form>
	<div id="save" class="none"></div>

	<!-- Le javascript
	================================================== -->
	<!-- Placed at the end of the document so the pages load faster -->
	<script src="lang/b28n.js"></script>
	<script>
		var ssid000 = "<%get_wireless_basic("SSID");%>",
			ssid111 = "<%get_wireless_basic("SSID1");%>";
			
		//handle translate
		(function() {
			B.setTextDomain(["base", "wireless"]);
			B.translate();
		})();
	</script>
	<script src="js/gozila.js"></script>
	<script src="js/wireless_basic.js"></script>
</body>
</html>