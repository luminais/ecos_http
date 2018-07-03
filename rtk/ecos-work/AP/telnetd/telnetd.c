#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/kernel/ktypes.h>         // base kernel types.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>                     // sprintf().
#include <time.h>                      // sprintf().
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
////#include <stdio.h>
//#include <stdlib.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
////#include <string.h>
//#include <errno.h>
////#include <fcntl.h>
//void active_keepalive(int sockfd);
//void set_keepalive_params(int sockfd, int idle, int count, int intvl);

#include <arpa/telnet.h>
#include "common.h"

#ifdef HAVE_TELNET_USERNAME_PASSWORD
#include "apmib.h"
#endif

#define TELNETD_THREAD_PRIORITY 16
#define TELNETD_THREAD_STACK_SIZE 0x00005000
#define CYG_NET_TELNET_SERVEROPT_PORT 23
#define TELNET_MAX_CONN 2
//#define INPUT_BUFFER_SIZE 1024
#define GET_INPUT_BUFFER_SIZE 128
static unsigned int INPUT_BUFFER_SIZE = 128;
#define OUT_BUFFER_SIZE 2048
static char *errorstring = "an error happened,please try again later\r\n#";

static char telnetd_started=0;
static char telnetd_running=0;
static int force_close = 0;
static int timeout = 60;
#ifdef HAVE_SYSTEM_REINIT
static char telnetd_cleaning=0;
#endif
cyg_uint8  telnetd_stack[TELNETD_THREAD_STACK_SIZE];
cyg_handle_t telnetd_thread;
cyg_thread  telnetd_thread_object;

static char telnetd_enable=1;
typedef enum {
	CONN_STATUS_UNUSE=0,
	CONN_STATUS_CONNCTED=1,
	CONN_STATUS_EXIT=2
} CONN_STATUS_T;
typedef struct connect_session_{
	cyg_int32 connfd;
	CONN_STATUS_T conn_status;
	int out_len;
	int in_len;
	int cmd_len;
	unsigned char *input_buffer;
	unsigned char *input_cmd_save;
	unsigned char out_buffer[OUT_BUFFER_SIZE];	
} CONN_SESSION_T,*CONN_SESSION_Tp;

int CONNOP_send(CONN_SESSION_Tp pConn,char* buffer,int len)
{
	if(!pConn||!buffer) 
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
    if(len > OUT_BUFFER_SIZE)
    {
        printf("CONNOP_send too much data to CONN_SESSION\n");
    }
    if(pConn->out_len + len > OUT_BUFFER_SIZE)
    {
        CONNOP_flush(pConn);
    }
	memcpy(pConn->out_buffer+pConn->out_len,buffer,len);
	pConn->out_len+=len;
    return 0;
}
int CONNOP_flush(CONN_SESSION_Tp pConn)
{
	if(!pConn||!pConn->connfd)
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
	write(pConn->connfd,pConn->out_buffer,pConn->out_len);
	bzero(pConn->out_buffer,sizeof(pConn->out_buffer));
	pConn->out_len=0;
    return 0;
}
int CONNOP_flush_error(CONN_SESSION_Tp pConn)
{
	if(!pConn||!pConn->connfd)
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
	write(pConn->connfd,errorstring,strlen(errorstring));
	bzero(pConn->out_buffer,sizeof(pConn->out_buffer));
	pConn->out_len=0;
    return 0;
}
int CONNOP_get(CONN_SESSION_Tp pConn)
{
	if(!pConn||!pConn->connfd)
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
    struct timeval expired;
    int maxfd = 0;
    int ret = 0;
	int timePassed=0;
    expired.tv_sec = 1;
    expired.tv_usec = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pConn->connfd, &readfds);
    maxfd = pConn->connfd;
	while(1)
	{
#ifdef HAVE_SYSTEM_REINIT
		if(telnetd_cleaning)
				return -1;
#endif
	    if (( ret =  select(maxfd + 1, &readfds, NULL, NULL, &expired)) < 0) 
	    {
	        printf("select error\n");
	        return -1;
	    }else if(ret == 0){
	        timePassed++;
/* modify by jack;2016-07-16; no timeout */
#if 0
			if(timePassed>=timeout)
			{
				printf("time out!\n");
	        	return -1;
			}
#endif
	    }
		else
			break;
	}
	//bzero(pConn->input_buffer,sizeof(pConn->input_buffer));
    if(FD_ISSET(pConn->connfd,&readfds))
    {
        bzero(pConn->input_buffer,GET_INPUT_BUFFER_SIZE);
        pConn->in_len=read(pConn->connfd,pConn->input_buffer,GET_INPUT_BUFFER_SIZE);
        if(pConn->in_len < 0)
        {
            printf("read error\n");
            return -1;
        }else if (pConn->in_len ==0 ){
            printf("client close socket. \n");
        }else
            return 0;
    }
    return -1;
}

