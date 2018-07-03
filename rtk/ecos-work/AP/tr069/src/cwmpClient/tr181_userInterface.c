#include "tr181_userInterface.h"


/*******************************************************************************
DEVICE.UserInterface Parameters
*******************************************************************************/
struct CWMP_OP tUserInterfaceLeafOP = { getUserInterface, setUserInterface };
struct CWMP_PRMT tUserInterfaceLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"PasswordRequired",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tUserInterfaceLeafOP}, // set problem
	{"PasswordUserSelectable",	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP}, // set problem
//	{"PasswordReset",           eCWMP_tBOOLEAN, CWMP_PASSWORD|CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
	{"UpgradeAvailable",        eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP}, // set problem
	{"WarrantyDate",            eCWMP_tDATETIME,CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPName",                 eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPHelpDesk",             eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPHomePage",             eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPHelpPage",             eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPLogo",                 eCWMP_tBASE64,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPLogoSize",             eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPMailServer",           eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"ISPNewsServer",           eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"TextColor",               eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP}, // hexBinary
//	{"BackgroundColor",         eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP}, // hexBinary
//	{"ButtonColor",             eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP}, // hexBinary
//	{"ButtonTextColor",         eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP}, // hexBinary
	{"AutoUpdateServer",        eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
	{"UserUpdateServer",        eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
//	{"AvailableLanguages",      eCWMP_tSTRING,	CWMP_READ,   			&tUserInterfaceLeafOP}, // list of string
//	{"CurrentLanguage",         eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,   &tUserInterfaceLeafOP},
};

enum eUserInterfaceLeaf
{
	ePasswordRequired,
	ePasswordUserSelectable,
//	ePasswordReset,
	eUpgradeAvailable,
	eWarrantyDate,
//	eISPName,
//	eISPHelpDesk,
//	eISPHomePage,
//	eISPHelpPage,
//	eISPLogo,
//	eISPLogoSize,
//	eISPMailServer,
//	eISPNewsServer,
//	eTextColor,
//	eBackgroundColor,
//	eButtonColor,
//	eButtonTextColor,
	eAutoUpdateServer,
	eUserUpdateServer,
//	eAvailableLanguages,
//	eCurrentLanguage
};

struct CWMP_LEAF tUserInterfaceLeaf[] =
{
	{ &tUserInterfaceLeafInfo[ePasswordRequired] },
	{ &tUserInterfaceLeafInfo[ePasswordUserSelectable] },
//	{ &tUserInterfaceLeafInfo[ePasswordReset] },
	{ &tUserInterfaceLeafInfo[eUpgradeAvailable] },
	{ &tUserInterfaceLeafInfo[eWarrantyDate] },
//	{ &tUserInterfaceLeafInfo[eISPName] },
//	{ &tUserInterfaceLeafInfo[eISPHelpDesk] },
//	{ &tUserInterfaceLeafInfo[eISPHomePage] },
//	{ &tUserInterfaceLeafInfo[eISPHelpPage] },
//	{ &tUserInterfaceLeafInfo[eISPLogo] },
//	{ &tUserInterfaceLeafInfo[eISPLogoSize] },
//	{ &tUserInterfaceLeafInfo[eISPMailServer] },
//	{ &tUserInterfaceLeafInfo[eISPNewsServer] },
//	{ &tUserInterfaceLeafInfo[eTextColor] },
//	{ &tUserInterfaceLeafInfo[eBackgroundColor] },
//	{ &tUserInterfaceLeafInfo[eButtonColor] },
//	{ &tUserInterfaceLeafInfo[eButtonTextColor] },
	{ &tUserInterfaceLeafInfo[eAutoUpdateServer] },
	{ &tUserInterfaceLeafInfo[eUserUpdateServer] },
//	{ &tUserInterfaceLeafInfo[eAvailableLanguages] },
//	{ &tUserInterfaceLeafInfo[eCurrentLanguage] },
	{ NULL }
};


/*******************************************************************************
Function
*******************************************************************************/
int getUserInterface(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 1 // rewrite
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int vUint=0;
	char buff[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PasswordRequired" )==0 )
	{
		mib_get( MIB_UIF_PW_REQUIRED, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PasswordUserSelectable" )==0 )
	{
		mib_get( MIB_UIF_PW_USER_SEL, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "UpgradeAvailable" )==0 )
	{
		mib_get( MIB_UIF_UPGRADE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "WarrantyDate" )==0 )
	{
		mib_get( MIB_UIF_WARRANTYDATE, (void *)&vUint);
		*data = timedup( vUint );
	}else if( strcmp( lastname, "AutoUpdateServer" )==0 )
	{
		mib_get( MIB_UIF_AUTOUPDATESERVER, (void *)buff);
		*data = strdup( buff );
	}else if( strcmp( lastname, "UserUpdateServer" )==0 )
	{
		mib_get( MIB_UIF_USERUPDATESERVER, (void *)buff);
		*data = strdup( buff );
	}else{
		return ERR_9005;
	}
#endif
	return 0;
}

int setUserInterface(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 1 // rewrite
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PasswordRequired" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_UIF_PW_REQUIRED, (void *)&vChar);
		return 0;
	}else if( strcmp( lastname, "PasswordUserSelectable" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_UIF_PW_USER_SEL, (void *)&vChar);
		return 0;
	}else if( strcmp( lastname, "UpgradeAvailable" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_UIF_UPGRADE, (void *)&vChar);
		return 0;
	}else if( strcmp( lastname, "WarrantyDate" )==0 )
	{
		unsigned int *i = data;
		if( i==NULL ) return ERR_9007;
		mib_set(MIB_UIF_WARRANTYDATE, (void *)i);
		return 0;
	}else if( strcmp( lastname, "AutoUpdateServer" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)>=256 ) return ERR_9007;
		mib_set(MIB_UIF_AUTOUPDATESERVER, (void *)buf);		
		return 0;
	}else if( strcmp( lastname, "UserUpdateServer" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)>=256 ) return ERR_9007;
		mib_set(MIB_UIF_USERUPDATESERVER, (void *)buf);		
		return 0;
	}else{
		return ERR_9005;
	}
#endif
	return 0;
}
