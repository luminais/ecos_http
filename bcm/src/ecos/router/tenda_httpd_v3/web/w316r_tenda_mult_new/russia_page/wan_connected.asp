<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body class="wan-connected">
<FORM name="frmSetup" method="POST" action="/goform/AdvSetWan">
<input type="hidden" name="GO" value="wan_connectd.asp" >
<input type="hidden" id="rebootTag" name="rebootTag">
<input type="hidden" id="v12_time" name="v12_time">
<input type="hidden" name="WANT2" id="WANT2">
	<fieldset>
		<legend>Internet Connection Setup</legend>
		<div class="control-group">
			<label for="WANT1" class="control-label">Internet Connection Type</label>
			<div class="controls">
				<select name="WANT1" id="WANT1">
					<option value=2>PPPoE</option>
					<option value=8>PPPOE Russia</option>
					<option value=0>Static IP</option>
					<option value=1>DHCP</option>
					<option value=3>PPTP</option>
					<option value=9>PPTP Russia</option>
					<!--<option value=4>L2TP</option>-->
				</select>
			</div>
		</div>
		<div id="wan_sec">
			<!--<table id="type_dhcp" class="none">
				<tr>
					<td class="control-label">MTU</td>
					<td class="controls">
						<input type="text" class=text name=dynamicMTU size=4 maxlength="4" value="1500">
						<span class="help-block">(<span>The default value is 1500. Do not modify it unless required by your ISP.</span>)</span>
					</td>
				</tr>
			</table>-->
			
			<table id="type_static" class="none">
				<tr>
					<td class="control-label">IP Address</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=WANIP size=16></td>
				</tr>
				<tr>
					<td class="control-label">Subnet Mask</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=WANMSK size=16></td>
				</tr>
				<tr>
					<td class="control-label">Gateway</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=WANGW size=16></td>
				</tr>
				<tr>
					<td class="control-label">DNS Server</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=DS1 size=16></td>
				</tr>
				<tr>
					<td class="control-label">Alternate DNS Server</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=DS2 size=16>(<span>Optional</span>)</td>
				</tr>
				<tr>
					<td class="control-label">MTU</td>
					<td class="controls">
						<input type="text" class=text name=staticMTU size=4 maxlength="4" value="1500">
						<span class="help-block">(<span>The default value is 1500. Do not modify it unless required by your ISP.</span>)</span>
					</td>
				</tr>
			</table>
			
			<fieldset id="type_ppoe" class="table-field none">
				<div class="control-group">
					<label class="control-label">PPPoE Username</label>
					<div class="controls">
						<input type="text" name="PUN" id="PUN" maxLength="64" class="text">
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">PPPoE Password</label>
					<div id=td_PPW class="controls">
						<input name="PPW" id="PPW" maxLength="64" type="password" class="text">
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">MTU</label>
					<div class="controls">
						<input type="text" class="text" name="MTU" size="4" value="1492">
						<span class="help-block">(<span>The default value is 1492. Do not modify it unless required by your ISP.</span>)</span>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">Service Name</label>
					<div class="controls">
						<input type="text" class="text" name="SVC" maxLength=50 size=25>
						<span class="help-block">(<span>Only enter this information if instructed by ISP.</span>)</span>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">Server Name</label>
					<div class="controls">
						<input type="text" class="text" name="AC" maxLength=50 size=25>
						<span class="help-block">(<span>Only enter this information if instructed by ISP.</span>)</span>
					</div>
				</div>
				<div id="pppoe_all" class="none">
					<p class="">Select the corresponding connection mode according to your situation.</p>
					<div class="control-group">
						<div class="controls"><label class="radio"><input name="PCM" type="radio" value="0" />Connect automatically: Connect automatically to the Internet after rebooting the system or connection failure.</label></div>
					</div>
					<div class="control-group">
						<div class="controls"><label class="radio"><input name="PCM" type="radio"  value="1" />Connect on demand: Re-establish your connection to the Internet when there’s data transmitting.</label></div>
					</div>
					<div class="control-group">
						<label class="control-label">Max Idle Time</label>
						<div class="controls">
							<input type="text" class="text" maxLength="4" name="PIDL" size="4">
							<span class="help-block">60-3600 <span>seconds</span></span>
						</div>
					</div>
					<div class="control-group">
						<label class="control-label"></label>
						<div class="controls"><label class="radio"><input name="PCM" type="radio" value="2" />Connect Manually: Require the user to manually connect to the Internet before each session.</label></div>
					</div>
					<div class="control-group">
						<label class="control-label"></label>
						<div class="controls"><label class="radio"><input name="PCM" type="radio" value="3" />Connect During Specified Time Period: Connect automatically to Internet during a specified time length.</label></div>
					</div>
					<p class="text-red">Wan_connected_Note</p>
					<div>Connection Time: From  
						<input class="text input-mic-mini" maxLength=2 name="hour1" size=2 value='<%aspTendaGetStatus("ppoe","h_s");%>'> Hours
						<input class="text input-mic-mini" maxLength=2 name="minute1" size=2 value='<%aspTendaGetStatus("ppoe","m_s");%>'> Minutes
						<span>To</span> 
						<input class="text input-mic-mini" maxLength=2 name="hour2" size=2 value='<%aspTendaGetStatus("ppoe","h_e");%>'> Hours
						<input class="text input-mic-mini" maxLength=2 name="minute2" size=2 value='<%aspTendaGetStatus("ppoe","m_e");%>'> Minutes
					</div>
				</div>
				<div id="pppoe_russia">
					<div class="control-group">
					   <label class="control-label">Address Mode</label>
					   <div class="controls">
						 <select name="pppoeAdrMode" id="pppoeAdrMode" onChange="onpppoeArdMode(document.frmSetup)"><option value="1">Static</option>
						 <option value="0">Dynamic</option>
						 </select>
					    </div>
				    </div>
					<div class="control-group">
					   <label class="control-label">IP Address</label>
					   <div class="controls">
							<input type="text" class="text" name="pppoeWANIP" size="15" maxlength="15">
					   </div>
				    </div>
					<div class="control-group">
					   <label class="control-label">Subnet Mask</label>
					   <div class="controls">
							<input type="text" class="text" name="pppoeWANMSK" size="15" maxlength="15">
					   </div>
				    </div>
				</div>
			</fieldset>
			<table id="type_dhcp" class="none">
				<tr>
					<td class="control-label">MTU</td>
					<td class="controls">
						<input type="text" class=text name=dynamicMTU size=4 maxlength="4" value="1500">
						<span class="help-block">(<span>The default value is 1500. Do not modify it unless required by your ISP.</span>)</span>
					</td>
				</tr>
			</table>		
			<table id="type_pptp" class="none">
				<tr><td class="control-label">PPTP <span>Server Address</span></td>
					<td class="controls"><input type="text" class=text name="pptpIP" size="15" maxlength="15"></td></tr>
				<tr><td class="control-label">Username</td>
					<td class="controls"><input type="text" class=text maxLength=64 name="pptpPUN" size=25></td></tr>
				<tr><td class="control-label">Password</td>
					<td class="controls"><input class=text maxLength=64 name="pptpPPW" size=25 type=password></td></tr>
				<tr><td class="control-label">MTU</td>
					<td class="controls"><input type="text" class=text name="pptpMTU" size=23 value="1492"></td></tr>
				<tr><td class="control-label">Address Mode</td>
					<td class="controls"><select name="pptpAdrMode" id="pptpAdrMode" onChange="onpptpArdMode(document.frmSetup)"><option value="1">Static</option>
						<option value="0">Dynamic</option>
						</select></td></tr>
				<tr><td class="control-label">IP Address</td>
					<td class="controls"><input type="text" class="text" name="pptpWANIP" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">Subnet Mask</td>
					<td class="controls"><input type="text" class="text" name="pptpWANMSK" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">Gateway</td>
					<td class="controls"><input type="text" class="text" name="pptpWANGW" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">MPPE</td>
					<td class="controls">
					<label class="checkbox"><input type="checkbox" class="text" name="mppeEn" value="1"><label>
					</td></tr>
			</table>
			
			<table id="type_l2tp" class="none">
				<tr><td class="control-label">L2TP <span>Server Address</span></td>
					<td class="controls"><input type="text" class=text name="l2tpIP" size="15" maxlength="15"></td></tr>
				<tr><td class="control-label">Username</td>
					<td class="controls"><input type="text" class=text maxLength=64 name="l2tpPUN" size=25></td></tr>
				<tr><td class="control-label">Password</td>
					<td class="controls"><input type="password" class=text maxLength=64 name="l2tpPPW" size=25></td></tr>
				<tr><td class="control-label">MTU</td>
					<td class="controls"><input type="text" class=text name="l2tpMTU" size=23 value="1492"></td></tr>
				<tr><td class="control-label">Address Mode</td>
					<td class="controls"><select name="l2tpAdrMode" id="l2tpAdrMode" onChange="onl2tpArdMode(document.frmSetup)"><option value="1">Static</option>
						<option value="0">Dynamic</option>
						</select></td></tr>
				<tr><td class="control-label">IP Address</td>
					<td class="controls"><input type="text" class="text" name="l2tpWANIP" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">Subnet Mask</td>
					<td class="controls"><input type="text" class="text" name="l2tpWANMSK" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">Gateway</td>
					<td class="controls"><input type="text" class="text" name="l2tpWANGW" size="15" maxlength="15"></td>
				</tr>
			</table>
			
