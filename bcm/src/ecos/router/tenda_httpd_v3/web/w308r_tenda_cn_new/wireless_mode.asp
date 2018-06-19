<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>Wireless | Mode Setting</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/libs/tenda.js"></script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
//'ssid1,11:22:33:44:55:67,6,WEP,60;ssid2,11:22:33:44:55:68,6,NONE,68';
var request = GetReqObj(),
	enablewireless="<% get_wireless_basic("WirelessEnable"); %>",
	ssid000 = "<% get_wireless_basic("SSID"); %>",
	mode, //0;AP 1:STATION
	ssid,
	resmac,
	channel, //1~13
	sec, //Disable,0,1,psk,psk2,psk psk2
	wpaAlg,
	wpakeyVa,
	RekeyInterval,
	DefaultKeyID,
	Key1Type,
	Key1Str,
	Key2Type,
	Key2Str,
	Key3Type,
	Key3Str,
	Key4Type,
	Key4Str,
	wl0_mode,
	http_request = false;
function initEvent() {
	T.dom.inputPassword("WEP1", "请输入密码");
	T.dom.inputPassword("WEP2", "请输入密码");
	T.dom.inputPassword("WEP3", "请输入密码");
	T.dom.inputPassword("WEP4", "请输入密码");
	T.dom.inputPassword("passphrase", "密码位数不得少于8位");
}
function init(){
	makeRequest("/goform/wirelessInitMode", "something");
}
function makeRequest(url, content) {
	http_request = GetReqObj();
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}
function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
		 	var str=http_request.responseText.split("\r");
		  	mode = str[0] ;
		 	ssid =ssid000; //str[1];
			resmac = str[2];
			channel = str[3];
			sec = str[4];
			wpaAlg = str[5];
			wpakeyVa = str[6];
			RekeyInterval = str[7];
			
			DefaultKeyID = str[8] ;
			Key1Type = str[9] ;
			Key1Str = str[10];
			Key2Type = str[11];
			Key2Str = str[12];
			Key3Type = str[13];
			Key3Str = str[14];
			Key4Type = str[15];
			Key4Str = str[16];
			wl0_mode = str[17];
			initValue();
			initEvent();
		}
	}
}

function checkHex(str){
	var len = str.length;

	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
				(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
				(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') ){
			continue;
		} else {
	        return false;
		}
	}
    return true;
}


function checkInjection(str){
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == ':' || str.charAt(i) == ',' ||
				str.charAt(i) == '\r' || str.charAt(i) == '\n'){
			return false;
		} else {
	        continue;
		}
	}
    return true;
}

function initValue() {
	var security_mode,
		f = document.forms[0];
		
	f.sta_security_mode.value = sec;
	f.sta_ssid.value = decodeSSID(ssid);
	f.sta_mac.value = resmac;
	f.sta_channel.value = channel;
	hideWep();
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.wireless_mode.cipher[0].disabled = true;
	document.wireless_mode.cipher[1].disabled = true;
	document.wireless_mode.cipher[2].disabled = true;
	document.wireless_mode.passphrase.disabled = true;
	if(wl0_mode == "sta") {
		document.getElementById("WEP1").value = Key1Str;
		document.getElementById("WEP2").value = Key2Str;
		document.getElementById("WEP3").value = Key3Str;
		document.getElementById("WEP4").value = Key4Str;
	}

	document.getElementById("WEP1Select").selectedIndex = (Key1Type == "0" ? 1 : 0);
	document.getElementById("WEP2Select").selectedIndex = (Key2Type == "0" ? 1 : 0);
	document.getElementById("WEP3Select").selectedIndex = (Key3Type == "0" ? 1 : 0);
	document.getElementById("WEP4Select").selectedIndex = (Key4Type == "0" ? 1 : 0);

	document.getElementById("wep_default_key").selectedIndex = parseInt(DefaultKeyID) - 1 ;
	
	if(wpaAlg == "tkip") {
		document.wireless_mode.cipher[1].checked = true;
	} else if(wpaAlg == "aes") {
		document.wireless_mode.cipher[0].checked = true;
	} else if(wpaAlg == "tkip+aes") {
		document.wireless_mode.cipher[2].checked = true;
	}
	if(sec == "psk"){
		document.wireless_mode.cipher[2].disabled = true;
	} else if (sec == "psk2" || sec == "psk psk2"){
		document.wireless_mode.cipher[2].disabled = false;
	}
	if(wl0_mode == "sta")
		document.getElementById("passphrase").value = wpakeyVa;
	security_mode = f.sta_security_mode.value;
	
	if (security_mode == "0" || security_mode == "1" ||security_mode == "WEPAUTO"){
		showWep(security_mode);
	} else if (security_mode == "psk" || security_mode == "psk2" || security_mode == "psk psk2"){
		<!-- WPA -->
		document.getElementById("div_wpa").style.display = "";
		document.getElementById("div_wpa_algorithms").style.display = "";
		document.wireless_mode.cipher[0].disabled = false;
		document.wireless_mode.cipher[1].disabled = false;
		if(security_mode == "psk" && document.wireless_mode.cipher[2].checked) {
			document.wireless_mode.cipher[2].checked = false;
		}	
		if(security_mode == "psk2" || security_mode == "psk psk2") {
			document.wireless_mode.cipher[2].disabled = false;
		}
		document.getElementById("wpa_passphrase").style.display = "";
		document.wireless_mode.passphrase.disabled = false;
	}
	
	document.getElementById("wdsScanTab").style.display = "none";
	document.getElementById("wlSurveyBtn").value="开启扫描";
		
	if (mode == "1" && wl0_mode == "sta") {
		document.wireless_mode.wlMode[1].checked = true;
		onWlMode('sta');
	} else {
		document.wireless_mode.wlMode[0].checked = true;
		onWlMode('ap');
		
	}
	if (wl0_mode == "wet") {
		document.wireless_mode.wlMode[1].disabled = true;
		document.wireless_mode.wlMode[0].checked = true;
		onWlMode('ap');
	}
	
}
var illegal_user_pass = new Array("\\r","\\n","\\","&","%","!", "#","$","^","*","(",")","-","+","=","?",";",":","'","\"","<",">",",","~");
var illegal_wl_pass = new Array("\\","'","\"","%");
function checkAllNum(str){
    for (var i=0; i<str.length; i++){
        if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
            continue;
        return false;
    }
    return true;
}

