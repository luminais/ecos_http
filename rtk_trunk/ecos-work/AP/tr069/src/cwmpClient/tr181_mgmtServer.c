#include "tr181_mgmtServer.h"
#include "tr181_mgableDev.h"
#include "cwmp_notify.h"
#ifdef TR069_ANNEX_G
#include "udp.h"
#include "stun.h"
#endif

#if 1 // rewrite
#define cwmpSettingChange(mib) do {} while (0)
#endif

#define CONFIG_SET(key, val) if ( mib_set(key, val)==0)  return ERR_9002
#define CONFIG_GET(key, ret) if ( mib_get(key, ret)==0)  return ERR_9002

#define CHECK_PARAM_NUM(input, min, max) if ( (input < min) || (input > max) ) return ERR_9007;
#define CHECK_PARAM_STR(str, min, max)  do { \
	int tmp; \
	if (!str) return ERR_9007; \
	tmp=strlen(str); \
	if ((tmp < min) || (tmp > max)) return ERR_9007; \
}	while (0)

#ifdef TR069_ANNEX_G
#define	STUN_BINDING_REQ				1
#define STUN_BINDING_MAINTAIN			2
#define STUN_BINDING_TIMEOUT_DISCOVERY	3
#define STUN_BINDING_TIMEOUT_MONITORING	4

#define CWMP_STUN_CNONCE_LEN			256
#define CWMP_STUN_SIGNATURE_LEN			40
#define CWMP_STUN_SIGNATURE_STRING_LEN	2048

static int stunThread = 0;
static int stunStop = 0;
static int stunState = STUN_BINDING_REQ;

static char udpConnReqAddr[CWMP_UDP_CONN_REQ_ADDR_LEN+1];
static char stunServerAddr[CWMP_STUN_SERVER_ADDR_LEN+1];
static unsigned int stunServerPort;
static char stunUsername[CWMP_STUN_USERNAME_LEN+1];
static char stunPassword[CWMP_STUN_PASSWORD_LEN+1];
static int stunMaxPeriod;
static unsigned int stunMinPeriod;
static unsigned int natDetected;

static char acsUrl[CWMP_ACS_URL_LEN+1];

static time_t g_ts = 0;
static unsigned int g_id = 0;
#endif

/*******************************************************************************
DEVICE.ManagementServer Parameters
*******************************************************************************/
struct CWMP_OP tMgmtServerLeafOP = { getMgmtServer,setMgmtServer };
struct CWMP_PRMT tMgmtServerLeafInfo[] =
{
	/*(name,				type,		flag,			op)*/
	{"EnableCWMP",                        eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP}, // factory default true
	{"URL",                               eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"Username",                          eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"Password",                          eCWMP_tSTRING,	CWMP_PASSWORD|CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"PeriodicInformEnable",              eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"PeriodicInformInterval",            eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"PeriodicInformTime",                eCWMP_tDATETIME,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"ParameterKey",                      eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tMgmtServerLeafOP},
	{"ConnectionRequestURL",              eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,	&tMgmtServerLeafOP},
	{"ConnectionRequestUsername",         eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"ConnectionRequestPassword",         eCWMP_tSTRING,	CWMP_PASSWORD|CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"UpgradesManaged",                   eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP}, // set problem
	{"KickURL",                           eCWMP_tSTRING,	CWMP_READ,	&tMgmtServerLeafOP},
	{"DownloadProgressURL",               eCWMP_tSTRING,	CWMP_READ,	&tMgmtServerLeafOP},
//	{"DefaultActiveNotificationThrottle", eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"CWMPRetryMinimumWaitInterval",      eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"CWMPRetryIntervalMultiplier",       eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
#ifdef TR069_ANNEX_G
	{"UDPConnectionRequestAddress",       eCWMP_tSTRING,	CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNEnable",                        eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNServerAddress",                 eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNServerPort",                    eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNUsername",                      eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNPassword",                      eCWMP_tSTRING,	CWMP_PASSWORD|CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNMaximumKeepAlivePeriod",        eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"STUNMinimumKeepAlivePeriod",        eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tMgmtServerLeafOP},
	{"NATDetected",                       eCWMP_tBOOLEAN,	CWMP_READ,	&tMgmtServerLeafOP},
#endif
	{"ManageableDeviceNumberOfEntries",   eCWMP_tUINT,		CWMP_READ,	&tMgmtServerLeafOP},
};

