 /******************************************************************************
  *
  * Name: if_819x.c - Realtek 819x ethernet driver
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_rltk_819x_switch.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <string.h> /* for memset */
#include "rtl_types.h"

/* Check if we should be dumping debug information or not */
#if defined RTLDBG_DEVS_ETH_RLTK_819X_SWITCH_CHATTER \
    && (RTLDBG_DEVS_ETH_RLTK_819X_SWITCH_CHATTER > 0)
#define DEBUG_RLTK819X_DRIVER
#endif

#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include "if_819x.h"

/* Allow platform-specific configuration of the driver */
#ifndef RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_INL
#error "RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_INL not defined"
#else
#include RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_INL
#endif

#ifndef RTLHWR_RLTK_819X_PLF_INIT
#define RTLHWR_RLTK_819X_PLF_INIT(sc) do {} while(0)
#endif

/*
 * If software cache coherency is required, the HAL_DCACHE_INVALIDATE
 * hal macro must be defined as well.
 */
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SOFTWARE_CACHE_COHERENCY
#if !defined HAL_DCACHE_INVALIDATE || !defined HAL_DCACHE_FLUSH
#error "HAL_DCACHE_INVALIDATE/HAL_DCACHE_FLUSH not defined for this platform but RTLPKG_DEVS_ETH_RLTK_819X_CACHE_COHERENCY was defined."
#endif
#endif

/* Local driver function declarations */
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
static cyg_uint32 rltk819x_isr(cyg_vector_t vector, cyg_addrword_t data);
#endif

static bool rltk819x_init(struct cyg_netdevtab_entry *tab);
static void rltk819x_start(struct eth_drv_sc *sc, unsigned char *enaddr,
                           int flags);
static void rltk819x_stop(struct eth_drv_sc *sc);
static int rltk819x_control(struct eth_drv_sc *sc, unsigned long key,
                            void *data, int   data_length);
static int rltk819x_can_send(struct eth_drv_sc *sc);
static void rltk819x_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len, int total_len, unsigned long key);
static void rltk819x_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len);
static void rltk819x_deliver(struct eth_drv_sc *sc);
static void rltk819x_poll(struct eth_drv_sc *sc);
static int rltk819x_int_vector(struct eth_drv_sc *sc);

extern int rtl865x_set_hwaddr(struct net_device *dev, void *addr);
extern void interrupt_dsr_tx(unsigned long cp);

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
/*
 * Interrupt service routine. We do not clear the interrupt status bits
 * (since this should really only be done after handling whatever caused
 * the interrupt, and that is done in the '_deliver' routine), but instead
 * clear the interrupt mask.
 *
 * If we are sharing interrupts with other devices, we have two options
 * (configurable):
 *
 * 1. Mask the interrupt vector completly. Personally I think this is a bad
 *    idea because the other devices sharing this interrupt are also masked
 *    until the network thread gets around to calling the '_deliver' routine.
 *
 * 2. Use the interrupt mask register in the 819x to mask just the interrupt
 *    request coming from the 819x. This way, the other devices' requests
 *    can still be serviced.
 */
//__IRAM_SECTION_
//__MIPS16
static cyg_uint32 rltk819x_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	Rltk819x_t *rltk819x_info;
	int call_dsr;

	rltk819x_info = (Rltk819x_t *)(((struct eth_drv_sc *)data)->driver_private);
	rtk_interrupt_count[vector]++;

	//diag_printf("rltk819x_isr(%s)\n", ((struct eth_drv_sc *)data)->dev_name);
	
	call_dsr = wrapper_isr(rltk819x_info);

	/* Mask the interrupt */
	cyg_interrupt_mask(vector);

	/* Acknowledge the interrupt for those platforms were this is necessary */
	cyg_interrupt_acknowledge(vector);

	if (call_dsr)
		return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
	else
		return (CYG_ISR_HANDLED);
}

#ifdef ISR_DIRECT
static void rltk819x_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	Rltk819x_t *rltk819x_info;
	rltk819x_info = (Rltk819x_t *)(((struct eth_drv_sc *)data)->driver_private);

	wrapper_dsr(rltk819x_info);

	eth_drv_dsr(vector, count, data);
}
#endif
#endif /* ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE */

/*
 * Initialize the network interface. Since the chips is reset by calling
 * _stop() and _start(), any code that will never need to be executed more
 * than once after system startup should go here.
 */
