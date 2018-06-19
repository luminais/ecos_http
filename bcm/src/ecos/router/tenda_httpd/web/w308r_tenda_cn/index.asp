<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>TENDA 11N无线路由器</title>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript>
def_PUN = "<%aspTendaGetStatus("ppoe","userid");%>";
def_PPW = "<%aspTendaGetStatus("ppoe","pwd");%>";
def_WANT = "<%aspTendaGetStatus("wan","connecttype");%>";
def_WANT2 = "<%aspTendaGetStatus("wan","connectsave");%>";
def_wirelesspassword = "<%get_wireless_password("wireless","wirelesspassword");%>";
/*
wan_mode:0-static,1-dynamic,2-pppoe,3-pptp,4-l2tp,5-1x,6-dhcp+
*/

addCfg("PUN", 50, def_PUN);
addCfg("PPW", 54, def_PPW );

addCfg("wirelesspassword",59, def_wirelesspassword);

var illegal_user_pass = new Array("\\r","\\n","\\","'","\"");

function chkPOE(f) {
	var pun = document.basicset.PUN.value;
	var ppw = document.basicset.PPW.value;
	if(pun == "" || ppw == "")
	{
		alert("帐号与口令不可为空!");
		return false;
	}
	else
	{
		if(!ill_check(pun,illegal_user_pass,"帐号")) return false;
		
		if(!ill_check(ppw,illegal_user_pass,"口令")) return false;
		
		form2Cfg(f);
		return true;
	}
} 

function doFinish(f) {
	form2Cfg(f);
	var aa;
	if(document.basicset.isp[0].checked == true)
	{
	 aa=3;
	}
	else if(document.basicset.isp[1].checked == true)
	{
	 aa=2;
	}
	f.WANT1.value = aa;
	if(aa == 3)
	{
		if(!chkPOE(f))
		{
		 return;
		}
	}
	var password=document.basicset.wirelesspassword.value;
	if(password=="")
	{
	 alert("无线密码不能为空！");
	 return false;
	}
	if(password.length!=8)
	{
	 alert("输入的密码长度只能为8个字符！");
	 return false;
	}
	var re =/^[0-9a-zA-Z_\-.,:]+$/;	
	if(!re.test(password)){
		alert("有无效字符!");
	return false;
	}
	var da = new Date();
	document.getElementById("v12_time").value = da.getTime()/1000;
	f.submit();
}

