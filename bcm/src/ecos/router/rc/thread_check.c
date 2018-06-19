
#include <sys/param.h>
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <bcmnvram.h>
#include <netdb.h>

#include <osl.h>
#include <siutils.h>

/*hqw add for num_thread_check*/
struct list 
{ 
	char name[128];
	int  state;
	int  remark;
	struct list *next; 
}; 
typedef struct list node; 
typedef node *link; 
link head = NULL;

#define UNLOCK 		0
#define LOCKED   		1

extern void
board_reboot(void);

/*
CLI> thread
id    state    Pri(Set) Name                   StackBase   Size   usage
0037  RUN      9  ( 9 ) DDNS                   0x80b665b8  8192   2128  
0038  SLEEP    8  ( 8 ) DHCP_server            0x8029c9d8  12288  2788  
0035  RUN      10 ( 10) IGD                    0x80b59348  16384  3816  
0036  EXIT     7  ( 7 ) WPSM                   0x80b5d448  32768  2552  
0012  RUN      1  ( 1 ) cli console            0x8027ff30  16384  1860  
0032  EXIT     8  ( 8 ) dhcpc_vlan2            0x8029a1d8  10240  2792  
0042  EXIT     6  ( 6 ) run_sh0                0x802772a8  16384  3440  
0040  EXIT     6  ( 6 ) SNTP                   0x80b48d88  4096   1504  
0001  RUN      31 ( 31) Idle Thread            0x802c0d60  2048   1012  
0002  SLEEP    6  ( 6 ) main                   0x80b4a608  32768  7496  
0003  SLEEP    5  ( 5 ) Network support        0x802c3180  16384  3828  
0004  RUN      8  ( 8 ) httpd_main             0x802ad790  32768  2968  
0033  SLEEP    7  ( 7 ) EAPD                   0x80297de8  8192   2540  
0034  SLEEP    7  ( 7 ) NAS                    0x80293ce4  16384  3872  
0043  RUN      9  ( 9 ) DNS daemon             0x802a54ec  12288  2044  
*/

int thread_check(void)
{
			cyg_handle_t thread = 0;
			cyg_uint16 id = 0;
			int more;
			int ret;

			int running;

			static int running6[6] = {0,0,0,0,0,0};
			static int check_time = 0;
			
			/* Loop over the threads, and generate a table row for
			 * each.
			 */

			running = 0;
			ret = UNLOCK;
			
			do 
			{
				
			    cyg_thread_info info;

			    more=cyg_thread_get_next( &thread, &id );
			    if (thread==0)
			        break;
			
			    cyg_thread_get_info( thread, id, &info );
				//printf("thread name = %s,\t state = %d;\n",info.name,info.state);
			
			    /* Translate the state into a string.
			     */
			    if( info.state == 0 && id != 12/*thread check*/ && id != 1/*Idle Thread*/){
				        running++;
			    }
				
			    cyg_thread_delay(1); //add by z10312 减少delay时间，以便于上一级函数1秒喂狗
				
			} while (more==true) ;

			if(check_time >= 0 && check_time <= 5)
			{
				running6[check_time] = running;
			}
			else
			{
				check_time = 0;
				return ret;
			}
				
			check_time++;
			
			if(check_time == 2)
			{
				if((running6[0]+ running6[1])> 36/*?FIX ME*/)
				{
					if(running6[1] < running6[0])
					{
						check_time = 0;
						running6[0] = 0;
						running6[1] = 0;
					}
					//else we meed more check times
				}else
				{
					//check again from start
					check_time = 0;
					running6[0] = 0;
					running6[1] = 0;
				}
			}
			
			if(check_time == 4)
			{
				if((running6[0]+ running6[1]+running6[2]+ running6[3])>= 18*4)
				{
					if(! (running6[0] == running6[1] && running6[1] == running6[2] && running6[2] == running6[3])){
						//check again from start
						running6[0] = 0;
						running6[1] = 0;
						running6[2] = 0;
						running6[3] = 0;
						check_time = 0;
					}
					//else we need more check times
				}else{
						//check again from start
						running6[0] = 0;
						running6[1] = 0;
						running6[2] = 0;
						running6[3] = 0;
						check_time = 0;
				}
			}

			if(check_time == 6)
			{
				if((running6[0]+ running6[1]+running6[2]+ running6[3] + running6[4] + running6[5])>= 18*6)
				{
					diag_printf("\n%s: running1=%d running2=%d running3=%d running4=%d running5=%d running6=%d\n",
							__FUNCTION__,
							running6[0],running6[1],running6[2],running6[3],running6[4],running6[5]);
					
					if(running6[0] == running6[1] && running6[1] == running6[2] && running6[2] == running6[3] && running6[3] == running6[4] && running6[4] == running6[5]){
						ret = LOCKED;
					}
				}
				//check again from start
				running6[0] = 0;
				running6[1] = 0;
				running6[2] = 0;
				running6[3] = 0;
				running6[4] = 0;
				running6[5] = 0;
				check_time = 0;
			}

			return 	ret;
}
#if 0
void clean_thread_list(void)
{
	link ptr = head;
	if(ptr == NULL)
		return;
	while(ptr != NULL)
	{
		ptr->remark = 0;
		ptr=ptr->next;
	}
	return;
}

