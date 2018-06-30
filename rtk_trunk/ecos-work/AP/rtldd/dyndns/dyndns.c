/*rewrite for dyndns ddns hf*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

#include "../base64encode.h"

#include "../rtldd.h"
#include "../rtldebug.h"
#include "dyndns.h"


#define BUF_LEN          2048

void print_version()
{
	printf("dyndns version 1.0\n");
}

void dyndns_usage(char *appname)
{

        printf("\nUsage: %s [...] %s -- [OPTION]... [USERNAME:PASSWORD] HOSTNAME\n\n",
                      appname, "dyndns");
        printf(
               "For security reasons use the environment variable LOGIN instead of\n"
               "passing the login information directly.\n\n"
               "Options:\n"
               "   -4    --ipv4 <address>        ip address version 4\n"
               "   -b    --with-backmx           enable backmx\n"
               "   -m    --mx <host>             hostname of your mail exchange\n"
               "   -o    --offline               set host to offline mode\n"
               "   -s    --system <system>       specify the system\n"
               "                                 (dyndns [default], statdns, custom)\n"
               "   -w    --wildcard <on|off>     switch wildcard on or off\n"
               "         --help                  print help and exit\n"
               "         --version               display version information and exit\n\n");
}

#if 0
int parse_cmd(struct ddParam *args, int argc , char *argv[])
{
	int c,n;
	char buf[BUF_SIZE];
	int optleft=0;
  	args->backmx_flag = 0;
        args->wildcard_flag = 0;
        args->offline_flag = 0;
        args->system = dd_system[0];

#if 0
	while(1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{ "ipv4",               1, 0, '4' },
			{ "help",               0, 0, 'h' },
			{ "mx",                 1, 0, 'm' },
			{ "offline",            0, 0, 'o' },
			{ "system",             1, 0, 's' },
			{ "wildcard",           1, 0, 'w' },
			{ "with-backmx",        0, 0, 'b' },
			{ "version",            0, 0, 'v' },
			{ "login",              0, 0, 'l' },
			{ "hostname",           0, 0, 'n' },
			{ NULL,                 0, 0, 0   }
		};
                c = getopt_long(argc, argv, "4:bm:os:w:",
                                long_options, &option_index);

                if(c == -1) break;

                switch(c) {
                case '4':
                        args->ipv4_addr = optarg;
                        break;
                case 'b':
                        args->backmx_flag = 1;
                        break;
                case 'h':
                        dyndns_usage(argv[0]);
                        exit(OK);
                case 'm':
                        args->mx = optarg;
                        break;
                case 'o':
                        args->offline_flag = 1;
                        break;
                case 's':
                        uppertolower(optarg, buf, BUF_SIZE);
                        for(n = 0;; n++) {
                                if(dd_system[n] == NULL) {
                                       DEBUG_ERR("invalid argument for `-s': %s",
                                                  optarg);
                                       return FAIL;
                                }
                                if(strncmp(buf,dd_system[n],strlen(dd_system[n])) == 0) {
                                        args->system = dd_system[n];
                                        break;
                                }
                        }
                        break;
                case 'v':
                        print_version();
                        exit(OK);
                case 'w':
                        uppertolower(optarg, buf, BUF_SIZE);
                        if(strcmp(buf, "on") == 0) {
                                args->wildcard_flag = 1;
                        } else if(strcmp(buf, "off") == 0) {
                                args->wildcard_flag = 2;
                        } else {
                                DEBUG_ERR("invalid argument for `-w': %s", optarg);
                                return 1;
                        }
                        break;
		case 'l':
			args->login=optarg;
			break;
		case 'n':
			args->hostname=optarg;
			break;
                }
        }
#endif
	
        switch(argc-1) {
        default:
                DEBUG_ERR("wrong usage");
                return FAIL;

        case 3:
                args->login = argv[argc-2];
        }
        args->hostname = argv[argc-1];
	return OK;

}
#endif

static int get_dyndns_response(int sock, char *message)
{
	memset(message,0,BUF_LEN);
	if(read(sock,message,BUF_LEN-1) == -1) {
		DEBUG_ERR("read() failed\n");
		return FAIL;
	}
	return OK;
}
static int 
send_dyndns_request(int sock,char *message)
{
	if(write(sock, message, strlen(message)) == -1) {
                DEBUG_ERR("write() failed\n");
                return FAIL;
        }
	return OK;
}

static int
build_dyndns_request(char *message, struct ddParam *args)
{

        char *b64user;

        if(strlen(args->login) > 128) {
                DEBUG_ERR("username is too long\n");
                return FAIL;
        }
        b64user = (char *)malloc((2 * strlen(args->login) + 1));
        if(b64user == NULL) {
                DEBUG_ERR("malloc() failed");
                return FAIL;
        }
        (void)memset(b64user, 0, 2 * strlen(args->login) + 1);

        base64encode(args->login, b64user);
        (void)snprintf(message, BUF_LEN,
                       "GET /nic/update?system=%s&hostname=%s&wildcard=%s"
                       "&backmx=%s&offline=%s",
                       args->system, args->hostname,
                       ONOROFF(args->wildcard_flag), YESORNO(args->backmx_flag),YESORNO(args->offline_flag));

        if(args->ipv4_addr) {
                (void)strncat(message, "&myip=", BUF_LEN-strlen(message));
                (void)strncat(message, args->ipv4_addr, BUF_LEN-strlen(message));
        }

        if(args->mx) {
                (void)strncat(message, "&mx=", BUF_LEN-strlen(message));
                (void)strncat(message, args->mx, BUF_LEN-strlen(message));
        }

        {
                char buffer[1024];

                (void)snprintf(buffer, 1024,
                               " HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Authorization: Basic %s\r\n"
                               "User-Agent: %s %s - %s\r\n"
                               "Connection: close\r\n"
                               "Pragma: no-cache\r\n\r\n",
                               DYNHOST, b64user, APP_NAME, VERSION, HOMEPAGE);
                (void)strncat(message, buffer, BUF_LEN - 1 - strlen(message));
        }
        DEBUG_PRINT("\n\nMessage:"
                    "\n--------------------------------------\n"
                    "%s--------------------------------------\n\n",
                    message);


        free(b64user);
        return OK;

}

static int check_servermsg(char *message)
{
        DEBUG_PRINT("\n\nServer message:"
                    "\n--------------------------------------\n"
                    "%s--------------------------------------\n\n",
                    message);
	char *ptr;
	int n;
        if(strstr(message, "HTTP/1.1 200 OK") ||
           strstr(message, "HTTP/1.0 200 OK")) {

                (void)strtok(message, "\n");
                while((ptr = strtok(NULL, "\n")) != NULL) {
                        for(n=0; dyndns_msgs[n].code != NULL; n++) {
                                if(strstr(ptr, dyndns_msgs[n].code)) {
                                        DEBUG_ERR("%s\n",dyndns_msgs[n].message);
                                        if(dyndns_msgs[n].error == 1) {
                                                return FAIL;
                                        } else {
                                                return OK;
                                        }
                                }
                        }
                }
        } else if(strstr(message, "401 Authorization Required")) {
                DEBUG_ERR("dyndns.org: wrong username or password");
        } else {
                DEBUG_ERR("dyndns.org: Internal Server Error");
        }

        return FAIL;
}

static int verify_servermsg(int sock, char *message)
{
	if(get_dyndns_response(sock,message) <0)
	{
		DEBUG_ERR("get response failed\n");
	}
	return check_servermsg(message);
}

int register_dyndns(int argc, char *argv[])
{
	int sock;
	int ret;
	DDPARAM_T param;
	char message[BUF_LEN];

	memset(&param,0,sizeof(param));	
	memset(message,0,BUF_LEN);

	/*default value*/
        param.backmx_flag = 0;
        param.wildcard_flag = 0;
        param.offline_flag = 0;
        param.system = dd_system[0];

	/*parse command line*/
	parse_cmd(&param,argc,argv);

	/*connect to server*/
	if((sock=create_connection(DYNHOST,DYNPORT)) < 0)
	{
		DEBUG_ERR("connect to server failed\n");
		return (-1);
	}
	/*send request*/
	build_dyndns_request(message,&param);
	send_dyndns_request(sock,message);
	/*verify feedback*/
	ret=verify_servermsg(sock,message);
	
	if(sock)
		close(sock);
	return ret;
}


