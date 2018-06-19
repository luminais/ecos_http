#include <stdlib.h>
#include <sys/param.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <shutils.h>
#include <ecos_oslib.h>
#include  "auto_conn.h"

int tenda_auto_conn_vif_extender = -1; 			//record auto-conn status
int AutoConnDebugLevel = AUTO_CONN_DEBUG_OFF;	//record auto-conn debug level


void auto_conn_debug_level(int level)
{
	if(level > 0)
		AutoConnDebugLevel = AUTO_CONN_DEBUG_ERR;
	else
		AutoConnDebugLevel = AUTO_CONN_DEBUG_OFF;

	diag_printf("auto_conn debug level = %d\n", AutoConnDebugLevel);
}

int get_auto_status()
{
	return tenda_auto_conn_vif_extender;
}

int is_undo_status()
{
	if(tenda_auto_conn_vif_extender == AUTO_CONN_VIF_EXTEND_UNDO)
		return 1;
	else
		return 0;
}

int is_init_status()
{
	if(tenda_auto_conn_vif_extender == AUTO_CONN_VIF_EXTEND_INIT)
		return 1;
	else
		return 0;
}

int is_doing_status()
{
	if(tenda_auto_conn_vif_extender == AUTO_CONN_VIF_EXTEND_DOING)
		return 1;
	else
		return 0;
}

int set_auto_status(int status)
{
	if(status < AUTO_CONN_VIF_EXTEND_UNDO || status > AUTO_CONN_VIF_EXTEND_DOING)
	{
		printf("set status fail!\n");
		return 0;
	}
	
	tenda_auto_conn_vif_extender = status;
	return 1;
}

void set_undo_status()
{
	tenda_auto_conn_vif_extender = AUTO_CONN_VIF_EXTEND_UNDO;
}

void set_init_status()
{
	tenda_auto_conn_vif_extender = AUTO_CONN_VIF_EXTEND_INIT;
}

void set_doing_status()
{
	tenda_auto_conn_vif_extender = AUTO_CONN_VIF_EXTEND_DOING;
}

void add_vif_ap_ifnames()
{
	char vif[]= "wl0.2";
	char add_br[] = "lan_ifnames" ;
	char name[IFNAMSIZ], *next = NULL;
	int need_to_add = 1;
	char buf[255];
	
	memset(buf,0,sizeof(buf));
	strcpy(buf, nvram_recv_safe_get(add_br));
	
	foreach(name, buf, next) {
		if (!strcmp(name,vif)){
			need_to_add = 0;
			break;
		}
	}
	
	if (need_to_add) {
		/* the first entry ? */
		if (*buf)
			strncat(buf," ",1);
		strncat(buf, vif, strlen(vif));
	}
	nvram_set(add_br, buf);

	char add_vifs[] = "wl0_vifs";
	memset(buf,0,sizeof(buf));
	need_to_add = 1;
	strcpy(buf, nvram_recv_safe_get(add_vifs));
	
	foreach(name, buf, next) {
		if (!strcmp(name,vif)){
			need_to_add = 0;
			break;
		}
	}

	if (need_to_add) {
		/* the first entry ? */
		if (*buf)
			strncat(buf," ",1);
		strncat(buf, vif, strlen(vif));
	}
	nvram_set(add_vifs, buf);
}

