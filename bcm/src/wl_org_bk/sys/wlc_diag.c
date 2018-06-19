/*
 * Diagnostic routines for Broadcom 802.11 Networking Driver.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_diag.c,v 1.106 2010-01-27 02:36:31 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wlc_diag.h>
#include <wlc_bmac.h>
#include <wl_export.h>


#define DIAG_LED_SLEEP		80000	/* led blink gap in us */
#define DIAG_LED_BLINK_TIMES	3   /* # of times led should blink */

#define WLC_ACTIVITY_LED	1	/*  activity LED is GPIO bit 0 */
#define WLC_RADIO_LED_B		2	/*  radio LED from GPIO bit 1 */
#define WLC_RADIO_LED_A		4	/*  radio LED from GPIO bit 2 */
#define WLC_RADIO_LEDS		6	/*  all radio LEDs: GPIO bit 1 and bit 2 */


#ifndef WLC_LOW_ONLY
/* TODO: this needs to be updated when gpiocontrol is transferred from dot11 to cc */
static void
wlc_diag_led_toggle(wlc_hw_info_t *wlc_hw, uint led_bits, bool want)
{
	if (want == ON) {
		si_gpioout(wlc_hw->sih, led_bits, led_bits, GPIO_DRV_PRIORITY);
	} else {
		si_gpioout(wlc_hw->sih, led_bits, 0, GPIO_DRV_PRIORITY);
	}
}
#endif

static uint32
wlc_diag_srom(wlc_hw_info_t *wlc_hw)
{
	int nw;
	uint32 macintmask;
	srom_rw_t *s;
	uint32 ssize;
	uint32 status = WL_DIAGERR_SUCCESS;
	uint8 crc;
	int crc_range;

	WL_INFORM(("wlc_diag_srom():\n"));

	/* space for the entire srom */
	ssize = sizeof(srom_rw_t) + SROM_MAX;
	s = (srom_rw_t *)MALLOC(wlc_hw->osh, ssize);

	if (s == NULL) {
		status = WL_DIAGERR_NOMEM;
		return status;
	}

	if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS) {
		crc_range = SROM_MAX;
	}
#if defined(BCMUSBDEV)
	else {
		crc_range = srom_size(wlc_hw->sih, wlc_hw->osh);
	}
#else /* def BCMUSBDEV || def BCMSDIODEV */
	else {
		crc_range = (SROM8_SIGN+1) * 2;	/* must big enough for SROM8 */
	}
#endif 

	WL_INFORM(("crc_range = %d\n", crc_range));

	/* intrs off */
	macintmask = wl_intrsoff(wlc_hw->wlc->wl);

	nw = crc_range / 2;
	/* read first small number words from srom, then adjust the length, read all */
	if (srom_read(wlc_hw->sih, wlc_hw->sih->bustype, (void *)wlc_hw->regs,
		wlc_hw->osh, 0, crc_range, s->buf, FALSE))
		goto out;

	WL_ERROR(("%s: old[SROM4_SIGN] 0x%x, old[SROM8_SIGN] 0x%x\n",
	          __FUNCTION__,  s->buf[SROM4_SIGN], s->buf[SROM8_SIGN]));
	printf("%s: old[SROM4_SIGN] 0x%x, old[SROM8_SIGN] 0x%x\n",
	          __FUNCTION__,  s->buf[SROM4_SIGN], s->buf[SROM8_SIGN]);
	if ((s->buf[SROM4_SIGN] == SROM4_SIGNATURE) ||
	    (s->buf[SROM8_SIGN] == SROM4_SIGNATURE)) {
		nw = SROM4_WORDS;
		crc_range = nw * 2;
		if (srom_read(wlc_hw->sih, wlc_hw->sih->bustype, (void *)wlc_hw->regs,
			wlc_hw->osh, 0, crc_range, s->buf, FALSE))
			goto out;
	} else {
		/* Assert that we have already read enough for sromrev 2 */
		ASSERT(crc_range >= SROM_WORDS * 2);
		nw = SROM_WORDS;
		crc_range = nw * 2;
	}

	crc = ~hndcrc8((uint8 *)s->buf, nw * 2 - 1, CRC8_INIT_VALUE);
	WL_INFORM(("crc = 0x%x, s->buf[%d-1]=0x%x\n", crc, nw, s->buf[nw-1]));

	if ((s->buf[nw - 1] >> 8) != crc)
		status = WL_DIAGERR_SROM_BADCRC;

