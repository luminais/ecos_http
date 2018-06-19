(function(window) {
var illegal_user_pass = ["\\r","\\n","\\","'","\""],
	illegal_wl_pass = ["\\","'","\"","%"];

addCfg("PUN", 50, def_PUN);
addCfg("PPW", 54, def_PPW );
addCfg("wirelesspassword",59, def_wirelesspassword);

function chkPOE(f) {
	var pun = T.dom.byId("PUN").value,
		ppw = T.dom.byId("PPW").value;
	if(pun == _("Enter username provided by ISP") || pun == "" || ppw == "") {
		alert(_("Please enter the valid PPPoE username and PPPoE password provided by your ISP!"));
		return false;
	} else {
		if(!ill_check(pun, illegal_user_pass, "PPPoE Username")) return false;
		if(!ill_check(ppw, illegal_user_pass, "PPPoE Password")) return false;		
		form2Cfg(f);
		return true;
	}
} 

function doFinish(f) {
	form2Cfg(f);
	var password = f.wirelesspassword.value,
		aa = 1,
		da;
		
	if(f.net_type[0].checked === true) {
	 	aa = 3;
	} else if(f.net_type[1].checked === true) {
	 	aa = 2;
	}
	f.WANT1.value = aa;
	if (aa === 3) {
		if(!chkPOE(f)) {
			return;
		}
	}

	if(password.length < 8 || password.length == 0){
		 alert(_("Security key must contain 8~63 characters!"));
		 return false;
	}
	if(!ill_check(password,illegal_wl_pass, _("Security Key"))) {
		return false;
	}
	da = new Date();
	document.getElementById("v12_time").value = da.getTime() / 1000;
	if(password != def_wirelesspassword || config_num == "0"){
		var con = confirm(_("The security key will be changed to %s! Please reconnect to %s using the new security key!",[password,decodeSSID(ssid000)]));
		if(con === false){
			f.wirelesspassword.value = def_wirelesspassword;
			return false;	
		}
	}

	f.submit();
}

function onIspchange(x) {
	if(x==0){
		document.basicset.net_type[0].checked = true;
		document.getElementById("PUN").disabled=false;
		document.getElementById("PPW").disabled=false;
		document.getElementById("ppoe_set").style.display="";
	}
	if(x==1){
		document.basicset.net_type[1].checked = true;
		document.getElementById("PUN").disabled=true;
		document.getElementById("PPW").disabled=true;
		document.getElementById("ppoe_set").style.display="none";
	}
	if(x==2){
		document.basicset.net_type[0].checked = false;
		document.basicset.net_type[1].checked = false;
		document.getElementById("PUN").disabled=false;
		document.getElementById("PPW").disabled=false;
		document.getElementById("ppoe_set").style.display="";
	}
}

function initEvent() {
	T.Event.on("adsl", "click", function () {
		onIspchange(0);
	});
	T.Event.on("atuo", "click", function () {
		onIspchange(1);
	});
	T.Event.on("submit_ok", "click", function () {
		doFinish(document.basicset);
	});
	T.Event.on("submit_cancel", "click", function () {
		location.replace(location.href);
	});

	T.dom.inputPassword("wirelesspassword", _("8~63 characters"));
}
function initDom(){
	T.dom.addPlaceholder("PUN", _("Enter username provided by ISP"), true);
	T.dom.inputPassword("PPW", _("Enter password provided by ISP"), true, true);
	if(def_WANT==2) {
	 	onIspchange(1);
	} else if(def_WANT==1) {
	 	onIspchange(2);
	} else {
		onIspchange(0);
	}
}
window.onload = function () {
	if(location.href != top.location.href)
	{
		top.location.href = "index.asp";
	}
	cfg2Form(document.basicset);
	initEvent();
	initDom();
	
	B.translate();
}
})(window);