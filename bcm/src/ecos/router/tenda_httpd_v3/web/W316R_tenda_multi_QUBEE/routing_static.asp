<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>Routing | Static Routing</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var list = "<%aspTendaGetStatus("wan","wan0_route");%>";
var resList;
var max = 5;
Butterlate.setTextDomain("index_routing_virtual");
function initTranslate(){
	var e=document.getElementById("add");
	e.value=_("add");
}
function init(f){
    initTranslate();
	resList = list.split(" ");
	showRT();
}
function showRT()
{
	var i;
	var m='<table border=1 cellspacing=0 class="content1" id=showTab>';
	for (i=0;i<resList.length;i++) {
		var rt=resList[i];
		var s=rt.split(":");
		if (s.length!=4) continue;
		m+='<tr align=center>';
		m+='<td>'+s[0];
		m+='</td>';
		m+='<td>'+s[1];
		m+='</td>';
		m+='<td>'+s[2];
		m+='</td>';
		m+='<td>';
		m+='<input class=button2 type=button value='+_("Delete")+' onMouseOver="style.color=\'#FF9933\'" onMouseOut="style.color=\'#000000\'"  onClick=delRte(this,' + i +  ')></td>';
		m+='</tr>';
	}
	m+='</table>';
	document.getElementById("routeList").innerHTML = m;
}
function delRte(obj,dex)
{
	document.getElementById("showTab").deleteRow(dex-1);
	var i=0;
	for(i=dex;i<resList.length;i++)
	{
		resList[i] = resList[eval(i+1)];
	}
	resList.length--;
	showRT();
}
function addRte(f) {
	var info;
	if (!verifyIP2(f.ip,_("Destination IP"),1)) return ;
	if (!ipMskChk(f.nm,_("Subnet mask"))) return ;
	if (!verifyIP2(f.gw,_("Gateway"))) return ;
	info = f.ip.value+':'+f.nm.value+':'+f.gw.value+':'+'1';
	  if(resList.length >= max+1)
	    {
	        window.alert(_("Max.")+max+_("static routing"));
		 	return;
	    }
	    for(var i=0; i<resList.length; i++)
	    {
			if(resList[i] == info)
			{
				window.alert(_("the specified static routing already exists"));
				return;
			}
	    }
	resList[resList.length]=info;
	showRT();
}
function preSubmit(f) 
{
	var loc = "/goform/RouteStatic?GO=routing_static.asp";
	loc += "&wan0_route=";
	for(var j=0; j<resList.length; j++)
	{
		loc +=  resList[j] +" ";
	}
	var code = 'location="' + loc + '"';
	eval(code);
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);table_onload('showTab');" class="bg">
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
				<form name=frmSetup method=POST action=/goform/RouteStatic>
				<INPUT type=hidden name=GO value=routing_static.asp>
					<input type=hidden name=cmd>
					<table border=0 class=content1 cellspacing=0 id="table1">
						<tr class=item1 align=center height=30 style="line-height:20px;">
						  <th><script>document.write(_("Destination network IP address"));</script></th>
						  <th><script>document.write(_("Subnet mask"));</script></th> 
						  <th><script>document.write(_("Gateway"));</script></th>
						  <th><script>document.write(_("Operate"));</script></th>
						</tr>
						<tr align=center>
						  <td nowrap>
							<input name=ip size=15 maxlength=15>         
						  </td>
						  <td nowrap>
							<input name=nm size=15 maxlength=15>
						  </td>
						  <td nowrap>
							<input name=gw size=15 maxlength=15>
						  </td>
						  <td><input id="add" class=button2 type="button" name="staticAdd" value=""  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  onClick=addRte(this.form)></td>
						</tr>
					</table>
					<div id="routeList" style="position:relative;visibility:visible;"></div> 
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
		<script>helpInfo(_("routing_static_Help_Inf1")+_("routing_static_Help_Inf2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');	  
    </script>
</BODY>
</HTML>


