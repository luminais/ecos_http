<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>Routing | Routing Table</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("index_routing_virtual");
var rte=new Array(<%mGetRouteTable("sys","0");%>);
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
						<th><script>document.write(_("Destination IP"));</script></th>
						<th><script>document.write(_("Subnet mask"));</script></th>
						<th><script>document.write(_("Gateway"));</script></th>
						<th><script>document.write(_("Hops"));</script></th>
						<th><script>document.write(_("Interface"));</script></th>
					</tr>
					<script>
						showRTE();
					</script>
				</table>
					<SCRIPT>
						var m='<input class=button2 value='+_("Refresh")+' onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  type=button onclick=refresh("routing_table.asp")>';
						tbl_tail(m);
					</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("routing_table_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("routing_table_Help_Inf2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






