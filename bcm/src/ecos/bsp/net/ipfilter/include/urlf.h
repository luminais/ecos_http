/*
 * URL filter include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: urlf.h,v 1.2 2010-07-30 06:38:09 Exp $
 *
 */
#ifndef	__URL_H__
#define __URL_H__

//roy+++,2010/09/20
enum {
	URLF_MODE_DISABLED = 0,
	URLF_MODE_DENY,
	URLF_MODE_PASS
};

#define URLF_PASS	0
#define URLF_BLOCK	1
//+++

#define NOT_CHECK_HTTPS	0
#define CHECK_HTTPS		1


struct urlfilter {
	unsigned int sip;
	unsigned int eip;
	char url[256];
	struct urlfilter *next;
};

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
struct parentCtl_devices_list{
	in_addr_t 	ip;
	unsigned char  mac[6];
	int mac_filter_status;
	struct parentCtl_devices_list *next;
};

struct parentCtl_url_list{
	char url[256];
	struct parentCtl_url_list *next;
};

struct parentCtl_config{
	struct parentCtl_devices_list  *devlist;
	time_t stime;
	time_t etime;
	unsigned char wday[8];
	int mode;
	struct parentCtl_url_list  *urllist;
	int url_filter_status;
};

extern struct parentCtl_config gParentCtlConfig;

struct backlist_device_list{
	struct backlist_device_list  *next;
	unsigned char  mac[6];
	unsigned char  flag;
};

extern struct backlist_device_list *backlist_device_list_head;

#endif
/* Functions */
int  urlfilter_add(struct urlfilter *entry);
void urlfilter_act(void);
void urlfilter_inact(void);
void urlfilter_flush(void);
//roy+++,2010/09/20
int urlfilter_del(struct urlfilter *entry);
int urlfilter_set_mode(int mode);
//+++

#endif	/* __URL_H__ */
