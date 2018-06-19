<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>System | Firmware Upgrade</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language=JavaScript>
var pc = 0,
	rebootTimer = 120,
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
		window.loaction = "driect_reboot.asp";
		clearTimeout(time); 
		return;
	}	
	setWidth(self, "table_lpc", pc + "%");
	document.getElementById("table_lpc_msg").innerHTML ="升级中..." + pc + "%";
	time = setTimeout("OnUpGrade()",rebootTimer);
}

function UpGrade(){        
	if (document.frmSetup.upgradeFile.value == "") {
		alert("请首先选择升级文件！");
		return ;
	}
	if(confirm('您确信要升级吗?')){
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
		<h2 class="legend">软件升级</h2>
		<table class="content1">
			<tr><td colspan=2 valign="top">通过升级本路由器的软件，您将获得新的功能。</td></tr>
		</table>
		<table class="content2" id="table1">
			<tr>
				<td nowrap>选择固件文件:<br />
					<input type="file" class="text" name="upgradeFile"/>
					<input id=fwsubmit type=button class="btn" value="升 级" onClick="UpGrade()"/>
				</td>
			</tr>
		</table>
		<table class="content2">
			<tr><td>当前系统版本:
				<script>document.write( FirwareVerion+'; 发布日期:'+FirwareDate )</script></td></tr>
			<tr><td colspan=2><span class="text-red">注意：升级过程不能关闭路由器电源，否则将导致路由器损坏而无法使用。</span>升级成功后，路由器将自动重启。升级过程约数分钟，请等候。</td>
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
</body>
</html>