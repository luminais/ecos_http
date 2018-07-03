#include "prmt_services.h"

#ifdef CONFIG_APP_TR104
#include "prmt_voiceservice.h"
#endif
#ifdef _PRMT_SERVICES_


#ifdef _PRMT_X_CT_COM_IPTV_
struct CWMP_OP tCT_IPTVLeafOP = { getCT_IPTV, setCT_IPTV };
struct CWMP_PRMT tCT_IPTVLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IGMPEnable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"ProxyEnable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"SnoopingEnable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"STBNumber",			eCWMP_tUINT,	CWMP_READ,	&tCT_IPTVLeafOP},
};
enum eCT_IPTVLeaf
{
	eCTIGMPEnable,
	eCTProxyEnable,
	eCTSnoopingEnable,
	eCTSTBNumber
};
struct CWMP_LEAF tCT_IPTVLeaf[]=
{
{ &tCT_IPTVLeafInfo[eCTIGMPEnable] },
{ &tCT_IPTVLeafInfo[eCTProxyEnable] },
{ &tCT_IPTVLeafInfo[eCTSnoopingEnable] },
{ &tCT_IPTVLeafInfo[eCTSTBNumber] },
{ NULL }
};
#endif


#if 0 //def _PRMT_X_CT_COM_MONITOR_
struct CWMP_OP tCT_MONITORLeafOP = { getCT_MONITOR, setCT_MONITOR };
struct CWMP_PRMT tCT_MONITORLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MONITORLeafOP},
{"MonitorNumber",		eCWMP_tUINT,	CWMP_READ,		&tCT_MONITORLeafOP}
};
enum eCT_MONITORLeaf
{
	eCTMNT_Enable,
	eCTMNT_Number
};
struct CWMP_LEAF tCT_MONITORLeaf[] =
{
{ &tCT_MONITORLeafInfo[eCTMNT_Enable] },
{ &tCT_MONITORLeafInfo[eCTMNT_Number] },
{ NULL }
};
#endif


#ifdef _PRMT_X_CT_COM_VPDN_
struct CWMP_OP tCT_VPDNLeafOP = { getCT_VPDN, setCT_VPDN };
struct CWMP_PRMT tCT_VPDNLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_VPDNLeafOP}
};
enum eCT_VPDNLeaf
{
	eCTVPDN_Enable
};
struct CWMP_LEAF tCT_VPDNLeaf[] =
{
{ &tCT_VPDNLeafInfo[eCTVPDN_Enable] },
{ NULL }
};
#endif 

#ifdef _PRMT_X_CT_COM_MWBAND_
struct CWMP_OP tCT_MWBANDLeafOP = { getCT_MWBAND, setCT_MWBAND };
struct CWMP_PRMT tCT_MWBANDLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Mode",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"TotalTerminalNumber",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"STBRestrictEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"STBNumber",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"CameraRestrictEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"CameraNumber",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"ComputerRestrictEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"ComputerNumber",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"PhoneRestrictEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"PhoneNumber",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP}
};
enum eCT_MWBANDLeaf
{
	eCTMMode,
	eCTMTotalTerminalNumber,
	eCTMSTBRestrictEnable,
	eCTMSTBNumber,
	eCTMCameraRestrictEnable,
	eCTMCameraNumber,
	eCTMComputerRestrictEnable,
	eCTMComputerNumber,
	eCTMPhoneRestrictEnable,
	eCTMPhoneNumber
};
struct CWMP_LEAF tCT_MWBANDLeaf[] =
{
{ &tCT_MWBANDLeafInfo[eCTMMode] },
{ &tCT_MWBANDLeafInfo[eCTMTotalTerminalNumber] },
{ &tCT_MWBANDLeafInfo[eCTMSTBRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMSTBNumber] },
{ &tCT_MWBANDLeafInfo[eCTMCameraRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMCameraNumber] },
{ &tCT_MWBANDLeafInfo[eCTMComputerRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMComputerNumber] },
{ &tCT_MWBANDLeafInfo[eCTMPhoneRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMPhoneNumber] },
{ NULL }
};
#endif

#ifdef CONFIG_APP_TR104
struct CWMP_OP tSRV_VoiceService_OP = { NULL, objVoiceService };
#endif

