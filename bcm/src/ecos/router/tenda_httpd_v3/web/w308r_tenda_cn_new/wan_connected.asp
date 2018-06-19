<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>LAN | LAN Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
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
addCfg("MTU", 27, def_MTU );
addCfg("SVC", 26, def_SVC);
addCfg("AC", 25, def_AC);
addCfg("l2tpIP",24,def_l2tpSIP);
addCfg("l2tpPUN",23,def_l2tpPUN);
addCfg("l2tpPPW",22,def_l2tpPPW);
addCfg("l2tpMTU",21,def_l2tpMTU);
addCfg("l2tpAdrMode",20,def_l2tpAdrMode);
addCfg("l2tpWANIP",19,def_l2tpIP);
addCfg("l2tpWANMSK",18,def_l2tpMSK);
addCfg("l2tpWANGW",17,def_l2tpWGW);
addCfg("pptpIP",16,def_pptpSIP);
addCfg("pptpPUN",15,def_pptpPUN);
addCfg("pptpPPW",14,def_pptpPPW);
addCfg("pptpMTU",13,def_pptpMTU);
addCfg("pptpAdrMode",12,def_pptpAdrMode);
addCfg("pptpWANIP",11,def_pptpIP);
addCfg("pptpWANMSK",10,def_pptpMSK);
addCfg("pptpWANGW",9,def_pptpWGW);
addCfg("mppeEn",8,def_pptpMPPE);
addCfg("dynamicMTU",7,def_dynamicMTU);
addCfg("staticMTU",6,def_staticMTU);
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
	var typeArr = ["type_static", "type_dhcp", "type_ppoe", "type_l2tp"],
		index = parseInt(m, 10),
		i;
	for (i = 0; i < 4; i++) {
		if(i !== index) {
			T.dom.addClass(T.dom.byId(typeArr[i]), "none");
		} else {
			T.dom.removeClass(T.dom.byId(typeArr[i]), "none");
		}
	}
}
function initHtml() {
	var m = getCfg("WANT");
	document.frmSetup.WANT1.value = m;
	if (m < 7 && m >= 0) {
		initWanSec(m);
	}
	if(m == 4){//l2tp
		onl2tpArdMode(f);
	} else if(m == 3) {//pptp
		onpptpArdMode(f);
	}
}
function initEvent() {
	T.Event.on("WANT1", "change", function () {
		ispSelectChange(document.frmSetup);
	});
	T.dom.addPlaceholder("PUN", "请输入宽带运营商提供的账号", true);
	T.dom.inputPassword("PPW", "请输入宽带运营商提供的密码", true);
} 
function init() {
	var f= document.frmSetup;
	cfg2Form(f);
	initHtml();
	initEvent();
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function ispSelectChange(f) {
	var m = document.frmSetup.WANT1.value;
	cfg2Form(f);
	initWanSec(m);
	if(m == 4) {//l2tp
		onl2tpArdMode(f);
	} else if(m == 3) {//pptp
		onpptpArdMode(f);
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
			if (!rangeCheck(f.PIDL,60,3600,"空闲时间")) return ;
		}	
		if(f.PUN.value == "请输入宽带运营商提供的账号" || f.PUN.value == "" || f.PPW.value == "") {
			alert("宽带用户名或宽带密码为空!");
			return ;
		}
		if(!ill_check(f.PUN.value,illegal_user_pass,"宽带用户名")) return;
		if(!ill_check(f.PPW.value,illegal_user_pass,"宽带密码")) return;
		if(!ill_check(f.SVC.value,illegal_user_pass,"服务名")) return;
		if(!ill_check(f.AC.value,illegal_user_pass,"服务器名")) return;
		if (!chkStrLen(f.PUN,0,255,"宽带用户名")) return ;
		if (!chkStrLen(f.PPW,0,255,"宽带密码")) return ;
		if(f.PCM[3].checked) {
			if (!rangeCheck(f.hour1,0,23,"时间")) return ;
			if (!rangeCheck(f.minute1,0,59,"时间")) return ;
			if (!rangeCheck(f.hour2,0,24,"时间")) return ;
			if (!rangeCheck(f.minute2,0,59,"时间")) return ;
			if((Number(f.hour1.value)*60+Number(f.minute1.value) > 1440)||
					(Number(f.hour2.value)*60+Number(f.minute2.value) > 1440)){
				alert("时间超出规定范围");
				return;
			}
			if((Number(f.hour1.value)*60+Number(f.minute1.value)) >=
					(Number(f.hour2.value)*60+Number(f.minute2.value))){
				alert("开始时间必须小于结束时间");
				return;
			}
		}	
		setCfg("PST",Number(f.hour1.value)*60+Number(f.minute1.value));
		setCfg("PET",Number(f.hour2.value)*60+Number(f.minute2.value));
	} else if ( m == 0){
		mtu = f.staticMTU.value;
		if (!verifyIP2(f.WANIP,"IP 地址")) return ;
		if (!ipMskChk(f.WANMSK,"子网掩码")) return ;
		if (!checkVerifyIp(f.WANIP.value,f.WANMSK.value,"IP 地址")) return;
		if (!verifyIP2(f.WANGW,"网关地址")) return ;
		if (!verifyIP2(f.DS1,"主DNS地址")) return ;
		if (!verifyIP0(f.DS2,"备用DNS地址")) return ;
		f.WANIP.value = clearInvalidIpstr(f.WANIP.value);
		f.WANMSK.value = clearInvalidIpstr(f.WANMSK.value);
		f.WANGW.value = clearInvalidIpstr(f.WANGW.value);
		f.DS1.value = clearInvalidIpstr(f.DS1.value);
		f.DS2.value = clearInvalidIpstr(f.DS2.value);
	} else if (m == 4) {
		mtu = f.l2tpMTU.value;
		if(f.elements['l2tpPUN'].value == "" || f.elements['l2tpPPW'].value == "") {
			alert("用户名或密码为空!");
			return ;
		}
		if(!ill_check(f.elements['l2tpPUN'].value,illegal_user_pass,"用户名")) return;	
		if(!ill_check(f.elements['l2tpPPW'].value,illegal_user_pass,"密码")) return;
		if(f.l2tpIP.value == "") {
			alert("服务器地址为空!");
			return ;
		}
		if(!ill_check(f.l2tpIP.value,illegal_user_pass,"服务器地址")) return;
		
		if(f.l2tpAdrMode.value == "1") {
			if (!verifyIP2(f.l2tpWANIP,"IP 地址")) return false;
			if (!ipMskChk(f.l2tpWANMSK,"子网掩码")) return false;
			if (!checkVerifyIp(f.l2tpWANIP.value,f.l2tpWANMSK.value,"IP 地址")) return;
			if (f.l2tpWANGW.value != "" && !verifyIP2(f.l2tpWANGW,"ISP 网关地址")) return false;
		}
		f.l2tpWANIP.value = clearInvalidIpstr(f.l2tpWANIP.value);
		f.l2tpWANMSK.value = clearInvalidIpstr(f.l2tpWANMSK.value);
		f.l2tpWANGW.value = clearInvalidIpstr(f.l2tpWANGW.value);
	} else if (m == 3) {
		mtu = f.pptpMTU.value;
		if(f.elements['pptpPUN'].value == "" || f.elements['pptpPPW'].value == "") {
			alert("用户名或密码为空!");
			return ;
		}
		if(!ill_check(f.elements['pptpPUN'].value,illegal_user_pass,"用户名")) return;	
	  if(!ill_check(f.elements['pptpPPW'].value,illegal_user_pass,"密码")) return;
		if(f.pptpIP.value == "") {
			alert("服务器地址为空!");
			return ;
		}
		if(!ill_check(f.pptpIP.value,illegal_user_pass,"服务器地址")) return;

		if(f.pptpAdrMode.value == "1") {
			if (!verifyIP2(f.pptpWANIP,"IP 地址")) return false;
			if (!ipMskChk(f.pptpWANMSK,"子网掩码")) return false;
			if (!checkVerifyIp(f.pptpWANIP.value,f.pptpWANMSK.value,"IP 地址")) return;
			if (f.pptpWANGW.value != "" && !verifyIP2(f.pptpWANGW,"ISP 网关地址")) return false;
		}
		f.pptpWANIP.value = clearInvalidIpstr(f.pptpWANIP.value);
		f.pptpWANMSK.value = clearInvalidIpstr(f.pptpWANMSK.value);
		f.pptpWANGW.value = clearInvalidIpstr(f.pptpWANGW.value);
	} else if(m == 1){
		mtu = f.dynamicMTU.value;	
	}
	
	if(m != 7){
		if (!isNumber(mtu, "MTU")) return ;
		if(parseInt(mtu,10) < 256 || parseInt(mtu,10) > 1500) {
			alert("MTU值范围：256~1500");
			return ;
		}
	}
	
	form2Cfg(f);
	document.getElementById("WANT2").value = parseInt(m) + 1;
	f.submit();
	showSaveMassage();
}
window.onload = init;
</script>

