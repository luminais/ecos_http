<HTML xmlns="http://www.w3.org/1999/xhtml">
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>150Mbps Travel Wireless Router</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<link rel=stylesheet type=text/css href=style.css />
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("system_tool");
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
		var oEvent = oEvent || window.event   
		var oElem = oEvent.target || oEvent.srcElement;
		SearchUrlInRight(oElem);
		return;
}
function SearchUrlInRight(loc)
{
	var arrAll = document.getElementsByTagName('a');
	var nodetmp;
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
    <td height="25" align="right" valign="bottom"><a href="index.asp"><script>document.write(_("Initial page settings"));</script></a><!--&nbsp;&nbsp;&nbsp;&nbsp;<a href=javascript:void(preLogout());><script>document.write(_("Log out"));</script></a>--></td>
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
        <td width="943" height="89"><img src="tendalogo.gif" width="250" height="49"></td>
      </tr>
      <tr>
        <td colspan="2">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
          <tr>
            <td width="33" rowspan="3">			</td>
            <td valign="top">
			<table width="679" border="0" cellpadding="0" cellspacing="0">
			  <tr>
			    <td width="7" style="background-image:url(left.gif); background-repeat:no-repeat;">				</td>
			    <td rowspan="2" valign="top" style="background-image:url(middle.gif); background-repeat:repeat;">
				<table width="100%" border="0" cellpadding="0" cellspacing="0" style="overflow:hidden;">
				  <tr>
					<td width="95" height="33" align="center"><a href="system_status.asp" target="mainFrame" onClick=changebg("menu1");changemenu("submenu1");MenuClick(event);>
					  <script>document.write(_("Advanced Settings"));</script></a></td>
					<td width="95" align="center"><a href="wireless_basic.asp" target="mainFrame" onClick=changebg("menu2");changemenu("submenu2");MenuClick(event);>
					  <script>document.write(_("Wireless settings"));</script></a></td>
					<td width="95" align="center"><a href="lan_dhcps.asp" target="mainFrame" onClick=changebg("menu3");changemenu("submenu3");MenuClick(event);>
					  <script>document.write(_("DHCP Server"));</script></a></td>
					<td width="95" align="center"><a href="nat_virtualportseg.asp" target="mainFrame" onClick=changebg("menu4");changemenu("submenu4");MenuClick(event);>
					  <script>document.write(_("Virtual Server"));</script></a></td>
					<td width="95" align="center"><a href="firewall_clientfilter.asp" target="mainFrame" onClick=changebg("menu5");changemenu("submenu5");MenuClick(event);>
					  <script>document.write(_("Security settings"));</script></a></td>
					<td width="95" align="center"><a href="routing_table.asp" target="mainFrame" onClick=changebg("menu6");changemenu("submenu6");MenuClick(event);>
					  <script>document.write(_("Routing settings"));</script></a></td>
					<td width="95" align="center"><a href="system_hostname.asp" target="mainFrame" onClick=changebg("menu7");changemenu("submenu7");MenuClick(event);>
					  <script>document.write(_("System Tools"));</script></a></td>
				  </tr>
				</table>
				</td>
			    <td width="7" style="background-image:url(right.gif); background-repeat:no-repeat">&nbsp;</td>
			  </tr>
			  <tr>
			    <td width="7" style="background-image:url(middle.gif); background-repeat:repeat;">&nbsp;</td>
			    <td width="7" style="background-image:url(middle.gif); background-repeat:repeat;">&nbsp;</td>
			  </tr>
			  <tr>
			   <td colspan="3" height="4" style=" background-image:url(gredline.gif); background-repeat:no-repeat;"></td>
			  </tr>
			  <tr>
				<td height="7"></td>
				<td width="667">	
				<table width="100%" border="0" cellpadding="0" cellspacing="0">
				  <tr>
					<td width="95" height="7" align="center" class="class1" id="menu1"></td>
					<td width="95" align="center" id="menu2"></td>
					<td width="95" align="center" id="menu3"></td>
					<td width="95" align="center" id="menu4"></td>
					<td width="95" align="center" id="menu5"></td>
					<td width="95" align="center" id="menu6"></td>
					<td width="95" align="center" id="menu7"></td>
				  </tr>
				</table></td>
				<td width="7" height="7">				</td>
			  </tr>
			</table>			</td>
          </tr>
          <tr>
            <td valign="top">	
			<table width="679" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu1" class="submenu" style="display:block; padding-left:10px; padding-right:5px;">
              <tr>
                <td valign="top">
                <a href="system_status.asp" style="color:#3366ff;" target="mainFrame" onclick=MenuClick(event);>
                  <script>document.write(_("System Status"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="wan_connected.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("WAN settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="lan.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("LAN settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="mac_clone.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("MAC Clone"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="wan_dns.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("DNS settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="wireless_mode.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("WAN medium type"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="net_tc.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Bandwidth control"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;
                  <a href="sys_iptAccount.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Traffic statistics"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wan_speed.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("WAN speed"));</script></a></td>
              </tr>
            </table>	
			<table width="679" height="21" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu2" class="submenu" style="display:none; padding-left:10px; padding-right:5px;">
              <tr>
                <td height="21" valign="top"><a href="wireless_basic.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Wireless Basic Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wireless_security.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Wireless Security"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wireless_filter.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Access Control"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="wireless_state.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Connection Status"));</script></a></td>
              </tr>
            </table>
			<table width="679" height="21" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu3" class="submenu" style="display:none; padding-left:10px; padding-right:5px;">
              <tr>
                <td height="21" valign="top"><a href="lan_dhcps.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("DHCP Server"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="lan_dhcp_clients.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("DHCP Client list"));</script></a></td>
              </tr>
            </table>
			<table width="679" height="21" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu4" class="submenu" style="display:none; padding-left:10px; padding-right:5px;">
              <tr>
                <td height="21" valign="top"><a href="nat_virtualportseg.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Port Range Forwarding"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="nat_dmz.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("DMZ Host"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="upnp_config.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("UPNP settings"));</script></a></td>
              </tr>
            </table>
			<table width="679" height="21" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu5" class="submenu" style="display:none; padding-left:10px; padding-right:5px;">
              <tr>
                <td height="21" valign="top"><a href="firewall_clientfilter.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Client Filter Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="firewall_mac.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("MAC Address Filter Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="firewall_urlfilter.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("URL Filter Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_remote.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Remote Web Management"));</script></a></td>
              </tr>
            </table>
			<table width="679" height="21" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu6" class="submenu" style="display:none; padding-left:10px; padding-right:5px;">
              <tr>
                <td height="21" valign="top"><a href="routing_table.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Routing table"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="routing_static.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Static routing"));</script></a></td>
              </tr>
            </table>
			<table width="679" height="21" border="0" align="left" cellpadding="0" cellspacing="0" id="submenu7" class="submenu" style="display:none; padding-left:10px; padding-right:5px;">
              <tr>
                <td height="21" valign="top"><a href="system_hostname.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Time Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="ddns_config.asp" target="mainFrame" onclick=MenuClick(event);>DDNS</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_backup.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Backup/Restore Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_restore.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Restore to Factory Default Settings"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_upgrade.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Upgrade"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_reboot.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Reboot the router"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_password.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Change password1"));</script></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="system_log.asp" target="mainFrame" onclick=MenuClick(event);><script>document.write(_("Syslog"));</script></a></td>
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
    </table>	</td>
  </tr>
  <tr>
    <td height="100%" valign="top"><iframe 
      style="Z-INDEX:4; VISIBILITY:inherit; WIDTH:100%; HEIGHT:100%" marginwidth=0 framespacing=0 marginheight=0 
	  name="mainFrame" id="mainFrame"
	  src="system_status.asp" frameborder="0" noresize scrolling="auto"> </iframe></td>
  </tr>
</table>
</td>
</tr>
</table>
</body>
</html>







