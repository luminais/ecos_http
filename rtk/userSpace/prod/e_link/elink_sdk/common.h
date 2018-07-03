
#ifndef _ELINK_COMMON_H_
#define _ELINK_COMMON_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__ECOS)
#include <unistd.h>

#include <arpa/inet.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#if !defined(__ECOS)
#include <endian.h>
#endif

#include "openssl/ossl_typ.h"
#include "openssl/bio.h"
#include "openssl/rsa.h"
#include "openssl/evp.h"

#include "uloop.h"
#include "utils.h"
#include "ulog.h"
#include "uuid.h"
#include "cJSON.h"
#define json_typeof(x) ((x)->type & 0xff)
#define json_integer_value(x) (x)->valueint
#define json_string_value(x) (x)->valuestring
#define json_array_foreach(array, index, value) \
	for (value = (array)->child, index = 0; value; value = value->next, index++)
		
#define TEST_PUBLIC_KEY "-----BEGIN PUBLIC KEY-----\n"\
					   "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCz9w7fTin0vJC02vb9rZpjx12\n"\
					   "IFZf5ZqMXMNsjWrqiyVUrBB06pUCUTS4R9HkUOo+YEXy/F1+6/h+WBaUNGoLy9SJ\n"\
					   "2jUTtcx4fXCES618YSy0KuedtZwImtKedmQRN2qJh/Vppiwv6MioxSKqUJQX3c2U\n"\
					   "24bR1316vKfaoloz5QIDAQAB\n"\
					   "-----END PUBLIC KEY-----\n"

#define REAL_PUBLIC_KEY "-----BEGIN PUBLIC KEY-----\n"\
					   "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCwKhuZ3bQD2DQFzap/5sT0cBtf\n"\
					   "PTRd47Mih1ivZ/71YQbEeHFc1EDCfR1ryW3zhaBb8VWseXM2yo6NI1KvFWb3w3Hx\n"\
					   "c2vOtq3pKO5R9jHezDLmRvdwNnxzYJa/o8sf9mQBGIIz/G7rhi2TZAeXseJeXAJ2\n"\
					   "WXvsOEL9jBM/cA4rowIDAQAB\n"\
					   "-----END PUBLIC KEY-----\n"

struct bksrv {
	char *addr;
	unsigned int port;
	unsigned char used;
};

#define NUM_OF_BACKUP_SRV 4
struct srvcfg {
	char *bssaddr;
	unsigned int bssport;
	char *addr;
	unsigned int port;
	struct bksrv backup[NUM_OF_BACKUP_SRV];
};

struct apcfg {
	char *mac;
	char *vendor;
	char *model;
	char *swver;
	char *hdver;
	char *sn;
	char *ipaddr;
	char *gwmac;
};

struct devcfg {
	char *token;
	char *id;
};

struct keyinfo {
	unsigned char *shared_key;  //local dh/ecdh
	unsigned char *client_key;  //tcp CKey
	unsigned char *server_key;  //tcp SKey
	unsigned char *public_key;  //rsa public key
	char *challenge_code; //in Activate and Connect msg
};

struct cmd_info {
	struct list_head list;
	char *content;
	unsigned short len;
};

struct req_info {
	struct list_head list;
	char *raw;
	unsigned short len;
};

struct spp_info {
	struct list_head list;
	unsigned int sequence;
	char *id;
	struct uloop_timeout timer;
};

struct elink_ctx {
	struct srvcfg server;
	struct apcfg ap;
	struct devcfg dev;
	struct keyinfo keys;
	int romote_fd;
	unsigned char is_bss;
	unsigned char is_bootstrap;
	char *curr_srv;
	char *msg; //fragment
	unsigned int curr_port;
	unsigned int sequence;
	unsigned int retry_count;
	unsigned int retry_cycle;
	struct list_head cmd_list; //local
	struct list_head req_list; //remote
	struct list_head spp_list; //SetPluginParams
};


#define ELINE_MESSAGE_TYPE_CFG "cfg"

#define ELINE_MESSAGE_TYPE_GET_STATUS "get_status"

#define ELINE_MESSAGE_TYPE_GET_STA_RSSI "getrssiinfo"

#define ELINE_MESSAGE_TYPE_DEASSOC "deassociation"


