/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    provider.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <ddns.h>
#include <base64.h>
#include <string.h>
#include <netdb.h>

//==============================================================================
//                                    MACROS
//==============================================================================
#define BUFFER_SIZE		4*1024

//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================
#ifdef EZIP_SUPPORT
int EZIP_update_entry(struct user_info *info);
static struct service_t EZIP_service = {
	EZIP_NAME,
	EZIP_update_entry,
	EZIP_DEFAULT_SERVER,	
	EZIP_DEFAULT_PORT,
	EZIP_REQUEST
};
#endif

int DYNDNS_update_entry(struct user_info *info);
static struct service_t DYNDNS_service = {
	DYNDNS_NAME,
	DYNDNS_update_entry,
	DYNDNS_DEFAULT_SERVER,
	DYNDNS_DEFAULT_PORT,
	DYNDNS_REQUEST
};

int DYNDNS_ST_update_entry(struct user_info *info);
static struct service_t DYNDNS_ST_service = {
	DYNDNS_ST_NAME,
	DYNDNS_ST_update_entry,
	DYNDNS_ST_DEFAULT_SERVER,
	DYNDNS_ST_DEFAULT_PORT,
	DYNDNS_ST_REQUEST
};

int DYNDNS_CU_update_entry(struct user_info *info);
static struct service_t DYNDNS_CU_service = {
	DYNDNS_CU_NAME,
	DYNDNS_CU_update_entry,
	DYNDNS_CU_DEFAULT_SERVER,
	DYNDNS_CU_DEFAULT_PORT,
	DYNDNS_CU_REQUEST
};

int ALLDDNS_update_entry(struct user_info *info);
static struct service_t ALLDDNS_service = {
	ALLDDNS_NAME,
	ALLDDNS_update_entry,
	ALLDDNS_DEFAULT_SERVER,
	ALLDDNS_DEFAULT_PORT,
	ALLDDNS_REQUEST
};

static struct service_t QDNS_service = {
	QDNS_NAME,
	DYNDNS_update_entry,
	QDNS_DEFAULT_SERVER,
	QDNS_DEFAULT_PORT,
	QDNS_REQUEST
};

#ifdef DTDNS_SUPPORT
int DTDNS_update_entry(struct user_info *info);
static struct service_t DTDNS_service = {
    DTDNS_NAME,          
    DTDNS_update_entry,  
    DTDNS_DEFAULT_SERVER,
    DTDNS_DEFAULT_PORT,  
    DTDNS_REQUEST        
};
#endif


int NOIP_update_entry(struct user_info *info);
static struct service_t NOIP_service = {
    NOIP_NAME,           
    NOIP_update_entry,   
    NOIP_DEFAULT_SERVER, 
    NOIP_DEFAULT_PORT,   
    NOIP_REQUEST         
};

int TZO_update_entry(struct user_info *info);
static struct service_t TZO_service = {
    TZO_NAME,           
    TZO_update_entry,   
    TZO_DEFAULT_SERVER, 
    TZO_DEFAULT_PORT,   
    TZO_REQUEST         
};

int CHANGEIP_update_entry(struct user_info *info);
static struct service_t CHANGEIP_service = {
    CHANGEIP_NAME,           
    CHANGEIP_update_entry,   
    CHANGEIP_DEFAULT_SERVER, 
    CHANGEIP_DEFAULT_PORT,   
    CHANGEIP_REQUEST         
};

int ODS_update_entry(struct user_info *info);
static struct service_t ODS_service = {
	ODS_NAME,
	ODS_update_entry,
	ODS_DEFAULT_SERVER,
	ODS_DEFAULT_PORT,
	ODS_REQUEST
};

int HN_update_entry(struct user_info *info);
static struct service_t HN_service = {
	HN_NAME,
	HN_update_entry,
	HN_DEFAULT_SERVER,
	HN_DEFAULT_PORT,
	HN_REQUEST
};

int ZOE_update_entry(struct user_info *info);
static struct service_t ZOE_service = {
	ZOE_NAME,
	ZOE_update_entry,
	ZOE_DEFAULT_SERVER,
	ZOE_DEFAULT_PORT,
	ZOE_REQUEST
};
int ORAY_update_entry(struct user_info *info);
int ORAY_init_entry(struct user_info *info);
static struct service_t ORAY_service = {
	ORAY_NAME,
	ORAY_update_entry,
	ORAY_DEFAULT_SERVER,
	ORAY_DEFAULT_PORT,
	ORAY_REQUEST,
	ORAY_init_entry,
	1
};

