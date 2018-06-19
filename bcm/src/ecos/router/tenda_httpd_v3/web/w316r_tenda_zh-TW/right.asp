<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>LAN | LAN Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript" type="text/javascript">
conType=new Array("固定 IP","浮動 IP","PPPoE","PPTP","L2TP","DHCP+");
state=new Array("停用","啟用");
conStat=new Array("未連線", "連線中...","已連線");

cableDSL="<%sysTendaGetStatus("wan","contstatus");%>";
subMask="<%sysTendaGetStatus("wan","wanmask");%>";
wanIP = "<%sysTendaGetStatus("wan","wanip");%>";
gateWay="<%sysTendaGetStatus("wan","gateway");%>";
dns1="<%sysTendaGetStatus("wan","dns1");%>";
dns2="<%sysTendaGetStatus("wan","dns2");%>";
conntime="<%sysTendaGetStatus("wan","connetctime");%>";


conTypeIdx="<%aspTendaGetStatus("wan","connecttype");%>";
run_code_ver="<%aspTendaGetStatus("sys","sysver");%>";
boot_code_ver="<%aspTendaGetStatus("sys","bootloadver");%>";
hw_ver="<%aspTendaGetStatus("sys","hardwarever");%>";
clients="<%aspTendaGetStatus("sys","conclient");%>";
uptime= "<%aspTendaGetStatus("sys","runtime");%>";
systime = "<%aspTendaGetStatus("sys","systime");%>";
function preSubmit1(idx) {   
   var f=document.systemStatus;

   f.action.value=idx;
   f.submit() ;
}
function timeStr(t)
{
	if(t < 0)
	{
		str='00:00:00';
		return str;
	}
	var s=t%60;
	var m=parseInt(t/60)%60;
	var h=parseInt(t/3600)%24;
	var d=parseInt(t/86400);

	var str='';
	if (d > 999) { return '永遠'; }
	if (d) str+=d+'天 ';
	str+=fit2(h)+':';
	str+=fit2(m)+':';
	str+=fit2(s);
	return str;
}
</script>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>

<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" style="background-color:#e9e9e9; overflow:hidden;" scrolling="none">
      <form name=systemStatus method=POST action=/goform/SysStatusHandle>
      <INPUT type=hidden name=CMD value=WAN_CON>
      <INPUT type=hidden name=GO value=lan.asp>
      <INPUT type=hidden name=action>
		<table border="0" align="center" cellpadding="0" cellspacing="0" class="left1" style="margin-top:5px;">
		  <tr>
			<td><strong>WAN埠狀態：</strong><br>			</td>
	      </tr>
		  <tr>
		  <td>連線狀態</td>
		  <td><script>document.write(conStat[cableDSL]);</script>		  </td>
		</tr>
		  <tr>
		  <td >WAN IP 位址</td> 
		  <td >
			<script>document.write(wanIP);</script>		  </td>
		</tr>
		<tr>
	      <td >子網路遮罩</td> 
	      <td >
			<script>document.write(subMask);</script>		  </td>
		</tr>
		<tr>
		  <td >預設閘道</td>
		  <td >
			<script>document.write(gateWay);</script>		  </td>
  		</tr>
		<tr>
		  <td >主要DNS伺服器</td>
		  <td >
			<script>document.write(dns1);</script>		  </td>
		</tr>
		<tr>
		  <td >次要DNS伺服器</td> 
		  <td >
			<script>document.write(dns2);</script>		  </td>
		</tr>

		<tr>
		  <td >連線方式</td> 
		  <td >
			<script>document.write(conType[conTypeIdx-1])</script>		  </td>
		</tr>
		<script>
		if ((conTypeIdx==2)||(conTypeIdx==3)||(conTypeIdx==4)||(conTypeIdx==5)||(conTypeIdx==6))
		{
		  	document.write("<tr><td >連線時間</td>");
		    document.write("<td >");
		    document.write(timeStr(conntime));
		    document.write("</td></tr>");
		}  
		</script>
		<tr height="30">
		  <script>
		  if (conTypeIdx==2) {//dhcp
		     document.write('<td><input type=button class=button value="釋放 IP" onclick=preSubmit1(1);></td>');
		     document.write('<td><input type=button class=button value="更新 IP" onclick=preSubmit1(2);></td>');
		  }
		  else if (conTypeIdx == 3) 
		  { //pppoe
		  	if(cableDSL == 0)//unlink
			{
		     	document.write('<td><input type=button class=button value="連線" onclick=preSubmit1(3);></td>');
		    	document.write('<td><input type=button class=button value="斷線" disabled="disabled"></td>');
			}
			else if(cableDSL == 1)//linked
			{
		     	document.write('<td><input type=button class=button value="連線" disabled="disabled"></td>');
		    	document.write('<td><input type=button class=button value="斷線" onclick=preSubmit1(4);></td>');
			}
			else//linking
			{
				 document.write('<td><input type=button class=button value="連線" onclick=preSubmit1(3);></td>');
				 document.write('<td><input type=button class=button value="斷線" onclick=preSubmit1(4);></td>');
			}
		  }
		  else if (conTypeIdx == 4) 
		  { //pptp
		     document.write('<td><input type=button class=button value="連線" onclick=preSubmit1(5);></td>');
		     document.write('<td><input type=button class=button value="斷線" onclick=preSubmit1(6);></td>');
		  }
		  else if (conTypeIdx == 5) 
		  { //l2tp
		     document.write('<td><input type=button class=button value="連線" onclick=preSubmit1(7);></td>');
		     document.write('<td><input type=button class=button value="斷線" onclick=preSubmit1(8);></td>');
		  }
		</script>
		  </tr>
		</table>
		
		<table border="0" align="center" cellpadding="0" cellspacing="0" class="left1" style="margin-top:5px;">
		  <tr>
			<td><strong>系統狀態：</strong><br>			</td>
	      </tr>
		  <tr>
		  <td>系統時間</td>
		  <td><SCRIPT>document.write(systime);</SCRIPT></td>
		</tr>
		<tr>
	      <td >使用者人數</td> 
	      <td >
			<script>document.write( clients );</script>		  </td>
		</tr>
		<tr>
		  <td >韌體版本編號</td> 
		  <td >
			<script>document.write( run_code_ver );</script>		  </td>
		</tr>

		<tr>
		  <td >硬體版本編號</td> 
		  <td >
			<script>document.write( hw_ver );</script>		  </td>
		</tr>
		</table>
        </form>
		</BODY>
</HTML>

