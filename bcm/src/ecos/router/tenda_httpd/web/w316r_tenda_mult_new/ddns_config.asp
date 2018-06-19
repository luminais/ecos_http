<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>DDNS | Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method=POST action=/goform/SysToolDDNS>
    <input type=hidden name=GO value=ddns_config.asp>
	<fieldset class="table-field">
		<legend>DDNS</legend>
		<div class="control-group">
			<label class="control-label">DDNS Service</label>
			<div class="controls">
				<label class="radio"><input name="ddnsEnable" value=1 type="radio" class="radio" onClick="OnSel()" /> Enable</label>
				<label class="radio"><input name="ddnsEnable" value=0 type="radio" class="radio" onClick="OnSel()" /> Disable</label> 
			</div>
		</div>
		<table>
			<tr>
				<td class="control-label">Service Provider</td>
				<td class="controls" nowrap>
					<select NAME="serverName" SIZE=1>
						<option value="1">dyndns.org</option> 
						<option value="5">no-ip.com</option>
					</select> 
					<a href="#" onClick="reg()">Sign up</a>
				</td>
			</tr>
			<tr>
				<td class="control-label">Username</td>
				<td class="controls"><input name="userName" class="text" maxlength="40" id="userName" size="15" ></td></tr>
			<tr>
				<td class="control-label">Password</td>
				<td class="controls"><input name="password" class="text" id="password" size="15" maxlength="20" type="text" ></td>
			</tr>
			<tr>
				<td class="control-label">Domain Name</td>
				<td class="controls"><input class="text" maxlength="40" id="domain" name="hostName" size="15"></td>
			</tr>
		</table>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn" value="Cancel" onClick="init(document.frmSetup)" />
	</div>				
</form>
<div id="save" class="none"></div>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "applications"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
addCfg("ddnsEnable",381,"<%aspTendaGetStatus("ddns","en");%>");//DDNS是否启用：0：不启用；1:启用
addCfg("ddns1",387,'1;dyndns;test1.dyndns.net;user;pass;;;;3600');
var severnam=<%aspTendaGetStatus("ddns","provide");%>;

function init(f) {
	f.elements["serverName"].value=((parseInt(severnam)==0)?1:severnam);//?????:0:oray.net;1:dyn.dns
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
	{	alert(_("Please complete all fields with your correct DDNS account information!"));
		return ;
	}
	var re =/^[ ]+$/;	
	var host = f.elements["hostName"].value;
	var user = f.elements["userName"].value;
	if(re.test(host) || re.test(user)) {
		alert(_("Please complete all fields with your correct DDNS account information!"));
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
</body>
</html>