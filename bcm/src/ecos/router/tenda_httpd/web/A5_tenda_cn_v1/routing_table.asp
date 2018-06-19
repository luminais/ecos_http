<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
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
						<th>目的IP</th>
						<th>子网掩码</th>
						<th>网关</th>
						<th>跳跃数</th>
						<th>接口</th>
					</tr>
				
					<script>
						showRTE();
					</script>
				</table>
					<SCRIPT>
						var m='<input class=button2 value="刷 新" onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  type=button onclick=refresh("routing_table.asp")>';
						tbl_tail(m);
					</SCRIPT>
				
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('跳跃数：接口跃点数。接口：有三种类型，vlan2:WAN口接口，ppp0:PPPoE接口，br0:内网设备接口。'
		)</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>





