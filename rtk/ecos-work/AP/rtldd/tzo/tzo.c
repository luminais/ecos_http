/*for tzo dyndns*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "../rtldd.h"
#include "../rtldebug.h"

#define BUF_LEN BUF_SIZE

#define TZOHOST	"xregistry2.tzo.com"
#define TZOPORT		21347

enum Status {
    RET_OK,
    RET_WARNING,
    RET_ERROR,
    RET_WRONG_USAGE,
    RET_RESTART,
    RET_TIMEOUT
};

int build_tzo_request(char *message,struct ddParam *param)
{
        char *username, *passwd;
        int ret;
        char login[128];

        strcpy(login, param->login);
        username = strtok(login, ":");
        passwd = strtok(NULL, "");

        (void)snprintf(message,BUF_LEN, "R %s,%s,%s",
                       param->hostname,
                       username, passwd);

        if(param->ipv4_addr) {
                (void)strncat(message, ",", BUF_LEN-strlen(message));
                (void)strncat(message, param->ipv4_addr, BUF_LEN-strlen(message));
        }		
	 return OK;
}

int send_tzo_request(int sock, char *message,struct ddParam *param)
{
	int ret;
	char *server_msg=malloc(BUF_SIZE);
	if(server_msg==NULL)
		return RET_ERROR;
	(void)memset(server_msg, 0, BUF_SIZE);
	if(read(sock, server_msg, BUF_SIZE-1) == -1) {
	        printf("%s: read() failed",
	                param->hostname);
	        ret = RET_ERROR;
	} else {
	        printf("Server Message: %s", server_msg);
	        if(strstr(server_msg, "TZO/Linksys Update Server")) {
	                printf("Message: %s\n", message);
		    strcat(message,"\r\n");
	                if(write(sock, message, strlen(message)) == -1)
				ret = RET_ERROR;
			else
	              	ret = RET_OK;
	        } else {
	                printf("%s: invalid server",
	                        param->hostname);
	                ret = RET_ERROR;
	        }
	}
	if(server_msg)
		free(server_msg);
	return ret;
}

int verify_servermsg(int sock, char *message, char *hostname)
{
	int ret;
	
	(void)memset(message, 0, BUF_SIZE);
	if((ret = read(sock, message, BUF_SIZE-1)) != -1) {
	    printf("Server Message: %s", message);
		
	    if(strncmp(message, "40", 2) == 0) {
	            ret = RET_OK;
	    } else {
	            ret = RET_ERROR;
	    }
	} else {
	   printf("%s: read() failed",
	            hostname);
	    ret = RET_ERROR;
	}
	return ret;

}

int register_tzo(int argc, char *argv[])
{
        int sock;
        int ret;
        DDPARAM_T param;
        char* message=malloc(BUF_LEN);
	 if(message == NULL)
	 	return RET_ERROR;
        memset(&param,0,sizeof(param));
	 memset(message,0,BUF_LEN);
	/*default value*/

        /*parse command line*/
       parse_cmd(&param,argc,argv);

        /*connect to server*/
        sock=create_connection(TZOHOST,TZOPORT);
        if(-1 == sock)
        {
                DEBUG_ERR("connect to server failed\n");
        }
        /*send request*/
        build_tzo_request(message,&param);
        ret=send_tzo_request(sock,message,&param);
        /*verify feedback*/
	 if(RET_OK == ret)
        	ret=verify_servermsg(sock,message,param.hostname);

        if(sock)
                close(sock);
	 if(message)
	 	free(message);
        return ret;	
}
