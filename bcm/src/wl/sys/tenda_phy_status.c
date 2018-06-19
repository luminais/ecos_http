
/*****************************************************************************
Copyright (C), 吉祥腾达，保留所有版权
File name ： tenda_phy_interference.c
Description : 接收灵敏度自动调节,提高抗干扰能力
Author ：jack deng
Version ：V1.0
Date ：2015-6-20
Others ：
History ：
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期： 修改者：
   内容：
2）...
 
*****************************************************************************/
#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <bcmutils.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_scb.h>

#include <wlc_rm.h>
#include "wlc_cac.h"
#include <wlc_ap.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phyreg_abg.h>
#include <wlc_phyreg_n.h>
#include <wlc_phyreg_ht.h>
#include <wlc_phyreg_lp.h>
#include <wlc_phyreg_ssn.h>
#include <wlc_phyreg_lcn.h>
#include <wlc_phyreg_lcn40.h>
#include <wlc_phytbl_n.h>
#include <wlc_phytbl_ht.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_lcn.h>
#include <wlc_phy_lcn40.h>
#include <wlc_phy_lp.h>
#include <wlc_phy_ssn.h>
#include <wlc_phy_abg.h>
#include <wlc_phy_n.h>
#include <wlc_phy_ht.h>

#include <bcmotp.h>

#ifdef TENDA_PHY_STATUS
/*******************************************************************
Function      : td_interference_cal_ma()
Description   : 
Input         :
Output        :
Return        :
Others        :
*******************************************************************/
static void td_phy_status_cal_ma(struct td_phy_status *in)
{
    int cur_index,last_index;

    cur_index = in->list_cur_index;
    last_index = in->list_last_index;

    in->ofdm_glitch_ma  += in->glitch_list[cur_index];
    in->ofdm_badplcp_ma += in->badplcp_list[cur_index];
    in->bphy_badplcp_ma  += in->bphy_glitch_list[cur_index];
    in->bphy_glitch_ma += in->bphy_badplcp_list[cur_index];
    in->rxbadfcs_ma += in->rxbadfcs_list[cur_index];

    in->ofdm_glitch_ma  -= in->glitch_list[last_index];
    in->ofdm_badplcp_ma -= in->badplcp_list[last_index];
    in->bphy_badplcp_ma  -= in->bphy_glitch_list[last_index];
    in->bphy_glitch_ma -= in->bphy_badplcp_list[last_index];
    in->rxbadfcs_ma -= in->rxbadfcs_list[last_index];
}

/*******************************************************************
Function      : td_interference_info_update()
Description   :
Input         :
Output        :
Return        :
Others        :
*******************************************************************/

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;

void td_phy_status_update(void *td_pi)
{
    phy_info_t *pi = (phy_info_t *)td_pi;
    struct td_phy_status *in = &(pi->td_status);
    u32 offset;
    u16 glitch_delta, badplcp_delta, bphy_glitch_delta, bphy_badplcp_delta, rxbadfcs_delta;
    u16 cur_counters;
#ifdef TENDA_PHY_INTERFERENCE
    struct td_interf_cfg *cfg = &(pi->td_cfg);
#endif
    in->list_cur_index = in->list_cur_index%TD_PHY_INTERF_MA_WZ;
    in->list_last_index = in->list_last_index%TD_PHY_INTERF_MA_WZ;

    /* read rx crs glitches */
    offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, rxcrsglitch);
    cur_counters = wlapi_bmac_read_shm(pi->sh->physhim, offset);
    glitch_delta = (u16)(cur_counters - in->pre_glitch_cnt);
    in->pre_glitch_cnt = cur_counters;

    /* read rxbadplcp  */
    offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, rxbadplcp);
    cur_counters = wlapi_bmac_read_shm(pi->sh->physhim, offset);
    badplcp_delta = (u16)(cur_counters - in->pre_badplcp_cnt);
    in->pre_badplcp_cnt = cur_counters;

    /* read bphy rx crs glitches */
    offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, bphy_rxcrsglitch);
    cur_counters = wlapi_bmac_read_shm(pi->sh->physhim, offset);
    bphy_glitch_delta = (u16)(cur_counters - in->pre_bphy_glitch_cnt);
    in->pre_bphy_glitch_cnt = cur_counters;

    /* read bphy rxbadplcp */
    offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, bphy_badplcp);
    cur_counters = wlapi_bmac_read_shm(pi->sh->physhim, offset);
    bphy_badplcp_delta = (u16)(cur_counters - in->pre_bphy_badplcp_cnt);
    in->pre_bphy_badplcp_cnt = cur_counters;

    /* read rxbadfcs */
    offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, rxbadfcs);
    cur_counters = wlapi_bmac_read_shm(pi->sh->physhim, offset);
    rxbadfcs_delta = (u16)(cur_counters - in->pre_rxbadfcs_cnt);
    in->pre_rxbadfcs_cnt = cur_counters;

    /* calculate info */
#ifdef TENDA_PHY_INTERFERENCE
    if (cfg->bphy_adjust_status == TD_BPHY_STATUS_FOLLOW) {
        in->bphy_badplcp_list[in->list_cur_index] = bphy_badplcp_delta;
        in->bphy_glitch_list[in->list_cur_index] = bphy_glitch_delta;
    } else {
        if (bphy_badplcp_delta > cfg->bphy_badplcp_th) {
            in->bphy_badplcp_list[in->list_cur_index] = cfg->bphy_badplcp_th;
        }
        if (bphy_glitch_delta > cfg->bphy_glitch_th) {
            in->bphy_glitch_list[in->list_cur_index] = cfg->bphy_glitch_th;
        }
    }
#else
    in->bphy_badplcp_list[in->list_cur_index] = bphy_badplcp_delta;
    in->bphy_glitch_list[in->list_cur_index] = bphy_glitch_delta;
#endif
    in->badplcp_list[in->list_cur_index] = badplcp_delta - bphy_badplcp_delta;
    in->glitch_list[in->list_cur_index] = glitch_delta - bphy_glitch_delta;
    in->rxbadfcs_list[in->list_cur_index] = rxbadfcs_delta;

    td_phy_status_cal_ma(in);
    in->list_cur_index++;
    in->list_last_index++;

#ifdef TENDA_AUTO_WIDTH
    td_auto_width_eval_itfr(pi->td_status.wlc,
        in->bphy_badplcp_ma + in->bphy_glitch_ma + in->ofdm_badplcp_ma + in->ofdm_glitch_ma);
#endif
}

void td_phy_status_init(void *td_wlc)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;
    phy_info_t *pi = (phy_info_t *)(wlc->band->pi);
    struct td_phy_status *in = &(pi->td_status);

    printf("--%s--\n",__func__);
    memset(in,0,sizeof(struct td_phy_status));
    in->list_cur_index = 0;
    in->list_last_index = TD_PHY_INTERF_MA_WZ - 1;
    in->wlc = wlc;
}
#endif
