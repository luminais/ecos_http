#ifndef     __H_8021X_H__
#define		__H_8021X_H__

#include <autoconf.h>

#include <syslog.h>
//roy+++
#include <router_net.h>
//

//==============================================================================
//                                    MACROS
//==============================================================================
#define X8021_DBG(c...)	syslog(LOG_INFO|LOG_DEBUG, ##c)
#define X8021_LOG(c...)	syslog(LOG_INFO, ##c)

#define TRUE  1

typedef enum code_enum
{
	RT_CODE_RT = 1,
	RT_CODE_SU = 3,
	RT_CODE_FL
}CODE_C8021X;

typedef enum eap_type_enum
{
	EAP_USR = 1,
	EAP_PWD = 4
}EAPTYPE;

typedef struct EAPformat
{
	unsigned char Version;
	unsigned char PktType;
	unsigned short Len1;
	
	unsigned char Code;
	unsigned char Id;
	unsigned short Len2;

	unsigned char EapType;
}__attribute__((packed)) EAP;

//add for 20110917
typedef struct MD5format
{
	unsigned char value_size;
	unsigned char value[16];
	unsigned char extra_data[1];
}__attribute__((packed)) MD5_data;

#define X8021_DEVNAME		"/dev/net/eapol"

struct x8021_ifc {
	int dev_fd;
	char ifname[16];
	char user_name[64];
	char password[64];
	int mtu;

	char macaddr[6];
	char rx_buf[256];
};



#ifdef __cplusplus
extern "C" {
#endif

void X8021_mainloop(struct x8021_ifc *config);

#ifdef __cplusplus
}
#endif

#endif	//__H_8021X_H__
