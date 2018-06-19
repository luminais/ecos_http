<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>System | Backup/Restore</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">

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

</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
		<input type=hidden name=GO value=system_backup.asp>
          <table class=space width=100%>
            <tr>
              <td><div align="center" id="waitfor"> 
			      <table cellpadding="0" cellspacing="0" class="content2">
			        <tr>
                      <td colspan="2" valign="top">&nbsp;&nbsp;您可以备份/恢复路由器的当前设置</td>
                    </tr>
				  </table>
                  <table cellpadding="0" cellspacing="0" class="content3" id="table1">
                   
                    <tr>
                      <td height="30">&nbsp;&nbsp;需选择您要保存配置参数的文件目录:
                        <input type="button" class=button2 value="备份" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="DownLoadCfg()"/></td>
                    </tr> 
					<form name=frmSetup method="POST" action="/cgi-bin/UploadCfg" enctype="multipart/form-data">
                    <tr>
                      <td>&nbsp;&nbsp;选择您想要导入的配置文件:<br>         
					  &nbsp;&nbsp;<input type="file" name="fileCfg" id="fileCfg"/>&nbsp;&nbsp;
					  <input class=button2 type="button" value="恢复" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="UpLoadCfg()"/>
					</td></tr></form>
                  </table>
              </div></td>
            </tr>
            <tr>
              <td></td>
            </tr>
          </table>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('单击“备份”便可以将当前配置以文件的形式备份到相应的目录，生成一个系统配置的备份文件。\
		同理，您只需要点击“浏览”，选取相应目录中的配置文件，点击“恢复”，完成后重新启动路由将可以恢复到所需要的系统配置。 '
		);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>





