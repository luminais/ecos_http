<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Configuration Tools</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var lanip = "<%aspTendaGetStatus("lan","lanip");%>";
var time = 0;
var pc = 0;
function preSubmit() {
	if(window.confirm("大约12秒后系统会重新链接至首页。")) {	
		document.frmSetup.style.cursor = "wait";
		document.getElementById("rebootBtn").disabled = true;
	    var code = "/goform/SysToolReboot";
		var request = GetReqObj();
		request.open("GET", code, true);
		//request.onreadystatechange = RequestRes;
		request.setRequestHeader("If-Modified-Since","0");
		request.send(null);	
		reboot();
	}
	else
		return;
}
function reboot(){
	pc+=2;
	if (pc > 100) { 
		window.top.location = "http://" + lanip;
		document.frmSetup.style.cursor = "auto";
		clearTimeout(time); 
		return;
	} 
	document.getElementById("lpc").style.width= pc + "%";
	document.getElementById("percent").innerHTML = pc + "%";
	time = setTimeout("reboot()",200);
}
</script>
</head>
<body>		
<form name=frmSetup method="POST" action="/goform/SysToolReboot">
<input type=hidden name=CMD value=SYS_CONF>
<input type=hidden name=GO value=system_reboot.asp>
<input type=hidden name=CCMD value=0>
	<fieldset>
		<legend>重启路由器</legend>
		<table class="content1">
			<tr><td colspan=2 valign="top">单击此按钮将使路由器重新启动。</td>
			</tr>
		</table>
		<table id="table1">
			<tr>
				<td width="50%">
					<input class="btn" onClick="preSubmit()" value="重启路由器" type="button"  id="rebootBtn">
				</td>
				<td id="percent" style="font-size:24px"></td>
			</tr>        
			<tr><td colspan="2">
			<table class="span6" style="border:1px solid #ccc; margin:5px 0px;">
				<tr>
					<td> 
						<table id="lpc" style="background-color:#3a87ad" height=20>
							<tr><td>&nbsp;</td></tr>
						</table> 
					</td>		
				</tr>
			</table> 
			</td>
			</tr>
		</table>
	</fieldset>
</form>
</body>
</html>