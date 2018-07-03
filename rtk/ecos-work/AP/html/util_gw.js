/*==============================================================================*/
/*   wlbasic.htm and wizard-wlan1.htm  tcpiplan.htm*/

// for WPS ---------------------------------------------------->>
var wps_warn1=util_gw_wps_warn1 +
				util_gw_wps_cause_disconn  + 
				util_gw_wps_want_to;
var wps_warn2=util_gw_wps_warn2 +
				util_gw_wps_cause_disconn + 
				util_gw_wps_want_to;
var wps_warn3=util_gw_wps_warn3+
				util_gw_wps_cause_disconn+ 
				util_gw_wps_want_to;
var wps_warn4=util_gw_wps_warn4+
				util_gw_wps_cause_disabled  + 
				util_gw_wps_want_to;
var wps_warn5=util_gw_wps_warn5 +
				util_gw_wps_cause_disabled  + 
				util_gw_wps_want_to;
var wps_warn6=util_gw_wps_warn6 +
				util_gw_wps_cause_disabled  + 
				util_gw_wps_want_to;
var wps_warn7=util_gw_wps_warn7 +
				util_gw_wps_cause_disabled  + 
				util_gw_wps_want_to;
var encrypt_11n = util_gw_wps_ecrypt_11n;
var encrypt_basic = util_gw_wps_ecrypt_basic;
var encrypt_confirm= util_gw_wps_ecrypt_confirm;

var wps_wep_key_old;

function check_wps_enc(enc, radius, auth)
{
	if (enc == 0 || enc == 1) {
		if (radius != 0)
			return 2;
	}		
	else {
		if (auth & 1)
			return 2;
	}
	return 0;
}

function check_wps_wlanmode(mo, type)
{
	if (mo == 2) {
		return 1;
	}
	if (mo == 1 && type != 0) {
		return 1;
	}
	return 0;
}
//<<----------------------------------------------- for WPS

function skip () { this.blur(); }
function disableTextField (field) {
  if (document.all || document.getElementById)
    field.disabled = true;
  else {
    field.oldOnFocus = field.onfocus;
    field.onfocus = skip;
  }
}

function enableTextField (field) {
  if (document.all || document.getElementById)
    field.disabled = false;
  else {
    field.onfocus = field.oldOnFocus;
  }
}

function verifyBrowser() {
	var ms = navigator.appVersion.indexOf("MSIE");
	ie4 = (ms>0) && (parseInt(navigator.appVersion.substring(ms+5, ms+6)) >= 4);
	var ns = navigator.appName.indexOf("Netscape");
	ns= (ns>=0) && (parseInt(navigator.appVersion.substring(0,1))>=4);
	if (ie4)
		return "ie4";
	else
		if(ns)
			return "ns";
		else
			return false;
}

function saveChanges_basic(form, wlan_id)
{
	
	
  var mode =form.elements["mode"+wlan_id] ;

	if (form.name=="wlanSetup") {
		// for support WPS2DOTX  ; ap mode
		hiddenSSIDEnabled = form.elements["hiddenSSID"+wlan_id] ;

		if(mode.selectedIndex==0 || mode.selectedIndex==3){
			if ( hiddenSSIDEnabled.selectedIndex==0 ) {
				if(!confirm(util_gw_ssid_hidden_alert)){
					return false;
				}
			}
		}
	}

  var ssid =form.elements["ssid"+wlan_id] ;			//mode.selectedIndex=4 means AP+MESH
  // P2P_SUPPORT
  if ( (mode.selectedIndex==0 || mode.selectedIndex==3 ) && ssid.value=="") {
	alert(util_gw_ssid_empty);

	ssid.value = ssid.defaultValue;
	ssid.focus();
	return false;
   }

   if (!form.elements["wlanDisabled"+wlan_id].checked) {
	var idx_value= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[idx_value].value;
	var band = parseInt(band_value, 10) + 1;

	var wlBandMode =form.elements["wlBandMode"].value ;
		
	if(wlBandMode == 3) // 3:BANDMODESIGNLE
	{
		var selectText=form.elements["band"+wlan_id].options[idx_value].text.substr(0,1);
		var bandOption = form.elements["band"+wlan_id].options.value;
		
		//if(selectText=='2') //match '2'
		if(bandOption == 0 || bandOption == 1 || (bandOption == 7 && selectText=='2') || bandOption == 2 || bandOption == 9 || bandOption == 10 || bandOption == 74)
			form.elements["Band2G5GSupport"].value = 1;//1:PHYBAND_2G
		else if(bandOption == 3 || (bandOption == 7 && selectText=='5') || bandOption == 11)
			form.elements["Band2G5GSupport"].value = 2;//2:PHYBAND_5G										
	}

	var basicRate=0;
	var operRate=0;
	if (band & 1) {
		basicRate|=0xf;
		operRate|=0xf;		
	}
	if ( (band & 2) || (band & 4) ) {
		operRate|=0xff0;
		if (!(band & 1)) {
			if (WiFiTest)
				basicRate=0x15f;
			else
				basicRate=0x1f0;
		}			
	}
	if (band & 8) {
		if (!(band & 3))
			operRate|=0xfff;	
		if (band & 1)
			basicRate=0xf;
		else if (band & 2)			
			basicRate=0x1f0;
		else
			basicRate=0xf;
	}
	
	operRate|=basicRate;
	if (band && band != usedBand[wlan_id]) {
		form.elements["basicrates"+wlan_id].value = basicRate;
		form.elements["operrates"+wlan_id].value = operRate;
	}
	else {
		form.elements["basicrates"+wlan_id].value = 0;
		form.elements["operrates"+wlan_id].value = 0;
	}
   }

   return true;
}
/*==============================================================================*/
function show_div(show,id) 
{
	var field=document.getElementById(id);
	if(!field) return;
	if(show)
		field.className  = "on" ;
    else	    
    	field.className  = "off" ;
}

/*   tcpipwan.htm */
/*-- keith: add l2tp support. 20080515  */
function wanShowDiv(pptp_bool, dns_bool, dnsMode_bool, pppoe_bool, static_bool, l2tp_bool)
{
 	show_div(pptp_bool,"pptp_div");
  	show_div(dns_bool,"dns_div");
  	show_div(dnsMode_bool,"dnsMode_div");
  	show_div(pppoe_bool,"pppoe_div");
  	show_div(static_bool,"static_div"); 
	show_div(l2tp_bool,"l2tp_div"); /*-- keith: add l2tp support. 20080515  */

  	if (pptp_bool==0 && pppoe_bool==0 && static_bool==0 && dns_bool && l2tp_bool==0 ) /*-- keith: add l2tp support. 20080515  */
  	  	show_div(1,"dhcp_div");  	
  	else
  		show_div(0,"dhcp_div");  
}

/*==============================================================================*/
/*   wlbasic.htm */
function enableWLAN(form, wlan_id)
{
	var idx_value= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[idx_value].value;
	var chan_boundIdx = form.elements["channelbound"+wlan_id].selectedIndex;	
	var mode_idx = form.elements["mode"+wlan_id].selectedIndex; 
	var mode_value =form.elements["mode"+wlan_id].options[mode_idx].value; 	
	
	if(form.elements["multipleAP"+wlan_id] != null) { // for multiple ap
		if (mode_value == 0 || mode_value == 3)
			enableButton(form.elements["multipleAP"+wlan_id]);
		else
			disableButton(form.elements["multipleAP"+wlan_id]);
	}
	
  if (mode_value !=1) {	//mode != client
  	disableTextField(form.elements["type"+wlan_id]); //network type
  	if(form.elements["showMac"+wlan_id]!= null) {
		// mode ==AP or AP+WDS or MPP+AP or MAP
  		if (mode_value ==0 || mode_value ==3 || mode_value ==4 || mode_value ==6){	
  			enableButton(form.elements["showMac"+wlan_id]);
			
			// plus note, just AP or AP+WDS need Multi-AP,under MPP+AP or MAP mode disable multi-AP
			if (mode_value ==0 || mode_value ==3)	
			if(form ==document.wlanSetup){  	
				if(form.elements["multipleAP"+wlan_id] != null)
					enableButton(form.elements["multipleAP"+wlan_id]);
			}		
  		}else{
  			disableButton(form.elements["showMac"+wlan_id]);
  			if(form ==document.wlanSetup){  	
				if(form.elements["multipleAP"+wlan_id] != null)
					disableButton(form.elements["multipleAP"+wlan_id]);
			}	
  		}
  	}
  	enableTextField(form.elements["chan"+wlan_id]);
  }
  else {	// mode == client
    	if (disableSSID[wlan_id])
  		disableTextField(form.elements["type"+wlan_id]);
  	else
   		enableTextField(form.elements["type"+wlan_id]);   	   	
    	
   	if(form.elements["showMac"+wlan_id] != null)
		disableButton(form.elements["showMac"+wlan_id]);
	if(form ==document.wlanSetup){  	
		if(form.elements["multipleAP"+wlan_id] != null)
			disableButton(form.elements["multipleAP"+wlan_id]);
	}	
	if (form.elements["type"+wlan_id].selectedIndex==0) {
		disableTextField(form.elements["chan"+wlan_id]);
	}
	else {
		enableTextField(form.elements["chan"+wlan_id]);
	}

  }
  if (disableSSID[wlan_id]){
	disableTextField(form.elements["ssid"+wlan_id]);
 	disableTextField(form.elements["mode"+wlan_id]);  	
  }
  else {
  	if (mode_value !=2)
  		enableTextField(form.elements["ssid"+wlan_id]);
  	else
  		disableTextField(form.elements["ssid"+wlan_id]);
  		
  	enableTextField(form.elements["mode"+wlan_id]); 
  }  
  enableTextField(form.elements["band"+wlan_id]);

  if(form.elements["mode"+wlan_id].selectedIndex == 1 && opmode != 2) // client mode but not wisp
  	enableCheckBox(form.elements["wlanMacClone"+wlan_id]);
  else
  	disableCheckBox(form.elements["wlanMacClone"+wlan_id]);
  	
	if(band_value == 9 || band_value ==10 || band_value==7 || band_value==11 || band_value==14 || band_value == 74){
	  	enableTextField(form.elements["channelbound"+wlan_id]);
	  
	  	
	  	if(chan_boundIdx == 1)
	  		enableTextField(form.elements["controlsideband"+wlan_id]);
	  	else 	
	  		 disableTextField(form.elements["controlsideband"+wlan_id]);
	 }
	if(form ==document.wlanSetup){  	
		enableTextField(form.elements["txRate"+wlan_id]);	
  		enableTextField(form.elements["hiddenSSID"+wlan_id]);	
	}
}
function band_11N_enable(value)
{
	if(value<0||value>127)
		return false;
	
	if(value==7||value==9||value==10||value==11||value==71||value==75)
		return true;
	else
		return false;
}
function disableWLAN(form, wlan_id)
{
  disableTextField(form.elements["mode"+wlan_id]);
  disableTextField(form.elements["band"+wlan_id]);
  disableTextField(form.elements["type"+wlan_id]); 
  disableTextField(form.elements["ssid"+wlan_id]);
  disableTextField(form.elements["chan"+wlan_id]);
  disableTextField(form.elements["channelbound"+wlan_id]);
  disableTextField(form.elements["controlsideband"+wlan_id]);
if(form == document.wlanSetup){  
  disableTextField(form.elements["hiddenSSID"+wlan_id]);
  disableTextField(form.elements["txRate"+wlan_id]);
  disableButton(form.elements["multipleAP"+wlan_id]);
}  
  disableCheckBox(form.elements["wlanMacClone"+wlan_id]);

  if(form.elements["showMac"+wlan_id]!= null)
  	disableButton(form.elements["showMac"+wlan_id]);
}
function updateIputState(form, wlan_id)
{
  if (form.elements["wlanDisabled"+wlan_id].checked)
 	disableWLAN(form, wlan_id);
  else
  	enableWLAN(form, wlan_id);
}

