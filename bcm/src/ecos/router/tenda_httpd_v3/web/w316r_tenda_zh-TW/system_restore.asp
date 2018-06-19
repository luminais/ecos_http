<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>System | Restore</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
function init()
{
}

function preSubmit(f) {
	if(window.confirm("路由器將自動重新啟動！IP位址將恢復為：192.168.0.1；使用者名稱和密碼恢復為：admin。如果網頁沒有重新整理，請將路由器關機30秒後再重新登錄！"))
	{
		f.submit() ;
   	} 
}

</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init();" class="bg">
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
				<form name=frmSetup method="POST" action=/goform/SysToolRestoreSet>
					<INPUT type=hidden name=CMD value=SYS_CONF>
					<INPUT type=hidden name=GO value=system_reboot.asp>
					<INPUT type=hidden name=CCMD value=0>
					<table cellpadding="0" cellspacing="0" class="content2">
				    <tr><td valign="top">&nbsp;&nbsp;點選此按鈕將使路由器的所有設定值，恢復到原廠預設值。</td>
				    </tr>
					</table>
					<table cellpadding="0" cellspacing="0" class="content3" id="table1">
						<tr><td height="30">&nbsp;&nbsp;<input class=button2 onClick="preSubmit(document.frmSetup);" value="恢復原廠預設值" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  type=button></td>
						</tr>
					</table>
				
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('恢復原廠預設值將使路由器的所有設定恢復到剛出廠時的預設狀態。其中：<br><br>\
			預設的密碼為：admin<br>\
			預設的IP 位址為：192.168.0.1<br>\
			預設的子網路遮罩為：255.255.255.0'
			);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


