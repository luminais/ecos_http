<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Firewall | Client Filtering</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script src="js/gozila.js" type="text/javascript"></script>
<script>
var curNum,
	tmp="",
	max=10,
	isempty=0,
	selectNum = [],
	weekday = ["星期日","星期一","星期二","星期三","星期四","星期五","星期六"],
	filter_mode = ["禁用","仅禁止","仅允许"],
	filter_mode_value = ["disable","deny","pass"];
	
addCfg("check_en",40,"<%getfirewall("lan","acl_mac");%>");//0:unselect;1;selected
//当前显示哪条
addCfg("curNum",72,"<%getfirewall("lan","acl_macNum");%>");//当前规则条:1~10
addCfg("ML1",60,'<%mAclGetMacFilter("1");%>');
addCfg("ML2",61,'<%mAclGetMacFilter("2");%>');
addCfg("ML3",62,'<%mAclGetMacFilter("3");%>');
addCfg("ML4",63,'<%mAclGetMacFilter("4");%>');
addCfg("ML5",64,'<%mAclGetMacFilter("5");%>');
addCfg("ML6",65,'<%mAclGetMacFilter("6");%>');
addCfg("ML7",66,'<%mAclGetMacFilter("7");%>');
addCfg("ML8",67,'<%mAclGetMacFilter("8");%>');
addCfg("ML9",68,'<%mAclGetMacFilter("9");%>');
addCfg("ML10",69,'<%mAclGetMacFilter("10");%>');

curNum = getCfg("curNum");
function init(f) {
	var filter_mode_index =  getCfg("check_en");
	f.filter_mode.options.selectedIndex = parseInt(filter_mode_index);
	
	filter_mode_change(f);

	f.curNum.selectedIndex = curNum-1;
	onChangeNum(f);
	isemptyRule();
}

