<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | System Log</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script language=JavaScript>
var syslog_list = "<%aspSysLogGet("system","system");%>",
	cnt = "<%aspSysLogGet("count");%>",
	curCnt = <%aspSysLogGet("curcount");%> - 1,
	pageBtn = "";
cnt = cnt / 10;
for(var i=0;i<cnt;i++) {
	if(i == curCnt){
		pageBtn += "<a href=# target=mainFrame style=\"color:#FF6633\" onClick=\"onClickCurPage(" + (i+1) + ")\">[" + (i+1) + "]</a>";				
	} else {
		pageBtn += "<a href=# target=mainFrame onClick=\"onClickCurPage(" + (i+1) + ")\">[" + (i+1) + "]</a>";				
	}
}
	
function print_systemlog() {
	document.write(syslog_list);
}

function preClear(f) {
	if(cnt == 0){
		refresh("system_log.asp");
		return ;
	}
   document.getElementById("TYPE").value = 0;
   f.submit() ;
}

function onClickCurPage(n){
	document.getElementById("curPage").value = n;
	document.getElementById("TYPE").value = 1;
	document.frmSetup.submit();
}
</script>
</head>
<body class="bg">
<form name="frmSetup" method="POST" action=/goform/SysToolSysLog>
    <input type="hidden" name=GO value="system_log.asp">
    <input type="hidden" id="TYPE" name="TYPE">
    <input type="hidden" id="curPage" name="curPage">
	<fieldset>
		<legend>系统日志</legend>
		<table class=content1>
			<tr>
			<td width=10% align="center" valign="top">第<font color="#FF6633" size="+1">
			  <script>document.write(curCnt+1);</script></font>页  日 志 内 容</td>
			</tr>
		</table>
		<table class="table" id="table1">
			<script> 
			   print_systemlog(); 
			</script>
		</table>
		<br>
		<table class="content2">
			<tr><td align="right">
				<script> 
				   document.write(pageBtn); 
				</script>		
			</td></tr>
		</table>
		<script>
			var bb='<input class="btn" onclick=refresh("system_log.asp") value="刷 新" type=button>&nbsp;';
			bb+='<input class="btn" onclick="preClear(document.frmSetup);" value="清除日志" type=button>';
			bb+='<input name=page value=log type=hidden>';
			bb+='<input name=op value=0 type=hidden>';
			tbl_tail(bb);
		</script>
	</fieldset>
</form>
</body>
</html>