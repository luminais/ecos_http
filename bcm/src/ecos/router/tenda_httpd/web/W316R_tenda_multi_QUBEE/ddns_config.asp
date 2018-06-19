<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>DDNS | Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript>
addCfg("ddnsEnable",381,"<%aspTendaGetStatus("ddns","en");%>");
addCfg("ddns1",387,'1;dyndns;test1.dyndns.net;user;pass;;;;3600');
Butterlate.setTextDomain("system_tool");
var severnam=<%aspTendaGetStatus("ddns","provide");%>;
function init(f){
	f.elements["serverName"].value=((parseInt(severnam)==0)?2:severnam);
	f.elements["hostName"].value="<%aspTendaGetStatus("ddns","url");%>";
	f.elements["userName"].value="<%aspTendaGetStatus("ddns","user_name");%>";
	f.elements["password"].value="<%aspTendaGetStatus("ddns","pwd");%>";
	cfg2Form(f);
	OnSel();
}
function preSubmit(f) {
  if(document.frmSetup.ddnsEnable[0].checked)
  {
	if (!strCheck(f.elements["hostName"],_("Hostname"))) return;
	if (!strCheck(f.elements["userName"],_("Username"))) return;
	if (!strCheck(f.elements["password"],_("Password"))) return;
	if (f.elements["hostName"].value == "" || f.elements["userName"].value == "" || f.elements["password"].value == "")
	{	alert(_("Please enter the entire DDNS information"));
		return ;
	}
	var re =/^[ ]+$/;	
	var host = f.elements["hostName"].value;
	var user = f.elements["userName"].value;
	if(re.test(host) || re.test(user))
	{
		alert(_("Please enter the correct DDNS information"));
		return;
	}
  }
	form2Cfg(f);
	f.submit();
}
function reg()
{
	var url_match=/(\()(.+)(\))/;
	var str=document.forms[0].serverName.options[document.forms[0].serverName.selectedIndex].text;
	window.open ("http://www."+str);
}

function OnSel()
{
	if(document.frmSetup.ddnsEnable[1].checked)
	{
		document.getElementById("userName").disabled = true;
		document.getElementById("password").disabled = true;
		document.getElementById("domain").disabled = true;
	}else{
		document.getElementById("userName").disabled = false;
		document.getElementById("password").disabled = false;
		document.getElementById("domain").disabled = false;
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
				<form name=frmSetup method=POST action=/goform/SysToolDDNS>
				<input type=hidden name=GO value=ddns_config.asp>
				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tr>
					  <td width=150 align="right"><script>document.write(_("DDNS Service"));</script></td>
					  <td>
						&nbsp;&nbsp;&nbsp;&nbsp;<input name="ddnsEnable" value=1 type="radio" class="radio" onClick="OnSel()" > <script>document.write(_("Enable"));</script> 
						<input name="ddnsEnable" value=0 type="radio" class="radio" onClick="OnSel()"> <script>document.write(_("Disable"));</script> </td></tr>
					<tr>
					  <td align="right"><script>document.write(_("Service Provider"));</script></td>
					<td nowrap>
					  &nbsp;&nbsp;&nbsp;&nbsp;<select NAME="serverName" SIZE=1>
						<option value="2">88ip.cn </option> 
						<option value="5">no-ip.com</option>
					  </select> 
					  <a class="content" href="#" onClick="reg()"><script>document.write(_("Sign up"));</script></a></td>
					</tr>
					<tr>
						<td align="right"><script>document.write(_("Username"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="userName" class="text" maxlength="40" id="userName" size="15" ></td></tr>
					<tr>
						<td align="right"><script>document.write(_("Password"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="password" class="text" id="password" size="15" maxlength="20" type="text" >
						</td>
					</tr>
					<tr>
						<td align="right"><script>document.write(_("Domain Name"));</script></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" maxlength="40" id="domain" name="hostName" size="15">
						</td>
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
<script>helpInfo(_("ddns_config_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("ddns_config_Help_Inf2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


