<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>Routing | Routing Table</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

var rte=new Array(<%mGetRouteTable("sys","0");%>);//'ip,submask,gateway,metric,inteface',、、、、、、、、、、、、、、

function showRTE()
{
	for (var i=0; i<rte.length;i++)
	{
		var s=rte[i].split(",");
		if (s.length!=5) break;
    var  m='<tr align=center>';
        m+='<td>'+s[0]+'</td>';
		m+='<td>'+s[1]+'</td>';
		m+='<td>'+s[2]+'</td>';
		m+='<td>'+s[3]+'</td>';
		m+='<td>'+s[4]+'</td>';
        m+='</tr>';
		document.write(m);
	}
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
				<form name=frmSetup>
				
				<table class=content1 border=1 cellspacing=0 id="table1">
					<tr align=center>
						<th>目的地IP位址</th>
						<th>子網路遮罩</th>
						<th>預設閘道</th>
						<th>節點數</th>
						<th>連接口</th>
					</tr>
				
					<script>
						showRTE();
					</script>
				</table>
					<SCRIPT>
						var m='<input class=button2 value="重新整理" onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  type=button onclick=refresh("routing_table.asp")>';
						tbl_tail(m);
					</SCRIPT>
				
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>
		helpInfo('節點數：連接口節點數。<br>&nbsp;&nbsp;&nbsp;&nbsp;連接口：有三種類型，vlan2:WAN端連接口、ppp0:PPPoE連接口、br0:區域網路設備連接口。');
		</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