static struct service_t OVH_service = {
	OVH_NAME,
	DYNDNS_update_entry,
	OVH_DEFAULT_SERVER,
	OVH_DEFAULT_PORT,
	OVH_REQUEST
};

static struct service_t EURODYNDNS_service = {
	EURODYNDNS_NAME,
	DYNDNS_update_entry,
	EURODYNDNS_DEFAULT_SERVER,
	EURODYNDNS_DEFAULT_PORT,
	EURODYNDNS_REQUEST
};

int REGFISH_update_entry(struct user_info *info);
static struct service_t REGFISH_service = {
	REGFISH_NAME,
	REGFISH_update_entry,
	REGFISH_DEFAULT_SERVER,
	REGFISH_DEFAULT_PORT,
	REGFISH_REQUEST
};

int M88IP_update_entry(struct user_info *info);
static struct service_t M88IP_service = {
	M88IP_NAME,
	M88IP_update_entry,
	M88IP_DEFAULT_SERVER,
	M88IP_DEFAULT_PORT,
	M88IP_REQUEST
};

int JUSTL_update_entry(struct user_info *info);
static struct service_t JUSTL_service = {
	JUSTL_NAME,
  	JUSTL_update_entry,
  	JUSTL_DEFAULT_SERVER,
  	JUSTL_DEFAULT_PORT,
  	JUSTL_REQUEST
};

static struct service_t DHK_service = {
	DHK_NAME,
  	JUSTL_update_entry,   //same as JUSTL???
  	DHK_DEFAULT_SERVER,
  	DHK_DEFAULT_PORT,
  	DHK_REQUEST
};

static struct service_t *service_list[]={
	&DYNDNS_service,
	&QDNS_service,
	&NOIP_service,
	&TZO_service,
	&CHANGEIP_service,
	&ODS_service,
	&HN_service,
	&ZOE_service,
	&ORAY_service,
	&OVH_service,
	&EURODYNDNS_service,
	&REGFISH_service,
	&M88IP_service,
	&JUSTL_service,
	&DHK_service,
	&ALLDDNS_service,
	&DYNDNS_ST_service,
	&DYNDNS_CU_service,
	NULL,
};

//==============================================================================
//                          LOCAL FUNCTION PROTOTYPES
//==============================================================================

//==============================================================================
//                              EXTERNAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
struct service_t *find_service(char *name)
{
	struct service_t *serv;
	int i;
	