enum eMgmtServerLeaf
{
	eMS_EnableCWMP,
	eMS_URL,
	eMS_Username,
	eMS_Password,
	eMS_PeriodicInformEnable,
	eMS_PeriodicInformInterval,
	eMS_PeriodicInformTime,
	eMS_ParameterKey,
	eMS_ConnectionRequestURL,
	eMS_ConnectionRequestUsername,
	eMS_ConnectionRequestPassword,
	eMS_UpgradesManaged,
	eMS_KickURL,
	eMS_DownloadProgressURL,
//	eMS_DefaultActiveNotificationThrottle,
	eMS_CWMPRetryMinimumWaitInterval,
	eMS_CWMPRetryIntervalMultiplier,
#ifdef TR069_ANNEX_G
	eMS_UDPConnectionRequestAddress,
	eMS_STUNEnable,
	eMS_STUNServerAddress,
	eMS_STUNServerPort,
	eMS_STUNUsername,
	eMS_STUNPassword,
	eMS_STUNMaximumKeepAlivePeriod,
	eMS_STUNMinimumKeepAlivePeriod,
	eMS_NATDetected,
#endif
	eMS_ManageableDeviceNumberOfEntries
};

struct CWMP_LEAF tMgmtServerLeaf[] =
{
	{ &tMgmtServerLeafInfo[eMS_EnableCWMP] },
	{ &tMgmtServerLeafInfo[eMS_URL] },
	{ &tMgmtServerLeafInfo[eMS_Username] },
	{ &tMgmtServerLeafInfo[eMS_Password] },
	{ &tMgmtServerLeafInfo[eMS_PeriodicInformEnable] },
	{ &tMgmtServerLeafInfo[eMS_PeriodicInformInterval] },
	{ &tMgmtServerLeafInfo[eMS_PeriodicInformTime] },
	{ &tMgmtServerLeafInfo[eMS_ParameterKey] },
	{ &tMgmtServerLeafInfo[eMS_ConnectionRequestURL] },
	{ &tMgmtServerLeafInfo[eMS_ConnectionRequestUsername] },
	{ &tMgmtServerLeafInfo[eMS_ConnectionRequestPassword] },
	{ &tMgmtServerLeafInfo[eMS_UpgradesManaged] },
	{ &tMgmtServerLeafInfo[eMS_KickURL] },
	{ &tMgmtServerLeafInfo[eMS_DownloadProgressURL] },
//	{ &tMgmtServerLeafInfo[eMS_DefaultActiveNotificationThrottle] },
	{ &tMgmtServerLeafInfo[eMS_CWMPRetryMinimumWaitInterval] },
	{ &tMgmtServerLeafInfo[eMS_CWMPRetryIntervalMultiplier] },
#ifdef TR069_ANNEX_G
	{ &tMgmtServerLeafInfo[eMS_UDPConnectionRequestAddress] },
	{ &tMgmtServerLeafInfo[eMS_STUNEnable] },
	{ &tMgmtServerLeafInfo[eMS_STUNServerAddress] },
	{ &tMgmtServerLeafInfo[eMS_STUNServerPort] },
	{ &tMgmtServerLeafInfo[eMS_STUNUsername] },
	{ &tMgmtServerLeafInfo[eMS_STUNPassword] },
	{ &tMgmtServerLeafInfo[eMS_STUNMaximumKeepAlivePeriod] },
	{ &tMgmtServerLeafInfo[eMS_STUNMinimumKeepAlivePeriod] },
	{ &tMgmtServerLeafInfo[eMS_NATDetected] },
#endif
	{ &tMgmtServerLeafInfo[eMS_ManageableDeviceNumberOfEntries] },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
#ifdef TR069_ANNEX_G
static void procUdpConnReq(char *buf, unsigned int bufLen)
{
	int i;
	char *p;
	time_t ts = 0; // timestamp
	unsigned int id = 0;
	char un[CWMP_STUN_USERNAME_LEN+1]; // username
	char cn[CWMP_STUN_CNONCE_LEN+1]; // Cnonce
	char sig[CWMP_STUN_SIGNATURE_LEN+1]; // Signature

	char sigStr[CWMP_STUN_SIGNATURE_STRING_LEN];
	char hash[20];
	char hashStr[CWMP_STUN_SIGNATURE_LEN+1];
	
	char connReqUN[CWMP_CONREQ_USERNAME_LEN+1];
	char connReqPW[CWMP_CONREQ_PASSWD_LEN+1];

	tr181_printf("UDP Conn Req: %s", buf);
	tr181_printf("Len (%d)", bufLen);

#if 0  // validation and authentication

	if (buf == NULL)
	{
		tr181_printf("buf NULL error!");
		return;
	}

	if (strncmp(buf, "GET", 3) != 0)
	{
		tr181_printf("not a GET msg!");
		return;
	}

	if (strstr(buf, "HTTP/1.1") == NULL)
	{
		tr181_printf("not a HTTP/1.1 msg!");
		return;
	}

	p = strtok(buf, "?=& ");

	while (p != NULL)
	{
		tr181_printf("strtok (%s)", p);

		if (strcmp(p, "ts") == 0)
		{
			p = strtok(NULL, "?=& ");

			if (p != NULL)
			{
				ts = (time_t)atoi(p);
				tr181_printf("ts (%d)", ts);
			}
		}
		else if (strcmp(p, "id") == 0)
		{
			p = strtok(NULL, "?=& ");

			if (p != NULL)
			{
				id = atol(p);
				tr181_printf("id (%ld)", id);
			}
		}
		else if (strcmp(p, "un") == 0)
		{
			p = strtok(NULL, "?=& ");

			if (p != NULL)
			{
				strcpy(un, p);
				tr181_printf("un (%s)", un);
			}
		}
		else if (strcmp(p, "cn") == 0)
		{
			p = strtok(NULL, "?=& ");

			if (p != NULL)
			{
				strcpy(cn, p);
				tr181_printf("cn (%s)", cn);
			}
		}
		else if (strcmp(p, "sig") == 0)
		{
			p = strtok(NULL, "?=& ");

			if (p != NULL)
			{
				strcpy(sig, p);
				tr181_printf("sig (%s)", sig);
			}
		}

		p = strtok(NULL, "?=& ");
	}

	if (ts <= g_ts)
	{
		tr181_printf("ts(%d) <= g_ts(%d) error!", ts, g_ts);
		return;
	}
	else
	{
		g_ts = ts;
	}

	if (id == g_id)
	{
		tr181_printf("id(%ld) == g_id(%ld) error!", id, g_id);
		return;
	}
	else
	{
		g_id = id;
	}

	mib_get(MIB_CWMP_CONREQ_USERNAME, connReqUN);

	if (strcmp(un, connReqUN) != 0)
	{
		tr181_printf("username (un) mismatch error!");
		return;
	}

	mib_get(MIB_CWMP_CONREQ_PASSWORD, connReqPW);

	sprintf(sigStr, "%d%ld%s%s", ts, id, un, cn);

	tr181_printf("sigStr (%s)", sigStr);

    /* use sigStr with connReqPW to generate Hmac for comparison. */
	computeHmac(hash, sigStr, strlen(sigStr), connReqPW, strlen(connReqPW));

#endif

	cwmpSendUdpConnReq();
}

static void stun_getHost(char *host, char *url)
{ 
	const char *s;
	int i, n;

	if (!url || !*url)
		return;

	s = strchr(url, ':');
	if (s && s[1] == '/' && s[2] == '/')
		s += 3;
	else
		s = url;

	tr181_printf("s (%s)", s);

	n = strlen(s);

	for (i = 0; i < n; i++)
	{ 
		host[i] = s[i];
		if (s[i] == '/' || s[i] == ':')
			break; 
	}

	host[i] = '\0';
	return;
}

static int stun_getHostByName(char *addr, struct in_addr *inaddr)
{ 
	in_addr_t iadd = -1;
	struct hostent hostent, *host = &hostent;

	iadd = inet_addr(addr);

	if (iadd != -1)
	{ 
		memcpy(inaddr, &iadd, sizeof(iadd));
		return 0;
	}

	host = gethostbyname(addr);

	if (!host)
	{ 
		tr181_printf("Host name not found");
		return -1;
	}

	memcpy(inaddr, host->h_addr, host->h_length);

	return 0;
}

static void *stun_thread(void *arg)
{
	StunAddress4 stunServer, stunMappedAddr;
	StunMessage stunMsg;
	Socket priFd, secFd, s;
	struct in_addr in;
	char checkAddr[CWMP_UDP_CONN_REQ_ADDR_LEN+1];
	char host[CWMP_ACS_URL_LEN+1];
	unsigned int checkNat;
	unsigned int stunPeriod;

	tr181_printf("stun started!!!");

	stunState = STUN_BINDING_REQ;

	mib_get(MIB_CWMP_STUN_SERVER_ADDR, stunServerAddr);
	mib_get(MIB_CWMP_STUN_SERVER_PORT, &stunServerPort);
	mib_get(MIB_CWMP_STUN_USERNAME, stunUsername);
	mib_get(MIB_CWMP_STUN_PASSWORD, stunPassword);
	mib_get(MIB_CWMP_STUN_MAX_KEEP_ALIVE_PERIOD, &stunMaxPeriod);
	mib_get(MIB_CWMP_STUN_MIN_KEEP_ALIVE_PERIOD, &stunMinPeriod);

	mib_get(MIB_CWMP_ACS_URL, acsUrl);

	tr181_printf("stunServerAddr (%s)", stunServerAddr);
	tr181_printf("stunServerPort (%d)", stunServerPort);
	tr181_printf("stunUsername (%s)", stunUsername);
	tr181_printf("stunPassword (%s)", stunPassword);
	tr181_printf("stunMaxPeriod (%d)", stunMaxPeriod);
	tr181_printf("stunMinPeriod (%d)", stunMinPeriod);
	tr181_printf("acsUrl (%s)", acsUrl);

	memset(&stunServer, 0, sizeof(StunAddress4));

	if (strlen(stunServerAddr) == 0)
	{
		stun_getHost(host, acsUrl);
	}
	else
	{
		stun_getHost(host, stunServerAddr);
	}

	tr181_printf("host %s", host);
		
	if (stun_getHostByName(host, &in) == -1)
		goto stun_err;
	
	stunServer.addr = (UInt32)in.s_addr;

	if (stunServerPort == 0)
	{
		stunServer.port = STUN_PORT; // use default stun port number
	}
	else
	{
		stunServer.port = stunServerPort;
	}

	in.s_addr = htonl(stunServer.addr);
	tr181_printf("stun server: %s:%d", inet_ntoa(in), stunServer.port);
	
	while (!stunStop)
	{
		tr181_printf("stunState (%d)", stunState);

		memset(&stunMsg, 0, sizeof(StunMessage));
		
		switch (stunState)
		{
			case STUN_BINDING_REQ:
				tr181_trace();

				priFd = openPort(0, 0);
				
				stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 0, 0, NULL, stunUsername, NULL, procUdpConnReq);

				if (stunMsg.hasMappedAddress)
				{
					s = openPort(11000, stunMsg.mappedAddress.ipv4.addr);

					if (s != INVALID_SOCKET) // not behind NAT
					{
						close(s);
						natDetected = 0;
					}
					else
					{
						natDetected = 1;
					}

					stunMappedAddr.addr = stunMsg.mappedAddress.ipv4.addr;
					stunMappedAddr.port = stunMsg.mappedAddress.ipv4.port;

					in.s_addr = htonl(stunMsg.mappedAddress.ipv4.addr);
					sprintf(udpConnReqAddr, "%s:%d", inet_ntoa(in),
						stunMsg.mappedAddress.ipv4.port);

					mib_set(MIB_CWMP_UDP_CONN_REQ_ADDR, udpConnReqAddr);
					mib_set(MIB_CWMP_NAT_DETECTED, &natDetected);

					tr181_printf("isNAT: %d, UDPConnReqAddr: %s", natDetected, udpConnReqAddr);

					if (natDetected)
					{
						/* Binding Req with BindingChange attr. */
						stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 1, 0, NULL, stunUsername, NULL, procUdpConnReq);

#ifdef _CWMP_WITH_SSL_
						if (stunMsg.hasErrorCode && stunMsg.errorCode.errorClass == 4 &&
							stunMsg.errorCode.number == 1) // Unauthorized
						{
							stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 1, 0, NULL, stunUsername, stunPassword, procUdpConnReq);

							if (stunMsg.hasErrorCode)
							{
								tr181_printf("Unauthorized Username/Password");
								close(priFd);
								break;
							}
						}
#endif

						if (stunMaxPeriod == stunMinPeriod)
						{
							stunPeriod = stunMinPeriod;
							stunState = STUN_BINDING_MAINTAIN;
						}
						else
						{
							stunPeriod = (stunMinPeriod + stunMaxPeriod) / 2;
							stunState = STUN_BINDING_TIMEOUT_DISCOVERY;
						}

						tr181_printf("stunState (%d) stunPeriod (%d) "
							"stunMinPeriod (%d) stunMaxPeriod (%d)",
							stunState, stunPeriod, stunMinPeriod, stunMaxPeriod);
					}
					else
					{
						tr181_trace();
						close(priFd);
						sleep(stunMaxPeriod);
					}
				}
				else
				{
					tr181_trace();
					close(priFd);
					sleep(stunMaxPeriod);
				}
				break;

