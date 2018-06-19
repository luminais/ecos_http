<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>Wireless | State</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="javascript">
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>";
var ssidlist = "<% get_wireless_basic("SsidList"); %>";
var Cur_ssid_index = "<% get_wireless_basic("Cur_wl_unit"); %>";

var SSID = new Array(2);
SSID[0] = "<% get_wireless_basic("SSID"); %>";
SSID[1] = "<% get_wireless_basic("SSID1"); %>";

function init(){
	if(enablewireless==1){
		//SSID = ssidlist.split(";");
		UpdateMBSSIDList();
	}else{
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href="wireless_basic.asp";
	}
}

function UpdateMBSSIDList() {
	var defaultSelected = false,
	    selected = false,
		optionStr = '&nbsp;&nbsp;&nbsp;&nbsp;<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">';
	
	for(var i=0; i<SSID.length; i++){
		if(SSID[i] != "") {
			if (Cur_ssid_index == i) {
				optionStr += '<option selected="selected">' + SSID[i] + '</option>';
			} else {
				optionStr += '<option>' + SSID[i] + '</option>';
			}
		}
	}
	optionStr += '</select>';
	document.getElementById('ssid-select-content').innerHTML = optionStr;
}

/*
 * When user select the different SSID, this function would be called.
 */ 
function selectMBSSIDChanged()
{
	var ssid_index;
	
	ssid_index = document.frmSetup.ssidIndex.options.selectedIndex;
	
	var loc = "/goform/onSSIDChange?GO=wireless_state.asp";

	loc += "&ssid_index=" + ssid_index; 
	var code = 'location="' + loc + '"';
   	eval(code);
}

</script>

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
				<FORM name=frmSetup action="" method=post >
				<input type="hidden" id="GO" name="GO" value="wireless_state.asp">
					<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tbody>
					<tr>
					<td width="100" align="right" class="head">选择SSID</td>
					<td id="ssid-select-content">
					  &nbsp;&nbsp;&nbsp;&nbsp;<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
							<!-- ....Javascript will update options.... -->
					  </select>
					</td>
					</tr>
					</tbody>
					</table>
					<table  class="content3" id="table2">
					   <TR> <TD valign="top">本页显示无线路由器的连接信息。
					   </TR>
						
						<TR> <TD>当前连接的主机列表：
							  <input type=button class=button2 value="刷新" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onclick=refresh("wireless_state.asp")></TD>
						</TR>
							<tr><td>	
							 <TABLE align=center cellPadding=0 cellSpacing=0 width=450>
									<TBODY>
									  <TR>
										<TD width=450><TABLE align=center border=1 cellPadding=0 cellSpacing=0 class=content1 style="margin-top:0px;" id="table1">
											<TBODY>
											  <TR>
												<TD align=middle width=20%>序号</TD>
												<TD align=middle width=50%>MAC地址</TD>
												<TD align=middle width=30%>带宽</TD>
											  </TR>
                              <%  get_wireless_station("stationinfo"); %>
											 
											</TBODY>
										</TABLE></TD>
									  </TR>
						 
								  </TABLE>
								  <br>
								  </td>
								<td class=vline rowspan=15 width="27"><br></td>
							  </tr>
											
							 <TR><TD class=hline></TD></TR>   
						   </tbody>
						</table>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo('此页面可以查看到无线连接的客户端信息。<br>\
			MAC地址：当前主机的无线网卡MAC地址。<br>\带宽：信道频率宽度。'
		);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






