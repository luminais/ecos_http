<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>System | System Log</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

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

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" class="bg">
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
					<td width=10% align="center" valign="top">第<font color="#FF6633" size="+1">
					  <script>document.write(curCnt+1);</script></font>頁 日誌內容</td>
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
						var bb='<input class=button2 onclick=refresh("system_log.asp") onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'" value="重新整理" type=button>&nbsp;';
							bb+='<input class=button2 onclick="preClear(document.frmSetup);" onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  value="清除系統日誌" type=button>';
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
		<script>helpInfo('查詢歷史操作紀錄<br>&nbsp;&nbsp;&nbsp;&nbsp;提醒您：如果日誌內容滿15頁(意即150條紀錄)將會自動清除內容。'
		);</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

