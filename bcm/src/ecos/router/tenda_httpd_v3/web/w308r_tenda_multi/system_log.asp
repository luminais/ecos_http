<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>System | System Log</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("system_tool");
var  syslog_list = "<%aspSysLogGet("system","system");%>";
var  cnt = "<%aspSysLogGet("count");%>";
var  curCnt = <%aspSysLogGet("curcount");%>-1;
var pageBtn = "";
cnt = cnt/10;
for(var i=0;i<cnt;i++)
{
	if(i == curCnt)
	{
		pageBtn += "<a href=# target=mainFrame class=content style=\"color:#FF6633\" onClick=\"onClickCurPage(" + eval(i+1) + ")\">[" + eval(i+1) + "]</a>";				
	}
	else
	{
		pageBtn += "<a href=# target=mainFrame class=content onClick=\"onClickCurPage(" + eval(i+1) + ")\">[" + eval(i+1) + "]</a>";				
	}
}	
function initTranslate(){
	var e=document.getElementById("Refresh_btn");
	e.value=_("Refresh");
	var e1=document.getElementById("Clear_btn");
	e1.value=_("Clear");
}
function print_systemlog()
{
	document.write(syslog_list);
}
function preClear(f) 
{
	if(cnt == 0)
	{
		refresh("system_log.asp");
		return ;
	}
   document.getElementById("TYPE").value = 0;
   f.submit() ;
}
function onClickCurPage(n)
{
	document.getElementById("curPage").value = n;
	document.getElementById("TYPE").value = 1;
	document.frmSetup.submit();
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" class="bg" onLoad="initTranslate();">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup method="POST" action=/goform/SysToolSysLog>
					<input type=hidden name=GO value="system_log.asp">
					<INPUT type=hidden id=TYPE name="TYPE">
					<input type="hidden" id="curPage" name="curPage">
				<table class=content2>
					<tr>
					<td width=10% align="center" valign="top"><script>document.write(_("the"));</script><font color="#FF6633" size="+1">
					  <script>document.write(curCnt+1);</script></font>&nbsp;&nbsp;<script>document.write(_("page log contents"));</script></td>
					</tr>
				</table>
				<table class=content1 border=1 cellspacing=0 style="margin-top:0px;" id="table1">
					<SCRIPT> 
					   print_systemlog(); 
					</SCRIPT>
				</table>
				<br>
				<table border="0" class="content1" style="margin-top:0px;">
				<tr><td align="right">
					<SCRIPT> 
					   document.write(pageBtn); 
					</SCRIPT>		
				</td></tr></table>
					<SCRIPT>
						var bb='<input id="Refresh_btn" class=button2 onclick=refresh("system_log.asp") onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'" value="" type=button>&nbsp;';
							bb+='<input id="Clear_btn" class=button2 onclick="preClear(document.frmSetup);" onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  value="" type=button>';
							bb+='<input name=page value=log type=hidden>';
							bb+='<input name=op value=0 type=hidden>';
						tbl_tail(bb);
					</SCRIPT>
					</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("system_log_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("system_log_Help_Inf2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






