<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>System | Backup/Restore</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">
Butterlate.setTextDomain("system_tool");
function initTranslate(){
	var e=document.getElementById("backup");
	e.value=_("Backup");
	var e1=document.getElementById("restore");
	e1.value=_("Restore");
}
function init()
{
 initTranslate();
}
function UpLoadCfg()
{
	if (document.frmSetup.fileCfg.value == "")
	{
		alert(_("Please select the file you want to import"));
		return ;
	}
	if(confirm(_("You need to restart after importing")))
	{
		document.frmSetup.submit() ;
	}
}
function DownLoadCfg()
{
	if(confirm(_("Export the configured parameters and specify a path to save them")))
	{
		refresh("/cgi-bin/DownloadCfg/RouterCfm.cfg");
	}
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" class="bg" onLoad="init();">
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
                      <td colspan="2" valign="top">&nbsp;&nbsp;<script>document.write(_("backup restore current configuration"));</script></td>
                    </tr>
				  </table>
                  <table cellpadding="0" cellspacing="0" class="content3" id="table1"> 
                    <tr>
                      <td height="30">&nbsp;&nbsp;<script>document.write(_("Select the file directory to save the configured parameters"));</script>
                        <input type="button" id="backup" class=button2 value="" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="DownLoadCfg()"/></td>
                    </tr> 
					<form name=frmSetup method="POST" action="/cgi-bin/UploadCfg" enctype="multipart/form-data">
                    <tr>
                      <td>&nbsp;&nbsp;<script>document.write(_("Select the configured file you want to import"));</script><br>         
					  &nbsp;&nbsp;<input type="file" name="fileCfg" id="fileCfg"/>&nbsp;&nbsp;
					  <input id="restore" class=button2 type="button" value="" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="UpLoadCfg()"/>
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
		<script>helpInfo(_("system_backup_Help_Inf"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


