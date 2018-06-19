<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312" />
<link rel=stylesheet /css href=style.css />
<title>Wireless | Mode Setting</title>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript" type="text/javascript">
//'ssid1,11:22:33:44:55:67,6,WEP,60;ssid2,11:22:33:44:55:68,6,NONE,68';
var mode ;//0;AP 1:STATION
var ssid;
var resmac;
var channel;//1~13
var sec;//Disable,0,1,psk,psk2,psk psk2
var wpaAlg;
var wpakeyVa;
var RekeyInterval;
//var wanchangemode; //huangxiaoli modify

var DefaultKeyID ;
var Key1Type ;
var Key1Str ;
var Key2Type ;
var Key2Str ;
var Key3Type;
var Key3Str ;
var Key4Type;
var Key4Str ;
var request = GetReqObj();
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>";

var ssid000 = "<% get_wireless_basic("SSID"); %>";
var wl0_mode;

function init(){
	
	makeRequest("/goform/wirelessInitMode", "something");
}
function makeRequest(url, content) {
	http_request = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) { // IE
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request) {
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
	}
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}
function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
		 	var str=http_request.responseText.split("\r");
			//alert(str);
		  	mode = str[0] ;
		 	ssid =ssid000;//str[1];
			resmac = str[2];
			channel = str[3];
			sec = str[4];
			wpaAlg = str[5];
			wpakeyVa = str[6];
			RekeyInterval = str[7];
			//wanchangemode = str[8];	//huangxiaoli modify
			
			//DefaultKeyID = str[9] ;
//			Key1Type = str[10] ;
//			Key1Str = str[11];
//			Key2Type = str[12];
//			Key2Str = str[13];
//			Key3Type = str[14];
//			Key3Str = str[15];
//			Key4Type = str[16];
//			Key4Str = str[17];
			
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
		} else {
			//alert('There was a problem with the request.');
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
		}else
	        return false;
	}
    return true;
}


function checkInjection(str)
{
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == ':' || str.charAt(i) == ',' ||
			  str.charAt(i) == '\r' || str.charAt(i) == '\n'){
				return false;
		}else
	        continue;
	}
    return true;
}
function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}


