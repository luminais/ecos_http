<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>System | Backup/Restore</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">

function UpLoadCfg()
{
	if (document.frmSetup.fileCfg.value == "")
	{
		alert("首先請選擇欲上傳的檔案！");
		return ;
	}
	if(confirm("上傳後，需要重新啟動！"))
	{
		document.frmSetup.submit() ;
	}
}

function DownLoadCfg()
{
	if(confirm("下載設定參數，請指定儲存參數的路徑(資料夾)！"))
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
                      <td colspan="2" valign="top">&nbsp;&nbsp;您可以備份／恢復路由器目前的設定</td>
                    </tr>
				  </table>
                  <table cellpadding="0" cellspacing="0" class="content3" id="table1">
                   
                    <tr>
                      <td height="30">&nbsp;&nbsp;請選擇您要儲存設定參數的資料夾：
                        <input type="button" class=button2 value="備份" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="DownLoadCfg()"/></td>
                    </tr> 
					<form name=frmSetup method="POST" action="/cgi-bin/UploadCfg" enctype="multipart/form-data">
                    <tr>
                      <td>&nbsp;&nbsp;請選擇您要上傳的設定文件：<br>         
					  &nbsp;&nbsp;<input type="file" name="fileCfg" id="fileCfg"/>&nbsp;&nbsp;
					  <input class=button2 type="button" value="恢復" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="UpLoadCfg()"/>
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
		<script>
		helpInfo('點選「備份」便可將目前的設定以檔案的形式備份到對應的資料夾，將會自動存為系統設定的備份檔案。<br>同理，您只須點選<瀏覽>，並選擇對應資料夾中正確的設定檔案，點選<恢復>，等待重新啟動完成後，路由器就會恢復到系統備份時的完整設定。');
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

