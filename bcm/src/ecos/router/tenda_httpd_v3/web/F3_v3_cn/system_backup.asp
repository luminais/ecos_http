<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Configuration Tools</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body class="bg">
<form name=frmSetup method="POST" action="/cgi-bin/UploadCfg" enctype="multipart/form-data">
	<fieldset>
		<legend>Backup/Restore</legend>
		<table class="content1">
			<tr>
			  <td colspan="2" valign="top">backup note</td>
			</tr>
		</table>
		<table class="content2" id="table1">           
			<tr>
			  <td height="30">Click here to save a configuration file to your computer:
				<input type="button" class="btn" value="Backup" onClick="DownLoadCfg()"/></td>
			</tr>           
			<tr>
			  <td><br>         
			  <input class="text" type="file" name="fileCfg" id="fileCfg"/>&nbsp;&nbsp;
			  <input class="btn" type="button" value="Restore" onClick="UpLoadCfg()"/>
			</td>
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
<script type="text/javascript" src="js/gozila.js"></script>
<script>
function UpLoadCfg()
{
	var arr = document.frmSetup.fileCfg.value.toLowerCase().split(".");
	if (document.frmSetup.fileCfg.value == "")
	{
		alert(_("Please select the configuration file before clicking on Restore."));
		return ;
	}
	if(arr.length > 1){
		if(arr[arr.length-1] != "cfg")
		{
			alert(_("The config file selected must have a cfg suffix in the name!"));
			return ;
		}
	}else{
		alert(_("The config file selected must have a cfg suffix in the name!"));
		return ;
	}
	if(confirm(_("To activate loaded settings, you must reboot the router.")))
	{
		document.frmSetup.submit() ;
	}
}

function DownLoadCfg()
{
	if(confirm(_("Please select a path on your computer to save the configuration file.")))
	{
		refresh("/cgi-bin/DownloadCfg/RouterCfm.cfg");
	}
}
</script>
</body>
</html>