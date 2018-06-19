<!DOCTYPE html>
<html class="login-html">
<head>
<meta charset="utf-8">
<title>Tenda 11N Wireless Router Login Screen</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body class="login-body">
<form name="Auth" method="post" action="/LoginCheck">
<input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
<input type="hidden" name="Password" value="">
</form>
	<div class="navbar">
		<div class="navbar-inner">
			<a class="brand" href="index.asp"></a>
		</div>
	</div>
	<div class="container login">
		<form name="Login" method="post" action="/LoginCheck">
			<input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
			<input type="hidden" name="checkEn" value="0">
			<h1 class="login-title">Login</h1>
			<div class="container">
				<p class="login-massage"><span>Default:</span> admin</p>
				<div class="control-group">
					<div class="control-label">Password:</div>
					<div class="controls">
						<input type="password" name="Password" maxlength="12" class="text input-medium"  onKeyDown="enterDown(document.Login,event);"/>
					</div>
				</div>
				<p id="pwd_err_msg" class="text-red none">Wrong password, please enter the password again!</p>
				<div class="control-group" id="login_tip" style="display:none;">
					<div class="controls">
						<label class="checkbox" for="nocheck">
							<input type="checkbox" id="nocheck" name="nocheck"/>No longer prompts
						</label>
					</div>
				</div>
				<div id="submitOperation" class="btn-group">
					<a href="#" class="btn btn-link" onClick="preSubmit(document.Login)">OK</a>
					<a href="login.asp" class="btn btn-link last">Cancel</a>
				</div>
			</div>
		</form>
	</div>
	<div style="height: 50px;"></div>
	<div class="page-footer"></div>
<script src="lang/b28n.js" type="text/javascript"></script>
<script src="js/encode.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain("base");
	B.translate();
})();

var error = [_("No Ethernet cable connected to WAN port!"), _("No Internet access, please check your online settings!")],
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
	//document.getElementById("message").innerHTML = showmessage;
	if(location.search.substring(1) == '0'){
		document.getElementById("pwd_err_msg").className = 'help-block text-red';
	}
}

function enterDown(f,e) {
//	var char_code = e.charCode ? e.charCode : e.keyCode;	//解决火狐浏览器不支持event事件的问题。
	var char_code = e.keyCode ? e.keyCode : e.which ? e.which : e.charCode; 
	if(char_code == 13) {
		preSubmit(f);
		if(e.preventDefault){ 
			e.preventDefault(); 
		}else{ 
			e.returnValue = false; 
		}
	}
}

function preSubmit(f) {	
	if(f.nocheck.checked) {
	 	f.checkEn.value=1;
	} else {
	 	f.checkEn.value=0;
	}
	if(f.Password.value=="") {
		alert(_("Password can not be empty!")); 
		f.Password.focus();
		return false;
	}
	if(f.Password.value.length>13) {
		alert(_("Password can not exceed 12 characters in length!")); 
		f.Password.focus();
		return false;
	}
	document.Auth.Password.value = str_encode(f.Password.value);
	document.Auth.submit();
}

window.onload = function () {
	init();
};
</script>
</body>
</html>