function CheckValue() {
	var f = document.forms[0];
	if (f.elements["wlMode"][1].checked == true) {
		var sid = f.sta_ssid.value;
		var secMode = f.sta_security_mode.selectedIndex;
		//var re = /^[\w`~!@#$^*()\-+={}\[\]\|:'<>.?\/ ]+$/;
		
		if(!checkSSID(sid)) {
			alert("SSID无效，请输入1-32个ASCII码!");
			f.sta_ssid.focus();
			return false;
		}
		if(secMode == 1 || secMode == 2) { //	WEP 
			return check_Wep(secMode);
		}
		else if(secMode == 3 || secMode == 4 ||secMode == 5)//WPA
		{
			var keyvalue = document.wireless_mode.passphrase.value;
			if (keyvalue.length == 0){
				alert('请输入密钥!');
				return false;
			}
			if ((keyvalue.length < 8) || (keyvalue.length > 63)){
				alert('密钥范围: 8~63 个字符!');
				return false;
			}
			if(!ill_check(document.wireless_mode.passphrase.value,illegal_wl_pass,"")) return false;
			if(document.wireless_mode.cipher[0].checked != true && 
			   document.wireless_mode.cipher[1].checked != true &&
			   document.wireless_mode.cipher[2].checked != true){
			   alert('请选择一个WPA加密规则。');
			   return false;
			}	
		}
		
	}
	
	return true;
}

//?a???
function SurveyClose(){
	var tbl = document.getElementById("wdsScanTab").style,
		formUrl = "/goform/WDSScan";
	
	if (tbl.display == "") {
		tbl.display = "none";
		document.getElementById("wlSurveyBtn").value="开启扫描";
	} else {
		tbl.display = "";
		document.getElementById("wlSurveyBtn").value="关闭扫描";
		request.open("GET", formUrl, true);
		request.onreadystatechange = RequestRes;
		request.setRequestHeader("If-Modified-Since","0");
		request.send(null);			
	}	
}

function RequestRes() {
	if (request.readyState == 4) {
		initScan(request.responseText);
		
		//reset this iframe height by call parent frame function
		window.parent.reinitIframe();
	}
}

