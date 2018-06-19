<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Firewall | Client Filtering</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script src="js/gozila.js" type="text/javascript"></script>
<script>
var netip, curNum,
	tmp="",
	max=10,
	isempty=0,
	selectNum = [],
	prot= ["tcp","udp","tcp/udp"],
	weekday = ["星期日","星期一","星期二","星期三","星期四","星期五","星期六"],
	filter_mode = ["禁用","仅禁止","仅允许"],
	filter_mode_value = ["disable","deny","pass"];

addCfg("lanip",10,"<%getfirewall("lan","lanip");%>");
addCfg("check_en",70,"<%getfirewall("lan","acl_en");%>");
//当前显示哪条
addCfg("curNum",72,"<%getfirewall("lan","curNum");%>");
addCfg("CL1",90,"<%mAclGetIP("1");%>");
addCfg("CL2",91,"<%mAclGetIP("2");%>");
addCfg("CL3",92,"<%mAclGetIP("3");%>");
addCfg("CL4",93,"<%mAclGetIP("4");%>");
addCfg("CL5",94,"<%mAclGetIP("5");%>");
addCfg("CL6",95,"<%mAclGetIP("6");%>");
addCfg("CL7",96,"<%mAclGetIP("7");%>");
addCfg("CL8",97,"<%mAclGetIP("8");%>");
addCfg("CL9",98,"<%mAclGetIP("9");%>");
addCfg("CL10",99,"<%mAclGetIP("10");%>");

netip = getCfg("lanip").replace(/\d{1,3}$/,"");
curNum = getCfg("curNum");

function init(f){
	var filter_mode_index =  getCfg("check_en");
	f.filter_mode.options.selectedIndex = parseInt(filter_mode_index);
	filter_mode_change(f);
	f.curNum.selectedIndex = curNum-1;
	onChangeNum(f);
	isemptyRule();
}

function isemptyRule() {
	var cl, i;
	
	isempty=0;
	for(i=1;i<11;i++) {
		var cl=getCfg("CL"+i);
		if(cl != "") {
			selectNum[i-1] = "exist";
			isempty=isempty+1;
		} else {
			selectNum[i-1] = "none";	
		}
	}
}

function filter_mode_change(f) {
	var on = f.filter_mode.options.selectedIndex;
	if(on != 0){
		//show_hide("enableTab",1);
		document.getElementById("enableTab").style.display ="";
	}else{
		//show_hide("enableTab",0);
		document.getElementById("enableTab").style.display ="none";
	}
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

//192.168.1.3-192.168.1.4:50-60,tcp,1-0,3600-0,on,desc
function preSubmit(f) {
	var st, et, num,
		loc = "/goform/SafeClientFilter?GO=firewall_clientfilter.asp";
		
	loc += "&check=" + f.elements["filter_mode"].value;
	if(f.elements["filter_mode"].value != "disable") {			
		sip = f.sip.value ;
		eip = f.eip.value ;
		spt = f.sport.value ;
		ept = f.eport.value ;
		sh = f.sh.value;
		eh = f.eh.value;
		sm = f.sm.value;
		em = f.em.value;
		startw = f.startw.value;
		endw = f.endw.value;
		remark = f.remark.value;
		num = f.curNum.selectedIndex + 1;
		loc += "&curNum=" + num; 

		if ((sip.length==0)&&(eip.length==0)&&(spt.length==0)&&(ept.length==0)&&(remark.length==0)) {
			if(isempty != 0 && selectNum[num-1] == "exist"){
				isempty=isempty-1;
			}
		} else {   
			if(selectNum[num-1] == "none"){
				isempty=isempty+1;
			}
			if (!rangeCheck(f.sip,1,254,"起始IP")||
					!rangeCheck(f.eip,1,254,"结束IP")) {
				return ;
			}
			if ( Number(sip) > Number(eip) ) { alert("起始IP > 结束IP"); return ; }

			if (!rangeCheck(f.sport,1,65535,"起始端口")||
					!rangeCheck(f.eport,1,65535,"结束端口")) {
				return ;
			}
			if ( Number(spt) > Number(ept) ) { alert("起始端口 > 结束端口"); return ; }
			
			if(Number(eh) < Number(sh)) {
				alert("起始时间 > 结束时间");
				return ;
			}
			if(Number(eh) == Number(sh)) {
				if(Number(em) < Number(sm)) {
					alert("起始时间 > 结束时间");
					return ;
				}
			}
			if(Number(startw)>Number(endw)) {
			        alert("起始星期 > 结束星期");
					return ;
			}

			loc += "&CL" + num + "=";
			sip = clearInvalidIpstr(sip);
			eip = clearInvalidIpstr(eip);
			spt = clearInvalidIpstr(spt);
			ept = clearInvalidIpstr(ept);
			loc += netip + sip + "-" + netip + eip + ":";
			loc += spt + "-" + ept + ",";
			loc += prot[ f.protocol.value] +",";
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
				alert("注释不可输入‘notag’");
				return ;
			}
			if(remark == "") {
				remark = "notag";
			} else {
				if(!ckRemark(remark))return ;
			}
			loc += remark;
			
		}
		if(f.elements["filter_mode"].value=="pass"&&isempty==0) {
			isemptyRule();
			alert("选择仅允许时必须输入一条规则！");
			return false;
		}
	}
	window.location = loc;
	showSaveMassage()
}

