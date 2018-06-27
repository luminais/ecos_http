#include <sys/time.h>
#include <sys/types.h>
#include "uloop.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include "common.h"

#include "utils.h"

static struct list_head timeouts = LIST_HEAD_INIT(timeouts);
#define ULOOP_IDLE_TIME       200
bool uloop_cancelled = false;
static int uloop_status = 0;

static int uloop_run_depth = 0;

extern struct elink_ctx g_elink_ctx;

int read_from_cloud()
{
	char head[ELINKCC_HEADER_LEN + 2] = {0};
   	int ret = 0;
	int message_len = 0;
	char *message = NULL;

	ret = socket_receive_full(g_elink_ctx.romote_fd, head, ELINKCC_HEADER_LEN);    
	if(ret < 0)
	{
		return -1;
	}

	message_len = _check_msg_header(head, ELINKCC_HEADER_LEN);

	if(!message_len)
	{
	    return -1;
	}  

	message = (char*)malloc(message_len);
	g_elink_ctx.msg = (char*)malloc(message_len - 1);
	
	ret = socket_receive_full(g_elink_ctx.romote_fd, message, message_len);    
	if(ret < 0)
	{
		free(g_elink_ctx.msg);
		free(message);
		g_elink_ctx.msg = NULL;
		return -1;
	}

	memset(g_elink_ctx.msg,0x0,message_len - 1);
	memcpy(g_elink_ctx.msg,message,message_len - ELINKCC_TAIL_LEN);
   
	return message_len;
}

extern int _process_remote_msg(char *data, unsigned int len);
static void uloop_run_events(int timeout)
{
	struct timeval tv;
	struct timeval *wait_tv = NULL;
	fd_set fds;
	int maxfd;
	int n = 0;
	int date_len = 0;
	if(timeout == -1)
	{
		wait_tv = NULL;
	}else
	{
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000) * 1000;
		wait_tv = &tv;
	}

	if(g_elink_ctx.romote_fd > 0)
	{
		FD_ZERO(&fds);
		FD_SET(g_elink_ctx.romote_fd, &fds);
		maxfd = g_elink_ctx.romote_fd;

		n = select(maxfd+1, &fds, NULL, NULL, wait_tv);
		if (n <= 0)
			return n;

			/* Check dhcp packet */
		if (FD_ISSET(g_elink_ctx.romote_fd, &fds)) {
			date_len = read_from_cloud();
			if(date_len > 0)
			{
				_process_remote_msg(g_elink_ctx.msg,date_len);
			}

		}
	}else
	{
		cyg_thread_delay(ULOOP_IDLE_TIME);
	}
		


	return;
}

static int tv_diff(struct timeval *t1, struct timeval *t2)
{
	return
		(t1->tv_sec - t2->tv_sec) * 1000 +
		(t1->tv_usec - t2->tv_usec) / 1000;
}

int uloop_timeout_add(struct uloop_timeout *timeout)
{
	struct uloop_timeout *tmp = NULL;
	struct list_head *h = &timeouts;

	if (timeout->pending)
		return -1;

	list_for_each_entry(tmp, &timeouts, list) {
		if (tv_diff(&tmp->time, &timeout->time) > 0) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&timeout->list, h);
	timeout->pending = true;

	return 0;
}

static void uloop_gettime(struct timeval *tv)
{
	struct timespec ts;

	clock_gettime(0, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}

int uloop_timeout_set(struct uloop_timeout *timeout, int msecs)
{
	struct timeval *time = &timeout->time;

	if (timeout->pending)
		uloop_timeout_cancel(timeout);

	uloop_gettime(time);

	time->tv_sec += msecs / 1000;
	time->tv_usec += (msecs % 1000) * 1000;

	if (time->tv_usec > 1000000) {
		time->tv_sec++;
		time->tv_usec -= 1000000;
	}

	return uloop_timeout_add(timeout);
}

int uloop_timeout_cancel(struct uloop_timeout *timeout)
{
	if (!timeout->pending)
		return -1;

	list_del(&timeout->list);
	timeout->pending = false;

	return 0;
}

int uloop_timeout_remaining(struct uloop_timeout *timeout)
{
	struct timeval now;

	if (!timeout->pending)
		return -1;

	uloop_gettime(&now);

	return tv_diff(&timeout->time, &now);
}

static int uloop_get_next_timeout(struct timeval *tv)
{
	struct uloop_timeout *timeout;
	int diff;

	if (list_empty(&timeouts))
		return -1;

	timeout = list_first_entry(&timeouts, struct uloop_timeout, list);
	diff = tv_diff(&timeout->time, tv);
	if (diff < 0)
		return 0;

	return diff;
}

static void uloop_process_timeouts(struct timeval *tv)
{
	struct uloop_timeout *t;

	while (!list_empty(&timeouts)) {
		t = list_first_entry(&timeouts, struct uloop_timeout, list);

		if (tv_diff(&t->time, tv) > 0)
			break;

		uloop_timeout_cancel(t);
		if (t->cb)
			t->cb(t);
	}
}

static void uloop_clear_timeouts(void)
{
	struct uloop_timeout *t, *tmp;

	list_for_each_entry_safe(t, tmp, &timeouts, list)
		uloop_timeout_cancel(t);
}

bool uloop_cancelling(void)
{
	return uloop_run_depth > 0 && uloop_cancelled;
}

int uloop_run_timeout(int timeout)
{
	int next_time = 0;
	struct timeval tv;

	uloop_run_depth++;

	uloop_status = 0;
	uloop_cancelled = false;
	while (!uloop_cancelled)
	{
		uloop_gettime(&tv);
		uloop_process_timeouts(&tv);

		if (uloop_cancelled)
			break;

		uloop_gettime(&tv);

		next_time = uloop_get_next_timeout(&tv);
		if (timeout >= 0 && timeout < next_time)
			next_time = timeout;
		uloop_run_events(next_time);
	}

	--uloop_run_depth;

	return uloop_status;
}

void uloop_done(void)
{
	uloop_clear_timeouts();
}
