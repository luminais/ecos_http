<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>Wireless | MAC Filtering</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var filter_mode = "<%get_wireless_filter("macmode");%>";	
var res = "<%get_wireless_filter("maclist");%>";
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>";
var flist = new Array();
if(res != "")
	flist = res.split(" ");
Butterlate.setTextDomain("wireless_set");
function initTranslate(){
	var e=document.getElementById("add");
	e.value=_("Add");
}
function init(f){
    initTranslate();
	if(enablewireless==1){	
		if(filter_mode == "disabled")
			f.FilterMode.selectedIndex = 0;
		else if(filter_mode == "allow")
			f.FilterMode.selectedIndex = 1;
		else if(filter_mode == "deny")
			f.FilterMode.selectedIndex = 2;
		var cur=document.frmSetup;
		cur.mac[0].value = "";
		cur.mac[1].value = "";
		cur.mac[2].value = "";
		cur.mac[3].value = "";
		cur.mac[4].value = "";
		cur.mac[5].value = "";
		onChangeRule();
		showList();
	}else{
		alert(_("This function can only be used after the wireless function is enabled"));
		top.mainFrame.location.href="wireless_basic.asp";
	}
}
function showList()
{
	var s = '<table align=center border=1 cellspacing=0 class="content1"  style="margin-top:5px; line-height:18px; width:55%;" id="listTab">';
	for(var i=0;i<flist.length;i++)
	{
		s += '<tr><td align="center">' + flist[i] + '</td><td align="center"><input type="button" class="button" value='+_("Delete")+' onClick="onDel(this,'+i+')"></td></tr>';
	}
	s += '</table>';
	document.getElementById("list").innerHTML = s;
}
function preSubmit(f) {
	var macL = '';
	for(var i=0; i<flist.length; i++)
	{
		macL += flist[i] + " ";
	}
	if(flist.length == 0 && f.FilterMode.selectedIndex == 1)
	{
	 alert(_("rule"));
	 return false;
	}
	document.getElementById("maclist").value = macL.replace(/\W$/,"");
	f.submit();
}
function onDel(obj,dex)
{
	if(confirm(_("Are you sure to delete")))
	{
		document.getElementById("listTab").deleteRow(dex);
		var i=0;
		for(i=dex;i<flist.length;i++)
		{
			flist[i] = flist[eval(i+1)];
		}
		flist.length--;
		showList();	
    }	
}
function addMac()
{
    var cur=document.frmSetup;
    var mac1=cur.mac[0].value;
    var mac2=cur.mac[1].value;
    var mac3=cur.mac[2].value;
    var mac4=cur.mac[3].value;
    var mac5=cur.mac[4].value;
    var mac6=cur.mac[5].value;
    var add_mac;
    var tmp_mac;
    var m = /^[0-9a-fA-F]{1,2}$/;
	for(i=0;i<6;i++)
	{
	     if(!m.test(cur.mac[i].value)){
			alert(_("Invalid MAC address"));
			return;
         }
	}
    if(!(mac1!="" && mac2!="" && mac3!="" &&
    	mac4!="" && mac5!="" && mac6!=""))
    {
        window.alert(_("Invalid MAC address"));
        return;
    }
    if((mac1.toLowerCase()=="ff")&&(mac2.toLowerCase()=="ff")&&
       (mac3.toLowerCase()=="ff")&&(mac4.toLowerCase()=="ff")&&
       (mac5.toLowerCase()=="ff")&&(mac6.toLowerCase()=="ff"))
    {
        window.alert(_("Please enter a unicast MAC address"));
        return;
    }
     if( (mac1.charAt(1) == "1") || (mac1.charAt(1) == "3") ||
		 (mac1.charAt(1) == "5") || (mac1.charAt(1) == "7") ||
		 (mac1.charAt(1) == "9") || (mac1.charAt(1).toLowerCase() == "b")|| 
		 (mac1.charAt(1).toLowerCase() == "d") || (mac1.charAt(1).toLowerCase() == "f") )
    {
        window.alert(_("Please enter a unicast MAC address"));
        return;
    }
    cur.mac[0].value=(mac1.length==2)?mac1:("0"+mac1);
    cur.mac[1].value=(mac2.length==2)?mac2:("0"+mac2);
    cur.mac[2].value=(mac3.length==2)?mac3:("0"+mac3);
    cur.mac[3].value=(mac4.length==2)?mac4:("0"+mac4);
    cur.mac[4].value=(mac5.length==2)?mac5:("0"+mac5);
    cur.mac[5].value=(mac6.length==2)?mac6:("0"+mac6);
    if(cur.mac[0].value=="00" && cur.mac[1].value=="00" && 
       cur.mac[2].value=="00" && cur.mac[3].value=="00" && 
       cur.mac[4].value=="00" && cur.mac[5].value=="00")
    {
        window.alert(_("Invalid MAC address"));
        return;
    }
    if(cur.mac[0].value=="01" && cur.mac[1].value=="00" && 
       (cur.mac[2].value=="5e"|cur.mac[2].value=="5E"))
    {
        window.alert(_("Please enter a unicast MAC address"));
        return;
    }
    add_mac=cur.mac[0].value+":"+cur.mac[1].value+":"+cur.mac[2].value+":"+
		       cur.mac[3].value+":"+cur.mac[4].value+":"+cur.mac[5].value;
    if(flist.length > 15)
    {
        window.alert(_("Max.16 MAC addresses"));
	 	return;
    }
    for(var i=0; i<flist.length; i++)
    {
		if(flist[i].toLowerCase() == add_mac.toLowerCase())
		{
			window.alert(_("The MAC address already exists"));
			return;
		}
    }
   	flist[flist.length] = add_mac;
	showList();
}
function onChangeRule()
{
	if(document.getElementById("FilterMode").value == "disabled")
	{
		document.getElementById("filterTab").style.display = "none";
		document.getElementById("list").style.display = "none";
	}
	else
	{
		document.getElementById("filterTab").style.display = "";
		document.getElementById("list").style.display = "";
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
				<form name=frmSetup method=POST action="/goform/WlanMacFilter">
				<input type=hidden name=GO value="wireless_filter.asp">
				<input type=hidden id="maclist" name="maclist">
				<table class=content2>
						<tr >
						  <td width="150" valign="top"><script>document.write(_("MAC address filter"));</script>:</td>	
						  <td valign="top" >        
						  <select size="1" name="FilterMode" id="FilterMode" onChange="onChangeRule()">
						  <option value="disabled"><script>document.write(_("Disable"));</script></option>
						  <option value="allow"><script>document.write(_("Permit"));</script></option>
						  <option value="deny"><script>document.write(_("Forbid"));</script></option>
						</select></td>
						</tr>
				</table>
				  <table border ="0"  style="margin-top:0px;" class="content1" id="filterTab">
					<tr><td colspan=2><script>document.write(_("Configure MAC address"));</script></td></tr>
					<tr>
					  <td width="75%" align="center"><script>document.write(_("MAC address"));</script></td>
					  <td width="25%" align="left"><script>document.write(_("Operate"));</script></td>
					</tr>
					<tr align=center>
					<td height="30">
					  <input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">    </td>
					<td align=left><input name="button"  type=button class=button2 id=add onClick="addMac()"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" value=""></td>
					</tr>
				  </table>  
					<div id="list" style="position:relative;visibility:visible;"></div>
				  <SCRIPT>
				   tbl_tail_save("document.frmSetup");
				  </SCRIPT>       
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("Wireless_filter_helpinfo"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('filterTab');
    </script>
</BODY>
</HTML>