<!--			<table id="type_RussiaPPPoE" class="none">
				<tr><td class="control-label">PPPoE Username</td>
				    <td class="controls"><input type="text" class="text" maxlength="64" name="PUN" size="25"></td></tr>
				<tr><td class="control-label">PPPoE Password</td>
					<td class="controls"><input type="password" class=text maxLength=64 name="PPW" size=25></td></tr>
				<tr><td class="control-label">MTU</td>
					<td class="controls">
					<input type="text" class=text name="MTU" size=23 value="1492">
					<span class="help-block">(<span>The default value is 1492. Do not modify it unless required by your ISP.</span>)</span>
					</td></tr>
				<tr><td class="control-label">Service name</td>
					<td class="controls">
						<input type="text" class="text" name="SVC" maxLength=50 size=25>
						<span class="help-block">(<span>Only enter this information if instructed by ISP.</span>)</span>
					</td></tr>
				<tr><td class="control-label">Server name</td>
					<td class="controls">
						<input type="text" class="text" name="AC" maxLength=50 size=25>
						<span class="help-block">(<span>Only enter this information if instructed by ISP.</span>)</span>
					</td></tr>
				<tr><td class="control-label">Address Mode</td>
					<td class="controls"><select name="pppoeAdrMode" id="pppoeAdrMode" onChange="onpppoeArdMode(document.frmSetup)"><option value="1">Static</option>
						<option value="0">Dynamic</option>
						</select></td></tr>
				<tr><td class="control-label">IP Address</td>
					<td class="controls"><input type="text" class="text" name="pppoeWANIP" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">Subnet Mask</td>
					<td class="controls"><input type="text" class="text" name="pppoeWANMSK" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">MTU</td>
					<td class="controls">
					<input type="text" class="text" name="dynamicMTU" size="4" value="1500">
					<span class="help-block">(<span>The default value is 1500. Do not modify it unless required by your ISP.</span>)</span>
					</td></tr>
			</table>
