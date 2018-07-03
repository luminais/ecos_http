#include "prmt_ddns.h"
#if defined(_PRMT_X_CT_COM_DDNS_)

unsigned int getDDNSInstNum( char *name );
int getDDNSCount( unsigned int ifindex );
int getDDNS( unsigned int ifindex, int id, DDNS_CFG_T *c );

//UNUSED
//int updateDDNS( unsigned int ifindex, int id, DDNS_CFG_T *p );
//int delDDNS( unsigned int ifindex, int id );
//int delSpecficDDNS( unsigned int ifindex );
//void modifyDDNSIfIndex( unsigned int old_id, unsigned int new_id );

struct CWMP_NODE *getDDNS_PrmtEntity( char *name );

struct CWMP_OP tDDNSEntityLeafOP = { getDDNSEntity, setDDNSEntity };
struct CWMP_PRMT tDDNSEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"DDNSCfgEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSProvider",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSUsername",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"ServicePort",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSDomainName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSHostName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP}
};
enum eDDNSEntityLeaf
{
	eDDNSCfgEnabled,
	eDDNSProvider,
	eDDNSUsername,
	eDDNSPassword,
	eServicePort,
	eDDNSDomainName,
	eDDNSHostName
};
struct CWMP_LEAF tDDNSEntityLeaf[] =
{
{ &tDDNSEntityLeafInfo[eDDNSCfgEnabled] },
{ &tDDNSEntityLeafInfo[eDDNSProvider] },
{ &tDDNSEntityLeafInfo[eDDNSUsername] },
{ &tDDNSEntityLeafInfo[eDDNSPassword] },
{ &tDDNSEntityLeafInfo[eServicePort] },
{ &tDDNSEntityLeafInfo[eDDNSDomainName] },
{ &tDDNSEntityLeafInfo[eDDNSHostName] },
{ NULL }
};


struct CWMP_PRMT tDDNSObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eDDNSObject
{
	eDDNS0
};
struct CWMP_LINKNODE tDDNSObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tDDNSObjectInfo[eDDNS0],	tDDNSEntityLeaf,	NULL,		NULL,			0},
};


int getDDNSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int 	devnum, mapnum, ipnum, pppnum;
	unsigned int	chainid;
	int		ddns_chainid=0;
	WANIFACE_T	*pEntry, wan_entity;
	DDNS_CFG_T *pDDNS, ddns_entity;
	struct CWMP_NODE *pPrmtDDNS=NULL;
	
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	mapnum = getDDNSInstNum( name );
	if( (devnum==0) || (mapnum==0) || ((ipnum==0)&&(pppnum==0))  ) return ERR_9005;

	pEntry = &wan_entity;
	if( getATMVCByInstNum( devnum, ipnum, pppnum, pEntry, &chainid )==0) 
		return ERR_9005;

	pPrmtDDNS = getDDNS_PrmtEntity( name );
	if(pPrmtDDNS==NULL) return ERR_9005;
	
	ddns_chainid = getChainID( pPrmtDDNS->next, mapnum );
	if(ddns_chainid==-1) return ERR_9005;	
	
	pDDNS = &ddns_entity;
	if( getDDNSbyInst( pEntry->ifIndex, mapnum, pDDNS ) )
		return ERR_9002;
	

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DDNSCfgEnabled" )==0 )
	{
		if( pDDNS->Enabled==0 )
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "DDNSProvider" )==0 )
	{	
		*data = strdup( pDDNS->provider );
	}
	else if( strcmp( lastname, "DDNSUsername" )==0 )
	{	
		*data = strdup( pDDNS->username );
	}
	else if( strcmp( lastname, "DDNSPassword" )==0 )
	{	
		*data = strdup( "" ); //strdup( pDDNS->password );
	}
	else if( strcmp( lastname, "ServicePort" )==0 )
	{
		char	strPort[16];
		sprintf( strPort, "%u", pDDNS->ServicePort );
		*data=strdup( strPort );
	}
	else if( strcmp( lastname, "DDNSDomainName" )==0 )
	{	
		*data = strdup( pDDNS->domainname );
#if 0	
		char *pch;
		
		pch=strstr( pDDNS->hostname, "." );
		if( pch )
		{
			pch++;
			*data=strdup( pch );
		}else
			*data=strdup( "" );
#endif		
	}
	else if( strcmp( lastname, "DDNSHostName" )==0 )
	{
		*data = strdup( pDDNS->hostname );
#if 0	
		char *pch;
		
		pch=strstr( pDDNS->hostname, "." );
		if( pch )
		{
			*pch=0;;
			*data=strdup( pDDNS->hostname );
		}else
			*data=strdup( "" );
#endif		
	}else{
		return ERR_9005;
	}

	return 0;
}

int setDDNSEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int 	devnum, mapnum, ipnum, pppnum;
	unsigned int	chainid;
	int		ddns_chainid=0;
	char		*buf=data;
	WANIFACE_T	*pEntry,wan_entity;
	DDNS_CFG_T *pDDNS, ddns_entity;
	DDNS_CFG_T target[2];
	struct CWMP_NODE *pPrmtDDNS=NULL;
	char		tmp[256]="";

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	mapnum = getDDNSInstNum( name );
	if( (devnum==0) || (mapnum==0) || ((ipnum==0)&&(pppnum==0))  ) return ERR_9005;

	pEntry = &wan_entity;
	if( getATMVCByInstNum( devnum, ipnum, pppnum, pEntry, &chainid )==0) 
		return ERR_9005;

	pPrmtDDNS = getDDNS_PrmtEntity( name );
	if(pPrmtDDNS==NULL) return ERR_9005;
	
	ddns_chainid = getChainID( pPrmtDDNS->next, mapnum );
	if(ddns_chainid==-1) return ERR_9005;
	
	pDDNS = &ddns_entity;
	if( getDDNSbyInst( pEntry->ifIndex, mapnum, pDDNS ) )
		return ERR_9002;

	memset( &target[0], 0, sizeof( DDNS_CFG_T ) );
	memset( &target[1], 0, sizeof( DDNS_CFG_T ) );
	memcpy(&target[0], &ddns_entity, sizeof(DDNS_CFG_T));
	if( strcmp( lastname, "DDNSCfgEnabled" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		pDDNS->Enabled = (*i==0)?0:1;
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DDNSProvider" )==0 )
	{	
		if( buf==NULL || (strlen(buf)==0) ||  (strlen(buf)>=sizeof(pDDNS->provider)) )
			return ERR_9007;


		strncpy( pDDNS->provider, buf, DDNS_PARA_NAME_LEN-1 );
		pDDNS->provider[DDNS_PARA_NAME_LEN-1]=0;
	
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
		
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DDNSUsername" )==0 )
	{
		if( buf==NULL || (strlen(buf)==0) ||  (strlen(buf)>=sizeof(pDDNS->username)) )
			return ERR_9007;

		strncpy( pDDNS->username, buf, DDNS_PARA_NAME_LEN-1 );
		pDDNS->username[DDNS_PARA_NAME_LEN-1]=0;
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DDNSPassword" )==0 )
	{	
		if( buf==NULL || (strlen(buf)==0) || (strlen(buf)>=sizeof(pDDNS->password)) )
			return ERR_9007;

		strncpy( pDDNS->password, buf, DDNS_PARA_NAME_LEN-1 );
		pDDNS->password[DDNS_PARA_NAME_LEN-1]=0;
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "ServicePort" )==0 )
	{	
		int iPort=0;

		if( buf==NULL || (strlen(buf)==0) ) return ERR_9007;
		
		iPort = atoi(buf);
		
		if(iPort<=0 || iPort>=65535) return ERR_9007;
		
		pDDNS->ServicePort=(unsigned short)iPort;
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DDNSDomainName" )==0 )
	{	
		char *pch;
		if( (buf==NULL) || (strlen(buf)==0) ) return ERR_9007;
#if 0		
///////////////////////////////////////		
		pch=strstr( pDDNS->hostname, "." );
		if(pch) *pch=0;
		strcpy( tmp, pDDNS->hostname );
		if( buf[0]!='.' ) strcat( tmp, "." );
		strcat( tmp, buf );
		if( strlen(tmp) >= sizeof(pDDNS->hostname) ) return ERR_9007;
		strcpy( pDDNS->hostname, tmp );
/////////////////////////////////////
#endif		
		strncpy( pDDNS->domainname, buf, DDNS_PARA_NAME_LEN-1 );
		pDDNS->domainname[DDNS_PARA_NAME_LEN-1]=0;
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DDNSHostName" )==0 )
	{	
		char *pch;
		if( (buf==NULL) || (strlen(buf)==0) ) return ERR_9007;
#if 0		
//////////////////		
		strcat( tmp, buf );
		if( pDDNS->hostname[0]=='.' )
			strcat( tmp, pDDNS->hostname );
		else{
			pch=strstr( pDDNS->hostname, "." );
			if(pch) strcat( tmp, pch );
		}
		if( strlen(tmp) >= sizeof(pDDNS->hostname) ) return ERR_9007;
		strcpy( pDDNS->hostname, tmp );
//////////////////	
#endif		
		strncpy( pDDNS->hostname, buf, DDNS_PARA_NAME_LEN-1 );
		pDDNS->hostname[DDNS_PARA_NAME_LEN-1]=0;
		memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

		mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
		//updateDDNS( pEntry->ifIndex, mapnum, pDDNS );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}else{
		return ERR_9005;
	}

	return 0;
}

int objDDNS(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int devnum,ipnum,pppnum;
	unsigned int chainid;
	WANIFACE_T *pEntry,wan_entity;
	DDNS_CFG_T target[2];
	DDNS_CFG_T *p, ddns_entity;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);		
	if( (name==NULL) || (entity==NULL) ) return -1;

	CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	if( (devnum==0) || ((ipnum==0)&&(pppnum==0))  ) return ERR_9005;

	pEntry = &wan_entity;
	if( getATMVCByInstNum( devnum, ipnum, pppnum, pEntry, &chainid )==0) 
		return ERR_9005;		
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		
		num = getDDNSCount( pEntry->ifIndex );
		CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tINITOBJ:num=%d\n", __FUNCTION__, __LINE__, num) );
		for( i=0; i<num;i++ )
		{
			p = &ddns_entity;
			memset( &target[0], 0, sizeof( DDNS_CFG_T ) );
			memset( &target[1], 0, sizeof( DDNS_CFG_T ) );
			if( getDDNS( pEntry->ifIndex, i, p ) )
				continue;
			memcpy(&target[0], &ddns_entity, sizeof(DDNS_CFG_T));
			
			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

				mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
				//updateDDNS( pEntry->ifIndex, i, p );
			}else
				MaxInstNum = p->InstanceNum;
			if( create_Object( c, tDDNSObject, sizeof(tDDNSObject), 1, MaxInstNum ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDDNSObject, sizeof(tDDNSObject), data );
		if( ret >= 0 )
		{
			DDNS_CFG_T entry;
		     	
			CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tADDOBJ\n", __FUNCTION__, __LINE__) );
			memset( &entry, 0, sizeof( DDNS_CFG_T ) );
			{ //default values for this new entry
				char ifname[16]="";
				ifGetName( pEntry->ifIndex, ifname, 16 );
				strcpy( entry.ifaceName, ifname );
#ifdef CONFIG_BOA_WEB_E8B_CH
				char wanname[30];
				getWanName(pEntry,wanname);
				//strcpy(entry.ifaceName,wanname);
				strcpy(entry.WANName,wanname);
				CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tADDOBJ,entry.ifaceName=%s\n", __FUNCTION__, __LINE__,entry.ifaceName) );
#endif
				//entry.ifIndex=pEntry->ifIndex;	
				entry.InstanceNum= *(int*)data;
				sprintf(entry.username, "%s", "rtkdyndns-tr069");
				sprintf(entry.password, "%s", "rtk1234-tr069");
				sprintf(entry.provider, "%s", "Dyndns-tr069");
				sprintf(entry.hostname, "%s", "tr069ddns.dyndns.org.tr069");
				sprintf(entry.domainname, "%s", "tr069ddns.dyndns.org.tr069");

			}
			mib_set(MIB_CT_DDNSCFG_ADD,  (void *)&entry);
		}
#ifndef _CWMP_APPLY_
		if(ret==0) ret=1;
#endif
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret, id;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tDELOBJ\n", __FUNCTION__, __LINE__) );
     		id = getChainID( entity->next, *(int*)data  );
     		if(id==-1) return ERR_9005;

		 if(getDDNSbyInst( pEntry->ifIndex, *(int*)data, &ddns_entity ))
			return -1;

		mib_set(MIB_CT_DDNSCFG_DEL, (void *)&ddns_entity);
     		//delDDNS( pEntry->ifIndex, id );
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
#ifndef _CWMP_APPLY_
		if(ret==0) ret=1;
#endif
		return ret;
	     	break;
	     }

	case eCWMP_tUPDATEOBJ:
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	CWMPDBG( 2, ( stderr, "<%s:%d>action=eCWMP_tUPDATEOBJ(name=%s)\n", __FUNCTION__, __LINE__, name ) );
	     	num = getDDNSCount( pEntry->ifIndex );
		CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tUPDATEOBJ:num=%d\n", __FUNCTION__, __LINE__, num) );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			
			
			p = &ddns_entity;
			memset( &target[0], 0, sizeof( DDNS_CFG_T ) );
			memset( &target[1], 0, sizeof( DDNS_CFG_T ) );
			
			if( getDDNS( pEntry->ifIndex, i, p )<0 )
				continue;
			memcpy(&target[0], &ddns_entity, sizeof(DDNS_CFG_T));
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDDNSObject, sizeof(tDDNSObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					memcpy(&target[1], &ddns_entity, sizeof(DDNS_CFG_T));

					mib_set(MIB_CT_DDNSCFG_MOD, (void *)&target);
					//updateDDNS( pEntry->ifIndex, i, p );
				}
			}	
	     	}
	     	
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	
	     	return 0;
	     }
	}
	return -1;
}

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
unsigned int getDDNSInstNum( char *name )
{
	return getInstNum( name, "X_CT-COM_DDNSConfiguration" );
}


