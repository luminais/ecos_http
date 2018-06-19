<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>WAN | DNS</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

var def_DNS1 = "<%aspTendaGetStatus("wan","dns1");%>";
var def_DNS2 = "<%aspTendaGetStatus("wan","dns2");%>";

var def_dnsen = "<%aspTendaGetStatus("wan","dnsen");%>";

addCfg("DS1",0x34,def_DNS1);
addCfg("DS2",0x35,def_DNS2);

function init(f)
{
	cfg2Form(f);
	if(parseInt(def_dnsen) == 1)
		f.DNSEN.checked = true;
	else
		f.DNSEN.checked = false;
		
	onSel();
}

function preSubmit(f) {

	if(document.frmSetup.DNSEN.checked == true)
	{
	    	if (!verifyIP2(f.DS1,"主要DNS位址")) return ;
	    	if((f.DS2.value !="") )
	    	{
	    		if (!verifyIP2(f.DS2,"次要DNS位址")) return ;
	    	}
			f.DSEN.value = "1";
    	}
	else
		f.DSEN.value = "0";
	

	form2Cfg(f);
	//document.getElementById("rebootTag").value = IsReboot();
	f.submit();
}

function onSel()
{
	if(document.frmSetup.DNSEN.checked == false)
		document.frmSetup.DS1.disabled = document.frmSetup.DS2.disabled = true;
	else
		document.frmSetup.DS1.disabled = document.frmSetup.DS2.disabled = false;
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
				<form name=frmSetup method=POST action=/goform/AdvSetDns>
				<input type=hidden name=GO value=wan_dns.asp>
				<input type="hidden" id="rebootTag" name="rebootTag">
				<input type="hidden" name="DSEN">
				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tr> 
						<td width="31%" align="right">網域名稱伺服器(DNS)設定</td>
						<td width="69%">&nbsp;&nbsp;&nbsp;
					    <input type="checkbox" name="DNSEN"  onClick="onSel()"></td>
					</tr>
					<tr> 
						<td align="right">主要DNS伺服器位址</td>  
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=DS1 class=text size=15 maxlength=15></td>
					</tr>
					<tr>
						<td align="right">次要DNS伺服器位址(選填)</td> 
						<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=DS2 class=text size=15 maxlength=15></td>
					</tr>
					
					</table>
					<table cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
				      <tr><td colspan="2">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
					請注意：設定完成後必須重新啟動路由器，變更設定值才會生效。
					  </td></tr>
					</table>
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>
		helpInfo('請注意：設定完成後必須重新啟動路由器，設定才會生效。網域名稱伺服器(DNS)設定，可以讓您輸入指定的DNS伺服器IP位址，並透過此指定的DNS伺服器進行域名解析，若未特別指定，則路由器會自動代入ISP提供的DNS伺服器<br><br>如果DNS伺服器IP填寫錯誤，會造成網頁無法瀏覽，建議使用預設值即可！'	);
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

