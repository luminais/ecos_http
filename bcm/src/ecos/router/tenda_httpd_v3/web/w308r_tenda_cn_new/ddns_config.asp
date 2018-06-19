<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>DDNS | Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
addCfg("ddnsEnable",381,"<%aspTendaGetStatus("ddns","en");%>");//DDNS是否启用：0：不启用；1:启用
addCfg("ddns1",387,'1;dyndns;test1.dyndns.net;user;pass;;;;3600');

var severnam=<%aspTendaGetStatus("ddns","provide");%>;

function init(f) {
	f.elements["serverName"].value=((parseInt(severnam)==0)?2:severnam);//?????:0:oray.net;1:dyn.dns
	f.elements["hostName"].value="<%aspTendaGetStatus("ddns","url");%>";	//域名
	f.elements["userName"].value="<%aspTendaGetStatus("ddns","user_name");%>";	//用户名
	f.elements["password"].value="<%aspTendaGetStatus("ddns","pwd");%>";	//密码
	cfg2Form(f);
	OnSel();
}

function preSubmit(f) {
  if(document.frmSetup.ddnsEnable[0].checked)
  {
	if (f.elements["hostName"].value == "" || f.elements["userName"].value == "" || f.elements["password"].value == "")
	{	alert("请输入完整的DDNS信息！");
		return ;
	}
	var re =/^[ ]+$/;	
	var host = f.elements["hostName"].value;
	var user = f.elements["userName"].value;
	if(re.test(host) || re.test(user)) {
		alert("请输入正确的DDNS信息！");
		return;
	}
  }
	form2Cfg(f);
	f.submit();
	showSaveMassage();
}
function reg() {
	var url_match=/(\()(.+)(\))/;
	var str=document.forms[0].serverName.options[document.forms[0].serverName.selectedIndex].text;

	window.open("http://"+"www." + str, "_blank");
}

function OnSel(){
	if(document.frmSetup.ddnsEnable[1].checked){
		document.getElementById("userName").disabled = true;
		document.getElementById("password").disabled = true;
		document.getElementById("domain").disabled = true;
	}else{
		document.getElementById("userName").disabled = false;
		document.getElementById("password").disabled = false;
		document.getElementById("domain").disabled = false;
	}
}

</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method=POST action=/goform/SysToolDDNS>
    <input type=hidden name=GO value=ddns_config.asp>
	<fieldset>
		<h2 class="legend">DDNS设置</h2>
		<div class="control-group">
			<label class="control-label">DDNS服务</label>
			<div class="controls">
				<label class="radio"><input name="ddnsEnable" value=1 type="radio" class="radio" onClick="OnSel()" /> 启用</label>
				<label class="radio"><input name="ddnsEnable" value=0 type="radio" class="radio" onClick="OnSel()" /> 不启用</label> 
			</div>
		</div>
		<table>
			<tr>
				<td class="control-label">服务提供商</td>
				<td class="controls" nowrap>
					<select NAME="serverName" SIZE=1>
						<option value="2">88ip.cn </option> 
						<option value="6">3322.org</option>
					</select> 
					<a href="#" onClick="reg()">注册去</a>
				</td>
			</tr>
			<tr>
				<td class="control-label">用户名</td>
				<td class="controls"><input name="userName" class="text" maxlength="40" id="userName" size="15" ></td></tr>
			<tr>
				<td class="control-label">密码</td>
				<td class="controls"><input name="password" class="text" id="password" size="15" maxlength="20" type="text" ></td>
			</tr>
			<tr>
				<td class="control-label">域名</td>
				<td class="controls"><input class="text" maxlength="40" id="domain" name="hostName" size="15"></td>
			</tr>
		</table>
	</fieldset>
    <script>tbl_tail_save("document.frmSetup");</script>					
</form>
<div id="save" class="none"></div>
</body>
</html>