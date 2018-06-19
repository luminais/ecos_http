<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>WAN | DNS</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var def_DNS1 = "<%aspTendaGetStatus("wan","dns1");%>";
var def_DNS2 = "<%aspTendaGetStatus("wan","dns2");%>";
var def_dnsen = "<%aspTendaGetStatus("wan","dnsen");%>";
addCfg("DS1",0x34,def_DNS1);
addCfg("DS2",0x35,def_DNS2);
Butterlate.setTextDomain("wan_set");
function init(f)
{
	cfg2Form(f);
	if(parseInt(def_dnsen) == 1)
		f.DNSEN.checked = true;
	else
		f.DNSEN.checked = false;	
	onSel();
}
function preSubmit(f) {
	if(document.frmSetup.DNSEN.checked == true)
	{
	    	if (!verifyIP2(f.DS1,_("Primary DNS"))) return ;
	    	if((f.DS2.value !="") )
	    	{
	    		if (!verifyIP2(f.DS2,_("Alternate DNS"))) return ;
	    	}
			f.DSEN.value = "1";
    	}
	else
		f.DSEN.value = "0";
	form2Cfg(f);
	f.submit();
}
function onSel()
{
	if(document.frmSetup.DNSEN.checked == false)
		document.frmSetup.DS1.disabled = document.frmSetup.DS2.disabled = true;
	else
		document.frmSetup.DS1.disabled = document.frmSetup.DS2.disabled = false;
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
				<form name=frmSetup method=POST action=/goform/AdvSetDns>
				<input type=hidden name=GO value=wan_dns.asp>
				<input type="hidden" id="rebootTag" name="rebootTag">
				<input type="hidden" name="DSEN">
				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tr> 
						<td width="150" align="right"><script>document.write(_("DNS setting"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;
					    <input type="checkbox" name="DNSEN"  onClick="onSel()"></td>
					</tr>
					<tr> 
						<td width="150" align="right"><script>document.write(_("Primary DNS address"));</script></td>  
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=DS1 class=text size=15 maxlength=15></td>
					</tr>
					<tr>
						<td width="150" align="right"><script>document.write(_("Alternate DNS address"));</script>&nbsp;(
					    <script>document.write(_("Optional"));</script>)</td> 
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=DS2 class=text size=15 maxlength=15></td>
					</tr>	
					</table>
					<table cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
				      <tr><td colspan="2">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
					<script>document.write(_("Note: After the settings are completed, reboot the device to activate the modified settings."));</script>	
					  </td></tr>
					</table>
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("Wan_dns_helpinfo1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Wan_dns_helpinfo2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


