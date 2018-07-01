//==========================================================================
//
//      devs/eth/rltk/819x/wlan/include/devs_eth_rltk_819x_wlan.inl
//
//      RealTek 819x wlan I/O definitions.
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   David Hsu
// Contributors:
// Date:        2010-4-2
// Purpose:     RealTek 819x wlan definitions
//####DESCRIPTIONEND####
//==========================================================================

// 
// CAUTION! This driver has *not* been tested on PC hardware.  It may work
// or not :-)  If there are problems, they are probably cache related.
// If you find such, please let us know.
//

#define CACHE_ALIGNED __attribute__ ((aligned (HAL_DCACHE_LINE_SIZE)))

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
static Rltk819x_t rltk819x_wlan0_wds0_priv_data = {
  0, 0
};
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 2
static Rltk819x_t rltk819x_wlan0_wds1_priv_data = {
  1, 0
};
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 4
static Rltk819x_t rltk819x_wlan0_wds2_priv_data = {
  2, 0
};
static Rltk819x_t rltk819x_wlan0_wds3_priv_data = {
  3, 0
};
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 8
static Rltk819x_t rltk819x_wlan0_wds4_priv_data = {
  4, 0
};
static Rltk819x_t rltk819x_wlan0_wds5_priv_data = {
  5, 0
};
static Rltk819x_t rltk819x_wlan0_wds6_priv_data = {
  6, 0
};
static Rltk819x_t rltk819x_wlan0_wds7_priv_data = {
  7, 0
};
#endif
#endif
#endif

ETH_DRV_SC(rltk819x_wlan0_wds_sc0,
           &rltk819x_wlan0_wds0_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds0",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 2
ETH_DRV_SC(rltk819x_wlan0_wds_sc1,
           &rltk819x_wlan0_wds1_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds1",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 4
ETH_DRV_SC(rltk819x_wlan0_wds_sc2,
           &rltk819x_wlan0_wds2_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds2",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
ETH_DRV_SC(rltk819x_wlan0_wds_sc3,
           &rltk819x_wlan0_wds3_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds3",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 8
ETH_DRV_SC(rltk819x_wlan0_wds_sc4,
           &rltk819x_wlan0_wds4_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds4",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
ETH_DRV_SC(rltk819x_wlan0_wds_sc5,
           &rltk819x_wlan0_wds5_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds5",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
ETH_DRV_SC(rltk819x_wlan0_wds_sc6,
           &rltk819x_wlan0_wds6_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds6",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
ETH_DRV_SC(rltk819x_wlan0_wds_sc7,
           &rltk819x_wlan0_wds7_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds7",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#endif
#endif
#endif
#endif // RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS


#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
static Rltk819x_t rltk819x_wlan0_vap0_priv_data = {
  0, 0
};
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
static Rltk819x_t rltk819x_wlan0_vap1_priv_data = {
  1, 0
};
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
static Rltk819x_t rltk819x_wlan0_vap2_priv_data = {
  2, 0
};
static Rltk819x_t rltk819x_wlan0_vap3_priv_data = {
  3, 0
};
#endif
#endif

ETH_DRV_SC(rltk819x_wlan0_vap_sc0,
           &rltk819x_wlan0_vap0_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va0",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
ETH_DRV_SC(rltk819x_wlan0_vap_sc1,
           &rltk819x_wlan0_vap1_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va1",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
ETH_DRV_SC(rltk819x_wlan0_vap_sc2,
           &rltk819x_wlan0_vap2_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va2",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
ETH_DRV_SC(rltk819x_wlan0_vap_sc3,
           &rltk819x_wlan0_vap3_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va3",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#endif
#endif
#endif // RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID


#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_REPEATER_MODE
static Rltk819x_t rltk819x_wlan0_vxd_priv_data = {
  8, 0
};

ETH_DRV_SC(rltk819x_wlan0_vxd_sc0,
           &rltk819x_wlan0_vxd_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-vxd",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#endif 

static Rltk819x_t rltk819x_wlan0_priv_data = {
  0, 0
};

ETH_DRV_SC(rltk819x_wlan_sc0,
           &rltk819x_wlan0_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME,
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );
#if !defined(CONFIG_RTL_8197F)

NETDEVTAB_ENTRY(rltk819x_wlan0_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME,
                rltk819x_wlan_init,
                &rltk819x_wlan_sc0);
#endif
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_PWLAN0
static Rltk819x_t rltk819x_pwlan0_priv_data = {
  9, 0
};

ETH_DRV_SC(rltk819x_wlan_sc9,
           &rltk819x_pwlan0_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_WLAN_PWLAN0_NAME,
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );

NETDEVTAB_ENTRY(rltk819x_wlan0_pnetdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_PWLAN0_NAME,
                rltk819x_wlan_pwlan_init,
                &rltk819x_wlan_sc9);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MESH_SUPPORT
static Rltk819x_t rltk819x_wlan0_mesh_priv_data = {
  10, 0
};

ETH_DRV_SC(rltk819x_wlan_mesh_sc10,
           &rltk819x_wlan0_mesh_priv_data,
           "wlan" "-msh",
           rltk819x_wlan_start,
           rltk819x_wlan_stop,
           rltk819x_wlan_control,
           rltk819x_wlan_can_send,
           rltk819x_wlan_send,
           rltk819x_wlan_recv,
           rltk819x_wlan_deliver,
           rltk819x_wlan_poll,
           rltk819x_wlan_int_vector
           );

NETDEVTAB_ENTRY(rltk819x_wlan0_mesh_netdev0,
                "rltk819x_" "wlan" "-msh",
                rltk819x_wlan_mesh_init,
                &rltk819x_wlan_mesh_sc10);
#endif

#if (__GNUC__ >= 4)
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds0",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc0);
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 2
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev1,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds1",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc1);
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 4
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev2,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds2",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc2);
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev3,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds3",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc3);
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM >= 8
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev4,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds4",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc4);
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev5,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds5",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc5);
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev6,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds6",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc6);
NETDEVTAB_ENTRY(rltk819x_wlan0_wds_netdev7,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-wds7",
                rltk819x_wlan_wds_init,
                &rltk819x_wlan0_wds_sc7);
#endif
#endif
#endif
#endif // RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
NETDEVTAB_ENTRY(rltk819x_wlan0_vap_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va0",
                rltk819x_wlan_vap_init,
                &rltk819x_wlan0_vap_sc0);
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
NETDEVTAB_ENTRY(rltk819x_wlan0_vap_netdev1,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va1",
                rltk819x_wlan_vap_init,
                &rltk819x_wlan0_vap_sc1);
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
NETDEVTAB_ENTRY(rltk819x_wlan0_vap_netdev2,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va2",
                rltk819x_wlan_vap_init,
                &rltk819x_wlan0_vap_sc2);
NETDEVTAB_ENTRY(rltk819x_wlan0_vap_netdev3,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-va3",
                rltk819x_wlan_vap_init,
                &rltk819x_wlan0_vap_sc3);
#endif
#endif
#endif // RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_REPEATER_MODE
NETDEVTAB_ENTRY(rltk819x_wlan0_vxd_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME "-vxd",
                rltk819x_wlan_vxd_init,
                &rltk819x_wlan0_vxd_sc0);

#endif
#endif

#if defined(CONFIG_RTL_8197F)
NETDEVTAB_ENTRY(rltk819x_wlan0_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_WLAN_WLAN0_NAME,
                rltk819x_wlan_init,
                &rltk819x_wlan_sc0);

#endif
#endif // RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
// EOF devs_eth_rltk_819x_wlan.inl
