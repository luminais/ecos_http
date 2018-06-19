<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
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
	f.elements["userName"].value="<%aspTendaGetStatus("ddns","user_name");%>";	//使用者名稱
	f.elements["password"].value="<%aspTendaGetStatus("ddns","pwd");%>";	//密碼

	cfg2Form(f);
	OnSel();
}

function preSubmit(f) {
	//if (!validNumCheck(f.elements["retryTime"],"重試時間")) return ;
  if(document.frmSetup.ddnsEnable[0].checked)
  {
	if (!strCheck(f.elements["hostName"],"主機名稱")) return;
	if (!strCheck(f.elements["userName"],"使用者名稱")) return;
	if (!strCheck(f.elements["password"],"密碼")) return;

	if (f.elements["hostName"].value == "" || f.elements["userName"].value == "" || f.elements["password"].value == "")
	{	alert("請輸入完整的DDNS資料！");
		return ;
	}
	var re =/^[ ]+$/;	
	var host = f.elements["hostName"].value;
	var user = f.elements["userName"].value;
	if(re.test(host) || re.test(user))
	{
		alert("請輸入正確的DDNS資料！");
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
					  <td width=100 align="right">DDNS服務設定</td>
					  <td>
						&nbsp;&nbsp;&nbsp;&nbsp;<input name="ddnsEnable" value=1 type="radio" class="radio" onClick="OnSel()" > 啟用 
						<input name="ddnsEnable" value=0 type="radio" class="radio" onClick="OnSel()"> 停用 </td></tr>
					<tr>
					  <td align="right">服務供應商</td>
					<td nowrap>
					  &nbsp;&nbsp;&nbsp;&nbsp;<select NAME="serverName" SIZE=1>
						<option value="2">88ip.cn </option> 
						<option value="6">3322.org</option>
					  </select> 
					  <a class="content" href="#" onClick="reg()">註冊帳號</a></td>
					</tr>
					
					<tr>
						<td align="right">使用者名稱</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="userName" class="text" maxlength="40" id="userName" size="15" ></td></tr>
					<tr>
						<td align="right">密碼</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="password" class="text" id="password" size="15" maxlength="20" type="text" >
						</td>
					</tr>
					<tr>
						<td align="right">網域名稱</td>
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
<script>helpInfo('您只要在DDNS服務商註冊過您的網域名稱，把註冊的使用者名稱與密碼填入相應位置並儲存設定，您就可以透過DDSN服務使用網域名稱直接連線到您在本地建立的伺服器或路由器。設定 DDNS 前，只需點選<註冊帳號>就可註冊DDNS網域服務名稱。<br>\
&nbsp;&nbsp;&nbsp;&nbsp;DDNS 服務：從下拉式功能表選擇您使用的DDNS服務提供者，並且輸入使用者名稱、密碼 與網域名稱 即可開啟該服務。'
);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

