<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>TENDA 11N无线路由器NAT | Virtual Server</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
addCfg("LANIP", 0,"<%aspTendaGetStatus("lan","lanip");%>");
//0;ee;1-2;tcp;10.10.10.5;3;0;;
addCfg("clientList1",1,"<%mNatPortSegment("1");%>");//格式：是否启用IP地址指定网络服务器（0：未启用；1:启用）；开始端口;结束端口；局域网IP地址；协议（0:TCP;1:UDP;2全部）
addCfg("clientList2",2,"<%mNatPortSegment("2");%>");
addCfg("clientList3",3,"<%mNatPortSegment("3");%>");
addCfg("clientList4",4,"<%mNatPortSegment("4");%>");
addCfg("clientList5",5,"<%mNatPortSegment("5");%>");
addCfg("clientList6",6,"<%mNatPortSegment("6");%>");
addCfg("clientList7",7,"<%mNatPortSegment("7");%>");
addCfg("clientList8",8,"<%mNatPortSegment("8");%>");
addCfg("clientList9",9,"<%mNatPortSegment("9");%>");
addCfg("clientList10",10,"<%mNatPortSegment("10");%>");//1;21;192.168.0.62
var max = 10,
	def_describe = "virtual",
	schedcmd = "0",
	prot = ["tcp/udp","tcp","udp"],
	netip = getCfg("LANIP").replace(/\.\d{1,3}$/,".");

function doFill() {
	var f = document.frmSetup,
		p = f.ports[f.ports.selectedIndex].value,
		i = f.ids[f.ids.selectedIndex].value;
		
	f.elements["sport"+i].value=p;
	f.elements["eport"+i].value=p;
}

function delOne(f,idx) {
	f.elements["sport"+idx].value="";
	f.elements["eport"+idx].value="";
	f.elements["pip"+idx].value="";
	f.elements["protocol"+idx].selectedIndex=0;
	f.elements["chk"+idx].checked=0;
	f.elements["del"+idx].checked=0;
}
   		
function init(f) {
	var s, s1;
	for(j=1;j<=max;j++){
   		s = getCfg("clientList" + j);
   		if(s.length > 0){
			//enable;describe;external port;potocol;ip;internal port;schedu type;time;data
			//0;ee;1-2;tcp;10.10.10.5;3;0;;
			s1 = s.split(";");

			//roy modify
			f.elements["pip"+j].value=decomList(s1[4],4,3,".");
			f.elements["sport"+j].value=decomList(s1[2],2,0,"-");
			f.elements["eport"+j].value=decomList(s1[2],2,1,"-");
			if(s1[3] == prot[0]) {
				f.elements["protocol"+j].selectedIndex = 2;
			} else if(s1[3] == prot[1]) {
				f.elements["protocol"+j].selectedIndex = 0;
			} else {
				f.elements["protocol"+j].selectedIndex = 1;
   			}
			f.elements["chk"+j].checked=parseInt(s1[0],10) ;
   		} else {
   			f.elements["pip"+j].value="";
   			f.elements["sport"+j].value="";
			f.elements["eport"+j].value="";
   			f.elements["chk"+j].checked=0;
   		}
   }
   
   	//reset this iframe height by call parent frame function
	window.parent.reinitIframe();
}

function preSubmit(f) {   
	//portlist[i]:是否启用（0：未用；1:启用);开始端口-结束端口；IP；协议(0:TCP;1:UDP;2全部)
	var ip, spt, ept,
		loc = "/goform/VirSerSeg?GO=nat_virtualportseg.asp";

	for (i=1;i<=max;i++) {
		ip = f.elements["pip"+i];
		spt = f.elements["sport"+i];
		ept = f.elements["eport"+i];
		
		if(ip.value=="" && spt.value=="" && ept.value=="") {
			;
		} else {
			if (!rangeCheck(spt,1,65535,"第"+i+"起始端口") ||
					!rangeCheck(ept,1,65535,"第"+i+"结束端口")) {
				return false;
			}
			if ( parseInt(spt.value) > Number(ept.value) ) {
				alert(i+"起始端口 > 结束端口");
				return false;
			}
   			if (!rangeCheck(ip,1,254,"第"+i+"私有IP")){
				return false;
			}
			
			if(f.elements["chk"+i].checked) {
				loc += "&PL" + i + "=1";//该项启用
			} else {
				loc += "&PL" + i + "=0";//未启用
			}
			
			//roy modify
			loc += ";" + def_describe +";" + spt.value +"-" + ept.value + ";";
			if(f.elements["protocol"+i].selectedIndex == 0){
				loc += prot[1];
			}else if(f.elements["protocol"+i].selectedIndex == 1){
				loc += prot[2];
			}else{
				loc += prot[0];
			}
			loc += ";" + netip + ip.value +";" + spt.value +";" + schedcmd + ";" +";";
			
   		}
   }
   window.location =  loc;
   showSaveMassage();
}

