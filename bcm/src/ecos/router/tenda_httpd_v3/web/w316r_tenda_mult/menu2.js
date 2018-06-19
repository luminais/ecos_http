function tbl_tail_save_str1(f)
{
	var m='<table width="75%" border="0" cellpadding="0" cellspacing="0" style="padding-top:20px;"><tr><td align="center">';
	m+="<input type=\"button\" class=\"button1\" value=\"Apply\"   onMouseOver=\"style.color='#FF9933'\" onMouseOut=\"style.color='#000000'\" onClick=preSubmit("+f+")>&nbsp;&nbsp;&nbsp;&nbsp;";
	m+="<input type=\"button\" class=\"button1\" value=\"Cancel\"   onMouseOver=\"style.color='#FF9933'\" onMouseOut=\"style.color='#000000'\" onClick=init("+f+")>";
	m+='</td></tr></table>';
	return m;
}

function tbl_tail_save1(f,button)
{
	document.write(tbl_tail_save_str1(f,button));
}


function helpInfo1(info)
{
	var m = '<table border="0" cellpadding="0" cellspacing="0"  class="left1" style="margin-top:25px;">';
	m += '<tr><td align="center"><b>Help information</b></td></tr>';
	m += '<tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;' + info +  '</td></tr>';
	m += '</table>';
	
	document.write(m);
}