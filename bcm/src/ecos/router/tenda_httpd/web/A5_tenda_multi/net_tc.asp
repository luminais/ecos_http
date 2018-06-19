<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>Bandwidth | Control</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var LANIP = "<%get_tc_othe("lanip");%>";
var maxlist = 10;
var ispUp = "<%get_tc_othe("isp_uprate");%>";
var ispDown = "<%get_tc_othe("isp_downrate");%>";
var check_en = "<%get_tc_othe("tc_en");%>";//´ø¿í¿ØÖÆ:0:Î´ÆôÓÃ£»1:ÆôÓÃ
Butterlate.setTextDomain("wan_set");
var resList;
var netip=LANIP.replace(/\.\d{1,3}$/,".");
var editTag = 0;//0:add n:edit
var btnVal = [_("Add to list"),_("Update to list")];
var pro = ["TCP&UDP","TCP","UDP"];
var Pro_Por_Ser = new Array
(
         new Array(0,"0",_("All services")),
		 new Array(1,"0",_("All services")+"(TCP)"),
		 new Array(2,"0",_("All services")+"(UDP)"),
         new Array(2,"53","DNS"),
         new Array(1,"21","FTP"),
         new Array(1,"80","HTTP"),
         new Array(1,"8080","HTTP Secondary"),
         new Array(1,"443","HTTPS"),
         new Array(1,"8443","HTTPS Secondary"),
         new Array(2,"69","TFTP"),
         new Array(1,"143","IMAP"),
         new Array(1,"119","NNTP"),
         new Array(1,"110","POP3"),
         new Array(2,"161","SNMP"),
         new Array(1,"25","SMTP"),
         new Array(1,"23","TELNET"),
         new Array(1,"8023","TELNET Secondary"),
         new Array(1,"992","TELNETSSL"),
         new Array(2,"67","DHCP")
)	 		
function initTranslate(){
	var e=document.getElementById("add");
	e.value=_("Add to list");
}
function init(f)
{
    initTranslate();
	init2();
    	resList = new Array(<%get_tc_list();%>);
	showList();
}
function init2()
{
   if(check_en == 1)
		document.getElementById("check").checked = true;
	else
		document.getElementById("check").checked = false;
	onCheck();
}
function onCheck()
{
	if(document.getElementById("check").checked)
	{
		document.getElementById("table2").style.display = "";
		document.getElementById("fluxCtlList").style.display = "";
	}
	else
	{
		document.getElementById("table2").style.display = "none";
		document.getElementById("fluxCtlList").style.display = "none";
	}
}
function preSubmit(f) 
{
	var upbw = 12800;
	var downbw =12800;
	if(isNaN(upbw) || isNaN(downbw) )
	{
	    alert(_("WAN port bandwidth can not be empty!"));
		return;
	}
	if((0 == upbw)||(0 == downbw))
	{
	    alert(_("Bandwidth can not be zero ,please re-enter"));
		return;
	}
	var loc = "/goform/trafficForm?GO=net_tc.asp";
	if (document.getElementById("check").checked)
	   loc += "&tc_enable=1";//
	else
	    loc += "&tc_enable=0";  
	loc += "&up_Band=" + upbw;
	loc += "&down_Band=" + downbw;
	loc += "&cur_number=" + resList.length;
	for(var j=0; j<resList.length; j++)
	{
		loc += "&tc_list_" + eval(j+1) + "=" + resList[j] ;
	}
	var code = 'location="' + loc + '"';
	eval(code);
}
function showList()
{
	var m='<table border=1 cellpadding="0" cellspacing="0" cellspacing=0 class=content1 id=showTab>';
	m+='<tr class=item1 align=center height=30>';
	m+='<th nowrap>'+_("No.")+'</th>';
	m+='<th nowrap>'+_("IP segment")+'</th>';
	m+='<th nowrap>'+_("Destination")+'</th>';
	m+='<th nowrap>'+_("Bandwidth range")+'</th>';
	m+='<th nowrap>'+_("Enable")+'</th>';
	m+='<th nowrap>'+_("Edit")+'</th>';
	m+='<th nowrap>'+_("Delete")+'</th>';
	m+='</tr>';
	for (i=0;i<resList.length;i++) {
		var s=resList[i].split(",");
		if (s.length!=8) 
			break;
		m+='<tr align=center>';
		m+='<td>'+eval(i+1)+'</td>';
		m+='<td>'+netip+s[1]+'~'+s[2]+'</td>';
		m+='<td>'+(parseInt(s[3],10)? _("Download"):_("Upload"))+'</td>';//0,??,1,??
		m+='<td>'+s[4]+'~'+s[5]+'</td>';
		m+='<td>'+(parseInt(s[6],2) ? "&radic;" : "&times;")+'</td>';
		m+='<td><input type=button class=button  onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'" value="'+_("Edit")+'" onclick="onEdit(' + i +  ')"></td>';
		m+='<td><input type=button class=button  onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'" value="'+_("Delete")+'" onclick="OnDel(this,' + i +  ')"></td>';
		m+='</tr>';
	}
	document.getElementById("fluxCtlList").innerHTML = m;

}
function onAdd()
{
	var f = document.forms[0];
	var upbw =12800;
	var downbw =12800;
	if (resList.length == maxlist && editTag == 0)
	{
	    alert(_("Max.")+maxlist+_("bandwidth control records"));
		return;
	}
	var pt = 80;
	var sip = parseInt(f.startIP.value,10);
	var eip = parseInt(f.endIP.value,10);
	if (isNaN(sip)||isNaN(eip))
	{
	    alert(_("Please enter the IP address"));
		return;
	}
	if (!rangeCheck(f.startIP,1,254,_("Start IP"))||
			!rangeCheck(f.endIP,1,254,_("End IP"))) return ;
	if(eip < sip)
	{
		alert(_("start IP bigger than end IP"));
		return ;
	}
	var linkTy = f.RC_link.value;
	var sbw = parseInt(f.minRate.value,10);
	var ebw = parseInt(f.maxRate.value,10);
	if(isNaN(sbw) || isNaN(ebw))
	{
	    alert(_("Please enter the bandwidth"));
		return;
	}
	if((0 == sbw)||(0 == ebw))
	{
	    alert(_("Bandwidth can not be zero ,please re-enter"));
		return;
	}
	var en = (f.RC_valid.checked) ? 1 : 0;
	var pro = 1;
	if(pt > 65535 || pt < 0)
	{
		alert(_("the port is beyond the range of 0~65535"));
		return ;
	}
	if(eip < sip)
	{
		alert(_("start IP bigger than end IP"));
		return ;
	}
	if(ebw < sbw)
	{
		alert(_("bandwidth range should begin from small to big"));
		return ;
	}
	var allinfo = pt+','+sip+','+eip+','+linkTy+','+sbw+','+ebw+','+en+','+pro;
	var comp1 = allinfo.split(",");
	var comp2;
	for (var k=0;k<resList.length;k++) 
	{
		if(editTag){
			if(k == editTag - 1)
				continue;
		}
		comp2 = resList[k].split(",");
		/*if(comp1[3] == comp2[3]){
			if(parseInt(comp1[1])>=parseInt(comp2[1])&&parseInt(comp1[1])<=parseInt(comp2[2])
				|| parseInt(comp1[2])>=parseInt(comp2[1])&&parseInt(comp1[2])<=parseInt(comp2[2])){
					alert(_("The configuration overlapped ,please re-enter"));
					return ;
			}
		}*/
		if(comp1[3] == comp2[3] && (comp1[0] == comp2[0]) && (comp1[7] == comp2[7])
		   || comp1[3] == comp2[3] && ((parseInt(comp1[0]) == 0 && parseInt(comp2[0]) != 0) || (parseInt(comp1[0]) != 0 && parseInt(comp2[0]) == 0))
		   || comp1[3] == comp2[3] && ((parseInt(comp1[7]) == 0 && parseInt(comp2[7]) != 0) || (parseInt(comp1[7]) != 0 && parseInt(comp2[7]) == 0)))
		{
			if(parseInt(comp1[1])>=parseInt(comp2[1])&&parseInt(comp1[1])<=parseInt(comp2[2])||
			   parseInt(comp1[2])>=parseInt(comp2[1])&&parseInt(comp1[2])<=parseInt(comp2[2]) ||
			   parseInt(comp1[1])<parseInt(comp2[1])&&parseInt(comp1[2])>parseInt(comp2[2]))
			{
				alert(_("The configuration overlapped ,please re-enter"));
				return ;
			}   
		}
	}	
	if(editTag == 0)
	{
		resList[resList.length]=allinfo;
	}
	else
	{
		resList[editTag-1]=allinfo;
		f.add_modify.value  = btnVal[0];
	}
	editTag = 0;
	showList();
}
function onEdit(n)
{
	var f = document.forms[0];
	var s = (resList[n]).split(",");
	f.startIP.value 	= s[1];
	f.endIP.value		= s[2];
	f.RC_link.value 	= s[3];
	f.minRate.value 	= s[4];
	f.maxRate.value		= s[5];
	f.RC_valid.checked	= parseInt(s[6],2);
	editTag = n+1;
	f.add_modify.value  = btnVal[1];
}
function OnDel(obj,dex)
{
	document.getElementById("showTab").deleteRow(dex+1);
	var i=0;
	for(i=dex;i<resList.length;i++)
	{
		resList[i] = resList[eval(i+1)];
	}
	resList.length--;
	document.forms[0].add_modify.value  = btnVal[0];
	showList();
}

