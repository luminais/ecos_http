<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="1; url=<%asp_error_redirect_url();%>">
  <title>Input Error</title>
</head>
<body>
<script src="lang/b28n.js"></script>
<script>
var errMessage = "<%asp_error_message();%>"
  , errMessageMap = {
      "ERROR: WAN NET is same as LAN": _('Error! The WAN IP address cannot be in the same net segment as the LAN IP address.'),
      "Check upgrade file error!": _('Please select a correct upgrade file!'),
      "Password error": _('The old password is incorrect.'),
      "ERROR: login forbi": _('Login devices is up to 4!')
    };
if(errMessage == "Get upgrade file failed!" || errMessage == "Check upgrade file error!") {
	top.location.href ='/direct_reboot.asp?122=1';
} else if(errMessageMap[errMessage]) {
	alert(errMessageMap[errMessage]);
} else {
	alert(errMessage);
}
</script>
</body>
</html>