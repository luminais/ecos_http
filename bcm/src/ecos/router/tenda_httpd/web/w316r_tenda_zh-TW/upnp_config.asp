<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>UPNP | Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

def_UpnpStatus = "<%aspTendaGetStatus("sys","upnpen");%>";
addCfg("UpnpStatus",371,def_UpnpStatus);

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
						<tr><td width=100 align="right">啟用UPnP</td> 
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
		<script>
		helpInfo('(通用隨插即用)允許自動發現和設定網路上支援UPnP的設備或程式。UPnP程式端目前僅被Windows ME和Windows XP以後的系統支援。');
		</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

