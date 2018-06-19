<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>Routing | Static Routing</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
//192.168.5.0:255.255.255.0:192.168.5.1:1 192.168.6.0:255.255.255.0:192.168.6.1:1
var list = "<%aspTendaGetStatus("wan","wan0_route");%>";
var resList;
var max = 5;

function init(f){
	resList = list.split(" ");
	showRT();
}

function showRT()
{
	var i;
	
	var m='<table border=1 cellspacing=0 class="content1" id=showTab>';
	for (i=0;i<resList.length;i++) {
		var rt=resList[i];
		var s=rt.split(":");
		if (s.length!=4) continue;

		m+='<tr align=center>';
		m+='<td>'+s[0];
		m+='</td>';

		m+='<td>'+s[1];
		m+='</td>';

		m+='<td>'+s[2];
		m+='</td>';

		m+='<td>';
		m+='<input class=button2 type=button value=" 删 除 " onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  onClick=delRte(this,' + i +  ')></td>';
		m+='</tr>';
	}
	m+='</table>';
	document.getElementById("routeList").innerHTML = m;
}

function delRte(obj,dex)
{
	document.getElementById("showTab").deleteRow(dex-1);
	var i=0;
	for(i=dex;i<resList.length;i++)
	{
		resList[i] = resList[eval(i+1)];
	}
	resList.length--;

	showRT();
}

function addRte(f) {
	var info;
	if (!verifyIP2(f.ip,"目的地址 ",1)) return ;
	if (!ipMskChk(f.nm,"子网掩码")) return ;
	if (!verifyIP2(f.gw,"网关")) return ;
	f.ip.value = clearInvalidIpstr(f.ip.value);
	f.nm.value = clearInvalidIpstr(f.nm.value);
	f.gw.value = clearInvalidIpstr(f.gw.value);
	
	info = f.ip.value+':'+f.nm.value+':'+f.gw.value+':'+'1';

	  if(resList.length >= max+1)
	    {
	        window.alert("静态路由最多能配置"+max+"条.");
		 	return;
	    }
	
	    for(var i=0; i<resList.length; i++)
	    {
			if(resList[i] == info)
			{
				window.alert("指定的静态路由已经存在.");
				return;
			}
	    }
		

	resList[resList.length]=info;

	showRT();
}

function preSubmit(f) 
{
	var loc = "/goform/RouteStatic?GO=routing_static.asp";
	loc += "&wan0_route=";
	
	for(var j=0; j<resList.length; j++)
	{
		loc +=  resList[j] +" ";
	}

	var code = 'location="' + loc + '"';
	eval(code);
}

</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);table_onload('showTab');" class="bg">
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
				<form name=frmSetup method=POST action=/goform/RouteStatic>
				<INPUT type=hidden name=GO value=routing_static.asp>
					<input type=hidden name=cmd>
					<table border=0 class=content1 cellspacing=0 id="table1">
						<tr class=item1 align=center height=30>
						  <th>目的网络IP</th>
						  <th>子网掩码</th> 
						  <th>网关</th>
						  <th>操作</th>
						</tr>
						<tr align=center>
						  <td nowrap>
							<input name=ip size=15 maxlength=15>         
						  </td>
						  <td nowrap>
							<input name=nm size=15 maxlength=15>
						  </td>
						  <td nowrap>
							<input name=gw size=15 maxlength=15>
						  </td>
						  <td><input class=button2 type="button" name="staticAdd" value=" &lt;&lt;添加"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  onClick=addRte(this.form)></td>
						</tr>
				
					</table>
					
					<div id="routeList" style="position:relative;visibility:visible;"></div> 
					
					<SCRIPT>
						tbl_tail_save("document.frmSetup");
					</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('静态路由：在网络中安装多个路由器时，您需要设置静态路由。静态路由功能确定数据沿网络前后通过路由器的路径。您可使用静态路由允许不同的 IP 域名用户通过此设备访问互联网。 请谨慎设置。\ 在许多条件下最好使用动态路由，因为此功能允许路由器自动检测网络布局的物理变化。如果要使用静态路由，路由器的 DHCP 设定必须关闭。设置需要重新启动有才能生效。'
		);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');	  
    </script>
</BODY>
</HTML>





