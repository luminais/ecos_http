/*
 * Extended C lib functions not supported by eCos package
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: libc2.c,v 1.4 2010-06-19 12:42:59 Exp $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef BCM
int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
	cyg_uint64 temp;
	extern cyg_uint64 board_current_usec(void);

	temp = board_current_usec();
	tv->tv_sec = time(0);
	tv->tv_usec = temp % 1000000;
	return (0);
}

char *
strdup(const char *src)
{
	int len;
	char *dst;

	len = strlen(src) + 1;
	if ((dst = (char *)malloc(len)) == NULL)
		return (NULL);

	strcpy(dst, src);
	return (dst);
}

char *
strsep(char **stringp, const char *delim)
{
	char *begin, *end;

	begin = *stringp;
	if (begin == NULL)
		return NULL;

	/* A frequent case is when the delimiter string contains only one
	 * character.  Here we don't need to call the expensive `strpbrk'
	 * function and instead work using `strchr'.
	 */
	if (delim[0] == '\0' || delim[1] == '\0') {
		char ch = delim[0];
		if (ch == '\0')
			end = NULL;
		else {
			if (*begin == ch)
				end = begin;
			else if (*begin == '\0')
				end = NULL;
			else
				end = strchr(begin + 1, ch);
		}
	}
	else
		/* Find the end of the token.  */
		end = strpbrk(begin, delim);

	if (end) {
		/* Terminate the token and set *STRINGP past NUL character.  */
		*end++ = '\0';
		*stringp = end;
	}
	else
		/* No more delimiters; this is the last token.  */
		*stringp = NULL;

	return begin;
}
#endif /*#ifdef BCM*/

#ifdef REALTEK

#endif

//roy+++,steal from ralink ecos sdk
//------------------------------------------------------------------------------
// FUNCTION
//		int str2arglist(char *buf, int *list, char c, int max)
//
// DESCRIPTION
//		convert string to argument array, ',' character was regarded as delimiter
//	and '\n' character regarded as end of string. 
//  
// PARAMETERS
//		buf: input string
//		list: output array pointer
//		c: delimiter
//		max: maximum number of sub-string
//  
// RETURN
//		number of arguments
//  
//------------------------------------------------------------------------------
int str2arglist(char *buf, int *list, char c, int max)
{
	char *idx = buf;
	int j=0;
	
	list[j++] = buf;
	while(*idx && j<max) {
		if(*idx == c || *idx == '\n') {
			*idx = 0;
			list[j++] = idx+1;
		}
		idx++;	
	}
	if(j==1 && !(*buf)) // No args
		j = 0;
		
	return j;
}

//+++
#define CYGNUM_KERNEL_THREADS_DATA_ENV 1
#ifdef CYGNUM_KERNEL_THREADS_DATA_ENV
char *
getenv(const char *name)
{
	char **env = (char **)cyg_thread_get_data(CYGNUM_KERNEL_THREADS_DATA_ENV);
	char *ptr;
	char *value = NULL;
	int len = 0;

	if (name == NULL || env == NULL)
		return NULL;
	len = strlen(name);
	for (; *env; env++) {
		ptr = *env;
		if (memcmp(ptr, name, len) == 0 && ptr[len] == '=') {
			value = &ptr[len+1];
			break;
		}
	}

	return value;
}
#endif /* CYGNUM_KERNEL_THREADS_DATA_ENV */
