#include "prmt_layer3fw.h"
#include <stdio.h>
#define DUMMY_IFINDEX				0xffff
#include "prmt_wancondevice.h"

#if 0
typedef enum { MEDIA_ATM, MEDIA_ETH } MEDIA_TYPE_T;
#define DUMMY_PPP_INDEX                         0xff
#define ALIASNAME_VC   "vc" 
#define ALIASNAME_PPP  "ppp"
#define ALIASNAME_ETH  "eth"
#define PPP_INDEX(x)                            ((x  >> 8) & 0x0ff)
#define MEDIA_INDEX(x)                          ((x >> 16) & 0x0ff)
#define VC_INDEX(x)                             (x & 0x0ff)
#define ETH_INDEX(x)                            (x & 0x0ff)
#define PPP_INDEX(x)                            ((x  >> 8) & 0x0ff)
char *ifGetName(int ifindex, char *buffer, unsigned int len)
{
	MEDIA_TYPE_T mType;

	if ( ifindex == DUMMY_IFINDEX )
		return 0;
	if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
	{
		mType = MEDIA_INDEX(ifindex);
		if (mType == MEDIA_ATM)
			snprintf( buffer, len,  "%s%u",ALIASNAME_VC, VC_INDEX(ifindex) );
		else if (mType == MEDIA_ETH)
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			snprintf( buffer, len, "%s%d",ALIASNAME_MWNAS, ETH_INDEX(ifindex));
#else
			snprintf( buffer, len,  "%s%u",ALIASNAME_ETH, ETH_INDEX(ifindex) );
#endif
		else
			return 0;
	}else{
		snprintf( buffer, len,  "%s%u",ALIASNAME_PPP, PPP_INDEX(ifindex) );
	}
	return buffer;
}
#endif

#ifdef _PRMT_WT107_
#define MAX_DYNAMIC_ROUTE_INSTNUM	10000
int getDynamicForwardingTotalNum(void);
int getDynamicForwardingEntryByInstNum( unsigned int instnum, STATICROUTE_T *pRoute );
#endif //_PRMT_WT107_

extern unsigned char transfer2IfIndxfromIfName( char *ifname );
extern unsigned int getInstNum( char *name, char *objname );
#ifdef ROUTE_SUPPORT
int queryRouteStatus( STATICROUTE_T *p );
#endif
unsigned int getForwardingInstNum( char *name );
unsigned int findMaxForwardingInstNum(void);
#ifdef ROUTE_SUPPORT
int getForwardingEntryByInstNum( unsigned int instnum, STATICROUTE_T *p, unsigned int *id );
#endif

/*******ForwardingEntity****************************************************************************************/
struct CWMP_OP tForwardingEntityLeafOP = { getFwEntity, setFwEntity };
struct CWMP_PRMT tForwardingEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tForwardingEntityLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"StaticRoute",			eCWMP_tBOOLEAN,	CWMP_READ,		&tForwardingEntityLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"Type",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"DestIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"DestSubnetMask",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"SourceIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"SourceSubnetMask",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"GatewayIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"ForwardingMetric",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
/*MTU*/
};
enum eForwardingEntityLeaf
{
	eFWEnable,
	eFWStatus,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eFWStaticRoute,
#endif
/*ping_zhang:20081217 END*/
	eFWType,
	eFWDestIPAddress,
	eFWDestSubnetMask,
	eFWSourceIPAddress,
	eFWSourceSubnetMask,
	eFWGatewayIPAddress,
	eFWInterface,
	eFWForwardingMetric
};
struct CWMP_LEAF tForwardingEntityLeaf[] =
{
{ &tForwardingEntityLeafInfo[eFWEnable] },
{ &tForwardingEntityLeafInfo[eFWStatus] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tForwardingEntityLeafInfo[eFWStaticRoute] },
#endif
/*ping_zhang:20081217 END*/
{ &tForwardingEntityLeafInfo[eFWType] },
{ &tForwardingEntityLeafInfo[eFWDestIPAddress] },
{ &tForwardingEntityLeafInfo[eFWDestSubnetMask] },
{ &tForwardingEntityLeafInfo[eFWSourceIPAddress] },
{ &tForwardingEntityLeafInfo[eFWSourceSubnetMask] },
{ &tForwardingEntityLeafInfo[eFWGatewayIPAddress] },
{ &tForwardingEntityLeafInfo[eFWInterface] },
{ &tForwardingEntityLeafInfo[eFWForwardingMetric] },
{ NULL }
};

