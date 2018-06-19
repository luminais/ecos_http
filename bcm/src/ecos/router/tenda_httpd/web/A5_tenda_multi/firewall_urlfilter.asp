<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>Firewall | URL Filtering</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
Butterlate.setTextDomain("dhcp_firewall");
var max=10;
var isempty=0;
var selectNum = new Array();
var tmp="";
var lanip = "<%getfirewall("lan","lanip");%>";
addCfg("lanip",10,lanip);
addCfg("check_en",70,"<%getfirewall("lan","url_en");%>");
addCfg("curNum",72,"<%getfirewall("lan","urlNum");%>");
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
var netip=getCfg("lanip").replace(/\d{1,3}$/,"");
var curNum = getCfg("curNum");
var weekday=new Array(_("Sunday"),_("Monday"),_("Tuesday"),_("Wednesday"),_("Thursday"),_("Friday"),_("Saturday"));
var filter_mode = new Array(_("Disable"),_("Forbid only")/*,_("Permit only")*/);
var filter_mode_value = new Array("disable","deny");//,"pass");
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
	isemptyRule();
}

function isemptyRule()
{
	isempty=0;
	for(var i=1;i<11;i++)
	{
		var cl=getCfg("UF"+i);
		if(cl != ""){
			selectNum[i-1] = "exist";
			isempty=isempty+1;
		}else{
			selectNum[i-1] = "none";	
		}
	}
}
function onChangeNum(f)
{
	var n = f.curNum.selectedIndex + 1;
	var c1 = getCfg("UF"+n).split(":");
	//huangxiaoli add for ':'
	if(c1.length > 2)
	{
		var c11='';
		for(var i=1;i<c1.length;i++)
		{
			if(i != c1.length-1){
				c11+=c1[i]+":";
			}else{
				c11+=c1[i];
			}
		}
	}
	else{
		c11=c1[1];
	}
	//add end
	if(getCfg("UF"+n)!="")
	{
		var ip=c1[0].split("-");
		f.sip.value = (ip[0].split("."))[3];
		f.eip.value = (ip[1].split("."))[3];
		var c2 = c1[1].split(",");
		f.url.value = c2[0];
		f.startw.value =  (c2[1].split("-"))[0];
		f.endw.value =   (c2[1].split("-"))[1];
		var t= c2[2].split("-");
		f.sh.value = parseInt(t[0]/3600)%24;
		f.sm.value = parseInt((t[0]-f.sh.value*3600)/60);
		f.eh.value = parseInt(t[1]/3600)%24;
		f.em.value = parseInt((t[1]-f.eh.value*3600)/60);
		f.en.checked = (c2[3] == "on");
		if(c2[4] == "notag")
		{
			c2[4] = "";
		}
		f.remark.value = c2[4];
	}
	else
	{
		onDel(f);
	}
	onSelected();	
}
function preSubmit(f) 
{
	var loc = "/goform/SafeUrlFilter?GO=firewall_urlfilter.asp";
	var st,et;
	loc += "&check=" + f.elements["filter_mode"].value;			
	if(f.elements["filter_mode"].value != "disable")
	{			
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
	var num = f.curNum.selectedIndex + 1;
	loc += "&curNum=" + num; 
			
		if ((sip.length==0)&&(eip.length==0)&&(remark.length==0)&&(urlstr.length==0)) {
			if(isempty != 0 && selectNum[num-1] == "exist"){
				isempty=isempty-1;
			}
		} 
		else 
		{
			if(selectNum[num-1] == "none"){
		    	isempty=isempty+1;
			}
	var illegal_user_pass = new Array(",");
	var re =/^[0-9a-zA-Z_\-.:]+$/;	
	if (!rangeCheck(f.sip,1,254,_("Start IP"))||
		!rangeCheck(f.eip,1,254,_("End IP"))) return ;
	if ( Number(sip) > Number(eip) ) { alert(_("Start IP")+" > "+_("End IP")); return ; }
	if(!ill_check(f.url.value,illegal_user_pass,_("URL character string"))) return false;
			if(f.url.value == "" || !re.test(f.url.value)){
				alert(_("contains illegal characters"));
				return ;
			}
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
			sip = clearInvalidIpstr(sip);
			eip = clearInvalidIpstr(eip);
	loc += netip + sip + "-" + netip + eip + ":";
	loc += urlstr + ",";
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
		}
	if(f.elements["filter_mode"].value=="pass"&&isempty==0)
	 {
			isemptyRule();
	 alert(_("one rule must be entered when 'permit only' is selected"));
	 return false;
	}
    }	
   var code = 'location="' + loc + '"';
   eval(code);
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
function onDel(f)
{
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
function onSelected()
{
	var frm = document.frmSetup
	if(document.getElementById("en").checked)
	{
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
	}
	else
	{
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
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
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
				<form name=frmSetup id="frmSetup" method=POST action=/goform/SafeUrlFilter>
				<table class=content1>
				<tr><td height="30"><script>document.write(_("Filter Mode"));</script> :&nbsp;
				<script>
				var text = '<select id="filter_mode" onChange="filter_mode_change(document.frmSetup)">';
				for(var i=0; i<2; i++){
					text +=	'<option value='+filter_mode_value[i]+' >'+filter_mode[i]+'</option>';	
				}
				text += '</select>';
				document.write(text);
				</script>
				</td>
				</tr>
				</table>
				<table id="enableTab" cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
				<tr>
				<td width="150" height="30" align="right"><script>document.write(_("Access Policy"));</script>:</td>
				<td>&nbsp;&nbsp;&nbsp;&nbsp;<script>
					var str = '<select id=curNum onChange=onChangeNum(document.frmSetup);>';
					for(var i=1;i<=max;i++)
					{
						str += '<option value='+i+'>'+ '(' +i+ ')'+'</option>';
					}
					str += '</select>';
					document.write(str);
				</script>
				</td>
				</tr>
				<tr>
				<td height="30" align="right"><script>document.write(_("Remarks"));</script>:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" size="12" maxlength="12" id="remark"></td>
				</tr>
				<tr>
				<td height="30" align="right"><script>document.write(_("Start IP"));</script>:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(netip);</script><input class="text" size="3" maxlength="3" id="sip"></td>
				</tr>
				<tr>
				<td height="30" align="right"><script>document.write(_("End IP"));</script>:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(netip);</script><input class="text" size="3" maxlength="3" id="eip"></td>
				</tr>
				<tr>
				<td height="30" align="right"><script>document.write(_("URL character string"));</script>:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<input class="text" size="30" maxlength="128" id="url"></td>
				</tr>
				<tr>
				<td height="30" align="right"><script>document.write(_("Time"));</script>:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<script>
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
				</script>
				</td>
				</tr>
				<tr>
				<td height="30" align="right"><script>document.write(_("Date"));</script>:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<script>
				var text = '<select id="startw">';
				for(var i=0; i<7; i++){
					text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
				}
				text += '</select>&nbsp;~&nbsp;<select id="endw">';
				for(var i=0; i<7; i++){
					text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
				}
				text += '</select>';
				document.write(text);
				</script>
				</td>
				</tr>
				<tr><td height="30" align="right"><script>document.write(_("Enable"));</script>:</td>
				  <td height="30">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" id="en" onClick="onSelected()">&nbsp;&nbsp;&nbsp;&nbsp;<script>document.write(_("Clear this item"));</script>:<input type="button" class="button2" id="delbtn" value=""  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="onDel(document.frmSetup)"></td>
				</tr>
				</table>
				<SCRIPT>tbl_tail_save("document.frmSetup",'urlfilter');</SCRIPT>
			</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
<script>
helpInfo(_("Firewall_URLFilter_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Firewall_URLFilter_Help_Inf2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Firewall_URLFilter_Help_Inf3"));
</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('enableTab');
    </script>
</BODY>
</HTML>