function disableButton (button) {
  //if (verifyBrowser() == "ns")
  //	return;
  if (document.all || document.getElementById)
    button.disabled = true;
  else if (button) {
    button.oldOnClick = button.onclick;
    button.onclick = null;
    button.oldValue = button.value;
    button.value = 'DISABLED';
  }
}

function enableButton (button) {
  //if (verifyBrowser() == "ns")
  //	return;
  if (document.all || document.getElementById)
    button.disabled = false;
  else if (button) {
    button.onclick = button.oldOnClick;
    button.value = button.oldValue;
  }
}

function showChannel5G(form, wlan_id)
{
	var sideBand=form.elements["controlsideband"+wlan_id].value;
	var dsf_enable=form.elements["dsf_enable"].value;
	var idx=0;
	var wlan_support_8812e=form.elements["wlan_support_8812e"].value;
	var defChanIdx;
	form.elements["chan"+wlan_id].length=startChanIdx[wlan_id];
	
	if (startChanIdx[wlan_id] == 0)
		defChanIdx=0;
	else
		defChanIdx=1;

	if (startChanIdx[wlan_id]==0) {
		if(dsf_enable == 1)		
			form.elements["chan"+wlan_id].options[0] = new Option(uyi_gw_chan_dfsauto, 0, false, false);
		else
			form.elements["chan"+wlan_id].options[0] = new Option(util_gw_chanauto, 0, false, false);
			
		if (0 == defaultChan[wlan_id]) {
			form.elements["chan"+wlan_id].selectedIndex = 0;
			defChanIdx = 0;
		}
		startChanIdx[wlan_id]++;		
	}
	
	idx=startChanIdx[wlan_id];
	

	if(wlan_support_8812e ==1)
	{
		var bound = form.elements["channelbound"+wlan_id].selectedIndex;
		var inc_scale;
		var chan;
		inc_scale = 4;
		var chan_str = 36;
		var chan_end = 64;

		var reg_chan_8812_full =new Array(16);
		var i;
		var ii;
		var iii;
		var iiii;
		var found; 
		var chan_pair;
		var reg_8812 = regDomain[wlan_id];
		
/* FCC */			reg_chan_8812_full[0]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","136","140","149","153","157","161","165");
/* IC */				reg_chan_8812_full[1]= new Array("36","40","44","48","52","56","60","64","149","153","157","161");
/* ETSI */			reg_chan_8812_full[2]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","120","124","128","132","136","140");                                         
/* SPAIN */			reg_chan_8812_full[3]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","120","124","128","132","136","140");                                         
/* FRANCE */			reg_chan_8812_full[4]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","120","124","128","132","136","140");                                          
/* MKK */			reg_chan_8812_full[5]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","120","124","128","132","136","140");                                          
/* ISRAEL */			reg_chan_8812_full[6]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","120","124","128","132","136","140");                                           
/* MKK1 */			reg_chan_8812_full[7]= new Array("34","38","42","46");                                                                                                  
/* MKK2 */			reg_chan_8812_full[8]= new Array("36","40","44","48");                                                                                               
/* MKK3 */			reg_chan_8812_full[9]= new Array("36","40","44","48","52","56","60","64");                                                                                        
/* NCC (Taiwan) */	reg_chan_8812_full[10]= new Array("56","60","64","100","104","108","112","116","136","140","149","153","157","161","165");                                                                                                                                                      
/* RUSSIAN */		reg_chan_8812_full[11]= new Array("36","40","44","48","52","56","60","64","132","136","140","149","153","157","161","165");
/* CN */				reg_chan_8812_full[12]= new Array("149","153","157","161","165");                                                                                           
/* Global */			reg_chan_8812_full[13]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","136","140","149","153","157","161","165");                                    
/* World_wide */		reg_chan_8812_full[14]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","136","140","149","153","157","161","165");                                       
/* Test */			reg_chan_8812_full[15]= new Array("36","40","44","48","52","56","60","64","100","104","108","112","116","120","124","128"," 132","136","140","144","149","153","157","161","165","169","173","177");		

		if(reg_8812 > 0)
			reg_8812 = reg_8812 - 1;
		if(reg_8812 > 15)
			reg_8812 = 15;

		if(reg_8812==7) //MKK1 are special case
		{
			if(bound <= 2)
			for(i = 0; i < reg_chan_8812_full[reg_8812].length; i++) {
				chan = reg_chan_8812_full[reg_8812][i];
			
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if (chan == defaultChan[wlan_id]) {
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx=idx;
			}
			idx ++;
		}
		}
		else
		{
			for(i = 0; i < reg_chan_8812_full[reg_8812].length; i++) {
				chan = reg_chan_8812_full[reg_8812][i];

				if(reg_8812 != 15 && reg_8812 != 10)
				if((dsf_enable == 0) && (chan >= 52) && (chan <= 144))
					continue;

				if( reg_8812 == 10)
					if((dsf_enable == 0) && (chan >= 100) && (chan <= 140))
						continue;

				if(reg_8812 != 15)
				if(bound==1)
				{
					for(ii=0; ii < reg_chan_8812_full[15].length; ii++)
					{
						if(chan == reg_chan_8812_full[15][ii])
							break;
					}
					
					if(ii%2 == 0)
						chan_pair = reg_chan_8812_full[15][ii+1];
					else
						chan_pair = reg_chan_8812_full[15][ii-1];

					found = 0;
					for(ii=0; ii < reg_chan_8812_full[reg_8812].length; ii++)
					{
						if(chan_pair == reg_chan_8812_full[reg_8812][ii])
						{
							found = 1;
							break;
						}
					}

					if(found == 0)
						chan = 0;
		
				}
				else if(bound==2)
				{
					for(ii=0; ii < reg_chan_8812_full[15].length; ii++)
					{
						if(chan == reg_chan_8812_full[15][ii])
							break;
					}

					for(iii=(ii-(ii%4)); iii<((ii-(ii%4)+3)) ; iii++)
					{
						found = 0;
						chan_pair = reg_chan_8812_full[15][iii];
			
						for(iiii=0; iiii < reg_chan_8812_full[reg_8812].length; iiii++)
						{
							if(chan_pair == reg_chan_8812_full[reg_8812][iiii])
							{
								found=1;
								break;
							}
			}

						if(found == 0)
						{
							chan = 0;
							break;
		}

					}

				}
			
				if(chan != 0)
				{
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if (chan == defaultChan[wlan_id]) {
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx=idx;
			}
					idx++;
		}

			}
		}
		
	}
	else
	if (regDomain[wlan_id]==1 //FCC (1)
	 || regDomain[wlan_id]==2 //IC (2)
	 || regDomain[wlan_id]==3 //ETSI (3)
	 || regDomain[wlan_id]==4 //SPAIN (4)
	 || regDomain[wlan_id]==5 //FRANCE (5)
	 || regDomain[wlan_id]==6 //MKK (6)
	 || regDomain[wlan_id]==7 //ISREAL (7)
	 || regDomain[wlan_id]==9 //MKK2 (9)
	 || regDomain[wlan_id]==10 //MKK3 (10)
	) 
	{
		var bound = form.elements["channelbound"+wlan_id].selectedIndex;
		var inc_scale;
		
		if (bound == 0) //20MHz
		{
			inc_scale = 4;
			chan_str = 36;
			chan_end = 48;
		}
		else if(bound == 1)//40MHz //8812
		{ 
			inc_scale = 8;
			if(sideBand == 0) // upper
			{
				chan_str = 40;
				chan_end = 48;
			}
			else // lower
			{
				chan_str = 36;
				chan_end = 44;
			}
		}
		else if(bound == 2)
		{
			inc_scale = 4;
			chan_str = 36;
			chan_end = 64;
		}
		
		for (chan=chan_str; chan<=chan_end; idx++, chan+=inc_scale) {
			
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if (chan == defaultChan[wlan_id]) {
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx=idx;
			}
		}
		
		if(bound != 2)
		{
		if (regDomain[wlan_id]==1 //FCC (1)
	 		|| regDomain[wlan_id]==2 //IC (2)
	 	)
	 	{
	 		if (bound == 0) {		//20MHz
					inc_scale = 4;
					chan_str = 149;
					chan_end = 161;
				}	
				else {
				inc_scale = 8;
				if(sideBand == 0) // upper
				{
					chan_str = 153;
					chan_end = 161;
				}
				else // lower
				{
					chan_str = 149;
					chan_end = 157;
				}
			}					
			for (chan=chan_str; chan<=chan_end; idx++, chan+=inc_scale) {
				form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
				if (chan == defaultChan[wlan_id]) {
					form.elements["chan"+wlan_id].selectedIndex = idx;
					defChanIdx=idx;
				}
			}	 			 		
	 	}
		}
	 	
			/* You can not select channel 165 in 40MHz whether upper or lower. */
	}
	else if (regDomain[wlan_id]==8) //MKK1 (8)
	{
		var bound = form.elements["channelbound"+wlan_id].selectedIndex;
		var inc_scale;
		
		if (bound == 0) //20MHz
		{
			inc_scale = 4;
			chan_str = 34;
			chan_end = 46;
		}	
		else if(bound == 1) //40MHz //8812
		{ 
			inc_scale = 8;
			if(sideBand == 0) // upper
			{
				chan_str = 38;
				chan_end = 46;
			}
			else // lower
			{
				chan_str = 34;
				chan_end = 42;
			}
		}
		else if(bound == 2)
		{
			inc_scale = 4;
			chan_str = 36;
			chan_end = 64;
		}
		
		for (chan=chan_str; chan<=chan_end; idx++, chan+=inc_scale) {
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if (chan == defaultChan[wlan_id]) {
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx=idx;
			}
		}		
	}
	else if (regDomain[wlan_id]==11) //NCC (11)
	{
		var bound = form.elements["channelbound"+wlan_id].selectedIndex;
		var inc_scale;
		if (bound == 0) //20MHz
		{
			inc_scale = 4;
			chan_str = 56;
			chan_end = 64;
		}	
		else if(bound == 1)//40MHz  //8812 
		{ 
			inc_scale = 8;
			if(sideBand == 0) // upper
			{
				chan_str = 60;
				chan_end = 64;
			}
			else // lower
			{
				chan_str = 56;
				chan_end = 64;
			}
		}				
		else if(bound == 2)
		{
			inc_scale = 4;
			chan_str = 36;
			chan_end = 64;
		}
			
		for (chan=chan_str; chan<=chan_end; idx++, chan+=inc_scale) {					
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if (chan == defaultChan[wlan_id]) {
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx=idx;
			}
		}
		
		if(bound !=2)
		{
		if (bound == 0) //20MHz
		{
			inc_scale = 4;
			chan_str = 149;
			chan_end = 165;
		}	
		else //40MHz
		{ 
			inc_scale = 8;
			if(sideBand == 0) // upper
			{
				chan_str = 153;
				chan_end = 161;
			}
			else // lower
			{
				chan_str = 149;
				chan_end = 157; /* You can not select channel 165 in 40MHz whether upper or lower. */
			}
		}				
			
		for (chan=chan_str; chan<=chan_end; idx++, chan+=inc_scale) {					
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if (chan == defaultChan[wlan_id]) {
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx=idx;
			}
		}
	}
	}
		
	form.elements["chan"+wlan_id].length = idx;
	if (defChanIdx==0)
		form.elements["chan"+wlan_id].selectedIndex = 0;
	if(form.elements["support_8881a_selective"].value!=0)
	{
		var chan_val;
		var chan_idx;
		var need_swap= form.elements["need_swap"].value;
	
		if (need_swap == 1)
			chan_val=form.elements["channel_2"].value;
		else
			chan_val=form.elements["channel_1"].value;

		for (chan_idx = 0; chan_idx < form.elements["chan"+wlan_id].options.length; chan_idx++) {        
			if (form.elements["chan"+wlan_id].options[chan_idx].value == chan_val) { 
		            break;        
			}        
		}

		if (need_swap == 1)
			form.elements["chan"+wlan_id].selectedIndex = chan_idx;
		else
			form.elements["chan"+wlan_id].selectedIndex = chan_idx;
	}
}