</head>
<body>
<FORM name=frmSetup method=POST action="/goform/AdvSetWan">
<input type=hidden name=GO value=wan_connectd.asp >
<input type="hidden" id="rebootTag" name="rebootTag">
<input type="hidden" id="v12_time" name="v12_time">
<input type="hidden" name="WANT2" id="WANT2">
	<fieldset>
		<h2 class="legend">上网设置</h2>
		<div class="control-group">
			<label for="WANT1" class="control-label">上网方式</label>
			<div class="controls">
				<select name="WANT1" id="WANT1">
					<option value=2>ADSL拨号</option>
					<option value=0>静态IP</option>
					<option value=1>自动获取</option>
				</select>
			</div>
		</div>
		<div id="wan_sec">
			<table id="type_dhcp" class="none">
				<tr>
					<td class="control-label">MTU</td>
					<td class="controls">
						<input type="text" class=text name=dynamicMTU size=4 maxlength="4" value="1500">
						<span class="help-block">(如非必要，请勿改动，默认值1500)</span>
					</td>
				</tr>
			</table>
			
			<table id="type_static" class="none">
				<tr>
					<td class="control-label">IP地址</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=WANIP size=16></td>
				</tr>
				<tr>
					<td class="control-label">子网掩码</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=WANMSK size=16></td>
				</tr>
				<tr>
					<td class="control-label">网关</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=WANGW size=16></td>
				</tr>
				<tr>
					<td class="control-label">DNS服务器</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=DS1 size=16></td>
				</tr>
				<tr>
					<td class="control-label">备用DNS服务器</td>
					<td class="controls"><input type="text" class=text maxLength=15 name=DS2 size=16>（可选）</td>
				</tr>
				<tr>
					<td class="control-label">MTU</td>
					<td class="controls">
						<input type="text" class=text name=staticMTU size=4 maxlength="4" value="1500">
						<span class="help-block">(如非必要，请勿改动，默认值1500)</span>
					</td>
				</tr>
			</table>
			
			<fieldset id="type_ppoe" class="none">
				<div class="control-group">
					<label class="control-label">宽带用户名</label>
					<div class="controls">
						<input type="text" name="PUN" id="PUN" maxLength="128" class="text">
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">宽带密码</label>
					<div id=td_PPW class="controls">
						<input name="PPW" id="PPW" maxLength="128" type="password" class="text">
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">MTU</label>
					<div class="controls">
						<input type="text" class="text" name="MTU" size="4" value="1492">
						<span class="help-block">(如非必要，请勿改动，默认值1492)</span>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">服务名</label>
					<div class="controls">
						<input type="text" class="text" name="SVC" maxLength=50 size=25>
						<span class="help-block">(如非必要，请勿填写)</span>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label">服务器名称</label>
					<div class="controls">
						<input type="text" class="text" name="AC" maxLength=50 size=25>
						<span class="help-block">(如非必要，请勿填写)</span>
					</div>
				</div>
				<p class="control-label control-label-title">请根据需要选择连接模式</p>
				<div class="control-group">
					<div class="controls"><label class="radio"><input name="PCM" type="radio" value="0" />自动连接，在开机和断线后自动进行连接。</label></div>
				</div>
				<div class="control-group">
					<div class="controls"><label class="radio"><input name="PCM" type="radio"  value="1" />按需连接，在有访问数据时自动进行连接。</label></div>
				</div>
				<div class="control-group">
					<label class="control-label">自动断线等待时间</label>
					<div class="controls">
						<input type="text" class="text" maxLength="4" name="PIDL" size="4">
						<span class="help-block">(60-3600,秒)</span>
					</div>
				</div>
				<div class="control-group">
					<label class="control-label"></label>
					<div class="controls"><label class="radio"><input name="PCM" type="radio" value="2" />手动连接，由用户手动进行连接。</label></div>
				</div>
				<div class="control-group">
					<label class="control-label"></label>
					<div class="controls"><label class="radio"><input name="PCM" type="radio" value="3" />定时连接，在指定的时段自动进行连接。</label></div>
				</div>
				<p class="text-red">注意：只有当您到“系统工具”菜单的“网络时间”项设置了当前时间后，“定时连接”功能才能生效。</p>
				<div>连接时段：从  
					<input class="text input-mini" maxLength=2 name=hour1 size=2 value='<%aspTendaGetStatus("ppoe","h_s");%>'> 时 
					<input class="text input-mini" maxLength=2 name=minute1 size=2 value='<%aspTendaGetStatus("ppoe","m_s");%>'> 分到 
					<input class="text input-mini" maxLength=2 name=hour2 size=2 value='<%aspTendaGetStatus("ppoe","h_e");%>'> 时 
					<input class="text input-mini" maxLength=2 name=minute2 size=2 value='<%aspTendaGetStatus("ppoe","m_e");%>'> 分
				</div>
			</fieldset>
			
			<table id="type_l2tp" class="none">
				<tr><td class="control-label">L2TP服务器地址</td>
					<td class="controls"><input type="text" class=text name="l2tpIP" size="15" maxlength="15"></td></tr>
				<tr><td class="control-label">用户名</td>
					<td class="controls"><input type="text" class=text maxLength=50 name="l2tpPUN" size=25></td></tr>
				<tr><td class="control-label">密码</td>
					<td class="controls"><input type="text" class=text maxLength=50 name="l2tpPPW" size=25 type=password></td></tr>
				<tr><td class="control-label">MTU</td>
					<td class="controls"><input type="text" class=text name="l2tpMTU" size=23 value="1492"></td></tr>
				<tr><td class="control-label">地址模式</td>
					<td class="controls"><select name="l2tpAdrMode" id="l2tpAdrMode" onChange="onl2tpArdMode(document.frmSetup)"><option value="1">Static</option>
						<option value="2">Dynamic</option>
						</select></td></tr>
				<tr><td class="control-label">IP地址</td>
					<td class="controls"><input type="text" class="text" name="l2tpWANIP" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">子网掩码</td>
					<td class="controls"><input type="text" class="text" name="l2tpWANMSK" size="15" maxlength="15"></td>
				</tr>
				<tr><td class="control-label">默认网关</td>
					<td class="controls"><input type="text" class="text" name="l2tpWANGW" size="15" maxlength="15"></td>
				</tr>
			</table>
		</div>
	</fieldset>
<script>tbl_tail_save('document.frmSetup')</script>
</form>
<div id="save" class="none"></div>
</body>
</html>