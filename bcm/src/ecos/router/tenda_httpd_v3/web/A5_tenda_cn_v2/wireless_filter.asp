<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>Wireless | MAC Filtering</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var filter_mode = "<%get_wireless_filter("macmode");%>";	
var res = "<%get_wireless_filter("maclist");%>";
var enablewireless="<% get_wireless_basic("WirelessEnable"); %>";

var Cur_ssid_index = "<% get_wireless_basic("Cur_wl_unit"); %>";
var wl0_mode = "<% get_wireless_basic("Cur_wl_mode"); %>";

var flist = new Array();
var SSID = new Array(2);

SSID[0] = "<% get_wireless_basic("SSID"); %>";
SSID[1] = "<% get_wireless_basic("SSID1"); %>";

if(res != "")
	flist = res.split(" ");

function init(f){
	if(enablewireless==1){

		//SSID = ssidlist.split(";");
		
		UpdateMBSSIDList();
		
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
		if(wl0_mode=="sta"){
			f.FilterMode.disabled = true;
		}
	}else{
		alert("开启无线功能后才可以使用本功能！");
		top.mainFrame.location.href="wireless_basic.asp";
	}
}

function showList()
{
	var s = '<table align=center border=1 cellspacing=0 class="content1"  style="margin-top:5px; line-height:18px; width:55%;" id="listTab">';
	for(var i=0;i<flist.length;i++)
	{
		s += '<tr><td align="center">' + flist[i] + '</td><td align="center"><input type="button" class="button" value="删除" onClick="onDel(this,'+i+')"></td></tr>';
	}
	s += '</table>';
	document.getElementById("list").innerHTML = s;
}

function preSubmit(f) {
	//document.getElementById("rebootTag").value = IsRebootW();
	var macL = '';
	for(var i=0; i<flist.length; i++)
	{
		macL += flist[i] + " ";
	}
	if(flist.length == 0 && f.FilterMode.selectedIndex == 1)
	{
	 alert("选择仅允许时必须输入一条规则！");
	 return false;
	}
	document.getElementById("maclist").value = macL.replace(/\W$/,"");
	f.submit();
}

function onDel(obj,dex)
{
	if(confirm('确信要删除吗？'))
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
			alert("无效的MAC地址.");
			return;
         }
	}

    if(!(mac1!="" && mac2!="" && mac3!="" &&
    	mac4!="" && mac5!="" && mac6!=""))
    {
        window.alert("无效的MAC地址.");
        return;
    }
    
    if((mac1.toLowerCase()=="ff")&&(mac2.toLowerCase()=="ff")&&
       (mac3.toLowerCase()=="ff")&&(mac4.toLowerCase()=="ff")&&
       (mac5.toLowerCase()=="ff")&&(mac6.toLowerCase()=="ff"))
    {
        window.alert("请输入单播MAC地址.");
        return;
    }
	//add by stanley
     if( (mac1.charAt(1) == "1") || (mac1.charAt(1) == "3") ||
		 (mac1.charAt(1) == "5") || (mac1.charAt(1) == "7") ||
		 (mac1.charAt(1) == "9") || (mac1.charAt(1).toLowerCase() == "b")|| 
		 (mac1.charAt(1).toLowerCase() == "d") || (mac1.charAt(1).toLowerCase() == "f") )
   
    {
        window.alert("请输入单播MAC地址.");
        return;
    }
	 //end  stanley
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
        window.alert("无效的MAC地址.");
        return;
    }
    
    if(cur.mac[0].value=="01" && cur.mac[1].value=="00" && 
       (cur.mac[2].value=="5e"|cur.mac[2].value=="5E"))
    {
        window.alert("请输入单播MAC地址.");
        return;
    }

    add_mac=cur.mac[0].value+":"+cur.mac[1].value+":"+cur.mac[2].value+":"+
		       cur.mac[3].value+":"+cur.mac[4].value+":"+cur.mac[5].value;

    if(flist.length > 15)
    {
        window.alert("MAC地址最多能配置16条.");
	 	return;
    }
	
    for(var i=0; i<flist.length; i++)
    {
		if(flist[i].toLowerCase() == add_mac.toLowerCase())
		{
			window.alert("指定的MAC地址已经存在.");
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

function UpdateMBSSIDList() {
	var defaultSelected = false;
	var selected = false;
	var optionStr = '&nbsp;&nbsp;&nbsp;&nbsp;<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">';
	for(var i=0; i<SSID.length; i++){
		if(SSID[i] != "") {
			if (Cur_ssid_index == i) {
				optionStr += '<option selected="selected">' + SSID[i] + '</option>';
			} else {
				optionStr += '<option>' + SSID[i] + '</option>';
			}
		}
	}
	optionStr += '</select>';	
	document.getElementById('ssid-select-content').innerHTML = optionStr;
}

/*
 * When user select the different SSID, this function would be called.
 */ 
function selectMBSSIDChanged()
{
	var ssid_index;
	
	ssid_index = document.frmSetup.ssidIndex.options.selectedIndex;
	
	var loc = "/goform/onSSIDChange?GO=wireless_filter.asp";

	loc += "&ssid_index=" + ssid_index; 
	var code = 'location="' + loc + '"';
   	eval(code);
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
				<!-- <input type="hidden" id="rebootTag" name="rebootTag"> -->
				<input type=hidden id="maclist" name="maclist">

				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
					<tbody>
					<tr>
					<td width="100" align="right" class="head">选择SSID</td>
					<td id="ssid-select-content">
					  &nbsp;&nbsp;&nbsp;&nbsp;<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">
							<!-- ....Javascript will update options.... -->
					  </select>
					</td>
					</tr>
					</tbody>
				</table>
				
				<table  class="content3" id="table2">
					   <TR> <TD valign="top" colspan=2>配置无线接口MAC地址过滤策略。
					   </TR>
						<tr >
						  <td width="100" valign="top" align="right" >MAC地址过滤:</td>	
						  <td valign="top" >        
						  &nbsp;&nbsp;&nbsp;&nbsp;<select size="1" name="FilterMode" id="FilterMode" onChange="onChangeRule()">
						  <option value="disabled">关闭</option>
						  <option value="allow">仅允许</option>
						  <option value="deny">仅禁止</option>
						</select></td>
						</tr>
				</table>
				  <table border ="0"  style="margin-top:0px;" class="content1" id="filterTab">
					<tr>
					  <td width="75%" align="center">MAC地址</td>
					  <td width="25%" align="left">操作</td>
					</tr>
					<tr align=center>
					<td height="30">
					  <input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">    </td>
					<td align=left><input name="button"  type=button class=button2 id=add onClick="addMac()"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" value="添加"></td>
					</tr>
				  </table>  
				  <!-- mac list -->
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
		<script>helpInfo('使用无线访问控制功能可以根据PC的无线网卡MAC地址控制其是否可以与路由器进行通信。要禁用无线访问控制功能，请选关闭,要设置该功能请选择仅允许或仅禁止。'
);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
	  table_onload1('table2');
	  table_onload('filterTab');
    </script>
</BODY>
</HTML>