/*******Forwarding****************************************************************************************/
struct CWMP_PRMT tForwardingOjbectInfo[] =
{
/*(name,	type,		flag,					op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eForwardingOjbect
{
	eFW0
};
struct CWMP_LINKNODE tForwardingObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tForwardingOjbectInfo[eFW0],	tForwardingEntityLeaf,	NULL,		NULL,			0},
};

/*******Layer3Forwarding****************************************************************************************/
struct CWMP_OP tLayer3ForwardingLeafOP = { getLayer3Fw,	setLayer3Fw };
struct CWMP_PRMT tLayer3ForwardingLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"DefaultConnectionService",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLayer3ForwardingLeafOP},
{"ForwardNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLayer3ForwardingLeafOP},
};
enum eLayer3ForwardingLeaf
{
	eFWDefaultConnectionService,
	eFWForwardNumberOfEntries
};
struct CWMP_LEAF tLayer3ForwardingLeaf[] =
{
{ &tLayer3ForwardingLeafInfo[eFWDefaultConnectionService] },
{ &tLayer3ForwardingLeafInfo[eFWForwardNumberOfEntries] },
{ NULL }
};
struct CWMP_OP tFW_Forwarding_OP = { NULL, objForwading };
struct CWMP_PRMT tLayer3ForwardingObjectInfo[] =
{
/*(name,			type,		flag,			)*/
{"Forwarding",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tFW_Forwarding_OP},
};
enum eLayer3ForwardingObject
{
	eFWForwarding
};
struct CWMP_NODE tLayer3ForwardingObject[] =
{
/*info,  					leaf,			node)*/
{&tLayer3ForwardingObjectInfo[eFWForwarding],	NULL,			NULL},
{NULL,						NULL,			NULL}
};
/***********************************************************************************************/
int getFwEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	fw_buf[256], buf[64], *tok;
	unsigned int object_num=0;
	struct CWMP_NODE *fw_entity;
	int	chain_id=0;
#ifdef ROUTE_SUPPORT
	STATICROUTE_T *fw=NULL, route_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	object_num=getForwardingInstNum( name );
	if(object_num==0) return ERR_9005;
	fw = &route_entity;
#ifdef _PRMT_WT107_
	if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
	{
		//dynamic route
		getDynamicForwardingEntryByInstNum( object_num, fw );
	}else
#endif //_PRMT_WT107_
	{
		//static route
		if(getForwardingEntryByInstNum( object_num, fw, &chain_id )<0)
			return ERR_9005;
	}
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		//fprintf( stderr, "<%s:%d>enable:%d\n",__FUNCTION__,__LINE__,fw->Enable  );fflush(NULL);
		*data = booldup( fw->Enable!=0 );
	}else if( strcmp( lastname, "Status" )==0 )
	{
		int RouteRet;
#ifdef _PRMT_WT107_
		if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
			RouteRet = (fw->Enable)?1:0;
		else
#endif
			RouteRet = queryRouteStatus(fw);
		switch(RouteRet)
		{
		case 1: //found
			*data = strdup( "Enabled" );
			break;
		case 0: //not found
			if( fw->Enable )
				*data = strdup( "Error" );
			else
				*data = strdup( "Disabled" );
			break;
		case -1://error
		default:
			*data = strdup( "Error" );
			break;
		}
	}
#ifdef _PRMT_WT107_
	else if( strcmp( lastname, "StaticRoute" )==0 )
	{
		if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
			*data = booldup(0);
		else
			*data = booldup(1);
	}
