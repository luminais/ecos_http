<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>System | Firmware Upgrade</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language=JavaScript>
var pc = 0,
	rebootTimer = 260,
	lanip = "<%aspTendaGetStatus("lan","lanip");%>",
	FirwareVerion="<%aspTendaGetStatus("sys","sysver");%>",//升级包版本
	FirwareDate="<%aspTendaGetStatus("sys","compimetime");%>";//升级日期
	
function init(f){
	f.reset();
}

function setWidth(windowObj, el, newwidth){
    if (document.all) {
		if (windowObj.document.all(el) ) {
			windowObj.document.all(el).style.width = newwidth ;
		}
	} else if (document.getElementById) {
		if (windowObj.document.getElementById(el) ) {
			windowObj.document.getElementById(el).style.width = newwidth;
		}
	}
}

function OnUpGrade(){
	pc+=1; 	
	if (pc > 100) {
		window.location = "direct_reboot.asp";
		clearTimeout(time); 
		return;
	}	
	//setWidth(self, "table_lpc", pc + "%");
	document.getElementById("table_lpc").style.width= pc + "%";
	document.getElementById("table_lpc_msg").innerHTML =_("Upgrading") + pc + "%";
	time = setTimeout("OnUpGrade()",rebootTimer);
}

function UpGrade(){        
	if (document.frmSetup.upgradeFile.value == "") {
		alert(_("Please select a firmware before you click on Upgrade!"));
		return ;
	}
	if(confirm(_("Please click on OK to confirm to upgrade your router!"))){
		document.getElementById("fwsubmit").disabled = true;
		document.frmSetup.submit() ;
		document.getElementById("table_lpc").style.display = "";
		OnUpGrade();
	} 	
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method="POST" action="/cgi-bin/upgrade" enctype="multipart/form-data">
	<fieldset>
		<legend>Upgrade</legend>
		<table class="content1">
			<tr><td colspan=2 valign="top">By upgrading the router’software, you’ll get new features.</td></tr>
		</table>
		<table class="content2" id="table1">
			<tr>
				<td nowrap>Select the firmware file<br />
					<input type="file" class="text" name="upgradeFile"/>
					<input id="fwsubmit" type="button" class="btn" value="Upgrade" onClick="UpGrade()"/>
				</td>
			</tr>
		</table>
		<table class="content2">
			<tr><td>
				<span>Current System Version</span>: <SCRIPT>document.write(FirwareVerion+';  <span>Publishing Date</span>:' + FirwareDate )</SCRIPT>
			</td></tr>
			<tr><td colspan=2>upgrade_note</td>
			</tr>
			<tr><td colspan="2" id="table_lpc_msg"  style="font-size:18px; color:#3a87ad"></td></tr>
			<tr><td colspan=2>
				<table id="table_lpc" style="display:none;background-color:#3a87ad; height:20px;">
					<tr><td></td></tr>
				</table> 
			</td></tr>
		</table>
	</fieldset>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "system_tool"]);
	B.translate();
})();
</script>
</body>
</html>