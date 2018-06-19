<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Configuration Tools</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
function preSubmit(f) {
	if(window.confirm("设备将自动重启！IP 将更新为：192.168.0.1。如果页面没有刷新请更新您电脑的网络设置后重新登录！")){
		f.submit() ;
   	} 
}

</script>
</head>
<body class="bg">
<form name=frmSetup method="POST" action="/goform/SysToolRestoreSet">
    <input type=hidden name=CMD value=SYS_CONF>
    <input type=hidden name=GO value=system_reboot.asp>
    <input type=hidden name=CCMD value=0>
	<fieldset>
		<h2 class="legend">恢复出厂设置</h2>
		<table class="content1" id="table1">
			<tr><td valign="top">单击此按钮将使路由器的所有设置恢复到出厂时的默认状态。</td></tr>
			<tr><td><input class="btn" onClick="preSubmit(document.frmSetup);" value="恢复出厂设置"  type=button></td>
			</tr>
		</table>
	</fieldset>
</form>
</body>
</html>