/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : wifi_radio.c
  版 本 号   : 初稿
  作    者   : lrl
  生成日期   : 2017年10月27日
  最近修改   :
  功能描述   : 

  功能描述   : 信道获取

  修改历史   :
  1.日    期   : 2017年10月27日
    作    者   : lrl
    修改内容   : 创建文件

******************************************************************************/

#include "wifi.h"

#ifdef D11AC_IOTYPES
static const char DE_aliases[]={"GB,ES,IT,FR,DE,NL,CZ,PL,EE,SE,CH,DK,BE,BG,AM,FI,SI,SK,RO,PT,AT,GR,DE,NL,CZ,PL"};
static const char US_aliases[]={"CA,US"};
static const char HK_aliases[]={"KR,RU,HK"};
#else
static const char CN_aliases[]={"KR,RU,CN"};
#endif
typedef struct countryIE {
	unsigned int		countryNumber;
	unsigned char 		countryA2[3];
	unsigned char		A_Band_Region;	//if support 5G A band? ;  0 == no support ; aBandRegion == real region domain
	unsigned char		G_Band_Region;	//if support 2.4G G band? ;  0 == no support ; bBandRegion == real region domain
	unsigned char 		countryName[24];
} COUNTRY_IE_ELEMENT;


static COUNTRY_IE_ELEMENT countryIEArray[] =
{
    /*
     format: countryNumber | CountryCode(A2) | region(2.4G) | region(5G) | long name
    */
    {8,"AL",   3, 3, "ALBANIA"},
    {12,"DZ",  3, 3, "ALGERIA"},
    {32,"AR",  3, 3, "ARGENTINA"},
    {51,"AM",  3, 3, "ARMENIA"},
    {36,"AU",  1, 3, "AUSTRALIA"},
    {40,"AT",  3, 3, "AUSTRIA"},
    {31,"AZ",  3, 3, "AZERBAIJAN"},
    {48,"BH",  3, 3, "BAHRAIN"},
    {112,"BY",  3, 3, "BELARUS"},
    {56,"BE",  3, 3, "BELGIUM"},
    {84,"BZ",  3, 3, "BELIZE"},
    {68,"BO",  3, 3, "BOLIVIA"},
    {76,"BR",  1, 3, "BRAZIL"},
    {96,"BN",  3, 3, "BRUNEI"},
    {100,"BG", 3, 3, "BULGARIA"},
    {124,"CA", 1, 1, "CANADA"},
    {152,"CL", 3, 3, "CHILE"},
    {156,"CN",13,13, "CHINA"},
    {170,"CO", 1, 1, "COLOMBIA"},
    {188,"CR", 3, 3, "COSTA RICA"},
    {191,"HR", 3, 3, "CROATIA"},
    {196,"CY", 3, 3, "CYPRUS"},
    {203,"CZ", 3, 3, "CZECH REPUBLIC"},
    {208,"DK", 3, 3, "DENMARK"},
    {214,"DO", 1, 1, "DOMINICAN REPUBLIC"},
    {218,"EC", 3, 3, "ECUADOR"},
    {818,"EG", 3, 3, "EGYPT"},
    {222,"SV", 3, 3, "EL SALVADOR"},
    {233,"EE", 3, 3, "ESTONIA"},
    {246,"FI", 3, 3, "FINLAND"},
    {250,"FR", 3, 3, "FRANCE"},
    {268,"GE", 3, 3, "GEORGIA"},
    {276,"DE", 3, 3, "GERMANY"},
    {300,"GR", 3, 3, "GREECE"},
    {320,"GT", 1, 1, "GUATEMALA"},
    {340,"HN", 3, 3, "HONDURAS"},
    {344,"HK", 1, 3, "HONG KONG"},
    {348,"HU", 3, 3, "HUNGARY"},
    {352,"IS", 3, 3, "ICELAND"},
    {356,"IN", 1, 1, "INDIA"},
    {360,"ID", 10, 3, "INDONESIA"},
    {364,"IR", 3, 3, "IRAN"},
    {372,"IE", 3, 3, "IRELAND"},
    {376,"IL", 7, 7, "ISRAEL"},
    {380,"IT", 3, 3, "ITALY"},
    {392,"JP", 6, 6, "JAPAN"},
    {400,"JO", 3, 3, "JORDAN"},
    {398,"KZ", 3, 3, "KAZAKHSTAN"},
    {410,"KR", 1, 3, "NORTH KOREA"},
    {408,"KP", 3, 3, "KOREA REPUBLIC"},
    {414,"KW", 3, 3, "KUWAIT"},
    {428,"LV", 3, 3, "LATVIA"},
    {422,"LB", 3, 3, "LEBANON"},
    {438,"LI", 3, 3, "LIECHTENSTEIN"},
    {440,"LT", 3, 3, "LITHUANIA"},
    {442,"LU", 3, 3, "LUXEMBOURG"},
    {446,"MO", 3, 3, "CHINA MACAU"},
    {807,"MK", 3, 3, "MACEDONIA"},
    {458,"MY", 3, 3, "MALAYSIA"},
    {484,"MX", 1, 3, "MEXICO"},
    {492,"MC", 3, 3, "MONACO"},
    {504,"MA", 3, 3, "MOROCCO"},
    {528,"NL", 3, 3, "NETHERLANDS"},
    {554,"NZ", 3, 3, "NEW ZEALAND"},
    {578,"NO", 3, 3, "NORWAY"},
    {512,"OM", 3, 3, "OMAN"},
    {586,"PK", 3, 3, "PAKISTAN"},
    {591,"PA", 1, 1, "PANAMA"},
    {604,"PE", 3, 3, "PERU"},
    {608,"PH", 3, 3, "PHILIPPINES"},
    {616,"PL", 3, 3, "POLAND"},
    {620,"PT", 3, 3, "PORTUGAL"},
    {630,"PR", 1, 1, "PUERTO RICO"},
    {634,"QA", 3, 3, "QATAR"},
    {642,"RO", 3, 3, "ROMANIA"},
    {643,"RU",12,12, "RUSSIAN"},
    {682,"SA", 2, 3, "SAUDI ARABIA"},
    {702,"SG", 3, 3, "SINGAPORE"},
    {703,"SK", 3, 3, "SLOVAKIA"},
    {705,"SI", 3, 3, "SLOVENIA"},
    {710,"ZA", 3, 3, "SOUTH AFRICA"},
    {724,"ES", 3, 3, "SPAIN"},
    {752,"SE", 3, 3, "SWEDEN"},
    {756,"CH", 3, 3, "SWITZERLAND"},
    {760,"SY", 3, 3, "SYRIAN ARAB REPUBLIC"},
    {158,"TW",13,11, "TAIWAN"},
    {764,"TH", 1, 3, "THAILAND"},
    {780,"TT", 3, 3, "TRINIDAD AND TOBAGO"},
    {788,"TN", 3, 3, "TUNISIA"},
    {792,"TR", 3, 3, "TURKEY"},
    {804,"UA", 3, 3, "UKRAINE"},
    {784,"AE", 1, 3, "UNITED ARAB EMIRATES"},
    {826,"GB", 3, 3, "UNITED KINGDOM"},
    {840,"US", 1, 1, "UNITED STATES"},
    {858,"UY", 3, 3, "URUGUAY"},
    {860,"UZ", 1, 1, "UZBEKISTAN"},
    {862,"VE", 3, 3, "VENEZUELA"},
    {704,"VN", 3, 3, "VIET NAM"},
    {887,"YE", 3, 3, "YEMEN"},
    {716,"ZW", 3, 3, "ZIMBABWE"},
	{70,"BA", 3, 3, "BOSNIA AND HERZEGOWINA"}, 
	{688,"RS", 3, 3, "SERBIA"},    
};

