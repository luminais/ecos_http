<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>Wireless | State</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="javascript">
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>";
var ssidlist = "<% get_wireless_basic("SsidList"); %>";
var Cur_ssid_index = "<% get_wireless_basic("Cur_wl_unit"); %>";

var SSID = new Array();

function init(){
	if(enablewireless==1){
	}else{
		alert("只有在啟用無線網路功能後，才可以使用本功能！");
		top.mainFrame.location.href="wireless_basic.asp";
	}
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
					<table  class="content3" id="table2">
					   <TR> <TD valign="top">本頁面將會顯示無線路由器目前的無線連線狀況。
					   </TR>
						
						<TR> <TD>目前已無線連線的使用者列表：
							  <input type=button class=button2 value="重新整理" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onclick=refresh("wireless_state.asp")></TD>
						</TR>
							<tr><td>	
							 <TABLE align=center cellPadding=0 cellSpacing=0 width=450>
									<TBODY>
									  <TR>
										<TD width=450><TABLE align=center border=1 cellPadding=0 cellSpacing=0 class=content1 style="margin-top:0px;" id="table1">
											<TBODY>
											  <TR>
												<TD align=middle width=20%>編號</TD>
												<TD align=middle width=50%>MAC位址</TD>
												<TD align=middle width=30%>頻道寬度</TD>
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
		<script>
		helpInfo('在此頁面可以查詢到無線網路使用者的基本使用資料。<br><br>MAC位址：連線電腦無線網卡的MAC位址<br>頻道寬度：頻道的頻率寬度');
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


