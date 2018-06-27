#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

#include "elink_log_debug.h"



/**************************************************

	Server socket operate .

**************************************************/

int socket_set_timeout(int sock)
{
	int ret;
	struct timeval tv;

	tv.tv_sec = 20;
	tv.tv_usec = 0;

	ret = 0;
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
		(char*)&tv, sizeof(tv));
	if (ret != 0)
	{		
		log_msg(LOG_LEVEL_ERROR , LOG_TRACE_STRING);
		return -1;
	}

	ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, 
		(char*)&tv, sizeof(tv));
	if (ret != 0)
	{
		log_msg(LOG_LEVEL_ERROR , LOG_TRACE_STRING);
		return -1;
	}
	
	log_msg(LOG_LEVEL_ERROR , LOG_TRACE_STRING);
	
	return 0;
}

int socket_send(int sock, unsigned char *data, int len)
{
	log_trace("Send data[%d]:", len);
	print_package(data, len);

	return send(sock, data, len , 0);
}

int socket_receive(int sock, unsigned char *data, int len)
{
	int ret = 0;

	log_msg(LOG_LEVEL_TRACE, LOG_TRACE_STRING);

	do 
	{
		ret = recv(sock, data, len, 0);
	} while (ret < 0 && errno == EINTR);

	if(ret <= 0)
	{
		log_msg(LOG_LEVEL_DEBUG, "%s : Recvfrom error happen.\n", __FUNCTION__);
		return -1;
	}
    
	return ret ;
}


int socket_receive_full(int sock, unsigned char *data, int len)
{
	int cc = 0;
	int total = 0;
	char *buf = data;

	while (len) 
	{
		do 
		{
			cc = recv(sock, buf, len, 0);
		} while (cc < 0 && errno == EINTR);

		if(cc == 0)
		{
			perror("server_socket_recvice:recvfrom");
			log_msg(LOG_LEVEL_DEBUG, "%s :	(peer close.) happen[ret=%d].\n", __FUNCTION__, cc);
			return -1;
		}
		else if(cc < 0)
		{
			perror("server_socket_recvice:recvfrom");
			log_msg(LOG_LEVEL_DEBUG, "%s : Recvfrom error happen[ret=%d].\n", __FUNCTION__, cc);
			log_debug("errno:%d\n",errno);
			return -1;
		}
		
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	log_msg(LOG_LEVEL_DEBUG,"server_socket_full_recvice : [recv package length : %d]\n", total);
	
	return total;
}

int socket_select(int sock, int timeout)
{
	fd_set rfds;
	struct timeval tv;
    int ret = 0;

	FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    ret = select(sock + 1, &rfds, NULL, NULL, &tv);
    switch(ret)
    {
    	case -1:
			log_debug("%s : error, ret=%d\n", __FUNCTION__, ret);
            return ret;
		case 0:
            log_debug("%s : timeout , ret=%d\n", __FUNCTION__, ret);
        	return ret;
        default:
            if(FD_ISSET(sock, &rfds))
                return 1;
            return 0;
    }
}


int socket_init(void)
{
	int sock;
    const int optval = 1;

	/* If debug version, we use UDP */
	sock = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM, SOCK_DGRAM
	if(sock < 0)
	{
		log_msg(LOG_LEVEL_ERROR , "Create server socket error.");
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) 
	{
		log_msg(LOG_LEVEL_ERROR , "Set server socket : SO_REUSEADDR error.");
		return -1;
	}	

	//socket_set_timeout(5);
    
	log_msg(LOG_LEVEL_ALERT , "%s : Success.\n", __FUNCTION__);
		
	return sock;

}

int socket_connect(int sock, char *ip, int port)
{
	struct sockaddr_in sa;
	struct in_addr in;
	int addrLen = 0;
	int ret = 0;
	struct hostent *host = NULL;
	static char dst[32] = {0};

	assert(ip);
		
	if(!Valid_Ipv4_Addr(ip))
	{
		struct hostent *host;
		struct in_addr serverip;
		int ret;			
		host = gethostbyname(ip);
		if(host !=NULL)
		{
	 		if( host->h_addrtype != AF_INET) {
	 			diag_printf("Error address\n");
				return -1;
	 		}		
			else
		 		memcpy(&in.s_addr, host->h_addr, sizeof(in.s_addr));
		}
	}else
		in.s_addr = inet_addr(ip);

	sa.sin_family = AF_INET;
	sa.sin_addr = in;
	sa.sin_port = htons(port);
	addrLen = sizeof(sa);
	ret = connect(sock, (struct sockaddr *)&sa, addrLen);
	if(ret < 0)
	{
		log_msg(LOG_LEVEL_ERROR , "Connect to server error.\n");
		return -1;
	}	
		
	return 0;
}


int socket_close(int sock)
{
	if(sock > 0)
		close(sock);
    return 0;
}

