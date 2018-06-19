#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
//#include <libtd.h>
#include <blowfish.h>
#include <sys/md5.h>
#include <base64.h>
#include <ddns.h>
#include <blowfish.h>

//#define DEBUG_ORAY		1
#ifdef	DEBUG_ORAY
#define ORAYDBG(fmt, ...)		diag_printf(fmt, ## __VA_ARGS__)
#else
#define ORAYDBG(fmt, ...)
#endif

#define userlog diag_printf 

#define MD5_SIZE		16

#define ORAY_PORT 6060

#define LOGIN_FAIL(err, msg) {rc=err; log=msg; goto end;}
#define	KEEPALIVE_FAIL(err, msg)	LOGIN_FAIL(err, msg)

// #define CLINET_VERSION "3.1.1.10"

#define UDP_OPCODE_UPDATE		10
#define UDP_OPCODE_UPDATE_OK 	50
#define UDP_OPCODE_UPDATE_ERROR 1000

#define ORAY_DOMAINNAME_ENABLE	25
#define ORAY_DOMAINNAME_DISABLE	9

static int oray_port;
//struct in_addr oray_server_addr;
static struct sockaddr_in me;

static unsigned char szChallenge[256];
static int nChallengeLen;
static unsigned long nChatID;
static unsigned long nStartID;

static int lastResponseID = -1;
static int lastResponse = -1;

static unsigned char PHServer[32];
static unsigned char UserType[8];
static unsigned char GetMiscInfoResult[8];
static int onlineStatus = 0;


int keepalive_ticks = 0;

#ifdef	__CONFIG_DDNS__
#if	!defined(CONFIG_ORAY_CLIENTID)
unsigned short	oray_clientid = 0x1001;
unsigned short	oray_clientver = 0x3612;
unsigned long	oray_clientkey = 0x19790629;
#else
unsigned short	oray_clientid = CONFIG_ORAY_CLIENTID;
unsigned short	oray_clientver = CONFIG_ORAY_CLIENTVER;
unsigned long	oray_clientkey = CONFIG_ORAY_CLIENTKEY;
#endif
#endif

#define	ORAY_PHSRV3_CLIENTID	0x1001
#define	ORAY_PHSRV3_CLIENTVER	0x3612
#define	ORAY_PHSRV3_CLIENTKEY	0x19790629
//extern	unsigned short oray_clientid;
//extern	unsigned short oray_clientver;
//extern	unsigned long oray_clientkey;

#define	AUTH_PHSRV3		0
#define	AUTH_ROUTER		1

struct ORAY_DOMAIN_INFO DomainNames[20];

struct DATA_KEEPALIVE
{
	long lChatID;
	long lOpCode;
	long lID;
	long lSum;
	long lReserved;
};

struct DATA_KEEPALIVE_EXT
{
	struct DATA_KEEPALIVE keepalive;
	long ip;
};

#define	KEEPALIVE_PACKET_LEN		20

#if BYTE_ORDER == BIG_ENDIAN

#define	h2l(x)		((((x)>>24)&0x000000ff) + (((x)>>8)&0x0000ff00) + (((x)<<8)&0x00ff0000) + (((x)<<24)&0xff000000))
#define	H2L(x)		((x)=h2l(x))
#define	HB2LB(p, l)	\
{	\
	unsigned long *d = (unsigned long *)(p);	\
	int n = (l)/4;	\
	int i;	\
	for (i=0; i<n; i++)	\
		H2L(d[i]);	\
}

#define	l2h(x)		h2l(x)
#define	L2H(x)		H2L(x)
#define	LB2HB(p, l)	HB2LB(p, l)

#else

#define	h2t(x)		(x)
#define	H2L(x)
#define	HB2LB(p, l)

#define	l2h(x)		(x)
#define	L2H(x)
#define	LB2HB(p, l)

#endif

