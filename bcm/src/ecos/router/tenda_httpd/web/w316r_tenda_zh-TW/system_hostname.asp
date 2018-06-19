<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<TITLE>System | Time Settings</TITLE>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
addCfg("HN",0,"ROUTER");
addCfg("DN",1,"DOMAIN.COM");

var tz = "<%aspTendaGetStatus("sys","timezone");%>";//时间
var time = "<%aspTendaGetStatus("sys","manualTime");%>";
var mode = "<%aspTendaGetStatus("sys","timeMode");%>";//0:关闭手动输入 1：启用手动输入

function init(f)
{
/*
	var t = time.split("-");
	f.year.value = t[0];
	f.month.value = t[1];
	f.day.value = t[2];
	f.hour.value = t[3];
	f.minute.value = t[4];
	f.second.value = t[5];
*/
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
	if (!strCheck(f.HN,"主機名稱")) return;
	if (!strCheck(f.DN,"網域名稱")) return;
	var loc = "/goform/SysToolTime?GO=system_hostname.asp";

	if(f.manualEN.checked == true)
	{
		var sy = f.year.value;
		var smo = f.month.value;
		var sd = f.day.value;
		var sh = f.hour.value;
		var smi = f.minute.value;
		var ss = f.second.value;
		var t = /^[0-9]{1,4}$/;
		
		if(!t.test(sy) || !t.test(smo) || !t.test(sd) || !t.test(sh) || !t.test(smi) || !t.test(ss))
		{
			alert("時間格式錯誤！");
			return false;
		}
		
		if(sy < 1901 || sy > 2037)
		{
			alert("年份設定超出範圍！(範圍：1901 ~ 2037)");
			return false;
		}
		
		if(sy.length<4 || Number(sy)<1970 || Number(sy)>2999 || Number(smo)>12 || Number(smo)<1 || smo.length<1 || sd.length<1 || Number(sd)<1)
		{
			alert("請輸入正確的年月日");
			return ;
		}
		if(Number(smo) == 2)//2月
		{
		    if(Number(sy)%400==0||(Number(sy)%4==0&&Number(sy)%100!=0))
			{
				if(Number(sd) > 29)
				{
					alert("請輸入正確的日");
					return ;
				}
			}
			else if(Number(sd) > 28)
			{
			     alert("請輸入正確的日");
				 return ;
			}
		}
		
		else if(Number(smo)==4 || Number(smo)==6 || Number(smo)==9 || Number(smo)==11)//4.6.9.11
		{
			if(Number(sd)>30)
			{
				alert("請輸入正確的日");
				return ;
			}
		}
		else//1.3.5.7.8.10.12
		{
			if(Number(sd)>31)
			{
				alert("請輸入正確的日");
				return ;
			}
		}
		
		if(Number(sh)>23 || Number(smi)>59 || Number(ss)>59)
		{
			alert("請輸入正確的時間");
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
	//loc += "&TZSel=" + f.TZ.selectedIndex;
	
	var code = 'location="' + loc + '"';
	eval(code);
}

//选择自设时间
function onManualSet()
{
	var f = document.frmSetup;
	if(f.manualEN.checked == true)
	{
		document.getElementById("setTab").disabled = false;
		document.getElementById("year").disabled = document.getElementById("month").disabled = document.getElementById("day").disabled = document.getElementById("hour").disabled = 
		document.getElementById("minute").disabled = document.getElementById("second").disabled = document.getElementById("year").disabled = document.getElementById("year").disabled = false;
	}
	else
	{
		document.getElementById("setTab").disabled = true;
		document.getElementById("year").disabled = document.getElementById("month").disabled = document.getElementById("day").disabled = document.getElementById("hour").disabled = 
		document.getElementById("minute").disabled = document.getElementById("second").disabled = document.getElementById("year").disabled = document.getElementById("year").disabled = true;
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
				<form name=frmSetup id="frmSetup" method=POST action=/goform/SysToolTime>
				<input type=hidden id=GO value="system_hostname.asp">
				<table cellpadding="0" cellspacing="0" class="content1" id="table1">
				
					<tr><td height="30">&nbsp;&nbsp;時區： 
					  <select class="list" id="TZ">
										<option value="0">（GMT-12：00）國際換日線以西</option>                               
										<option value="1">（GMT-11：00）中途島、薩摩亞群島</option>                              
										<option value="2">（GMT-10：00）夏威夷、檀香山(火奴鲁鲁)</option>                                
										<option value="3">（GMT-09：00）阿拉斯加</option>                                        
										<option value="4">（GMT-08：00）太平洋時間（美國和加拿大）、蒂華納</option>               
										<option value="5">（GMT-07：00）山區時間（美國和加拿大）</option>                        
										<option value="6">（GMT-07：00）奇瓦瓦，拉巴斯，馬薩特蘭</option>                        
										<option value="7">（GMT-07：00）亞利桑那</option>                                        
										<option value="8">（GMT-06：00）中部時間（美國和加拿大）</option>                        
										<option value="9">（GMT-06：00）瓜達拉哈拉、墨西哥城、蒙特雷</option>              
										<option value="10">（GMT-05：00）東部時間（美國和加拿大）</option>                 
										<option value="11">（GMT-05：00）印第安那（東）</option>                               
										<option value="12">（GMT-05：00）波哥大、利馬、基多</option>                           
										<option value="13">（GMT-04：00）大西洋時間（加拿大）</option>                         
										<option value="14">（GMT-04：00）馬瑙斯</option>                                       
										<option value="15">（GMT-04：00）卡拉卡斯、拉巴斯</option>                             
										<option value="16">（GMT-03：30）紐芬蘭、拉布拉多</option>                             
										<option value="17">（GMT-03：00）巴西利亞</option>                                     
										<option value="18">（GMT-03：00）格陵蘭</option>                                       
										<option value="19">（GMT-03：00）布宜諾斯艾利斯</option>                       
										<option value="20">（GMT-02：00）大西洋中部</option>                                   
										<option value="21">（GMT-01：00）亞速爾群島</option>                                   
										<option value="22">（GMT-01：00）佛德角群島</option>                               
										<option value="23">（GMT）格林威治標準時間：都柏林、愛丁堡、里斯本、倫敦</option>      
										<option value="24">（GMT）卡薩布蘭卡、蒙羅維亞</option>                                
										<option value="25">（GMT+01:00）貝爾格勒、布拉提斯拉瓦、布達佩斯、盧布亞納、布拉格</option>
										<option value="26">（GMT+01:00）塞拉耶佛、史高比耶、華沙、札格雷布</option>            
										<option value="27">（GMT+01:00）布魯塞爾、哥本哈根、馬德里、巴黎</option>              
										<option value="28">（GMT+01:00）西中非</option>                                        
										<option value="29">（GMT+01:00）阿姆斯特丹、柏林、伯恩、羅馬、斯德哥爾摩、維也納</option>
										<option value="30">（GMT+02:00）雅典、布加勒斯特、伊斯坦堡</option>                  
										<option value="31">（GMT+02:00）明斯克</option>                                        
										<option value="32">（GMT+02:00）開羅</option>                                          
										<option value="33">（GMT+02:00）赫爾辛基、基輔、里加、索菲亞、塔林、維爾紐斯</option>    
										<option value="34">（GMT+02:00）耶路撒冷</option>                                      
										<option value="35">（GMT+02:00）溫荷克</option>                                  
										<option value="36">（GMT+02:00）哈拉雷、普利托里亞</option>                          
										<option value="37">（GMT+03:00）莫斯科、聖彼德堡、伏爾加格勒</option>                
										<option value="38">（GMT+03:00）科威特、利雅德</option>                              
										<option value="39">（GMT+03:00）巴格達</option>                                      
										<option value="40">（GMT+03:00）奈洛比</option>                                      
										<option value="41">（GMT+03:30）德黑蘭</option>                                      
										<option value="42">（GMT+04:00）阿布達比、馬斯喀特</option>                          
										<option value="43">（GMT+04:00）巴庫</option>                                        
										<option value="44">（GMT+04:00）葉里溫</option>                                      
										<option value="45">（GMT+04:00）第比利斯</option>                                  
										<option value="46">（GMT+04:30）喀布爾</option>                                    
										<option value="47">（GMT+05:00）伊斯蘭堡、卡拉奇、塔什干</option>                  
										<option value="48">（GMT+05:00）葉卡捷琳堡</option>                                
										<option value="49">（GMT+05:30）欽奈、加爾各答、孟買、新德里</option>              
										<option value="50">（GMT+05:45）加德滿都</option>                                    
										<option value="51">（GMT+06:00）阿斯坦納、達卡</option>                            
										<option value="52">（GMT+06:00）阿拉木圖、新西伯利亞</option>                      
										<option value="53">（GMT+06:00）可倫坡</option>                                      
										<option value="54">（GMT+06:30）仰光（仰光）</option>                              
										<option value="55">（GMT+07:00）克拉斯諾亞爾斯克</option>                            
										<option value="56">（GMT+07:00）曼谷、河內、雅加達</option>                          
										<option value="57">（GMT+08:00）台北、北京、重慶、香港特別行政區、烏魯木齊</option>        
										<option value="58">（GMT+08:00）伊爾庫次克、烏蘭巴托</option>                        
										<option value="59">（GMT+08:00）吉隆玻、新加坡</option>                                
										<option value="60">（GMT+08:00）珀斯</option>                                        
										<option value="61">（GMT+09:00）亞庫次克</option>                                    
										<option value="62">（GMT+09:00）大阪、札幌、東京</option>                          
										<option value="63">（GMT+09:00）漢城</option>                                        
										<option value="64">（GMT+09:30）達爾文</option>                                      
										<option value="65">（GMT+09:30）阿德萊德</option>                                    
										<option value="66">（GMT+10:00）布里斯班</option>                                    
										<option value="67">（GMT+10:00）坎培拉、墨爾本、悉尼</option>                      
										<option value="68">（GMT+10:00）關島、莫爾斯貝港</option>                            
										<option value="69">（GMT+10:00）符拉迪沃斯托克</option>                              
										<option value="70">（GMT+10:00）霍巴特</option>                                      
										<option value="71">（GMT+11:00）馬加丹、所羅門群島、新喀裡多</option>              
										<option value="72">（GMT+12:00）斐濟群島、堪察加半島、馬紹爾群島</option>          
										<option value="73">（GMT+12:00）奧克蘭、威靈頓</option>                            
										<option value="74">（GMT+13:00）努瓜婁發</option>                                
					</select>
					</td></tr>
					</table>
				<table cellpadding="0" cellspacing="0" class="content3">
					<tr><td height="30">&nbsp;&nbsp;（注意：只有在連上網際網路後才能取得GMT時間。）</td>
					</tr>
				</table>
				<table cellpadding="0" cellspacing="0" class="content3" id="table2">
					<tr><td height="30">&nbsp;&nbsp;自訂時間：
					  <input type="checkbox" id="manualEN" onClick="onManualSet()">
					<tr><td height="30" id="setTab">
						&nbsp;&nbsp;<input type="text" class="text" id="year" size="4" maxlength="4">年
						<input type="text" class="text" id="month" size="2" maxlength="2">月
						<input type="text" class="text" id="day" size="2" maxlength="2">日
						<input type="text" class="text" id="hour" size="2" maxlength="2">時
						<input type="text" class="text" id="minute" size="2" maxlength="2">分
						<input type="text" class="text" id="second" size="2" maxlength="2">秒
					</td>
					</tr>
				</table>
					
					<SCRIPT>tbl_tail_save("document.frmSetup");</SCRIPT>
					
				</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td width="264" align="center" valign="top" height="100%">
		<script>
		helpInfo('本頁面可設定路由器的系統時間、您可以選擇自己設定時間或者從網際網路上取得標準的GMT時間、然後系統會自動連接NTP伺服器進行時間同步。<br><br>請注意：關閉路由器電源後、時間就會回到初始值、當您下次開機連上Internet後、路由器將會自動取得GMT時間。另外，您必須先連上Internet取得GMT時間或到此頁設定時間後、其他功能（如防火牆）中的時間設定才會生效。');
		</script>

		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
	  table_onload('table2');
    </script>
</BODY>
</HTML>