int CONNOP_sendChar(CONN_SESSION_Tp pConn,char char_data)
{
	if(!pConn)
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
    if( CONNOP_send(pConn,&char_data,1) < 0) 
    {
        return -1;
    }
    return 0;
}
int CONNOP_saveCmdChar(CONN_SESSION_Tp pConn,char char_data)
{
	if(!pConn)
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
	if(pConn->cmd_len+2>=INPUT_BUFFER_SIZE)
	{
        INPUT_BUFFER_SIZE = 2*INPUT_BUFFER_SIZE;
        //if(INPUT_BUFFER_SIZE > OUT_BUFFER_SIZE)
        //{
        //    fprintf(stderr,"cmd char over size! INPUT_BUFFER SIZE bigger than OUT_BUFFER_SIZE\n");
        //    return -1;
        //}
        unsigned char *ptr = NULL;
        ptr = malloc(INPUT_BUFFER_SIZE);
        if(!ptr) 
        {
            fprintf(stderr,"cmd char over size!\n");
            return -1;
        }
        bzero(ptr,INPUT_BUFFER_SIZE);
        //input_cmd_save[cmd_len] always '\0',so need not copy
        memcpy(ptr,pConn->input_cmd_save,pConn->cmd_len);
        free(pConn->input_cmd_save);
        pConn->input_cmd_save=ptr; 
		//fprintf(stderr,"cmd char over size!\n");
        //return -1;
	}
	pConn->input_cmd_save[pConn->cmd_len++]=char_data;
	pConn->input_cmd_save[pConn->cmd_len]='\0';
    //if(pConn->cmd_len + 2 > OUT_BUFFER_SIZE)
    //{
    //    fprintf(stderr,"cmd char over size! bigger than OUT_BUFFER_SIZE\n");
    //    return -1;
    //}
    return 0;
}

int CONNOP_bsCmdChar(CONN_SESSION_Tp pConn)//back
{
	if(!pConn)
    {
		fprintf(stderr,"do not apply pConn and buffer\n");
        return -1;
    }
	pConn->input_cmd_save[--pConn->cmd_len]=0;
    return 0;
}
int send_iac(CONN_SESSION_Tp pConn,unsigned char command, unsigned char option)
{
	if( CONNOP_sendChar(pConn,IAC) < 0)
    {
        return -1;
    }
	if( CONNOP_sendChar(pConn,command) < 0)
    {
        return -1;
    }
	if( CONNOP_sendChar(pConn,option) < 0)
    {
        return -1;
    }
    return 0;
}
#if 0
void output_telnet(char* head,unsigned char* data)
{
	int indx=0;
	char buffer_out[OUT_BUFFER_SIZE]={0};
	char temp[64]={0};
	if(!head||!data) return;
	while(data[indx])
	{
		switch(data[indx])
		{
			case IAC:
				strcat(buffer_out,":IAC ");
				indx++;
				switch(data[indx])
				{
					case DO:
						strcat(buffer_out,"DO ");
						break;
					case DONT:
						strcat(buffer_out,"DONT ");
						break;
					case WILL:
						strcat(buffer_out,"WILL ");
						break;
					case WONT:
						strcat(buffer_out,"WONT ");
						break;					
					default:
						sprintf(temp,"UNKNOWN %d ",data[indx]);
						strcat(buffer_out,temp);
						break;
				}
				indx++;	
				switch(data[indx])
				{
					case TELOPT_ECHO:
						strcat(buffer_out,"TELOPT_ECHO ");
						break;
					case TELOPT_SGA:
						strcat(buffer_out,"TELOPT_SGA ");
						break;
					case TELOPT_STATUS:
						strcat(buffer_out,"TELOPT_STATUS ");
						break;
					case TELOPT_TM:
						strcat(buffer_out,"TELOPT_TM ");
						break;
					default:
						sprintf(temp,"UNKNOWN %d ",data[indx]);
						strcat(buffer_out,temp);
						break;
				}
				indx++;	
				strcat(buffer_out,"\n");
				break;
			default:
				sprintf(temp,"%c",data[indx]);
				strcat(buffer_out,temp);
				indx++;
				break;
		}
	}
	printf("%s %s",head,buffer_out);
}
#endif

