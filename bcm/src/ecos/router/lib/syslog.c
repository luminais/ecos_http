/*
 * syslog
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: syslog.c,v 1.6 2010-07-22 03:08:10 Exp $
 */
#include <sys/param.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include <ecos_oslib.h>

#ifndef __CONFIG_SYSLOG_LOG_SIZE__
#define __CONFIG_SYSLOG_LOG_SIZE__ 128
#endif
#ifndef __CONFIG_SYSLOG_LOG_NUM__
#define __CONFIG_SYSLOG_LOG_NUM__ 512
#endif

typedef struct syslog_log
{
	int  time;	/* log time */
	int  len;   	/* Length of log message */
	pid_t pid;
	uint8_t data[__CONFIG_SYSLOG_LOG_SIZE__];	/* data pointer */
} log_t;

#define LOG_HEADER_SIZE	12

struct syslog_desc
{
	int  start;
	int  count;
	int  read;	/* offset of read pointer */
};

static cyg_mutex_t syslog_mutex;
static int  syslog_initialized = 0;

static struct syslog_desc log_desc;
static  struct syslog_desc *descp = &log_desc;

static log_t syslog_buf[__CONFIG_SYSLOG_LOG_NUM__];

extern void set_gLog_cur_page(int cur);

void syslog_mutex_lock(void)
{
	cyg_mutex_lock(&syslog_mutex);
}

void syslog_mutex_unlock(void)
{
	cyg_mutex_unlock(&syslog_mutex);
}

void syslog(int level, const char *fmtStr, ...)
{
	va_list args;
	log_t *log_msg;
	int buf_len;
	uint8_t buf[__CONFIG_SYSLOG_LOG_SIZE__];

	/* Print with DEBUG flag */
	if (LOG_PRI(level) == LOG_DEBUG) {
		/* Should print ctime here */
		va_start(args, fmtStr);
		diag_vprintf(fmtStr, args);
		va_end(args);

		/* New line */
		printf("\n");
	}

	if (!syslog_initialized)
		return;

	syslog_mutex_lock();

	if (descp->count < __CONFIG_SYSLOG_LOG_NUM__) {
		log_msg = &syslog_buf[descp->count];
		descp->count++;
	}
	else if (descp->count == __CONFIG_SYSLOG_LOG_NUM__) {
#if 0//roy modify,if count arrive at 150,then start count at begining		
		log_msg = &syslog_buf[descp->start];
		descp->start++;
		if (descp->start == __CONFIG_SYSLOG_LOG_NUM__)
			descp->start = 0;
#else
		log_msg = &syslog_buf[descp->start];
		descp->count = 1;
		set_gLog_cur_page(1);
#endif
	}
	else {
		/* Should not get here */
		syslog_mutex_unlock();
		return;
	}

	/* Get thread's name */
	log_msg->pid = oslib_pid();
	log_msg->time = time((time_t*)NULL);

	va_start(args, fmtStr);
	buf_len = vsnprintf(buf, __CONFIG_SYSLOG_LOG_SIZE__-1, fmtStr, args);
	va_end(args);

	buf[__CONFIG_SYSLOG_LOG_SIZE__-1] = 0;
	log_msg->len = sprintf(log_msg->data, "%s", buf);

	syslog_mutex_unlock();
}

int syslog_open()
{
	if (!syslog_initialized)
		return 0;
	syslog_mutex_lock();
	descp->read = descp->start;
	return 0;
}

int syslog_close()
{
	if (!syslog_initialized)
		return 0;
	syslog_mutex_unlock();
	return 0;
}

