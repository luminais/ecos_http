/* (c) 2006 Thomas Bernard
 * http://miniupnp.free.fr/  */
/* $Id: miniupnpdpath.h,v 1.1.1.1 2007-08-06 10:04:43 root Exp $
 * Paths and other URLs in the miniupnpd http server */
#ifndef __MINIUPNPDPATH_H__
#define __MINIUPNPDPATH_H__

#define ROOTDESC_PATH "/picsdesc.xml"

#define DUMMY_PATH			"/dummy.xml"

#define WANCFG_PATH			"/wancfg.xml"
#define WANCFG_CONTROLURL	"/upnp/control/WANCommonInterfaceConfig"
#define WANCFG_EVENTURL		"/upnp/event/WANCommonInterfaceConfig"

#define WANIPC_PATH			"/wanipcn.xml"
#define WANIPC_CONTROLURL	"/upnp/control/WANIPConnection"
#define WANIPC_EVENTURL		"/upnp/event/WANIPConnection"

#define WANINFO_PATH		"/WANInfo.xml"
#define WANINFO_CONTROLURL	"/upnp/control/OSInfo1"
#define WANINFO_EVENTURL	"/upnp/event/OSInfo1"
#endif

