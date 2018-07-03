#ifdef DAYLIGHT_SAVING_TIME

#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/io/flash.h>
#include <cyg/io/watchdog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


#include "sys_utility.h"
#include "hw_settings.h"
#include "net_api.h"
#include "apmib.h"
#include "../common/common.h"

//structure of dayligth start and end time point
struct dst_time_point
{
	int dst_mon;
	int dst_mweek;
	int dst_wday;
	int dst_hour;
};

static struct dst_time_point dst_start, dst_end;
unsigned char dst_time_loop_stack[4096];
cyg_handle_t dst_time_loop_thread_handle;
cyg_thread dst_time_loop_thread_obj;
static int dst_time_flag = 0;

//weekday
int caculate_weekday(int year, int mon, int day)
{
	int w;
	year = year + 1900;
	mon = mon + 1;
	if(mon == 1 || mon == 2)
	{
		mon += 12;
		year--;
	}
	w = (day + 1 + 2*mon +3*(mon+1)/5 + year + year/4 - year/100 + year/400)%7;
//	diag_printf("%s:%d %d/%d/%d is %d\n",__FILE__,__LINE__, year, mon, day, w);
	return w;
}

/* getDSTOffset
	if 'tm_time' is between 'dst_start' and 'dst_end', return 3600
	else return 0
 */
int get_dst_offset(const struct tm* tm_time, int flag)
{
	int dst_offset = 0, wday_std, week_std, wday_dst, week_dst, tmp;
	int mday;
	int mon_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	time_t tm;
	struct tm *tm_time_std, *tm_time_dst;
	
	if(dst_start.dst_mon<0 || dst_end.dst_mon<0)
		return 0;

	if(flag == 0)
	{
		tm_time_std = tm_time;
		tm = mktime_rewrite(tm_time);//	tm = tm+3600;
		tm_time_dst = localtime(&tm);
	}
	else if(flag == 1)
	{
		tm_time_std = tm_time;
		tm_time_dst = tm_time;
	}
	else
	{
		tm_time_dst = tm_time;
		tm = mktime_rewrite(tm_time);
		tm = tm -7200;
		tm_time_std = localtime(&tm);
	}
//	diag_printf("%s:%d time1 hour is %d, time2 hour is %d\n",__FILE__,__LINE__,tm_time_std->tm_hour, tm_time_dst->tm_hour);
	
	wday_std = caculate_weekday(tm_time_std->tm_year, tm_time_std->tm_mon, tm_time_std->tm_mday);
	tmp = (tm_time_std->tm_mday - wday_std - 1)%7;
	if(tmp <= 0)
		week_std = (tm_time_std->tm_mday - wday_std - 1)/7 + 1;
	else
		week_std = (tm_time_std->tm_mday - wday_std - 1)/7 + 2;

	wday_dst = caculate_weekday(tm_time_dst->tm_year, tm_time_dst->tm_mon, tm_time_dst->tm_mday);
	tmp = (tm_time_dst->tm_mday - wday_dst - 1)%7;
	if(tmp <= 0)
		week_dst = (tm_time_dst->tm_mday - wday_dst - 1)/7 + 1;
	else
		week_dst = (tm_time_dst->tm_mday - wday_dst - 1)/7 + 2;

	//diag_printf("%s:%d tmp is %d, week is %d\n",__FILE__,__LINE__,tmp, week);
	//diag_printf("%s:%d time=%d/%d/%d/%d\n",__FILE__,__LINE__,tm_time->tm_year, tm_time->tm_mon, tm_time->tm_mday, tm_time->tm_hour);

	if(dst_start.dst_mon < dst_end.dst_mon)
	{
		if(dst_start.dst_mon < tm_time_std->tm_mon && dst_end.dst_mon > tm_time_dst->tm_mon)
			return 3600;
		else if(dst_start.dst_mon == tm_time_std->tm_mon)
		{
			mday = (tm_time_std->tm_mday - wday_std + dst_start.dst_wday)%7;
			if(mday == 0)
				mday = 7;
			mday = mday + (dst_start.dst_mweek - 1)*7;
			if(mday > mon_day[tm_time_std->tm_mon])
				mday = mday - 7;
//			diag_printf("%d %d %d is %d\n", dst_start.dst_mon, dst_start.dst_mweek, dst_start.dst_wday, mday);
			if(tm_time_std->tm_mday > mday)
				return 3600;
			else if(tm_time_std->tm_mday == mday)
			{
				if(tm_time_std->tm_hour >= dst_start.dst_hour)
					return 3600;
				else
					return 0;
			}
			else
				return 0;
		}
		else if(dst_end.dst_mon == tm_time_dst->tm_mon)
		{
			mday = (tm_time_dst->tm_mday - wday_dst + dst_end.dst_wday)%7;
			if(mday == 0)
				mday = 7;
			mday = mday + (dst_end.dst_mweek - 1)*7;
			if(mday > mon_day[tm_time_dst->tm_mon])
				mday = mday - 7;
//			diag_printf("%d %d %d is %d\n", dst_end.dst_mon, dst_end.dst_mweek, dst_end.dst_wday, mday);
			if(tm_time_dst->tm_mday < mday)
				return 3600;
			else if(tm_time_dst->tm_mday == mday)
			{
				if(tm_time_dst->tm_hour < dst_end.dst_hour)
					return 3600;
				else
					return 0;
			}
			else
				return 0;
		}
		else
			return 0;
	}
	else
	{
		if(dst_start.dst_mon < tm_time_std->tm_mon || dst_end.dst_mon > tm_time_dst->tm_mon)
			return 3600;
		else if(dst_start.dst_mon == tm_time_std->tm_mon)
		{
			mday = (tm_time_std->tm_mday - wday_std + dst_start.dst_wday)%7;
			if(mday == 0)
				mday = 7;
			mday = mday + (dst_start.dst_mweek - 1)*7;
			if(mday > mon_day[tm_time_std->tm_mon])
				mday = mday - 7;
//			diag_printf("%d %d %d is %d\n", dst_start.dst_mon, dst_start.dst_mweek, dst_start.dst_wday, mday);
			if(tm_time_std->tm_mday > mday)
				return 3600;
			else if(tm_time_std->tm_mday == mday)
			{
				if(tm_time_std->tm_hour >= dst_start.dst_hour)
					return 3600;
				else
					return 0;
			}
			else
				return 0;
		}
		else if(dst_end.dst_mon == tm_time_dst->tm_mon)
		{
			mday = (tm_time_dst->tm_mday - wday_dst + dst_end.dst_wday)%7;
			if(mday == 0)
				mday = 7;
			mday = mday + (dst_end.dst_mweek - 1)*7;
			if(mday > mon_day[tm_time_dst->tm_mon])
				mday = mday - 7;
//			diag_printf("%d %d %d is %d\n", dst_end.dst_mon, dst_end.dst_mweek, dst_end.dst_wday, mday);
			if(tm_time_dst->tm_mday < mday)
				return 3600;
			else if(tm_time_dst->tm_mday == mday)
			{
				if(tm_time_dst->tm_hour < dst_end.dst_hour)
					return 3600;
				else
					return 0;
			}
			else
				return 0;
		}
		else
			return 0;
	}
}

