#include "tr181_lanConfigSecurity.h"


/*******************************************************************************
Device.LANConfigSecurity.
*******************************************************************************/
struct CWMP_OP tLANCfgSecurityLeafOP = { getLANCfgSecurityInfo, setLANCfgSecurityInfo };
struct CWMP_PRMT tLANCfgSecurityLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"ConfigPassword",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tLANCfgSecurityLeafOP},
};

enum eLANCfgSecurityLeaf
{
	eLANCfgSecurity
};

struct CWMP_LEAF tLANCfgSecurityLeaf[] =
{
	{ &tLANCfgSecurityLeafInfo[eLANCfgSecurity]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getLANCfgSecurityInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned char buf[64+1]={0};
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ConfigPassword" )==0 )
	{
#if 1
		*data = strdup( "" ); 
#else
		CONFIG_GET(MIB_CWMP_LAN_CONFIGPASSWD, buf);
		*data = strdup(buf);
#endif
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setLANCfgSecurityInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf = data;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	if( strcmp( lastname, "ConfigPassword" )==0 )
	{
		CHECK_PARAM_STR(buf, 0, 64+1);
		CONFIG_SET(MIB_CWMP_LAN_CONFIGPASSWD, buf);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

