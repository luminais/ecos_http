/***********************************************************
Copyright (C), 1998-2014, Tenda Tech. Co., Ltd.
FileName: batchupgrade.c
Description: eCosv1.0 批量升级, main entry
Version : 1.0
Date: 2014.09.25
Function List:
History:
<author>   <time>     <version >   <desc>
wzs        2014.09.25   	1.0        		new
************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <batchupgrade.h>
#include <flashutl.h>
#include <osl.h>
#include <siutils.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <etioctl.h>
#include <bcmgpio.h>

#define BATCH_UPGRADE_SIZE	16*1024
	
static cyg_handle_t upgrade_daemon_handle;
static char upgrade_daemon_stack[BATCH_UPGRADE_SIZE];
static cyg_thread upgrade_daemon_thread;

#define CHECK_TIME 10

struct upgrade_mem *imagePtr = NULL;		// upgrade file data list.
struct upgrade_mem *curPtr;				// cur list loc.
int imageLen = 0;							// total upgrade data size.
extern int UPG_DATA_FLAG;
extern int UPG_BDCT_FLAG;
extern int UPG_RESTORE_FLAG;

#define NET_BUF_MEM		0
#define NET_BUF_MBUF	1
#define NET_BUF_CLUST	2

#define HEAP_RESOLVE_SIZE (100 * 1024)	/* upgrade will use at least 65535 bytes */
#define CLUST_RESOLVE_SIZE (100 * 1024)
#define CFE_TRX_NVRAM_SIZE ((64*2)*1024/*boot size*/ + 32*1024/*nvram size*/)
#ifndef MCLSHIFT
#define MCLSHIFT        11              /* convert bytes to m_buf clusters */
#define MCLBYTES        (1 << MCLSHIFT) /* size of a m_buf cluster */
#endif

#define __CONFIG_DOUBLE_CFG__ 1

#define FREE() do{\
	batch_upgrade_free(imagePtr);	\
	UPGRADE_DEBUG("[Batch upgrade]: Exit\n");	\
}while(0)
	

 struct upgrade_mem{
	struct upgrade_mem *next;
	int totlen;
	int inuse;
	char *data;
};

typedef struct mem_free {
	int misc_free;
	int mbuf_free;
	int clust_free;
	int heap_free;
}MEM_FREE, *PMEM_FREE;

extern int cyg_net_get_mem_stats( int which, cyg_mempool_info *p );
extern char* cluster_malloc(void);
extern void sys_reboot(void);
extern int nvram_set(const char *name, const char *value);
extern int nvram_commit();
extern void want_sleep(int second);

void set_all_led_blink()
{	
	static int led_on = 0;
	int s;
	unsigned int gpio_pin = 0;
	int led_mask=0,led_value = 0,value =0 ;
	int vecarg[2];
	struct ifreq ifr;

	/* Light all the other led except lan&wan, e.g. wl/sys/wps */
	for(gpio_pin = 5; gpio_pin <= 6; gpio_pin++)
	{
		/*enable the wl gpio value input*/
		bcmgpio_connect(gpio_pin, BCMGPIO_DIRN_OUT);
		if(led_on)
		{
			led_mask = ((unsigned long)1 << gpio_pin);
			led_value = (~value << gpio_pin);
			led_value &= led_mask;
		}
		else
		{
			led_mask = ((unsigned long)1 << gpio_pin);
			led_value = (value << gpio_pin);
		}
		bcmgpio_out(led_mask, led_value);
		bcmgpio_disconnect(gpio_pin);
	}
	
	/* Light lan&wan led */
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("open socket failed\n");
		return;
	}

	vecarg[0] = 0x0 << 16;
	vecarg[0] |= (0x12 & 0xffff);
	if(led_on)
	{
		vecarg[1] = 0x155;
	}
	else
	{
		vecarg[1] = 0x0;
	}
	ifr.ifr_data = (caddr_t) vecarg;
	strncpy(ifr.ifr_name, "eth0", sizeof (ifr.ifr_name));
	if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0)
	{
		printf("Error: etcrobowr\n");
		close(s);//test by Coverity
		return ;
	}

	close(s);
	led_on = (led_on?0:1);
	timeout((timeout_fun *)set_all_led_blink, NULL, 30);
}