			case STUN_BINDING_MAINTAIN:
				sleep(stunPeriod);
				
				/* jump to STUN_BINDING_REQ if local IP changed */
				
				stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 0, 0, NULL, stunUsername, NULL, procUdpConnReq);

				if (stunMsg.hasMappedAddress)
				{
					s = openPort(11000, stunMsg.mappedAddress.ipv4.addr);

					if (s != INVALID_SOCKET) // not behind NAT
					{
						close(s);
						checkNat = 0;
					}
					else
					{
						checkNat = 1;
					}

					in.s_addr = htonl(stunMsg.mappedAddress.ipv4.addr);
					sprintf(checkAddr, "%s:%d", inet_ntoa(in),
						stunMsg.mappedAddress.ipv4.port);

					tr181_printf("isNAT: %d, UDPConnReqAddr: %s", checkNat, checkAddr);

					if (checkNat != natDetected)
					{
						tr181_printf("natDetected changed!!!");
						mib_set(MIB_CWMP_NAT_DETECTED, &checkNat);
						natDetected = checkNat;
					}

					if (strcmp(checkAddr, udpConnReqAddr) != 0)
					{
						tr181_printf("UDPConnReqAddr changed!!!");
						mib_set(MIB_CWMP_UDP_CONN_REQ_ADDR, checkAddr);
						strcpy(udpConnReqAddr, checkAddr);

						/* do sometheing according to G.2.1.3 */
						if (natDetected)
						{
							/* Binding Req with BindingChange attr. */
							stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 1, 0, NULL, stunUsername, NULL, procUdpConnReq);

#ifdef _CWMP_WITH_SSL_
							if (stunMsg.hasErrorCode && stunMsg.errorCode.errorClass == 4 &&
								stunMsg.errorCode.number == 1) // Unauthorized
							{
								stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 1, 0, NULL, stunUsername, stunPassword, procUdpConnReq);

								if (stunMsg.hasErrorCode)
								{
									tr181_printf("Unauthorized Username/Password");
									stunState = STUN_BINDING_REQ;
									close(priFd);
									break;
								}
							}
#endif
						}
					}
				}
				break;

			case STUN_BINDING_TIMEOUT_DISCOVERY:
				sleep(stunPeriod);

				secFd = openPort(stunMappedAddr.port+1, 0);

				stunTest_tr111_send(secFd, &stunServer, 1, 1, 0, 0, 1, &stunMappedAddr);

				stunTest_tr111_recv(priFd, 1, &stunMsg);

				if (stunMsg.hasMappedAddress) // not timeout yet
				{
					tr181_printf("not timeout yet, stunPeriod (%d)", stunPeriod);
					stunPeriod *= 2;
					tr181_printf("double stunPeriod (%d)", stunPeriod);
				}
				else // already timeout
				{
					tr181_printf("already timeout, stunPeriod (%d)", stunPeriod);
					stunPeriod /= 2;
					tr181_printf("stunPeriod / 2 = (%d)", stunPeriod);
					stunState = STUN_BINDING_TIMEOUT_MONITORING;
				}

				close(secFd);
				break;

			case STUN_BINDING_TIMEOUT_MONITORING:
				stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 0, 0, NULL, stunUsername, NULL, procUdpConnReq);

				if (stunMsg.hasMappedAddress)
				{
					if (s != INVALID_SOCKET) // not behind NAT
					{
						close(s);
						checkNat = 0;
					}
					else
					{
						checkNat = 1;
					}

					stunMappedAddr.addr = stunMsg.mappedAddress.ipv4.addr;
					stunMappedAddr.port = stunMsg.mappedAddress.ipv4.port;

					in.s_addr = htonl(stunMsg.mappedAddress.ipv4.addr);
					sprintf(checkAddr, "%s:%d", inet_ntoa(in),
						stunMsg.mappedAddress.ipv4.port);

					tr181_printf("isNAT: %d, UDPConnReqAddr: %s", checkNat, checkAddr);

					if (checkNat != natDetected)
					{
						tr181_printf("natDetected changed!!!");
						mib_set(MIB_CWMP_NAT_DETECTED, &checkNat);
						natDetected = checkNat;
					}

					if (strcmp(checkAddr, udpConnReqAddr) != 0)
					{
						tr181_printf("UDPConnReqAddr changed!!!");
						mib_set(MIB_CWMP_UDP_CONN_REQ_ADDR, checkAddr);
						strcpy(udpConnReqAddr, checkAddr);

						/* do sometheing according to G.2.1.3 */
						if (natDetected)
						{
							/* Binding Req with BindingChange attr. */
							stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 1, 0, NULL, stunUsername, NULL, procUdpConnReq);

#ifdef _CWMP_WITH_SSL_
							if (stunMsg.hasErrorCode && stunMsg.errorCode.errorClass == 4 &&
								stunMsg.errorCode.number == 1) // Unauthorized
							{
								stunTest_tr111(priFd, &stunServer, 1, 1, &stunMsg, 1, 1, 0, NULL, stunUsername, stunPassword, procUdpConnReq);

								if (stunMsg.hasErrorCode)
								{
									tr181_printf("Unauthorized Username/Password (%d-%d)",
										         stunMsg.errorCode.errorClass,
										         stunMsg.errorCode.number);
									
									stunState = STUN_BINDING_REQ;
									close(priFd);
									break;
								}
							}
#endif
						}
					}


					if (!natDetected)
					{
						tr181_trace();
						sleep(stunPeriod);
						break;
					}
				}
				else
				{
					tr181_trace();
					sleep(stunPeriod);
					break;
				}

				tr181_printf("stunPeriod (%d)", stunPeriod);

				sleep(stunPeriod);

				memset(&stunMsg, 0, sizeof(StunMessage));

				secFd = openPort(stunMappedAddr.port+1, 0);

				stunTest_tr111_send(secFd, &stunServer, 1, 1, 0, 0, 1, &stunMappedAddr);

				stunTest_tr111_recv(priFd, 1, &stunMsg);

				close(secFd);

				if (stunMsg.hasMappedAddress) // not timeout yet
				{
					tr181_printf("not timeout yet");
				}
				else // already timeout
				{
					tr181_printf("already timeout");

					close(priFd);

					stunState = STUN_BINDING_REQ;
				}
				break;

			default:
				break;
		}
	}