	for(i=0; service_list[i]; i++)
	{
		serv = service_list[i];
		if(strcmp(serv->name, name) == 0)
			return serv;
	}
	return NULL;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int do_connect(int *sock, char *host, unsigned short port)
{
	struct sockaddr_in addr;
	int len;
	int result;
	struct hostent *hostinfo;

	// set up the socket
	if((*sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		diag_printf("====%s\n", __FUNCTION__);
		perror("socket");
		return(-1);
    }
		
	// get the host address
	hostinfo = gethostbyname(host);  
	
	if(!hostinfo)
	{
		DDNS_DBG("gethostbyname error"); 
		close(*sock);
		return(-1);
    }
   
	addr.sin_family = AF_INET; 
	addr.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
	addr.sin_port = htons(port);

	// connect the socket
	len = sizeof(addr);
	if((result=connect(*sock, (struct sockaddr *)&addr, len)) == -1) 
	{
		perror("connect");
		close(*sock);
		return(-1);
	}

	return 0;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
static void output(int fd, void *buf)
{
	fd_set writefds;
	int max_fd;
	struct timeval tv;
	int ret;

	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);
	max_fd = fd;
	
	tv.tv_sec = 0;
    tv.tv_usec = 20000;          /* timed out every 20 ms */
    
	ret = select(max_fd + 1, NULL, &writefds, NULL, &tv);

	if(ret == -1)
	{
    	DDNS_DBG("[DDNS]: select error\n");
	}
	else if(ret == 0)
	{
		DDNS_DBG("select timeout\n");
	}
	else
	{
		/* if we woke up on client_sockfd do the data passing */
		if(FD_ISSET(fd, &writefds))
		{
			if(send(fd, buf, strlen(buf), 0) == -1)
				DDNS_DBG("error send()ing request\n");
		}
		else
			DDNS_DBG("socket was not exist!!\n");
    }
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int read_input(int fd, void *buf, int len)
{
	fd_set readfds;
	int max_fd;
	struct timeval tv;
	int ret;
	int bread = -1;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	max_fd = fd;
	
	tv.tv_sec = 9;
	tv.tv_usec = 0;          
	ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);

	if(ret == -1)
	{
		DDNS_DBG("[DDNS]: select error\n");
	}
	else if(ret == 0)
	{
		DDNS_DBG("select timeout\n");
	}
	else
	{
		/* if we woke up on client_sockfd do the data passing */
		if(FD_ISSET(fd, &readfds))
		{
			if((bread=recv(fd, buf, len, 0)) == -1)
			{
				DDNS_DBG("error recv()ing reply\n");
			}
		}
		else
			DDNS_DBG("socket was not exist!!\n");
	}

  return(bread);
}

#ifdef EZIP_SUPPORT
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int EZIP_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(bp, "GET %s?mode=update&", info->service->default_request);
	bp += len;
	if(MY_IP)
	{
		len = sprintf(bp, "ipaddress=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp,	"wildcard=%s&"
						"mx=%s&"
						"url=%s&"
						"host=%s&"
						" HTTP/1.0\r\n"
						"Authorization: Basic %s\r\n"
						"User-Agent: %s-%s %s [%s]\r\n"
						"Host: %s\r\n\r\n",
						info->wildcard ? "yes" : "no",
						info->mx,
						info->url,
						info->host,
						auth,
						ROUTER_NAME, FW_VERSION, FW_OS, "",
						info->service->default_server);
						
	bp += len;
	
	*bp = '\0';
	output(fd, buf);

	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		retval = UPDATERES_OK;
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}

error:
	free(buf);
	return retval;
}
#endif

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DYNDNS_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=dyndns&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "wildcard=%s&", info->wildcard ? "ON" : "OFF");
	bp += len;
 	
	if(*(info->mx) != '\0')
	{
		len = sprintf(bp, "mx=%s&", info->mx);
		bp += len;
	}
  	
	len = sprintf(bp, "backmx=%s&", "NO");
	bp += len;

	len = sprintf(bp, "offline=%s&", "NO");
	bp += len;

	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						ROUTER_NAME,
						info->service->default_server);
 						
	bp += len;
	
	*bp = '\0';
	
	
	output(fd, buf);
	
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	//diag_printf("DYNDNS_update_entry: buf:%s\n", buf);
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\ngood") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			if(strstr(buf, "\nnohost") != NULL)
			{
				DDNS_DBG("invalid hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnotfqdn") != NULL)
			{
				DDNS_DBG("malformed hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\n!yours") != NULL)
			{
				DDNS_DBG("host is not under your control\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nabuse") != NULL)
			{
				DDNS_DBG("host has been blocked for abuse\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnochg") != NULL)
			{
				DDNS_DBG("your IP address has not changed since the last update\n");
				retval = UPDATERES_OK;
			}
			else if(strstr(buf, "\nbadauth") != NULL)
			{
				DDNS_DBG("authentication failure\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadsys") != NULL)
			{
				DDNS_DBG("invalid system parameter\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadagent") != NULL)
			{
				DDNS_DBG("this useragent has been blocked\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnumhost") != NULL)
			{
				DDNS_DBG("Too many or too few hosts found\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\ndnserr") != NULL)
			{
				DDNS_DBG("dyndns internal error\n");
				retval = UPDATERES_ERROR;
			}
			else
			{
				DDNS_DBG("error processing request\n");
				retval = UPDATERES_ERROR;
			}
		}
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DYNDNS_ST_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=statdn&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "wildcard=%s&", info->wildcard ? "ON" : "OFF");
	bp += len;
 	
	if(*(info->mx) != '\0')
	{
		len = sprintf(bp, "mx=%s&", info->mx);
		bp += len;
	}

	len = sprintf(bp, "backmx=%s&", "NO");
	bp += len;

  	
	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						ROUTER_NAME,
						info->service->default_server);
 						
	bp += len;
	
	*bp = '\0';
	
	
	output(fd, buf);
	
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	//diag_printf("DYNDNS_update_entry: buf:%s\n", buf);
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\ngood") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			if(strstr(buf, "\nnohost") != NULL)
			{
				DDNS_DBG("invalid hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnotfqdn") != NULL)
			{
				DDNS_DBG("malformed hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\n!yours") != NULL)
			{
				DDNS_DBG("host is not under your control\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nabuse") != NULL)
			{
				DDNS_DBG("host has been blocked for abuse\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnochg") != NULL)
			{
				DDNS_DBG("your IP address has not changed since the last update\n");
				retval = UPDATERES_OK;
			}
			else if(strstr(buf, "\nbadauth") != NULL)
			{
				DDNS_DBG("authentication failure\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadsys") != NULL)
			{
				DDNS_DBG("invalid system parameter\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadagent") != NULL)
			{
				DDNS_DBG("this useragent has been blocked\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnumhost") != NULL)
			{
				DDNS_DBG("Too many or too few hosts found\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\ndnserr") != NULL)
			{
				DDNS_DBG("dyndns internal error\n");
				retval = UPDATERES_ERROR;
			}
			else
			{
				DDNS_DBG("error processing request\n");
				retval = UPDATERES_ERROR;
			}
		}
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DYNDNS_CU_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=custom&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	

	len = sprintf(bp, "offline=%s&", "NO");
	bp += len;

	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						ROUTER_NAME,
						info->service->default_server);
 						
	bp += len;
	
	*bp = '\0';
	
	
	output(fd, buf);
	
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	//diag_printf("DYNDNS_update_entry: buf:%s\n", buf);
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\ngood") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			if(strstr(buf, "\nnohost") != NULL)
			{
				DDNS_DBG("invalid hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnotfqdn") != NULL)
			{
				DDNS_DBG("malformed hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\n!yours") != NULL)
			{
				DDNS_DBG("host is not under your control\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nabuse") != NULL)
			{
				DDNS_DBG("host has been blocked for abuse\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnochg") != NULL)
			{
				DDNS_DBG("your IP address has not changed since the last update\n");
				retval = UPDATERES_OK;
			}
			else if(strstr(buf, "\nbadauth") != NULL)
			{
				DDNS_DBG("authentication failure\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadsys") != NULL)
			{
				DDNS_DBG("invalid system parameter\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadagent") != NULL)
			{
				DDNS_DBG("this useragent has been blocked\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnumhost") != NULL)
			{
				DDNS_DBG("Too many or too few hosts found\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\ndnserr") != NULL)
			{
				DDNS_DBG("dyndns internal error\n");
				retval = UPDATERES_ERROR;
			}
			else
			{
				DDNS_DBG("error processing request\n");
				retval = UPDATERES_ERROR;
			}
		}
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int ALLDDNS_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
  	bp = buf;
  	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s\n", info->service->default_server);
		retval = UPDATERES_ERROR;
		goto error;
	}
		