typedef	unsigned char		__uint8_t;
typedef	short			__int16_t;
typedef	unsigned short		__uint16_t;
typedef	int			__int32_t;
typedef	unsigned int		__uint32_t;
typedef	unsigned long long	__uint64_t;
#define	__is_constant(x)	0

#define	__bswap16_const(x)	(((x) >> 8) | (((x) << 8) & 0xff00))
#define	__bswap32_const(x)	(((x) >> 24) | (((x) >> 8) & 0xff00) |	\
	(((x) << 8) & 0xff0000) | (((x) << 24) & 0xff000000))
#define	__bswap64_const(x)	(((x) >> 56) | (((x) >> 40) & 0xff00) |	\
	(((x) >> 24) & 0xff0000) | (((x) >> 8) & 0xff000000) |		\
	(((x) << 8) & ((__uint64_t)0xff << 32)) |			\
	(((x) << 24) & ((__uint64_t)0xff << 40)) |			\
	(((x) << 40) & ((__uint64_t)0xff << 48)) | (((x) << 56)))


static __inline __uint16_t
__bswap16_var(__uint16_t _x)
{

	return ((_x >> 8) | ((_x << 8) & 0xff00));
}

static __inline __uint32_t
__bswap32_var(__uint32_t _x)
{

	return ((_x >> 24) | ((_x >> 8) & 0xff00) | ((_x << 8) & 0xff0000) |
	    ((_x << 24) & 0xff000000));
}

static __inline __uint64_t
__bswap64_var(__uint64_t _x)
{

	return ((_x >> 56) | ((_x >> 40) & 0xff00) | ((_x >> 24) & 0xff0000) |
	    ((_x >> 8) & 0xff000000) | ((_x << 8) & ((__uint64_t)0xff << 32)) |
	    ((_x << 24) & ((__uint64_t)0xff << 40)) |
	    ((_x << 40) & ((__uint64_t)0xff << 48)) | ((_x << 56)));
}

#define	__bswap16(x)	((__uint16_t)(__is_constant((x)) ?		\
	__bswap16_const((__uint16_t)(x)) :  __bswap16_var((__uint16_t)(x))))
#define	__bswap32(x)	((__uint32_t)(__is_constant((x)) ?		\
	__bswap32_const((__uint32_t)(x)) :  __bswap32_var((__uint32_t)(x))))
#define	__bswap64(x)	((__uint64_t)(__is_constant((x)) ?		\
	__bswap64_const((__uint64_t)(x)) :  __bswap64_var((__uint64_t)(x))))
#define	bswap16(x)	__bswap16(x)
#define	bswap32(x)	__bswap32(x)
#define	bswap64(x)	__bswap64(x)

#define	htobe16(x)	bswap16((x))
#define	htobe32(x)	bswap32((x))
#define	htobe64(x)	bswap64((x))
#define	htole16(x)	((uint16_t)(x))
#define	htole32(x)	((uint32_t)(x))
#define	htole64(x)	((uint64_t)(x))

#define	be16toh(x)	bswap16((x))
#define	be32toh(x)	bswap32((x))
#define	be64toh(x)	bswap64((x))
#define	le16toh(x)	((uint16_t)(x))
#define	le32toh(x)	((uint32_t)(x))
#define	le64toh(x)	((uint64_t)(x))

//DDBBGG
//#define ENABLE_INTERVAL_DEBUG 1
//#define USE_LOCAL_BSS_SERVER 1
//#define USE_LOCAL_TCP_SERVER 1
#define ELINK_VERSION "1.0.122"

#define ELINK_MAGIC 0x3f721fb5
#define ELINK_MAGIC_LEN 4
#define ELINK_CONTENT_LEN 4
#define ELINK_HEADER_LEN (ELINK_MAGIC_LEN + ELINK_CONTENT_LEN)

#define ELINKCC_MAGIC 0x43545347574d5032 //CTSGWMP2
#define ELINKCC_MAGIC_LEN 8
#define ELINKCC_HEADER_LEN (ELINKCC_MAGIC_LEN + ELINK_CONTENT_LEN)
#define ELINKCC_TAIL_LEN 2 //\r\n
#define ELINK_MSG_LEN(_len)	((_len) + ELINKCC_HEADER_LEN + ELINKCC_TAIL_LEN)

