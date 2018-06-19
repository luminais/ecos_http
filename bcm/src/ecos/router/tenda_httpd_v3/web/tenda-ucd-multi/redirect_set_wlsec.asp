<!DOCTYPE html>
<html class="index-html">
<head>
<meta charset="utf-8" />
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="cache-control" content="no-cache,must-revalidate">
<title>TENDA 11N Wireless Router</title>
<link rel="stylesheet" type="text/css" href="css/reasy-1.0.0.css">

<!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
<!--[if lt IE 9]>
<script src="js/libs/html5shiv.js"></script>
<![endif]-->
<script src="lang/b28n.js"></script>
<script src="js/libs/reasy-1.0.0.js"></script>
<script src="js/libs/reasy-ui-1.0.0.js"></script>
<script src="js/common.js?v=6.0.0"></script>
<script>
    var g = {};

    g.ssid0 =  "<%get_wireless_basic("SSID");%>",
    g.def_wirelesspassword = "<%get_wireless_password("wireless","wirelesspassword");%>";

    function init() {
      var locationHref = location.href.split("?"),
        usernameElem = document.getElementById('wireless-username'),
        passwordElem = document.getElementById('wpa_aeskey_value');

      usernameElem.value = decodeSSID(g.ssid0);

      if (locationHref[1] === "1") {
        passwordElem.value = g.def_wirelesspassword;
        g.dialog = $.dialog({
          title: _('Message from webpage'),
          model: 'message',
          content: '<p class="text-error">' + _('Please close the browser and connect to the WiFi again.') + '</p>' +
            '<p>' + _('WiFi Name (SSID):') + decodeSSID(g.ssid0) +
            '<p>' + _('WiFi Password:') + g.def_wirelesspassword
        });
      }

      $("#wpa_aeskey_value").addPlaceholder(_('The password cannot be less than 8 characters.'), true);
    }

    function selChange() {
      document.getElementById("div_wpa").style.visibility = "visible";
      document.getElementById("div_wpa").style.display = "block";
    }

    function checkData() {
      var keyvalue = document.security_form.wpa_aeskey_value.value;

        if (keyvalue.length == 0 ||
				keyvalue === _('The password cannot be less than 8 characters.')) {
          alert(_('Please enter the WIFI password!'));
          return false;
        }

        if ((keyvalue.length < 8) || (keyvalue.length > 63)) {
          alert(_('WiFi Password Range: 8~63 characters!'));
          return false;
        }

      return true;
    }

	function shuDown() {
		g.dialog = $.dialog({
			 title: _('Message from webpage'),
			 model: 'message',
			 content: '<p class="f14 text-error">' + _('You cannot browse the webpage properly unless rebooting the browser.')+'</p>'
		 });
	}

	function preSubmit() {
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
      return false;
    }
</script>
</head>
<body onLoad="init()" class="redirect-body">
<header class="masthead">
  <div class="container head-inner"> <a class="brand" href="#" title="Tenda"></a>
    <div class="easy-logo"></div>
  </div>
  <div class="footer"></div>
</header>
<div class="container">
  <form method="post" class="login" name="security_form" action="/goform/redirectSetwlSecurity" onsubmit="preSubmit()">
    <h1 class="login-title" align="center">For your wireless network security, suggest setting up a WiFi password.</h1>
    <div class="container">
      <div class="control-group" id="div_wpa">
        <label class="control-label" >WiFi Name (SSID)</label>
        <div class="controls">
          <input class="text input-large" type="text" name="ssid1" id="wireless-username">
        </div>
      </div>
      <div class="control-group" id="div_wpa">
        <label class="control-label" >WiFi Password</label>
        <div class="controls">
          <input class="text input-large" type="text" name="wpa_aeskey_value" id="wpa_aeskey_value">
        </div>
      </div>
      <div class="control-group none">
        <label class="control-label" ></label>
        <div class="controls">
          <input type="hidden" name="prompt_radio" value="0">
          <label class="checkbox">
            <input type="checkbox" id="prompt_checkbox" value="1">
            Do not remind me any more.</label>
        </div>
      </div>
      <div class="btn-group">
        <input type="button" name="button_save" value="Save" class="btn" onClick="preSubmit()">
        <input type="button" name="button_cancel" value="Close" class="btn" onClick="shuDown()">
        <input type="hidden" name="btn_loc_select" value="0">
      </div>
    </div>
  </form>
</div>
</body>
</html>