#endif //_PRMT_WT107_
	else if( strcmp( lastname, "Type" )==0 )
	{
		if( fw->Type==0 )
			*data = strdup( "Network" );
		else if( fw->Type==1 )
			*data = strdup( "Host" );
		else if( fw->Type==2 )
			*data = strdup( "Default" );
		else 
			return ERR_9002;
	}
	else if( strcmp( lastname, "DestIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->dstAddr)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "DestSubnetMask" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->netmask)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "SourceIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->SourceIP)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "SourceSubnetMask" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->SourceMask)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "GatewayIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->gateway)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char tmp[256];
		//change from eth0 to internetgatewaydevice.xxx.xxxx.xxxx.xxxx
		//strcpy( tmp, fw->Interface );
		if( transfer2PathName( fw->ifIndex, tmp )<0 ) 
			*data = strdup( "" );//return ERR_9007;
		else
			*data = strdup( tmp );
	}
	else if( strcmp( lastname, "ForwardingMetric" )==0 )
	{
		*data = intdup( fw->metric );
	}
	else{
		return ERR_9005;
	}
	
#endif
	return 0;
}

int setFwEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	//printf("enter setFwEntity, name=%s\n", name);
	char	*lastname = entity->info->name;
	char	fw_buf[256], *tok;
	char	*buf=data;
	int	object_num=0;
	struct CWMP_NODE *fw_entity;
	int	chain_id=0;
#ifdef ROUTE_SUPPORT
	STATICROUTE_T *fw=NULL,route_entity;
	STATICROUTE_T target[2];
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
#ifdef _CWMP_APPLY_
	STATICROUTE_T route_old;
#endif
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	aaaa;
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	object_num=getForwardingInstNum( name );
	//printf("object_num=%d\n", object_num);
	if(object_num==0) return ERR_9005;
#ifdef _PRMT_WT107_
	aaaa;
	if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM ) return ERR_9001; //reject to modify a dynamic route
#endif //_PRMT_WT107_
	fw = &route_entity;
	memset( &target[0], 0, sizeof( STATICROUTE_T ) );
	memset( &target[1], 0, sizeof( STATICROUTE_T ) );

	if(getForwardingEntryByInstNum( object_num, fw, &chain_id )<0){
		diag_printf("will return ERR_9005\n");
		return ERR_9005;
	}
	memcpy(&target[0], &route_entity, sizeof(STATICROUTE_T));

#ifdef _CWMP_APPLY_
	memcpy( &route_old, fw, sizeof(STATICROUTE_T) );
#endif

	//printf("lastname=%s\n", lastname);
	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
	#else
		int *i = data;
