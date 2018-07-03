#include "prmt_mngmtserver.h"

#ifdef _TR_111_PRMT_
int gDeviceNumber=0;

/****ManageDevEntity*******************************************************************************************/
struct CWMP_OP tManageDevEntityLeafOP = { getManageDevEntity, NULL };
struct CWMP_PRMT tManageDevEntityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"ManufacturerOUI",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
{"SerialNumber",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
{"ProductClass",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Host",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
#endif
/*ping_zhang:20081217 END*/
};
enum eManageDevEntityLeaf
{
	eMDManufacturerOUI,
	eMDSerialNumber,
	eMDProductClass,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eMDHost
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tManageDevEntityLeaf[] =
{
{ &tManageDevEntityLeafInfo[eMDManufacturerOUI] },
{ &tManageDevEntityLeafInfo[eMDSerialNumber] },
{ &tManageDevEntityLeafInfo[eMDProductClass] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tManageDevEntityLeafInfo[eMDHost] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};

/****ManageableDevice*******************************************************************************************/
struct CWMP_PRMT tManageableDeviceOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eManageableDeviceOjbect
{
	eMD0
};
struct CWMP_LINKNODE tManageableDeviceObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tManageableDeviceOjbectInfo[eMD0],tManageDevEntityLeaf,	NULL,		NULL,			0},
};
/***********************************************************************************************/
#endif

/*******ManagementServer****************************************************************************************/
struct CWMP_OP tManagementServerLeafOP = { getMngmntServer,setMngmntServer };
struct CWMP_PRMT tManagementServerLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"URL",					eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"Username",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"Password",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"PeriodicInformEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"PeriodicInformInterval",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"PeriodicInformTime",			eCWMP_tDATETIME,CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"ParameterKey",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tManagementServerLeafOP},
{"ConnectionRequestURL",		eCWMP_tSTRING,	CWMP_READ,		&tManagementServerLeafOP},
{"ConnectionRequestUsername",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"ConnectionRequestPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"UpgradesManaged",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"KickURL",				eCWMP_tSTRING,	CWMP_READ,		&tManagementServerLeafOP},
{"DownloadProgressURL",			eCWMP_tSTRING,	CWMP_READ,		&tManagementServerLeafOP},
#ifdef _TR_111_PRMT_
{"ManageableDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tManagementServerLeafOP},
/*ManageableDeviceNotificationLimit*/
#endif
};
enum eManagementServerLeaf
{
	eMSURL,
	eMSUsername,
	eMSPassword,
	eMSPeriodicInformEnable,
	eMSPeriodicInformInterval,
	eMSPeriodicInformTime,
	eMSParameterKey,
	eMSConnectionRequestURL,
	eMSConnectionRequestUsername,
	eMSConnectionRequestPassword,
	eMSUpgradesManaged,
	eMSKickURL,
	eMSDownloadProgressURL,
#ifdef _TR_111_PRMT_
	eMSManageableDeviceNumberOfEntries,
#endif
};
struct CWMP_LEAF tManagementServerLeaf[] =
{
{ &tManagementServerLeafInfo[eMSURL] },
{ &tManagementServerLeafInfo[eMSUsername] },
{ &tManagementServerLeafInfo[eMSPassword] },
{ &tManagementServerLeafInfo[eMSPeriodicInformEnable] },
{ &tManagementServerLeafInfo[eMSPeriodicInformInterval] },
{ &tManagementServerLeafInfo[eMSPeriodicInformTime] },
{ &tManagementServerLeafInfo[eMSParameterKey] },
{ &tManagementServerLeafInfo[eMSConnectionRequestURL] },
{ &tManagementServerLeafInfo[eMSConnectionRequestUsername] },
{ &tManagementServerLeafInfo[eMSConnectionRequestPassword] },
{ &tManagementServerLeafInfo[eMSUpgradesManaged] },
{ &tManagementServerLeafInfo[eMSKickURL] },
{ &tManagementServerLeafInfo[eMSDownloadProgressURL] },
#ifdef _TR_111_PRMT_
{ &tManagementServerLeafInfo[eMSManageableDeviceNumberOfEntries] },
#endif
{ NULL	}
};
#ifdef _TR_111_PRMT_
struct CWMP_OP tMS_ManageableDevice_OP = { NULL, objManageDevice };
struct CWMP_PRMT tManagementServerObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"ManageableDevice",			eCWMP_tOBJECT,	CWMP_READ,		&tMS_ManageableDevice_OP}
};
enum eManagementServerObject
{
	eMSManageableDevice
};
struct CWMP_NODE tManagementServerObject[] =
{
/*info,  						leaf,		next)*/
{&tManagementServerObjectInfo[eMSManageableDevice],	NULL,		NULL},
{NULL,							NULL,		NULL}
};
#endif

