<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta http-equiv="refresh" content="0; url=<%asp_error_redirect_url();%>">
  <title>Input Error</title>
</head>
<body>
  <script language="JavaScript">
    alert("<%asp_error_message();%>");
  </script>
</body>
</html>