function showChannel2G(form, wlan_id, bound_40, band_value)
{
	var start = 1;
	var end = 14;
	if (regDomain[wlan_id]==1 || regDomain[wlan_id]==2) {
		start = 1;
		end = 11;
	}
	if (regDomain[wlan_id]==3 || regDomain[wlan_id]==12) {
		start = 1;
		end = 13;
	}
	if (regDomain[wlan_id]==4) {
		start = 10;
		end = 11;
	}
	if (regDomain[wlan_id]==5) {
		start = 10;
		end = 13;
	}
	if (regDomain[wlan_id]==6) {
		start = 1;
		end = 14;
	}
	if (regDomain[wlan_id]==7) {
		start = 3;
		end = 13;
	}
	if (regDomain[wlan_id]==11||regDomain[wlan_id]==13) {
		start = 1;
		end = 11;
	}



	if(band_value == 9 || band_value == 10 || band_value==7 || band_value==74){
		if(bound_40 ==1){
			var sideBand_idex = form.elements["controlsideband"+wlan_id].selectedIndex;
			var sideBand=form.elements["controlsideband"+wlan_id].options[sideBand_idex].value;
			if(regDomain[wlan_id]==4){
				if(sideBand ==0){  //upper
					start = 11;
					end = 11;
				}else if(sideBand ==1){ //lower
					start = 10;
					end = 10;
				}
			}else if(regDomain[wlan_id]==5){
				if(sideBand ==0){  //upper
					start = 13;
					end = 13;
				}else if(sideBand ==1){ //lower
					start = 10;
					end = 10;
				}
			}else{
				if(sideBand ==0){  //upper
					start = 5;
					if (regDomain[wlan_id]==1 || regDomain[wlan_id]==2 || regDomain[wlan_id]==11 || regDomain[wlan_id]==13 )
						end = 11;
					else  				
						end = 13;			
					
				}else if(sideBand ==1){ //lower
					end = 9;
					//end = 7; orig
					if(regDomain[wlan_id]==7)
						start = 3;
					else 
						start = 1;
				}
			}
		}
	}
	defChanIdx=0;
	form.elements["chan"+wlan_id].length=0;

	idx=0;
	form.elements["chan"+wlan_id].options[0] = new Option(util_gw_chanauto, 0, false, false);
	
	if(wlan_channel[wlan_id] ==0){
		form.elements["chan"+wlan_id].selectedIndex = 0;
		defChanIdx = 0;
	}

	idx++;	
	if(band_value == 74 && form.elements["channelbound"+wlan_id].selectedIndex == "2")
	{
		start = 1;
		end = 13;
		for (chan=start; chan<=end; idx++) {
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if(chan == wlan_channel[wlan_id]){
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx = idx;
			}
			chan=chan+4;
		}
	}
	else{
		for (chan=start; chan<=end; chan++, idx++) {
			form.elements["chan"+wlan_id].options[idx] = new Option(chan, chan, false, false);
			if(chan == wlan_channel[wlan_id]){
				form.elements["chan"+wlan_id].selectedIndex = idx;
				defChanIdx = idx;
			}
		}
	}
	form.elements["chan"+wlan_id].length=idx;
	startChanIdx[wlan_id] = idx;
	if(form.elements["support_8881a_selective"].value!=0)
	{
		var chan_val;
		var chan_idx;
		var need_swap= form.elements["need_swap"].value;
	
		if (need_swap == 1)
			chan_val=form.elements["channel_2"].value;
		else
			chan_val=form.elements["channel_1"].value;

	    for (chan_idx = 0; chan_idx < form.elements["chan"+wlan_id].options.length; chan_idx++) {        
	        if (form.elements["chan"+wlan_id].options[chan_idx].value == chan_val) { 
	            break;        
	        }        
	    }

		if (need_swap == 1)
			form.elements["chan"+wlan_id].selectedIndex = chan_idx;
		//else
			//form.elements["chan"+wlan_id].selectedIndex = chan_idx;
    }
}
function updateChan_channebound(form, wlan_id)
{
	var idx_value= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[idx_value].value;
	var bound = form.elements["channelbound"+wlan_id].selectedIndex;
	var adjust_chan;
	var Band2G5GSupport=form.elements["Band2G5GSupport"].value;
	
if(form.name == "wizard")
	{
		switch(wlan_id)
		{
			case 0:
				if(form.elements["wlan1_phyband"].value == "5GHz")
					Band2G5GSupport = 2;
				else
					Band2G5GSupport = 1;
				break;
				
			case 1:
				if(form.elements["wlan2_phyband"].value == "5GHz")
					Band2G5GSupport = 2;
				else
					Band2G5GSupport = 1;
				break;
			
		}
		
	}
var currentBand;		

	if(band_value ==3 || band_value ==11){
		currentBand = 2;
	}
	else if(band_value ==0 || band_value ==1 || band_value ==2 || band_value == 9 || band_value ==10 || band_value ==74){
		currentBand = 1;
	}
	else if(band_value == 4 || band_value==5 || band_value==6 || band_value==14){
		currentBand = 3;
	}
	else if(band_value == 7) //7:n
	{
		if(idx_value != 1)
			currentBand =1;
		else
			currentBand =2;
	}
	if(band_value==9 || band_value==10 || band_value ==7 || band_value ==74){	
		if(bound ==0)
			adjust_chan=0;
		if(bound ==1)
			adjust_chan=1;	
	}else
		adjust_chan=0;	  
    

	if (currentBand == 3) {
		showChannel2G(form, wlan_id, adjust_chan, band_value);
		showChannel5G(form, wlan_id);
	}
  
	if (currentBand == 2) {
		startChanIdx[wlan_id]=0;
		showChannel5G(form, wlan_id);
	}
	
  	if (currentBand == 1)
		showChannel2G(form, wlan_id, adjust_chan, band_value);
 	
 	/*if(band_value==9 || band_value==10 || band_value ==7 || band_value ==11 || band_value ==14 || band_value ==74){
	  	if(form.elements["chan"+wlan_id].value == 0){ // 0:auto	  
	  		disableTextField(form.elements["controlsideband"+wlan_id]);	
		}
	}*/
}
function isBand2G(band_value,Band2G5GSupport,idx_value)
{
	if(band_value==0
	||band_value==1
	||band_value==2
	||(band_value==7 && (Band2G5GSupport==1||Band2G5GSupport==3 && idx_value>5))
	||band_value==8
	||band_value==9
	||band_value==10
	||band_value ==74
	)
		return true;
	else
		return false;
}
function isBand5G(band_value,Band2G5GSupport,idx_value)
{
	if(band_value==3
	||(band_value==7 && (Band2G5GSupport==2||Band2G5GSupport==3 && idx_value<=5))
	||band_value==11
	||band_value==63
	||band_value==67
	||band_value==71
	||band_value==75
	)
		return true;
	else
		return false;
}
function updateChan(form, wlan_id)
{
	var idx_value= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[idx_value].value;
	var Band2G5GSupport=form.elements["Band2G5GSupport"].value;
	
	if(form.name == "wizard")
	{
		switch(wlan_id)
		{
			case 0:
				if(form.elements["wlan1_phyband"].value == "5GHz")
					Band2G5GSupport = 2;
				else
					Band2G5GSupport = 1;
				break;
				
			case 1:
				if(form.elements["wlan2_phyband"].value == "5GHz")
					Band2G5GSupport = 2;
				else
					Band2G5GSupport = 1;
				break;
			
		}
		
	}	

	if(isBand5G(band_value,Band2G5GSupport,idx_value)){ // 3:5g_a 11:5g_an 7:n 2:PHYBAND_5G
		currentBand = 2;
	}
	else if(isBand2G(band_value,Band2G5GSupport,idx_value)){
		currentBand = 1;
	}else if(band_value == 4 || band_value==5 || band_value==6 || band_value==14){
		currentBand = 3;
	}


  if ((lastBand[wlan_id] != currentBand) || (lastRegDomain[wlan_id] != regDomain[wlan_id])) {
  	lastBand[wlan_id] = currentBand;
	lastRegDomain[wlan_id] = regDomain[wlan_id];
	if (currentBand == 3) {
		showChannel2G(form, wlan_id, 0, band_value);
		showChannel5G(form, wlan_id);
	}
	
  if (currentBand == 2) {
		startChanIdx[wlan_id]=0;
		showChannel5G(form, wlan_id);
	}
	
  	if (currentBand == 1)
		showChannel2G(form, wlan_id, 0, band_value);
  }

  	if(band_value==9 || band_value==10 || band_value ==7 || band_value ==11 || band_value ==14 || band_value ==74){
	  	//if(form.elements["chan"+wlan_id].value ==0)
	  	{ // 0:auto
	  		disableTextField(form.elements["controlsideband"+wlan_id]);	
		}
	}
}

