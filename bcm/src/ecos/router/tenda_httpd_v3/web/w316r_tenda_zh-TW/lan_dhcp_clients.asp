<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>LAN | DHCP Client List</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language="JavaScript">

def_DHS = "<%aspTendaGetStatus("lan","dhcps_start");%>";//DHCP服务设置 IP池开始位址：最后一个字节
def_DHE = "<%aspTendaGetStatus("lan","dhcps_end");%>";//DHCP服务设置 IP池结束位址：最后一个字节
def_LEASE = "<%aspTendaGetStatus("lan","lease_time");%>";//add by roy

addCfg("DHS",1,def_DHS);
addCfg("DHE",3,def_DHE);

var dhcpList=new Array(<%TendaGetDhcpClients("list");%>);//客户端列表 格式：'電腦名稱;IP位址;MAC位址;静态(0:close;1:open);租約時間(秒数)',''、、、、、、、、、

var StaticList = new Array(<%TendaGetDhcpClients("staticlist");%>);//静态列表//'ip;mac',、、、
var ipmaceninit = "<%TendaGetDhcpClients("dhcpipmacbind");%>";//????//'00000000',???
def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>";//LAN口设置的IP位址
addCfg("LANIP",0,def_LANIP);
ctime=0;

var LANIP=getCfg("LANIP");
var netip=LANIP.replace(/\.\d{1,3}$/,".");
var dhs=getCfg("DHS").match(/\d{1,3}$/);
var dhe=getCfg("DHE").match(/\d{1,3}$/);

function showList()
{
	var m='<table class=content1 border=1 style="margin-top:0px;" cellpadding="0" cellspacing="0">';
	m+='<tr class=item1 align=center height=30>';
	m+='<th nowrap>電腦名稱</th>';
	m+='<th nowrap>IP 位址</th>';
	m+='<th nowrap>MAC 位址</th>';
	m+='<th nowrap>租約時間</th>';
	m+='</tr>';
	for (i=0;i<dhcpList.length;i++) {
		//;10.10.10.100;00:0C:43:30:52:66;0;3427243784d
		var s=dhcpList[i].split(";");
		//if (s.length!=4) break;
		//if (s.length!=5) break;//roy modified
		
		m+='<tr class=value1 align=center>';
		if(s[0] == "")
			m+='<td>'+"&nbsp;"+'</td>';
		else
			m+='<td>'+s[0]+'</td>';
		m+='<td>'+s[1]+'</td>';
		m+='<td>'+s[2]+'</td>';
		//m+='<td>'+timeStr(s[3]-ctime)+'</td>';
		m+='<td>'+timeStr(s[4]-ctime)+'</td>';//roy mdified
		m+='</tr>';
	}
	document.getElementById("dhcplist").innerHTML = m;

}

function showStaticList()
{
	var m='<table class=content1 border=1 id=staticTab cellpadding="0" cellspacing="0">';
	m+='<tr class=item1 align=center height=30>';
	m+='<th nowrap>編號</th>';
	m+='<th nowrap>IP 位址</th>';
	m+='<th nowrap>MAC 位址</th>';
	m+='<th nowrap>IP-MAC 绑定</th>';
	m+='<th nowrap>刪除</th>';
	m+='</tr>';
	for (i=0;i<StaticList.length;i++)
	{
		//hostname;ip;mac;flag;lease
		var s=StaticList[i].split(";");
		//if (s.length!=2) break;
		if (s.length <4) break;//roy modified //modify by stanley
				
		m+='<tr class=value1 align=center>';
		m+='<td>'+eval(i+1)+'</td>';
		m+='<td>'+s[1]+'</td>';
		m+='<td>'+s[2]+ '</td>';
		
	
		//if( parseInt(ipmaceninit.charAt(i)) )
		if(s[3] == "1")//modify by stanley
			m+='<td ><input type="checkbox" id="' + ("en" + eval(i+1)) +'" checked ></td>';
		else
			m+='<td ><input type="checkbox" id="' + ("en" + eval(i+1)) +'" ></td>';
		m+='<td><input type=button class=button  onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'" value="Delete" onclick="OnDel(this,' + i +  ')"></td>';
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
			box = document.getElementById("en"+(i+2));
			box.id = "en"+(i+1);
		}
	}
	StaticList.length--;
	showStaticList();
}

