<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Firewall | URL Filtering</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>

<body class="bg">
<form name="frmSetup" id="frmSetup" method="POST" action="/goform/SafeUrlFilter">
	<fieldset>
		<legend>URL Filter Settings</legend>
		<div class="control-group">
			<label class="control-label">Filter Mode</label>
			<div class="controls" id="filter-mode-content"></div>
		</div>
		<table id="enableTab" class="content1">
			<tr>
				<td class="control-label">Access Policy</td>
				<td class="controls" id="cur-num-content"></td>
			</tr>
			<tr>
				<td class="control-label">Policy Name(Optional)</td>
				<td class="controls"><input class="text" size="12" maxlength="12" id="remark"></td>
			</tr>
			<tr>
				<td class="control-label">Start IP</td>
				<td class="controls"><span id="start-netip"></span><input class="text input-medium" size="3" maxlength="3" id="sip"></td>
			</tr>
			<tr>
				<td class="control-label">End IP</td>
				<td class="controls"><span id="end-netip"></span><input class="text input-medium" size="3" maxlength="3" id="eip"></td>
			</tr>
			<tr>
				<td class="control-label">URL Character String</td>
				<td class="controls"><input class="text" size="30" maxlength="128" id="url"></td>
			</tr>
			<tr>
				<td class="control-label">Time</td>
				<td class="controls" id="time-content"></td>
			</tr>
			<tr>
				<td class="control-label">Day(s)</td>
				<td class="controls" id="week-content"></td>
			</tr>
			<tr><td class="control-label">Enable</td>
				<td class="controls"><input type="checkbox" id="en" onClick="onSelected()">&nbsp;&nbsp;&nbsp;&nbsp;<span>Clear this item</span>:<input type="button" class="btn btn-mini" id="delbtn" value="Clear" onClick="onDel(document.frmSetup)"></td>
			</tr>
		</table>
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
	B.setTextDomain(["base", "qos_security"]);
	B.translate();
})();
</script>
<script src="js/gozila.js" type="text/javascript"></script>
<script>
var netip, curNum,
	max = 10,
	isempty = 0,
	selectNum = [],
	tmp = "",
	weekday = [_("Sun"),_("Mon"),_("Tue"),_("Wed"),_("Thu"),_("Fri"),_("Sat")],
	filter_mode = [_("Disable"),_("Forbid Only")],
	filter_mode_value = ["disable","deny","pass"];
	
addCfg("lanip",10,"<%getfirewall("lan","lanip");%>");
addCfg("check_en",70,"<%getfirewall("lan","url_en");%>");
//当前显示哪条
addCfg("curNum",72,"<%getfirewall("lan","urlNum");%>");//当前规则条:1~10
addCfg("UF1",90,"<%mAclGetUrl("1");%>");
addCfg("UF2",91,"<%mAclGetUrl("2");%>");
addCfg("UF3",92,"<%mAclGetUrl("3");%>");
addCfg("UF4",93,"<%mAclGetUrl("4");%>");
addCfg("UF5",94,"<%mAclGetUrl("5");%>");
addCfg("UF6",95,"<%mAclGetUrl("6");%>");
addCfg("UF7",96,"<%mAclGetUrl("7");%>");
addCfg("UF8",97,"<%mAclGetUrl("8");%>");
addCfg("UF9",98,"<%mAclGetUrl("9");%>");
addCfg("UF10",99,"<%mAclGetUrl("10");%>");
netip = getCfg("lanip").replace(/\d{1,3}$/,"");
curNum = getCfg("curNum");

function isemptyRule(){
	isempty = 0;
	for(var i=1;i<11;i++) {
		var cl=getCfg("UF"+i);
		if(cl != ""){
			selectNum[i-1] = "exist";
			isempty=isempty+1;
		} else {
			selectNum[i-1] = "none";	
		}
	}
}