-->		</div>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.frmSetup)" />
	</div>
</form>
<div id="save" class="none"></div>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "advanced"]);
	B.translate();
})();
</script>
<script src="js/libs/tenda.js" type="text/javascript"></script>
<script src="js/gozila.js" type="text/javascript"></script>
<script>
var def_WANT="<%aspTendaGetStatus("wan","connecttype");%>",
	def_PUN="<%aspTendaGetStatus("ppoe","userid");%>",
	def_PPW="<%aspTendaGetStatus("ppoe","pwd");%>",
	def_PIDL="<%aspTendaGetStatus("ppoe","idletime");%>",
	def_PCM="<%aspTendaGetStatus("ppoe","conmode");%>",
	def_MTU="<%aspTendaGetStatus("ppoe","mtu");%>",
	def_SVC="<%aspTendaGetStatus("ppoe","sev");%>",
	def_AC="<%aspTendaGetStatus("ppoe","ac");%>",
	def_pppoeAdrMode="<%aspTendaGetStatus("ppoe","pppoeAdrMode");%>",//new
	def_pppoeIP="<%aspTendaGetStatus("ppoe","pppoeWANIP");%>",//new
	def_pppoeMSK="<%aspTendaGetStatus("ppoe","pppoeWANMSK");%>",//new
	def_pppoeGW="<%aspTendaGetStatus("ppoe","pppoeWANGW");%>",//new
	def_WANIP="<%aspTendaGetStatus("wan","wanip");%>",
	def_WANMSK="<%aspTendaGetStatus("wan","wanmask");%>",
	def_WANGW="<%aspTendaGetStatus("wan","staticgateway");%>",
	def_DNS1="<%aspTendaGetStatus("wan","dns1");%>",
	def_DNS2="<%aspTendaGetStatus("wan","dns2");%>",
	def_l2tpSIP="<%aspTendaGetStatus("wan","l2tpIP");%>",
	def_l2tpPUN="<%aspTendaGetStatus("wan","l2tpPUN");%>",
	def_l2tpPPW="<%aspTendaGetStatus("wan","l2tpPPW");%>",
	def_l2tpMTU="<%aspTendaGetStatus("wan","l2tpMTU");%>",
	def_l2tpAdrMode="<%aspTendaGetStatus("wan","l2tpAdrMode");%>",
	def_l2tpIP="<%aspTendaGetStatus("wan","l2tpWANIP");%>",
	def_l2tpMSK="<%aspTendaGetStatus("wan","l2tpWANMSK");%>",
	def_l2tpWGW="<%aspTendaGetStatus("wan","l2tpWANGW");%>",
	def_pptpSIP="<%aspTendaGetStatus("wan","pptpIP");%>",
	def_pptpPUN="<%aspTendaGetStatus("wan","pptpPUN");%>",
	def_pptpPPW="<%aspTendaGetStatus("wan","pptpPPW");%>",
	def_pptpMTU="<%aspTendaGetStatus("wan","pptpMTU");%>",
	def_pptpAdrMode="<%aspTendaGetStatus("wan","pptpAdrMode");%>",
	def_pptpIP="<%aspTendaGetStatus("wan","pptpWANIP");%>",
	def_pptpMSK="<%aspTendaGetStatus("wan","pptpWANMSK");%>",
	def_pptpWGW="<%aspTendaGetStatus("wan","pptpWANGW");%>",
	def_pptpMPPE="<%aspTendaGetStatus("wan","pptpMPPE");%>",
	def_dynamicMTU="<%aspTendaGetStatus("wan","dynamicMTU");%>",
	def_staticMTU="<%aspTendaGetStatus("wan","staticMTU");%>";
