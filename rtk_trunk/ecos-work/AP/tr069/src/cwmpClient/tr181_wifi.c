#include "tr181_wifi.h"


#define RADIO_NUM			2	/* one instance of INTERFACE */
#define SSID_NUM			10	/* one instance of INTERFACE */
#define ACCESS_POINT_NUM	10	/* one instance of INTERFACE */
#define END_POINT_NUM		1	/* one instance of INTERFACE */


/*******************************************************************************
DEVICE.Wifi
*******************************************************************************/
struct CWMP_OP tWifiLeafOP = { getWifiInfo, NULL };
struct CWMP_PRMT tWifiLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"RadioNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tWifiLeafOP},
	{"SSIDNumberOfEntries",			eCWMP_tUINT,	CWMP_READ,		&tWifiLeafOP},
	{"AccessPointNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tWifiLeafOP},
//	{"EndPointNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tWifiLeafOP},
};

enum eWifiLeaf
{
	eWifiRadioNumberOfEntries,
	eWifiSSIDNumberOfEntries,
	eWifiAccessPointNumberOfEntries
//	eWifiEndPointNumberOfEntries
};

struct CWMP_LEAF tWifiLeaf[] =
{
	{ &tWifiLeafInfo[eWifiRadioNumberOfEntries]  },
	{ &tWifiLeafInfo[eWifiSSIDNumberOfEntries]  },
	{ &tWifiLeafInfo[eWifiAccessPointNumberOfEntries]  },
//	{ &tWifiLeafInfo[eWifiEndPointNumberOfEntries]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getWifiInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "RadioNumberOfEntries" )==0 )
	{
		*data = uintdup( RADIO_NUM ); 
	}else if( strcmp( lastname, "SSIDNumberOfEntries" )==0 )
	{
		*data = uintdup( SSID_NUM ); 
	}else if( strcmp( lastname, "AccessPointNumberOfEntries" )==0 )
	{
		*data = uintdup( ACCESS_POINT_NUM ); 
	}else if( strcmp( lastname, "EndPointNumberOfEntries" )==0 )
	{
		*data = uintdup( END_POINT_NUM ); 
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}