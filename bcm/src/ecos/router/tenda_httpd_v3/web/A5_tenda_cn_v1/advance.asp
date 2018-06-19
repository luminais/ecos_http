<HTML xmlns="http://www.w3.org/1999/xhtml">
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>150M 便携无线路由器</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<link rel=stylesheet type=text/css href=style.css />
<SCRIPT language=JavaScript>
function changebg(heading)
{
 	for(var i=1; i<=7; i++)
	{
		var head = "menu" + i;
		if(heading == head)
		{
			 window.document.getElementById(heading).className = "class1";
		}
		else
			 window.document.getElementById(head).className = "class2";
	}


}

function changemenu(heading)
{
 	for(var i=1; i<=7; i++)
	{
		var head = "submenu" + i;
		if(heading == head)
		{
			 window.document.getElementById(heading).style.display = "";
		}
		else
			 window.document.getElementById(head).style.display = "none";
	}


}
function MenuClick(oEvent)
{
		//下面是为了让当前活动的链接显示绿色	
		var oEvent = oEvent || window.event   
		var oElem = oEvent.target || oEvent.srcElement; // 此做法是为了兼容FF浏览器  
		SearchUrlInRight(oElem);
		
		return;
}
function SearchUrlInRight(loc)
{
	var arrAll = document.getElementsByTagName('a');
	var nodetmp;
	//alert(loc);
	for(var i=0;i<arrAll.length;i++)
	{
		if(arrAll[i].tagName=="A")
		{
			arrAll[i].style.color=(navigator.appName.indexOf("Microsoft")==-1)?null:"";
			if(arrAll[i].href==loc){
				nodetmp=arrAll[i];
			}	
		}
	}
	if(nodetmp!=undefined)
		nodetmp.style.color="#3366ff";
}
</SCRIPT>
</HEAD>

<body>
<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
<tr>
<td height="26" valign="top">
<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr class="top">
    <td height="25" align="right" valign="bottom"><a href="index.asp">首页设置</a>&nbsp;<!--<a href=javascript:void(preLogout());>退出登录</a>--></td>
    <td width="10" align="right" valign="bottom">&nbsp;</td>
  </tr>
</table>
<div style="width:100%; height:1px; background-color:#c0c7cd; overflow:hidden; padding:0px; margin:0px;"></div>
<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td height="5"></td>
  </tr>
</table>

