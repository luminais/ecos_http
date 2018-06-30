#include "prmt_queuemnt.h"
#ifdef IP_QOS

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getQueueInstNum( char *name );
unsigned int getClassInstNum( char *name );
unsigned int findClassInstNum(void);
int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T *p, unsigned int *id);


struct CWMP_OP tQueueEntityLeafOP = { getQueueEntity, setQueueEntity };
struct CWMP_PRMT tQueueEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"QueueKey",		eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueStatus",		eCWMP_tSTRING,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueBufferLength",	eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
//{"QueueWeight",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueuePrecedence",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"REDThreshold",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"REDPercentage",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"DropAlgorithm",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"SchedulerAlgorithm",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"ShapingRate",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"ShapingBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP}
};
enum eQueueEntityLeaf
{
	eQE_QueueKey,
	eQE_QueueEnable,
	eQE_QueueStatus,
	eQE_QueueInterface,
	eQE_QueueBufferLength,
	eQE_QueuePrecedence,
	eQE_DropAlgorithm
};
struct CWMP_LEAF tQueueEntityLeaf[] =
{
{ &tQueueEntityLeafInfo[eQE_QueueKey] },
{ &tQueueEntityLeafInfo[eQE_QueueEnable] },
{ &tQueueEntityLeafInfo[eQE_QueueStatus] },
{ &tQueueEntityLeafInfo[eQE_QueueInterface] },
{ &tQueueEntityLeafInfo[eQE_QueueBufferLength] },
{ &tQueueEntityLeafInfo[eQE_QueuePrecedence] },
{ &tQueueEntityLeafInfo[eQE_DropAlgorithm] },
{ NULL }
};

struct CWMP_PRMT tQueueObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eQueueObject
{
	eQueueObject0
};
struct CWMP_LINKNODE tQueueObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tQueueObjectInfo[eQueueObject0],	tQueueEntityLeaf,	NULL,		NULL,			0},
};