out:
	/* restore intrs */
	wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

	if (s)
		MFREE(wlc_hw->osh, s, ssize);
	return status;
}

#ifndef WLC_LOW_ONLY
static void
wlc_diag_led_blink(wlc_hw_info_t *wlc_hw, uint leds)
{
	int i;

	for (i = 0; i < DIAG_LED_BLINK_TIMES; i++) {
		wlc_diag_led_toggle(wlc_hw, leds, ON);
		OSL_DELAY(DIAG_LED_SLEEP);
		wlc_diag_led_toggle(wlc_hw, leds, OFF);
		OSL_DELAY(DIAG_LED_SLEEP);
	}

}
#endif

static uint32
wlc_diag_led(wlc_hw_info_t *wlc_hw)
{
#ifndef WLC_LOW_ONLY
	uint32 ledoe = WLC_ACTIVITY_LED | WLC_RADIO_LEDS;
	uint32 orig_ledoe;

	/* save led outen */
	orig_ledoe = si_gpioouten(wlc_hw->sih, 0, 0, GPIO_DRV_PRIORITY);

	/* Enable the first 3 gpios */
	si_gpioouten(wlc_hw->sih, ledoe, ledoe, GPIO_DRV_PRIORITY);

	wlc_diag_led_toggle(wlc_hw, WLC_ACTIVITY_LED | WLC_RADIO_LEDS, OFF);

	wlc_diag_led_blink(wlc_hw, WLC_ACTIVITY_LED);

	if (NBANDS_HW(wlc_hw) == 2) {
		/* DELL request to blink individual lines first */
		wlc_diag_led_blink(wlc_hw, WLC_RADIO_LED_B);

		wlc_diag_led_blink(wlc_hw, WLC_RADIO_LED_A);
	}

	wlc_diag_led_blink(wlc_hw, WLC_RADIO_LEDS);

	wlc_diag_led_blink(wlc_hw, WLC_ACTIVITY_LED | WLC_RADIO_LEDS);

	wlc_diag_led_toggle(wlc_hw, WLC_ACTIVITY_LED | WLC_RADIO_LEDS, OFF);

	/* restore original led outen */
	si_gpioouten(wlc_hw->sih, orig_ledoe, orig_ledoe, GPIO_DRV_PRIORITY);

	return WL_DIAGERR_SUCCESS;
#else
	/* LED diag takes too long to finish and cause RPC with return call to fail.
	 * So bypass wlc_diag_led for now. Will require change to make "wl diag"
	 * request and result asynchronous later.
	 */
	 return WL_DIAGERR_SUCCESS;
#endif /* WLC_LOW_ONLY */
}

static uint32
wlc_diag_reg(wlc_hw_info_t *wlc_hw)
{
	if (!wlc_bmac_validate_chip_access(wlc_hw))
		return WL_DIAGERR_REG_FAIL;

	return WL_DIAGERR_SUCCESS;
}

/* Run bist on current core. core may have specific requirements and bist hazards */
static uint32
wlc_diag_runbist(wlc_info_t *wlc)
{
#ifndef WLC_LOW_ONLY
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t*)wlc->hw;
	uint32 status = WL_DIAGERR_SUCCESS;
	uint32 cflags, sflags;
	uint32 biststatus = 0, biststatus2 = 0;
	uint16 bw;

	/* only supports d11 for now */
	ASSERT(si_coreid(wlc_hw->sih) == D11_CORE_ID);

	if (!wlc_hw->sbclk)
		return status;

	/* pre processing */
	bw = wlc_phy_bw_state_get(wlc_hw->band->pi);