function initValue()
{
	//huangxiaoli modify
	/*if(parseInt(wanchangemode) == 0)
	{
		document.wireless_mode.changemode[0].checked = true;
	}
	else
	{	
		document.wireless_mode.changemode[1].checked = true;
	}*/
	var security_mode;
	var f = document.forms[0];
	f.sta_security_mode.value = sec;

	f.sta_ssid.value = decodeSSID(ssid);
	f.sta_mac.value = resmac;
	f.sta_channel.value = channel;
	
	
	hideWep();
	
	

	document.getElementById("div_wpa").style.visibility = "hidden";
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.visibility = "hidden";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.visibility = "hidden";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.getElementById("wpa_key_renewal_interval").style.visibility = "hidden";
	document.getElementById("wpa_key_renewal_interval").style.display = "none";
	document.wireless_mode.cipher[0].disabled = true;
	document.wireless_mode.cipher[1].disabled = true;
	document.wireless_mode.cipher[2].disabled = true;
	document.wireless_mode.passphrase.disabled = true;
	document.wireless_mode.keyRenewalInterval.disabled = true;
	
	
	
	document.getElementById("WEP1").value = Key1Str;
	document.getElementById("WEP2").value = Key2Str;
	document.getElementById("WEP3").value = Key3Str;
	document.getElementById("WEP4").value = Key4Str;

	document.getElementById("WEP1Select").selectedIndex = (Key1Type == "0" ? 1 : 0);
	document.getElementById("WEP2Select").selectedIndex = (Key2Type == "0" ? 1 : 0);
	document.getElementById("WEP3Select").selectedIndex = (Key3Type == "0" ? 1 : 0);
	document.getElementById("WEP4Select").selectedIndex = (Key4Type == "0" ? 1 : 0);

	document.getElementById("wep_default_key").selectedIndex = parseInt(DefaultKeyID) - 1 ;
	
	
	if(wpaAlg == "tkip")
		document.wireless_mode.cipher[1].checked = true;
	else if(wpaAlg == "aes")
		document.wireless_mode.cipher[0].checked = true;
	else if(wpaAlg == "tkip+aes")
		document.wireless_mode.cipher[2].checked = true;
	if(sec == "psk")
	{
	 document.wireless_mode.cipher[2].disabled = true;
	}
	else if(sec == "psk2" || sec == "psk psk2")
	{
	 document.wireless_mode.cipher[2].disabled = false;
	}
	document.getElementById("passphrase").value = wpakeyVa;
	document.getElementById("keyRenewalInterval").value = RekeyInterval;
	security_mode = f.sta_security_mode.value;
	
	
	
	if (security_mode == "0" || security_mode == "1" ||security_mode == "WEPAUTO"){
		showWep(security_mode);
	}else if (security_mode == "psk" || security_mode == "psk2" || security_mode == "psk psk2"){
		<!-- WPA -->
		document.getElementById("div_wpa").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_wpa").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_wpa").style.display = "table";
		}
		document.getElementById("div_wpa_algorithms").style.visibility = "visible";
		document.getElementById("div_wpa_algorithms").style.display = style_display_on();
		document.wireless_mode.cipher[0].disabled = false;
		document.wireless_mode.cipher[1].disabled = false;
		if(security_mode == "psk" && document.wireless_mode.cipher[2].checked)
		{
			document.wireless_mode.cipher[2].checked = false;
			//document.wireless_mode.cipher[2].disabled = true;
		}	
		 if(security_mode == "psk2" || security_mode == "psk psk2")
			document.wireless_mode.cipher[2].disabled = false;
		document.getElementById("wpa_passphrase").style.visibility = "visible";
		document.getElementById("wpa_passphrase").style.display = style_display_on();
		document.wireless_mode.passphrase.disabled = false;
		//document.getElementById("wpa_key_renewal_interval").style.visibility = "visible";
		//document.getElementById("wpa_key_renewal_interval").style.display = style_display_on();
		//document.wireless_mode.keyRenewalInterval.disabled = false;
	}
	
	document.getElementById("wdsScanTab").style.display = "none";
	document.getElementById("wlSurveyBtn").value="开启扫描";
		
	if(mode == "1" && wl0_mode == "sta")
	{
		//f.elements[mode].checked = true;
		document.wireless_mode.wlMode[1].checked = true;
		//document.wireless_mode.wl_mode.value = "sta";
		onWlMode('sta');
	}
	else
	{
		document.wireless_mode.wlMode[0].checked = true;
		//document.wireless_mode.wl_mode.value = "ap";
		onWlMode('ap');
		
	}
	if(wl0_mode == "wet")
	{
		document.wireless_mode.wlMode[1].disabled = true;
		document.wireless_mode.wlMode[0].checked = true;
		//document.wireless_mode.wl_mode.value = "ap";		//此参数用来提交到后台判断是否要重启
		onWlMode('ap');
	}
	
}
var illegal_user_pass = new Array("\\r","\\n","\\","&","%","!", "#","$","^","*","(",")","-","+","=","?",";",":","'","\"","<",">",",","~");
function checkAllNum(str){
    for (var i=0; i<str.length; i++){
        if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
            continue;
        return false;
    }
    return true;
}