function init(){

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
	//if(StaticList.length == 0)
		//return;
	for (var i=0;i<StaticList.length;i++) {
		//hostname+';'+netip+ip.value+';'+mac+';'+flag+';'+def_LEASE
		s=StaticList[i].split(";");
		if(document.getElementById("en"+(i+1)).checked){
			ipmacen += "1";
			StaticList[i] = (s[0]+";"+s[1]+";"+s[2]+";"+ipmac_enable+";"+s[4]);
		}else{
			StaticList[i] = (s[0]+";"+s[1]+";"+s[2]+";"+ipmac_disable+";"+s[4]);
			ipmacen += "0";
		}	
		
		//loc += "&list" + eval(i+1) + "='" + StaticList[i]  + "'";
		loc += "&list" + eval(i+1) + "=" + StaticList[i];
	}

	loc += "&IpMacEN=" + ipmacen;
	loc += "&LISTLEN=" + StaticList.length;
    
	var code = 'location="' + loc + '"';
    eval(code);

   
}
function add_static(ip, mac)
{
	//if (!rangeCheck(ip,Number(dhs),Number(dhe),"IP Address")) return ;
	var f=document.frmSetup;
	var hostname="";//add by roy
	var flag = "1";//static lease,add by roy
	f.staticIpAddress.value = netip+ip.value;
	//add by stanley
	if(StaticList.length >15)
	{
		alert("最多只能新增16條固定IP-MAC綁定規則");
		return ;
	}
	//add end
	if (!verifyIP2(f.staticIpAddress,"IP 位址")) return ;
	if (!macsCheck(mac,"MAC 位址")) return ;
	if(!ckMacReserve(mac))return ;
	var all=StaticList.toString()+LANIP+';';
	if (all.indexOf(netip+ip.value+';') >=0) { alert("不可以新增重複的IP位址！"); return; }
	
	
	for (var k=0;k<StaticList.length;k++) {
		if (StaticList[k].toString().indexOf(mac) >=0) {
			if (!confirm("不可以新增重複的MAC位址！")) return ;
			rmEntry(StaticList, k);
		}
	}
	//StaticList[StaticList.length]=(netip+ip.value+';'+mac);
	StaticList[StaticList.length]=(hostname+';'+netip+ip.value+';'+mac+';'+flag+';'+def_LEASE);//modified by roy
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
								<td colspan="3" bgcolor="#9D9D9D">&nbsp;&nbsp;<b>固定分配</b>								</td>
							</tr>
					</table>
					<div style="width:75%; height:1px; background-color:#c0c7cd; overflow:hidden; padding:0px; margin-top:1px;"></div>
					<table cellpadding="0" cellspacing="0" class="content3" id="table1">
							<tr>
								<td width="100" align="right" class="item1">IP 位址</td>
								<td class="value1" nowrap colspan="2">
									&nbsp;&nbsp;&nbsp;&nbsp;<SCRIPT>		
										document.write(netip) ;
									</SCRIPT>
									<input id="IP" class="text" size="3" maxlength="3">
								</td>
							</tr>
							<tr>
								<td align="right" class="item1">MAC 位址</td>
								<td class="value1">
									&nbsp;&nbsp;&nbsp;&nbsp;<input id="MAC" class="text" size="2" maxlength="2" width="">: <input id="MAC" class="text" size="2" maxlength="2">:
									<input id="MAC" class="text" size="2" maxlength="2">: <input id="MAC" class="text" size="2" maxlength="2">:
									<input id="MAC" class="text" size="2" maxlength="2">: <input id="MAC" class="text" size="2" maxlength="2">
								</td>
								<td align=right>
									<input type="button" class="button2"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" value="新增" id="ADD" onClick="add_static(IP,combinMAC2(MAC));">
								</td>
							</tr>
						</table>
						<div id="staticlist" style="position:relative;visibility:visible;"></div>
						<br>
						<hr width="75%">
						<table cellpadding="0" cellspacing="0" class="content1">
							<tr>
								<td align="right">
									<input type="button" class="button2" value="重新整理"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onclick='refresh("lan_dhcp_clients.asp")'>
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
		<script>helpInfo('DHCP使用者列表可以顯示使用者從路由器的DHCP伺服器取得的IP位址，電腦的MAC位址...等資訊。您可以使用「固定分配」功能，手動設定，保留特定IP位址給特定的電腦(MAC位址)。<br><br>設定說明：如果要清除已經設定過的項目，請在選擇該項目後點選「清空」按鈕，然後點選「儲存」才會真的清除。<br>請注意：設定完成後，必須重新啟動路由器，設定值才會生效。'
		);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload1('table1');
    </script>
</BODY>
</HTML>

