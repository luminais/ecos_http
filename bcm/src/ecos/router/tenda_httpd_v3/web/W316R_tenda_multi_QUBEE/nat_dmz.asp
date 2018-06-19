<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>NAT | DMZ</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var def_LANMASK = "<%aspTendaGetStatus("lan", "lanmask");%>";
var def_LANIP = "<%aspTendaGetStatus("lan", "lanip");%>";
var def_dmzen = "<%aspTendaGetStatus("lan","dmzen");%>";
var def_DMZIP = "<%aspTendaGetStatus("lan","dmzip");%>";
addCfg("LANIP", 0,def_LANIP);
addCfg("en", 1, def_dmzen);
addCfg("DMZ1", 2, def_DMZIP);
Butterlate.setTextDomain("index_routing_virtual");
var netip=getCfg("LANIP").replace(/\.\d{1,3}$/,".");
function init(f){
	f.dmzip.value=def_DMZIP;
	cfg2Form(f);
}
function preSubmit(f) {
	var m;
	if (f.dmzip.value=='' && (!f.en.checked) ) m='';
	else
	{
		if (f.dmzip.value==getCfg("LANIP")) { alert(_("Can not use the router IP address")); return; }
		if (!verifyIP2(f.dmzip,_("DMZ host IP address"))) return ;
		if ( !ipCheck(def_LANIP,f.dmzip.value, def_LANMASK) ) 
		{
			alert(f.dmzip.value+ _("not in the same segment with LAN IP"));
			f.dmzip.value="0.0.0.0";
			return ;
		}
		if(f.dmzip.length<8)
			alert(_("illegal IP"));
		if (f.dmzip.value=='' || (!f.en.checked) ) m='';
		else
		{
		  m="1;0;"+f.dmzip.value;
		}
	}
	setCfg("DMZ1",m);
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
        <td width="679" valign="top" headers="100%">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup method=POST action=/goform/VirSerDMZ>
				<input type=hidden name=GO value=nat_dmz.asp >
				<table cellpadding="0" cellspacing="0" class="content2">
				<TR><TD colspan=3 valign="top">&nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(_("DMZ_Note"));</script></TD>
				</TR>
				</table>
				<table cellpadding="0" cellspacing="0" class="content1" id="table1" style="margin-top:0px;">
					<tr>
						<td width="150" align="right" nowrap><script>document.write(_("DMZ host IP address"));</script></td>
						<td width="157">&nbsp;&nbsp;&nbsp;&nbsp;
					    <input class=text name=dmzip size=16 maxlength=15></td>
						<td width="132"><input type=checkbox name=en value=1>&nbsp;<script>document.write(_("Enable"));</script></td>
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
		<script>helpInfo(_("DMZ_Help_Inf"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


