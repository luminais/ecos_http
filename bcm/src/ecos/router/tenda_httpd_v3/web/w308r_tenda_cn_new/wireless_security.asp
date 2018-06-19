<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<meta http-equiv="Pragma" content="no-cache" />
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/libs/tenda.js"></script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var mode, pin, wpsmethod, enablewireless, isWPSConfiguredASP, secs, f,
	timerID 		= null,
	timerRunning 	= false,
	timeout	 		= 10,
	delay 			= 1000,
	OOB 			= 0,
	num_pwd			= 0,
	changed         = 0,
	SSID = [],
	Secumode = [],
	AuthMode = [],
	EncrypType = [],
	DefaultKeyID = [],
	Key1Type = [],
	Key1Str = [],
	Key2Type = [],
	Key2Str = [],
	Key3Type = [],
	Key3Str = [],
	Key4Type = [],
	Key4Str = [],
	WPAPSK = [],
	Wepenable = [],
	RekeyInterval = [],
	Cur_ssid_index = [],
	wl0_mode = [],
	SSID_ASP = new Array(2);
	
SSID_ASP[0] = "<% get_wireless_basic("SSID"); %>";
SSID_ASP[1] = "<% get_wireless_basic("SSID1"); %>";
function trim(s){
    try{
        return s.replace(/^\s+|\s+$/g,"");
    }catch(e){
        return s;
    }
}
function InitializeTimer(){
    secs = timeout;
    StopTheClock();
    StartTheTimer();
}
function StopTheClock() {
    if(timerRunning) {
        clearTimeout(timerID);
	}
    timerRunning = false;
}
function StartTheTimer(){
    if (secs==0){
		StopTheClock();	
		secs = timeout;
		StartTheTimer();
    } else {
        secs = secs - 1;
        timerRunning = true;
        timerID = self.setTimeout("StartTheTimer()", delay);
    }
}
function checkHex(str){
	var r=/^[0-9a-fA-F]+$/;
	if(r.test(str)){
		return true;
	} else {
		return false;
	}
}
function checkInjection(str){
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == ':' || str.charAt(i) == ',' ||
			  str.charAt(i) == '\r' || str.charAt(i) == '\n' ||
			  str.charAt(i) == '\"' || str.charAt(i) == '\'' ||
			  str.charAt(i) == '\\' || str.charAt(i) == '\/' ||
			  str.charAt(i) == '“' || str.charAt(i) == '‘' ||
			  str.charAt(i) == '’' || str.charAt(i) == '”' ||
			  str.charAt(i) == ' ' ){
				return false;
		}
	}
    return true;
}
function initHtml() {
	var f = document.security_form;
	T.dom.inputPassword("WEP1", "请输入密码");
	T.dom.inputPassword("WEP2", "请输入密码");
	T.dom.inputPassword("WEP3", "请输入密码");
	T.dom.inputPassword("WEP4", "请输入密码");
	T.dom.inputPassword("passphrase", "密码位数不得少于8位");
}

var illegal_user_pass = ["\\r","\\n","\\","&","%","!", "#","$","^","*","(",")","-","+","=","?",";",":","'","\"","<",">",",","~"];
var illegal_wl_pass = ["\\","\"","%"];
var http_request = false;
function makeRequest(url, content){
	http_request = GetReqObj();
	http_request.onreadystatechange = alertContents;
	http_request.open('GET', url, true);
	http_request.send(content);
}

