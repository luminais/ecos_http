<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>DHCP | Server</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>";
var LANIP=def_LANIP;
var netip=LANIP.replace(/\.\d{1,3}$/,".");
Butterlate.setTextDomain("dhcp_firewall");
function init()
{
	document.LANDhcpsSet.DHEN.checked = <%aspTendaGetStatus("lan","dhcps");%>;
	document.LANDhcpsSet.DHLT.value = "<%aspTendaGetStatus("lan","lease_time");%>";
	document.LANDhcpsSet.dips.value = (("<%aspTendaGetStatus("lan","dhcps_start");%>").split("."))[3];
	document.LANDhcpsSet.dipe.value = (("<%aspTendaGetStatus("lan","dhcps_end");%>").split("."))[3];
}
function preSubmit(f) {
	var loc = "/goform/DhcpSetSer?GO=lan_dhcps.asp";
	if (!rangeCheck(f.dips,1,254,_("IP pool start address"))) return ;
	if (!rangeCheck(f.dipe,1,254,_("IP pool end address"))) return ;
   	if (Number(f.dips.value)>Number(f.dipe.value)) {
      alert(_("IP pool start address can not bigger than IP pool end address"));
      return ;
   	}
	if(f.DHEN.checked)
	{
		loc += "&dhcpEn=1";
	}
	else
	{
		loc += "&dhcpEn=0";
	}
	loc += "&dips=" + netip + f.dips.value;
	loc += "&dipe=" + netip + f.dipe.value;
	loc += "&DHLT=" + f.DHLT.value;	
	var code = 'location="' + loc + '"';
	eval(code);
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init()" class="bg">
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
				<form name=LANDhcpsSet method=POST action=/goform/DhcpSetSer>
				<input type=hidden name=GO  value="lan_dhcps.asp">
				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tr> 
					  <td width="150" align="right"><script>document.write(_("DHCP Server"));</script></td> 
					  <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
					    <input type="checkbox" name=DHEN value=1><script>document.write(_("Enable"));</script></td>
					</tr>
					<tr> 
					  <td width="150" align="right"><script>document.write(_("IP pool start address"));</script></td> 
					  <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
					    <SCRIPT>document.write(netip);</SCRIPT>
						<input NAME=dips class=text  SIZE="3"></td>
					</tr>
					<tr> 
						<td width="150" align="right"><script>document.write(_("IP pool end address"));</script></td> 
						<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
						  <SCRIPT>document.write(netip);</SCRIPT>
						<input NAME=dipe class=text  SIZE=3></td>
					</tr>
					<tr> 
					  <td width="150" align="right"><script>document.write(_("Lease Time"));</script></td> 
					  <td align="left">
						&nbsp;&nbsp;&nbsp;&nbsp;
						<select NAME=DHLT SIZE=1>
						<option VALUE="3600"><script>document.write(_("One hour"));</script></option>
						<option VALUE="7200"><script>document.write(_("Two hours"));</script></option>
						<option VALUE="10800"><script>document.write(_("Three hours"));</script></option>
						<option VALUE="86400"><script>document.write(_("One day"));</script></option>
						<option VALUE="172800"><script>document.write(_("Two days"));</script></option>
						<option VALUE="604800"><script>document.write(_("One week"));</script></option>
						</select>
					  </td>
					</tr>
				</table>  
				<input type=hidden name=dhcpEn>
					<SCRIPT>tbl_tail_save("document.LANDhcpsSet");</SCRIPT>
				</form>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("Lan_dncps_helpinfo1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Lan_dncps_helpinfo2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Lan_dncps_helpinfo3"));			        </script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