#ifdef WL11N
	if (WLCISNPHY(wlc_hw->band)) {
		/* BIST can be run only in 80Mhz(20Mhz channel), not 160Mhz/40Mhz phy clock
		 * save the current phy_bw, force it to 20Mhz channel, restore after finishing bist
		 */
		wlc_phy_bw_state_set(wlc_hw->band->pi, WL_CHANSPEC_BW_20);
		wlc_bmac_phy_reset(wlc_hw);
		wlc_phy_runbist_config(wlc_hw->band->pi, ON);
	}
#endif	/* WL11N */

	cflags = si_core_cflags(wlc_hw->sih, 0, 0);

	sflags = 0;

	/* run the bist */

	si_core_cflags(wlc_hw->sih, ~0, (cflags | SICF_BIST_EN | SICF_FGC | SICF_PCLKE));

	/* Wait for bist done */
	SPINWAIT((((sflags = si_core_sflags(wlc_hw->sih, 0, 0)) & SISF_BIST_DONE) == 0), 40000);

	if (!(sflags & SISF_BIST_DONE)) {
		status = WL_DIAGERR_MEMORY_TIMEOUT;
		goto done;
	}

#ifdef WL11N
	if (WLCISNPHY(wlc_hw->band) && NREV_LT(wlc_hw->band->phyrev, 2)) {
		/* check phy bist signature */
		if (!wlc_phy_bist_check_phy(wlc->band->pi))
			status = WL_DIAGERR_MEMORY_BADPATTERN;

		goto done;
	}
#endif /* WL11N */

	if (sflags & SISF_BIST_ERROR) {
		biststatus = R_REG(wlc_hw->osh, &wlc_hw->regs->biststatus);
		if (D11REV_GT(wlc_hw->corerev, 5))
			biststatus2 = R_REG(wlc_hw->osh, &wlc_hw->regs->biststatus2);
		WL_ERROR(("wl%d: wlc_diag_runbist, sflags 0x%x status 0x%x, status2 0x%x",
			wlc_hw->unit, sflags, biststatus, biststatus2));

		status = WL_DIAGERR_MEMORY_FAIL;
	}

done:
	si_core_cflags(wlc_hw->sih, ~0, cflags);
	wlc_hw->ucode_loaded = FALSE;

	/* post processing */
#ifdef WL11N
	if (WLCISNPHY(wlc_hw->band)) {
		wlc_phy_runbist_config(wlc_hw->band->pi, OFF);
		wlc_phy_bw_state_set(wlc_hw->band->pi, bw);
		wlc_bmac_phy_reset(wlc_hw);
	}
#endif /* WL11N */

	return WL_DIAGERR_SUCCESS;

#else
	/* after bist test, "wl up" will cause the host driver to hang. bypass this test for now. */
	return 0;
#endif /* WLC_LOW_ONLY */
}