int ChangeNtoRN(unsigned char * buffer,const int maxLen)
{
	unsigned char * tmpBuffer=(unsigned char*)malloc(maxLen);
	int origIdx=0,dstIdx=0;
	if(!tmpBuffer)
	{
		fprintf(stderr,"malloc (%d)fail!\n",maxLen);
		return -1;
	}
	if(!buffer)
    {
        free(tmpBuffer);
        return -1;
    }
	bzero(tmpBuffer,maxLen);
	while(buffer[origIdx])
	{
		if(buffer[origIdx]=='\n')
			tmpBuffer[dstIdx++]='\r';
		tmpBuffer[dstIdx++]=buffer[origIdx++];		
		//fprintf(stderr,"tmpBuffer=%s\nbuffer=%s!\n",tmpBuffer,buffer);
		if(dstIdx>=maxLen)
		{
			fprintf(stderr,"buffer(%d) oversize(%d)!\n",maxLen,dstIdx);
			free(tmpBuffer);
			return -1;
		}
	}
	memcpy(buffer,tmpBuffer,dstIdx);
	free(tmpBuffer);
	return dstIdx;
}
int run_cmd(CONN_SESSION_Tp pConn,unsigned char * input_buffer)
{
	if(!strcmp(pConn->input_cmd_save,"exit"))
	{
		if( CONNOP_send(pConn,"\r\n",2) < 0)
        {
            return -1;
        }
		if( CONNOP_send(pConn,"exit telnet!\r\n",sizeof("exit telnet!\r\n")) < 0)
        {
            return -1;
        }
		pConn->conn_status=CONN_STATUS_EXIT;		
	}
	else if(!strcmp(pConn->input_cmd_save,""))
	{
		if( CONNOP_send(pConn,"\r\n#",3) < 0)
        {
            return -1;
        }
	}
	else
	{
		int cmdOutLen;
		unsigned char *cmdOutPut=(unsigned char *)malloc(OUT_BUFFER_SIZE);
		if(!cmdOutPut)
        {
            return -1;
        }
		bzero(cmdOutPut,OUT_BUFFER_SIZE);
		cmdOutLen=getShellOutput_telnetd(pConn->input_cmd_save,cmdOutPut,OUT_BUFFER_SIZE/2);	
		if(cmdOutLen<0) 
		{
			free(cmdOutPut);
            return -1;
		}
        int readSize = cmdOutLen;
        int i = 0;
//		if( CONNOP_send(pConn,"\r\n",2) < 0)
//        {
//            return -1;
//        }
        while(readSize == OUT_BUFFER_SIZE/2 - 1)
        {
            i++; 
            FILE *fileTmp=NULL;
            cmdOutLen = ChangeNtoRN(cmdOutPut,OUT_BUFFER_SIZE);
            if(cmdOutLen < 0){
                if( CONNOP_flush_error(pConn) < 0)
                {
                    return -1;
                }
                free(cmdOutPut);
                return -1;
            }

            if( CONNOP_send(pConn,cmdOutPut,cmdOutLen) < 0)
            {
                return -1;
            }
			if( CONNOP_flush(pConn) < 0)
            {
                return -1;
            }
            bzero(cmdOutPut,OUT_BUFFER_SIZE);
            if((fileTmp=fopen(_PATH_TMP_LOG,"r"))==NULL)
            {
                free(cmdOutPut);
                diag_printf("open output file fail!%s\n",_PATH_TMP_LOG);
                return -1;
            }
            if(fseek(fileTmp,(OUT_BUFFER_SIZE/2 - 1) * i,SEEK_SET) < 0 )
            {
                free(cmdOutPut);
                diag_printf("lseek output file fail!%s\n",_PATH_TMP_LOG);
                return -1;
            }
            readSize=fread(cmdOutPut,1,OUT_BUFFER_SIZE/2 - 1,fileTmp);
            fclose(fileTmp);
        }
		cmdOutLen=ChangeNtoRN(cmdOutPut,OUT_BUFFER_SIZE);
        if(cmdOutLen < 0){
            if( CONNOP_flush_error(pConn) < 0)
            {
                return -1;
            }
            free(cmdOutPut);
            return -1;
        }
        if( CONNOP_send(pConn,cmdOutPut,cmdOutLen) < 0)
        {
            return -1;
        }
        if( CONNOP_flush(pConn) < 0)
        {
            return -1;
        }



        //stderr***********************************************
        FILE *fileTmp0=NULL;
        int readSize0 = 0;
		bzero(cmdOutPut,OUT_BUFFER_SIZE);
        if((fileTmp0=fopen(_PATH_TMP_LOG0,"r"))==NULL)
        {
            diag_printf("open output file fail!%s\n",_PATH_TMP_LOG0);
            return -1;
        }
        readSize0=fread(cmdOutPut,1,OUT_BUFFER_SIZE/2 - 1,fileTmp0);
        fclose(fileTmp0);
		if(readSize0<0) 
		{
			free(cmdOutPut);
            return -1;
		}
        int ii=0;
        int cmdOutLen0 = 0;
        while(readSize0 == OUT_BUFFER_SIZE/2 - 1)
        {
            ii++; 
            FILE *fileTmp=NULL;
            cmdOutLen0 = ChangeNtoRN(cmdOutPut,OUT_BUFFER_SIZE);
            if(cmdOutLen0 < 0){
                if( CONNOP_flush_error(pConn) < 0)
                {
                    return -1;
                }
                free(cmdOutPut);
                return -1;
            }

            if( CONNOP_send(pConn,cmdOutPut,cmdOutLen0) < 0)
            {
                return -1;
            }
			if( CONNOP_flush(pConn) < 0)
            {
                return -1;
            }
            bzero(cmdOutPut,OUT_BUFFER_SIZE);
            if((fileTmp=fopen(_PATH_TMP_LOG0,"r"))==NULL)
            {
                free(cmdOutPut);
                diag_printf("open output file fail!%s\n",_PATH_TMP_LOG0);
                return -1;
            }
            if(fseek(fileTmp,(OUT_BUFFER_SIZE/2 - 1) * ii,SEEK_SET) < 0 )
            {
                free(cmdOutPut);
                diag_printf("lseek output file fail!%s\n",_PATH_TMP_LOG0);
                return -1;
            }
            readSize0 = fread(cmdOutPut,1,OUT_BUFFER_SIZE/2 - 1,fileTmp);
            fclose(fileTmp);
        }
		cmdOutLen0=ChangeNtoRN(cmdOutPut,OUT_BUFFER_SIZE);
        if(cmdOutLen0 < 0){
            if( CONNOP_flush_error(pConn) < 0)
            {
                return -1;
            }
            free(cmdOutPut);
            return -1;
        }
        if( CONNOP_send(pConn,cmdOutPut,cmdOutLen0) < 0)
        {
            return -1;
        }
        //stderr************************************************


		if( CONNOP_send(pConn,"\r\n#",3) < 0)
        {
            return -1;
        }
		free(cmdOutPut);
    }
    //bzero(pConn->input_cmd_save,sizeof(pConn->input_cmd_save));
    bzero(pConn->input_cmd_save,INPUT_BUFFER_SIZE);
    pConn->cmd_len=0;

    //if((pConn->cmd_len < 256)&&(INPUT_BUFFER_SIZE > 128))
    if((INPUT_BUFFER_SIZE - pConn->cmd_len - 1 >= INPUT_BUFFER_SIZE/2 )&&(INPUT_BUFFER_SIZE > 128))
    //if(INPUT_BUFFER_SIZE - pConn->cmd_len - 1 >= INPUT_BUFFER_SIZE/2 )
    {
        INPUT_BUFFER_SIZE = INPUT_BUFFER_SIZE/2;
        unsigned char *ptr = NULL;
        ptr = malloc(INPUT_BUFFER_SIZE);
        if(!ptr) 
        {
            fprintf(stderr,"malloc fail!\n");
            return -1;
        }
        bzero(ptr,INPUT_BUFFER_SIZE);
        free(pConn->input_cmd_save);
        pConn->input_cmd_save=ptr; 
    }

    return 0;
}

