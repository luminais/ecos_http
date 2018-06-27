#include "tr181_gatewayInfo.h"

#define TR069_ANNEX_F_FILE	"/var/udhcpc/tr069_annex_f.dat"

/*******************************************************************************
DEVICE.GatewayInfo Parameters
*******************************************************************************/
struct CWMP_OP tGatewayInfoLeafOP = { getGatewayInfo, NULL };
struct CWMP_PRMT tGatewayInfoLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"ManufacturerOUI",					eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,	&tGatewayInfoLeafOP},
	{"ProductClass",					eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,	&tGatewayInfoLeafOP},
	{"SerialNumber",					eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,	&tGatewayInfoLeafOP},
};

enum eGatewayInfoLeaf
{
	eManufacturerOUI,		
	eProductClass,
	eSerialNumber
};

struct CWMP_LEAF tGatewayInfoLeaf[] =
{
	{ &tGatewayInfoLeafInfo[eManufacturerOUI]  },
	{ &tGatewayInfoLeafInfo[eProductClass]  },
	{ &tGatewayInfoLeafInfo[eSerialNumber]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int _getGatewayInfo(char *sOUI, char *sSN, char *sClass)
{
	FILE *fp;
	int ret = -1;

	if (sOUI == NULL || sSN == NULL || sClass == NULL)
		return ret;

	sOUI[0] = sSN[0] = sClass[0] = 0;

	fp = fopen(TR069_ANNEX_F_FILE, "r");
	
	if (fp)
	{
		char buf[160], *p, *n;

		if (fgets(buf, 160, fp))
		{
			char *s1 = NULL, *s2 = NULL, *s3 = NULL;

			s1 = strtok(buf, "?\n\r");
			s2 = strtok(NULL, "?\n\r");
			s3 = strtok(NULL, "?\n\r");
			
			if (s1 && s2 && s3)
			{
				strncpy(sOUI, s1, 6);
				sOUI[6] = 0;
				
				strncpy(sClass, s2, 64);
				sClass[64] = 0;
				
				strncpy(sSN, s3, 64);
				sSN[64] = 0;
				
				ret = 0;
			}
			else if (s1 && s2)
			{
				strncpy(sOUI, s1, 6);
				sOUI[6] = 0;
				
				sClass[0] = 0;
				
				strncpy(sSN, s2, 64);
				sSN[64] = 0;
				
				ret = 0;
			} // else error
		}

		fclose(fp);
	}
	
	return ret;
}

int getGatewayInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char sOUI[7] = "";
	char sSN[65] = "";
	char sClass[65] = "";

	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	_getGatewayInfo(sOUI, sSN, sClass);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
		*data = strdup(sOUI);
	}else if( strcmp( lastname, "ProductClass" )==0 )
	{
		*data = strdup(sClass);
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
		*data = strdup(sSN);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
