// by cairui
#if defined(ECOS)
#include <cyg/kernel/kapi.h>
#include <stdlib.h>
#include <sys/time.h>
// USE_RDTSCLL_TIME and RDTSC are both undefined
//#include <unistd.h>
//#include <fcntl.h>
//#include <errno.h>
//#include <sys/times.h>
//#include <time.h>

#include "../osLayer.h"

#if defined(USE_RDTSCLL_TIME) || defined(RDTSC)
#include <asm/timex.h>
/*
	As defined in asm/timex.h for x386:
*/
#ifndef rdtscll
	#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#endif

static sslTime_t	hiresStart; 	/* zero-time */
static sslTime_t	hiresFreq; 		/* tics per second */
#else /* USE_RDTSCLL_TIME */
static uint32		prevTicks; 		/* Check wrap */
static sslTime_t	elapsedTime; 	/* Last elapsed time */
#endif

#ifdef USE_MULTITHREADING
#include <pthread.h>
static pthread_mutexattr_t	attr;
#endif

/* max sure we don't retry reads forever */
#define	MAX_RAND_READS		1024

static 	int32	urandfd = -1;
static 	int32	randfd	= -1;

int32 sslOpenOsdep(void)
{
#if 0
/*
	Open /dev/random access non-blocking.
*/
	if ((randfd = open("/dev/random", O_RDONLY | O_NONBLOCK)) < 0) {
		return -1;
	}
	if ((urandfd = open("/dev/urandom", O_RDONLY)) < 0) {
		close(randfd);
		return -1;
	}
#ifdef USE_MULTITHREADING
	pthread_mutexattr_init(&attr);
#ifndef OSX
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif /* !OSX */
#endif /* USE_MULTITHREADING */

	return psOpenMalloc(MAX_MEMORY_USAGE);
#else
	return 0;
#endif
}

int32 sslCloseOsdep(void)
{
#if 0
	psCloseMalloc();
#ifdef USE_MULTITHREADING
	pthread_mutexattr_destroy(&attr);
#endif
	close(randfd);
	close(urandfd);
	return 0;
#else
	return 0;
#endif
}

/*
	Read from /dev/random non-blocking first, then from urandom if it would 
	block.  Also, handle file closure case and re-open.
*/

int32 sslGetEntropy(unsigned char *bytes, int32 size)
{
	printf("enter sslGetEntropy\n");
#if 0
	int32				rc, sanity, retry, readBytes;
	unsigned char 	*where = bytes;

	sanity = retry = rc = readBytes = 0;

	while (size) {
		if ((rc = read(randfd, where, size)) < 0 || sanity > MAX_RAND_READS) {
			if (errno == EINTR) {
				if (sanity > MAX_RAND_READS) {
					return -1;
				}
				sanity++;
				continue;
			} else if (errno == EAGAIN) {
				break;
			} else if (errno == EBADF && retry == 0) {
				close(randfd);
				if ((randfd = open("/dev/random", O_RDONLY | O_NONBLOCK)) < 0) {
					break;
				}
				retry++;
				continue;
			} else {
				break;
			}
		}
		readBytes += rc;
		where += rc;
		size -= rc;
	}


	sanity = retry = 0;	
	while (size) {
		if ((rc = read(urandfd, where, size)) < 0 || sanity > MAX_RAND_READS) {
			if (errno == EINTR) {
				if (sanity > MAX_RAND_READS) {
					return -1;
				}
				sanity++;
				continue;
			} else if (errno == EBADF && retry == 0) {
				close(urandfd);
				if ((urandfd = 
					open("/dev/urandom", O_RDONLY | O_NONBLOCK)) < 0) {
					return -1;
				}
				retry++;
				continue;
			} else {
				return -1;
			}
		}
		readBytes += rc;
		where += rc;
		size -= rc;
	}
	return readBytes;
#else
	int ret = rand()%100;
	printf("out sslGetEntropy, ret=%d\n", ret);
		
	return (ret);
#endif
}

/******************************************************************************/


/*****************************************************************************/
/*
	Use a platform specific high resolution timer
*/

int32 sslInitMsecs(sslTime_t *timePtr)
{
	printf("enter sslInitMsecs\n");
	//struct tms		tbuff;
	cyg_tick_count_t t;
	uint32 deltat, deltaticks;
                                                                                
/*
 *	times() returns the number of clock ticks since the system
 *	was booted.  If it is less than the last time we did this, the
 *	clock has wrapped around 0xFFFFFFFF, so compute the delta, otherwise
 *	the delta is just the difference between the new ticks and the last
 *	ticks.  Convert the elapsed ticks to elapsed msecs using rounding.
 */
	//if ((t = times(&tbuff)) >= prevTicks) {
	if ((t = cyg_current_time()) >= prevTicks) {
		deltaticks = t - prevTicks;
	} else {
		deltaticks = (0xFFFFFFFF - prevTicks) + 1 + t;
	}
	deltat = ((deltaticks * 1000) + (CLK_TCK / 2)) / CLK_TCK;
                                                                     
/*
 *	Add the delta to the previous elapsed time.
 */
	elapsedTime.usec += ((deltat % 1000) * 1000);
	if (elapsedTime.usec >= 1000000) {
		elapsedTime.usec -= 1000000;
		deltat += 1000;
	}
	elapsedTime.sec += (deltat / 1000);
	prevTicks = t;
                                                                                
/*
 *	Return the current elapsed time.
 */
	timePtr->usec = elapsedTime.usec;
	timePtr->sec = elapsedTime.sec;
	printf("out sslInitMsecs\n");
	return (timePtr->usec / 1000) + timePtr->sec * 1000;
}

/*
	Return the delta in seconds between two time values
*/
long sslDiffMsecs(sslTime_t then, sslTime_t now)
{
	return (long)((now.sec - then.sec) * 1000);
}

/*
	Return the delta in seconds between two time values
*/
int32 sslDiffSecs(sslTime_t then, sslTime_t now)
{
	return (int32)(now.sec - then.sec);
}

/*
	Time comparison.  1 if 'a' is less than or equal.  0 if 'a' is greater
*/
int32	sslCompareTime(sslTime_t a, sslTime_t b)
{
	if (a.sec < b.sec) {
		return 1;
	} else if (a.sec == b.sec) {
		if (a.usec <= b.usec) {
			return 1;
		} else {
			return 0;
		}
	}	
	return 0;
}

#endif /* ECOS */
