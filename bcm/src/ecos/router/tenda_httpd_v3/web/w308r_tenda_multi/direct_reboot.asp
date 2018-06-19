<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>Direct | Reboot</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">
var lanip = "<%aspTendaGetStatus("lan","lanip");%>";
var time = 0;
var pc = 0;
Butterlate.setTextDomain("system_tool");
function init(f)
{
	reboot();
}
function reboot()
{
	pc+=1; 
	if (pc > 100) 
	{ 
		window.top.location = "http://" + lanip;
		clearTimeout(time); 
		return;
	} 
	//setWidth(self, "lpc", pc + "%");
	document.getElementById("lpc").style.width= pc+"%";
	document.getElementById("percent").innerHTML = _("Rebooting") + pc + "%";
	time = setTimeout("reboot()",250);
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
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top"><table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
                <tr>
                  <td align="center" valign="top">
				  <form name=frmSetup method="POST">
					<table class=content1>
						<tr><td id="percent" style="font-size:24px; color:#3366CC"></td></tr>								
					<tr> <TD align="left" width="400" colspan="2">
					<table width="100%" align="left" bgcolor="#ffffff" cellpadding="0" cellspacing="0" bordercolor="#999999" id="AutoNumber19" style="border-style: solid; border-width: 1px">
				   <tr>
					<td align=left> 
					<table id=lpc bgcolor="#3366CC" height=20>
					<tr>
					  <td></TD>
					</TR>
					</table> 
				   </TD>				
				   </TR>
				   </table> 
					</TD>
					</tr>
					</table>
				</FORM>
				</td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("direct_reboot_Help_Inf"));</script>	
		</td>
      </tr>
    </table>
</BODY>
</HTML>






