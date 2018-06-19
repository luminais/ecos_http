
var mode, ssid, wl_mac, channel, sec, wpaAlg, wpakeyVa, RekeyInterval, DefaultKeyID,
	Key1Type, Key1Str, Key2Type, Key2Str, Key3Type, Key3Str, Key4Type, Key4Str, maxChannel,
	request = GetReqObj(),
	wl_enable, f,
	illegal_user_pass = new Array("\\r","\\n","\\","&","%","!", "#","$","^","*","(",")","-","+","=","?",";",":","'","\"","<",">",",","~"),
	illegal_wl_pass = ["\\","\"","%"],
	http_request=false;
	
function init() {
	http_request = GetReqObj();
	http_request.onreadystatechange = alertContents;
	http_request.open('GET', "/goform/wirelessInitMode", true);
	http_request.send(null);
}

function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
		 	var str=http_request.responseText.split("\r");
		  	mode = str[0] ;//ap,wds,apclient,wisp
		 	ssid =ssid000;//ssid=str[1];			
			channel = str[2];
			sec = str[3];
			wpaAlg = str[4];
			wpakeyVa = str[5];
			RekeyInterval = str[6];
			DefaultKeyID = str[7] ;
			Key1Type = str[8] ;
			Key1Str = str[9];
			Key2Type = str[10];
			Key2Str = str[11];
			Key3Type = str[12];
			Key3Str = str[13];
			Key4Type = str[14];
			Key4Str = str[15];
			wl_enable=str[16];//wl0_mode = str[17];
			wdsList = str[17];
			maxChannel = str[18];
			showChannel();
			initValue();
			initEvent();
		}
	}
}

function showChannel() {
	var idx, bstr;
	var AutoSelectx = _("AutoSelect");
	var Channelx = _("Channel");

	bstr = '<select id="channel" name="channel"><option value=0>' + AutoSelectx + '</option>';
	
	if(maxChannel == 11) {
		for (idx = 0; idx < 11; idx++) {
			if(channel == idx  + 1) {
				bstr+='<option value='+(idx+1)+' selected>'+(2412+5*idx)+'MHz (' + Channelx + ' '+(idx+1)+')</option>';
			} else {
				bstr+='<option value='+(idx+1)+'>'+(2412+5*idx)+'MHz (' + Channelx+ ' '+(idx+1)+')</option>';
			}
		}
	} else if(maxChannel == 13) {
		for (idx = 0; idx < 13; idx++) {
			if(channel == idx  + 1) {
				bstr+='<option value='+(idx+1)+' selected>'+(2412+5*idx)+ 'MHz (' + Channelx+ ' '+(idx+1)+')</option>';
			} else {
				bstr+='<option value='+(idx+1)+'>'+(2412+5*idx)+ 'MHz (' + Channelx+ ' '+(idx+1)+')</option>';
			}
		}
	} else if (maxChannel == 14) {
		for(idx = 0; idx < 13; idx++) {
			if(channel == idx + 1) {
				bstr+='<option value='+(idx+1)+' selected>'+(2412+5*idx)+ 'MHz (' + Channelx+ ' '+(idx+1)+')</option>';
			} else {
				bstr+='<option value='+(idx+1)+'>'+(2412+5*idx)+ 'MHz (' + Channelx+ ' '+(idx+1)+')</option>';
			}
		}
		if(channel == 14) {
			bstr=+'<option value=14 selected>2484MHz (' + Channelx + ' 14)</option>';
		} else {
			bstr=+'<option value=14>2484MHz (' + Channelx + ' 14)</option>';
		}
	}
	bstr+='</select>';
	document.getElementById("channelWrap").innerHTML = bstr;
}