static CHANNEL_ELEMENT_T channels_2G_20M[] = 
{
    {0, 1,  {0}},
    {1, 11, {1,2,3,4,5,6,7,8,9,10,11}},         //FCC
    {2, 11, {1,2,3,4,5,6,7,8,9,10,11}},         //IC
    {3, 13, {1,2,3,4,5,6,7,8,9,10,11,12,13}},   //ETSI world
    {4, 13, {1,2,3,4,5,6,7,8,9,10,11,12,13}},   //SPAIN
    {5, 4, {10,11,12,13}},      //FRANCE
    {6, 14, {1,2,3,4,5,6,7,8,9,10,11,12,13,14}}, //MKK , Japan  
    {7, 11,  {1,2,3,4,5,6,7,8,9,10,11}},        //ISRAEL
    {8, 14,  {1,2,3,4,5,6,7,8,9,10,11,12,13,14}}, // MKK1
    {9, 14,  {1,2,3,4,5,6,7,8,9,10,11,12,13,14}}, // MKK2
    {10,14,  {1,2,3,4,5,6,7,8,9,10,11,12,13,14}}, // MKK3
    {11,11,  {1,2,3,4,5,6,7,8,9,10,11}},  //NCC (Taiwan)
    {12,13,  {1,2,3,4,5,6,7,8,9,10,11,12,13}}, // RUSSIAN
    {13,13,  {1,2,3,4,5,6,7,8,9,10,11,12,13}}, // CN
    {14,14,  {1,2,3,4,5,6,7,8,9,10,11,12,13,14}}, // GLOBAL
    {15,13,  {1,2,3,4,5,6,7,8,9,10,11,12,13}}, // World_wide
    {16,14,  {1,2,3,4,5,6,7,8,9,10,11,12,13,14}} // Test
};


