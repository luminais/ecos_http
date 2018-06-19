<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<title>WAN | Triffic Control</title>
<link rel="stylesheet" href="css/screen.css">
</head>
<body class="net-tc">
<form name="trafficCtl" method="POST" action="/goform/TrafficCtl">
<input type=hidden name=GO value=net_tc.asp >
	<fieldset>
		<legend>Bandwidth Control</legend>
		<div class="control-group">
			<label class="control-label">Enable Bandwidth Control</label>
			<div class="controls">
				<label class="checkbox"><input type=CHECKBOX id="check"  onClick="onCheck()">Enable</label>
			</div>
		</div>
		<table id="bandwith_box">
		   <tr>
			<td class="control-label">IP Address</td>
			<td class="controls"><span id="netip_prefix"></span>
				<input type="text" name="startIP" class="text input-mini" size="3" maxlength="3"><b>~</b>
				<input type="text" name="endIP" class="text input-mini" size="3" maxlength="3"></td></tr>
		   <tr>
			<td class="control-label">Upload/Download</td>
			<td class="controls">
				<select name="RC_link">
				  <option value="0">Upload</option>
				  <option value="1">Download</option>
				</select>
			</td>
		   </tr>
		  <tr><td class="control-label">Bandwidth Range</td>
			<td  class="controls">
				<input type="text" name="minRate" maxlength="5" size="5" class="text input-small" >~
				<input type="text" name="maxRate" maxlength="5" size="5" class="text input-small">(KByte/s)
			</td>
		  </tr>
		  <tr>
			<td class="control-label">Enable</td>
			<td class="controls"><input type="checkbox" name="RC_valid" value="1"> </td>
		</tr>
		</table>
		<div class="btn-group">
			<input  class="btn btn-small" type=button value="Add To List" name="add_modify" onClick="onAdd()">
		</div>
		<div id="fluxCtlList" style="position:relative;visibility:visible;"></div>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.trafficCtl)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.trafficCtl)" />
	</div>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "qos_security"]);
	B.translate();
})();
</script>
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var LANIP = "<%get_tc_othe("lanip");%>",
	maxlist = 10,
	ispUp = "<%get_tc_othe("isp_uprate");%>",
	ispDown = "<%get_tc_othe("isp_downrate");%>",
	check_en = "<%get_tc_othe("tc_en");%>",	//带宽控制:0:未启用；1:启用

	//0port,1sip,2eip,3(0:up,1:down),4minrate,5maxrate,6(1:enable,0:disable),7(0:TCP&UDP,1:tcp,2:udp)
	//ex:'21,123,124,0,512,512,1,1','65535,111,125,1,123,456,0,2'
	//0:代表所有端口
	resList,
	netip = LANIP.replace(/\.\d{1,3}$/,"."),
	editTag = 0,//0:add n:edit
	btnVal = [_("Add To List"),_("Update To List")],
	pro = ["TCP&UDP","TCP","UDP"],
	Pro_Por_Ser = [
		//0:both;1:tcp;2:udp
		[0,"0",_("All services")],
		[1,"0",_("All services")+"(TCP)"],
		[2,"0",_("All services")+"(UDP)"],
		[2,"53","DNS"],
		[1,"21","FTP"],
		[1,"80","HTTP"],
		[1,"8080","HTTP Secondary"],
		[1,"443","HTTPS"],
		[1,"8443","HTTPS Secondary"],
		[2,"69","TFTP"],
		[1,"143","IMAP"],
		[1,"119","NNTP"],
		[1,"110","POP3"],
		[2,"161","SNMP"],
		[1,"25","SMTP"],
		[1,"23","TELNET"],
		[1,"8023","TELNET Secondary"],
		[1,"992","TELNETSSL"],
		[2,"67","DHCP"]
	];	 		

//客户端过滤是否启用
function onCheck() {
	if(document.getElementById("check").checked) {
		//document.getElementById("table1").style.display = "";
		document.getElementById("bandwith_box").style.display = "";
		document.getElementById("fluxCtlList").style.display = "";
		
		//reset this iframe height by call parent iframe function
		window.parent.reinitIframe();
	} else {
		//document.getElementById("table1").style.display = "none";
		document.getElementById("bandwith_box").style.display = "none";
		document.getElementById("fluxCtlList").style.display = "none";
	}
}