#define DH_128_KEY_LEN 16
#define ECDH_112_KEY_LEN 14
#define ECDH_112_KEY_SIZE 15

#define AES_128_BLOCK_SIZE 16
#define CKEY_SKEY_LEN 16

#define ELINK_SN_LEN 34
#define ELINK_TIMEOUT_5S 5
#define ELINK_TIMEOUT_10S 10
#define ELINK_TIMEOUT_20S 20

#define UNKNOWN_FD -1

#define ELINK_SERVER_IP "0.0.0.0"
#define ELINK_SERVER_PORT "32768"

#if USE_LOCAL_BSS_SERVER
#define DEFAULT_BSS_ADDR "127.0.0.1"
#define DEFAULT_BSS_PORT 6666
#else
#define DEFAULT_BSS_ADDR "apbss1.189cube.com"
#define DEFAULT_BSS_PORT 8088
#endif

#define ELINK_UNIX_SOCKET "/tmp/ctc_elinkap.sock"
#define ELINKCC_CONFIG "/tmp/ctc_elinkap.json"
#define ELINKCC_PEM "/tmp/ctc_elinkap.pem"
#define ELINKCC_LOGLEVEL "/tmp/ctc_elinkap.loglevel"

#define ELINKCC_100MS_INTERVAL 100
#define ELINKCC_200MS_INTERVAL 200
#define ELINKCC_100S_INTERVAL 100 * 1000
#define ELINKCC_30S_INTERVAL 30 * 1000
#if ENABLE_INTERVAL_DEBUG
#define ELINKCC_150M_INTERVAL 90 * 1000
#else
#define ELINKCC_150M_INTERVAL 150 * 60 * 1000
#endif
#define ELINKCC_CMD_LIST_SIZE 10

#define ELINK_PLUGIN_NAME "eLinkAP"

#define ELINK_CS_PREFIX "elinkcs"
#define ELINK_CC_PREFIX "elinkcc"

#define ELINK_SRV_PREFIX "elinksrv"
#define ELINK_CLI_PREFIX "elinkclt"

#define KEEP_ALIVE_INTERVAL 18 * 1000

#define XSTR(x) #x
#define STR(x) XSTR(x)

#define IS_EMPTY_STRING(s)  ((s == NULL) || (*s == '\0'))
#define IS_VALID_PORT(p) (((p) > 0) && ((p) < 65536))

#define FREE(x) do { if (x != NULL) { free(x); x = NULL; } } while (0);

#define CHECK_JSON_VALUE(tmp) if(NULL == tmp) \
								{printf( "JSON msg error.%s:%d\n", __FUNCTION__, __LINE__); goto bad_msg;}

typedef void (*SignalHandler)(int sig);

void _set_signal_handler(int sig, SignalHandler handler);

int _strcmp(const char *str1, const char *str2);
unsigned char *_get_random_string(int size);
int _get_random_number(int from, int to);

int _check_msg_header(const char *buf, int size);
int _check_msg_tail(const char *buf);

unsigned char *_add_elink_hdr_local(unsigned char *buf, unsigned int len);
unsigned char *_add_elink_hdr_remote(unsigned char *buf, unsigned int len);

void _bin_print(unsigned char *value, unsigned int len);
void _char_print(unsigned char *value, unsigned int len);

int _do_aes_cbc_crypt(unsigned char *in, int inlen, unsigned char **out, int *outlen, unsigned char *key, int do_encrypt);
int _do_aes_ecb_crypt(unsigned char *in, int inlen, unsigned char **out, int *outlen, unsigned char *key, int do_encrypt);

int _do_rsa_encrypt(unsigned char *data, int data_len, unsigned char **encrypted, unsigned char *key);
int _do_rsa_decrypt(unsigned char *data, int data_len, unsigned char **decrypted);

int _do_b64_cmd(unsigned char *src, int srclen, unsigned char **dst, int encode);

int _get_int_json_value(cJSON *obj, char *key, int type, void *buf);
int _get_string_json_value(cJSON *obj, char *key, char **buf);
cJSON *_config_to_jobj(const char *file);

int _get_sys_time(char **buf);
int _get_uuid_string(char **uuid);
int _convert_mac(char *mac);


#endif
