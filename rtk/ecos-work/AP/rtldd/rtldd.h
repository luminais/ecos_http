/*headder files for rtldd*/
#ifndef __RTLDD_H__
#define __RTLDD_H__
#define OK (0)
#define FAIL (-1)
#define ORAY_DDNS "oray"
#define DYNDNS_DDNS "dyndns"
#define TZO_DDNS    "tzo"


#define APP_NAME "rtldd"
#define HOMEPAGE "http://reaktek.com"
#define VERSION "1.0.0"

#define BUF_SIZE         2048
/*Param Struct*/
typedef struct ddParam {
        unsigned char backmx_flag;
        const char *hostname;
        char *ipv4_addr;
        char *mx;
        unsigned char offline_flag;
        const char *system;
        char *login;
        unsigned char wildcard_flag;
}  DDPARAM_T, *pDDPARM_T;

/*Connection*/
int create_connection(const char *hostname, int port);
int create_udpconnection(const char *hostname, int port);
int parse_cmd(struct ddParam *args, int argc , char *argv[]);
#endif
