<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>LAN | DHCP Client List</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language="JavaScript">
def_DHS = "<%aspTendaGetStatus("lan","dhcps_start");%>";
def_DHE = "<%aspTendaGetStatus("lan","dhcps_end");%>";
def_LEASE = "<%aspTendaGetStatus("lan","lease_time");%>";
addCfg("DHS",1,def_DHS);
addCfg("DHE",3,def_DHE);
var dhcpList=new Array(<%TendaGetDhcpClients("list");%>);
var StaticList = new Array(<%TendaGetDhcpClients("staticlist");%>);
var ipmaceninit = "<%TendaGetDhcpClients("dhcpipmacbind");%>";
def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>";
addCfg("LANIP",0,def_LANIP);
ctime=0;
Butterlate.setTextDomain("dhcp_firewall");
var LANIP=getCfg("LANIP");
var netip=LANIP.replace(/\.\d{1,3}$/,".");
var dhs=getCfg("DHS").match(/\d{1,3}$/);
var dhe=getCfg("DHE").match(/\d{1,3}$/);
function initTranslate(){
	var e=document.getElementById("Add");
	e.value=_("Add");
	var e=document.getElementById("Refresh");
	e.value=_("Refresh");
}
function showList()
{
	var m='<table class=content1 border=1 style="margin-top:0px;" cellpadding="0" cellspacing="0">';
	m+='<tr class=item1 align=center height=30>';
	m+='<th nowrap>'+_("Host Name")+'</th>';
	m+='<th nowrap>'+_("IP Address")+'</th>';
	m+='<th nowrap>'+_("MAC address")+'</th>';
	m+='<th nowrap>'+_("Lease Time")+'</th>';
	m+='</tr>';
	for (i=0;i<dhcpList.length;i++) {
		var s=dhcpList[i].split(";");
		m+='<tr class=value1 align=center>';
		if(s[0] == "")
			m+='<td>'+"&nbsp;"+'</td>';
		else
			m+='<td>'+s[0]+'</td>';
		m+='<td>'+s[1]+'</td>';
		m+='<td>'+s[2]+'</td>';
		m+='<td>'+timeStr(s[4]-ctime)+'</td>';
		m+='</tr>';
	}
	document.getElementById("dhcplist").innerHTML = m;
}
function showStaticList()
{
	var m='<table class=content1 border=1 id=staticTab cellpadding="0" cellspacing="0">';
	m+='<tr class=item1 align=center height=30>';
	m+='<th nowrap>'+_("NO.")+'</th>';
	m+='<th nowrap>'+_("IP Address")+'</th>';
	m+='<th nowrap>'+_("MAC address")+'</th>';
	//m+='<th nowrap>'+_("IP-MAC Binding")+'</th>';
	m+='<th nowrap>'+_("Delete")+'</th>';
	m+='</tr>';
	for (i=0;i<StaticList.length;i++)
	{
		var s=StaticList[i].split(";");
		if (s.length <4) break;		
		m+='<tr class=value1 align=center>';
		m+='<td>'+eval(i+1)+'</td>';
		m+='<td>'+s[1]+'</td>';
		m+='<td>'+s[2]+ '</td>';
		
	
		//if( parseInt(ipmaceninit.charAt(i)) )
		//if(s[3] == "1")//modify by stanley
		//	m+='<td ><input type="checkbox" id="' + ("en" + eval(i+1)) +'" checked ></td>';
		//else
		//	m+='<td ><input type="checkbox" id="' + ("en" + eval(i+1)) +'" ></td>';
		m+='<td><input type=button class=button  onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'" value="'+_("Delete")+'" onclick="OnDel(this,' + i +  ')"></td>';
		m+='</tr>';
	}
	document.getElementById("staticlist").innerHTML = m;
}
function OnDel(obj,dex)
{
	document.getElementById("staticTab").deleteRow(dex+1);
	var i=0;
	var box;
	for(i=dex;i<StaticList.length;i++)
	{
		StaticList[i] = StaticList[eval(i+1)];
		if(i != StaticList.length -1)
		{
			//box = document.getElementById("en"+(i+2));
			//box.id = "en"+(i+1);
		}
	}
	StaticList.length--;
	showStaticList();
}
function init(){
    initTranslate();
	for(i=0; i<6; i++)
		document.frmSetup.elements['MAC'][i].value = "";
	document.frmSetup.elements['IP'].value = '';
	showList();
	showStaticList();
}
function preSubmit(f) {
	var loc = "/goform/DhcpListClient?GO=lan_dhcp_clients.asp";
	var ipmacen = "";
	var ipmac_enable=1;
	var ipmac_disable=2;
	var s;
	for (var i=0;i<StaticList.length;i++) {
		s=StaticList[i].split(";");
		//if(document.getElementById("en"+(i+1)).checked){
			ipmacen += "1";
			StaticList[i] = (s[0]+";"+s[1]+";"+s[2]+";"+ipmac_enable+";"+s[4]);
		//}else{
		//	StaticList[i] = (s[0]+";"+s[1]+";"+s[2]+";"+ipmac_disable+";"+s[4]);
		//	ipmacen += "0";
		//}	
		
		loc += "&list" + eval(i+1) + "=" + StaticList[i];
	}
	loc += "&IpMacEN=" + ipmacen;
	loc += "&LISTLEN=" + StaticList.length;
	var code = 'location="' + loc + '"';
    eval(code);
}
function add_static(ip, mac)
{
	var f=document.frmSetup;
	var hostname="";
	var flag = "1";
	ip.value = clearInvalidIpstr(ip.value);
	f.staticIpAddress.value = netip+ip.value;
	if(StaticList.length >15)
	{
		alert(_("Max.16 static IP-MAC binding items"));
		return ;
	}
	if (!verifyIP2(f.staticIpAddress,_("IP Address"))) return ;
	if (!macsCheck(mac,_("MAC address"))) return ;
	if(!ckMacReserve(mac))return ;
	var all=StaticList.toString()+LANIP+';';
	if (all.indexOf(netip+ip.value+';') >=0) { alert(_("NO repeated IP!")); return; }
	for (var k=0;k<StaticList.length;k++) {
		if (StaticList[k].toString().indexOf(mac) >=0) {
			if (!confirm(_("No repeated MAC address!"))) return ;
			rmEntry(StaticList, k);
		}
	}
	StaticList[StaticList.length]=(hostname+';'+netip+ip.value+';'+mac+';'+flag+';'+def_LEASE);
	showStaticList();
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init();" class="bg">
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
				<form name="frmSetup" id="frmSetup" method="POST" action="/goform/DhcpListClient">
					<INPUT type=hidden name=staticIpAddress value="">
					<table cellpadding="0" cellspacing="0" class="content1">
							<tr>
								<td colspan="3" bgcolor="#9D9D9D">&nbsp;&nbsp;<b> <script>document.write(_("Static assignment"));</script></b></td>
							</tr>
					</table>
					<div style="width:80%; height:1px; background-color:#c0c7cd; overflow:hidden; padding:0px; margin-top:1px;"></div>
					<table cellpadding="0" cellspacing="0" class="content3" id="table1">
							<tr>
							  <td width="100" align="right" class="item1"><script>document.write(_("IP Address"));</script></td>
								<td class="value1" nowrap colspan="2">
									&nbsp;&nbsp;&nbsp;&nbsp;<SCRIPT>		
										document.write(netip) ;
									</SCRIPT>
									<input id="IP" class="text" size="3" maxlength="3">
								</td>
							</tr>
							<tr>
							  <td width="100" align="right" class="item1"><script>document.write(_("MAC address"));</script></td>
								<td class="value1">
									&nbsp;&nbsp;&nbsp;&nbsp;<input id="MAC" class="text" size="2" maxlength="2" onKeyUp="value=correctMacChar(this.value)">: <input id="MAC" class="text" size="2" maxlength="2" onKeyUp="value=correctMacChar(this.value)">:
									<input id="MAC" class="text" size="2" maxlength="2" onKeyUp="value=correctMacChar(this.value)">: <input id="MAC" class="text" size="2" maxlength="2" onKeyUp="value=correctMacChar(this.value)">:
									<input id="MAC" class="text" size="2" maxlength="2" onKeyUp="value=correctMacChar(this.value)">: <input id="MAC" class="text" size="2" maxlength="2" onKeyUp="value=correctMacChar(this.value)">
								</td>
								<td align=right>
									<input type="button" class="button2"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" value="" id="Add" onClick="add_static(IP,combinMAC2(MAC));">
								</td>
							</tr>
						</table>
						<div id="staticlist" style="position:relative;visibility:visible;"></div>
						<br>
						<hr width="80%">
						<table cellpadding="0" cellspacing="0" class="content1">
							<tr>
								<td align="right">
									<input id="Refresh" type="button" class="button2" value=""  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onclick='refresh("lan_dhcp_clients.asp")'>
								</td>
							</tr>
						</table>
						<div id="dhcplist" style="position:relative;visibility:visible;"></div>
						<br>
						<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
					</form>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("lan_dhcp_clients_helpinfo1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("lan_dhcp_clients_helpinfo2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("lan_dhcp_clients_helpinfo3"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload1('table1');
    </script>
</BODY>
</HTML>