//	len = sprintf(buf,"GET %s/?uid=%s&pass=%s&hostname=%s"
//	            "&myip=%s HTTP/1.0..Host :%s..Connection: close..Accept: */ *....\n\n",
//	            info->service->default_request,
//	            info->usrname,
//	            info->usrpwd,
//	            info->host,
//	            MY_IP_STR,
//				  info->service->default_server);

	len = sprintf(buf,"GET %s/?uid=%s&pass=%s&hostname=%s"
	            "&myip=%s HTTP/1.0\nUser-Agent: %s DUC %s\n\n",
	            info->service->default_request,
	            info->usrname,
	            info->usrpwd,
	            info->host,
	            MY_IP_STR,
	            ROUTER_NAME,
	            FW_VERSION);
  	
  	bp += len;
	
	*bp = '\0';
	output(fd, buf); 
	
  	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
 	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	if(strstr(buf, "\ngood") != NULL)
			{
				retval = UPDATERES_OK;
			}}
  	
  	close(fd);
  	buf[btot] = '\0';
  	//diag_printf("NOIP_update_entry: buf:%s\n", buf);

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
			
			if(strstr(buf, "\nnohost") != NULL)
			{
				DDNS_DBG("invalid hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnotfqdn") != NULL)
			{
				DDNS_DBG("malformed hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\n!yours") != NULL)
			{
				DDNS_DBG("host is not under your control\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nabuse") != NULL)
			{
				DDNS_DBG("host has been blocked for abuse\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnochg") != NULL)
			{
				printf("###your IP address has not changed since the last update\n");
				DDNS_DBG("your IP address has not changed since the last update\n");
				retval = UPDATERES_OK;
			}
			else if(strstr(buf, "\nbadauth") != NULL)
			{
				DDNS_DBG("authentication failure\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadsys") != NULL)
			{
				DDNS_DBG("invalid system parameteUPDATERES_ERRORr\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadagent") != NULL)
			{
				DDNS_DBG("this useragent has been blocked\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnumhost") != NULL)
			{
				DDNS_DBG("Too many or too few hosts found\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\ndnserr") != NULL)
			{
				DDNS_DBG("allddns internal error\n");
				retval = UPDATERES_ERROR;
			}
			else if(strstr(buf, "\ngood") != NULL)
			{
				printf("####UPDATERES OK\n");
				retval = UPDATERES_OK;
			}
			else
			{
				printf("ddns UPDATERES_ERROR\n");
				DDNS_DBG("error processing request\n");
				retval = UPDATERES_ERROR;
			}
		
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
error:
	free(buf);
	return retval;
}