#if 0
struct sCWMP_ENTITY tPolicerEntity[] =
{
/*(name,		type,		flag,			accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"PolicerKey",		eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"PolicerEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PolicerStatus",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CommittedRate",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CommittedBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*ExcessBurstSize*/
/*PeakRate*/
/*PeakBurstSize*/
{"MeterType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PossibleMeterTypes",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"ConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*PartialConformingAction*/
{"NonConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CountedPackets",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CountedBytes",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"",			eCWMP_tNONE,	0,			NULL,		NULL,		NULL,		NULL,		NULL}
};

struct sCWMP_ENTITY tPolicer[] =
{
/*(name,			type,		flag,					accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
//{"1",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
};
#endif


struct CWMP_OP tClassEntityLeafOP = { getClassEntity, setClassEntity };
struct CWMP_PRMT tClassEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ClassificationKey",		eCWMP_tUINT,	CWMP_READ,		&tClassEntityLeafOP},
{"ClassificationEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassificationStatus",	eCWMP_tSTRING,	CWMP_READ,		&tClassEntityLeafOP},
{"ClassificationOrder",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassInterface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestIP",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestIPExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceIP",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourceIPExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"Protocol",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ProtocolExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestPort",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestPortRangeMax",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestPortExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourcePort",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourcePortRangeMax",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourcePortExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourceMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*SourceMACMask*/
//{"SourceMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*DestMACMask*/
//{"DestMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*Ethertype, EthertypeExclude*/
/*SSAP, SSAPExclude*/
/*DSAP, DSAPExclude*/
/*LLCControl, LLCControlExclude*/
/*SNAPOUI, SNAPOUIExclude*/
/*SourceVendorClassID, SourceVendorClassIDExclude*/
/*DestVendorClassID, DestVendorClassIDExclude*/
/*SourceClientID, SourceClientIDExclude*/
/*DestClientID, DestClientIDExclude*/
/*SourceUserClassID, SourceUserClassIDExclude*/
/*DestUserClassID, DestUserClassIDExclude*/
/*TCPACK, TCPACKExclude*/
/*IPLengthMin, IPLengthMax, IPLengthExclude*/
//{"DSCPCheck",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DSCPExclude",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DSCPMark",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"EthernetPriorityCheck",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"EthernetPriorityExclude",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"EthernetPriorityMark",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"VLANIDCheck",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"VLANIDExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ForwardingPolicy",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ClassPolicer",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassQueue",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP}
/*ClassApp*/
};
enum eClassEntityLeaf
{
	eQC_ClassificationKey,
	eQC_ClassificationEnable,
	eQC_ClassificationStatus,
	eQC_ClassificationOrder,
	eQC_ClassInterface,
	eQC_DestIP,
	eQC_DestMask,
	eQC_SourceIP,
	eQC_SourceMask,
	eQC_Protocol,
	eQC_ProtocolExclude,
	eQC_DestPort,
	eQC_SourcePort,
	eQC_EthernetPriorityCheck,
	eQC_EthernetPriorityMark,
	eQC_ClassQueue
};
struct CWMP_LEAF tClassEntityLeaf[] =
{
{ &tClassEntityLeafInfo[eQC_ClassificationKey] },
{ &tClassEntityLeafInfo[eQC_ClassificationEnable] },
{ &tClassEntityLeafInfo[eQC_ClassificationStatus] },
{ &tClassEntityLeafInfo[eQC_ClassificationOrder] },
{ &tClassEntityLeafInfo[eQC_ClassInterface] },
{ &tClassEntityLeafInfo[eQC_DestIP] },
{ &tClassEntityLeafInfo[eQC_DestMask] },
{ &tClassEntityLeafInfo[eQC_SourceIP] },
{ &tClassEntityLeafInfo[eQC_SourceMask] },
{ &tClassEntityLeafInfo[eQC_Protocol] },
{ &tClassEntityLeafInfo[eQC_ProtocolExclude] },
{ &tClassEntityLeafInfo[eQC_DestPort] },
{ &tClassEntityLeafInfo[eQC_SourcePort] },
{ &tClassEntityLeafInfo[eQC_EthernetPriorityCheck] },
{ &tClassEntityLeafInfo[eQC_EthernetPriorityMark] },
{ &tClassEntityLeafInfo[eQC_ClassQueue] },
{ NULL }
};

struct CWMP_PRMT tClassObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eClassObject
{
	eClassObject0
};
struct CWMP_LINKNODE tClassObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tClassObjectInfo[eClassObject0],	tClassEntityLeaf,	NULL,		NULL,			0},
};



struct CWMP_OP tQueueMntLeafOP = { getQueueMnt,	setQueueMnt };
struct CWMP_PRMT tQueueMntLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"MaxQueues",			eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxClassificationEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"ClassificationNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxAppEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"AppNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxFlowEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"FlowNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxPolicerEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"PolicerNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxQueueEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"QueueNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
//{"DefaultForwardingPolicy",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultPolicer",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultQueue",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultDSCPMark",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"DefaultEthernetPriorityMark",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"AvailableAppList",		eCWMP_tSTRING,	CWMP_READ,		&tQueueMntLeafOP}
};
enum eQueueMntLeaf
{
	eQ_Enable,
	eQ_MaxQueues,
	eQ_MaxClassificationEntries,
	eQ_ClassificationNumberOfEntries,
	eQ_MaxAppEntries,
	eQ_AppNumberOfEntries,
	eQ_MaxFlowEntries,
	eQ_FlowNumberOfEntries,
	eQ_MaxPolicerEntries,
	eQ_PolicerNumberOfEntries,
	eQ_MaxQueueEntries,
	eQ_QueueNumberOfEntries,
	eQ_DefaultEthernetPriorityMark,
	eQ_AvailableAppList
};
struct CWMP_LEAF tQueueMntLeaf[] =
{
{ &tQueueMntLeafInfo[eQ_Enable] },
{ &tQueueMntLeafInfo[eQ_MaxQueues] },
{ &tQueueMntLeafInfo[eQ_MaxClassificationEntries] },
{ &tQueueMntLeafInfo[eQ_ClassificationNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxAppEntries] },
{ &tQueueMntLeafInfo[eQ_AppNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxFlowEntries] },
{ &tQueueMntLeafInfo[eQ_FlowNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxPolicerEntries] },
{ &tQueueMntLeafInfo[eQ_PolicerNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxQueueEntries] },
{ &tQueueMntLeafInfo[eQ_QueueNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_DefaultEthernetPriorityMark] },
{ &tQueueMntLeafInfo[eQ_AvailableAppList] },
{ NULL }
};


