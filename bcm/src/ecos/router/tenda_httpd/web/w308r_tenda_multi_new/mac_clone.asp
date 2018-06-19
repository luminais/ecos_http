<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<title>WAN | Dynamic IP</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body>
<form name="frmSetup" method=POST action="/goform/AdvSetMacClone">
<input type=hidden name=GO value=mac_clone.asp >
<input type=hidden name=cloneEn />
<input type=hidden name=WMAC />
	<fieldset class="table-field">
		<legend>MAC Clone</legend>
		<div id="mac_td" class="control-group">
			<label class="control-label">MAC Address</label>
			<div class="controls">
				<input type="text" class="text" name="WMAC1" size="17" maxlength="17" />
			</div>
		</div>
		<div id="noauto" class="btn-group">
			<input type="button" class="btn btn-small" id="def_mac" value="Restore Default MAC" />
			<input type="button" class="btn btn-small" id="clone_mac" value="Clone MAC Address" />
		</div>
	</fieldset>
	<div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.frmSetup)" />
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
<script language=JavaScript src="js/libs/tenda.js"></script>
<script language=JavaScript src="js/gozila.js"></script>
<script language=JavaScript>
var cln_MAC = "<%aspTendaGetStatus("sys","manmac");%>";
	def_MAC = "<%aspTendaGetStatus("sys","wanmac");%>";
	fac_MAC = "<%aspTendaGetStatus("sys","fmac");%>";
function preSubmit(f) {
	var mac,
		j=0,
		len,
		i;
	f.WMAC.value = f.WMAC1.value;
	mac = f.WMAC.value;
	
	if(mac == "" || mac == "00:00:00:00:00:00") {
		alert(_("Please specify a valid MAC address!"));
		return;
	}

	if(!CheckMAC(mac)) {
		alert(_("The MAC address of %s is invalid! Please specify a valid MAC address!",[mac]));
		return;
	}
	if(!ckMacReserve(mac))return ;
	f.WMAC.value = 	mac.toLocaleUpperCase();
	form2Cfg(f);
	f.submit();
	showSaveMassage();
}
	
(function (window) {
	"use strict";
	addCfg("WMAC1",31,def_MAC);
	addCfg("cloneEn",32,"1");

	function defMac(f) {
		f.cloneEn.value=0;
		decomMAC2(f.WMAC1, fac_MAC, 1);
		f.WMAC.value=f.WMAC1.value;
	}

	function cloneMac(f) {
		f.cloneEn.value=1;
		decomMAC2(f.WMAC1, cln_MAC, 1);
		f.WMAC.value=f.WMAC1.value;
	}

	function initEvent(f) {
		T.Event.on("def_mac", "click", function () {
			defMac(f);
		});
		T.Event.on("clone_mac", "click", function () {
			cloneMac(f);
		});
	}
	function init(){
		var f = document.frmSetup;
		cfg2Form(f);
		initEvent(f);
	}
	window.onload = init;
})(window);
</script>
</body>
</html>