function preSubmit(f) {
	//var upbw = parseInt(f.up_Band.value);	
	//var downbw = parseInt(f.down_Band.value);
	var upbw = 12800;
	var downbw =12800;
	if(isNaN(upbw) || isNaN(downbw) ) {
	    alert(_("WAN port bandwidth can not be empty!"));
		return;
	}
	if((0 == upbw)||(0 == downbw)) {
	    alert(_("The value of bandwidth should not be 0! Please enter a valid bandwidth value."));
		return;
	}
	var loc = "/goform/trafficForm?GO=net_tc.asp";
	if (document.getElementById("check").checked) {
	   loc += "&tc_enable=1";//
	} else {
	    loc += "&tc_enable=0";
	}
	loc += "&up_Band=" + upbw;
	loc += "&down_Band=" + downbw;
	loc += "&cur_number=" + resList.length;
	for(var j=0; j<resList.length; j++) {
		loc += "&tc_list_" + (j+1) + "=" + resList[j] ;
	}
	window.location = loc;
}

function showList(){
	var m='<table class="table" id="showTab">';
	m+='<thead><tr>';
	m+='<th nowrap>'+_("No.")+'</th>';
	m+='<th nowrap>'+_("IP Range")+'</th>';
	m+='<th nowrap>'+_("Destination")+'</th>';
	m+='<th nowrap>'+_("Bandwidth Range")+'</th>';
	m+='<th nowrap>'+_("Enable")+'</th>';
	m+='<th nowrap>'+_("Edit")+'</th>';
	m+='<th nowrap>'+_("Delete")+'</th>';
	m+='</tr></thead><tbody>';
	for (i=0;i<resList.length;i++) {
		var s=resList[i].split(",");
		if (s.length!=8) 
			break;
		m+='<tr align=center>';
		m+='<td>'+(i+1)+'</td>';
		m+='<td>'+netip+s[1]+'~'+s[2]+'</td>';
		m+='<td>'+ (parseInt(s[3],10) ? _("Download") : _("Upload")) + '</td>'; //0,上传,1,下载
		m+='<td>'+s[4]+'~'+s[5]+'</td>';
		m+='<td>' + (parseInt(s[6],2) ? "&radic;" : "&times;") + '</td>';
		m+='<td><input type=button class="btn btn-mini" value='+_("Edit")+' onclick="onEdit(' + i +  ')"></td>';
		m+='<td><input type=button class="btn btn-mini" value='+_("Delete")+' onclick="OnDel(this,' + i +  ')"></td>';
		m+='</tr>';
	}
	m += '</tbody></table>';
	document.getElementById("fluxCtlList").innerHTML = m;
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function onAdd(){
	var f = document.forms[0];
	//var upbw = f.up_Band.value;	
	//var downbw = f.down_Band.value;
	var upbw =12800;
	var downbw =12800;	
	//if( ((upbw>12800)||(upbw<1) )  || ( (downbw>12800)||(downbw<1)))
	//{
	//    alert("WAN带宽有效值范围为1~12800,请重新输入！");
	//	return;
	//}	
	if (resList.length == maxlist && editTag == 0) {
	    alert(_("Up to %s bandwidth control entries can be added.",[maxlist]));
		return;
	}	
	//var pt = f.servicePort.value;
	var pt = 80;
	var sip = parseInt(f.startIP.value,10);
	var eip = parseInt(f.endIP.value,10);	
	if (isNaN(sip)||isNaN(eip)) {
	    alert(_("Please enter the IP address"));
		return;
	}
	if (!rangeCheck(f.startIP,1,254,_("Start IP"))||
			!rangeCheck(f.endIP,1,254,_("End IP"))) return ;
	if(eip < sip) {
		alert(_("Start IP should not be greater than End IP! Please enter valid IP information."));
		return ;
	}	
	var linkTy = f.RC_link.value;
	var sbw = parseInt(f.minRate.value,10);
	var ebw = parseInt(f.maxRate.value,10);
	if(isNaN(sbw) || isNaN(ebw)) {
	    alert(_("Please enter a valid bandwidth value!"));
		return;
	}
	if((0 == sbw)||(0 == ebw)) {
	    alert(_("The value of bandwidth should not be 0! Please enter a valid bandwidth value."));
		return;
	}
//huangxiaoli add
	if (!rangeCheck(f.minRate,0,999999,"最低带宽")||
			!rangeCheck(f.maxRate,0,999999,"最高带宽")) return ;
//end add
	var en = (f.RC_valid.checked) ? 1 : 0;
	//var pro = f.protocol.value;
	var pro = 1;
	if(pt > 65535 || pt < 0) {
		alert(_("The port is beyond the range of 0~65535!"));
		return ;
	}	
	if(eip < sip) {
		alert(_("start IP bigger than end IP"));
		return ;
	}	
	if(ebw < sbw) {
		alert(_("Starting bandwidth value should not be greater than ending bandwidth!"));
		return ;
	}	

   	//0port,1sip,2eip,3(0:up,1:down),4minrate,5maxrate,6(1:enable,0:disable),7(0:TCP&UDP,1:tcp,2:udp)   	
	var allinfo = pt+','+sip+','+eip+','+linkTy+','+sbw+','+ebw+','+en+','+pro;
	var comp1 = allinfo.split(",");
	var comp2;
	for (var k=0;k<resList.length;k++) {
		if(editTag){
			if(k == editTag - 1)
				continue;
		}
		comp2 = resList[k].split(",");
		if(comp1[3] == comp2[3] && (comp1[0] == comp2[0]) && (comp1[7] == comp2[7])
		   || comp1[3] == comp2[3] && ((parseInt(comp1[0]) == 0 && parseInt(comp2[0]) != 0) || (parseInt(comp1[0]) != 0 && parseInt(comp2[0]) == 0))
		   || comp1[3] == comp2[3] && ((parseInt(comp1[7]) == 0 && parseInt(comp2[7]) != 0) || (parseInt(comp1[7]) != 0 && parseInt(comp2[7]) == 0)))
		{
			if(parseInt(comp1[1])>=parseInt(comp2[1])&&parseInt(comp1[1])<=parseInt(comp2[2])||
			   parseInt(comp1[2])>=parseInt(comp2[1])&&parseInt(comp1[2])<=parseInt(comp2[2]) ||
			   parseInt(comp1[1])<parseInt(comp2[1])&&parseInt(comp1[2])>parseInt(comp2[2]))
			{
				alert(_("The same IP address can't be contained in two Bandwidth control rules, please retry."));
				return ;
			}   
		}

	}		
	if (editTag == 0) {
		resList[resList.length]=allinfo;
	} else {
		resList[editTag-1]=allinfo;
		f.add_modify.value  = btnVal[0];
	}
	editTag = 0;
	showList();
}

function onEdit(n) {
	var f = document.forms[0];
	var s = (resList[n]).split(",");
	//0port,1sip,2eip,3(0:up,1:down),4minrate,5maxrate,6(1:enable,0:disable),7(0:TCP&UDP,1:tcp,2:udp)
	//f.servicePort.value = s[0];
	f.startIP.value 	= s[1];
	f.endIP.value		= s[2];
	f.RC_link.value 	= s[3];
	f.minRate.value 	= s[4];
	f.maxRate.value		= s[5];
	f.RC_valid.checked	= parseInt(s[6],2);
	//f.protocol.value	= s[7];
	editTag = n+1;
	f.add_modify.value  = btnVal[1];
}

function OnDel(obj,dex){
	document.getElementById("showTab").deleteRow(dex+1);
	var i = 0;
	for(i=dex;i<resList.length;i++) {
		resList[i] = resList[i + 1];
	}
	resList.length--;
	document.forms[0].add_modify.value  = btnVal[0];
	showList();
}

function onselPort(){
	var f = document.forms[0],
		i1;
	for(i1=0; i1<Pro_Por_Ser.length; i1++) {
		if (i1 == f.elements["selPort"].value ){
		  break;
		}
		
    }	
	f.elements["protocol"].selectedIndex = parseInt (Pro_Por_Ser[i1][0]);
	f.elements["servicePort"].value = Pro_Por_Ser[i1][1];	
}

function show_Option_Pro() {
   var i,in_html;
   for (i=0; i<pro.length; i++) {
      in_html += "<option value="+i+">"+pro[i]+"</option>";
   }
   document.write(in_html);
}
function show_Option_Ser(){
   var i1,in_html='';
   for(i1=0; i1<Pro_Por_Ser.length; i1++) {        

		in_html += "<option value="+i1+">"+Pro_Por_Ser[i1][2]+"</option>";	
   }
   document.write(in_html);
}

function init2() {
	if(check_en == 1) {
		document.getElementById("check").checked = true;
	} else {
		document.getElementById("check").checked = false;
	}
	onCheck();
}

function init(f){
	document.getElementById("netip_prefix").innerHTML = netip;
	init2();
    resList = new Array(<%get_tc_list();%>);
	showList();
}

window.onload = function () {
	init(document.trafficCtl);
}
</script>
</body>
</html>