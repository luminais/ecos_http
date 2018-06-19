<!DOCTYPE html>
<html class="index-html">
<head>
<meta charset="utf-8">
<meta http-equiv="Pragma" content="no-cache" />
<title>Tenda</title>
<link rel="stylesheet" type="text/css" href="css/reasy-1.0.0.css">
<!-- Fav and touch icons -->
<link rel="shortcut icon" href="favicon.ico">
<script src="lang/b28n.js"></script>
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
        if(!pattern.test(this.value.slice(-1))) {
            return true;
        }
        if(clearCapTip) {
            clearTimeout(clearCapTip);
        }
        msgElem.innerHTML = _('You entered capitals!');
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
        document.getElementById("message-error").innerHTML = _('Password error.');
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
        document.getElementById("message-error").innerHTML = _('The password field cannot be empty.');
        document.getElementById("forgetpwd").style.display = "none";
        f.Password.focus();
        return false;
    }
    if(passwordVal.length>32 || passwordVal.length < 3) {
      document.getElementById("message-error").innerHTML = _('Password length is within 3~32.');
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
    <p  style="color: red;">Your browser is outdated. For better display, suggest updating to IE8 or newer version,  or setting up Firefox or Google.</p>
    <a href="#" onclick="document.getElementById('nosupport').style.display='none';">Close</a>
    </div>
<![endif]-->
<div class="masthead">
  <div class="container head-inner"> <a class="brand" href="#" title="Tenda"></a>
    <div class="easy-logo"></div>
  </div>
  <div class="footer"></div>
</div>
<div class="container">
  <form name="Login" class="login" method="post" action="/LoginCheck">
    <input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
    <input type="hidden" name="checkEn" value="0">
    <h1 class="login-title">Login</h1>
    <div class="container">
      <p class="login-massage text-error"><span id="message-error">&nbsp;</span><a id="forgetpwd" >Forget your password?<span class="tip">Restore the router to factory default if you forget the password.<br/>
        Do as follows: Connect the router to the power supply. Press and hold the WIFI/RST button for about 10 seconds, and then release.</span></a></p>
      <div class="control-group none" id="passwrod-tip">
        <div class="control-label">Tips</div>
        <div class="controls controls-text" id="passwrod-tip-text"></div>
      </div>
      <div id="login-password-box" class="control-group password-box">
        <label class="control-label" for="login-password"></label>
        <div class="controls">
          <input type="password" id="login-password" name="Password" class="text" />
          <span id="password-placeholder">password</span> </div>
      </div>
      <div id="submitOperation" class="btn-group">
        <input type="button" value="Login" class="btn" onClick="preSubmit(document.Login)" />
      </div>
    </div>
  </form>
</div>
</body>
</html>
