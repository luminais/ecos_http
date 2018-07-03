/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ddns_main.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <network.h>
#include <ddns.h>
#include <stdlib.h>
#include <stdio.h>


//==============================================================================
//                                    MACROS
//==============================================================================

#define	MINUTE_TICKS			(30*100)
#define DDNS_MIN_UPDATE_TIME	(cyg_current_time()+MINUTE_TICKS)

#define DDNS_EVENT_CONFIG		0x01
#define DDNS_EVENT_DOWN			0x02
#define	DDNS_EVENT_UPDATE		0x04
#define DDNS_EVENT_CORRTIME		0x08

#define DDNS_EVENT_ALL      (DDNS_EVENT_CONFIG | DDNS_EVENT_DOWN | DDNS_EVENT_UPDATE | DDNS_EVENT_CORRTIME)

//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================
cyg_flag_t ddns_event_id;
extern int DDNS_running;

struct user_info *ddns_user_list = NULL;
//static int DDNS_sleep = 1;

//==============================================================================
//                              EXTERNAL FUNCTIONS
//==============================================================================
extern void update_ddns_config(struct user_info *info);
extern void oray_set_online_status(int);

void load_config(void);

void DDNS_init(void) 
{	
	load_config();
}

void DDNS_down(void) 
{
	oray_set_online_status(0);
	if (DDNS_running)
		cyg_flag_setbits(&ddns_event_id, DDNS_EVENT_DOWN); //send msg to DDNS_main
}

void DDNS_config(void)
{
	if (DDNS_running)
		cyg_flag_setbits(&ddns_event_id, DDNS_EVENT_CONFIG);
}

void DDNS_update(void)
{
	if (DDNS_running)
		cyg_flag_setbits(&ddns_event_id, DDNS_EVENT_UPDATE);
}

void DDNS_corrtime(void)
{
	if (DDNS_running)
		cyg_flag_setbits(&ddns_event_id, DDNS_EVENT_CORRTIME);
}


