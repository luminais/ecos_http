//==========================================================================
//
//      devs/eth/rltk/819x/switch/include/devs_eth_rltk_819x_switch.inl
//
//      RealTek 819x ethernet I/O definitions.
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
// Date:        2009-11-13
// Purpose:     RealTek 819x ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

// 
// CAUTION! This driver has *not* been tested on PC hardware.  It may work
// or not :-)  If there are problems, they are probably cache related.
// If you find such, please let us know.
//

#define CACHE_ALIGNED __attribute__ ((aligned (HAL_DCACHE_LINE_SIZE)))

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH0
static Rltk819x_t rltk819x_eth0_priv_data = {
  0, 0
};

ETH_DRV_SC(rltk819x_sc0,
           &rltk819x_eth0_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH0_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif // RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH0

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1
static Rltk819x_t rltk819x_eth1_priv_data = {
  1, 0
};

ETH_DRV_SC(rltk819x_sc1,
           &rltk819x_eth1_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH1_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif // RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
static Rltk819x_t rltk819x_eth2_priv_data = {
  2, 0
};

ETH_DRV_SC(rltk819x_sc2,
           &rltk819x_eth2_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH2_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif // RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH3
static Rltk819x_t rltk819x_eth3_priv_data = {
  3, 0
};

ETH_DRV_SC(rltk819x_sc3,
           &rltk819x_eth3_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH3_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif // RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH4
static Rltk819x_t rltk819x_eth4_priv_data = {
  4, 0
};

ETH_DRV_SC(rltk819x_sc4,
           &rltk819x_eth4_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH4_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif // RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_PETH0
static Rltk819x_t rltk819x_eth5_priv_data = {
  5, 0
};

ETH_DRV_SC(rltk819x_sc5,
           &rltk819x_eth5_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_PETH0_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH7
static Rltk819x_t rltk819x_eth6_priv_data = {
  6, 0
};

ETH_DRV_SC(rltk819x_sc6,
           &rltk819x_eth6_priv_data,
           RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH7_NAME,
           rltk819x_start,
           rltk819x_stop,
           rltk819x_control,
           rltk819x_can_send,
           rltk819x_send,
           rltk819x_recv,
           rltk819x_deliver,
           rltk819x_poll,
           rltk819x_int_vector
           );
#endif // RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH7


#if (__GNUC__ >= 4)
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH0
NETDEVTAB_ENTRY(rltk819x_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH0_NAME,
                rltk819x_init,
                &rltk819x_sc0);
#endif
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH1
NETDEVTAB_ENTRY(rltk819x_netdev1,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH1_NAME,
                rltk819x_init,
                &rltk819x_sc1);
#endif

#if (__GNUC__ < 4)
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH0
NETDEVTAB_ENTRY(rltk819x_netdev0,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH0_NAME,
                rltk819x_init,
                &rltk819x_sc0);
#endif
#endif


#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
NETDEVTAB_ENTRY(rltk819x_netdev2,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH2_NAME,
                rltk819x_init,
                &rltk819x_sc2);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH3
NETDEVTAB_ENTRY(rltk819x_netdev3,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH3_NAME,
                rltk819x_init,
                &rltk819x_sc3);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH4
NETDEVTAB_ENTRY(rltk819x_netdev4,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH4_NAME,
                rltk819x_init,
                &rltk819x_sc4);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_PETH0
NETDEVTAB_ENTRY(rltk819x_netdev5,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_PETH0_NAME,
                rltk819x_peth0_init,
                &rltk819x_sc5);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH7
NETDEVTAB_ENTRY(rltk819x_netdev6,
                "rltk819x_" RTLDAT_DEVS_ETH_RLTK_819X_SWITCH_ETH7_NAME,
                rltk819x_init,
                &rltk819x_sc6);
#endif
// EOF devs_eth_rltk_819x_switch.inl
