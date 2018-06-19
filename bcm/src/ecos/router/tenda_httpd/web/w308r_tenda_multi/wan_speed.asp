<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>WAN | Speed</title>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu2.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">
Butterlate.setTextDomain("wan_set");
var WANSPEED = "<%aspTendaGetStatus("wan","wanspeed");%>";

function init()
{
	document.wanspeed.ws[WANSPEED].checked = true;
}

function preSubmit(f)
{
	f.submit();
}
</script>
<link rel=stylesheet type=text/css href=style.css>
</head>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init();" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top"><table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
                <tr>
                  <td align="center" valign="top">
				  <form name="wanspeed" method=post action=/goform/setWanSpeed>
					<div align="center" class="content1" style="width:100%;">
					<table cellpadding="1" cellspacing="0" border="0" class="content1" style="margin-top:0px; width:80%;">
						<tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(_("Choose the WAN speed"));</script>:</td></tr>
					</table>
					<table cellpadding="1" cellspacing="0" border="0" class="content1" id="table1" style="margin-top:0px; width:80%;">
						<tr><td width="20%" align="left"><input type="radio" name="ws" value="0">AUTO</td></tr>
						<tr><td width="20%" align="left"><input type="radio" name="ws" value="1">10M HALF-duplex</td></tr>
						<tr><td width="20%" align="left"><input type="radio" name="ws" value="2">10M FULL-duplex</td></tr>
						<tr><td width="20%" align="left"><input type="radio" name="ws" value="3">100M HALF-duplex</td></tr>
						<tr><td width="20%" align="left"><input type="radio" name="ws" value="4">100M FULL-duplex</td></tr>
						
					</table>
					</div><br >
					<SCRIPT>tbl_tail_save("document.wanspeed");</SCRIPT>
					</form>
					</td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">	
		<script>helpInfo(_("Wan_speed_helpinfo1"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</html>