void clean_account(struct user_info **info)
{
	struct user_info *acc, *next_acc;
	
	acc = *info;
	while (acc)
	{
		next_acc = acc->next;
		free(acc);
		acc = next_acc;
	}
	
	*info = NULL;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void DDNS_add_account(struct user_info *account)
{
	struct user_info *acc = ddns_user_list;
	struct user_info *pre_acc = NULL;
	
	if (!account)
		return;
	//search if account exist in list
	while(acc)
	{
		if (acc->service == account->service && strcmp(acc->host, account->host) == 0)
		{
			if (pre_acc)
				pre_acc->next = acc->next;
			else
				ddns_user_list = acc->next;
			
			free(acc); //remove the old one
			break;
		}
		pre_acc = acc;
		acc = acc->next;
	}
	//Insert or put in top
	account->next = ddns_user_list;
	ddns_user_list = account;
}


//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void do_update(struct user_info *info)
{
	//Call the ddns update process
	info->status = info->service->update_entry(info);	
	DDNS_DBG("%s: status=%d",__FUNCTION__,info->status);
	switch (info->status)
	{
	case UPDATERES_OK:
		nvram_set(ADVANCE_DDNS_STATUS,"Connected");		
    	info->ip = MY_IP;
	
		// Set re-update time
		if (info->service->manual_timeout == 0)
		{
			DDNS_LOG("update %s with IP %s successfully", info->service->default_server, MY_IP_STR);
 			info->updated_time = time(0);//GMTtime(0);
			info->ticks = info->trytime * 100;
		}
		break;
	//针对回传good添加另一种	判断
	case UPDATERES_READY:
	    	DDNS_LOG("READY: ready to update %s with IP %s", info->service->default_server, MY_IP_STR);
	    	info->ip = 0;
	    	info->updated_time = 0;
		info->ticks = MINUTE_TICKS;		//  one minute later
		break;

		
	case UPDATERES_ERROR:
    	DDNS_LOG("ERROR: failed to update %s with IP %s", info->service->default_server, MY_IP_STR);
    	info->ip = 0;
    	info->updated_time = 0;
		info->ticks = MINUTE_TICKS;		// Try it one minute later
		break;

	case UPDATERES_SHUTDOWN:
	default:
		nvram_set(ADVANCE_DDNS_STATUS,"Disconnected");
    	DDNS_LOG("SHUTDOWN: failed to update %s with IP %s", info->service->default_server, MY_IP_STR);
    	info->ip = -1;
    	info->updated_time = 0;
		info->ticks = 0;				// Never retry
		break;
    }
	
	update_ddns_config(info);
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void check_update(void)
{
	struct user_info *info;

	if (MY_IP == 0)
		return;
		
	for (info = ddns_user_list; info; info=info->next)
	{	
		/* UPDATERES_SHUTDOWN mean server has something wrong, 
		   so we should not keep to update */
		if (info->status == UPDATERES_SHUTDOWN)
			continue;
		/*pxy revise 2013.06.20*/		
#if 0		
		if (info->ip != MY_IP)
		{
			if (info->service->init_entry)
				(*info->service->init_entry)(info);
			
			do_update(info);
		}
#else
		do_update(info);
#endif
	}
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void do_corrtime(void)
{
	struct user_info *info;

	if (MY_IP == 0)
		return;

	for (info = ddns_user_list; info; info=info->next)
	{
		if (info->status == UPDATERES_SHUTDOWN)
			continue;

		// Check if time corrected,
		if (info->updated_time != 0 && info->updated_time < 631123200)
		{
			//????????????????????????????????????
			// This is not the time we update it
			// It's the time NTP set correct time
			// TBC
			info->updated_time = time(0);//GMTtime(0);

			update_ddns_config(info);
		}
	}
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void do_retrytime(void)
{
	struct user_info *info;
DDNS_DBG("%s: %x\n",__FUNCTION__,MY_IP);
	if (MY_IP == 0)
		return;

	for (info = ddns_user_list; info; info=info->next)
	{
		DDNS_DBG("%s: %d\n",__FUNCTION__,info->ticks);
		if (info->ticks != 0)
		{
			info->ticks -= MINUTE_TICKS;
			if (info->ticks <= 0 || info->ip != MY_IP)
			{
				do_update(info);
			}
		}
	}
}
extern int ORAY_init_entry(struct user_info *info);
void DDNS_mainloop(void) 
{
	unsigned long event;
	/* Do init */
	DDNS_init();

	/*make sure ddns_event_id init first*/
	cyg_flag_init(&ddns_event_id);

	nvram_set(ADVANCE_DDNS_STATUS,"Connectting");

	while(1)
	{
		if ((event = cyg_flag_timed_wait(&ddns_event_id,
						DDNS_EVENT_ALL,
						(CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR),
						DDNS_MIN_UPDATE_TIME)) != 0)
		{
			if (event & DDNS_EVENT_CONFIG)
			{
				DDNS_DBG("DDNS_config\n");
				check_update();
			}

			if (event & DDNS_EVENT_UPDATE)
			{
				DDNS_DBG("DDNS_update\n");
				ORAY_init_entry(NULL);
				if(strcmp(nvram_get(ADVANCE_DDNS_STATUS),"Disconnected"))
					nvram_set(ADVANCE_DDNS_STATUS,"Connectting");
				check_update();
			}

			if (event & DDNS_EVENT_DOWN)
			{
//				DDNS_DBG("DDNS_down\n");
				ORAY_init_entry(NULL);
				break;
			}

			if (event & DDNS_EVENT_CORRTIME)
			{
				DDNS_DBG("DDNS_corrtime\n");
				do_corrtime();
			}
		}
		else
		{
			// Come here every minute
			DDNS_DBG("%s: do_retrytime\n",__FUNCTION__);
			do_retrytime();
		}	
	}
	nvram_set(ADVANCE_DDNS_STATUS,"Disconnected");
	nvram_commit();
	cyg_flag_destroy(&ddns_event_id);
	clean_account(&ddns_user_list);
}