def_WANT = parseInt(def_WANT) -1;
addCfg("WANT",32,def_WANT);
addCfg("PUN", 31, def_PUN);
addCfg("PPW", 30, def_PPW );
addCfg("PIDL", 29, def_PIDL);
addCfg("PCM", 28, def_PCM );
addCfg("MTU", 27, +def_MTU );
addCfg("SVC", 26, def_SVC);
addCfg("AC", 25, def_AC);
addCfg("l2tpIP",24,def_l2tpSIP);
addCfg("l2tpPUN",23,def_l2tpPUN);
addCfg("l2tpPPW",22,def_l2tpPPW);
addCfg("l2tpMTU",21,+def_l2tpMTU);
addCfg("l2tpAdrMode",20,def_l2tpAdrMode);
addCfg("l2tpWANIP",19,def_l2tpIP);
addCfg("l2tpWANMSK",18,def_l2tpMSK);
addCfg("l2tpWANGW",17,def_l2tpWGW);
addCfg("pptpIP",16,def_pptpSIP);
addCfg("pptpPUN",15,def_pptpPUN);
addCfg("pptpPPW",14,def_pptpPPW);
addCfg("pptpMTU",13,+def_pptpMTU);
addCfg("pptpAdrMode",12,def_pptpAdrMode);
addCfg("pptpWANIP",11,def_pptpIP);
addCfg("pptpWANMSK",10,def_pptpMSK);
addCfg("pptpWANGW",9,def_pptpWGW);
addCfg("pppoeAdrMode",33,def_pppoeAdrMode);
addCfg("pppoeWANIP",34,def_pppoeIP);
addCfg("pppoeWANMSK",35,def_pppoeMSK);
addCfg("pppoeWANGW",36,def_pppoeGW);
addCfg("mppeEn",8,def_pptpMPPE);
addCfg("dynamicMTU",7,+def_dynamicMTU);
addCfg("staticMTU",6,+def_staticMTU);
addCfg("WANIP",5,def_WANIP);
addCfg("WANMSK",4,def_WANMSK);
addCfg("WANGW",3,def_WANGW);
addCfg("DS1",1,def_DNS1);
addCfg("DS2",2,def_DNS2);
var page=0,
	illegal_user_pass = new Array("\\r","\\n","\\","'","\"");

