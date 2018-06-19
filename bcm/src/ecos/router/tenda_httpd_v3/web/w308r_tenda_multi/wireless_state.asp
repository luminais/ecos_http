<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>Wireless | State</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="javascript">
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>";
var ssidlist = "<% get_wireless_basic("SsidList"); %>";
var Cur_ssid_index = "<% get_wireless_basic("Cur_wl_unit"); %>";

var SSID = new Array();
Butterlate.setTextDomain("wireless_set");
function initTranslate(){
	var e=document.getElementById("refresh_btn");
	e.value=_("Refresh");
}
function init(){
	initTranslate();
	if(enablewireless==1){
		SSID = ssidlist.split("\t");
		UpdateMBSSIDList();
	}else{
		alert(_("This function can only be used after the wireless function is enabled"));
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
					<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tbody>
					<tr>
					<td width="100" align="right" class="head"><script>document.write(_("Select SSID"));</script></td>
					<td id="ssid-select-content">
					  &nbsp;&nbsp;&nbsp;&nbsp;<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
							<!-- ....Javascript will update options.... -->
					  </select>
					</td>
					</tr>
					</tbody>
					</table>
					<table  class="content3" id="table2">
					   <TR> <TD valign="top"><script>document.write(_("This page displays the connection information of the wireless router"));</script>
					   </TR>
						<TR> <TD><script>document.write(_("The currently connected hosts list"));</script>
							  <input id="refresh_btn" type=button class=button2 value="" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onclick=refresh("wireless_state.asp")></TD>
						</TR>
							<tr><td>	
							 <TABLE align=center cellPadding=0 cellSpacing=0 width=450>
									<TBODY>
									  <TR>
										<TD width=450><TABLE align=center border=1 cellPadding=0 cellSpacing=0 class=content1 style="margin-top:0px;" id="table1">
											<TBODY>
											  <TR>
												<TD align=middle width=20%><script>document.write(_("NO."));</script></TD>
												<TD align=middle width=50%><script>document.write(_("MAC address"));</script></TD>
												<TD align=middle width=30%><script>document.write(_("Bandwidth"));</script></TD>
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
		<script>helpInfo(_("Wireless_state_helpinfo1")+"<br>"+_("Wireless_state_helpinfo2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>