//roy +++
extern void hmac_md5(const unsigned char* text, int text_len, const unsigned char* key, int key_len, unsigned char *digest);
 /* pointer to data stream */
/* length of data stream */
/* pointer to authentication key */
/* length of authentication key */
/* caller digest to be filled in */
//+++

int oray_recv(int s, char *buf, int len)
{
	struct timeval to;
	fd_set fds;
	int n;
	int bytes = 0;


	to.tv_sec = 5;
	to.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(s, &fds);
	n = select(s+1, &fds, NULL, NULL, &to);  /* Wait on read or error */
	if (n < 0)
	{
		return -3;
	}
	else if (n == 0)
	{
		return 0;
	}
	else
	{
		bytes = recv(s, buf, len, 0);
		if (bytes < 0)
		{
			return -3;
		}
	}

	return bytes;
}


static unsigned char zbuf[2048];
static unsigned char zdata[1024];
static char *ChangeOrayVersion(unsigned short ver)
{
	static char buf[12];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%1x.%1x.%1x.%1x", ((ver>>12)&0xF), ((ver>>8)&0xF), ((ver>>4)&0xF), ((ver)&0xF));
	return buf;
}


static int getmiscinfo(const char *userid, const char*pwd, struct hostent *site)
{
	int	info_sock;
	struct sockaddr_in me;
	MD5_CTX mdContext;
	unsigned char md5[MD5_SIZE], passwd_md5[2*MD5_SIZE+1];
	char *postdata, *postbuf;
	int ret;
	char buf[256+1];
	int i, j;
	int flag = 0;
	char *proc_blk;
	int proc_blk_len;
	char *hostname = CONFIG_ORAY_SERVER;
	char *OrayVersion;


	info_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (info_sock < 0)
		return -3;
	memset(&me, 0, sizeof(struct sockaddr_in));
	memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
//	oray_server_addr.s_addr = me.sin_addr.s_addr;
	me.sin_family = AF_INET;
	me.sin_len = sizeof(struct sockaddr_in);
	me.sin_port = htons(80);
	if (connect(info_sock, (struct sockaddr*)&me, sizeof(struct sockaddr_in)) != 0)
	{
		close(info_sock);
		return -3;
	}


	// Get MD5 password
	MD5Init(&mdContext);
	MD5Update(&mdContext, pwd, strlen(pwd));
	MD5Final(md5, &mdContext);

	sprintf(passwd_md5, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			md5[0], md5[1], md5[2], md5[3],
			md5[4], md5[5], md5[6], md5[7],
			md5[8], md5[9], md5[10],md5[11],
			md5[12],md5[13],md5[14],md5[15]);

	// Send XML out
	postdata = zdata;	// Change to malloc
	postbuf = zbuf;

	OrayVersion=ChangeOrayVersion(oray_clientver);
	ORAYDBG("oray: version=%s\n", OrayVersion);
	ORAYDBG("oray: uname=%s, pwd=%s, md5pwd=%s\n", userid, pwd, passwd_md5);

	sprintf(postdata,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
					"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
					"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
					"xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
					"<soap:Header>\r\n"
					"<AuthHeader xmlns=\"http://tempuri.org/\">\r\n"
					"<Account xmlns=\"http://tempuri.org/\">%s</Account>\r\n"
					"<Password xmlns=\"http://tempuri.org/\">%s</Password>\r\n"
					"</AuthHeader>\r\n"
					"</soap:Header>\r\n"
					"<soap:Body>\r\n"
					"<GetMiscInfo xmlns=\"http://tempuri.org/\">\r\n"
					"<clientversion xmlns=\"http://tempuri.org/\">%s</clientversion>\r\n"
					"</GetMiscInfo>\r\n"
					"</soap:Body>\r\n"
					"</soap:Envelope>\r\n", 
					userid,
					passwd_md5,
					OrayVersion);

	sprintf(postbuf,"POST /userinfo.asmx HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Content-Type: text/xml; charset=utf-8\r\n"
					"Content-Length: %d\r\nSOAPAction: http://tempuri.org/GetMiscInfo\r\n\r\n%s",
					hostname,(int)strlen(postdata), postdata);

	ret = send(info_sock, postbuf, strlen(postbuf), 0);

	// Read until nothing left
	//while (oray_recv(info_sock, buf, sizeof(buf)) == sizeof(buf))

	strcpy(UserType, "0");
	UserType[1]=0;
	proc_blk = 0;
	proc_blk_len = 0;

	//while ((len = fread(buf, 1, sizeof(buf),  infp)) > 0)
	while ((ret = oray_recv(info_sock, buf, sizeof(buf)-1)) > 0) 
	{
		if(!proc_blk)
		{
			proc_blk = (char*) malloc( ret+1 );
		}
		else
		{
			proc_blk = (char*) realloc( (void*) proc_blk, proc_blk_len+ret+1 );
		}

		if(proc_blk == NULL)
			return -3;

		memcpy( &(proc_blk[proc_blk_len]), buf, ret );
		proc_blk_len += ret;
    	proc_blk[proc_blk_len] = '\0';
	}
	
#if 0
	// Simon test here
	for (i=0; i<proc_blk_len; i++)
	{
		if ((i%64) == 0)
			diag_printf("\n");
		
		if ((unsigned char)proc_blk[i] >= 128)
			diag_printf(" ");
		else
			diag_printf("%c", proc_blk[i]);
	}
#endif
	//Parse PHServer
	for(i=0; i<proc_blk_len; i++)
	{	
		if(strncmp(proc_blk+i, "<PHServer>", 10)==0)
		{
			for(j=i+10; j<proc_blk_len; j++)
			{
				if(strncmp(proc_blk+j, "</PHServer>", 11)==0)
				{
					char *sp;
					strncpy(PHServer, proc_blk+i+10, j-i-10);
					PHServer[j-i-10] = 0;
					if((sp=strchr(PHServer,' '))!=NULL)
						*sp = 0;
					ORAYDBG("oray: PHServer=%s\n", PHServer);
					flag = 1;
					break;
				}
			}
			if (flag == 1)
				break;
		}
	}

	flag = 0;

	//Parse UserType
	for(i=0; i<proc_blk_len; i++)
	{	
		if(strncmp(proc_blk+i, "<UserType>", 10)==0)
		{
			for(j=i+10; j<proc_blk_len; j++)
			{
				if(strncmp(proc_blk+j, "</UserType>", 11)==0)
				{
					strncpy(UserType, proc_blk+i+10, j-i-10);
					UserType[j-i-10] = 0;
					ORAYDBG("oray: UserType=%s\n", UserType);
					flag = 1;
					break;
				}
			}
			if (flag == 1)
				break;
		}
	}
	flag = 0;
	//GetMiscInfoResult
	for(i=0; i<proc_blk_len; i++)
	{	
		if(strncmp(proc_blk+i, "<GetMiscInfoResult>", 19)==0)
		{

			for(j=i+19; j<proc_blk_len; j++)
			{
				if(strncmp(proc_blk+j, "</Get", 5)==0)
				{				
					strncpy(GetMiscInfoResult, proc_blk+i+19, j-i-19);					
					GetMiscInfoResult[j-i-19] = 0;
					ORAYDBG("oray: GetMiscInfoResult=%s\n", GetMiscInfoResult);
					flag = 1;
					break;
				}
			}
			if (flag == 1)
				break;
		}
	}



	if(proc_blk)
	{
		free(proc_blk);
	}
	
	close(info_sock);
	ORAYDBG("oray: GetMiscInfo done\n");
	return 0;
}