char *syslog_get()
{
	static uint8_t print_buf[__CONFIG_SYSLOG_LOG_SIZE__ + 128];
	log_t *log_msg;
	int len;
	char name[128];

	if (!syslog_initialized)
		return NULL;
#if 1
	if(descp->read < __CONFIG_SYSLOG_LOG_NUM__ && descp->read < descp->count){
		log_msg = &syslog_buf[descp->read++];
	}else{
		descp->read = 0;
		return NULL;
	}
#else	
	if (descp->count < __CONFIG_SYSLOG_LOG_NUM__ &&
		descp->read < __CONFIG_SYSLOG_LOG_NUM__ &&
		descp->read < descp->count) {
		log_msg = &syslog_buf[descp->read++];
	}
	else if (descp->count == __CONFIG_SYSLOG_LOG_NUM__) {
		if (descp->start == 0) {
			if (descp->read < __CONFIG_SYSLOG_LOG_NUM__)
				log_msg = &syslog_buf[descp->read++];
			else
				return NULL;
		}
		else {
			if (descp->read == __CONFIG_SYSLOG_LOG_NUM__) {
				descp->read = 0;
				log_msg = &syslog_buf[descp->read++];
			}
			else if (descp->read < __CONFIG_SYSLOG_LOG_NUM__)
				log_msg = &syslog_buf[descp->read++];
			else
				return NULL;
		}

	}
	else
		return NULL;
#endif
	sprintf(print_buf, "%s", ctime(&log_msg->time));
	len = strlen(print_buf);
	/* Name of thread */
	name[0] = 0;
	oslib_getnamebypid(log_msg->pid, name);
	sprintf(print_buf+len-1, ":%s", name);
	len = strlen(print_buf);
	/* Print log message */
	sprintf(print_buf+len-1, ":%s", log_msg->data);
	return print_buf;
}

int syslog_get2(char *log_time,char *log_type,char *log_data)
{
	log_t *log_msg;
	int len;
	char name[128];

	char time_info[40];
	struct tm TM;
	
	if (!syslog_initialized)
		return 0;
#if 1
	if(descp->read < __CONFIG_SYSLOG_LOG_NUM__ && descp->read < descp->count){
		log_msg = &syslog_buf[descp->read++];
	}else{
		descp->read = 0;
		return 0;
	}
#else	
	if /*(1)*/(descp->count < __CONFIG_SYSLOG_LOG_NUM__ &&
		descp->read < __CONFIG_SYSLOG_LOG_NUM__ &&
		descp->read < descp->count) {
		log_msg = &syslog_buf[descp->read++];
	}
	else if (descp->count == __CONFIG_SYSLOG_LOG_NUM__) {
		if (descp->start == 0) {
			if (descp->read < __CONFIG_SYSLOG_LOG_NUM__)
				log_msg = &syslog_buf[descp->read++];
			else
				return 0;
		}
		else {//descp->start != 0
			if (descp->read == __CONFIG_SYSLOG_LOG_NUM__) {
				descp->read = 0;
				log_msg = &syslog_buf[descp->read++];
			}
			else if (descp->read < __CONFIG_SYSLOG_LOG_NUM__)
				log_msg = &syslog_buf[descp->read++];
			else
				return 0;
		}

	}
	else{
		if(descp->read == descp->count)
			descp->read = 0;//(1) out
		return 0;
	}
#endif		
		
	gmtime_r(&log_msg->time,&TM);

	if(TM.tm_year == 70)//tenda modify 系统初始时间显示为2011-04-01
	{
		sprintf(time_info,"%04d-%02d-%02d %02d:%02d:%02d",
				TM.tm_year + 1900 + 41,TM.tm_mon + 1 + 3,TM.tm_mday,
					TM.tm_hour,TM.tm_min,TM.tm_sec);
	
	}else
		{
			sprintf(time_info,"%04d-%02d-%02d %02d:%02d:%02d",
				TM.tm_year + 1900,TM.tm_mon + 1,TM.tm_mday,
					TM.tm_hour,TM.tm_min,TM.tm_sec);
		}
	snprintf(log_time, 32,"%s", time_info);

	if(oslib_getnamebypid(log_msg->pid, name)!=1)
		snprintf(log_type, 32,"%s", name);
	else
		snprintf(log_type, 32,"%s", "system");
	
	snprintf(log_data, __CONFIG_SYSLOG_LOG_SIZE__ - 1,"%s", log_msg->data);
	log_data[__CONFIG_SYSLOG_LOG_SIZE__ - 1] = 0;
	return 1;
}

int
syslog_init()
{
	//memset(descp, 0, sizeof(*descp));
	/* init mutext */
	cyg_mutex_init(&syslog_mutex);
	syslog_initialized = 1;
	descp->start = 0;
	descp->count = 0;
	descp->read = 0;
	return 0;
}
//roy+++,2010/09/20
int
syslog_clear()
{
	syslog_open();
	//memset(descp, 0, sizeof(*descp));
	descp->start = 0;
	descp->count = 0;
	descp->read = 0;
	syslog_close();
	return 0;
}
//+++

