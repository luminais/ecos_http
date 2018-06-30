#ifndef CWMPLIB_H_
#define CWMPLIB_H_


extern int gNeedSendGetRPC;
extern int gNeedSSLAuth;
extern int gSkipMReboot;
extern int gDelayStart;
extern int gNoDebugMsg;
extern int gSeflReboot;
extern int gDisConReqAuth;
extern int gBringLanMacAddrInInform;
extern int gTR069_SSLAllowSelfSignedCert;
/*star:20100105 START when there is already a session, if return 503 after receive connection request*/
extern int gReturn503;
/*star:20100105 END*/
extern int gUseTR181;

extern void cwmpinit_SendGetRPC( int flag );
extern void cwmpinit_SSLAuth( int flag );
extern void cwmpinit_SkipMReboot( int flag );
extern void cwmpinit_DelayStart( int flag );
extern void cwmpinit_NoDebugMsg( int flag );
extern void cwmpinit_DisConReqAuth( int flag );
extern void cwmpinit_OnlyDefaultWanIPinInform( int flag );
extern void cwmpinit_BringLanMacAddrInInform( int flag );
extern void cwmpinit_SslSetAllowSelfSignedCert( int flag );
extern void cwmpinit_UseTR181( int useTR181);
extern void cwmp_closeDebugMsg(void);

/*******************************************************************************/
/**** APIs for applying the new values****/
/*******************************************************************************/
#define CWMP_PRI_SH	-2	/*SUPER HIGH*/
#define CWMP_PRI_H	-1	/*HIGH*/
#define CWMP_PRI_N	 0	/*NORMAL*/
#define CWMP_PRI_L	 1	/*LOW*/
#define CWMP_PRI_SL	 2	/*SUPER LOW*/

struct CWMP_APPLY
{
	int	priority;
	int	(*cb_fun)( int action, int i, void *olddata);
	int	action;
	int	id;
	void	*olddata;
	struct CWMP_APPLY	*next;
};

extern int apply_add(
	int			priority,
	void			*cb_fun,
	int			action,
        int	 		id,
        void 			*olddata,
        int			size
);
extern int apply_destroy( void );
extern int apply_takeaction( void );
/*******************************************************************************/
/**** End APIs for applying the new values****/
/*******************************************************************************/

#endif 