function onl2tpArdMode(f) { 
	if(f.l2tpAdrMode.selectedIndex == 0){
		f.l2tpWANIP.disabled = false;
		f.l2tpWANMSK.disabled = false;
		f.l2tpWANGW.disabled = false;
	} else {
		f.l2tpWANIP.disabled = true;
		f.l2tpWANMSK.disabled = true;
		f.l2tpWANGW.disabled = true;
	}
}

function onpptpArdMode(f) {
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
function initWanSec(m) {
	var typeArr = ["type_static", "type_dhcp", "type_ppoe", "type_pptp", "type_l2tp","","","","pppoe_russia"],
		index = parseInt(m, 10),
		i;
	for (i = 0; i < 9; i++) {
		if(i !== index) {
			T.dom.addClass(T.dom.byId(typeArr[i]), "none");
		} else {
			T.dom.removeClass(T.dom.byId(typeArr[i]), "none");
		}
	}
	if(index == 8) {
		T.dom.removeClass(T.dom.byId(typeArr[2]), "none");
		T.dom.addClass(T.dom.byId("pppoe_all"), "none");
		T.dom.removeClass(T.dom.byId(typeArr[1]), "none");
	} else if(index == 2) {
		T.dom.removeClass(T.dom.byId("pppoe_all"), "none");
	}
	
	//显示相应帮助文档
	window.parent.selectHelp(index);
}
function initHtml(f) {
	var m = getCfg("WANT");
	document.frmSetup.WANT1.value = m;
	if(m == 9){
		m = 3;
	}
	if (m < 9 && m >= 0) {
		initWanSec(m);
	}
	if(m == 4){//l2tp
		onl2tpArdMode(f);
	} else if(m == 3) {//pptp
		onpptpArdMode(f);	
	} else if(m == 8) {
		onpppoeArdMode(f);
	}
}
function initEvent() {
	T.Event.on("WANT1", "change", function () {
		ispSelectChange(document.frmSetup);
	});
	T.dom.addPlaceholder("PUN", _("Enter username provided by ISP"), true);
	T.dom.inputPassword("PPW", _("Enter password provided by ISP"), true, true);
} 
function init() {
	var f= document.frmSetup;
	initHtml(f);
	cfg2Form(f);
	initEvent();
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function ispSelectChange(f) {
	var m = document.frmSetup.WANT1.value;
	if(m == 9)
 		m = 3;
	cfg2Form(f);
	initWanSec(m);
	if(m == 4) {//l2tp
		onl2tpArdMode(f);
	} else if(m == 3) {//pptp
		onpptpArdMode(f);
	} else if(m == 8) {
		onpppoeArdMode(f);
	} else if (m == 2) { //为了解决火狐低版本不显示占位符的问题
		document.getElementById("PUN").focus();
		document.getElementById("PUN").blur();
	}
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}
function preSubmit(f) {
	var m = document.frmSetup.WANT1.value,
		mtu,
		da;
	if ( m == 2) { //pppoe
		da = new Date();
		f.v12_time.value = da.getTime() / 1000;
		mtu = f.MTU.value;
		if(f.PCM[1].checked){
			if (!rangeCheck(f.PIDL,60,3600,_("Idle time"))) return ;
		}	
		if(f.PUN.value == "Enter user name provided by ISP" || f.PUN.value == "" || f.PPW.value == "" || f.PPW.value == "Enter password provided by ISP") {
			alert(_("Please enter the valid PPPoE username and PPPoE password provided by your ISP!"));
			return ;
		}
		if(!ill_check(f.PUN.value,illegal_user_pass,_("PPPoE Username"))) return;
		if(!ill_check(f.PPW.value,illegal_user_pass,_("PPPoE Password"))) return;
		if(!ill_check(f.SVC.value,illegal_user_pass,_("service name"))) return;
		if(!ill_check(f.AC.value,illegal_user_pass,_("server name"))) return;
		if (!chkStrLen(f.PUN,0,255,_("PPPoE Username"))) return ;
		if (!chkStrLen(f.PPW,0,255,_("PPPoE Password"))) return ;
		if(f.PCM[3].checked) {
			if (!rangeCheck(f.hour1,0,23,_("Time"))) return ;
			if (!rangeCheck(f.minute1,0,59,_("Time"))) return ;
			if (!rangeCheck(f.hour2,0,24,_("Time"))) return ;
			if (!rangeCheck(f.minute2,0,59,_("Time"))) return ;
			if((Number(f.hour1.value)*60+Number(f.minute1.value) > 1440)||
					(Number(f.hour2.value)*60+Number(f.minute2.value) > 1440)){
				alert(_("Please specify valid time information!"));
				return;
			}
			if((Number(f.hour1.value)*60+Number(f.minute1.value)) >=
					(Number(f.hour2.value)*60+Number(f.minute2.value))){
				alert(_("Start time must be earlier than end time!"));
				return;
			}
		}	
		setCfg("PST",Number(f.hour1.value)*60+Number(f.minute1.value));
		setCfg("PET",Number(f.hour2.value)*60+Number(f.minute2.value));
	} else if (m==8) {
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
	} else if ( m == 0){
		mtu = f.staticMTU.value;
		if (!verifyIP2(f.WANIP,_("IP address"))) return ;
		if (!ipMskChk(f.WANMSK,_("subnet mask"))) return ;
		if (!verifyIP2(f.WANGW,_("gateway"))) return ;
		if (!verifyIP2(f.DS1,_("DNS server"))) return ;
		if (!verifyIP0(f.DS2,_("alternate DNS server address"))) return ;
		f.WANIP.value = clearInvalidIpstr(f.WANIP.value);
		f.WANMSK.value = clearInvalidIpstr(f.WANMSK.value);
		f.WANGW.value = clearInvalidIpstr(f.WANGW.value);
		f.DS1.value = clearInvalidIpstr(f.DS1.value);
		f.DS2.value = clearInvalidIpstr(f.DS2.value);
	} else if (m == 4) {
		mtu = f.l2tpMTU.value;
		if(f.elements['l2tpPUN'].value == "" || f.elements['l2tpPPW'].value == "") {
			alert(_("Please enter the valid User name and Password provided by your ISP."));
			return ;
		}
		if(!ill_check(f.elements['l2tpPUN'].value,illegal_user_pass,_("Username"))) return;	
		if(!ill_check(f.elements['l2tpPPW'].value,illegal_user_pass,_("Password"))) return;
		if(f.l2tpIP.value == "") {
			alert(_("Please enter a valid server address!"));
			return ;
		}
		if(!ill_check(f.l2tpIP.value,illegal_user_pass,_("server address"))) return;
		
		if(f.l2tpAdrMode.value == "1") {
			if (!verifyIP2(f.l2tpWANIP,_("IP address"))) return false;
			if (!ipMskChk(f.l2tpWANMSK,_("Subnet Mask"))) return false;
			if (f.l2tpWANGW.value != "" && !verifyIP2(f.l2tpWANGW,_("ISP gateway"))) return false;
		}
		f.l2tpWANIP.value = clearInvalidIpstr(f.l2tpWANIP.value);
		f.l2tpWANMSK.value = clearInvalidIpstr(f.l2tpWANMSK.value);
		f.l2tpWANGW.value = clearInvalidIpstr(f.l2tpWANGW.value);
	} else if (m == 3 || m == 9) {
		mtu = f.pptpMTU.value;
		if(f.elements['pptpPUN'].value == "" || f.elements['pptpPPW'].value == "") {
			alert(_("Please enter the valid User name and Password provided by your ISP."));
			return ;
		}
		if(!ill_check(f.elements['pptpPUN'].value,illegal_user_pass,_("Username"))) return;	
	  if(!ill_check(f.elements['pptpPPW'].value,illegal_user_pass,_("Password"))) return;
		if(f.pptpIP.value == "") {
			alert(_("Please enter a valid server address!"));
			return ;
		}
		if(!ill_check(f.pptpIP.value,illegal_user_pass,_("Server address"))) return;

		if(f.pptpAdrMode.value == "1") {
			if (!verifyIP2(f.pptpWANIP,_("IP address"))) return false;
			if (!ipMskChk(f.pptpWANMSK,_("Subnet Mask"))) return false;
			if (f.pptpWANGW.value != "" && !verifyIP2(f.pptpWANGW,_("ISP gateway"))) return false;
		}
		f.pptpWANIP.value = clearInvalidIpstr(f.pptpWANIP.value);
		f.pptpWANMSK.value = clearInvalidIpstr(f.pptpWANMSK.value);
		f.pptpWANGW.value = clearInvalidIpstr(f.pptpWANGW.value);
	} else if(m == 1){
		mtu = f.dynamicMTU.value;	
	}
	
	if(m != 7){
		if (!isNumber(mtu, _("MTU value! It must be numeric"))) return ;
		if(parseInt(mtu,10) < 256 || parseInt(mtu,10) > 1500) {
			alert(_("Please enter an MTU value between 256 and 1500!"));
			return ;
		}
	}
	
	form2Cfg(f);
	document.getElementById("WANT2").value = parseInt(m) + 1;
	f.submit();
	showSaveMassage();
}
function onpppoeArdMode(f) {
	if(f.pppoeAdrMode.selectedIndex == 0){
		f.pppoeWANIP.disabled = false;
		f.pppoeWANMSK.disabled = false;
		
	} else {
		f.pppoeWANIP.disabled = true;
		f.pppoeWANMSK.disabled = true;	
	}
}
window.onload = init;
</script>
</body>
</html>