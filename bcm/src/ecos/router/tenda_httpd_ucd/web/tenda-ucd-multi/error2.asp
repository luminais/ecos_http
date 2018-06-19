<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="0; url=<%asp_error_redirect_url();%>">
  <title>Input Error</title>
</head>
<body>
  <script>
    alert("<%asp_error_message();%>");
    window.open ("http://www.tenda.com.cn");
  </script>
</body>
</html>