#define CONFIG_SET(key, val) if ( mib_set(key, val)==0)  return ERR_9002
#define CONFIG_GET(key, ret) if ( mib_get(key, ret)==0)  return ERR_9002
#define CHECK_PARAM_STR(str, min, max)  do { \
	int tmp; \
	if (!str) return ERR_9007; \
	tmp=strlen(str); \
	if ((tmp < min) || (tmp > max)) return ERR_9007; \
}	while (0)

#define CHECK_PARAM_NUM(input, min, max) if ( (input < min) || (input > max) ) return ERR_9007;

enum {
	EN_URL = 0,
	EN_USERNAME, 
	EN_PASSWORD,
	EN_PERIODIC_INFORM_ENABLE,
	EN_PERIODIC_INTERVAL,
	EN_PERIODIC_TIME,
	EN_PARAMETER_KEY,
	EN_CONNREQ_URL,
	EN_CONNREQ_USERNAME,
	EN_CONNREQ_PASSWORD,
	EN_UPGRADE_MANAGED,
	EN_KICKURL,
	EN_DOWNLOADURL
#ifdef _TR_111_PRMT_
	,EN_MANAGEABLEDEVICENUMBER
#endif
};

void MgmtSrvSetParamKey(const char *key) {
/*star:20091228 START add for store parameterkey*/
#if 1
	unsigned char gParameterKey[32+1];
	gParameterKey[0]='\0';
	if (key)
	{
		strncpy(gParameterKey, key, sizeof(gParameterKey) -1);
		gParameterKey[sizeof(gParameterKey) -1]=0;
	}
	mib_set(MIB_CWMP_PARAMETERKEY,gParameterKey);
#else
	gParameterKey[0]='\0';
	if (key)
	{
		strncpy(gParameterKey, key, sizeof(gParameterKey) -1);
		gParameterKey[sizeof(gParameterKey) -1]=0;
	}
#endif
/*star:20091228 END*/
}

