<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>LAN | LAN Settings</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
def_WANT="<%aspTendaGetStatus("wan","connecttype");%>";
def_PUN="<%aspTendaGetStatus("ppoe","userid");%>";
def_PPW="<%aspTendaGetStatus("ppoe","pwd");%>";
def_PIDL="<%aspTendaGetStatus("ppoe","idletime");%>";
def_PCM="<%aspTendaGetStatus("ppoe","conmode");%>";
def_MTU="<%aspTendaGetStatus("ppoe","mtu");%>";
def_SVC="<%aspTendaGetStatus("ppoe","sev");%>";
def_AC="<%aspTendaGetStatus("ppoe","ac");%>";
def_pppoeAdrMode="<%aspTendaGetStatus("ppoe","pppoeAdrMode");%>";//new
def_pppoeIP="<%aspTendaGetStatus("ppoe","pppoeWANIP");%>";//new
def_pppoeMSK="<%aspTendaGetStatus("ppoe","pppoeWANMSK");%>";//new
def_pppoeGW="<%aspTendaGetStatus("ppoe","pppoeWANGW");%>";//new
def_WANIP="<%aspTendaGetStatus("wan","wanip");%>";
def_WANMSK="<%aspTendaGetStatus("wan","wanmask");%>";
def_WANGW="<%aspTendaGetStatus("wan","staticgateway");%>";
def_DNS1="<%aspTendaGetStatus("wan","dns1");%>";
def_DNS2="<%aspTendaGetStatus("wan","dns2");%>";
def_l2tpSIP="<%aspTendaGetStatus("wan","l2tpIP");%>";
def_l2tpPUN="<%aspTendaGetStatus("wan","l2tpPUN");%>";
def_l2tpPPW="<%aspTendaGetStatus("wan","l2tpPPW");%>";
def_l2tpMTU="<%aspTendaGetStatus("wan","l2tpMTU");%>";
def_l2tpAdrMode="<%aspTendaGetStatus("wan","l2tpAdrMode");%>";
def_l2tpIP="<%aspTendaGetStatus("wan","l2tpWANIP");%>";
def_l2tpMSK="<%aspTendaGetStatus("wan","l2tpWANMSK");%>";
def_l2tpWGW="<%aspTendaGetStatus("wan","l2tpWANGW");%>";
def_pptpSIP="<%aspTendaGetStatus("wan","pptpIP");%>";
def_pptpPUN="<%aspTendaGetStatus("wan","pptpPUN");%>";
def_pptpPPW="<%aspTendaGetStatus("wan","pptpPPW");%>";
def_pptpMTU="<%aspTendaGetStatus("wan","pptpMTU");%>";
def_pptpAdrMode="<%aspTendaGetStatus("wan","pptpAdrMode");%>";
def_pptpIP="<%aspTendaGetStatus("wan","pptpWANIP");%>";
def_pptpMSK="<%aspTendaGetStatus("wan","pptpWANMSK");%>";
def_pptpWGW="<%aspTendaGetStatus("wan","pptpWANGW");%>";
def_pptpMPPE="<%aspTendaGetStatus("wan","pptpMPPE");%>";
def_dynamicMTU="<%aspTendaGetStatus("wan","dynamicMTU");%>";
def_staticMTU="<%aspTendaGetStatus("wan","staticMTU");%>";
def_WANT = parseInt(def_WANT) -1;
addCfg("WANT",34,def_WANT);
addCfg("PUN", 50, def_PUN);
addCfg("PPW", 54, def_PPW );
addCfg("PIDL", 53, def_PIDL);
addCfg("PCM", 55, def_PCM );
addCfg("MTU", 56, def_MTU );
addCfg("SVC", 57, def_SVC);
addCfg("AC", 58, def_AC);
addCfg("l2tpIP",41,def_l2tpSIP);
addCfg("l2tpPUN",42,def_l2tpPUN);
addCfg("l2tpPPW",43,def_l2tpPPW);
addCfg("l2tpMTU",44,def_l2tpMTU);
addCfg("l2tpAdrMode",45,def_l2tpAdrMode);
addCfg("l2tpWANIP",46,def_l2tpIP);
addCfg("l2tpWANMSK",47,def_l2tpMSK);
addCfg("l2tpWANGW",48,def_l2tpWGW);
addCfg("pptpIP",49,def_pptpSIP);
addCfg("pptpPUN",50,def_pptpPUN);
addCfg("pptpPPW",51,def_pptpPPW);
addCfg("pptpMTU",52,def_pptpMTU);
addCfg("pptpAdrMode",53,def_pptpAdrMode);
addCfg("pptpWANIP",54,def_pptpIP);
addCfg("pptpWANMSK",55,def_pptpMSK);
addCfg("pptpWANGW",56,def_pptpWGW);
addCfg("pppoeAdrMode",61,def_pppoeAdrMode);
addCfg("pppoeWANIP",62,def_pppoeIP);
addCfg("pppoeWANMSK",63,def_pppoeMSK);
addCfg("pppoeWANGW",64,def_pppoeGW);
addCfg("mppeEn",56,def_pptpMPPE);
addCfg("dynamicMTU",52,def_dynamicMTU);
addCfg("staticMTU",52,def_staticMTU);
addCfg("WANIP",31,def_WANIP);
addCfg("WANMSK",32,def_WANMSK);
addCfg("WANGW",33,def_WANGW);
addCfg("DS1",1,def_DNS1);
addCfg("DS2",2,def_DNS2);
Butterlate.setTextDomain("wan_set");
var page=0;
var illegal_user_pass = new Array("\\r","\\n","\\","'","\"");
function init(f)
{
	m=getCfg("WANT");
	document.frmSetup.WANT1.value=m;
	if(m == 9){
		m = 3;
	}
	if (m<9 && m>=0)
	{
		/*if(m==8)
		{
			document.getElementById("wan_sec").innerHTML= pages[5];
		}*/
		document.getElementById("wan_sec").innerHTML= pages[m];
	}
	cfg2Form(f);
	if(m == 4)
		onl2tpArdMode(f);
	else if(m == 3)
		onpptpArdMode(f);    
	else if(m == 8)
		onpppoeArdMode(f);
    document.getElementById("message").innerHTML=help[m];
    table_onload1('table1');
}

