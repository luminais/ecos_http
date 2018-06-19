<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>Virtual | Server</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
def_LANIP = "<%aspTendaGetStatus("lan","lanip");%>";
addCfg("LANIP", 0,def_LANIP);

//0;ee;1-2;tcp;10.10.10.5;3;0;;
addCfg("clientList1",1,"<%mNatPortSegment("1");%>");//格式：是否啟用IP地址指定网络服務器（0：未啟用；1:啟用）；開始通訊埠;結束通訊埠；局域网IP地址；通訊協定（0:TCP;1:UDP;2全部）
addCfg("clientList2",2,"<%mNatPortSegment("2");%>");
addCfg("clientList3",3,"<%mNatPortSegment("3");%>");
addCfg("clientList4",4,"<%mNatPortSegment("4");%>");
addCfg("clientList5",5,"<%mNatPortSegment("5");%>");
addCfg("clientList6",6,"<%mNatPortSegment("6");%>");
addCfg("clientList7",7,"<%mNatPortSegment("7");%>");
addCfg("clientList8",8,"<%mNatPortSegment("8");%>");
addCfg("clientList9",9,"<%mNatPortSegment("9");%>");
addCfg("clientList10",10,"<%mNatPortSegment("10");%>");//1;21;192.168.0.62

var max=10;
var def_describe = "virtual";
var schedcmd = "0";
var prot=["tcp/udp","tcp","udp"];

var netip=getCfg("LANIP").replace(/\.\d{1,3}$/,".");

function doFill()
{
var f=document.frmSetup;
var p=f.ports[f.ports.selectedIndex].value;
var i=f.ids[f.ids.selectedIndex].value ;
	f.elements["sport"+i].value=p;
        f.elements["eport"+i].value=p;
}

function delOne(f,idx)
{
f.elements["sport"+idx].value="";
f.elements["eport"+idx].value="";
f.elements["pip"+idx].value="";
f.elements["protocol"+idx].selectedIndex=0;
f.elements["chk"+idx].checked=0;
f.elements["del"+idx].checked=0;
}
   		
function init(f) {
	for(j=1;j<=max;j++){
   		var s = getCfg("clientList"+j);
   		if(s.length > 0){
			//enable;describe;external port;potocol;ip;internal port;schedu type;time;data
			//0;ee;1-2;tcp;10.10.10.5;3;0;;
			var s1 = s.split(";");
  			//f.elements["pip"+j].value=decomList(decomList(s,5,3,";"),4,3,".");
			//f.elements["sport"+j].value=decomList(s,5,1,";");
			//f.elements["eport"+j].value=decomList(s,5,2,";");
			//f.elements["protocol"+j].selectedIndex = decomList(s,5,4,";");
   			//f.elements["chk"+j].checked=parseInt(decomList(s,5,0,";"),10) ;
			//roy modify
			f.elements["pip"+j].value=decomList(s1[4],4,3,".");
			f.elements["sport"+j].value=decomList(s1[2],2,0,"-");
			f.elements["eport"+j].value=decomList(s1[2],2,1,"-");
			if(s1[3] == prot[0])
				f.elements["protocol"+j].selectedIndex = 2;
			else if(s1[3] == prot[1])
				f.elements["protocol"+j].selectedIndex = 0;
			else
				f.elements["protocol"+j].selectedIndex = 1;
   			f.elements["chk"+j].checked=parseInt(s1[0],10) ;
   		}else{
   			f.elements["pip"+j].value="";
   			f.elements["sport"+j].value="";
			f.elements["eport"+j].value="";
   			f.elements["chk"+j].checked=0;
   		}
   }
}

function preSubmit(f) {   //portlist[i]:是否啟用（0：未用；1:啟用);開始通訊埠-結束通訊埠；IP；通訊協定(0:TCP;1:UDP;2全部)
	var loc = "/goform/VirSerSeg?GO=nat_virtualportseg.asp";

	for (i=1;i<=max;i++) 
	{
		var ip = f.elements["pip"+i].value;
		var spt = f.elements["sport"+i].value;
		var ept = f.elements["eport"+i].value

		if(ip.value=="" && spt.value=="" && ept.value=="")
			;
		else
		{
			//---------------------------------weige add----------------------------------------
			if(i != max)
			{
				for(j=i+1; j<=max; j++)
				{
					var ip_check = f.elements["pip"+j];
					var spt_check = f.elements["sport"+j];
					var ept_check = f.elements["eport"+j];
					
					if(ip_check.value=="" && spt_check.value=="" && ept_check.value=="")
						;
					else
					{
						if((spt_check.value == ept_check.value && spt.value == ept.value && spt.value == spt_check.value) && (ip_check.value != ip.value))
						{
							//if(ip_check.value != ip.value)
							alert("One port can forward only one lan ip!");
							//else
								//alert("You've setting the rule!");
							return ;
						}
						else if(spt_check.value == spt.value && ept_check.value == ept.value && ip_check.value == ip.value)
						{
							alert("You've set the same rule!");
							return ;
						}
					}
				}
			}
			//------------------end--------------------------

			if (!rangeCheck(f.elements["sport"+i],1,65535,"第"+i+"開始通訊埠")||
				!rangeCheck(f.elements["eport"+i],1,65535,"第"+i+"結束通訊埠")) return ;
			if ( Number(spt) > Number(ept) ) { alert(i+"開始通訊埠 > 結束通訊埠"); return ; }
   			if (!rangeCheck(f.elements["pip"+i],1,254,"第"+i+"虛擬IP")) return;
			
			if(f.elements["chk"+i].checked)
				loc += "&PL" + i + "=1";//该项啟用
			else
				loc += "&PL" + i + "=0";//未啟用
			
			//loc += ";" + spt + ";" + ept + ";" + netip + ip;
			//loc += ";" + f.elements["protocol"+i].selectedIndex;
			//roy modify
			loc += ";" + def_describe +";" + spt +"-" + ept + ";";
			if(f.elements["protocol"+i].selectedIndex == 0){
				loc += prot[1];
			}else if(f.elements["protocol"+i].selectedIndex == 1){
				loc += prot[2];
			}else{
				loc += prot[0];
			}
			loc += ";" + netip + ip +";" + spt +";" + schedcmd + ";" +";";
			
   		}
   }
   var code = 'location="' + loc + '"';
   eval(code);
}