//initilize scan table
function initScan(scanInfo){	
	if(scanInfo != '') {
		var iserror=scanInfo.split("\n");
		
		if(iserror[0]!="stanley") {
			var str1 = scanInfo.split("\r");
			var len = str1.length;
			document.getElementById("wdsScanTab").style.display = "";
			document.getElementById("wlSurveyBtn").value="关闭扫描";
			
			var tbl = document.getElementById("wdsScanTab").getElementsByTagName('tbody')[0];
			//delete
			var maxcell = tbl.rows.length;
			for (var j = maxcell; j > 1; j --) {
				tbl.deleteRow(j - 1);
			}	
		
			var cont = parseInt(len);
			for (i = 0; i < cont; i ++){
				var str = str1[i].split("\t");
				nr=document.createElement('tr');
				nc=document.createElement('td');
				nr.appendChild(nc);

				nc.innerHTML = "<input type=\"radio\" name=\"wlsSlt\" value=\"radiobutton\" onclick=\"macAcc()\"/>";
				
				nc=document.createElement('td');
				nr.appendChild(nc);
				nc.innerHTML = str[0];
				
				nc=document.createElement('td');
				nr.appendChild(nc);
				nc.innerHTML = str[1];
			
				nc=document.createElement('td');
				nr.appendChild(nc);
				nc.innerHTML = str[2];
				
				nc=document.createElement('td');
				nr.appendChild(nc);
				nc.innerHTML = str[3];
				
				nc=document.createElement('td');
				nr.appendChild(nc);
				nc.innerHTML = str[4];
				
				nr.align = "center";
				tbl.appendChild(nr);
			}
		}
	} else {
		document.getElementById("wdsScanTab").style.display = "none";
		document.getElementById("wlSurveyBtn").value="开启扫描";
	}
	
}

//???MAC
function macAcc() {
	var f = document.forms[0];
	if(!confirm("您确定要连接到此AP吗?")) {
		return ;
	}
	var tbl = document.getElementById("wdsScanTab");
	var Slt = document.wireless_mode.wlsSlt;
	var mac,sc;
	var maxcell = tbl.rows.length;
	
	for (var r = maxcell; r > 1; r --) {
		if (maxcell == 2) {
			sc = Slt.checked;
		} else {
			sc = Slt[r - 2].checked;
		}	
		if (sc) {
			mac = tbl.rows[r - 1].cells[2].innerHTML;
			f.elements["sta_mac"].value = mac;
			f.elements["sta_ssid"].value = decodeSSID(tbl.rows[r - 1].cells[1].innerHTML);
			f.elements["sta_channel"].selectedIndex = tbl.rows[r - 1].cells[3].innerHTML;// - 1;
		}
	}
}

function onWlMode(s){
	var f = document.forms[0];
	if(s == "ap") {
		document.getElementById("wl_wan_note").style.display = "none";
		f.elements["sta_ssid"].disabled = true;
		f.elements["sta_mac"].disabled = true;
		f.elements["sta_channel"].disabled = true;
		f.elements["sta_security_mode"].disabled = true;
		f.elements["wlSurveyBtn"].disabled = true;
		document.getElementById("wdsScanTab").style.display = "none";
		document.wireless_mode.wl_mode.value = "ap";
	} else {
		document.getElementById("wl_wan_note").style.display = "";
		f.elements["sta_ssid"].disabled = false;
		f.elements["sta_mac"].disabled = false;
		f.elements["sta_channel"].disabled = false;
		f.elements["sta_security_mode"].disabled = false;
		f.elements["wlSurveyBtn"].disabled = false;
		if(s == "sta"){
			document.wireless_mode.wl_mode.value = "sta";
		}else if(s == "wet"){
			document.wireless_mode.wl_mode.value = "wet";
		}
	}
	onChangeSec();
}