//192.168.1.7-192.168.1.8:www.baidu.com,1-0,3600-0,on,desc
function onChangeNum(f){
	var n = f.curNum.selectedIndex + 1,
		c1 = getCfg("UF"+n).split(":"),
		ip = c1[0].split("-");
		c11 = "";
		
	//huangxiaoli add for ':'
	if(c1.length > 2){
		for(var i=1;i<c1.length;i++) {
			if(i != c1.length-1){
				c11+=c1[i]+":";
			}else{
				c11+=c1[i];
			}
		}
	} else {
		c11=c1[1];
	}
	//add end
	if(getCfg("UF"+n)!=""){
		f.sip.value = (ip[0].split("."))[3];
		f.eip.value = (ip[1].split("."))[3];

		//www.baidu.com,1-0,3600-0,on,desc
		var c2 = c11.split(",");
		
		f.url.value = c2[0];

		// 1-0,3600-0,on,desc
		f.startw.value =  (c2[1].split("-"))[0];
		f.endw.value =   (c2[1].split("-"))[1];

		// 3600-0,on,desc

		var t = c2[2].split("-");
		
		f.sh.value = parseInt(t[0]/3600)%24;
		f.sm.value = parseInt((t[0]-f.sh.value*3600)/60);
		f.eh.value = parseInt(t[1]/3600)%24;
		f.em.value = parseInt((t[1]-f.eh.value*3600)/60);
		
		//on,desc
		f.en.checked = (c2[3] == "on");
		
		if(c2[4] == "notag"){
			c2[4] = "";
		}
		f.remark.value = c2[4];	
	} else {
		onDel(f);
	}
	onSelected();	
}

function preSubmit(f)  {
	var st, et, num,
		loc = "/goform/SafeUrlFilter?GO=firewall_urlfilter.asp",
		re =/^[0-9a-zA-Z_\-.:]+$/;
		
	loc += "&check=" + f.elements["filter_mode"].value;
	if(f.elements["filter_mode"].value != "disable") {			
		sip = f.sip.value ;
		eip = f.eip.value ;
		sh = f.sh.value;
		eh = f.eh.value;
		sm = f.sm.value;
		em = f.em.value;
		startw = f.startw.value;
		endw = f.endw.value;
		remark = f.remark.value;
		urlstr = f.url.value;
		num = f.curNum.selectedIndex + 1;
		loc += "&curNum=" + num; 
			
		if ((sip.length==0)&&(eip.length==0)&&(remark.length==0)&&(urlstr.length==0)) {
			if(isempty != 0 && selectNum[num-1] == "exist"){
				isempty=isempty-1;
			}
		} else {
			if(selectNum[num-1] == "none"){
		    	isempty=isempty+1;
			}
			//var illegal_user_pass = new Array("~!@#$%^&*();:',\"");
	
			if (!rangeCheck(f.sip,1,254,_("start IP address"))||
					!rangeCheck(f.eip,1,254,_("end IP address"))){
				return ;
			}
			if ( Number(sip) > Number(eip) ) {
				alert(_("Start IP should not be greater than end IP. Please enter a valid IP address."));
				return ;
			}

			if(f.url.value == "" || !re.test(f.url.value)){
				alert(_("Please enter a valid keyword or domain name" + "."));
				return ;
			}
			if(Number(eh) < Number(sh)) {
				alert(_("time note"));
				return ;
			}
			if(Number(eh) == Number(sh)) {
				if(Number(em) < Number(sm)) {
					alert(_("time note"));
					return ;
				}
			}
			if(Number(startw)>Number(endw)) {
			        alert(_("week note"));
					return ;
			}
			//192.168.1.7-192.168.1.8:www.baidu.com,1-0,3600-0,on,desc
			loc += "&CL" + num + "=";
			sip = clearInvalidIpstr(sip);
			eip = clearInvalidIpstr(eip);
			loc += netip + sip + "-" + netip + eip + ":";
			loc += urlstr + ",";
			loc += startw + "-" + endw +",";
			st = sh*3600+sm*60;
			et = eh*3600+em*60;
			loc += st + "-" + et+",";
			
			if(f.en.checked) {
				loc += "on" + ",";
			} else {
				loc += "off" + ",";	
			}
			if(remark == "notag") {
				alert(_("notag is not allowed to be set to be policy Name!"));
				return ;
			}
			if(remark == "") {
				remark = "notag";
			} else {
				if(!ckRemark(remark)) {
					return ;
				}
			}
			loc += remark;			
		}
		if(f.elements["filter_mode"].value=="pass"&&isempty==0) {
			isemptyRule();
			alert(_("You must set at least one rule when you choose Permit Only!"));
			return false;
		}
    }	
	window.location = loc;
	showSaveMassage();
}

