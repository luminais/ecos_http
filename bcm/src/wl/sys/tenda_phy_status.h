#ifndef __TENDA_PHY_STATUS_H__
#define __TENDA_PHY_STATUS_H__

#if defined(WLC_HIGH) && !defined(WLC_LOW)
#undef TENDA_PHY_STATUS
#endif

#define TD_PHY_INTERF_MA_WZ     8   /* 统计采集窗口大小 */

struct td_phy_status {
    unsigned short pre_bphy_badplcp_cnt;
    unsigned short pre_bphy_glitch_cnt;
    unsigned short pre_badplcp_cnt;
    unsigned short pre_glitch_cnt;
    unsigned short pre_rxbadfcs_cnt;

    unsigned short bphy_badplcp_ma;
    unsigned short bphy_glitch_ma;
    unsigned short ofdm_badplcp_ma;
    unsigned short ofdm_glitch_ma;
    unsigned short rxbadfcs_ma;

    unsigned short bphy_badplcp_list[TD_PHY_INTERF_MA_WZ];
    unsigned short bphy_glitch_list[TD_PHY_INTERF_MA_WZ];
    unsigned short badplcp_list[TD_PHY_INTERF_MA_WZ];   //OFDM
    unsigned short glitch_list[TD_PHY_INTERF_MA_WZ];    //OFDM
    unsigned short rxbadfcs_list[TD_PHY_INTERF_MA_WZ];

    //unsigned char inferf_ma_index;
    unsigned char list_last_index;
    unsigned char list_cur_index;

    void *wlc;
};

void td_phy_status_update(void *td_pi);
void td_phy_status_init(void *td_wlc);
#endif
