<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="0; url=<%asp_error_redirect_url();%>">
  <title>Input Error</title>
</head>
<body>
	<script src="lang/b28n.js" type="text/javascript"></script>
	<script>
	//handle translate
	(function() {
		B.setTextDomain("base");
		B.translate();
	})();
	</script>
  <script language="JavaScript">
  	var errMessage = "<%asp_error_message();%>";
  	if(errMessage == "ERROR: WAN NET is same as LAN")
    	alert(_("wan ip error message"));
	else if(errMessage == "Check upgrade file error!")
		alert(_("Please specify a valid firmware file for upgrade!"));
	else 
		alert(errMessage);
  </script>
</body>
</html>