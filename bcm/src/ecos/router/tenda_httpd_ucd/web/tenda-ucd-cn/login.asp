<!DOCTYPE html>
<html class="index-html">
<head>
<meta charset="utf-8">
<meta http-equiv="Pragma" content="no-cache" />
<title>Tenda</title>
	<link rel="stylesheet" type="text/css" href="css/reasy-1.0.0.css">
	<!-- Fav and touch icons -->
	<link rel="shortcut icon" href="favicon.ico">
	<script src="js/encode.js"></script>
	<script>
	var clearCapTip;
	function showPlaceholder(show) {
		if (show) {
			document.getElementById('password-placeholder').style.display ='block';
		} else {
			document.getElementById('password-placeholder').style.display ='none';
		}
	}
	
	//Bind events to DOM elements
	function initControl() {
		var loginPasElem = document.getElementById('login-password'),
			loginPasBoxElem = document.getElementById('login-password-box');
			
		loginPasElem.onfocus = function () {
			loginPasBoxElem.className += " password-box-focus";
		}
		loginPasElem.onblur = function () {
			loginPasBoxElem.className = "control-group password-box";
		}
		
		loginPasElem.onkeyup = function (e) {
			showPlaceholder(loginPasElem.value === '');
			//添加大写提示
			var e = e || window.event,
				myKeyCode  =  e.charCode || e.keyCode || e.which,
				pattern = /[A-Z]/g,
				msgElem = document.getElementById("message-error");
			if (myKeyCode < 65 || myKeyCode > 90) {
				return true;
			}
			if(!pattern.test(this.value[this.value.length - 1])) {
				return true;
			}
			if(clearCapTip) {
				clearTimeout(clearCapTip);
			}
			msgElem.innerHTML = '你输的是大写字母！';
			document.getElementById("forgetpwd").style.display = "none";
			clearCapTip = window.setTimeout(function(){
				msgElem.innerHTML = " ";
			}, 700);
		}
		document.getElementById('password-placeholder').onclick = function() {
			loginPasElem.focus();
		}
	}
	
	function init(){
		var showmessage = "";
		
		if(location.href != top.location.href) {
			top.location.href = "login.asp";
		}
		initControl();
		
		document.Login.Password.value = "";
		showPlaceholder(true)
		document.Login.Password.focus();

		if(location.search.substring(1) == '0') {
			document.getElementById("message-error").innerHTML = '密码输入错误';
			document.getElementById("forgetpwd").style.display = "inline";
		}
		
	}

	function enterDown(f,events) {
		
		//解决火狐浏览器不支持event事件的问题。
		var e = events || window.event,
			char_code = e.charCode || e.keyCode;
		
		if(char_code == 13) {
			if(e.preventDefault) {
				e.preventDefault();
			} else  {
				e.returnValue = false;
			}
			preSubmit(f);
		}
		
		
		return false;
	}

	document.onkeydown = function(e) {
		
		enterDown(document.Login, e);
	};

	function preSubmit(f) {
		var passwordVal = f.Password.value;
		
		if(passwordVal=="") {
			document.getElementById("message-error").innerHTML = '密码不能为空';
			document.getElementById("forgetpwd").style.display = "none";
			f.Password.focus();
			return false;
		}
		if(passwordVal.length>32 || passwordVal.length < 3) {
		  document.getElementById("message-error").innerHTML = '密码的长度只能为3-32个字符';
		  document.getElementById("forgetpwd").style.display = "none";
			f.Password.focus();
			return false;
		}
		f.Password.value = str_encode(passwordVal);
		
		f.submit();
	}

	</script>
	</head>

<body class="login-body" onLoad="init()">
	<!--[if lt IE 7]>
		<div id="nosupport">
		<p  style="color: red;">您的浏览器版本太低，为了更好的显示页面，请升级到IE8及以上版本或安装火狐，谷歌等浏览器</p>
		<a href="#" onclick="document.getElementById('nosupport').style.display='none';">关闭提示</a>
		</div>
	<![endif]-->
	<div class="masthead">
		<div class="container head-inner">
			<a class="brand" href="#" title="Tenda"></a>
        <div class="easy-logo"></div>
		</div>
		<div class="footer"></div>
	</div>
	
	<div class="container">
		<form name="Login" class="login" method="post" action="/LoginCheck">
			<input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
			<input type="hidden" name="checkEn" value="0">
			<h1 class="login-title">登录</h1>
			<div class="container">
				<p class="login-massage text-error"><span id="message-error">&nbsp;</span><a id="forgetpwd" >忘记密码?<span class="tip">如忘记密码，只能将路由器复位。<br/>方法：连接电源，长按路由器上的RST键约10秒后松开(注意：此操作将还原路由器所有设置，您需重新设置路由器才能上网)。</span></a></p>
				<div class="control-group none" id="passwrod-tip">
					<div class="control-label">密码提示</div>
					<div class="controls controls-text" id="passwrod-tip-text"></div>
				</div>
				<div id="login-password-box" class="control-group password-box">
					<label class="control-label" for="login-password"></label>
					<div class="controls">
						<input type="password" id="login-password" name="Password" class="text" />
						<span id="password-placeholder">密 码</span>
					</div>
				</div>
				<div id="submitOperation" class="btn-group">
					<input type="button" value="登  录" class="btn" onClick="preSubmit(document.Login)" />
				</div>
			</div>
		</form>
	</div>
</body>
</html>