#ifdef DTDNS_SUPPORT
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DTDNS_update_entry(struct user_info *info)
{	
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp =  buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(buf,"GET %sid=%s&pw=%s&ip=%s HTTP/1.0\r\n"
	            "Host: www.dtdns.com\r\n"
	            "User-Agent: %s\r\n"
	            "\r\n"
	            "Connection: Keep-Alive", 
	            info->service->default_request,
	            info->host,
	            info->usrpwd,
		        MY_IP_STR,
		        ROUTER_NAME);	
		
	bp += len;
	
	*bp = '\0';
	output(fd, buf); 	  	  	
	
  	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
  	
  	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
  	
	switch(ret)
  	{
    case -1:
    	DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

    case 200:
		retval = UPDATERES_OK;
		break;

    case 401:		
      	DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

    default:
		retval = UPDATERES_ERROR;
		break;
	}  	
	
error:
	free(buf);
	return retval;
}
#endif


//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int NOIP_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char *p;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
  	bp = buf;
  	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s\n", info->service->default_server);
		retval = UPDATERES_ERROR;
		goto error;
	}
		
	len = sprintf(buf,"GET http://%s/update.php?username=%s&pass=%s&host=%s"
	            "&ip=%s HTTP/1.0\nUser-Agent: %s DUC %s\n\n",
	            info->service->default_request,
	            info->usrname,
	            info->usrpwd,
	            info->host,
	            MY_IP_STR,
	            ROUTER_NAME,
	            FW_VERSION);
  	
  	bp += len;
	
	*bp = '\0';
	output(fd, buf); 
	
  	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
  	//diag_printf("NOIP_update_entry: buf:%s\n", buf);

	if( (p = strstr(buf, "status=")) == NULL )
	{
		diag_printf(" not find status!\n");
		goto error;	
	}
	else
	{
		//diag_printf("get string!!!!! p:%s p[7]:%s\n", p, &p[7]);
		ret = atoi(&p[7]);	
	}
		
  	//diag_printf("ret:%d\n", ret);
  	switch(ret)
  	{
  		case NOIP_SUCCESS:
  		case NOIP_ALREADYSET:
  			diag_printf("UPDATERES_OK!!\n");
			retval = UPDATERES_OK;
  			break;
  			
  		default:
  			DDNS_DBG("authentication failure\n");
  			retval = UPDATERES_SHUTDOWN;
			break;
  	}
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int TZO_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf,	"GET %s?TZOName=%s&Email=%s&TZOKey=%s&IPAddress=%s&"
						" HTTP/1.0\r\n"
						"User-Agent: %s-%s %s [%s]\r\n"
						"Host: %s\r\n\r\n",
						info->service->default_request,
						info->host,
						info->usrname,
	            		info->usrpwd,
	            		MY_IP_STR,
	            		ROUTER_NAME, FW_VERSION, FW_OS, "",
	            		info->service->default_server);
	bp += len;
	
	*bp = '\0';
	output(fd, buf);       		
  
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
	
	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		retval = UPDATERES_OK;
		break;

	case 302:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int ODS_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, fd, len;
	int retval = UPDATERES_OK;

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	if((bytes = read_input(fd, buf, BUFFER_SIZE)) < 1)
	{
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
	buf[bytes] = '\0';
	if( atoi(buf) != 100)
	{
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
  

	/* send login command */
	len = sprintf(buf, "LOGIN %s %s\n", info->usrname, info->usrpwd);
	bp += len;
	*bp = '\0';
	output(fd, buf);
	
	if((bytes = read_input(fd, buf, BUFFER_SIZE)) < 1)
	{
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
	buf[bytes] = '\0';
	if( (atoi(buf) != 225) && (atoi(buf) != 226))
	{
		DDNS_DBG("error talking to server\n");
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	 /* send delete command */
	len = sprintf(buf, "DELRR %s A\n", info->host);
	bp += len;
	*bp = '\0';
	output(fd, buf);
	
	if((bytes = read_input(fd, buf, BUFFER_SIZE)) < 1)
	{
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
	buf[bytes] = '\0';
	if( atoi(buf) != 901)
	{
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
  
	/* send address command */
	len = sprintf(buf, "ADDRR %s A %s\n", info->host, MY_IP_STR);
	bp += len;
	*bp = '\0';
	output(fd, buf);
	
	if((bytes = read_input(fd, buf, BUFFER_SIZE)) < 1)
	{
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
	buf[bytes] = '\0';
	if( (atoi(buf) != 795) && (atoi(buf) != 796))
	{
		DDNS_DBG("error talking to server\n");
		close(fd);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int HN_update_entry(struct user_info *info)
{
	char *buf, *bp;
	char *p;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;

	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf, "GET %s?VER=1&", info->service->default_request);
	bp += len;
	
 	if(MY_IP)
	{
		len = sprintf(bp, "IP=%s", MY_IP_STR);
		bp += len;
	}
	len = sprintf(bp,	" HTTP/1.1\r\n"
						"Authorization: Basic %s\r\n"
						"User-Agent: %s %s - %s\r\n"
						"Host: %s\r\n\r\n",
						auth,
						ROUTER_NAME, FW_VERSION, FW_OS,
						info->service->default_server);
						
	bp += len;
	
	*bp = '\0';
	output(fd, buf);       		
  	    	
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
	
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
	
	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if((p=strstr(buf, "DDNS_Response_")) != NULL)
      	{
         	sscanf(p, "DDNS_Response_%*code=%3d", &ret);
      	}
      	else
      	{
      		retval = UPDATERES_ERROR;
			break;
		}
		
      	switch(ret)
      	{
        case 101:
			retval = UPDATERES_OK;
			break;

        case 201:
			DDNS_DBG("Last update was less than 300 seconds ago.\n");
			retval = UPDATERES_ERROR;
			break;

        case 202:
			DDNS_DBG("Server error.\n");
			retval = UPDATERES_ERROR;
			break;

        case 203:
			DDNS_DBG("Failure because account is frozen (by admin).\n");
			retval = UPDATERES_SHUTDOWN;
			break;

        case 204:
			DDNS_DBG("Failure because account is locked (by user).\n");
			retval = UPDATERES_SHUTDOWN;
			break;

        default:
			retval = UPDATERES_ERROR;
			break;	
		}
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;


	}

error:
	free(buf);
	return retval;
}
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int ZOE_update_entry(struct user_info *info)
{
	char *buf, *bp;
	char *p;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;

	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf, "GET %s?VER=1&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "host=%s&", info->host);
	bp += len;
	
 	if(MY_IP)
	{
		len = sprintf(bp, "dnsto=%s", MY_IP_STR);
		bp += len;
	}
	len = sprintf(bp,	" HTTP/1.1\r\n"
						"Authorization: Basic %s\r\n"
						"User-Agent: %s %s - %s\r\n"
						"Host: %s\r\n\r\n",
						auth,
						ROUTER_NAME, FW_VERSION, FW_OS,
						info->service->default_server);
						
	bp += len;
	
	*bp = '\0';
	output(fd, buf); 
	    		
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
  	  	
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
	
	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if((p=strstr(buf, "CODE")) != NULL)
      	{
         	sscanf(p, "CODE=\"%3d\"", &ret);
      	}
      	else
      	{
      		retval = UPDATERES_ERROR;
			break;
		}
		
      	switch(ret)
      	{
        case 200:
			retval = UPDATERES_OK;
			break;

        case 201:
			DDNS_DBG("No records need updating.\n");
			retval = UPDATERES_ERROR;
			break;

        case 701:
			DDNS_DBG("Zone is not set up in this account.\n");
			retval = UPDATERES_ERROR;
			break;

        case 702:
			DDNS_DBG("Update failed.\n");
			retval = UPDATERES_ERROR;
			break;

        case 703:
			DDNS_DBG("one of either parameters 'zones' or 'host' are required.\n");
			retval = UPDATERES_ERROR;
			break;

		case 704:
			DDNS_DBG("Zone must be a valid 'dotted' internet name.\n");
			retval = UPDATERES_ERROR;
			break;
			
		case 705:
			DDNS_DBG("Zone cannot be empty.\n");
			retval = UPDATERES_ERROR;
			break;
				
		case 707:
			DDNS_DBG("Too frequent updates for the same host, adjust client settings.\n");
			DDNS_DBG("Duplicate updates for the same host/ip, adjust client settings.\n");
			retval = UPDATERES_ERROR;
			break;		
        default:
			retval = UPDATERES_ERROR;
			break;	
		}
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;


	}

error:
	free(buf);
	return retval;
}
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
static int ORAY_mode;

int ORAY_init_entry(struct user_info *info)
{
	ORAY_mode = 0;
	return 0;
}

int ORAY_update_entry(struct user_info *info)
{
	int rc;
	
	extern int oray_ddns_login(const char *, const char*, const char *, const char *);
	extern int oray_keepalive(void);
	extern int keepalive_ticks;
	
	if (ORAY_mode == 0 || ORAY_mode == 2)
	{
		rc = oray_ddns_login(info->usrname, info->usrpwd, MY_IP_STR, info->host);
		switch (rc)
		{
		case 0:
			DDNS_LOG("update %s with IP %s successfully", info->service->default_server, MY_IP_STR);
			if (ORAY_mode == 0)
				info->updated_time = time(0);//GMTtime(0);
			
			ORAY_mode = 1;
			info->ticks = keepalive_ticks;
			return UPDATERES_OK;
			
		case -1:
			info->ticks = 60 * 100;
			return UPDATERES_ERROR;
			
		default:
			return UPDATERES_SHUTDOWN;
		}
	}
	else
	{
		rc = oray_keepalive();
		switch (rc)
		{
		case 0:
		case -1:
			info->ticks = keepalive_ticks;
			return UPDATERES_OK;
		
		case -2:
		default:
			ORAY_mode = 2;
			info->ticks = 60 * 100;
			return UPDATERES_OK;
		}
	}
	
	return rc;
}


//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int CHANGEIP_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=dyndns&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "wildcard=%s&", info->wildcard ? "ON" : "OFF");
	bp += len;
 	
	if(*(info->mx) != '\0')
	{
		len = sprintf(bp, "mx=%s&", info->mx);
		bp += len;
	}
  	
	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s-%s %s [%s]\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						ROUTER_NAME, FW_VERSION, FW_OS, "",
						info->service->default_server);
 						
	bp += len;
	
	*bp = '\0';
	
	
	output(fd, buf);
	
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "Successful Update!") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			retval = UPDATERES_ERROR;
		}
      break;
		break;

	case 401:
		DDNS_DBG("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int REGFISH_update_entry(struct user_info *info)
{
	char *buf, *bp;
	char *p;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf,	"GET %s?ipv4=%s&authtype=standard&username=%s&password=%s&fqdn=%s", 
						info->service->default_request,
						MY_IP_STR,
						info->usrname,
						info->usrpwd,
						info->host);
	bp += len;
	

	len = sprintf(buf,	" HTTP/1.1\r\n"
						"User-Agent: %s %s - %s\r\n"
						"Host: %s\r\n\r\n",
						ROUTER_NAME, FW_VERSION, FW_OS,
						info->service->default_server);
						
	bp += len;
	
	*bp = '\0';
	output(fd, buf);       		
  
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
	
	switch(ret)
	{
	case -1:
		DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if((p=strstr(buf, "DDNS_Response_")) != NULL)
      	{
         	sscanf(p, "DDNS_Response_%*code=%3d", &ret);
      	}
      	else
      	{
      		retval = UPDATERES_ERROR;
			break;
		}
		
      	switch(ret)
      	{
      	case 100:
        case 101:
			retval = UPDATERES_OK;
			break;

        case 401:
			DDNS_DBG("standard authentication failed: wrong username or password\n");
			retval = UPDATERES_SHUTDOWN;
			break;

        case 402:
			DDNS_DBG("secure authentication failed: wrong security hash\n");
			retval = UPDATERES_SHUTDOWN;
			break;

        case 403:
			DDNS_DBG("standard authentication failed: domain name does not exist\n");
			retval = UPDATERES_SHUTDOWN;
			break;

        case 412:
			DDNS_DBG("invalid hostname\n");
			retval = UPDATERES_SHUTDOWN;
			break;

        default:
			retval = UPDATERES_ERROR;
			break;	
		}
		break;

	default:
		retval = UPDATERES_ERROR;
		break;


	}

error:
	free(buf);
	return retval;
}
int M88IP_update_entry(struct user_info *info)
{
    char *buf, *bp,*temp;
	int bytes, btot, fd, len;
	int retval = UPDATERES_ERROR;
	

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;

	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf,	"GET %s?userid=%s&userpwd=%s HTTP/1.1\r\n"
						"HOST: %s\r\n"
 						"User-Agent: %s\r\n"
						"Connection: close\r\n\r\n", 
						info->service->default_request,
						info->usrname,
						info->usrpwd,
						info->service->default_server,
						ROUTER_NAME
						);
						
	bp += len;
	*bp = '\0';
	output(fd, buf);        		
  
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';

	temp=(char*)strstr(buf,"\r\n\r\n");
	if(!temp){
	    retval=UPDATERES_ERROR;
	    //DDNS_DBG("Server return msg error:%s",buf);
	    goto error;
	}
	if(strncmp(temp+4,"host",4)==0){
	    retval=UPDATERES_OK;
	}else{
	    //DDNS_DBG("msg:%s\n",buf);
	    retval=UPDATERES_ERROR;
	}
error:
	free(buf);
	return retval;
}

int JUSTL_update_entry(struct user_info *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd;
	int len =  0;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;

	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		DDNS_DBG("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	len = sprintf(bp, "GET %s?opcode=ADD&", info->service->default_request);
	bp += len;
	len = sprintf(bp, "%s=%s&", "username", info->usrname);
	bp += len;
	len = sprintf(bp, "%s=%s&", "password", info->usrpwd);
	bp += len;
	len = sprintf(bp, "%s=%s&", "host", info->host);
	bp += len;
	if(MY_IP)
	{
		len = sprintf(bp, "%s=%s&", "ip", MY_IP_STR);
		bp += len;
	}
	len = sprintf(bp, " HTTP/1.0\015\012");
	bp += len;
	len = sprintf(bp, "Authorization: Basic %s\015\012", auth);
	bp += len;
	len = sprintf(bp, "User-Agent: %s-%s %s [%s]\015\012", 
    				ROUTER_NAME, FW_VERSION, FW_OS, "");
    bp += len;  
    len = sprintf(bp, "Host: %s\015\012", info->service->default_server);
	bp += len;
	len = sprintf(bp, "\015\012");
	bp += len;
	
	*bp = '\0';
	output(fd, buf);       		
	bp = buf;
  	bytes = 0;
  	btot = 0;  	 
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
	switch(ret)
  {
    case -1:
    	DDNS_DBG("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

    case 200:
      	if(strstr(buf, "okay") != NULL)
      	{
       		retval = UPDATERES_OK;
			break;
      	}
      	else
      	{
        	DDNS_DBG("error processing request\n");
        	retval = UPDATERES_ERROR;
			break;
      	}
      	break;

    case 401:
      	DDNS_DBG("authentication failure\n");
      	retval = UPDATERES_SHUTDOWN;
      	break;

    default:
    	*auth = '\0';
        sscanf(buf, " HTTP/1.%*c %*3d %255[^\r\n]", auth);
        DDNS_DBG("unknown return code: %d\n", ret);
        DDNS_DBG("server response: %s\n", auth);
	    retval = UPDATERES_ERROR;
    	break;
  }
error:
	free(buf);
	return retval;
}