int getMngmntServer(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	//printf("enter getMngmntServer, name=%s, type=%d\n", name, *type);
	//char	*lastname = entity->name;
	unsigned char buf[256+1]={0};
	//unsigned char ch=0;
	unsigned int ret_cr=0;
	unsigned int  in=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	switch(getIndexOf(tManagementServerLeaf, entity->info->name)) {
	case EN_URL: //URL
		CONFIG_GET(MIB_CWMP_ACS_URL, buf);
		*data = strdup( buf );
		break;
	case EN_USERNAME: //Username
//	#if DEBUG
		CONFIG_GET(MIB_CWMP_ACS_USERNAME, buf);
		*data = strdup(buf);
//	#else
//		*data = strdup("");
//	#endif
		break;
	case EN_PASSWORD: // Password
	#if DEBUG
		CONFIG_GET(MIB_CWMP_ACS_PASSWORD, buf);
		*data = strdup(buf);
	#else
		*data = strdup("");
	#endif
		break;
	case EN_PERIODIC_INFORM_ENABLE: // #PeriodicInformEnable
		CONFIG_GET(MIB_CWMP_INFORM_ENABLE, &in);
		*data = uintdup(in);
		break;	
	case EN_PERIODIC_INTERVAL: // PeriodicInformInterval
		CONFIG_GET(MIB_CWMP_INFORM_INTERVAL, &in);
		*data = uintdup(in);
		break;	
	case EN_PERIODIC_TIME: // PeriodicInformTime
		CONFIG_GET(MIB_CWMP_INFORM_TIME, &in);
		*data = timedup(in);
		break;	
	case EN_PARAMETER_KEY: // ParameterKey
		{
/*star:20091228 START add for store parameterkey*/
			unsigned char gParameterKey[32+1];
			CONFIG_GET(MIB_CWMP_PARAMETERKEY,gParameterKey);
/*star:20091228 END*/
		*data = strdup(gParameterKey);
		break;	
		}
	case EN_CONNREQ_URL: // ConnectionRequestURL
#if 1
		if (cwmp_getConReqURL(buf, 256))			
			*data = strdup(buf);
		else
			*data = strdup("");
#else
			*data = strdup("http://172.29.38.72:7547/tr069");
#endif
		break;	
	case EN_CONNREQ_USERNAME: // ConnectionRequestUsername
		CONFIG_GET(MIB_CWMP_CONREQ_USERNAME, buf);
		*data = strdup(buf);
		break;	
	case EN_CONNREQ_PASSWORD: // ConnectionRequestPassword
	#if DEBUG
		CONFIG_GET(MIB_CWMP_CONREQ_PASSWORD, buf);
		*data = strdup(buf);
	#else
		*data = strdup("");
	#endif
		break;	
	case EN_UPGRADE_MANAGED: // UpgradesManaged
		//unsigned int ret_cr;
		CONFIG_GET(MIB_CWMP_ACS_UPGRADESMANAGED, &ret_cr);
		//printf("UpgradesManaged: get mib as %d\n", ret_cr);
		*data = uintdup(ret_cr);
		break;
	case EN_KICKURL:
		CONFIG_GET(MIB_CWMP_ACS_KICKURL, buf);
		*data = strdup(buf);
		break;	
	case EN_DOWNLOADURL:
		CONFIG_GET(MIB_CWMP_ACS_DOWNLOADURL, buf);
		*data = strdup(buf);
		break;				
#ifdef _TR_111_PRMT_
	case EN_MANAGEABLEDEVICENUMBER:
	{
/*star:20100127 START add to update gDeviceNumber, used for notification list check*/
		FILE *fp;
		int count=0;
		fp=fopen( TR111_DEVICEFILE, "r" );
		
		while( fp && fgets( buf,160,fp ) )
		{
			char *p;
			
			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{
				count++;
			}
		}
		if(fp) fclose(fp);
		gDeviceNumber = count;
/*star:20100127 END*/		
		*data = uintdup(gDeviceNumber);
	}
		break;
#endif
	default:
		return ERR_9005;
				
	}

	//printf("out getMngmntServer\n");
	return 0;

}

int setMngmntServer(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	unsigned int *pNum;
	//unsigned char byte; //by cairui, if is char, mib can't set
	unsigned int byte;
	unsigned int iVal;
	
	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	//printf("name is <%s>\n", lastname);

	switch(getIndexOf(tManagementServerLeaf, entity->info->name)) {
	case EN_URL: //URL
		CHECK_PARAM_STR(buf, 0, 256+1);
#if defined(CTC_WAN_NAME)&&defined(CONFIG_BOA_WEB_E8B_CH)
		char tmpstr[256+1];
		CONFIG_GET(MIB_CWMP_ACS_URL, tmpstr);
		CONFIG_SET(MIB_CWMP_ACS_URL_OLD,tmpstr);
#endif
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
//		storeOldACS();
/*star:20100305 END*/
		CONFIG_SET(MIB_CWMP_ACS_URL, buf);
		break;
	case EN_USERNAME: //Username
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_ACS_USERNAME, buf);
		break;
	case EN_PASSWORD: // Password
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_ACS_PASSWORD, buf);
		break;
	case EN_PERIODIC_INFORM_ENABLE: // #PeriodicInformEnable
