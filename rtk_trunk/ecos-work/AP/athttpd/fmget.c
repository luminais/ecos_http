/* =================================================================
 *
 *      fmget.c
 *
 *      Handles the asp requests.
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####                                     
 * -------------------------------------------                       
 * This file is part of eCos, the Embedded Configurable Operating System.
 * Copyright (C) 2005, 2007 Free Software Foundation, Inc.           
 *
 * eCos is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 or (at your option) any later
 * version.                                                          
 *
 * eCos is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.                                                 
 *
 * You should have received a copy of the GNU General Public License 
 * along with eCos; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.     
 *
 * As a special exception, if other files instantiate templates or use
 * macros or inline functions from this file, or you compile this file
 * and link it with other works to produce a work based on this file,
 * this file does not by itself cause the resulting work to be covered by
 * the GNU General Public License. However the source code for this file
 * must still be made available in accordance with section (3) of the GNU
 * General Public License v2.                                        
 *
 * This exception does not invalidate any other reasons why a work based
 * on this file might be covered by the GNU General Public License.  
 * -------------------------------------------                       
 * ####ECOSGPLCOPYRIGHTEND####                                       
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    hf_shi
 *  Contributors: 
 *                
 *  Date:         2012-06-01
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <dirent.h>
#include <network.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#if 0
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/handler.h>
#include <cyg/athttpd/forms.h>
#endif

#include <pkgconf/devs_eth_rltk_819x_wlan.h>

#include "athttpd.h"
#include "asp.h"
#include "fmget.h"
#include "apmib.h"
#include "common.h"
#include "utility.h"
#include "asp_form.h"
#include "../system/sys_utility.h"

#include "socket.h"

//#define DEF_MSSID_NUM 4
#if defined(CONFIG_RTL_VAP_SUPPORT)
#define DEF_MSSID_NUM RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM
#else
#define DEF_MSSID_NUM 0
#endif


extern unsigned char *fwVersion;

extern int wlan_num;
//fix the issue of when change wan type from pppoe to dhcp, the wan interface is still ppp0, not eth1 (amy reported, realsil can not reproduce)
//char *PPPOE_IF= "ppp0";
char PPPOE_IF[8]= "ppp0";
//char *WAN_IF = "eth1";
char WAN_IF[8] = "eth1";
char *BRIDGE_IF = "eth0";
char *ELAN_IF = "eth0";
char *ELAN2_IF = "eth2";
char *ELAN3_IF = "eth3";
char *ELAN4_IF = "eth4";


//int wlan_idx = 0;
static COUNTRY_IE_ELEMENT countryIEArray[] =
{
	/*
	 format: countryNumber | CountryCode(A2) | support (5G) A band? | support (2.4G)G band? |
	*/
	{8,"AL ", 0,3, "ALBANIA"},
	{12,"DZ ", 0,3, "ALGERIA"},
	{32,"AR ", 0,3, "ARGENTINA"},
	{51,"AM ", 0,3,"ARMENIA"},
	{36,"AU ", 0,3, "AUSTRALIA"},
	{40,"AT ", 0,3,"AUSTRIA"},
	{31,"AZ ", 0,3,"AZERBAIJAN"},
	{48,"BH ", 0,3,"BAHRAIN"},
	{112,"BY", 0,3,"BELARUS"},
	{56,"BE ", 0,3,"BELGIUM"},
	{84,"BZ ", 0,8,"BELIZE"},
	{68,"BO ", 0,8,"BOLIVIA"},
	{76,"BR ", 0,3,"BRAZIL"},
	{96,"BN ", 0,3,"BRUNEI"},
	{100,"BG ", 0,3,"BULGARIA"},
	{124,"CA ", 0,1,"CANADA"},
	{152,"CL ", 0,3,"CHILE"},
	{156,"CN ", 0,3,"CHINA"},
	{170,"CO ", 0,1,"COLOMBIA"},
	{188,"CR ", 0,3,"COSTA RICA"},
	{191,"HR ", 0,3,"CROATIA"},
	{196,"CY ", 0,3,"CYPRUS"},
	{203,"CZ ", 0,3,"CZECH REPUBLIC"},
	{208,"DK ", 0,3,"DENMARK"},
	{214,"DO ", 0,1,"DOMINICAN REPUBLIC"},
	{218,"EC ", 0,3,"ECUADOR"},
	{818,"EG ", 0,3,"EGYPT"},
	{222,"SV ", 0,3,"EL SALVADOR"},
	{233,"EE ", 0,3,"ESTONIA"},
	{246,"FI ", 0,3,"FINLAND"},
	{250,"FR ", 0,3,"FRANCE"},
	{268,"GE ", 0,3,"GEORGIA"},
	{276,"DE ", 0,3,"GERMANY"},
	{300,"GR ", 0,3,"GREECE"},
	{320,"GT ", 0,1,"GUATEMALA"},
	{340,"HN ", 0,3,"HONDURAS"},
	{344,"HK ", 0,3,"HONG KONG"},
	{348,"HU ", 0,3,"HUNGARY"},
	{352,"IS ", 0,3,"ICELAND"},
	{356,"IN ", 0,3,"INDIA"},
	{360,"ID ", 0,3,"INDONESIA"},
	{364,"IR ", 0,3,"IRAN"},
	{372,"IE ", 0,3,"IRELAND"},
	{376,"IL ", 0,7,"ISRAEL"},
	{380,"IT ", 0,3,"ITALY"},
	{392,"JP ", 3,6,"JAPAN"},
	{400,"JO ", 0,3,"JORDAN"},
	{398,"KZ ", 0,3,"KAZAKHSTAN"},
	{410,"KR ", 2,3,"NORTH KOREA"},
	{408,"KP ", 2,3,"KOREA REPUBLIC"},
	{414,"KW ", 0,3,"KUWAIT"},
	{428,"LV ", 0,3,"LATVIA"},
	{422,"LB ", 0,3,"LEBANON"},
	{438,"LI ", 0,3,"LIECHTENSTEIN"},
	{440,"LT ", 0,3,"LITHUANIA"},
	{442,"LU ", 0,3,"LUXEMBOURG"},
	{446,"MO ", 0,3,"CHINA MACAU"},
	{807,"MK ", 0,3,"MACEDONIA"},
	{458,"MY ", 0,3,"MALAYSIA"},
	{484,"MX ", 0,1,"MEXICO"},
	{492,"MC ", 0,3,"MONACO"},
	{504,"MA ", 0,3,"MOROCCO"},
	{528,"NL ", 0,3,"NETHERLANDS"},
	{554,"NZ ", 0,8,"NEW ZEALAND"},
	{578,"NO ", 0,3,"NORWAY"},
	{512,"OM ", 0,3,"OMAN"},
	{586,"PK ", 0,3,"PAKISTAN"},
	{591,"PA ", 0,1,"PANAMA"},
	{604,"PE ", 0,3,"PERU"},
	{608,"PH ", 0,3,"PHILIPPINES"},
	{616,"PL ", 0,3,"POLAND"},
	{620,"PT ", 0,3,"PORTUGAL"},
	{630,"PR ", 0,1,"PUERTO RICO"},
	{634,"QA ", 0,3,"QATAR"},
	{642,"RA ", 0,3,"ROMANIA"},
	{643,"RU ", 0,3,"RUSSIAN"},
	{682,"SA ", 0,3,"SAUDI ARABIA"},
	{702,"SG ", 4,3,"SINGAPORE"},
	{703,"SK ", 0,3,"SLOVAKIA"},
	{705,"SI ", 0,3,"SLOVENIA"},
	{710,"ZA ", 0,3,"SOUTH AFRICA"},
	{724,"ES ", 0,3,"SPAIN"},
	{752,"SE ", 0,3,"SWEDEN"},
	{756,"CH ", 0,3,"SWITZERLAND"},
	{760,"SY ", 0,3,"SYRIAN ARAB REPUBLIC"},
	{158,"TW ", 1,1,"TAIWAN"},
	{764,"TH ", 0,3,"THAILAND"},
	{780,"TT ", 0,3,"TRINIDAD AND TOBAGO"},
	{788,"TN ", 0,3,"TUNISIA"},
	{792,"TR ", 0,3,"TURKEY"},
	{804,"UA ", 0,3,"UKRAINE"},
	{784,"AE ", 0,3,"UNITED ARAB EMIRATES"},
	{826,"GB ", 0,3,"UNITED KINGDOM"},
	{840,"US ", 0,1,"UNITED STATES"},
	{858,"UY ", 0,3,"URUGUAY"},
	{860,"UZ ", 0,1,"UZBEKISTAN"},
	{862,"VE ", 0,8,"VENEZUELA"},
	{704,"VN ", 0,3,"VIET NAM"},
	{887,"YE ", 0,3,"YEMEN"},
	{716,"ZW ", 0,3,"ZIMBABWE"}
};

static REG_DOMAIN_TABLE_ELEMENT_T Bandtable_2dot4G[]={
		{0, 0,  ""},
		{1, 11, "FCC"},			//FCC
		{2, 11, "IC"},			//IC
		{3, 13, "ETSI"},			//ETSI world
		{4, 2,  "SPAIN"},			//SPAIN
		{5, 11, "FRANCE"},			//FRANCE
		{6, 13, "MKK"},			//MKK , Japan	
		{7, 7,  "ISRAEL"},			//ISRAEL
		{8, 13, "KOREA"}                //ETSIC Korea
};

static REG_DOMAIN_TABLE_ELEMENT_T Bandtable_5G[]={	

		{0, 1 ,""},
		/*FCC*/
		{1, 13 ,"FCC"},	
		/*ETSI*/
		{2, 19 ,"ETSI"},	

		/*Japan*/
		{3, 23 ,"JAPAN"},	

		/*Singapore*/
		{4, 8 ,"SINGAPORE"},
		/*china*/
		{5, 5 ,"CHINA"},

		/*lsrael*/
		{6, 8 ,"ISRAEL"}
};

#ifndef HAVE_NOWIFI
int getModeCombobox(int argc, char **argv)
{
	int val = 0;
	int opmode;
	apmib_get( MIB_OP_MODE, (void *)&opmode);

	if ( !apmib_get( MIB_WLAN_MODE, (void *)&val) )
			return -1;

#ifdef CONFIG_RTK_MESH
#ifdef CONFIG_NEW_MESH_UI
	  if ( val == 0 ) {
      	  	return web_write_chunked("<option selected value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MESH</option>"
   	  	 "<option value=\"5\">MESH</option>"  );
      	  }
	  if ( val == 1 ) {
     	  	 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option selected value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MESH</option>"
   	  	 "<option value=\"5\">MESH</option>"  );
      	  }
	  if ( val == 2 ) {
     	  	 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
 	  	 "<option selected value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MESH</option>"
   	  	 "<option value=\"5\">MESH</option>"  );
   	  }
	  if ( val == 3 ) {
     	  	 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
 	  	 "<option  value=\"2\">WDS</option>"
   	  	 "<option selected value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MESH</option>"
   	  	 "<option value=\"5\">MESH</option>"  );
   	  }
   	  if ( val == 4 ) {
		 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option selected value=\"4\">AP+MESH</option>"
   	  	 "<option value=\"5\">MESH</option>"  );
   	  }
   	  if ( val == 5 ) {
		 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MESH</option>"
   	  	 "<option selected value=\"5\">MESH</option>"  );
   	  }
	  else
	  return 0;

#else
  	if ( val == 0 ) {
      	  	return web_write_chunked( "<option selected value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>" );
      	  }
	  if ( val == 1 ) {
     	  	 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option selected value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>"  );
      	  }
	  if ( val == 2 ) {
     	  	 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
 	  	 "<option selected value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>"  );
   	  }
	  if ( val == 3 ) {
     	  	 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
 	  	 "<option  value=\"2\">WDS</option>"
   	  	 "<option selected value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>"  );
   	  }
   	  if ( val == 4 ) {
		 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option selected value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>"  );
   	  }
   	  if ( val == 5 ) {
		 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option selected value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>"  );
   	  }
   	   if ( val == 6 ) {
		 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option selected value=\"6\">MAP</option>"
   	  	 "<option value=\"7\">MP</option>"  );
   	  }
   	   if ( val == 7 ) {
		 return web_write_chunked("<option value=\"0\">AP</option>"
   	  	 "<option value=\"1\"><script>dw(wlbasic_client)</script></option>"
   	  	 "<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"
   	  	 "<option value=\"4\">AP+MPP</option>"
   	  	 "<option value=\"5\">MPP</option>"
   	  	 "<option value=\"6\">MAP</option>"
   	  	 "<option selected  value=\"7\">MP</option>" );
   	}
	else
   	return 0;
#endif
#else

  	if ( val == 0 ) {
  		char tmp[300];
  		memset(tmp,0x00,sizeof(tmp));
  		sprintf(tmp,"%s","<option selected value=\"0\">AP</option>");
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)// keith. disabled if no this mode in 96c


#else

#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
	if(opmode == BRIDGE_MODE && val == CLIENT_MODE)
	{
   		strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
	}
	else
	{

	}
#else
   	  strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
#endif //#if defined(CONFIG_POCKET_ROUTER_SUPPORT)

#endif

#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_WDS_SUPPORT)// keith. disabled if no this mode in 96c
#else
   	  strcat(tmp,"<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"    );
#endif

#ifdef CONFIG_RTL_P2P_SUPPORT
   	  strcat(tmp,"<option value=\"8\">P2P</option> ");
#endif
      return web_write_chunked(tmp);
      	  }

	  if ( val == 1 ) {
	  	char tmp[300];
  		memset(tmp,0x00,sizeof(tmp));
  		sprintf(tmp,"%s","<option value=\"0\">AP</option>");
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)// keith. disabled if no this mode in 96c
#else
   	  strcat(tmp,"<option selected value=\"1\"><script>dw(wlbasic_client)</script></option>");
#endif

#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_WDS_SUPPORT)// keith. disabled if no this mode in 96c
#else
   	  strcat(tmp,"<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"     );
#endif
#ifdef CONFIG_RTL_P2P_SUPPORT
   	  strcat(tmp,"<option value=\"8\">P2P</option> ");
#endif

      return web_write_chunked(tmp);
      	  }

	  if ( val == 2 ) {
		char tmp[300];
  		memset(tmp,0x00,sizeof(tmp));
  		sprintf(tmp,"%s","<option value=\"0\">AP</option>");
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)// keith. disabled if no this mode in 96c
#else

#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
	if(opmode == BRIDGE_MODE && val == CLIENT_MODE)
	{
   	  strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
	}
	else
	{

	}
#else
	strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
#endif //#if defined(CONFIG_POCKET_ROUTER_SUPPORT)

#endif

#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_WDS_SUPPORT)// keith. disabled if no this mode in 96c
#else
   	  strcat(tmp,"<option selected value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"    );
#endif
#ifdef CONFIG_RTL_P2P_SUPPORT
   	  strcat(tmp,"<option value=\"8\">P2P</option> ");
#endif
      return web_write_chunked(tmp);
   	  }
	  if ( val == 3 ) {
		char tmp[300];
  		memset(tmp,0x00,sizeof(tmp));
  		sprintf(tmp,"%s","<option value=\"0\">AP</option>");
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)// keith. disabled if no this mode in 96c
#else

#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
	if(opmode == BRIDGE_MODE && val == CLIENT_MODE)
	{
   	  strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
	}
	else
	{

	}
#else
	strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
#endif //#if defined(CONFIG_POCKET_ROUTER_SUPPORT)

#endif

#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_WDS_SUPPORT)// keith. disabled if no this mode in 96c
#else
   	  strcat(tmp,"<option value=\"2\">WDS</option>"
   	  	 "<option selected value=\"3\">AP+WDS</option>"   );
#endif
#ifdef CONFIG_RTL_P2P_SUPPORT
   	  strcat(tmp,"<option value=\"8\">P2P</option> ");
#endif
      return web_write_chunked(tmp);
   	  } 
#ifdef CONFIG_RTL_P2P_SUPPORT
	  if ( val == 8 ) {
		char tmp[300];
  		memset(tmp,0x00,sizeof(tmp));
  		sprintf(tmp,"%s","<option value=\"0\">AP</option>");
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)// keith. disabled if no this mode in 96c			 	
#else			 	

#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
	if(opmode == BRIDGE_MODE && val == CLIENT_MODE)
	{
   	  strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
	}
	else
	{

	}
#else
	strcat(tmp,"<option value=\"1\"><script>dw(wlbasic_client)</script></option>");
#endif //#if defined(CONFIG_POCKET_ROUTER_SUPPORT)

#endif   	  	 

#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_WDS_SUPPORT)// keith. disabled if no this mode in 96c
#else
   	  strcat(tmp,"<option value=\"2\">WDS</option>"
   	  	 "<option value=\"3\">AP+WDS</option>"   );
#endif

   	  strcat(tmp,"<option selected value=\"8\">P2P</option> ");

      return web_write_chunked(tmp);
   	  }
#endif	  
	  else
   	  	return 0;
#endif
}

#endif




int isDhcpClientExist(char *interface)
{
	struct in_addr intaddr;
	if( getInAddr(interface, IP_ADDR, (void *)&intaddr ) )
	{
		if(intaddr.s_addr!=0)
			return 1;
	}
	return 0;
}



void inline setWlanIdx(int index)
{
//#ifdef CONFIG_RTL_92D_SUPPORT
	if(apmib_get_wlanidx()!=index && index<NUM_WLAN_INTERFACE)
		apmib_set_wlanidx(index);
//#endif
}
/*
  * getIndex 
  * used to Get Index such as wlan_num wan_num ... etc
  * if the return value be parse or indicats a number or bool 
  * please using getIndex function
  * remember to use strncmp
  */

