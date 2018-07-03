/*
 * ulog - simple logging functions
 *
 * Copyright (C) 2015 Jo-Philipp Wich <jow@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#define _GNU_SOURCE

#include "ulog.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


static int _ulog_channels = -1;
static int _ulog_facility = -1;
static int _ulog_threshold = LOG_DEBUG;
static int _ulog_initialized = 0;
static const char *_ulog_ident = NULL;
FILE *log_fp = NULL;
int log_file_size = 0;

#define ELINKCC_LOG_FILE "/var/elinkcc.log"
#define MAX_LOG_FILE_SIZE 16384

static const char *ulog_default_ident(void)
{
	FILE *self;
	static char line[64];
	char *p = NULL;
	char *sbuf;

	if ((self = fopen("/proc/self/status", "r")) != NULL) {
		while (fgets(line, sizeof(line), self)) {
			if (!strncmp(line, "Name:", 5)) {
				strtok_r(line, "\t\n", &sbuf);
				p = strtok_r(NULL, "\t\n", &sbuf);
				break;
			}
		}

		fclose(self);
	}

	return p;
}

static void ulog_defaults(void)
{
	if (_ulog_initialized)
		return;

	if (_ulog_channels < 0) {
		_ulog_channels = ULOG_STDIO;
	}

	if (_ulog_facility < 0) {
		_ulog_facility = LOG_USER;
	}

	if (_ulog_ident == NULL && _ulog_channels != ULOG_STDIO)
		_ulog_ident = ulog_default_ident();

	if (_ulog_channels & ULOG_SYSLOG)
		openlog(_ulog_ident, 0, _ulog_facility);

	_ulog_initialized = 1;
}


static void _rotate_file(void)
{
	fclose(log_fp);

	unlink(ELINKCC_LOG_FILE".old");
	rename(ELINKCC_LOG_FILE, ELINKCC_LOG_FILE".old");

	log_fp = fopen(ELINKCC_LOG_FILE, "a+");
}

static void ulog_file(FILE *fp, int priority, const char *fmt, va_list ap)
{
	struct timespec ts;
	char *tmstr = NULL;
	struct tm local_tm;
	int len = 0, ret = 0;
	const char *levelstr[] = {"", "", "", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"};

	clock_gettime(CLOCK_REALTIME, &ts);

	localtime_r(&ts.tv_sec, &local_tm);
	len = sprintf(&tmstr, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
				   local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
				   local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, ts.tv_nsec / 1000000);

	if (len > 0) {
		ret = fprintf(fp, "%s (%s): %s: ", tmstr, levelstr[priority], _ulog_ident);
	} else {
		ret = fprintf(fp, "(%s): %s: ", levelstr[priority], _ulog_ident);
	}

	ret += vfprintf(fp, fmt, ap);

	if (log_fp && (fp == log_fp)) {
		log_file_size += ret;

		if (log_file_size > MAX_LOG_FILE_SIZE) {
			_rotate_file();
			log_file_size = 0;
		}
		fflush(log_fp);
	}	

	free(tmstr);
}

static void ulog_syslog(int priority, const char *fmt, va_list ap)
{
	vsyslog(priority, fmt, ap);
}

void ulog_open(int channels, int facility, const char *ident)
{
	ulog_close();

	_ulog_channels = channels;
	_ulog_facility = facility;
	_ulog_ident = ident;

	if (channels & ULOG_FILE) {
		struct stat statbuf;
		int fd = 0;
	
		log_fp = fopen(ELINKCC_LOG_FILE, "a+");

		if (log_fp) {
			fd = fileno(log_fp);
			if (fd > 0) {
				fstat(fd, &statbuf);
				log_file_size = statbuf.st_size;
			}
		}
	}
}

void ulog_close(void)
{
	if (!_ulog_initialized)
		return;

	if (_ulog_channels & ULOG_SYSLOG)
		closelog();

	if (_ulog_channels & ULOG_FILE && log_fp) {
		fclose(log_fp);
		log_fp = NULL;
	}

	_ulog_initialized = 0;
}

void ulog_threshold(int threshold)
{
	_ulog_threshold = threshold;
}

void ulog(int priority, const char *fmt, ...)
{
	va_list ap;

	if (priority > _ulog_threshold)
		return;

	ulog_defaults();

	va_start(ap, fmt);
	if (_ulog_channels & ULOG_STDIO)
		ulog_file(stdout, priority, fmt, ap);

	if (_ulog_channels & ULOG_FILE)
		ulog_file(log_fp ? log_fp : stdout, priority, fmt, ap);

	if (_ulog_channels & ULOG_SYSLOG)		
		ulog_syslog(priority, fmt, ap);
	
	va_end(ap);
}