struct tm get_time_from_flash()
{
	int cur_year=0;
	struct tm tm_time;
	apmib_get( MIB_SYSTIME_YEAR, (void *)&cur_year);

	if (cur_year != 0) 
	{
		tm_time.tm_year = cur_year - 1900;
		apmib_get( MIB_SYSTIME_MON, (void *)&(tm_time.tm_mon));
		apmib_get( MIB_SYSTIME_DAY, (void *)&(tm_time.tm_mday));
		apmib_get( MIB_SYSTIME_HOUR, (void *)&(tm_time.tm_hour));
		apmib_get( MIB_SYSTIME_MIN, (void *)&(tm_time.tm_min));
		apmib_get( MIB_SYSTIME_SEC, (void *)&(tm_time.tm_sec));
	}
	return tm_time;
}

void get_start_end_time_point(char *time_zone)
{
//	diag_printf("timezone is %s\n", time_zone);
	if(strcmp(time_zone,"9 1") == 0 || strcmp(time_zone,"8 1") == 0 ||
		strcmp(time_zone,"7 2") == 0 || strcmp(time_zone,"6 1") == 0 ||
		strcmp(time_zone,"6 2") == 0 || strcmp(time_zone,"5 2") == 0 ||
		strcmp(time_zone,"5 3") == 0 )
	{
		//sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
		dst_start.dst_mon = 4;
		dst_start.dst_mweek = 1;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 2;
	}
	else if(strcmp(time_zone,"4 3") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M10.2.0/00:00:00,M3.2.0/00:00:00");
		dst_start.dst_mon = 10;
		dst_start.dst_mweek = 2;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 0;
		dst_end.dst_mon = 3;
		dst_end.dst_mweek = 2;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 0;
	}
	else if(strcmp(time_zone,"3 1") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M4.1.0/00:00:00,M10.5.0/00:00:00");
		dst_start.dst_mon = 4;
		dst_start.dst_mweek = 1;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 0;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 0;
	}
	else if(strcmp(time_zone,"3 2") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M2.2.0/00:00:00,M10.2.0/00:00:00");
		dst_start.dst_mon = 2;	
		dst_start.dst_mweek = 2;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 0;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 2;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 0;
	}
	else if(strcmp(time_zone,"1 1") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/00:00:00,M10.5.0/01:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 0;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 1;
	}
	else if(strcmp(time_zone,"0 2") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/01:00:00,M10.5.0/02:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 1;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 2;
	}
	else if(strcmp(time_zone,"-1") == 0 || strcmp(time_zone,"-2 1") == 0)	
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 3;
	}
	else if(strcmp(time_zone,"-2 2") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/03:00:00,M10.5.0/04:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 3;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 4;
	}
	else if(strcmp(time_zone,"-2 3") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M4.5.5/00:00:00,M9.5.5/00:00:00");
		dst_start.dst_mon = 4;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 5;
		dst_start.dst_hour = 0;
		dst_end.dst_mon = 9;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 5;
		dst_end.dst_hour = 0;
	}
	else if(strcmp(time_zone,"-2 5") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/03:00:00,M10.5.5/04:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 3;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 5;
		dst_end.dst_hour = 4;
	}
	else if(strcmp(time_zone,"-2 6") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.5/02:00:00,M10.1.0/02:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 5;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 1;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 2;
	}
	else if(strcmp(time_zone,"-3 2") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00")'
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 3;
	}
	else if(strcmp(time_zone,"-4 2") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/04:00:00,M10.5.0/05:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 4;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 5;
	}
	else if(strcmp(time_zone,"-9 4") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M10.5.0/02:00:00,M4.1.0/03:00:00");
		dst_start.dst_mon = 10;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 4;
		dst_end.dst_mweek = 1;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 3;
	}
	else if(strcmp(time_zone,"-10 2") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M10.5.0/02:00:00,M4.1.0/03:00:00");
		dst_start.dst_mon = 10;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 4;
		dst_end.dst_mweek = 1;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 3;
	}
	else if(strcmp(time_zone,"-10 4") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M10.1.0/02:00:00,M4.1.0/03:00:00");
		dst_start.dst_mon = 10;
		dst_start.dst_mweek = 1;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 4;
		dst_end.dst_mweek = 1;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 3;
	}
	else if(strcmp(time_zone,"-10 5") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 5;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 2;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 5;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 3;
	}
	else if(strcmp(time_zone,"-12 1") == 0)
	{
		//sprintf( str_datnight, "%s", "PDT,M3.2.0/03:00:00,M10.1.0/02:00:00");
		dst_start.dst_mon = 3;
		dst_start.dst_mweek = 2;
		dst_start.dst_wday = 0;
		dst_start.dst_hour = 3;
		dst_end.dst_mon = 10;
		dst_end.dst_mweek = 1;
		dst_end.dst_wday = 0;
		dst_end.dst_hour = 2;
	}
	else
	{
		dst_start.dst_mon = -1;
		dst_start.dst_mweek = -1;
		dst_start.dst_wday = -1;
		dst_start.dst_hour = -1;
		dst_end.dst_mon = -1;
		dst_end.dst_mweek = -1;
		dst_end.dst_wday = -1;
		dst_end.dst_hour = -1;
	}
	dst_start.dst_mon--;
	dst_end.dst_mon--;
