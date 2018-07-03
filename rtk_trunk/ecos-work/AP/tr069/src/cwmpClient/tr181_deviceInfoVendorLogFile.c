#include "tr181_deviceInfoVendorLogFile.h"


/*******************************************************************************
DEVICE.DeviceInfo.VendorLogFile.{i}. Parameters
*******************************************************************************/
struct CWMP_OP tVendorLogFileLeafOP = { getVendorLogFile, NULL };
struct CWMP_PRMT tVendorLogFileLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"Name",			eCWMP_tSTRING,	CWMP_READ,	&tVendorLogFileLeafOP},
	{"MaximumSize",		eCWMP_tUINT,	CWMP_READ,	&tVendorLogFileLeafOP},
	{"Persistent",		eCWMP_tBOOLEAN,	CWMP_READ,	&tVendorLogFileLeafOP},
};

enum eVendorLogFileoLeaf
{
	eVendorLogFileName,			
	eVendorLogFileMaximumSize,			
	eVendorLogFilePersistent
};

struct CWMP_LEAF tVendorLogFileLeaf[] =
{
	{ &tVendorLogFileLeafInfo[eVendorLogFileName]  },
	{ &tVendorLogFileLeafInfo[eVendorLogFileMaximumSize]  },
	{ &tVendorLogFileLeafInfo[eVendorLogFilePersistent]  },
	{ NULL	}
};

/*******************************************************************************
DEVICE.DeviceInfo.VendorLogFile.{i}.
*******************************************************************************/
struct CWMP_PRMT tLinkVendorLogFileOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkVendorLogFileOjbect
{
	eLinkVendorLogFile1
};
struct CWMP_NODE tLinkVendorLogFileObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkVendorLogFileOjbectInfo[eLinkVendorLogFile1],	tVendorLogFileLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
DEVICE.DeviceInfo.VendorLogFile.
*******************************************************************************/
struct CWMP_PRMT tVendorLogFileObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"VendorLogFile",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eVendorLogFileObject
{
	eVendorLogFile,
};

struct CWMP_NODE tVendorLogFileObject[] =
{
	/*info,  					leaf,			node)*/
	{&tVendorLogFileObjectInfo[eVendorLogFile],	NULL,	tLinkVendorLogFileObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int getVendorLogFile(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};
	int enabled;
	int syslogdEn;

	mib_get(MIB_SCRLOG_ENABLED, (void *)&enabled);

	if ( !(enabled & 1))
		syslogdEn = 0;
	else
		syslogdEn = 1;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Name" )==0 )
	{
		if (syslogdEn)
			*data = strdup( "/var/log/messages" );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "MaximumSize" )==0 )
	{
		if (syslogdEn)
		{
#ifdef RINGLOG
			*data = uintdup( (1024*MAX_LOG_SIZE*(LOG_SPLIT+1)) ); // syslogd -L -s 8 -b 7
#else
			*data = uintdup( 1024*200*2 ); // syslogd default
#endif
		}
		else
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "Persistent" )==0 )
	{
		*data = booldup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}


