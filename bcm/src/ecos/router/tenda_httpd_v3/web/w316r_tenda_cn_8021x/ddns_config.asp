<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>DDNS | Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
addCfg("ddnsEnable",381,"<%aspTendaGetStatus("ddns","en");%>");//DDNS是否启用：0：不启用；1:启用
addCfg("ddns1",387,'1;dyndns;test1.dyndns.net;user;pass;;;;3600');

var severnam=<%aspTendaGetStatus("ddns","provide");%>;

function init(f){

	f.elements["serverName"].value=((parseInt(severnam)==0)?2:severnam);//?????:0:oray.net;1:dyn.dns
	f.elements["hostName"].value="<%aspTendaGetStatus("ddns","url");%>";	//域名
	f.elements["userName"].value="<%aspTendaGetStatus("ddns","user_name");%>";	//用户名
	f.elements["password"].value="<%aspTendaGetStatus("ddns","pwd");%>";	//密码

	cfg2Form(f);
	OnSel();
}

function preSubmit(f) {
	//if (!validNumCheck(f.elements["retryTime"],"重试时间")) return ;
  if(document.frmSetup.ddnsEnable[0].checked)
  {
	if (!strCheck(f.elements["hostName"],"主机名")) return;
	if (!strCheck(f.elements["userName"],"用户名")) return;
	if (!strCheck(f.elements["password"],"密码")) return;

	if (f.elements["hostName"].value == "" || f.elements["userName"].value == "" || f.elements["password"].value == "")
	{	alert("请输入完整的DDNS信息！");
		return ;
	}
	var re =/^[ ]+$/;	
	var host = f.elements["hostName"].value;
	var user = f.elements["userName"].value;
	if(re.test(host) || re.test(user))
	{
		alert("请输入正确的DDNS信息！");
		return;
	}
  }
	form2Cfg(f);
	f.submit();
	//subForm(f,'do_cmd.htm','DDNS','ddns_config.htm');
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
					  <td width=100 align="right">DDNS服务</td>
					  <td>
						&nbsp;&nbsp;&nbsp;&nbsp;<input name="ddnsEnable" value=1 type="radio" class="radio" onClick="OnSel()" > 启用 
						<input name="ddnsEnable" value=0 type="radio" class="radio" onClick="OnSel()"> 不启用 </td></tr>
					<tr>
					  <td align="right">服务提供商</td>
					<td nowrap>
					  &nbsp;&nbsp;&nbsp;&nbsp;<select NAME="serverName" SIZE=1>
						<option value="2">88ip.cn </option> 
						<option value="6">3322.org</option>
					  </select> 
					  <a class="content" href="#" onClick="reg()">注册去</a></td>
					</tr>
					
					<tr>
						<td align="right">用户名</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="userName" class="text" maxlength="40" id="userName" size="15" ></td></tr>
					<tr>
						<td align="right">密码</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="password" class="text" id="password" size="15" maxlength="20" type="text" >
						</td>
					</tr>
					<tr>
						<td align="right">域名</td>
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
<script>helpInfo('DDNS(动态 DNS)服务允许您为动态 WAN IP 地址分配一个固定域名，从而可监控您的 LAN 中的 Web, FTP 或其它 TCP/IP 。设定 DDNS 前，您只需点击“注册去”就可访问相应的网站，去注册一个域名。<br>\
&nbsp;&nbsp;&nbsp;&nbsp;DDNS 服务：从下拉菜单选择相应的DDNS服务提供商，并且输入用户名、密码 与域名 即可开启该服务。'
);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>





