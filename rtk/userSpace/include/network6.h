#include "../rtadvd/ra.h"

#define DNS_LEN 46
#define PREFIX_LEN 46
#define IPV6_LEN 46

typedef struct dhcp6c_cfg_t
{
	char ifname[20];
	long iaid;
	long iapd;
	long slid;
	int stless;
	int gpd;
	int aftr;
}DHCP6C_CFG,*P_DHCP6C_CFG;

typedef struct dhcp6c_status_t
{
	char ifname[20];
	char ipv6_addr[40];
	char ipv6_prefix[40];
	char pri_dns[40];
	char sec_dns[40];
	char gateway_v6[40];
	char aftr_name[40];

	char lan6_addr[40];
}DHCP6C_STATUS,*P_DHCP6C_STATUS;

typedef struct lan6_status_t
{
	char lan_pri_dns[40];
	char lan_sec_dns[40];
	int  lan_prefix_mode;
	int  lan_dhcp6_type;
	int  lan_dns_type;
	char lan_start_id[40];
	char lan_end_id[40];
	int  lan_ipv6_enable;
	char prefix[40];
}LAN6_STATUS,*P_LAN6_STATUS;

typedef struct dhcp6s_cfg_t
{
	unsigned int vltime;
	int dhcp6s_mode;
	int d6ns_mode;
	char pri_dns[DNS_LEN];
	char sec_dns[DNS_LEN];
	char start_addr[IPV6_LEN];
	char end_addr[IPV6_LEN];
	char prefix[PREFIX_LEN];
	int prefix_len;
	char d6sname[128];
	char ifname[20];
	char domain[128];
}DHCP6S_CFG,*P_DHCP6S_CFG;

typedef struct rtadvd_cfg_t
{
    int rtadvd_mFlag;
    int rtadvd_oFlag;
    unsigned int ra_maxinterval;
    unsigned int ra_mininterval;
    char ra_interface[RA_INTF_LEN];
    struct routeradv_prefix_opt rapfopt[RA_MAXPREFIX];
}RTADVD_CFG, *P_RTADVD_CFG;

#define IFV6CONFIG_ADD 1
#define IFV6CONFIG_DEL 2
#define EUI64_GBIT	0x01
#define EUI64_UBIT	0x02

extern int lan6id2addr(char *id1, char *id2, char *prefix);