/*rtlconnection.c is for creating tcp and udp connections*/
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "rtldebug.h"

int create_connection(char *hostname, int port)
{	
	struct  sockaddr_in addr;
	struct  hostent *host;
	int s;

        if((host = gethostbyname(hostname)) == NULL) {
		DEBUG_ERR("gethostbyname failed\n");
                return -1;
        }
        addr.sin_family =       AF_INET;
        addr.sin_port   =       htons(port);
	addr.sin_addr   =       *(struct in_addr*)host->h_addr;

	s = socket(AF_INET, SOCK_STREAM, 0);
        if(s == -1) {
                DEBUG_ERR("socket failed\n"); 
                return -1;
        }

        if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                DEBUG_ERR("connect() failed\n");
		close(s);
                return -1;
        }

        return s;
}

int create_udpconnection(char *hostname, int port)
{

        struct  sockaddr_in addr;
        struct  hostent *host;
        int s;

        if((host = gethostbyname(hostname)) == NULL) {
                DEBUG_ERR("gethostbyname() failed\n");
                return -1;
        }
        addr.sin_family =       AF_INET;
        addr.sin_port   =       htons(port);

        addr.sin_addr   =       *(struct in_addr*)host->h_addr;

        s = socket(AF_INET, SOCK_DGRAM, 0);
        if(s == -1) {
                DEBUG_ERR("socket() failed\n");
                return -1;
        }

        if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                DEBUG_ERR("connect() failed\n");
		close(s);
                return -1;
        }
        return s;
}