//选择不同规则条
//192.168.1.3-192.168.1.4:50-60,tcp,1-0,3600-0,on,desc
function onChangeNum(f) {	
	var i, ip, c2, t,
		n 	= f.curNum.selectedIndex + 1,
		c1 	= getCfg("CL"+n).split(":");
	
	if(getCfg("CL"+n)!="") {
		//192.168.1.3-192.168.1.4
		ip = c1[0].split("-");

		f.sip.value = (ip[0].split("."))[3];
		f.eip.value = (ip[1].split("."))[3];

		//50-60,tcp,1-0,3600-0,on,desc
		c2 = c1[1].split(",");

		f.sport.value = (c2[0].split("-"))[0];
		f.eport.value = (c2[0].split("-"))[1];

		for(i=0;i<3;i++){
			if(c2[1] == prot[i])
				f.protocol.selectedIndex = i;
		}

		// 1-0,3600-0,on,desc
		f.startw.value =  (c2[2].split("-"))[0];
		f.endw.value =   (c2[2].split("-"))[1];

		// 3600-0,on,desc
		t = c2[3].split("-");
		
		f.sh.value = parseInt(t[0]/3600)%24;
		f.sm.value = parseInt((t[0]-f.sh.value*3600)/60);
		f.eh.value = parseInt(t[1]/3600)%24;
		f.em.value = parseInt((t[1]-f.eh.value*3600)/60);
		
		//on,desc
		f.en.checked = (c2[4] == "on");

		if(c2[5] == "notag") {
			c2[5] = "";
		}
		f.remark.value = c2[5];
	} else {
		onDel(f);
	}
	onSelected();	
}

//清除
function onDel(f) {
	//f.en.checked = false;
	f.remark.value="";
	f.sip.value="";
	f.eip.value="";
	f.sport.value="";
	f.eport.value="";
	f.sh.value=0;
	f.eh.value=0;
	f.sm.value=0;
	f.em.value=0;
	f.startw.value=0;
	f.endw.value=6;
	f.en.checked = true;
	onSelected();
}

//启用操作
function onSelected() {
	var frm = document.frmSetup
	if(document.getElementById("en").checked) {
		//document.getElementById("enableTab").disabled = false;
		document.getElementById("remark").disabled = false;
		document.getElementById("sip").disabled = false;
		document.getElementById("eip").disabled = false;
		document.getElementById("sport").disabled = false;
		document.getElementById("eport").disabled = false;
		document.getElementById("protocol").disabled = false;
		document.getElementById("sh").disabled = false;
		document.getElementById("sm").disabled = false;
		document.getElementById("eh").disabled = false;
		document.getElementById("em").disabled = false;
		document.getElementById("startw").disabled = false;
		document.getElementById("endw").disabled = false;
	} else{
		//document.getElementById("enableTab").disabled = true;	
		document.getElementById("remark").disabled = true;
		document.getElementById("sip").disabled = true;
		document.getElementById("eip").disabled = true;
		document.getElementById("sport").disabled = true;
		document.getElementById("eport").disabled = true;
		document.getElementById("protocol").disabled = true;
		document.getElementById("sh").disabled = true;
		document.getElementById("sm").disabled = true;
		document.getElementById("eh").disabled = true;
		document.getElementById("em").disabled = true;
		document.getElementById("startw").disabled = true;
		document.getElementById("endw").disabled = true;
	}
}
</script>
</head>