function onselPort()
{
	var f = document.forms[0];
	var i1;
   for(i1=0; i1<Pro_Por_Ser.length; i1++)
   {
	  if (i1 == f.elements["selPort"].value )
	  {
		  break;
	  }	
    }	
	f.elements["protocol"].selectedIndex = parseInt (Pro_Por_Ser[i1][0]);
	f.elements["servicePort"].value = Pro_Por_Ser[i1][1];
}
function show_Option_Pro()
{
   var i,in_html;
   for (i=0; i<pro.length; i++)
   {
      in_html += "<option value="+i+">"+pro[i]+"</option>";
   }
   document.write(in_html);
}
function show_Option_Ser()
{
   var i1,in_html='';
   for(i1=0; i1<Pro_Por_Ser.length; i1++)
   {  
	  in_html += "<option value="+i1+">"+Pro_Por_Ser[i1][2]+"</option>";
    }
   document.write(in_html);
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.trafficCtl);" class="bg">
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
				<form name=trafficCtl method=POST action=/goform/TrafficCtl>
				<input type=hidden name=GO value=net_tc.asp >
				<table cellpadding="0" cellspacing="0" class="content2"> 
					<tr><td valign="top"><script>document.write(_("Enable Bandwidth Control"));</script> 
					  <input type=CHECKBOX id="check"  onClick="onCheck()">
					<script>document.write(_("Enable"));</script></td>
					</tr>
				</table>
				<table id="table2" cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
				   <tr >
					<td width="150" align="right"><script>document.write(_("IP address"));</script>:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
					  <script>document.write(netip);</script>
						<input type="text" name="startIP" size="3" maxlength="3"><b>~</b>
						<input type="text" name="endIP" size="3" maxlength="3"></td>
				   </tr>
				   <tr>
					<td width="150" align="right" ><script>document.write(_("Upload/Download"));</script>:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
					  <select name="RC_link">
					  <option value="0"><script>document.write(_("Upload"));</script></option>
					  <option value="1"><script>document.write(_("Download"));</script></option>
					</select></td>
				   </tr>
				  <tr><td width="150" align="right"><script>document.write(_("Bandwidth range"));</script>:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
					  <input type="text" name="minRate" maxlength="5" size="5" >~
						<input type="text" name="maxRate" maxlength="5" size="5">
						(KByte/s)</td>
				  </tr>
				  <tr>
					<td width="150" align="right"><script>document.write(_("Enable"));</script>:</td>
					<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;
					  <input type="checkbox" name="RC_valid" value="1"> </td></tr>
					<tr><td height="30" colspan='3' align='center'>
					<INPUT  class=button2 id="add" type=button value="" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" name="add_modify"onClick="onAdd()">
					</td>
					</tr>
				</table>  
				<div id="fluxCtlList" style="position:relative;visibility:visible;"></div>  
				<SCRIPT>tbl_tail_save("document.trafficCtl");</SCRIPT>
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("Net_tc_helpinfo1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Net_tc_helpinfo2")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("Net_tc_helpinfo3")+_("Net_tc_helpinfo4"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table2');
    </script>
</BODY>
</HTML>






