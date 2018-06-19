<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>System | Firmware Upgrade</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

var pc = 0;

FirwareVerion="<%aspTendaGetStatus("sys","sysver");%>";//更新韌體包版本
FirwareDate="<%aspTendaGetStatus("sys","compimetime");%>";//更新韌體日期

function init(f){
	f.reset();
}

function setWidth(windowObj, el, newwidth)
{
    if (document.all)
	{
	  if (windowObj.document.all(el) )
        windowObj.document.all(el).style.width = newwidth ;
	}
	else if (document.getElementById)
	{
	  if (windowObj.document.getElementById(el) )
	    windowObj.document.getElementById(el).style.width = newwidth;
	}
}

function OnUpGrade()
{
	pc+=1; 	
	
	if (pc > 100) 
	{      
		clearTimeout(time); 
		return;
	}
	
	//setWidth(self, "table_lpc", pc + "%");
	document.getElementById("table_lpc").style.width= pc+"%";
	document.getElementById("table_lpc_msg").innerHTML ="更新韌體中..." + pc + "%";
	
	setTimeout("OnUpGrade()",150);
}

function UpGrade(){        
	if (document.frmSetup.upgradeFile.value == "")
	{
		alert("首先，請選擇要更新的韌體檔案！");
		return ;
	}
	if(confirm('您確定要更新韌體嗎？')){
	   document.getElementById("fwsubmit").disabled = true;
	   document.frmSetup.submit() ;
	   document.getElementById("table_lpc").style.display = "";
	   OnUpGrade();
	} 	
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
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
				<table cellpadding="0" cellspacing="0" class="content2">
				<tr><td colspan=2 valign="top">&nbsp;&nbsp;透過更新本路由器的韌體，您可以獲取您需要的新功能，或者問題修正等好處。</td>
				</tr>
				</table>				

				<table cellpadding="0" cellspacing="0" class="content3" id="table1">				
				<form name=frmSetup method="POST" action="/cgi-bin/upgrade" enctype="multipart/form-data">
					<tr><td nowrap>&nbsp;&nbsp;選擇韌體檔案：<br>
					&nbsp;&nbsp;<input type="file" name="upgradeFile"/>&nbsp;&nbsp;&nbsp;&nbsp;
					<input id=fwsubmit type=button class=button2 value=更新韌體 onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="UpGrade()"/>
					</td>
					</tr>
					</form>
				</table>
				
				<table cellpadding="0" cellspacing="0" class="content3">
					<tr><td>&nbsp;&nbsp;目前的系統版本：
						<SCRIPT>document.write( FirwareVerion+';  發布日期：'+FirwareDate )</SCRIPT></td></tr>
					<tr><td colspan=2>&nbsp;&nbsp;請注意：更新過程不可關閉路由器電源，否則將導致路由器毀損而無法使用。更新成功後，路由器將會自動重新啟動。更新過程可能需耗費數分鐘，請耐心等候。</td></tr>
					<tr><td colspan="2" id="table_lpc_msg"  style="font-size:18px; color:#CC0000">
					</td></tr>
					<tr><td colspan=2>
					<table id=table_lpc bgcolor="#CC0000" height=20  style="display:none">
					<tr>
					  <td></TD>
				    </TR>
					</table> 
					</td></tr>
				</table>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>
		helpInfo('拜訪Tenda的網站（www.tenda.com.cn），下載更新版本的韌體。<br><br>透過瀏覽找到對應的韌體升級檔。<br>點選「更新韌體」更新完成後，路由器將自動重新啟動。');
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

