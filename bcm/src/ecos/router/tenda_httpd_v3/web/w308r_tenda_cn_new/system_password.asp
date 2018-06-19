<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | Administrator Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var oldpasspord="<%aspTendaGetStatus("sys","password");%>";

function chkPwdUpdate(p,pv,c) {
	if (c.value=='0') return true;
    if (p.value !== pv.value){
		alert("密码不一致!");
		return false;
	}
    if (!confirm('改变密码吗？')) {
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
	if (!chkStrLen(f.SYSPS,0,12,"密码")){
		return;
	} 
	if (f.SYSPSC.value=='1') { //if passwd changed, send old passwd too
		addFormElm("CPW", f.SYSOPS.value);
	}
	if(checkText(f.SYSPS.value)) {
		if (!chkPwdUpdate(f.SYSPS,f.SYSPS2,f.SYSPSC)) {
    	 	return;
		}
    	f.submit();
	} else {
		alert("密码只能由字母和数字组成!");
		return;
	}
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name="frmSetup" method="POST" action="/goform/SysToolChangePwd">
<input type=hidden name=GO value="system_password.asp">
<input type=hidden name="SYSPSC" value=0>
<input type=hidden name='CPW' disabled="disabled">
	<fieldset>
		<h2 class="legend">登录密码</h2>
		<p>默认登录密码为空，设置登录密码后，再次进入路由器界面时需输入正确的登录密码。</p>
		<p class="text-red">注意：密码只能由数字、字母组成。</p>
		<table id="table1">
			<tr id="oldpwd" style="display:none">
				<td class="control-label">旧密码</td>
				<td class="controls"><input class="text" type="password" maxlength="12" name="SYSOPS" size="15" onKeyPress=chkPwd1Chr2(this,this.form.SYSPS,this.form.SYSPS2,this.form.SYSPSC);></td>
			</tr>
			<tr>
				<td class="control-label">新密码</td>
				<td class="controls"><input class="text" type="password" maxlength="12" name="SYSPS" size="15" onKeyPress=chkPwd1Chr2(this.form.SYSOPS,this,this.form.SYSPS2,this.form.SYSPSC);></td>
			</tr>
			<tr>
				<td class="control-label">确认新密码</td>
				<td class="controls"><input class="text" type="password" maxlength="12" name="SYSPS2" size="15"  onKeyPress=chkPwd1Chr2(this.form.SYSOPS,this.form.SYSPS,this,this.form.SYSPSC);></td>
			</tr>
		</table>
	</fieldset>
	<div class="btn-group">
		<input type="button" class="btn" value="确 定" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn" value="取 消" onClick="init(document.frmSetup)" />
	</div>
</form>
</body>
</html>