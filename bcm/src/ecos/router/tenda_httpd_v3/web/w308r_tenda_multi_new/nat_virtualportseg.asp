<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Virtual Server</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
B.setTextDomain(["base", "applications"]);
</script>
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
	
	//handle translate
	B.translate();
	
	for(j=1;j<=max;j++){
   		s = getCfg("clientList" + j);
   		if(s.length > 0){
			//enable;describe;external port;potocol;ip;internal port;schedu type;time;data
			//0;ee;1-2;tcp;10.10.10.5;3;0;;
			s1 = s.split(";");

			//roy modifyl
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
			if (!rangeCheck(spt,1,65535,_("start port for item %s",[i])) ||
					!rangeCheck(ept,1,65535,_("end port for item %s",[i]))) {
				return false;
			}
			if ( parseInt(spt.value) > Number(ept.value) ) {
				alert(_("Please specify a valid start/end port value between 1-65535! Start port must be smaller than end port!"));
				return false;
			}
   			if (!rangeCheck(ip,1,254,_("IP address for item %s",[i]))){
				return false;
			}
			
			if(f.elements["chk"+i].checked) {
				loc += "&PL" + i + "=1";//该项启用
			} else {
				loc += "&PL" + i + "=0";//未启用
			}
			
			//roy modify
			loc += ";" + def_describe +";" + spt.value +"-" + ept.value + ";";
			if (f.elements["protocol"+i].selectedIndex == 0) {
				loc += prot[1];
			} else if (f.elements["protocol"+i].selectedIndex == 1) {
				loc += prot[2];
			} else {
				loc += prot[0];
			}
			loc += ";" + netip + ip.value +";" + spt.value +";" + schedcmd + ";" +";";
			
   		}
		for (j=i+1;j<=max;j++){
			if(ip.value == f.elements["pip"+j].value && spt.value == f.elements["sport"+j].value && ept.value == f.elements["eport"+j].value && f.elements["protocol"+i].selectedIndex == f.elements["protocol"+j].selectedIndex && f.elements["pip"+j].value != "" && f.elements["sport"+j].value != "" && f.elements["eport"+j].value != ""){
				alert(_("The same rules cannot be set!"))
				return false;
			}
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
	text +=	'<option value="2"> '+_("Both")+' </option>';
	text +=	'</select></td>'; 
	text += '<td nowrap><input type="checkbox" name="chk'+idx+'" value="1"></td>';
	text += '<td nowrap><input type=checkbox name=del'+idx+' onClick=delOne(document.frmSetup,'+idx+');> </td>';
	text += '</tr>';
	document.writeln(text);
}
</script>
</head>
<body onLoad="init(document.frmSetup);" style="min-height:601px"> <!-- 设置最小高度 -->
<form name=frmSetup method=POST action="/goform/VirSerSeg">
	<input type=hidden name=GO value=nat_virtualportseg.asp >
	<input type=hidden name=ITEMCNT >
	<fieldset>
		<legend>Port Range Forwarding</legend>
		<table>
			<tr><td valign="top">NAT_Portseg_MSG</td>
			</tr>
		</table>
		<table class="table">
			<thead>
				<tr> 
					<th width="8%">NO.</th>
					<th width="25%">Start Port-End Port</th>
					<th width="25%">LAN IP</th>
					<th width="14%">Protocol</th>
					<th width="14%">Enable</th>
					<th width="14%">Delete</th>
				</tr>
			</thead>
			<script>
				for(i=1;i<=max;i++) {
					addRule(i);
				}
			</script>
		</table>
		<table class="table">
			<tr align="center"><td>Well-known service ports:</td>
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
				<td><input class="btn btn-mini" name=fill onClick="doFill()" type="button" value="Add to"></td>
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