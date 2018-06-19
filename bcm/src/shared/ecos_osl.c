/*
 *  Copyright 2006, Broadcom Corporation
 *   All Rights Reserved.

 *  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *  the contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of Broadcom Corporation.
 */
/*
 * eCos OS Independent Layer
 *
 * Copyright(c) 2006 Broadcom Corp.
 * $Id: ecos_osl.c,v 1.9.20.1 2010-12-15 03:15:52 Exp $
 */

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/pci/bcmpci.h>
#include <cyg/hal/plf_cache.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <osl.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <bcmallocache.h>

#define PCI_CFG_RETRY 10

#define BUF_SIZE    (1024*4)

struct osl_info {
	void 		*devinfo;
	bcmcache_t	*pkttag_pool;	/* Working set of allocated memory pool */
	uint32		alloced;		/* Total allocated pkttags */
	void 		*tx_ctx;
	pktfree_cb_fn_t tx_fn;
};

/* Global ASSERT type flag */
uint32 g_assert_type = 0;

void*
osl_attach(void *pdev, uint bustype, bool pkttag)
{
	osl_t *osh;

	osh = (osl_t *)malloc(sizeof(osl_t));

	if (osh) {
		bzero(osh, sizeof(osl_t));
		osh->devinfo = pdev;
		if (pkttag) {
			osh->pkttag_pool = bcmcache_create(osh, "pkttag", OSL_PKTTAG_SZ);
		}
	}
	return (void*)osh;
}

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

	/* Destroy working set cache if exists */
	if (osh->pkttag_pool) {
#ifdef BCMDBG
		if (osh->alloced != 0)
			printf("Pkttag leak of %d pkttags. Packets leaked\n", osh->alloced);
#endif /* BCMDBG */
		ASSERT(osh->alloced == 0);
		bcmcache_destroy(osh->pkttag_pool);
	}

	free(osh);
}

void
osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx)
{
	if (osh == NULL)
		return;

	osh->tx_fn = tx_fn;
	osh->tx_ctx = tx_ctx;
}

void*
osl_pktget(osl_t *osh, uint len, bool send)
{
	struct mbuf *m;
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m) {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
			m_free(m);
			m = 0;
			goto done;
		} else {
			if (len > MCLBYTES) {
				diag_printf("osl_pktget: MCLBYTES=%d, len=%d\n", MCLBYTES, len);
			}
			m->m_data += PKTRSV;
			m->m_len = len;
		}
		/* Set pkttag if specified */
		if (osh && osh->pkttag_pool) {
			void *pkttag = osl_pkttag_alloc(osh);
			if (pkttag == NULL) {
				/* SAM */
//				osl_pktfree(osh, m, send);
				osl_pktfree(osh, m, false);
				return NULL;
			}
			osl_pktsettag(m, pkttag);
		} else
			osl_pktsettag(m, NULL);
	}
done:
	return ((void*)m);
}

void
osl_pktfree(osl_t *osh, void *m, bool send)
{
	void *n;
	void *pkttag;

	if (osh && (pkttag = osl_pkttag(m))) {
	/*SAM*/
    		if (send && osh->tx_fn)
                	osh->tx_fn(osh->tx_ctx, m, 0);

		ASSERT(osh->pkttag_pool);
		osl_pkttag_free(osh, pkttag);
		n = osl_pktnext(osh, m);
		while (n) {
			if ((pkttag = osl_pkttag(n)))
				osl_pkttag_free(osh, pkttag);
			n = osl_pktnext(osh, n);
		}
	}
	m_freem((struct mbuf *)m);
}

void*
osl_pktdup(osl_t *osh, void *m)
{
	void *pkttag = NULL;
	void *dup, *next;

	if (osh && osh->pkttag_pool)
		if ((pkttag = osl_pkttag_alloc(osh)) == NULL)
			return NULL;

	dup = (void*) m_copym2((struct mbuf *)m, 0, M_COPYALL, M_DONTWAIT);
	if (dup) {
		/* set the packettag for first mbuf */
		osl_pktsettag(dup, pkttag);

		/* clear pkttag pointer in m_next chain, avoid double free */
		next = osl_pktnext(osh, dup);
		while (next) {
			if (osl_pkttag(next))
				osl_pktsettag(next, NULL);
			next = osl_pktnext(osh, next);
		}
	} else if (pkttag) {
		osl_pkttag_free(osh, pkttag);
	}

	return (dup);
}

uchar*
osl_pktpush(void *m, int bytes)
{
	ASSERT(M_LEADINGSPACE((struct mbuf*)m) >= bytes);
	((struct mbuf *)m)->m_data -= bytes;
	((struct mbuf *)m)->m_len += bytes;

	return (uchar *)((struct mbuf *)m)->m_data;
}

uchar*
osl_pktpull(void *m, int bytes)
{
	m_adj((struct mbuf *)m, bytes);
	return (uchar *)((struct mbuf *)m)->m_data;
}

/* Convert a native(OS) packet to driver packet.
 * In the process, native packet is destroyed, there is no copying
 * Also, a packettag is attached if requested.
 * The conversion fails if pkttag cannot be allocated
 */
void*
osl_pkt_frmnative(osl_t *osh, void *m)
{
	void *pkttag = NULL;

	/* no pkttag, no packet */
	if (osh && osh->pkttag_pool)
		if ((pkttag = osl_pkttag_alloc(osh)) == NULL)
			return NULL;

	osl_pktsettag(m, pkttag);

	return (void *)m;
}

/* Convert a driver packet to native(OS) packet
 * In the process, packettag is removed
 */