void want_sleep(int second)
{	
	if (0 < second)
	{
		cyg_thread_delay(second * 100);
	}

	return;
}

int batch_upgrade_get_flash_free()
{
	int flash_size = get_flashsize();
	int resolve = 0;

	resolve += CFE_TRX_NVRAM_SIZE;
#ifdef 	__CONFIG_DOUBLE_CFG__
	resolve += 32*1024;
#endif

	UPGRADE_DEBUG("flash size: %d, avalible size: %d\n", flash_size, flash_size - resolve);

	return flash_size - resolve;
}

int batch_upgrade_get_mem_free(PMEM_FREE pmfree)
{
	struct mallinfo mem_info;
	cyg_mempool_info info;

	if(!pmfree){
		UPGRADE_DEBUG("%s(): Null pointer!\n", __func__);
		return -1;
	}
		
	mem_info = mallinfo();
	pmfree->heap_free = mem_info.fordblks;

	cyg_net_get_mem_stats(NET_BUF_CLUST, &info);
	pmfree->clust_free = info.freemem;

	cyg_net_get_mem_stats(NET_BUF_MEM, &info);
	pmfree->misc_free = info.freemem;

	cyg_net_get_mem_stats(NET_BUF_MBUF, &info);
	pmfree->mbuf_free = info.freemem;

	return 0;	
}

int batch_upgrade_init()
{
	MEM_FREE mfree;
	int heap_free, clust_free, flash_free, true_size;

	/* Do memory prepare */
	flash_free = batch_upgrade_get_flash_free();
	batch_upgrade_get_mem_free(&mfree);
	heap_free = mfree.heap_free - HEAP_RESOLVE_SIZE;
	heap_free = (heap_free / MCLBYTES) * MCLBYTES;	// make sure heap multi blocks of MCLBYTES
	flash_free = (flash_free / MCLBYTES) * MCLBYTES;
	clust_free = mfree.clust_free - CLUST_RESOLVE_SIZE;
	
	true_size = flash_free <= heap_free? flash_free: heap_free;

	/* Initilize the image buf, filling it with heap memory first*/
	imagePtr = (struct upgrade_mem *)malloc(sizeof(struct upgrade_mem));
	if(imagePtr == NULL)
	{
		UPGRADE_DEBUG("%s: malloc failed!\n", __func__);
		return -2;
	}
	imagePtr->totlen = heap_free;
	imagePtr->inuse = 0;
	imagePtr->next = NULL;
	imagePtr->data = (char *)malloc(heap_free);
	if(imagePtr->data == NULL){
		UPGRADE_DEBUG("%s: malloc failed!\n", __func__);
		return -2;
	}
	UPGRADE_DEBUG("[Batch upgrade]: heap malloc size: %d\n", true_size);
	memset(imagePtr->data, 0x0, heap_free);
	curPtr = imagePtr;

	return 1;
}

int batch_upgrade_cache(char *data, unsigned int len)
{
	struct upgrade_mem *pnode;
	
	if(len > curPtr->totlen - curPtr->inuse)
	{
		pnode = (struct upgrade_mem *)malloc(sizeof(struct upgrade_mem));
		if(pnode == NULL)
		{
			UPGRADE_DEBUG("%s: malloc failed!\n", __func__);
			return -1;
		}

		pnode->totlen = MCLBYTES;
		pnode->inuse = 0;
		pnode->next = NULL;
		pnode->data = (char *)cluster_malloc();	/* clust only malloc MCLBYTES bytes */
		if(pnode->data == NULL)
		{
			UPGRADE_DEBUG("%s: cluster malloc failed!\n", __func__);

			/* here must free allocated before */
			//batch_upgrade_free(imagePtr);		// move to main(), "data_flag=4" handles whit it. 
			free(pnode);
			return -1;
		}

		curPtr->next = pnode;
		curPtr = pnode;
	}

	memcpy(&curPtr->data[curPtr->inuse], data, len);
	curPtr->inuse += len;
	
	imageLen += len;
	
	printf(".");
	return 0;
}

