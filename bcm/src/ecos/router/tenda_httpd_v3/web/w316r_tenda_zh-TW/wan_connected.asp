<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>WAN | WAN Settings</TITLE>
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
addCfg("mppeEn",56,def_pptpMPPE);
addCfg("dynamicMTU",52,def_dynamicMTU);
addCfg("staticMTU",52,def_staticMTU);
addCfg("WANIP",31,def_WANIP);
addCfg("WANMSK",32,def_WANMSK);
addCfg("WANGW",33,def_WANGW);
addCfg("DS1",1,def_DNS1);
addCfg("DS2",2,def_DNS2);
var page=0;

var illegal_user_pass = new Array("\\r","\\n","\\","'","\"");

function init(f)
{
	m=getCfg("WANT");
	document.frmSetup.WANT1.value=m;
	if (m<7 && m>=0)
	{
		document.getElementById("wan_sec").innerHTML= pages[m];
	}
	ispSelectChange(f);     //weige modify
}

function ispSelectChange(f)
{
 var m=document.frmSetup.WANT1.value;
 document.getElementById("wan_sec").innerHTML= pages[m];
 document.getElementById("message").innerHTML=help[m];
 	cfg2Form(f);
 //----------------weige modify----------------------
	if(f.l2tpAdrMode)
		f.l2tpAdrMode.value=def_l2tpAdrMode;
	if(f.pptpAdrMode)
		f.pptpAdrMode.value=def_pptpAdrMode;
 //------------end------------------
	if(m == 4)//l2tp
		onl2tpArdMode(f);
	else if(m == 3)//pptp
		onpptpArdMode(f);
 table_onload1('table1');
}
function preSubmit(f) {
	var m=document.frmSetup.WANT1.value;
	var mtu;
	if (m==2)//pppoe
	{
		var da = new Date();
		f.v12_time.value = da.getTime()/1000;
		mtu = f.MTU.value;
		if(f.PCM[1].checked)
		{
			if (!rangeCheck(f.PIDL,60,3600,"閒置時間")) return ;
		}	
		if(f.PUN.value == "" || f.PPW.value == "")
		{
			alert("使用者名稱或密碼不可以空白！");
			return ;
		}
		if(!ill_check(f.PUN.value,illegal_user_pass,"使用者名稱")) return;
		if(!ill_check(f.PPW.value,illegal_user_pass,"密碼")) return;
		if (!chkStrLen(f.PUN,0,255,"使用者名稱")) return ;
		if (!chkStrLen(f.PPW,0,255,"密碼")) return ;
		if(f.PCM[3].checked)
		{
			if (!rangeCheck(f.hour1,0,23,"時間")) return ;
			if (!rangeCheck(f.minute1,0,59,"時間")) return ;
			if (!rangeCheck(f.hour2,0,24,"時間")) return ;
			if (!rangeCheck(f.minute2,0,59,"時間")) return ;
			if((Number(f.hour1.value)*60+Number(f.minute1.value) > 1440)||(Number(f.hour2.value)*60+Number(f.minute2.value) > 1440))
			{
				alert("時間超過規定範圍！");
				return;
			}
			if((Number(f.hour1.value)*60+Number(f.minute1.value)) >= (Number(f.hour2.value)*60+Number(f.minute2.value)))
			{
				alert("開始時間必須小於結束時間！");
				return;
			}
		}	
		setCfg("PST",Number(f.hour1.value)*60+Number(f.minute1.value));
		setCfg("PET",Number(f.hour2.value)*60+Number(f.minute2.value));
	}
	else if (m==0)
	{
		mtu = f.staticMTU.value;
		if (!verifyIP2(f.WANIP,"IP 位址")) return ;
		if (!ipMskChk(f.WANMSK,"子網路遮罩")) return ;
		if (!verifyIP2(f.WANGW,"預設閘道位址")) return ;
		if (!verifyIP2(f.DS1,"主要DNS位址")) return ;
		if (!verifyIP0(f.DS2,"次要DNS位址")) return ;
	}
	else if (m == 4)
	{
		mtu = f.l2tpMTU.value;
		if(f.elements['l2tpPUN'].value == "" || f.elements['l2tpPPW'].value == "")
		{
			alert("使用者名稱或密碼不可以空白！");
			return ;
		}
		if(!ill_check(f.elements['l2tpPUN'].value,illegal_user_pass,"使用者名稱")) return;	
		if(!ill_check(f.elements['l2tpPPW'].value,illegal_user_pass,"密碼")) return;
		if(f.l2tpIP.value == "")
		{
			alert("伺服器位址不可以空白！");
			return ;
		}
		if(!ill_check(f.l2tpIP.value,illegal_user_pass,"伺服器位址")) return;
		
		if(f.l2tpAdrMode.value == "1")
		{
			if (!verifyIP2(f.l2tpWANIP,"IP 位址")) return false;
			if (!ipMskChk(f.l2tpWANMSK,"子網路遮罩")) return false;
			if (f.l2tpWANGW.value != "" && !verifyIP2(f.l2tpWANGW,"ISP 預設閘道位址")) return false;
		}
	}
	else if (m == 3)
	{
		mtu = f.pptpMTU.value;
		if(f.elements['pptpPUN'].value == "" || f.elements['pptpPPW'].value == "")
		{
			alert("使用者名稱或密碼不可以空白！");
			return ;
		}
		if(!ill_check(f.elements['pptpPUN'].value,illegal_user_pass,"使用者名稱")) return;	
	  if(!ill_check(f.elements['pptpPPW'].value,illegal_user_pass,"密碼")) return;
		if(f.pptpIP.value == "")
		{
			alert("伺服器位址不可以空白！");
			return ;
		}
		if(!ill_check(f.pptpIP.value,illegal_user_pass,"伺服器位址")) return;

		if(f.pptpAdrMode.value == "1")
		{
			if (!verifyIP2(f.pptpWANIP,"IP 位址")) return false;
			if (!ipMskChk(f.pptpWANMSK,"子網路遮罩")) return false;
			if (f.pptpWANGW.value != "" && !verifyIP2(f.pptpWANGW,"ISP 預設閘道位址")) return false;
		}
	}
	else if(m == 1)
	{
		mtu = f.dynamicMTU.value;
	}
	
	if(m != 7){
		if (!IsNumCheck(mtu)) return ;
		if(parseInt(mtu,10) < 256 || parseInt(mtu,10) > 1500)
		{
			alert("MTU值範圍：256~1500");
			return ;
		}
	}	
	form2Cfg(f);
	document.getElementById("WANT2").value = parseInt(m) + 1;
	f.submit();
} 
function onl2tpArdMode(f) 
{ 
	if(f.l2tpAdrMode.selectedIndex == 0){
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
	if(f.pptpAdrMode.selectedIndex == 0){
		f.pptpWANIP.disabled = false;
		f.pptpWANMSK.disabled = false;
		f.pptpWANGW.disabled = false;
	}else{
		f.pptpWANIP.disabled = true;
		f.pptpWANMSK.disabled = true;
		f.pptpWANGW.disabled = true;
	}
}
var help=new Array('&nbsp;&nbsp;&nbsp;&nbsp;固定 IP：如果您的ISP業者(如：Hinet、台灣大寬頻)提供固定IP(一般來說你會有一組IP參數，如：IP位址、子網路遮罩、預設閘道...等)，請選擇固定 IP模式。','&nbsp;&nbsp;&nbsp;&nbsp;浮動 IP：如果您的ISP業者提供浮動IP(通常電腦會自動取得IP位址，不須特別設定)，請選擇浮動 IP模式。','&nbsp;&nbsp;&nbsp;&nbsp;PPPoE：如果您的ISP業者提供PPPoE(通常會提供您一組使用者名稱與密碼)，請選擇PPPoE模式。','&nbsp;&nbsp;&nbsp;&nbsp;PPTP：輸入ISP提供的PPTP伺服器IP位址、使用者名稱、密碼，對於WAN埠IP 位址、子網路遮罩、預設閘道等參數，您可以選擇自動取得或者手動輸入ISP提供的參數。MTU：輸入網路離街的MTU值，如果不清楚該如何填寫，請使用預設值。','&nbsp;&nbsp;&nbsp;&nbsp;L2TP：輸入ISP提供的L2TP伺服器IP位址、使用者名稱、密碼，對於WAN埠IP 位址、子網路遮罩、預設閘道等參數，您可以選擇自動取得或者手動輸入ISP提供的參數。MTU：輸入網路離街的MTU值，如果不清楚該如何填寫，請使用預設值。');
var pages=new Array(
'<TABLE class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<TR><TD width="100" align="right">IP位址</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=WANIP size=16></TD></TR>\
	<TR><TD align="right">子網路遮罩</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=WANMSK size=16></TD></TR>\
	<TR><TD align="right">預設閘道</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=WANGW size=16></TD></TR>\
	<TR><TD height=21 align="right">主要DNS伺服器</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=DS1 size=16></TD></TR>\
	<TR><TD align="right">次要DNS伺服器</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=15 name=DS2 size=16>（選填）</TD></TR>\
	<tr><td align="right">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=staticMTU size=4 maxlength="4" value="1500">(如非必要，請勿變更，預設值1500)</td></tr>\
	</TABLE>'
,'<table class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;" >\
		<tr><td width="100" align="right">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=dynamicMTU size=4 maxlength="4" value="1500">(如非必要，請勿變更，預設值1500)</td></tr>\
	</table>'
,'<table  class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<tr><td width="100" align="right">使用者名稱</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input name=PUN maxLength=128 class=text size=25></td></tr>\
	<tr><td width="100" align="right">密碼</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input type=password maxLength=128 class=text name=PPW size=25></td></tr>\
	<tr><td width="100" align="right">MTU</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=MTU size=4 value="1492">(如非必要，請勿變更，預設值1492)</td></tr>\
	<tr><td width="100" align="right">服務名</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=SVC maxLength=50 size=25>(如非必要，請勿填寫)</td></tr>\
	<tr><td width="100" align="right">伺服器名稱</td>\
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<input class=text name=AC maxLength=50 size=25>(如非必要，請勿填寫)</td></tr>\
	<TR><TD colSpan=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;請依據您的需求，選擇相對應的連線模式：</TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=0>自動連線，在開機和斷線後自動進行連線。</TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=1>依需求連線，在有連線至網際網路的動作時，自動進行連線。</TD></TR>\
	<TR><td colspan=2 height=30> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;自動斷線 閒置時間： \
				<INPUT class=text maxLength=4 name=PIDL size=4> (60-3600,秒) </TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=2>手动连接，由用户手动进行连接。</TD></TR>\
	<TR><TD colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<INPUT name=PCM type=radio class="radio" value=3>定时连接，在指定的时段自动进行连接。</TD></TR>\
	<TR><td colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;請注意：只有當您到「系統工具」選單的「時間設定」項目<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;設定了目前的時間後，「定時連線」的功能才會生效。</TD></TR>\
	<TR><td colspan=2 height=30>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;連線時間：從 \
		<INPUT class=text maxLength=2 name=hour1 size=2 value="<%aspTendaGetStatus("ppoe","h_s");%>"> 時\
		<INPUT class=text maxLength=2 name=minute1 size=2 value="<%aspTendaGetStatus("ppoe","m_s");%>"> 分到 \
		<INPUT class=text maxLength=2 name=hour2 size=2 value="<%aspTendaGetStatus("ppoe","h_e");%>"> 時\
		<INPUT class=text maxLength=2 name=minute2 size=2 value="<%aspTendaGetStatus("ppoe","m_e");%>"> 分</TD></TR>\
	</table>'
	
,'<TABLE class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<TR><TD width=105 align="right">PPTP伺服器位址</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="pptpIP" size=15 maxlength="15"></TD></TR>\
	<TR><TD align="right">使用者名稱</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="pptpPUN" size=25></TD></TR>\
	<TR><TD align="right">密碼</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="pptpPPW" size=25 type=password></TD></TR>\
	<TR><TD align="right">MTU</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="pptpMTU" size=23 value="1492"></TD></TR>\
	<TR><TD align="right">位址模式</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<SELECT name="pptpAdrMode" onChange="onpptpArdMode(document.frmSetup)"><option value="1">Static</option>\
			<option value="2">Dynamic</option>\
			</SELECT></TD></TR>\
	<TR><TD align="right">IP位址</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pptpWANIP" maxlength="15" size="15"></TD></TR>\
	<TR><TD align="right">子網路遮罩</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pptpWANMSK" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">預設閘道</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="pptpWANGW" size="15" maxlength="15"></TD></TR>\
	</TABLE>'
	,'<TABLE class="content1" id=\"table1\" cellpadding="0" cellspacing="0" style="margin-top:0px;">\
	<TR><TD align="right" width=105>L2TP伺服器位址</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="l2tpIP" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">使用者名稱</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="l2tpPUN" size=25></TD></TR>\
	<TR><TD align="right">密碼</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text maxLength=50 name="l2tpPPW" size=25 type=password></TD></TR>\
	<TR><TD align="right">MTU</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class=text name="l2tpMTU" size=23 value="1492"></TD></TR>\
	<TR><TD align="right">位址模式</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<SELECT name="l2tpAdrMode" id="l2tpAdrMode" onChange="onl2tpArdMode(document.frmSetup)"><option value="1">Static</option>\
			<option value="2">Dynamic</option>\
			</SELECT></TD></TR>\
	<TR><TD align="right">IP位址</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="l2tpWANIP" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">子網路遮罩</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="l2tpWANMSK" size="15" maxlength="15"></TD></TR>\
	<TR><TD align="right">預設閘道</TD>\
		<TD>&nbsp;&nbsp;&nbsp;&nbsp;<INPUT class="text" name="l2tpWANGW" size="15" maxlength="15"></TD></TR>\
	</TABLE>'
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
				<td width="100" align="right">模&nbsp; 式</td>
			    <td>&nbsp;&nbsp;&nbsp;&nbsp;<select name="WANT1" id="WANT1" onChange="ispSelectChange(document.frmSetup)">
					<option value=2>PPPOE</option>
					<option value=0>固定IP</option>
					<option value=1>DHCP</option>
<!--
					<option value=3>PPTP</option>
					<option value=4>L2TP</option>
-->
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
	<tr><td align="center"><b>說明資訊</b></td></tr>
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
