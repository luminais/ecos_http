 /******************************************************************************
  *
  * Name: if_819x_wlan.c - Realtek 819x wlan driver
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_rltk_819x_wlan.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <string.h> /* for memset */

/* Check if we should be dumping debug information or not */
#if defined RTLDBG_DEVS_ETH_RLTK_819X_CHATTER \
    && (RTLDBG_DEVS_ETH_RLTK_819X_CHATTER > 0)
#define DEBUG_RLTK819X_WLAN_DRIVER
#endif

#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include "if_819x_wlan.h"
#include "./rtl8192cd/8192cd.h"
#include "./rtl8192cd/8192cd_cfg.h"

/* Allow platform-specific configuration of the driver */
#ifndef RTLDAT_DEVS_ETH_RLTK_819X_WLAN0_INL
#error "RTLDAT_DEVS_ETH_RLTK_819X_WLAN0_INL not defined"
#else
#include RTLDAT_DEVS_ETH_RLTK_819X_WLAN0_INL
#endif

#ifndef RTLDAT_DEVS_ETH_RLTK_819X_WLAN1_INL
#error "RTLDAT_DEVS_ETH_RLTK_819X_WLAN1_INL not defined"
#else
#include RTLDAT_DEVS_ETH_RLTK_819X_WLAN1_INL
#endif

#ifndef RTLHWR_RLTK_819X_WLAN_PLF_INIT
#define RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc) do {} while(0)
#endif

/*
 * If software cache coherency is required, the HAL_DCACHE_INVALIDATE
 * hal macro must be defined as well.
 */
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_SOFTWARE_CACHE_COHERENCY
#if !defined HAL_DCACHE_INVALIDATE || !defined HAL_DCACHE_FLUSH
#error "HAL_DCACHE_INVALIDATE/HAL_DCACHE_FLUSH not defined for this platform but RTLPKG_DEVS_ETH_RLTK_819X_CACHE_COHERENCY was defined."
#endif
#endif

/* Local driver function declarations */
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
static cyg_uint32 rltk819x_wlan_isr(cyg_vector_t vector, cyg_addrword_t data);
#endif

static bool rltk819x_wlan_init(struct cyg_netdevtab_entry *tab);
#ifdef CONFIG_RTL_819X_ECOS
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
static bool rltk819x_wlan_pwlan_init(struct cyg_netdevtab_entry *tab);
#endif
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
static bool rltk819x_wlan_vxd_init(struct cyg_netdevtab_entry *tab);
#endif
static void rltk819x_wlan_start(struct eth_drv_sc *sc, unsigned char *enaddr,
                           int flags);
static void rltk819x_wlan_stop(struct eth_drv_sc *sc);
static int rltk819x_wlan_control(struct eth_drv_sc *sc, unsigned long key,
                            void *data, int   data_length);
static int rltk819x_wlan_can_send(struct eth_drv_sc *sc);
static void rltk819x_wlan_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len, int total_len, unsigned long key);
static void rltk819x_wlan_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len);
static void rltk819x_wlan_deliver(struct eth_drv_sc *sc);
static void rltk819x_wlan_poll(struct eth_drv_sc *sc);
static int rltk819x_wlan_int_vector(struct eth_drv_sc *sc);

	
/* External functions */
extern void *rltk_wlan_init(int dev_num);
extern int  rtl8192cd_set_hwaddr(struct net_device*dev, void *addr);
extern int ecos_send_wlan(struct net_device *dev, unsigned char *data, int size);
extern int ecos_send_wlan_mesh(struct net_device *dev, unsigned char *data, int size);
extern struct net_device *rltk_get_wds_net_device(int dev_num, int wds_num);
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
static bool rltk819x_wlan_pwlan_init(struct cyg_netdevtab_entry *tab);
#endif

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
extern void *rltk_wlan_vxd_init(int wlan_idx);
#endif
#ifdef CONFIG_RTL_VAP_SUPPORT
extern void *rltk_wlan_vap_init(int wlan_idx, int vap_idx);
#endif

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
 * 2. Use the interrupt mask register in the 8196 to mask just the interrupt
 *    request coming from the 8196. This way, the other devices' requests
 *    can still be serviced.
 */
