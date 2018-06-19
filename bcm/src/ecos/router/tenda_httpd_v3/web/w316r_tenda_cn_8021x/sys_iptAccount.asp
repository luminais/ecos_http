<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>Traffic | Statistics</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">
var enable_iptAccount="<%get_stream_stat_en("stream_stat_en");%>";
var accountUnit= "MByte";
var http_request = false;
function makeRequest(url, content) {
	http_request = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) { // IE
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request) {
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
	}
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}
function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			    iptAccountUpdateHTML(http_request.responseText);
		} else {
			alert('There was a problem with the request.');
		}
	}
}

function iptAccountUpdateHTML(str){
	var e=document.getElementById("tbody1");

	while(e.rows.length>0){
		e.deleteRow(0);
	}
	if(str==""){
		return;
	}
	var data=str.split("\n");
	var items;
	for(k=0;k<data.length - 1;k++){
		items=data[k].split(";");
		var rownum=e.rows.length;
		var ishave=false;
		for(i=0;i<rownum;i++){
			if(e.rows[i].cells[0].innerHTML==items[0]){
				e.rows[i].cells[1].innerHTML=items[1];
				e.rows[i].cells[2].innerHTML=items[2];
				e.rows[i].cells[3].innerHTML=items[3];
				e.rows[i].cells[4].innerHTML=items[4];
				e.rows[i].cells[5].innerHTML=items[5];
				e.rows[i].cells[6].innerHTML=items[6];
				ishave=true;
			}
		}
		if(ishave==false){
			var newtr=document.createElement("tr");
			var newtd1=document.createElement("td");
			newtr.appendChild(newtd1);
			var newtd2=document.createElement("td");
			newtr.appendChild(newtd2);
			var newtd3=document.createElement("td");
			newtr.appendChild(newtd3);
			var newtd4=document.createElement("td");
			newtr.appendChild(newtd4);
			var newtd5=document.createElement("td");
			newtr.appendChild(newtd5);
			var newtd6=document.createElement("td");
			newtr.appendChild(newtd6);
			var newtd7=document.createElement("td");
			newtr.appendChild(newtd7);
			e.appendChild(newtr);
			newtr.style.height=25;
			newtd1.innerHTML=items[0];
			newtd2.innerHTML=items[1];
			newtd3.innerHTML=items[2];
			newtd4.innerHTML=items[3];
			newtd5.innerHTML=items[4];
			newtd6.innerHTML=items[5];
			newtd7.innerHTML=items[6];
		}
	}
}
function updateIptAccount(){
	makeRequest("/goform/updateIptAccount", "something");
	setTimeout("updateIptAccount()",1000);
}
function init(){
	var e=document.getElementById("enableiptAccount");
	if(enable_iptAccount==1){
		e.checked=true;
		updateIptAccount();
	}else{
		e.checked=false;
	}
	enable_Account();
}
function enable_Account(){
	var e=document.getElementById("enableiptAccount");
	if(e.checked){
		document.getElementById("div_iptAccountTable").style.display="block";
	}else{
		document.getElementById("div_iptAccountTable").style.display="none";
	}
}
function preSubmit(f){
   var fm = document.forms[0];
   if(fm.elements["enableiptAccount"].checked)
	  fm.elements["enableiptAccountEx"].value = "1";
   else
      fm.elements["enableiptAccountEx"].value = "0";

	f.submit();
}
</script>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init();" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top"><table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
                <tr>
                  <td align="center" valign="top">
				  <form name="iptAccountform" method=post action=/goform/iptAcount_mng>
					<div align="left" class="content1" style="width:85%;">
					<input id="enableiptAccount" name="enableiptAccount" value=1 onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" type="checkbox" onClick='enable_Account()' />
					启用流量统计<input type="hidden" name="enableiptAccountEx" /></div><br >
					<div id="div_iptAccountTable">
					<table cellpadding="1" cellspacing="0" border="1" class="content1" id="table1" style="margin-top:0px; width:85%;">
						<thead style="height:24px; text-align:center;" valign="middle">
							<tr><td width="11%" align="center">IP 地址</td>
							<td width="16%" align="center">上行速率(KByte/s)</td>
							<td width="16%" align="center">下行速率(KByte/s)</td>
							<td width="17%" align="center">发送报文个数</td>
							<td width="12%" align="center">发送字节
							  <script>document.write(accountUnit);</script></td>
							<td width="17%" align="center">接收报文个数</td>
							<td width="11%" align="center">接收字节
							  <script>document.write(accountUnit);</script></td>
							</tr>
						</thead>
						<tbody id="tbody1" align="right">
						</tbody>
					</table>
					</div>
					<SCRIPT>
						tbl_tail_save('document.iptAccountform');
					</SCRIPT>
					</form>
	</td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">	
		<script>helpInfo('流量统计功能可以查看每台电脑的流量信息。'
		);</script>	
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>





