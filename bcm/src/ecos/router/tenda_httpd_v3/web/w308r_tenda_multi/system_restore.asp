<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>System | Restore</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("system_tool");
function initTranslate(){
	var e=document.getElementById("restore");
	e.value=_("Restore to factory default");
}
function init()
{
 initTranslate();
}
function preSubmit(f) {
	if(window.confirm(_("restore_note")))
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
				    <tr><td valign="top">&nbsp;&nbsp;<script>document.write(_("Click this button to restore the router all settings to factory default"));</script></td>
				    </tr>
					</table>
					<table cellpadding="0" cellspacing="0" class="content3" id="table1">
						<tr><td height="30">&nbsp;&nbsp;<input id="restore" class=button2 onClick="preSubmit(document.frmSetup);" value="" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  type=button></td>
						</tr>
					</table>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("system_restore_Help_Inf1")/*+"<br>"+_("system_restore_Help_Inf2")*/+"<br>"+_("system_restore_Help_Inf3")+"<br>"+_("system_restore_Help_Inf4")+"<br>"+_("system_restore_Help_Inf5"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






