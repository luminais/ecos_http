<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Configuration Tools</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
function preSubmit(f) {
	if(window.confirm(_("restore_note"))){
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
		<legend>Restore To Factory Default</legend>
		<table class="content1" id="table1">
			<tr><td valign="top">Click this button to restore all settings to factory default.</td></tr>
			<tr><td><input class="btn" onClick="preSubmit(document.frmSetup);" value="Restore To Factory Default"  type=button></td>
			</tr>
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