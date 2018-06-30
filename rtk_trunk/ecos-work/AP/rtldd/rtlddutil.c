/*rtlddutil.c offers util functions for rtldd*/
#include <stdio.h>
#include "rtldd.h"
void uppertolower(char *str, char *buf, int size)
{

        int n;

        for(n = 0; n < size && str[n] != '\0'; n++) {
                buf[n] = tolower(str[n]);
        }
        buf[n] = '\0';

        return;

}

int parse_cmd(struct ddParam *args, int argc , char *argv[])
{
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
                printf("wrong usage");
                return FAIL;

        case 3:
                args->login = argv[argc-2];
        }
        args->hostname = argv[argc-1];
        return OK;

}