struct CWMP_OP tQM_Class_OP = { NULL, objClass };
//struct CWMP_OP tQM_Policer_OP = { NULL, objPolicer };
struct CWMP_OP tQM_Queue_OP = { NULL, objQueue };
struct CWMP_PRMT tQueueMntObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Classification",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Class_OP},
//{"Policer",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Policer_OP},
{"Queue",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Queue_OP}
};
enum tQueueMntObject
{
	eQ_Classification,
	//eQ_Policer,
	eQ_Queue
};
struct CWMP_NODE tQueueMntObject[] =
{
/*info,  					leaf,			node)*/
{&tQueueMntObjectInfo[eQ_Classification],	NULL,			NULL},
{&tQueueMntObjectInfo[eQ_Queue],		NULL,			NULL},
{NULL,						NULL,			NULL}
};


int getQueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "QueueKey" )==0 )
	{
		*data=uintdup( qinst );
	}else if( strcmp( lastname, "QueueEnable" )==0 )
	{
		 *data=booldup( 1 );
	}else if( strcmp( lastname, "QueueStatus" )==0 )
	{
		 *data=strdup( "Enabled" );
	}else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		 *data=strdup( "" );
	}else if( strcmp( lastname, "QueueBufferLength" )==0 )
	{
		 *data=uintdup( 8*2048 );
//	}else if( strcmp( lastname, "QueueWeight" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{
		 *data=uintdup( qinst );
//	}else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		 *data=strdup( "DT" );
//	}else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
//	{
//		 *data=strdup( "SP" );
//	}else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int setQueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;



	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	if( strcmp( lastname, "QueueEnable" )==0 )
	{

		int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i==0) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "QueueWeight" )==0 )
//	{
	}else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{

		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i!=qinst) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
	}else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strcmp( buf, "DT" )!=0 ) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
//	{
//		if(buf==NULL) return ERR_9007;
//		if( strcmp( buf, "SP" )!=0 ) return ERR_9001;
//		return 0;
//	}else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int objQueue(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	num=IPQOS_NUM_PRIOQ;
		for( i=0; i<num;i++ )
		{
			MaxInstNum = i+1;
			if( create_Object( c, tQueueObject, sizeof(tQueueObject), 1, MaxInstNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tDELOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	return 0;
	     }
	}
	return -1;
}

#if 0
int getPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	char	*lastname = entity->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->type;
	*data = NULL;
	if( strcmp( lastname, "PolicerKey" )==0 )
	{
		*data=uintdup( 0 );
	}else if( strcmp( lastname, "PolicerEnable" )==0 )
	{
		 *data=booldup( 0 );
	}else if( strcmp( lastname, "PolicerStatus" )==0 )
	{
		 *data=strdup( "Disabled" );
	}else if( strcmp( lastname, "CommittedRate" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "CommittedBurstSize" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MeterType" )==0 )
	{
		 *data=strdup( "SimpleTokenBucket" );
	}else if( strcmp( lastname, "PossibleMeterTypes" )==0 )
	{
		 *data=strdup( "SimpleTokenBucket,SingleRateThreeColor,TwoRateThreeColor" );
	}else if( strcmp( lastname, "ConformingAction" )==0 )
	{
		 *data=strdup( "Null" );
	}else if( strcmp( lastname, "NonConformingAction" )==0 )
	{
		 *data=strdup( "Drop" );
	}else if( strcmp( lastname, "CountedPackets" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "CountedBytes" )==0 )
	{
		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	char	*lastname = entity->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PolicerEnable" )==0 )
	{
	}else if( strcmp( lastname, "CommittedRate" )==0 )
	{
	}else if( strcmp( lastname, "CommittedBurstSize" )==0 )
	{
	}else if( strcmp( lastname, "MeterType" )==0 )
	{
	}else if( strcmp( lastname, "ConformingAction" )==0 )
	{
	}else if( strcmp( lastname, "NonConformingAction" )==0 )
	{
	}else{
		return ERR_9005;
	}
	
	return 0;
}




int objPolicer(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return 0;
	     }
	case eCWMP_tDELOBJ:
	     {
		return 0;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	return 0;
	     }
	}
	return -1;
}
#endif

int getClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	char buf[256]={0};
	MIB_CE_IP_QOS_T *p, qos_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ClassificationKey" )==0 )
	{
		*data = uintdup( p->InstanceNum );
	}else if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
		 *data=booldup( p->enable );
	}else if( strcmp( lastname, "ClassificationStatus" )==0 )
	{
		if( p->enable==0 )
		 	*data=strdup( "Disabled" );
		else
			*data=strdup( "Enabled" );
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
		 *data=uintdup( chainid+1 );
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( p->phyPort==0xff )
			*data=strdup( "LAN" );
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
		else if( p->phyPort==0 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4");
		else if( p->phyPort==1 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3");
		else if( p->phyPort==2 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2");
		else if( p->phyPort==3 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
#else
		else if( p->phyPort==0 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
#endif
#ifdef WLAN_SUPPORT
		else if( p->phyPort==5 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1");
#ifdef WLAN_MBSSID
		else if( p->phyPort==6 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.2");
		else if( p->phyPort==7 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.3");
		else if( p->phyPort==8 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.4");
		else if( p->phyPort==9 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.5");
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else 
			return ERR_9002;
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&p->dip)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "DestMask" )==0 )
	{
		unsigned long mask=0;
		int i,mbit;
		mbit = p->dmaskbit;
		for(i=0;i<32;i++)
		{
			mask = mask << 1;
			if(mbit)
			{ 
				mask = mask | 0x1;
				mbit--;
			}
		}
		mask = ntohl(mask);
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&mask)));
		*data = strdup( buf );
//	}else if( strcmp( lastname, "DestIPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "SourceIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&p->sip)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "SourceMask" )==0 )
	{
		unsigned long mask=0;
		int i,mbit;
		mbit = p->smaskbit;
		for(i=0;i<32;i++)
		{
			mask = mask << 1;
			if(mbit)
			{ 
				mask = mask | 0x1;
				mbit--;
			}
		}
		mask = ntohl(mask);
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&mask)));
		*data = strdup( buf );
//	}else if( strcmp( lastname, "SourceIPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "Protocol" )==0 )
	{
		if( p->protoType==PROTO_NONE )
			*data=intdup( -1 );
		else if( p->protoType==PROTO_TCP )
			*data=intdup( 6 );
		else if( p->protoType==PROTO_UDP )
			*data=intdup( 17 );
		else if( p->protoType==PROTO_ICMP )
			*data=intdup( 1 );
		else
			return ERR_9002;			
//	}else if( strcmp( lastname, "ProtocolExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "DestPort" )==0 )
	{
		if( p->dPort==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->dPort );
//	}else if( strcmp( lastname, "DestPortRangeMax" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DestPortExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "SourcePort" )==0 )
	{
		if( p->sPort==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->sPort );
//	}else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "SourcePortExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "SourceMACAddress" )==0 )
//	{
//		 *data=strdup( "" );
//	}else if( strcmp( lastname, "SourceMACExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DestMACAddress" )==0 )
//	{
//		 *data=strdup( "" );
//	}else if( strcmp( lastname, "DestMACExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DSCPCheck" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DSCPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{
		if( p->m_1p==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( (int)p->m_1p-1 );
//	}else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{
		if( p->m_1p==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( (int)p->m_1p-1 );
//	}else if( strcmp( lastname, "VLANIDCheck" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "VLANIDExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "ForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "ClassPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{
		 *data=intdup( p->prior+1 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setClassEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	MIB_CE_IP_QOS_T *p, qos_entity, old_qos_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;


	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;
	memcpy( &old_qos_entity, &qos_entity, sizeof(MIB_CE_IP_QOS_T) );

	if( strcmp( lastname, "ClassificationEnable" )==0 )
	{

		int *i = data;

		if(i==NULL) return ERR_9007;
		p->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{

		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i==0 ) return ERR_9007;
		if( *i!=(chainid+1) ) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp( buf, "LAN" )==0 )
			p->phyPort=0xff;
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0 )
			p->phyPort=0;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 )
			p->phyPort=1;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 )
			p->phyPort=2;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=3;
#else
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=0;
#endif
#ifdef WLAN_SUPPORT
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" )==0 )
			p->phyPort=5;
#ifdef WLAN_MBSSID
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" )==0 )
			p->phyPort=6;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" )==0 )
			p->phyPort=7;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" )==0 )
			p->phyPort=8;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" )==0 )
			p->phyPort=9;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( p->dip, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DestMask" )==0 )
	{
		unsigned long mask;
		int intVal, i, mbit;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, (struct in_addr *)&mask )==0 ) //the ip address is error.
			return ERR_9007;
		mask = htonl(mask);
		mbit=0; intVal=0;
		for (i=0; i<32; i++)
		{
			if (mask&0x80000000)
			{
				if (intVal) return ERR_9007;
				mbit++;
			}
			else
				intVal=1;
			mask <<= 1;
		}
		p->dmaskbit = mbit;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "DestIPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "SourceIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( p->sip, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "SourceMask" )==0 )
	{
		unsigned long mask;
		int intVal, i, mbit;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, (struct in_addr *)&mask )==0 ) //the ip address is error.
			return ERR_9007;
		mask = htonl(mask);
		mbit=0; intVal=0;
		for (i=0; i<32; i++)
		{
			if (mask&0x80000000)
			{
				if (intVal) return ERR_9007;
				mbit++;
			}
			else
				intVal=1;
			mask <<= 1;
		}
		p->smaskbit = mbit;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "SourceIPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "Protocol" )==0 )
	{


		int *pro = data;

		if(pro==NULL) return ERR_9007;
		switch(*pro)
		{
		case -1:
			p->protoType=PROTO_NONE;
			break;
		case 1:
			p->protoType=PROTO_ICMP;
			break;
		case 6:
			p->protoType=PROTO_TCP;
			break;
		case 17:
			p->protoType=PROTO_UDP;
			break;
		default:
			return ERR_9001;
		}
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "ProtocolExclude" )==0 )
//	{
	}else if( strcmp( lastname, "DestPort" )==0 )
	{

		int *dport = data;

		if( dport==NULL ) return ERR_9007;
		if( *dport==-1 )
			p->dPort = 0;
		else if( (*dport>=1) && (*dport<=65535) )
			p->dPort = *dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "DestPortRangeMax" )==0 )
//	{
//	}else if( strcmp( lastname, "DestPortExclude" )==0 )
//	{
	}else if( strcmp( lastname, "SourcePort" )==0 )
	{

		int *sport = data;

		if( sport==NULL ) return ERR_9007;
		if( *sport==-1 )
			p->sPort = 0;
		else if( (*sport>=1) && (*sport<=65535) )
			p->sPort = *sport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
//	{
//	}else if( strcmp( lastname, "SourcePortExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "SourceMACAddress" )==0 )
//	{
//	}else if( strcmp( lastname, "SourceMACExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "DestMACAddress" )==0 )
//	{
//	}else if( strcmp( lastname, "DestMACExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "DSCPCheck" )==0 )
//	{
//	}else if( strcmp( lastname, "DSCPExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "DSCPMark" )==0 )
//	{
	}else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{


		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->m_1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
//	{
	}else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{


		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->m_1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "VLANIDCheck" )==0 )
