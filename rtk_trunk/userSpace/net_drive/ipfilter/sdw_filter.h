#ifndef _SWD_FILTER_H_
#define _SWD_FILTER_H_

#include <stdio.h>

#define POST_MAC_LEN 					32 //转换后的mac地址长度

#define IP_LEN_16						16

#define MAC_LEN_20						20

#define ARRAY_LEN_128					128

#define ARRAY_LEN_256					256

#define ARRAY_LEN_1024					1024

#define ARRAY_LEN_2048 2048

#define MAX_ORIGIN_URL 					500 //长传url最大长度

#define MAX_HOST_URL					100

#define URL_RECORD_MAX_NUM			50

#define MAX_FULL_URL_LEN				1500

#define SEND_URL_END					"%2C"			

#define SEND_JSON_END 					"%5D"
	
#define SEND_JSON_START				"data=%5B"

#define AES_BUF_LEN (1025)

enum{
	PARSE_ERROR = 0,
	PARSE_OK,
};


extern int close( int fd );
extern in_addr_t inet_addr(const char *cp);
extern int inet_aton(register const char *cp, struct in_addr *addr);
extern int	connect __P((int, const struct sockaddr *, socklen_t));
extern int	getsockname __P((int, struct sockaddr *, socklen_t *));
extern ssize_t	recv __P((int, void *, size_t, int));
extern ssize_t	send __P((int, const void *, size_t, int));
extern int	setsockopt __P((int, int, int, const void *, socklen_t));
extern int	socket __P((int, int, int));

extern int get_ip(char *host , char *ip) ;
extern int create_tcp_socket(void);
extern int oslib_getpidbyname(char *tname) ;

extern int get_url_path(char *full_path, char *url, char *path, int *port);

#endif 