<body class="bg" onLoad="init(document.frmSetup);">
<form name="frmSetup" method="POST" action="/goform/SafeClientFilter">
	<fieldset>
		<h2 class="legend">端口过滤</h2>
		<div class="control-group">
			<label class="control-label">过滤模式</label>
			<div class="controls">
			<script>
				var text = '<select id="filter_mode" onChange="filter_mode_change(document.frmSetup)">';
				for(var i=0; i<3; i++){
					text +=	'<option value='+filter_mode_value[i]+' >'+filter_mode[i]+'</option>';	
				}    
				text += '</select>';    
				document.write(text);
			</script>
			</div>
		</div>
		<table id="enableTab" class="content1" style="margin-top:0px;">
			<tr>
				<td class="control-label">请选择</td>
				<td class="controls">
					<script>
						var str = '<select id=curNum onChange=onChangeNum(document.frmSetup);>';        
						for(var i=1;i<=max;i++) {
							str += '<option value='+i+'>'+ '(' +i+ ')'+'</option>';
						}
						str += '</select>';
						document.write(str);
					</script>
				</td>
			</tr>
			<tr><td class="control-label">注释</td>
			  <td class="controls"><input class="text" id="remark" size="12" maxlength="12"></td>
			</tr>
			<tr>
			  <td class="control-label">开始IP</td>
			  <td class="controls"><script>document.write(netip);</script><input class="text input-medium" id="sip" size="3" maxlength="3"></td>
			</tr>
			<tr>
			  <td class="control-label">结束IP</td>
			  <td class="controls"><script>document.write(netip);</script><input class="text input-medium" id="eip" size="3" maxlength="3"></td>
			</tr>
			<tr><td class="control-label">端口</td>
			  <td class="controls"><input class="text input-small" id="sport" size="4" maxlength="5">~<input class="text input-small" id="eport" size="4" maxlength="5"></td>
			</tr>
			<tr><td class="control-label">类型</td>
			<td class="controls"><select id="protocol" ><option value="0"> TCP </option><option value="1"> UDP </option><option value="2"> 全部 </option></select></td>  
			</tr>
			<tr>
				<td class="control-label">时间</td>
				<td class="controls">
					<script>
					var text = '<select id="sh" class="input-mini">';
					for(var i=0; i<=23; i++){
						text +=	'<option value='+i+' >'+i+'</option>';	
					}
					text += '</select>:<select id="sm" class="input-mini">';
					for(var j=0;j<12;j++)
					{
						text += '<option value=' + (j *5) + '>' + (j * 5) + '</option>';
					}
					text += '</select>&nbsp;~&nbsp;<select id="eh" class="input-mini">';
					for(var i=0; i<=23; i++){
						text +=	'<option value='+i+' >'+i+'</option>';	
					}
					text += '</select>:<select id="em" class="input-mini">';
					for(var j=0;j<12;j++)
					{
						text += '<option value=' + (j * 5)+ '>' + (j * 5) + '</option>';
					}
					text += '</select>';
					document.write(text);
					</script>
				</td>
			</tr>
			<tr>
				<td class="control-label">日期</td>
				<td class="controls">
				<script>
					var text = '<select id="startw" name="startw" class="input-small">';
					for(var i=0; i<7; i++){
						text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
					}
					
					text += '</select>&nbsp;~&nbsp;<select id="endw" name="endw" class="input-small">';
					for(var i=0; i<7; i++){
						text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
					}
					
					text += '</select>';
					document.write(text);
				</script>
				</td>
			</tr>
			<tr><td class="control-label">启用:</td>
			  <td class="controls"><input name="checkbox" type="checkbox" id="en" onClick="onSelected()">&nbsp;&nbsp;&nbsp;&nbsp;
			  清空该项:<input type="button" class="btn btn-mini" id="delbtn" value="清 空" onClick="onDel(document.frmSetup)"></td>
			</tr>
		</table>
	</fieldset>
<script>tbl_tail_save("document.frmSetup");</script>
</form>
<div id="save" class="none"></div>	
</body>
</html>