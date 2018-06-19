/*
 * Common kernel net device
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: kdev.c,v 1.3 2010-08-06 11:19:35 Exp $
 */
#include <pkgconf/io_fileio.h>
#include <pkgconf/io.h>
#include <sys/types.h>
#include <cyg/io/file.h>
#include <cyg/io/io.h>
#include <cyg/fileio/fileio.h>
#include <cyg/io/devtab.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <kdev.h>

extern void if_qflush __P((struct ifqueue *));

struct kdev_ctrl {
	kdev_node_t *dev;
	void *devpriv;
	int used;
	struct ifqueue inq;
	struct selinfo rcv_sel;
};

#undef malloc
#undef free
extern void *malloc(int);
extern void free(void *);

static bool kdev_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo kdev_lookup(struct cyg_devtab_entry **tab,
	struct cyg_devtab_entry *sub_tab, const char *name);
static Cyg_ErrNo kdev_write(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len);
static Cyg_ErrNo kdev_read(cyg_io_handle_t handle, void *buf, cyg_uint32 *len);
static Cyg_ErrNo kdev_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info);
static Cyg_ErrNo kdev_get_config(cyg_io_handle_t handle, cyg_uint32 key,
	void *buf, cyg_uint32 *len);
static Cyg_ErrNo kdev_set_config(cyg_io_handle_t handle, cyg_uint32 key,
	const void *buf, cyg_uint32 *len);
static Cyg_ErrNo kdev_ioctl(cyg_io_handle_t handle, CYG_ADDRWORD com, CYG_ADDRWORD data);
static Cyg_ErrNo kdev_close(cyg_io_handle_t handle);

static DEVIO_TABLE(
	kdev_devio,
	kdev_write,
	kdev_read,
	kdev_select,
	kdev_get_config,
	kdev_set_config);

DEVTAB_ENTRY(
	kdev,
	"/dev/net/",
	0,
	&kdev_devio,
	kdev_init,
	kdev_lookup,
	0);

/*
 * kdhcpd device init, only did once when system initializing.
 */
static bool
kdev_init(struct cyg_devtab_entry *tab)
{
	/* Initialize ioctl entry */
	tab->handlers->ioctl = kdev_ioctl;
	tab->handlers->close = kdev_close;
	return true;
}

static Cyg_ErrNo
kdev_lookup(struct cyg_devtab_entry **tab,
	struct cyg_devtab_entry *sub_tab,
	const char *name)
{
	cyg_devtab_entry_t *t, *p;
	kdev_node_t *kn;
	struct kdev_ctrl *ctrl;
	char buf[KDEV_NAMSIZ] = {0};
	char *pathname;
	char *devname;
	int len, tlen;
	int s;

	/* This is /dev/net/ global control */
	if (*name == 0)
		return ENOERR;

	/* Check if the device has been created */
	for (p = &kdev, t = p->next; t; p = t, t = t->next) {
		if (strcmp(name, t->name) == 0)
			break;
	}
	if (t == 0) {
		devname = strchr(name, '/');
		if (!devname)
			len = strlen(name);
		else
			len = devname - name;

		if (len > KDEV_NAMSIZ-1)
			return ENOENT;

		/* Copy node name part */
		memcpy(buf, name, len);

		tlen = sizeof(*t) + sizeof(*ctrl) + strlen(name);
		tlen = (tlen + 3) & ~3;

		for (kn = __kdev_tab__;
		     kn != &__kdev_tab_end__; kn++) {
			/* Check node name */
			if (strcmp(kn->node_name, buf) != 0)
				continue;

			t = (cyg_devtab_entry_t *)malloc(tlen);
			if (t == 0)
				return ENOMEM;

			ctrl = (struct kdev_ctrl *)(t+1);

			/* Save pathname after ctrl */
			pathname = (char *)(ctrl + 1);
			strcpy(pathname, name);
			if (devname)
				devname++;	/* Skip '/' */
			else
				devname = "";	/* null char string */

			/* Init ctrl */
			memset(ctrl, 0, sizeof(*ctrl));
			ctrl->dev = kn;

			/* Clone from general device, kdev */
			*t = kdev;
			t->name = pathname;
			t->priv = (void *)ctrl;
			t->next = 0;

			/* Append to kdhcpddev link */
			p->next = t;

			if (ctrl->dev->open) {
				s = splimp();

				/* Open device */
				ctrl->devpriv = (*ctrl->dev->open)(t, devname);
				if (ctrl->devpriv == NULL) {
					p->next = 0;
					free(t);
					splx(s);
					return ENOENT;
				}

				/* Create wait line */
				ctrl->inq.ifq_maxlen = IFQ_MAXLEN;
				cyg_selinit(&ctrl->rcv_sel);

				splx(s);

			}
			break;
		}
		if (kn == &__kdev_tab_end__)
			return ENOENT;
	}

	/* Update use count */
	ctrl = (struct kdev_ctrl *)t->priv;
	ctrl->used++;

	/* Return new hander */
	*tab = t;
	return ENOERR;
}