function showBand_MultipleAP(form, wlan_id, band_root, index_id)
{
  var idx=0;
  var band_value=bandIdx[wlan_id];

  if(band_root ==0){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (B)", "0", false, false);
}else if(band_root ==1){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (G)", "1", false, false);
}else if(band_root ==2){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (B)", "0", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (G)", "1", false, false);	
 	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (B+G)", "2", false, false);
}else if(band_root ==9){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (G)", "1", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (N)", "7", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (G+N)", "9", false, false);
}else if(band_root ==10){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (B)", "0", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (G)", "1", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (N)", "7", false, false);
 	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (B+G)", "2", false, false);
 	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (G+N)", "9", false, false);
 	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (B+G+N)", "10", false, false);
	if(wlan_support_ac2g)
		form.elements["band"+index_id].options[idx++] = new Option("2.4 GHz (B+G+N+AC)", "74", false, false);
}else if(band_root ==3){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A)", "3", false, false);
}else if(band_root ==7){
	if(form.elements["Band2G5GSupport"].value==2)
		form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (N)", "7", false, false);
	else
		form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("2.4 GHz (N)", "7", false, false);

}else if(band_root ==11){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A)", "3", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (N)", "7", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A+N)", "11", false, false);
}else if(band_root ==63){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (AC)", "63", false, false);	
}else if(band_root ==67){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A)", "3", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (AC)", "63", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A+AC)", "67", false, false);	
}else if(band_root ==71){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (N)", "7", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (AC)", "63", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (N+AC)", "71", false, false);	
}else if(band_root ==75){
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A)", "3", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (N)", "7", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (AC)", "63", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A+N)", "11", false, false);
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A+AC)", "67", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (N+AC)", "71", false, false);	
	form.elements["wl_band_ssid"+index_id].options[idx++] = new Option("5 GHz (A+N+AC)", "75", false, false);	
}


form.elements["wl_band_ssid"+index_id].selectedIndex = 0;
 form.elements["wl_band_ssid"+index_id].length = idx;
}


function showBandAP(form, wlan_id)
{
  var idx=0;
  var band_value=bandIdx[wlan_id];
	var Band2G5GSupport=form.elements["Band2G5GSupport"].value;
	var wlBandMode=form.elements["wlBandMode"].value;
	var i;

if(form.name == "wizard")
{
	switch(wlan_id)
	{
		case 0:
			if(form.elements["wlan1_phyband"].value == "5GHz")
				Band2G5GSupport = 2;
			else
				Band2G5GSupport = 1;
			break;
	
		case 1:
			if(form.elements["wlan2_phyband"].value == "5GHz")
				Band2G5GSupport = 2;
			else
				Band2G5GSupport = 1;
			break;
		
	}

}
	
	if(Band2G5GSupport == 2 || wlBandMode == 3) // 2:PHYBAND_5G 3:BANDMODESIGNLE
	{
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (A)", "3", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (N)", "7", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (A+N)", "11", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (AC)", "63", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (N+AC)", "71", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (A+N+AC)", "75", false, false);

	}
	
	if(Band2G5GSupport == 1 || wlBandMode == 3) // 1:PHYBAND_2G 3:BANDMODESIGNLE
	{
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B)", "0", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (G)", "1", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (N)", "7", false, false); 
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B+G)", "2", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (G+N)", "9", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B+G+N)", "10", false, false);
		if(wlan_support_ac2g)
			form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B+G+N+AC)", "74", false, false);

	}


	for(i=0 ; i<idx ; i++)
	{
		if(form.elements["band"+wlan_id].options[i].value == band_value)
		{			
			if(band_value == 7 && wlBandMode == 3)// 2g and 5g has the same band value in N.
			{
				var selectText=form.elements["band"+wlan_id].options[i].text.substr(0,1);
				
				if( (Band2G5GSupport == 2 && selectText == '5') //2:PHYBAND_5G
				||	(Band2G5GSupport == 1 && selectText == '2') //1:PHYBAND_2G
				) 
				{
					form.elements["band"+wlan_id].selectedIndex = i;
					break;					
				}			
			}
			else
			{	
				form.elements["band"+wlan_id].selectedIndex = i;
				break;
			}
		}				
	}	

 form.elements["band"+wlan_id].length = idx;
}
        
     
function showBandClient(form, wlan_id)
{
  var idx=0;
   var band_value=bandIdx[wlan_id];
var Band2G5GSupport=form.elements["Band2G5GSupport"].value;
	var wlBandMode=form.elements["wlBandMode"].value;
	var i;

if(form.name == "wizard")
	{
		switch(wlan_id)
		{
			case 0:
				if(form.elements["wlan1_phyband"].value == "5GHz")
					Band2G5GSupport = 2;
				else
					Band2G5GSupport = 1;
				break;
				
			case 1:
				if(form.elements["wlan2_phyband"].value == "5GHz")
					Band2G5GSupport = 2;
				else
					Band2G5GSupport = 1;
				break;
			
		}
		
	}

	
	if(Band2G5GSupport == 2 || wlBandMode == 3) // 2:PHYBAND_5G 3:BANDMODESIGNLE
	{
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (A)", "3", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (N)", "7", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (AC)", "63", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (A+N)", "11", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (N+AC)", "71", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("5 GHz (A+N+AC)", "75", false, false);
	}

	if(Band2G5GSupport == 1 || wlBandMode == 3) // 1:PHYBAND_2G 3:BANDMODESIGNLE
	{
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B)", "0", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (G)", "1", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (N)", "7", false, false); 
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B+G)", "2", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (G+N)", "9", false, false);
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B+G+N)", "10", false, false);
		if(wlan_support_ac2g)
			form.elements["band"+wlan_id].options[idx++] = new Option("2.4 GHz (B+G+N+AC)", "74", false, false);

	}

	if (wlBandMode == 3)
		form.elements["band"+wlan_id].options[idx++] = new Option("2.4GHz + 5 GHz (A+B+G+N)", "14", false, false);

	for(i=0 ; i<idx ; i++)
	{
		if(form.elements["band"+wlan_id].options[i].value == band_value)
		{
			if(band_value == 7 && wlBandMode == 3)// 2g and 5g has the same band value in N.
			{
				var selectText=form.elements["band"+wlan_id].options[i].text.substr(0,1);
				
				if( (Band2G5GSupport == 2 && selectText == '5') //2:PHYBAND_5G
				||	(Band2G5GSupport == 1 && selectText == '2') //1:PHYBAND_2G
				) 
				{
			form.elements["band"+wlan_id].selectedIndex = i;
			break;
		}				
	}	
			else
			{	
				form.elements["band"+wlan_id].selectedIndex = i;
				break;
			}
		}				
	}	

 form.elements["band"+wlan_id].length = idx;
}

function showBand(form, wlan_id)
{
  if (APMode[wlan_id] != 1)
	showBandAP(form, wlan_id);
  else
 	showBandClient(form, wlan_id);
}
function get_by_id(id){
	with(document){
	return getElementById(id);
	}
}
function get_by_name(name){
	with(document){
	return getElementsByName(name);
	}
}
function updateMode(form, wlan_id)
{
	var chan_boundid;
	var controlsidebandid;
	var wlan_wmm1;
	var wlan_wmm2;
	var networktype;
	var mode_idx =form.elements["mode"+wlan_id].selectedIndex;
	var mode_value = form.elements["mode"+wlan_id].options[mode_idx].value; 
	var idx_value= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[idx_value].value;
	if (form.elements["mode"+wlan_id].selectedIndex != 1) {
  		if (APMode[wlan_id] == 1) {
			if (bandIdxAP[wlan_id] < 0){
				bandIdx[wlan_id]=2;	// set B+G as default
			}else{
				bandIdx[wlan_id]=bandIdxAP[wlan_id];
			}
		}  
	}else {
	  	if (APMode[wlan_id] != 1) {
			if (bandIdxClient[wlan_id] < 0) {
	 			if (RFType[wlan_id] == 10)
					bandIdx[wlan_id]=2;	// set B+G as default
				else
					bandIdx[wlan_id]=6;	// set A+B+G as default
			}
			else{
				bandIdx[wlan_id]=bandIdxClient[wlan_id];
			}
		}	
	}
	APMode[wlan_id] =form.elements["mode"+wlan_id].selectedIndex;
	showBand(form, wlan_id);
  	if(form == document.wlanSetup){
  		wlan_wmm1 = form.elements["wlanwmm"+wlan_id];
  		wlan_wmm2 =  get_by_id("wlan_wmm");
	}

	networktype = form.elements["type"+wlan_id];
	if(mode_value !=1) {
		networktype.disabled = true;
	}else {
		networktype.selectedIndex = networkType[wlan_id];
		networktype.disabled = false;		
	}
	
 	chan_boundid = get_by_id("channel_bounding");
  	controlsidebandid = get_by_id("control_sideband");  
	
  	if(bandIdx[wlan_id] == 9 || bandIdx[wlan_id] == 10 ||  bandIdx[wlan_id] == 7 || bandIdx[wlan_id] == 11 || bandIdx[wlan_id] == 14 || bandIdx[wlan_id] == 63 || bandIdx[wlan_id] == 71 || bandIdx[wlan_id] == 67 ||bandIdx[wlan_id] == 75 ||bandIdx[wlan_id] == 74){
		chan_boundid.style.display = "";
	 	controlsidebandid.style.display = "";
		 if(form == document.wlanSetup){
			wlan_wmm1.disabled = true;
		 		//wlan_wmm2.disabled = true;
		}
	}else{
		chan_boundid.style.display = "none";
		controlsidebandid.style.display = "none";
	 	 if(form == document.wlanSetup){
	 		wlan_wmm1.disabled = false;
	 		//wlan_wmm2.disabled = false;
	 	}
	 }
	  updateIputState(form, wlan_id);
	 if(form==document.wizard){
		var chan_number_idx=form.elements["chan"+wlan_id].selectedIndex;
		var chan_number= form.elements["chan"+wlan_id].options[chan_number_idx].value;	
		if(chan_number == 0)
			disableTextField(form.elements["controlsideband"+wlan_id]);	
		else{
			if(form.elements["channelbound"+wlan_id].selectedIndex == "0")
	 			disableTextField(form.elements["controlsideband"+wlan_id]);	
	 		else
				enableTextField(form.elements["controlsideband"+wlan_id]);		
		}
	}
}
function should_swap(form, wlan_id)
{
if(!form.elements["support_8881a_selective"].value)
	return;
	var band_index= form.elements["band"+wlan_id].selectedIndex;
	var band_value= +(form.elements["band"+wlan_id].options[band_index].value) + 1;
	var selectText=form.elements["band"+wlan_id].options[band_index].text.substr(0,1);
	var Band2G5GSupport=form.elements["Band2G5GSupport"].value;

	if ((selectText == 2 && Band2G5GSupport == 1) || (selectText == 5 && Band2G5GSupport == 2))
		form.elements["need_swap"].value = "0";
	else
	{
		form.elements["need_swap"].value = "1";
		if (Band2G5GSupport == 1)
			form.elements["Band2G5GSupport"].value = "2";
		else
			form.elements["Band2G5GSupport"].value = "1";
	}	
}