int getIndex(int argc, char ** argv)
{
	char *name;
	char * tmpbuf = malloc(TMP_BUF_SIZE);
	if(tmpbuf == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
        return -1;
	}
	memset(tmpbuf,0,TMP_BUF_SIZE);
	char * buffer = malloc(TMP_BUF_SIZE);
	if(buffer == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
		free(tmpbuf);
        return -1;
	}
	memset(buffer,0,TMP_BUF_SIZE);
	DHCP_T dhcp;
	int	ret=0;
	int intVal=0;
	name=argv[0];
	
	
	if( !strncmp(name, "no-cache",strlen("no-cache"))) {
		sprintf(tmpbuf, "%s\n%s\n%s\n",
			"<meta http-equiv=\"Pragma\" content=\"no-cache\">",
			"<meta HTTP-equiv=\"Cache-Control\" content=\"no-cache\">",
			"<meta HTTP-EQUIV=\"Expires\" CONTENT=\"Mon, 01 Jan 1990 00:00:01 GMT\">");
		ret=strlen(tmpbuf);
		
	}//added start by winfred_wang
	else if(!strcmp(name,"isPureAP")){
	#if defined(CONFIG_ECOS_AP_SUPPORT) 
		sprintf(buffer, "%d", 1) ;
	#else 
		sprintf(buffer, "%d", 0) ;
	#endif
	ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", buffer);
	}
	else if(!strcmp(name,"is97F")){
	#if defined(CONFIG_RTL_8197F) 
		sprintf(buffer, "%d", 1) ;
	#else 
		sprintf(buffer, "%d", 0) ;
	#endif
	ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", buffer);
	}
	else if(!strcmp(name,"logEnabled")){
		if(!apmib_get(MIB_SCRLOG_ENABLED,(void *)&intVal))
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
		}
	else if(!strcmp(name,"rtLogEnabled")){
		if(!apmib_get(MIB_REMOTELOG_ENABLED,(void *)&intVal))
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
		}	
	else if(!strcmp(name,"dosEnabled")){
#ifdef DOS_SUPPORT
		if(!apmib_get(MIB_DOS_ENABLED,(void *)&intVal))
			goto GET_INDEX_FAIL;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
		}
	else if(!strcmp(name,"doson")){
#ifdef DOS_SUPPORT
		intVal=1;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
		}
	else if ( !strcmp(name, "wlanMode")) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "stp")) {
   		if ( !apmib_get( MIB_STP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}	
	else if ( !strcmp(name, "lan_dhcpc_enable")) {
#ifdef HOME_GATEWAY		
   		if ( !apmib_get(MIB_OP_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		if((intVal == GATEWAY_MODE) || (intVal == WISP_MODE))
		{//have wan
			#if 0
			if ( !apmib_get(MIB_WAN_DHCP, (void *)&intVal) )
				goto GET_INDEX_FAIL;
			if(intVal==DHCP_CLIENT)
				intVal=0;
			else
				intVal=1;
			#endif
			intVal=0;
		}
		else
#endif
			intVal=1;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "lan_dhcpc_auto_enable")) {
#ifdef DHCP_AUTO_SUPPORT		
   		intVal=1;
#else
		intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}	
	else if( !strncmp(name, "wlan_num",strlen("wlan_num"))) {
		intVal=getWlanNum();
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if( !strncmp(name, "wlan_support_92D",strlen("wlan_support_92D"))) {
#if defined(CONFIG_RTL_92D_SUPPORT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "wlan_support_8881A",strlen("wlan_support_8881A"))) {
#if defined(CONFIG_RTL_8881A_SUPPORT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "wlanBand2G5GSelect",strlen("wlanBand2G5GSelect"))) {
		if ( !apmib_get( MIB_WLAN_BAND2G5G_SELECT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if( !strncmp(name, "wlan_support_92D_concurrent",strlen("wlan_support_92D_concurrent")))
	{
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)	//92D + 92C
	sprintf(buffer, "%d", 2) ;
#else
	#if defined(CONFIG_RTL_92D_SUPPORT) //92D
		sprintf(buffer, "%d", 1) ;
	#else //92C
		sprintf(buffer, "%d", 0) ;
	#endif
#endif
	ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", buffer);
	}
	else if( !strncmp(name, "wlan_dual_band",strlen("wlan_dual_band")))
	{
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if( !strncmp(name, "wlanNetWorkType",strlen("wlanNetWorkType"))) {
		
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if( !strncmp(name, "isWDSDefined",strlen("isWDSDefined"))) {
#ifdef CONFIG_WLAN_WDS_SUPPORT
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "isMeshDefined",strlen("isMeshDefined"))) {
#ifdef CONFIG_RTK_MESH
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "isNewMeshUI",strlen("isNewMeshUI"))) {
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	}
	else if( !strcmp(name, "isSupportWlanSch")) {
#ifdef HAVE_WLAN_SCHEDULE
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "isP2PSupport",strlen("isP2PSupport"))) {
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	}	
	else if( !strncmp(name, "rtl8021x_client_support",strlen("rtl8021x_client_support")))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	}
	else if( !strncmp(name, "fw_support",strlen("fw_support")))
	{
	#ifdef HAVE_FIREWALL
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
	#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	#endif
	}
    else if( !strncmp(name, "ipfilter_range_enable",strlen("ipfilter_range_enable")))
	{
	#ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
	#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	#endif
	}
	else if(!strncmp(name,"pptp_comment_start",strlen("pptp_comment_start")))
	{
#ifdef HAVE_PPTP
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"<!--");
#endif
	}
	else if(!strncmp(name,"pptp_comment_end",strlen("pptp_comment_end")))
	{
#ifdef HAVE_PPTP
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "-->");
#endif
	}
	else if(!strncmp(name,"DHCP_for_tr069_comment_start",strlen("DHCP_for_tr069_comment_start")))
	{
#ifdef CONFIG_TR069
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"<!--");
#endif
	}
	else if(!strncmp(name,"DHCP_for_tr069_comment_end",strlen("DHCP_for_tr069_comment_end")))
	{
#ifdef CONFIG_TR069
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"-->");
#endif
	}
	else if(!strncmp(name,"l2tp_comment_start",strlen("l2tp_comment_start")))
	{
#ifdef HAVE_L2TP
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "<!--");
#endif
	}
	else if(!strncmp(name,"l2tp_comment_end",strlen("l2tp_comment_end")))
	{
#ifdef HAVE_L2TP
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "-->");
#endif
	}
	else if( !strncmp(name, "vlan_support",strlen("vlan_support")))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "vlan_enable",strlen("vlan_enable")))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if ( !apmib_get( MIB_VLAN_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "vlanInterfaceNum",strlen("vlanInterfaceNum")))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if ( !apmib_get( MIB_NETIFACE_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "vlanNum",strlen("vlanNum")))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if ( !apmib_get( MIB_VLAN_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}

	else if( !strncmp(name, "vlan_currentId",strlen("vlan_currentId")))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if ( !apmib_get( MIB_CURRENT_VLAN_ID, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "netIfNum",strlen("netIfNum")))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if ( !apmib_get( MIB_NETIFACE_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if( !strncmp(name, "vlan_wan_support",strlen("vlan_wan_support")))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	}
    else if (!strncmp(name, "bridge_vlan_enable",strlen("bridge_vlan_enable")))
    {
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
        #else
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
        #endif     
    }
    else if (!strncmp(name, "vlan_currentfwdrule",strlen("vlan_currentfwdrule")))
    {
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        if ( !apmib_get( MIB_CURRENT_VLAN_FORWARD_RULE, (void *)&intVal) )
        	goto GET_INDEX_FAIL;
        ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
        #else
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
        #endif
    }
    /*else if (!strncmp(name, "vlan_iswan",strlen("vlan_iswan")))
    {
	#if defined(HAVE_FIREWALL)
	#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
        if ( !apmib_get( MIB_CURRENT_VLAN_ID, (void *)&intVal) )
        	goto GET_INDEX_FAIL;
        if (vlanIDIsWan(intVal) == 1)
            ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
        else    
            ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
        #else
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
        #endif
		#endif
    }*/
	else if( !strncmp(name, "qos_support",strlen("qos_support")))
	{
	#ifdef QOS_BY_BANDWIDTH
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
	#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	#endif
	}
	else if( !strncmp(name, "route_support",strlen("route_support")))
	{
#ifdef ROUTE_SUPPORT
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
#ifdef ROUTE_SUPPORT
	else if(!strcmp(name, "staticRouteNum"))
	{
		if ( !apmib_get( MIB_STATICROUTE_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}
	else if(!strcmp(name, "dynamicRouteEnabled"))
	{
		if ( !apmib_get( MIB_DYNAMICROUTE_ENABLED, (void *)&intVal) )
             goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}	
	else if(!strcmp(name, "nat_enabled"))
	{
		if ( !apmib_get( MIB_NAT_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}
	else if(!strcmp(name, "ripEnabled"))
	{
		if ( !apmib_get( MIB_RIP_ENABLED, (void *)&intVal) )
             goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}
//#ifdef ROUTE6D_SUPPORT
	else if(!strcmp(name, "rip6Enabled"))
	{
		if ( !apmib_get( MIB_RIP6_ENABLED, (void *)&intVal) )
             goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}
//#endif
	else if(!strcmp(name, "rip6Support"))
	{
#ifdef ROUTE6D_SUPPORT
		ret = snprintf(tmpbuf, TMP_BUF_SIZE, "%d", 1);
#else
		ret = snprintf(tmpbuf, TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if(!strcmp(name, "staticRouteEnabled"))
	{
		if ( !apmib_get( MIB_STATICROUTE_ENABLED, (void *)&intVal) )
             goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}
#endif
	else if( !strncmp(name, "snmp_support",strlen("snmp_support")))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	}
	else if( !strcmp(name, "dos_support"))
	{
#ifdef DOS_SUPPORT
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if(!strcmp(name,("dos_jscomment_start")))
	{
#ifdef DOS_SUPPORT

		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
#else

		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","/*");
#endif
	}
	else if(!strcmp(name,("dos_jscomment_end")))
	{
#ifdef DOS_SUPPORT
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","*/");
#endif

	}
	else if( !strcmp(name, "syslog_support"))
	{
#ifdef SYSLOG_SUPPORT
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
#endif
	}
	else if(!strncmp(name, "wan_num",strlen("wan_num")))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
	}
	else if(!strncmp(name, "wlan_idx",strlen("wlan_idx")))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",apmib_get_wlanidx());
	}
	else if(!strncmp(name, "opMode",strlen("opMode")))
	{
		if ( !apmib_get( MIB_OP_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "show_wlan_num"))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", wlan_num);
	}
	else if(!strncmp(name, "WiFiTest",strlen("WiFiTest")))
	{
		if ( !apmib_get( MIB_WIFI_SPECIFIC, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if(!strncmp(name, "wispWanId",strlen("wispWanId")))
	{
		if ( !apmib_get( MIB_WISP_WAN_ID, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}

	else if(!strncmp(name, "ntpEnabled",strlen("ntpEnabled")))
	{
		if ( !apmib_get( MIB_NTP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
#ifdef CONFIG_RTK_MESH
	else if ( !strcmp(name, "wlanMeshEnabled")) {
		//new feature:Mesh enable/disable
		if ( !apmib_get( MIB_WLAN_MESH_ENABLE, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshRootEnabled")) {
		if ( !apmib_get( MIB_WLAN_MESH_ROOT_ENABLE, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshEncrypt")) {
		if ( !apmib_get( MIB_WLAN_MESH_ENCRYPT, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshPskFormat")) {
		if ( !apmib_get( MIB_WLAN_MESH_PSK_FORMAT, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshPskValue")) {
		int i;
		buffer[0]='\0';
		if ( !apmib_get(MIB_WLAN_MESH_WPA_PSK,	(void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		for (i=0; i<strlen(buffer); i++)
			buffer[i]='*';
		buffer[i]='\0';
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);
	}
	else if ( !strcmp(name, "meshWpaAuth")) {
		if ( !apmib_get( MIB_WLAN_MESH_WPA_AUTH, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshWpa2Cipher")) {
		if ( !apmib_get( MIB_WLAN_MESH_WPA2_CIPHER_SUITE, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
#ifdef _MESH_ACL_ENABLE_
	else if ( !strcmp(name, "meshAclNum")) {
		if ( !apmib_get( MIB_WLAN_MESH_ACL_NUM, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshAclEnabled")) {
		if ( !apmib_get( MIB_WLAN_MESH_ACL_ENABLED, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
#endif
#endif // CONFIG_RTK_MESH
		//indispensable!! MESH related , no matter mesh enable or not
	else if ( !strcmp(name, "isMeshDefined")) {
#ifdef CONFIG_RTK_MESH
		intVal = 1;
#else
		intVal = 0;
#endif
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);

	}

	else if ( !strcmp(name, "isNewMeshUI")) {
#ifdef CONFIG_NEW_MESH_UI
		intVal = 1;
#else
		intVal = 0;
#endif
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if(!strncmp(name, "DaylightSave",strlen("DaylightSave")))
	{
		if ( !apmib_get( MIB_DAYLIGHT_SAVE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
#ifdef HOME_GATEWAY

	else if ( !strcmp(name, "portFwEnabled")) {
		if ( !apmib_get( MIB_PORTFW_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "portFwNum")) {
	if ( !apmib_get( MIB_PORTFW_TBL_NUM, (void *)&intVal) )
		goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
#endif	
	else if ( !strcmp(name, "ntpServerId")) {
   		if ( !apmib_get( MIB_NTP_SERVER_ID, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}

	else if ( !strcmp(name, "languageType")) {
   		if ( !apmib_get( MIB_WEB_LANGUAGE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "dhcp-current")) {
   		if ( !apmib_get( MIB_DHCP, (void *)&dhcp) )
			goto GET_INDEX_FAIL;
		if ( dhcp == DHCP_CLIENT && !isDhcpClientExist(BRIDGE_IF))
			dhcp = DHCP_LAN_NONE;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",(int)dhcp);
		
	}
	else if(!strcmp(name,"multi_language_support"))
	{
#ifdef HAVE_MULTI_LANGUAGE
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"1");
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"0");
#endif
	
	}
	else if(!strcmp(name,"multi_language_en"))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",ENGLISH);	
	}
	else if(!strcmp(name,"multi_language_sc"))
	{
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",SIMPLE_CHINESE);	
	}
	else if(!strncmp(name, "wlan1_phyband",strlen("wlan1_phyband")))
	{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined (CONFIG_RTL_8881A_SELECTIVE)
				int wlanBand2G5GSelect;
				int val=0;
				if(!apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect))
					goto GET_INDEX_FAIL;
				bzero(tmpbuf,sizeof(tmpbuf));
#ifdef CONFIG_WLANIDX_MUTEX
				int s = apmib_save_idx();
#else
				apmib_save_idx();
#endif				
				if(SetWlan_idx("wlan0"))
				{
					if(!apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&val))
						goto GET_INDEX_FAIL;
					if(val == PHYBAND_5G && (wlanBand2G5GSelect==BANDMODE5G || wlanBand2G5GSelect==BANDMODEBOTH || wlanBand2G5GSelect==BANDMODESINGLE))
						ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","5GHz");
					else if(val == PHYBAND_2G && (wlanBand2G5GSelect==BANDMODE2G || wlanBand2G5GSelect==BANDMODEBOTH || wlanBand2G5GSelect==BANDMODESINGLE))
						ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","2.4GHz");
					else
						ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
				}
#ifdef CONFIG_WLANIDX_MUTEX
				apmib_revert_idx(s);
#else
				apmib_revert_idx();
#endif
#else
				ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
#endif
//	printf("%s:%d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);

	}
	else if (!strncmp(name, "wlan2_phyband",strlen("wlan2_phyband")))
	{
#if defined(CONFIG_RTL_92D_SUPPORT) //|| defined (CONFIG_RTL_8881A_SELECTIVE)
		int wlanBand2G5GSelect;
		int val=0;
		if(!apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect))
			goto GET_INDEX_FAIL;
		bzero(tmpbuf,sizeof(tmpbuf));
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		if(SetWlan_idx("wlan1"))
		{
			if(!apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&val))
				goto GET_INDEX_FAIL;
			
//			printf("%s:%d wlanBand2G5GSelect=%d val=%d\n",__FUNCTION__,__LINE__,wlanBand2G5GSelect,val);
			if(val == PHYBAND_5G && (wlanBand2G5GSelect==BANDMODE5G || wlanBand2G5GSelect==BANDMODEBOTH))
				ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","5GHz");
			else if(val == PHYBAND_2G && (wlanBand2G5GSelect==BANDMODE2G || wlanBand2G5GSelect==BANDMODEBOTH))
				ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","2.4GHz");
			else
				ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
		}
		//printf("%s:%d wlanBand2G5GSelect=%d val=%d\n",__FUNCTION__,__LINE__,wlanBand2G5GSelect,val);
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif

#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
#endif
//	printf("%s:%d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);

	}
	/*************		lan interface	**************/
	else if(!strncmp(name,"lanDhcpMode",strlen("lanDhcpMode")))
	{
		if ( !apmib_get( MIB_DHCP, (void *)&intVal) )
			goto GET_INDEX_FAIL;		
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
	else if(!strcmp(name ,"dhcpLeaseTime"))
	{
		if ( !apmib_get( MIB_DHCP_LEASE_TIME, (void *)&intVal) )
			goto GET_INDEX_FAIL;		
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
	else if(!strcmp(name ,"static_dhcp_onoff"))
	{
		if ( !apmib_get( MIB_DHCPRSVDIP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;		
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
/**************		wan interface	*****************/
	else if( !strncmp(name, "wanDhcp",strlen("wanDhcp")))
	{
		//printf("%s:%d tmpbuf=%d\n",__FILE__,__LINE__,intVal);
#ifdef CONFIG_ECOS_AP_SUPPORT
		intVal = 0;
#else
		if ( !apmib_get( MIB_WAN_DHCP, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#endif
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);		
	}
	else if( !strncmp(name, "wanDNS",strlen("wanDNS")))
	{
#ifdef CONFIG_ECOS_AP_SUPPORT
			intVal = 0;
#else
		if ( !apmib_get( MIB_DNS_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#endif
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if(!strcmp(name, "ipv6")){
#ifdef CONFIG_IPV6
		intVal = 1;		
#else
		intVal = 0;
#endif	
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
#ifdef HOME_GATEWAY

	else if( !strncmp(name, "tr069DhcpEnabled",strlen("tr069DhcpEnabled")))
	{
#ifdef CONFIG_TR069
		if ( !apmib_get( MIB_PPPOE_DHCP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#else
		intVal = 0;
#endif
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}	
	else if( !strncmp(name, "upnpEnabled",strlen("upnpEnabled")))
	{
		if ( !apmib_get( MIB_UPNP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "igmpproxyDisabled",strlen("igmpproxyDisabled")))
	{
		if ( !apmib_get( MIB_IGMP_PROXY_DISABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "pingWanAccess",strlen("pingWanAccess")))
	{
		if ( !apmib_get( MIB_PING_WAN_ACCESS_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "webWanAccess",strlen("webWanAccess")))
	{
		if ( !apmib_get( MIB_WEB_WAN_ACCESS_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	
	#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
	else if ( !strncmp(name, "webAccessPort", strlen("webAccessPort"))) {
		if ( !apmib_get( MIB_WEB_WAN_ACCESS_PORT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
	else if ( !strncmp(name, "wanAccessPortEnable", strlen("wanAccessPortEnable"))) {
		#ifdef CONFIG_RTL_WEB_WAN_ACCESS_PORT
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
		#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
		#endif

	}
	#endif
	else if( !strncmp(name, "VPNPassThruIPsec",strlen("VPNPassThruIPsec")))
	{
		if ( !apmib_get( MIB_VPN_PASSTHRU_IPSEC_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "VPNPassThruPPTP",strlen("VPNPassThruPPTP")))
	{
		if ( !apmib_get( MIB_VPN_PASSTHRU_PPTP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "VPNPassThruL2TP",strlen("VPNPassThruL2TP")))
	{
		if ( !apmib_get( MIB_VPN_PASSTHRU_L2TP_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "ipv6passthrouh",strlen("ipv6passthrouh")))
	{
		if ( !apmib_get( MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
#endif
    else if (!strncmp(name, "net_sniper_enable",strlen("net_sniper_enable")))
    {
        #ifdef CONFIG_RTL_NETSNIPER_SUPPORT
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
        #else
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
        #endif     
    }
    else if (!strncmp(name, "net_sniper_wantype_enable",strlen("net_sniper_wantype_enable")))
    {
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",1);
        #else
        ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
        #endif     
    }
    else if( !strncmp(name, "netsniper",strlen("netsniper")))
	{
	    #ifdef CONFIG_RTL_NETSNIPER_SUPPORT
		if ( !apmib_get( MIB_NET_SNIPER_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
        #else        
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",0);
        #endif
	}
	else if( !strcmp(name, "enableGetServIpByDomainName"))
	{
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",1);
#else
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",0);
#endif
	}
#ifdef HOME_GATEWAY
	else if( !strcmp(name, "l2tpGetServIpByDomainName"))
	{
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
		if(!apmib_get(MIB_L2TP_GET_SERV_BY_DOMAIN,(void*)&intVal))
			goto GET_INDEX_FAIL;
#else
		intVal=0;
#endif
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
	else if( !strcmp(name, "pptpGetServIpByDomainName"))
	{
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
		if(!apmib_get(MIB_PPTP_GET_SERV_BY_DOMAIN,(void*)&intVal))
			goto GET_INDEX_FAIL;
#else
		intVal=0;
#endif
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}	
	else if( !strncmp(name, "pptpSecurity",strlen("pptpSecurity")))
	{
		if ( !apmib_get( MIB_PPTP_SECURITY_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "pptpCompress",strlen("pptpCompress")))
	{
		if ( !apmib_get( MIB_PPTP_MPPC_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}	
	else if( !strncmp(name, "fixedIpMtuSize",strlen("fixedIpMtuSize")))
	{
		if ( !apmib_get( MIB_FIXED_IP_MTU_SIZE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "dhcpMtuSize",strlen("dhcpMtuSize")))
	{
		if ( !apmib_get( MIB_DHCP_MTU_SIZE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "pppConnectType",strlen("pppConnectType")))
	{
		if ( !apmib_get( MIB_PPP_CONNECT_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "wan-ppp-idle",strlen("wan-ppp-idle")))
	{
		if ( !apmib_get( MIB_PPP_IDLE_TIME, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "pppMtuSize",strlen("pppMtuSize")))
	{
		if ( !apmib_get( MIB_PPP_MTU_SIZE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "pptpConnectType",strlen("pptpConnectType")))
	{
		if ( !apmib_get( MIB_PPTP_CONNECTION_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "wan-pptp-idle",strlen("wan-pptp-idle")))
	{
		if ( !apmib_get( MIB_PPTP_IDLE_TIME, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "pptpMtuSize",strlen("pptpMtuSize")))
	{
		if ( !apmib_get( MIB_PPTP_MTU_SIZE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if ( !strcmp(name, ("pptp_wan_ip_mode"))) {
#if defined(CONFIG_DYNAMIC_WAN_IP)
			int wanIpType;
			if ( !apmib_get( MIB_PPTP_WAN_IP_DYNAMIC,  (void *)&wanIpType) )
				goto GET_INDEX_FAIL;
			ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",wanIpType);
#else
			ret= snprintf(tmpbuf,TMP_BUF_SIZE, "0");
#endif
		}

	else if ( !strcmp(name, ("l2tp_wan_ip_mode"))) {
#if defined(CONFIG_DYNAMIC_WAN_IP)
		int wanIpType;
		if ( !apmib_get( MIB_L2TP_WAN_IP_DYNAMIC,  (void *)&wanIpType) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",wanIpType);
#else
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "0");
#endif
	}

	else if( !strncmp(name, "l2tpConnectType",strlen("l2tpConnectType")))
	{
		if ( !apmib_get( MIB_L2TP_CONNECTION_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "wan-l2tp-idle",strlen("wan-l2tp-idle")))
	{
		if ( !apmib_get( MIB_L2TP_IDLE_TIME, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if( !strncmp(name, "l2tpMtuSize",strlen("l2tpMtuSize")))
	{
		if ( !apmib_get( MIB_L2TP_MTU_SIZE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if(!strncmp(name,"ddnsEnabled",strlen("ddnsEnabled")))
	{
		if(!apmib_get(MIB_DDNS_ENABLED,(void *)&intVal))
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	//	printf("%s:%d ddneEnabled=%d\n",__FILE__,__LINE__,intVal);
	}
	else if(!strncmp(name,"ddnsType",strlen("ddnsType")))
	{
		if(!apmib_get(MIB_DDNS_TYPE,(void *)&intVal))
			goto GET_INDEX_FAIL;
		ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "ipFilterEnabled")) {
		if ( !apmib_get( MIB_IPFILTER_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "portFilterEnabled")) {
		if ( !apmib_get( MIB_PORTFILTER_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "macFilterEnabled")) {
		if ( !apmib_get( MIB_MACFILTER_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "urlFilterEnabled")) {
		if ( !apmib_get( MIB_URLFILTER_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "ipFilterNum")) {
		if ( !apmib_get( MIB_IPFILTER_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "portFilterNum")) {
		if ( !apmib_get( MIB_PORTFILTER_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "macFilterNum")) {
		if ( !apmib_get( MIB_MACFILTER_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "urlFilterNum")) {
		if ( !apmib_get( MIB_URLFILTER_TBL_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
#if defined(QOS_BY_BANDWIDTH)
		else if ( !strcmp(name, "qosEnabled")) {
			if ( !apmib_get( MIB_QOS_ENABLED, (void *)&intVal) )
				goto GET_INDEX_FAIL;
			ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);			
		}
		else if ( !strcmp(name, "qosMode")) {
			if ( !apmib_get( MIB_QOS_MODE, (void *)&intVal) )
				goto GET_INDEX_FAIL;
			ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);			
		}
		else if ( !strcmp(name, "qosManualUplinkSpeed")) {
			if ( !apmib_get( MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&intVal) )
				goto GET_INDEX_FAIL;
			ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);				
		}
		else if ( !strcmp(name, "qosRuleNum")) {
			if ( !apmib_get( MIB_QOS_RULE_TBL_NUM, (void *)&intVal) )
				goto GET_INDEX_FAIL;
			ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);			
		}
		else if ( !strcmp(name, "qosManualDownlinkSpeed")) {
			if ( !apmib_get( MIB_QOS_MANUAL_DOWNLINK_SPEED, (void *)&intVal) )
				goto GET_INDEX_FAIL;			
			ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);			
		}
#endif
#ifdef DOS_SUPPORT
		else if(!strncmp(name,"syssynFlood",strlen("syssynFlood"))){
			if(!apmib_get(MIB_DOS_SYSSYN_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"sysfinFlood",strlen("sysfinFlood"))){
			if(!apmib_get(MIB_DOS_SYSFIN_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"sysudpFlood",strlen("sysudpFlood"))){
			if(!apmib_get(MIB_DOS_SYSUDP_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"sysicmpFlood",strlen("sysicmpFlood"))){
			if(!apmib_get(MIB_DOS_SYSICMP_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"pipsynFlood",strlen("pipsynFlood"))){
			if(!apmib_get(MIB_DOS_PIPSYN_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"pipfinFlood",strlen("pipfinFlood"))){
			if(!apmib_get(MIB_DOS_PIPFIN_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"pipudpFlood",strlen("pipudpFlood"))){
			if(!apmib_get(MIB_DOS_PIPUDP_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"pipicmpFlood",strlen("pipicmpFlood"))){
			if(!apmib_get(MIB_DOS_PIPICMP_FLOOD,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
		else if(!strncmp(name,"blockTime",strlen("blockTime"))){
			if(!apmib_get(MIB_DOS_BLOCK_TIME,(void *)&intVal))
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
			}
#endif

#else
		else if ( !strcmp(name, ("pptp_wan_ip_mode"))) 
		{
			ret= snprintf(tmpbuf,TMP_BUF_SIZE, "0");
		}
		else if ( !strcmp(name, ("l2tp_wan_ip_mode"))) 
		{
			ret= snprintf(tmpbuf,TMP_BUF_SIZE, "0");
		}

#endif
#ifndef HAVE_NOWIFI
	else if(!strcmp(name,"wlan_idx")) {
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",apmib_get_wlanidx());
	}
	else if(!strcmp(name,"Band2G5GSupport")) {
	#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		intVal = get_gpio_2g5g();	
		if (intVal == 2) intVal = 1;
		if (intVal == 5) intVal = 2;
	#else
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
	#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"dsf_enable")) {
#if defined(CONFIG_RTL_DFS_SUPPORT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif		
	}
	else if(!strcmp(name,"wlanBand2G5GSelect")) {
		apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)intVal) ;
	}
	else if(!strcmp(name,"regDomain")) {
		if ( !apmib_get( MIB_HW_REG_DOMAIN, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)intVal);
	}
	else if(!strcmp(name,"country_str")) {
		apmib_get(MIB_WLAN_COUNTRY_STRING, (void *)tmpbuf);
		ret=strlen(tmpbuf);		
	}
	else if(!strcmp(name,"RFType")) {
		if ( !apmib_get( MIB_HW_RF_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",(int)intVal) ;	
	}
	else if(!strcmp(name,"wlanMode")) {
		if ( !apmib_get( MIB_WLAN_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
	else if(!strcmp(name,"band")) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_BAND, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)intVal) ;
	}
	else if(!strcmp(name,"networkType")) {
		if ( !apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strncmp(name, "ChannelBonding", strlen("ChannelBonding"))) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if ( strstr(name, "_2")) {
			if ( !apmib_get(MIB_WLAN_CHANNEL_BONDING_2, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		else 
#endif
		{
			if ( !apmib_get(MIB_WLAN_CHANNEL_BONDING, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strcmp(name,"wlan_support_8812e")) {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1) ;
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0) ;
#endif		
	}
	else if(!strcmp(name,"wlan_support_ac2g")) //ac2g
	{
#if defined(CONFIG_RTL_AC2G_256QAM) || defined(CONFIG_WLAN_HAL_8814AE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1) ;
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0) ;
#endif
	}
	else if(!strcmp(name,"regDomain")) {
		if ( !apmib_get( MIB_HW_REG_DOMAIN, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)intVal);	
	}
	else if(!strncmp(name, "channel", strlen("channel"))) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if(strstr(name, "_2"))
		{
			if ( !apmib_get( MIB_WLAN_CHANNEL_2, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}else
#endif
		{
			if ( !apmib_get( MIB_WLAN_CHANNEL, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	
	else if ( !strncmp(name, "operRate", strlen("operRate"))) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if ( strstr(name, "_2")) {
			if ( !apmib_get( MIB_WLAN_SUPPORTED_RATES_2, (void *)&intVal) )
				goto GET_INDEX_FAIL;	
		}
		else 
#endif
		{
			if ( !apmib_get( MIB_WLAN_SUPPORTED_RATES, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strncmp(name, "basicRate", strlen("basicRate"))) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if ( strstr(name, "_2")) {
			if ( !apmib_get( MIB_WLAN_BASIC_RATES_2, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		else 
#endif
		{
		if ( !apmib_get( MIB_WLAN_BASIC_RATES, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}

	else if(!strcmp(name,"rateAdaptiveEnabled")) {
		if ( !apmib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}

	else if ( !strcmp(name, "band_side_2g")) {	
		if ( !apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if(intVal == 2)
			apmib_get(MIB_WLAN_CONTROL_SIDEBAND_2, (void *)&intVal);
#endif
		else if(intVal == 1)
			apmib_get(MIB_WLAN_CONTROL_SIDEBAND, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "band_side_5g")) {	
		if ( !apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal))
			goto GET_INDEX_FAIL;
		if(intVal == 2)
			apmib_get(MIB_WLAN_CONTROL_SIDEBAND, (void *)&intVal);
#ifdef CONFIG_RTL_8881A_SELECTIVE
		else if(intVal == 1)
			apmib_get(MIB_WLAN_CONTROL_SIDEBAND_2, (void *)&intVal);
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "channel_2g")) {	
		if ( !apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal))
			goto GET_INDEX_FAIL;
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if(intVal == 2)
		{
			apmib_get(MIB_WLAN_CHANNEL_2, (void *)&intVal);
		}
#endif
		else if(intVal == 1)
			apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "channel_5g")) {	
		if ( !apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal))
			goto GET_INDEX_FAIL;
		if(intVal == 2)
			apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
#ifdef CONFIG_RTL_8881A_SELECTIVE
		else if(intVal == 1)
			apmib_get(MIB_WLAN_CHANNEL_2, (void *)&intVal);
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}

	else if(!strcmp(name,"fixTxRate")) {
		if ( !apmib_get( MIB_WLAN_FIX_RATE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;	
	}
	else if(!strcmp(name,"rf_used")) {
#if 1	/*FIXME!!!*/		
		struct _misc_data_ misc_data;
		if (getMiscData(WLAN_IF, &misc_data) < 0)
		{
			intVal = 0;
		}
		else
		{
			intVal = misc_data.mimo_tr_used;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);		
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1) ;
#endif
	}
#if 1 //defined(CONFIG_RTL_VAP_SUPPORT)
	else if(!strcmp(name,"wlan_mssid_num")) {
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", DEF_MSSID_NUM);
	}
#endif
	else if(!strcmp(name,"WiFiTest")) {
		apmib_get(MIB_WIFI_SPECIFIC, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strcmp(name,"2G_ssid")) {
		short wlanif;
		unsigned char wlanIfStr[10];
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		wlanif = whichWlanIfIs(PHYBAND_2G);

		if(wlanif >= 0)
		{
			memset(wlanIfStr,0x00,sizeof(wlanIfStr));
			sprintf((char *)wlanIfStr, "wlan%d",wlanif);

			if(SetWlan_idx((char *)wlanIfStr))
			{
				apmib_get(MIB_WLAN_SSID, (void *)buffer);
			}
			
		}
		else
		{
			;//ssid is empty
		}
#ifdef CONFIG_RTL_8812_SUPPORT
		apmib_get(MIB_WLAN_SSID, (void *)buffer);
#endif
		translate_control_code(buffer);
		
		cyg_httpd_write_chunked(buffer,strlen(buffer));	
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
		free(tmpbuf);
		free(buffer);
		return 0;		
	}
	else if(!strcmp(name,"5G_ssid")) {
		short wlanif;
		unsigned char wlanIfStr[10];
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		wlanif = whichWlanIfIs(PHYBAND_5G);

		if(wlanif >= 0)
		{
			memset(wlanIfStr,0x00,sizeof(wlanIfStr));
			sprintf((char *)wlanIfStr, "wlan%d",wlanif);

			if(SetWlan_idx((char *)wlanIfStr))
			{
				apmib_get(MIB_WLAN_SSID, (void *)buffer);
			}
		}
		else
		{
			;//ssid is empty
		}
#ifdef CONFIG_RTL_8812_SUPPORT
		apmib_get(MIB_WLAN_SSID, (void *)buffer);
#endif
		translate_control_code(buffer);
		//after translate control code, the ssid length maybe larger than 33.(max=32*6)
		cyg_httpd_write_chunked(buffer, strlen(buffer));
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
		free(tmpbuf);
		free(buffer);
		return 0;		
	}
	else if(!strcmp(name,"networkType")) {
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		sprintf(buffer,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(buffer);
		apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);
		sprintf(buffer,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(buffer);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif		
	}
	else if(!strcmp(name,"encrypt")) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_ENCRYPT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
#ifdef WIFI_SIMPLE_CONFIG
	else if ( !strcmp(name, "wscDisable")) {
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "wscConfig")) {
		apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "wscRptConfig"))
	{
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		sprintf(buffer,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(buffer);
		apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&intVal);
		sprintf(buffer, "%d", intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);
		sprintf(buffer,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(buffer);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if ( !strcmp(name, "wscRunon"))
	{
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		apmib_get(MIB_WSC_INTF_INDEX, (void *)&intVal);
		sprintf(buffer, "%d", intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if ( !strcmp(name, "wps_by_reg")) {
		apmib_get(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "wps_auth")) {
		apmib_get(MIB_WLAN_WSC_AUTH, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "wpsRpt_auth")) {
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		sprintf(buffer,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(buffer);
		apmib_get(MIB_WLAN_WSC_AUTH, (void *)&intVal);
		sprintf(buffer, "%d", intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);
		sprintf(buffer,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(buffer);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if ( !strcmp(name, "wps_enc")) {
		apmib_get(MIB_WLAN_WSC_ENC, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if ( !strcmp(name, "wpsRpt_enc")) {
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		sprintf(buffer,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(buffer);
		apmib_get(MIB_WLAN_WSC_ENC, (void *)&intVal);
		sprintf(buffer, "%d", intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);
		sprintf(buffer,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(buffer);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
#endif // WIFI_SIMPLE_CONFIG
	else if(!strcmp(name,"wlanMode")) {
		if ( !apmib_get( MIB_WLAN_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);		
	}
	else if(!strcmp(name,"wps_by_reg")) {
		apmib_get(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);		
	}
	else if(!strcmp(name,"encrypt")) {
		if ( !apmib_get( MIB_WLAN_ENCRYPT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);	
	}
	else if(!strcmp(name,"enable1X")) {
		if ( !apmib_get( MIB_WLAN_ENABLE_1X, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strcmp(name,"wpaAuth")) {
		if ( !apmib_get( MIB_WLAN_WPA_AUTH, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strcmp(name,"wmmEnabled")) {
		if ( !apmib_get(MIB_WLAN_WMM_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}	
	else if(!strcmp(name,"encrypt")) {
		if ( !apmib_get( MIB_WLAN_ENCRYPT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strcmp(name,"wpaCipher")) {
		if ( !apmib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}	
	else if(!strcmp(name,"wpa2Cipher")) {
		if ( !apmib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strncmp(name, "ControlSideBand", strlen("ControlSideBand"))) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if ( strstr(name, "_2")) {
				if ( !apmib_get(MIB_WLAN_CONTROL_SIDEBAND_2, (void *)&intVal) )
					goto GET_INDEX_FAIL;	
		}
		else 
#endif
		{
			if ( !apmib_get(MIB_WLAN_CONTROL_SIDEBAND, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strcmp(name,"hiddenSSID")) {
		if ( !apmib_get( MIB_WLAN_HIDDEN_SSID, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}	
	else if(!strcmp(name,"isRepeaterDisplay")) {
#if !defined(UNIVERSAL_REPEATER) || defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)// keith. disabled if no this mode in 96c
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#endif	
	}
	else if(!strcmp(name,"wlanMacClone")) {
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_MACCLONE_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);		
	}
	else if ( !strcmp(name, "isWispDisplay")) {
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_CLIENT_MODE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);		
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);		
#endif
	}
#ifdef UNIVERSAL_REPEATER
	else if ( !strcmp(name, "multiRepeaterEnabled")) {
#if defined(RTL_MULTI_REPEATER_MODE_SUPPORT)
        ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", 1);
#else
        ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif				
	}
	else if ( !strcmp(name, "multiAPRepeaterStr")) {
#if defined(RTL_MULTI_REPEATER_MODE_SUPPORT) 
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal); 
		if(1==intVal) //now only 92e support it ,so it only stay in 2.4G
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "MultipleAP-MultipleRepeater");
		else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "MultipleAP");	
#else	
        ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "MultipleAP");
#endif
		
	}   
	else if ( !strcmp(name, "repeaterEnabled")) {
		int id;
		if (0 == apmib_get_wlanidx())
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		if ( !apmib_get( id, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if ( !strcmp(name, "isRepeaterEnabled")) {
#if 1
		int val, intVal2;
		if (0 == apmib_get_wlanidx())
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&intVal);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&intVal);

		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal2);
		apmib_get(MIB_WLAN_MODE, (void *)&val);

		if (intVal != 0 &&  intVal != 2 && val != WDS_MODE && !(val==CLIENT_MODE && intVal2==ADHOC))
		{
			val = 1;
		}
		else
		{
			val = 0;
		}

#else
		if (wlan_idx == apmib_get_wlanidx())
			strcpy(buffer, "wlan0-vxd");
		else
			strcpy(buffer, "wlan1-vxd");
		if ( isVxdInterfaceExist(buffer))
			val = 1;
		else
			val = 0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", val);
	}
	else if ( !strcmp(name, "repeaterMode")) {
		if ( !apmib_get( MIB_WLAN_MODE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		if (intVal == AP_MODE || intVal == AP_WDS_MODE)
			intVal = CLIENT_MODE;
		else
			intVal = AP_MODE;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);		
	}
#endif // UNIVERSAL_REPEATER

	else if(!strcmp(name,"wlanDisabled")) {
		if ( !apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);	
	}
	else if(!strcmp(name,"wlan_num")) {
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", wlan_num);		
	}
	/*****advanced page*****/
	else if(!strcmp(name ,"wlan_interface_92D"))
	{
#if defined(CONFIG_RTL_92D_SUPPORT)//support 92d
	#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)//support 92C+92D
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
	if((int)intVal == 2)//5G 92D
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1) ;
	else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0) ;
	#else//only support 92D
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", 1) ;
	#endif
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", 0) ;
#endif
	}
	/* exist	
	else if(!strcmp(name ,"rateAdaptiveEnabled"))
	{
	}
	*/
	else if(!strcmp(name ,"aggregation"))
	{
		if ( !apmib_get(MIB_WLAN_AGGREGATION, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strcmp(name ,"preamble"))
	{
		if ( !apmib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
	}
	else if(!strcmp(name ,"iappDisabled"))
	{
		if ( !apmib_get( MIB_WLAN_IAPP_DISABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strcmp(name ,"protectionDisabled"))
	{
		if ( !apmib_get( MIB_WLAN_PROTECTION_DISABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);	
	}
#ifdef HAVE_HS2_SUPPORT
   else if ( !strcmp(name, "hs2Enabled")) {
       if ( !apmib_get( MIB_WLAN_HS2_ENABLE, (void *)&intVal) )
           goto GET_INDEX_FAIL;
	   ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);
   }
#else
   else if ( !strcmp(name, "hs2Enabled")) {
       ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 2);
   }
#endif
	else if(!strcmp(name ,"shortGIEnabled"))
	{
		if ( !apmib_get(MIB_WLAN_SHORT_GI, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strcmp(name ,"block_relay"))
	{
		if ( !apmib_get( MIB_WLAN_BLOCK_RELAY, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}	
	else if(!strcmp(name ,"tx_stbc"))
	{
		if ( !apmib_get( MIB_WLAN_STBC_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);		
	}
	else if(!strncmp(name, "coexist", strlen("coexist")))
	{
#ifdef CONFIG_RTL_8881A_SELECTIVE
		if ( strstr(name, "_2")) {
			if ( !apmib_get( MIB_WLAN_COEXIST_ENABLED_2, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		else 
#endif
		{
			if ( !apmib_get( MIB_WLAN_COEXIST_ENABLED, (void *)&intVal) )
				goto GET_INDEX_FAIL;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strcmp(name,"wlan_support_8881a_selective"))
	{
#if defined(CONFIG_RTL_8881A_SELECTIVE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);		
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);		
#endif
	}
	else if(!strcmp(name ,"tx_beamforming"))
	{
		if ( !apmib_get( MIB_WLAN_TX_BEAMFORMING, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}
	else if(!strcmp(name ,"RFPower"))
	{
		if ( !apmib_get( MIB_WLAN_RFPOWER_SCALE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal);	
	}	
	/***** security page *****/
	else if(!strcmp(name,"wlan_root_mssid_rpt_num"))
	{
#if defined(UNIVERSAL_REPEATER) 		
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", NUM_VWLAN+1+1); /// 1:root ; 1:rpt
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", NUM_VWLAN+1); /// 1:root ;
#endif
	}
	else if(!strcmp(name,"mssid_idx"))
	{
#ifdef MBSSID
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", mssid_idx);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}	
	else if(!strcmp(name,"ApSupport1X"))
	{
#ifdef HAVE_AUTH
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif	
	}
	else if(!strcmp(name,"iapp_support"))
	{
#ifdef HAVE_IAPP
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif	
	}
	else if(!strcmp(name,"clientModeSupport1X"))
	{
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif	
	}
	else if(!strcmp(name,"clientModeSupportWapi"))
	{

#ifdef CONFIG_RTL_WAPI_SUPPORT
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif	
	}
	else if(!strcmp(name,"authType"))
	{
		if ( !apmib_get( MIB_WLAN_AUTH_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;	
	}
	else if(!strcmp(name,"wapiAuth"))
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if ( !apmib_get(MIB_WLAN_WAPI_AUTH, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#else
		intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;	
	}
	else if(!strcmp(name,"wep"))
	{
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_WEP, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"keyType"))
	{
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"wep64IsNull"))
	{
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_WEP64_KEY1, (void *)buffer) )
			goto GET_INDEX_FAIL;
		if(strlen(buffer)==0) intVal=1;
		else intVal=0;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", intVal) ;
	}
	else if(!strcmp(name,"wep128IsNull"))
	{
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_WEP128_KEY1, (void *)buffer) )
			goto GET_INDEX_FAIL;
		if(strlen(buffer)==0) intVal=1;
		else intVal=0;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"pskFormat"))
	{
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_PSK_FORMAT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"wapiPskFormat"))
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if ( !apmib_get( MIB_WLAN_WAPI_PSK_FORMAT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#else
		intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"eapType"))
	{
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		if ( !apmib_get( MIB_WLAN_EAP_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#else
		intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}
	else if(!strcmp(name,"eapInsideType"))
	{
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		if ( !apmib_get( MIB_WLAN_EAP_INSIDE_TYPE, (void *)&intVal) )
			goto GET_INDEX_FAIL;
#else
		intVal=0;
#endif	
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	/*** multiple ap ***/
	else if(!strcmp(name,"wmmEnabled"))
	{
		if ( !apmib_get(MIB_WLAN_WMM_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"wlanAccess"))
	{
		if ( !apmib_get(MIB_WLAN_ACCESS, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}	
	/**acl**/
	else if(!strcmp(name,"wlanAcNum"))
	{
		if ( !apmib_get( MIB_WLAN_MACAC_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"wlanAcEnabled"))
	{
		if ( !apmib_get( MIB_WLAN_MACAC_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	/*wds*/
	else if ( !strcmp(name, "wlanWdsEnabled")) {
		if ( !apmib_get( MIB_WLAN_WDS_ENABLED, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if ( !strcmp(name, "wlanWdsNum")) {
		if ( !apmib_get( MIB_WLAN_WDS_NUM, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;

	}
	else if ( !strcmp(name, "wdsEncrypt")) {
		if ( !apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;

	}
	else if ( !strcmp(name, "wdsWepFormat")) {
		if ( !apmib_get( MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;

	}
	else if ( !strcmp(name, "wdsPskFormat")) {
		if ( !apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal) )
			goto GET_INDEX_FAIL;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	/**WPS**/
	else if(!strcmp(name,"is_rpt_wps_support")){
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		intVal=1;
#else
		intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}
	else if(!strcmp(name,"encrypt_rpt")){
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		char tmpStr[10];
		sprintf(tmpStr,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(tmpStr);
		apmib_get( MIB_WLAN_ENCRYPT, (void *)&intVal);
		sprintf(tmpStr,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(tmpStr);
#else
		intVal=0;
#endif

		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}	
	else if(!strcmp(name,"enable1x_rpt")){
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		char tmpStr[10];
		sprintf(tmpStr,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(tmpStr);
		apmib_get( MIB_WLAN_ENABLE_1X, (void *)&intVal);
		sprintf(tmpStr,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(tmpStr);
#else
		intVal=0;
#endif

		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}
	else if(!strcmp(name,"wpa_auth_rpt")){
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		char tmpStr[10];
		sprintf(tmpStr,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(tmpStr);
		apmib_get( MIB_WLAN_WPA_AUTH, (void *)&intVal);
		sprintf(tmpStr,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(tmpStr);
#else
		intVal=0;
#endif

		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}
	else if(!strcmp(name,"wlanMode_rpt")){
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		sprintf(buffer,"wlan%d-vxd0",apmib_get_wlanidx());
		SetWlan_idx(buffer);
		apmib_get(MIB_WLAN_MODE, (void *)&intVal);
		sprintf(buffer, "%d", intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", intVal);
		sprintf(buffer,"wlan%d",apmib_get_wlanidx());
		SetWlan_idx(buffer);
#else
		intVal=0;
#endif

		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}
	else if(!strcmp(name,"networkType_rpt")){
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
		intVal=1;
#else
		intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}
	else if(!strcmp(name,"lockdown_stat")){
#ifdef HAVE_WPS
				extern int wscd_lock_stat;
				intVal=wscd_lock_stat;
#else
				intVal=0;
#endif
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;		
	}	
#endif
#ifdef HOME_GATEWAY
	else if(!strcmp(name,"dmzEnabled")){
			if ( !apmib_get( MIB_DMZ_ENABLED, (void *)&intVal) )
				goto GET_INDEX_FAIL;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)intVal) ;
	}
	else if(!strcmp(name,"pppConnectStatus"))
	{

		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",isConnectPPP());
	}
#endif
	cyg_httpd_write_chunked(tmpbuf,ret);
	free(tmpbuf);
	free(buffer);
	return 0;
GET_INDEX_FAIL:
	printf("get index fail!!name=%s\n",name);
	ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",0);
	cyg_httpd_write_chunked(tmpbuf,ret);
	free(tmpbuf);
	free(buffer);
	return -1;
}

/*
  * getInfo
  * used to get information from apmib
  * remember to use strncmp
  */
int getInfo(int argc, char ** argv)
{
	char *name;
	int i,intVal;
	struct in_addr	intaddr;

	int counter=0;
	char * tmpbuf = malloc(TMP_BUF_SIZE);
	if(tmpbuf == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
        return -1;
	}
	memset(tmpbuf,0,TMP_BUF_SIZE);
	char * buffer = malloc(TMP_BUF_SIZE);
	if(buffer == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
		free(tmpbuf);
        return -1;
	}
	memset(buffer,0,TMP_BUF_SIZE);

	unsigned long sec, mn, hr, day;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	int ret=strlen("");
	name=argv[0];
	time_t current_secs;	
	struct tm * tm_time;
	DHCP_T dhcp;
	char *iface=NULL;
	OPMODE_T opmode=-1;
	WLAN_MODE_T wlanmode = -1;
	int wispWanId=0;
	bss_info bss;
	struct user_net_device_stats stats;
	
	#ifdef HAVE_TR069
#define req_get_cstream_var(wp, format, args...) get_cstream_var(postData, len, format, ##args)
#define req_format_write(wp, format, args...) __req_format_write(format, ##args)
	if(!strcmp(name, "tr069-autoexec-0"))
	{
		//#define MIB_CWMP_FLAG  	CWMP_ID + 26
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
                {
                	free(tmpbuf);
                	free(buffer);
                	return -1;
                }
                //#define CWMP_FLAG_AUTORUN	0x20
                if(intVal & CWMP_FLAG_AUTORUN){
					free(tmpbuf);
					free(buffer);
			return web_write_chunked("");
                        //return req_format_write(wp,"");
                }else{
					free(tmpbuf);
					free(buffer);
			return web_write_chunked("%s","checked");
                        //return req_format_write(wp,"checked");
                }
	}else if(!strcmp(name, "tr069-autoexec-1")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                 
                if(intVal & CWMP_FLAG_AUTORUN){
					free(tmpbuf);
					free(buffer);
			return web_write_chunked("%s","checked");
                }else{
					free(tmpbuf);
					free(buffer);
			return web_write_chunked("");
                }
	}else if(!strcmp(name, "acs_url")) {
                if ( !apmib_get( MIB_CWMP_ACS_URL, (void *)buffer) )            
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
                return counter;
        }else if(!strcmp(name, "acs_username")) {
                if ( !apmib_get( MIB_CWMP_ACS_USERNAME, (void *)buffer) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;
        }else if(!strcmp(name, "acs_password")) {
                if ( !apmib_get( MIB_CWMP_ACS_PASSWORD, (void *)buffer) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;
        }else if(!strcmp(name, "tr069-inform-0")) {
                if ( !apmib_get( MIB_CWMP_INFORM_ENABLE, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal == 1){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked(""); 
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s", "checked");
                }    
        }else if(!strcmp(name, "tr069-inform-1")) {
                if ( !apmib_get( MIB_CWMP_INFORM_ENABLE, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal == 1){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s", "checked");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked(""); 
                }    
        }else if(!strcmp(name, "inform_interval")) {
                if ( !apmib_get( MIB_CWMP_INFORM_INTERVAL, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                sprintf(buffer, "%d", intVal);
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;                
        }else if(!strcmp(name, "tr069_interval")) {
                if ( !apmib_get( MIB_CWMP_INFORM_ENABLE, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal == 1){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s", "disabled");
                }
        }else if(!strcmp(name, "conreq_name")) {
                if ( !apmib_get( MIB_CWMP_CONREQ_USERNAME, (void *)buffer) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;
        }else if(!strcmp(name, "conreq_pw")) {
                if ( !apmib_get( MIB_CWMP_CONREQ_PASSWORD, (void *)buffer) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;
        }else if(!strcmp(name, "conreq_path")) {
                if ( !apmib_get( MIB_CWMP_CONREQ_PATH, (void *)buffer) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;               
        }else if(!strcmp(name, "conreq_port")) {
                if ( !apmib_get( MIB_CWMP_CONREQ_PORT, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                sprintf(buffer, "%d", intVal );
				counter=web_write_chunked("%s", buffer);
				free(tmpbuf);
				free(buffer);
				return counter;
        }else if(!strcmp(name, "tr069-dbgmsg-0")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_DEBUG_MSG){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }
        }else if(!strcmp(name, "tr069-dbgmsg-1")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_DEBUG_MSG){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }
        }else if(!strcmp(name, "tr069-sendgetrpc-0")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_SENDGETRPC){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }
        }else if(!strcmp(name, "tr069-sendgetrpc-1")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_SENDGETRPC){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }
        }else if(!strcmp(name, "tr069-skipmreboot-0")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_SKIPMREBOOT){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }
        }else if(!strcmp(name, "tr069-skipmreboot-1")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_SKIPMREBOOT){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }
        }else if(!strcmp(name, "tr069-delay-0")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_DELAY){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }
        }else if(!strcmp(name, "tr069-delay-1")) {
                if ( !apmib_get( MIB_CWMP_FLAG, (void *)&intVal) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                
                if(intVal & CWMP_FLAG_DELAY){
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("%s","checked");
                }else{
						free(tmpbuf);
						free(buffer);
                        return web_write_chunked("");
                }
        }else if(!strcmp(name, "cwmp_tr069_menu")) {
				free(tmpbuf);
				free(buffer);
                return web_write_chunked("%s", "manage.addItem('TR-069 config', 'tr069config.htm', '', 'Setup TR-069 configuration');" );
        }else if(!strcmp(name, "tr069_nojs_menu")) {
#if 1
				free(tmpbuf);
				free(buffer);
                return web_write_chunked("%s",
                                "document.write('"\
                                "<tr><td><b>cwmp_tr069_menu</b></td></tr>"\
                                "<tr><td><a href=\"tr069config.htm\" target=\"view\">TR-069 config</a></td></tr>"\
                                "')");
#else
		req_format_write(wp,"manage.addItem(tr069_setting, 'tr069config.htm', '', 'CWMP TR069 settings');\n");	
#endif
        }
	#endif
	if(!strcmp(name,"remotelog_support")){
	#ifdef REMOTELOG_SUPPORT
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", 1);
	#else
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", 0);	
	#endif
	}
	if(!strcmp(name,"rtLogServer")){
		if(!apmib_get(MIB_REMOTELOG_SERVER,(void *)buffer))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",inet_ntoa(*((struct in_addr *)buffer)));
		}
	if( !strcmp(name,"domainName")) {
		if(!apmib_get(MIB_DOMAIN_NAME,(void *)buffer))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);		
	}
	#ifdef HAVE_NBSERVER
	if( !strcmp(name,"netbiosName")) {
		if(!apmib_get(MIB_NETBIOS_NAME,(void *)buffer))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);	
	}
	#endif
	else if( !strcmp(argv[0],"netbiosName_enable")) 
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0");	
		#ifdef HAVE_NBSERVER
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","1");	
		#endif
	}
#if 1/* CONFIG_CUTE_MAHJONG */
	else if(!strcmp(name,"is_cmj"))
	{
	#if defined(CONFIG_CUTE_MAHJONG)
	    free(tmpbuf);
	    free(buffer);
  		return web_write_chunked("%d", 1);
	#else
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", 0);
	#endif
	}
	else if(!strcmp(name,"is_xdg"))
	{
	#if defined(CONFIG_RTL_XDG)
	    free(tmpbuf);
	    free(buffer);
  		return web_write_chunked("%d", 1);
	#else
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", 0);
	#endif
	}
	else if(!strcmp(name,"is_etherlink"))
	{
		#if !defined(HAVE_NOETH)
		if (wan_link_status(1))
			intVal = 1;
		else
		#endif
			intVal = 0;

		free(tmpbuf);
		free(buffer);
  		return web_write_chunked("%d", intVal);
	}	
	else if(!strcmp(name,"wan_detect"))
	{
#ifdef CONFIG_ECOS_AP_SUPPORT
		intVal = 0;
#else
		if(!apmib_get(MIB_WAN_DETECT,(void *)&intVal)) 
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
#endif
		
		free(tmpbuf);
		free(buffer);
  		return web_write_chunked("%d", intVal);
	}
	else if(!strcmp(name,"cmj_upgradekit"))
	{
	#if defined(CONFIG_8881A_UPGRADE_KIT)
		if(!apmib_get(MIB_UPGRADE_KIT,(void *)&intVal)) 
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		
		free(tmpbuf);
		free(buffer);
  		return web_write_chunked("%d", intVal);
	#else
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", 0);
	#endif
	}	
	else if(!strcmp(name, "ui_mode"))
	{
	#if defined(MIB_UI_MODE)
		/*1 is easy mode, 0 is adv(ori) mode*/
		if(!apmib_get(MIB_UI_MODE,(void *)&intVal)) 
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		
	#else
	#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && defined(CONFIG_CUTE_MAHJONG_RTK_UI)
		intVal=0;
	#else
		intVal = 1;
	#endif
	#endif
		
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", intVal);
	}
	else if(!strcmp(name, "gpio_2g5g"))
	{
	#if defined(CONFIG_CUTE_MAHJONG)
	    free(tmpbuf);
	    free(buffer);
  		return web_write_chunked("%d", get_gpio_2g5g());
	#else
	    free(tmpbuf);
	    free(buffer);
		return web_write_chunked("%d", 0);
	#endif
	}
	else if(strstr(name, "commentHtml_uiMode"))
	{
	#if defined(MIB_UI_MODE)
		if(!apmib_get(MIB_UI_MODE_ENABLED,(void *)&intVal)) 
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		
	#else
		intVal = 0;
	#endif

		if (intVal == 0) {
			if (strstr(name, "beg"))
				sprintf(buffer, "%s", "<!--");
			else if (strstr(name, "end"))
				sprintf(buffer, "%s", "-->");
			else 
			{
				free(tmpbuf);
				free(buffer);
				return -1;
			}
		}
		else
			sprintf(buffer, "%s", "");
		
		counter=web_write_chunked("%s", buffer);
		free(tmpbuf);
		free(buffer);
		return counter;
	}
	else if(!strcmp(argv[0],("is_SALES20131014")))
	{
		#if defined(CONFIG_CMJ_SALES20131014)
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","1");
		#else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0");
		#endif	
	}
	else if(!strcmp(argv[0],("is_rpt_mode")))
	{
	#ifdef BRIDGE_REPEATER		
		apmib_get(MIB_OP_MODE,(void *)&intVal);
		if(intVal==BRIDGE_MODE)
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","1");
		else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0");
	#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0");
	#endif
	}
#endif /* #if defined(CONFIG_CUTE_MAHJONG) */
#ifdef CONFIG_RTK_MESH
   	else if ( !strcmp(name, "meshEncrypt")) {
   		ENCRYPT_T encrypt;
		if ( !apmib_get( MIB_WLAN_MESH_ENCRYPT,  (void *)&encrypt) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if ( encrypt == ENCRYPT_DISABLED)
			strcpy( buffer, "Disabled");
		else if ( encrypt == ENCRYPT_WPA2)
			strcpy( buffer, "WPA2");
		else
			buffer[0] = '\0';
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);
   	}
	else if(!strcmp(argv[0],("mesh_comment_start")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
	}
	else if(!strcmp(argv[0],("mesh_comment_end")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
	}
	else if(!strcmp(argv[0],("mesh_jscomment_start")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
	}
	else if(!strcmp(argv[0],("mesh_jscomment_end")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","");
	}
	else if(!strcmp(argv[0],("mesh_comment_start")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<!--");
	}
	else if(!strcmp(argv[0],("mesh_comment_end")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","-->");
	}
	else if(!strcmp(argv[0],("mesh_jscomment_start")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","/*");
	}
	else if(!strcmp(argv[0],("mesh_jscomment_end")))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","*/");
	}
	else if ( !strcmp(name, "meshPskValue")) {
		int i;
		buffer[0]='\0';
		if ( !apmib_get(MIB_WLAN_MESH_WPA_PSK,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
   		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);
	}		
	else if ( !strcmp(name, "meshID")) {
		if ( !apmib_get(MIB_WLAN_MESH_ID,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		translate_control_code(buffer);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);
	}
	
#ifdef 	_11s_TEST_MODE_

	else if ( !strcmp(name, "meshTestParam1")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM1, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam2")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM2, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam3")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM3, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam4")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM4, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam5")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM5, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam6")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM6, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam7")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM7, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam8")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM8, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParam9")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAM9, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParama")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAMA, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParamb")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAMB, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);;
	}
	else if ( !strcmp(name, "meshTestParamc")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAMC, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParamd")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAMD, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParame")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAME, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParamf")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAMF, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",intVal);
	}
	else if ( !strcmp(name, "meshTestParamStr1")) {
		if ( !apmib_get( MIB_WLAN_MESH_TEST_PARAMSTR1, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);
	}
#endif
#endif // CONFIG_RTK_MESH

	else if( !strncmp(name, "userName",strlen("userName"))) {
		if(!apmib_get(MIB_USER_NAME,(void *)buffer))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s",buffer);		
	}	
	else if( !strcmp(name, "countDownTime")) {
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d",20);
		
	}
	else if( !strcmp(name, "homepage")) {
#if defined(HTTP_FILE_SERVER_SUPPORTED)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","http_files.htm");
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","home.htm");
#endif
	}
	else if ( !strcmp(name, "ip-rom")) {
		if ( !apmib_get( MIB_IP_ADDR, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", inet_ntoa(*((struct in_addr *)buffer)));
		
	}
	else if ( !strcmp(name, "mask-rom")) {
		if ( !apmib_get( MIB_SUBNET_MASK, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "gateway-rom")) {
		if ( !apmib_get( MIB_DEFAULT_GATEWAY, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "dhcpRangeStart")) {	

		if ( !apmib_get( MIB_DHCP_CLIENT_START, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "dhcpRangeEnd")) {
		if ( !apmib_get( MIB_DHCP_CLIENT_END, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", inet_ntoa(*((struct in_addr *)buffer)));	
	}
	else if ( !strcmp(name, "domainName")) {
		if ( !apmib_get( MIB_DOMAIN_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "bridgeMac")) {
		if ( !apmib_get(MIB_ELAN_MAC_ADDR,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%02x%02x%02x%02x%02x%02x",(unsigned char)buffer[0], (unsigned char)buffer[1],
						(unsigned char)buffer[2], (unsigned char)buffer[3], (unsigned char)buffer[4], (unsigned char)buffer[5]);
	}
	else if ( !strcmp(name, "redirect_form_name")) {		
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",REDIRECT_FORM_NAME);
	}
	else if(!strcmp(name,"status_warning"))
	{
#ifdef REBOOT_CHECK
		if(needReboot == 1)
		{
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "<tr><td></td></tr><tr><td><font size=2><font color='#FF0000'> \
 															Below status shows currnt settings, but does not take effect. \
															</font></td></tr>");
		}
		else
#endif
		{
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "");
		}

		
	}
	else if ( !strcmp(name, "ip")) {
		if ( getInAddr(BRIDGE_IF, IP_ADDR, (void *)&intaddr ) )
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", inet_ntoa(intaddr) );
		else
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0.0.0.0");
	}
	else if ( !strcmp(name, "mask")) {
		if ( getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&intaddr ))
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", inet_ntoa(intaddr) );
		else
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "0.0.0.0");
	}
   	else if ( !strcmp(name, "gateway")) {
		DHCP_T dhcp;
  		apmib_get( MIB_DHCP, (void *)&dhcp);
#if defined(BRIDGE_REPEATER) && defined(DHCP_AUTO_SUPPORT)
		int opMode;
		apmib_get( MIB_OP_MODE, (void *)&opMode);
		if(opMode != 0)
			dhcp = DHCP_LAN_CLIENT;
#endif
		if ( dhcp == DHCP_SERVER ) {			
		// if DHCP server, default gateway is set to LAN IP
			if ( getInAddr(BRIDGE_IF, IP_ADDR, (void *)&intaddr ) )
				ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", inet_ntoa(intaddr) );
			else
				ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s","0.0.0.0");
		}
		else if(( dhcp == DHCP_LAN_CLIENT )){
			char intfname[32];
			struct in_addr ip,mask,gateway,dns1,dns2;
				
			if(!isFileExist("/etc/lan_info"))
				ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0.0.0.0");
			else{
				memset(intfname,0,sizeof(intfname));
				/*Get lan info*/
				get_WanInfo("/etc/lan_info",intfname,&ip, &mask, &gateway, &dns1, &dns2);				
				//diag_printf("%s %d intfname %s ip(0x%x) mask(0x%x) gateway(0x%x) dns1(0x%x) dns2(0x%x)\n",__FUNCTION__,__LINE__,intfname,ip,mask,gateway,dns1,dns2);
				ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", inet_ntoa(gateway) );
			}
		}
		else{//get from mib
		if ( apmib_get(MIB_DEFAULT_GATEWAY,(void*)&intaddr))
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s", inet_ntoa(intaddr) );
		else
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0.0.0.0");
		}
	}
	else if ( !strcmp(name, "indexHtm")) {
#if defined(CONFIG_RTL_8197F) && defined (CONFIG_ECOS_AP_SUPPORT)
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s","status.htm");
#else
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s","wizard.htm");
#endif
	}

	else if ( !strcmp(name, "ntpTimeZone")) {
		if ( !apmib_get( MIB_NTP_TIMEZONE, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "ntpServerIp1")) {
		if ( !apmib_get( MIB_NTP_SERVER_IP1, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "ntpServerIp2")) {
		if ( !apmib_get( MIB_NTP_SERVER_IP2, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
#ifdef HOME_GATEWAY
	else if ( !strcmp(name, "wan-ip-rom")) {
		if ( !apmib_get( MIB_WAN_IP_ADDR, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "wan-mask-rom")) {
		if ( !apmib_get( MIB_WAN_SUBNET_MASK, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "wan-gateway-rom")) {
		if ( !apmib_get( MIB_WAN_DEFAULT_GATEWAY, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		//if (!memcmp(buffer, "\x0\x0\x0\x0", 4))
		//	strcpy(buffer,"0.0.0.0");
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "hostName")) 
	{
		if ( !apmib_get( MIB_HOST_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}    
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    else if ( !strcmp(name, "dhcpPlusUserName")) {
		if ( !apmib_get( MIB_DHCP_PLUS_USER_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "dhcpPlusPassword")) {
		if ( !apmib_get( MIB_DHCP_PLUS_PASS_WORD, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
    #endif
	else if ( !strcmp(name, "pppUserName")) {
		if ( !apmib_get( MIB_PPP_USER_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "pppPassword")) {
		if ( !apmib_get( MIB_PPP_PASSWORD, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "pppServiceName")) {
		if ( !apmib_get( MIB_PPP_SERVICE_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "pptpIp")) {
		if ( !apmib_get( MIB_PPTP_IP_ADDR, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "pptpSubnet")) {
		if ( !apmib_get( MIB_PPTP_SUBNET_MASK, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}		
#if defined(CONFIG_DYNAMIC_WAN_IP)
	else if ( !strcmp(name, "pptpDefGw")) {
		if ( !apmib_get( MIB_PPTP_DEFAULT_GW,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}	
#endif	
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)
		else if ( !strcmp(name, "pptpServerDomain")) {
			memset(buffer,0x00,TMP_BUF_SIZE);
			apmib_get( MIB_PPTP_SERVER_DOMAIN, (void *)buffer);
			ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
		}
#endif
	else if ( !strcmp(name, "pptpServerIp")) {
		if ( !apmib_get( MIB_PPTP_SERVER_IP_ADDR, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "pptpUserName")) {
		if ( !apmib_get( MIB_PPTP_USER_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "pptpPassword")) {
		if ( !apmib_get( MIB_PPTP_PASSWORD, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "l2tpIp")) {
		if ( !apmib_get( MIB_L2TP_IP_ADDR, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "l2tpSubnet")) {
		if ( !apmib_get( MIB_L2TP_SUBNET_MASK, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}	
#if defined(CONFIG_DYNAMIC_WAN_IP)
	else if ( !strcmp(name, "l2tpDefGw")) {
		if ( !apmib_get( MIB_L2TP_DEFAULT_GW,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}	
#endif
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)
	else if ( !strcmp(name, "l2tpServerDomain")) {
		memset(buffer,0x00,TMP_BUF_SIZE);
		apmib_get( MIB_L2TP_SERVER_DOMAIN, (void *)buffer);
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}

#endif	
	else if ( !strcmp(name, "l2tpServerIp")) {
		if ( !apmib_get( MIB_L2TP_SERVER_IP_ADDR, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "l2tpUserName")) {
		if ( !apmib_get( MIB_L2TP_USER_NAME, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "l2tpPassword")) {
		if ( !apmib_get( MIB_L2TP_PASSWORD, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}
	else if ( !strcmp(name, "wan-dns1")) {
		if ( !apmib_get( MIB_DNS1, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "wan-dns2")) {
		if ( !apmib_get( MIB_DNS2, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}
	else if ( !strcmp(name, "wan-dns3")) {
		if ( !apmib_get( MIB_DNS3, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",inet_ntoa(*((struct in_addr *)buffer)));
	}	
	else if ( !strcmp(name, "wanMac")) {
		if ( !apmib_get(MIB_WAN_MAC_ADDR,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%02x%02x%02x%02x%02x%02x",(unsigned char)buffer[0], (unsigned char)buffer[1],
						(unsigned char)buffer[2], (unsigned char)buffer[3], (unsigned char)buffer[4], (unsigned char)buffer[5]);
	}
	else if(!strcmp(name,"ddnsDomainName"))
	{
		if ( !apmib_get(MIB_DDNS_DOMAIN_NAME, (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=strlen(tmpbuf);
	}
	else if(!strcmp(name,"ddnsUser"))
	{
		if ( !apmib_get(MIB_DDNS_USER, (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=strlen(tmpbuf);
	}
	else if(!strcmp(name,"ddnsPassword"))
	{
		if ( !apmib_get(MIB_DDNS_PASSWORD, (void *)tmpbuf))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=strlen(tmpbuf);
	}
#endif
	else if ( !strcmp(name, "ssid")) {
		
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get( MIB_WLAN_SSID, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		translate_control_code(buffer);
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}

#ifndef HAVE_NOWIFI
	else if(!strcmp(name,"info_country"))
	{
		if(sizeof(countryIEArray)==0)
			ret= snprintf(tmpbuf,TMP_BUF_SIZE,"[]");
		else
		{
			//ret= snprintf(tmpbuf,TMP_BUF_SIZE,"[");
			web_write_chunked("%s","[");
			for(i=0;i<sizeof(countryIEArray)/sizeof(COUNTRY_IE_ELEMENT);i++)
			{				
				bzero(buffer,TMP_BUF_SIZE);
				/*country code, abb,5g idx,2g idx,name*/				
				snprintf(buffer,TMP_BUF_SIZE,"[%d,'%s',%d,%d,'%s']",countryIEArray[i].countryNumber,countryIEArray[i].countryA2,
					countryIEArray[i].A_Band_Region,countryIEArray[i].G_Band_Region,countryIEArray[i].countryName);
				if(i !=(sizeof(countryIEArray)/sizeof(COUNTRY_IE_ELEMENT)-1))					
					strcat(buffer,",");
				if(strlen(tmpbuf)+strlen(buffer)>=TMP_BUF_SIZE)				
				{					
					web_write_chunked(tmpbuf);
					bzero(tmpbuf,sizeof(tmpbuf));
				}
				strcat(tmpbuf,buffer);	
			}
			web_write_chunked("%s]",tmpbuf);
			
			free(tmpbuf);
			free(buffer);
			return 0;
		}	
	}
	else if(!strcmp(name,"info_2g"))
	{
		if(sizeof(Bandtable_2dot4G)==0)
			web_write_chunked("%s","[]");
		else
		{
			web_write_chunked( "%s","[");
			for(i=0;i<sizeof(Bandtable_2dot4G)/sizeof(REG_DOMAIN_TABLE_ELEMENT_T);i++)
			{
				web_write_chunked("[%d,%d,'%s'",Bandtable_2dot4G[i].region,Bandtable_2dot4G[i].channel_set,Bandtable_2dot4G[i].area);
				if(i ==(sizeof(Bandtable_2dot4G)/sizeof(REG_DOMAIN_TABLE_ELEMENT_T)-1))
					web_write_chunked("%s","]");
				else
					web_write_chunked("%s","],");
			}
			web_write_chunked("%s","]");
		}
		
		free(tmpbuf);
		free(buffer);
		return 0;

	}
	else if(!strcmp(name,"info_5g"))
	{
		if(sizeof(Bandtable_5G)==0)
			web_write_chunked("%s","[]");
		else
		{
			web_write_chunked( "%s","[");
			for(i=0;i<sizeof(Bandtable_5G)/sizeof(REG_DOMAIN_TABLE_ELEMENT_T);i++)
			{
				web_write_chunked("[%d,%d,'%s'",Bandtable_5G[i].region,Bandtable_5G[i].channel_set,Bandtable_5G[i].area);
				if(i ==(sizeof(Bandtable_5G)/sizeof(REG_DOMAIN_TABLE_ELEMENT_T)-1))
					web_write_chunked("%s","]");
				else
					web_write_chunked("%s","],");
			}
			web_write_chunked("%s","]");
		}
		
		free(tmpbuf);
		free(buffer);
		return 0;

	}
	else if(!strcmp(name,"is_ulinker"))
	{
#if defined(CONFIG_RTL_ULINKER)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "1" );
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0");
#endif
	}
	else if(!strcmp(name,"ulinker_opMode"))
	{
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 2) ;
#if defined(CONFIG_RTL_ULINKER)
		int opMode, wlanMode, rpt_enabled;
		apmib_get( MIB_OP_MODE, (void *)&opMode);
		apmib_get( MIB_WLAN_MODE, (void *)&wlanMode);
		if(apmib_get_wlanidx() == 0)
			apmib_get( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);						
		else
			apmib_get( MIB_REPEATER_ENABLED2, (void *)&rpt_enabled);
			
		if(opMode == 0)
		{
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 2) ;
		}
		else
		{
			if(wlanMode == AP_MODE)
			{	
				if(rpt_enabled == 1)
					ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 3);
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
			}
			else
				ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 1);
				
		}

#endif
	}
	else if ( !strcmp(name, "dhcp-current") ) {
   		if ( !apmib_get( MIB_DHCP, (void *)&dhcp) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}

		if (dhcp==DHCP_CLIENT) {
			if (!isDhcpClientExist(BRIDGE_IF) &&
					!getInAddr(BRIDGE_IF, IP_ADDR, (void *)&intaddr))
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_dhcp_get_ip)</script>");
			if (isDhcpClientExist(BRIDGE_IF))
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "DHCP");
		}
		else
		{
			free(tmpbuf);
			free(buffer);
			return web_write_chunked( "<script>dw(status_fixed)</script>");
		}
	}
	else if ( !strcmp(name, "hwaddr")) {
		if ( getInAddr(BRIDGE_IF, HW_ADDR, (void *)&hwaddr ) ) {
			pMacAddr = (unsigned char *)hwaddr.sa_data;
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x", pMacAddr[0], pMacAddr[1],
				pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
		}
		else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "00:00:00:00:00:00");
	}
#ifdef HOME_GATEWAY
	else if ( !strcmp(name, "wanTxPacketNum")) {
#ifdef RTK_USB3G
		apmib_get(MIB_WAN_DHCP, (void *)&wantype);
#endif
  	apmib_get( MIB_OP_MODE, (void *)&opmode);
	if( !apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId))
	{
		free(tmpbuf);
		free(buffer);
		return -1;
	}
		if(opmode == WISP_MODE) {
			if(0 == wispWanId)
				iface = "wlan0";
			else if(1 == wispWanId)
				iface = "wlan1";
		}
#ifdef RTK_USB3G
        else if (wantype == USB3G)
            iface = PPPOE_IF;
#endif
		else
			iface = WAN_IF;
		if ( getStats(iface, &stats) < 0)
			stats.tx_packets = 0;
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.tx_packets);
	}
	else if ( !strcmp(name, "wanRxPacketNum")) {
#ifdef RTK_USB3G
        apmib_get(MIB_WAN_DHCP, (void *)&wantype);
#endif
		apmib_get( MIB_OP_MODE, (void *)&opmode);
		if( !apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}

		if(opmode == WISP_MODE) {
			if(0 == wispWanId)
				iface = "wlan0";
			else if(1 == wispWanId)
				iface = "wlan1";
		}
#ifdef RTK_USB3G
        else if (wantype == USB3G)
            iface = PPPOE_IF;
#endif
		else
			iface = WAN_IF;
		if ( getStats(iface, &stats) < 0)
			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.rx_packets);
	}
	else if ( !strcmp(name, "wanDhcp-current")) {
//	diag_printf("%s:%d\n",__FILE__,__LINE__);
#if defined(CONFIG_RTL_8198_AP_ROOT)
		free(tmpbuf);
		free(buffer);
		return web_write_chunked("Brian 5BGG");
#else
 		int isWanPhy_Link=0;

 		if ( !apmib_get( MIB_OP_MODE, (void *)&opmode) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if( !apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
 		if ( !apmib_get( MIB_WAN_DHCP, (void *)&dhcp) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if(opmode == BRIDGE_MODE){
			free(tmpbuf);
			free(buffer);
			return web_write_chunked("<script>dw(status_disconn)<\\/script>");
		}
		
		if(opmode != WISP_MODE){
 			//isWanPhy_Link=getWanLink("eth1");			
			isWanPhy_Link=getWanLink(WAN_IF);
 		}
        
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT        
		if ( dhcp == DHCP_CLIENT || dhcp == DHCP_PLUS) {
        #else
		if ( dhcp == DHCP_CLIENT) {
        #endif
			if(opmode == WISP_MODE) {
#ifdef CONFIG_SMART_REPEATER
				setWlanIdx(wispWanId);
				apmib_get(MIB_WLAN_MODE,(void *)&wlanmode);
				if(wlanmode == CLIENT_MODE){
					if(0 == wispWanId)
						iface = "wlan0";
					else if(1 == wispWanId)
						iface = "wlan1";
					
				}
				else{
					if(0 == wispWanId)
						iface = "wlan0-vxd0";
					else if(1 == wispWanId)
						iface = "wlan1-vxd0";
				}
				getWlBssInfo(iface, &bss);
				if(bss.state != STATE_CONNECTED)		
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_dhcp_get_ip)<\\/script>");
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","DHCP");	
#else
				//SMART REPEATER is enable in default setting, don't care this situation
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","DHCP");	
#endif
			}
			else
			{
				iface = WAN_IF;
			 	if (!isDhcpClientExist(iface))
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_dhcp_get_ip)<\\/script>");
				else{
					if(isWanPhy_Link < 0)
						ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_dhcp_get_ip)<\\/script>");
					else
						ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","DHCP");
				}
			}

		}
		else if ( dhcp == STATIC_IP ){
			if(opmode == WISP_MODE)
			{
#ifdef CONFIG_SMART_REPEATER

				setWlanIdx(wispWanId);
				apmib_get(MIB_WLAN_MODE,(void *)&wlanmode);
				if(wlanmode == CLIENT_MODE){
					if(0 == wispWanId)
						iface = "wlan0";
					else if(1 == wispWanId)
						iface = "wlan1";
					
				}
				else{
					if(0 == wispWanId)
						iface = "wlan0-vxd0";
					else if(1 == wispWanId)
						iface = "wlan1-vxd0";
				}
				getWlBssInfo(iface, &bss);
				if(bss.state != STATE_CONNECTED)		
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_fixed+ \",\" + status_disconn)<\\/script>");
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_fixed+ \",\" + status_conn)<\\/script>");
#else
				//SMART REPEATER is enable in default setting, don't care this situation
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_fixed+ \",\" + status_conn)<\\/script>");
#endif
			}
			else
			{
				if(isWanPhy_Link < 0)
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_fixed+ \",\" + status_disconn)<\\/script>");
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","<script>dw(status_fixed+ \",\" + status_conn)<\\/script>");
			}
		}
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT        
		else if ( dhcp ==  PPPOE || dhcp ==  PPPOE_HENAN || dhcp ==  PPPOE_NANCHANG|| dhcp ==  PPPOE_OTHER1 || dhcp ==  PPPOE_OTHER2) {
        #else
		else if ( dhcp ==  PPPOE ) {
        #endif
			
			if ( isConnectPPP()){
				if(isWanPhy_Link < 0)
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","PPPoE <script>dw(status_disconn)<\\/script>");
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","PPPoE <script>dw(status_conn)<\\/script>");
			}else
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "PPPoE <script>dw(status_disconn)<\\/script>");
		}
		else if ( dhcp ==  PPTP ) {
			if ( isConnectPPP()){
				if(isWanPhy_Link < 0)
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "PPTP <script>dw(status_disconn)<\\/script>");
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "PPTP <script>dw(status_conn)<\\/script>");
			}else
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "PPTP <script>dw(status_disconn)<\\/script>");
		}
		else if ( dhcp ==  L2TP ) { /* # keith: add l2tp support. 20080515 */
			if ( isConnectPPP()){
				if(isWanPhy_Link < 0)
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","L2TP <script>dw(status_disconn)<\\/script>");
				else
					ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "L2TP <script>dw(status_conn)<\\/script>");
			}else
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", "L2TP <script>dw(status_disconn)<\\/script>");
		}
#endif //#if defined(CONFIG_RTL_8198_AP_ROOT)
	}
	else if ( !strcmp(name, "wan-ip"))
  	{
#if defined(CONFIG_RTL_8198_AP_ROOT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0.0.0.0");
#else
  	char strWanIP[16];
		getWanInfo(strWanIP,NULL,NULL,NULL);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", strWanIP);
#endif
	}
   	else if ( !strcmp(name, "wan-mask")) {
#if defined(CONFIG_RTL_8198_AP_ROOT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0.0.0.0");
#else
			char strWanMask[16];
		
			getWanInfo(NULL,strWanMask,NULL,NULL);

			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", strWanMask);
#endif
	}
   	else if ( !strcmp(name, "wan-gateway")) {
#if defined(CONFIG_RTL_8198_AP_ROOT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0.0.0.0");
#else
			char strWanDefIP[16];
			getWanInfo(NULL,NULL,strWanDefIP,NULL);
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", strWanDefIP);
#endif
	}
	else if ( !strcmp(name, "wan-hwaddr")) {
#if defined(CONFIG_RTL_8198_AP_ROOT)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0.0.0.0");
#else
		char strWanHWAddr[18];		
		getWanInfo(NULL,NULL,NULL,strWanHWAddr);

    #ifdef RTK_USB3G
    {   /* when wantype is 3G, we dpn't need to show MAC */
        DHCP_T wan_type;
        apmib_get(MIB_WAN_DHCP, (void *)&wan_type);

        if (wan_type == USB3G)
        {
			free(tmpbuf);
			free(buffer);
            return web_write_chunked("");
        }
    }
    #endif /* #ifdef RTK_USB3G */

		ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s", strWanHWAddr);
#endif
	}
#endif
	else if(!strcmp(name,"wlProfileSupport"))
	{
#if defined(WLAN_PROFILE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "1" );
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0");
#endif
	}
	else if(!strcmp(name,"profile_comment_start"))
	{
#if defined(WLAN_PROFILE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "" );
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "<!--");
#endif
	}
	else if(!strcmp(name,"profile_comment_end"))
	{
#if defined(WLAN_PROFILE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "" );
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "-->");
#endif
	}
	else if(!strcmp(name,"isPocketRouter"))
	{
#if defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_ULINKER)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "1" );
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0");
#endif
	}	
	else if(!strcmp(name,"pocketRouter_Mode"))
	{
#if defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_ULINKER)
		apmib_get( MIB_OP_MODE, (void *)&intVal);
		if(intVal == 1) //opmode is bridge
		{
			apmib_get( MIB_WLAN_MODE, (void *)&intVal);
			if(intVal == 0) //wlan is AP mode
			{
				sprintf(buffer, "%s", "2" );
			}
			else if(intVal == 1) //wlan is client mode
			{
				sprintf(buffer, "%s", "1" );
			}
			else
			{
				sprintf(buffer, "%s", "0" );
			}
		}
		else if(intVal == 0) //opmode is router
		{
			sprintf(buffer, "%s", "3" );
		}
		
#elif defined(CONFIG_POCKET_AP_SUPPORT)
		apmib_get( MIB_WLAN_MODE, (void *)&intVal);
		if(intVal == 0) //wlan is AP mode
		{
			sprintf(buffer, "%s", "2" );
		} else {
			sprintf(buffer, "%s", "1" );
		}
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0");

#endif
	
	}	
	else if(!strcmp(name,"repeaterSSID"))
	{
		if ( !apmib_get( MIB_WLAN_SSID, (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);

		if (apmib_get_wlanidx() == 0)
			intVal = MIB_REPEATER_SSID1;
		else
			intVal = MIB_REPEATER_SSID2;
		apmib_get(intVal, (void *)buffer);
		translate_control_code(buffer);
		ret= snprintf(tmpbuf,TMP_BUF_SIZE, "%s",buffer);
	}		
	else if(!strcmp(name,"wlProfile_checkbox"))
	{
		int profile_enabled_id, wlProfileEnabled;
#if defined(WLAN_PROFILE)		
		if(apmib_get_wlanidx() == 0)
		{
			profile_enabled_id = MIB_PROFILE_ENABLED1;
		}
		else
		{	
			profile_enabled_id = MIB_PROFILE_ENABLED2;
		}

		apmib_get( profile_enabled_id, (void *)&wlProfileEnabled);

		if(wlProfileEnabled == 1)
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "checked");
		else
#endif //#if defined(WLAN_PROFILE)			
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "");
	}	
	else if(!strcmp(name,"wlProfile_value"))
	{
		int profile_enabled_id, wlProfileEnabled;
#if defined(WLAN_PROFILE)		
		if(apmib_get_wlanidx() == 0)
		{
			profile_enabled_id = MIB_PROFILE_ENABLED1;
		}
		else
		{	
			profile_enabled_id = MIB_PROFILE_ENABLED2;
		}

		apmib_get( profile_enabled_id, (void *)&wlProfileEnabled);

		if(wlProfileEnabled == 1)
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "1");
		else
#endif //#if defined(WLAN_PROFILE)			
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s", "0");

	}
	else if ( !strcmp(name, "opMode")) {
		apmib_get( MIB_OP_MODE, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
	}
	else if(!strcmp(name, "vlanOnOff"))
	{
#if defined(VLAN_CONFIG_SUPPORTED)
		apmib_get( MIB_VLANCONFIG_ENABLED, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d",intVal);
#else
		ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%d",0);
#endif
	}
	/*advance page*/
	else if(!strcmp(name,"wlan_xTxR"))
	{
		int chipVersion = getWLAN_ChipVersion();

#if defined(CONFIG_RTL_8812_SUPPORT)
		free(tmpbuf);
		free(buffer);
		return web_write_chunked( "%s","2*2");
#endif

		if(chipVersion == 1)
		{
			free(tmpbuf);
			free(buffer);
			return web_write_chunked( "%s","1*1");
		}
		else if(chipVersion == 2)
		{
			free(tmpbuf);
			free(buffer);
			return web_write_chunked("%s","2*2");
		}
#if defined(CONFIG_RTL_92D_SUPPORT)
		else if(chipVersion == 3)
		{
			apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&intVal);
			if(BANDMODEBOTH == intVal)
			{
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","1*1");
			}
			else
			{
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","2*2");
			}
		}
#endif
		else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE,"%s","0*0");
	}
	else if(!strcmp(name,"fragThreshold"))
	{
		if ( !apmib_get( MIB_WLAN_FRAG_THRESHOLD, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}
	else if(!strcmp(name,"rtsThreshold"))
	{
		if ( !apmib_get( MIB_WLAN_RTS_THRESHOLD, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}
	else if(!strcmp(name,"beaconInterval"))
	{
		if ( !apmib_get( MIB_WLAN_BEACON_INTERVAL, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}
#if defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER)
	else if ( !strcmp(name, "ScGoodRssiThreshold")) {
		if ( !apmib_get(MIB_WLAN_SC_GOOD_RSSI_THRESHOLD,  (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}
	else if ( !strcmp(name, "ScNormalRssiThreshold")) {
		if ( !apmib_get(MIB_WLAN_SC_NORMAL_RSSI_THRESHOLD,  (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}
	else if ( !strcmp(name, "ScPoorRssiThreshold")) {
		if ( !apmib_get(MIB_WLAN_SC_POOR_RSSI_THRESHOLD,  (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );
	}
#else
	else if ( !strcmp(name, "twinkle_rssi_enabled")) {
                intVal = 0;
                ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );
        }
#endif // defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER)

	/***** security page *****/
	else if(!strcmp(name,"wlEnableProfile"))
	{
		int profile_enabled_id, profileEnabledVal;
#if defined(WLAN_PROFILE)
		if(apmib_get_wlanidx() == 0)
		{
			profile_enabled_id = MIB_PROFILE_ENABLED1;
		}
		else
		{
			profile_enabled_id = MIB_PROFILE_ENABLED2;
		}
	
		apmib_get(profile_enabled_id, (void *)&profileEnabledVal);
#else
		profileEnabledVal=0;
#endif //#if defined(WLAN_PROFILE)
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", profileEnabledVal );	
	}
	else if(!strcmp(name,"wlan_profile_num"))
	{
	
		int profile_num_id, entryNum;
#if defined(WLAN_PROFILE)
		if(apmib_get_wlanidx() == 0)
		{
			profile_num_id = MIB_PROFILE_NUM1;
		}
		else
		{
			profile_num_id = MIB_PROFILE_NUM2;
		}
	
		apmib_get(profile_num_id, (void *)&entryNum);
#else
		entryNum=0;
#endif //#if defined(WLAN_PROFILE)	
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", entryNum );	
	}
	else if(!strcmp(name,"pskValue"))
	{
		if(argc>1&&argv[1])	setWlanIdx(atoi(argv[1]));
		if ( !apmib_get(MIB_WLAN_WPA_PSK,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		#if 0	//Brad modify 20080703
		for (i=0; i<strlen(buffer); i++)
			buffer[i]='*';
		buffer[i]='\0';
		#endif
		translate_control_code_sprintf(buffer);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,buffer);
	}
#ifdef WIFI_SIMPLE_CONFIG
	else if(!strcmp(name,"pskValueUnmask"))
	{
		if ( !apmib_get(MIB_WLAN_WPA_PSK,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		translate_control_code(buffer);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,buffer);
	}
#ifdef HAVE_WPS
 	else if ( !strcmp(name, "wps_key")) {
 		int id;
		apmib_get(MIB_WLAN_WSC_ENC, (void *)&intVal);
		buffer[0]='\0';
		if (intVal == WSC_ENCRYPT_WEP) {
			unsigned char tmp[100];
			apmib_get(MIB_WLAN_WEP, (void *)&intVal);
			apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&id);
			if (intVal == 1) {
				if (id == 0)
					id = MIB_WLAN_WEP64_KEY1;
				else if (id == 1)
					id = MIB_WLAN_WEP64_KEY2;
				else if (id == 2)
					id = MIB_WLAN_WEP64_KEY3;
				else
					id = MIB_WLAN_WEP64_KEY4;
				apmib_get(id, (void *)tmp);
				convert_bin_to_str(tmp, 5, buffer);
			}
			else {
				if (id == 0)
					id = MIB_WLAN_WEP128_KEY1;
				else if (id == 1)
					id = MIB_WLAN_WEP128_KEY2;
				else if (id == 2)
					id = MIB_WLAN_WEP128_KEY3;
				else
					id = MIB_WLAN_WEP128_KEY4;
				apmib_get(id, (void *)tmp);
				convert_bin_to_str(tmp, 13, buffer);
			}
		}
		else {
			if (intVal==0 || intVal == WSC_ENCRYPT_NONE)
				strcpy(buffer, "N/A");
			else
				apmib_get(MIB_WLAN_WSC_PSK, (void *)buffer);
		}
		translate_control_code(buffer);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,buffer);
	}
#endif
#endif	
	else if(!strcmp(name,"wlan_onoff_tkip"))
	{
		apmib_get(MIB_WLAN_11N_ONOFF_TKIP, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );			
	}
	else if(!strcmp(name,"onoff_tkip_comment_start"))
	{
		int wlanMode=0;

		apmib_get(MIB_WLAN_11N_ONOFF_TKIP, (void *)&intVal);
		apmib_get(MIB_WLAN_BAND, (void *)&wlanMode);
		if(intVal == 0 && (wlanMode >= BAND_11N)/*(wlanMode==8 || wlanMode==10 || wlanMode==11 || wlanMode==BAND_5G_11AN)*/)
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s","<!--");
		else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s","");
	}
	else if(!strcmp(name,"onoff_tkip_comment_end"))
	{
		int wlanMode=0;

		apmib_get(MIB_WLAN_11N_ONOFF_TKIP, (void *)&intVal);
		apmib_get(MIB_WLAN_BAND, (void *)&wlanMode);
		if(intVal == 0 && (wlanMode >= BAND_11N)/*(wlanMode==8 || wlanMode==10 || wlanMode==11 || wlanMode==BAND_5G_11AN)*/)
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s","-->");
		else
			ret=snprintf(tmpbuf,TMP_BUF_SIZE, "%s","");
	}
	else if(!strcmp(name,"wlanband"))
	{
		apmib_get(MIB_WLAN_BAND, (void *)&intVal);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}
	else if ( !strcmp(name, "wscLoocalPin")) {
		apmib_get(MIB_HW_WSC_PIN,  (void *)buffer);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,buffer);
	}	
#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if(!strcmp(name,"wapiOption"))
	{
		web_write_chunked("<option value=\"7\"> WAPI </option>");
		free(tmpbuf);
		free(buffer);
		return 0;
	}
	else if(!strcmp(name,"wapiMenu"))
	{
#if defined(CONFIG_RTL_8198) || defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_8196C)
		web_write_chunked("menu.addItem(\"WAPI\");");
		web_write_chunked("wlan_wapi = new MTMenu();");
//#if !defined(CONFIG_RTL_8196C)
		web_write_chunked("wlan_wapi.addItem(\"Certification Install\", \"wlwapiinstallcert.htm\", \"\", \"Install Ceritification\");");
#ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
		web_write_chunked("wlan_wapi.addItem(\"Certification Manage\", \"wlwapiCertManagement.htm\", \"\", \"Manage Ceritification\");");
#endif
//#endif
		web_write_chunked("for(i=0; i < wlan_num ; i++){");
		web_write_chunked("wlan_name= \"wlan\" +(i+1);");
		web_write_chunked("if(wlan_num == 1)");
		web_write_chunked("wlan0_wapi = wlan_wapi ;");
		web_write_chunked("else{");
		web_write_chunked("if(1 == wlan_support_92D){");
		web_write_chunked("if(i==0 && wlan1_phyband != \"\"){");
		web_write_chunked("wlan_name=wlan_name+\"(\"+wlan1_phyband+\")\";");
		web_write_chunked("}else if(i==1 && wlan2_phyband != \"\"){");
		web_write_chunked("wlan_name=wlan_name+\"(\"+wlan2_phyband+\")\";");
		web_write_chunked("}else{");
		web_write_chunked("continue;}}");
		web_write_chunked("if(wlBandMode == 3)"); //3:BANDMODESIGNLE
		web_write_chunked("wlan_name = \"wlan1\";");
		web_write_chunked("wlan_wapi.addItem(wlan_name);");
		web_write_chunked("wlan0_wapi= new MTMenu();}");
		web_write_chunked("wlan0_wapi.addItem(\"Key Update\", get_form(\"wlwapiRekey.htm\",i), \"\", \"Key update\");");
		web_write_chunked("if(wlan_num != 1)");
		web_write_chunked("wlan_wapi.makeLastSubmenu(wlan0_wapi);");
		web_write_chunked("}");
		web_write_chunked("menu.makeLastSubmenu(wlan_wapi);");
#endif
		
		free(tmpbuf);
		free(buffer);
		return 0;
	}
	else if(!strcmp(argv[0],"wapiCertSupport"))
	{
// 8198 and POCKET ROUTER support both wapi psk and wapi cert
// 8196c (not include POCKET ROUTER) only support wapi psk
//#if defined(CONFIG_RTL_8198) || defined(CONFIG_POCKET_ROUTER_SUPPORT)
#if defined(CONFIG_RTL_8198) || defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_8196C) 

#else
		web_write_chunked("disabled");
#endif
		free(tmpbuf);
		free(buffer);
		return 0;
	}
	else if(!strcmp(name, "wapiUcastTime"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_UCAST_TIME,  (void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	//	return 0;
	}else if(!strcmp(name, "wapiUcastPackets"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_UCAST_PACKETS, (void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	//	return 0;
	}	else if(!strcmp(name, "wapiMcastTime"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_MCAST_TIME,  (void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	//	return 0;
	}
	else if(!strcmp(name, "wapiMcastPackets"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_MCAST_PACKETS,  (void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	//	return 0;
	}
	else if(!strcmp(name, "wapiPskValue"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_PSK,	(void*)buffer))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		translate_control_code(buffer);
		counter=web_write_chunked("%s", buffer);
		free(tmpbuf);
		free(buffer);
		return counter;		
		//return 0;
	}else if(!strcmp(name, "wapiASIp"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_ASIPADDR,  (void*)buffer))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if (!memcmp(buffer, "\x0\x0\x0\x0", 4))
		{
			free(tmpbuf);
			free(buffer);
			return web_write_chunked("");
		}
		
		counter=web_write_chunked("%s", inet_ntoa(*((struct in_addr *)buffer)));
		free(tmpbuf);
		free(buffer);
		return counter;
	}
	else if(!strcmp(name, "wapiCertSel"))
	{
		if ( !apmib_get(MIB_WLAN_WAPI_CERT_SEL,  (void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
		//return 0;
	}
	else if(!strcmp(name,"wapiCert"))
	{
		int index;
		int count;
		int i;
		struct stat status;
		char tmpbuf[10];

		CERTS_DB_ENTRY_Tp cert=(CERTS_DB_ENTRY_Tp)malloc(128*sizeof(CERTS_DB_ENTRY_T));
		//Search Index 1--all, 2--serial.no, 3--owner, 4--type, 5--status
		if (!apmib_get(MIB_WLAN_WAPI_SEARCHINDEX,  (void*)&index))
		{
			free(cert);
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if(!apmib_get(MIB_WLAN_WAPI_SEARCHINFO,  (void*)buffer))
		{
			free(cert);
			free(tmpbuf);
			free(buffer);
			return -1;
		}

		/*update wapiCertInfo*/
		system("openssl ca -updatedb 2>/dev/null");
		if (stat(WAPI_CERT_CHANGED, &status) == 0) { // file existed
			system("storeWapiFiles -allUser");
		}

		count=searchWapiCert(cert,index,buffer);
		if(count == 0)
			web_write_chunked("%s","[]");
		else
		{
			web_write_chunked("%s","[");
			for(i=0;i<count;i++)
			{
				sprintf(tmpbuf, "%08X",cert[i].serial);
				web_write_chunked("['%s','%s','%d','%d',",cert[i].userName,tmpbuf,cert[i].validDays,cert[i].validDaysLeft);
				if(0 == cert[i].certType)
				{
					web_write_chunked("'%s',","X.509");
				}
				if(0==cert[i].certStatus)
				{
					web_write_chunked("'%s'","actived");
				}else if(1 ==cert[i].certStatus)
				{
					web_write_chunked("'%s'","expired");
				}else if(2 ==cert[i].certStatus)
				{
					web_write_chunked("%s'","revoked");
				}
				if(i ==(count-1))
					web_write_chunked("%s","]");
				else
					web_write_chunked("%s","],");
			}
			web_write_chunked("%s","]");
		}
		free(cert);
		free(tmpbuf);
		free(buffer);
		return 0;
	}
	else if(!strcmp(name,"caCertExist"))
	{
		 struct stat status;
		 if (stat(CA_CERT, &status) < 0)
		 {
			intVal=0;	//CA_CERT not exist
		 }
		 else
		 {
			intVal=1;	//CA_CERT exists
		 }
		 ret=snprintf(tmpbuf,TMP_BUF_SIZE,	"%d", intVal );  
		//return 0;
	}
	else if(!strcmp(name,"asCerExist"))
	{
		 struct stat status;
		 if (stat(CA_CER, &status) < 0)
		 {
			intVal=0;	//AS_CER not exist
		 }
		 else
		 {
			intVal=1;	//AS_CER exists
		 }
		 ret=snprintf(tmpbuf,TMP_BUF_SIZE,	"%d", intVal );  
		//return 0;
	}
	else if(!strcmp(name,"notSyncSysTime"))
	{
		 struct stat status;
		 time_t  now;
			struct tm *tnow;

		 if (stat(SYS_TIME_NOT_SYNC_CA, &status) < 0)
		 {
			//SYS_TIME_NOT_SYNC_CA not exist

			now=time(0);
						tnow=localtime(&now);
					//printf("now=%ld, %d %d %d %d %d %d, tm_isdst=%d\n",now, 1900+tnow->tm_year,tnow->tm_mon+1,tnow->tm_mday,tnow->tm_hour,tnow->tm_min,tnow->tm_sec, tnow->tm_isdst);//Added for test

			if(1900+tnow->tm_year < 2009)
			{
				intVal=1;	//current year of our system < 2009 which means our system hasn't sync time yet
			}
			else
			{
				intVal=0;	//SYS_TIME_NOT_SYNC_CA not exist and current time >= year 2009 which means our system has sync time already
			}
		 }
		 else
		 {
			intVal=1;	//SYS_TIME_NOT_SYNC_CA exists which means our system hasn't sync time yet
			sprintf(buffer, "rm -f %s 2>/dev/null", SYS_TIME_NOT_SYNC_CA);
			system(buffer);
		 }
		 ret=snprintf(tmpbuf,TMP_BUF_SIZE,	"%d", intVal );  
		//return 0;
	}
	 
	else if(!strcmp(argv[0],"wapiLocalAsSupport"))
	{
#ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
		web_write_chunked("%s","true");
#else
		web_write_chunked("%s","false");
#endif
		free(tmpbuf);
		free(buffer);
		return 0;
	}
#else
	else if(!strncmp(name,"wapi",4))
	{
		/*if wapi not enabled*/
		free(tmpbuf);
		free(buffer);
		return 0;
	}
	else if(!strcmp(name,"wapiLocalAsSupport"))
	{
		web_write_chunked("%s","false");
		free(tmpbuf);
		free(buffer);
		return 0;
	}
#endif
	else if ( !strcmp(name, "clientnum")) {
		apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&intVal);

		if (intVal == 1)	// disable
			intVal = 0;
		else if(!check_wlan_downup(apmib_get_wlanidx()))//if wlanx down
			intVal = 0;
		else {
			if ( getWlStaNum(WLAN_IF, &intVal) < 0)
			intVal = 0;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,	"%d", intVal ); 
	}

#if 0	
	else if(!strcmp(name,"wapiLocalAsSupport"))
	{}
	else if(!strcmp(name,"wapiOption"))
	{}
	else if(!strcmp(name,"wapiLocalAsOption"))
	{}	
	else if(!strcmp(name,"wapiPskValue"))
	{}	
	else if(!strcmp(name,"wapiASIp"))
	{}	
	else if(!strcmp(name,"wapiCertSel"))
	{}	
#endif	
	else if(!strcmp(name,"rsIp"))
	{
		if ( !apmib_get( MIB_WLAN_RS_IP,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if (!memcmp(buffer, "\x0\x0\x0\x0", 4))
		{
			free(tmpbuf);
			free(buffer);
			return web_write_chunked("");
		}
		
		counter=web_write_chunked("%s", inet_ntoa(*((struct in_addr *)buffer)) );
		free(tmpbuf);
		free(buffer);
		return counter;
	}	
	else if(!strcmp(name,"rsPort"))
	{
		if ( !apmib_get( MIB_WLAN_RS_PORT, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,  "%d", intVal );	
	}	
	else if(!strcmp(name,"rsPassword"))
	{
		if ( !apmib_get( MIB_WLAN_RS_PASSWORD,  (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=strlen(tmpbuf);
	}	
	else if(!strcmp(name,"eapUserId"))
	{
 #ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		if ( !apmib_get( MIB_WLAN_EAP_USER_ID,  (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
#endif
  		ret=strlen(tmpbuf);
	}	
	else if(!strcmp(name,"radiusUserName"))
	{
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		if ( !apmib_get( MIB_WLAN_RS_USER_NAME,  (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
#endif
  		ret=strlen(tmpbuf);
	}	
	else if(!strcmp(name,"radiusUserPass"))
	{
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		if ( !apmib_get( MIB_WLAN_RS_USER_PASSWD,  (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
#endif	
		ret=strlen(tmpbuf);
	}	
	else if(!strcmp(name,"radiusUserCertPass"))
	{
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		if ( !apmib_get( MIB_WLAN_RS_USER_CERT_PASSWD,  (void *)tmpbuf) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
#endif	
		ret=strlen(tmpbuf);
	}
	else if(!strcmp(name,"wdsPskValue"))
	{
		int i;
		tmpbuf[0]='\0';
		if ( !apmib_get(MIB_WLAN_WDS_PSK,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
#if 0
		for (i=0; i<strlen(tmpbuf); i++)
			tmpbuf[i]='*';
		tmpbuf[i]='\0';
		ret=strlen(tmpbuf);
#endif
		translate_control_code(buffer);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE,buffer);

	}
#endif /*NOT HAVE_NOWIFI*/
	else if(!strcmp(name,"ip-rom"))
	{
		if ( !apmib_get( MIB_IP_ADDR,  (void *)buffer) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		counter=web_write_chunked("%s", inet_ntoa(*((struct in_addr *)buffer)) );
		free(tmpbuf);
		free(buffer);
		return counter;
	}
	else if(!strcmp(name,"fwVersion"))
	{
		ret =snprintf(tmpbuf, TMP_BUF_SIZE, "%s", fwVersion);
	}
	else if ( !strcmp(name, "uptime")) {
#if 0
	struct sysinfo info ;

	sysinfo(&info);
	sec = (unsigned long) info.uptime ;

	sec = (unsigned long)get_uptime();
#else	
	sec = (unsigned long)get_uptime();
#endif
	day = sec / 86400;
	sec %= 86400;
	hr = sec / 3600;
	sec %= 3600;
	mn = sec / 60;
	sec %= 60;

    free(tmpbuf);
    free(buffer);
return web_write_chunked( "%ldday:%ldh:%ldm:%lds",
						day, hr, mn, sec);
	}
	else if ( !strcmp(name, "buildTime")) {
#if 0
		FILE *fp;
		regex_t re;
		regmatch_t match[2];
		int status;

		fp = fopen("/proc/version", "r");
		if (!fp) {
			fprintf(stderr, "Read /proc/version failed!\n");
			return web_write_chunked( "<script>dw(status_unknown)</script>");
	   	}
		else
		{
			fgets(buffer, sizeof(buffer), fp);
			fclose(fp);
		}

		if (regcomp(&re, "#[0-9][0-9]* \\(.*\\)$", 0) == 0)
		{
			status = regexec(&re, buffer, 2, match, 0);
			regfree(&re);
			if (status == 0 &&
				match[1].rm_so >= 0)
			{
				buffer[match[1].rm_eo] = 0;
   				return web_write_chunked(  &buffer[match[1].rm_so]);
			}
		}
#endif
		free(tmpbuf);
		free(buffer);
		return web_write_chunked(BUILD_TIME);
	}
#ifdef HOME_GATEWAY
	else if ( !strcmp(name, "dmzHost")) {
	if ( !apmib_get( MIB_DMZ_HOST,  (void *)buffer) )
	{
		free(tmpbuf);
		free(buffer);
		return -1;
	}
	
#ifdef IPCONFLICT_UPDATE_FIREWALL
		*((unsigned int*)(buffer))=get_conflict_ip(*((unsigned int*)(buffer))); 
#endif
		if (!memcmp(buffer, "\x0\x0\x0\x0", 4))
		{
			free(tmpbuf);
			free(buffer);
			return web_write_chunked("");
		}
		
		counter=web_write_chunked("%s", inet_ntoa(*((struct in_addr *)buffer)) );
		free(tmpbuf);
		free(buffer);
	    return counter;
	}
#endif
	/*sntp web page*/
else if ( !strcmp(name, "year")) {

		time(&current_secs);
		tm_time = localtime(&current_secs);
		#if 0
		sprintf(buffer , "%2d/%2d/%d %2d:%2d:%2d %s",
				(tm_time->tm_mon),
				(tm_time->tm_mday), (tm_time->tm_year+ 1900),
				(tm_time->tm_hour),
				(tm_time->tm_min),(tm_time->tm_sec)
				, _tzname[tm_time->tm_isdst]);
		#endif
		ret=sprintf(tmpbuf,"%d", (tm_time->tm_year+ 1900));
	}	
	else if ( !strcmp(name, "month")) {
		time(&current_secs);
		tm_time = localtime(&current_secs);
		ret=sprintf(tmpbuf,"%d", (tm_time->tm_mon+1));
	}
	else if ( !strcmp(name, "day")) {
		time(&current_secs);
		tm_time = localtime(&current_secs);
		ret=sprintf(tmpbuf,"%d", (tm_time->tm_mday));
	}
	else if ( !strcmp(name, "hour")) {
		time(&current_secs);
		tm_time = localtime(&current_secs);
		ret=sprintf(tmpbuf,"%d", (tm_time->tm_hour));
	}
	else if ( !strcmp(name, "minute")) {
		time(&current_secs);
		tm_time = localtime(&current_secs);
		ret=sprintf(tmpbuf,"%d", (tm_time->tm_min));
	}
	else if ( !strcmp(name, "second")) {
		time(&current_secs);
		tm_time = localtime(&current_secs);
		ret=sprintf(tmpbuf,"%d", (tm_time->tm_sec));
	}
	else if ( !strcmp(name, "state_drv")) {
		char *pMsg;
		if ( getWlBssInfo(WLAN_IF, &bss) < 0)
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		switch (bss.state) {
		case STATE_DISABLED:
			pMsg = "Disabled";
			break;
		case STATE_IDLE:
			pMsg = "Idle";
			break;
		case STATE_STARTED:
			pMsg = "Started";
			break;
		case STATE_CONNECTED:
			pMsg = "Connected";
			break;
		case STATE_WAITFORKEY:
			pMsg = "Waiting for keys";
			break;
		case STATE_SCANNING:
			pMsg = "Scanning";
			break;
		default:
			pMsg=NULL;
		}
		ret = sprintf(tmpbuf, "%s", pMsg);
	}

#ifdef UNIVERSAL_REPEATER
	else if ( !strcmp(name, "repeaterState")) {
		char *pMsg;
		if (apmib_get_wlanidx() == 0)
			strcpy(buffer, "wlan0-vxd0");
		else
			strcpy(buffer, "wlan1-vxd0");
		getWlBssInfo(buffer, &bss);
		switch (bss.state) {
		case STATE_DISABLED:
			pMsg = "Disabled";
			break;
		case STATE_IDLE:
			pMsg = "Idle";
			break;
		case STATE_STARTED:
			pMsg = "Started";
			break;
		case STATE_CONNECTED:
			pMsg = "Connected";
			break;
		case STATE_WAITFORKEY:
			pMsg = "Waiting for keys";
			break;
		case STATE_SCANNING:
			pMsg = "Scanning";
			break;
		default:
			pMsg=NULL;
		}
		ret = sprintf(tmpbuf, "%s", pMsg);
	}
 	else if ( !strcmp(name, "repeaterClientnum")) {
		if (apmib_get_wlanidx() == 0)
			strcpy(buffer, "wlan0-vxd0");
		else
			strcpy(buffer, "wlan1-vxd0");
 		if(getWlStaNum(buffer, &intVal)<0)
 			intVal=0;
		sprintf(buffer, "%d", intVal );
		ret = sprintf(tmpbuf, buffer);
	}
	else if ( !strcmp(name, "wlanRepeaterTxPacketNum")) {
		if (apmib_get_wlanidx() == 0)
			strcpy(buffer, "wlan0-vxd");
		else
			strcpy(buffer, "wlan1-vxd");
		if ( getStats(buffer, &stats) < 0)
			stats.tx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.tx_packets);

	}		
	else if ( !strcmp(name, "wlanRepeaterRxPacketNum")) {
		if (apmib_get_wlanidx() == 0)
			strcpy(buffer, "wlan0-vxd");
		else
			strcpy(buffer, "wlan1-vxd");
		if ( getStats(buffer, &stats) < 0)
			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d",(int)stats.rx_packets);
	}
	else if ( !strcmp(name, "repeaterSSID_drv")) {
#if 0 //defined(CONFIG_RTL_819X) && !defined(CONFIG_WLAN_REPEATER_MODE)// keith. disabled if no this mode in 96c
		ret =  sprintf(tmpbuf, "%s", "e0:00:19:78:01:10");
#else
		if (apmib_get_wlanidx() == 0)
			strcpy(buffer, "wlan0-vxd0");
		else
			strcpy(buffer, "wlan1-vxd0");
		getWlBssInfo(buffer, &bss);
		sprintf(buffer,"%s",bss.ssid);
		translate_control_code(buffer);
		ret =  sprintf(tmpbuf,"%s", buffer);
#endif
	}
	else if ( !strcmp(name, "repeaterBSSID")) {
		if (apmib_get_wlanidx() == 0)
			strcpy(buffer, "wlan0-vxd0");
		else
			strcpy(buffer, "wlan1-vxd0");
		getWlBssInfo(buffer, &bss);
		ret =  sprintf(tmpbuf, "%02x:%02x:%02x:%02x:%02x:%02x", bss.bssid[0], bss.bssid[1],
				bss.bssid[2], bss.bssid[3], bss.bssid[4], bss.bssid[5]);
	}
#endif // UNIVERSAL_REPEATER
	else if ( !strcmp(name, "wlanModeByStr")) {
		if ( !apmib_get( MIB_WLAN_MODE, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
        
        if(intVal==AP_MODE){
            #if defined(RTL_MULTI_REPEATER_MODE_SUPPORT)
            if(atoi(argv[1])==2 || atoi(argv[1])==3)
                ret=sprintf(tmpbuf, "%s", "STA");
            else
            #endif
                ret=sprintf(tmpbuf, "%s", "AP");
        }else if(intVal==CLIENT_MODE){
		    ret=sprintf(tmpbuf, "%s", "STA");
        }else{
		    ret=sprintf(tmpbuf, "%d", intVal);
        }
	}  
    else if (!strcmp(name,"WLDISABLE_diselect")) {
	    int id;
		if (0 == apmib_get_wlanidx())
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		if ( !apmib_get( id, (void *)&intVal) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
	#if defined(RTL_MULTI_REPEATER_MODE_SUPPORT)
    if((intVal!=2) && (atoi(argv[1])==2 || atoi(argv[1])==3))
   		ret=sprintf(tmpbuf, "%s", "disabled");
    else
	#endif
   		ret=sprintf(tmpbuf, "%s", "");
    }
    else if (!strcmp(name,"wlanModeDisabled")) {
       #if defined(RTL_MULTI_REPEATER_MODE_SUPPORT)
       if(atoi(argv[1])==2 || atoi(argv[1])==3)
            ret=sprintf(tmpbuf, "%s", "disabled");
	   else
       #endif
            ret=sprintf(tmpbuf, "%s", "");
    }
	else if ( !strcmp(name, "ssid_drv")) {	
		if ( getWlBssInfo(WLAN_IF, &bss) < 0)
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		memcpy(buffer, bss.ssid, SSID_LEN+1);
		translate_control_code(buffer);
		ret=sprintf(tmpbuf,"%s", buffer);
	}
	else if ( !strcmp(name, "bssid_drv")) {
		if ( getWlBssInfo(WLAN_IF, &bss) < 0)
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret = sprintf(tmpbuf,"%02x:%02x:%02x:%02x:%02x:%02x", bss.bssid[0], bss.bssid[1],
				bss.bssid[2], bss.bssid[3], bss.bssid[4], bss.bssid[5]);
	}
	else if ( !strcmp(name, "channel_drv")) {
		if ( getWlBssInfo(WLAN_IF, &bss) < 0)
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if (bss.channel)
			sprintf(buffer, "%d", bss.channel);
		else{
			strcpy(buffer,"0");
			//buffer[0] = '\0';
		}

		ret=sprintf(tmpbuf,"%s", buffer);
	}
	else if ( !strcmp(name, "wlanTxPacketNum")) {
		if ( getStats(WLAN_IF, &stats) < 0)
			stats.tx_packets = 0;
		ret = sprintf(tmpbuf, "%d", (int)stats.tx_packets);

	}
	else if ( !strcmp(name, "wlanRxPacketNum")) {
		if ( getStats(WLAN_IF, &stats) < 0)
			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.rx_packets);
   		
	}
	else if ( !strcmp(name, "lanTxPacketNum")) {
		if ( getStats(ELAN_IF, &stats) < 0)
			stats.tx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.tx_packets);
	}
	else if ( !strcmp(name, "lanRxPacketNum")) {
		if ( getStats(ELAN_IF, &stats) < 0)
			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)stats.rx_packets);
	}
	else if ( !strcmp(name, "lan2TxPacketNum")) {
#if defined(VLAN_CONFIG_SUPPORTED)
			if ( getStats(ELAN2_IF, &stats) < 0)
			stats.tx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.tx_packets);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if ( !strcmp(name, "lan2RxPacketNum")) {
#if defined(VLAN_CONFIG_SUPPORTED)
		if ( getStats(ELAN2_IF, &stats) < 0)
			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.rx_packets);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", 0);
#endif
	}
	else if ( !strcmp(name, "lan3TxPacketNum")) {
#if defined(VLAN_CONFIG_SUPPORTED)
		if ( getStats(ELAN3_IF, &stats) < 0)

			stats.tx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.tx_packets);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", 0);
#endif
	}
	else if ( !strcmp(name, "lan3RxPacketNum")) {
#if defined(VLAN_CONFIG_SUPPORTED)
		if ( getStats(ELAN3_IF, &stats) < 0)

			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.rx_packets);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if ( !strcmp(name, "lan4TxPacketNum")) {
#if defined(VLAN_CONFIG_SUPPORTED)
		if ( getStats(ELAN4_IF, &stats) < 0)

			stats.tx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d", (int)stats.tx_packets);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if ( !strcmp(name, "lan4RxPacketNum")) {
#if defined(VLAN_CONFIG_SUPPORTED)
	if ( getStats(ELAN4_IF, &stats) < 0)

			stats.rx_packets = 0;
		ret = snprintf(tmpbuf,TMP_BUF_SIZE,"%d", (int)stats.rx_packets);
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "%d", 0);
#endif
	}
	else if(!strcmp(name,"show_netif_sel"))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		NETIFACE_T netifEntry={0};
		if(!apmib_get(MIB_NETIFACE_TBL_NUM,(void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=0;
		bzero(tmpbuf,sizeof(tmpbuf));
		for(i=1;i<=intVal;i++)
		{
			*((char *)&netifEntry) = (char)i;
			if (!apmib_get(MIB_NETIFACE_TBL,(void *)&netifEntry))
			{
				free(tmpbuf);
				free(buffer);
				return -1;
			}
			ret+=snprintf(buffer,TMP_BUF_SIZE, "<option value=%d>%s</option>\n",netifEntry.netifId,netifEntry.netifName);
			strcat(tmpbuf,buffer);
		}
		cyg_httpd_write_chunked(tmpbuf,ret);
		
		free(tmpbuf);
		free(buffer);
		return ret;
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "<option value=0>vlan not support</option>\n");
#endif
	}
	else if(!strcmp(name,"show_vlan_sel"))
	{
#ifdef CONFIG_RTL_VLAN_SUPPORT
		VLAN_T vlanEntry={0};
		if(!apmib_get(MIB_VLAN_TBL_NUM,(void*)&intVal))
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		ret=0;
		bzero(tmpbuf,sizeof(tmpbuf));
		//return 0;
		for(i=1;i<=intVal;i++)
		{
			*((char *)&vlanEntry) = (char)i;
			if (!apmib_get(MIB_VLAN_TBL,(void *)&vlanEntry))
			{
				free(tmpbuf);
				free(buffer);
				return -1;
			}
			ret+=snprintf(buffer,TMP_BUF_SIZE, "<option value=%d>%d</option>\n",vlanEntry.vlanId,vlanEntry.vlanId);
			strcat(tmpbuf,buffer);
		}
		cyg_httpd_write_chunked(tmpbuf,ret);
		
		free(tmpbuf);
		free(buffer);
		return ret;
#else
		ret = snprintf(tmpbuf,TMP_BUF_SIZE, "<option value=0>vlan not support</option>\n");
#endif
	}
	else if ( !strcmp(name, "wep")) {
		ENCRYPT_T encrypt;

		strcpy( buffer, "Disabled");

#if defined(WLAN_PROFILE)
		int wlan_mode;
		
		int profile_enabled_id, profileEnabledVal,profile_num_id,profileNum,profile_table_id;
		int wlan_idx=apmib_get_wlanidx();
		int vwlan_idx=apmib_get_vwlanidx();
//printf("\r\n wlan_idx=[%d],vwlan_idx=[%d],__[%s-%u]\r\n",wlan_idx,vwlan_idx,__FILE__,__LINE__);	
#if 1
		if(wlan_idx == 0)
		{
			profile_enabled_id = MIB_PROFILE_ENABLED1;
			profile_num_id = MIB_PROFILE_NUM1;
			profile_table_id=MIB_PROFILE_TBL1;
		}
		else
		{
			profile_enabled_id = MIB_PROFILE_ENABLED2;
			profile_num_id = MIB_PROFILE_NUM2;			
			profile_table_id=MIB_PROFILE_TBL2;
		}
#endif
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);


		apmib_get(profile_enabled_id, (void *)&profileEnabledVal);
		apmib_get(profile_num_id, (void *)&profileNum);

		if( (vwlan_idx == 0 || vwlan_idx == NUM_VWLAN_INTERFACE) 
			&& profileEnabledVal == 1 
			&& (wlan_mode == CLIENT_MODE)&&profileNum>0 )
		{
#if 0
			if( vwlan_idx == NUM_VWLAN_INTERFACE)
			{
				sprintf(ifname,"wlan%d-vxd",wlan_idx);
			}
			else
			{
				sprintf(ifname,"wlan%d",wlan_idx);
			}
			sprintf(openFileStr,"cat /proc/%s/mib_ap_profile | grep in_use_profile",ifname);
			fp = open(openFileStr, "r");
			if(fp && (NULL != fgets(inUseProfileStr, sizeof(inUseProfileStr),fp)))
			{
				char *searchPtr;

				searchPtr = strstr(inUseProfileStr,"in_use_profile"); //move to first, 

				sscanf(searchPtr, "in_use_profile: %d", &inUseProfile);
				close(fp);
			}

			if(inUseProfile >= 0)
			{
				WLAN_PROFILE_T entry;
				memset(&entry,0x00, sizeof(WLAN_PROFILE_T));
				*((char *)&entry) = (char)(inUseProfile+1);

				if(wlan_idx == 0)
					apmib_get(MIB_PROFILE_TBL1, (void *)&entry);
				else
					apmib_get(MIB_PROFILE_TBL2, (void *)&entry);
				
				if (entry.encryption == WEP64)
					strcpy( buffer, "WEP 64bits");
				else if (entry.encryption == WEP128)
					strcpy( buffer, "WEP 128bits");
				else if (entry.encryption == 3)
					strcpy( buffer, "WPA");
				else if (entry.encryption == 4)
		                      strcpy( buffer, "WPA2");
				else 
		                      strcpy( buffer, "Disabled");

//printf("\r\n buffer[%s],__[%s-%u]\r\n",buffer,__FILE__,__LINE__);			

			}
#else
			WLAN_PROFILE_T entry;
			if ( getWlBssInfo(WLAN_IF, &bss) < 0)
			{
				free(tmpbuf);
				free(buffer);
				return -1;
			}
			for(i=1;i<=profileNum;i++)
			{
				*((char *)&entry) = (char)(i);
				apmib_get(profile_table_id, (void *)&entry);

				if(!strcmp(bss.ssid,entry.ssid))
				{
					if (entry.encryption == WEP64)
					strcpy( buffer, "WEP 64bits");
				else if (entry.encryption == WEP128)
					strcpy( buffer, "WEP 128bits");
				else if (entry.encryption == 3)
					strcpy( buffer, "WPA");
				else if (entry.encryption == 4)
						  strcpy( buffer, "WPA2");
				else 
						  strcpy( buffer, "Disabled");
				}
			}
#endif

		}
		else
#endif //#if defined(WLAN_PROFILE
		{
		
                if ( !apmib_get( MIB_WLAN_ENCRYPT,  (void *)&encrypt) )
				{
					free(tmpbuf);
					free(buffer);
					return -1;
				}
                if (encrypt == ENCRYPT_DISABLED)
                        strcpy( buffer, "Disabled");
                else if (encrypt == ENCRYPT_WPA)
                        strcpy( buffer, "WPA");
				else if (encrypt == ENCRYPT_WPA2)
			                    strcpy( buffer, "WPA2");
				else if (encrypt == (ENCRYPT_WPA | ENCRYPT_WPA2))
			                    strcpy( buffer, "WPA2 Mixed");
				else if (encrypt == ENCRYPT_WAPI)
				strcpy(buffer,"WAPI");
                else {
                        WEP_T wep;
                        if ( !apmib_get( MIB_WLAN_WEP,  (void *)&wep) )
						{
							free(tmpbuf);
							free(buffer);
							return -1;
						}
                        if ( wep == WEP_DISABLED )
                                strcpy( buffer, "Disabled");
                        else if ( wep == WEP64 )
                                strcpy( buffer, "WEP 64bits");
                        else if ( wep == WEP128)
                                strcpy( buffer, "WEP 128bits");
                }
		}
                ret=sprintf(tmpbuf,"%s",buffer);
	}
	else if ( !strcmp(name, "wdsEncrypt")) {
   		WDS_ENCRYPT_T encrypt;
		if ( !apmib_get( MIB_WLAN_WDS_ENCRYPT,  (void *)&encrypt) )
		{
			free(tmpbuf);
			free(buffer);
			return -1;
		}
		if ( encrypt == WDS_ENCRYPT_DISABLED)
			strcpy( buffer, "Disabled");
		else if ( encrypt == WDS_ENCRYPT_WEP64)
			strcpy( buffer, "WEP 64bits");
		else if ( encrypt == WDS_ENCRYPT_WEP128)
			strcpy( buffer, "WEP 128bits");
		else if ( encrypt == WDS_ENCRYPT_TKIP)
			strcpy( buffer, "TKIP");
		else if ( encrypt == WDS_ENCRYPT_AES)
			strcpy( buffer, "AES");
		else
			buffer[0] = '\0';
   		ret = sprintf(tmpbuf,"%s", buffer);
   	}
	else if ( !strcmp(name, "pocketRouter_html_wan_hide_s")) {
			apmib_get( MIB_OP_MODE, (void *)&intVal);
#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			if(intVal == 0)
				web_write_chunked( "%s","");
			else if(intVal == 1)
				web_write_chunked( "%s","<!--");
#elif defined(CONFIG_POCKET_AP_SUPPORT)
			web_write_chunked( "%s","<!--");
#elif defined(CONFIG_RTL_8198_AP_ROOT) || defined(CONFIG_ECOS_AP_SUPPORT)
			web_write_chunked( "%s","<!--");
#else
			web_write_chunked( "%s","");
#endif
			free(tmpbuf);
			free(buffer);
			return 0;
		}
		else if ( !strcmp(name, "pocketRouter_html_wan_hide_e")) {
			apmib_get( MIB_OP_MODE, (void *)&intVal);
#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			if(intVal == 0)
				web_write_chunked( "%s","");
			else if(intVal == 1)
				web_write_chunked( "%s","-->");
#elif defined(CONFIG_POCKET_AP_SUPPORT)
			web_write_chunked( "%s","-->");
#elif defined(CONFIG_RTL_8198_AP_ROOT) || defined(CONFIG_ECOS_AP_SUPPORT)
			web_write_chunked( "%s","-->");
#else
			web_write_chunked( "%s","");
#endif	
			free(tmpbuf);
			free(buffer);
			return 0;
		}
		else if ( !strcmp(name, "pocketRouter_html_lan_hide_s")) {
			apmib_get( MIB_OP_MODE, (void *)&intVal);
#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			if(intVal == 1)
				web_write_chunked( "%s","");
			else if(intVal == 0)
				web_write_chunked( "%s","<!--");
#else
			web_write_chunked( "%s","");
#endif
			free(tmpbuf);
			free(buffer);
			return 0;
		}
		else if ( !strcmp(name, "pocketRouter_html_lan_hide_e")) {
			apmib_get( MIB_OP_MODE, (void *)&intVal);
#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			if(intVal == 1)
				web_write_chunked( "%s","");
			else if(intVal == 0)
				web_write_chunked( "%s","-->");
#else
			web_write_chunked( "%s","");
#endif
			free(tmpbuf);
			free(buffer);
			return 0;
		}

/////////added by hf_shi/////////////////
#ifdef CONFIG_IPV6
	else if(!strncmp(name, "IPv6_",5)){
		    free(tmpbuf);
		    free(buffer);
			return getIPv6Info(argc, argv);
	}
#else
	else if (!strncmp(name, "IPv6_", 5)) {
		free(tmpbuf);
		free(buffer);
   		return 0;
	}
#endif

	for(i=0;i<NUM_WLAN_INTERFACE;i++)
	{
		sprintf(buffer,"wlan%d-status",i);
		if(!strcmp(name,buffer))
		{
			sprintf(WLAN_IF,"wlan%d",i);
			SetWlan_idx(WLAN_IF);
		}
	}

	/*end*/
	cyg_httpd_write_chunked(tmpbuf,ret);
    free(tmpbuf);
    free(buffer);
	return 0;
}

#ifdef MBSSID

int getVirtualIndex(int argc, char **argv)
{
	int ret, vwlan_idx;
	char WLAN_IF_old[40]={0};
	
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	vwlan_idx = atoi(argv[--argc]);
#if defined(CONFIG_RTL_ULINKER)
	if(vwlan_idx == 5) //vxd
		apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
#endif
	if (vwlan_idx > NUM_VWLAN_INTERFACE) {
		web_write_chunked("0");
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
		return 0;
	}


//#if defined(CONFIG_RTL_8196B)
//	if (vwlan_idx == 5) { //rtl8196b support repeater mode only first, no mssid
//#else
	if (vwlan_idx > 0) {
//#endif
		strcpy(WLAN_IF_old, WLAN_IF);
		sprintf(WLAN_IF, "%s-va%d", WLAN_IF_old, vwlan_idx-1);
	}


	apmib_set_vwlanidx(vwlan_idx);
	ret = getIndex(argc, argv);

//#if defined(CONFIG_RTL_8196B)
//	if (vwlan_idx == 5)
//#else
	if (vwlan_idx > 0)
//#endif
		strcpy(WLAN_IF, WLAN_IF_old);

#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif

	return ret;
}

int getVirtualInfo(int argc, char ** argv)
{
	int ret,  vwlan_idx;
	char WLAN_IF_old[40];
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	
	#if 0   
	{
		int i=0;
		printf("\nwlanidx=%d\t",wlan_idx);
		for(i=0;i<argc;i++)
		{
			printf("%s ",argv[i]);
		}
		printf("\n\n");
	 }
#endif
	vwlan_idx = atoi(argv[--argc]);
#if defined(CONFIG_RTL_ULINKER)
	if(vwlan_idx == 5) //vxd
		apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
#endif	

	if (vwlan_idx > NUM_VWLAN_INTERFACE) {
		//fprintf(stderr, "###%s:%d wlan_idx=%d vwlan_idx=%d###\n", __FILE__, __LINE__, apmib_get_wlanidx(), vwlan_idx);
		web_write_chunked("0");
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
		return 0;
	}
	
	//#if defined(CONFIG_RTL_8196B)
	//	if (vwlan_idx == 5) { //rtl8196b support repeater mode only first, no mssid
	//#else
	if (vwlan_idx > 0) {
	//#endif
		strcpy(WLAN_IF_old, WLAN_IF);
		sprintf(WLAN_IF, "%s-va%d", WLAN_IF_old, vwlan_idx-1);
	}

	apmib_set_vwlanidx(vwlan_idx);
	//fprintf(stderr, "###%s:%d wlan_idx=%d vwlan_idx=%d###\n", __FILE__, __LINE__, wlan_idx, vwlan_idx);
	ret = getInfo(argc, argv);
	//fprintf(stderr, "###%s:%d wlan_idx=%d vwlan_idx=%d###\n", __FILE__, __LINE__, wlan_idx, vwlan_idx);


	//#if defined(CONFIG_RTL_8196B)
	//	if (vwlan_idx == 5)
	//#else
	if (vwlan_idx > 0)
	//#endif
		strcpy(WLAN_IF, WLAN_IF_old);


#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
	return ret;
}
#else
int getVirtualIndex(int argc, char **argv)
{
	int vwlan_idx=0,ret=0;
	vwlan_idx = atoi(argv[--argc]);
	if(vwlan_idx!=0)
		web_write_chunked("0");
	else
		ret = getIndex(argc, argv);
	return ret;	

}
int getVirtualInfo(int argc, char ** argv)
{
	int vwlan_idx=0,ret=0;
	vwlan_idx = atoi(argv[--argc]);
	if(vwlan_idx!=0)
		web_write_chunked("0");
	else
		ret = getInfo(argc, argv);
	return ret;	
}
#endif

#if defined(HAVE_FIREWALL)
int urlFilterList(int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	URLFILTER_T entry;
	char tmpbuf[TMP_BUF_SIZE]="";
	if ( !apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"70%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(urlfilter_filterlist_yrladdr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	
	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_URLFILTER_TBL, (void *)&entry))
			return -1;

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"70%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",entry.urlAddr);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		
	}
	return nBytesSent;

}

int portFilterList(int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	PORTFILTER_T entry;
	char	*type, portRange[20];
	char tmpbuf[TMP_BUF_SIZE]="";

	if ( !apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_portrange)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_proto)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_comm)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_PORTFILTER_TBL, (void *)&entry))
			return -1;

		if ( entry.protoType == PROTO_BOTH )
			type = "TCP+UDP";
		else if ( entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";

		if ( entry.fromPort == 0)
			strcpy(portRange, "----");
		else if ( entry.fromPort == entry.toPort )
			snprintf(portRange, 20, "%d", entry.fromPort);
		else
			snprintf(portRange, 20, "%d-%d", entry.fromPort, entry.toPort);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",portRange);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",type);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",entry.comment);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	
	}
	return nBytesSent;
}

int macFilterList(int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	MACFILTER_T entry;
	char tmpBuf[100];
	char tmpbuf[TMP_BUF_SIZE]="";
	if ( !apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"50%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(macfilter_filterlist_macaddr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_comm)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_MACFILTER_TBL, (void *)&entry))
			return -1;

		snprintf(tmpBuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",tmpBuf);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",entry.comment);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);

		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	}
	return nBytesSent;
}

int ipFilterList(int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	IPFILTER_T entry;
	char	*type, *ip;
	char tmpbuf[TMP_BUF_SIZE]="";
    #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
    char tmpIpAddrbuf[sizeof "123.456.789.0123"] = {0};
    char ipRange[64] = {0};
    #endif

	if ( !apmib_get(MIB_IPFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
    #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
    nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"60%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_localipaddr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_proto)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_comm)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
    #else
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<tr><td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_localipaddr)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
		"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_proto)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_comm)</script></b></font></td>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(firewall_tbl_select)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
    #endif
	
	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_IPFILTER_TBL, (void *)&entry))
			return -1;

#ifdef IPCONFLICT_UPDATE_FIREWALL               
		*((unsigned int*)(entry.ipAddr))=get_conflict_ip(*((unsigned int*)(entry.ipAddr)));
		*((unsigned int*)(entry.ipAddrEnd))=get_conflict_ip(*((unsigned int*)(entry.ipAddrEnd)));
#endif
		ip = inet_ntoa(*((struct in_addr *)entry.ipAddr));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = "----";
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        sprintf(tmpIpAddrbuf, "%s", ip);
        ip = inet_ntoa(*((struct in_addr *)entry.ipAddrEnd));  
		if ( !strcmp(ip, "0.0.0.0") || !strcmp(ip, tmpIpAddrbuf))	
            sprintf(ipRange, "%s", tmpIpAddrbuf);
        else
            sprintf(ipRange, "%s-%s", tmpIpAddrbuf, ip);            
        #endif

		if ( entry.protoType == PROTO_BOTH )
			type = "TCP+UDP";
		else if ( entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";
        
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<tr><td align=center width=\"60%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",ipRange);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",type);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",entry.comment);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"
				,i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
        #else
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
				"<tr><td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",ip);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",type);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",entry.comment);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		nBytesSent=snprintf(tmpbuf, TMP_BUF_SIZE,
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"
				,i);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
        #endif

	}
	return nBytesSent;
}
#endif

int getLangInfo(int argc, char ** argv)
{
	LANGUAGE_TYPE_T langType;
	char* name=argv[0];
	int ret=0;
	char*tmpbuf=NULL;
	if(!apmib_get( MIB_WEB_LANGUAGE, (void *)&langType))
	{
		printf("can't get MIB_WEB_LANGUAGE value.\n");
		return -1;
	}
	tmpbuf = malloc(TMP_BUF_SIZE);
	if(!tmpbuf)
	{
		printf("malloc fail!\n");
		return -1;
	}
	bzero(tmpbuf,TMP_BUF_SIZE);
	if(strcmp(name,"charset")==0)
	{
		switch(langType)
		{
#ifdef HAVE_MULTI_LANGUAGE
			case ENGLISH:
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"utf-8");	
				break;
			case SIMPLE_CHINESE:
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"utf-8");
				break;
#endif
			default:
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"utf-8");	
				break;
		}
	}
	else if(strcmp(name,"lang")==0)
	{
		switch(langType)
		{
#ifdef HAVE_MULTI_LANGUAGE
			case ENGLISH:
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"language_en.js");	
				break;
			case SIMPLE_CHINESE:
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"language_sc.js");	
				break;
#endif
			default:
				ret=snprintf(tmpbuf,TMP_BUF_SIZE,"language_en.js");	
				break;
		}
	}else
	{
		printf("can't find %s\n",name);
		ret=snprintf(tmpbuf,TMP_BUF_SIZE," ");	
	}
	cyg_httpd_write_chunked(tmpbuf,ret);
	if(tmpbuf)
	{
		free(tmpbuf);
		tmpbuf=NULL;
	}
	return 0;
}