stun_err:
	
	close(priFd);
	close(secFd);
	close(s);

	tr181_printf("stun stopped!!!");

	stunThread = 0;
}

void cwmpStartStun()
{
	pthread_t stun_pid;
	
	tr181_trace();
	
	if (stunThread)
	{
		tr181_printf("stun already in progress!!!");
		return;
	}

	stunThread = 1;
	stunStop = 0;
	if( pthread_create( &stun_pid, NULL, stun_thread, 0 ) != 0 )
	{
		tr181_printf("stun thread create fail!!!");
		stunThread = 0;
		return;
	}
	
	pthread_detach(stun_pid);
}

void cwmpStopStun()
{
	tr181_trace();
	
	stunStop = 1;
}
#endif

int getMgmtServer(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned char buf[256+1]={0};
	unsigned char ch=0;
	unsigned int  in=0;
	int           in1=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	notify_set_attributes( "Device.GatewayInfo.ManufacturerOUI", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );
	notify_set_attributes( "Device.GatewayInfo.ProductClass", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );
	notify_set_attributes( "Device.GatewayInfo.SerialNumber", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );

	*type = entity->info->type;
	*data = NULL;
	switch(getIndexOf(tMgmtServerLeaf, entity->info->name)) {
	case eMS_URL:
		CONFIG_GET(MIB_CWMP_ACS_URL, buf);
		*data = strdup( buf );
		break;
	case eMS_Username:
		CONFIG_GET(MIB_CWMP_ACS_USERNAME, buf);
		*data = strdup(buf);
		break;
	case eMS_Password:
#if DEBUG
		CONFIG_GET(MIB_CWMP_ACS_PASSWORD, buf);
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
		break;
	case eMS_PeriodicInformEnable:
		CONFIG_GET(MIB_CWMP_INFORM_ENABLE, &in);
		*data = booldup(in);
		break;	
	case eMS_PeriodicInformInterval:
		CONFIG_GET(MIB_CWMP_INFORM_INTERVAL, &in);
		*data = uintdup(in);
		break;	
	case eMS_PeriodicInformTime:
		CONFIG_GET(MIB_CWMP_INFORM_TIME, &in);
		*data = timedup(in);
		break;	
	case eMS_ParameterKey: // ParameterKey
		{
			unsigned char gParameterKey[32+1];
			CONFIG_GET(MIB_CWMP_PARAMETERKEY,gParameterKey);
			*data = strdup(gParameterKey);
		break;	
		}
	case eMS_ConnectionRequestURL:
		if (MgmtSrvGetConReqURL(buf, 256))			
			*data = strdup(buf);
		else
			*data = strdup("");
		break;	
	case eMS_ConnectionRequestUsername:
		CONFIG_GET(MIB_CWMP_CONREQ_USERNAME, buf);
		*data = strdup(buf);
		break;	
	case eMS_ConnectionRequestPassword:
	#if DEBUG
		CONFIG_GET(MIB_CWMP_CONREQ_PASSWORD, buf);
		*data = strdup(buf);
	#else
		*data = strdup("");
	#endif
		break;	
	case eMS_UpgradesManaged:
		CONFIG_GET(MIB_CWMP_ACS_UPGRADESMANAGED, &ch);
		*data = booldup(ch);
		break;
	case eMS_KickURL:
		CONFIG_GET(MIB_CWMP_ACS_KICKURL, buf);
		*data = strdup(buf);
		break;	
	case eMS_DownloadProgressURL:
		CONFIG_GET(MIB_CWMP_ACS_DOWNLOADURL, buf);
		*data = strdup(buf);
		break;				

#if 0 // rewrite
	case eMS_CWMPRetryMinimumWaitInterval:
		CONFIG_GET(MIB_CWMP_RETRY_MIN_WAIT_INTERVAL, &in);
		*data = uintdup(in);
		break;

	case eMS_CWMPRetryIntervalMultiplier:
		CONFIG_GET(MIB_CWMP_RETRY_INTERVAL_MUTIPLIER, &in);
		*data = uintdup(in);
		break;
#endif

#ifdef TR069_ANNEX_G
	case eMS_UDPConnectionRequestAddress:
		CONFIG_GET(MIB_CWMP_UDP_CONN_REQ_ADDR, buf);
		*data = strdup(buf);
		break;
		
	case eMS_STUNEnable:
		CONFIG_GET(MIB_CWMP_STUN_EN, &in);
		*data = booldup(in);
		break;
		
	case eMS_STUNServerAddress:
		CONFIG_GET(MIB_CWMP_STUN_SERVER_ADDR, buf);
		*data = strdup(buf);
		break;
		
	case eMS_STUNServerPort:
		CONFIG_GET(MIB_CWMP_STUN_SERVER_PORT, &in);
		*data = uintdup(in);
		break;
		
	case eMS_STUNUsername:
		CONFIG_GET(MIB_CWMP_STUN_USERNAME, buf);
		*data = strdup(buf);
		break;
		
	case eMS_STUNPassword:
		CONFIG_GET(MIB_CWMP_STUN_PASSWORD, buf);
		*data = strdup(buf);
		break;
		
	case eMS_STUNMaximumKeepAlivePeriod:
		CONFIG_GET(MIB_CWMP_STUN_MAX_KEEP_ALIVE_PERIOD, &in1);
		*data = intdup(in1);
		break;
		
	case eMS_STUNMinimumKeepAlivePeriod:
		CONFIG_GET(MIB_CWMP_STUN_MIN_KEEP_ALIVE_PERIOD, &in);
		*data = uintdup(in);
		break;
		
	case eMS_NATDetected:
		CONFIG_GET(MIB_CWMP_NAT_DETECTED, &in);
		*data = uintdup(in);
		break;
#endif

	case eMS_ManageableDeviceNumberOfEntries:
	{
		FILE *fp;
		int count=0;
		fp=fopen( TR069_ANNEX_F_DEVICE_FILE, "r" );
		
		while( fp && fgets( buf,160,fp ) )
		{
			char *p;
			
			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{
				count++;
			}
		}
		if(fp) fclose(fp);
		gMgableDevNum = count;
		*data = uintdup(gMgableDevNum);
	}
		break;

	default:
		return ERR_9005;
				
	}

	return 0;

}

