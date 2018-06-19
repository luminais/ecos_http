<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var enable_iptAccount="<%get_stream_stat_en("stream_stat_en");%>";
var accountUnit= "MByte";
var http_request = false;
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
		alert(_("Giving up :(Cannot create an XMLHTTP instance!)"));
		return false;
	}
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}
function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			    iptAccountUpdateHTML(http_request.responseText);
		}
	
	
	}
}

function iptAccountUpdateHTML(str){
	var e=document.getElementById("tbody1");
	while(e.rows.length>0){
		e.deleteRow(0);
	}
	if(str==""){
		return;
	}
	var data=str.split("\n");
	var items;
	for(k=0;k<data.length - 1;k++){
		items=data[k].split(";");
		var rownum=e.rows.length;
		var ishave=false;
		for(i=0;i<rownum;i++){
			if(e.rows[i].cells[0].innerHTML==items[0]){
				e.rows[i].cells[1].innerHTML=items[1];
				e.rows[i].cells[2].innerHTML=items[2];
				e.rows[i].cells[3].innerHTML=items[3];
				e.rows[i].cells[4].innerHTML=items[4];
				e.rows[i].cells[5].innerHTML=items[5];
				e.rows[i].cells[6].innerHTML=items[6];
				ishave=true;
			}
		}
		if(ishave==false){
			var newtr=document.createElement("tr");
			var newtd1=document.createElement("td");
			newtr.appendChild(newtd1);
			var newtd2=document.createElement("td");
			newtr.appendChild(newtd2);
			var newtd3=document.createElement("td");
			newtr.appendChild(newtd3);
			var newtd4=document.createElement("td");
			newtr.appendChild(newtd4);
			var newtd5=document.createElement("td");
			newtr.appendChild(newtd5);
			var newtd6=document.createElement("td");
			newtr.appendChild(newtd6);
			var newtd7=document.createElement("td");
			newtr.appendChild(newtd7);
			e.appendChild(newtr);
			newtr.style.height=25;
			newtd1.innerHTML=items[0];
			newtd2.innerHTML=items[1];
			newtd3.innerHTML=items[2];
			newtd4.innerHTML=items[3];
			newtd5.innerHTML=items[4];
			newtd6.innerHTML=items[5];
			newtd7.innerHTML=items[6];
		}
	}
}
function updateIptAccount(){
	makeRequest("/goform/updateIptAccount", "something");
	setTimeout("updateIptAccount()",1000);
}
function init(){
	var e=document.getElementById("enableiptAccount");
	if(enable_iptAccount==1){
		e.checked=true;
		updateIptAccount();
	}else{
		e.checked=false;
	}
	enable_Account();
}
function enable_Account(){
	var e=document.getElementById("enableiptAccount");
	if(e.checked){
		document.getElementById("div_iptAccountTable").style.display="block";
	}else{
		document.getElementById("div_iptAccountTable").style.display="none";
	}
}
function preSubmit(f){
   var fm = document.forms[0];
   if(fm.elements["enableiptAccount"].checked)
	  fm.elements["enableiptAccountEx"].value = "1";
   else
      fm.elements["enableiptAccountEx"].value = "0";
	f.submit();
}
</script>
</head>
<body onLoad="init();">
<form name="iptAccountform" method=post action="/goform/iptAcount_mng">
	<input type="hidden" name="enableiptAccountEx" />
	<fieldset>
		<legend>Traffic Statistics</legend>
		<div class="control-group">
			<div class="controls">
				<label for="enableiptAccount" class="checkbox">
				<input id="enableiptAccount" name="enableiptAccount" value=1 type="checkbox" onClick='enable_Account()' />
				Enable Traffic Statistics</label>
			</div>
		</div>
		<div id="div_iptAccountTable">
			<table class="table">
				<thead>
					<tr><td width="17%" align="center">IP Address</td>
					<td width="14%" align="center"><span>Uplink Rate</span>(KByte/s)</td>
					<td width="14%" align="center"><span>Downlink Rate</span>(KByte/s)</td>
					<td width="15%" align="center">Sent Message</td>
					<td width="12%" align="center">Sent Bytes
					  <script>document.write(accountUnit);</script></td>
					<td width="15%" align="center">Received Message</td>
					<td width="12%" align="center">Received Bytes
					  <script>document.write(accountUnit);</script></td>
					</tr>
				</thead>
				<tbody id="tbody1" align="right">
				</tbody>
			</table>
		</div>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.iptAccountform)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.iptAccountform)" />
	</div>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "qos_security"]);
	B.translate();
})();
</script>
</body>
</html>