//	{
//	}else if( strcmp( lastname, "VLANIDExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "ForwardingPolicy" )==0 )
//	{
//	}else if( strcmp( lastname, "ClassPolicer" )==0 )
//	{
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{


		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<1 || *i>IPQOS_NUM_PRIOQ ) return ERR_9007;
		p->prior = (unsigned char)(*i-1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#else
		return 1;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int objClass(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_CE_IP_QOS_T *p, qos_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = findClassInstNum();
		num = mib_chain_total( MIB_IP_QOS_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ) )
				continue;
			
			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, i );
			}
			if( create_Object( c, tClassObject, sizeof(tClassObject), 1, p->InstanceNum ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, MaxInstNum );
		return 0;	     		     	
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	unsigned int num=0;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		num = mib_chain_total( MIB_IP_QOS_TBL );
		if(num>=MAX_QOS_RULE) return ERR_9004;
		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_QOS_T qos_entry;
			memset( &qos_entry, 0, sizeof( MIB_CE_IP_QOS_T ) );
			qos_entry.InstanceNum = *(unsigned int*)data;
			qos_entry.phyPort=0xff;
			qos_entry.prior=3;
			qos_entry.m_iptos=0xff;
			mib_chain_add( MIB_IP_QOS_TBL, (unsigned char*)&qos_entry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
		unsigned int i,num;
		int ret=0;
		MIB_CE_IP_QOS_T *p,qos_entity;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	
		num = mib_chain_total( MIB_IP_QOS_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
				continue;
			if( p->InstanceNum == *(unsigned int*)data )
				break;
		}	     	
		if(i==num) return ERR_9005;
		
#ifdef _CWMP_APPLY_
		//why don't use CWMP_RESTART?
		//for IPQoS, use the chain index to count the mark number, get_classification_mark()
		//after deleting one rule, the mark numbers will change
		//hence, stop all rules=>delete one rule=>start the rest rules;
		apply_IPQoSRule( CWMP_STOP, -1, NULL );
#endif
	     	mib_chain_delete( MIB_IP_QOS_TBL, i );
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
#ifdef _CWMP_APPLY_
		apply_IPQoSRule( CWMP_START, -1, NULL );
#else
		if( ret == 0 ) return 1;
#endif
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	num = mib_chain_total( MIB_IP_QOS_TBL );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_IP_QOS_T *p,qos_entity;

			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ) )
				continue;
			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, i );
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