#endif
		fw->Enable = (*i==0)? 0:1;
		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		if(  strcmp( buf, "Network" )==0 )
			fw->Type = 0;
		else if(  strcmp( buf, "Host" )==0 )
			fw->Type = 1;		
		else if(  strcmp( buf, "Default" )==0 )
			fw->Type = 2;
		else
			return ERR_9007;
		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DestIPAddress" )==0 )
	{
		//printf("DestIPAddress\n");
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy(fw->dstAddr, &in, sizeof(struct in_addr));
		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DestSubnetMask" )==0 )
	{
		//printf("DestSubnetMask\n");
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy(fw->netmask, &in, sizeof(struct in_addr));

		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "SourceIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
#if 1
		if( (strlen(buf)==0) || (strcmp(buf, pzeroip)==0) )
			return 0;
		else
			return ERR_9001;//deny
#else
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->SourceIP, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
#endif
	}else if( strcmp( lastname, "SourceSubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
#if 1
		if( (strlen(buf)==0) || (strcmp(buf, pzeroip)==0) )
			return 0;
		else
			return ERR_9001;//deny
#else
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->SourceMask, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
#endif
	}
	else if( strcmp( lastname, "GatewayIPAddress" )==0 )
	{
		//printf("GatewayIPAddress\n");
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy(fw->gateway, &in, sizeof(struct in_addr));
		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		//printf("Interface\n");
		//change from internetgatewaydevice.xxx.xx.xxx to eth0
		//strcpy( fw->Interface, "eth0" );
		unsigned int newifindex;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) 
			fw->ifIndex=DUMMY_IFINDEX;//return ERR_9007;
		else{
			newifindex = transfer2IfIndex( buf );
			if( newifindex==DUMMY_IFINDEX ) return ERR_9007;
			fw->ifIndex = newifindex;
		}
		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "ForwardingMetric" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( *i < -1 ) return ERR_9007;
		fw->metric = *i;
		memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
		mib_set(MIB_STATICROUTE_MOD, (void *)&target);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else{
		return ERR_9005;
	}
	
#endif
	return 0;
}

int getLayer3Fw(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DefaultConnectionService" )==0 )
	{
		char buf[256];
		if( getDefaultRouteIfaceName(buf)<0 )
			*data = strdup( "" );
		else
			*data = strdup( buf );
	}else if( strcmp( lastname, "ForwardNumberOfEntries" )==0 )
	{
		unsigned int total=0;
#ifdef ROUTE_SUPPORT
		mib_get(MIB_STATICROUTE_TBL_NUM, (void *)&total);
#ifdef _PRMT_WT107_
		total += getDynamicForwardingTotalNum();
#endif //_PRMT_WT107_
#endif
		*data = uintdup( total );
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int setLayer3Fw(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "DefaultConnectionService" )==0 )
	{
		char *buf=data;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( setDefaultRouteIfaceName( buf ) < 0 ) return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DefaultRoute, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif //_CWMP_APPLY_
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int objForwading(char *name, struct CWMP_LEAF *e, int type, void *data)
{
#ifdef ROUTE_SUPPORT
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	STATICROUTE_T *p,route_entity;
	STATICROUTE_T target[2];
	CWMPDBG( 1, ( "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) ); 

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		//printf("type = eCWMP_tINITOBJ\n");
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
					
#ifdef _PRMT_WT107_
		{
			//dynamic route
			unsigned int dyn_route_num=0;
			dyn_route_num=getDynamicForwardingTotalNum();
			if(dyn_route_num>0)
			{
				if( create_Object( c, tForwardingObject, sizeof(tForwardingObject), dyn_route_num, 1 ) < 0 )
					return -1;		
			}
			add_objectNum( name, MAX_DYNAMIC_ROUTE_INSTNUM );
		}
#endif //_PRMT_WT107_

#ifdef ROUTE_SUPPORT
		MaxInstNum=findMaxForwardingInstNum();
#ifdef _PRMT_WT107_
		if(MaxInstNum<=MAX_DYNAMIC_ROUTE_INSTNUM) MaxInstNum=MAX_DYNAMIC_ROUTE_INSTNUM;
#endif //_PRMT_WT107_
		mib_get(MIB_STATICROUTE_TBL_NUM, (void *)&num);
		
		for( i=1; i<=num;i++ )
		{
			p = &route_entity;
			*((char *)p) = (char)i;
			memset( &target[0], 0, sizeof( STATICROUTE_T ) );
			memset( &target[1], 0, sizeof( STATICROUTE_T ) );
			if ( !mib_get(MIB_STATICROUTE_TBL, (void *)p))
				continue;

			memcpy(&target[0], &route_entity, sizeof(STATICROUTE_T));
			if( (p->InstanceNum==0) //maybe createn by web or cli
#ifdef _PRMT_WT107_
			    || (p->InstanceNum<=MAX_DYNAMIC_ROUTE_INSTNUM)
#endif //_PRMT_WT107_
			  )
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
				mib_set(MIB_STATICROUTE_MOD, (void *)&target);
			}
			CWMPDBG( 1, ("<%s:%d>name:%s(action:%d), 1 eCWMP_tINITOBJ p->InstanceNum=%d\n", __FUNCTION__, __LINE__, name,type,p->InstanceNum ) );
			if( create_Object( c, tForwardingObject, sizeof(tForwardingObject), 1, p->InstanceNum ) < 0 )
				return -1;
			CWMPDBG( 1, ("<%s:%d>name:%s(action:%d), 2 eCWMP_tINITOBJ p->InstanceNum=%d\n", __FUNCTION__, __LINE__, name,type,p->InstanceNum ) );
		}
#endif
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		//printf("type = eCWMP_tADDOBJ\n");
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tForwardingObject, sizeof(tForwardingObject), data );
		if( ret >= 0 )
		{
			STATICROUTE_T fentry;
			memset( &fentry, 0, sizeof( STATICROUTE_T ) );
			fentry.InstanceNum = *(unsigned int*)data;
			fentry.metric = -1;
			fentry.ifIndex = DUMMY_IFINDEX;
			mib_set(MIB_STATICROUTE_ADD, (void *)&fentry);
			ret=1;
		}
		
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
		//printf("type = eCWMP_tDELOBJ\n");
	     	int ret;
	     	unsigned int id;
	     	STATICROUTE_T route_old;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#ifdef _PRMT_WT107_
		if( *(unsigned int*)data<=MAX_DYNAMIC_ROUTE_INSTNUM ) //dynamic route
			return ERR_9001;
#endif //_PRMT_WT107_

		if( getForwardingEntryByInstNum( *(unsigned int*)data, &route_old, &id )<0 )
			return ERR_9005;
#ifdef _CWMP_APPLY_
		apply_Layer3Forwarding( CWMP_STOP, id, &route_old );
#endif
		mib_set(MIB_STATICROUTE_DEL, (void *)&route_old);
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		
#ifndef _CWMP_APPLY_
		if( ret == 0 ) return 1;
#endif
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
		//printf("type = eCWMP_tUPDATEOBJ\n");
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	old_table = (struct CWMP_LINKNODE*)entity->next;
	     	entity->next = NULL;

#ifdef _PRMT_WT107_
		num=getDynamicForwardingTotalNum();
	     	for( i=1; i<=num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

			remove_entity = remove_SiblingEntity( &old_table, i );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=i;					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tForwardingObject, sizeof(tForwardingObject), &MaxInstNum );
			}
	     	}
#endif //_PRMT_WT107_

#ifdef ROUTE_SUPPORT
		mib_get(MIB_STATICROUTE_TBL_NUM, (void *)&num);
	     	for( i=1; i<=num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

			p = &route_entity;
			*((char *)p) = (char)i;
			memset( &target[0], 0, sizeof( STATICROUTE_T ) );
			memset( &target[1], 0, sizeof( STATICROUTE_T ) );
			if ( !mib_get(MIB_STATICROUTE_TBL, (void *)p))
				continue;

			memcpy(&target[0], &route_entity, sizeof(STATICROUTE_T));
			
#ifdef _PRMT_WT107_
			if(p->InstanceNum<=MAX_DYNAMIC_ROUTE_INSTNUM) p->InstanceNum=0; //to get an new instnum > MAX_DYNAMIC_ROUTE_INSTNUM
#endif //_PRMT_WT107_			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				CWMPDBG( 1, ("<%s:%d>name:%s(action:%d), 1 eCWMP_tUPDATEOBJ MaxInstNum=%d\n", __FUNCTION__, __LINE__, name,type,MaxInstNum ) );
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tForwardingObject, sizeof(tForwardingObject), &MaxInstNum );
				
				CWMPDBG( 1, ("<%s:%d>name:%s(action:%d), 2 eCWMP_tUPDATEOBJ MaxInstNum=%d\n", __FUNCTION__, __LINE__, name,type,MaxInstNum ) );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					memcpy(&target[1], &route_entity, sizeof(STATICROUTE_T));
					mib_set(MIB_STATICROUTE_MOD, (void *)&target);
					
				}
			}	
	     	}