function initValue() {
	f = document.forms[0];
	if(wl_enable==0) {
		document.getElementById("wireless_enable").style.display="none";
		f.extra_mode.disabled=true;
	}
	f.security.value = sec;	
	f.ssid.value = decodeSSID(ssid);
	//f.sta_mac.value = wl_mac;
	f.channel.value = channel;
	f.WEP1.value = Key1Str;
	f.WEP2.value = Key2Str;
	f.WEP3.value = Key3Str;
	f.WEP4.value = Key4Str;

	f.WEP1Select.selectedIndex = (Key1Type == "0" ? 1 : 0);
	f.WEP2Select.selectedIndex = (Key2Type == "0" ? 1 : 0);
	f.WEP3Select.selectedIndex = (Key3Type == "0" ? 1 : 0);
	f.WEP4Select.selectedIndex = (Key4Type == "0" ? 1 : 0);
	f.wep_default_key.selectedIndex = parseInt(DefaultKeyID[0]) - 1 ;
	if(wpaAlg == "tkip")
		f.cipher[1].checked = true;
	else if(wpaAlg == "aes")
		f.cipher[0].checked = true;
	else if(wpaAlg == "tkip+aes")
		f.cipher[2].checked = true;
	if(sec == "psk")
	 	f.cipher[2].disabled = true;
	else if(sec == "psk2" || sec == "psk psk2")
		f.cipher[2].disabled = false;
	f.passphrase.value = wpakeyVa;
	f.keyRenewalInterval.value = RekeyInterval;
	initWds();
	if(mode == "ap" && wdsList == "") {
		f.extra_mode.selectedIndex = 0;
		//document.getElementById("wireless_enable").style.display="none";
	} else if(mode == "wet") {
		f.extra_mode.selectedIndex = 1;
	} else if(mode == "sta") {
		f.extra_mode.selectedIndex = 2;
	}
	else//(wdsList!="")
		f.extra_mode.selectedIndex = 3;
	onWlMode();
}

function initEvent() {
	T.dom.inputPassword("WEP1", "");
	T.dom.inputPassword("WEP2", "");
	T.dom.inputPassword("WEP3", "");
	T.dom.inputPassword("WEP4", "");
	T.dom.inputPassword("passphrase", "");
}

function initWds()
{
	var wdslistArray;
	if (wdsList != "")
	{
		wdslistArray = wdsList.split(" ");
		for(i = 1; i <= wdslistArray.length; i++)
		eval("document.formSetup.wds_"+i).value = wdslistArray[i - 1];
	}
	document.getElementById("wdsScanTab").style.display = "none";
	document.getElementById("wlSurveyBtn").value="Open Scan";
}
function SurveyClose()
{
	var tbl = document.getElementById("wdsScanTab").style;
	if (tbl.display == "")
	{
		tbl.display = "none";
		document.getElementById("wlSurveyBtn").value="Open Scan";
	}
	else
	{
		tbl.display = "";
		document.getElementById("wlSurveyBtn").value="Close Scan";
	   
	    var code = "/goform/WDSScan";
		request.open("GET", code, true);
		request.onreadystatechange = RequestRes;
		request.setRequestHeader("If-Modified-Since","0");
		request.send(null);	
	}	
}

function RequestRes()
{
	if (request.readyState == 4){
		initScan(request.responseText);
		window.parent.reinitIframe();
	}
}
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
			document.getElementById("wlSurveyBtn").value="Close Scan";			
			var tbl = document.getElementById("wdsScanTab").getElementsByTagName('tbody')[0];	
			var maxcell = tbl.rows.length;
			for (var j = maxcell; j > 1; j --)
				tbl.deleteRow(j - 1);
			var cont = parseInt(len);
			for (i = 0; i < cont; i ++)
			{
				var str=str1[i].split("\t");
				nr=document.createElement('tr');
				nc=document.createElement('td');
				nr.appendChild(nc);	
				nc.innerHTML = '<input type="radio" name="wlsSlt" value="radiobutton" onclick="macAcc(event)"/>';				
				nc=document.createElement('td');
				nc.title = str[0];
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
		document.getElementById("wlSurveyBtn").value="Open Scan";
	}
}
function macAcc()
{
	f = document.forms[0];
	if(!confirm("Please click OK to confirm to connect to selected AP!")){
		return ;
	}
	var target = arguments[0].target||arguments[0].srcElement,
		tr = target.parentNode.parentNode,
		secu_type = tr.childNodes[4].innerHTML;
		 mac = tr.childNodes[2].innerHTML;
	if(f.elements["extra_mode"].selectedIndex==3)
	{
		if(checkRepeat(mac) == false) {
				return;
		}
		if(f.wds_1.value.length != 17)
			f.wds_1.value=mac;
		else if(f.wds_2.value.length != 17)
			f.wds_2.value=mac;
		else
		{
			alert("AP MAC enteries are full! Please remove the MAC enteries you  don't want.");
			return;
		}
	}
	f.elements["ssid"].value = decodeSSID(tr.childNodes[1].innerHTML);
	f.elements["channel"].selectedIndex = tr.childNodes[3].innerHTML;
	show_security(secu_type);
}