function isemptyRule(){
	isempty=0;
	for(var i=1;i<11;i++) {
		var cl=getCfg("ML"+i);
		if(cl != ""){
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
		document.getElementById("enableTab").style.display = "" ;
	} else {
		document.getElementById("enableTab").style.display="none";
	}
	
	//reset this iframe height by call parent frame function
	window.parent.reinitIframe();
}

//mac,1-0,3600-0,on,desc
function onChangeNum(f) {
	var n = f.curNum.selectedIndex + 1,
		c1 = getCfg("ML"+n).split(","),
		j;
		
	if(getCfg("ML"+n)!="") {
		for( j=0;j<6;j++) {
			var m = (c1[0]).split(":");
			f.mac[j].value = m[j];
		}

		f.startw.value =  (c1[1].split("-"))[0];
		f.endw.value =   (c1[1].split("-"))[1];

		var t= c1[2].split("-");
		
		f.sh.value = parseInt(t[0]/3600)%24;
		f.sm.value = parseInt((t[0]-f.sh.value*3600)/60);
		f.eh.value = parseInt(t[1]/3600)%24;
		f.em.value = parseInt((t[1]-f.eh.value*3600)/60);
		f.en.checked = (c1[3] == "on");		
		if(c1[4] == "notag") {
			c1[4] = "";
		}
		f.remark.value = c1[4];

	} else {
		onDel(f);
	}		
	onSelected();
}

function preSubmit(f) {
	var loc = "/goform/SafeMacFilter?GO=firewall_mac.asp";
	var st,et;
	
	loc += "&check=" + f.elements["filter_mode"].value;
	if(f.elements["filter_mode"].value != "disable") {
		var mac = combinMAC2(f.mac); 
		
		sh = f.sh.value;
		eh = f.eh.value;
		sm = f.sm.value;
		em = f.em.value;
		remark = f.remark.value;
		startw = f.startw.value;
		endw = f.endw.value;
		var num = f.curNum.selectedIndex + 1;
		
		loc += "&curNum=" + num; 

		if (mac.length==0&&remark.length==0) {
			if(isempty != 0 && selectNum[num-1] == "exist"){
				isempty=isempty-1;
			}
		} else {
			if(selectNum[num-1] == "none"){
		    	isempty=isempty+1;
			}
			if (mac=="") { alert("空MAC地址"); return }
			if (!macsCheck(mac,"MAC 地址")) return ;
			if(!ckMacReserve(mac))return ;
			if(Number(eh) < Number(sh)) {
				alert("起始时间 > 结束时间");
				return ;
			}
			if(Number(eh) == Number(sh)) {
				if(Number(em) < Number(sm)) {
					alert("起始时间 >= 结束时间");
					return ;
				}
			}
			if(Number(startw)>Number(endw)) {
			        alert("起始星期 > 结束星期");
					return ;
			}
			//mac,1-0,3600-0,on,desc
			
			loc += "&CL" + num + "=";
			
			loc += mac + ",";
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
	showSaveMassage();
}

function onDel(f) {
	for(var i=0;i<6;i++) {
		f.mac[i].value = "";
	}

	f.remark.value="";
	f.sh.value=0;
	f.eh.value=0;
	f.sm.value=0;
	f.em.value=0;
	f.startw.value=0;
	f.endw.value=6;
	f.en.checked = true;
	onSelected()
}

function onSelected() {
	var frm = document.frmSetup
	if(document.getElementById("en").checked) {
		//document.getElementById("enableTab").disabled = false;
		document.getElementById("remark").disabled = false;
		for(i=0;i<6;i++) {
			frm.mac[i].disabled = false;
		}
		document.getElementById("sh").disabled = false;
		document.getElementById("sm").disabled = false;
		document.getElementById("eh").disabled = false;
		document.getElementById("em").disabled = false;
		document.getElementById("startw").disabled = false;
		document.getElementById("endw").disabled = false;
	} else {
		//document.getElementById("enableTab").disabled = true;	
		document.getElementById("remark").disabled = true;
		for(i=0;i<6;i++) {
			frm.mac[i].disabled = true;
		}
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

<body onLoad="init(document.frmSetup);">
<form name="frmSetup" id="frmSetup" method="POST" action="/goform/SafeMacFilter">
	<fieldset>
		<legend>MAC地址过滤</legend>
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
					
					for(var i=1;i<=max;i++){
						str += '<option value='+i+'>'+ '(' +i+ ')'+'</option>';
					}
					str += '</select>';
					document.write(str);
				</script>
				</td>
			</tr>
			<tr><td class="control-label">注释</td>
				<td class="controls">
				<input class="text" size="12" maxlength="12" id="remark"></td>
			</tr>
			<tr>
				<td class="control-label">MAC 地址</td>
				<td class="controls">
				<input class="text input-mic-mini" id="mac" size=2 maxlength=2 onKeyUp="toNextMac(document.frmSetup, this, 1)">:
				<input class="text input-mic-mini" id="mac" size=2 maxlength=2 onKeyUp="toNextMac(document.frmSetup, this, 2)">:
				<input class="text input-mic-mini" id="mac" size=2 maxlength=2 onKeyUp="toNextMac(document.frmSetup, this, 3)">:
				<input class="text input-mic-mini" id="mac" size=2 maxlength=2 onKeyUp="toNextMac(document.frmSetup, this, 4)">:
				<input class="text input-mic-mini" id="mac" size=2 maxlength=2 onKeyUp="toNextMac(document.frmSetup, this, 5)">:
				<input class="text input-mic-mini" id="mac" size=2 maxlength=2 onKeyUp="toNextMac(document.frmSetup, this)">
				</td>
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
						text += '<option value=' + (j *5)+ '>' + (j*5) + '</option>';
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
					document.write(text);
					</script>
				</td>
			</tr>
			<tr>
				<td class="control-label">日期</td>
				<td class="controls">
				<script>
				var text = '<select id="startw" class="input-small">';
				for(var i=0; i<7; i++){
					text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
				}
				
				text += '</select>&nbsp;~&nbsp;<select id="endw" class="input-small">';
				for(var i=0; i<7; i++){
					text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
				}
				
				text += '</select>';
				document.write(text);
				</script>
				</td>
				</tr>
			<tr>
				<td class="control-label">启用</td>
				<td class="controls">
				<input type="checkbox" id="en" onClick="onSelected()">&nbsp;&nbsp;&nbsp;&nbsp;清空该项:<input type="button" class="btn btn-mini" id="delbtn" value="清 空" onClick="onDel(document.frmSetup)"></td>
			</tr>
		</table>
	</fieldset>
<script>tbl_tail_save("document.frmSetup");</script>    
</form>
<div id="save" class="none"></div>
</body>
</html>