#endif
	     	
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

	     	return 0;
	     }
	}
	
#endif
	return -1;
}

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
int getChainID( struct CWMP_LINKNODE *ctable, int num )
{
	int id=-1;
	char buf[32];

//	sprintf( buf, "%d", num );
	while( ctable )
	{
		id++;
		//if( strcmp(ctable->name, buf)==0 )
		if( ctable->instnum==(unsigned int)num )
			break;
		ctable = ctable->sibling;
	}	
	return id;
}


//return -1:error,  0:not found, 1:found in /proc/net/route
//refer to user/busybox/route.c:displayroutes()
#ifdef ROUTE_SUPPORT
int queryRouteStatus( STATICROUTE_T *p )
{
	int ret=-1;

	//fprintf( stderr, "<%s:%d>Start\n",__FUNCTION__,__LINE__ );
	
	if(p)
	{
		char *routename="/proc/net/route";
		FILE *fp;
		
		fp=fopen( routename, "r" );
		if(fp)
		{
			char buff[256];
			int  i=0;
			while( fgets(buff, sizeof(buff), fp) != NULL )
			{
				int flgs, ref, use, metric;
				char ifname[16], ifname2[16]; 
				unsigned long int d,g,m;

				//fprintf( stderr, "<%s:%d>%s\n",__FUNCTION__,__LINE__,buff );
				i++;
				if(i==1) continue; //skip the first line
				 
				//Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT
				if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
					ifname, &d, &g, &flgs, &ref, &use, &metric, &m)!=8)
					break;//Unsuported kernel route format
				
				if( ifGetName( p->ifIndex, ifname2, sizeof(ifname2) )==0 ) //any interface
					ifname2[0]=0;

				//fprintf( stderr, "<%s:%d>%s==%s(%d)\n",__FUNCTION__,__LINE__,ifname,ifname2, ( (strlen(ifname2)==0) || (strcmp(ifname, ifname2)==0) ) );
				//fprintf( stderr, "<%s:%d>%06x==%06x(%d)\n",__FUNCTION__,__LINE__,*((unsigned long int*)(p->destID)),d,(*((unsigned long int*)(p->destID))==d) );
				//fprintf( stderr, "<%s:%d>%06x==%06x(%d)\n",__FUNCTION__,__LINE__,*((unsigned long int*)(p->netMask)),m,(*((unsigned long int*)(p->netMask))==m) );
				//fprintf( stderr, "<%s:%d>%06x==%06x(%d)\n",__FUNCTION__,__LINE__,*((unsigned long int*)(p->nextHop)),g,(*((unsigned long int*)(p->nextHop))==g) );
				//fprintf( stderr, "<%s:%d>%d==%d(%d)\n",__FUNCTION__,__LINE__, p->FWMetric, metric,( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) );

				if(  (p->Enable) &&
				     ( (p->ifIndex==DUMMY_IFINDEX) || (strcmp(ifname, ifname2)==0) ) && //interface: any or specific
				     (*((unsigned long int*)(p->dstAddr))==d) &&  //destIPaddress
				     (*((unsigned long int*)(p->netmask))==m) && //netmask
				     (*((unsigned long int*)(p->gateway))==g) && //GatewayIPaddress
				     ( (p->metric==-1 && metric==0) || (p->metric==metric )	) //ForwardingMetric
				  )
				{
					//fprintf( stderr, "<%s:%d>Found\n",__FUNCTION__,__LINE__ );
					ret=1;//found
					break;
				}
			}
			
			if( feof(fp) && (ret!=1) )
			{
					//fprintf( stderr, "<%s:%d>Not Found\n",__FUNCTION__,__LINE__ );
					ret=0; //not found				
			}
			fclose(fp);
		}
	}

	//fprintf( stderr, "<%s:%d>End(ret:%d)\n",__FUNCTION__,__LINE__,ret );
	
	return ret;
}
#endif

