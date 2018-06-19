<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8" />
<meta http-equiv="Pragma" content="no-cache" />
<title>System | System Settings</title>
<link type="text/css" rel="stylesheet" href="css/screen.css">
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup id="frmSetup" method=POST action=/goform/SysToolTime>
<input type=hidden id=GO value="system_hostname.asp">
	<fieldset>
		<legend>Time Settings</legend>
		<table class="content1" id="table1">
			<tr>
				<td>Time Zone<br />
				  <select id="TZ" style="width:auto">
					<option value="0">(GMT-12:00) West of International Date Line</option>                               
					<option value="1">(GMT-11:00) Midway Island, Samoa</option>                              
					<option value="2">(GMT-10:00) Hawaii, Honolulu</option>                                
					<option value="3">(GMT-09:00) Alaska</option>                                        
					<option value="4">(GMT-08:00) Pacific Time</option>               
					<option value="5">(GMT-07:00) Mountain Time</option>                        
					<option value="6">(GMT-07:00) Chihuahua, La Paz, Mazatlan</option>                        
					<option value="7">(GMT-07:00) Arizona</option>                                        
					<option value="8">(GMT-06:00) Central Time</option>                        
					<option value="9">(GMT-06:00) Guadalajara, Mexico City, Monterrey</option>              
					<option value="10">(GMT-05:00) Eastern Time</option>                 
					<option value="11">(GMT-05:00) India to Ann</option>                               
					<option value="12">(GMT-05:00) Bogota, Lima, Quito</option>                           
					<option value="13">(GMT-04:00) Caracas, La Paz</option>                         
					<option value="14">(GMT-04:00) Atlantic Time</option>                                       
					<option value="15">(GMT-04:00) Manaus</option>                             
					<option value="16">(GMT-03:30) Newfoundland and Labrador</option>                             
					<option value="17">(GMT-03:00) Brasilia Asia</option>                                     
					<option value="18">(GMT-03:00) Greenland</option>                                       
					<option value="19">(GMT-03:00) Buenos Aires, Georgetown</option>                       
					<option value="20">(GMT-02:00) Atlantic</option>                                   
					<option value="21">(GMT-01:00) Azores</option>                                   
					<option value="22">(GMT-01:00) Cape Verde Islands</option>                               
					<option value="23">(GMT) Greenwich Mean Time</option>      
					<option value="24">(GMT) Casablanca , Monrovia</option>                                
					<option value="25">(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague</option>
					<option value="26">(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb</option>            
					<option value="27">(GMT+01:00) Brussels, Copenhagen, Madrid, Paris</option>              
					<option value="28">(GMT+01:00) West Africa</option>                                        
					<option value="29">(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna</option>
					<option value="30">(GMT+02:00) Athens, Bucharest, Istanbul</option>                  
					<option value="31">(GMT+02:00) Minsk</option>                                        
					<option value="32">(GMT+02:00) Cairo</option>                                          
					<option value="33">(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius</option>    
					<option value="34">(GMT+02:00) Jerusalem</option>                                      
					<option value="35">(GMT+02:00) temperature 'll have</option>                                  
					<option value="36">(GMT+02:00) grams. Harare, Pretoria</option>                          
					<option value="37">(GMT+03:00) Moscow, St. Petersburg, Volgograd</option>                
					<option value="38">(GMT+03:00) Kuwait, Riyadh</option>                              
					<option value="39">(GMT+03:00) Baghdad</option>                                      
					<option value="40">(GMT+03:00) Nairobi</option>                                      
					<option value="41">(GMT+03:30) Tehran</option>                                      
					<option value="42">(GMT+04:00) Abu Dhabi, Muscat</option>                          
					<option value="43">(GMT+04:00) Baku</option>                                        
					<option value="44">(GMT+04:00) Yerevan</option>                                      
					<option value="45">(GMT+04:00) Tbilisi</option>                                  
					<option value="46">(GMT+04:30) Kabul</option>                                    
					<option value="47">(GMT+05:00) Islamabad, Karachi, Tashkent</option>                  
					<option value="48">(GMT+05:00) Yekaterinburg</option>                                
					<option value="49">(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi</option>              
					<option value="50">(GMT+05:45) Kathmandu</option>                                    
					<option value="51">(GMT+06:00) Astana, Dhaka</option>                            
					<option value="52">(GMT+06:00) Almaty, Novosibirsk</option>                      
					<option value="53">(GMT+06:00) Colombo</option>                                      
					<option value="54">(GMT+06:30) Yangon</option>                              
					<option value="55">(GMT+07:00) Krasnoyarsk</option>                            
					<option value="56">(GMT+07:00) Bangkok, Hanoi, Jakarta</option>                          
					<option value="57">(GMT+08:00) Beijing, Chongqing, Hong Kong, Urumqi</option>        
					<option value="58">(GMT+08:00) Irkutsk grams, Ulaanbaatar</option>                        
					<option value="59">(GMT+08:00) Kuala Lumpur, Singapore</option>                                
					<option value="60">(GMT+08:00) Perth</option>                                        
					<option value="61">(GMT+09:00) Yakutsk</option>                                    
					<option value="62">(GMT+09:00) Osaka, Sapporo, Tokyo</option>                          
					<option value="63">(GMT+09:00) Seoul</option>                                        
					<option value="64">(GMT+09:30) Darwin</option>                                      
					<option value="65">(GMT+09:30) Adelaide</option>                                    
					<option value="66">(GMT+10:00) Brisbane</option>                                    
					<option value="67">(GMT+10:00) Canberra, Melbourne Sydney</option>                      
					<option value="68">(GMT+10:00) Guam, Port Moresby</option>                            
					<option value="69">(GMT+10:00) Vladivostok</option>                              
					<option value="70">(GMT+10:00) Hobart</option>                                      
					<option value="71">(GMT+11:00) Magadan, Solomon Islands, New Caledonia and more</option>              
					<option value="72">(GMT+12:00) Fiji Islands, Kamchatka Peninsula, the Marshall Islands</option>          
					<option value="73">(GMT+12:00) Auckland , Wellington</option>                            
					<option value="74">(GMT+13:00) Nuku'alofa</option>                                
				</select>
			</td></tr>
		</table>
		<table class="content2">
			<tr><td height="30">hostname_Note</td>
			</tr>
		</table>
		<table class="content2" id="table2">
			<tr>
			<td>
				<label class="checkbox"><input type="checkbox" id="manualEN" onClick="onManualSet()" />Customized Time</label>
			</td>
			</tr>
			<tr><td id="setTab">
				<input type="text" class="text input-mini" id="year" size="4" maxlength="4">Year
				<input type="text" class="text input-mic-mini" id="month" size="2" maxlength="2">Month
				<input type="text" class="text input-mic-mini" id="day" size="2" maxlength="2">Day
				<input type="text" class="text input-mic-mini" id="hour" size="2" maxlength="2">Hour
				<input type="text" class="text input-mic-mini" id="minute" size="2" maxlength="2">Minute
				<input type="text" class="text input-mic-mini" id="second" size="2" maxlength="2">Second
				</td>
			</tr>
		</table>
	</fieldset>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.frmSetup)" />
	</div>   
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "system_tool"]);
	B.translate();
})();
</script>
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
			alert(_("Please enter valid time information!"));
			return false;
		}		
		if(sy < 1971 || sy > 2037) {
			alert(_("Please enter valid year between 1971-2037!"));
			return false;
		}
		if(sy.length<4 || Number(sy)<1971 || Number(sy)>2999 || Number(smo)>12 || Number(smo)<1 || smo.length<1 || sd.length<1 || Number(sd)<1)
		{
			alert(_("Please enter valid year/month/day info!"));
			return ;
		}
		if(Number(smo) == 2)//2月
		{
		    if(Number(sy)%400==0||(Number(sy)%4==0&&Number(sy)%100!=0))
			{
				if(Number(sd) > 29)
				{
					alert(_("Please enter a valid day!"));
					return ;
				}
			}
			else if(Number(sd) > 28)
			{
			     alert(_("Please enter a valid day!"));
				 return ;
			}
		}		
		else if(Number(smo)==4 || Number(smo)==6 || Number(smo)==9 || Number(smo)==11)//4.6.9.11
		{
			if(Number(sd)>30)
			{
				alert(_("Please enter a valid day!"));
				return ;
			}
		}
		else//1.3.5.7.8.10.12
		{
			if(Number(sd)>31)
			{
				alert(_("Please enter a valid day!"));
				return ;
			}
		}		
		if(Number(sh)>23 || Number(smi)>59 || Number(ss)>59)
		{
			alert(_("Please enter valid time info!"));
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
</body>
</html>