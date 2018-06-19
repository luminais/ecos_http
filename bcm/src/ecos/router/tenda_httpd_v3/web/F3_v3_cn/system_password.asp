<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Administrator Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body onLoad="init(document.frmSetup);">
<form name="ChangePasswd" method="post" action="/goform/SysToolChangePwd">
<input type=hidden name=GO value="system_password.asp">
<input type=hidden name="SYSOPS" value="">
<input type=hidden name="SYSPS" value="">
<input type=hidden name="SYSPS2" value="">
</form>
<form name="frmSetup" method="POST" action="/goform/SysToolChangePwd">
<input type=hidden name=GO value="system_password.asp">
<input type=hidden name="SYSPSC" value=0>
<input type=hidden name='CPW' disabled="disabled">
	<fieldset>
		<legend>Change Password</legend>
		<p>Administrator Login Credentials</p>
		<p class="text-red">Password must be alpha-numeric.</p>
		<table>
			<tr id="oldpwd" style="display:none">
				<td class="control-label">Old Password</td>
				<td class="controls"><input class="text" type="text" maxlength="12" name="SYSOPS" size="15" onKeyPress="chkPwd1Chr2(this,this.form.SYSPS,this.form.SYSPS2,this.form.SYSPSC);" /></td>
			</tr>
			<tr>
				<td class="control-label">New Password</td>
				<td class="controls"><input class="text" type="text" maxlength="12" name="SYSPS" size="15" onKeyPress="chkPwd1Chr2(this.form.SYSOPS,this,this.form.SYSPS2,this.form.SYSPSC);" /></td>
			</tr>
			<tr>
				<td class="control-label">Confirm New Password</td>
				<td class="controls"><input class="text" type="text" maxlength="12" name="SYSPS2" size="15"  onKeyPress="chkPwd1Chr2(this.form.SYSOPS,this.form.SYSPS,this,this.form.SYSPSC);" /></td>
			</tr>
		</table>
	</fieldset>
	<div class="btn-group">
		<input type="button" class="btn" value="Ok" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.frmSetup)" />
	</div>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "system_tool"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/gozila.js"></script>
<script type="text/javascript" src="js/encode.js"></script>
<script>
var oldpasspord="<%aspTendaGetStatus("sys","password");%>";

function chkPwdUpdate(p,pv,c) {
	if (c.value=='0') return true;
    if (p.value !== pv.value){
		alert(_("The passwords you entered do not match."));
		return false;
	}
    if (!confirm(_("Change password?"))) {
        c.value=0 ;
        p.value = pv.value = p.defaultValue;
        return false;
    }
    return true;
}
function chkPwd1Chr2(po,p,pv,c){
   	if (c.value=='0') {
  		po.value = p.value = pv.value = ""; // reset to null;
  		c.value='1';
	}
}

function init(f){
	if(oldpasspord == "1") {
		document.getElementById("oldpwd").style.display = "";
	}
}
function addFormElm(name,val){
	var elem = document.frmSetup[name];
	elem.disabled = false;
	elem.value = val;
}
function preSubmit(f) { 
	if (!chkStrLen(f.SYSPS,0,12,_("Password"))){
		return;
	} 
	if (f.SYSPSC.value=='1') { //if passwd changed, send old passwd too
		addFormElm("CPW", f.SYSOPS.value);
	}
	if(checkText(f.SYSPS.value)) {
		if (!chkPwdUpdate(f.SYSPS,f.SYSPS2,f.SYSPSC)) {
    	 	return;
		}
    	document.ChangePasswd.SYSOPS.value = str_encode(f.SYSOPS.value);
		document.ChangePasswd.SYSPS.value = str_encode(f.SYSPS.value);
		document.ChangePasswd.SYSPS2.value = str_encode(f.SYSPS2.value);
		document.ChangePasswd.submit();
	} else {
		alert(_("Password must be alpha-numeric characters."));
		return;
	}
}
</script>
</body>
</html>