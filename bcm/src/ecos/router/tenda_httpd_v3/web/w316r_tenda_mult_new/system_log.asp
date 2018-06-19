<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>System | System Log</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script src="lang/b28n.js" type="text/javascript"></script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
//handle translate
B.setTextDomain(["base", "system_tool"]);

var syslog_list = "<%aspSysLogGet("system","system");%>",
	cnt = "<%aspSysLogGet("count");%>"  / 10,
	curCnt = <%aspSysLogGet("curcount");%> - 1,
	pageBtn = "";

for(var i = 0; i < cnt; i++) {
	if(i == curCnt){
		pageBtn += "<a href=#  style=\"color:#FF6633\" onClick=\"onClickCurPage(event, " + (i+1) + ")\">[" + (i+1) + "]</a>";				
	} else {
		pageBtn += "<a href=# onClick=\"onClickCurPage(event, " + (i+1) + ")\">[" + (i+1) + "]</a>";				
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

function onClickCurPage(e, n){
/*	var e = e || window.event;
	
	if (e.preventDefault) { 
		e.preventDefault();
	} else {
		e.returnValue = false;
	}*/
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
		<legend>Syslog</legend>
		<table class=content1>
			<tr>
			<td width=10% align="center" valign="top">Logs in page <font color="#FF6633" size="+1">
			  <script>document.write(curCnt+1);</script></font></td>
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
		<div class="btn-group">
			<input class="btn" onClick="refresh('system_log.asp')" value="Refresh" type="button" />			
			<input class="btn" onClick="preClear(document.frmSetup);" value="Clear" type="button" />
			<input name=page value=log type=hidden>
			<input name=op value=0 type=hidden>
		</div>
	</fieldset>
</form>
<script>
	//handle translate
	B.translate();
</script>
</body>
</html>