unsigned int getForwardingInstNum( char *name )
{
	return getInstNum( name, "Forwarding" );
}

unsigned int findMaxForwardingInstNum(void)
{
	unsigned int ret=0, i,num;
#ifdef ROUTE_SUPPORT
	STATICROUTE_T *p,entity;

	mib_get(MIB_STATICROUTE_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		p = &entity;
		
		*((char *)p) = (char)i;
		if ( !mib_get(MIB_STATICROUTE_TBL, (void *)p))
			continue;
		
		if( p->InstanceNum > ret )
			ret = p->InstanceNum;
	}
#endif
	return ret;
}

#ifdef ROUTE_SUPPORT
int getForwardingEntryByInstNum( unsigned int instnum, STATICROUTE_T *p, unsigned int *id )
{
	diag_printf("enter getForwadingEntryByInstNum, instnum=%d, id=%d\n", instnum, *id);
	int ret=-1;
	unsigned int i,num;
#ifdef ROUTE_SUPPORT
	if( (instnum==0) || (p==NULL) || (id==NULL) )
		return ret;
	mib_get(MIB_STATICROUTE_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		//printf("1: i=%d\n", i);
		*((char *)p) = (char)i;
		if ( !mib_get(MIB_STATICROUTE_TBL, (void *)p))
			continue;

		//printf("2: p->InstanceNum=%d, instnum=%d", i, instnum);
		if( p->InstanceNum==instnum )
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	//printf("out getForwadingEntryByInstNum\n");
#endif
	return ret;
}
#endif

#ifdef _PRMT_WT107_
int getDynamicForwardingTotalNum(void)
{
	int ret=0;
	char *routename="/proc/net/route";
	FILE *fp;
	
	fp=fopen( routename, "r" );
	if(fp)
	{
		char buff[256];
		
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			int flgs, ref, use, metric;
			char ifname[16], ifname2[16]; 
			unsigned long int d,g,m;

			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				ifname, &d, &g, &flgs, &ref, &use, &metric, &m)!=8)
				break;//Unsuported kernel route format
			
			{
				unsigned int i,num, is_match_static=0;
				STATICROUTE_T *p,entity;
				
				num = mib_chain_total( MIB_IP_ROUTE_TBL );
				for( i=0; i<num;i++ )
				{
					p = &entity;
					if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ))
						continue;
						
					if( ifGetName( p->ifIndex, ifname2, sizeof(ifname2) )==0 ) //any interface
						ifname2[0]=0;
		
					if(  (p->Enable) &&
					     ( (p->ifIndex==DUMMY_IFINDEX) || (strcmp(ifname, ifname2)==0) ) && //interface: any or specific
					     (*((unsigned long int*)(p->destID))==d) &&  //destIPaddress
					     (*((unsigned long int*)(p->netMask))==m) && //netmask
					     (*((unsigned long int*)(p->nextHop))==g) && //GatewayIPaddress
					     ( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) //ForwardingMetric
					  )
					{
						is_match_static=1; //static route
						break;
					}
					
				}				
				if(is_match_static==0) ret++; //dynamic route
			}
		}
		fclose(fp);
	}
	return ret;
}