#ifdef _PRMT_X_CT_COM_DATATYPE
		pNum = (unsigned int *)(&tmpbool);		
#else
		pNum = (unsigned int *)data;
#endif
		CHECK_PARAM_NUM(*pNum, 0, 1);
		iVal = (*pNum == 0) ? 0 : 1;
		CONFIG_SET(MIB_CWMP_INFORM_ENABLE, &iVal);	
/*star:20100112 START move to port_update_userdata()*/
		//cwmpMgmtSrvInformInterval();
/*star:20100112 END*/
		break;	
	case EN_PERIODIC_INTERVAL: // PeriodicInformInterval
#ifdef _PRMT_X_CT_COM_DATATYPE
		pNum = &tmpuint;		
#else
		pNum = (unsigned int *)data;
#endif	
		if (*pNum < 1) return ERR_9007;		
		CONFIG_SET(MIB_CWMP_INFORM_INTERVAL, pNum);
/*star:20100112 START move to port_update_userdata()*/
		//cwmpMgmtSrvInformInterval();
/*star:20100112 END*/
		break;	
	case EN_PERIODIC_TIME: // PeriodicInformTime
		pNum = (unsigned int *)buf;
		//printf("time is %u\n", *pNum);
		CONFIG_SET(MIB_CWMP_INFORM_TIME, buf);
/*star:20100112 START move to port_update_userdata()*/
		//cwmpMgmtSrvInformInterval();
/*star:20100112 END*/
		break;	

	case EN_CONNREQ_USERNAME: // ConnectionRequestUsername
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_CONREQ_USERNAME, buf);
		break;	
	case EN_CONNREQ_PASSWORD: // ConnectionRequestPassword
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_CONREQ_PASSWORD, buf);
		break;	
	case EN_UPGRADE_MANAGED: // UpgradesManaged
#ifdef _PRMT_X_CT_COM_DATATYPE
		pNum = (unsigned int *)(&tmpbool);		
#else
		pNum = (unsigned int *)data;
#endif
		CHECK_PARAM_NUM(*pNum, 0, 1);
		byte = (*pNum == 0) ? 0 : 1;
		//printf("set MIB_CWMP_ACS_UPGRADESMANAGED as %d\n", byte);
		CONFIG_SET(MIB_CWMP_ACS_UPGRADESMANAGED, &byte);	
#if 0
		unsigned int ret_cr=10;
		CONFIG_GET(MIB_CWMP_ACS_UPGRADESMANAGED, &ret_cr);
		printf("before update, get MIB_CWMP_ACS_UPGRADESMANAGED as %d\n", ret_cr);
		//printf("and update mib\n");
		//mib_update(CURRENT_SETTING);
		//sleep(3);
		CONFIG_GET(MIB_CWMP_ACS_UPGRADESMANAGED, &ret_cr);
		printf("after update, get MIB_CWMP_ACS_UPGRADESMANAGED as %d\n", ret_cr);
#endif
		break;
	default:
		return ERR_9005;
				
	}

	return 0;


}





