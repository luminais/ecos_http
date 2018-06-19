<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=gb2312">
<TITLE>Firewall | MAC Filtering</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
var max=10;
var isempty=0;
var selectNum = new Array();
var tmp="";
addCfg("check_en",40,"<%getfirewall("lan","acl_mac");%>");//0:unselect;1;selected
//当前显示哪条
addCfg("curNum",72,"<%getfirewall("lan","acl_macNum");%>");//当前规则条:1~10
addCfg("ML1",60,'<%mAclGetMacFilter("1");%>');
addCfg("ML2",61,'<%mAclGetMacFilter("2");%>');
addCfg("ML3",62,'<%mAclGetMacFilter("3");%>');
addCfg("ML4",63,'<%mAclGetMacFilter("4");%>');
addCfg("ML5",64,'<%mAclGetMacFilter("5");%>');
addCfg("ML6",65,'<%mAclGetMacFilter("6");%>');
addCfg("ML7",66,'<%mAclGetMacFilter("7");%>');
addCfg("ML8",67,'<%mAclGetMacFilter("8");%>');
addCfg("ML9",68,'<%mAclGetMacFilter("9");%>');
addCfg("ML10",69,'<%mAclGetMacFilter("10");%>');


var weekday=new Array("星期日","星期一","星期二","星期三","星期四","星期五","星期六");
var filter_mode = new Array("禁用","仅禁止","仅允许");
var filter_mode_value = new Array("disable","deny","pass");

var curNum = getCfg("curNum");


function init(f)
{
	var filter_mode_index =  getCfg("check_en");
	f.filter_mode.options.selectedIndex = parseInt(filter_mode_index);
	
	filter_mode_change(f);

	f.curNum.selectedIndex = curNum-1;
	onChangeNum(f);
	isemptyRule();
}

function isemptyRule()
{
	isempty=0;
	for(var i=1;i<11;i++)
	{
		var cl=getCfg("ML"+i);
		if(cl != ""){
			selectNum[i-1] = "exist";
			isempty=isempty+1;
		}else{
			selectNum[i-1] = "none";	
		}
	}
}
function filter_mode_change(f)
{
	var on = f.filter_mode.options.selectedIndex;
	if(on != 0){
		document.getElementById("enableTab").style.display = "" ;
	}else{
		document.getElementById("enableTab").style.display="none";
	}
}

//mac,1-0,3600-0,on,desc
function onChangeNum(f)
{
	var n = f.curNum.selectedIndex + 1;
	var c1 = getCfg("ML"+n).split(",");
	var j;
	if(getCfg("ML"+n)!="")
	{
		for( j=0;j<6;j++)
		{
			var m = (c1[0]).split(":");
			f.mac[j].value = m[j];
		}

		f.startw.value =  (c1[1].split("-"))[0];
		f.endw.value =   (c1[1].split("-"))[1];

		var t= c1[2].split("-");
		
		f.sh.value = parseInt(t[0]/3600)%24;
		f.sm.value = parseInt((t[0]-f.sh.value*3600)/60);
		f.eh.value = parseInt(t[1]/3600)%24;
		f.em.value = parseInt((t[1]-f.eh.value*3600)/60);

		f.en.checked = (c1[3] == "on");

		
		if(c1[4] == "notag")
			c1[4] = "";
		f.remark.value = c1[4];

	}
	else{
		onDel(f);
	}
		
	onSelected();
}