function init() {
	if(location.href != top.location.href)
	{
		top.location.href = "index.asp";
	}
	cfg2Form(document.basicset);
	document.getElementById("td_PPW").innerHTML='<input name=PPW id=PPW maxLength=50 type=password class=text1 size=25 onFocus="chgPPW(this.value)" value='+def_PPW+'>';
//---------------weige modify----------------------
	if(def_WANT==2)
		onIspchange(1);
	else if(def_WANT==3)
	 	onIspchange(0);
	else if(def_WANT2==2)
		onIspchange(1);
	else if(def_WANT2==3)
		onIspchange(0);
}
//---------------end---------------------
/*huang add*/
function chgPPW(val)
{
	if(document.getElementById("PPW").type == "password"){
		document.getElementById("td_PPW").innerHTML='<input name=PPW id=PPW maxLength=50 type=text class=text1 size=25 onBlur="chgPPW(this.value)" value='+val+'>';
		document.getElementById("PPW").focus();
		document.getElementById("PPW").focus();
		document.getElementById("PPW").value=val;
		}
	else if(document.getElementById("PPW").type == "text"){
		document.getElementById("td_PPW").innerHTML='<input name=PPW id=PPW maxLength=50 type=password class=text1 size=25 onFocus="chgPPW(this.value)"  value='+val+'>';
		}
}
/*end add*/
function onIspchange(x)
{
 if(x==0)
 {
  document.basicset.isp[0].checked = true;
  document.getElementById("PUN").disabled=false;
  document.getElementById("PPW").disabled=false;
  document.getElementById("pppoeset").style.display="";
 }
 if(x==1)
 {
  document.basicset.isp[1].checked = true;
  document.getElementById("PUN").disabled=true;
  document.getElementById("PPW").disabled=true;
  document.getElementById("pppoeset").style.display="none";
 }
}
</script>
<link rel=stylesheet type=text/css href=style.css>
<style type="text/css">
<!--
.STYLE1 {
	color: #5aa1cb;
	font-size: 30px;
	font-weight: bold;
}
.STYLE2 {color: #FF0000}
-->
</style>
</head>

<body onLoad="init()">
<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">  
  <tr>
    <td valign="top">
  <table width="100%" align="center" cellpadding="0" cellspacing="0" class="top">
  <tr>
    <td height="25" align="right" valign="bottom"><a href="advance.asp">高级设置</a></td>
    <td width="10" align="right" valign="bottom">&nbsp;</td>
  </tr>
  </table>
<div style="width:100%; height:1px; background-color:#c0c7cd; overflow:hidden; padding:0px; margin:0px;"></div>
<table width="897" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td height="118" colspan="2" valign="top">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td height="100" align="center" valign="bottom"><img src="tenda.gif" width="511" height="90"></td>
      </tr>
    </table>
	<form name="basicset" action="/goform/WizardHandle" method="post">
	<input type="hidden" name="GO" value="index.asp">
	<input type="hidden" id="v12_time" name="v12_time">
	<input type="hidden" name="WANT1" id="WANT1">
	<table width="60%" border="0" align="center" cellpadding="0" cellspacing="0" class="basicset">
		<tr>
		  <td height="45" colspan="2" align="center"><span class="STYLE1">上&nbsp; 网</span></td>
		</tr>
		<TR>
		  <TD width="170" align="right">上网方式：</TD>
		  <TD align="left">&nbsp;&nbsp;&nbsp;<input name="isp" value=3 type="radio" onClick="onIspchange(0)">
		  ADSL拨号&nbsp;
		  <input name="isp" value=2 type="radio" onClick="onIspchange(1)">					  
		  自动获取</TD>
		  </TR>
	</table>
	<div id="pppoeset">
	<table width="60%" border="0" align="center" cellpadding="0" cellspacing="0" class="basicset">
		<TR>
		  <TD width="170" align="right">上网账号：</TD>
		  <TD align="left"><INPUT class=text1 maxLength=128 name=PUN id="PUN" size=25></TD>
		</TR>
		
		<TR>
		  <TD align="right">上网口令：</TD>
		  <TD align="left" id=td_PPW></TD>
		</TR>
	</table>
	</div>
	<table width="60%" border="0" align="center" cellpadding="0" cellspacing="0" class="basicset">
		<TR>
		  <TD colspan="2" align="center"><span class="STYLE2">其他上网类型请点击“<a href='advance.asp' style="font-size:12px; color:'#0000FF'" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#0000FF'">高级设置</a>”</span></TD>
		</TR>
	</table>
<hr width="50%" color="#dedfe1">
<table width="60%" border="0" align="center" cellpadding="0" cellspacing="0" class="basicset">
  <tr>
    <td height="45" colspan="2" align="center" class="STYLE1">无线加密</td>
  </tr>
  <tr>
    <td width="170" align="right">密 码：</td>
    <td><input name="wirelesspassword" type="text" class="text1" maxlength="8">&nbsp;(默认密码：12345678)<!--（只能允许输入8个字符）--></td>
  </tr>
</table>
<br>
<hr width="60%" color="#dedfe1">
<br>
<table width="60%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td height="45" align="center" valign="top"><input type="button" value="确定" class="button1" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="doFinish(document.basicset);">&nbsp;&nbsp;<input type="reset" value="取消" class="button1" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"></td>
  </tr>
</table>
</form>
	</td>
  </tr>
</table>
</td>
</tr>
</table>
</body>
</html>






