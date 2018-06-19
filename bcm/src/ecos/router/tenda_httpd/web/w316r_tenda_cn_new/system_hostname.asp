<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<meta http-equiv="Pragma" content="no-cache" />
<title>System | System Settings</title>
<link type="text/css" rel="stylesheet" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
addCfg("HN",0,"ROUTER");
addCfg("DN",1,"DOMAIN.COM");
var tz = "<%aspTendaGetStatus("sys","timezone");%>",//时间
	time = "<%aspTendaGetStatus("sys","manualTime");%>",
	mode = "<%aspTendaGetStatus("sys","timeMode");%>";//0:关闭手动输入 1：启用手动输入
function init(f){
	var today = new Date();
	f.year.value = today.getFullYear();
	f.month.value = today.getMonth()+1;
	f.day.value = today.getDate();
	f.hour.value = today.getHours();
	f.minute.value = today.getMinutes();
	f.second.value = today.getSeconds();
	if(mode == 2)
	{
		f.manualEN.checked = true;	
	}
	else
	{		
		f.manualEN.checked = false;
	}
	
	f.TZ.selectedIndex = parseInt(tz);
	onManualSet();
}

function preSubmit(f) {
	var loc = "/goform/SysToolTime?GO=system_hostname.asp";
	if(f.manualEN.checked == true) {
		var sy = f.year.value;
		var smo = f.month.value;
		var sd = f.day.value;
		var sh = f.hour.value;
		var smi = f.minute.value;
		var ss = f.second.value;
		var t = /^[0-9]{1,4}$/;
		
		if(!t.test(sy) || !t.test(smo) || !t.test(sd) ||
				!t.test(sh) || !t.test(smi) || !t.test(ss)){
			alert("时间格式有误");
			return false;
		}		
		if(sy < 1971 || sy > 2037) {
			alert("时间年设置超出范围（1971-2037）！");
			return false;
		}
		if(sy.length<4 || Number(sy)<1971 || Number(sy)>2999 || Number(smo)>12 || Number(smo)<1 || smo.length<1 || sd.length<1 || Number(sd)<1)
		{
			alert("请输入合法的年月日");
			return ;
		}
		if(Number(smo) == 2)//2月
		{
		    if(Number(sy)%400==0||(Number(sy)%4==0&&Number(sy)%100!=0))
			{
				if(Number(sd) > 29)
				{
					alert("请输入合法的日");
					return ;
				}
			}
			else if(Number(sd) > 28)
			{
			     alert("请输入合法的日");
				 return ;
			}
		}		
		else if(Number(smo)==4 || Number(smo)==6 || Number(smo)==9 || Number(smo)==11)//4.6.9.11
		{
			if(Number(sd)>30)
			{
				alert("请输入合法的日");
				return ;
			}
		}
		else//1.3.5.7.8.10.12
		{
			if(Number(sd)>31)
			{
				alert("请输入合法的日");
				return ;
			}
		}		
		if(Number(sh)>23 || Number(smi)>59 || Number(ss)>59)
		{
			alert("请输入合法的时间");
			return ;
		}
		if(sh == "")
			sh = "00";
		if(smi == "")
			smi = "00";
		if(ss == "")
			ss = "00";
		
		loc += "&manualEN=2";
		//loc += "&time=" + sy + "-" + smo + "-" + sd + "-" + sh + "-" + smi + "-" + ss;
		var curdate = new Date(Date.UTC(sy, smo-1 ,sd, sh, smi,ss));
		var computer = parseInt(curdate.getTime()/1000) ;
		loc += "&time=" + computer;
	}
	else
	{
		loc += "&manualEN=0";
		loc += "&time=";
	}
	loc += "&TZ=" + f.TZ.value;
	
	window.location = loc;
}

