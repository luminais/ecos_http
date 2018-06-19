<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Configuration Tools</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
function UpLoadCfg()
{
	if (document.frmSetup.fileCfg.value == "")
	{
		alert("请首先选择导入的文件！");
		return ;
	}
	if(confirm("导入后，需要重新启功！"))
	{
		document.frmSetup.submit() ;
	}
}

function DownLoadCfg()
{
	if(confirm("导出配置参数，请指定保存参数的路径！"))
	{
		refresh("/cgi-bin/DownloadCfg/RouterCfm.cfg");
	}
}
</script>
</head>
<body class="bg">
<form name=frmSetup method="POST" action="/cgi-bin/UploadCfg" enctype="multipart/form-data">
	<fieldset>
		<legend>备份/恢复设置</legend>
		<table class="content1">
			<tr>
			  <td colspan="2" valign="top">您可以备份/恢复路由器的当前设置</td>
			</tr>
		</table>
		<table class="content2" id="table1">           
			<tr>
			  <td height="30">需选择您要保存配置参数的文件目录:
				<input type="button" class="btn" value="备 份" onClick="DownLoadCfg()"/></td>
			</tr>           
			<tr>
			  <td>选择您想要导入的配置文件:<br>         
			  <input class="text" type="file" name="fileCfg" id="fileCfg"/>&nbsp;&nbsp;
			  <input class="btn" type="button" value="恢 复" onClick="UpLoadCfg()"/>
			</td>
			</tr>
		</table>
	</fieldset>
</form>
</body>
</html>