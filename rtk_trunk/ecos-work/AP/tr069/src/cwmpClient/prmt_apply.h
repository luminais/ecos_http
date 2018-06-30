#ifndef _PRMT_APPLY_H_
#define _PRMT_APPLY_H_

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _CWMP_APPLY_

int apply_UserAccount( int action_type, int id, void *olddata );

// prmt_layer3fw.c
// MIB_CE_IP_ROUTE_T needs to realize! Its definition in .c isn't right!
int apply_Layer3Forwarding( int action_type, int id, void *olddata );
int apply_DefaultRoute( int action_type, int id, void *olddata );

// prmt_landevice.c
int apply_DHCP( int action_type, int id, void *olddata );
int apply_DNS( int action_type, int id, void *olddata );
int apply_LANIP( int action_type, int id, void *olddata );

// prmt_landevice_wlan.c
int apply_WLAN( int action_type, int id, void *olddata );

// prmt_wancondevice.c
// OldPort_Entity and vtlsvr_entryx needs to realize! Their definitions in .c aren't right!
int apply_PortForwarding( int action_type, int id, void *olddata );

// prmt_time.c
int apply_NTP( int action_type, int id, void *olddata );

// prmt_landevice_eth.c
int apply_ETHER( int action_type, int id, void *olddata );

#ifdef _CWMP_MAC_FILTER_
int apply_MACFILTER( int action_type, int id, void *olddata );
#endif

// prmt_ddns.c
#if defined(_PRMT_X_CT_COM_DDNS_)
int apply_DDNS( int action_type, int id, void *olddata );
#endif

// prmt_deviceinfo.c
#ifdef _PRMT_X_CT_COM_ALARM_
int apply_Alarm( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_X_TELEFONICA_ES_IGMPCONFIG_
int apply_IGMPProxy( int action_type, int id, void *olddata );
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int Apply_CaptivePortal( int action_type, int id, void *olddata );
#endif

#ifdef IP_QOS
int apply_IPQoSRule( int action_type, int id, void *olddata );
int apply_IPQoS( int action_type, int id, void *olddata );
#endif

#endif //_CWMP_APPLY_	
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_APPLY_H_*/