#ifdef _TR_111_PRMT_
extern unsigned int getInstNum( char *name, char *objname );
int getManageDeviceInfo( unsigned int num, char *sOUI, char *sSN, char *sClass );
int getManageDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	num=0;
	char		sOUI[7]="", sSN[65]="", sClass[65]="";
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	num = getInstNum( name, "ManageableDevice" );
	if(num==0) return ERR_9005;
	getManageDeviceInfo( num, sOUI, sSN, sClass );

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
		*data = strdup( sOUI );
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
		*data = strdup( sSN );
	}else if( strcmp( lastname, "ProductClass" )==0 )
	{
		*data = strdup( sClass );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Host" )==0 )
	{
		extern unsigned int gDHCPTotalHosts;
		extern int updateDHCP();
		char hostStrHead[] = "InternetGatewayDevice.LANDevice.1.Hosts.Host.";
		char hostStr[1000];
		int i,len=0;
		
		updateDHCP();
		
		if(gDHCPTotalHosts>0)
		{
			for(i=0; i<gDHCPTotalHosts; i++)
			{
				if( len+strlen(hostStrHead)+16 > sizeof(hostStr) )
					break;

				if(i!=0)
					len += sprintf(hostStr+len,",");
				len += sprintf(hostStr+len,"%s%d",hostStrHead,i+1);
			}
			*data = strdup(hostStr);
		}
		else
			*data = strdup( "" );
#endif
/*ping_zhang:20081217 END*/
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int objManageDevice(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	FILE *fp;
	char buf[160];

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int MaxInstNum=0,count=0;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		fp=fopen( TR111_DEVICEFILE, "r" );
		if(fp==NULL) return 0;
		
		while( fgets( buf,160,fp ) )
		{
			char *p;
			
			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{
				if( MaxInstNum < atoi(p) )
					MaxInstNum = atoi(p);
				
				if( create_Object( c, tManageableDeviceObject, sizeof(tManageableDeviceObject), 1, atoi(p) ) < 0 )
					break;
				count++;
				//c = & (*c)->sibling;
			}
		}
		fclose(fp);
		gDeviceNumber = count;
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int count=0;
	     	struct CWMP_LINKNODE *old_table;
	
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;

		fp=fopen( TR111_DEVICEFILE, "r" );
	     	while( fp && fgets( buf,160,fp ) )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			char *p;
			
			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{	
				remove_entity = remove_SiblingEntity( &old_table, atoi(p) );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{ 
					unsigned int MaxInstNum;
					MaxInstNum = atoi(p);					
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tManageableDeviceObject, sizeof(tManageableDeviceObject), &MaxInstNum );
				}
				count++;
			}
	     	}
			
	     	if(fp) fclose(fp);

	     	gDeviceNumber = count;
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     	return 0;
	     }
	}
	
	return -1;
}



int getManageDeviceInfo( unsigned int num, char *sOUI, char *sSN, char *sClass )
{
	FILE *fp;
	int ret=-1;
	
	if( num<=0 || sOUI==NULL || sSN==NULL || sClass==NULL ) return ret;
	
	sOUI[0]=sSN[0]=sClass[0]=0;
	
	fp=fopen( TR111_DEVICEFILE, "r" );
	if(fp)
	{
		char buf[160], *p, *n;
		
		while( fgets( buf,160,fp ) )
		{
			p = strtok( buf, " \n\r" );
			n = strtok( NULL, "\n\r" );
			if( p && (atoi(p)==num) && n )
			{
				char *s1=NULL, *s2=NULL, *s3=NULL;
				
				s1 = strtok( n, "?\n\r" );
				s2 = strtok( NULL, "?\n\r" );
				s3 = strtok( NULL, "?\n\r" );
				if( s1 && s2 && s3 )
				{
					strncpy( sOUI, s1, 6 );
					sOUI[6]=0;
					strncpy( sClass, s2, 64 );
					sClass[64]=0;
					strncpy( sSN, s3, 64 );
					sSN[64]=0;
					ret = 0;
				}else if( s1 && s2 )
				{
					strncpy( sOUI, s1, 6 );
					sOUI[6]=0;
					sClass[0]=0;
					strncpy( sSN, s2, 64 );
					sSN[64]=0;
					ret = 0;
				}//else error
				break;
			}
		}
		
		fclose(fp);
	}
	return ret;
}
#endif