static Cyg_ErrNo
kdev_write(cyg_io_handle_t handle, const void *_buf, cyg_uint32 *len)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
	struct kdev_ctrl *ctrl = (struct kdev_ctrl *)t->priv;
	int s;
	int rc = 0;

	if (ctrl->dev->write) {
		s = splimp();
		rc = (*ctrl->dev->write)(ctrl->devpriv, (char *)_buf, *len);
		splx(s);
	}
	else {
		/* Can't write */
		*len = 0;
	}

	return rc;
}

static Cyg_ErrNo
kdev_read(cyg_io_handle_t handle, void *_buf, cyg_uint32 *len)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
	struct kdev_ctrl *ctrl = (struct kdev_ctrl *)t->priv;
	int s;
	struct mbuf *m;
	int cplen;

	/* Check readability */
	if (ctrl->inq.ifq_maxlen == 0) {
		*len = 0;
		return EINVAL;
	}

	/* Take one packet */
	s = splimp();
	IF_DEQUEUE(&ctrl->inq, m);
	splx(s);

	if (m == 0) {
		*len = 0;
		return EINVAL;
	}

	/* Copy to buffer */
	cplen = m->m_pkthdr.len;
	if (cplen > *len)
		cplen = *len;

	m_copydata(m, 0, cplen, _buf);

	/* Free this one and return */
	*len = cplen;
	m_freem(m);

	return 0;
}

static cyg_bool
kdev_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
	struct kdev_ctrl *ctrl = (struct kdev_ctrl *)t->priv;
	int rc = 1;
	int s = splimp();

	switch (which) {
	case FREAD:
		if (ctrl->inq.ifq_head) {
			/* Packets in line */
			break;
		}

		cyg_selrecord((CYG_ADDRWORD)info, &ctrl->rcv_sel);
		rc = 0;
		break;

	case FWRITE:
		/* Default for write is selected */
		break;

	case 0:
	default:
		rc = -EINVAL;
		break;
	}

	splx(s);
	return rc;
}

static Cyg_ErrNo
kdev_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf, cyg_uint32 *len)
{
	return -EINVAL;
}

static Cyg_ErrNo
kdev_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, cyg_uint32 *len)
{
	return -EINVAL;
}

static Cyg_ErrNo
kdev_ioctl(cyg_io_handle_t handle, CYG_ADDRWORD cmd, CYG_ADDRWORD data)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
	struct kdev_ctrl *ctrl = (struct kdev_ctrl *)t->priv;
	int s;
	int rc = -EINVAL;

	if (ctrl->dev->ioctl) {
		s = splimp();
		rc = (*ctrl->dev->ioctl)(ctrl->devpriv, cmd, (char *)data);
		splx(s);
	}
	return rc;
}

static Cyg_ErrNo
kdev_close(cyg_io_handle_t handle)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
	cyg_devtab_entry_t *p;
	int s;
	struct kdev_ctrl *ctrl;

	/* This is /dev/net/ global control */
	if (t == &kdev)
		return ENOERR;

	for (p = &kdev, t = p->next; t; p = t, t = t->next) {
		if (t == (cyg_devtab_entry_t *)handle) {
			/* Free this entry if no one refers to it */
			ctrl = (struct kdev_ctrl *)t->priv;
			if (--ctrl->used <= 0) {
				/* Dequeue and free */
				p->next = t->next;

				/* close if any */
				if (ctrl->dev->close) {
					s = splimp();
					(*ctrl->dev->close)(ctrl->devpriv);
					splx(s);
				}

				/* Free if queue if any */
				if (ctrl->inq.ifq_maxlen)
					if_qflush(&ctrl->inq);

				/* Free device */
				free(t);
			}
			break;
		}
	}

	return ENOERR;
}

/* keve nodes */
CYG_HAL_TABLE_BEGIN(__kdev_tab__, kdevtab);
CYG_HAL_TABLE_END(__kdev_tab_end__, kdevtab);

/*
 * Common input function sending up by devices
 */
int
kdev_input(void *devtab, struct mbuf *m)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)devtab;
	struct kdev_ctrl *ctrl;
	int s;

	/* device check */
	if (t == NULL) {
		m_freem(m);
		return 0;
	}

	/* Check queue depth */
	ctrl = (struct kdev_ctrl *)t->priv;
	if (ctrl->inq.ifq_maxlen == 0) {
		m_freem(m);
		return 0;
	}

	/* Quene and wake up device */
	s = splimp();
	if (IF_QFULL(&ctrl->inq)) {
		IF_DROP(&ctrl->inq);
		m_freem(m);
	}
	else {
		IF_ENQUEUE(&ctrl->inq, m);
	}
	splx(s);

	/* Wakeup handler */
	selwakeup(&ctrl->rcv_sel);
	return 0;
}