static CHANNEL_ELEMENT_T channels_5G_20M[] = 
{
    {0, 1 ,{0}},
    {1, 20 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165}},  // FCC
    {2, 12 ,{36,40,44,48,52,56,60,64,149,153,157,161}}, // IC
    {3, 19 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140}}, // ETSI
    {4, 19 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140}}, // SPAIN    
    {5, 19 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140}}, // FRANCE
    {6, 19 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140}}, // MKK
    {7, 19 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140}}, // ISRAEL
    {8, 4 ,{34,38,42,46}},  // MKK1
    {9, 4 ,{34,38,42,46}},  // MKK2
    //{10, 8 ,{36,40,44,48,52,56,60,64}}, // MKK3 
    {10, 4 ,{149,153,157,161}}, // Indonesia 
    {11, 15 ,{56,60,64,100,104,108,112,116,136,140,149,153,157,161,165}}, // NCC(Taiwan)
    {12, 16 ,{36,40,44,48,52,56,60,64,132,136,140,149,153,157,161,165}}, // RUSSIAN
    {13, 13 ,{36,40,44,48,52,56,60,64,149,153,157,161,165}}, // CN
    {14, 20 ,{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165}},    // GLOBAL
    {15, 20,{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165}},// World_wide
    {16, 28,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128, 132,136,140,144,149, \
            153,157,161,165,169,173,177}}, // Test

};

PI32 get_channel_table_size(WLAN_RATE_TYPE wl_rate)
{
    if(wl_rate == WLAN_RATE_24G)
        return sizeof(channels_2G_20M)/sizeof(channels_2G_20M[0]);
    else if(wl_rate == WLAN_RATE_5G)
        return sizeof(channels_5G_20M)/sizeof(channels_5G_20M[0]);
    else
        return 0;
}

CHANNEL_ELEMENT_T *get_channel_table_element(WLAN_RATE_TYPE wl_rate, unsigned char region)
{
    if(wl_rate == WLAN_RATE_24G)
        return &channels_2G_20M[region];
    else if(wl_rate == WLAN_RATE_5G)
        return &channels_5G_20M[region];
    else
        return NULL;
    
}

PIU16 get_country_regdomain(WLAN_RATE_TYPE wl_rate,char *countrycode)
{
    COUNTRY_IE_ELEMENT *pc = NULL;
    int size = sizeof(countryIEArray)/sizeof(countryIEArray[0]);
    int i;

    for(i = 0; i < size && countrycode; i++){
        pc = &countryIEArray[i];
        if(strcmp(countrycode, pc->countryA2) == 0){
            if(wl_rate == WLAN_RATE_24G)
                return pc->G_Band_Region;
            else if(wl_rate == WLAN_RATE_5G)
                return pc->A_Band_Region;
            else
                break;
        }
    }

    // invalid regdomain
    return 0;
}

PI32 Is_DFS_support(char *ifname)
{
	int ret;
	int disable = 1;
	int len = sizeof(disable);
	
	ret = get_mib_by_ioctl(ifname, "disable_DFS", disable, &len);

	if(ret)
	{
		/* not support */
		return 0;
	}
	else
	{
		if(disable == 0)
		{
			/* support and enabled */
			return 1;
		}
		else
		{
			/* support and disabled */
			return 0;
		}
	}
}

PI32 Is_DFS_channel(int channel)
{
	if(channel >= 52 && channel <= 140)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

