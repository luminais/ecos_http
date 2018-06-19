//'ssid1,11:22:33:44:55:67,6,WEP,60;ssid2,11:22:33:44:55:68,6,NONE,68';
var request = GetReqObj(),
	illegal_user_pass = new Array("\\r", "\\n", "\\", "&", "%", "!", "#", "$",
			"^", "*","(",")","-","+","=","?",";",":","'","\"","<",">",",","~"),
	illegal_wl_pass = new Array("\\","'","\"","%"),
	http_request = false,
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
	wl0_mode;

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
	document.getElementById("wlSurveyBtn").value = _("Open Scan");
		
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

function checkAllNum(str){
    for (var i=0; i<str.length; i++){
        if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' )) {
            ;
		} else {
			return false;
		}
       
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
			//alert(_("SSID can not contain comma, semicolon, double quotation marks, ampersand, percent and backslash!"));
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
				alert(_("Please enter a key"));
				return false;
			}
			if ((keyvalue.length < 8) || (keyvalue.length > 63)){
				alert(_("Security key must contain 8~63 characters!"));
				return false;
			}
			if(!ill_check(document.wireless_mode.passphrase.value,illegal_wl_pass,"")) return false;
			if(document.wireless_mode.cipher[0].checked != true && 
			   document.wireless_mode.cipher[1].checked != true &&
			   document.wireless_mode.cipher[2].checked != true){
			   alert(_("Please select a WPA algorithm"));
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
		document.getElementById("wlSurveyBtn").value = _("Open Scan");
	} else {
		tbl.display = "";
		document.getElementById("wlSurveyBtn").value = _("Close Scan");
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
			document.getElementById("wlSurveyBtn").value = _("Close Scan");
			
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
		document.getElementById("wlSurveyBtn").value = _("Open Scan");
	}
	
}

//???MAC
function macAcc() {
	if(!confirm(_("Are you sure to connect to this AP?"))) {
		return ;
	}
	
	var f = document.forms[0],
		mac, sc,
		tbl = document.getElementById("wdsScanTab");
		Slt = document.wireless_mode.wlsSlt;
		maxcell = tbl.rows.length;
	
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
	var f = document.forms[0],
		idx = parseInt(f.elements["sta_security_mode"].selectedIndex,10);
	
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
		
	} else if(((idx == 1) || (idx == 2))  && (f.wlMode[1].checked == true)) {
		document.getElementById("div_wpa").style.display = "none"
		showWep();
	} else {
		document.getElementById("div_wpa").style.display = "none"
	}
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function hideWep(){
	document.getElementById("div_wep").style.display = "none";
}

function showWep(){
	document.getElementById("div_wep").style.display = "";
}


function check_Wep(securitymode){
	var key_input,
		defaultid = document.wireless_mode.wep_default_key.value,
		keyvalue = document.wireless_mode.wep_key_1.value,
		f = document.wireless_mode,
		form_ele = f.elements;

	if (defaultid == 2) {
		keyvalue = document.wireless_mode.wep_key_2.value;
	} else if (defaultid == 3) {
		keyvalue = document.wireless_mode.wep_key_3.value;
	} else if (defaultid == 4) {
		keyvalue = document.wireless_mode.wep_key_4.value;
	}	

	if (keyvalue.length == 0 &&  (securitymode == 1 || securitymode == 2)){ // shared wep  || md5
		alert(_("Please enter the WEP key %s!", [defaultid]));
		return false;
	}

	for(var i=1;i < 5;i++) {
		if(!ill_check(form_ele["wep_key_"+i].value,illegal_wl_pass,_("security key"))) return false;
		keylength = form_ele["wep_key_"+i].value.length;
		if (keylength != 0){
			if (form_ele["WEP"+i+"Select"].options.selectedIndex == 0){
				if(keylength != 5 && keylength != 13) {
					alert(__("WEP key %s is invalid! Please enter 5 or 13 ASCII characters.", [i]));
					return false;
				}
				if(checkInjection(document.wireless_mode.wep_key_1.value)== false){
					alert(_('Wep key %s contains invalid characters.', [i]));
					return false;
				}	
			}
			else{
				if(keylength != 10 && keylength != 26) {
					alert(__("WEP key %s is invalid! Please enter 10 or 26 Hex characters.", [i]));
					return false;
				}
				if(checkHex(form_ele["wep_key_"+i].value) == false){
					alert(__("Wep key %s contains invalid characters."),[i]);
					return false;
				}
			}
		}
	}
	
	return true;
}                                                                                                                                                                                                                                

function preSubmit(f){
	if(enablewireless == 0 && f.elements["wlMode"][1].checked){
		alert(_("This feature can be used only if wireless is enabled."));
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

function initEvent() {
	T.dom.inputPassword("WEP1", "");
	T.dom.inputPassword("WEP2", "");
	T.dom.inputPassword("WEP3", "");
	T.dom.inputPassword("WEP4", "");
	T.dom.inputPassword("passphrase", _("Enter WISP AP's security key"));
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

function makeRequest(url, content) {
	http_request = GetReqObj();
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}

function init(){
	makeRequest("/goform/wirelessInitMode", "something");
}

window.onload = init;