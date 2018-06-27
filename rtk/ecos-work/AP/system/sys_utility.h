#ifndef SYS_UTILITY_H
#define SYS_UTILITY_H

#include <time.h>
#include <cyg/kernel/kapi.h>

#define NULL_FILE 0
#define NULL_STR ""

#ifdef SYS_INIT_USING_MBOX
extern cyg_mbox sys_mbox;
extern cyg_handle_t sys_mbox_hdl;
#else
extern cyg_flag_t sys_flag;
#endif



#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#define DNS_INFO_FILN  "/etc/resolv.conf"
#define WAN_INFO_FILE "/etc/wan_info"
#define DHCPD_CONF_FILE            "/etc/udhcpd.conf"

#define DHCPD_ADDRPOOL_DB      "/etc/dhcpdb.pool"
#define DHCPD_BINDING_DB         "/etc/dhcpdb.bind"
#define DHCPD_RELAY_DB             "/etc/dhcpdb.relay"
#define SYSCMD_THREAD_PRIORITY 16
#ifdef ULINK_DHCP_AUTO
#define ULINK_DHCP_AUTO_CHECK_GETIP_FILE "/etc/ulink_dhcpauto_dhcpcGetIp"
#endif
#ifdef DHCP_AUTO_SUPPORT
#define DHCP_AUTO_CHECK_GETIP_FILE "/etc/ulink_dhcpauto_dhcpcGetIp"


#endif
typedef enum SITE_SURVEY_STATUS_
{
	SITE_SURVEY_STATUS_OFF,
	SITE_SURVEY_STATUS_RUNNING,
	SITE_SURVEY_STATUS_WAITING
} SITE_SURVEY_STATUS;
extern SITE_SURVEY_STATUS siteSurveyStatus;
#ifdef __ECOS
#define _IPV6_LAN_INTERFACE "eth0"
#define _IPV6_WAN_INTERFACE "eth1"
#else
#define _IPV6_LAN_INTERFACE "br0"
#define _IPV6_WAN_INTERFACE "eth1"
#endif

#ifdef 	ECOS_DBG_STAT
#define MODULE_NAME_SIZE	16
typedef struct dbg_rtl{
	char	name[MODULE_NAME_SIZE];
	unsigned int count_add;
	unsigned int size;
	unsigned int count_del;
}dbg_stat;

/*DEBUG type */
#define DBG_TYPE_MAX		5
#define	DBG_TYPE_MALLOC		0
#define DBG_TYPE_FILE		1
#define DBG_TYPE_SOCKET		2
#define DBG_TYPE_MBUF		3
#define DBG_TYPE_SKB		4

/*module name mapping*/
#define DBG_MODULE_MAX				32

/*action type*/
#define	DBG_ACTION_ADD		0
#define DBG_ACTION_DEL		1


int dbg_stat_add(int dbg_type, int module_type, int action_type, unsigned int size);
void dbg_stat_show();
void dbg_stat_init();
int dbg_stat_register(char *name);
#endif

#if defined(ECOS_MEM_CHAIN_API) 
#define MEM_CHAIN_UPGRADE_LIST_NUM		10

#define NEED_HEAP						1<<0
#define NEED_MISCPOOL					1<<1
#define NEED_CLUSTPOOL					1<<2

struct mem_chain_upgrade_list
{
	char*							mem_addr;
	int 							size;
	struct mem_chain_upgrade_list* 	next;
};

typedef struct mem_chain_upgrade_header
{
	int								size;
	struct	mem_chain_upgrade_list* header;
}Mem_Chain_Upgrade_Header;

typedef struct mem_chain_upgrade_offset_mem
{
	int 					offset;
	int						size;
	char*					mem;
}Mem_Chain_Upgrade_Offset_Mem;

typedef struct mem_chain_upgrade_address_info{
	int						block_num;
	int						offset;
	unsigned char			mem_list_num;
}Men_Chain_Upgrade_Address_Info;

int mem_chain_upgrade_read(int fd,void *buf,size_t count);
void mem_chain_upgrade_free(void* p);
void *mem_chain_upgrade_malloc(unsigned int num_bytes,unsigned int heap_keep,unsigned int miscpool_keep,unsigned int clust_keep);
int mem_chain_upgrade_fwChecksumOk(char *data, int len);
int mem_chain_upgrade_CHECKSUM_OK(unsigned char * data,int len);
int mem_chain_upgrade_memcmp( const void *s1, const void *s2, size_t n);
void *mem_chain_upgrade_memcpy(void *dest, const void *src, size_t n);
char *mem_chain_upgrade_memstr(char *membuf, char *param, int memsize);
int mem_chain_upgrade_value_set(char* data,char value);
int mem_chain_upgrade_flash_write_count(unsigned int block_size,unsigned int* offset,unsigned char* ram,unsigned int write_count,unsigned int* this_write);
void* mem_chain_upgrade_mem_convert(char* data);
int mem_chain_upgrade_node_size(char* data);

#if defined(HAVE_FIREWALL)
void set_firewall_forward(void);
void reset_firewall_forward(void);
#endif
#endif

void do_reset(int type);
int RunSystemCmd(char *filepath, ...);
int rtk_flash_read(char *buf, int offset, int len);
int rtk_flash_write(char *buf, int offset, int len);

void generate_pin_code(char *pinbuf);
int check_pin_code_of_hw_settings(void);
void wps_led_control(int value);
int wps_button_pressed(void);

void file_gunzip(char *infile, char *outfile);
__attribute__((nomips16)) void show_interrupt_table(void);
time_t get_epoch_build_time(void);
double get_uptime(void);

#ifdef CYGPKG_CPULOAD
void cpuload_measurement_start(void);
void cpuload_measurement_end(void);
void cpuload_set_sleep_time(int sleep_time);
#endif
int isValidName(char *str);
int write_line_to_file(char *filename, int mode, char *line_data);
void delDefaultRoute(void);
#endif /* SYS_UTILITY_H */


