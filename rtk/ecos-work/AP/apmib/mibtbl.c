/*
 *      MIB table declaration
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: mibtbl.c,v 1.30 2009/09/03 06:58:11 keith_huang Exp $
 *
 */

/* Include files */
#include "apmib.h"
#include "mibtbl.h"

/* Global variable definitions */

/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "DEF_" in ahead of mib name when access
 * default setting MIB.
 */

mib_table_entry_T mib_dhcpRsvdIp_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_DHCPRSVDIP_IMPORT
#include "mibdef.h"
#undef MIB_DHCPRSVDIP_IMPORT

#undef MIBDEF
{0}
};

#ifdef WLAN_PROFILE
mib_table_entry_T mib_wlan_profile_tbl1[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_WLAN_PROFILE_IMPORT
#include "mibdef.h"
#undef MIB_WLAN_PROFILE_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_wlan_profile_tbl2[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_WLAN_PROFILE_IMPORT
#include "mibdef.h"
#undef MIB_WLAN_PROFILE_IMPORT

#undef MIBDEF
{0}
};

#endif

mib_table_entry_T mib_schedule_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_SCHEDULE_IMPORT
#include "mibdef.h"
#undef MIB_SCHEDULE_IMPORT

#undef MIBDEF
{0}
};

#ifdef KLD_ENABLED
mib_table_entry_T mib_virtualServ_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_VIRTUALSERV_IMPORT
#include "mibdef.h"
#undef MIB_VIRTUALSERV_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_fwmacfilter_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_FWMACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_FWMACFILTER_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_domainFilter_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_DOMAINFILTER_IMPORT
#include "mibdef.h"
#undef MIB_DOMAINFILTER_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_firewallRule_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_FIREWALLRULE_IMPORT
#include "mibdef.h"
#undef MIB_FIREWALLRULE_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_lockClient_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_LOCK_CLIENT_IMPORT
#include "mibdef.h"
#undef MIB_LOCK_CLIENT_IMPORT

#undef MIBDEF
{0}
};

#endif

#if	1//def HOME_GATEWAY
mib_table_entry_T mib_portfw_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_PORTFW_IMPORT
#include "mibdef.h"
#undef MIB_PORTFW_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_ipfilter_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_IPFILTER_IMPORT
#include "mibdef.h"
#undef MIB_IPFILTER_IMPORT

#undef MIBDEF
{0}
};
	
mib_table_entry_T mib_portfilter_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_PORTFILTER_IMPORT
#include "mibdef.h"
#undef MIB_PORTFILTER_IMPORT

#undef MIBDEF
{0}
};
	
mib_table_entry_T mib_macfilter_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_MACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_MACFILTER_IMPORT

#undef MIBDEF
{0}
};
	
mib_table_entry_T mib_triggerport_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_TRIGGERPORT_IMPORT
#include "mibdef.h"
#undef MIB_TRIGGERPORT_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_urlfilter_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_URLFILTER_IMPORT
#include "mibdef.h"
#undef MIB_URLFILTER_IMPORT

#undef MIBDEF
{0}
};

#if 0
mib_table_entry_T mib_vlanconfig_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_VLAN_CONFIG_IMPORT
#include "mibdef.h"
#undef MIB_VLAN_CONFIG_IMPORT

#undef MIBDEF
{0}
};
#endif

#ifdef ROUTE_SUPPORT
mib_table_entry_T mib_staticroute_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_STATICROUTE_IMPORT
#include "mibdef.h"
#undef MIB_STATICROUTE_IMPORT

#undef MIBDEF
{0}
};	
#endif


#ifdef VPN_SUPPORT
mib_table_entry_T mib_ipsectunnel_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_IPSECTUNNEL_IMPORT
#include "mibdef.h"
#undef MIB_IPSECTUNNEL_IMPORT

#undef MIBDEF
{0}
};
#endif

#endif // #ifdef HOME_GATEWAY

#ifdef TLS_CLIENT
mib_table_entry_T mib_certroot_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_CERTROOT_IMPORT
#include "mibdef.h"
#undef MIB_CERTROOT_IMPORT

#undef MIBDEF
{0}
};	

mib_table_entry_T mib_certuser_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_CERTUSER_IMPORT
#include "mibdef.h"
#undef MIB_CERTUSER_IMPORT
{0}
};		
#endif