int getDDNSCount( unsigned int ifindex )
{
	int count = 0;	
	DDNS_CFG_T *p, ddns_entity;
	unsigned int total,i;
	char ifname[16];
	
	if( ifGetName( ifindex, ifname, 16 ) )
	{	
		mib_get(MIB_CT_DDNSCFG_TBL_NUM, (void *)&total);
		for( i=1;i<=total;i++ )
		{
			p = &ddns_entity;
			*((char *)p) = (char)i;
			if(!mib_get(MIB_CT_DDNSCFG_TBL, (void *)p))
				continue;
			if( strcmp(p->ifaceName, ifname)==0 ) count++;
			
		}
	}else{
		diag_printf("%s:GetifName fail\n",__FUNCTION__);
	}

	return count;
}
int getDDNSbyInst( unsigned int ifindex, int instnum, DDNS_CFG_T *c )
{
	unsigned int total,i;
	char ifname[16];
	
	if( (c==NULL) ) return -1;

	if( ifGetName( ifindex, ifname, 16 ) )
	{	
		
		mib_get(MIB_CT_DDNSCFG_TBL_NUM, (void *)&total);
		for( i=1;i<=total;i++ )
		{
			
			*((char *)c) = (char)i;
			if(!mib_get(MIB_CT_DDNSCFG_TBL, (void *)c))
				continue;
			if(!strcmp(ifname,c->ifaceName) && c->InstanceNum==instnum){
			return 0;
			}
			
		}
	}
	
	return -1;
}


