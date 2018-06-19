<!DOCTYPE html>
<html class="login-html">
<head>
<meta charset="utf-8">
<title>TENDA 11N无线路由器登录界面</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language=JavaScript>
var error=new Array("WAN口无网线连接！","路由器无法访问互联网，请检查您的上网设置。"),
	message="<%sysTendaGetStatus("wan","linkstatus");%>",
	message2="<%sysTendaGetStatus("wan","login_where");%>";
function init(){
	if(location.href != top.location.href)
	{
		top.location.href = "login.asp";
	}
	var showmessage = "";
	
	document.Login.Password.value="";
	document.Login.Password.focus();
	if(message==0) {
		showmessage=error[0];
	} else if(message == 2 || message==3) {
		showmessage=error[1];
	}
	if(message2 == 0){
		//from lan,donnt display login tip
		//document.getElementById("login_tip").style.display = '';		
	}
	document.getElementById("message").innerHTML = showmessage;
	if(location.search.substring(1) == '1'){
		document.getElementById("pwd_err_msg").className = 'help-block text-red';
	}
}

function enterDown(f,e) {
	var char_code = e.charCode ? e.charCode : e.keyCode;	//解决火狐浏览器不支持event事件的问题。
	if(char_code == 13) {
		preSubmit(f);
	} else {
		return;
	}
	return;
}

function preSubmit(f) {	
	if(f.nocheck.checked) {
	 	f.checkEn.value=1;
	} else {
	 	f.checkEn.value=0;
	}
	if(f.Password.value=="") {
		alert("密码不能为空!"); 
		f.Password.focus();
		return false;
	}
	if(f.Password.value.length>13) {
	  alert("密码的长度不能超过12个字符!"); 
		f.Password.focus();
		return false;
	}
	f.submit();
}

</script>
</head>

<body class="login-body" onLoad="init()">
	<div class="navbar">
		<div class="navbar-inner">
			<a class="brand" href="index.asp"></a>
		</div>
	</div>
	<div class="container login">
		<form name="Login" method="post" action="/LoginCheck">
			<input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
			<input type="hidden" name="checkEn" value="0">
			<h1 class="login-title">登录</h1>
			<p id="message" class="login-massage text-red"></p>
			<div class="container">
			<div class="control-group">
				<div class="control-label">密码</div>
				<div class="controls">
					<input type="password" name="Password" maxlength="12" class="text"  onKeyDown="enterDown(document.Login,event);"/>
					<span id="pwd_err_msg" class="help-block none">登录密码不正确，请重新输入！</span>
				</div>
			</div>
			<div class="control-group" id="login_tip" style="display:none;">
				<div class="controls">
					<label class="checkbox" for="nocheck">
						<input type="checkbox" id="nocheck" name="nocheck"/>不再提示
					</label>
				</div>
			</div>
			<div id="submitOperation" class="btn-group">
				<input type="button" value="确 定" class="btn btn-link" onClick="preSubmit(document.Login)" />
				<input type="reset" value="取 消" class="btn btn-link" />
			</div>
			</div>
		</form>
	</div>
</body>
</html>
