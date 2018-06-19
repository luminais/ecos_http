<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>System | Remote Management</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var def_RMEN = "<%getfirewall("wan","wanweben");%>";
var def_RMPORT = "<%getfirewall("wan","webport");%>";
var def_RMIP = "<%getfirewall("wan","rmanip");%>";
var lan_ip = "<%aspTendaGetStatus("lan", "lanip");%>";
var lan_ipmask = "<%aspTendaGetStatus("lan", "lanmask");%>";
Butterlate.setTextDomain("dhcp_firewall");
function init(f)
{
	var en = parseInt(def_RMEN);
	f.RMEN.checked = en;
	onSwitch();
    f.RMsIP1.value = def_RMIP;
	f.RMPORT.value = def_RMPORT;
}
function preSubmit(f) {     
	f.RMsIP1.value = clearInvalidIpstr(f.RMsIP1.value);
	f.RMPORT.value = clearInvalidIpstr(f.RMPORT.value);    
	var sip1 = f.RMsIP1.value;
	var port = f.RMPORT.value;
	var loc = "/goform/SafeWanWebMan?GO=system_remote.asp";
	if(f.RMEN.checked)
	{	
		if (!rangeCheck(f.RMPORT,1,65535,"Port")) return;
		loc += "&RMEN=1&port=" + port;
		if (!verifyIP0(f.RMsIP1,_("IP Address"))) return ;
		if(sip1 == "")
			sip1 = "0.0.0.0";
		if (ipCheck(lan_ip,sip1,lan_ipmask)) {
			alert(f.RMsIP1.value+ _("Can not be at the same segment with LAN  IP"));
			f.RMsIP1.value="0.0.0.0";
			return ;
			}
		loc += "&IP=" + sip1;		
	}
	else
	{
		loc += "&RMEN=0";
	}
   var code = 'location="' + loc + '"';
   eval(code);
}
function onSwitch()
{
	if(document.frmSetup.RMEN.checked)
	{
		document.getElementById("RMPORT").disabled = false;	
		document.getElementById("ipTab").style.display = "";
	}
	else
	{
		document.getElementById("ipTab").style.display = "none";
		document.getElementById("RMPORT").disabled = true;
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
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup method="POST" action=/goform/SafeWanWebMan>
				<input type=hidden id=GO value= system_remote.asp>
				<table class=content1>
					<tr> 
					  <td height="30"><script>document.write(_("Enable"));</script>&nbsp;&nbsp;<input type="checkbox" id=RMEN onClick="onSwitch()"></td>
					</tr>
				</table>
				<table cellpadding="0" cellspacing="0" class="content1" id="ipTab" style="margin-top:0px;">				
					<tr>
					  <td width="150" height="30" align="right"><script>document.write(_("Port"));</script></td>
					  <td height="30">
					    &nbsp;&nbsp;&nbsp;&nbsp;<input class=text id=RMPORT size=5 maxlength=5 value="80"></td>
					</tr>
					<tr>
					  <td height="30" align="right"><script>document.write(_("IP Address"));</script></td>
					  <td height="30">
					    &nbsp;&nbsp;&nbsp;&nbsp;<input class=text id=RMsIP1 size=15 maxlength=15 ></td>
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
		<script>helpInfo(_("remote_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("remote_Help_Inf2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("remote_Help_Inf3"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('ipTab');
    </script>
</BODY>
</HTML>