int getQueueMnt(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int num=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_MPMODE, (void *)&vChar);
		vChar = vChar&0x02;
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "MaxQueues" )==0 )
	{
		*data=uintdup( IPQOS_NUM_PRIOQ );
	}else if( strcmp( lastname, "MaxClassificationEntries" )==0 )
	{
		*data=uintdup( MAX_QOS_RULE );
	}else if( strcmp( lastname, "ClassificationNumberOfEntries" )==0 )
	{
		num = mib_chain_total(MIB_IP_QOS_TBL);
		*data=uintdup( num );
	}else if( strcmp( lastname, "MaxAppEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "AppNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxFlowEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "FlowNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxPolicerEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "PolicerNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxQueueEntries" )==0 )
	{
		 *data=uintdup( IPQOS_NUM_PRIOQ );
	}else if( strcmp( lastname, "QueueNumberOfEntries" )==0 )
	{
		 *data=uintdup( IPQOS_NUM_PRIOQ );
//	}else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
		 *data=intdup( -1 );
	}else if( strcmp( lastname, "AvailableAppList" )==0 )
	{
		 *data=strdup( "" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setQueueMnt(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;


	if( strcmp( lastname, "Enable" )==0 )
	{

		int *i = data;

		if(i==NULL) return ERR_9007;
		mib_get(MIB_MPMODE, (void *)&vChar);
		if(*i==0)
			vChar &= 0xfd;
		else
			vChar |= 0x02;
		mib_set(MIB_MPMODE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		//apply_IPQoS has higher priority than apply_IPQoSRule
		apply_add( CWMP_PRI_H, apply_IPQoS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
	}else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{

		int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i!=-1) return ERR_9001;
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/

unsigned int getQueueInstNum( char *name )
{
	return getInstNum( name, "Queue" );
}

unsigned int getClassInstNum( char *name )
{
	return getInstNum( name, "Classification" );
}

unsigned int findClassInstNum(void)
{
	unsigned int ret=0, i,num;
	MIB_CE_IP_QOS_T *p,qos_entity;
	
	num = mib_chain_total( MIB_IP_QOS_TBL );
	for( i=0; i<num;i++ )
	{
		p = &qos_entity;
		if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
			continue;
		if( p->InstanceNum > ret )
			ret = p->InstanceNum;
	}
	
	return ret;
}

int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) ) return ret;
	
	num = mib_chain_total( MIB_IP_QOS_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
			continue;
		if( p->InstanceNum == instnum )
		{
			*id = i;
			ret=0;
			break;
		}
	}
	
	return ret;
}

#endif /*IP_QOS*/
