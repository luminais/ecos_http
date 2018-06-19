<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>UPNP | Settings</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
def_UpnpStatus = "<%aspTendaGetStatus("sys","upnpen");%>";
addCfg("UpnpStatus",371,def_UpnpStatus);
Butterlate.setTextDomain("index_routing_virtual");
function init(f){
	cfg2Form(f);
}
function preSubmit(f) {    
	form2Cfg(f);
	f.submit();
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
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup id=frmSetup method=POST action=/goform/VirSerUpnp>
				<input type=hidden name=GO id=GO value=upnp_config.asp>
					<table width=80% cellpadding="0" cellspacing="0" class=content1 id="table1">
						<tr><td width=150 align="right"><script>document.write(_("Enable UPnP"));</script></td> 
							<td>&nbsp;&nbsp;&nbsp;&nbsp;<input type=CHECKBOX name=UpnpStatus id=UpnpStatus value=1></td>
						</tr>
					</table>
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("UPNP_Help_Inf"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>





