<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>System | Administrator Settings</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
addCfg("SYSUN",104, "<%aspTendaGetStatus("sys","username");%>");
addCfg("SYSPS", 106, "" );
addCfg("SYSOPS", 106, "" );
addCfg("SYSPS2", 106, "" );
Butterlate.setTextDomain("system_tool");
function init(f){
	cfg2Form(f);
}
function preSubmit(f) { 
	if (!chkStrLen(f.SYSPS,0,12,_("Password")+" ")) return ;
	form2Cfg(f);
	if (f.SYSPSC.value=='1')
		addFormElm("CPW",f.SYSOPS.value);
	if(check_text2(f.SYSOPS.value)&&check_text2(f.SYSPS.value))
	{
			if (!chkPwdUpdate(f.SYSPS,f.SYSPS2,f.SYSPSC)){
    	 	return;
		  }
    	f.submit();
	}else
	{
		alert(_("The initial password can only consist of letters and numbers"));
		return;
	}	
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0"  height="100%">
              <tr>
                <td align="center" valign="top">
					<form name=frmSetup method=POST action=/goform/SysToolChangePwd>
					<input type=hidden name=GO value="system_password.asp">
					<input type=hidden name="SYSPSC" value=0>
					<table cellpadding="0" cellspacing="0" class="content2">
					<tr>
					  <td height="25" colspan=2 valign="top" nowrap>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <script>document.write(_("On this page ,you can change  the administrator password"));</script></td>
					</tr>
					<tr>
					  <td height="25" colspan=2 valign="top" nowrap>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <script>document.write(_("password_note"));</script></td>
					</tr>
					</table>
				<table cellpadding="0" cellspacing="0" class="content3" id="table1">
					<tr><td width="150" align="right"><script>document.write(_("Old password"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" type=text maxlength="12" name="SYSOPS" size="15" onKeyPress=chkPwd1Chr2(this,this.form.SYSPS,this.form.SYSPS2,this.form.SYSPSC);></td></tr>
					<tr><td width="150" align="right"><script>document.write(_("New password"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" type=text maxlength="12" name="SYSPS" size="15" onKeyPress=chkPwd1Chr2(this.form.SYSOPS,this,this.form.SYSPS2,this.form.SYSPSC);></td></tr>
					<tr><td width="150" align="right"><script>document.write(_("Confirm new password"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" type=text maxlength="12" name="SYSPS2" size="15"  onKeyPress=chkPwd1Chr2(this.form.SYSOPS,this.form.SYSPS,this,this.form.SYSPSC);></td></tr>
				</table>	
					<SCRIPT>
						tbl_tail_save("document.frmSetup");
					</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("system_password_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("system_password_Help_Inf2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("system_password_Help_Inf3")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("system_password_Help_Inf4"));</script>
		</td>
      </tr>
    </table> 
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






