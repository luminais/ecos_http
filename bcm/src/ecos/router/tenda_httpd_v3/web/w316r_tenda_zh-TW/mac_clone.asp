<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>WAN | MAC Clone</title>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<link rel=stylesheet type=text/css href=style.css />
<SCRIPT language=JavaScript>
var cln_MAC = "<%aspTendaGetStatus("sys","manmac");%>";
var def_MAC = "<%aspTendaGetStatus("sys","wanmac");%>";
var fac_MAC = "<%aspTendaGetStatus("sys","fmac");%>";
addCfg("WMAC",31,def_MAC);
addCfg("cloneEn",32,"1");
function init(f)
{
	cfg2Form(f);
}

function defMac(f)
{
	f.cloneEn.value=0;
	decomMAC2(f.WMAC, fac_MAC, 1);
}
function cloneMac(f)
{
	f.cloneEn.value=1;
    decomMAC2(f.WMAC, cln_MAC, 1);
}

function preSubmit(f)
{
  	var mac ;
	mac = f.WMAC.value;

	if(!CheckMAC(mac))
	{
		alert("MAC:"+ mac +" 無效!");
		return;
	}
	if(!ckMacReserve(mac))return ;
		
	form2Cfg(f);
	//document.getElementById("rebootTag").value = IsReboot();
	f.submit();
	/*huang add*/
	/*if(document.getElementById("rebootTag").value == 1)
	{
		f.submit();
	}
	else{
		return;
	}*/
	/*add end*/
}
</SCRIPT>
</head>
<body leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
    <tr>
    <td width="33">&nbsp;</td>
    <td width="679" valign="top">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
        <tr>
          <td align="center" valign="top"><table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
        <tr>
           <td align="center" valign="top">
			<form name=frmSetup method=POST action=/goform/AdvSetMacClone>
			<input type=hidden name=GO value=mac_clone.asp >
			<!--<input type="hidden" id="rebootTag" name="rebootTag">-->
		     <table cellpadding="0" cellspacing="0" class="content2">   
				<tr>
				   <td colspan="2" align="left" valign="top">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;WAN MAC地址克隆</td>	
                </tr>
			</table>
			<table cellpadding="0" cellspacing="0" class="content1" id="table1" style="margin-top:2px">
                 <tr>
                     <td width="180" align="right">MAC地址：</td>
                      <input type=hidden name=cloneEn />
                     <td width="316" align="left" valign="middle">&nbsp;&nbsp;&nbsp;&nbsp;
                     <input class=text name=WMAC size=17 maxlength=17 /></td>
                 </tr>
                 <tr>
                    <td align="right" height="30"><input type=button value="恢復默認MAC地址" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  onclick="defMac(this.form);" /></td>
                    <td>&nbsp;&nbsp;&nbsp;&nbsp;<input type=button value="克隆MAC地址" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="cloneMac(this.form);" /></td>
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
        <script>helpInfo('MAC地址：本路由器對廣域網的MAC地址，此值一般不用更改。但某些ISP可能會要求對MAC地址進行綁定,\
此時ISP會提供一個有效的MAC地址給用户，您只要根據它所提供的值，輸入到“MAC地址”欄然后單擊“保存”，您即可改變本路由器對廣域網的MAC地址。<br>\
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;克隆MAC地址： 點擊此按鈕將把當前進行管理主機的MAC地址復制到路由器WAN口MAC地址上。');</script>
		</td>
      </tr>
    </table>
<script type="text/javascript">table_onload('table1');</script>
</body>
</html>