static uint32
wlc_diag_lb_int(wlc_info_t *wlc, uint32 lb_int)
{
#define WAIT_COUNT_XI	100	/* wait count for tx interrupt */

	wlc_hw_info_t *wlc_hw = wlc->hw;
	d11regs_t *regs;
	osl_t *osh;
	void *p;
	void *rx_p;
	uint patterns[] = {
		0x00000000, 0xffffffff, 0x55555555, 0xaaaaaaaa,
		0x33333333, 0xcccccccc, 0x66666666, 0x99999999
	};
	uint patternlen;
	uint8 *data;
	uint8 *rx_data;
	uint32 intstatus;
	uint32 macintstatus;
	uint16 buf;
	int i;
	uint32 status = WL_DIAGERR_SUCCESS;

	ASSERT((lb_int == WL_DIAG_INTERRUPT) || (lb_int == WL_DIAG_LOOPBACK));

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	if (!PIO_ENAB(wlc->pub)) {
		/* init tx and rx dma channels */
		dma_txinit(wlc->hw->di[TX_DATA_FIFO]);
		dma_rxinit(wlc->hw->di[RX_FIFO]);

		/* post receive buffers */
		dma_rxfill(wlc->hw->di[RX_FIFO]);
	} else {
		/* init sw pio tx fifo depths */
		buf = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE0);

		if (wlc_pio_txdepthget(wlc->hw->pio[TX_DATA_FIFO]) == 0) {
			wlc_pio_txdepthset(wlc->hw->pio[TX_DATA_FIFO], (buf << 8));
			{
				uint tmp = wlc_pio_txdepthget(wlc->hw->pio[TX_DATA_FIFO]);
				if ((D11REV_LE(wlc_hw->corerev, 7)) && tmp)
					wlc_pio_txdepthset(wlc->hw->pio[TX_DATA_FIFO], tmp - 4);
			}
		}

		/* set FIFO mode for rx channel */
		wlc_rxfifo_setpio(wlc->hw);
	}

	/* enable fifo-level loopback
	 * FifoSel field of RXE RCV_CTL has been init'd to RX_FIFO at init
	 */
	if (!PIO_ENAB(wlc->pub))
		dma_fifoloopbackenable(wlc->hw->di[TX_DATA_FIFO]);
	else
		dma_txpioloopback(osh, &regs->fifo.f32regs.dmaregs[TX_DATA_FIFO].xmt);

	/* alloc tx packet */
	if ((p = PKTGET(osh, sizeof(patterns), TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: tx PKTGET error\n", wlc_hw->unit, __FUNCTION__));
		status = WL_DIAGERR_NOMEM;
		goto cleanup;
	}
	data = (uint8 *)PKTDATA(osh, p);
	ASSERT(ISALIGNED((uintptr)data, sizeof(uint32)));

	bcopy((uint8 *)patterns, data, sizeof(patterns));

	/* clear device interrupts */
	macintstatus = R_REG(osh, &regs->macintstatus);
	W_REG(osh, &regs->macintstatus, macintstatus);
	intstatus = R_REG(osh, &regs->intctrlregs[TX_DATA_FIFO].intstatus);
	W_REG(osh, &regs->intctrlregs[TX_DATA_FIFO].intstatus, intstatus);

	/* tx */
	wlc_bmac_txfifo(wlc_hw, TX_DATA_FIFO, p, TRUE, INVALIDFID, 1);

	/* expect tx interrupt */
	for (i = 0; i < WAIT_COUNT_XI; i++) {
		OSL_DELAY(10);	/* wait for fifo loopback to finish */
		intstatus = R_REG(osh, &regs->intctrlregs[TX_DATA_FIFO].intstatus);

		if (intstatus & (I_PC | I_PD | I_DE | I_RU | I_RO | I_XU)) {
			status = (lb_int == WL_DIAG_INTERRUPT) ? WL_DIAGERR_INTERRUPT_FAIL :
			    WL_DIAGERR_LOOPBACK_FAIL;

			macintstatus = R_REG(osh, &regs->macintstatus);
			WL_ERROR(("wl%d: %s: error int: macintstatus 0x%x intstatus[%d] 0x%x\n",
				wlc_hw->unit, __FUNCTION__, macintstatus, TX_DATA_FIFO,
				intstatus));
			goto cleanup;
		}
		if (intstatus & I_XI)
			break;
	}
	if (!(intstatus & I_XI)) {
		status = (lb_int == WL_DIAG_INTERRUPT) ? WL_DIAGERR_INTERRUPT_FAIL :
		    WL_DIAGERR_LOOPBACK_FAIL;
		macintstatus = R_REG(osh, &regs->macintstatus);
		WL_ERROR(("wl%d: %s: timeout waiting tx int; macintstatus 0x%x intstatus[%d]"
			" 0x%x\n",
			wlc_hw->unit, __FUNCTION__, macintstatus, TX_DATA_FIFO, intstatus));
		goto cleanup;
	}

	/* rx */
	if (PIO_ENAB(wlc->pub)) {
		SPINWAIT((rx_p = wlc_pio_rx(wlc->hw->pio[RX_FIFO])) == NULL, 5000);
	} else {
		SPINWAIT((rx_p = dma_rx(wlc->hw->di[RX_FIFO])) == NULL, 5000);
	}

	/* no packet received */
	if (rx_p == NULL) {
		status = (lb_int == WL_DIAG_INTERRUPT) ? WL_DIAGERR_INTERRUPT_FAIL :
		    WL_DIAGERR_LOOPBACK_FAIL;
		WL_ERROR(("wl%d: %s: no packet looped back\n", wlc_hw->unit, __FUNCTION__));
		goto cleanup;
	}

	/* done if choice is interrupt diag
	 * Since rx interrupt is not asserted for fifo loopback, the surest way
	 * to ensure flushing is to have dma rx done.
	 */
	if (lb_int == WL_DIAG_INTERRUPT) {
		status = WL_DIAGERR_SUCCESS;
	} else {
		/* compare */
		rx_data = (uint8 *)PKTDATA(wlc->osh, rx_p) + wlc->hwrxoff;
		if (PIO_ENAB(wlc->pub))
			rx_data -= 4;
		patternlen = sizeof(patterns);
		if (bcmp(data, rx_data, patternlen) == 0)
			status = WL_DIAGERR_SUCCESS;
		else {
			status = WL_DIAGERR_LOOPBACK_FAIL;
			WL_ERROR(("wl%d: %s: data miscompare\n", wlc_hw->unit, __FUNCTION__));
		}
	}

cleanup:
	if (!PIO_ENAB(wlc->pub)) {
		/* reset tx and rx dma channels; also disable fifo loopback */
		dma_txreset(wlc->hw->di[TX_DATA_FIFO]);
		dma_rxreset(wlc->hw->di[RX_FIFO]);

		/* free posted tx and rx packets */
		dma_txreclaim(wlc->hw->di[TX_DATA_FIFO], HNDDMA_RANGE_ALL);
		dma_rxreclaim(wlc->hw->di[RX_FIFO]);
	} else {
		wlc_pio_reset(wlc->hw->pio[TX_DATA_FIFO]);
		wlc_pio_reset(wlc->hw->pio[RX_FIFO]);
	}

	return status;
}

/*
 * diagnostics entry:
 *   return code only means if the test is conducted
 *   *result is the detailed test result, non-zero means failed
 */
int
wlc_diag(wlc_info_t *wlc, uint32 diagtype, uint32 *result)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;

	*result = WL_DIAGERR_FAIL_TO_RUN;

	if (wlc_hw->up)
		return BCME_NOTDOWN;

	if (!wlc_hw->clk)
		return BCME_NOCLK;

	/* initialize result to success, each testcase will override it for fail reason */
	*result = 0;

	switch (diagtype) {
	case WL_DIAG_MEMORY:
		*result = wlc_diag_runbist(wlc);
		break;
	case WL_DIAG_LED:
		*result = wlc_diag_led(wlc_hw);
		break;

	case WL_DIAG_REG:
		*result = wlc_diag_reg(wlc_hw);
		break;

	case WL_DIAG_SROM:
		*result = wlc_diag_srom(wlc_hw);
		break;

	case WL_DIAG_INTERRUPT:
	case WL_DIAG_LOOPBACK:
		*result = wlc_diag_lb_int(wlc, diagtype);
		break;

	default:
		*result = WL_DIAGERR_NOT_SUPPORTED; /* request diag type is not supported */
		break;
	} /* end of switch (diagtype) */

	return 0;
}