//	diag_printf("%s:%d PDT, %d.%d.%d.%d-%d.%d.%d.%d\n",__FILE__,__LINE__,
//		dst_start.dst_mon+1,dst_start.dst_mweek, dst_start.dst_wday, dst_start.dst_hour,
//		dst_end.dst_mon+1, dst_end.dst_mweek, dst_end.dst_wday, dst_end.dst_hour);
}

void set_time_zone_offsets()
{
	time_t current_second;
	struct tm *tm_time;
	Cyg_libc_time_dst states;
	time_t stdOffset, dstOffset, stdOffsetOld, dstOffsetOld;
	int addVal=0;
	
	time(&current_second);

	states=cyg_libc_time_getzoneoffsets(&stdOffset,&dstOffset);
	tm_time = localtime(&current_second);
	stdOffsetOld = stdOffset;
	dstOffsetOld = dstOffset;
		
//	diag_printf("flag is %d, mon %d day %d hour is %d, addval is %d\n", dst_time_flag, tm_time->tm_mon, tm_time->tm_mday, tm_time->tm_hour, addVal);

	addVal=get_dst_offset(tm_time, 1);
//	diag_printf("addVal is %d\n", addVal);
	if(addVal == 3600)
	{
		if(dst_time_flag == 0)
			dst_time_flag = 1;
		else if(dst_time_flag == 2)
			addVal = 0;
	}
	else
	{
		if(dst_time_flag == 1)
			dst_time_flag = 2;
		else if(dst_time_flag == 2)
			dst_time_flag = 0;
	}
	dstOffset = stdOffset + addVal;

//	diag_printf("flag is %d, mon %d day %d hour is %d, addval is %d\n", dst_time_flag, tm_time->tm_mon, tm_time->tm_mday, tm_time->tm_hour, addVal);
	if(stdOffsetOld != stdOffset || dstOffsetOld != dstOffset)
		cyg_libc_time_setzoneoffsets(stdOffset, dstOffset);
}