#if defined(GW_QOS_ENGINE)
mib_table_entry_T mib_qos_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_QOS_IMPORT
#include "mibdef.h"
#undef MIB_QOS_IMPORT
{0}
};	
#endif

#if defined(QOS_BY_BANDWIDTH)
mib_table_entry_T mib_qos_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_IPQOS_IMPORT
#include "mibdef.h"
#undef MIB_IPQOS_IMPORT
{0}
};	
#endif

#ifdef CONFIG_RTK_MESH //Keith remove
//new feature:Mesh enable/disable

//#ifdef	_MESH_ACL_ENABLE_ Keith remove
mib_table_entry_T mib_mech_acl_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_MESH_MACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_MESH_MACFILTER_IMPORT
{0}
};	
#endif //Keith remove
#ifdef CONFIG_RTL_VLAN_SUPPORT
mib_table_entry_T mib_vlan_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#define MIB_VLAN_IMPORT
#include "mibdef.h"
#undef MIB_VLAN_IMPORT
{0}
};
mib_table_entry_T mib_netiface_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#define MIB_NETIFACE_IMPORT
#include "mibdef.h"
#undef MIB_NETIFACE_IMPORT
{0}
};
mib_table_entry_T mib_vlan_netif_bind_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#define MIB_VLAN_NETIF_BIND_IMPORT
#include "mibdef.h"
#undef MIB_VLAN_NETIF_BIND_IMPORT
{0}
};

#endif

#if defined(HAVE_TR069)
#if defined(WLAN_SUPPORT)
mib_table_entry_T mib_cwmp_wlanconf_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_CWMP_WLANCONF_IMPORT
#include "mibdef.h"
#undef MIB_CWMP_WLANCONF_IMPORT

#undef MIBDEF
{0}
};
#endif //#if defined(WLAN_SUPPORT)
#endif

mib_table_entry_T mib_root_table[]={
{MIB_ROOT, "MIB_ROOT",		TABLE_LIST_T, 	0, _UNIT_SIZE(APMIB_T), _UNIT_SIZE(APMIB_T), _UNIT_SIZE(APMIB_T), 0, mib_table},
{0}
};


mib_table_entry_T mib_table[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_IMPORT
#include "mibdef.h"
#undef MIB_IMPORT

#undef MIBDEF
{0}
};


mib_table_entry_T wlan_acl_addr_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_WLAN_MACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_WLAN_MACFILTER_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T wlan_wds_tbl[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_WDS_IMPORT
#include "mibdef.h"
#undef MIB_WDS_IMPORT

#undef MIBDEF
{0}
};

/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "WLANx_" in ahead of mib name.
 * When access default setting, it needs appened a keyword "DEF_" in front of
 * "WLANx_" keyword.
 */

mib_table_entry_T mib_wlan_table[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBWLANID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},

#define MIB_CONFIG_WLAN_SETTING_IMPORT
#include "mibdef.h"
#undef MIB_CONFIG_WLAN_SETTING_IMPORT

#undef MIBDEF

{0}
};

mib_table_entry_T hwmib_root_table[]={
{_MIBID_NAME(HW_ROOT),		TABLE_LIST_T, 	0, _UNIT_SIZE(HW_SETTING_T), _UNIT_SIZE(HW_SETTING_T), _UNIT_SIZE(HW_SETTING_T), 0, hwmib_table},
{0}
};

/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "HW_" in ahead of mib name.
 */

mib_table_entry_T hwmib_table[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBHWID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#define MIB_HW_IMPORT
#include "mibdef.h"
#undef MIB_HW_IMPORT

#undef MIBDEF
{0}
};


/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "HW_WLANx_" in ahead of mib name.
 */

mib_table_entry_T hwmib_wlan_table[]={
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBHWID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#define MIB_HW_WLAN_IMPORT
#include "mibdef.h"
#undef MIB_HW_WLAN_IMPORT

#undef MIBDEF
{0}
};

#ifdef MIB_TLV

mib_table_entry_T* mib_get_table(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return hwmib_root_table;
		case DEFAULT_SETTING:
		case CURRENT_SETTING:
			return mib_root_table;
		default:
			return 0;
	}
	
}
#endif
