<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language="JavaScript">
var lanip = "<%aspTendaGetStatus("lan","lanip");%>";
var time = 0;
var pc = 0;
function init(f){
	reboot();
}
function reboot()
{
	pc+=1;
	if (pc > 100) 
	{ 
		window.top.location = "http://" + lanip;
		clearTimeout(time); 
		return;
	} 
	setWidth(self, "lpc", pc + "%");
	document.getElementById("percent").innerHTML = "重启中..." + pc + "%"
	time = setTimeout("reboot()",125);
}
function setWidth(windowObj, el, newwidth) {
    if (document.all)
	{
	  if (windowObj.document.all(el) )
        windowObj.document.all(el).style.width = newwidth ;
	}
	else if (document.getElementById)
	{
	  if (windowObj.document.getElementById(el) )
	    windowObj.document.getElementById(el).style.width = newwidth;
	}
}
</script>
</head>
<body class="bg" onLoad="init(document.frmSetup);">
<form name=frmSetup method="POST">
<table class=content1>
	<tr><td>重启路由将使所改变的设置生效。<span class="text-red">在重启时，会自动断开所有连接。</span></td>
	</tr>
    <tr><td id="percent" style="font-size:20px; color:#3a87ad"></td></tr>        
    <tr><td align="left" width="400px" colspan="2">
        <div id="lpc" style="background-color:#3a87ad; height:20px; width:2px"></div>
    </td>
    </tr>
</table>
</form>
</body>
</html>