function rule_entry(idx)
{
	
	text =	'<tr class=value1 align=center>';
	text += '<td nowrap>'+idx+'.</td>';
	text += '<td nowrap>'; 
	text += '<input maxLength=5 class=text name="sport'+idx+'" size="6" >-'; 
	text += '<input maxLength=5 class=text name="eport'+idx+'" size="6" >'; 
	text += '</td>';
	text += '<td nowrap>';
	text += netip;
	text += '<input maxLength=3 class=text name="pip'+idx+'" size="3" >';   
	text += '</td>';
	text +=	'<td nowrap>&nbsp;<select name="protocol'+idx+'" >';
	text +=	'<option value="0"> TCP </option>';
	text +=	'<option value="1"> UDP </option>';
	text +=	'<option value="2"> 全部 </option>';
	text +=	'</select></td>'; 
	text += '<td nowrap><input type="checkbox" name="chk'+idx+'" value="1"></td>';
	text += '<td nowrap><input type=checkbox name=del'+idx+' onClick=delOne(document.frmSetup,'+idx+');> </td>';
	text += '</tr>';
	document.writeln(text);
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
				<form name=frmSetup method=POST action=/goform/VirSerSeg>
				<INPUT type=hidden name=GO value=nat_virtualportseg.asp >
				<INPUT type=hidden name=ITEMCNT >
				<table width=90% class="content1" style="line-height:25px;">
					<tr><td valign="top">&nbsp;&nbsp;&nbsp;&nbsp;開放通訊埠定義了廣域網路(WAN)端的連線通訊埠與內部區域網路(LAN)中電腦虛擬IP間的對應關係，如果有架設網站或伺服器，可以透過本項設定可將網站或伺服器由虛擬IP位址的特定通訊埠，對應到路由器由ISP取得的實體IP位址的相同通訊埠。
					</td>
					</tr>
					</table>
					<table cellpadding="0" cellspacing="0" class="content1" id="table1" style="margin-top:0px;">
						<tr align=center height=30> 
							<td nowrap width=10%>ID</td>
							<th nowrap width=20%>開始通訊埠-結束通訊埠</th>
							<th nowrap width=20%>內部網路IP</th>
							<th nowrap width=15%>通訊協定</th>
							<th nowrap width=10%>啟用</th>
							<th nowrap width=10%>刪除</th>
						</tr>
							<script>
								for(i=1;i<=max;i++)
									rule_entry(i);
							</script>
				<TR><TD width=25% c>常用服務通訊埠:</TD>
						<TD noWrap width=20%><SELECT class=list name=ports>
						<OPTION selected value=53>DNS(53)</OPTION>
						<OPTION value=21>FTP(21)</OPTION>
						<OPTION value=70>GOPHER(70)</OPTION>
						<OPTION value=80>HTTP(80)</OPTION>
						<OPTION value=119>NNTP(119)</OPTION>
						<OPTION value=110>POP3(110)</OPTION>
						<OPTION value=1723>PPTP(1723)</OPTION>
						<OPTION value=25>SMTP(25)</OPTION>
						<OPTION value=1080>SOCK(1080)</OPTION>
						<OPTION value=23>TELNET(23)</OPTION></SELECT>
						</TD>
					<TD width=73 valign="middle" noWrap><INPUT class=button2 name=fill onclick=doFill(); type=button  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" value=填寫到>&nbsp;ID</TD>
					<TD noWrap width=117><SELECT class=list name=ids> 
					<OPTION selected value=1>1</OPTION>
					<OPTION value=2>2</OPTION>
					<OPTION value=3>3</OPTION>
					<OPTION value=4>4</OPTION>
					<OPTION value=5>5</OPTION>
					<OPTION value=6>6</OPTION>
					<OPTION value=7>7</OPTION>
					<OPTION value=8>8</OPTION>
					<OPTION value=9>9</OPTION>
					<OPTION value=10>10</OPTION>
					<!--
					<OPTION value=11>11</OPTION>
					<OPTION value=12>12</OPTION>
					-->
					</SELECT>
					</TD>
					<td></td>
					<td></td>
				</TR>
				</TABLE>
				
					
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
				
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>
		helpInfo('開始通訊埠 - 結束通訊埠：網際網路服務通訊埠。<br>啟用：必須要「勾選」啟用，該項目才會生效。<br>刪除：協助您把該項目全部資料清除，清除後必須點選「儲存」才會正式生效。<br>填寫到：協助您把常用的服務通訊埠參數，填寫到您要設定的指定項目。');
		</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>