void dst_time_loop()
{	
	while(1)
	{
		set_time_zone_offsets();
		sleep(10);
	}
}

void dst_time_loop_main(void)
{	
	/* Create the thread */
	cyg_thread_create(8,
		      dst_time_loop,
		      0,
		      "dst_time_loop",
		      &dst_time_loop_stack,
		      sizeof(dst_time_loop_stack),
		      &dst_time_loop_thread_handle,
		      &dst_time_loop_thread_obj);
		/* Let the thread run when the scheduler starts */
	cyg_thread_resume(dst_time_loop_thread_handle);
}

void start_dst_time_loop()
{
	int daylight_save;
	
	apmib_get(MIB_DAYLIGHT_SAVE,(void *)&daylight_save);
	if(daylight_save == 1)
		dst_time_loop_main();
}

#if 0
int get_dst_time_flag()
{
	return dst_time_flag;
}

void set_dst_time_flag(int flag)
{
	dst_time_flag = flag;
}
#endif

void _set_timezone()
{
	char tmpStr[32]={0}, time_zone[32]={0};
	char *ptr_std, *ptr_dst;
	int daylight_save;
	int stdOffset=0, dstOffset=0, addVal=0;
	struct tm tm_time;
	
	apmib_get(MIB_NTP_TIMEZONE, (void *)tmpStr);
	apmib_get(MIB_DAYLIGHT_SAVE,(void *)&daylight_save);
	
	//diag_printf("#### %s:%d timeZone(%s) daylight_save(%d)\n",__FILE__,__LINE__,tmpStr,daylight_save);

	if (tmpStr[0] == '\0') //sanity check
		strcpy(tmpStr, "-8 4");
	/**/
	memcpy(time_zone, tmpStr, 32);
	ptr_std=strtok(tmpStr," ");
	ptr_dst=strtok(NULL," ");

	if(strcmp(time_zone,"3 1")  == 0 ||
		strcmp(time_zone,"-3 4") == 0 ||
	 	strcmp(time_zone,"-4 3") == 0 ||
	 	strcmp(time_zone,"-5 3") == 0 ||
	 	strcmp(time_zone,"-9 4") == 0 ||
	 	strcmp(time_zone,"-9 5") == 0 )
	{
         stdOffset = -3600*atoi(ptr_std)+30*60; // GMT+9:30 add 30mins
	}
	else
		stdOffset = -3600*atoi(ptr_std);
	
	//diag_printf("%s %d ptr_std(%s) ptr_dst(%s)\n",__FUNCTION__,__LINE__,ptr_std,ptr_dst);
	if(daylight_save == 1)
	{
		cyg_libc_time_setdst(CYG_LIBC_TIME_DSTON);
		tm_time = get_time_from_flash();
		get_start_end_time_point(time_zone);
		addVal=get_dst_offset(&tm_time, 0);
		if(addVal == 3600)
			dst_time_flag = 1;
		else
			dst_time_flag = 0;
//		diag_printf("%s:%d offset is %d flag is %d\n", __FILE__,__LINE__, addVal, dst_time_flag);
		dstOffset = stdOffset + addVal;
	}
	else
	{
		cyg_libc_time_setdst(CYG_LIBC_TIME_DSTOFF);
		dstOffset = 0;
	}
	//diag_printf("%s:%d std=%s dst=%s\n",__FILE__,__LINE__,ptr_std,ptr_dst);
	//dstOffset=-3600*atoi(ptr_std)-3600*atoi(ptr_dst);
//	diag_printf("%s:%d stdOffset=%d dstOffset=%d\n",__FILE__,__LINE__,stdOffset,dstOffset);
	cyg_libc_time_setzoneoffsets(stdOffset, dstOffset);
}

#endif