function swap_mib(form, wlan_id)
{
if(!form.elements["support_8881a_selective"].value)
	return;
	var band_index= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[band_index].value;
	var cur_band= form.elements["cur_band"].value;
	var need_swap= form.elements["need_swap"].value;

	if (need_swap==1) {
		form.elements["cur_band"].value = +(form.elements["band"+wlan_id].options[band_index].value) + 1;

		/* 2 to c */
		form.elements["chan0"].value = form.elements["channel_2"].value;
		form.elements["channelbound0"].value = form.elements["ChannelBonding_2"].value;
		form.elements["controlsideband0"].value = form.elements["ControlSideBand_2"].value;
		form.elements["operrates0"].value = form.elements["operRate_2"].value;
		form.elements["basicrates0"].value = form.elements["basicRate_2"].value;

		/* 1 to 2 */
		form.elements["channel_2"].value = form.elements["channel_1"].value;
		form.elements["ChannelBonding_2"].value = form.elements["ChannelBonding_1"].value;
		form.elements["ControlSideBand_2"].value = form.elements["ControlSideBand_1"].value;
		form.elements["operRate_2"].value = form.elements["operRate_1"].value;
		form.elements["basicRate_2"].value = form.elements["basicRate_1"].value;

		/* c to 1 */
		form.elements["channel_1"].value = form.elements["chan0"].value;
		form.elements["ChannelBonding_1"].value = form.elements["channelbound0"].value;
		form.elements["ControlSideBand_1"].value = form.elements["controlsideband0"].value;
		form.elements["operRate_1"].value = form.elements["operrates0"].value;
		form.elements["basicRate_1"].value = form.elements["basicrates0"].value;

		form.elements["need_swap"].value = "0";
	}
}
function updateBand(form, wlan_id)
{
	var band_index= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[band_index].value;
  if (APMode[wlan_id] != 1){
	bandIdxAP[wlan_id] = band_value;
  }else{
	bandIdxClient[wlan_id] =band_value;
  }	

  updateChan(form, wlan_id);
  
}

function updateRepeaterState(form, wlan_id)
{   
  if(!form.elements["wlanDisabled"+wlan_id].checked &&  	
    ((form.elements["mode"+wlan_id].selectedIndex!=1) ||
       ((form.elements["mode"+wlan_id].selectedIndex==1) &&
     	(form.elements["type"+wlan_id].selectedIndex==0))) 
     ){     	
     	  if(form == document.wlanSetup){	
	enableCheckBox(form.elements["repeaterEnabled"+wlan_id]);
	if (form.elements["repeaterEnabled"+wlan_id].checked)
 		enableTextField(form.elements["repeaterSSID"+wlan_id]);
  	else
  		disableTextField(form.elements["repeaterSSID"+wlan_id]);
  }
  }
  else {
  		 if(form == document.wlanSetup){	
			disableCheckBox(form.elements["repeaterEnabled"+wlan_id]);
			disableTextField(form.elements["repeaterSSID"+wlan_id]);
		}
  }
}

function updateType(form, wlan_id)
{
	var mode_selected=0;
	var Type_selected=0;
	var index_channelbound=0;
  updateChan(form, wlan_id);
  updateIputState(form, wlan_id);
  updateRepeaterState(form, wlan_id);
  Type_selected = form.elements["type"+wlan_id].selectedIndex;
  mode_selected=form.elements["mode"+wlan_id].selectedIndex;
  //if client and infrastructure mode
  	if(mode_selected ==1){
		if(Type_selected == 0){
			disableTextField(form.elements["controlsideband"+wlan_id]);
			disableTextField(form.elements["channelbound"+wlan_id]);
		}else{
			enableTextField(form.elements["channelbound"+wlan_id]);
			index_channelbound=form.elements["channelbound"+wlan_id].selectedIndex;
		if(index_channelbound ==0)
			disableTextField(form.elements["controlsideband"+wlan_id]);	
		else if(index_channelbound ==2)
			disableTextField(form.elements["controlsideband"+wlan_id]);	
		else
			enableTextField(form.elements["controlsideband"+wlan_id]);
		}
	}
	
		var chan_number_idx=form.elements["chan"+wlan_id].selectedIndex;
		var chan_number= form.elements["chan"+wlan_id].options[chan_number_idx].value;	
		if(chan_number == 0)
			disableTextField(form.elements["controlsideband"+wlan_id]);	
		else{
			if(form.elements["channelbound"+wlan_id].selectedIndex == "0")
	 			disableTextField(form.elements["controlsideband"+wlan_id]);	
			else if(form.elements["channelbound"+wlan_id].selectedIndex == "2")
				disableTextField(form.elements["controlsideband"+wlan_id]);	
	 		else
				enableTextField(form.elements["controlsideband"+wlan_id]);		
		}
}
function pskFormatChange(form,wlan_id)
{
	if (form.elements["pskFormat"+wlan_id].selectedIndex ==0){
		form.elements["pskValue"+wlan_id].maxLength = "63";
	}
	else{
		form.elements["pskValue"+wlan_id].maxLength = "64";
	}
}
/*==============================================================================*/
/*   wlwpa.htm */
function disableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
 	var radioButton = radioArrOrButton;
 	var radioArray = radioButton.form[radioButton.name];
  }
  else
 	var radioArray = radioArrOrButton;
 	radioArray.disabled = true;
 	for (var b = 0; b < radioArray.length; b++) {
 	if (radioArray[b].checked) {
 		radioArray.checkedElement = radioArray[b];
 		break;
	}
  }
  for (var b = 0; b < radioArray.length; b++) {
 	radioArray[b].disabled = true;
 	radioArray[b].checkedElement = radioArray.checkedElement;
  }
}

function enableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
 	var radioButton = radioArrOrButton;
 	var radioArray = radioButton.form[radioButton.name];
  }
  else
 	var radioArray = radioArrOrButton;

  radioArray.disabled = false;
  radioArray.checkedElement = null;
  for (var b = 0; b < radioArray.length; b++) {
 	radioArray[b].disabled = false;
 	radioArray[b].checkedElement = null;
  }
}

function preserve () { this.checked = this.storeChecked; }
function disableCheckBox (checkBox) {
  if (!checkBox.disabled) {
    checkBox.disabled = true;
    if (!document.all && !document.getElementById) {
      checkBox.storeChecked = checkBox.checked;
      checkBox.oldOnClick = checkBox.onclick;
      checkBox.onclick = preserve;
    }
  }
}

function enableCheckBox (checkBox)
{
  if (checkBox.disabled) {
    checkBox.disabled = false;
    if (!document.all && !document.getElementById)
      checkBox.onclick = checkBox.oldOnClick;
  }
}

function openWindow(url, windowName, wide, high) {
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=500;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';
	window.open( url, windowName, settings );
}
function ppp_getDigit(str, num)
{ 
	var i=1; 
	// replace the char '/' with character '.'  
	str = str.replace(/[/]/,".");	  
	if ( num != 1 ) 
	{  	
		while (i!=num && str.length!=0) 
		{		
			if ( str.charAt(0) == '.') 
			{			
				i++;		
			}
			str = str.substring(1);  	
		}
		if ( i!=num )  		
			return -1;  
	}  
	for (i=0; i<str.length; i++) 
	{  	
		if ( str.charAt(i) == '.') 
		{
			str = str.substring(0, i);		
			break;
		}  
	}  
	if ( str.length == 0)  	
		return -1;  
	var d = parseInt(str, 10); 
	return d;
}

function ppp_checkDigitRange(str, num, min, max)
{	  
	var d = ppp_getDigit(str,num);  
	if ( d > max || d < min )      	
		return false;  
	return true;
}

function ppp_validateKey(str)
{   
	for (var i=0; i<str.length; i++) 
	{    
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') 
			||(str.charAt(i) == '.' ) || (str.charAt(i) == '/'))			
			continue;	
		return 0;  
	}  
	return 1;
}
function validateKey(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '.' ) )
			continue;
	return 0;
  }
  return 1;
}

function intvalidateKey(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') )
			continue;
	return 0;
  }
  return 1;
}

function getDigit(str, num)
{
  var i=1;
  if ( num != 1 ) {
  	while (i!=num && str.length!=0) {
		if ( str.charAt(0) == '.' ) {
			i++;
		}
		str = str.substring(1);
  	}
  	if ( i!=num )
  		return -1;
  }
  for (i=0; i<str.length; i++) {
  	if ( str.charAt(i) == '.' ) {
		str = str.substring(0, i);
		break;
	}
  }
  if ( str.length == 0)
  	return -1;
  var d = parseInt(str, 10);
  return d;
}

function checkDigitRange(str, num, min, max)
{
  var d = getDigit(str,num);
  if ( d > max || d < min || (num==1 && d==127))
      	return false;
  return true;
}


function check_wpa_psk(form, wlan_id)
{
	var str = form.elements["pskValue"+wlan_id].value;
	if (form.elements["pskFormat"+wlan_id].selectedIndex==1) {
		if (str.length != 64) {
			alert('Pre-Shared Key value should be 64 characters.');
			form.elements["pskValue"+wlan_id].focus();
			return false;
		}
		takedef = 0;
		if (defPskFormat[wlan_id] == 1 && defPskLen[wlan_id] == 64) {
			for (var i=0; i<64; i++) {
    				if ( str.charAt(i) != '*')
					break;
			}
			if (i == 64 )
				takedef = 1;
  		}
		if (takedef == 0) {
			for (var i=0; i<str.length; i++) {
    				if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
					(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
					(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
					continue;
				alert('Invalid Pre-Shared Key value. It should be in hex number (0-9 or a-f).');
				form.elements["pskValue"+wlan_id].focus();
				return false;
  			}
		}
	}
	else {
		if (str.length < 8) {
			alert('Pre-Shared Key value should be set at least 8 characters.');
			form.elements["pskValue"+wlan_id].focus();
			return false;
		}
		if (str.length > 63) {
			alert('Pre-Shared Key value should be less than 64 characters.' );
			form.elements["pskValue"+wlan_id].focus();
			return false;
		}
	}


  
  return true;
}

function saveChanges_wpa(form, wlan_id)
{
  var method = form.elements["method"+wlan_id] ;
  var wpaAuth= form.elements["wpaAuth"+wlan_id] ;

  if (method.selectedIndex>=2 && (wpaAuth.value == "psk" || wpaAuth[1].checked))
	return check_wpa_psk(form, wlan_id);	
 
    if (form.elements["use1x"+wlan_id].value != "OFF" && form.elements["radiusPort"+wlan_id].disabled == false ) {
	if (form.elements["radiusPort"+wlan_id].value=="") {
		alert(util_gw_empty_radius_port + util_gw_decimal_rang);
		form.elements["radiusPort"+wlan_id].focus();
		return false;
  	}
	if (validateKey(form.elements["radiusPort"+wlan_id].value)==0) {
		alert(util_gw_invalid_radius_port + util_gw_decimal_rang);
		form.elements["radiusPort"+wlan_id].focus();
		return false;
	}
        port = parseInt(form.elements["radiusPort"+wlan_id].value, 10);

 	if (port > 65535 || port < 1) {
		alert(util_gw_invalid_radius_port+util_gw_decimal_rang);
		form.elements["radiusPort"+wlan_id].focus();
		return false;
  	}

	if ( checkIpAddr(form.elements["radiusIP"+wlan_id], util_gw_invalid_radius_ip) == false )
	    return false;
   } 
   	
   
   
   return true;
}
/*==============================================================================*/
/*   tcpiplan.htm  */
function checkMask(str, num)
{
  var d = getDigit(str,num);
  if(num==1)
  {
  	if( !(d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 ))
  		return false;
  }
  else
  {
  	if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 ))
  		return false;
  }
  return true;
}

