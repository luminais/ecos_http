<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<title>WAN | Dynamic IP</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language=JavaScript src="js/libs/tenda.js"></script>
<script language=JavaScript src="js/gozila.js"></script>
</head>

<body>
<form name=frmSetup method=POST action="/goform/AdvSetMacClone">
<input type=hidden name=GO value=mac_clone.asp >
<input type=hidden name=cloneEn />
<input type=hidden name=WMAC />
	<fieldset class="table-field">
		<legend>MAC地址克隆</legend>
		<div class="control-group">
			<label class="control-label">克隆方式</label>
			<div class="controls">
				<label class="radio" for="auto">
						<input type="radio" hidefocus="true" name="autoenable" value="0" checked="checked" id="auto"/>自动克隆
				</label>
				<label class="radio" for="hand">
						<input type="radio" hidefocus="true" name="autoenable" value="1" id="hand" />手动克隆
				</label>
			</div>
		</div>
		<div id="mac_td" class="control-group">
			<label class="control-label">MAC地址</label>
			<div class="controls">
				<input type="text" class="text" name="WMAC1" size="17" maxlength="17" />
			</div>
		</div>
		<div id="noauto" class="btn-group none">
			<input type="button" class="btn btn-small" id="def_mac" value="恢复默认MAC地址" />
			<input type="button" class="btn btn-small" id="clone_mac" value="克隆MAC地址" />
		</div>
	</fieldset>
<script>tbl_tail_save("document.frmSetup");</script>
</form>
<div id="save" class="none"></div>
<script language=JavaScript>
var cln_MAC = "<%aspTendaGetStatus("sys","manmac");%>";
	def_MAC = "<%aspTendaGetStatus("sys","wanmac");%>";
	fac_MAC = "<%aspTendaGetStatus("sys","fmac");%>";
	cloneway="<%aspTendaGetStatus("sys","clnway");%>";
	wl_MAC_list = new Array(<%get_wireless_station("macinfo");%>);
function preSubmit(f) {
	var mac,
		j=0,
		len,
		i;
	f.WMAC.value = f.WMAC1.value;
	if(f.autoenable[0].checked) {
		for(i = 0, len = wl_MAC_list.length; i < len; i++) {
			if(cln_MAC == wl_MAC_list[i]) {
				j = 1;
			}
		}
		if(j==0) {
				f.WMAC.value=cln_MAC;
		}
	}
	mac = f.WMAC.value;

	if(!CheckMAC(mac)) {
		alert("MAC:"+ mac +" 无效!");
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
	
	function oncloneuse(x) {
		var f = document.frmSetup;
		if(x==0) {
			T.dom.addClass("noauto", "none");
			f.WMAC1.disabled=true;
		}
		if(x==1) {
			T.dom.removeClass("noauto", "none");
			f.WMAC1.disabled=false;
		}
	}

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
		T.Event.on("auto", "click", function () {
			oncloneuse(0);
		});
		T.Event.on("hand", "click", function () {
			oncloneuse(1);
		});
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
		if (cloneway=="0") {
			f.autoenable[0].click();
		} else if(cloneway=="1") {
			f.autoenable[1].click();
		}
	}
	window.onload = init;
})(window);
</script>
</body>
</html>
