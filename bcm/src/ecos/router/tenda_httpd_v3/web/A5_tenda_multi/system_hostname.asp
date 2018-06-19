<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>System | Time Settings</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<SCRIPT language=JavaScript>
addCfg("HN",0,"ROUTER");
addCfg("DN",1,"DOMAIN.COM");
var tz = "<%aspTendaGetStatus("sys","timezone");%>";
var time = "<%aspTendaGetStatus("sys","manualTime");%>";
var mode = "<%aspTendaGetStatus("sys","timeMode");%>";
Butterlate.setTextDomain("system_tool");
function init(f)
{
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
	if (!strCheck(f.HN,_("Hostname"))) return;
	if (!strCheck(f.DN,_("Domain Name"))) return;
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
			alert(_("Time format wrong"));
			return false;
		}
		if(sy.length<4 || Number(smo)>12 || Number(smo)<1 || smo.length<1 || sd.length<1 || Number(sd)<1)
		{
			alert(_("Please enter the valid year ,month and date"));
			return ;
		}
		if( Number(sy)<=1969 || Number(sy)>=2038)
		{
			alert(_("The year only can be 1970 to 2037"));
			return ;
		}
		if(Number(smo) == 2)
		{
		    if(Number(sy)%400==0||(Number(sy)%4==0&&Number(sy)%100!=0))
			{
				if(Number(sd) > 29)
				{
					alert(_("Enter a valid date"));
					return ;
				}
			}
			else if(Number(sd) > 28)
			{
			     alert(_("Enter a valid date"));
				 return ;
			}
		}
		else if(Number(smo)==4 || Number(smo)==6 || Number(smo)==9 || Number(smo)==11)//4.6.9.11
		{
			if(Number(sd)>30)
			{
				alert(_("Enter a valid date"));
				return ;
			}
		}
		else
		{
			if(Number(sd)>31)
			{
				alert(_("Enter a valid date"));
				return ;
			}
		}
		if(Number(sh)>23 || Number(smi)>59 || Number(ss)>59)
		{
			alert(_("Enter a valid time"));
			return ;
		}
		if(sh == "")
			sh = "00";
		if(smi == "")
			smi = "00";
		if(ss == "")
			ss = "00";
		loc += "&manualEN=2";
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
	var code = 'location="' + loc + '"';
	eval(code);
}
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
					<tr><td height="30">&nbsp;&nbsp;<script>document.write(_("Time zone"));</script>: 
					  <select class="list" id="TZ">
						<option value="0">(GMT-12:00)<script>document.write(_("West of International Date Line"));</script></option>
						<option value="1">(GMT-11:00)<script>document.write(_("Midway Island, Samoa"));</script></option>
						<option value="2">(GMT-10:00)<script>document.write(_("Hawaii, Honolulu"));</script></option>
						<option value="3">(GMT-09:00)<script>document.write(_("Alaska"));</script></option>
						<option value="4">(GMT-08:00)<script>document.write(_("Pacific Time"));</script></option>               
						<option value="5">(GMT-07:00)<script>document.write(_("Mountain Time"));</script></option>                        
						<option value="6">(GMT-07:00)<script>document.write(_("Chihuahua, La Paz, Mazatlan"));</script></option>
						<option value="7">(GMT-07:00)<script>document.write(_("Arizona"));</script></option>  
						<option value="8">(GMT-06:00)<script>document.write(_("Central Time"));</script></option>                        
						<option value="9">(GMT-06:00)<script>document.write(_("Guadalajara, Mexico City, Monterrey"));</script></option>
						<option value="10">(GMT-05:00)<script>document.write(_("Eastern Time"));</script></option>
						<option value="11">(GMT-05:00)<script>document.write(_("India to Ann"));</script></option>                               
						<option value="12">(GMT-05:00)<script>document.write(_("Bogota, Lima, Quito"));</script></option>                           
						<option value="13">(GMT-04:30)<script>document.write(_("Caracas, La Paz"));</script></option>                           
						<option value="14">(GMT-04:00)<script>document.write(_("Atlantic Time"));</script></option>                         
						<option value="15">(GMT-04:00)<script>document.write(_("Manaus"));</script></option>                                
						<option value="16">(GMT-03:30)<script>document.write(_("Newfoundland and Labrador"));</script></option>
						<option value="17">(GMT-03:00)<script>document.write(_("Brasilia Asia"));</script></option>
						<option value="18">(GMT-03:00)<script>document.write(_("Greenland"));</script></option>   
						<option value="19">(GMT-03:00)<script>document.write(_("Buenos Aires, Georgetown"));</script></option>                       
						<option value="20">(GMT-02:00)<script>document.write(_("Atlantic"));</script></option>
						<option value="21">(GMT-01:00)<script>document.write(_("Azores"));</script></option>                                   
						<option value="22">(GMT-01:00)<script>document.write(_("Cape Verde Islands"));</script></option>
						<option value="23">(GMT)<script>document.write(_("Greenwich Mean Time"));</script></option>      
						<option value="24">(GMT)<script>document.write(_("Casablanca , Monrovia"));</script></option>                                
						<option value="25">(GMT+01:00)<script>document.write(_("Belgrade, Bratislava, Budapest, Ljubljana, Prague"));</script></option>
						<option value="26">(GMT+01:00)<script>document.write(_("Sarajevo, Skopje, Warsaw, Zagreb"));</script></option>            
						<option value="27">(GMT+01:00)<script>document.write(_("Brussels, Copenhagen, Madrid, Paris"));</script></option>
						<option value="28">(GMT+01:00)<script>document.write(_("West Africa"));</script></option> 
						<option value="29">(GMT+01:00)<script>document.write(_("Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna"));</script></option>
						<option value="30">(GMT+02:00)<script>document.write(_("Athens, Bucharest, Istanbul"));</script></option>                  
						<option value="31">(GMT+02:00)<script>document.write(_("Minsk"));</script></option>   
						<option value="32">(GMT+02:00)<script>document.write(_("Cairo"));</script></option>                                          
						<option value="33">(GMT+02:00)<script>document.write(_("Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius"));</script></option>
						<option value="34">(GMT+02:00)<script>document.write(_("Jerusalem"));</script></option>                                      
						<option value="35">(GMT+02:00)<script>document.write(_("temperature 'll have"));</script></option>    
						<option value="36">(GMT+02:00)<script>document.write(_("grams. Harare, Pretoria"));</script></option>
						<option value="37">(GMT+03:00)<script>document.write(_("Moscow, St. Petersburg, Volgograd"));</script></option>
						<option value="38">(GMT+03:00)<script>document.write(_("Kuwait, Riyadh"));</script></option>
						<option value="39">(GMT+03:00)<script>document.write(_("Baghdad"));</script></option> 
						<option value="40">(GMT+03:00)<script>document.write(_("Nairobi"));</script></option>                                      
						<option value="41">(GMT+03:30)<script>document.write(_("Tehran"));</script></option>                                      
						<option value="42">(GMT+04:00)<script>document.write(_("Abu Dhabi, Muscat"));</script></option>                          
						<option value="43">(GMT+04:00)<script>document.write(_("Baku"));</script></option>     
						<option value="44">(GMT+04:00)<script>document.write(_("Yerevan"));</script></option>                                      
						<option value="45">(GMT+04:00)<script>document.write(_("Tbilisi"));</script></option>                                  
						<option value="46">(GMT+04:30)<script>document.write(_("Kabul"));</script></option>                                    
						<option value="47">(GMT+05:00)<script>document.write(_("Islamabad, Karachi, Tashkent"));</script></option>                  
						<option value="48">(GMT+05:00)<script>document.write(_("Yekaterinburg"));</script></option>                                
						<option value="49">(GMT+05:30)<script>document.write(_("Chennai, Kolkata, Mumbai, New Delhi"));</script></option>
						<option value="50">(GMT+05:45)<script>document.write(_("Kathmandu"));</script></option>
						<option value="51">(GMT+06:00)<script>document.write(_("Astana, Dhaka"));</script></option>                            
						<option value="52">(GMT+06:00)<script>document.write(_("Almaty, Novosibirsk"));</script></option>                      
						<option value="53">(GMT+06:00)<script>document.write(_("Colombo"));</script></option>     
						<option value="54">(GMT+06:30)<script>document.write(_("Yangon"));</script></option>                              
						<option value="55">(GMT+07:00)<script>document.write(_("Krasnoyarsk"));</script></option>                            
						<option value="56">(GMT+07:00)<script>document.write(_("Bangkok, Hanoi, Jakarta"));</script></option>
						<option value="57">(GMT+08:00)<script>document.write(_("Beijing, Chongqing, Hong Kong, Urumqi"));</script></option>        
						<option value="58">(GMT+08:00)<script>document.write(_("Irkutsk grams, Ulaanbaatar"));</script></option> 
						<option value="59">(GMT+08:00)<script>document.write(_("Kuala Lumpur, Singapore"));</script></option>            
						<option value="60">(GMT+08:00)<script>document.write(_("Perth"));</script></option>     
						<option value="61">(GMT+09:00)<script>document.write(_("Yakutsk"));</script></option>                                    
						<option value="62">(GMT+09:00)<script>document.write(_("Osaka, Sapporo, Tokyo"));</script></option>                          
						<option value="63">(GMT+09:00)<script>document.write(_("Seoul"));</script></option>  
						<option value="64">(GMT+09:30)<script>document.write(_("Darwin"));</script></option>                                      
						<option value="65">(GMT+09:30)<script>document.write(_("Adelaide"));</script></option>                                    
						<option value="66">(GMT+10:00)<script>document.write(_("Brisbane"));</script></option>                                    
						<option value="67">(GMT+10:00)<script>document.write(_("Canberra, Melbourne Sydney"));</script></option>                      
						<option value="68">(GMT+10:00)<script>document.write(_("Guam, Port Moresby"));</script></option>
						<option value="69">(GMT+10:00)<script>document.write(_("Vladivostok"));</script></option>                              
						<option value="70">(GMT+10:00)<script>document.write(_("Hobart"));</script></option>  
						<option value="71">(GMT+11:00)<script>document.write(_("Magadan, Solomon Islands, New Caledonia and more"));</script></option>
						<option value="72">(GMT+12:00)<script>document.write(_("Fiji Islands, Kamchatka Peninsula, the Marshall Islands"));</script></option>          
						<option value="73">(GMT+12:00)<script>document.write(_("Auckland , Wellington"));</script></option>
						<option value="74">(GMT+13:00)<script>document.write(_("Nuku'alofa"));</script></option>                                
					</select>
					</td></tr>
					</table>
				<table cellpadding="0" cellspacing="0" class="content3">
					<tr><td height="30">&nbsp;&nbsp;(<script>document.write(_("hostname_Note"));</script>)</td>
					</tr>
				</table>
				<table cellpadding="0" cellspacing="0" class="content3" id="table2">
					<tr><td height="30">&nbsp;&nbsp;<script>document.write(_("Customized time"));</script>:
					  <input type="checkbox" id="manualEN" onClick="onManualSet()">
					<tr><td height="30" id="setTab">
						&nbsp;&nbsp;<input type="text" class="text" id="year" size="4" maxlength="4"><script>document.write(_("Year"));</script>
						<input type="text" class="text" id="month" size="2" maxlength="2"><script>document.write(_("Month"));</script>
						<input type="text" class="text" id="day" size="2" maxlength="2"><script>document.write(_("Date"));</script>
						<input type="text" class="text" id="hour" size="2" maxlength="2"><script>document.write(_("Hour"));</script>
						<input type="text" class="text" id="minute" size="2" maxlength="2"><script>document.write(_("Minute"));</script>
						<input type="text" class="text" id="second" size="2" maxlength="2"><script>document.write(_("Second"));</script>
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
		<script>helpInfo(_("system_hostname_Help_Inf1")+"<br>&nbsp;&nbsp;&nbsp;&nbsp;"+_("system_hostname_Help_Inf2"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
	  table_onload('table2');
    </script>
</BODY>
</HTML>