function checkWholeMask(str)
{
	if(str.length==0)
		return false;
	var d1 = getDigit(str,1);
	var d2 = getDigit(str,2);
	var d3 = getDigit(str,3);
	var d4 = getDigit(str,4);
	if(d1==-1||d2==-1||d3==-1||d4==-1||d1==0)
		return false;
	if(d1!=255&&d2!=0)
		return false;
	if(d2!=255&&d3!=0)
		return false;
	if(d3!=255&&d4!=0)
		return false;
	return true;
}


function checkSubnet(ip, mask, client)
{
  var ip_d = getDigit(ip, 1);
  var mask_d = getDigit(mask, 1);
  var client_d = getDigit(client, 1);
  if ( (ip_d & mask_d) != (client_d & mask_d ) )
	return false;

  ip_d = getDigit(ip, 2);
  mask_d = getDigit(mask, 2);
  client_d = getDigit(client, 2);
  if ( (ip_d & mask_d) != (client_d & mask_d ) )
	return false;

  ip_d = getDigit(ip, 3);
  mask_d = getDigit(mask, 3);
  client_d = getDigit(client, 3);
  if ( (ip_d & mask_d) != (client_d & mask_d ) )
	return false;

  ip_d = getDigit(ip, 4);
  mask_d = getDigit(mask, 4);
  client_d = getDigit(client, 4);
  if ( (ip_d & mask_d) != (client_d & mask_d ) )
	return false;

  return true;
}
function checkIPMask(field)
{

  if (field.value=="") {
      	alert(util_gw_mask_empty + util_gw_ip_format);
	field.value = field.defaultValue;
	field.focus();
	return false;
  }
  if(field.value=="0.0.0.0"){
  		alert(util_gw_mask_invalid);
		field.value = field.defaultValue;
		field.focus();
		return false;
  }
  if ( validateKey( field.value ) == 0 ) {
      	alert(util_gw_mask_invalid + util_gw_decimal_value_rang);
      	field.value = field.defaultValue;
	field.focus();
	return false;
  }
  if ( !checkMask(field.value,1) ) {
      	alert(util_gw_mask_invalid1 + util_gw_mask_rang1);
	field.value = field.defaultValue;
	field.focus();
	return false;
  }

  if ( !checkMask(field.value,2) ) {
      	alert(util_gw_mask_invalid2 + util_gw_mask_rang);
	field.value = field.defaultValue;
	field.focus();
	return false;
  }
  if ( !checkMask(field.value,3) ) {
      	alert(util_gw_mask_invalid3 + util_gw_mask_rang);
	field.value = field.defaultValue;
	field.focus();
	return false;
  }
  if ( !checkMask(field.value,4) ) {
      	alert(util_gw_mask_invalid4 + util_gw_mask_rang);
	field.value = field.defaultValue;
	field.focus();
	return false;
  }
  if(!checkWholeMask(field.value)){
  		alert(util_gw_mask_invalid);
	field.value = field.defaultValue;
	field.focus;
	return false;
  }
  return true;
}  
function checkIpAddr(field, msg)
{
  if (field.value=="") {
	alert(util_gw_ipaddr_empty);
	field.value = field.defaultValue;
	field.focus();
	return false;
  }
   if ( validateKey(field.value) == 0) {
      alert(msg + util_gw_ipaddr_nodecimal);
      field.value = field.defaultValue;
      field.focus();
      return false;
   }
   if ( !checkDigitRange(field.value,1,1,223) ) {
      alert(msg+util_gw_ipaddr_1strange);
      field.value = field.defaultValue;
      field.focus();
      return false;
   }
   if ( !checkDigitRange(field.value,2,0,255) ) {
      alert(msg + util_gw_ipaddr_2ndrange);
      field.value = field.defaultValue;
      field.focus();
      return false;
   }
   if ( !checkDigitRange(field.value,3,0,255) ) {
      alert(msg + util_gw_ipaddr_3rdrange);
      field.value = field.defaultValue;
      field.focus();
      return false;
   }
   /*if ( !checkDigitRange(field.value,4,1,254) ) {
      alert(msg + util_gw_ipaddr_4thrange);
      field.value = field.defaultValue;
      field.focus();
      return false;
   }*/
   return true;
}

function checkHostIPValid(ipAddr,mask,msg)
{		
	if(!checkIpAddr(ipAddr,msg))	    return false;
	var ip_int = ipv4_to_unsigned_integer(ipAddr.value);
	var mask_int = ipv4_to_unsigned_integer(mask.value);
	var mask_str = mask_int.toString(2);
	//alert(mask_str);
	var index0 = 32 - mask_str.indexOf('0');
	//alert("mask len:"+index0); 	

	var tmp = Math.pow(2,index0) -1;
	//alert("tmp:"+tmp);

	//var tmp_str = tmp.toString(2);
	//alert("tmp_str:"+tmp_str);

	var host = ip_int & tmp;
	//alert("host:"+host);

	if(host == 0 || host == tmp){
		alert(msg);
		return false;
	}
	return true;

}
function checkIntDigitValid(str)
{
	var reg = /^[1-9][0-9]*/;
	if(!reg.exec(str))
	{		
		return false;
	}
	return true;
}
function checkIntSize(field,rangeStart,rangeEnd,msg)
{
	if (field != null)
	{
	     var d2 = parseInt(field.value, 10);
	     if (!intvalidateKey(field.value)||
			(d2 > rangeEnd || d2 < rangeStart) ) 
		{
			alert(msg);
			field.value = field.defaultValue;
			field.focus();
			return false;
	    }
	 } 
	return true;
}
function checkStrNoNULL(field,msg)
{
	if(field!=null)
	{
		if(field.value=="")
		{
			alert(msg);
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	return true;
}
function checkGateway(field_gw,field_ip,field_mask)
{
	if (field_gw.value!="" && field_gw.value!="0.0.0.0") 
	{
		if (!checkIpAddr(field_gw, util_gw_invalid_degw_ip))  return false;
		if (!checkSubnet(field_ip.value,field_mask.value,field_gw.value)) 
		{
		  alert(util_gw_invalid_gw_ip + util_gw_locat_subnet);
		  field_gw.value = field_gw.defaultValue;
		  field_gw.focus();
		  return false;
		}
	}
	else
		field_gw.value = '0.0.0.0';
	return true;
}
function check_macAddr(field)
{
	if(!field) return true;
	if(field.value=="")
	{
		field.value="000000000000";
		return true;
	}
	var str=field.value;
	if ( str.length != 12) 
	{
		alert(util_gw_mac_complete + util_gw_12hex);
		field.value = field.defaultValue;
		field.focus();
		return false;
  	}	
	
	if(parseInt(str.substr(0, 2), 16) & 0x01 != 0)
	{
		alert(util_gw_invalid_mac + util_gw_bcast_mcast_mac);		
		field.focus();
		field.select();
		return false;
	}
	
	var reg = /[0-9a-fA-F]{12}/;
	if(!reg.exec(field.value))
	{
		field.focus();
		field.select();
		alert(util_gw_invalid_mac + util_gw_hex_rang);
		return false;
	}
	return true;   	
}
function check_AllFF_Mac(field)
{
	var str = field.value;
	if(str.toLowerCase() == "ffffffffffff"){
		alert(util_gw_mac_ff);
		return false;
	}
	return true;
}
function check_All00_Mac(field)
{
	var str = field.value;
	if(str == "000000000000"){
		alert(util_gw_mac_zero);
		return false;
	}
	return true;
}

function is_Float(field)
{
	var str = field.value;
	if(str.indexOf('.') != -1){
		return true;
	}
	return false;
}
function checkMacAddr_ACL(field,msg)
{
    if(field.value=="000000000000")
    {
		field.focus();
		field.select();
        alert(msg);
        return false
    }
    return check_macAddr(field)
}

function ppp_checkSubNetFormat(field,msg)
{
	if (field.value=="") 
	{		
		alert(util_gw_ip_empty + util_gw_ip_format);		
		field.value = field.defaultValue;		
		field.focus();		
		return false;   
	}
	if ( ppp_validateKey(field.value) == 0) 
	{      
		alert(msg + util_gw_invalid_value  + util_gw_decimal_value_rang);      
		field.value = field.defaultValue;      
		field.focus();      
		return false;   
	}   
	if ( !ppp_checkDigitRange(field.value,1,0,255) ) 
	{      
		alert(msg+ util_gw_check_ppp_rang1 + util_gw_should_be + '0-255.');      
		field.value = field.defaultValue;      
		field.focus();      
		return false;   
	}   
	if ( !ppp_checkDigitRange(field.value,2,0,255) ) 
	{      
		alert(msg + util_gw_check_ppp_rang2 + util_gw_should_be + '0-255.');      
		field.value = field.defaultValue;      
		field.focus();      
		return false;   
	}   
	if ( !ppp_checkDigitRange(field.value,3,0,255) ) 
	{      
		alert(msg + util_gw_check_ppp_rang3 + util_gw_should_be + '0-255.');      
		field.value = field.defaultValue;      
		field.focus();      
		return false;   
	}   
	if ( !ppp_checkDigitRange(field.value,4,0,254) ) 
	{      
		alert(msg + util_gw_check_ppp_rang4 + util_gw_should_be + '1-254.');      
		field.value = field.defaultValue;      
		field.focus();     
		return false;   
	}   
	if ( !ppp_checkDigitRange(field.value,5,1,32) )
	{      
		alert(msg + util_gw_check_ppp_rang5 + util_gw_should_be + '1-32.');      
		field.value = field.defaultValue;      
		field.focus();      
		return false;   
	}      
	return true;
}

/////////////////////////////////////////////////////////////////////////////
/*wlwep.htm*/
function validateKey_wep(form, idx, str, len, wlan_id)
{
 if (idx >= 0) {

  if (str.length ==0)
  	return 1;

  if ( str.length != len) {
  	idx++;
	alert(util_gw_invalid_key_length + idx + util_gw_invalid_value + util_gw_should_be + len + util_gw_char);
	return 0;
  }
  }
  else {
	if ( str.length != len) {
		alert(util_gw_invalid_wep_key_value + len + util_gw_char);
		return 0;
  	}
  }
  if ( str == "*****" ||
       str == "**********" ||
       str == "*************" ||
       str == "**************************" )
       return 1;

  if (form.elements["format"+wlan_id].selectedIndex==0)
       return 1;

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert(util_gw_invalid_key_value + util_gw_hex_rang);
	return 0;
  }

  return 1;
}

function setDefaultWEPKeyValue(form, wlan_id)
{
  if (form.elements["length"+wlan_id].selectedIndex == 0) {
	if ( form.elements["format"+wlan_id].selectedIndex == 0) {
		form.elements["key"+wlan_id].maxLength = 5;
		form.elements["key"+wlan_id].value = "*****";
	}
	else {
		form.elements["key"+wlan_id].maxLength = 10;
		form.elements["key"+wlan_id].value = "**********";

	}
  }
  else {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0) {
		form.elements["key"+wlan_id].maxLength = 13;
		form.elements["key"+wlan_id].value = "*************";

	}
	else {
		form.elements["key"+wlan_id].maxLength = 26;
		form.elements["key"+wlan_id].value ="**************************";
	}
  }

// for WPS ---------------------------------------->>
//  wps_wep_key_old =  form.elements["key"+wlan_id].value;
//<<----------------------------------------- for WPS
  
}
function saveChanges_wepkey(form, wlan_id)
{
  var keyLen;
  if (form.elements["length"+wlan_id].selectedIndex == 0) {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0)
		keyLen = 5;
	else
		keyLen = 10;
  }
  else {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0)
		keyLen = 13;
	else
		keyLen = 26;
  }

  if (validateKey_wep(form, 0,form.elements["key"+wlan_id].value, keyLen, wlan_id)==0) {
	form.elements["key"+wlan_id].focus();
	return false;
  }



  return true;
}