int getDDNS( unsigned int ifindex, int id, DDNS_CFG_T *c )
{
	unsigned int total,i;
	char ifname[16];
	
	if( (id < 0) || (c==NULL) ) return -1;

	if( ifGetName( ifindex, ifname, 16 ) )
	{	
		
		mib_get(MIB_CT_DDNSCFG_TBL_NUM, (void *)&total);
		for( i=1;i<=total;i++ )
		{
			
			*((char *)c) = (char)i;
			if(!mib_get(MIB_CT_DDNSCFG_TBL, (void *)c))
				continue;
			//if(c->ifIndex==ifindex)
			if( strcmp(c->ifaceName, ifname)==0 )
			{
				id--;
				if(id==-1)
				{
		     			return 0;
		     		}
			}
		}
	}
	
	return -1;
}
#if 0 //UNUSED
int updateDDNS( unsigned int ifindex, int instnum, DDNS_CFG_T *p )
{
	int ret = -1;
	unsigned int total,i;
	char ifname[16];
	DDNS_CFG_T *c, ddns_entity;
	DDNS_CFG_T target[2];
	if( instnum == 0) return ret;
	if( id < 0) return ret;
	if( ifGetName( ifindex, ifname, 16 ) )
	{
	
		mib_get(MIB_CWMP_CT_DDNSCFG_TBL_NUM, (void *)&total);
		
		for( i=1;i<=total;i++ )
		{
			
			c = &ddns_entity;
			memset( &target[0], 0, sizeof( DDNS_CFG_T ) );
			memset( &target[1], 0, sizeof( DDNS_CFG_T ) );
			*((char *)c) = (char)i;
			if(!mib_get(MIB_CWMP_CT_DDNSCFG_TBL, (void *)c))
				continue;
	
			//if(c->ifIndex==ifindex)
			memcpy(&target[0], &ddns_entity, sizeof(DDNS_CFG_T));
			
			if( strcmp(c->ifaceName, ifname)==0 && c->InstanceNum==instnum)
			{
				memcpy(&target[1], p, sizeof(DDNS_CFG_T)); 
				mib_set(MIB_CWMP_CT_DDNSCFG_MOD, (void *)&target);
				ret=0;
				break;
			}
		}
	}
	return ret;	
}