#ifdef HAVE_TELNET_USERNAME_PASSWORD
int validate_username_password(CONN_SESSION_Tp pConn)
{
	char usernameOfDUT[MAX_NAME_LEN+2]={0}; //username of router
	char passwordOfDUT[MAX_NAME_LEN+2]={0}; //password of router
	int usernameOfDUT_Length=0;
	int passwordOfDUT_Length=0;
	
	char * usernamePrompt = "please input your username:\r\n";
	char * passwordPrompt = "please input your password:\r\n";
	char * invalidationPrompt = "username or password is error!\r\n";
	int isNeedValidation=0;              //0: don't need validation; 1: need validation

	FILE *fp = NULL;                     //read username and password from file saved in router
	if((fp = fopen(TELNET_USERNAME_PASSWORD_FILE,"r")) == NULL) //check the file whether exist and readable
	{
		printf("file open error!");
		return -1;
	}
	fgets(usernameOfDUT,MAX_NAME_LEN+2,fp); //read  router's username
	fgets(passwordOfDUT,MAX_NAME_LEN+2,fp); //read  router's password
	fclose(fp); 				          //close file pointer
	
	usernameOfDUT_Length=strlen(usernameOfDUT)-1;
	passwordOfDUT_Length=strlen(passwordOfDUT)-1;
	if(usernameOfDUT_Length==0 || passwordOfDUT_Length==0)
	{
		isNeedValidation=0;//0: don't need validation
	}
	else
	{
		isNeedValidation=1;//1: need validation
	}

	if(isNeedValidation == 1)//begin validate username and password
	{
		while(1)
		{
			char usernameInput[MAX_NAME_LEN]={0}; //username input by user
			char passwordInput[MAX_NAME_LEN]={0}; //password input by user
			int usernameInput_Length=0;
			int passwordInput_Length=0;
			int isPassValidation=1;  //0: don't pass validation; 1: pass validation
			int is_contain_r=-1;     //if  is_contain_r>=0, pConn->input_buffer contain '\r' and  is_contain_r point to the position of '\r'
			if(CONNOP_send(pConn,usernamePrompt,strlen(usernamePrompt)) == 0)
			{
				CONNOP_flush(pConn);
			}
			else
			{
				printf("Error,when send username prompt message!\n");
				return -1;
			}
			while(1)//read character of username form console until get character '\r'
			{
				if(CONNOP_get(pConn) == -1)//time out or read error!
				{
					if(CONNOP_send(pConn,"time out!",strlen("time out!")) == 0)
					{
						CONNOP_flush(pConn);
					}
					else
					{
						printf("Error,when send 'time out' prompt message!\n");
					}
					return -1;
				}

				int i=0;
				for(i=0; i < pConn->in_len; i++)
				{
					if(pConn->input_buffer[i] == '\r')
					{
						is_contain_r=i;
						break;
					}
				}
				if(is_contain_r >= 0)
				{
					for(i=0;i < is_contain_r;i++)
					{
						usernameInput[usernameInput_Length]=pConn->input_buffer[i];
						usernameInput_Length++;
					}
					break;
				}
				else
				{
					for(i=0; i < pConn->in_len; i++)
					{
						usernameInput[usernameInput_Length]=pConn->input_buffer[i];
						usernameInput_Length++;
					}
				}
			}

			is_contain_r=-1;
			if(CONNOP_send(pConn,passwordPrompt,strlen(passwordPrompt)) == 0)
			{
				CONNOP_flush(pConn);
			}
			else
			{
				printf("Error,when send user name prompt message!\n");
				return -1;
			}

			while(1)//read character of password form console until get character '\r'
			{
				if(CONNOP_get(pConn) == -1)//time out or read error!
				{
					if(CONNOP_send(pConn,"time out!",strlen("time out!")) == 0)
					{
						CONNOP_flush(pConn);
					}
					else
					{
						printf("Error,when send 'time out' prompt message!\n");
					}
					return -1;
				}

				int i=0;
				for(i=0;i < pConn->in_len; i++)
				{
					if(pConn->input_buffer[i] == '\r')
					{
						is_contain_r=i;
						break;
					}
				}
				if(is_contain_r >= 0)
				{
					for(i=0;i < is_contain_r;i++)
					{
						passwordInput[passwordInput_Length]=pConn->input_buffer[i];
						passwordInput_Length++;
					}
					break;
				}
				else
				{
					for(i=0; i < pConn->in_len; i++)
					{
						passwordInput[passwordInput_Length]=pConn->input_buffer[i];
						passwordInput_Length++;
					}
				}
			}

			if(usernameInput_Length != usernameOfDUT_Length || passwordInput_Length != passwordOfDUT_Length)//verify username and password
			{
				isPassValidation = 0;
			}
			else
			{
				if(strncmp(usernameInput,usernameOfDUT,usernameInput_Length) != 0 || strncmp(passwordInput,passwordOfDUT,passwordInput_Length) != 0)
				{
					isPassValidation = 0;
				}
			}
			if(isPassValidation == 0)//don't pass verification
			{
				if(CONNOP_send(pConn,invalidationPrompt,strlen(invalidationPrompt)) == 0)
				{
					CONNOP_flush(pConn);
				}
				else
				{
					printf("Error,when send invalidation prompt message!\n");
					return -1;
				}
			}
			else //isPassValidation == 1, pass verification
			{
				break;
			}
		}
	}
	return 0;
}
#endif
int run_telnetd(cyg_int32 connfd)
{
	int input_buffer_idx=0,in_len=0;
	unsigned char * input_buffer=NULL;
	CONN_SESSION_Tp pConn= (CONN_SESSION_Tp)malloc(sizeof(CONN_SESSION_T));
	if(!pConn) 
    {
        free(pConn);
        return -1;
    }
	bzero(pConn,sizeof(CONN_SESSION_T));
    INPUT_BUFFER_SIZE = 128;
    pConn->input_buffer = malloc(GET_INPUT_BUFFER_SIZE);
    if(!pConn->input_buffer) return -1;
    bzero(pConn->input_buffer,GET_INPUT_BUFFER_SIZE);

    pConn->input_cmd_save= malloc(INPUT_BUFFER_SIZE);
    if(!pConn->input_cmd_save) return -1;
    bzero(pConn->input_cmd_save,INPUT_BUFFER_SIZE);

	pConn->connfd=connfd;
	pConn->conn_status=CONN_STATUS_CONNCTED;

//-----------------add user name and password validation-----------------
#ifdef HAVE_TELNET_USERNAME_PASSWORD
	if(validate_username_password(pConn) == -1)
	{
		return -1;
	}
#endif
//---------------end of add user name and password validation-------------
	if( send_iac(pConn,WILL, TELOPT_ECHO) < 0)
    {
        free(pConn->input_buffer);
        free(pConn->input_cmd_save);
        free(pConn);
        return -1;
    }
	if( send_iac(pConn,WILL, TELOPT_SGA) < 0)
    {
        free(pConn->input_buffer);
        free(pConn->input_cmd_save);
        free(pConn);
        return -1;
    }
	if( CONNOP_flush(pConn) < 0)
    {
        free(pConn->input_buffer);
        free(pConn->input_cmd_save);
        free(pConn);
        return -1;
    }

	while(1)
	{
	
#ifdef HAVE_SYSTEM_REINIT
		if(telnetd_cleaning)
				break;
#endif
        force_close = 1;
		input_buffer_idx=0;
        if( CONNOP_get(pConn) < 0)
        {
            free(pConn->input_buffer);
            free(pConn->input_cmd_save);
            free(pConn);
            return -1;
        }
		input_buffer=pConn->input_buffer;
		in_len=pConn->in_len;
		while(telnetd_enable&&input_buffer_idx<in_len)
		{
            force_close = 0;
			switch(input_buffer[input_buffer_idx])
			{
				case IAC:					
					switch(input_buffer[input_buffer_idx+1])
					{
						case DONT:
							switch(input_buffer[input_buffer_idx+2])
							{
								case TELOPT_SGA:
									fprintf(stderr,"client not support TELOPT_SGA!!\n");
									return -1;	
								case TELOPT_ECHO:
									fprintf(stderr,"client not support echo!!\n");
									return -1;
								default:
									if( send_iac(pConn,WONT,input_buffer[input_buffer_idx+2]) < 0)
                                    {
                                        free(pConn->input_buffer);
                                        free(pConn->input_cmd_save);
                                        free(pConn);
                                        return -1;
                                    }
									input_buffer_idx+=3;								
									break;
							}
							break;
						case DO:
							switch(input_buffer[input_buffer_idx+2])
							{
								case TELOPT_SGA:
									//printf("TELOPT_SGA OK!\n");
									if( CONNOP_send(pConn,"server SGA OK!\r\n#",strlen("server SGA OK!\r\n#")) < 0)
                                    {
                                        free(pConn->input_buffer);
                                        free(pConn->input_cmd_save);
                                        free(pConn);
                                        return -1;
                                    }
									input_buffer_idx+=3;
									break;
								case TELOPT_ECHO:
									//printf("TELOPT_ECHO OK!\n");
									input_buffer_idx+=3;
									if( CONNOP_send(pConn,"server echo ok!\r\n",strlen("server echo ok!\r\n")) < 0)
                                    {
                                        free(pConn->input_buffer);
                                        free(pConn->input_cmd_save);
                                        free(pConn);
                                        return -1;
                                    }
									break;
								default:
									if( send_iac(pConn,WONT,input_buffer[input_buffer_idx+2]) < 0)
                                    {
                                        free(pConn->input_buffer);
                                        free(pConn->input_cmd_save);
                                        free(pConn);
                                        return -1;
                                    }
									input_buffer_idx+=3;
									break;
							}
							break;
						case WONT:
							switch(input_buffer[input_buffer_idx+2])
							{
								//case TELOPT_ECHO:
									//fprintf(stderr,"client not support echo!!\n");
									//return -1;
								default:
									if( send_iac(pConn,DONT,input_buffer[input_buffer_idx+2]) < 0)
                                    {
                                        free(pConn->input_buffer);
                                        free(pConn->input_cmd_save);
                                        free(pConn);
                                        return -1;
                                    }
									input_buffer_idx+=3;
									break;
							}
							break;
						case WILL:
							switch(input_buffer[input_buffer_idx+2])
							{
								/*case TELOPT_NAWS:
									if( send_iac(pConn,DONT,TELOPT_NAWS) < 0)
                                    {
                                    return -1;
                                    }
									input_buffer_idx+=3;
									break;
									break;*/
								default:
									if( send_iac(pConn,DONT,input_buffer[input_buffer_idx+2]) < 0)
                                    {
                                        free(pConn->input_buffer);
                                        free(pConn->input_cmd_save);
                                        free(pConn);
                                        return -1;
                                    }
									input_buffer_idx+=3;
									break;
							}
							break;
						case IAC:
							if( CONNOP_sendChar(pConn,IAC) < 0)
                            {
                                free(pConn->input_buffer);
                                free(pConn->input_cmd_save);
                                free(pConn);
                                return -1;
                            }
							input_buffer_idx+=2;
							break;						
						default:
							input_buffer_idx+=2;
							break;
					}
					break;
				case '\r':
					if( run_cmd(pConn,input_buffer) < 0)
                    {
                        free(pConn->input_buffer);
                        free(pConn->input_cmd_save);
                        free(pConn);
                        return -1;
                    }
					if((input_buffer[input_buffer_idx+1]=='\n' ) ||(input_buffer[input_buffer_idx+1]=='\0'))
						input_buffer_idx+=2;
					else
						input_buffer_idx++;
					//if(input_buffer[input_buffer_idx+1]=='\n')
					break;
				case '\n':
					input_buffer_idx++;
					break;
				case '\b':
					if( CONNOP_bsCmdChar(pConn) < 0)
                    {
                        free(pConn->input_buffer);
                        free(pConn->input_cmd_save);
                        free(pConn);
                        return -1;
                    }
					if( CONNOP_sendChar(pConn,input_buffer[input_buffer_idx]) < 0)
                    {
                        free(pConn->input_buffer);
                        free(pConn->input_cmd_save);
                        free(pConn);
                        return -1;
                    }
					input_buffer_idx++;
					break;
				default://data
					if( CONNOP_sendChar(pConn,input_buffer[input_buffer_idx]) < 0)
                    {
                        free(pConn->input_buffer);
                        free(pConn->input_cmd_save);
                        free(pConn);
                        return -1;
                    }
					if( CONNOP_saveCmdChar(pConn,input_buffer[input_buffer_idx]) < 0)
                    {
                        free(pConn->input_buffer);
                        free(pConn->input_cmd_save);
                        free(pConn);
                        return -1;
                    }
					input_buffer_idx++;
					break;
			}//end of switch
		}//end of analy
		if(pConn->out_len)
		{
			if( CONNOP_flush(pConn) < 0)
            {
                free(pConn->input_buffer);
                free(pConn->input_cmd_save);
                free(pConn);
                return -1;
            }
		}
		if(!telnetd_enable)
		    in_len = 0;
        if((in_len == 0)&&(force_close == 1))
        {
            if( CONNOP_send(pConn,"\r\n",2) < 0)
            {
                free(pConn->input_buffer);
                free(pConn->input_cmd_save);
                free(pConn);
                return -1;
            }
            if( CONNOP_send(pConn,"exit telnet!\r\n",sizeof("exit telnet!\r\n")) < 0)
            {
                free(pConn->input_buffer);
                free(pConn->input_cmd_save);
                free(pConn);
                return -1;
            }
            pConn->conn_status=CONN_STATUS_EXIT;		
        }
        if(pConn->conn_status==CONN_STATUS_EXIT)
			break;
	}//end of while(1)
    free(pConn->input_buffer);
    free(pConn->input_cmd_save);
    free(pConn);
    return 0;
}
int telnetd_main(cyg_addrword_t data)
{
	cyg_int32 rc=0,listener=0,yes=1,connfd=0;
	struct sockaddr_in server_conn={0},client_conn={0};
	socklen_t connlen=0;
//	//CONN_SESSION_T conSession;
//	unsigned char input_cmd_store[INPUT_BUFFER_SIZE]={0};
//	unsigned char input_buffer[INPUT_BUFFER_SIZE]={0};
//	unsigned char out_buffer[OUT_BUFFER_SIZE]={0};	
//	//int cmd_len=0;
    init_all_network_interfaces();
	listener = socket(AF_INET, SOCK_STREAM, 0);	
	CYG_ASSERT(listener > 0, "Socket create failed");
    if (listener < 0) {
        return -1;
    }
    rc = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1) {
		close(listener);
        return -1;
    }

	server_conn.sin_family = AF_INET;
	server_conn.sin_addr.s_addr = INADDR_ANY;
	server_conn.sin_port = htons(CYG_NET_TELNET_SERVEROPT_PORT);

	rc = bind(listener,
              (struct sockaddr *)&server_conn, 
              sizeof(struct sockaddr)); 
    CYG_ASSERT(rc == 0, "bind() returned error");
    if (rc != 0) {
		close(listener);
        return -1;
    }

    rc = listen(listener, TELNET_MAX_CONN);
    CYG_ASSERT(rc == 0, "listen() returned error");
    if (rc != 0) {
		close(listener);
        return -1;
    }

	
	connlen=sizeof(client_conn);
	while(1)
	{
#ifdef HAVE_SYSTEM_REINIT
		if(telnetd_cleaning)
			break;
		connfd=accept_timeout(listener,&client_conn,&connlen,1);
		if(connfd == -ETIMEDOUT)
		{
			continue;
		}
#else
		connfd=accept(listener,&client_conn,&connlen);
#endif
		if(telnetd_enable)
		    run_telnetd(connfd);
		else
		   printf("client try to connect telnetd when telnetd disable,input 'telnetd enable' if you allow\n");
        close(connfd);
	}
    close(listener);