function setDefaultKeyValue(form, wlan_id)
{
  if (form.elements["length"+wlan_id].selectedIndex == 0) {
	if ( form.elements["format"+wlan_id].selectedIndex == 0) {
		form.elements["key"+wlan_id].maxLength = 5;
		form.elements["key"+wlan_id].value = "*****";
		

	}
	else {
		form.elements["key"+wlan_id].maxLength = 10;
		form.elements["key"+wlan_id].value = "**********";
		

	}
  }
  else {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0) {
		form.elements["key"+wlan_id].maxLength = 13;		
		form.elements["key"+wlan_id].value = "*************";		


	}
	else {
		form.elements["key"+wlan_id].maxLength = 26;
		form.elements["key"+wlan_id].value ="**************************";		
	
	}
  }


  
}


function saveChanges_wep(form, wlan_id)
{
  var keyLen;
  if (form.elements["length"+wlan_id].selectedIndex == 0) {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0)
		keyLen = 5;
	else
		keyLen = 10;
  }
  else {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0)
		keyLen = 13;
	else
		keyLen = 26;
  }

  if (validateKey_wep(form, 0,form.elements["key"+wlan_id].value, keyLen, wlan_id)==0) {
	form.elements["key"+wlan_id].focus();
	return false;
  }

  


  return true;
}



function lengthClick(form, wlan_id)
{
  updateFormat(form, wlan_id);
}

///////////////////////////////////////////////////////////////////////
//ntp.htm and wizard-ntp.htm
var ntp_zone_index=4;

function ntp_entry(name, value) { 
	this.name = name ;
	this.value = value ;
} 

var ntp_zone_array=new Array(65);
ntp_zone_array[0]=new ntp_entry(util_gw_array0,"12 1");
ntp_zone_array[1]=new ntp_entry(util_gw_array1,"11 1");
ntp_zone_array[2]=new ntp_entry(util_gw_array2, "10 1");
ntp_zone_array[3]=new ntp_entry(util_gw_array3, "9 1");
ntp_zone_array[4]=new ntp_entry(util_gw_array4, "8 1");
ntp_zone_array[5]=new ntp_entry(util_gw_array5, "7 1");
ntp_zone_array[6]=new ntp_entry(util_gw_array6, "7 2");
ntp_zone_array[7]=new ntp_entry(util_gw_array7, "6 1");
ntp_zone_array[8]=new ntp_entry(util_gw_array8, "6 2");
ntp_zone_array[9]=new ntp_entry(util_gw_array9, "6 3");
ntp_zone_array[10]=new ntp_entry(util_gw_array10, "5 1");
ntp_zone_array[11]=new ntp_entry(util_gw_array11, "5 2");
ntp_zone_array[12]=new ntp_entry(util_gw_array12, "5 3");
ntp_zone_array[13]=new ntp_entry(util_gw_array13, "4 1");
ntp_zone_array[14]=new ntp_entry(util_gw_array14, "4 2");
ntp_zone_array[15]=new ntp_entry(util_gw_array15, "4 3");
ntp_zone_array[16]=new ntp_entry(util_gw_array16, "3 1");
ntp_zone_array[17]=new ntp_entry(util_gw_array17, "3 2");
ntp_zone_array[18]=new ntp_entry(util_gw_array18, "3 3");
ntp_zone_array[19]=new ntp_entry(util_gw_array19, "2 1");
ntp_zone_array[20]=new ntp_entry(util_gw_array20, "1 1");
ntp_zone_array[21]=new ntp_entry(util_gw_array21, "0 1");
ntp_zone_array[22]=new ntp_entry(util_gw_array22, "0 2");
ntp_zone_array[23]=new ntp_entry(util_gw_array23, "-1 1");
ntp_zone_array[24]=new ntp_entry(util_gw_array24, "-1 2");
ntp_zone_array[25]=new ntp_entry(util_gw_array25, "-1 3");
ntp_zone_array[26]=new ntp_entry(util_gw_array26, "-1 4");
ntp_zone_array[27]=new ntp_entry(util_gw_array27, "-1 5");
ntp_zone_array[28]=new ntp_entry(util_gw_array28, "-1 6");
ntp_zone_array[29]=new ntp_entry(util_gw_array29, "-2 1");
ntp_zone_array[30]=new ntp_entry(util_gw_array30, "-2 2");
ntp_zone_array[31]=new ntp_entry(util_gw_array31, "-2 3");
ntp_zone_array[32]=new ntp_entry(util_gw_array32, "-2 4");
ntp_zone_array[33]=new ntp_entry(util_gw_array33, "-2 5");
ntp_zone_array[34]=new ntp_entry(util_gw_array34, "-2 6");
ntp_zone_array[35]=new ntp_entry(util_gw_array35, "-3 1");
ntp_zone_array[36]=new ntp_entry(util_gw_array36, "-3 2");
ntp_zone_array[37]=new ntp_entry(util_gw_array37, "-3 3");
ntp_zone_array[38]=new ntp_entry(util_gw_array38, "-3 4");
ntp_zone_array[39]=new ntp_entry(util_gw_array39, "-4 1");
ntp_zone_array[40]=new ntp_entry(util_gw_array40, "-4 2");
ntp_zone_array[41]=new ntp_entry(util_gw_array41, "-4 3");
ntp_zone_array[42]=new ntp_entry(util_gw_array42, "-5 1");
ntp_zone_array[43]=new ntp_entry(util_gw_array43, "-5 2");
ntp_zone_array[44]=new ntp_entry(util_gw_array44, "-5 3");
ntp_zone_array[45]=new ntp_entry(util_gw_array45, "-6 1");
ntp_zone_array[46]=new ntp_entry(util_gw_array46, "-6 2");
ntp_zone_array[47]=new ntp_entry(util_gw_array47, "-7 1");
ntp_zone_array[48]=new ntp_entry(util_gw_array48, "-8 1");
ntp_zone_array[49]=new ntp_entry(util_gw_array49, "-8 2");
ntp_zone_array[50]=new ntp_entry(util_gw_array50, "-8 3");
ntp_zone_array[51]=new ntp_entry(util_gw_array51, "-8 4");
ntp_zone_array[52]=new ntp_entry(util_gw_array52, "-9 1");
ntp_zone_array[53]=new ntp_entry(util_gw_array53, "-9 2");
ntp_zone_array[54]=new ntp_entry(util_gw_array54, "-9 3");
ntp_zone_array[55]=new ntp_entry(util_gw_array55, "-9 4");
ntp_zone_array[56]=new ntp_entry(util_gw_array56, "-9 5");
ntp_zone_array[57]=new ntp_entry(util_gw_array57, "-10 1");
ntp_zone_array[58]=new ntp_entry(util_gw_array58, "-10 2");
ntp_zone_array[59]=new ntp_entry(util_gw_array59, "-10 3");
ntp_zone_array[60]=new ntp_entry(util_gw_array60, "-10 4");
ntp_zone_array[61]=new ntp_entry(util_gw_array61, "-10 5");
ntp_zone_array[62]=new ntp_entry(util_gw_array62, "-11 1");
ntp_zone_array[63]=new ntp_entry(util_gw_array63, "-12 1");
ntp_zone_array[64]=new ntp_entry(util_gw_array64, "-12 2");

var ntp_server_array=new Array(3);
ntp_server_array[0]=new ntp_entry(ntp_server_Europe1,"131.188.3.220");
ntp_server_array[1]=new ntp_entry(ntp_server_Europe2,"130.149.17.8");
ntp_server_array[2]=new ntp_entry(ntp_server_asia1,"203.117.180.36");
/*
ntp_server_array[3]=new ntp_entry(ntp_server_Europe2,"217.144.143.91");
ntp_server_array[4]=new ntp_entry(ntp_server_Australia,"223.27.18.137");
ntp_server_array[5]=new ntp_entry(ntp_server_asia1,"133.100.11.8");
ntp_server_array[6]=new ntp_entry(ntp_server_asia2,"210.72.145.44");
*/

function setTimeZone(field, value){
    field.selectedIndex = 4 ;
    for(i=0 ;i < field.options.length ; i++){
    	if(field.options[i].value == value){
		field.options[i].selected = true;
		break;
}
}

}

function setNtpServer(field, ntpServer){
    field.selectedIndex = 0 ;
    for(i=0 ;i < field.options.length ; i++){
    	if(field.options[i].value == ntpServer){
		field.options[i].selected = true;
		break;
	}
    }
}
function updateState_ntp(form)
{
	if(form.enabled.checked){
		enableTextField(form.timeZone);
		enableTextField(form.ntpServerIp1);
		enableCheckBox (form.dlenabled);
		if(form.ntpServerIp2 != null)
			enableTextField(form.ntpServerIp2);
	}
	else{
		disableTextField(form.timeZone);
		disableTextField(form.ntpServerIp1);
		disableCheckBox (form.dlenabled);
		if(form.ntpServerIp2 != null)
			disableTextField(form.ntpServerIp2);
	}
}

function saveChanges_ntp(form)
{
	if(form.ntpServerIp2.value != ""){
		if ( checkIpAddr(form.ntpServerIp2, util_gw_invalid_ip) == false )
		    return false;
	}
	else
		form.ntpServerIp2.value = "0.0.0.0" ;
	return true;
}
function getRefToDivNest(divID, oDoc) 
{
  if( !oDoc ) { oDoc = document; }
  if( document.layers ) {
	if( oDoc.layers[divID] ) { return oDoc.layers[divID]; } else {
	for( var x = 0, y; !y && x < oDoc.layers.length; x++ ) {
		y = getRefToDivNest(divID,oDoc.layers[x].document); }
	return y; } }
  if( document.getElementById ) { return document.getElementById(divID); }
  if( document.all ) { return document.all[divID]; }
  return document[divID];
}

