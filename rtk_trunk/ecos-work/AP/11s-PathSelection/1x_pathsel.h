#ifdef _RTL8187B_
#ifndef _1X_PATHSEL_H
#define _1X_PATHSEL_H

#include "pathselection.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
//#include <linux/byteorder/little_endian.h>
#else
//#include <linux/byteorder/big_endian.h>
#endif

typedef enum{

		// add by Jason 2007.01.04:pass 802.11s pathselection packet by event queue
		//modify by shlu 2007.01.17
		DOT11_EVENT_PATHSEL_GEN_RREQ = 59, 
		DOT11_EVENT_PATHSEL_GEN_RERR = 60,
		DOT11_EVENT_PATHSEL_RECV_RREQ = 61,                    
		DOT11_EVENT_PATHSEL_RECV_RREP = 62,                   
		DOT11_EVENT_PATHSEL_RECV_RERR = 63,
		DOT11_EVENT_PATHSEL_RECV_PANN = 65,
		DOT11_EVENT_PATHSEL_RECV_RANN = 66,
		DOT11_EVENT_PATHSEL_ENABLE_PORTAL = 67
   
} DOT11_EVENT;


#endif // _1X_PATHSEL_H
#endif //_RTL8187B_ 