extern int watchdog ;
extern si_t *bcm947xx_sih;
extern int tenda_upload_fw(char *stream, int offset_in,int flash_offset);
int batch_upgrade(char *stream)
{
	int image_offset = 0;

#if 0
	if(1 == UPG_RESTORE_FLAG)
	{
		UPGRADE_DEBUG("[Batch upgrade]: Restore+_+\n");
		nvram_set("restore_defaults","1");
		nvram_commit();
		want_sleep(5);
	}
#endif	
	/* Get image offset */
	if(get_flash_offset(&image_offset) != TPI_RET_OK)
		return READ_FLASH_FAIL;
	UPGRADE_DEBUG("[Batch upgrade]: disable watchdog !bcm947xx_sih:%p\n",bcm947xx_sih);
	watchdog = 0;
	si_watchdog_ms(bcm947xx_sih, 0);
	
	UPGRADE_DEBUG("[Batch upgrade]: Upgrading+_+\n");
	
	/* Write to flash */
	if(tenda_upload_fw(stream,0,image_offset) < 0)
		return UPGRADE_FAIL;
	
	return UPGRADE_OK;
}

/*
 *return:
 * -1 : turn off the timer
 *  1 : next check
*/
int batch_upgrade_check_loop()
{
	static int count = 0;

	UPGRADE_DEBUG("[Batch upgrade]: check(%d)...\n", count);	
	
	if(UPG_BDCT_FLAG == 0) // no data coming
	{
		count ++;
		if(count == CHECK_TIME)
		{
			ifupgrade_deinit();
			UPGRADE_DEBUG("[Batch upgrade]:No data coming...\n");
			return -1;
		}
	}
	else
	{
		batch_upgrade_start();
		return -1;
	}

	return 1;
	
}

int batch_upgrade_check()
{
	int ret = 0;
	printf (" %s %d \n", __FUNCTION__, __LINE__);
	ret = batch_upgrade_check_loop();
	
	if(-1 == ret)
	{		
		untimeout((timeout_fun *)&batch_upgrade_check, 0);
	}
	else
	{	
		timeout((timeout_fun *)batch_upgrade_check, NULL, 100);
	}
	return 0;
}

void batch_upgrade_free( struct upgrade_mem *list)
{
 	struct upgrade_mem *p, *next;

	if(!list){
		return ;
	}

	p = list;
	next = p->next;

	if(p){	// heap free
		if(p->data){
			free(p->data);
			p->data = NULL;
		}
		free(p);
		p = NULL;
	}

	p = next;

	while(p){	// clust free
		next = p->next;
		if(p->data){
			cluster_free(p->data);
			p->data = NULL;
		}
		free(p);
		p = NULL;
		p = next;
	}
}

void batch_upgrade_main()
{
 	int ret;
	while(1)
	{
		if(UPG_DATA_FLAG == 3) // validate ok 
		{
			if(UPGRADE_OK != (ret =batch_upgrade((char *)imagePtr)))
			{
				UPGRADE_DEBUG("\n[Batch upgrade]:%d Upgrade failed+_+#\n", ret);
				FREE();
				UPG_DATA_FLAG = 0;
				return;
			}
			else
			{
				UPGRADE_DEBUG("\n[Batch upgrade]: Upgrade success+_+\n");		
				sys_reboot();
				return;
			}
		}
		else if(UPG_DATA_FLAG == 4) // validate failed
		{
			ate_set_all_led_on();
			FREE();
			UPG_DATA_FLAG = 0;
			return;
		}

		if(UPG_BDCT_FLAG == 3) // upgrade success
		{
			set_all_led_blink();
			FREE();
			UPG_BDCT_FLAG = 0;
			return;
		}
		
		cyg_thread_delay(1);
	}
}

int batch_upgrade_start()
{	
	UPGRADE_DEBUG("[Batch upgrade]: Start\n");
	
	if(batch_upgrade_init() < 0)
		return -1;

	if(1 == UPG_RESTORE_FLAG)//恢复出厂设置
	{
		UPGRADE_DEBUG("[Batch upgrade]: Restore+_+\n");
		nvram_set("restore_defaults","1");
		nvram_commit();
		UPG_RESTORE_FLAG = 0;
		want_sleep(2);
	}
		
	cyg_thread_create(
		10, 
		(cyg_thread_entry_t *)batch_upgrade_main,
		0, 
		"batch_upgrade",
		upgrade_daemon_stack, 
		sizeof(upgrade_daemon_stack), 
		&upgrade_daemon_handle, 
		&upgrade_daemon_thread);
	cyg_thread_resume(upgrade_daemon_handle);
	
	return 0;
}