//选择自设时间
function onManualSet(){
	var f = document.frmSetup;
	if(f.manualEN.checked == true){
		document.getElementById("setTab").disabled = false;
		document.getElementById("year").disabled = document.getElementById("month").disabled = document.getElementById("day").disabled = document.getElementById("hour").disabled = 
		document.getElementById("minute").disabled = document.getElementById("second").disabled = document.getElementById("year").disabled = document.getElementById("year").disabled = false;
	} else {
		document.getElementById("setTab").disabled = true;
		document.getElementById("year").disabled = document.getElementById("month").disabled = document.getElementById("day").disabled = document.getElementById("hour").disabled = 
		document.getElementById("minute").disabled = document.getElementById("second").disabled = document.getElementById("year").disabled = document.getElementById("year").disabled = true;
	}

}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup id="frmSetup" method=POST action=/goform/SysToolTime>
<input type=hidden id=GO value="system_hostname.asp">
	<fieldset>
		<legend>网络时间</legend>
		<table class="content1" id="table1">
			<tr>
				<td>时区<br />
				  <select id="TZ" style="width:auto">
					<option value="0">（GMT-12：00）国际日期变更线以西</option>                               
					<option value="1">（GMT-11：00）中途岛，萨摩亚群岛</option>                              
					<option value="2">（GMT-10：00）夏威夷，火奴鲁鲁</option>                                
					<option value="3">（GMT-09：00）阿拉斯加</option>                                        
					<option value="4">（GMT-08：00）太平洋时间（美国和加拿大）;蒂华纳</option>               
					<option value="5">（GMT-07：00）山地时间（美国和加拿大）</option>                        
					<option value="6">（GMT-07：00）奇瓦瓦，拉巴斯，马萨特兰</option>                        
					<option value="7">（GMT-07：00）亚利桑那</option>                                        
					<option value="8">（GMT-06：00）中部时间（美国和加拿大）</option>                        
					<option value="9">（GMT-06：00）瓜达拉哈拉，墨西哥城，蒙特雷</option>              
					<option value="10">（GMT-05：00）东部时间（美国和加拿大）</option>                 
					<option value="11">（GMT-05：00）印地安那（东）</option>                               
					<option value="12">（GMT-05：00）波哥大，利马，基多</option>                           
					<option value="13">（GMT-04：00）大西洋时间（加拿大）</option>                         
					<option value="14">（GMT-04：00）马瑙斯</option>                                       
					<option value="15">（GMT-04：00）加拉加斯，拉巴斯</option>                             
					<option value="16">（GMT-03：30）纽芬兰和拉布拉多</option>                             
					<option value="17">（GMT-03：00）巴西利亚</option>                                     
					<option value="18">（GMT-03：00）格陵兰</option>                                       
					<option value="19">（GMT-03：00）布宜诺斯艾利斯，乔治敦</option>                       
					<option value="20">（GMT-02：00）大西洋中部</option>                                   
					<option value="21">（GMT-01：00）亚速尔群岛</option>                                   
					<option value="22">（GMT-01：00）佛得角群岛</option>                               
					<option value="23">（GMT）格林威治标准时间：都柏林，爱丁堡，里斯本，伦敦</option>      
					<option value="24">（GMT）卡萨布兰卡，蒙罗维亚</option>                                
					<option value="25">（GMT+01:00）贝尔格莱德，布拉迪斯拉发，布达佩斯，卢布尔雅那，布拉格</option>
					<option value="26">（GMT+01:00）萨拉热窝，斯科普里，华沙，萨格勒布</option>            
					<option value="27">（GMT+01:00）布鲁塞尔，哥本哈根，马德里，巴黎</option>              
					<option value="28">（GMT+01:00）西中非</option>                                        
					<option value="29">（GMT+01:00）阿姆斯特丹，柏林，伯尔尼，罗马，斯德哥尔摩，维也纳</option>
					<option value="30">（GMT+02:00）雅典，布加勒斯特，伊斯坦布尔</option>                  
					<option value="31">（GMT+02:00）明斯克</option>                                        
					<option value="32">（GMT+02:00）开罗</option>                                          
					<option value="33">（GMT+02:00）赫尔辛基，基辅，里加，索非亚，塔林，维尔纽斯</option>    
					<option value="34">（GMT+02:00）耶路撒冷</option>                                      
					<option value="35">（GMT+02:00）温得和克</option>                                  
					<option value="36">（GMT+02:00）哈拉雷，比勒陀利亚</option>                          
					<option value="37">（GMT+03:00）莫斯科，圣彼得堡，伏尔加格勒</option>                
					<option value="38">（GMT+03:00）科威特，利雅得</option>                              
					<option value="39">（GMT+03:00）巴格达</option>                                      
					<option value="40">（GMT+03:00）内罗毕</option>                                      
					<option value="41">（GMT+03:30）德黑兰</option>                                      
					<option value="42">（GMT+04:00）阿布扎比，马斯喀特</option>                          
					<option value="43">（GMT+04:00）巴库</option>                                        
					<option value="44">（GMT+04:00）埃里温</option>                                      
					<option value="45">（GMT+04:00）第比利斯</option>                                  
					<option value="46">（GMT+04:30）喀布尔</option>                                    
					<option value="47">（GMT+05:00）伊斯兰堡，卡拉奇，塔什干</option>                  
					<option value="48">（GMT+05:00）叶卡捷琳堡</option>                                
					<option value="49">（GMT+05:30）钦奈，加尔各答，孟买，新德里</option>              
					<option value="50">（GMT+05:45）加德满都</option>                                    
					<option value="51">（GMT+06:00）阿斯塔纳，达卡</option>                            
					<option value="52">（GMT+06:00）阿拉木图，新西伯利亚</option>                      
					<option value="53">（GMT+06:00）科伦坡</option>                                      
					<option value="54">（GMT+06:30）仰光（仰光）</option>                              
					<option value="55">（GMT+07:00）克拉斯诺亚尔斯克</option>                            
					<option value="56">（GMT+07:00）曼谷，河内，雅加达</option>                          
					<option value="57">（GMT+08:00）北京，重庆，香港特别行政区，乌鲁木齐</option>        
					<option value="58">（GMT+08:00）伊尔库次克，乌兰巴托</option>                        
					<option value="59">（GMT+08:00）吉隆坡，新加坡</option>                                
					<option value="60">（GMT+08:00）珀斯</option>                                        
					<option value="61">（GMT+09:00）雅库茨克</option>                                    
					<option value="62">（GMT+09:00）大阪，札幌，东京</option>                          
					<option value="63">（GMT+09:00）首尔</option>                                        
					<option value="64">（GMT+09:30）达尔文</option>                                      
					<option value="65">（GMT+09:30）阿德莱德</option>                                    
					<option value="66">（GMT+10:00）布里斯班</option>                                    
					<option value="67">（GMT+10:00）堪培拉，墨尔本，悉尼</option>                      
					<option value="68">（GMT+10:00）关岛，莫尔兹比港</option>                            
					<option value="69">（GMT+10:00）符拉迪沃斯托克</option>                              
					<option value="70">（GMT+10:00）霍巴特</option>                                      
					<option value="71">（GMT+11:00）马加丹，所罗门群岛，新喀里多</option>              
					<option value="72">（GMT+12:00）斐济群岛，堪察加半岛，马绍尔群岛</option>          
					<option value="73">（GMT+12:00）奥克兰，惠灵顿</option>                            
					<option value="74">（GMT+13:00）努库阿洛法</option>                                
				</select>
			</td></tr>
		</table>
		<table class="content2">
			<tr><td height="30">（注意：仅在连上互联网后才能获取GMT时间。）</td>
			</tr>
		</table>
		<table class="content2" id="table2">
			<tr>
			<td>
				<label class="checkbox"><input type="checkbox" id="manualEN" onClick="onManualSet()" />自定义时间</label>
			</td>
			</tr>
			<tr><td id="setTab">
				<input type="text" class="text input-mini" id="year" size="4" maxlength="4">年
				<input type="text" class="text input-mic-mini" id="month" size="2" maxlength="2">月
				<input type="text" class="text input-mic-mini" id="day" size="2" maxlength="2">日
				<input type="text" class="text input-mic-mini" id="hour" size="2" maxlength="2">时
				<input type="text" class="text input-mic-mini" id="minute" size="2" maxlength="2">分
				<input type="text" class="text input-mic-mini" id="second" size="2" maxlength="2">秒
				</td>
			</tr>
		</table>
	</fieldset>
    <script>tbl_tail_save("document.frmSetup");</script>    
</form>
</body>
</html>