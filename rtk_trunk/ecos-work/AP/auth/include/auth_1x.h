#ifndef _AUTH_1X_H
#define _AUTH_1X_H
#define RTL_1X_MAX_INTERFACE_SUPPORT 10
#define RTL_1X_INTERFACE_LEN 16

#include "1x_common.h"

typedef struct _rtl_1x_global_config_t{
	  Dot1x_Authenticator RTLAuthenticator;
	  
#ifdef RTL_WPA_CLIENT
#include "1x_supp_pae.h"

	  struct _Dot1x_Client		  RTLClient;
#endif
  
	  u_char svrip[IP_ADDRSIZE+1];
	  u_char dev_svr[LIB1X_MAXDEVLEN];
	  u_char dev_supp[LIB1X_MAXDEVLEN];
	  u_char mode[20];
	  u_short udp_svrport;
#ifdef RTL_RADIUS_2SET
	  u_char svrip2[IP_ADDRSIZE+1];
	  u_short udp_svrport2;
#endif
	  u_char oursvr_addr[ ETHER_ADDRLEN];
	  u_char oursupp_addr[ ETHER_ADDRLEN];
	  u_char svraddr[ ETHER_ADDRLEN];
	  u_char ourip[IP_ADDRSIZE+1];
  
#ifdef RTL_WPA_CLIENT
#if defined(CONFIG_RTL_802_1X_CLIENT_SUPPORT)
	  char xsup_conf[50];			  
#endif
#endif
  }rtl_1x_global_config_t;
#endif