void*
osl_pkt_tonative(osl_t *osh, void *m)
{
	if (osh && osh->pkttag_pool) {
		osl_pkttag_free(osh, osl_pkttag(m));
		osl_pktsettag(m, NULL);
	}
	return m;
}

void*
osl_pkttag_alloc(osl_t *osh)
{
	void *pkttag = NULL;

	if (osh == NULL)
		return NULL;

	pkttag = bcmcache_alloc(osh->pkttag_pool);

	if (pkttag) {
		bzero(pkttag, OSL_PKTTAG_SZ);
		osh->alloced++;
	}

	return pkttag;
}

void
osl_pktsettag(void *m, void *n)
{
	M_SETCTX((struct mbuf *) m, n);

	if (n)
		((struct mbuf *)(m))->m_flags |= M_PKTTAG;
}

/* Free a packet tag back to working set */
void
osl_pkttag_free(osl_t *osh, void *pkttag)
{
	if (osh && pkttag) {
		bcmcache_free(osh->pkttag_pool, pkttag);
		ASSERT(osh->alloced > 0);
		osh->alloced--;
	}
}

uchar*
osl_pktdata(osl_t *osh, void *m)
{
	return (uchar *)((struct mbuf *)(m))->m_data;
}

uint
osl_pktlen(osl_t *osh, void *m)
{
	return ((struct mbuf *)(m))->m_len;
}

void*
osl_pktnext(osl_t *osh, void *m)
{
	return ((struct mbuf*)m)->m_next;

}

void
osl_pktsetnext(osl_t *osh, void *m, void *n)
{
	((struct mbuf*)m)->m_next = (struct mbuf*)n;
}

void
osl_pktsetlen(osl_t *osh, void *m, int dlen)
{
	((struct mbuf*)m)->m_len = dlen;
}

void*
osl_pktlink(void *m)
{
	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);
	return ((struct mbuf *)(m))->m_nextpkt;
}

void
osl_pktsetlink(void *m, void *n)
{
	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);
	((struct mbuf *)(m))->m_nextpkt = (struct mbuf *)(n);
}

void*
osl_pkttag(void *m)
{
	if (((struct mbuf *)(m))->m_flags & M_PKTTAG)
		return (void *)M_GETCTX((struct mbuf *) m, struct mbuf *);
	return NULL;
}

uint
osl_pktheadroom(osl_t *osh, void *m)
{
	return (M_LEADINGSPACE((struct mbuf*)m));
}

uint
osl_pkttailroom(osl_t *osh, void *m)
{
	return (M_TRAILINGSPACE((struct mbuf*)m));
}

bool
osl_ptkshared(void *m)
{
	return (M_SHAREDCLUSTER((struct mbuf *)(m)));
}

uint
osl_pktprio(void *m)
{
	ASSERT(m);
	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);

	return (uint)((struct mbuf *)m)->m_pkthdr.priority;
}

void
osl_pktsetprio(void *m, uint x)
{
	ASSERT(m);
	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);

	((struct mbuf *)m)->m_pkthdr.priority = x;
}


uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	uint val;
	uint retry = PCI_CFG_RETRY;

	ASSERT(osh);

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		pci_read_config_dword(osh->devinfo, offset, &val);
		if (val != 0xffffffff)
			break;
	} while (retry--);

	return (val);
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	uint retry = PCI_CFG_RETRY;

	ASSERT(osh);

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		pci_write_config_dword(osh->devinfo, offset, val);
		if (offset != PCI_BAR0_WIN)
			break;
		if (osl_pci_read_config(osh, offset, size) == val)
			break;
	} while (retry--);
}

/* return bus # for the pci device pointed by osh->devinfo */
uint
osl_pci_bus(osl_t *osh)
{
	ASSERT(osh && osh->devinfo);

	return ((struct pci_dev *)osh->devinfo)->bus->number;
}

/* return slot # for the pci device pointed by osh->devinfo */
uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->devinfo);

	return PCI_SLOT(((struct pci_dev *)osh->devinfo)->devfn);
}

void
osl_pcmcia_attr(void *osh, uint offset, char *buf, int size, bool write)
{
}

void
osl_pcmcia_read_attr(void *osh, uint offset, void *buf, int size)
{
	osl_pcmcia_attr(osh, offset, (char *) buf, size, FALSE);
}

void
osl_pcmcia_write_attr(void *osh, uint offset, void *buf, int size)
{
	osl_pcmcia_attr(osh, offset, (char *) buf, size, TRUE);
}

void*
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, ulong *pap)
{
	void *va;
	uint16 align = (1 << align_bits);

	/* fix up the alignment requirements first */
	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

	if ((va = malloc(size)) == NULL)
		return (NULL);

	HAL_DCACHE_FLUSH((vaddr_t)va, size);
	*pap = (ulong)CYGARC_PHYSICAL_ADDRESS(va);
	va = (void *)CYGARC_UNCACHED_ADDRESS(va);
	return (va);
}

void
osl_dma_free_consistent(osl_t *osh, uint size, void *va, ulong pa)
{
	va = ((void*)(CYGARC_CACHED_ADDRESS(CYGARC_PHYSICAL_ADDRESS(va))));
	free(va);
}

void*
osl_dma_map(osl_t *osh, void *va, uint size, uint direction)
{
	if (direction == DMA_TX)
		HAL_DCACHE_STORE((vaddr_t)va, size);	/* write back */
	else
		HAL_DCACHE_FLUSH((vaddr_t)va, size);	/* write back and invalidate */
	return ((void*)CYGARC_PHYSICAL_ADDRESS(va));
}

void
osl_assert(char *exp, char *file, int line)
{
	char tempbuf[255];

	sprintf(tempbuf, "\nassertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	printf("ASSERT: %s\n", tempbuf);
}
