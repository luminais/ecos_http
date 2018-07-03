//==========================================================================
//
//      tests/set_mac_address.c
//
//      Set_Mac_Address Utility - this carefully does NOTHING unless you
//      edit this source file to confirm that you really want to do it.
//
//==========================================================================
// ####BSDALTCOPYRIGHTBEGIN####                                             
// -------------------------------------------                              
// Portions of this software may have been derived from FreeBSD, OpenBSD,   
// or other sources, and if so are covered by the appropriate copyright     
// and license included herein.                                             
// -------------------------------------------                              
// ####BSDALTCOPYRIGHTEND####                                               
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: 
// Date:         2000-05-03
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#ifdef CYGBLD_DEVS_ETH_DEVICE_H    // Get the device config if it exists
#include CYGBLD_DEVS_ETH_DEVICE_H
#endif

#include <network.h>

#include <netinet/if_ether.h>
#include <stdio.h> 
// ------------------------------------------------------------------------

int set_mac_address( const char *interface, char *mac_address )
{
    int s, i;
    struct ifreq ifr;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 0;
    }

    //printf( "%s socket is %d:\n", interface, s );

    strcpy(ifr.ifr_name, interface);

    for ( i = 0; i < ETHER_ADDR_LEN; i++ )
        ifr.ifr_hwaddr.sa_data[i] = mac_address[i];

    /*printf( "Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",
                 ifr.ifr_hwaddr.sa_data[0],
                 ifr.ifr_hwaddr.sa_data[1],
                 ifr.ifr_hwaddr.sa_data[2],
                 ifr.ifr_hwaddr.sa_data[3],
                 ifr.ifr_hwaddr.sa_data[4],
                 ifr.ifr_hwaddr.sa_data[5] );
    */
    if (ioctl(s, SIOCSIFHWADDR, &ifr)) {
        perror("SIOCSIFHWADDR");
        close( s );
        return 0;
    }

    //printf( "%s ioctl(SIOCSIFHWADDR) succeeded\n", interface );

    close( s );

    return 1;
}

// ------------------------------------------------------------------------

int get_mac_address( const char *interface, char *mac_address )
{
    int s, i;
    struct ifreq ifr;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 0;
    }

    //printf( "%s socket is %d:\n", interface, s );

    strcpy(ifr.ifr_name, interface);

    if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
        perror("SIOCGIFHWADDR");
        close( s );
        return 0;
    }

    //printf( "%s ioctl(SIOCGIFHWADDR) succeeded\n", interface );

    close( s );
    
    for ( i = 0; i < ETHER_ADDR_LEN; i++ )
        mac_address[i] = ifr.ifr_hwaddr.sa_data[i];

    /*printf( "Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",
                 mac_address[0],
                 mac_address[1],
                 mac_address[2],
                 mac_address[3],
                 mac_address[4],
                 mac_address[5] );
    */

    return 1;
}