function alertContents(){	
	var f = document.security_form;
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			parseAllData(http_request.responseText);
			UpdateMBSSIDList();
			LoadFields();
			if(mode == "disabled"){
				f.wpsenable[0].checked = true;
				document.getElementById("wifiuse_wps").style.display="none";
				document.getElementById("rstOOB").disabled = true;
	 			document.getElementById("security_mode").disabled=false;
	 			f.passphrase.disabled=false;
	 			f.keyRenewalInterval.disabled=false;
				document.getElementById("wl_wan_wps").style.display = ""; 			
			} else if (mode == "enabled"){
				f.wpsenable[1].checked = true;
				onwpsuse(1);
				if(wpsmethod == "pbc"){
				   f.wpsMode[0].checked = true;
				   document.getElementById("PIN").style.display = "none";
				}else if(wpsmethod == "pin"){
				   f.wpsMode[1].checked = true;
				   document.getElementById("PIN").style.display = "";
				   f.PIN.value = pin;
				}
			}
			if(wl0_mode == "sta") {
				onwpsuse(1);
				document.getElementById("wl_wan_wps").style.display = "none";			
			}	
			initHtml();
			InitializeTimer();
		}
	}
}
var http_request1 = null;
function makeRequest1(url, content) {
	http_request1 = GetReqObj();
	http_request1.onreadystatechange = alertContents1;
	http_request1.open('GET', url, true);
	http_request1.send(content);
}
function alertContents1() {
	if (http_request1.readyState == 4) {
		if (http_request1.status == 200) {
		    if(OOB == 1){
				window.document.body.style.cursor = "auto";
				document.getElementById("rstOOB").disabled = false;
				OOB = 0;
			}	
		}
	}
}
function UpdateMBSSIDList() {
	f=document.security_form;
	var defaultSelected = false;
	var selected = false;
	var optionStr = '<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">';
	
	old_MBSSID = Cur_ssid_index[0];
	for(var i=0; i<SSID.length; i++){
		if(SSID[i] != "") {
			if (old_MBSSID == i) {
				optionStr += '<option selected="selected">' + SSID[i] + '</option>';
			} else {
				optionStr += '<option>' + SSID[i] + '</option>';
			}
		}
	}
	optionStr += '</select>';	
	document.getElementById('ssid-select-content').innerHTML = optionStr;
}
var http_request2 = null;
function alertContents2(){
	if (http_request2.readyState == 4) {
		if (http_request2.status == 200) {
		 	var str=http_request2.responseText.split("\r");
		  	mode = str[0] ;
		 	pin =str[1];
			wpsmethod = str[2];
			enablewireless= str[3];
			isWPSConfiguredASP = str[4];
			initAll();
		}
	}
}
function resetOOB(){
   window.document.body.style.cursor = "wait";
   OOB = 1;
   makeRequest1("/goform/OOB", "something");
}
function parseAllData(str) {
	var all_str = str.split("\n");
	for (var i=0; i<all_str.length; i++) {
		var fields_str = [];
		fields_str = all_str[i].split("\r");

		SSID[i] = SSID_ASP[i];
		if(i == 1) break;
		Secumode[i] = trim(fields_str[1]);
		AuthMode[i] = trim(fields_str[2]);
		EncrypType[i] = trim(fields_str[3]);
		DefaultKeyID[i] = trim(fields_str[4]);
		Key1Type[i] = trim(fields_str[5]);
		Key1Str[i] = trim(fields_str[6]);
		Key2Type[i] = trim(fields_str[7]);
		Key2Str[i] = trim(fields_str[8]);
		Key3Type[i] = trim(fields_str[9]);
		Key3Str[i] = trim(fields_str[10]);
		Key4Type[i] = trim(fields_str[11]);
		Key4Str[i] = trim(fields_str[12]);
		WPAPSK[i] = trim(fields_str[13]);
		Wepenable[i] = trim(fields_str[14]);
		RekeyInterval[i] = trim(fields_str[15]);
		Cur_ssid_index[i] = trim(fields_str[16]);
		wl0_mode[i] = trim(fields_str[17]);
		if(AuthMode[i] == "0" && (Secumode[i]) == "" && Wepenable[i] == "disabled" )
			AuthMode[i] = "Disable";
		if(AuthMode[i] == "0" && Secumode[i] == "psk" &&  Wepenable[i] == "disabled"  )
			AuthMode[i] = "psk";
		if(AuthMode[i] == "0" && Secumode[i] == "psk2" &&  Wepenable[i] == "disabled" )
			AuthMode[i] = "psk2";
		if(AuthMode[i] == "0" && (Secumode[i] == "psk psk2") &&  Wepenable[i] == "disabled" )
			AuthMode[i] = "psk psk2";
		if(AuthMode[i] == "0" && Secumode[i] == "0" && Wepenable[i] == "enabled")//open
			AuthMode[i] = "0";
		if(AuthMode[i] == "1" && Secumode[i] == "1" && Wepenable[i] == "enabled")//share
			AuthMode[i] = "1";
	}
}
function checkData() {
	f = document.security_form;	
	var securitymode = f.security_mode.value;
	if (securitymode == "0" || securitymode == "1") {
		return check_Wep(securitymode);
	} else  if (securitymode == "psk" || securitymode == "psk2" || securitymode == "psk psk2" ){
		var keyvalue = f.passphrase.value;
		if ((keyvalue.length < 8) || (keyvalue.length > 63)){
			alert('密码范围: 8~63 个字符!');
			return false;
		}
		if(!ill_check(f.passphrase.value,illegal_wl_pass,"密码")) return false;
		/*if(f.cipher[0].checked != true && 
		   f.cipher[1].checked != true &&
   		   f.cipher[2].checked != true){
   		   alert('请选择一个WPA加密规则。');
   		   return false;
		}
		var x=Number(f.keyRenewalInterval.value);
		f.keyRenewalInterval.value=x;
		if(isNaN(x)){
			alert('密码更新周期无效。');
			return false;
		}
		if(x < 60){
			alert('警告：密码更新周期太短,范围：60~9999');
			return false;
		}*/
		if(keyvalue!=WPAPSK)
		{
			var con=confirm("无线密码已更改为 "+keyvalue+"，请以新的无线密码重新连接无线网络（无线信号名称："+decodeSSID(SSID[old_MBSSID])+"）。");
			if(con==false)
			{	
				f.passphrase.value = WPAPSK;
				return false;
			}
			num_pwd=1;
		}
	}
	return true;
}