int setMgmtServer(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	unsigned int *pNum;
	unsigned char byte;
	unsigned int iVal;
	int          *pInt;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	switch(getIndexOf(tMgmtServerLeaf, entity->info->name)) {
	case eMS_URL:
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_ACS_URL, buf);
		cwmpSettingChange(MIB_CWMP_ACS_URL);
		break;
	case eMS_Username:
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_ACS_USERNAME, buf);
		cwmpSettingChange(MIB_CWMP_ACS_USERNAME);
		break;
	case eMS_Password:
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_ACS_PASSWORD, buf);
		cwmpSettingChange(MIB_CWMP_ACS_PASSWORD);
		break;
	case eMS_PeriodicInformEnable:
		pNum = (unsigned int *)data;
		CHECK_PARAM_NUM(*pNum, 0, 1);
		iVal = (*pNum == 0) ? 0 : 1;
		CONFIG_SET(MIB_CWMP_INFORM_ENABLE, &iVal);	
		cwmpSettingChange(MIB_CWMP_INFORM_ENABLE);
		break;	
	case eMS_PeriodicInformInterval:
		pNum = (unsigned int *)data;
		if (*pNum < 1) return ERR_9007;		
		CONFIG_SET(MIB_CWMP_INFORM_INTERVAL, pNum);
		cwmpSettingChange(MIB_CWMP_INFORM_INTERVAL);
		break;	
	case eMS_PeriodicInformTime:
		pNum = (unsigned int *)buf;
		CONFIG_SET(MIB_CWMP_INFORM_TIME, buf);
		cwmpSettingChange(MIB_CWMP_INFORM_TIME);
		break;	

	case eMS_ConnectionRequestUsername:
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_CONREQ_USERNAME, buf);
		cwmpSettingChange(MIB_CWMP_CONREQ_USERNAME);
		break;	
	case eMS_ConnectionRequestPassword:
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_CWMP_CONREQ_PASSWORD, buf);
		cwmpSettingChange(MIB_CWMP_CONREQ_PASSWORD);
		break;	
	case eMS_UpgradesManaged:
		pNum = (unsigned int *)data;
		CHECK_PARAM_NUM(*pNum, 0, 1);
		byte = (*pNum == 0) ? 0 : 1;
		CONFIG_SET(MIB_CWMP_ACS_UPGRADESMANAGED, &byte);	
		break;
