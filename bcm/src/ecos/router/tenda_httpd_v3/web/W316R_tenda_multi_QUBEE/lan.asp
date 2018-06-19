<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>LAN | LAN Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("wan_set");
addCfg("LANIP",0,"<%aspTendaGetStatus("lan","lanip");%>");
addCfg("LANMASK",1,"<%aspTendaGetStatus("lan","lanmask");%>");
function same_net_with_lan(f)
{
	var new_mask = f.LANMASK.value.split(".");
	var new_ip = f.LANIP.value.split(".");

	var old_mask = getCfg("LANMASK").split(".");
	var old_ip = getCfg("LANIP").split(".");

	var i_new_mask = new_mask[0]<<24|new_mask[1]<<16|new_mask[2]<<8|new_mask[3];
	var i_new_ip = new_ip[0]<<24|new_ip[1]<<16|new_ip[2]<<8|new_ip[3];

	var i_old_mask = old_mask[0]<<24|old_mask[1]<<16|old_mask[2]<<8|old_mask[3];
	var i_old_ip = old_ip[0]<<24|old_ip[1]<<16|old_ip[2]<<8|old_ip[3];

	if((i_new_mask&i_new_ip) == (i_old_mask&i_old_ip))
		return true;
	else
		return false;
}
function init(f)
{	
	cfg2Form(f);
}
function preSubmit(f) {      
	var str="";
	if (!verifyIP2(f.LANIP,_("IP address"))) return ;
	if ( !tenda_ipMskChk(f.LANMASK,_("Subnet Mask"), f.LANIP)) return ;	
	
	if(!same_net_with_lan(f)){
		str+="The segment of the router IP has been changed,please re-configure the related parameter!";
	}
	str+=_("If the webpage doesn't update automatically,please update your computer's network settings and then log on with the new IP.")
	form2Cfg(f);
	if (window.confirm(str))
	{
		f.submit();				
	}else
		return;
}

</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(LanSet);" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top"><table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
                <tr>
                  <td align="center" valign="top"><form name=LanSet method=POST action=/goform/AdvSetLanip>
                      <input type=hidden name=GO value=lan.asp> 
					  <table cellpadding="0" cellspacing="0" class="content2">                       
					    <tr>
                          <td colspan=2 align="left" valign="top">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <script>document.write(_("This page is used to set the basic network parameters for LAN"));</script></td>
                        </tr>
					  </table>
                      <table cellpadding="0" cellspacing="0" class="content1" id="table1" style="margin-top:0px;">
                        <tr>
                          <td width=150 align="right"><script>document.write(_("LAN MAC address"));</script></td>
                          <td>&nbsp;&nbsp;&nbsp;&nbsp;<%aspTendaGetStatus("sys","lanmac");%></td>
                        </tr>
                        <tr>
                          <td width="150" align="right"><script>document.write(_("IP address"));</script></td>
                          <td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text maxlength=15 name=LANIP size=15 ></td>
                        </tr>
                        <tr>
                          <td width="150" align="right"><script>document.write(_("Subnet Mask"));</script></td>
                          <td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text maxlength=15 name=LANMASK size=15></td>
                        </tr>
                      </table>
                    <script>tbl_tail_save("document.LanSet");</script>
                  </form></td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("Lan_helpinfo"));</script>		
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>


