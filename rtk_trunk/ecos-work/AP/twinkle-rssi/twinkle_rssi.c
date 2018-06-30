#include "twinkle_rssi.h"

static int get_file_value(unsigned char *target, unsigned char *value, unsigned char *cmd)
{
	FILE *fp=stdout;
	unsigned char line[200], name[64], tmp[128];
	unsigned char *p;
	
	if((stdout=fopen(SC_FILE_OUTPUT,"w"))==NULL)
	{
		fprintf(stderr,"redirect output fail!\n");
		stdout=fp;
		return -1;
	}

	run_clicmd(cmd);
	
	fclose(stdout);
	stdout=fp;	

	if((fp=fopen(SC_FILE_OUTPUT,"r"))==NULL)
	{
		fprintf(stderr,"open sc temp file fail!\n");
		return -1;
	}
	while ( fgets(line, 200, fp) ) {
		if (line[0] == '#')
			continue;
		if(strstr(line, target)==0)
			continue;
		sscanf(line, "%[^:]",tmp);
		p=tmp;
		while(*p == ' ')
			p++;
		strcpy(name, p);
		
		sscanf(line, "%*[^:]:%[^?]",tmp);
		p=tmp;
		while(*p == ' ')
			p++;
		strcpy(value, p);
		value[strlen(value) - 1] = '\0';
		break;
	}
	fclose(fp);
}

static void get_current_rssi_status(int *current_rssi_level)
{
	char ifname[10]="wlan0-vxd";
	char buffer[64];
	char value[32];

	sprintf(buffer, "%s mib_bssdesc", ifname);
	get_file_value("rssi", value, buffer);
	*current_rssi_level = atoi(value);
}


static void milliseconds_sleep(unsigned long mSec)
{
	struct timeval tv;
	tv.tv_sec = mSec/1000;
	tv.tv_usec = (mSec%1000)*1000;
	int err;
	do{
		err = select(0,NULL,NULL,NULL,&tv);
	}while(err<0 && errno==EINTR);
}

static void get_signal_rssi_threshold(char* wlan, int *good, int *normal, int *poor)
{
	apmib_get(MIB_WLAN_SC_GOOD_RSSI_THRESHOLD, (void *)good);
	apmib_get(MIB_WLAN_SC_NORMAL_RSSI_THRESHOLD, (void *)normal);
	apmib_get(MIB_WLAN_SC_POOR_RSSI_THRESHOLD, (void *)poor);
    printf("getSignalRssiThreshold: %d %d %d \n",*good,*normal,*poor);
}

int twinkle_rssi_main()
{
	int led_blink_flag=0;
    int led_blink_timer=0;
    int sinal_good_rssi_threshold;
    int sinal_normal_rssi_threshold;
    int sinal_poor_rssi_threshold;
	int current_rssi_level;
	unsigned char buffer[32];
	
	//get rssi threshold
	get_signal_rssi_threshold("wlan0",&sinal_good_rssi_threshold,&sinal_normal_rssi_threshold,&sinal_poor_rssi_threshold);

	while(1){
		//wait 250ms
		milliseconds_sleep(250);
		led_blink_timer++;
		//get current rssi
		get_current_rssi_status(&current_rssi_level);
		//compare current rssi and threshold
		/*if(current_rssi_level == 0)
			led_blink_flag = 0;
        else if(current_rssi_level >= sinal_good_rssi_threshold)
        	led_blink_flag = 1;
        else if(current_rssi_level<sinal_good_rssi_threshold && current_rssi_level>=sinal_normal_rssi_threshold)
        	led_blink_flag = led_blink_timer%2;
        else if(current_rssi_level<sinal_normal_rssi_threshold && current_rssi_level>=sinal_poor_rssi_threshold)
        	led_blink_flag = (led_blink_timer%4)<2 ? 0:1;
        else if(current_rssi_level<=sinal_poor_rssi_threshold)
         	led_blink_flag = (led_blink_timer%8)<4 ? 0:1;*/

		if(current_rssi_level == 0)
			led_blink_flag = 0;
        else if(current_rssi_level >= sinal_good_rssi_threshold)
        		led_blink_flag = led_blink_timer%2;
        else if(current_rssi_level<sinal_good_rssi_threshold && current_rssi_level>=sinal_normal_rssi_threshold)
        	led_blink_flag = 1;
        else if(current_rssi_level<=sinal_normal_rssi_threshold)
         	led_blink_flag = (led_blink_timer%8)<4 ? 0:1;
		
       if(led_blink_flag){
	   		sprintf(buffer, "wlan0-vxd led 1");
			run_clicmd(buffer);//wlan led
			//system_led_on();//system led
			
       	}
       else{
			sprintf(buffer, "wlan0-vxd led 0");
			run_clicmd(buffer);//wlan led
      		//system_led_off();//system led
       	}
       //if(led_blink_timer%10 == 0)
          	//printf("simple config: rssi=%d  time=%d  \n",current_rssi_level,led_blink_timer);    
	}
	return 0;
}


#define TWINKLE_RSSI_THREAD_STACK_SIZE 0x00006000
#define TWINKLE_RSSI_THREAD_PRIORITY 16
cyg_uint8  twink_rssi_stack[TWINKLE_RSSI_THREAD_STACK_SIZE];
cyg_handle_t twink_rssi_thread;
cyg_thread  twink_rssi_thread_object;
static int twinkle_rssi_enable_flag = 0;
	
void create_twinkle_rssi(void)
{
	if(twinkle_rssi_enable_flag)
		return;
	else
		twinkle_rssi_enable_flag = 1;
		
	/* Create the thread */
	cyg_thread_create(TWINKLE_RSSI_THREAD_PRIORITY,
		      twinkle_rssi_main,
		      0,
		      "twinkle_rssi",
		      &twink_rssi_stack,
		      sizeof(twink_rssi_stack),
		      &twink_rssi_thread,
		      &twink_rssi_thread_object);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(twink_rssi_thread);
}