int getDynamicForwardingEntryByInstNum( unsigned int instnum, STATICROUTE_T *pRoute )
{
	int ret=-1;
	char *routename="/proc/net/route";
	FILE *fp;
	
	if( (instnum==0) || (pRoute==NULL) ) return ret;
	memset( pRoute, 0, sizeof(STATICROUTE_T) );
	pRoute->InstanceNum = instnum;
	pRoute->FWMetric = -1;
	pRoute->ifIndex = DUMMY_IFINDEX;

	fp=fopen( routename, "r" );
	if(fp)
	{
		char buff[256];
		int count=0;
		
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			int flgs, ref, use, metric;
			char ifname[16], ifname2[16]; 
			unsigned long int d,g,m;

			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				ifname, &d, &g, &flgs, &ref, &use, &metric, &m)!=8)
				break;//Unsuported kernel route format
			
			{
				unsigned int i,num, is_match_static=0;
				MIB_CE_IP_ROUTE_T *p,entity;
				
				num = mib_chain_total( MIB_IP_ROUTE_TBL );
				for( i=0; i<num;i++ )
				{
					p = &entity;
					if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ))
						continue;
						
					if( ifGetName( p->ifIndex, ifname2, sizeof(ifname2) )==0 ) //any interface
						ifname2[0]=0;

					if(  (p->Enable) &&
					     ( (p->ifIndex==DUMMY_IFINDEX) || (strcmp(ifname, ifname2)==0) ) && //interface: any or specific
					     (*((unsigned long int*)(p->destID))==d) &&  //destIPaddress
					     (*((unsigned long int*)(p->netMask))==m) && //netmask
					     (*((unsigned long int*)(p->nextHop))==g) && //GatewayIPaddress
					     ( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) //ForwardingMetric
					  )
					{
						is_match_static=1; //static route
						break;
					}
				}
				if(is_match_static==0) count++; //dynamic route
			}
			
			if(count==instnum)
			{
				if( (strncmp(ifname,"ppp",3)==0) || (strncmp(ifname,"vc",2)==0) )
					pRoute->ifIndex = transfer2IfIndxfromIfName(ifname);
				else
					pRoute->ifIndex = DUMMY_IFINDEX;
				*((unsigned long int*)(pRoute->destID))=d;
				*((unsigned long int*)(pRoute->netMask))=m;
				*((unsigned long int*)(pRoute->nextHop))=g;
				pRoute->FWMetric=metric;
				pRoute->Enable = (flgs&RTF_UP)?1:0;
				if(flgs&RTF_HOST)
					pRoute->Type = 1; //host
				else if( (d==0) && (m==0) )
					pRoute->Type = 2; //default
				else
					pRoute->Type = 0; //network

				break;
			}
		}
		fclose(fp);
	}	
	
	return ret;
}
#endif //_PRMT_WT107_
