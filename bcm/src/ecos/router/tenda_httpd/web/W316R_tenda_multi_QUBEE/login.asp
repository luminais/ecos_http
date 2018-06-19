<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<title>Tenda 11N Wireless Router</title>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var error=new Array(_("No Ethernet cable connected to WAN port!"),_("No Internet access, please check your online settings!"));
message="<%sysTendaGetStatus("wan","linkstatus");%>";
message2="<%sysTendaGetStatus("wan","login_where");%>";
Butterlate.setTextDomain("index_routing_virtual");



function initTranslate(){
	var e=document.getElementById("sure");
	e.value=_("Ok");
	var e1=document.getElementById("cancel");
	e1.value=_("Cancel");
}
function init(){
	if(location.href != top.location.href)
	{
		top.location.href = "login.asp";
	}
	initTranslate();
	Login.Password.value="";
	Login.Password.focus();
	if(message==0)
	{
	 document.getElementById("message").innerHTML=_("No Ethernet cable connected to WAN port!");
	}
	else if(message==2 || message==3)
	{
	 document.getElementById("message").innerHTML=_("No Internet access, please check your online settings!");
	}
	else
	{
	 document.getElementById("message").innerHTML="&nbsp;";
	}
	if(message2 == 0)
	{
		show_hide("login_tip",0);		
	}
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
/*	if(f.Password.value=="")
	{
		alert(_("Password can not be empty!")); 
		f.Password.focus();
		return false;
	}
*/
	if(f.Password.value.length>13)
	{
	    alert(_("Password can not exceed 12 characters in length!")); 
		f.Password.focus();
		return false;
	}
	f.submit();
}
</script>
<link rel=stylesheet type=text/css href=style.css>
<style type="text/css">
.login{COLOR: #000000; FONT-FAMILY:"Times New Roman"; font-size:12px; border:solid 1px #fbac36; line-height:30px; margin-top:100px;}
.STYLE1 {color: #FF0000}
</style>
</head>
<body onLoad="init()">
<form name="Login" method="post" action="/LoginCheck">
<input type=hidden name=Username value="<%aspTendaGetStatus("sys","username");%>">
<input type="hidden" name="checkEn" value="0">
<table width="400" border="0" align="center" cellpadding="0" cellspacing="0" class="login">
	<tr><td style="background-color:#fbac36; height:35px;" colspan="2"><font size="+1" style="font-weight:bold"> &nbsp;&nbsp;<script>document.write(_("Login"));</script></font></td></tr>
  <tr>
    <td width="120" align="right">&nbsp;</td>
    <td><span class="STYLE1">
      <div id="message"></div>
    </span></td>
  </tr>
  
 
  <tr>
    <td width="120" align="right"><script>document.write(_("Password"));</script>:</td>
    <td>&nbsp;&nbsp;<input type="text" name="Password" maxlength="12" style="width:130px;"/>
    <span class="STYLE1">(<script>document.write(_("Initial password"));</script>:&nbsp;&nbsp;NULL)</span></td>
  </tr>
  <tr id="login_tip">
    <td >&nbsp;</td>
    <td>&nbsp;&nbsp;<input type="checkbox" name="nocheck"/>
    <script>document.write(_("No longer prompts "));</script></td>
  </tr>
  <tr>
    <td height="35" colspan="2" align="center" valign="bottom"><input type="button" id="sure" value="" class="button1" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="preSubmit(document.Login)">&nbsp;&nbsp;<input type="reset" id="cancel" value="" class="button1" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"></td>
  </tr>
  <tr>
    <td colspan="2" align="center">&nbsp;</td>
  </tr>
</table>
</form>
</body>
</html>