function preSubmit(f)
{
	var loc = "/goform/SafeMacFilter?GO=firewall_mac.asp";
	var st,et;
	
	loc += "&check=" + f.elements["filter_mode"].value;
	if(f.elements["filter_mode"].value != "disable")
	{
		var mac = combinMAC2(f.mac); 
		
		sh = f.sh.value;
		eh = f.eh.value;
		sm = f.sm.value;
		em = f.em.value;
		remark = f.remark.value;
		startw = f.startw.value;
		endw = f.endw.value;
		var num = f.curNum.selectedIndex + 1;
		
		loc += "&curNum=" + num; 

		if (mac.length==0&&remark.length==0)
		{
			if(isempty != 0 && selectNum[num-1] == "exist"){
				isempty=isempty-1;
			}
		}	
		else
		{
			if(selectNum[num-1] == "none"){
		    	isempty=isempty+1;
			}
			if (mac=="") { alert("空MAC地址"); return }
			if (!macsCheck(mac,"MAC 地址")) return ;
			if(!ckMacReserve(mac))return ;
			if(Number(eh) < Number(sh))
			{
				alert("起始时间 > 结束时间");
				return ;
			}
			if(Number(eh) == Number(sh))
			{
				if(Number(em) < Number(sm))
				{
					alert("起始时间 >= 结束时间");
					return ;
				}
			}
			if(Number(startw)>Number(endw))
			{
			        alert("起始星期 > 结束星期");
					return ;
			}
			//mac,1-0,3600-0,on,desc
			
			loc += "&CL" + num + "=";
			
			loc += mac + ",";
			loc += startw + "-" + endw +",";
			st = sh*3600+sm*60;
			et = eh*3600+em*60;
			loc += st + "-" + et+",";
			
			if(f.en.checked)
				loc += "on" + ",";
			else
				loc += "off" + ",";	
				
			if(remark == "notag")
			{
				alert("注释不可输入‘notag’");
				return ;
			}
			if(remark == "")
				remark = "notag";
			else
			{
				if(!ckRemark(remark))return ;
			}
			loc += remark;
		}
		if(f.elements["filter_mode"].value=="pass"&&isempty==0)
		{
			isemptyRule();
			alert("选择仅允许时必须输入一条规则！");
			return false;
		}
	}
   	var code = 'location="' + loc + '"';
   	eval(code);
}

function onDel(f)
{
	for(var i=0;i<6;i++)
		f.mac[i].value = "";

	f.remark.value="";
	f.sh.value=0;
	f.eh.value=0;
	f.sm.value=0;
	f.em.value=0;
	f.startw.value=0;
	f.endw.value=6;
	f.en.checked = true;
	onSelected()
}

