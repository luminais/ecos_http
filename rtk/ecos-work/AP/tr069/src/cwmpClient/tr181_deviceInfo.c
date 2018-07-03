#include "tr181_deviceInfo.h"


#define MANUFACTURER_STR	"REALTEK SEMICONDUCTOR CORP."	// DEF_MANUFACTURER_STR
#define MANUFACTUREROUI_STR	"00E04C"						// DEF_MANUFACTUREROUI_STR
#define PRODUCTCLASS_STR	"DEVICE"
#define HWVERSION_STR		"81xx"


extern char *fwVersion;	// defined in version.c


/*******************************************************************************
DEVICE.DeviceInfo Parameters
*******************************************************************************/
struct CWMP_OP tDevInfoLeafOP = { getDevInfo, setDevInfo };
struct CWMP_PRMT tDevInfoLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"Manufacturer",					eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP},
	{"ManufacturerOUI",					eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP},
	{"ModelName",						eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tDevInfoLeafOP},
	{"Description",						eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tDevInfoLeafOP},
	{"ProductClass",					eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP},
	{"SerialNumber",					eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP},
	{"HardwareVersion",					eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP},
	{"SoftwareVersion",					eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,	&tDevInfoLeafOP},
//	{"AdditionalHardwareVersion",		eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP}, // list of string
//	{"AdditionalSoftwareVersion",		eCWMP_tSTRING,	CWMP_READ,	&tDevInfoLeafOP}, // list of string
	{"ProvisioningCode",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE|CWMP_FORCE_ACT,	&tDevInfoLeafOP},
	{"UpTime",							eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tDevInfoLeafOP},
	{"FirstUseDate",					eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,	&tDevInfoLeafOP},
//	{"VendorConfigFileNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tDevInfoLeafOP},
};

enum eDevInfoLeaf
{
	eManufacturer,			
	eManufacturerOUI,		
	eModelName,
	eDescription,
	eProductClass,
	eSerialNumber,
	eHardwareVersion,
	eSoftwareVersion,
//	eAdditionalHardwareVersion,
//	eAdditionalSoftwareVersion,
	eProvisioningCode,
	eUpTime,
	eFirstUseDate,
//	eVendorConfigFileNumberOfEntries
};

struct CWMP_LEAF tDevInfoLeaf[] =
{
	{ &tDevInfoLeafInfo[eManufacturer]  },
	{ &tDevInfoLeafInfo[eManufacturerOUI]  },
	{ &tDevInfoLeafInfo[eModelName]  },
	{ &tDevInfoLeafInfo[eDescription]  },
	{ &tDevInfoLeafInfo[eProductClass]  },
	{ &tDevInfoLeafInfo[eSerialNumber]  },
	{ &tDevInfoLeafInfo[eHardwareVersion]  },
	{ &tDevInfoLeafInfo[eSoftwareVersion]  },
//	{ &tDevInfoLeafInfo[eAdditionalHardwareVersion]  },
//	{ &tDevInfoLeafInfo[eAdditionalSoftwareVersion]  },
	{ &tDevInfoLeafInfo[eProvisioningCode]  },
	{ &tDevInfoLeafInfo[eUpTime]  },
	{ &tDevInfoLeafInfo[eFirstUseDate]  },
//	{ &tDevInfoLeafInfo[eVendorConfigFileNumberOfEntries]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getDevInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Manufacturer" )==0 )
	{
		*data = strdup( MANUFACTURER_STR );
	}else if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
		*data = strdup( MANUFACTUREROUI_STR );
	}else if( strcmp( lastname, "ModelName" )==0 )
	{
		*data = strdup( "ModelName" );
	}else if( strcmp( lastname, "Description" )==0 )
	{
		*data = strdup( "Description" );
	}else if( strcmp( lastname, "ProductClass" )==0 )
	{
		*data = strdup( PRODUCTCLASS_STR );
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
		char tmpBuff[512];
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( MIB_CWMP_SERIALNUMBER, (void *)tmpBuff);
#else
		mib_get(MIB_HW_NIC1_ADDR,  (void *)tmpBuff);

		sprintf(buf, "%02x%02x%02x%02x%02x%02x", (unsigned char)tmpBuff[0], (unsigned char)tmpBuff[1], 
			(unsigned char)tmpBuff[2], (unsigned char)tmpBuff[3], (unsigned char)tmpBuff[4], (unsigned char)tmpBuff[5]);
			
		
#endif
		*data = strdup( buf );
	}else if( strcmp( lastname, "HardwareVersion" )==0 )
	{
#if 1
		int hwVer;
		char tmpBuff[512];
		
		mib_get( MIB_HW_BOARD_VER, (void *)&hwVer);
		sprintf(tmpBuff, "%d", hwVer);
		*data = strdup(tmpBuff);
#else
		*data = strdup( HWVERSION_STR );
#endif
	}else if( strcmp( lastname, "SoftwareVersion" )==0 )
	{
		*data = strdup( fwVersion );
	}
	else if( strcmp( lastname, "AdditionalHardwareVersion" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "AdditionalSoftwareVersion" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		mib_get( MIB_CWMP_PROVISIONINGCODE, (void *)buf);
		*data = strdup( buf );	
#if 0 // rewrite
	}else if( strcmp( lastname, "UpTime" )==0 )
	{
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
#endif
	}else if( strcmp( lastname, "FirstUseDate" )==0 )
	{
		*data = timedup( 0 );
	}else if( strcmp( lastname, "VendorConfigFileNumberOfEntries" )==0 )
	{
		*data = uintdup( 1 );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setDevInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	
	if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		if( buf ) len = strlen( buf );
		if( len ==0 )
			mib_set( MIB_CWMP_PROVISIONINGCODE, (void *)"");
		else if( len < 64 )
			mib_set( MIB_CWMP_PROVISIONINGCODE, (void *)buf);
		else
			return ERR_9007;
			
		return 0;
	}else
		return ERR_9005; 
	
	return 0;
}
