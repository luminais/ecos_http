
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //For sleep
#include <string.h>
#include "process_api.h"
#include "biz_typedef.h"
#include "biz_register.h"
#include "biz_others.h"

#include <ecos_oslib.h>


extern void ucloud_start(void);
#ifndef CONFIG_APP_COSTDOWN
//#define STACK_SIZE	(32*1024)
#define STACK_SIZE	(8*1024)

static cyg_handle_t biz_handle;
static char biz_stack[STACK_SIZE];
static cyg_thread biz_thread;
static int biz_m_fd;


extern void ucloud_start(void);

int biz_m_api_lib_thread_fd_get(void)
{
	return biz_m_fd;
}

static void biz_m_api_lib_thread_fd_set(int fd)
{
	biz_m_fd = fd;
}

static void _sig_handler(int sig) {
	//signal(SIGINT, SIG_DFL);
	//signal(SIGTERM, SIG_DFL);
	uc_api_lib_destroy();
}

int biz_main_loop()
{
	/* F9 does not need signal */
	//signal(SIGPIPE, SIG_IGN);
	//signal(SIGINT, _sig_handler);
	//signal(SIGTERM, _sig_handler);
	int fd;
	cyg_thread_delay(1*100);
	
	fd = uc_api_lib_connect();
	biz_m_api_lib_thread_fd_set(fd);

    /* you can register your module cmd here*/
    if (-1 == biz_ucloud_info_init(fd)) {
		BIZ_ERROR("biz_ucloud_info_init failed\n");
		return 0;
    }

    if (-1 == biz_m_sys_init(fd)) {
		BIZ_ERROR("biz_m_sys_init failed\n");
		return 0;
    }
    
    if (-1 == biz_m_safe_check_init(fd)) {
		BIZ_ERROR("biz_m_safe_check_init failed\n");
        return 0;
    }

    if (-1 == biz_m_wan_init(fd)) {
		BIZ_ERROR("biz_m_wan_init failed\n");
        	return 0;
    }
	
    if (-1 == biz_m_login_init(fd)) {
		BIZ_ERROR("biz_m_login_init failed\n");
        	return 0;
    }

    if (-1 == biz_m_wifi_init(fd)) {
		BIZ_ERROR("biz_m_wifi_init failed\n");
        	return 0;
    }

    if (-1 == biz_m_hand_qos_init(fd)) {
		BIZ_ERROR("biz_m_hand_qos_init failed\n");
        	return 0;
    }
	
    if (-1 == biz_m_ol_host_init(fd)) {
		BIZ_ERROR("biz_m_ol_host_init failed\n");
        	return 0;
    }

    if (-1 == biz_m_energy_init(fd)) {
		BIZ_ERROR("biz_m_energy_init failed\n");
        	return 0;
    }

    if (-1 == biz_m_rub_net_init(fd)) {
		BIZ_ERROR("biz_m_rub_net_init failed\n");
        	return 0;
    }


    if (-1 == biz_m_dev_nickname_init(fd)) {
		BIZ_ERROR("biz_m_dev_nickname_init failed\n");
        	return 0;
    }

    /* ÑÓÊ±£¬µÈ´ýÁªÍø */
	cyg_thread_delay(20*100);

	int i;
	while(1){
		for (i = 0; i <= 1000; i++) {
			biz_m_wifi_push_wifi_info();
			cyg_thread_delay(5*100);
			biz_m_rub_net_push_strange_host_info();
		}
	};
	
	return 0;
}

static void biz_main(void)
{
	biz_main_loop();
}

void biz_proc_start(void)
{
                cyg_thread_create(
                        14,      
                        (cyg_thread_entry_t *)biz_main, 
                        0, 
                        "business proc",
                        biz_stack, 
                        sizeof(biz_stack), 
                        &biz_handle, 
                        &biz_thread);
                cyg_thread_resume(biz_handle);

        return ;
}
#endif

void  biz_m_ucloud_proc_start(void)
{
	ucloud_start();
#ifndef CONFIG_APP_COSTDOWN
	biz_proc_start();
#endif
}