#ifdef HAVE_SYSTEM_REINIT
	telnetd_cleaning=0;
#endif
	return 0;
}

int telnetd_start(unsigned int argc, unsigned char *argv[])
{
	if(argc!=1&&argc!=0&&argc!=2)
	{
		printf(" invalid input!\n should be 'telnetd enable/disable/[timeout [user_time]]'\n");
		return(-1);
	}
	
	if(argc == 1 &&!strcmp(argv[0],"enable"))
	{
		telnetd_enable = 1;
		printf("telnetd enabled\n");
		return 0;
	}else if(argc == 1&&!strcmp(argv[0],"disable")) 
	{
		telnetd_enable = 0;
		printf("telnetd disabled\n");
		return 0;
	}else if(argc == 2&&!strcmp(argv[0],"timeout")) 
	{
		timeout = atoi(argv[1]);
		if(telnetd_running)
		{
		    printf("set telnetd timeout %d seconds\n",timeout);
		    return 0;
		}
	}else if(argc == 1 &&!strcmp(argv[0],"timeout"))
	{
		timeout = 60;
		if(telnetd_running)
		{
		    printf("set telnetd default timeout 60 seconds\n");
		    return 0;
		}
	}else if(!argc)
	{
		if(!telnetd_running)
		{
		    timeout = 60; //default timeout seconds
		}
	}
	else
	{
		printf(" invalid input!\n should be 'telnetd enable/disable/[timeout [user_time]]'\n");
		return(-1);
	}

	if (!telnetd_started)
	{		
		
		cyg_thread_create(TELNETD_THREAD_PRIORITY,
		telnetd_main,
		0,
		"telnetd",
		telnetd_stack,
		sizeof(telnetd_stack),
		&telnetd_thread,
		&telnetd_thread_object);
		
		diag_printf("Starting telnetd thread\n");
		cyg_thread_resume(telnetd_thread);
		telnetd_started=1;
		telnetd_running=1;
		return(0);
	}
	else
	{
		if(telnetd_running==0)
		{
			cyg_thread_resume(telnetd_thread);
			telnetd_running=1;
		}
		printf("telnetd is already running\n");
		return(-1);
	}
}
#ifdef HAVE_SYSTEM_REINIT
void clean_telnetd()
{
	if(telnetd_running)
	{
		telnetd_cleaning=1;
		while(telnetd_cleaning){
			cyg_thread_delay(20);
		}

		telnetd_running=0;
		telnetd_started=0;
		cyg_thread_kill(telnetd_thread);
		cyg_thread_delete(telnetd_thread);
	}
}

#endif