static int getdomaininfo(const char *userid, const char*pwd, struct hostent *site)
{
	unsigned char DomainName[50], SC[3];
	int	info_sock;
	struct sockaddr_in me;
	MD5_CTX mdContext;
	unsigned char md5[MD5_SIZE], passwd_md5[2*MD5_SIZE+1];
	char *postdata, *postbuf;
	int ret;
	char buf[256+1];
	int i, j, x;
	int flag = 0;
	char *proc_blk;
	int proc_blk_len;
	char *hostname = CONFIG_ORAY_SERVER;


	info_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (info_sock < 0)
		return -3;

	memset(&me, 0, sizeof(struct sockaddr_in));
	memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
//	oray_server_addr.s_addr = me.sin_addr.s_addr;
	me.sin_family = AF_INET;
	me.sin_len = sizeof(struct sockaddr_in);
	me.sin_port = htons(80);
	if (connect(info_sock, (struct sockaddr*)&me, sizeof(struct sockaddr_in)) != 0)
	{
		close(info_sock);
		return -3;
        }

	// Get MD5 password
	MD5Init(&mdContext);
    	MD5Update(&mdContext, pwd, strlen(pwd));
	MD5Final(md5, &mdContext);

	sprintf(passwd_md5, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			md5[0], md5[1], md5[2], md5[3],
			md5[4], md5[5], md5[6], md5[7],
			md5[8], md5[9], md5[10],md5[11],
			md5[12],md5[13],md5[14],md5[15]);

	ORAYDBG("oray: uname=%s, pwd=%s, md5pwd=%s\n", userid, pwd, passwd_md5);
	
	// Send XML out
	postdata = zdata;	// Change to malloc
	postbuf = zbuf;

	sprintf(postdata,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
					"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
					"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
					"xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
					"<soap:Header>\r\n"
					"<AuthHeader xmlns=\"http://tempuri.org/\">\r\n"
					"<Account xmlns=\"http://tempuri.org/\">%s</Account>\r\n"
					"<Password xmlns=\"http://tempuri.org/\">%s</Password>\r\n"
					"</AuthHeader>\r\n"
					"</soap:Header>\r\n"
					"<soap:Body>\r\n"
					"<GetDomains xmlns=\"http://tempuri.org/\" />\r\n"
					"</soap:Body>\r\n"
					"</soap:Envelope>\r\n", 
					userid,
					passwd_md5);

	sprintf(postbuf,"POST /DomainInfo.asmx HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Content-Type: text/xml; charset=utf-8\r\n"
					"Content-Length: %d\r\nSOAPAction: http://tempuri.org/GetDomains\r\n\r\n%s",
					hostname, (int)strlen(postdata), postdata);

	ret = send(info_sock, postbuf, strlen(postbuf), 0);

	// Read until nothing left
	//while (oray_recv(info_sock, buf, sizeof(buf)) == sizeof(buf))

	proc_blk = 0;
	proc_blk_len = 0;

	//while ((len = fread(buf, 1, sizeof(buf),  infp)) > 0)
	while ((ret = oray_recv(info_sock, buf, sizeof(buf)-1)) > 0) 
	{
		if(!proc_blk)
		{
			proc_blk = (char*) malloc( ret+1 );
		}
		else
		{
			proc_blk = (char*) realloc( (void*) proc_blk, proc_blk_len+ret+1 );
		}

		if(proc_blk == NULL)
			return -3;

		memcpy( &(proc_blk[proc_blk_len]), buf, ret );
		proc_blk_len += ret;
    	proc_blk[proc_blk_len] = '\0';
	}
	
	memset(DomainNames,0,sizeof(DomainNames));
	//Parse DomainName
	x = 0;
	for(i=0; i<proc_blk_len; i++)
	{	
		if(strncmp(proc_blk+i, "<DomainName>", 12)==0)
		{
			for(j=i+12; j<proc_blk_len; j++)
			{
				if(strncmp(proc_blk+j, "</DomainName>", 13)==0)
				{
					strncpy(DomainName, proc_blk+i+12, j-i-12);
					DomainName[j-i-12] = 0;
					strcat(DomainNames[x].DName,DomainName);
					ORAYDBG("oray: domain name%d=%s\n", x, DomainName);
					x++;

					flag = 1;
					break;
				}
			}
		}
	}
	

	//Parse StatusCode
	x = 0;
	for(i=0; i<proc_blk_len; i++)
	{	
		if(strncmp(proc_blk+i, "<StatusCode>", 12)==0)
		{
			for(j=i+12; j<proc_blk_len; j++)
			{
				if(strncmp(proc_blk+j, "</StatusCode>", 13)==0)
				{
					strncpy(SC, proc_blk+i+12, j-i-12);
					SC[j-i-12] = 0;
					DomainNames[x].Statuscode = atoi(SC);
					ORAYDBG("oray: domain status%d=%d\n", x, DomainNames[x].Statuscode);
					x++;

					flag = 1;
					break;
				}
			}
		}
	}

	if(proc_blk)
	{
		free(proc_blk);
	}

	close(info_sock);
	ORAYDBG("oray: GetDomainInfo done\n");
	
	return 0;
}