static cyg_uint32
rltk819x_wlan_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	Rltk819x_t *rltk819x_info;
	int call_dsr;

	rltk819x_info = (Rltk819x_t *)(((struct eth_drv_sc *)data)->driver_private);
	rtk_interrupt_count[vector]++;

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
static void rltk819x_wlan_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
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
static bool
rltk819x_wlan_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	void *dev;

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("%s(%s) is called\n", __FUNCTION__, tab->name);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rltk_wlan_init((rltk819x_info->device_num>>16) & 0xffff);
	if (dev == NULL) {
		DBG_ERR("%s: rltk_wlan_init(0x%x) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	if ((rltk819x_info->device_num & 0xffff) == 0) { /* root Ethernet driver */
#ifdef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
#ifdef DEBUG_RLTK819X_WLAN_DRIVER
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
                             rltk819x_wlan_isr,
#ifdef ISR_DIRECT
                             rltk819x_wlan_dsr,
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
	RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
static bool
rltk819x_wlan_wds_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev;

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("%s(%s) is called\n", __FUNCTION__, tab->name);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rltk_get_wds_net_device(((rltk819x_info->device_num>>16) & 0xffff), (rltk819x_info->device_num & 0xffff));
	if (dev == NULL) {
		DBG_ERR("%s: rltk_get_wds_net_device(0x%x) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	 /* platform depends initialize */
	RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}
#endif

#ifdef CONFIG_RTK_MESH
static bool
rltk819x_wlan_mesh_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev;
	
	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rltk_get_mesh_net_device(((rltk819x_info->device_num>>16) & 0xffff), (rltk819x_info->device_num & 0xffff));
	if (dev == NULL) {
		DBG_ERR("%s: rltk_get_mesh_net_device(0x%x) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	 /* platform depends initialize */
	RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
static bool
rltk819x_wlan_vap_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev;

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("%s(%s) is called\n", __FUNCTION__, tab->name);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rltk_wlan_vap_init(((rltk819x_info->device_num>>16) & 0xffff), (rltk819x_info->device_num & 0xffff));
	if (dev == NULL) {
		DBG_ERR("%s: rltk_wlan_vap_init(0x%x) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	 /* platform depends initialize */
	RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
extern struct net_device *rltk_get_pwlan_net_device(int dev_num, int device_num);

static bool
rltk819x_wlan_pwlan_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev=NULL;

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("%s(%s) is called\n", __FUNCTION__, tab->name);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rltk_get_pwlan_net_device(0, rltk819x_info->device_num);
	
	//diag_printf("dev:%s,%d,%s(%d) is called\n", dev->name,rltk819x_info->device_num,__FUNCTION__, __LINE__);
	if (dev == NULL) {
		DBG_ERR("%s: rltk_get_pwlan_net_device(0, %d) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	 /* platform depends initialize */
	RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}
#endif

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
static bool
rltk819x_wlan_vxd_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev;

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("%s(%s) is called\n", __FUNCTION__, tab->name);
#endif

	sc = (struct eth_drv_sc *)(tab->device_instance);
	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	rltk819x_info->sc = sc;

	wrapper_init();

	dev = rltk_wlan_vxd_init((rltk819x_info->device_num>>16) & 0xffff);
	if (dev == NULL) {
		DBG_ERR("%s: rltk_wlan_vxd_init(%d) failed\n",
				__FUNCTION__, rltk819x_info->device_num);
		return false;
	}

	wrapper_binding(rltk819x_info, dev);
	
	 /* platform depends initialize */
	RTLHWR_RLTK_819X_WLAN_PLF_INIT(sc);

	/* Initialize upper level driver */
	(sc->funs->eth_drv->init)(sc, rltk819x_info->mac);

	return true;
}
#endif


/*
 * (Re)Start the chip, initializing data structures and enabling the
 * transmitter and receiver. Currently, 'flags' is unused by eCos.
 */
static void
rltk819x_wlan_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
	Rltk819x_t *rltk819x_info =  (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_start(%s)\n", sc->dev_name);
#endif

	wrapper_start(rltk819x_info);
}

/*
 * Stop the chip, disabling the transmitter and receiver.
 */
static void
rltk819x_wlan_stop(struct eth_drv_sc *sc)
{
	Rltk819x_t *rltk819x_info =  (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_stop(%s)\n", sc->dev_name);
#endif

	wrapper_stop(rltk819x_info);
}

#ifdef CONFIG_RTL_ALP
#define B1_G1	40
#define B1_G2	48

#define B2_G1	56
#define B2_G2	64

#define B3_G1	104
#define B3_G2	112
#define B3_G3	120
#define B3_G4	128
#define B3_G5	136
#define B3_G6	144

#define B4_G1	153
#define B4_G2	161
#define B4_G3	169
#define B4_G4	177

void assign_diff_AC(unsigned char* pMib, unsigned char* pVal)
{
	int x=0, y=0;

	memset((pMib+35), pVal[0], (B1_G1-35));
	memset((pMib+B1_G1), pVal[1], (B1_G2-B1_G1));
	memset((pMib+B1_G2), pVal[2], (B2_G1-B1_G2));
	memset((pMib+B2_G1), pVal[3], (B2_G2-B2_G1));
	memset((pMib+B2_G2), pVal[4], (B3_G1-B2_G2));
	memset((pMib+B3_G1), pVal[5], (B3_G2-B3_G1));
	memset((pMib+B3_G2), pVal[6], (B3_G3-B3_G2));
	memset((pMib+B3_G3), pVal[7], (B3_G4-B3_G3));
	memset((pMib+B3_G4), pVal[8], (B3_G5-B3_G4));
	memset((pMib+B3_G5), pVal[9], (B3_G6-B3_G5));
	memset((pMib+B3_G6), pVal[10], (B4_G1-B3_G6));
	memset((pMib+B4_G1), pVal[11], (B4_G2-B4_G1));
	memset((pMib+B4_G2), pVal[12], (B4_G3-B4_G2));
	memset((pMib+B4_G3), pVal[13], (B4_G4-B4_G3));

}

#include <stdio.h>
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif

/*
 * 8196 control function. Unlike a 'real' ioctl function, this function is
 * not required to tell the caller why a request failed, only that it did
 * (see the eCos documentation).
 */
static int
rltk819x_wlan_control(struct eth_drv_sc *sc, unsigned long key, void *data,
                 int data_length)
{
	int i;
	Rltk819x_t *rltk819x_info;
	struct net_device *dev;
	int error = 0;
#ifdef CONFIG_RTL_ALP
	struct rtl8192cd_priv *priv;
	int ret=0;
#endif

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_control(%08x, %lx)\n", sc, key);
#endif

	rltk819x_info = (Rltk819x_t *)(sc->driver_private);
#ifdef CONFIG_RTL_ALP
	dev = (struct net_device *)rltk819x_info->dev;
	priv = (struct rtl8192cd_priv *)dev->priv;
#endif
	if ((key >= SIOCDEVPRIVATE && key <= SIOCDEVPRIVATE + 15) ||
	    (key >= SIOCIWFIRST && key <= SIOCIWLAST)) {
	    	dev = (struct net_device *)(rltk819x_info->dev);
		error = (dev->do_ioctl)(dev, data, key);
		//diag_printf("rltk819x_wlan_control key=%x error=%d\n", key, error);
		return error;
	}

	switch (key) {
#ifdef ETH_DRV_SET_MAC_ADDRESS
	case ETH_DRV_SET_MAC_ADDRESS:
		if ( 6 != data_length )
			return 1;
		/* Set the mac address */
		rtl8192cd_set_hwaddr(rltk819x_info->dev, data);
		for (i = 0; i < 6; ++i) {
			rltk819x_info->mac[i] = *(((cyg_uint8 *)data) + i);
		}
		return 0;
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
#ifdef CONFIG_RTL_ALP
			ret=0;
			break;
#endif
#endif
		// drop through
#ifdef ETH_DRV_GET_IF_STATS
	case ETH_DRV_GET_IF_STATS:
#ifdef CONFIG_RTL_ALP
			ret=0;
			break;
#endif
#endif
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
		break;
#endif

#ifdef ETH_DRV_SET_MC_LIST
	case ETH_DRV_SET_MC_LIST:
#ifdef CONFIG_RTL_ALP
			ret=0;
#endif
		break;
#endif // ETH_DRV_SET_MC_LIST

#ifdef ETH_DRV_SET_MC_ALL
	case ETH_DRV_SET_MC_ALL:
#ifdef CONFIG_RTL_ALP
			ret=0;
#endif
		break;
#endif // ETH_DRV_SET_MC_ALL
	
#ifdef CONFIG_RTL_ALP
#define DBG_PRINT
#ifdef ETH_DRV_SET_WIFI
	case ETH_DRV_SET_WIFI:
			{
				struct wifireq *wifir;
				wifir = (struct wifireq *)data;
				switch(wifir->type)
				{
					case ETH_DRV_SET_WIFI_OPMODE:
						diag_printf("%s: ETH_DRV_SET_WIFI_OPMODE: %s\n", __func__, (char *)wifir->value);
						if (wifir->length != 0){
							//diag_printf("%s: ETH_DRV_SET_WIFI_OPMODE: %s\n", __func__, (char *)wifir->value);
							if (!memcmp((char *)wifir->value, "ap", strlen("ap"))){
								//priv->dot11OperationEntry.opmode = WIFI_AP_STATE;
								OPMODE = WIFI_AP_STATE;
							} else if (!memcmp((char *)wifir->value, "client", strlen("client"))){
								//priv->dot11OperationEntry.opmode = WIFI_STATION_STATE;
								OPMODE = WIFI_STATION_STATE;
							} else if (!memcmp((char *)wifir->value, "repeater", strlen("repeater"))){
								//diag_printf("Repeater isn't supported!!!\n");
							}
						}
						break;

					case ETH_DRV_SET_WIFI_SSID:
						if (wifir->length<=32)
						{
							/* dot11DesiredSSID, dot11DesiredSSIDLen */
							memset(SSID, 0x0, 32);
							memcpy((char *)SSID, (char *)wifir->value, wifir->length);
							SSID_LEN = wifir->length;

							/* dot11SSIDtoScan, dot11SSIDtoScanLen */
							memset(priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0x0, 32);
							memcpy(priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan, (char *)wifir->value, wifir->length);
							priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = wifir->length;

							ret = 0;
						}
						else
							ret = -1;
						break;
#if 0
					case ETH_DRV_SET_WIFI_AUTOCHANNEL:
						ret = 0;
						if ((bool *)(wifir->value==true)){
						}else if ((bool *)(wifir->value==false)){
						}else
							ret = -1;
						break;
#endif
					case ETH_DRV_SET_WIFI_CHANNEL:
						priv->pmib->dot11RFEntry.dot11channel = (int *)wifir->value;
						if ((int *)wifir->value == 0){
							priv->pmib->dot11RFEntry.dot11ch_low = 1;
							priv->pmib->dot11RFEntry.dot11ch_hi = 11;
						}
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_SSIDHIDDEN:
						ret = 0;
						if ((bool *)(wifir->value)==true){
							HIDDEN_AP = 1;
						}else if((bool *)(wifir->value)==false){
							HIDDEN_AP = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_SECURITY:
						ret = 0;
						priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 0;		//None
						priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;	//None
						if (!strcmp((char *)(wifir->value), "disable")){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;		//0
						}else if (!strcmp((char *)(wifir->value), "wpapsk")){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _TKIP_PRIVACY_;	//2:WPA
							priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 1;						//WPA: bit0=1
						}else if (!strcmp((char *)(wifir->value), "wpa2psk")){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _TKIP_PRIVACY_;	//2:WPA
							priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 2;						//WPA2: bit1=1
						}else if (!strcmp((char *)(wifir->value), "wpaautopsk")){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _TKIP_PRIVACY_;	//2:WPA
							priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 3;						//WPA2: bit0=1;bit1=1
						}else if (!strcmp((char *)(wifir->value), "wep40")){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_40_PRIVACY_;	//1:WEP40
						}else if (!strcmp((char *)(wifir->value), "wep104")){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_104_PRIVACY_;//5:WEP104
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_AUTH:
						ret = 0;
						if (!strcmp((char *)(wifir->value), "shared")){
							priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 1; //Shared key
						}else if (!strcmp((char *)(wifir->value), "open")){
							priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0; //OPEN
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WEPKEYINDEX:
						if (((int *)(wifir->value)>=0)&&((int *)(wifir->value)<=3)){
							priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = (int *)(wifir->value);
							ret = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WEP40KEY0:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[0]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[0]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP40KEY1:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[1]), 0x0, sizeof(union Keytype));	
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[1]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP40KEY2:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[2]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[2]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP40KEY3:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[3]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[3]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP104KEY0:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[0]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[0]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP104KEY1:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[1]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[1]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP104KEY2:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[2]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[2]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WEP104KEY3:
						memset(&(priv->pmib->dot11DefaultKeysTable.keytype[3]), 0x0, sizeof(union Keytype));
						memcpy(&(priv->pmib->dot11DefaultKeysTable.keytype[3]), (char *)(wifir->value), wifir->length);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_CIPHER:
						ret = 0;
						if (!memcmp((char *)wifir->value, "auto", strlen("auto"))){
							priv->pmib->dot1180211AuthEntry.dot11WPACipher = 10; //bit2: wrap
							priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 10;//bit2: wrap
						}else if (!memcmp((char *)wifir->value, "tkip", strlen("tkip"))){
							priv->pmib->dot1180211AuthEntry.dot11WPACipher = 2; //bit1: tkip
							priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 2;//bit1: tkip
						}else if (!memcmp((char *)wifir->value, "aes", strlen("aes"))){
							priv->pmib->dot1180211AuthEntry.dot11WPACipher = 8; //bit3: ccmp
							priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 8;//bit3: ccmp
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_PSK:
						memset(priv->pmib->dot1180211AuthEntry.dot11PassPhrase, 0x0, 65);
						memcpy(priv->pmib->dot1180211AuthEntry.dot11PassPhrase, (char *)(wifir->value), 64);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_TXPOWER:
						{
							int ver;
							char *buff;
							FILE *wlconf_ptr;
							PARAM_HEADER_T hsHeader;
							HW_SETTING_T hs;
							int idx;
							int intVal;

							wlconf_ptr=bf_open("wlconf");
							if (wlconf_ptr!=NULL)
							{
								bf_read((char *)&hsHeader, sizeof(hsHeader), wlconf_ptr);
								if (!memcmp(&hsHeader.signature[TAG_LEN], "01", strlen("01")))
									ver = 1;
								else
									ver = -1;
								if (memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
										(ver!=HW_SETTING_VER) || // version not equal to current
										(hsHeader.len<(sizeof(HW_SETTING_T)+1))) // length is less than current
								{
									diag_printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",
											hsHeader.signature[0], hsHeader.signature[1], ver, hsHeader.len);
									return NULL;
								}
								if (ver > HW_SETTING_VER)
									diag_printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);

								bf_read((char *)&hs, sizeof(hs), wlconf_ptr);
								
								intVal = hs.boardVer;
								if (intVal == 1)
									priv->pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
								else if(intVal == 2)
									priv->pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
								else
									priv->pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R

								
								//start to setup caldata
								for (idx=0; idx<MAX_2G_CHANNEL_NUM; idx++)
								{
									priv->pmib->dot11RFEntry.pwrlevelCCK_A[idx]     = hs.wlan[0].pwrlevelCCK_A[idx];
									priv->pmib->dot11RFEntry.pwrlevelCCK_B[idx]     = hs.wlan[0].pwrlevelCCK_B[idx];
									priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[idx] = hs.wlan[0].pwrlevelHT40_1S_A[idx];
									priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[idx] = hs.wlan[0].pwrlevelHT40_1S_B[idx];
									priv->pmib->dot11RFEntry.pwrdiffHT40_2S[idx]    = hs.wlan[0].pwrdiffHT40_2S[idx];
									priv->pmib->dot11RFEntry.pwrdiffHT20[idx]       = hs.wlan[0].pwrdiffHT20[idx];
									priv->pmib->dot11RFEntry.pwrdiffOFDM[idx]       = hs.wlan[0].pwrdiffOFDM[idx];
								}
								priv->pmib->dot11RFEntry.dot11RFType = hs.wlan[0].rfType;
								
								memcpy(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A, hs.wlan[0].pwrlevel5GHT40_1S_A, MAX_5G_CHANNEL_NUM_MIB);
								memcpy(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B, hs.wlan[0].pwrlevel5GHT40_1S_B, MAX_5G_CHANNEL_NUM_MIB);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S, hs.wlan[0].pwrdiff5GHT40_2S, MAX_5G_CHANNEL_NUM_MIB);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff5GHT20, hs.wlan[0].pwrdiff5GHT20, MAX_5G_CHANNEL_NUM_MIB);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff5GOFDM, hs.wlan[0].pwrdiff5GOFDM, MAX_5G_CHANNEL_NUM_MIB);
								// 5G
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A, (unsigned char*) hs.wlan[0].pwrdiff_5G_20BW1S_OFDM1T_A);	
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A, (unsigned char*)hs.wlan[0].pwrdiff_5G_40BW2S_20BW2S_A);
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A, (unsigned char*)hs.wlan[0].pwrdiff_5G_80BW1S_160BW1S_A);
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A, (unsigned char*)hs.wlan[0].pwrdiff_5G_80BW2S_160BW2S_A);

								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B, (unsigned char*)hs.wlan[0].pwrdiff_5G_20BW1S_OFDM1T_B);	
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B, (unsigned char*)hs.wlan[0].pwrdiff_5G_40BW2S_20BW2S_B);
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B, (unsigned char*)hs.wlan[0].pwrdiff_5G_80BW1S_160BW1S_B);
								//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, (void *)buf1);
								assign_diff_AC(priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B, (unsigned char*)hs.wlan[0].pwrdiff_5G_80BW2S_160BW2S_B);

								// 2G
								//apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, (void *)buf1);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A, hs.wlan[0].pwrdiff_20BW1S_OFDM1T_A, MAX_2G_CHANNEL_NUM_MIB);	
								//apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, (void *)buf1);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A, hs.wlan[0].pwrdiff_40BW2S_20BW2S_A, MAX_2G_CHANNEL_NUM_MIB);

								//apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, (void *)buf1);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B, hs.wlan[0].pwrdiff_20BW1S_OFDM1T_B, MAX_2G_CHANNEL_NUM_MIB);
								//apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, (void *)buf1);
								memcpy(priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B, hs.wlan[0].pwrdiff_40BW2S_20BW2S_B, MAX_2G_CHANNEL_NUM_MIB);

								//apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
								priv->pmib->dot11RFEntry.tssi1 = (unsigned int)hs.wlan[0].TSSI1;

								//apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
								priv->pmib->dot11RFEntry.tssi2 = (unsigned int)hs.wlan[0].TSSI2;
								
								//apmib_get(MIB_HW_11N_TRSWITCH, (void *)&intVal);
								priv->pmib->dot11RFEntry.trswitch = (unsigned int)hs.wlan[0].trswitch;	
								
								//apmib_get(MIB_HW_11N_TRSWPAPE_C9, (void *)&intVal);
								priv->pmib->dot11RFEntry.trsw_pape_C9 = hs.wlan[0].trswpape_C9;
								
								//apmib_get(MIB_HW_11N_TRSWPAPE_CC, (void *)&intVal);
								priv->pmib->dot11RFEntry.trsw_pape_CC = hs.wlan[0].trswpape_CC;
								
								//apmib_get(MIB_HW_11N_TARGET_PWR, (void *)&intVal);
								priv->pmib->dot11RFEntry.target_pwr = (unsigned int)hs.wlan[0].target_pwr;;

	
								//apmib_get(MIB_HW_11N_PA_TYPE, (void *)&intVal);
								priv->pmib->dot11RFEntry.pa_type = (unsigned int)hs.wlan[0].pa_type;

								
							}
							bf_close(wlconf_ptr);
						}
						if (((int *)(wifir->value)>=0)&&((int *)(wifir->value)<=17)){
							if (priv->pmib->dot11RFEntry.dot11RFType == 10) {
								int intVal = (int *)(wifir->value);
								if (intVal) {
									for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++) {
										if(priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0){ 
											if ((priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] - intVal) >= 1)
												priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] -= intVal;
											else
												priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
										}
										if(priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0){ 
											if ((priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] - intVal) >= 1)
												priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] -= intVal;
											else
												priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
										}
										if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){ 
											if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] - intVal) >= 1)
												priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] -= intVal;
											else
												priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
										}
										if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){ 
											if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] - intVal) >= 1)
												priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] -= intVal;
											else
												priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
										}
									}
							for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++) {
								if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){ 
									if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] - intVal) >= 1)
										priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] -= intVal;
									else
										priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;					
								}
							if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){ 
								if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] - intVal) >= 1)
									priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] -= intVal;
								else
									priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
							}
							}		
		
						} 
							} 
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_xCap:
						 {	
							int ver;
							char *buff;
							FILE *wlconf_ptr;
							PARAM_HEADER_T hsHeader;
							HW_SETTING_T hs;
							int idx;
							unsigned int intVal;
				 
		
							wlconf_ptr=bf_open("wlconf");
							if (wlconf_ptr!=NULL)
							{
								bf_read((char *)&hsHeader, sizeof(hsHeader), wlconf_ptr);
								if (!memcmp(&hsHeader.signature[TAG_LEN], "01", strlen("01")))
									ver = 1;
								else
									ver = -1;
								if (memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
										(ver!=HW_SETTING_VER) || // version not equal to current
										(hsHeader.len<(sizeof(HW_SETTING_T)+1))) // length is less than current
								{
									diag_printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",
											hsHeader.signature[0], hsHeader.signature[1], ver, hsHeader.len);
									return NULL;
								}
								if (ver > HW_SETTING_VER)
									diag_printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);

								bf_read((char *)&hs, sizeof(hs), wlconf_ptr);

								//start to setup caldata
								if ((int *)(wifir->value) != 255){
									diag_printf("--------set WIFI_xCap from %d to %d !-----------\n",priv->pmib->dot11RFEntry.xcap, (int *)(wifir->value));
									priv->pmib->dot11RFEntry.xcap = (int *)(wifir->value);
								}else{
		
								if ( wrgac17_get_wlan2g5g() == 5) 
									intVal = hs.wlan[0].xCap;
								else
									intVal = hs.wlan[0].xCap2;
							
								diag_printf("--------set WIFI_xCap from %d to %d !-----------\n",priv->pmib->dot11RFEntry.xcap, intVal);

								priv->pmib->dot11RFEntry.xcap = intVal;
		
								}
							}
							bf_close(wlconf_ptr);
						}						
						break;
					case ETH_DRV_SET_WIFI_THER:
						 {	
							int ver;
							char *buff;
							FILE *wlconf_ptr;
							PARAM_HEADER_T hsHeader;
							HW_SETTING_T hs;
							int idx;
							unsigned int intVal;
				 
		
							wlconf_ptr=bf_open("wlconf");
							if (wlconf_ptr!=NULL)
							{
								bf_read((char *)&hsHeader, sizeof(hsHeader), wlconf_ptr);
								if (!memcmp(&hsHeader.signature[TAG_LEN], "01", strlen("01")))
									ver = 1;
								else
									ver = -1;
								if (memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
										(ver!=HW_SETTING_VER) || // version not equal to current
										(hsHeader.len<(sizeof(HW_SETTING_T)+1))) // length is less than current
								{
									diag_printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",
											hsHeader.signature[0], hsHeader.signature[1], ver, hsHeader.len);
									return NULL;
								}
								if (ver > HW_SETTING_VER)
									diag_printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);

								bf_read((char *)&hs, sizeof(hs), wlconf_ptr);

								//start to setup caldata
								if ((int *)(wifir->value) != 255){
									diag_printf("--------set WIFI_THER from %d to %d !-----------\n",priv->pmib->dot11RFEntry.ther, (int *)(wifir->value));
									priv->pmib->dot11RFEntry.ther = (int *)(wifir->value);
								}else{
									if ( wrgac17_get_wlan2g5g() == 5) 
										intVal = hs.wlan[0].Ther;
									else
										intVal = hs.wlan[0].Ther2;		
									diag_printf("--------set WIFI_THER from %d to %d !-----------\n",priv->pmib->dot11RFEntry.ther, intVal);
		
									priv->pmib->dot11RFEntry.ther = intVal;
									
								}
							}
							bf_close(wlconf_ptr);
						}						
						break;
					case ETH_DRV_SET_WIFI_BEACONINTERVAL:
						if (((int *)(wifir->value)>=20)&&((int *)(wifir->value)<=1000)){
							priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = (int *)(wifir->value);
							ret = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_RTSTHRESHOLD:
						if (((int *)(wifir->value)>=256) && ((int *)(wifir->value)<=2346)){
							RTSTHRSLD = (int *)(wifir->value);
							ret = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_FRAGMENTATION:
						if (((int *)(wifir->value)>=1500) && ((int *)(wifir->value)<=2346)){
							FRAGTHRSLD = (int *)(wifir->value);
							ret = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_DTIMINTERVAL:
						if (((int *)(wifir->value)>=1) && ((int *)(wifir->value)<=255)){
							priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod = (int *)(wifir->value);
							ret = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_PREAMBLE:
						ret = 0;
						if (!memcmp((char *)(wifir->value), "short", strlen("short"))){
							SHORTPREAMBLE = 1;
						}else if (!memcmp((char *)(wifir->value), "long", strlen("long"))){
							SHORTPREAMBLE = 0;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WIFIMODE:
						ret = 0;
						if (!memcmp("11bgn", (char *)(wifir->value), strlen("11bgn"))){
							priv->pmib->dot11BssType.net_work_type = (WIRELESS_11B | WIRELESS_11G | WIRELESS_11N);
							priv->pmib->dot11StationConfigEntry.legacySTADeny = 0;
						}else if (!memcmp("11bg", (char *)(wifir->value), strlen("11bg"))){
							priv->pmib->dot11BssType.net_work_type = (WIRELESS_11B | WIRELESS_11G);
							priv->pmib->dot11StationConfigEntry.legacySTADeny = 0;
						}else if (!memcmp("11n", (char *)(wifir->value), strlen("11n"))){
							priv->pmib->dot11BssType.net_work_type = 11;
							priv->pmib->dot11StationConfigEntry.legacySTADeny = 3;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_BANDWIDTH:
						ret = 0;
						if (!memcmp("auto", (char *)(wifir->value), strlen("auto"))){
							priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20_40;	
						}else if (!memcmp("20", (char *)(wifir->value), strlen("20"))){
							priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_SHORTGI:
						if ((bool *)(wifir->value)==true){
							priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
							priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
						}else if ((bool *)(wifir->value)==false){
							priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 0;
							priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 0;
						}else
							ret = -1;
						break;

					case ETH_DRV_SET_WIFI_ADDACL:
						ret = 0;
						if (priv->pmib->dot11StationConfigEntry.dot11AclNum<NUM_ACL)
						{
							memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr[priv->pmib->dot11StationConfigEntry.dot11AclNum], (char *)(wifir->value), MACADDRLEN);
							priv->pmib->dot11StationConfigEntry.dot11AclNum++;
						}
						else
						{
							ret = -1;
						}
						break;

					case ETH_DRV_SET_WIFI_FLUSHACL:
						memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr[priv->pmib->dot11StationConfigEntry.dot11AclNum+1], (char *)(wifir->value), MACADDRLEN*NUM_ACL);
						priv->pmib->dot11StationConfigEntry.dot11AclNum=0;
						break;

					case ETH_DRV_SET_WIFI_ACLMODE:
						if (!memcmp("disable", (char *)(wifir->value), strlen("disable"))){
							priv->pmib->dot11StationConfigEntry.dot11AclMode = 0;
						}else if (!memcmp("allow", (char *)(wifir->value), strlen("allow"))){
							priv->pmib->dot11StationConfigEntry.dot11AclMode = 1;
						}else if (!memcmp("reject", (char *)(wifir->value), strlen("reject"))){
							priv->pmib->dot11StationConfigEntry.dot11AclMode = 2;
						}else
							ret = -1;
						break;

					case ETH_DRV_SET_WIFI_AMPDU:
						if ((bool *)(wifir->value)==true){
							priv->pmib->dot11nConfigEntry.dot11nAMPDU = 1;
						}else if ((bool *)(wifir->value)==false){
							priv->pmib->dot11nConfigEntry.dot11nAMPDU = 0;
						}else
							ret = -1;
						break;

					case ETH_DRV_SET_WIFI_WIFI_SPEC:
						if (((int *)(wifir->value)>=0) && ((int *)(wifir->value)<=2)){
							priv->pmib->dot11OperationEntry.wifi_specific = (int *)(wifir->value);
							ret = 0;
						}else
							ret = -1;
						break;
					
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
					case ETH_DRV_SET_WIFI_WDS_STATUS:
						if ((bool *)(wifir->value)==true || (bool *)(wifir->value)==false){
							priv->pmib->dot11WdsInfo.wdsEnabled	= (bool *)(wifir->value);
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WDS_PURE:
						if ((bool *)(wifir->value)==true || (bool *)(wifir->value)==false){
							priv ->pmib->dot11WdsInfo.wdsPure = (bool *)(wifir->value);
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WDS_ADDR:
						if (priv->pmib->dot11WdsInfo.wdsNum < RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM){
							memcpy(priv->pmib->dot11WdsInfo.entry[priv->pmib->dot11WdsInfo.wdsNum].macAddr, (char *)(wifir->value), MACADDRLEN);
						}else{
							ret = -1;
						} 
						break;
					case ETH_DRV_SET_WIFI_WDS_NUM:
						if ((int *)(wifir->value) <= RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM){
							priv ->pmib->dot11WdsInfo.wdsNum = (int *)(wifir->value);
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WDS_SECURITY:
						ret = 0;
						if (!strcmp((char *)(wifir->value), "disable")){
							priv->pmib->dot11WdsInfo.wdsPrivacy = 0;
						}else if (!strcmp((char *)(wifir->value), "wpa2psk")){
							priv->pmib->dot11WdsInfo.wdsPrivacy = 4;
						}else if (!strcmp((char *)(wifir->value), "wep40")){
							priv->pmib->dot11WdsInfo.wdsPrivacy = 1;
						}else if (!strcmp((char *)(wifir->value), "wep104")){
							priv->pmib->dot11WdsInfo.wdsPrivacy = 5;
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_WDS_WEPKEY:
						memset(priv->pmib->dot11WdsInfo.wdsWepKey, 0x0, 26);
						memcpy(priv->pmib->dot11WdsInfo.wdsWepKey, (char *)(wifir->value), 26);
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WDS_PSK:
						memset(priv->pmib->dot11WdsInfo.wdsPskPassPhrase, 0x0, 64);
						memcpy(priv->pmib->dot11WdsInfo.wdsPskPassPhrase, (char *)(wifir->value), 64);
						ret = 0;
						break;
#endif /*RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS*/
					case ETH_DRV_SET_WIFI_REG_DOMAIN:
						priv->pmib->dot11StationConfigEntry.dot11RegDomain = 6;
						if ((int *)(wifir->value)==1 || (int *)(wifir->value)==2 ||
						    (int *)(wifir->value)==3 || (int *)(wifir->value)==5 ||
							(int *)(wifir->value)==6 || (int *)(wifir->value)==7 ||
							(int *)(wifir->value)==8 || (int *)(wifir->value)==10 ){
								priv->pmib->dot11RFEntry.dot11ch_low = 1;
								priv->pmib->dot11RFEntry.dot11ch_hi = 13;
						}else{ 
								priv->pmib->dot11RFEntry.dot11ch_low = 1;
								priv->pmib->dot11RFEntry.dot11ch_hi = 11; 
						}
						ret = 0;
						break;
					case ETH_DRV_SET_WIFI_WMM:
						ret = 0;
						if ((bool *)(wifir->value)==true || (bool *)(wifir->value)==false){
							priv->pmib->dot11QosEntry.dot11QosEnable = (bool *)(wifir->value);
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_COEXIST:
						ret = 0;
						if ((bool *)(wifir->value)==true || (bool *)(wifir->value)==false){
							priv->pmib->dot11nConfigEntry.dot11nCoexist = (bool *)(wifir->value);
						}else
							ret = -1;
						break;
					case ETH_DRV_SET_WIFI_TXRATE:
						ret = 0;
						if ((int *)(wifir->value)==0){
							priv->pmib->dot11StationConfigEntry.autoRate = 1;
						}else if ((int *)(wifir->value)>0){
							priv->pmib->dot11StationConfigEntry.autoRate = 0;
							priv->pmib->dot11StationConfigEntry.fixedTxRate = (int *)(wifir->value);
						}else{
							ret = -1;
						}
						break;
					case ETH_DRV_SET_WIFI_SITESURVEY:
						{
							extern int rtl8192cd_ss_req_toXml(struct rtl8192cd_priv *priv, unsigned char *data, int len);
							ret = 0;
							if ((int *)(wifir->value)==1){
								rtl8192cd_ss_req_toXML(priv);
							}
							ret = 0;
						}
						break;
					case ETH_DRV_SET_WIFI_GET_SITESURVEY:
						{
							extern int rtl8192cd_get_ss_status_toXML(struct rtl8192cd_priv *priv, int *data);
							ret = 0;
							if ((int *)(wifir->value)==1){
								rtl8192cd_get_ss_status_toXML(priv, &ret);
							}
							ret = 0;
						}
						break;
					case ETH_DRV_SET_WIFI_COPY_MIB:
						{	
							diag_printf("test:rltk819x_wlan_control  copy mib\n");
                					memcpy(priv->pmib, GET_ROOT(priv)->pmib, sizeof(struct wifi_mib));
                					ret = 0;
						}
                				break;
					default:
						ret = -1;
						break;			
				}
			}
			break;
#endif /*ETH_DRV_SET_WIFI*/
/* +++ */

	default:
		ret=1;
		break;
#endif
	}

#ifdef CONFIG_RTL_ALP
	return ret;
#else
	return 1;
#endif
}

/*
 * Check if a new packet can be sent.
 */
static int
rltk819x_wlan_can_send(struct eth_drv_sc *sc)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

	return wrapper_can_send(rltk819x_info);
}

/*
 * Send a packet over the wire.
 */
static void
rltk819x_wlan_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len,
              int total_len, unsigned long key)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_send(%s, %08x, %d, %d, %08lx)\n",
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
static void
rltk819x_wlan_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
              int sg_len)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_recv(%s)\n", sc->dev_name);
#endif

	wrapper_deliver(rltk819x_info, sg_list, sg_len);
}

/*
 * This function does all the heavy lifting associated with interrupts.
 */
static void
rltk819x_wlan_deliver(struct eth_drv_sc *sc)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);

#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_deliver(%s)\n", sc->dev_name);
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
	
	  if ((rltk819x_info->device_num & 0xffff) == 0) {
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
rltk819x_wlan_poll(struct eth_drv_sc *sc)
{
#ifdef DEBUG_RLTK819X_WLAN_DRIVER
	diag_printf("rltk819x_wlan_poll(%s)\n", sc->dev_name);
#endif

	rltk819x_wlan_deliver(sc);
}

/*
 * Return the interrupt vector used by this device.
 */
static int
rltk819x_wlan_int_vector(struct eth_drv_sc *sc)
{
  return ((Rltk819x_t *)(sc->driver_private))->vector;
}

int rltk819x_send_wlan(struct eth_drv_sc *sc, unsigned char *data, int size)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	
	return ecos_send_wlan((struct net_device *)rltk819x_info->dev, data, size);
}
int rltk819x_send_wlan_mesh(struct eth_drv_sc *sc, unsigned char *data, int size)
{
	Rltk819x_t *rltk819x_info = (Rltk819x_t *)(sc->driver_private);
	
	return ecos_send_wlan_mesh((struct net_device *)rltk819x_info->dev, data, size);
}

