#include "tr181_mgableDev.h"

/*******************************************************************************
Global Variable
*******************************************************************************/
int gMgableDevNum = 0;


/*******************************************************************************
Forward Reference
*******************************************************************************/
int getMgableDevInfo(unsigned int num, char *sOUI, char *sSN, char *sClass);

/*******************************************************************************
DEVICE.ManagementServer.ManageableDevice Entity
*******************************************************************************/
struct CWMP_OP tMaableDevEntityLeafOP = { getMgableDevEntity, setMgableDevEntity };
struct CWMP_PRMT tMaableDevEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
//	{"Alias",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tMaableDevEntityLeafOP}, // not implement yet
	{"ManufacturerOUI",	eCWMP_tSTRING,	CWMP_READ,	&tMaableDevEntityLeafOP},
	{"SerialNumber",	eCWMP_tSTRING,	CWMP_READ,	&tMaableDevEntityLeafOP},
	{"ProductClass",	eCWMP_tSTRING,	CWMP_READ,	&tMaableDevEntityLeafOP},
//	{"Host",			eCWMP_tSTRING,	CWMP_READ,	&tMaableDevEntityLeafOP}, // list of string
};
enum eMgableDevEntityLeaf
{
//	eMgableDevAlias,
	eMgableDevManufacturerOUI,
	eMgableDevSerialNumber,
	eMgableDevProductClass,
//	eMgableDevHost
};
struct CWMP_LEAF tMgableDevEntityLeaf[] =
{
//	{ &tMaableDevEntityLeafInfo[eMgableDevAlias] },
	{ &tMaableDevEntityLeafInfo[eMgableDevManufacturerOUI] },
	{ &tMaableDevEntityLeafInfo[eMgableDevSerialNumber] },
	{ &tMaableDevEntityLeafInfo[eMgableDevProductClass] },
//	{ &tMaableDevEntityLeafInfo[eMgableDevHost] },
	{ NULL }
};

/*******************************************************************************
DEVICE.ManagementServer.ManageableDevice LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkMgableDevOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkMgableDevOjbect
{
	eLinkMgableDev0
};
struct CWMP_LINKNODE tLinkMgableDevObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkMgableDevOjbectInfo[eLinkMgableDev0],	tMgableDevEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
DEVICE.ManagementServer.ManageableDevice
*******************************************************************************/
struct CWMP_OP tMgableDev_OP = { NULL, objMgableDev };
struct CWMP_PRMT tMgableDevObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"ManageableDevice",	eCWMP_tOBJECT,	CWMP_READ,	&tMgableDev_OP},
};

enum eMgableDevObject
{
	eMgableDev
};

struct CWMP_NODE tMgableDevObject[] =
{
	/*info,  					leaf,			node)*/
	{&tMgableDevObjectInfo[eMgableDev],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int objMgableDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	FILE *fp;
	char buf[160];

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			int MaxInstNum=0,count=0;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			fp=fopen( TR069_ANNEX_F_DEVICE_FILE, "r" );
			if(fp==NULL) return 0;

			while( fgets( buf,160,fp ) )
			{
				char *p;

				p = strtok( buf, " \n\r" );
				if( p && atoi(p)>0 )
				{
					if( MaxInstNum < atoi(p) )
						MaxInstNum = atoi(p);

					if( create_Object( c, tLinkMgableDevObject, sizeof(tLinkMgableDevObject), 1, atoi(p) ) < 0 )
						break;
					count++;
					//c = & (*c)->sibling;
				}
			}
			fclose(fp);
			gMgableDevNum = count;
			add_objectNum( name, MaxInstNum );
			return 0;
		}

		case eCWMP_tUPDATEOBJ:	
		{
			int count=0;
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			fp=fopen( TR069_ANNEX_F_DEVICE_FILE, "r" );
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
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tLinkMgableDevObject, sizeof(tLinkMgableDevObject), &MaxInstNum );
					}
					count++;
				}
			}

			if(fp) fclose(fp);

			gMgableDevNum = count;
			if( old_table )
			destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			return 0;
		}

		default:
			break;
	}
	
	return 0;
}

int getMgableDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	num=0;
	char		sOUI[7]="", sSN[65]="", sClass[65]="";
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	num = getInstanceNum( name, "ManageableDevice" );
	if(num==0) return ERR_9005;
	getMgableDevInfo( num, sOUI, sSN, sClass );

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
#if 0 // may be used
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
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setMgableDevEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getMgableDevInfo(unsigned int num, char *sOUI, char *sSN, char *sClass)
{
	FILE *fp;
	int ret=-1;
	
	if( num<=0 || sOUI==NULL || sSN==NULL || sClass==NULL ) return ret;
	
	sOUI[0]=sSN[0]=sClass[0]=0;
	
	fp=fopen( TR069_ANNEX_F_DEVICE_FILE, "r" );
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
