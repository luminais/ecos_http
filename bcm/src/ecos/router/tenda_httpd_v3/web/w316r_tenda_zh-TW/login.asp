<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>TENDA 11N無線寬頻路由器登入介面</title>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var error=new Array("路由器WAN埠沒有連接網路線！","沒有連結到網際網路，請檢查您的連線設定！");
message="<%sysTendaGetStatus("wan","linkstatus");%>";
message2="<%sysTendaGetStatus("wan","login_where");%>";
function init(){
	if(location.href != top.location.href)
	{
		top.location.href = "login.asp";
	}
	var showmessage = "";
	Login.Password.value="";
	Login.Password.focus();
	if(message==0)
	{
	 showmessage=error[0];
	}
	else if(message==2 || message==3)
	{
	 showmessage=error[1];
	}
	else
	{
	 showmessage="&nbsp;";
	}
	if(message2 == 0)
	{
		//from lan,donnt display login tip
		show_hide("login_tip",0);		
	}
	document.getElementById("message").innerHTML=showmessage;
}
function preSubmit(f)
{	
    	if(f.nocheck.checked)
	{
	 	f.checkEn.value=1;
	}
	else
	{
	 	f.checkEn.value=0;
	}
	if(f.Password.value=="")
	{
		alert("密碼不可以空白！"); 
		f.Password.focus();
		return false;
	}
	if(f.Password.value.length>13)
	{
	    alert("密碼長度不可以超過12個字！(英文、數字)"); 
		f.Password.focus();
		return false;
	}
	f.submit();
}

</script>
<link rel=stylesheet type=text/css href=style.css>
<style type="text/css">
.login{COLOR: #000000; FONT-FAMILY: "新細明體", "Times New Roman"; font-size:12px; border:solid 1px #fbac36; line-height:30px; margin-top:100px;}
.STYLE1 {color: #FF0000}
</style>
</head>

<body onLoad="init()">
<form name="Login" method="post" action="/LoginCheck">
<input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
<input type="hidden" name="checkEn" value="0">
<table width="400" border="0" align="center" cellpadding="0" cellspacing="0" class="login">
	<tr><td style="background-color:#fbac36; height:35px;" colspan="2"><font size="+1" style="font-weight:bold">登入路由器</font></td></tr>
  <tr>
    <td width="131" align="right">&nbsp;</td>
    <td width="267"><span class="STYLE1">
      <div id="message"></div></span></td>
  </tr>
  <tr>
    <td width="131" align="right">密碼：</td>
    <td><input type="text" name="Password" maxlength="12" style="width:130px;"/>
    <span class="STYLE1">（原始密碼：admin）</span></td>
  </tr>
  <tr id="login_tip">
    <td >&nbsp;</td>
    <td><input type="checkbox" name="nocheck"/>不再提示</td>
  </tr>
  <tr>
    <td height="35" colspan="2" align="center" valign="bottom"><input type="button" value="確定" class="button1" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="preSubmit(document.Login)">&nbsp;&nbsp;<input type="reset" value="取消" class="button1" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"></td>
  </tr>
  <tr>
    <td colspan="2" align="center">&nbsp;</td>
  </tr>
</table>
</form>
</body>
</html>