extern  void sys_led_turn_on_off(int val);

static bool
rltk819x_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	void *dev;

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("%s() is called\n", __FUNCTION__);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;
	//diag_printf("sc:%s,%d,[%s]:[%d].\n",sc->dev_name,rltk819x_info->device_num,__FUNCTION__,__LINE__);
	wrapper_init();

	dev = rtl865x_probe(rltk819x_info->device_num);
	if (dev == NULL) {
		DBG_ERR("%s: rtl865x_probe(%d) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;		
	}

	wrapper_binding(rltk819x_info, dev);
	
	if (rltk819x_info->device_num == 0) { /* root Ethernet driver */
#ifdef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
#ifdef DEBUG_RLTK819X_DRIVER
		diag_printf(" Does not generate interrupts.\n");
#endif
		rltk819x_info->interrupt_handle = 0;
#else
	     /*
		* Note that we use the generic eth_drv_dsr routine instead of
		* our own.
		*/

		rltk819x_info->isr_priority = 0;
		rtk_interrupt_count[rltk819x_info->vector] = 0;
		cyg_drv_interrupt_create(rltk819x_info->vector,
                             rltk819x_info->isr_priority,
                             (CYG_ADDRWORD)sc,
                             rltk819x_isr,
#ifdef ISR_DIRECT
				rltk819x_dsr,
#else
                             eth_drv_dsr,
#endif                             
                             &rltk819x_info->interrupt_handle,
                             &rltk819x_info->interrupt);
		
		cyg_drv_interrupt_attach(rltk819x_info->interrupt_handle);
#endif

	     /*
		* This is the indicator for "uses an interrupt". The code was lifted
		* from the eCos Intel 82559 driver.
		*/
		if (rltk819x_info->interrupt_handle != 0) {
		  	cyg_drv_interrupt_acknowledge(rltk819x_info->vector);

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
			cyg_drv_interrupt_unmask(rltk819x_info->vector);
#endif
		}
	}

	 /* platform depends initialize */
	RTLHWR_RLTK_819X_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);
    sys_led_turn_on_off(0);
	return true;
}

extern struct net_device *rtl_get_peth0_net_device(int device_num);

static bool
rltk819x_peth0_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev=NULL;

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("%s(%s) is called\n", __FUNCTION__, tab->name);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rtl_get_peth0_net_device(rltk819x_info->device_num);
	
	//diag_printf("dev:%s,%d,%s(%d) is called\n", dev->name,rltk819x_info->device_num,__FUNCTION__, __LINE__);
	if (dev == NULL) {
		DBG_ERR("%s: rltk_get_peth0_net_device(0, %d) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	 /* platform depends initialize */
	RTLHWR_RLTK_819X_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}


/*
 * (Re)Start the chip, initializing data structures and enabling the
 * transmitter and receiver. Currently, 'flags' is unused by eCos.
 */
static void
rltk819x_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
	Rltk819x_t *rltk819x_info =  (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_start(%s)\n", sc->dev_name);
#endif

	wrapper_start(rltk819x_info);
}

/*
 * Stop the chip, disabling the transmitter and receiver.
 */
static void
rltk819x_stop(struct eth_drv_sc *sc)
{
	Rltk819x_t *rltk819x_info =  (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_stop(%s)\n", sc->dev_name);
#endif

	wrapper_stop(rltk819x_info);
}

/*
 * 819x control function. Unlike a 'real' ioctl function, this function is
 * not required to tell the caller why a request failed, only that it did
 * (see the eCos documentation).
 */
static int
rltk819x_control(struct eth_drv_sc *sc, unsigned long key, void *data,
                 int data_length)
{
	int i;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev;
	int error = 0;

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_control(%s, 0x%lx)\n", sc->dev_name, key);
#endif

	rltk819x_info = (Rltk819x_t *)(sc->driver_private);

	if (key >= SIOCDEVPRIVATE && key <= SIOCDEVPRIVATE + 15) {
	    	dev = (struct net_device *)(rltk819x_info->dev);
		error = (dev->do_ioctl)(dev, data, key);
//		diag_printf("rltk819x_control  error=%d key=%d\n", error,key);
		return error;
	}
	
	switch (key) {
#ifdef ETH_DRV_SET_MAC_ADDRESS
	case ETH_DRV_SET_MAC_ADDRESS:
		if ( 6 != data_length )
			return 1;
		/* Set the mac address */
		if ( 0 == rtl865x_set_hwaddr(rltk819x_info->dev, data)) {
			for (i = 0; i < 6; ++i) {
				rltk819x_info->mac[i] = *(((cyg_uint8 *)data) + i);
			}
			return 0;
		}
		return 1;
#endif

#ifdef ETH_DRV_GET_MAC_ADDRESS
	case ETH_DRV_GET_MAC_ADDRESS:
		if (6 != data_length)
			return 1;

		memcpy(data, rltk819x_info->mac, 6);
		return 0;
#endif

#ifdef ETH_DRV_GET_IF_STATS_UD
	case ETH_DRV_GET_IF_STATS_UD: // UD == UPDATE
		//ETH_STATS_INIT( sc );    // so UPDATE the statistics structure
#endif
		// drop through
#ifdef ETH_DRV_GET_IF_STATS
	case ETH_DRV_GET_IF_STATS:
#endif
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
#ifdef CONFIG_RTL_819X
		dev = (struct net_device *)(rltk819x_info->dev);
		error = (dev->do_ioctl)(dev, data, ETH_DRV_GET_IF_STATISTIC);
		return error;
#else
		break;
#endif
#endif

#ifdef ETH_DRV_SET_MC_LIST
	case ETH_DRV_SET_MC_LIST:
		break;
#endif // ETH_DRV_SET_MC_LIST

#ifdef ETH_DRV_SET_MC_ALL
	case ETH_DRV_SET_MC_ALL:
		break;
#endif // ETH_DRV_SET_MC_ALL
	
	}
	return 1;
}

/*
 * Check if a new packet can be sent.
 */
//__IRAM_SECTION_
//__MIPS16
static int rltk819x_can_send(struct eth_drv_sc *sc)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

	return wrapper_can_send(rltk819x_info);
}

/*
 * Send a packet over the wire.
 */
//__IRAM_SECTION_
//__MIPS16
static void rltk819x_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len,
              int total_len, unsigned long key)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_send(%s, %08x, %d, %d, %08lx)\n",
			sc->dev_name, sg_list, sg_len, total_len, key);
