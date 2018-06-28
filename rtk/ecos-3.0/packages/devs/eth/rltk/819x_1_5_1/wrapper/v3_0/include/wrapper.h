 /******************************************************************************
  *
  * Name: wrapper.h - Wrapper header file
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#define DUMP_DEBUG_ERR
//#define DUMP_DEBUG_INFO
//#define DUMP_DEBUG_TRACE
//#define CHECK_ASSERT

#include "sys_support.h"
#include "skbuff.h"

#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>


#ifdef DUMP_DEBUG_ERR
#define DBG_ERR diag_printf
#else
#define DBG_ERR(format, args...)
#endif

#ifdef DUMP_DEBUG_TRACE
#define DBG_TRACE diag_printf
#else
#define DBG_TRACE(format, args...)
#endif

#ifdef DUMP_DEBUG_INFO
#define DBG_INFO diag_printf
#else
#define DBG_INFO(format, args...)
#endif

#ifdef CHECK_ASSERT
#define DBG_ASSERT	CYG_ASSERT
#else
#define DBG_ASSERT(a, b)
#endif

#ifndef FAILED
#define FAILED (-1)
#endif


/*
 * Device driver private data
 */
typedef struct {
	/* Device number. Used for actually finding the device */
	cyg_uint32 device_num;

	/* Binding net device */
	void *dev;
	struct eth_drv_sc *sc;
	
	/* Our current MAC address */
	unsigned char mac[6];

	/* pending Rx packet */
	void *skb;

	/* Interrupt handling stuff */
	cyg_vector_t  vector;
	cyg_handle_t  interrupt_handle;
	cyg_interrupt interrupt;

	/* device ISR priority */
	cyg_priority_t isr_priority;

#ifdef ISR_DIRECT	
	struct sk_buff_head rx_queue;	/* queue for indication packet */		
#endif
#ifdef TX_PKT_FREE_QUEUE
	struct sk_buff_head tx_queue;	/* queue for free tx buffer */
#endif
} Rltk819x_t;


/*
 *  Exported subroutines
 */

extern void *alloc_local(int size);
#ifdef ISR_DIRECT
extern void netif_rx(struct sk_buff *skb, struct net_device *root_dev);
#else
extern void netif_rx(struct sk_buff *skb);
#endif
extern struct net_device *alloc_etherdev(int sizeof_priv);
extern void wrapper_init(void);
extern void wrapper_binding(Rltk819x_t *info, void *dev);
extern int wrapper_isr(Rltk819x_t *info);
extern void wrapper_dsr(Rltk819x_t *info);
extern void wrapper_start(Rltk819x_t *info);
extern void wrapper_stop(Rltk819x_t *info);
extern int wrapper_can_send(Rltk819x_t *info);
extern void wrapper_tx(Rltk819x_t *info, struct eth_drv_sg *sg_list, int sg_len, \
			int total_len, unsigned long key);
extern void wrapper_que_free_tx_pkt(struct net_device *root_dev, struct sk_buff *skb);
extern void wrapper_up(struct sk_buff *skb);
extern void wrapper_deliver(Rltk819x_t *info, struct eth_drv_sg *sg_list, int sg_len);
#ifdef ISR_DIRECT
extern void	wrapper_que_up(struct sk_buff *skb, struct net_device *root_dev);
extern void wrapper_que_retrieve(struct net_device *dev);
extern int wrapper_que_len(struct net_device *dev);
#endif
extern void wrapper_free_tx_queue(struct net_device *dev);
extern struct net_device_stats *wrapper_get_stats(char *ifname);
#endif /* __WRAPPER_H__*/