function onChangeSec() {
	var f = document.forms[0];
	var idx = parseInt(f.elements["sta_security_mode"].selectedIndex,10);
	
	hideWep();
	
	if(((idx == 3) || (idx == 4) || (idx == 5)) && (f.wlMode[1].checked == true)) {
		document.getElementById("div_wpa").style.display = "";
		document.getElementById("div_wpa_algorithms").style.display = "";
		document.wireless_mode.cipher[0].disabled = false;
		document.wireless_mode.cipher[1].disabled = false;
		if(idx == 3){
			document.wireless_mode.cipher[2].checked = false;
			document.wireless_mode.cipher[2].disabled = true;
		}	
		if((idx == 4) || (idx == 5)) {
			document.wireless_mode.cipher[2].disabled = false;
		}

		document.getElementById("wpa_passphrase").style.display = "";
		document.wireless_mode.passphrase.disabled = false;
		
	}
	else if(((idx == 1) || (idx == 2))  && (f.wlMode[1].checked == true)) {
		document.getElementById("div_wpa").style.display = "none"
		showWep();
	}
	else {
		document.getElementById("div_wpa").style.display = "none"
	}
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function hideWep(){
	document.getElementById("div_wep").style.display = "none";
}

function showWep(){
	<!-- WEP -->
	document.getElementById("div_wep").style.display = "";
}


function check_Wep(securitymode){
	var key_input,
		defaultid = document.wireless_mode.wep_default_key.value,
		keyvalue = document.wireless_mode.wep_key_1.value;

	if (defaultid == 2) {
		keyvalue = document.wireless_mode.wep_key_2.value;
	} else if (defaultid == 3) {
		keyvalue = document.wireless_mode.wep_key_3.value;
	} else if (defaultid == 4) {
		keyvalue = document.wireless_mode.wep_key_4.value;
	}	
		
	if(!ill_check(document.wireless_mode.wep_key_1.value,illegal_wl_pass,"密码")) {
		return false;
	}
	if(!ill_check(document.wireless_mode.wep_key_2.value,illegal_wl_pass,"密码")) {
		return false;
	}
	if(!ill_check(document.wireless_mode.wep_key_3.value,illegal_wl_pass,"密码")){
		return false;
	}
	if(!ill_check(document.wireless_mode.wep_key_4.value,illegal_wl_pass,"密码")){ 
		return false;
	}

	if (keyvalue.length == 0 &&  (securitymode == 1 || securitymode == 2)){ // shared wep  || md5
		alert('请输入WEP密钥'+defaultid+' !');
		return false;
	}

	var keylength = document.wireless_mode.wep_key_1.value.length;
	if (keylength != 0){
		if (document.wireless_mode.WEP1Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('WEP密钥1无效，请输入5或13个ASCII码！');
				return false;
			}
			if(checkInjection(document.wireless_mode.wep_key_1.value)== false){
				alert('WEP密钥1包含无效字符。');
				return false;
			}
		}
		if (document.wireless_mode.WEP1Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('WEP密钥1无效，请输入10或26个Hex码！');
				return false;
			}
			if(checkHex(document.wireless_mode.wep_key_1.value) == false){
				alert('WEP密钥1包含无效字符。');
				return false;
			}
		}
	}

	keylength = document.wireless_mode.wep_key_2.value.length;
	if (keylength != 0){
		if (document.wireless_mode.WEP2Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('WEP密钥2无效，请输入5或13个ASCII码!');
				return false;
			}
			if(checkInjection(document.wireless_mode.wep_key_1.value)== false){
				alert('WEP密钥2包含无效字符。');
				return false;
			}			
		}
		if (document.wireless_mode.WEP2Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('WEP密钥2无效，请输入10或26个Hex码！');
				return false;
			}
			if(checkHex(document.wireless_mode.wep_key_2.value) == false){
				alert('WEP密钥2包含无效字符。');
				return false;
			}
		}
	}

	keylength = document.wireless_mode.wep_key_3.value.length;
	if (keylength != 0){
		if (document.wireless_mode.WEP3Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('WEP密钥3无效，请输入5或13个ASCII码!');
				return false;
			}
			if(checkInjection(document.wireless_mode.wep_key_1.value)== false){
				alert('WEP密钥3包含无效字符。');
				return false;
			}
		}
		if (document.wireless_mode.WEP3Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('WEP密钥3无效，请输入10或26个Hex码！');
				return false;
			}
			if(checkHex(document.wireless_mode.wep_key_3.value) == false){
				alert('WEP密钥3包含无效字符。');
				return false;
			}			
		}
	}

	keylength = document.wireless_mode.wep_key_4.value.length;
	if (keylength != 0){
		if (document.wireless_mode.WEP4Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('WEP密钥4无效，请输入5或13个ASCII码!');
				return false;
			}
			if(checkInjection(document.wireless_mode.wep_key_1.value)== false){
				alert('WEP密钥4包含无效字符。');
				return false;
			}			
		}
		if (document.wireless_mode.WEP4Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('WEP密钥4无效，请输入10或26个Hex码！');
				return false;
			}

			if(checkHex(document.wireless_mode.wep_key_4.value) == false){
				alert('WEP密钥4包含无效字符。');
				return false;
			}			
		}
	}
	return true;
}                                                                                                                                                                                                                                

function preSubmit(f){
	if(enablewireless == 0 && f.elements["wlMode"][1].checked){
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href="wireless_basic.html";
		return false;
	}
    if (CheckValue()) {
	    f.submit();
		if(0) {
			window.location.href = "wireless_detection.asp";
		}
	}	
}

</script>
</head>

