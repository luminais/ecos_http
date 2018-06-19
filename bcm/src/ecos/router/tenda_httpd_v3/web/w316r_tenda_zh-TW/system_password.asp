<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>System | Administrator Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

addCfg("SYSUN",104, "<%aspTendaGetStatus("sys","username");%>");
addCfg("SYSPS", 106, "" );
addCfg("SYSOPS", 106, "" );
addCfg("SYSPS2", 106, "" );

function init(f){
	cfg2Form(f);
}

function preSubmit(f) { 

	if (!chkStrLen(f.SYSPS,3,12,"password ")) return ;
	form2Cfg(f);
	if (f.SYSPSC.value=='1') // if passwd changed, send old passwd too
		addFormElm("CPW",f.SYSOPS.value);
	if(check_text(f.SYSOPS.value)&&check_text(f.SYSPS.value))
	{
			if (!chkPwdUpdate(f.SYSPS,f.SYSPS2,f.SYSPSC)){
    	 	return;
		  }
    	f.submit();
	}else
	{
		alert("密碼只能由英文字母和數字組成！");
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
					  <td height="25" colspan=2 valign="top" nowrap>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 本頁可修改系統管理員的密碼。</td>
					</tr>
					<tr>
					  <td height="25" colspan=2 valign="top" nowrap>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 請注意：密碼只能由數字、英文字母組成。</td>
					</tr>
					</table>
				<table cellpadding="0" cellspacing="0" class="content3" id="table1">
					<tr><td width="100" align="right">舊密碼</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" type=text maxlength="12" name="SYSOPS" size="15" onKeyPress=chkPwd1Chr2(this,this.form.SYSPS,this.form.SYSPS2,this.form.SYSPSC);></td></tr>
					<tr><td width="100" align="right">新密碼</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" type=text maxlength="12" name="SYSPS" size="15" onKeyPress=chkPwd1Chr2(this.form.SYSOPS,this,this.form.SYSPS2,this.form.SYSPSC);></td></tr>
					<tr><td width="100" align="right">確認新密碼</td>
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
		<script>helpInfo('路由器預設密碼為admin，建議修改此預設密碼。如果不設定密碼，您內部網路上所有的使用者只要在密碼輸入admin就可以設定路由器。<br>\<br>\
		舊密碼：請輸入舊密碼。初次使用路由器時，預設密碼是admin。<br>\
		新密碼：輸入新密碼。密碼必須為3 - 12個字元，且不含空格。<br>\
		確認新密碼：重新輸入新密碼進行確認。<br>\
		請注意：密碼遺失或忘記就無法透過任何方式得知。如果密碼遺失或忘記，您必須將路由器恢復至原廠預設值。'
		);</script>		
		</td>
      </tr>
    </table> 
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