function progressBar( oBt, oBc, oBg, oBa, oWi, oHi, oDr ) 
{
  MWJ_progBar++; this.id = 'MWJ_progBar' + MWJ_progBar; this.dir = oDr; this.width = oWi; this.height = oHi; this.amt = 0;
  //write the bar as a layer in an ilayer in two tables giving the border
  document.write( '<span id = "progress_div" class = "off" > <table border="0" cellspacing="0" cellpadding="'+oBt+'">'+
	'<tr><td>Please wait...</td></tr><tr><td bgcolor="'+oBc+'">'+
		'<table border="0" cellspacing="0" cellpadding="0"><tr><td height="'+oHi+'" width="'+oWi+'" bgcolor="'+oBg+'">' );
  if( document.layers ) {
	document.write( '<ilayer height="'+oHi+'" width="'+oWi+'"><layer bgcolor="'+oBa+'" name="MWJ_progBar'+MWJ_progBar+'"></layer></ilayer>' );
  } else {
	document.write( '<div style="position:relative;top:0px;left:0px;height:'+oHi+'px;width:'+oWi+';">'+
			'<div style="position:absolute;top:0px;left:0px;height:0px;width:0;font-size:1px;background-color:'+oBa+';" id="MWJ_progBar'+MWJ_progBar+'"></div></div>' );
  }
  document.write( '</td></tr></table></td></tr></table></span>\n' );
  this.setBar = resetBar; //doing this inline causes unexpected bugs in early NS4
  this.setCol = setColour;
}

function resetBar( a, b ) 
{
  //work out the required size and use various methods to enforce it
  this.amt = ( typeof( b ) == 'undefined' ) ? a : b ? ( this.amt + a ) : ( this.amt - a );
  if( isNaN( this.amt ) ) { this.amt = 0; } if( this.amt > 1 ) { this.amt = 1; } if( this.amt < 0 ) { this.amt = 0; }
  var theWidth = Math.round( this.width * ( ( this.dir % 2 ) ? this.amt : 1 ) );
  var theHeight = Math.round( this.height * ( ( this.dir % 2 ) ? 1 : this.amt ) );
  var theDiv = getRefToDivNest( this.id ); if( !theDiv ) { window.status = 'Progress: ' + Math.round( 100 * this.amt ) + '%'; return; }
  if( theDiv.style ) { theDiv = theDiv.style; theDiv.clip = 'rect(0px '+theWidth+'px '+theHeight+'px 0px)'; }
  var oPix = document.childNodes ? 'px' : 0;
  theDiv.width = theWidth + oPix; theDiv.pixelWidth = theWidth; theDiv.height = theHeight + oPix; theDiv.pixelHeight = theHeight;
  if( theDiv.resizeTo ) { theDiv.resizeTo( theWidth, theHeight ); }
  theDiv.left = ( ( this.dir != 3 ) ? 0 : this.width - theWidth ) + oPix; theDiv.top = ( ( this.dir != 4 ) ? 0 : this.height - theHeight ) + oPix;
}

function setColour( a ) 
{
  //change all the different colour styles
  var theDiv = getRefToDivNest( this.id ); if( theDiv.style ) { theDiv = theDiv.style; }
  theDiv.bgColor = a; theDiv.backgroundColor = a; theDiv.background = a;
}

function showcontrolsideband_updated(form, band, wlan_id, rf_num, index)
{
  var idx=0;
  var i;
  var controlsideband_str;

  if((band==7 && index==1) || band ==11 || band ==63 || band ==71 || band ==67 || band ==75)
  {
	 form.elements["controlsideband"+wlan_id].options[idx++] = new Option("Auto", "0", false, false);
	 form.elements["controlsideband"+wlan_id].options[idx++] = new Option("Auto", "1", false, false);
  }
  else
  {
	 form.elements["controlsideband"+wlan_id].options[idx++] = new Option("Upper", "0", false, false);
	 form.elements["controlsideband"+wlan_id].options[idx++] = new Option("Lower", "1", false, false);
  }
  	
  form.elements["controlsideband"+wlan_id].length = idx;
  form.elements["controlsideband"+wlan_id].selectedIndex = 0;
 
	 for (i=0; i<idx; i++) {
	 	controlsideband_str = form.elements["controlsideband"+wlan_id].options[i].value;
	 if(wlan_controlsideband[wlan_id]  == controlsideband_str)
	 	form.elements["controlsideband"+wlan_id].selectedIndex = i;
	 }
}

function showchannelbound_updated(form, band, wlan_id, rf_num)
{
  var idx=0;
  var i;
  var channelbound_str;

 form.elements["channelbound"+wlan_id].options[idx++] = new Option("20MHz", "0", false, false);
 form.elements["channelbound"+wlan_id].options[idx++] = new Option("40MHz", "1", false, false);
 
 if(band==3 || band == 75|| band ==71|| band ==63 || band ==67 ){
 form.elements["channelbound"+wlan_id].options[idx++] = new Option("80MHz", "2", false, false);
 }
 
 form.elements["channelbound"+wlan_id].length = idx;
 
 for (i=0; i<idx; i++) {
 	channelbound_str = form.elements["channelbound"+wlan_id].options[i].value;
 if(wlan_channelbound[wlan_id]  == channelbound_str)
 	form.elements["channelbound"+wlan_id].selectedIndex = i;
 }


}
function showtxrate_updated(form, band, wlan_id, rf_num)
{
  var idx=0;
  var i;
  var txrate_str;
  var channel_width_20M_flag = (form.elements["channelbound" + wlan_idx].selectedIndex == 0) ? 1 : 0;

 form.elements["txRate"+wlan_id].options[idx++] = new Option("Auto", "0", false, false);
 
 if(band == 0 || band ==2 || band ==10 || band==74){
 form.elements["txRate"+wlan_id].options[idx++] = new Option("1M", "1", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("2M", "2", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("5.5M", "3", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("11M", "4", false, false);
}
 if(band ==9 || band ==10 || band ==1 || band ==2 || band == 11 || band == 3 || band==75 || band==67 || band==74){
 form.elements["txRate"+wlan_id].options[idx++] = new Option("6M", "5", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("9M", "6", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("12M", "7", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("18M", "8", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("24M", "9", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("36M", "10", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("48M", "11", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("54M", "12", false, false);
}
 if(band ==9 || band ==10 || band == 7 || band == 11 || band ==71|| band ==75 || band==74){
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS0", "13", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS1", "14", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS2", "15", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS3", "16", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS4", "17", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS5", "18", false, false);
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS6", "19", false, false); 
 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS7", "20", false, false);
 if (rf_num >=2) {
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS8", "21", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS9", "22", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS10", "23", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS11", "24", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS12", "25", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS13", "26", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS14", "27", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("MCS15", "28", false, false);
 }
}

if(band ==63 || band ==71 || band == 75 || band==67 || band==74){
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS0", "30", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS1", "31", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS2", "32", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS3", "33", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS4", "34", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS5", "35", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS6", "36", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS7", "37", false, false);
	 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS8", "38", false, false);
	 if(!channel_width_20M_flag){
	 	form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS1-MCS9", "39", false, false);
	}
	 
	 if(rf_num >=2){//8812_1t1r
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS0", "40", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS1", "41", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS2", "42", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS3", "43", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS4", "44", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS5", "45", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS6", "46", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS7", "47", false, false);
		 form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS8", "48", false, false);
		if(!channel_width_20M_flag){
		 	form.elements["txRate"+wlan_id].options[idx++] = new Option("NSS2-MCS9", "49", false, false);
		 }
	 }
}
 form.elements["txRate"+wlan_id].length = idx;
 
 for (i=0; i<idx; i++) {
 	txrate_str = form.elements["txRate"+wlan_id].options[i].value;
 if(wlan_txrate[wlan_id]  == txrate_str)
 	form.elements["txRate"+wlan_id].selectedIndex = i;
 }


}
var MultiLanguage = 0;
function mavis_write(string_name)
{
	document.write(eval("string_name[" + MultiLanguage + "]"));
}

function update_controlsideband(form, wlan_id)
{
	var index=form.elements["channelbound"+wlan_id].selectedIndex;
	var wlan_support_8812e=form.elements["wlan_support_8812e"].value;
	var idx_value= form.elements["band"+wlan_id].selectedIndex;
	var band_value= form.elements["band"+wlan_id].options[idx_value].value;
	
	if(index ==0 || index==2 || (wlan_support_8812e==1 && (band_value==11 || band_value==63 || band_value==71 || band_value==75 || (band_value==7 && idx_value==1)))) //8812
		disableTextField(form.elements["controlsideband"+wlan_id]);	
	else
		enableTextField(form.elements["controlsideband"+wlan_id]);
	updateChan_channebound(form, wlan_id);
	var chan_number_idx=form.elements["chan"+wlan_id].selectedIndex;
	var chan_number_value=form.elements["chan"+wlan_id].value;	
	
	if(chan_number_idx==0 && chan_number_value==0)
		disableTextField(form.elements["controlsideband"+wlan_id]);	

}

function updateChan_selectedIndex(form, wlan_id)
{
	var chan_number_idx=form.elements["chan"+wlan_id].selectedIndex;
	var chan_number= form.elements["chan"+wlan_id].options[chan_number_idx].value;
	var wlan_support_8812e=form.elements["wlan_support_8812e"].value;
	
	wlan_channel[wlan_id] = chan_number;
	if(chan_number == 0){
		disableTextField(form.elements["controlsideband"+wlan_id]);	
	}
	else{
		if(form.elements["channelbound"+wlan_id].selectedIndex == "0")
 			disableTextField(form.elements["controlsideband"+wlan_id]);	
		else if(form.elements["channelbound"+wlan_id].selectedIndex == "2")
 			disableTextField(form.elements["controlsideband"+wlan_id]);
 		else
			enableTextField(form.elements["controlsideband"+wlan_id]);		
		}
	if((wlan_support_8812e==1) && (chan_number > 14)) //8812
		disableTextField(form.elements["controlsideband"+wlan_id]);
}

function clean_option(field)
{
	while (field.options.length > 0)  
			field.options[0] = null; 
}
function add_option_item(field,name,value)
{
	field.options.add(new Option(name, value));
}

/*
 * ipv4_to_unsigned_integer
 *	Convert an IPv4 address dotted string to an unsigned integer.
 */
function ipv4_to_unsigned_integer(ipaddr)
{
	var ip = ipaddr + "";
	var got = ip.match (/^\s*(\d{1,3})\s*[.]\s*(\d{1,3})\s*[.]\s*(\d{1,3})\s*[.]\s*(\d{1,3})\s*$/);
	if (!got) {
		return null;
	}
	var x = 0;
	var q = 0;
	for (var i = 1; i <= 4; i++) {
		q = parseInt(got[i], 10);
		if (q < 0 || q > 255) {
			return null;
		}
		x = x * 256 + q;
	}
	return x;
}

