<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>Firewall | Client Filtering</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("dhcp_firewall");
var max=10;
var isempty=0;
var tmp="";
var lanip = "<%getfirewall("lan","lanip");%>";
addCfg("lanip",10,lanip);
addCfg("check_en",70,"<%getfirewall("lan","acl_en");%>");
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
var netip=getCfg("lanip").replace(/\d{1,3}$/,"");
var curNum = getCfg("curNum");
var prot= new Array("tcp","udp","tcp/udp");
var weekday=new Array(_("Sunday"),_("Monday"),_("Tuesday"),_("Wednesday"),_("Thursday"),_("Friday"),_("Saturday"));
var filter_mode = new Array(_("Disable"),_("Forbid only"),_("Permit only"));
var filter_mode_value = new Array("disable","deny","pass");
function initTranslate(){
	var e=document.getElementById("delbtn");
	e.value=_("Clear");
}
function init(f)
{
	var filter_mode_index =  getCfg("check_en");
	f.filter_mode.options.selectedIndex = parseInt(filter_mode_index);
	filter_mode_change(f);
	f.curNum.selectedIndex = curNum-1;
	onChangeNum(f);
	initTranslate();
}
function filter_mode_change(f)
{
	var on = f.filter_mode.options.selectedIndex;
	if(on != 0){
		document.getElementById("enableTab").style.display ="";
	}else{
		document.getElementById("enableTab").style.display ="none";
	}
}
function preSubmit(f)
{
	var loc = "/goform/SafeClientFilter?GO=firewall_clientfilter.asp";
	var st,et;
	loc += "&check=" + f.elements["filter_mode"].value;		
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
	var num = f.curNum.selectedIndex + 1;
	loc += "&curNum=" + num; 
	isempty=isempty+1;
	if (!rangeCheck(f.sip,1,254,_("Start IP"))||
		!rangeCheck(f.eip,1,254,_("End IP"))) return ;
	if ( Number(sip) > Number(eip) ) { alert(_("Start IP")+" > "+_("End IP")); return ; }
	if (!rangeCheck(f.sport,1,65535,_("Start Port"))||
		!rangeCheck(f.eport,1,65535,_("End Port"))) return ;
	if ( Number(spt) > Number(ept) ) { alert(_("Start Port")+" > "+_("End Port")); return ; }
	if(Number(eh) < Number(sh))
	{
		alert(_("Start Time")+" > "+_("End Time"));
		return ;
	}
	if(Number(eh) == Number(sh))
	{
		if(Number(em) < Number(sm))
		{
			alert(_("Start Time")+" > "+_("End Time"));
			return ;
		}
	}
	if(Number(startw)>Number(endw))
	{
			alert(_("Start Week")+" > "+_("End Week"));
			return ;
	}
	loc += "&CL" + num + "=";
	loc += netip + sip + "-" + netip + eip + ":";
	loc += spt + "-" + ept + ",";
	loc += prot[ f.protocol.value] +",";
	loc += startw + "-" + endw +",";
	st = sh*3600+sm*60;
	et = eh*3600+em*60;
	loc += st + "-" + et+",";
	if(f.en.checked)
		loc += "on" + ",";
	else
		loc += "off" + ",";	
	if(remark == "notag")
	{
		alert(_("not to enter 'notag' in the Remarks field"));
		return ;
	}
	if(remark == "")
		remark = "notag";
	else
	{
		if(!ckRemark(remark))return ;
	}
	loc += remark;
	if(f.elements["filter_mode"].value=="pass"&&isempty==0)
	{
	 for(i=1;i<11;i++)
	 {
	  var cl=getCfg("CL"+i);
	  tmp+=cl;
	 }
	 if(tmp=="")
	 {
	 alert(_("one rule must be entered when 'permit only' is selected"));
	 return false;
	 }
	}
   var code = 'location="' + loc + '"';
   eval(code);
}
function onChangeNum(f)
{	
	var n = f.curNum.selectedIndex + 1;
	var i;
	var c1 = getCfg("CL"+n).split(":");
	if(getCfg("CL"+n)!="")
	{
		var ip=c1[0].split("-");
		f.sip.value = (ip[0].split("."))[3];
		f.eip.value = (ip[1].split("."))[3];
		var c2 = c1[1].split(",");
		f.sport.value = (c2[0].split("-"))[0];
		f.eport.value = (c2[0].split("-"))[1];
		for(i=0;i<3;i++){
			if(c2[1] == prot[i])
				f.protocol.selectedIndex = i;
		}
		f.startw.value =  (c2[2].split("-"))[0];
		f.endw.value =   (c2[2].split("-"))[1];
		var t= c2[3].split("-");
		f.sh.value = parseInt(t[0]/3600)%24;
		f.sm.value = parseInt((t[0]-f.sh.value*3600)/60);
		f.eh.value = parseInt(t[1]/3600)%24;
		f.em.value = parseInt((t[1]-f.eh.value*3600)/60);
		f.en.checked = (c2[4] == "on");
		if(c2[5] == "notag")
		{
			c2[5] = "";
		}
		f.remark.value = c2[5];
	}
	else
	{
		onDel(f);
	}
	onSelected();	
}
function onDel(f)
{
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
}
function onSelected()
{
	var frm = document.frmSetup
	if(document.getElementById("en").checked)
	{
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
	}
	else
	{
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
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">		
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup method=POST action=/goform/SafeClientFilter>
					<table class=content1>
					<tr><td height="30"><script>document.write(_("Filter Mode"));</script>:&nbsp;
					<script>
					var text = '<select id="filter_mode" onChange="filter_mode_change(document.frmSetup)">';
					for(var i=0; i<3; i++){
						text +=	'<option value='+filter_mode_value[i]+' >'+filter_mode[i]+'</option>';	
					}
					text += '</select>';
					document.write(text);
					</script>
					</td></tr>
					</table>
					<table id="enableTab" cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
					<tr>
					  <td width="150" height="30" align="right"><script>document.write(_("Access Policy"));</script>:</td>
					  <td>
					  &nbsp;&nbsp;&nbsp;&nbsp;<script>
						var str = '<select id=curNum onChange=onChangeNum(document.frmSetup);>';
						for(var i=1;i<=max;i++)
						{
							str += '<option value='+i+'>'+ '(' +i+ ')'+'</option>';
						}
						str += '</select>';
						document.write(str);
					</script>					</td>
					</tr>
					<tr><td height="30" align="right"><script>document.write(_("Remarks"));</script>:</td>
					  <td height="30">
					  &nbsp;&nbsp;&nbsp;&nbsp;<input class="text" id="remark" size="12" maxlength="12"></td>
					</tr>
					<tr>
					  <td height="30" align="right"><script>document.write(_("Start IP"));</script>:</td>
					  <td height="30">
					  &nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(netip);</script><input class="text" id="sip" size="3" maxlength="3"></td>
					</tr>
					<tr>
					  <td height="30" align="right"><script>document.write(_("End IP"));</script>:</td>
					  <td height="30">
					  &nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(netip);</script><input class="text" id="eip" size="3" maxlength="3"></td>
					</tr>
					<tr><td height="30" align="right"><script>document.write(_("Port"));</script>:</td>
					  <td height="30">
					 &nbsp;&nbsp;&nbsp;&nbsp;<input class="text" id="sport" size="4" maxlength="5">~<input class="text" id="eport" size="4" maxlength="5"></td>
					</tr>
					<tr><td height="30" align="right"><script>document.write(_("Type"));</script>:</td>
					<td height="30">
					  &nbsp;&nbsp;&nbsp;&nbsp;<select id="protocol" ><option value="0"> TCP </option><option value="1"> UDP </option><option value="2"> <script>document.write(_("Both"));</script> </option></select></td>  
					</tr>
					<tr>
					 <td height="30" align="right"><script>document.write(_("Time"));</script>:</td>
					 <td height="30">
					&nbsp;&nbsp;&nbsp;&nbsp;<script>
					var text = '<select id="sh">';
					for(var i=0; i<=23; i++){
						text +=	'<option value='+i+' >'+i+'</option>';	
					}
					text += '</select>:<select id="sm">';
					for(var j=0;j<12;j++)
					{
						text += '<option value=' + eval(j *5)+ '>' + eval(j*5) + '</option>';
					}
					text += '</select>&nbsp;~&nbsp;<select id="eh">';
					for(var i=0; i<=23; i++){
						text +=	'<option value='+i+' >'+i+'</option>';	
					}
					text += '</select>:<select id="em">';
					for(var j=0;j<12;j++)
					{
						text += '<option value=' + eval(j *5)+ '>' + eval(j*5) + '</option>';
					}
					text += '</select>';
					document.write(text);
					</script>					</td>
					</tr>
					<tr>
					 <td height="30" align="right"><script>document.write(_("Date"));</script>:</td>
					 <td height="30">
					&nbsp;&nbsp;&nbsp;&nbsp;<script>
					var text = '<select id="startw" name="startw">';
					for(var i=0; i<7; i++){
						text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
					}
					text += '</select>&nbsp;~&nbsp;<select id="endw" name="endw">';
					for(var i=0; i<7; i++){
						text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
					}
					text += '</select>';
					document.write(text);
					</script>					</td>
					</tr>
					<tr><td height="30" align="right"><script>document.write(_("Enable"));</script>:</td>
					  <td height="30" colspan="2">&nbsp;&nbsp;&nbsp;&nbsp;<input name="checkbox" type="checkbox" id="en" onClick="onSelected()">					    &nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(_("Clear this item"));</script>:
					    <input type="button" class="button2" id="delbtn" value=""  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="onDel(document.frmSetup)"></td>
					</tr>
					</table>
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
<script>helpInfo(_("Firewall_clientfilter_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Firewall_clientfilter_Help_Inf2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Firewall_clientfilter_Help_Inf3")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Firewall_clientfilter_Help_Inf4")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Firewall_clientfilter_Help_Inf5"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('enableTab');
    </script>
</BODY>
</HTML>


