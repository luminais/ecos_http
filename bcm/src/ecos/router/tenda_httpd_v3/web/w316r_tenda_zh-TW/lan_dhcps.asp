<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>DHCP | Server</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>";

var LANIP=def_LANIP;
var netip=LANIP.replace(/\.\d{1,3}$/,".");

function init()
{
	document.LANDhcpsSet.DHEN.checked = <%aspTendaGetStatus("lan","dhcps");%>;//是否啟用：0：关闭；1：啟用；
	document.LANDhcpsSet.DHLT.value = "<%aspTendaGetStatus("lan","lease_time");%>";//过期时间：3600：一小時；7200：二小時； 10800：三小時； 86400：一天； 172800：两天； 604800：一周

	document.LANDhcpsSet.dips.value = (("<%aspTendaGetStatus("lan","dhcps_start");%>").split("."))[3];//IP範圍開始位址
	document.LANDhcpsSet.dipe.value = (("<%aspTendaGetStatus("lan","dhcps_end");%>").split("."))[3];//IP池结束地址
}

function preSubmit(f) {

	var loc = "/goform/DhcpSetSer?GO=lan_dhcps.asp";

	if (!rangeCheck(f.dips,1,254,"IP範圍開始位址")) return ;
	if (!rangeCheck(f.dipe,1,254,"IP範圍結束位址")) return ;
   
   	if (Number(f.dips.value)>Number(f.dipe.value)) {
      alert("IP開始位址不可以大於結束位址！");
      return ;
   	}

	if(f.DHEN.checked)
	{
		loc += "&dhcpEn=1";
	}
	else
	{
		loc += "&dhcpEn=0";
	}
	
	loc += "&dips=" + netip + f.dips.value;
	loc += "&dipe=" + netip + f.dipe.value;
	loc += "&DHLT=" + f.DHLT.value;
		
	var code = 'location="' + loc + '"';
	eval(code);
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init()" class="bg">
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
				<form name=LANDhcpsSet method=POST action=/goform/DhcpSetSer>
				<input type=hidden name=GO  value="lan_dhcps.asp">
				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tr> 
					  <td width="100" align="right">DHCP伺服器設定</td> 
					  <td>&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name=DHEN value=1>啟用</td>
					</tr>
					<tr> 
					  <td align="right">IP範圍開始位址</td> 
					  <td>&nbsp;&nbsp;&nbsp;&nbsp;<SCRIPT>document.write(netip);</SCRIPT>
						<input NAME=dips class=text  SIZE="3"></td>
					</tr>
					<tr> 
						<td align="right">IP範圍結束位址</td> 
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<SCRIPT>document.write(netip);</SCRIPT>
						<input NAME=dipe class=text  SIZE=3></td>
					</tr>
					<tr> 
					  <td align="right">過期時間</td> 
					  <td>
						&nbsp;&nbsp;&nbsp;&nbsp;<select NAME=DHLT SIZE=1>
						<option VALUE="3600">一小時</option>
						<option VALUE="7200">二小時</option>
						<option VALUE="10800">三小時</option>
						<option VALUE="86400">一天</option>
						<option VALUE="172800">兩天</option>
						<option VALUE="604800">一星期</option>
						</select>
					  </td>
					</tr>
				</table>  
				<input type=hidden name=dhcpEn>
					<SCRIPT>tbl_tail_save("document.LANDhcpsSet");</SCRIPT>
				</form>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>
		helpInfo('TCP/IP協定設定包含IP位址、子網路遮罩、預設閘道以及DNS伺服器等。對一般人來說要為區域網路中所有的電腦設定正確的TCP/IP協定與相關參數不是一件容易的事，即便會設定，這也是一件麻煩的事，但DHCP伺服器就提供了這樣的功能。如果您使用本路由器中的DHCP伺服器功能，DHCP伺服器就會自動分配IP與相關參數給區域網路內的所有電腦。<br><br>IP位址範圍：請輸入一個開始IP與一個結束IP，路由器會自動以此2 IP位址為範圍，分配IP位址給去育網路內的電腦。<br>過期時間：電腦(DHCP使用者)要求時，才會分配過期時間。');
		</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