function securityMode(){
	f=document.security_form;
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wep").style.display = "none";
	document.getElementById("div_security_shared_mode").style.display = "none";
	f.passphrase.disabled = true;
	f.keyRenewalInterval.disabled = true;
	document.getElementById("div_wep").disabled = false;
	document.getElementById("div_security_shared_mode").disabled = false;
	var security_mode = f.security_mode.value;	
    if (security_mode == "0" || security_mode == "1") {
		document.getElementById("div_wep").style.display = "";
		if(security_mode == "1")
			document.getElementById("div_security_shared_mode").style.display="";
	} else if (security_mode == "psk" || security_mode == "psk2" || security_mode == "psk psk2"){
		<!-- WPA -->
		document.getElementById("div_wpa").style.display = "";
		if(security_mode == "psk") {
			//f.cipher[2].checked = false; 
			f.cipher[2].disabled=true;
			document.getElementById("tui").style.display="";
		}
		if(security_mode == "psk2" || security_mode == "psk psk2"){
			f.cipher[2].disabled = false;
			document.getElementById("tui").style.display="none";
		}
		f.passphrase.disabled = false;
		f.keyRenewalInterval.disabled = false;
	}
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function check_Wep(securitymode){
	f = document.security_form;
	var defaultid = f.wep_default_key.value,
		keyvalue,
		keylength,
		form_ele=f.elements,
		keyvalue0;
	if(defaultid==1) {
		keyvalue0 = Key1Str;
	} else if (defaultid==2) {
		keyvalue0=Key2Str;
	} else if (defaultid==3) {
		keyvalue0=Key3Str;
	} else {
		keyvalue0=Key4Str;
	}
	keyvalue=form_ele["wep_key_"+defaultid].value;	
	if (keyvalue.length == 0 &&  (securitymode == "1" || securitymode == "0")){
		alert('请输入WEP密码'+defaultid+' !');
		return false;
	}
	for(var i=1;i<5;i++) {
		if(!ill_check(form_ele["wep_key_"+i].value,illegal_wl_pass,"密码")) return false;
		keylength = form_ele["wep_key_"+i].value.length;
		if (keylength != 0){
			if (form_ele["WEP"+i+"Select"].options.selectedIndex == 0){
				if(keylength != 5 && keylength != 13) {
					alert('WEP密码'+i+'无效，请输入5或13个ASCII码！');
					return false;
				}
			}
			else{
				if(keylength != 10 && keylength != 26) {
					alert('WEP密码'+i+'无效，请输入10或26个Hex码！');
					return false;
				}
				if(checkHex(form_ele["wep_key_"+i].value) == false){
					alert('WEP密码'+i+'包含无效字符。');
					return false;
				}
			}
		}
	}
	if(defaultid != DefaultKeyID) {
		var con=confirm("无线密码改为采用WEP密码"+defaultid+" 密码为 "+keyvalue+"，请以新的无线密码重新连接无线网络（无线信号名称："+decodeSSID(SSID[old_MBSSID])+"）。");
		if(con==false)
			{	
				f.wep_default_key.value = DefaultKeyID;
				form_ele["wep_key_"+defaultid].value = keyvalue0;
				return false;
			}
		num_pwd=1;
	} else if (keyvalue!=keyvalue0) {
		var con=confirm("无线密码已更改为 "+keyvalue+"，请以新的无线密码重新连接无线网络（无线信号名称："+decodeSSID(SSID[old_MBSSID])+"）。");
		if(con==false) {
			form_ele["wep_key_"+defaultid].value = keyvalue0;
			return false;
		}
		num_pwd=1;
	}
	return true;
}

function preSubmit(){
	var submitStr = "/goform/wirelessSetSecurity?GO=wireless_security.asp",
		t = /^[0-9]{8}$/,
		wifiEn,
		wpsmethod,
		v;
		
	f = document.security_form;
    if(f.wpsenable[1].checked == true){
		wifiEn = "enabled";
        if(f.wpsMode[0].checked) {
			wpsmethod ="pbc"
		} else if(f.wpsMode[1].checked) {
			wpsmethod ="pin";
		}
		submitStr += "&wifiEn=" + wifiEn ;		
	    submitStr += "&wpsmethod="+ wpsmethod;	
	    if(wifiEn == "enabled" && wpsmethod == "pin"){
			v = f.PIN.value;
			if(!t.test(v)){
					alert("PIN值无效！请输入8位十进制数");
					return false;
			}	
			submitStr += "&PIN=" + v;
		}
		location = submitStr;
	} else if(f.wpsenable[0].checked == true){
	    f.wifiEn.value="disabled";
		f.wpsmethod.value="pbc";
		if (checkData() == true){
			changed = 0;
			f.submit();
			if(num_pwd == 0){
				showSaveMassage();
			}
		}
	}
}

function LoadFields(){
	var result;
	f=document.security_form;
	sp_select = document.getElementById("security_mode");
	sp_select.options.length = 0;
	sp_select.options[sp_select.length] = new Option("禁用","Disable");
	sp_select.options[sp_select.length] = new Option("Open","0");	//0:open
	sp_select.options[sp_select.length] = new Option("Shared","1");	//1:share
	sp_select.options[sp_select.length] = new Option("WPA - PSK(推荐)","psk");
	sp_select.options[sp_select.length] = new Option("WPA2 - PSK","psk2");
	sp_select.options[sp_select.length] = new Option("Mixed WPA/WPA2 - PSK","psk psk2");
	// WEP
	document.getElementById("WEP1").value = Key1Str;
	document.getElementById("WEP2").value = Key2Str;
	document.getElementById("WEP3").value = Key3Str;
	document.getElementById("WEP4").value = Key4Str;
	document.getElementById("WEP1Select").selectedIndex = (Key1Type == "0" ? 1 : 0);
	document.getElementById("WEP2Select").selectedIndex = (Key2Type == "0" ? 1 : 0);
	document.getElementById("WEP3Select").selectedIndex = (Key3Type == "0" ? 1 : 0);
	document.getElementById("WEP4Select").selectedIndex = (Key4Type == "0" ? 1 : 0);

	document.getElementById("wep_default_key").selectedIndex = parseInt(DefaultKeyID) - 1 ;
	sp_select.value = Secumode[0] || "Disable";
	// WPA
	if(EncrypType == "tkip")
		f.cipher[1].checked = true;
	else if(EncrypType == "aes")
		f.cipher[0].checked = true;
	else if(EncrypType == "tkip+aes")
		f.cipher[2].checked = true;
	f.passphrase.value = WPAPSK;
	f.keyRenewalInterval.value = RekeyInterval;
	securityMode();
}
function initAll() {	
	if(enablewireless==1){
	    document.getElementById("rstOOB").disabled = true;
		makeRequest("/goform/wirelessGetSecurity", "something");
	}else{
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href="wireless_basic.asp";
	}
}

function onwpsuse(x){
	f = document.security_form;
	if(x==0){
		document.getElementById("div_wep").style.display = "none";
		document.getElementById("div_security_shared_mode").style.display = "none";
		document.getElementById("wifiuse_wps").style.display="none";
		document.getElementById("div_wpa").style.display="";
		document.getElementById("tui").style.display="";
		document.getElementById("rstOOB").disabled=true;
		document.getElementById("security_mode").disabled=false;
		f.security_mode.options.selectedIndex=3;
		document.getElementById("div_wpa").disabled=false;
		f.cipher[0].checked = true;
		f.cipher[2].disabled=true;
		f.passphrase.disabled=false;
		document.getElementById("wl_wan_wps").style.display = "";
		//f.keyRenewalInterval.disabled=false; 
	}else if(x==1){
		document.getElementById("wifiuse_wps").style.display="";
		document.getElementById("rstOOB").disabled=false;
		document.getElementById("security_mode").disabled=true;
		document.getElementById("div_wpa").disabled=true;
		document.getElementById("div_wep").disabled = true;
		document.getElementById("div_security_shared_mode").disabled = true;
		f.passphrase.disabled=true;
		document.getElementById("wl_wan_wps").style.display = "";
		
		//reset this iframe height by call parent frame function
		window.parent.reinitIframe();
	}
} 
function onSel(n) {
	if(n) {
		document.getElementById("PIN").style.display = '';
	} else {
		document.getElementById("PIN").style.display = 'none';
	}
}

function init(){
	http_request2 =GetReqObj();
	http_request2.onreadystatechange = alertContents2;
	http_request2.open('GET', "/goform/wirelessInitSecurity", true);
	http_request2.send("something");
}

function selectMBSSIDChanged() {
	var ssid_index;
	// check if any security settings changed
	if(changed){
		var ret = confirm("确信忽略更改?");
		if(!ret){
			document.security_form.ssidIndex.options.selectedIndex = old_MBSSID;
			return false;
		}
		else
			changed = 0;
	}
	ssid_index = document.security_form.ssidIndex.options.selectedIndex;
	
	var loc = "/goform/onSSIDChange?GO=wireless_security.asp";

	loc += "&ssid_index=" + ssid_index; 
	location = loc;
}
</script>
</head>
<body onLoad="init();">
<form method="post" name="security_form" action="/goform/wirelessSetSecurity">
	<input type="hidden" name="wifiEn" id="wifiEn" />
    <input type="hidden" name="wpsmethod" id="wpsmethod" />
    <input type="hidden" id="GO" name="GO" value="wireless_security.asp" />
	<fieldset>
		<h2 class="legend">无线加密</h2>
		<div class="control-group">
			<label class="control-label">选择无线信号名称(SSID)</label>
			<div class="controls">
				<span id="ssid-select-content"><select name="ssidIndex" id="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
				</select></span>
			</div>
		</div>
		<div class="control-group">
			<label class="control-label" for="security_mode">安全模式</label>
			<div class="controls">
				<select name="security_mode" id="security_mode" size="1" onChange="securityMode()"></select>
			</div>
		</div>
		<div class="control-group" id="div_security_shared_mode">
			<label class="control-label">加密类型</label>
			<div class="controls">
				<select name="security_shared_mode">
				  <option value="enable">WEP</option>
				</select>
			</div>
		</div>
		<!-- WEP -->
		<div id="div_wep" name="div_wep">
			<div class="control-group"> 
				<label class="control-label">默认密码</label>
				<div class="controls">
					<select name="wep_default_key" id="wep_default_key" size="1">
						<option value="1" selected="selected">密码 1</option>
						<option value="2">密码 2</option>
						<option value="3">密码 3</option>
						<option value="4">密码 4</option>
					</select>
				</div>
			</div>
			<div class="control-group">
				<label class="control-label" for="WEP1">WEP密码1</label>
				<div class="controls">
					<input type="password" name="wep_key_1" id="WEP1" maxlength="26" class="text input-medium"/>
					<select class="input-small" id="WEP1Select" name="WEP1Select"> 
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
					</select>
				</div>
			</div>    
			<div class="control-group"> 
				<label class="control-label">WEP密码2</label>
				<div class="controls" id="td_wep2">
					<input type="password" name="wep_key_2" id="WEP2" maxlength="26" class="text input-medium"/>
					<select id="WEP2Select" name="WEP2Select" class="input-small">
						<option value="1">ASCII</option>
						<option value="0">Hex</option>
					</select>
				</div>
			</div>
			<div class="control-group"> 
				<label class="control-label">WEP密码3</label>
				<div id="td_wep3" class="controls">
					<input type="password" name="wep_key_3" id="WEP3" maxlength="26" class="text input-medium" />
					<select id="WEP3Select" name="WEP3Select" class="input-small">
						<option value="1">ASCII</option>
						<option value="0">Hex</option>
					</select>
				</div>
			</div>
			<div class="control-group"> 
				<label class="control-label">WEP密码4</label>
				<div class="controls" id="td_wep4">
					<input type="password" name="wep_key_4" id="WEP4" maxlength="26" class="text input-medium"/>
					<select id="WEP4Select" name="WEP4Select" class="input-small">
						<option value="1">ASCII</option>
						<option value="0">Hex</option>
					</select>
					<span class="help-block">默认ASCII密码为（ASCII）</span>
				</div>
			</div>
		</div>
		<!-- WPA -->
		<fieldset id="div_wpa" name="div_wpa">        
			<div class="control-group"> 
				<label class="control-label">WPA加密规则</label>
				<div class="controls" id="wpa_rule">
					<label class="radio"><input name="cipher" value="aes" type="radio" checked="checked" />AES<span id="tui" style="display:none">(推荐)</span></label>
					<label class="radio"><input name="cipher" value="tkip" type="radio" />TKIP</label>
					<label class="radio"><input name="cipher" value="tkip+aes" type="radio" />TKIP&amp;AES</label>
				</div>
			</div>
			<div class="control-group">
				<label class="control-label">密码</label>
				<div class="controls" id="td_wlpwd">
					<input name="passphrase" id="passphrase" type="password" size="28" maxlength="63" class="text" />
					<span class="help-block">默认密码为（12345678）</span>
				</div>
			</div>        
			<div class="control-group" style="display:none">
				<label class="control-label" style="display:none">密码更新周期</label>
				<div class="controls"><input name="keyRenewalInterval" size="4" maxlength="4" /> 秒</div>
			</div>    
		</fieldset>
		<div id="wl_wan_wps">
		<hr />
		<p class="text-red">如需设置无线密码，请在下方选择“禁用WPS”</p>
		<div class="control-group">
			<label class="control-label" id="wpsWPS_text">WPS设置</label>
			<div class="controls">
				<label class="radio"><input type="radio" name="wpsenable" value="disabled" checked="checked" onClick="onwpsuse(0)" />禁用</label>
				<label class="radio"><input type="radio" name="wpsenable" value="enabled" onClick="onwpsuse(1)" />启用</label>
			</div>
		</div>
		<div id="wifiuse_wps" class="control-group">
			<label class="control-label">WPS设置模式</label>
			<div nowrap="nowrap" class="controls">
				<label class="radio"><input type="radio" name="wpsMode" value="pbc" onClick="onSel(0)" checked="checked" />PBC&nbsp;</label>
				<label class="radio"><input type="radio" name="wpsMode" value="pin" onClick="onSel(1)" />PIN</label>
				<input type="text" name="PIN" id="PIN" size="8" maxlength="8" class="text input-small" style="display:none" />
			</div>	
		</div>
		<table align="right">
			<tr>
				<td align="right"><input class="btn btn-mini" type=button id="rstOOB" name="rstOOB" value='重设OOB' onClick="resetOOB();" /></td>
			</tr>
		</table></div>
	</fieldset>
	<script>tbl_tail_save("document.security_form");</script>
</form>
<div id="save" class="none"></div>
</body>
</html>