int add_list(char *name,int state)
{
	if(head == NULL)
	{
		head = (link)malloc(sizeof(node));
		if(head == NULL)
		{
			return -1;
		}
		strcpy(head->name,name);
		head->state = state;
		head->remark = 0;
		head->next = NULL;
		return 0;
	}
	link ptr,ptr1,ptr2;
	ptr = head;
	ptr1 = head;
	ptr2 = ptr1;
	while(ptr1 != NULL)
	{
		//printf("name =%s,ptr1->name=%s,state=%d,remark=%d,length=%d\n",name,ptr1->name,ptr1->state,ptr1->remark,strlen(ptr1->name));
		if(strncmp(name,ptr1->name,strlen(ptr1->name)) != 0)
		{
			ptr2 = ptr1;
			ptr1 = ptr1->next;
		}
		else
		{
			//状态由run(0)变成run(0)则该节点remark++，状态由run(0)变成sleep(1)、run(0)变成EXIT(16)
			//sleep(1)变成EXIT(16)、sleep(1)变成run(0)、exit(16)变成run(0)、exit(16)变成sleep(1)则将
			//所有节点的remark变成0，如果某一个节点的remark为6则认为是cpu已经挂掉
			if(state == 0)
			{
				if(ptr1->state == 0)
				{
					ptr1->remark = ptr1->remark + 1;
					//printf("name =%s,state=%d,remark=%d\n",
						  		//ptr1->name,ptr1->state,ptr1->remark);
				}
				else
				{
					clean_thread_list();
				}
			}
			else if(state != ptr1->state)
			{
				clean_thread_list();
			}
			ptr1->state = state;
			//printf("%s->state = %d\n",ptr1->name,state);
			return 0;
		}
	}
	ptr2->next = (link)malloc(sizeof(node));
	if(ptr2->next == NULL)
	{
		return -1;
	}
	strcpy(ptr2->next->name,name);
	ptr2->next->state = state;
	ptr2->next->remark = 0;
	ptr2->next->next = NULL;
	return 0;
}

#define DOWN 		    0
#define UP   		    1
#define FAIL   		    -1


int check_list(void)
{
	link ptr = head;
	if(ptr == NULL)
		return UP;
	while(ptr != NULL)
	{
		if(ptr->remark == 20)
		{
			return DOWN;
		}
		else
		{
			ptr = ptr->next;
		}
	}
	return UP;
}

int num_thread_check(void)
{
	cyg_handle_t thread = 0;
	cyg_uint16 id = 0;
	int more;

	int running;
	
	/* Loop over the threads, and generate a table row for
	 * each.
	 */

	running = 0;
	
	do 
	{
		
	    cyg_thread_info info;

	    more=cyg_thread_get_next( &thread, &id );
	    if (thread==0)
	        break;
	
	    cyg_thread_get_info( thread, id, &info );
		if(info.set_pri != 1 && info.set_pri != 31 && strcmp("Network support",info.name) != 0)
		{		
			//printf("thread name = %s,state = %d,set_pri = %d\n",info.name,info.state,info.set_pri);
			if(add_list(info.name,info.state) == -1)
			{
				return FAIL;
			}
		}
	
	    /* Translate the state into a string.
	     */
	    if( info.state == 0 && info.set_pri != 1 && info.set_pri != 31 && strcmp("Network support",info.name) != 0){
		        running++;
	    }

	    cyg_thread_delay(10);
		
	} while (more==true) ;
	if(running > 6)
	{
		if(check_list() == DOWN)
		{
			return DOWN;
		}
	}
	return UP;
}
#endif
extern void show_splx(void);

//add by z10312 移植喂狗模块到线程检测机制 2015-0506
extern si_t *bcm947xx_sih;
#define THREAD_CHECK_MAX 5
#define THREAD_CHECK_WATCHDOG_DELAY_NUM 10
//无配置时,默认喂狗时间为15秒,有配置时,根据配置喂狗,0 则为禁止喂狗
int watchdog = 15000;  //ms
//end


void task_monitor()
{
	
	//add by z10312 移植喂狗模块到线程检测机制 2015-0506
	int i = 0;
	char *watchdog_value;
	watchdog_value = nvram_safe_get("watchdog");
	if (strlen(watchdog_value) > 0 )
	{
		watchdog = atoi(watchdog_value);
		printf (" %s %d si_watchdog_ms waitime=%dms= \n", __FUNCTION__, __LINE__, watchdog);
	}
	//end
			
	while (1) {
		
		i++;
		cyg_thread_delay(100);
		
	    /* Add the thead status check here */
		//if(num_thread_check() == DOWN)
	    if (i >= THREAD_CHECK_MAX)
	    {
	    	
		    if(thread_check() == LOCKED)
		    {
   			printf("-------------------------------reboot---------------------\n");
   			show_splx();
   			board_reboot();               /* See src/ecos/bsp/arch/mips/brcm-boards/bcm947xx/board_misc.c*/
		    }
			
		    i = 0;		
	    }
			
		//add by z10312 移植喂狗模块到线程检测机制 2015-0506
		if(watchdog > 0)
		{	
			//printf ("z10312 %s %d si_watchdog_ms waitime=%dms= \n", __FUNCTION__, __LINE__, watchdog);
			si_watchdog_ms(bcm947xx_sih, watchdog);
		}	
		//endif
		
	}
}


/* Entry point of CLI */
static cyg_handle_t thread_check_handle;
static cyg_thread thread_check_thread;
static char thread_check_stack[1024+1024];

void
thread_check_start(void)
{
	cyg_thread_create(
		1,
		(cyg_thread_entry_t *)task_monitor,
		0,
		"thread check",
		(void *)&thread_check_stack[0],
		sizeof(thread_check_stack),
		&thread_check_handle,
		&thread_check_thread);
	
	cyg_thread_resume(thread_check_handle);
}

