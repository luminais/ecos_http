<!DOCTYPE html>
<html class="index-html">
<head>
	<meta charset="utf-8" />
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache,must-revalidate">
	<title>TENDA 11N无线路由器</title>
	<link rel="stylesheet" type="text/css" href="css/reasy-1.0.0.css">
	<style>
		.login{
			width:445px;
			height:226px;
		}
		.login .control-label{
			font-weight:normal;
		}
		
		.redirect-body .login-title{
			font-size:14px;
		}
	</style>
  
  <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
  <!--[if lt IE 9]>
    <script src="js/libs/html5shiv.js"></script>
  <![endif]-->

	<script src="js/libs/reasy-1.0.0.js"></script>
	<script src="js/libs/reasy-ui-1.0.0.js"></script>
	<script src="js/common.js?v=6.0.0"></script>
</head>
<body onLoad="init()" class="redirect-body">

	<header class="masthead">
		<div class="container head-inner">
			<a class="brand" href="#" title="Tenda"></a>
		<div class="easy-logo"></div>
		</div>
		<div class="footer"></div>
	</header>
	
	<div class="container">
	  <form method="post" class="login" name="security_form" action="/goform/redirectSetwlSecurity" onsubmit="preSubmit()">
		<h1 class="login-title" align="center">为保障无线网络安全，建议您设置无线密码。</h1>
		<div class="container">
		<div class="control-group" id="div_wpa">
			<label class="control-label" >无线信号名称</label>
			<div class="controls">
		      <input class="text input-large validatebox" type="text" name="ssid1" id="wireless-username" required="required" maxLength="32" />
		    </div>
		</div>
		<div class="control-group" id="div_wpa">
			<label class="control-label" >无线密码</label> 
			<div class="controls">
			<input class="text input-large validatebox" type="text" name="wpa_aeskey_value" id="wpa_aeskey_value" data-options='{"type":"ssidPasword","args":[8, 63]}'>
			</div>
		</div>
		<div class="control-group none">
		<label class="control-label" ></label> 
		<div class="controls">
		  <input type="hidden" name="prompt_radio" value="0">
		  <label class="checkbox"><input type="checkbox" id="prompt_checkbox" value="1">不再提示</label>        
		</div>
		</div>
		<div class="control-group">
			<div class="controls" style="margin-top:10px;">
		  <input type="button" name="button_save" value="保 存" class="btn" onClick="preSubmit()">
		  <input type="button" name="button_cancel" value="关 闭" class="btn" onClick="shuDown()">
		  <input type="hidden" name="btn_loc_select" value="0">
		  </div>
		</div>
		</div>
		</form>
	</div>
</body>
	<script language="javascript">
    var g = {};
	
	g.ssid0 = "<%get_wireless_basic("SSID");%>";
	g.def_wirelesspassword = "<%get_wireless_password("wireless","wirelesspassword");%>";

    function init() {
      var locationHref = location.href.split("?"),
        usernameElem = document.getElementById('wireless-username'),
        passwordElem = document.getElementById('wpa_aeskey_value');
       
      usernameElem.value = decodeSSID(g.ssid0);

      if (locationHref[1] === "1") {
        passwordElem.value = g.def_wirelesspassword;
        g.dialog = $.dialog({
          title: '来自网页的消息',
          model: 'message',
          content: '<p class="text-error">请关闭浏览器，并重新连接无线网络。</p>' +
            '<p>无线信号名称：' + decodeSSID(g.ssid0) +
            '<p>无线密码：' + g.def_wirelesspassword +
			'<div class="btn-group">'+
			'<input type="button" value="确定" class="btn" onClick="goHome()">'+
			'</div>'
        });
      }

      $("#wpa_aeskey_value").addPlaceholder("密码位数不得少于8位", true);
    }
	
	function goHome() {
		window.location.href="/index.htm";
	}

    function selChange() {
      document.getElementById("div_wpa").style.visibility = "visible";
      document.getElementById("div_wpa").style.display = "block";
    }

    function checkData() {
      var keyvalue = document.security_form.wpa_aeskey_value.value;
      
        if (keyvalue.length == 0 || keyvalue === '密码位数不得少于8位') {
          alert('请输入无线密码!');
          return false;
        }
        
        if ((keyvalue.length < 8) || (keyvalue.length > 63)) {
          alert('无线密码范围: 8~63 个字符!');
          return false;
        }
        
      return true;
    }
	
	function shuDown() {
		g.dialog = $.dialog({
			 title: '来自网页的消息',
			 model: 'message',
			 content: '<p class="f14 text-error">您必须重启浏览器才能正常浏览</p>'
		 });
	}
    
	function preSubmit() {
		g.validate.checkAll();
	}
	
	g.validate = $.validate({
		custom: function() {
            var ssidVal = document.getElementById('wireless-username').value;
            if(!(/^[ -~]+$/g).test(ssidVal) && ssidVal.length > 10) {
                    document.getElementById('wireless-username').focus();
                    return '当无线信号名称中含有中文字符时，最长只能输入10个字符';
            }
        },
		success: function() {
			var noPrompt = document.getElementById('prompt_checkbox'),
				promptSubmit = document.security_form.prompt_radio;
				  
			if (checkData() == true) {
				if (noPrompt.checked) {
					promptSubmit.value = '1';
				} else {
					promptSubmit.value = '0';
				}
				document.security_form.submit();
			}
			//return false;		
		}, 
		error: function(msg) {
			if (msg) {
				alert(msg);
			}
		}
	});
      
	</script>

</html>
