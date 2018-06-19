<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="1; url=<%asp_error_redirect_url();%>">
  <title>Input Error</title>
</head>
<body>
<script>
var errMessage = "<%asp_error_message();%>"
  , errMessageMap = {
      "ERROR: WAN NET is same as LAN": "错误：WAN口IP与LAN口IP不能在同一网段！",
      "Check upgrade file error!": "请选择正确的升级文件！",
      "Password error": "你输入的旧密码错误！",
      "ERROR: login forbi": "登录人数已满4人！"
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