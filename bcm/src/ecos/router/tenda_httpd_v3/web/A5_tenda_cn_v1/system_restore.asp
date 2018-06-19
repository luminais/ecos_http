<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>System | Restore</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
function init()
{
}

function preSubmit(f) {
	if(window.confirm("设备将自动重启！ip将更新为：192.168.2.1，密码更新为：admin。如果页面没有刷新请更新您电脑的网络设置后重新登录！"))
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
				    <tr><td valign="top">&nbsp;&nbsp;单击此按钮将使路由器的所有设置恢复到出厂时的默认状态。</td>
				    </tr>
					</table>
					<table cellpadding="0" cellspacing="0" class="content3" id="table1">
						<tr><td height="30">&nbsp;&nbsp;<input class=button2 onClick="preSubmit(document.frmSetup);" value="恢复出厂设置" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  type=button></td>
						</tr>
					</table>
				
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('恢复出厂设置将使路由器的所有设置恢复到出厂时的默认状态。其中：<br>\
			默认的密码为：admin<br>\
			默认的IP地址为：192.168.2.1<br>\
			默认的子网掩码为：255.255.255.0'
			);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






