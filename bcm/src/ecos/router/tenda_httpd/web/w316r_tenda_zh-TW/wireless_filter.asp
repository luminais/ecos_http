<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>Wireless | MAC Filtering</TITLE>
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

function init(f){
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
		alert("必須要開啟無線網路功能後，才可以使用本功能功能！");
		top.mainFrame.location.href="wireless_basic.asp";
	}
}

function showList()
{
	var s = '<table align=center border=1 cellspacing=0 class="content1"  style="margin-top:5px; line-height:18px; width:55%;" id="listTab">';
	for(var i=0;i<flist.length;i++)
	{
		s += '<tr><td align="center">' + flist[i] + '</td><td align="center"><input type="button" class="button" value="刪除" onClick="onDel(this,'+i+')"></td></tr>';
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
	 alert("選擇「僅允許(白名單)」時，必須至少輸入一條規則！");
	 return false;
	}
	document.getElementById("maclist").value = macL.replace(/\W$/,"");
	f.submit();
}

function onDel(obj,dex)
{
	if(confirm('確定要刪除嗎？'))
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
			alert("無效的MAC位址");
			return;
         }
	}

    if(!(mac1!="" && mac2!="" && mac3!="" &&
    	mac4!="" && mac5!="" && mac6!=""))
    {
        window.alert("無效的MAC位址");
        return;
    }
    
    if((mac1.toLowerCase()=="ff")&&(mac2.toLowerCase()=="ff")&&
       (mac3.toLowerCase()=="ff")&&(mac4.toLowerCase()=="ff")&&
       (mac5.toLowerCase()=="ff")&&(mac6.toLowerCase()=="ff"))
    {
        window.alert("請輸入unicast MAC位址");
        return;
    }
	//add by stanley
     if( (mac1.charAt(1) == "1") || (mac1.charAt(1) == "3") ||
		 (mac1.charAt(1) == "5") || (mac1.charAt(1) == "7") ||
		 (mac1.charAt(1) == "9") || (mac1.charAt(1).toLowerCase() == "b")|| 
		 (mac1.charAt(1).toLowerCase() == "d") || (mac1.charAt(1).toLowerCase() == "f") )
   
    {
        window.alert("請輸入unicast MAC位址");
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
        window.alert("无效的MAC位址.");
        return;
    }
    
    if(cur.mac[0].value=="01" && cur.mac[1].value=="00" && 
       (cur.mac[2].value=="5e"|cur.mac[2].value=="5E"))
    {
        window.alert("請輸入unicast MAC位址");
        return;
    }

    add_mac=cur.mac[0].value+":"+cur.mac[1].value+":"+cur.mac[2].value+":"+
		       cur.mac[3].value+":"+cur.mac[4].value+":"+cur.mac[5].value;

    if(flist.length > 15)
    {
        window.alert("MAC位址最多只能設定16條！");
	 	return;
    }
	
    for(var i=0; i<flist.length; i++)
    {
		if(flist[i].toLowerCase() == add_mac.toLowerCase())
		{
			window.alert("指定的MAC位址已經存在");
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
				<!-- <input type="hidden" id="rebootTag" name="rebootTag"> -->
				<input type=hidden id="maclist" name="maclist">
				<table  cellpadding="0" cellspacing="0" class="content3" id="table2">
					   <TR> <TD valign="top" colspan=2>設定無線網路卡MAC位址連線權限。
					   </TR>
						<tr >
						  <td width="250" valign="top" align="right" >MAC位址權限設定：</td>	
						  <td width="238" valign="top" >        
						  &nbsp;&nbsp;&nbsp;&nbsp;
						  <select size="1" name="FilterMode" id="FilterMode" onChange="onChangeRule()">
						  <option value="disabled">關閉</option>
						  <option value="allow">僅允許(白名單)</option>
						  <option value="deny">僅禁止(黑名單)</option>
						</select></td>
					  </tr>
				</table>
				  <table border ="0"  style="margin-top:0px;" class="content1" id="filterTab">
					<tr>
					  <td width="75%" align="center">MAC位址</td>
					  <td width="25%" align="left">運作</td>
					</tr>
					<tr align=center>
					<td height="30">
					  <input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">:
					<input class=text id=mac size=2 maxlength=2 onKeyUp="value=value.replace(/[^0-9a-fA-F]/g,'')" onbeforepaste="clipboardData.setData('text',clipboardData.getData('text').replace(/[^0-9a-fA-F]/g,''))">    </td>
					<td align=left><input name="button"  type=button class=button2 id=add onClick="addMac()"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" value="新增"></td>
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
		<script>
		helpInfo('使用無線網路權限設定功能，可以根據電腦的無線網卡的MAC位址，控制其是否可與路由器連線。要停用無線網路權限設定功能，請選擇「關閉」，要設定該功能請選擇選「僅允許(白名單)」或「僅禁止(黑名單)」。');
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  <!--table_onload('table1');-->
	  table_onload1('table2');
	  table_onload('filterTab');
    </script>
</BODY>
</HTML>