function show_security(secu_type) {
	var f = document.forms[0];
	
	if (secu_type == "NONE" || secu_type == "UNKNOW") {
		f.security.value = "Disable";
		onChangeSec();
	} else if (secu_type == "Open/Shared") {
		f.security.value = "1";
		onChangeSec();
		for (var i = 1; i < 5; i++) {
			f.elements["wep_key_" + i].value = "";
		}
	} else {
		var secuArr = [];
		secuArr = secu_type.split("_");
		if (secuArr[0] == "WPA") {
			f.security.value = "psk";
		} else if (secuArr[0] == "WPA2") {
			f.security.value = "psk2";
		} else {
			f.security.value = "psk psk2";
		}
		onChangeSec();
		if (secuArr[1] == "AES") {
			f.cipher[0].checked = true;
		} else if (secuArr[1] == "TKIP") {
			f.cipher[1].checked = true;
		} else if (secuArr[1] == "AESTKIP") {
			f.cipher[2].checked = true;
		}
		f.passphrase.value = "";
	}
	window.parent.reinitIframe();
}

function checkRepeat(mac){
	for(var i=1;i<=2;i++) {
		if(mac.toUpperCase() == document.forms[0].elements["wds_"+i].value.toUpperCase()) {
			alert(_("This address already exists"));
			return false;
		}
	}
}

function onWlMode()
{
	var index = document.forms[0].extra_mode.selectedIndex;	
	if(index==0)
		document.getElementById("wireless_enable").style.display="none";
	else{
		document.getElementById("wireless_enable").style.display="";
		onChangeSec();
		if(index==3)
		{
			document.getElementById("wds_mac1").style.display="";
			document.getElementById("wds_mac2").style.display="";
		}
		else
		{
			document.getElementById("wds_mac1").style.display="none";
			document.getElementById("wds_mac2").style.display="none";
		}
	}
	window.parent.reinitIframe();
}

function clear_data()
{
	f=document.forms[0];
	f.channel.selectedIndex=0;
	f.ssid.value="";
	f.security.selectedIndex=0;
	f.wds_1.value="";
	f.wds_2.value="";
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wep").style.display = "none";
}

function onChangeSec()
{
	f = document.forms[0];
	var idx = parseInt(f.elements["security"].selectedIndex,10);
	if(idx == 3 || idx == 4 || idx == 5)
	{
		document.getElementById("div_wpa").style.display = "";
		document.getElementById("div_wep").style.display = "none";
		if(idx == 3)
		{
			f.cipher[2].disabled = true;
			if(f.cipher[2].checked)
			{
				f.cipher[2].checked = false;
				f.cipher[0].checked = true;	
			}			
		}
		else f.cipher[2].disabled = false;
	}
	else if(idx == 1 || idx == 2)
	{
		document.getElementById("div_wep").style.display = "";
		document.getElementById("div_wpa").style.display = "none";
	}
	else
	{
		document.getElementById("div_wep").style.display = "none";
		document.getElementById("div_wpa").style.display = "none";
	}
	window.parent.reinitIframe();
		
}
function checkHex(str)
{
	var r=/^[0-9a-fA-F]+$/;
	if(r.test(str)) return true;
	else return false;
}