#endif

//	CYG_ASSERT(total_len <= TX_BUF_SIZE, "packet too long");

	cyg_drv_isr_lock();

	wrapper_tx(rltk819x_info, sg_list, sg_len, total_len, key);

	cyg_drv_isr_unlock();
}

/*
 * This routine is called by eCOS to receive a packet.
 */
//__IRAM_SECTION_
//__MIPS16
static void rltk819x_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
              int sg_len)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_recv(%s)\n", sc->dev_name);
#endif

	wrapper_deliver(rltk819x_info, sg_list, sg_len);
}

/*
 * This function does all the heavy lifting associated with interrupts.
 */
//__IRAM_SECTION_
//__MIPS16
static void rltk819x_deliver(struct eth_drv_sc *sc)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_deliver(%s)\n", sc->dev_name);
#endif

	cyg_drv_isr_lock();

#ifdef ISR_DIRECT
	wrapper_que_retrieve((struct net_device *)rltk819x_info->dev);
#else
	wrapper_dsr(rltk819x_info);
#endif

#ifdef TX_PKT_FREE_QUEUE
	wrapper_free_tx_queue((struct net_device *)rltk819x_info->dev);
#endif
	if (rltk819x_info->device_num == 0) {
		cyg_drv_interrupt_unmask(rltk819x_info->vector);
	}
	

	cyg_drv_isr_unlock();
}

/*
 * '_poll' does the same thing as '_deliver'. It is called periodically when
 * the ethernet driver is operated in non-interrupt mode, for instance by
 * RedBoot.
 */
static void
rltk819x_poll(struct eth_drv_sc *sc)
{
#ifdef DEBUG_RLTK819X_DRIVER
	diag_printf("rltk819x_poll(%s)\n", sc->dev_name);
#endif

	rltk819x_deliver(sc);
}

/*
 * Return the interrupt vector used by this device.
 */
static int
rltk819x_int_vector(struct eth_drv_sc *sc)
{
  return ((Rltk819x_t *)(sc->driver_private))->vector;
}

int rltk819x_send_eth(struct eth_drv_sc *sc, unsigned char *data, int size)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	
	//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return ecos_send_eth((struct net_device *)rltk819x_info->dev, data, size);
}