/*huang add*/
function chgPPW(val)
{
	if(document.getElementById("PPW").type == "password"){
		document.getElementById("td_PPW").innerHTML='&nbsp;&nbsp;&nbsp;&nbsp;<input name=PPW id=PPW maxLength=50 type=text class=text size=25 onBlur="chgPPW(this.value)" value='+val+'>';
		document.getElementById("PPW").focus();
		document.getElementById("PPW").focus();
		document.getElementById("PPW").value=val;
		}
	else if(document.getElementById("PPW").type == "text"){
		document.getElementById("td_PPW").innerHTML='&nbsp;&nbsp;&nbsp;&nbsp;<input name=PPW id=PPW maxLength=50 type=password class=text size=25 onFocus="chgPPW(this.value)" value='+val+'>';
		}
}
/*end add*/

function ispSelectChange(f)
{
 var m=document.frmSetup.WANT1.value;
 if(m == 9)
 	m = 3;
 document.getElementById("wan_sec").innerHTML= pages[m];
 document.getElementById("message").innerHTML=help[m];
 	cfg2Form(f);
	if(m == 4)
		onl2tpArdMode(f);
	else if(m == 3)
		onpptpArdMode(f);
	else if(m == 8)
		onpppoeArdMode(f);
 table_onload1('table1');
}
function preSubmit(f) {
	var m=document.frmSetup.WANT1.value;
	var mtu;
	if (m==2)
	{
		var da = new Date();
		f.v12_time.value = da.getTime()/1000;
		mtu = f.MTU.value;
		if(f.PCM[1].checked)
		{
			if (!rangeCheck(f.PIDL,60,3600,_("Idle time"))) return ;
		}	
		if(f.PUN.value == "" || f.PPW.value == "")
		{
			alert(_("Username or Password is empty"));
			return ;
		}
		if(!ill_check(f.PUN.value,illegal_user_pass,_("Account"))) return;
		if(!ill_check(f.PPW.value,illegal_user_pass,_("Password"))) return;
		if (!chkStrLen(f.PUN,0,255,_("Username"))) return ;
		if (!chkStrLen(f.PPW,0,255,_("Password"))) return ;
		if(f.PCM[3].checked)
		{
			if (!rangeCheck(f.hour1,0,23,_("Time"))) return ;
			if (!rangeCheck(f.minute1,0,59,_("Time"))) return ;
			if (!rangeCheck(f.hour2,0,24,_("Time"))) return ;
			if (!rangeCheck(f.minute2,0,59,_("Time"))) return ;
			if((Number(f.hour1.value)*60+Number(f.minute1.value) > 1440)||(Number(f.hour2.value)*60+Number(f.minute2.value) > 1440))
			{
				alert(_("Time exceeds the specified range"));
				return;
			}
			if((Number(f.hour1.value)*60+Number(f.minute1.value)) >= (Number(f.hour2.value)*60+Number(f.minute2.value)))
			{
				alert(_("start time must be smaller than end time."));
				return;
			}
		}	
		setCfg("PST",Number(f.hour1.value)*60+Number(f.minute1.value));
		setCfg("PET",Number(f.hour2.value)*60+Number(f.minute2.value));
	}
	else if (m==8)
	{
		mtu = f.MTU.value;
		if(f.PUN.value == "" || f.PPW.value == "")
		{
			alert(_("Username or Password is empty"));
			return ;
		}
		if(!ill_check(f.PUN.value,illegal_user_pass,_("Account"))) return;
		if(!ill_check(f.PPW.value,illegal_user_pass,_("Password"))) return;
		if (!chkStrLen(f.PUN,0,255,_("Username"))) return ;
		if (!chkStrLen(f.PPW,0,255,_("Password"))) return ;
		if(f.pppoeAdrMode.value == "1")
		{
			if (!verifyIP2(f.pppoeWANIP,_("IP address"))) return false;
			if (!ipMskChk(f.pppoeWANMSK,_("Subnet Mask"))) return false;
			
		}
	}
	else if (m==0)
	{
		mtu = f.staticMTU.value;
		if (!verifyIP2(f.WANIP,_("IP address"))) return ;
		if (!ipMskChk(f.WANMSK,_("Subnet Mask"))) return ;
		if (!verifyIP2(f.WANGW,_("Gateway"))) return ;
		if (!verifyIP2(f.DS1,_("Primary DNS"))) return ;
		if (!verifyIP0(f.DS2,_("Alternate DNS"))) return ;
		f.WANIP.value = clearInvalidIpstr(f.WANIP.value);
		f.WANMSK.value = clearInvalidIpstr(f.WANMSK.value);
		f.WANGW.value = clearInvalidIpstr(f.WANGW.value);
		f.DS1.value = clearInvalidIpstr(f.DS1.value);
		f.DS2.value = clearInvalidIpstr(f.DS2.value);	
	}
	else if (m == 4)
	{
		mtu = f.l2tpMTU.value;
		if(f.elements['l2tpPUN'].value == "" || f.elements['l2tpPPW'].value == "")
		{
			alert(_("Username or Password is empty"));
			return ;
		}
		if(!ill_check(f.elements['l2tpPUN'].value,illegal_user_pass,_("Username"))) return;	
		if(!ill_check(f.elements['l2tpPPW'].value,illegal_user_pass,_("Password"))) return;
		if(f.l2tpIP.value == "")
		{
			alert(_("Server address is empty"));
			return ;
		}
		if(!ill_check(f.l2tpIP.value,illegal_user_pass,_("Server address"))) return;
		
		if(f.l2tpAdrMode.value == "1")
		{
			if (!verifyIP2(f.l2tpWANIP,_("IP address"))) return false;
			if (!ipMskChk(f.l2tpWANMSK,_("Subnet Mask"))) return false;
			if (f.l2tpWANGW.value != "" && !verifyIP2(f.l2tpWANGW,_("ISP Gateway"))) return false;
		}
		f.l2tpWANIP.value = clearInvalidIpstr(f.l2tpWANIP.value);
		f.l2tpWANMSK.value = clearInvalidIpstr(f.l2tpWANMSK.value);
		f.l2tpWANGW.value = clearInvalidIpstr(f.l2tpWANGW.value);
	}
	else if (m == 3 || m == 9)
	{
		mtu = f.pptpMTU.value;
		if(f.elements['pptpPUN'].value == "" || f.elements['pptpPPW'].value == "")
		{
			alert(_("Username or Password is empty"));
			return ;
		}
		if(!ill_check(f.elements['pptpPUN'].value,illegal_user_pass,_("Username"))) return;	
	  if(!ill_check(f.elements['pptpPPW'].value,illegal_user_pass,_("Password"))) return;
		if(f.pptpIP.value == "")
		{
			alert(_("Server address is empty"));
			return ;
		}
		if(!ill_check(f.pptpIP.value,illegal_user_pass,_("Server address"))) return;

		if(f.pptpAdrMode.value == "1")
		{
			if (!verifyIP2(f.pptpWANIP,_("IP address"))) return false;
			if (!ipMskChk(f.pptpWANMSK,_("Subnet Mask"))) return false;
			if (f.pptpWANGW.value != "" && !verifyIP2(f.pptpWANGW,_("ISP Gateway"))) return false;
		}
		f.pptpWANIP.value = clearInvalidIpstr(f.pptpWANIP.value);
		f.pptpWANMSK.value = clearInvalidIpstr(f.pptpWANMSK.value);
		f.pptpWANGW.value = clearInvalidIpstr(f.pptpWANGW.value);
	}
	else if(m == 1)
	{
		mtu = f.dynamicMTU.value;
	}
	
	if(m != 9){
		if (!IsNumCheck(mtu)) return ;
		if(parseInt(mtu,10) < 256 || parseInt(mtu,10) > 1500)
		{
			alert(_("MTU value range")+":256~1500");
			return ;
		}
	}	
	form2Cfg(f);
	document.getElementById("WANT2").value = parseInt(m) + 1;
	f.submit();
} 
function onpppoeArdMode(f)
{
	if(f.pppoeAdrMode.selectedIndex == 0){
		f.pppoeWANIP.disabled = false;
		f.pppoeWANMSK.disabled = false;
		
	}else{
		f.pppoeWANIP.disabled = true;
		f.pppoeWANMSK.disabled = true;
		
	}
}
function onl2tpArdMode(f) 
{ 
	if(f.l2tpAdrMode.selectedIndex == 1){
		f.l2tpWANIP.disabled = false;
		f.l2tpWANMSK.disabled = false;
		f.l2tpWANGW.disabled = false;
	}
	else{
		f.l2tpWANIP.disabled = true;
		f.l2tpWANMSK.disabled = true;
		f.l2tpWANGW.disabled = true;
	}
}
function onpptpArdMode(f)
{
	if(f.pptpAdrMode.selectedIndex == 1){
		f.pptpWANIP.disabled = false;
		f.pptpWANMSK.disabled = false;
		f.pptpWANGW.disabled = false;
	}else{
		f.pptpWANIP.disabled = true;
		f.pptpWANMSK.disabled = true;
		f.pptpWANGW.disabled = true;
	}
}
<!-- mod by nhj -->
var help=new Array('&nbsp;&nbsp;&nbsp;&nbsp;'+_("Wan_connected_helpinfo1"),'&nbsp;&nbsp;&nbsp;&nbsp;'+_("Wan_connected_helpinfo2"),'&nbsp;&nbsp;&nbsp;&nbsp;PPPOE:'+_("Wan_connected_helpinfo3"),'&nbsp;&nbsp;&nbsp;&nbsp;PPTP:'+_("Wan_connected_helpinfo4"),'&nbsp;&nbsp;&nbsp;&nbsp;L2TP:'+_("Wan_connected_helpinfo5"),'','','','&nbsp;&nbsp;&nbsp;&nbsp;PPPOE Russia:'+_("Wan_connected_helpinfo6"));
var pages=new Array(
'<TABLE class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<TR><TD width="100" align="right">'+_("IP address")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=WANIP size=16></TD></TR>\
	<TR><TD width="100"  align="right">'+_("Subnet Mask")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=WANMSK size=16></TD></TR>\
	<TR><TD width="100"  align="right">'+_("Gateway")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=WANGW size=16></TD></TR>\
	<TR><TD width="100"  height=21 align="right">'+_("DNS server")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=DS1 size=16></TD></TR>\
	<TR><TD width="100"  align="right">'+_("Alternate DNS server")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=DS2 size=16>&nbsp;('+_("Optional")+')</TD></TR>\
	<tr><td width="100"  align="right" valign="top">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=staticMTU size=4 maxlength="4" value="1500">&nbsp;('+_("DO NOT modify it unless necessary, the default is 1500")+')</td></tr>\
	</TABLE>'

,'<table class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;" >\
		<tr><td width="100" align="right" valign="top">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=dynamicMTU size=4 maxlength="4" value="1500">&nbsp;('+_("DO NOT modify it unless necessary, the default is 1500")+')</td></tr>\
	</table>'

,'<table  class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<tr><td width="100" align="right">'+_("Access Account")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=PUN id=PUN maxLength=50 class=text size=25></td></tr>\
	<tr><td width="100" align="right">'+_("Access Password")+'</td>\
		<td id=td_PPW>&nbsp;&nbsp;&nbsp;&nbsp;<input name=PPW id=PPW maxLength=50 type=password class=text size=25 onFocus="chgPPW(this.value)"></td></tr>\
	<tr><td width="100" align="right" valign="top">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=MTU size=4 value="1492">&nbsp;('+_("DO NOT modify it unless necessary, the default is 1492")+')</td></tr>\
	<tr><td width="100" align="right">'+_("Service name")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=SVC maxLength=50 size=25>&nbsp;('+_("Don not enter the information unless necessary.")+')</td></tr>\
	<tr><td width="100" align="right">'+_("Server name")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=AC maxLength=50 size=25>&nbsp;('+_("Don not enter the information unless necessary.")+')</td></tr>\
	<TR><TD colSpan=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+_("Select the corresponding connection mode according to your situation.")+':</TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=0>'+_("Connect automatically, Connect automatically to the Internet after rebooting the system or connection failure.")+'</TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=1>'+_("Connect on demand: Re-establish your connection to the Internet when there's data transmitting.")+'</TD></TR>\
	<TR><td colspan=2 height=30> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+_("Max.idle time")+' \
				<INPUT class=text maxLength=4 name=PIDL size=4> (60-3600 '+_("Second")+') </TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=2>'+_("Connect manually: Connect to the Internet manually by the user.")+'</TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=3>'+_("Connect on fixed time: Connect automatically to the Internet during the time you fix.")+'</TD></TR>\
	<TR><td colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+_("Wan_connected_Note")+'</TD></TR>\
	<TR><td colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+_("Connection time: from ")+'  \
		<INPUT class=text maxLength=2 name=hour1 size=2 value="<%aspTendaGetStatus("ppoe","h_s");%>"> '+_("hours")+' \
		<INPUT class=text maxLength=2 name=minute1 size=2 value="<%aspTendaGetStatus("ppoe","m_s");%>"> '+_("minutes")+" "+_("to")+' \
		<INPUT class=text maxLength=2 name=hour2 size=2 value="<%aspTendaGetStatus("ppoe","h_e");%>"> '+_("hours")+' \
		<INPUT class=text maxLength=2 name=minute2 size=2 value="<%aspTendaGetStatus("ppoe","m_e");%>"> '+_("minutes")+'</TD></TR>\
	</table>'
	
,'<TABLE class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<TR><TD width=105 align="right">PPTP '+_("Server address")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="pptpIP" size=25 maxlength="64"></TD></TR>\
	<TR><TD align="right">'+_("Username")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="pptpPUN" size=25></TD></TR>\
	<TR><TD align="right">'+_("Password")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="pptpPPW" size=25 type=password></TD></TR>\
	<TR><TD align="right">MTU</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="pptpMTU" size=23 value="1492"></TD></TR>\
	<TR><TD align="right">'+_("Address mode")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<SELECT name="pptpAdrMode" onChange="onpptpArdMode(document.frmSetup)"><option value="0">Dynamic</option>\
			<option value="1">Static</option>\
			</SELECT></TD></TR>\
	<TR><TD align="right">'+_("IP address")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pptpWANIP" maxlength="15" size="15"></TD></TR>\
	<TR><TD align="right">'+_("Subnet Mask")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pptpWANMSK" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">'+_("Gateway")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pptpWANGW" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">MPPE</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT type="checkbox" name="mppeEn" value="1"></TD></TR>\
	</TABLE>'
	,'<TABLE class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<TR><TD align="right" width=105>L2TP '+_("Server address")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="l2tpIP" size="25" maxlength="64"></TD></TR>\
	<TR><TD align="right">'+_("Username")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="l2tpPUN" size=25></TD></TR>\
	<TR><TD align="right">'+_("Password")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="l2tpPPW" size=25 type=password></TD></TR>\
	<TR><TD align="right">MTU</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="l2tpMTU" size=23 value="1492"></TD></TR>\
	<TR><TD align="right">'+_("Address mode")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<SELECT name="l2tpAdrMode" id="l2tpAdrMode" onChange="onl2tpArdMode(document.frmSetup)"><option value="0">Dynamic</option>\
			<option value="1">Static</option>\
			</SELECT></TD></TR>\
	<TR><TD align="right">'+_("IP address")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="l2tpWANIP" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">'+_("Subnet Mask")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="l2tpWANMSK" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">'+_("Gateway")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="l2tpWANGW" size="15" maxlength="15"></TD></TR>\
	</TABLE>'
	
	,'<table id="table1"></table>'
	,'<table id="table1"></table>'
	,'<table id="table1"></table>'
	
	,'<table  class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<tr><td width="100" align="right">'+_("Access Account")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=PUN maxLength=50 class=text size=25></td></tr>\
	<tr><td width="100" align="right">'+_("Access Password")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input type=password maxLength=50 class=text name=PPW size=25></td></tr>\
	<tr><td width="100" align="right">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=MTU size=4 value="1492">&nbsp;('+_("Do not modify it unless necessary, the default is 1492")+')</td></tr>\
		<tr><td width="100" align="right">'+_("Service name")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=SVC maxLength=50 size=25>&nbsp;('+_("Do not enter the information unless necessary.")+')</td></tr>\
	<tr><td width="100" align="right">'+_("Server name")+'</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=AC maxLength=50 size=25>&nbsp;('+_("Do not enter the information unless necessary.")+')</td></tr>\
		<TR><TD align="right">'+_("Address mode")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<SELECT name="pppoeAdrMode" id="pppoeAdrMode" onChange="onpppoeArdMode(document.frmSetup)"><option value="1">Static</option>\
			<option value="0">Dynamic</option>\
			</SELECT></TD></TR>\
	<TR><TD align="right">'+_("IP address")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pppoeWANIP" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">'+_("Subnet Mask")+'</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pppoeWANMSK" size="15" maxlength="15"></TD></TR>\
		<tr><td width="100" align="right">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=dynamicMTU size=4 maxlength="4" value="1500">&nbsp;('+_("Do not modify it unless necessary, the default is 1500")+')</td></tr>\
	</table>'	
);
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0"  onLoad="init(document.frmSetup);" class="bg">
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
			<FORM name=frmSetup method=POST action=/goform/AdvSetWan>
			<INPUT type=hidden name=GO value=wan_connectd.asp >
			<input type="hidden" id="rebootTag" name="rebootTag">
			<input type="hidden" id="v12_time" name="v12_time">
			<input type="hidden" name="WANT2" id="WANT2">
			<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" class="content1" id="table2">
			  <tr>
				<td width="100" align="right"><script>document.write(_("mode"));</script></td>
			    <td>&nbsp;&nbsp;&nbsp;&nbsp;<select name="WANT1" id="WANT1" onChange="ispSelectChange(document.frmSetup)">
					<option value=2>PPPOE</option>
                    <option value=8>PPPOE Russia</option>                    
					<option value=0><script>document.write(_("Static IP"));</script></option>
					<option value=1>DHCP</option>
					<option value=3>PPTP</option>
                    <option value=9>PPTP Russia</option>
					<!--<option value=4>L2TP</option>-->
				 </select>
				 </td>
			  </tr>
			</table>
				<div id=wan_sec style="visibility:visible;">
				</div>  
				<SCRIPT>tbl_tail_save('document.frmSetup')</SCRIPT>
          </FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
	<table border="0" cellpadding="0" cellspacing="0"  class="left1" style="margin-top:25px;">
	<tr><td align="center"><b><script>document.write(_("Help information"));</script></b></td></tr>
	<tr><td><div id="message"></div></td></tr>
	</table>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table2');
    </script>
</BODY>
</HTML>