function checkInjection(str)
{
	var r=/^[^:,\r\n"'\\\/\s]+$/;
	if(!r.test(str))
		return false;
	else
		return true;
}
function check_Wep(securitymode)
{
	f = document.forms[0];
	var defaultid = f.wep_default_key.value;
	var keyvalue,keylength;
	var form_ele=f.elements;
	keyvalue=form_ele["wep_key_"+defaultid].value;	
	if (keyvalue.length == 0 &&  (securitymode == "1" || securitymode == "0")){
		alert('Please enter the WEP key '+defaultid+'!');
		return false;
	}
	for(var i=1;i<5;i++)
	{
		//if(!ill_check(form_ele["wep_key_"+i].value,illegal_user_pass,"key")) return false;
		keylength = form_ele["wep_key_"+i].value.length;
		if (keylength != 0){
			if (form_ele["WEP"+i+"Select"].options.selectedIndex == 0){
				if(keylength != 5 && keylength != 13) {
					alert('WEP key '+i +' is invalid! Please enter 5 or 13 ASCII characters.');
					return false;
				}
				if(!ill_check(form_ele["wep_key_"+i].value,illegal_wl_pass, _("security key"))) return false;
			}
			else{
				if(keylength != 10 && keylength != 26||checkHex(form_ele["wep_key_"+i].value) == false) {
					alert('WEP key '+i +' is invalid! Please enter 10 or 26 Hex characters.');
					return false;
				}
			}
		}
	}
	return true;
}
function CheckValue()
{
	f = document.forms[0];
	var sel=f.elements["extra_mode"].selectedIndex;
	if (sel ==1 || sel ==2)
	{
		var sid = f.ssid.value;
			secMode = f.security.selectedIndex,re=/[,;'"\\]/;
		//var re =/^[\n\r]+$/;
		if(sid == ""){
			alert("Please enter a valid SSID of uplink wireless access point!");
			f.ssid.focus();
			return false;
		}
		if (!checkSSID(sid)) {
			f.ssid.focus();
			return false;
		}
		if(secMode == 1 || secMode == 2)//	WEP
			return check_Wep(secMode);
		else if(secMode == 3 || secMode == 4 ||secMode == 5)//WPA
		{
			var keyvalue = f.passphrase.value;
			if (keyvalue.length == 0)
			{
				alert('Please enter a valid security key!');
				return false;
			}
			if ((keyvalue.length < 8) || (keyvalue.length > 64)){
				alert('Security key must contain 8~63 ASCII characters or 8~64 Hex characters!');
				return false;
			}
			if (keyvalue.length == 64) {
				if (checkHex(keyvalue) == false) {
					alert(_("Security key must contain 8~63 ASCII characters or 8~64 Hex characters!"));
					return false;	
				}
			}
			if(!ill_check(keyvalue,illegal_wl_pass, _("security key"))){
				return false;
			}
		}
	}
	else if (sel==3)
	{
		var re = /([A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}/,
			r=/[13579bdf]/i,
			all_wds_list='',
			wdsmac1=document.formSetup.wds_1.value,
			wdsmac2=document.formSetup.wds_2.value;
		if(wdsmac1!=""||wdsmac2!=""){
			if(r.test(wdsmac1.charAt(1))||r.test(wdsmac2.charAt(1)))
   		 	{
      		  	alert("Please enter a unicast MAC address!");
       		 	return false;
   			}
			if ((wdsmac1!=""&&!re.test(wdsmac1))||(wdsmac2!=""&&!re.test(wdsmac2))){
				alert("Please enter a valid MAC address!");
				return false;
			}
			if(wdsmac1==wdsmac2)
			{
				alert("Please enter a different MAC address!");
				return false;
			}
			if(wdsmac1!=""){
				all_wds_list=wdsmac1;
				if(wdsmac2!="")
					all_wds_list+=" "+wdsmac2;
			}
			else
				all_wds_list=wdsmac2;
			f.wds_list.value=all_wds_list;
		}
		else
		{
			alert("AP MAC address field should not be left empty!");
			f.wds_1.focus();
			return false;
		}
	}
	return true;
}

function preSubmit(f)
{
	if(wl_enable == 0){
		alert("This feature only can be supported when wireless is enabled.");
		top.mainFrame.location.href="wireless_basic.asp";
		return false;
	}
    if (true == CheckValue()){
		//if(f.channel.value != channel||f.ssid.value!=ssid)
		if(window.confirm("Please click OK to save the settings and the router will reboot automatically."))
		{
			f.submit() ;
		}
	}
}