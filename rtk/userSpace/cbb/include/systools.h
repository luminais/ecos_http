#ifndef __SYSTOOLS_H__
#define __SYSTOOLS_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#ifndef __VSRSION_H__
#include <version.h>
#endif

#include <sys/param.h>
#define MCLBYTES 				1856
#define NET_BUF_MEM			0
#define NET_BUF_MBUF			1
#define NET_BUF_CLUST			2
#define HEAP_RESOLVE_SIZE 		(100 * 1024)	/* upgrade will use at least 65535 bytes */
#define CLUST_RESOLVE_SIZE 	(100 * MCLBYTES)
#define RTK_FLASH_SIZE tenda_get_flash_size()	//flash大小
#define RTK_BOOT_SIZE (120*1024)	//BOOT区大小
#define RTK_ENVRAM_SIZE (8*1024)	//envram大小
#define RTK_NVRAM_SISZ (32*1024)	//nvram大小，还没有备份
#define RTK_TRX_SIZE (RTK_FLASH_SIZE-RTK_BOOT_SIZE-RTK_ENVRAM_SIZE-RTK_NVRAM_SISZ)	//升级文件大小
#define UPGRADE_SUCCESSFUL   100  //升级成功
#define UPGRADE_FALI   202  //升级失败
#define UPGRADE_FILE_FAIL   201   //升级文件错误
#define UPGRADE_FILE_BIG_FAIL   203   //升级文件过大
#if 0
struct upgrade_mem{
	struct upgrade_mem *next;
	PI32 	totlen;
	PI32 	inuse;
	PI8_P 	data;
};
#endif
typedef struct mem_free {
	int misc_free;
	int mbuf_free;
	int clust_free;
	int heap_free;
}MEM_FREE, *PMEM_FREE;
typedef struct upgrade_server_info
{
	PIU8 	uri[PI_BUFLEN_256];
	PIU8 	dir[PI_BUFLEN_256];
	PI32 	port;
}UPGRADE_SERVER_INFO,*p_UPGRADE_SERVER_INFO;
/*API*/

/*GPI*/

/*TPI*/
extern RET_INFO tpi_systools_action(RC_MODULES_COMMON_STRUCT *var);
#endif/*__SYSTOOLS_H__*/
