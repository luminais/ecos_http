(function(window) {
var illegal_user_pass = ["\\r","\\n","\\","'","\""],
	illegal_wl_pass = ["\\","\"","%"];

addCfg("PUN", 50, def_PUN);
addCfg("PPW", 54, def_PPW );
addCfg("wirelesspassword",59, def_wirelesspassword);

function chkPOE(f) {
	var pun = T.dom.byId("PUN").value,
		ppw = T.dom.byId("PPW").value;
	if(pun == "请输入宽带运营商提供的账号" || pun == "" || ppw == "") {
		alert("宽带用户名与宽带密码不可为空!");
		return false;
	} else {
		if(!ill_check(pun, illegal_user_pass, "宽带用户名")) return false;		
		if(!ill_check(ppw, illegal_user_pass, "宽带密码")) return false;		
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

	if(password.length < 8 && password.length != 0){
		 alert("输入的密码长度不能少于8个字符！");
		 return false;
	}
	if(!ill_check(password,illegal_wl_pass,"无线密码")) return false;
	for(i=0;i<password.length;i++){
		var c = password.substr(i,1);
		var ts = escape(c);
		if(ts.substring(0,2) == "%u"){
			alert("请不要提交中文字符!");
			return false;
		}
	}

	if(cloneway == "0"){
		var j=0;
		for(var i=0 ;i < wl_MAC_list.length;i++) {
			if(cln_MAC == wl_MAC_list[i]) {
				j=1;
			}
		}
		for(var k=0; k < wl_MAC_list2.length; k++) {
			if(cln_MAC == wl_MAC_list2[i]) {
				j=1;
			}
		}
		if(j==0) {
			if(config_num == "0"){
				f.MACC.value=cln_MAC;
			} else {
				if(def_wirelesspassword == password) {
					f.MACC.value=cln_MAC;
				}
			}
		}	
	}
	
	da = new Date();
	document.getElementById("v12_time").value = da.getTime()/1000;

	if (password.length == 0 ) {	
		alert("无线密码不能为空！");
		return false;
	}
	if(password != def_wirelesspassword || config_num == "0"){
		var con = confirm("无线密码已更改为 " + password +
				"，请以新的无线密码重新连接无线网络（无线信号名称：" +
				decodeSSID(ssid000) + "）。");
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

	T.dom.inputPassword("wirelesspassword", "密码位数不得少于8位");
}
function initDom(){
	T.dom.addPlaceholder("PUN", "请输入宽带运营商提供的账号", true);
	T.dom.inputPassword("PPW", "请输入宽带运营商提供的密码", true, true);
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
}
})(window);