int delDDNS( unsigned int ifindex, int id )
{
	int ret = -1;
	unsigned int total,i;
	char ifname[16];
	DDNS_CFG_T *c, ddns_entity;
	if( id < 0) return ret;

	if( ifGetName( ifindex, ifname, 16 ) )
	{
		
		mib_get(MIB_CWMP_CT_DDNSCFG_TBL_NUM, (void *)&total);
		for( i=1;i<=total;i++ )
		{
			
			c = &ddns_entity;
			*((char *)c) = (char)i;
			if(!mib_get(MIB_CWMP_CT_DDNSCFG_TBL, (void *)c))
				continue;
				
			//if(c->ifIndex==ifindex)
			if( strcmp(c->ifaceName, ifname)==0 )
			{
				id--;
				if(id==-1)
				{
					mib_set(MIB_CWMP_CT_DDNSCFG_DEL, (void *)&ddns_entity);
					ret=0;
					break;
				}
			}
		}
	}
	
	return ret;	
}


int delSpecficDDNS( unsigned int ifindex )
{
	int total,i;
	char ifname[16];
	DDNS_CFG_T *c, ddns_entity;
	if( ifGetName( ifindex, ifname, 16 ) )
	{
		mib_get(MIB_CWMP_CT_DDNSCFG_TBL_NUM, (void *)&total);
		for( i=1;i<=total;i++ )
		//for( i=total-1;i>=0;i-- )
		{
			
			c = &ddns_entity;
			*((char *)c) = (char)i;
			if(!mib_get(MIB_CWMP_CT_DDNSCFG_TBL, (void *)c))
				continue;
			
	
			//if(c->ifIndex==ifindex)
			if( strcmp(c->ifaceName, ifname)==0 )
				mib_set(MIB_CWMP_CT_DDNSCFG_DEL, (void *)&ddns_entity);
		}
	}
	
	return 0;	
}


void modifyDDNSIfIndex( unsigned int old_id, unsigned int new_id )
{
	unsigned int total,i;
	char new_ifname[16];
	char old_ifname[16];
	DDNS_CFG_T *c, ddns_entity;
	DDNS_CFG_T target[2];
	if( ifGetName( old_id, old_ifname, 16 ) && ifGetName( new_id, new_ifname, 16 ) )
	{
		
		mib_get(MIB_CWMP_CT_DDNSCFG_TBL_NUM, (void *)&total);
		for( i=1;i<=total;i++ )
		{
			
			c = &ddns_entity;
			*((char *)c) = (char)i;
			memset( &target[0], 0, sizeof( DDNS_CFG_T ) );
			memset( &target[1], 0, sizeof( DDNS_CFG_T ) );
			if(!mib_get(MIB_CWMP_CT_DDNSCFG_TBL, (void *)c))
				continue;
			memcpy(&target[0], &ddns_entity, sizeof(DDNS_CFG_T));
			//if(c->ifIndex==old_id)
			if( strcmp(c->ifaceName, old_ifname)==0 )
			{
				//c->ifIndex = new_id;
				strcpy( c->ifaceName, new_ifname );
				memcpy(&target[1], c, sizeof(DDNS_CFG_T)); 
				mib_set(MIB_CWMP_CT_DDNSCFG_MOD, (void *)&target);
			}
		}
	}

	return;
}
#endif

struct CWMP_NODE *getDDNS_PrmtEntity( char *name )
{
	struct CWMP_NODE *p=NULL;
	char *dupname;
	char strfmt[]=".X_CT-COM_DDNSConfiguration.";
	char *tok;

	dupname = strdup( name );
	if( dupname )
	{
		tok = strstr( dupname, strfmt );
		if(tok)
		{
			dupname[ tok+strlen(strfmt)-dupname ]=0;
			get_ParameterEntity(dupname, (struct CWMP_LEAF **)&p);
		}
		free( dupname );
	}
	
	return p;
}

#endif /*_PRMT_X_CT_COM_DDNS_*/