</td>
</tr>
<tr>
<td valign="top">
<table width="976" border="0" align="center" cellpadding="0" cellspacing="0" bgcolor="#e9e9e9" height="100%">
  <tr>
    <td valign="top">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33" height="89">&nbsp;</td>
        <td width="943" height="89"><img src="tendalogo.gif" width="300" height="49"></td>
      </tr>
      <tr>
        <td colspan="2">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
          <tr>
            <td width="33" rowspan="3">			</td>
            <td valign="top">
			<table width="679" border="0" cellpadding="0" cellspacing="0">
			  <tr>
				<td width="6" height="37" style="background-image:url(left.gif); background-repeat:no-repeat;"></td>
				<td width="667" align="left" valign="top" style=" background-image:url(middle.gif); background-repeat:repeat-x">
				<table width="100%" border="0" cellpadding="0" cellspacing="0">
				  <tr>
					<td height="37" align="center"><a href="system_status.asp" target="mainFrame" onClick=changebg("menu1");changemenu("submenu1");MenuClick(event);>高级设置</a></td>
					<td align="center"><a href="wireless_basic.asp" target="mainFrame" onClick=changebg("menu2");changemenu("submenu2");MenuClick(event);>无线设置</a></td>
					<td align="center"><a href="lan_dhcps.asp" target="mainFrame" onClick=changebg("menu3");changemenu("submenu3");MenuClick(event);>DHCP服务器</a></td>
					<td align="center"><a href="nat_virtualportseg.asp" target="mainFrame" onClick=changebg("menu4");changemenu("submenu4");MenuClick(event);>虚拟服务器</a></td>
					<td align="center"><a href="firewall_clientfilter.asp" target="mainFrame" onClick=changebg("menu5");changemenu("submenu5");MenuClick(event);>安全设置</a></td>
					<td align="center"><a href="routing_table.asp" target="mainFrame" onClick=changebg("menu6");changemenu("submenu6");MenuClick(event);>路由设置</a></td>
					<td align="center"><a href="system_hostname.asp" target="mainFrame" onClick=changebg("menu7");changemenu("submenu7");MenuClick(event);>系统工具</a></td>
				  </tr>
				</table></td>
				<td width="6" style="background-image:url(right.gif);"></td>
			  </tr>
			  <tr>
				<td height="7"></td>
				<td>	
				<table width="100%" border="0" cellpadding="0" cellspacing="0">
				  <tr>
					<td width="87" height="7" align="center" class="class1" id="menu1"></td>
					<td width="87" align="center" id="menu2"></td>
					<td width="115" align="center" id="menu3"></td>
					<td width="109" align="center" id="menu4"></td>
					<td width="90" align="center" id="menu5"></td>
					<td width="88" align="center" id="menu6"></td>
					<td width="91" align="center" id="menu7"></td>
				  </tr>
				</table></td>
				<td height="7">				</td>
			  </tr>
			</table>			</td>
          </tr>
          <tr>
            <td height="21" valign="top">
			
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu1" class="submenu" style="display:block;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="system_status.asp" style="color:#3366ff;" target="mainFrame" onclick=MenuClick(event);>系统状态</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wan_connected.asp" target="mainFrame" onclick=MenuClick(event);>WAN口设置</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="lan.asp" target="mainFrame" onclick=MenuClick(event);>LAN口设置</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="mac_clone.asp" target="mainFrame" onclick=MenuClick(event);>MAC克隆</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wan_dns.asp" target="mainFrame" onclick=MenuClick(event);>DNS设置</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="net_tc.asp" target="mainFrame" onclick=MenuClick(event);>带宽控制</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="sys_iptAccount.asp" target="mainFrame" onclick=MenuClick(event);>流量统计</a>&nbsp;&nbsp;&nbsp;&nbsp;<!--<a href="wan_speed.asp" target="mainFrame" onclick=MenuClick(event);>WAN速率控制--></td>
              </tr>
            </table>
			
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu2" class="submenu" style="display:none;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="wireless_basic.asp" target="mainFrame" onclick=MenuClick(event);>无线基本设置</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wireless_security.asp" target="mainFrame" onclick=MenuClick(event);>无线安全</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wireless_filter.asp" target="mainFrame" onclick=MenuClick(event);>访问控制</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wireless_state.asp" target="mainFrame" onclick=MenuClick(event);>连接状态</a></td>
              </tr>
            </table>
			
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu3" class="submenu" style="display:none;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="lan_dhcps.asp" target="mainFrame" onclick=MenuClick(event);>DHCP服务器</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="lan_dhcp_clients.asp" target="mainFrame" onclick=MenuClick(event);>DHCP客户列表</a></td>
              </tr>
            </table>
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu4" class="submenu" style="display:none;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="nat_virtualportseg.asp" target="mainFrame" onclick=MenuClick(event);>端口段映射</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="nat_dmz.asp" target="mainFrame" onclick=MenuClick(event);>DMZ主机</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="upnp_config.asp" target="mainFrame" onclick=MenuClick(event);>UPNP设置</a></td>
              </tr>
            </table>
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu5" class="submenu" style="display:none;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="firewall_clientfilter.asp" target="mainFrame" onclick=MenuClick(event);>客户端过滤</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="firewall_mac.asp" target="mainFrame" onclick=MenuClick(event);>MAC地址过滤</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="firewall_urlfilter.asp" target="mainFrame" onclick=MenuClick(event);>URL过滤</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_remote.asp" target="mainFrame" onclick=MenuClick(event);>远端WEB管理</a></td>
              </tr>
            </table>
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu6" class="submenu" style="display:none;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="routing_table.asp" target="mainFrame" onclick=MenuClick(event);>路由列表</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="routing_static.asp" target="mainFrame" onclick=MenuClick(event);>静态路由</a></td>
              </tr>
            </table>
			<table width="100%" height="21" border="0" align="center" cellpadding="0" cellspacing="0" id="submenu7" class="submenu" style="display:none;">
              <tr>
                <td height="21" valign="top">&nbsp;&nbsp;<a href="system_hostname.asp" target="mainFrame" onclick=MenuClick(event);>网络时间</a>&nbsp;&nbsp;<a href="ddns_config.asp" target="mainFrame" onclick=MenuClick(event);>DDNS</a>&nbsp;&nbsp;<a href="system_backup.asp" target="mainFrame" onclick=MenuClick(event);>备份/恢复设置</a>&nbsp;&nbsp;<a href="system_restore.asp" target="mainFrame" onclick=MenuClick(event);>恢复出厂设置</a>&nbsp;&nbsp;<a href="system_upgrade.asp" target="mainFrame" onclick=MenuClick(event);>升级</a>&nbsp;&nbsp;<a href="system_reboot.asp" target="mainFrame" onclick=MenuClick(event);>重启路由器</a>&nbsp;&nbsp;<a href="system_password.asp" target="mainFrame" onclick=MenuClick(event);>修改密码</a>&nbsp;&nbsp;<a href="system_log.asp" target="mainFrame" onclick=MenuClick(event);>系统日志</a></td>
              </tr>
            </table>			</td>
          </tr>
        </table>
		<table width="100%" border="0" cellpadding="0" cellspacing="0" style="background-image:url(bg2.jpg);">
		  <tr>
			<td height="5"></td>
		  </tr>
		</table>		</td>
      </tr>
    </table>
	</td>
  </tr>
  <tr>
    <td height="100%" valign="top">
	<iframe 
      style="Z-INDEX:4; VISIBILITY:inherit; WIDTH:100%; HEIGHT:100%" marginWidth=0 frameSpacing=0 marginHeight=0 
	  name="mainFrame" id="mainFrame"
	  src="system_status.asp" frameborder="0" noresize scrolling="auto">
	</iframe>

	</td>
  </tr>
</table>
</td>
</tr>
</table>
</body>
</html>






