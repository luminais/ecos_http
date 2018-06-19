<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
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
	document.LANDhcpsSet.DHEN.checked = <%aspTendaGetStatus("lan","dhcps");%>;//是否启用：0：关闭；1：启用；
	document.LANDhcpsSet.DHLT.value = "<%aspTendaGetStatus("lan","lease_time");%>";//过期时间：3600：一小时；7200：二小时； 10800：三小时； 86400：一天； 172800：两天； 604800：一周

	document.LANDhcpsSet.dips.value = (("<%aspTendaGetStatus("lan","dhcps_start");%>").split("."))[3];//IP池开始地址
	document.LANDhcpsSet.dipe.value = (("<%aspTendaGetStatus("lan","dhcps_end");%>").split("."))[3];//IP池结束地址
}

function preSubmit(f) {

	var loc = "/goform/DhcpSetSer?GO=lan_dhcps.asp";

	if (!rangeCheck(f.dips,1,254,"IP池开始地址")) return ;
	if (!rangeCheck(f.dipe,1,254,"IP池终止地址")) return ;
   
   	if (Number(f.dips.value)>Number(f.dipe.value)) {
      alert("IP池开始地址不能大于结束地址 !!!");
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
					  <td width="100" align="right">DHCP服务器</td> 
					  <td>&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name=DHEN value=1>启用</td>
					</tr>
					<tr> 
					  <td align="right">IP池开始地址</td> 
					  <td>&nbsp;&nbsp;&nbsp;&nbsp;<SCRIPT>document.write(netip);</SCRIPT>
						<input NAME=dips class=text  SIZE="3"></td>
					</tr>
					<tr> 
						<td align="right">IP池结束地址</td> 
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<SCRIPT>document.write(netip);</SCRIPT>
						<input NAME=dipe class=text  SIZE=3></td>
					</tr>
					<tr> 
					  <td align="right">过期时间</td> 
					  <td>
						&nbsp;&nbsp;&nbsp;&nbsp;<select NAME=DHLT SIZE=1>
						<option VALUE="3600">一小时</option>
						<option VALUE="7200">二小时</option>
						<option VALUE="10800">三小时</option>
						<option VALUE="86400">一天</option>
						<option VALUE="172800">两天</option>
						<option VALUE="604800">一周</option>
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
		<script>helpInfo('DHCP服务器提供了为客户端自动分配IP地址的功能，如果您使用本路由器的DHCP服务器功能的话，您可以让DHCP服务器自动替您配置局域网中各计算机的TCP/IP协议。<br>\		&nbsp;&nbsp;&nbsp;&nbsp;IP地址池:输入一个起始IP地址和一个终止IP地址以形成分配动态IP地址的范围。\
		<br>&nbsp;&nbsp;&nbsp;&nbsp;过期时间: 电脑(DHCP客户端)要求时才分配过期时间。');
		</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>