#if 0 // rewrite
	case eMS_CWMPRetryMinimumWaitInterval:
		pNum = (unsigned int *)data;
		if (*pNum < 1) return ERR_9007;		
		CONFIG_SET(MIB_CWMP_RETRY_MIN_WAIT_INTERVAL, pNum);
		cwmpSettingChange(MIB_CWMP_RETRY_MIN_WAIT_INTERVAL);
		break;	
	case eMS_CWMPRetryIntervalMultiplier:
		pNum = (unsigned int *)data;
		if (*pNum < 1000) return ERR_9007;		
		CONFIG_SET(MIB_CWMP_RETRY_INTERVAL_MUTIPLIER, pNum);
		cwmpSettingChange(MIB_CWMP_RETRY_INTERVAL_MUTIPLIER);
		break;	
#endif
#ifdef TR069_ANNEX_G
	case eMS_STUNEnable:
		pNum = (unsigned int *)data;
		CHECK_PARAM_NUM(*pNum, 0, 1);
		iVal = (*pNum == 0) ? 0 : 1;
		CONFIG_SET(MIB_CWMP_STUN_EN, &iVal);	

		if (iVal)
			cwmpStartStun();
		else
			cwmpStopStun();
		break;
		
	case eMS_STUNServerAddress:
		CHECK_PARAM_STR(buf, 0, CWMP_STUN_SERVER_ADDR_LEN+1);
		CONFIG_SET(MIB_CWMP_STUN_SERVER_ADDR, buf);
		break;
		
	case eMS_STUNServerPort:
		pNum = (unsigned int *)data;
		if ((*pNum < 0) || (*pNum > 65535)) return ERR_9007;		
		CONFIG_SET(MIB_CWMP_STUN_SERVER_PORT, pNum);
		break;
		
	case eMS_STUNUsername:
		CHECK_PARAM_STR(buf, 0, CWMP_STUN_USERNAME_LEN+1);
		CONFIG_SET(MIB_CWMP_STUN_USERNAME, buf);
		break;
		
	case eMS_STUNPassword:
		CHECK_PARAM_STR(buf, 0, CWMP_STUN_PASSWORD_LEN+1);
		CONFIG_SET(MIB_CWMP_STUN_PASSWORD, buf);
		break;
		
	case eMS_STUNMaximumKeepAlivePeriod:
		pInt = (int *)data;
		if (*pInt < -1) return ERR_9007;	
		CONFIG_SET(MIB_CWMP_STUN_MAX_KEEP_ALIVE_PERIOD, pInt);
		break;
		
	case eMS_STUNMinimumKeepAlivePeriod:
		pNum = (unsigned int *)data;
		CONFIG_SET(MIB_CWMP_STUN_MIN_KEEP_ALIVE_PERIOD, pNum);
		break;
#endif

	default:
		return ERR_9005;
				
	}

	return 0;


}