function onSelected()
{
	var frm = document.frmSetup
	if(document.getElementById("en").checked)
	{
		//document.getElementById("enableTab").disabled = false;
		document.getElementById("remark").disabled = false;
		for(i=0;i<6;i++)
		{
		 frm.mac[i].disabled = false;
		}
		document.getElementById("sh").disabled = false;
		document.getElementById("sm").disabled = false;
		document.getElementById("eh").disabled = false;
		document.getElementById("em").disabled = false;
		document.getElementById("startw").disabled = false;
		document.getElementById("endw").disabled = false;
	}
	else
	{
		//document.getElementById("enableTab").disabled = true;	
		document.getElementById("remark").disabled = true;
		for(i=0;i<6;i++)
		{
		 frm.mac[i].disabled = true;
		}
		document.getElementById("sh").disabled = true;
		document.getElementById("sm").disabled = true;
		document.getElementById("eh").disabled = true;
		document.getElementById("em").disabled = true;	
		document.getElementById("startw").disabled = true;
		document.getElementById("endw").disabled = true;
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
			<form name=frmSetup id="frmSetup" method=POST action=/goform/SafeMacFilter>
				<table class=content1>
				<tr><td height="30" colspan=7>过滤模式 :&nbsp;
				<script>
				var text = '<select id="filter_mode" onChange="filter_mode_change(document.frmSetup)">';
				for(var i=0; i<3; i++){
					text +=	'<option value='+filter_mode_value[i]+' >'+filter_mode[i]+'</option>';	
				}
				
				text += '</select>';
				
				document.write(text);
				</script>
				</td>
				</tr>
				<TR><TD class=hline></TD></TR>
				</table>
				<table id="enableTab" cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
				<tr>
				<td width="100" height="30" align="right">请选择:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;
				<script>
					var str = '<select id=curNum onChange=onChangeNum(document.frmSetup);>';
					
					for(var i=1;i<=max;i++)
					{
						str += '<option value='+i+'>'+ '(' +i+ ')'+'</option>';
					}
					str += '</select>';
					document.write(str);
				</script>
				</td>
				</tr>
				<tr><td height="30" align="right">注释:</td>
				  <td height="30">&nbsp;&nbsp;&nbsp;&nbsp;
				  <input class="text" size="12" maxlength="12" id="remark"></td>
				</tr>
				<tr>
				<td height="30" align="right">MAC 地址:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;
				<input class=text id=mac size=2 maxlength=2 onKeyUp="value=correctMacChar(this.value)">:
				<input class=text id=mac size=2 maxlength=2 onKeyUp="value=correctMacChar(this.value)">:
				<input class=text id=mac size=2 maxlength=2 onKeyUp="value=correctMacChar(this.value)">:
				<input class=text id=mac size=2 maxlength=2 onKeyUp="value=correctMacChar(this.value)">:
				<input class=text id=mac size=2 maxlength=2 onKeyUp="value=correctMacChar(this.value)">:
				<input class=text id=mac size=2 maxlength=2 onKeyUp="value=correctMacChar(this.value)">
				</td>
				</tr>
				<tr>
				<td height="30" align="right">时间:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;
				<script>
				var text = '<select id="sh">';
				for(var i=0; i<=23; i++){
					text +=	'<option value='+i+' >'+i+'</option>';	
				}
				text += '</select>:<select id="sm">';
				for(var j=0;j<12;j++)
				{
					text += '<option value=' + eval(j *5)+ '>' + eval(j*5) + '</option>';
				}
				text += '</select>&nbsp;~&nbsp;<select id="eh">';
				for(var i=0; i<=23; i++){
					text +=	'<option value='+i+' >'+i+'</option>';	
				}
				text += '</select>:<select id="em">';
				for(var j=0;j<12;j++)
				{
					text += '<option value=' + eval(j *5)+ '>' + eval(j*5) + '</option>';
				}
				text += '</select>';
				document.write(text);
				</script>
				</td>
				</tr>
				<tr>
				<td height="30" align="right">日期:</td>
				<td height="30">&nbsp;&nbsp;&nbsp;&nbsp;
				<script>
				var text = '<select id="startw">';
				for(var i=0; i<7; i++){
					text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
				}
				
				text += '</select>&nbsp;~&nbsp;<select id="endw">';
				for(var i=0; i<7; i++){
					text +=	'<option value='+i+' >'+weekday[i]+'</option>';	
				}
				
				text += '</select>';
				document.write(text);
				</script>
				</td>
				</tr>
				<tr><td height="30" align="right">启用:</td>
				  <td height="30">&nbsp;&nbsp;&nbsp;&nbsp;
				  <input type="checkbox" id="en" onClick="onSelected()">&nbsp;&nbsp;&nbsp;&nbsp;清空该项:<input type="button" class="button2" id="delbtn" value="清空"  onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="onDel(document.frmSetup)"></td>
				</tr>
				</table>
			
				
				<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
			
				
			</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
<script>helpInfo(' 您可以通过MAC地址过滤功能控制局域网中计算机对Internet的访问。详细使用请参考产品说明书。<br>\
&nbsp;&nbsp;&nbsp;&nbsp;操作说明：如果要清空已设置过的项，选中该项后点击清空按钮然后保存方生效。<br>\
仅禁止：仅禁止条目内的MAC访问网络<br>\
仅允许：仅允许条目内的MAC访问网络<br>\
&nbsp;&nbsp;&nbsp;&nbsp;注意：时间设置为0:0~0:0表示全部时间段。'
);</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('enableTab');
    </script>
</BODY>
</HTML>





