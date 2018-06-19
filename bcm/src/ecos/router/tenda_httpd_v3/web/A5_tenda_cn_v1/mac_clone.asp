<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312" />
<title>WAN | MAC Clone</title>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
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
		alert("MAC:"+ mac +" 无效!");
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
<link rel=stylesheet type=text/css href=style.css>
</head>

<body  leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
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
					<tr><td colspan="2" align="left" valign="top">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;WAN MAC地址克隆。</td></tr>
					</table>

					<table cellpadding="0" cellspacing="0" class="content1" id="table1" style="margin-top:2px">
                      <tr>
                        <td width="100" align="right">MAC地址：</td>
                        <input type=hidden name=cloneEn />
                        <td width="160" align="left" valign="middle">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=WMAC size=17 maxlength=17 /></td>
                      </tr>
                      <tr>
                        <td valign="middle" colspan="2" align="center" height="30"><input type=button value="恢复默认MAC地址" onmouseover="style.color='#FF9933'" onmouseout="style.color='#000000'"  onclick="defMac(this.form);" />
                          &nbsp;
                          <input type=button value="克隆MAC地址" onmouseover="style.color='#FF9933'" onmouseout="style.color='#000000'"  onclick="cloneMac(this.form);" /></td>
                      </tr>
                    </table>
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
					</FORM>                    </td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
        <script>helpInfo('MAC地址：本路由器对广域网的MAC地址，此值一般不用更改。但某些ISP可能会要求对MAC地址进行绑定,\
		此时ISP会提供一个有效的MAC地址给用户，您只要根据它所提供的值，输入到“MAC地址”栏然后单击“确定”，您即可改变本路由器对广域网的MAC地址。<br>\
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;克隆MAC地址： 点击此按钮将把当前进行管理主机的MAC地址复制到路由器WAN口MAC地址上。');</script>
		</td>
      </tr>
    </table>
<script type="text/javascript">table_onload('table1');</script>
</body>
</html>