struct CWMP_PRMT tServicesObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
#ifdef _PRMT_X_CT_COM_IPTV_
{"X_CT-COM_IPTV",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
{"X_CT-COM_MWBAND",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#if 0 //def _PRMT_X_CT_COM_MONITOR_
{"X_CT-COM_Monitor",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
{"X_CT-COM_VPDN",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef CONFIG_APP_TR104
{"VoiceService",		eCWMP_tOBJECT,	CWMP_READ,	&tSRV_VoiceService_OP},
#endif
};
enum eServicesObject
{
#ifdef _PRMT_X_CT_COM_IPTV_
	eSX_CTCOM_IPTV,
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
	eSX_CTCOM_MWBAND,
#endif
#if 0 //def _PRMT_X_CT_COM_MONITOR_
	eSX_CTCOM_MONITOR,
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
	eSX_CTCOM_VPDN,
#endif
#ifdef CONFIG_APP_TR104
	eSX_VoiceService,
#endif
	eSX_CTCOM_DUMMY,
};
struct CWMP_NODE tServicesObject[] =
{
/*info,  				leaf,			next)*/
#ifdef _PRMT_X_CT_COM_IPTV_
{&tServicesObjectInfo[eSX_CTCOM_IPTV],	tCT_IPTVLeaf,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
{&tServicesObjectInfo[eSX_CTCOM_MWBAND],tCT_MWBANDLeaf,		NULL},
#endif
#if 0 //def _PRMT_X_CT_COM_MONITOR_
{&tServicesObjectInfo[eSX_CTCOM_MONITOR],tCT_MONITORLeaf,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
{&tServicesObjectInfo[eSX_CTCOM_VPDN],	tCT_VPDNLeaf,		NULL},
#endif
#ifdef CONFIG_APP_TR104
{&tServicesObjectInfo[eSX_VoiceService],	NULL,		NULL},
#endif
{NULL,					NULL,			NULL}	
};
#ifdef _PRMT_X_CT_COM_USERINFO_
struct CWMP_OP tCT_UserInfoLeafOP = { getCT_UserInfo, setCT_UserInfo };
struct CWMP_PRMT tCT_UserInfoLeafInfo[] =
{
/*(name,			type,		flag,				op)*/
{"UserId",			eCWMP_tSTRING,	CWMP_WRITE | CWMP_READ,		&tCT_UserInfoLeafOP},
{"Status",			eCWMP_tBOOLEAN,	CWMP_WRITE | CWMP_READ,		&tCT_UserInfoLeafOP},
{"Limit",			eCWMP_tUINT,	CWMP_WRITE | CWMP_READ,		&tCT_UserInfoLeafOP},
{"Times",			eCWMP_tUINT,	CWMP_WRITE | CWMP_READ,		&tCT_UserInfoLeafOP},

{"Result",			eCWMP_tUINT,	CWMP_WRITE | CWMP_READ,		&tCT_UserInfoLeafOP},

};
enum eCT_UserInfoLeaf
{
	eCT_Q_UserId,
	eCT_Q_Status,
	eCT_Q_Limit,
	eCT_Q_Times,

	eCT_Q_Result

};
struct CWMP_LEAF tCT_UserInfoLeaf[] =
{
{ &tCT_UserInfoLeafInfo[eCT_Q_UserId] },
{ &tCT_UserInfoLeafInfo[eCT_Q_Status] },
{ &tCT_UserInfoLeafInfo[eCT_Q_Limit] },
{ &tCT_UserInfoLeafInfo[eCT_Q_Times] },

{ &tCT_UserInfoLeafInfo[eCT_Q_Result] },

{ NULL },
};

int getCT_UserInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int vChar;
	unsigned int vUint;
	char buf[128];
	
	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "UserId") == 0) {
		mib_get(MIB_CWMP_USERINFO_USERID, buf);
		*data = strdup(buf);
	} else if (strcmp(lastname, "UserName") == 0) {
		mib_get(MIB_CWMP_USERINFO_USERNAME, buf);
		*data = strdup(buf);
	} else if (strcmp(lastname, "Status") == 0) {
		mib_get(MIB_CWMP_USERINFO_STATUS, &vChar);
		*data = booldup(vChar);
	} else if (strcmp(lastname, "Limit") == 0) {
		mib_get(MIB_CWMP_USERINFO_LIMIT, &vUint);
		*data = uintdup(vUint);
	} else if (strcmp(lastname, "Times") == 0) {
		mib_get(MIB_CWMP_USERINFO_TIMES, &vUint);
		*data = uintdup(vUint);
	}

	else if (strcmp(lastname, "Result") == 0) {
		mib_get(MIB_CWMP_USERINFO_RESULT, &vUint);
		*data = uintdup(vUint);
	}

	else {
		return ERR_9005;
	}
	
	return 0;
}

int setCT_UserInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned char vChar = 0;
	
	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint = 0;
	unsigned int tmpuint = 0;
	int tmpbool = 0;
	if (changestring2int(data, entity->info->type, type, &tmpint, &tmpuint, &tmpbool) < 0)
		return ERR_9006;
#else
	if (entity->info->type != type)
		return ERR_9006;
#endif
	
	if (strcmp(lastname, "UserId") == 0) {
		if (buf == NULL)
			return ERR_9007;
		if (strlen(buf) >= MAX_NAME_LEN)
			return ERR_9007;
		mib_set(MIB_CWMP_USERINFO_USERID, buf);

		return 0;
	} else if (strcmp(lastname, "UserName") == 0) {
		if (buf == NULL)
			return ERR_9007;
		if (strlen(buf) >= MAX_NAME_LEN)
			return ERR_9007;
		mib_set(MIB_CWMP_USERINFO_USERNAME, buf);

		return 0;
	} else if (strcmp(lastname, "Status") == 0) {
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif
		unsigned int regInformStatus;
		unsigned int regStatus;
		unsigned int regTimes;
		unsigned int regResult;
		unsigned int needReboot;

		if (i == NULL)
			return ERR_9007;

		/*xl_yue:20081225 get the inform status to avoid acs responses twice for only once informing*/
		mib_get(MIB_CWMP_REG_INFORM_STATUS, &regInformStatus);
		/*xl_yue:20081225 END*/
		if (regInformStatus != CWMP_REG_REQUESTED) {
			return 0;
		}	
		regInformStatus = CWMP_REG_RESPONSED;
		mib_set(MIB_CWMP_REG_INFORM_STATUS, &regInformStatus);

		mib_get(MIB_CWMP_USERINFO_TIMES, &regTimes);
		regStatus = *i;
		if (regStatus == 0) {
			regTimes = 0;
			/* 
			 * regStatus == 4 indicates timeout,
			 * so regTimes should not be increased
			 * according to the specification
			 */
		} else if (regStatus != 4) {
			regTimes++;
		}
		mib_set(MIB_CWMP_USERINFO_STATUS, &regStatus);
		mib_set(MIB_CWMP_USERINFO_TIMES, &regTimes);
	
		regResult = NO_SET;
		mib_set(MIB_CWMP_USERINFO_RESULT, &regResult);
		needReboot = 0;
		mib_set(MIB_CWMP_NEED_REBOOT, &needReboot);
		unlink(REBOOT_DELAY_FILE);
		return 0;
	} else if (strcmp(lastname, "Limit") == 0) {
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if (i == NULL)
			return ERR_9007;
		mib_set(MIB_CWMP_USERINFO_LIMIT, i);

		return 0;
	} else if (strcmp(lastname, "Times") == 0) {
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if (i == NULL)
			return ERR_9007;
		mib_set(MIB_CWMP_USERINFO_TIMES, i);

		return 0;
	}

	else if (strcmp(lastname, "Result") == 0) {
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		unsigned int vUint;
		FILE *fp;

		if (i == NULL) return ERR_9007;
		vUint = *i;

	
		if (vUint != NOW_SETTING && vUint != SET_SUCCESS && vUint != SET_FAULT)
			return ERR_9002;
		mib_set(MIB_CWMP_USERINFO_RESULT, &vUint);
		
#ifdef E8B_NEW_DIAGNOSE		
		switch (vUint) {
		case NOW_SETTING:
			fp = fopen(REMOTE_SETSAVE_FILE, "r");
			if (fp) {
				fclose(fp);
				unlink(REMOTE_SETSAVE_FILE);
				va_cmd("/bin/flatfsd", 1, 1, "-s");
			}
			break;
		case SET_SUCCESS:
			fp = fopen(REBOOT_DELAY_FILE, "w");
			if (fp) {
				fprintf(fp, "%s", "1");
				fclose(fp);
			}
			break;
		case SET_FAULT:
		#ifdef CONFIG_DEV_xDSL
			clearpvcfile();
		#endif
			break;
		default:
			break;
		}
#endif
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}

#endif

#ifdef _PRMT_X_CT_COM_IPTV_
int getCT_IPTV(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "IGMPEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_IPTV_IGMPENABLE, (void *)&vChar);
		if(vChar==1)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "ProxyEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_IPTV_PROXYENABLE, (void *)&vChar);
		if(vChar==1)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "SnoopingEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_IPTV_SNOOPINGENABLE, (void *)&vChar);
		if(vChar==1)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "STBNumber" )==0 )
	{
		//NOT Support YET--20120104
		*data = uintdup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_IPTV(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "IGMPEnable" )==0 )
	{
		char mode;
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_IPTV_IGMPENABLE, (void *)&vChar);
	#if 0	 //Not Support Yet, should check igmp snoop function in advance--20120104
		if (vChar) {
			/* IGMP snooping default enabled, if IGMPEnable is true */
			mib_get(MIB_MPMODE, &mode);
			mode |= MP_IGMP_MASK;
			mib_set(MIB_MPMODE, &mode);

			/* IGMP proxy default enabled, if IGMPEnable is true */
			mib_set(MIB_IGMP_PROXY, &vChar);
		}
	#endif
	}
	else if( strcmp( lastname, "ProxyEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_IPTV_PROXYENABLE, (void *)&vChar);
	}
	else if( strcmp( lastname, "SnoopingEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_IPTV_SNOOPINGENABLE, (void *)&vChar);
	}
	else{
		return ERR_9005;
	}
	
#ifdef _CWMP_APPLY_
	return 0;
#else
	return 1;
#endif
}
#endif //_PRMT_X_CT_COM_IPTV_


#if 0 //def _PRMT_X_CT_COM_MONITOR_
int getCT_MONITOR(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_MNT_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "MonitorNumber" )==0 )
	{
		mib_get( CWMP_CT_MNT_NUMBER, (void *)&vUint);
		*data = uintdup( vUint );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_MONITOR(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MNT_ENABLE, (void *)&vChar);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_MONITOR_

#ifdef _PRMT_X_CT_COM_VPDN_
int getCT_VPDN(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_VPDN_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_VPDN(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_VPDN_ENABLE, (void *)&vChar);
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //_PRMT_X_CT_COM_VPDN_


#ifdef _PRMT_X_CT_COM_MWBAND_
int getCT_MWBAND(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int vChar=0;
	int  vUint=0;
	int	vInt=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Mode" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_MODE, (void *)&vInt);
		*data = intdup( vInt );
	}else if( strcmp( lastname, "TotalTerminalNumber" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_NUMBER, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "STBRestrictEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_STB_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_STB_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "CameraRestrictEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_CMR_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "CameraNumber" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_CMR_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "ComputerRestrictEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_PC_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "ComputerNumber" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_PC_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "PhoneRestrictEnable" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_PHN_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PhoneNumber" )==0 )
	{
		mib_get( MIB_CWMP_CT_MWBAND_PHN_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCT_MWBAND(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Mode" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if( *i<0 || *i>2 ) return ERR_9007;
		mib_set(MIB_CWMP_CT_MWBAND_MODE, (void *)i);
	}else if( strcmp( lastname, "TotalTerminalNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( MIB_CWMP_CT_MWBAND_NUMBER, (void *)i);
#ifdef E8B_NEW_DIAGNOSE
		FILE *fp;
		char buf[32];

		fp = fopen(NEW_SETTING, "r+");
		if (fp) {
			fgets(buf, sizeof(buf), fp);
			if (strcmp(buf, USERLIMIT_FILE)) {
				unlink(USERLIMIT_FILE);
			} else {
				rewind(fp);
				ftruncate(fileno(fp), 0);
				fprintf(fp, "%s", USERLIMIT_FILE);
			}
			fclose(fp);
		}

		fp = fopen(USERLIMIT_FILE, "w");
		if (fp) {
			fprintf(fp, "%d", *i);
			fclose(fp);

			fp = fopen(REMOTE_SETSAVE_FILE, "r");
			if (fp) {
				fclose(fp);
				unlink(REMOTE_SETSAVE_FILE);
				va_cmd("/bin/flatfsd", 1, 1, "-s");
			}
		}
#endif
	}else if( strcmp( lastname, "STBRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_MWBAND_STB_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( MIB_CWMP_CT_MWBAND_STB_NUM, (void *)i);
	}else if( strcmp( lastname, "CameraRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_MWBAND_CMR_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "CameraNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( MIB_CWMP_CT_MWBAND_CMR_NUM, (void *)i);
	}else if( strcmp( lastname, "ComputerRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_MWBAND_PC_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "ComputerNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( MIB_CWMP_CT_MWBAND_PC_NUM, (void *)i);		
	}else if( strcmp( lastname, "PhoneRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_MWBAND_PHN_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "PhoneNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( MIB_CWMP_CT_MWBAND_PHN_NUM, (void *)i);
	}else{
		return ERR_9005;
	}

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	proc_write_for_mwband();
	return 0;
#else
	return 1;
#endif
}
#endif //_PRMT_X_CT_COM_MWBAND_



#endif /*_PRMT_SERVICES_*/