function addRule(idx) {
	
	text =	'<tr class=value1 align=center>';
	text += '<td nowrap>'+idx+'.</td>';
	text += '<td nowrap>'; 
	text += '<input maxLength=5 class="text input-mini" name="sport'+idx+'" size="6" >-'; 
	text += '<input maxLength=5 class="text input-mini" name="eport'+idx+'" size="6" >'; 
	text += '</td>';
	text += '<td nowrap>';
	text += netip;
	text += '<input maxLength=3 class="text input-mic-mini" name="pip'+idx+'" size="3" >';   
	text += '</td>';
	text +=	'<td nowrap>&nbsp;<select name="protocol'+idx+'" class="input-small" >';
	text +=	'<option value="0"> TCP </option>';
	text +=	'<option value="1"> UDP </option>';
	text +=	'<option value="2"> 全部 </option>';
	text +=	'</select></td>'; 
	text += '<td nowrap><input type="checkbox" name="chk'+idx+'" value="1"></td>';
	text += '<td nowrap><input type=checkbox name=del'+idx+' onClick=delOne(document.frmSetup,'+idx+');> </td>';
	text += '</tr>';
	document.writeln(text);
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method=POST action="/goform/VirSerSeg">
	<input type=hidden name=GO value=nat_virtualportseg.asp >
	<input type=hidden name=ITEMCNT >
	<fieldset>
		<h2 class="legend">端口段映射</h2>
		<table>
			<tr><td valign="top">端口段映射定义了广域网服务端口范围的访问和局域网网络服务器之间的映射关系，所有对该广域网服务端口段范围内的访问将会被重定位给通过IP地址指定的局域网网络服务器。</td>
			</tr>
		</table>
		<table class="table">
			<thead>
				<tr> 
					<th width="8%">ID</th>
					<th width="25%">开始端口-结束端口</th>
					<th width="25%">内网IP</th>
					<th width="14%">协议</th>
					<th width="14%">启用</th>
					<th width="14%">删除</th>
				</tr>
			</thead>
			<script>
				for(i=1;i<=max;i++) {
					addRule(i);
				}
			</script>
		</table>
		<table class="table">
			<tr align="center"><td>常用服务端口:</td>
				<td nowrap>
					<select class="input-small" name=ports>
						<option selected value=53>DNS(53)</option>
						<option value=21>FTP(21)</option>
						<option value=70>GOPHER(70)</option>
						<option value=80>HTTP(80)</option>
						<option value=119>NNTP(119)</option>
						<option value=110>POP3(110)</option>
						<option value=1723>PPTP(1723)</option>
						<option value=25>SMTP(25)</option>
						<option value=1080>SOCK(1080)</option>
						<option value=23>TELNET(23)</option>
					</select>
				</td>
				<td><input class="btn btn-mini" name=fill onClick="doFill()" type="button" value="填充到&gt;&gt;"></td>
				<td>ID
					<select class="input-small" name=ids> 
						<option selected value=1>1</option>
						<option value=2>2</option>
						<option value=3>3</option>
						<option value=4>4</option>
						<option value=5>5</option>
						<option value=6>6</option>
						<option value=7>7</option>
						<option value=8>8</option>
						<option value=9>9</option>
						<option value=10>10</option>
					</select>
				</td>
			</tr>
		</table>
	</fieldset>
    <script>tbl_tail_save("document.frmSetup");</script>
</form>
<div id="save" class="none"></div>
</body>
</html>