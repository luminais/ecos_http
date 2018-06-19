//function Click(){ 
//	window.event.returnValue=false; 
//} 
//	document.oncontextmenu=Click; 
function tbl_head_str(title,img)
{
	var m='<table width="100%" border="0" cellpadding="0" cellspacing="0">';
	m+='<tr><td valign=middle  background="./menuok.gif" class=title> ';
	m+='<span class=style1>&nbsp;'+title+'</span>';
	m+='</td></tr></table>';
	return m;
}
function tbl_head(title,img)
{
	document.write(tbl_head_str(title,img));
}
function tbl_tail_str(button)
{
	var m='<table width="75%" border="0" cellpadding="0" cellspacing="0"><tr><td height="40">&nbsp;';
	if (button!='')
	{
		m+=button;
	    m+='</td></tr>';
	}
	m+='</table>';
	return m;
}
function tbl_tail(button)
{
	document.write(tbl_tail_str(button));
}
function tbl_tail_save_str(f)
{
	var m='<table width="75%" border="0" cellpadding="0" cellspacing="0" style="padding-top:20px;"><tr><td align="center">';
	m+="<input type=button class=button1 value="+_('Ok')+" onMouseOver=\"style.color='#FF9933'\" onMouseOut=\"style.color='#000000'\" onClick=preSubmit("+f+")>&nbsp;&nbsp;&nbsp;&nbsp;";
	m+="<input type=\"button\" class=\"button1\" value="+_('Cancel')+" onMouseOver=\"style.color='#FF9933'\" onMouseOut=\"style.color='#000000'\" onClick=init("+f+")>";
	m+='</td></tr></table>';
	return m;
}
function tbl_tail_save(f,button)
{
	document.write(tbl_tail_save_str(f,button));
}
function helpInfo(info)
{
	var m = '<table border="0" cellpadding="0" cellspacing="0"  class="left1" style="margin-top:25px;">';
	m += '<tr><td align="center"><b>'+_("Help information")+'</b></td></tr>';
	m += '<tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;' + info +  '</td></tr>';
	m += '</table>';
	
	document.write(m);
}
function popupHelp(info)
{
     window.open (info,'newwindow', 'height=400, width=600, top=200, left=200, toolbar=no, menubar=no, scrollbars=yes, resizable=no,location=no, status=no'); 	
}