function CheckValue()
{
	var f = document.forms[0];
	if(f.elements["wlMode"][1].checked == true)
	{
		//var resid = /^\w{0,}$/;
		var sid = f.sta_ssid.value;
		var secMode = f.sta_security_mode.selectedIndex;
		//var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
		//if (!re.test(document.wireless_mode.sta_mac.value)) {
		//	alert("请输入正确的MAC地址！");
		//	return false;
		//}

		//var re =/^[\w`~!@#$^*()\-+={}\[\]\|:'<>.?/]+$/;	
		if(!checkSSID(sid))
		{
			alert("SSID无效，请输入1-32个ASCII码!");
			f.sta_ssid.focus();
			return false;
		}
		if(secMode == 1 || secMode == 2)//	WEP
		{
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
			if(!ill_check(document.wireless_mode.passphrase.value,illegal_user_pass,"密钥")) return false;
			if(document.wireless_mode.cipher[0].checked != true && 
			   document.wireless_mode.cipher[1].checked != true &&
			   document.wireless_mode.cipher[2].checked != true){
			   alert('请选择一个WPA加密规则。');
			   return false;
			}
	
			if(checkAllNum(document.wireless_mode.keyRenewalInterval.value) == false){
				alert('密钥更新周期无效。');
				return false;
			}
			if(document.wireless_mode.keyRenewalInterval.value < 60){
				alert('警告：密钥更新周期太短,范围：60~9999');
				return false;
			}

		}
		
	}
	
	return true;
}

//?a??é¨?è
function SurveyClose()
{
	var tbl = document.getElementById("wdsScanTab").style;
	
	if (tbl.display == "")
	{
		tbl.display = "none";
		document.getElementById("wlSurveyBtn").value="开启扫描";
	}else
	{
		tbl.display = "";
		document.getElementById("wlSurveyBtn").value="关闭扫描";
	   
	   //òì2?′?ê?
	    var code = "/goform/WDSScan";
		request.open("GET", code, true);
		request.onreadystatechange = RequestRes;
		request.setRequestHeader("If-Modified-Since","0");
		request.send(null);			
	}	
}

function RequestRes()
{
	if (request.readyState == 4)
	{
		initScan(request.responseText);
	}
}

