<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>Firewall | MAC Control</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method=POST action="/goform/WlanMacFilter">
	<input type=hidden name=GO value="wireless_filter.asp">
	<!-- <input type="hidden" id="rebootTag" name="rebootTag"> -->
	<input type=hidden id="maclist" name="maclist">
	
	<fieldset>
		<legend>Access Control</legend>
		<div class="control-group">
			<label class="control-label">MAC Address Filter</label>	
			<div class="controls">
				<select size="1" name="FilterMode" id="FilterMode" onChange="onChangeRule()">
					<option value="disabled">Disable</option>
					<option value="allow">Permit</option>
					<option value="deny">Forbid</option>
				</select>
			</div>
	   </div>
		<table class="table" id="filterTab">
			<tr>
			  <td width="75%">MAC Address</td>
			  <td width="25%"></td>
			</tr>
			<tr align="center">
				<td height="30">
				<input class="text input-mic-mini" id="mac0" size="2" maxlength="2" onKeyUp="toNext(this,'mac1')" />:
				<input class="text input-mic-mini" id="mac1" size="2" maxlength="2" onKeyUp="toNext(this,'mac2')" />:
				<input class="text input-mic-mini" id="mac2" size="2" maxlength="2" onKeyUp="toNext(this,'mac3')" />:
				<input class="text input-mic-mini" id="mac3" size="2" maxlength="2" onKeyUp="toNext(this,'mac4')" />:
				<input class="text input-mic-mini" id="mac4" size="2" maxlength="2" onKeyUp="toNext(this,'mac5')" />:
				<input class="text input-mic-mini" id="mac5" size="2" maxlength="2" onKeyUp="toNext(this,'add')" /></td>
				<td align="left"><input name="button"  type="button" class="btn btn-small" id="add" onClick="addMac()" value="Add" /></td>
			</tr>
		</table>
		<!-- mac list -->
		<div id="list" style="position:relative;visibility:visible;"></div>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.frmSetup)" />
			<input type="button" class="btn last" value="Cancel" onClick="init(document.frmSetup)" />
	</div>      
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "wireless"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var filter_mode = "<%get_wireless_filter("macmode");%>",	
	res = "<%get_wireless_filter("maclist");%>",
	enablewireless = "<%get_wireless_basic("WirelessEnable");%>",
	flist = [];
	
if(res != ""){
	flist = res.split(" ");
}
function toNext(obj,aa){
	obj.value=obj.value.replace(/[^0-9a-fA-F]/g,'')
	if(obj.value.length == 2 && aa)
		document.getElementById(aa).focus();
}
function init(f){
	if(enablewireless==1){		
		if(filter_mode == "disabled")
			f.FilterMode.selectedIndex = 0;
		else if(filter_mode == "allow")
			f.FilterMode.selectedIndex = 1;
		else if(filter_mode == "deny")
			f.FilterMode.selectedIndex = 2;
				
		for(var i=0;i<6;i++) {
			document.getElementById("mac"+i).value="";
		}
		onChangeRule();
		showList();
	} else {
		alert(_("This feature can be used only if wireless is enabled."));
		top.mainFrame.location.href="wireless_basic.asp";
	}
}

function showList() {
	var s = '<table class="table" id="listTab">';
	for(var i=0;i<flist.length;i++) {
		s += '<tr><td width="75%">' + flist[i] + '</td><td width="25%"><input type="button" class="btn btn-small" value='+_("Delete")+' onClick="onDel(this,'+i+')"></td></tr>';
	}
	s += '</table>';
	document.getElementById("list").innerHTML = s;
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function preSubmit(f) {
	var macL = '';
	for(var i=0; i<flist.length; i++)
		macL += flist[i] + " ";
	if(flist.length == 0 && f.FilterMode.selectedIndex == 1) {
	 alert(_("You must set at least one rule when you choose Permit."));
	 return false;
	}
	document.getElementById("maclist").value = macL.replace(/\W$/,"");
	f.submit();
}

function onDel(obj,dex) {
	if(confirm(_("Click on OK to confirm that you want to delete this MAC address."))){
		document.getElementById("listTab").deleteRow(dex);
		var i=0;
		for(i=dex;i<flist.length;i++) {
			flist[i] = flist[i + 1];
		}
		flist.length--;
		showList();	
    }	
}

function addMac(){
    var add_mac,
		mac=new Array(),
		m = /[13579bdf]/i;
		
	for(var i=0;i<6;i++) {
		mac[i]=document.getElementById("mac"+i).value;
	}
	add_mac=mac[0]+":"+mac[1]+":"+mac[2]+":"+mac[3]+":"+mac[4]+":"+mac[5];
	if(add_mac == ":::::") {
		window.alert(_("Please enter a valid MAC address!"));
		return;
	}
	else{
	if(mac[0]=="" || mac[1]=="" || mac[3]==""||mac[4]==""||mac[5]=="") {
		window.alert(_("Please enter a valid MAC address!"));
		return;
	}
	for(var i=0;i<6;i++) {
		mac[i]=(mac[i].length==2)?mac[i]:("0"+mac[i]);
	}
	add_mac=mac[0]+":"+mac[1]+":"+mac[2]+":"+mac[3]+":"+mac[4]+":"+mac[5];
	if(add_mac.toLowerCase()=="ff:ff:ff:ff:ff:ff") {	
		window.alert(_("Please enter a valid MAC address!"));
		return;
	}
	if (add_mac=="00:00:00:00:00:00") {	
		window.alert(_("Please enter a valid MAC address!"));
		return;
	}
	if (m.test(add_mac.charAt(1))) {	
		window.alert(_("Please enter a unicast MAC address!"));
		return;
	}
	}
    if(flist.length > 15) {
        window.alert(_("Up to 16 MAC address entries are supported!"));
	 	return;
    }	
    for(var i=0; i<flist.length; i++) {
		if(flist[i].toLowerCase() == add_mac.toLowerCase()) {
			window.alert(_("Specified MAC address already exists!"));
			return;
		}
    }
	
   	flist[flist.length] = add_mac.toUpperCase();
	showList();
}
function onChangeRule() {
	if(document.getElementById("FilterMode").value == "disabled") {
		document.getElementById("filterTab").style.display = "none";
		document.getElementById("list").style.display = "none";
	} else {
		document.getElementById("filterTab").style.display = "";
		document.getElementById("list").style.display = "";
	}
}

</script>
</body>
</html>