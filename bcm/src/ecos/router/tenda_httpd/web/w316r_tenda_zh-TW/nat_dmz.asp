<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>NAT | DMZ</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>

var def_LANMASK = "<%aspTendaGetStatus("lan", "lanmask");%>";
var def_LANIP = "<%aspTendaGetStatus("lan", "lanip");%>";
var def_dmzen = "<%aspTendaGetStatus("lan","dmzen");%>";//DMZ主机IP地址是否啟用：0：未啟用；1:啟用
var def_DMZIP = "<%aspTendaGetStatus("lan","dmzip");%>";//MZ主机IP地址

addCfg("LANIP", 0,def_LANIP);
addCfg("en", 1, def_dmzen);
addCfg("DMZ1", 2, def_DMZIP);

var netip=getCfg("LANIP").replace(/\.\d{1,3}$/,".");

function init(f){
	f.dmzip.value=def_DMZIP;
	cfg2Form(f);
}

function preSubmit(f) {
	var m;
	if (f.dmzip.value=='' && (!f.en.checked) ) m='';
	else
	{
		if (f.dmzip.value==getCfg("LANIP")) { alert("不可以使用路由器的IP位址！"); return; }
		if (!verifyIP2(f.dmzip,"DMZ主機的IP位址")) return ;
		if ( !ipCheck(def_LANIP,f.dmzip.value, def_LANMASK) ) 
		{
			alert(f.dmzip.value+ "與路由器的LAN IP位址不同網段！");
			f.dmzip.value="0.0.0.0";
			return ;
		}
		if(f.dmzip.length<8)
			alert("不正確的IP位址！");
		if (f.dmzip.value=='' || (!f.en.checked) ) m='';
		else
		{
		  m="1;0;"+f.dmzip.value;
		}
	}
	setCfg("DMZ1",m);
	form2Cfg(f);
	f.submit();

}

</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top" headers="100%">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup method=POST action=/goform/VirSerDMZ>
				<input type=hidden name=GO value=nat_dmz.asp >
				<table cellpadding="0" cellspacing="0" class="content2">
				<TR><TD colspan=3 valign="top">請注意：設定DMZ主機後，與該IP相關的所有防火牆設定，將不會有作用。</TD>
				</TR>
				</table>
				<table cellpadding="0" cellspacing="0" class="content1" id="table1" style="margin-top:0px;">
					<tr>
						<td width="100" align="right" nowrap>DMZ主機的IP位址</td>
						<td width="157">&nbsp;&nbsp;&nbsp;&nbsp;
					    <input class=text name=dmzip size=16 maxlength=15></td>
						<td width="132"><input type=checkbox name=en value=1>&nbsp;啟用</td>
					</tr>
				
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
		helpInfo('在某些特殊狀況下，需要把內部區域網路中的一台電腦，完全對應到網際網路上，以實現雙向通訊，此時可以把該台電腦設定為DMZ主機。<br><br>請注意：啟用DMZ主機後，將會關閉本設備對DMZ主機的防火牆保護。');
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