int oray_ddns_login(const char *userid, const char*pwd, const char *ip, const char *domain)
{
	struct hostent *site;
	
	int host_sock = -1;
	FILE *sock_file = 0;
	int rc = 0, x = 0, EN_DomainNames = 0;
	char *log = 0;
	
	int len;
	unsigned char buf[256];
	unsigned char secret[16];
	unsigned char buf2[256];
	unsigned char dname_buf[512];
//	unsigned char tempbuf3[50];
	
	unsigned long clientkey = oray_clientkey;
	unsigned long clientinfo = (((oray_clientid << 16) & 0xffff0000) | oray_clientver);
	int auth_mode;
	
	long challengetime = 0;
	long challengetime_new;
	char *hostname = CONFIG_ORAY_SERVER;
	if (oray_clientid == ORAY_PHSRV3_CLIENTID &&
		oray_clientver == ORAY_PHSRV3_CLIENTVER &&
		oray_clientkey == ORAY_PHSRV3_CLIENTKEY)
	{
		auth_mode = AUTH_PHSRV3;
	}
	else
	{
		auth_mode = AUTH_ROUTER;
	}
	
	ORAYDBG("auth_mode = %s\n", (auth_mode == AUTH_PHSRV3 ? "AUTH_PHSRV3" : "AUTH_ROUTER"));
	ORAYDBG("WEBSERVICE server = %s\n id=%x, ver=%x, key=%x, clientinfo=%x\n", 
		hostname, oray_clientid, oray_clientver, oray_clientkey, clientinfo);

	oray_port = ORAY_PORT;
	site = gethostbyname(hostname);
	if (site == NULL)
		LOGIN_FAIL(-1, "Resolve oray.net failed");

	//************************
	// Do GetMiscInfo
	//************************
	if (getmiscinfo(userid, pwd, site) != 0)
		LOGIN_FAIL(-1, NULL);
	ORAYDBG("PHServer=%s\n", PHServer);
	ORAYDBG("UserType=%s\n", UserType);
	ORAYDBG("GetMiscInfoResult=%s\n", GetMiscInfoResult);

	if (strcmp(GetMiscInfoResult, "0") != 0)
	{
		ORAYDBG("GetMiscInfoResult=%s !=0\n",GetMiscInfoResult);
		LOGIN_FAIL(535, "GetMiscInfoResult !=0")
	}	

	if (strcmp(UserType, "1") == 0)
		keepalive_ticks = 30 * 100;
	else
		keepalive_ticks = 60 * 100;


	//************************
	// Do GetDomainInfo
	//************************
	if (getdomaininfo(userid, pwd, site) != 0)
	{
		//LOGIN_FAIL(-1, NULL);
		LOGIN_FAIL(-1, "getdomaininfo !=0");
	}

	memset(dname_buf, 0, sizeof(dname_buf));
	while (DomainNames[x].Statuscode > 0)
	{
		//ORAYDBG("\nDomainNames[%d].DName=%s\nStatusCode=%d\n\n", x, DomainNames[x].DName, DomainNames[x].Statuscode);
		if (DomainNames[x].Statuscode == ORAY_DOMAINNAME_ENABLE)
		{
			sprintf(buf,"regi a %s\r\n",DomainNames[x].DName);
			strcat(dname_buf, buf);
			EN_DomainNames++;
		}
		x++;
	}

	site = gethostbyname(PHServer);
	if (site == NULL)
		LOGIN_FAIL(-1, "PHServer resolve error!");
		//return -1;
	

	//************************
	// Do Update
	//************************
	host_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (host_sock < 0)
		LOGIN_FAIL(-1, "host_sock error!");
	memset(&me, 0, sizeof(struct sockaddr_in));
	memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
	me.sin_family = AF_INET;
	me.sin_len = sizeof(struct sockaddr_in);
	me.sin_port = htons(oray_port);
	
	if (connect(host_sock, (struct sockaddr*)&me, sizeof(struct sockaddr_in)) != 0)
		LOGIN_FAIL(-1, "connect to 6060 failed");
	
	// Receive: 220 oray.net DHRP Server Ready
	len = oray_recv(host_sock, buf, sizeof(buf)-1);
	if (len <= 0)
		LOGIN_FAIL(-1, NULL);
	
	buf[len] = 0;
	if (atoi(buf) != 220)
		LOGIN_FAIL(-1, "dhrp server not ready");
	
	if (auth_mode == AUTH_ROUTER)
		strcpy(buf, "auth router\r\n");
	else
		strcpy(buf, "auth phsrv3\r\n");
		
	// Send: auth
	send(host_sock, buf, strlen(buf), 0);
	
	// Receive: 334 Challenge string
	//			6 bytes + 4 bytes challenge time + 6 bytes
	// 			Where challenge time is LITTLE ENDIAN format
	len = oray_recv(host_sock, buf, sizeof(buf)-1);
	if (len <= 0)
		LOGIN_FAIL(-1, "cannot receive auth response");
	
	buf[len] = 0;
	if (atoi(buf) != 334)
		LOGIN_FAIL(535, "auth mechanism not support");
	
	buf[len-2] = 0;	// wrep out CR-LF
	ORAYDBG("oray: base64 challenge=\"%s\"\n", buf+4);
	nChallengeLen = base64_decode(buf+4, szChallenge, sizeof(szChallenge));
	
	// Do secret
	hmac_md5(pwd, strlen(pwd), szChallenge, nChallengeLen, secret);
	
	// Get chellengetime
	memcpy(&challengetime, szChallenge+6, sizeof(challengetime));
	L2H(challengetime);
	
	if (auth_mode == AUTH_ROUTER)
	{
		int nMoveBits;
		
		challengetime_new = challengetime | ~clientkey;
		nMoveBits = challengetime_new % 30;
		
		ORAYDBG("Before challengetime = %lx\n", challengetime);
		ORAYDBG("challengetime_new = %lx, nMoveBits = %d\n", challengetime_new, nMoveBits);
		
		if (nMoveBits >= 0)
		{
			challengetime_new = (challengetime_new<<(32-nMoveBits)) | ((challengetime_new>>nMoveBits) & ~(0xffffffff<<(32-nMoveBits)));
		}
		else
		{
			nMoveBits = -nMoveBits;
			challengetime_new = (challengetime_new<<nMoveBits) | ((challengetime_new>>(32-nMoveBits)) & ~(0xffffffff<<nMoveBits));
		}
		
		ORAYDBG("After challengetime_new = %lx\n", challengetime_new);
	}
	else
	{
		challengetime_new = challengetime | ~clientkey;
	}
	
	// Prepare userid + ' ' + 4 bytes challengetime + 4 bytes client info + client secret
	memset(buf, 0, sizeof(buf));
	strcpy(buf, userid);
	buf[strlen(userid)] = ' ';
	
	len = strlen(buf);
	
	H2L(challengetime_new);
	memcpy(buf+len, &challengetime_new, 4);
	
	H2L(clientinfo);
	memcpy(buf+len+4, &clientinfo, 4);
	memcpy(buf+len+8, secret, 16);
	
	// Do base64 encode, and send out
	base64_encode(buf, len+4+4+16, buf2, 256);
	strcat(buf2, "\r\n");
	
	send(host_sock, buf2, strlen(buf2), 0);

	// Simon test here...	
//	buf2[strlen(buf2)-2] = 0;
//	len = base64_decode(buf2, buf, sizeof(buf));
	
	// duplicate the socket to file
	// And read the response 250
	sock_file=fdopen(host_sock,"r+");
	fgets(buf,sizeof buf,sock_file);
	if (atoi(buf) != 250)
		LOGIN_FAIL(535, "authentication fail");


	// Read domain name until . 
//	memset(buf2,0,sizeof(buf2));
	while (fgets(buf,sizeof buf,sock_file) && buf[0]!='.'){}
#if 0
	while (fgets(buf,sizeof buf,sock_file) && buf[0]!='.')
	{
		//if(domain && strcmp(buf,domain)==0)
		if (domain && memcmp(buf,domain,strlen(domain))==0)
		{
			if ((strlen(domain)+2) == strlen(buf))
				sprintf(buf2,"regi a %s",buf);
		}	
		else if (!domain)
			sprintf(buf2+strlen(buf2), "regi a %s",buf);
	}
#endif
	if (DomainNames == 0)
		LOGIN_FAIL(535, "domain not found");
	
	// Send REGI A [domain name] CNFM and check response code 250
	strcat(dname_buf, "cnfm\r\n");
	send(host_sock, dname_buf, strlen(dname_buf), 0);
	diag_printf("\n--------\nTry to enable Domain Name as below:\n%s\n---------\n",dname_buf);

	//For pass domain name that has been ENABLE in response buf
	for(x=0;x<EN_DomainNames;x++)
	{
		fgets(buf, sizeof buf, sock_file);
		ORAYDBG("%d.DomainNames =%s\n", x, buf);
	}

	//fgets(buf, sizeof buf, sock_file);
	if ((atoi(buf) != 250) && (EN_DomainNames > 0))
		LOGIN_FAIL(535, "CNFM fail");

	// Get session id and index
	//CBlowfish_SetKey(server_key, 16);
	
	fgets(buf, sizeof(buf), sock_file);
	ORAYDBG("ID = %s", buf);

	nChatID = atoi(buf+4);
	nStartID = atoi(strchr(buf+4, ' '));
	
	ORAYDBG("oray: ChatID=%d, StartID=%d\n", nChatID, nStartID);
	lastResponseID = -1;
	lastResponse = cyg_current_time();	
	
	// Send Quit and close socket and file
	strcpy(buf, "quit\r\n");

	send(host_sock, buf, strlen(buf), 0);

	fgets(buf, sizeof(buf), sock_file);
	
	log = "Register OK";
	
end:
	if (sock_file)
		fclose(sock_file);
	if (host_sock != -1)
		close(host_sock);
	if (log)
		userlog("DDNS:ORAY %s\n", log);
	
	onlineStatus = (rc == 0)? 1 : -1;
	
	return rc;
}