<BODY onLoad="init();" >
	<form method=post name=wireless_mode action="/goform/wirelessMode" >
		<input type="hidden" id="wl_mode" name="wl_mode" />
		<fieldset>
			<h2 class="legend">WAN口介质类型</h2>
			<div class="control-group">
				<label class="control-label">WAN介质类型</label>
				<div class="controls">
					<label class="radio"><input type="radio" name="wlMode" checked="checked" value="0" onClick="onWlMode('ap')" />有线WAN</label>
					<label class="radio"><input type="radio" name="wlMode" value="1" onClick="onWlMode('sta')" />无线WAN</label>
				</div>
			</div>
			<div id="wl_wan_note" style="display:none;">
			<p class="text-red" style=" padding-top:10px; padding-bottom:5px;">请输入上级无线设备的相关参数</p>
			<div  class="control-group">
				<label class="control-label">无线信号名称</label>
				<div class="controls"><input type="text" class="text" name="sta_ssid" /></div>
			</div>
			<div  class="control-group" style="display:none">
				<label class="control-label">MAC</label>
				<div class="controls"><input type="text" class="text" name="sta_mac" size="17" maxlength="17" /></div>
			</div>
			<div  class="control-group">
				<label class="control-label">信道</label>
				<div class="controls">
					<select name="sta_channel">
					<script>
						document.write('<option value=' + 0 + '>' +"自动选择"	+ '</option>');
						for(var i=1;i<12; i++) {
							document.write('<option value=' + i + '>' + i + '</option>');
						}
					</script>
					</select>
				</div>
			</div>
			<div  class="control-group">
			    <label class="control-label">安全模式</label>
			    <div class="controls">
					<select name="sta_security_mode" onChange="onChangeSec()">
						<option value="Disable">禁止</option>
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
			<td class="control-label">默认密码</td>
			<td class="controls">
				<select name="wep_default_key" id="wep_default_key" size="1">
					<option value="1" selected="selected">密码 1</option>
					<option value="2">密码 2</option>
					<option value="3">密码 3</option>
					<option value="4">密码 4</option>
				</select>
			</td>
		  </tr>
		  
		  <tr> 
			<td class="control-label">WEP密码1</td>
			<td class="controls" id="td_wep1">
				<input name="wep_key_1" id="WEP1" class="text input-medium" type="password"  maxlength="26">
				<select class="input-small" id="WEP1Select" name="WEP1Select" > 
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select>
			</td>
		  </tr>

		  <tr> 
			<td class="control-label">WEP密码2</td>
			<td class="controls" id="td_wep2">
				<input name="wep_key_2" id="WEP2" class="text input-medium" type="password"  maxlength="26" >
				<select class="input-small" id="WEP2Select" name="WEP2Select">
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select></td>
		  </tr>
		  <tr> 
			<td class="control-label">WEP密码3</td>
			<td class="controls" id="td_wep3">
				<input name="wep_key_3" id="WEP3" type="password" class="text input-medium" maxlength="26">
				<select class="input-small" id="WEP3Select" name="WEP3Select">
					<option value="1">ASCII</option>
					<option value="0">Hex</option>
				</select></td>
		  </tr>
		  <tr> 
			<td class="control-label">WEP密码4</td>
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
				<label class="control-label">WPA/WPA2加密规则</label>
				<div class="controls">
					<label class="radio"><input name="cipher" id="cipher" value="aes" type="radio" />AES</label>
					<label class="radio"><input name="cipher" id="cipher" value="tkip" type="radio" checked="checked" />TKIP</label>
					<label class="radio"><input name="cipher" id="cipher" value="tkip+aes" type="radio" />TKIP&amp;AES</label>
				</div>
			</div>
			<div id="wpa_passphrase" name="wpa_passphrase"  class="control-group" style="visibility: visible;">
				<label class="control-label" for="passphrase">密码</label>
				<div class="controls" id="td_wlpwd">
					<input name="passphrase" id="passphrase" class="text" type="password" maxlength="63" />   
				</div>
			</div>  				
		  </fieldset>
		<!--//////////////////////END WPA//////////////////////////////-->
		
		<div style="width:100%;margin:10px auto; text-align:center">
			<input name="wlSurveyBtn" id="wlSurveyBtn" type="button" class="btn btn-small" onClick="SurveyClose()" value="关闭扫描" />
		</div>
		<table id="wdsScanTab" class="table" style="display:none">
			<thead>
				<tr>
				  <th width="10%"><div align="center">选择</div></td>
				  <th width="20%"><div align="center">SSID</div></td>
				  <th width="30%"><p align="center">MAC地址</p></td>
				  <th width="10%"><div align="center">信道</div></td>
				  <th width="15%"><div align="center">安全</div></td>
				  <th width="15%"><div align="center">信号强度</div></td>
				</tr>
			</thead>
			<tbody>
			</tbody>
		</table>
		</div>
		<SCRIPT>tbl_tail_save("document.wireless_mode");</SCRIPT>
	</form>
</body>
</html> 