//initilize scan table
function initScan(scanInfo)
{	
	if(scanInfo != '')
	{
	 var iserror=scanInfo.split("\n");
	 if(iserror[0]!="stanley")
	 {
		var str1 = scanInfo.split("\r");
		var len = str1.length;
		document.getElementById("wdsScanTab").style.display = "";
		document.getElementById("wlSurveyBtn").value="关闭扫描";
		
		var tbl = document.getElementById("wdsScanTab").getElementsByTagName('tbody')[0];
		//delete
		var maxcell = tbl.rows.length;
		for (var j = maxcell; j > 1; j --)
		{
			tbl.deleteRow(j - 1);
		}	
	
		var cont = parseInt(len);
		for (i = 0; i < cont; i ++)
		{
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
	}
	else
	{
		document.getElementById("wdsScanTab").style.display = "none";
		document.getElementById("wlSurveyBtn").value="开启扫描";
	}
}

//×??ˉìí?óMAC
function macAcc()
{
	var f = document.forms[0];
	if(!confirm("您确定要连接到此AP吗?"))
	{
		return ;
	}
	var tbl = document.getElementById("wdsScanTab");
	var Slt = document.wireless_mode.wlsSlt;
	var mac,sc;
	var maxcell = tbl.rows.length;
	
	for (var r = maxcell; r > 1; r --)
	{
		if (maxcell == 2)
			sc = Slt.checked;
		else
			sc = Slt[r - 2].checked;
			
		if (sc)
		{
			mac = tbl.rows[r - 1].cells[2].innerHTML;
			f.elements["sta_mac"].value = mac;
			f.elements["sta_ssid"].value = decodeSSID(tbl.rows[r - 1].cells[1].innerHTML);
			f.elements["sta_channel"].selectedIndex = tbl.rows[r - 1].cells[3].innerHTML;// - 1;
		}
	}
}

function onWlMode(s)
{
	var f = document.forms[0];
	if(s == "ap")
	{
		f.elements["sta_ssid"].disabled = true;
		f.elements["sta_mac"].disabled = true;
		f.elements["sta_channel"].disabled = true;
		f.elements["sta_security_mode"].disabled = true;
		f.elements["wlSurveyBtn"].disabled = true;
		document.getElementById("wdsScanTab").style.display = "none";
		document.wireless_mode.wl_mode.value = "ap";
	}
	else
	{
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

function onChangeSec()
{
	hideWep();
	
	var f = document.forms[0];
	var idx = parseInt(f.elements["sta_security_mode"].selectedIndex,10);
	
	if(((idx == 3) || (idx == 4) || (idx == 5)) && (f.wlMode[1].checked == true))
	{

		//IsShow("div_wpa",true);
		document.getElementById("div_wpa").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_wpa").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_wpa").style.display = "table";
		}
		document.getElementById("div_wpa_algorithms").style.visibility = "visible";
		document.getElementById("div_wpa_algorithms").style.display = style_display_on();
		document.wireless_mode.cipher[0].disabled = false;
		document.wireless_mode.cipher[1].disabled = false;
		if(idx == 3 )
		{
			document.wireless_mode.cipher[2].checked = false;
			document.wireless_mode.cipher[2].disabled = true;
		}	
		 if((idx == 4) || (idx == 5))
			document.wireless_mode.cipher[2].disabled = false;
		document.getElementById("wpa_passphrase").style.visibility = "visible";
		document.getElementById("wpa_passphrase").style.display = style_display_on();
		document.wireless_mode.passphrase.disabled = false;
		//document.getElementById("wpa_key_renewal_interval").style.visibility = "visible";
		//document.getElementById("wpa_key_renewal_interval").style.display = style_display_on();
		//document.wireless_mode.keyRenewalInterval.disabled = false;
		
	}
	else if(((idx == 1) || (idx == 2))  && (f.wlMode[1].checked == true))
	{
		IsShow("div_wpa",false);
		showWep();
	}
	else
	{
		IsShow("div_wpa",false);
	}
/*	
	if(idx == 3)
	{
	 document.wireless_mode.cipher[0].disabled = false;
	 document.wireless_mode.cipher[1].disabled = false; 	
	 document.wireless_mode.cipher[2].disabled = true;
	}
	else if((idx ==4) || (idx == 5))
	{
	 document.wireless_mode.cipher[0].disabled = false;
	 document.wireless_mode.cipher[1].disabled = false;  	
	 document.wireless_mode.cipher[2].disabled = false;
	}
	*/
	
	
	
	
	
}

function hideWep()
{
	document.getElementById("div_wep").style.visibility = "hidden";
	document.getElementById("div_wep").style.display = "none";
}

function showWep(mode)
{
	<!-- WEP -->
	document.getElementById("div_wep").style.visibility = "visible";

	if (window.ActiveXObject) { // IE 
		document.getElementById("div_wep").style.display = "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari...
		document.getElementById("div_wep").style.display = "table";
	}


}


function check_Wep(securitymode)
{
	var defaultid = document.wireless_mode.wep_default_key.value;
	var key_input;

	if ( defaultid == 1 )
		var keyvalue = document.wireless_mode.wep_key_1.value;
	else if (defaultid == 2)
		var keyvalue = document.wireless_mode.wep_key_2.value;
	else if (defaultid == 3)
		var keyvalue = document.wireless_mode.wep_key_3.value;
	else if (defaultid == 4)
		var keyvalue = document.wireless_mode.wep_key_4.value;
		
		
	if(!ill_check(document.wireless_mode.wep_key_1.value,illegal_user_pass,"密钥")) return false;
	if(!ill_check(document.wireless_mode.wep_key_2.value,illegal_user_pass,"密钥")) return false;
	if(!ill_check(document.wireless_mode.wep_key_3.value,illegal_user_pass,"密钥")) return false;
	if(!ill_check(document.wireless_mode.wep_key_4.value,illegal_user_pass,"密钥")) return false;

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
				alert('WEP密钥3无效，请输入5或13个ASCII码！');
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
				alert('WEP密钥4无效，请输入5或13个ASCII码！');
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
}                                                                                                                                  function setChange(c){
	changed = c;
}                                                                                               


function preSubmit(f)
{
	if(enablewireless == 0 && f.elements["wlMode"][1].checked){
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href="wireless_basic.asp";
		return false;
	}
    	if (true == CheckValue())
	{
	    f.submit();
		//alert(mode);
		
	//	if((f.elements["wlMode"][1].checked && mode == 1) || ( !f.elements["wlMode"][1].checked && mode == 0))
		if(0)
		{
			//alert("******");
			window.location.href = "wireless_detection.asp";
		}
	}	
}

function onWPAAlgorithmsClick(type)
{
/*	if(type == 0)
	{
		alert("您选择的是“TKIP”加密规则，网络模式将被强制转换为“11b/g混合模式”");
		return;	
	}
	if(type == 1)
	{
		alert("您选择的是“AES”加密规则，网络模式将被强制转换为“11b/g/n混合模式”");
		return;
	}
	if(type == 2)
	{
		alert("您选择的是“TKIP&AES”加密规则，网络模式将被强制转换为“11b/g/n混合模式”");
		return;
	}*/
}
</script>
<link rel=stylesheet type=text/css href=style.css>
<style type="text/css">
<!--
.STYLE1 {font-size: 12px}
-->
</style>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init();" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top"><table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
                <tr>
                  <td align="center" valign="top">
				  <form method=post name=wireless_mode action="/goform/wirelessMode" >
				  <input type="hidden" id="wl_mode" name="wl_mode" />
				  <table cellpadding="0" cellspacing="0" class="content1" id="table1">
				  <!--<tr><td width="100" align="right">WAN介质切换模式:</td>
					<td>&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" name="changemode" checked="checked" value="0" >
					手动&nbsp;&nbsp;
					<input type="radio" name="changemode" value="1" >自动</td></tr>-->
					<tr><td width="100" align="right">WAN介质类型:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" name="wlMode" checked="checked" value="0" onClick="onWlMode('ap')">
					有线WAN&nbsp;&nbsp;
					<input type="radio" name="wlMode" value="1" onClick="onWlMode('sta')">无线WAN</td></tr>
					<tr><td width="100" align="right">SSID:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input type="text" name="sta_ssid"></td></tr>
					<tr style="display:none"><td width="100" align="right">MAC:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input type="text" name="sta_mac" size="17" maxlength="17"></td></tr>
					<tr><td width="100" align="right">信道:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<select name="sta_channel">
						<script>
							document.write('<<option value=' + 0 + '>' +"自动选择"	+ '</option>');
							for(var i=1;i<12; i++)
								document.write('<<option value=' + i + '>' + i + '</option>');
						</script>
					</select></td></tr>
					<tr><td width="100" align="right">安全模式:</td>
					<td align="left">
							&nbsp;&nbsp;&nbsp;&nbsp;<select name="sta_security_mode" onChange="onChangeSec()">
								<option value="Disable">禁止</option>
								<option value="0">Open</option>
								<option value="1">Shared</option>
								<option value="psk">WPA-PSK</option>
								<option value="psk2">WPA2-PSK</option>
								<option value="psk psk2">Mixed WPA/WPA2 - PSK</option>
							</select>				
						</td></tr>
				  </table>
<!-- WEP -->
<!--<table id="div_wep" name="div_wep" width="90%" style="visibility:hidden; display:none">-->
<table cellpadding="0" cellspacing="0" class="content1" id="div_wep" style="margin-top:0px;" name="div_wep">
  <tbody>
  <tr> 
    <td colspan="1" align="right" class="head STYLE3"> 默认密钥&nbsp;&nbsp;</td>
	<td colspan="2" align="left">&nbsp;&nbsp;&nbsp;&nbsp;<select name="wep_default_key" id="wep_default_key" size="1" onChange="setChange(1)">
	<option value="1" selected="selected">密钥 1</option>
	<option value="2">密钥 2</option>
	<option value="3">密钥 3</option>
	<option value="4">密钥 4</option>
      </select>    </td>
  </tr>
  
  <tr> 
    <!--<td width="1%" rowspan="4" align="right" class="head1 STYLE3"></td>-->
    <td width="100" align="right" class="head2 STYLE3">WEP密钥1 : </td>
    <td width="200" align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_1" id="WEP1" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td width="196" align="left"><select id="WEP1Select" name="WEP1Select" onChange="setChange(1)"> 
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>

  <tr> 
    <td align="right" class="head2 STYLE3">WEP密钥2 : </td>
    <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_2" id="WEP2" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td align="left"><select id="WEP2Select" name="WEP2Select" onChange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
  <tr> 
    <td align="right" class="head2 STYLE3">WEP密钥3 : </td>
    <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_3" id="WEP3" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td align="left"><select id="WEP3Select" name="WEP3Select" onChange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
  <tr> 
    <td align="right" class="head2 STYLE3">WEP密钥4 : </td>
    <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_4" id="WEP4" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td align="left"><select id="WEP4Select" name="WEP4Select" onChange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
</tbody></table>

				<!--//////////////////////WPA//////////////////////////////////-->
				<table class="content3" cellpadding="0" cellspacing="0" id="div_wpa"  style="visibility:hidden; display:none">
				  <tr  id="div_wpa_algorithms" name="div_wpa_algorithms" style="visibility: visible;"> 
					<td width="100" align="right">WPA/WPA2 算法:</td>
					<td align="left">
					  &nbsp;&nbsp;&nbsp;&nbsp;
					  <input name="cipher" id="cipher" value="aes" type="radio" onClick="onWPAAlgorithmsClick(1)">AES &nbsp;
					  <input name="cipher" id="cipher" value="tkip" type="radio" checked="checked" onClick="onWPAAlgorithmsClick(0)">TKIP &nbsp;
					  <input name="cipher" id="cipher" value="tkip+aes" type="radio" onClick="onWPAAlgorithmsClick(2)">TKIP&AES &nbsp;
					  </td>
				  </tr>
				  <tr id="wpa_passphrase" name="wpa_passphrase" style="visibility: visible;">
					<td width="100" align="right">密钥:</td>
					<td align="left">
					  &nbsp;&nbsp;&nbsp;&nbsp;<input name="passphrase" id="passphrase" size="28" maxlength="64" value="">   
					 </td>
				  </tr>  				
				  <tr id="wpa_key_renewal_interval" name="wpa_key_renewal_interval" style="display:none">
					<td align="right" class="head">密钥更新周期</td>
					<td align="left">
					  &nbsp;&nbsp;&nbsp;&nbsp;<input name="keyRenewalInterval" id="keyRenewalInterval" size="4" maxlength="4" value="" onKeyUp="setChange(1)"> 秒
					</td>
				  </tr>
				  </table>
				<!--//////////////////////END WPA//////////////////////////////-->
				  <p>
					<input name="wlSurveyBtn" id="wlSurveyBtn" type="button" class="button" onClick="SurveyClose()" value="关闭扫描" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" />
				  <table width="442" id="wdsScanTab" class="content3" style="visibility: visible; display:none">
					<tr bgcolor="#CCCCCC">
					  <td width="10%"><div align="center">选择</div></td>
					  <td width="20%"><div align="center">SSID</div></td>
					  <td width="30%"><p align="center">MAC地址</p></td>
					  <td width="10%"><div align="center">信道</div></td>
					  <td width="15%"><div align="center">安全</div></td>
					  <td width="15%"><div align="center">信号强度</div></td>
					</tr>
				  </table>
						<SCRIPT>tbl_tail_save("document.wireless_mode");</SCRIPT>
				</form> 
				  </td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('有线WAN模式:路由器设备的接入方式由有线WAN口接入；无线WAN模式，接入方式为无线接入，需要设置ISP的SSID、信道以及安全等相关参数。另外，接入认证方式，请在快速设置中设置。'
		);</script>		
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
	  table_onload1('div_wpa');
	  table_onload1('div_wep');
    </script>
</BODY>
</HTML>