int oray_keepalive(void)
{
	struct DATA_KEEPALIVE data;
	struct DATA_KEEPALIVE_EXT rdata;
	unsigned char p1[KEEPALIVE_PACKET_LEN];
	unsigned char p2[KEEPALIVE_PACKET_LEN];
	int len;
	
	int	sock = -1;
	int rc = 0;
	char *log = 0;
	
	//*************************
	// Send keep alive
	//*************************
	// Set key first
	CBlowfish_SetKey(szChallenge, nChallengeLen);

	// Prepare KEEPALIVE data
	memset(&data, 0, sizeof(data));

	data.lChatID = nChatID;
	data.lID = nStartID;
	data.lOpCode = UDP_OPCODE_UPDATE;
	data.lSum = 0 - (data.lID + data.lOpCode);
	data.lReserved = 0;
	ORAYDBG("oray: keepalive ChatID=%d, StartID=%d, lsum=%d\n", nChatID, nStartID, data.lSum);
	
	memcpy(p1, &data, KEEPALIVE_PACKET_LEN);
	memcpy(p2, p1, KEEPALIVE_PACKET_LEN);
	
	CBlowfish_EnCode(p1+4, p2+4, KEEPALIVE_PACKET_LEN-4);
	HB2LB(p2, KEEPALIVE_PACKET_LEN);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		KEEPALIVE_FAIL(-1, NULL);
	
	if (connect(sock, (struct sockaddr*)&me, sizeof(struct sockaddr_in)) != 0)
		KEEPALIVE_FAIL(-1, NULL);
	
	send(sock, p2, KEEPALIVE_PACKET_LEN, 0);

	//*****************************
	// Receive keep alive response
	//*****************************
	len = oray_recv(sock, (char *)&rdata, sizeof(rdata));
	//if (len < sizeof(rdata))
	if (len <= 0)
	{
		if ((cyg_current_time() - lastResponse) > 3*60*100)
		{
			KEEPALIVE_FAIL(-2, "Keep alive response time overs 3 minutes");
		}
		else
		{
			KEEPALIVE_FAIL(-1, NULL);
		}
	}
	
	LB2HB(&rdata, sizeof(rdata));
	memcpy(p1, &rdata.keepalive, KEEPALIVE_PACKET_LEN);
	memcpy(p2, p1, KEEPALIVE_PACKET_LEN);
	
	CBlowfish_DeCode(p1+4, p2+4, KEEPALIVE_PACKET_LEN-4);
	memcpy(&data, p2, KEEPALIVE_PACKET_LEN);
	
	if (data.lChatID != nChatID)
		KEEPALIVE_FAIL(-1, "Wrong ChatID");
	
	if ((data.lOpCode + data.lID + data.lSum) != 0)
		KEEPALIVE_FAIL(-1, "Wrong Checksum");
	
	if (data.lOpCode == UDP_OPCODE_UPDATE_ERROR)
		KEEPALIVE_FAIL(-2, "UDP_OPCODE_UPDATE_ERROR");
	
	if (data.lOpCode != UDP_OPCODE_UPDATE_OK)
		KEEPALIVE_FAIL(-1, "Unknown OpCode");
	
	nStartID = data.lID + 1;
	if ((data.lID - lastResponseID) > 3 && lastResponseID != -1)
		KEEPALIVE_FAIL(-2, "responseID too large");
	
	lastResponseID = data.lID;
	lastResponse = cyg_current_time();
	
	//log = "Keep Alive OK";
	
end:
	if (sock != -1)
		close(sock);
	if (log)
		userlog("DDNS:ORAY %s\n", log);
	
	return rc;
}

int oray_online_status(void)
{
	return onlineStatus;
}

/*
 * status values:
 * -1 update failed
 * 0: shutdown
 * 1: update success
 */
void oray_set_online_status(int status)
{
	onlineStatus = status;
}

int oray_passport_type(void)
{
	if (strcmp(UserType, "1") == 0)
		return 1;
	
	return 0;
}

int Oray_DDNS_Webshow(int x, unsigned char *buf, int *status)
{
	memcpy(buf, DomainNames[x].DName, 50);
	*status = DomainNames[x].Statuscode;

	return 0;
}