function filter_mode_change(f){
	var on = f.filter_mode.options.selectedIndex;
	if(on != 0){
		document.getElementById("enableTab").style.display ="";
	} else {
		document.getElementById("enableTab").style.display ="none";
	}
	
	//reset this iframe height by call parent frame function
	window.parent.reinitIframe();
}

function onDel(f) {
	f.remark.value="";
	f.url.value="";
	f.sip.value="";
	f.eip.value="";
	f.sh.value=0;
	f.eh.value=0;
	f.sm.value=0;
	f.em.value=0;
	f.startw.value=0;
	f.endw.value=6;
	f.en.checked = true;
	onSelected();
}

function onSelected(){
	var frm = document.frmSetup
	if(document.getElementById("en").checked) {
		//document.getElementById("enableTab").disabled = false;
		document.getElementById("remark").disabled = false;
		document.getElementById("sip").disabled = false;
		document.getElementById("eip").disabled = false;
		document.getElementById("sh").disabled = false;
		document.getElementById("sm").disabled = false;
		document.getElementById("eh").disabled = false;
		document.getElementById("em").disabled = false;
		document.getElementById("url").disabled = false;
		document.getElementById("startw").disabled = false;
		document.getElementById("endw").disabled = false;
	} else {
		//document.getElementById("enableTab").disabled = true;
		document.getElementById("remark").disabled = true;
		document.getElementById("sip").disabled = true;
		document.getElementById("eip").disabled = true;
		document.getElementById("sh").disabled = true;
		document.getElementById("sm").disabled = true;
		document.getElementById("eh").disabled = true;
		document.getElementById("em").disabled = true;
		document.getElementById("url").disabled = true;
		document.getElementById("startw").disabled = true;
		document.getElementById("endw").disabled = true;
	}
}

function initFilterMode() {
	var text = '<select id="filter_mode" onChange="filter_mode_change(document.frmSetup)">';
	
	for(var i=0; i < 2; i++){
		text +=	'<option value='+filter_mode_value[i]+' >'+filter_mode[i]+'</option>';	
	}    
	text += '</select>';
	document.getElementById("filter-mode-content").innerHTML = text;
}
function initCurNum() {
	var text = '<select id=curNum onChange=onChangeNum(document.frmSetup);>';
	
	for(var i=1;i<=max;i++){
		text += '<option value='+i+'>'+ '(' +i+ ')'+'</option>';
	}
	text += '</select>';
	document.getElementById("cur-num-content").innerHTML = text;
}

function initTime() {
	var text = '<select id="sh" class="input-mini">';
	for(var i=0; i<=23; i++){
		text +=	'<option value='+i+' >'+i+'</option>';	
	}
	text += '</select>:<select id="sm" class="input-mini">';
	for(var j=0;j<12;j++) {
		text += '<option value=' + (j *5)+ '>' + (j * 5) + '</option>';
	}
	text += '</select>&nbsp;~&nbsp;<select id="eh" class="input-mini">';
	for(var i=0; i<=23; i++){
		text +=	'<option value='+i+' >'+i+'</option>';	
	}
	text += '</select>:<select id="em" class="input-mini">';
	for(var j=0;j<12;j++)
	{
		text += '<option value=' + (j *5)+ '>' + (j*5) + '</option>';
	}
	text += '</select>';
	document.getElementById("time-content").innerHTML = text;
}

function initWeek() {
	var text = '<select id="startw" class="input-small">';
	for(var i=0; i<7; i++){
		text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
	}
	
	text += '</select>&nbsp;~&nbsp;<select id="endw" class="input-small">';
	for(var i=0; i<7; i++){
		text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
	}
	
	text += '</select>';
	
	document.getElementById("week-content").innerHTML = text;
}
function initHtml() {
	document.getElementById("start-netip").innerHTML = netip;
	document.getElementById("end-netip").innerHTML = netip;
	initFilterMode();
	initCurNum();
	initTime();
	initWeek();
}
function init(f){
	var filter_mode_index = getCfg("check_en");
	
	initHtml();
	f.filter_mode.options.selectedIndex = parseInt(filter_mode_index);
	filter_mode_change(f);
	f.curNum.selectedIndex = curNum-1;
	onChangeNum(f);
	isemptyRule();
}
window.onload = function () {
	init(document.frmSetup);
};
</script>
</body>
</html>