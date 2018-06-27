/*==========================================================================
//
//      dhcp_prot.c
//
//      DHCP protocol implementation for DHCP client
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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
// Author(s):   hmt
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:        2000-07-01
// Purpose:     DHCP support
// Description: 
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
//#include <pkgconf/net.h>

#ifdef CYGPKG_NET_DHCP

#ifdef CYGPKG_NET_SNTP
#include <pkgconf/net_sntp.h>
#endif /* CYGPKG_NET_SNTP */

#if 0
#define perror( txt ) // nothing
#endif

#include <network.h>
#include "dhcp.h"
#include <errno.h>

#include <cyg/infra/cyg_ass.h>

#ifdef INET6
#include <net/if_var.h>
#include <netinet6/in6_var.h>
#endif
#include <net/if_var.h>
#include <net/if_arp.h>

#ifdef CHECK_RECV_INTERFACE_INDEX
#include <net/if_dl.h>
#include <sys/uio.h>
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "../system/sys_utility.h"
#include "../system/sys_init.h"
#include "../apmib/apmib.h"
#include <sys/stat.h>

#ifdef ECOS_DBG_STAT
extern int dbg_dhcpc_index;
#endif

#ifdef CHECK_RECV_INTERFACE_INDEX
#define _ALIGN(n) (((n)+3)&~3)  // Is this right?
#endif

//#define ETH_P_IP        0x0800 

static char wandhcpc_running=0;    // Boolean (running loop)
static char wandhcpc_started=0;
static cyg_uint8 wandhcpc_quitting=0;
static cyg_uint8 landhcpc_quitting=0;


static cyg_uint8  wandhcpc_stack[DHCPC_THREAD_STACK_SIZE];
static cyg_handle_t  wandhcpc_thread;
static cyg_thread  wandhcpc_thread_object;	

static cyg_uint8  landhcpc_stack[DHCPC_THREAD_STACK_SIZE];
cyg_handle_t  landhcpc_thread;
cyg_thread	landhcpc_thread_object;

char landhcpc_running=0;    // Boolean (running loop)
char landhcpc_started=0;

static char wan_ifname[32];
struct bootp wan_bootp_data;
cyg_uint8 wan_dhcpstate = 0;
cyg_sem_t wandhcp_needs_attention;
struct dhcp_lease wan_lease = { &wandhcp_needs_attention, 0,0,0,0,0,0 };

static char lan_ifname[32];
struct bootp lan_bootp_data;
cyg_uint8 lan_dhcpstate = 0;
cyg_sem_t landhcp_needs_attention;
struct dhcp_lease lan_lease = { &landhcp_needs_attention, 0,0,0,0,0,0 };

static cyg_uint8 recon_flag=0;
//static cyg_uint8 dhcpc_func_off=0;
//static cyg_uint8 dhcpc_cleaned=0;

static cyg_uint8 wan_status_flag=0;

//static struct buffer sndbuf, rcvbuf;
//static struct msg sndmsg, rcvmsg;

#define CYGNUM_NET_DHCP_MIN_RETRY_TIME    6 //10

//for dlink webpage release/renew function
#define DHCP_RENEW_FROM_WEBPAG 100
#define DHCP_RELEASE_FROM_WEBPAG 50

#define DHCP_WAN_STATUS_UP   2
#define DHCP_WAN_STATUS_DOWN  1

extern void get_lan_ip_addr(struct in_addr *addr, struct in_addr *mask);

extern int  cyg_arc4random(void);

#ifdef CYGOPT_NET_DHCP_OPTION_HOST_NAME

static char dhcp_hostname[CYGNUM_NET_DHCP_OPTION_HOST_NAME_LEN+1];

// Set the hostname used by the DHCP TAG_HOST_NAME option.  We
// copy the callers name into a private buffer, since we don't
// know the context in which the callers hostname was allocated.
void dhcp_set_hostname(char *hostname)
{
    CYG_ASSERT( (strlen(hostname)<=CYGNUM_NET_DHCP_OPTION_HOST_NAME_LEN), "dhcp hostname too long" );
    strncpy(dhcp_hostname, hostname, CYGNUM_NET_DHCP_OPTION_HOST_NAME_LEN);
}
#endif

/* Forward reference prototypes. */
static int unset_tag( struct bootp *ppkt, unsigned char tag );

// ------------------------------------------------------------------------
// Returns a pointer to the end of dhcp message (or NULL if invalid)
// meaning the address of the byte *after* the TAG_END token in the vendor
// data.

static unsigned char *
scan_dhcp_size( struct bootp *ppkt )
{
    unsigned char *op;
    
    op = &ppkt->bp_vend[0];
    // First check for the cookie!
    if ( op[0] !=  99 ||
         op[1] != 130 ||
         op[2] !=  83 ||
         op[3] !=  99 ) {
        CYG_FAIL( "Bad DHCP cookie" );
        return NULL;
    }
    op += 4;

    // This will only scan the options field.
    while (*op != TAG_END) {
        if ( *op == TAG_PAD ) {
            op++;
        } else {
          op += *(op+1)+2;
        }
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in dhcp_size" );
            return NULL;
        }
    }
    // Check op has not gone wild
    CYG_ASSERT( op > (unsigned char *)(&ppkt[0]), "op pointer underflow!" );
    // Compare op with non-existent "next" struct bootp in the array.
    CYG_ASSERT( op < (unsigned char *)(&ppkt[1]), "op pointer overflow!" );
    return op + 1; // Address of first invalid byte
}
#ifdef CONFIG_RTL_819X
static unsigned char *rtl_exclude_pad(struct bootp *src, struct bootp *dst)
{
    unsigned char *op;

   //#ifdef CONFIG_RTL_819X
   static cyg_uint8 *rtl_receive_pkt=NULL;
   static unsigned int rtl_receive_pkt_len=0;

   unsigned char *rtl_op=NULL, *rtl_start=NULL, *rtl_end=NULL, *tmp_first=NULL;
   unsigned char first_pad=0, end_pad=0;
   unsigned int rtl_offset=0;
   rtl_start=(unsigned char *)src;
   //#endif
    
    op = &src->bp_vend[0];
    // First check for the cookie!
    if ( op[0] !=  99 ||
         op[1] != 130 ||
         op[2] !=  83 ||
         op[3] !=  99 ) {
        CYG_FAIL( "Bad DHCP cookie" );
        return NULL;
    }
    op += 4;

    // This will only scan the options field.
    while (*op != TAG_END) {
        if ( *op == TAG_PAD ) { 
		 		
	//#ifdef CONFIG_RTL_819X
	if(first_pad==0)
	{
		tmp_first=op;
		first_pad=1;
		rtl_op=op;
		rtl_offset=rtl_op-rtl_start;
		rtl_receive_pkt_len+=rtl_offset;
		
		rtl_receive_pkt=(unsigned char*)malloc(sizeof(unsigned char)*512);
		if(rtl_receive_pkt==NULL)
		{
			diag_printf("\n%s:%d malloc fail!\n",__FUNCTION__,__LINE__);
			return NULL;
		}
		memset(rtl_receive_pkt, 0, sizeof(unsigned char)*512);
		memcpy(rtl_receive_pkt, rtl_start, rtl_offset);
	}
	//#endif		

	op++;
	
        } else 
        {
	   //#ifdef CONFIG_RTL_819X
          if(first_pad==1 && end_pad==0)
          {
		 end_pad=1;
		 rtl_end=op;
		 
		 //diag_printf("\n%s:%d  pad_size=%d\n", __FUNCTION__,__LINE__,rtl_end-tmp_first);
	   }
	   //#endif
	   
          op += *(op+1)+2;
        }
        if ( op > &src->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in dhcp_size" );

	    //#ifdef CONFIG_RTL_819X
	   if(rtl_receive_pkt)
	   {
		   free(rtl_receive_pkt);
		   rtl_receive_pkt=NULL;
	   }
	    //#endif
		
            return NULL;
        }
    }

	  //#ifdef CONFIG_RTL_819X
          if(*op == TAG_END && first_pad==1 && end_pad==0)
          {
		 end_pad=1;
		 rtl_end=op;
		 
		 //diag_printf("\n%s:%d  pad_size=%d\n", __FUNCTION__,__LINE__,rtl_end-tmp_first);
	   }
	   //#endif

    // Check op has not gone wild
    CYG_ASSERT( op > (unsigned char *)(&src[0]), "op pointer underflow!" );
    // Compare op with non-existent "next" struct bootp in the array.
    CYG_ASSERT( op < (unsigned char *)(&src[1]), "op pointer overflow!" );
	
//#ifdef CONFIG_RTL_819X
	if(first_pad==1 && end_pad==1)
	{
		rtl_op=op+1;
		
	       memcpy(rtl_receive_pkt+rtl_offset, rtl_end, rtl_op-rtl_end);	
		 rtl_offset=rtl_op-rtl_end;
		rtl_receive_pkt_len+=rtl_offset;

		memcpy(dst, rtl_receive_pkt, rtl_receive_pkt_len);		

		//diag_printf("\n%s:%d rtl_receive_pkt_len=%d\n", __FUNCTION__,__LINE__,rtl_receive_pkt_len);
	}
	else
	{
		memcpy(dst, rtl_start, op + 1-rtl_start);
	}

	if(rtl_receive_pkt)
	{
		free(rtl_receive_pkt);
		rtl_receive_pkt=NULL;
	}
//#endif

    return op + 1; // Address of first invalid byte
}
#endif

// ------------------------------------------------------------------------
// Get the actual packet size of an initialized buffer

static int
dhcp_size( struct bootp *ppkt )
{
    unsigned char *op;

    op = scan_dhcp_size( ppkt );
    if ( !op ) return 0;
    return (op - (unsigned char *)ppkt);
}


// ------------------------------------------------------------------------
// Get the actual packet size of an initialized buffer
// This will also pad the packet with 0 if length is less
// than BP_STD_TX_MINPKTSZ.

static int
dhcp_size_for_send( struct bootp *ppkt )
{
    unsigned char *op;

    op = scan_dhcp_size( ppkt );
    if ( !op ) return 0; // Better not scribble!
    // Zero extra bytes until the packet is large enough.
    for ( ; op < (((unsigned char *)ppkt) + BP_STD_TX_MINPKTSZ); op++ )
        *op = 0;
    return (op - (unsigned char *)ppkt);
}

// ------------------------------------------------------------------------
// Insert/set an option value in an initialized buffer

static int
set_fixed_tag( struct bootp *ppkt,
               unsigned char tag,
               cyg_uint32 value,
               int len)
{
    unsigned char *op;

    // Initially this will only scan the options field.
    
    op = &ppkt->bp_vend[4];
    while (*op != TAG_END) {
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_fixed_tag" );
            return false;
        }
        if (*op == tag)                 // Found it...
            break;
        if ( *op == TAG_PAD ) {
            op++;
        } else {
          op += *(op+1)+2;
        }
    }
    
    if (*op == tag) { // Found it...
        /* There are three possibilities:
         * 1) *(op+1) == len
         * 2) *(op+1) > len
         * 3) *(op+1) < len
         * For 1, just overwrite the existing option data.
         * For 2, overwrite the existing option data and pullup the
         *        remaining option data (if any).
         * For 3, pullup any remaining option data to remove the option
         *        and then add the option to the end.
         * For simplicity, for case 2 and 3, we just call unset_tag()
         * and re-add the option to the end.
         */
        if ( *(op+1) != len ) {
            /* Remove existing option entry. */
            unset_tag(ppkt, tag);
            /* Adjust the op pointer to re-add at the end. */
            op = scan_dhcp_size(ppkt);
            CYG_ASSERT(op!=NULL, "Invalid options size in set_fixed_tag" );
            op--;
            CYG_ASSERT(*op==TAG_END, "Missing TAG_END in set_fixed_tag");
            if ( op + len + 2 > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
                CYG_FAIL( "Oversize DHCP packet in set_fixed_tag replace" );
                return false;
            }
            *op = tag;
            *(op+1) = len;
            *(op + len + 2) = TAG_END;
        }
    }
    else { // overwrite the end tag and install a new one
        if ( op + len + 2 > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_fixed_tag append" );
            return false;
        }
        *op = tag;
        *(op+1) = len;
        *(op + len + 2) = TAG_END;
    }
    // and insert the value.  Net order is BE.
    op += len + 2 - 1;              // point to end of value
    while ( len-- > 0 ) {
        *op-- = (unsigned char)(value & 255);
        value >>= 8;
    }
    return true;
}

static int
set_variable_tag( struct bootp *ppkt,
               unsigned char tag,
               cyg_uint8 *pvalue,
               int len)
{
    unsigned char *op;

    // Initially this will only scan the options field.
    op = &ppkt->bp_vend[4];
    while (*op != TAG_END) {
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_variable_tag" );
            return false;
        }
        if (*op == tag)                 // Found it...
            break;
        if ( *op == TAG_PAD ) {
            op++;
        } else {
          op += *(op+1)+2;
        }
    }
    
    if (*op == tag) { // Found it...
        /* There are three possibilities:
         * 1) *(op+1) == len
         * 2) *(op+1) > len
         * 3) *(op+1) < len
         * For 1, just overwrite the existing option data.
         * For 2, overwrite the existing option data and pullup the
         *        remaining option data (if any).
         * For 3, pullup any remaining option data to remove the option
         *        and then add the option to the end.
         * For simplicity, for case 2 and 3, we just call unset_tag()
         * and re-add the option to the end.
         */
        if ( *(op+1) != len ) {
            /* Remove existing option entry. */
            unset_tag(ppkt, tag);
            /* Adjust the op pointer to re-add at the end. */
            op = scan_dhcp_size(ppkt);
            CYG_ASSERT(op!=NULL, "Invalid options size in set_variable_tag" );
            op--;
            CYG_ASSERT(*op==TAG_END, "Missing TAG_END in set_variable_tag");
            if ( op + len + 2 > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
                CYG_FAIL( "Oversize DHCP packet in set_variable_tag replace" );
                return false;
            }
            *op = tag;
            *(op+1) = len;
            *(op + len + 2) = TAG_END;
        }
    }
    else { // overwrite the end tag and install a new one
        if ( op + len + 2 > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_variable_tag append" );
            return false;
        }
        *op = tag;
        *(op+1) = len;
        *(op + len + 2) = TAG_END;
    }
    // and insert the value.  No order is implied.
    op += 2;               // point to start of value
    while ( len-- > 0 ) {
        *op++ = *pvalue++;
    }
    return true;
}

static int
unset_tag( struct bootp *ppkt,
           unsigned char tag )
{
    unsigned char *op, *nextp = 0, *killp = 0;

    // Initially this will only scan the options field.
    
    op = &ppkt->bp_vend[4];
    while (*op != TAG_END) {
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in unset_tag" );
            return false;
        }
        if (*op == tag) {               // Found it...
            killp = op;                 // item to kill
            nextp = op + *(op+1)+2;     // next item address
        }
        if ( *op == TAG_PAD ) {
            op++;
        } else {
          op += *(op+1)+2;
        }
    }

    if ( !killp )
        return false;

    // Obliterate the found op by copying down: *op is the end.
    while( nextp <= op )                // <= to copy the TAG_END too.
        *killp++ = *nextp++;
    
    return true;
}

#ifdef SUPPORT_STATIC_ROUTE_OPTION 
/* option 33 */
int OptionStaticRoute(struct bootp *packet)
{
	int s = 0;
	uint32_t des_ip, gw;
	uint8_t option_len=0,flag_tmp=0, temp[256];
	uint8_t host_ip[16], gw_ip[16];
	int length=sizeof(temp);
	if (!get_bootp_option(packet, TAG_IP_STATIC_ROUTES, temp, &length)) 
	{
//		diag_printf("No DHCP ACK with option 33!\n");
		return 0;
	} 
	else 
	{
//		diag_printf("DHCP ACK with option 33!\n");
		option_len = length;

		if((option_len > 0) && (option_len%8) == 0)
		{
			while(flag_tmp < option_len)
			{
				((unsigned char *)&des_ip)[0] = temp[flag_tmp];
				((unsigned char *)&des_ip)[1] = temp[flag_tmp+1];
				((unsigned char *)&des_ip)[2] = temp[flag_tmp+2];
				((unsigned char *)&des_ip)[3] = temp[flag_tmp+3];
				((unsigned char *)&gw)[0] = temp[flag_tmp+4];
				((unsigned char *)&gw)[1] = temp[flag_tmp+5];
				((unsigned char *)&gw)[2] = temp[flag_tmp+6];
				((unsigned char *)&gw)[3] = temp[flag_tmp+7];
				if((((unsigned char *)&des_ip)[0] == 0) && (((unsigned char *)&des_ip)[1] == 0) &&
					(((unsigned char *)&des_ip)[2] == 0) && (((unsigned char *)&des_ip)[3] == 0))
				{
//					diag_printf("%s, illegal destion 0.0.0.0 for static routes (option 33)\n",__FUNCTION__);
					flag_tmp = flag_tmp + 8;
					continue;
				}
				
				
				inet_ntoa_r(*((struct in_addr*)&des_ip), host_ip);
//				diag_printf("%s:%d host_ip=%s\n",__FUNCTION__,__LINE__,host_ip);
				
				inet_ntoa_r(*((struct in_addr*)&gw), gw_ip);
//				diag_printf("%s:%d gw_ip=%s\n",__FUNCTION__,__LINE__,gw_ip);
				
				RunSystemCmd(NULL_FILE, "route", "add", "-host", host_ip, "-netmask", "255.255.255.255", "-gateway", gw_ip, "-metric", "1", NULL_STR);
				flag_tmp = flag_tmp + 8;
			}
		}
		else
		{
//			diag_printf("%s, Incorrect option length %u for dhcp option 33 (static route)\n",__FUNCTION__,option_len);
			return 0;
		}
	}
	return 1;
}


/* option 121 */
int OptionClasslessStaticRoute(struct bootp *packet)
{
	/*
Note:
	- Vista dhcp client will send 121 and 249, and if dhcp server response 
		with 121, vista will only accept 121, ignore 249
	- We don't support router address is 0.0.0.0, refer to rfc 3442, page 5
	- if there are "static_routing" option in nvram, i will
		- remove old records
		- then find empty entry to record new records
		- maximun numbers of records are defined as CONFIG_STATIC_ROUTE_NUMBER
		- the name in each entry list are definded as NVRAM_STATIC_ROUTE_NAME
			i will take it as the condition to recognize the records added by me 
			to remove/add to nvram
	 */
	int s = 0;
	uint32_t des_ip, netmask, gw;
	uint8_t option_len=0,mask_num=0,flag_tmp=0, temp[256];
	uint8_t net_ip[16], mask_ip[16], gw_ip[16];
	int length=sizeof(temp);
	if (!get_bootp_option(packet, TAG_CLASSLESS_STATIC_ROUTE, temp, &length)) 
	{
//		diag_printf("No DHCP ACK with option 121!\n");
		return 0;
	} 
	else 
	{
//		diag_printf("DHCP ACK with option 121!\n");
	
		option_len = length;

		if(option_len >= 5)
		{ //rfc specifies minimun length is 5
			while(flag_tmp < option_len)
			{
				mask_num = temp[flag_tmp];
				if(mask_num == 0)
				{
					/* netmask is 0.0.0.0, default gw */
					((unsigned char *)&des_ip)[0] = ((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+1];
					((unsigned char *)&gw)[1] = temp[flag_tmp+2];
					((unsigned char *)&gw)[2] = temp[flag_tmp+3];
					((unsigned char *)&gw)[3] = temp[flag_tmp+4];
					flag_tmp = flag_tmp+5;
				}
				else if( 1 <= mask_num && mask_num <= 8 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+2];
					((unsigned char *)&gw)[1] = temp[flag_tmp+3];
					((unsigned char *)&gw)[2] = temp[flag_tmp+4];
					((unsigned char *)&gw)[3] = temp[flag_tmp+5];
					flag_tmp = flag_tmp+6;
				}
				else if( 9 <= mask_num && mask_num <= 16 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+3];
					((unsigned char *)&gw)[1] = temp[flag_tmp+4];
					((unsigned char *)&gw)[2] = temp[flag_tmp+5];
					((unsigned char *)&gw)[3] = temp[flag_tmp+6];
					flag_tmp = flag_tmp+7;
				}
				else if( 17 <= mask_num && mask_num <= 24 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+4];
					((unsigned char *)&gw)[1] = temp[flag_tmp+5];
					((unsigned char *)&gw)[2] = temp[flag_tmp+6];
					((unsigned char *)&gw)[3] = temp[flag_tmp+7];
					flag_tmp = flag_tmp+8;
				}
				else if( 25 <= mask_num && mask_num <= 32 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = temp[flag_tmp+4];
					((unsigned char *)&gw)[0] = temp[flag_tmp+5];
					((unsigned char *)&gw)[1] = temp[flag_tmp+6];
					((unsigned char *)&gw)[2] = temp[flag_tmp+7];
					((unsigned char *)&gw)[3] = temp[flag_tmp+8];
					flag_tmp = flag_tmp+9;
				}
				else
				{
					//error , should not go through here !!!
//					diag_printf("%s, Unknown mask num %u !\n",__FUNCTION__,mask_num);
					continue;
	   			}
		        	memset(&netmask,0,sizeof(uint32_t));
		        	for(s=0;s<mask_num;s++)
                			netmask = netmask | (1<<(31-s));			
				
				inet_ntoa_r(*((struct in_addr*)&des_ip), net_ip);
//				diag_printf("%s:%d net_ip=%s\n",__FUNCTION__,__LINE__,net_ip);				
				inet_ntoa_r(*((struct in_addr*)&netmask), mask_ip);
//				diag_printf("%s:%d mask_ip=%s\n",__FUNCTION__,__LINE__,mask_ip);				
				inet_ntoa_r(*((struct in_addr*)&gw), gw_ip);
//				diag_printf("%s:%d gw_ip=%s\n",__FUNCTION__,__LINE__,gw_ip);
				
				RunSystemCmd(NULL_FILE, "route", "add", "-net", net_ip, "-netmask", mask_ip, "-gateway", gw_ip, "-metric", "1", NULL_STR);

			}
		} 
		else 
		{		
//			diag_printf("%s, Invalid option length %u !\n",__FUNCTION__,option_len);
			return 0;
		}
	}
	return 1;
}

int OptionMicroSoftClasslessStaticRoute(struct bootp *packet)
{
	/*
Note:
	- Vista dhcp client will send 121 and 249, and if dhcp server response 
		with 121, vista will only accept 121, ignore 249
	- We don't support router address is 0.0.0.0, refer to rfc 3442, page 5
	*/
	int s = 0;
	uint32_t des_ip, netmask, gw;
	uint8_t option_len=0,mask_num=0,flag_tmp=0, temp[256];
	uint8_t net_ip[16], mask_ip[16], gw_ip[16];
	int length=sizeof(temp);
	if (!get_bootp_option(packet, TAG_MS_CLASSLESS_STATIC_ROUTE, temp, &length))
	{
//		diag_printf("No DHCP ACK with option 249!\n");
		return 0;
	} 
	else 
	{
//		diag_printf("DHCP ACK with option 249!\n");
		option_len = length;
		
		if(option_len >= 5)
		{ //rfc specifies minimun length is 5
			while(flag_tmp < option_len)
			{
				mask_num = temp[flag_tmp];
				if(mask_num == 0)
				{
					/* netmask is 0.0.0.0, default gw */
					((unsigned char *)&des_ip)[0] = ((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+1];
					((unsigned char *)&gw)[1] = temp[flag_tmp+2];
					((unsigned char *)&gw)[2] = temp[flag_tmp+3];
					((unsigned char *)&gw)[3] = temp[flag_tmp+4];
					flag_tmp = flag_tmp+5;
				}
				else if( 1 <= mask_num && mask_num <= 8 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+2];
					((unsigned char *)&gw)[1] = temp[flag_tmp+3];
					((unsigned char *)&gw)[2] = temp[flag_tmp+4];
					((unsigned char *)&gw)[3] = temp[flag_tmp+5];
					flag_tmp = flag_tmp+6;
				}
				else if( 9 <= mask_num && mask_num <= 16 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = (&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+3];
					((unsigned char *)&gw)[1] = temp[flag_tmp+4];
					((unsigned char *)&gw)[2] = temp[flag_tmp+5];
					((unsigned char *)&gw)[3] = temp[flag_tmp+6];
					flag_tmp = flag_tmp+7;
				}
				else if( 17 <= mask_num && mask_num <= 24 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+4];
					((unsigned char *)&gw)[1] = temp[flag_tmp+5];
					((unsigned char *)&gw)[2] = temp[flag_tmp+6];
					((unsigned char *)&gw)[3] = temp[flag_tmp+7];
					flag_tmp = flag_tmp+8;
				}
				else if( 25 <= mask_num && mask_num <= 32 )
				{
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = temp[flag_tmp+4];
					((unsigned char *)&gw)[0] = temp[flag_tmp+5];
					((unsigned char *)&gw)[1] = temp[flag_tmp+6];
					((unsigned char *)&gw)[2] = temp[flag_tmp+7];
					((unsigned char *)&gw)[3] = temp[flag_tmp+8];
					flag_tmp = flag_tmp+9;
				}
				else
				{
					//error , should not go through here !!!
//					diag_printf("%s, Unknown mask num %u !\n",__FUNCTION__,mask_num);
	    				continue;
				}
				memset(&netmask,0,sizeof(uint32_t));
				for(s=0;s<mask_num;s++)
					netmask = netmask | (1<<(31-s));				
				
				inet_ntoa_r(*((struct in_addr*)&des_ip), net_ip);
//				diag_printf("%s:%d net_ip=%s\n",__FUNCTION__,__LINE__,net_ip);				
				inet_ntoa_r(*((struct in_addr*)&netmask), mask_ip);
//				diag_printf("%s:%d mask_ip=%s\n",__FUNCTION__,__LINE__,mask_ip);				
				inet_ntoa_r(*((struct in_addr*)&gw), gw_ip);
//				diag_printf("%s:%d gw_ip=%s\n",__FUNCTION__,__LINE__,gw_ip);
				
				RunSystemCmd(NULL_FILE, "route", "add", "-net", net_ip, "-netmask", mask_ip, "-gateway", gw_ip, "-metric", "1", NULL_STR);
    			}	   
		}
		else
		{
//			diag_printf("%s, Invalid option length %u !\n",__FUNCTION__,option_len);
			return 0;
		}
    	}
	return 1;
}
#endif

#if defined(HAVE_TR069)
/* option 43 */
int OptionVendorSpecInfo(struct bootp *packet)
{	
	uint8_t temp[256], acs_url[128], flag_tmp=0;
	uint8_t sub_code, sub_len, *sub_data=NULL;
	
	int length=sizeof(temp);
	if (!get_bootp_option(packet, TAG_VEND_SPECIFIC, temp, &length)) 
	{
		//diag_printf("No DHCP ACK with option 43!\n");
		return 0;
	} 
	else 
	{
		//diag_printf("DHCP ACK with option 43!\n");
		if(length>2)
		{
			while(length>0)
			{
				sub_code=temp[flag_tmp];
				sub_len=temp[flag_tmp+1];
				sub_data=&temp[flag_tmp+2];

				if(length<sub_len+2)
					break;
				
				switch(sub_code)
				{
					case 1: //ACS URL
						strcpy(acs_url, sub_data);
						apmib_set(MIB_CWMP_ACS_URL, (void *)acs_url);
						break;

					default:
						//other field type
						break;
				}
				length=length-2-sub_len;
				flag_tmp=flag_tmp+2+sub_len;
			}
		}
		else 
		{		
			//diag_printf("%s, Invalid option length %u !\n",__FUNCTION__,option_len);
			return 0;
		}
	}
	return 1;
}
#endif

// ------------------------------------------------------------------------
// Bring up an interface enough to broadcast, before we know who we are

static int
bring_half_up(const char *intf, struct ifreq *ifrp )
{
    int s = -1;
    int one = 1;

    struct sockaddr_in *addrp;
//    struct ecos_rtentry route;
    int retcode = false;

    // Ensure clean slate
//    cyg_route_reinit();  // Force any existing routes to be forgotten

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
	 diag_printf("create socket fails!\n");
        goto out;
    }
	
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
	 diag_printf("set socket option SO_BROADCAST fails!\n");
        goto out;
    }

	if(strcmp(intf, "eth0")==0)
	{
		memset(ifrp, 0, sizeof(*ifrp)); 
		strcpy(ifrp->ifr_name, intf);
		
		if (ioctl(s, SIOCDIFADDR, ifrp)) 
		{
	       	 perror("SIOCIFADDR");
	       	// goto out;
	    	}
	}

	 if(strcmp(intf, "eth0")==0)
	 {
	 	struct in_addr lan_addr, lan_mask;
		char str_lan_addr[16]={0}, str_lan_mask[16]={0};
		
		get_lan_ip_addr(&lan_addr, &lan_mask);
		inet_ntoa_r(lan_addr, str_lan_addr);
		inet_ntoa_r(lan_mask, str_lan_mask);

		RunSystemCmd(NULL_FILE, "ifconfig", "eth0", str_lan_addr, "netmask" , str_lan_mask, NULL_STR);
	 }

    addrp = (struct sockaddr_in *) &ifrp->ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = INADDR_ANY;

    strcpy(ifrp->ifr_name, intf);

    if(strcmp(intf, "eth0")!=0)
    {
    if (ioctl(s, SIOCSIFADDR, ifrp)) { /* set ifnet address */
        perror("SIOCSIFADDR");
	 diag_printf("set ifnet address fails!\n");
        goto out;
    }

    if (ioctl(s, SIOCSIFNETMASK, ifrp)) { /* set net addr mask */
        perror("SIOCSIFNETMASK");
	 diag_printf("set net addr mask fails!\n");
        goto out;
    }

    /* the broadcast address is 255.255.255.255 */
    memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
    if (ioctl(s, SIOCSIFBRDADDR, ifrp)) { /* set broadcast addr */
        perror("SIOCSIFBRDADDR");
	 diag_printf("set broadcast addr fails!\n");	
        goto out;
    }
    }

    ifrp->ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, ifrp)) { /* set ifnet flags */
        perror("SIOCSIFFLAGS up");
	 diag_printf("set ifnet flags fails!\n");	
        goto out;
    }


#if 0
    if (ioctl(s, SIOCGIFHWADDR, ifrp) < 0) { /* get MAC address */
        perror("SIOCGIFHWADDR 1");
	 diag_printf("get MAC address fails!\n");	
        goto out;
    }

    // Set up routing
    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    addrp->sin_len = sizeof(*addrp);  // Size of address

    memset(&route, 0, sizeof(route));

    addrp->sin_addr.s_addr = INADDR_ANY;
    memcpy(&route.rt_dst, addrp, sizeof(*addrp));
    memcpy(&route.rt_gateway, addrp, sizeof(*addrp));
	
    inet_aton("255.0.0.0",&addrp->sin_addr);
    memcpy(&route.rt_genmask, addrp, sizeof(*addrp));
		
    route.rt_dev = ifrp->ifr_name;
    route.rt_flags = RTF_UP;
    route.rt_metric = 0;

    if (ioctl(s, SIOCDELRT, &route)) { /* add route */
        diag_printf("del route fails!\n");
    }
#endif

#if 0
    // Set up routing
    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    addrp->sin_len = sizeof(*addrp);  // Size of address


    /* the broadcast address is 255.255.255.255 */
    memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
    memset(&route, 0, sizeof(route));
    memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

    addrp->sin_addr.s_addr = INADDR_ANY;
    memcpy(&route.rt_dst, addrp, sizeof(*addrp));
    memcpy(&route.rt_genmask, addrp, sizeof(*addrp));

    route.rt_dev = ifrp->ifr_name;
    route.rt_flags = RTF_UP|RTF_GATEWAY;
    route.rt_metric = 0;

    if (ioctl(s, SIOCADDRT, &route)) { /* add route */
        if (errno != EEXIST) {
            perror("SIOCADDRT 3");
	     diag_printf("add route fails!\n");
            goto out;
        }
    }
#endif

    retcode = true;
 out:
    if (s != -1) 
    {
      close(s);	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
    }

    return retcode;
}


// ------------------------------------------------------------------------
// DHCP retransmission timeouts and number of tries
// 
// To work better with simulated failures (or real ones!) so that the rest
// of the system is tested, rather than DHCP renewal failures pulling
// everything down, we try a little more zealously than the RFC suggests.

static unsigned char timeout_random = 0;

struct timeout_state {
    unsigned int secs;
    int countdown;
};

static inline void reset_timeout( struct timeval *ptv, struct timeout_state *pstate )
{
#if 1
    timeout_random++;
    pstate->countdown = 4; // initial fast retries
    pstate->secs = 3 + (timeout_random & 3);
    ptv->tv_sec = 0;
    ptv->tv_usec = 65536 * (2 + (timeout_random & 3)); // 0.1 - 0.3S, about
#endif
    //ptv->tv_sec=1;
    //ptv->tv_usec=500000;
}

static inline int next_timeout( struct timeval *ptv, struct timeout_state *pstate )
{
#if 1
    if ( 0 < pstate->countdown-- )
        return true;
    if ( 0 == ptv->tv_sec )
        ptv->tv_sec = pstate->secs;
    else {
        timeout_random++;
        pstate->secs = ptv->tv_sec * 2 - 2 + (timeout_random & 3);
        pstate->countdown = 2; // later fast retries
        ptv->tv_sec = 0;
    }
    // If longer, too many tries...
    return pstate->secs < CYGNUM_NET_DHCP_MIN_RETRY_TIME; 
#endif
	//ptv->tv_sec=1;
	//ptv->tv_usec=500000;
	//return 1;
}

// ------------------------------------------------------------------------
// Lease expiry and alarms to notify it

static cyg_alarm_t alarm_function;

static void alarm_function(cyg_handle_t alarm, cyg_addrword_t data)
{
    struct dhcp_lease *lease = (struct dhcp_lease *)data;
    lease->which |= lease->next;
    if ( lease->needs_attention )
        cyg_semaphore_post( lease->needs_attention );

    // Step the lease on into its next state of being alarmed ;-)
    if ( lease->next & DHCP_LEASE_EX ) {
        cyg_alarm_disable( alarm );
    }
    else if ( lease->next & DHCP_LEASE_T2 ) {
        lease->next = DHCP_LEASE_EX;
        cyg_alarm_initialize( lease->alarm, lease->expiry, 0 );
        cyg_alarm_enable( lease->alarm );
    }
    else if ( lease->next & DHCP_LEASE_T1 ) {
        lease->next = DHCP_LEASE_T2;
        cyg_alarm_initialize( lease->alarm, lease->t2, 0 );
        cyg_alarm_enable( lease->alarm );
    }
}

static inline void no_lease( struct dhcp_lease *lease )
{
    if ( lease->alarm ) {
        // Already set: delete this.
        cyg_alarm_disable( lease->alarm );
        cyg_alarm_delete( lease->alarm );
        lease->alarm = 0;
    }
}

static inline void new_lease( struct bootp *bootp, struct dhcp_lease *lease )
{
    cyg_tick_count_t now = cyg_current_time();
    cyg_tick_count_t then;
    cyg_uint32 tag = 0;
    cyg_uint32 expiry_then;
    cyg_resolution_t resolution = 
        cyg_clock_get_resolution(cyg_real_time_clock());
    cyg_handle_t h;
    unsigned int length;
    
    // Silence any jabbering from past lease on this interface
    no_lease( lease );
    lease->which = lease->next = 0;
    cyg_clock_to_counter(cyg_real_time_clock(), &h);
    cyg_alarm_create( h, alarm_function, (cyg_addrword_t)lease,
                      &lease->alarm, &lease->alarm_obj );

    // extract the lease time and scale it &c to now.
    length = sizeof(tag);
    if(!get_bootp_option( bootp, TAG_DHCP_LEASE_TIME, &tag ,&length))
        tag = 0xffffffff;

    if ( 0xffffffff == tag ) {
        lease->expiry = 0xffffffff;
        lease->t2     = 0xffffffff;
        lease->t1     = 0xffffffff;
        return; // it's an infinite lease, hurrah!
    }

    then = (cyg_uint64)(ntohl(tag));
    expiry_then = then;

    then *= 1000000000; // into nS - we know there is room in a tick_count_t
    then = (then / resolution.dividend) * resolution.divisor; // into system ticks
    lease->expiry = now + then;
    length = sizeof(tag);
    if (get_bootp_option( bootp, TAG_DHCP_REBIND_TIME, &tag, &length ))
        then = (cyg_uint64)(ntohl(tag));
    else
        then = expiry_then - expiry_then/4;
    then *= 1000000000; // into nS - we know there is room in a tick_count_t
    then = (then / resolution.dividend) * resolution.divisor; // into system ticks
    lease->t2 = now + then;

    length = sizeof(tag);
    if (get_bootp_option( bootp, TAG_DHCP_RENEWAL_TIME, &tag, &length ))
        then = (cyg_uint64)(ntohl(tag));
    else
        then = expiry_then/2;
    then *= 1000000000; // into nS - we know there is room in a tick_count_t
    then = (then / resolution.dividend) * resolution.divisor; // into system ticks
    lease->t1 = now + then;

#if 0 // for testing this mechanism
    lease->expiry = now + 5000; // 1000 here makes for failure in the DHCP test
    lease->t2     = now + 3500;
    lease->t1     = now + 2500;
#endif

#ifdef CYGDBG_NET_DHCP_CHATTER
    diag_printf("new_lease:\n");
    diag_printf("  expiry = %d\n",lease->expiry);
    diag_printf("      t1 = %d\n",lease->t1);
    diag_printf("      t2 = %d\n",lease->t2);
#endif

    lease->next = DHCP_LEASE_T1;

    cyg_alarm_initialize( lease->alarm, lease->t1, 0 );
    cyg_alarm_enable( lease->alarm );
}

// ------------------------------------------------------------------------
// Set all the tags we want to use when sending a packet.
// This has expanded to a large, explicit set to interwork better
// with a variety of DHCP servers.

static void set_default_dhcp_tags( struct bootp *xmit )
{
    // Explicitly request full set of params that are default for LINUX
    // dhcp servers, but not default for others.  This is rather arbitrary,
    // but it preserves behaviour for people using those servers.
    // Perhaps configury of this set will be needed in future?
    //
    // Here's the set:
    FILE	*fp;
    static cyg_uint8 req_list[]  = {
#ifdef CYGOPT_NET_DHCP_PARM_REQ_LIST_REPLACE
        CYGOPT_NET_DHCP_PARM_REQ_LIST_REPLACE ,
#else
        TAG_DHCP_SERVER_ID    ,     //     DHCP server id: 10.16.19.66
        TAG_DHCP_LEASE_TIME   ,     //     DHCP time 51: 60
        TAG_DHCP_RENEWAL_TIME ,     //     DHCP time 58: 30
        TAG_DHCP_REBIND_TIME  ,     //     DHCP time 59: 52
        TAG_SUBNET_MASK       ,     //     subnet mask: 255.255.255.0
        TAG_GATEWAY           ,     //     gateway: 10.16.19.66
        TAG_DOMAIN_SERVER     ,     //     domain server: 10.16.19.66
        TAG_DOMAIN_NAME       ,     //     domain name: hmt10.cambridge.redhat.com
        TAG_IP_BROADCAST      ,     //     IP broadcast: 10.16.19.255
#endif
#ifdef SUPPORT_STATIC_ROUTE_OPTION 
	//static route option 33, 121, 249 for russia customer
	 TAG_IP_STATIC_ROUTES	,
	 TAG_CLASSLESS_STATIC_ROUTE		,
	 TAG_MS_CLASSLESS_STATIC_ROUTE	,
#endif

	 TAG_VEND_SPECIFIC	,
	 
#ifdef CYGNUM_NET_SNTP_UNICAST_MAXDHCP
        TAG_NTP_SERVER        ,     //     NTP Server Addresses(es)
#endif
#ifdef CYGOPT_NET_DHCP_PARM_REQ_LIST_ADDITIONAL
        CYGOPT_NET_DHCP_PARM_REQ_LIST_ADDITIONAL ,
#endif
    };

    if ( req_list[0] ) // So that one may easily turn it all off by configury
        set_variable_tag( xmit, TAG_DHCP_PARM_REQ_LIST,
                          &req_list[0], sizeof( req_list ) );

#ifdef CYGOPT_NET_DHCP_OPTION_HOST_NAME
{
    int nlen;
	fp = fopen("/etc/hosts.conf","r");
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_FILE, DBG_ACTION_ADD, 0);
#endif

	if(fp){
		fscanf(fp,"%s",dhcp_hostname);
		fclose(fp);
		
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_FILE, DBG_ACTION_DEL, 0);
#endif

	}
	nlen = strlen(dhcp_hostname);
    if (nlen > 0)
    	set_variable_tag( xmit, TAG_HOST_NAME, dhcp_hostname, nlen + 1);
}
#endif
#ifdef CYGOPT_NET_DHCP_OPTION_DHCP_CLIENTID_MAC
//if(xmit->bp_vend[6]==DHCPDISCOVER)   //for realtek server
{
	cyg_uint8 id[16+1];	/* sizeof bp_chaddr[] + 1 */

	id[0] = 1;  /* 1-byte hardware type: 1=ethernet. */
    CYG_ASSERT( xmit->bp_hlen<=(sizeof(id)-1), "HW address invalid" );
    memcpy(&id[1], &xmit->bp_chaddr, xmit->bp_hlen);
    set_variable_tag( xmit, TAG_DHCP_CLIENTID, id, xmit->bp_hlen+1);
}
#endif

#if defined(HAVE_TR069)
/////////////////////////add option 60
	cyg_uint8 opt60_val[8];
	strcpy(opt60_val, "Realtek");
	set_variable_tag( xmit, TAG_DHCP_CLASSID, opt60_val, strlen("Realtek"));
////////////////////////////////////
#endif

    // Explicitly specify our max message size.
    set_fixed_tag( xmit, TAG_DHCP_MAX_MSGSZ, BP_MINPKTSZ, 2 );
}

// ------------------------------------------------------------------------
// the DHCP state machine - this does all the work

//int do_dhcp(const char *intf, struct bootp *res, cyg_uint8 *pstate, struct dhcp_lease *lease)
unsigned short cksum(unsigned short *buf, int n)
{
    unsigned long sum;
    unsigned short result;

    for (sum = 0; n > 0; n--) {
      sum += *buf++;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = (u_short) (~sum);
    if (result == 0)
      result = 0xffff;

    return(result);
}

#if 0
void make_discover(struct bootp *dhcpmsg, int dhcp_len, cyg_uint32 xid, struct ifreq* ifr)
//void make_discover(struct bootp *dhcpmsg, int dhcp_len)
{
   int i = 0;
   cyg_uint32 tmpul = xid;
   
//   int dhcp_len=dhcp_size_for_send(dhcpmsg);
   
   memcpy(sndmsg.dhcp, dhcpmsg, dhcp_len); 
  
   /* fill udp header */
   sndmsg.udp->uh_sport = htons(IPPORT_BOOTPC);
   sndmsg.udp->uh_dport = htons(IPPORT_BOOTPS);
   sndmsg.udp->uh_ulen = htons(dhcp_len + UDPHL);
   sndmsg.udp->uh_sum = 0;

#if 0
   /* fill pseudo udp header */
   spudph.srcip.s_addr = 0;
   spudph.dstip.s_addr = 0xffffffff;
   spudph.zero = 0;
   spudph.prto = IPPROTO_UDP;
   spudph.ulen = snd.udp->uh_ulen;
#endif
  
   /* fill ip header */
   sndmsg.ip->ip_v = IPVERSION;
   sndmsg.ip->ip_hl = IPHL >> 2;
   sndmsg.ip->ip_tos = 0;
   sndmsg.ip->ip_len = htons(dhcp_len + UDPHL + IPHL);
//tmpul = generate_xid();
   tmpul += (tmpul >> 16);
   sndmsg.ip->ip_id = (u_short) (~tmpul);
   sndmsg.ip->ip_off = htons(IP_DF);						  /* XXX */
   sndmsg.ip->ip_ttl = 0x20;								  /* XXX */
   sndmsg.ip->ip_p = IPPROTO_UDP;
   sndmsg.ip->ip_src.s_addr = 0;
   sndmsg.ip->ip_dst.s_addr = 0xffffffff;
   sndmsg.ip->ip_sum = 0;
   sndmsg.ip->ip_sum = cksum((u_short *)sndmsg.ip, sndmsg.ip->ip_hl * 2);

#if 0
   /* fill ether frame header */
   for (i = 0; i < 6; i++) 
   {
	 sndmsg.ether->ether_dhost[i] = 0xff;
	 sndmsg.ether->ether_shost[i] = ifr->ifr_hwaddr.sa_data[i];
   }
   
   sndmsg.ether->ether_type = htons(ETHERTYPE_IP); 
//   return (ETHERHL+IPHL+UDPHL+dhcp_len);
#endif
 }
#endif

void clearWanInfo(char *if_name, char *filename)
{
	if(!if_name  || !filename) 
		return;

	char tmpbuf[64];
	char nulladdr[]="0.0.0.0";
	
	sprintf(tmpbuf, "interface:%s\n", if_name);
	write_line_to_file(filename, 1, tmpbuf);	

	sprintf(tmpbuf, "ip:%s\n", nulladdr);
	write_line_to_file(filename, 2, tmpbuf);	

	sprintf(tmpbuf, "netmask:%s\n", nulladdr);
	write_line_to_file(filename, 2, tmpbuf);

	sprintf(tmpbuf, "gateway:%s\n", nulladdr);
	write_line_to_file(filename, 2, tmpbuf);

	sprintf(tmpbuf, "dns1:%s\n", nulladdr);
	write_line_to_file(filename, 2, tmpbuf);

	sprintf(tmpbuf, "dns2:%s\n", nulladdr);
	write_line_to_file(filename, 2, tmpbuf);	
}

void clearDnsInfo(char *filename)
{
	if(!filename) 
		return;

	char tmpbuf[64];
	char nulladdr[]="0.0.0.0";
	
	sprintf(tmpbuf, "nameserver %s\n", nulladdr);
	write_line_to_file(filename, 1, tmpbuf);	

	sprintf(tmpbuf, "nameserver %s\n", nulladdr);
	write_line_to_file(filename, 2, tmpbuf);		
}

void writeDnsAddrToFile(unsigned char *dns, int len)
{
	if(!dns)
		return;
	int i;
	unsigned char tmpbuf[32];
	for(i=0;i<len;i+=4)
	{		
		sprintf(tmpbuf, "nameserver %d.%d.%d.%d\n", dns[i], dns[i+1], dns[i+2], dns[i+3]);
	
		if(i==0)
			write_line_to_file("/etc/resolv.conf", 1, tmpbuf);			
		else
			write_line_to_file("/etc/resolv.conf", 2, tmpbuf);
	}
}

#if 0
int check_offerpacket(struct bootp *dhcppacket, char *if_name)
{
	if(!dhcppacket)
		return 0;
	
	int length=sizeof(struct in_addr);
	
	struct in_addr server_ip, gateway, netmask;
	get_bootp_option(dhcppacket, TAG_DHCP_SERVER_ID, &server_ip, &length);
//	diag_printf("%s:%d	server_ip=%s\n",__FUNCTION__,__LINE__, inet_ntoa(server_ip));

	get_bootp_option(dhcppacket, TAG_GATEWAY, &gateway, &length);

//	diag_printf("%s:%d	gateway=%s\n",__FUNCTION__,__LINE__, inet_ntoa(gateway));
	
	get_bootp_option(dhcppacket, TAG_SUBNET_MASK, &netmask, &length);
//	diag_printf("%s:%d	netmask=%s\n",__FUNCTION__,__LINE__, inet_ntoa(netmask));
	
//	if((server_ip.s_addr & netmask.s_addr)!=(gateway.s_addr & netmask.s_addr))
//		return 0;

#ifdef HAVE_DHCPD
	if(strcmp(if_name, "eth1")==0)
	{
		int sfd=0;
		sfd=socket(AF_INET, SOCK_DGRAM, 0);
		if (sfd < 0) 
		{           
			diag_printf("create socket fails!\n");
			return 0;
		}
		struct ifreq if_req;
		struct in_addr eth0_netmask, eth0_ipaddr;
		bzero(&if_req, sizeof(if_req));
	
		strcpy(if_req.ifr_name, "eth0");
		if (ioctl(sfd, SIOCGIFNETMASK, &if_req) < 0) 
		{
			diag_printf("ioctl(SIOCGIFNETMASK) fails!\n");
			close(sfd);
			return 0;
		}		
		eth0_netmask.s_addr= ((struct sockaddr_in *)&if_req.ifr_addr)->sin_addr.s_addr;
		
		if (ioctl(sfd, SIOCGIFADDR, &if_req) < 0) 
		{
			diag_printf("ioctl(SIOCGIFADDR) fails!\n");
			close(sfd);
			return 0;
		}
		eth0_ipaddr.s_addr= ((struct sockaddr_in *)&if_req.ifr_addr)->sin_addr.s_addr;

		if((server_ip.s_addr & netmask.s_addr)==(eth0_ipaddr.s_addr & eth0_netmask.s_addr))
		{
			close(sfd);
			return 0;		
		}
	}
#endif

	return 1;	
}
#endif

int writeInfoToFile(struct bootp *dhcppacket, char *if_name, char *filename)
{
	if(!dhcppacket || !if_name || !filename)
		return 0;

	struct in_addr client_ip, gateway, netmask;
	int length, i;	
	unsigned char tmpbuf[128];
	unsigned char dns_addr[16];

	sprintf(tmpbuf, "interface:%s\n", if_name);
	write_line_to_file(filename, 1, tmpbuf);	

	client_ip = dhcppacket->bp_yiaddr;
	sprintf(tmpbuf, "ip:%s\n", inet_ntoa(client_ip));
	write_line_to_file(filename, 2, tmpbuf);	

	length=sizeof(struct in_addr);	 
	get_bootp_option(dhcppacket, TAG_SUBNET_MASK, &netmask, &length);
	sprintf(tmpbuf, "netmask:%s\n", inet_ntoa(netmask));
	write_line_to_file(filename, 2, tmpbuf);

	get_bootp_option(dhcppacket, TAG_GATEWAY, &gateway, &length);
	sprintf(tmpbuf, "gateway:%s\n", inet_ntoa(gateway));
	write_line_to_file(filename, 2, tmpbuf);
	
#if defined(HAVE_TR069)
	if(strcmp(if_name, "eth1")==0) //for ACS route
		write_line_to_file("/var/dhcpc_route.conf", 1, inet_ntoa(gateway));
#endif
	
//#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
	sprintf(tmpbuf, "%s", inet_ntoa(gateway));
	write_line_to_file("/etc/dhcpc_route.conf", 1, tmpbuf);
//#endif
	length=sizeof(dns_addr);
	get_bootp_option(dhcppacket, TAG_DOMAIN_SERVER, dns_addr, &length);
	for(i=0;i<length;i+=4)
	{		
		sprintf(tmpbuf, "dns%d:%d.%d.%d.%d\n", i/4+1, dns_addr[i], dns_addr[i+1], dns_addr[i+2], dns_addr[i+3]);
		write_line_to_file(filename, 2, tmpbuf);		
	} 
	return 1;	 
}


 void clearUpEvent(char *filename)
 {
	 if(filename==NULL)
		 return;
		 
	  char tmpbuf[2];
	  sprintf(tmpbuf,"0");
	  write_line_to_file(filename, 1, tmpbuf);
	  return;
  } 
 
 int getUpEvent(char *filename)
 {
	 struct stat status;
	 char buff[5];
	 FILE *fp;
 
	 if ( stat(filename, &status) < 0)
		 return -1;
	 fp = fopen(filename, "r");
	 if (!fp) {
			 printf("Can not open file :%s\n", filename);
		 return -1;
	 }
	 fgets(buff, sizeof(buff), fp);
	 fclose(fp);
	 return (atoi(buff));
 }
 
 
#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
 void addRouteForDnsServer(struct bootp *packet, cyg_uint8 *ifname)
 {
	 if(packet==NULL || ifname==NULL)
		 return;
	 
	 struct in_addr gw_addr;
	 cyg_uint8 dns_addr[16], gw_buf[16], dns_buf[16];
	 int gw_len=sizeof(gw_addr.s_addr);
	 int dns_len=sizeof(dns_addr);
	 int i;
	 if(get_bootp_option( packet, TAG_GATEWAY, &(gw_addr.s_addr), &gw_len) && 
		 get_bootp_option( packet, TAG_DOMAIN_SERVER, dns_addr, &dns_len))
	 {
		 inet_ntoa_r(gw_addr, gw_buf);
		 for(i=0;i<dns_len;i+=4)
		 {
			 sprintf(dns_buf, "%d.%d.%d.%d", dns_addr[i], dns_addr[i+1], dns_addr[i+2], dns_addr[i+3]);
			 
			 RunSystemCmd(NULL_FILE, "route", "delete", "-host", dns_buf, "-netmask", "255.255.255.255", "-gateway", gw_buf, "-interface", ifname, NULL_STR);
			 
			 RunSystemCmd(NULL_FILE, "route", "add", "-host", dns_buf, "-netmask", "255.255.255.255", "-gateway", gw_buf, "-interface", ifname, NULL_STR);
		 }
	 }	 
 }
#endif

#if defined(CONFIG_8881A_UPGRADE_KIT)
extern int auto_dhcp;
#define MAX_SELECT_COUNT 5
static int select_time_count = 0;
#endif

#define DHCPC_SEND_ARP 0

//#ifdef SEND_ARP_TO_CHECK_IP_CONFLICT
 static int arp_check(char *ifname, int type, struct in_addr *sip, struct in_addr *tip, u_char *enaddr)
 {	 
	 //struct in_addr sip;
	 //sip.s_addr=htonl(INADDR_ANY);
	 
	 struct arpcom *ac; 	
	 struct ifnet *pifnet=NULL;  
	 pifnet=rtl_getIfpByName(ifname);
	 
	 if(pifnet==NULL)
	 {		 
		 //diag_printf("%s:%d ####\n",__FUNCTION__,__LINE__);
		 return 1;
	 }
	 
	 ac=(struct arpcom *)pifnet;	   
	 
	 extern void rtl_arpRequest(struct arpcom *ac, int type, struct in_addr *sip, struct in_addr *tip, u_char *enaddr);
	 rtl_arpRequest(ac, type, sip, tip, enaddr);			 
 
	 sleep(1);
	 
	 if(rtl_checkArpReply(type))
	 	return 0;
	 
	 return 1;
 }
//#endif
 int do_dhcp(cyg_addrword_t data)
{
	//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	char *intf=NULL;
	struct bootp *res=NULL;
	cyg_uint8 *pstate=NULL;
	struct dhcp_lease *lease=NULL;	
	int retval;
	char filename[32];
	unsigned char mac_addr[IFHWADDRLEN];
	struct in_addr arp_client_ip, arp_server_ip;

	#ifdef CHECK_RECV_INTERFACE_INDEX
	struct sockaddr_storage from;
	struct msghdr mhdr;
	struct iovec iov;
	unsigned char cmsgbuf[512];
	int recv_len;
	struct cmsghdr *cm;
	struct sockaddr_dl *sdl;
	unsigned short bind_if_idx;
	int recv_if_flag=1;
	int match_flag=0;
	#endif

	
	if(((cyg_uint8)data) & 0x01)
	{	
		intf=wan_ifname+6;
		res=&wan_bootp_data;
		pstate=&wan_dhcpstate;
		lease=&wan_lease;
		wandhcpc_running=1;
	}
	else
	{
		intf=lan_ifname+6;
		res=&lan_bootp_data;
		pstate=&lan_dhcpstate;
		lease=&lan_lease;
		landhcpc_running=1;
	}

	#ifdef CHECK_RECV_INTERFACE_INDEX
	bind_if_idx=if_nametoindex(intf);
	//printf("\n%s:%d intf=%s bind_if_idx=%d\n",__FUNCTION__,__LINE__,intf,bind_if_idx);
	#endif
	
    struct ifreq ifr;
    struct sockaddr_in cli_addr, broadcast_addr, server_addr, rx_addr;
//    struct sockaddr client_addr;
    int s = -1;
    socklen_t addrlen;
    int one = 1;
    unsigned char mincookie[] = {99,130,83,99,255} ;
    struct timeval tv, so_tv;
    struct timeout_state timeout_scratch;
    cyg_uint8 oldstate = *pstate;
    cyg_uint8 msgtype = 0, seen_bootp_reply = 0;
    int length;
    unsigned char dns_addr[16];
	
    cyg_uint32 xid;
    int msglen=0;
	
    int status_val=0;
    int down_if_flag=0;

    so_tv.tv_sec=1;
    so_tv.tv_usec=500000;
	
#define CHECK_XID() (  /* and other details */                                  \
    ntohl(received->bp_xid)   != xid            || /* not the same transaction */      \
    received->bp_htype != xmit->bp_htype || /* not the same ESA type    */      \
    received->bp_hlen  != xmit->bp_hlen  || /* not the same length      */      \
    bcmp( &received->bp_chaddr, &xmit->bp_chaddr, xmit->bp_hlen )               \
    )

    // IMPORTANT: xmit is the same as res throughout this; *received is a
    // scratch buffer for reception; its contents are always copied to res
    // when we are happy with them.  So we always transmit from the
    // existing state.
    struct bootp rx_local;
    struct bootp *received = &rx_local;
    struct bootp *xmit = res;
    struct bootp xmit2;
    int xlen;

    // First, get a socket on the interface in question.  But Zeroth, if
    // needs be, bring it to the half-up broadcast only state if needs be.

#if 0
    if ( DHCPSTATE_INIT      == oldstate
         || DHCPSTATE_FAILED == oldstate
         || 0                == oldstate ) {
        // either explicit init state or the beginning of time or retry

//	if ( strcmp(intf, "eth0") && !bring_half_up( intf, &ifr ) )
	if (! bring_half_up( intf, &ifr ) )
            return false;		
	 
        *pstate = DHCPSTATE_INIT;
        lease->which = lease->next = 0;
    }
#endif
    
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }	
	
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
        goto out;
    }

   if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, intf, strlen(intf))) 
   {
 	diag_printf("set setsockopt SO_BINDTODEVICE fails!\n");
 	goto out;
   }

   #ifdef CHECK_RECV_INTERFACE_INDEX
   if (setsockopt(s, IPPROTO_IP, IP_RECVIF, &recv_if_flag, sizeof(recv_if_flag))) 
   {
 	diag_printf("set setsockopt IP_RECVIF fails!\n");
 	goto out;
   }
   #endif

    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_len = sizeof(cli_addr);
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(IPPORT_BOOTPC);
	
    memset((char *) &broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_len = sizeof(broadcast_addr);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(IPPORT_BOOTPS);

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_len = sizeof(server_addr);
    server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // overwrite later
    server_addr.sin_port = htons(IPPORT_BOOTPS);

    if (setsockopt(s, SOL_SOCKET, SO_USEBCASTADDR, &one, sizeof(one))) 
    {
        diag_printf("set setsockopt SO_BINDTODEVICE fails!\n");
        goto out;
    }
	
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        perror("setsockopt SO_REUSEADDR");
        goto out;
    }
	
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        perror("setsockopt SO_REUSEPORT");
        goto out;
    }
	
    if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &so_tv, sizeof(so_tv)))
    {
         perror("setsockopt SO_RCVTIMEO Error\n");
	  goto out;
    }
	
    if(bind(s, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        perror("bind error");
        goto out;
    }	
    
    // Now, we can launch into the DHCP state machine.  I think this will
    // be the neatest way to do it; it returns from within the switch arms
    // when all is well, or utterly failed.

    reset_timeout( &tv, &timeout_scratch );

    // Choose a new XID: first get the ESA as a basis:
    strcpy(&ifr.ifr_name[0], intf);
    if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        perror("SIOCGIFHWADDR 2");
        goto out;
    }
   memcpy(mac_addr, (unsigned char*)(ifr.ifr_hwaddr.sa_data), IFHWADDRLEN);   
    // Choose from scratch depending on ifr_hwaddr...[]
    xid = ifr.ifr_hwaddr.sa_data[5];
    xid |= (ifr.ifr_hwaddr.sa_data[4]) << 8;
    xid |= (ifr.ifr_hwaddr.sa_data[3]) << 16;
    xid |= (ifr.ifr_hwaddr.sa_data[2]) << 24;
    xid ^= (cyg_arc4random() & 0xffff0000);	

    // Avoid adjacent ESAs colliding by increment
#define NEW_XID(_xid) CYG_MACRO_START (_xid)+= 0x10000; CYG_MACRO_END
    

RE_CONNECT:
	if(recon_flag==1)
		recon_flag=0;

#if defined(CONFIG_8881A_UPGRADE_KIT)
	select_time_count = 0;
#endif
	
//	diag_printf("%s:%d oldstate=%d\n",__FUNCTION__,__LINE__,oldstate);

	 if ( DHCPSTATE_INIT	  == oldstate
			 || DHCPSTATE_FAILED == oldstate
			 || 0				 == oldstate ) {
			// either explicit init state or the beginning of time or retry
	
		if (! bring_half_up( intf, &ifr ) )
				goto out;
//				return false;	
		
//		 diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
		 
			*pstate = DHCPSTATE_INIT;
			lease->which = lease->next = 0;
		}	 

    while ( 1 ) {

		if(wandhcpc_quitting || landhcpc_quitting){
			//diag_printf("%s:%d####wandhcpc_quitting=%d\n",__FUNCTION__,__LINE__,wandhcpc_quitting);
			no_lease( lease );
 			do_dhcp_down_net( intf, res, &oldstate, lease);
			*pstate=0;
			/*break to close s*/
			break;
		}
		
		if(recon_flag==1)
		{	
//			diag_printf("%s:%d RECONNECT!\n",__FUNCTION__,__LINE__);
#ifdef DHCP_AUTO_SUPPORT
			if(isFileExist(DHCP_AUTO_CHECK_GETIP_FILE))
				unlink(DHCP_AUTO_CHECK_GETIP_FILE);
#endif
			no_lease( lease );
 			do_dhcp_down_net( intf, res, &oldstate, lease );
			oldstate=DHCPSTATE_INIT;
			syslogAll_printk("DHCPC disconnect!\n");
			syslogAll_printk("DHCPC reinit!\n");			
			goto RE_CONNECT;
		}		
		
//	diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
        // If we are active rather than in the process of shutting down,
        // check for any lease expiry every time round, so that alarms
        // *can* change the course of events even when already renewing,
        // for example.
        if ( DHCPSTATE_DO_RELEASE   != *pstate
             && DHCPSTATE_NOTBOUND  != *pstate
             && DHCPSTATE_FAILED    != *pstate ) {
            cyg_uint8 lease_state;
			
//		diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
		
            cyg_scheduler_lock();
            lease_state = lease->which;
            lease->which = 0; // flag that we have noticed it
            cyg_scheduler_unlock();

            if ( lease_state & DHCP_LEASE_EX ) {
                // then the lease has expired completely!
//                diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
                *pstate = DHCPSTATE_NOTBOUND;
            }
            else if ( lease_state & DHCP_LEASE_T2 ) {
                // Time to renew
                reset_timeout( &tv, &timeout_scratch ); // next conversation
                *pstate = DHCPSTATE_REBINDING;
            }
            else if ( lease_state & DHCP_LEASE_T1 ) {
                // Time to renew
                reset_timeout( &tv, &timeout_scratch ); // next conversation
                *pstate = DHCPSTATE_RENEWING;
            }
        }		
		
//	 diag_printf("%s:%d *ptate=%d\n",__FUNCTION__,__LINE__,*pstate);
        switch ( *pstate ) {
	
        case DHCPSTATE_INIT:
            // Send the DHCPDISCOVER packet
            // Fill in the BOOTP request - DHCPDISCOVER packet
            bzero(xmit, sizeof(*xmit));
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_htype = HTYPE_ETHERNET;
            xmit->bp_hlen = IFHWADDRLEN;
            xmit->bp_xid = htonl(xid);
            xmit->bp_secs = htons(cyg_current_time() / 100);
#ifdef BOOTP_COMPAT
            xmit->bp_flags = htons(0x0000); // UNICAST FLAG
#else
	     xmit->bp_flags = htons(0x8000); // BROADCAST FLAG
#endif
            bcopy(mac_addr, xmit->bp_chaddr, xmit->bp_hlen);			
            bcopy(mincookie, xmit->bp_vend, sizeof(mincookie));
            // remove the next line to test ability to handle bootp packets.
            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPDISCOVER, 1 );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
			
	     msglen=dhcp_size_for_send(xmit);
	     //diag_printf("####msglen=%d\n", msglen);
			
// 	     make_discover(xmit, msglen, xid, &ifr);

#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_INIT sending:\n" );
            show_bootp( intf, xmit );
#endif            	     
            if((length=sendto(s, xmit, msglen, 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr))) < 0) 
	     {
//		  diag_printf("########send DHCPSTATE_INIT packet fails!\n");
////               *pstate = DHCPSTATE_FAILED;
		sleep(1);
 		*pstate =DHCPSTATE_INIT;
                break;
            }
	     syslogAll_printk("DHCPC send DISCOVER packet!\n");
            seen_bootp_reply = 0;
            *pstate = DHCPSTATE_SELECTING;
            break;

        case DHCPSTATE_SELECTING:
            // This is a separate state so that we can listen again
            // *without* retransmitting.
            
            // listen for the DHCPOFFER reply

#if defined(CONFIG_8881A_UPGRADE_KIT)
		if (auto_dhcp == 1) {
			if (select_time_count < MAX_SELECT_COUNT)
				select_time_count++;
			else {
				if (select_time_count++ == MAX_SELECT_COUNT)
					kick_event(AUTODHCP_EVENT);

				sleep(60);
				break;
			}
		}
#endif

	#if 0
            if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
            {
			perror("setsockopt SO_RCVTIMEO Error\n");
			goto out;
	     }
	#endif

            addrlen = sizeof(rx_addr);

	#ifdef CHECK_RECV_INTERFACE_INDEX
	match_flag=0;
	memset(&iov, 0, sizeof(iov));
	memset(&mhdr, 0, sizeof(mhdr));

	iov.iov_base = received;
	iov.iov_len = sizeof(struct bootp);
	mhdr.msg_name = &from;
	mhdr.msg_namelen = sizeof(from);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (caddr_t)cmsgbuf;
	mhdr.msg_controllen = sizeof(cmsgbuf);

	if ((recv_len = recvmsg(s, &mhdr, 0)) < 0)		
	#else	
       if (recvfrom(s, received, sizeof(struct bootp), 0, (struct sockaddr *)&rx_addr, &addrlen) < 0) 
	#endif
		{                     	
                // No packet arrived (this time)
                if ( seen_bootp_reply ) { // then already have a bootp reply
                    // Save the good packet in *xmit
                    bcopy( received, xmit, dhcp_size(received) );
                    *pstate = DHCPSTATE_BOOTP_FALLBACK;
                    NEW_XID( xid ); // Happy to advance, so new XID
                    reset_timeout( &tv, &timeout_scratch );
                    break;
                }       
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
////                    *pstate = DHCPSTATE_FAILED;
			*pstate =DHCPSTATE_INIT;
			sleep(1);
                    break;
                }
                *pstate = DHCPSTATE_INIT; // to retransmit
                sleep(1);
                break;
            }
            // Check for well-formed packet with correct termination (not truncated)
//            memcpy(received, rcvmsg.dhcp, sizeof(struct bootp));

		#ifdef CHECK_RECV_INTERFACE_INDEX
		if(wandhcpc_running)
		{
			for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) 
		 	{
				if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVIF) 
				{
		    			//printf("\n%s:%d recv control msg!!!\n",__FUNCTION__,__LINE__);
					sdl = (struct sockaddr_dl *)(CMSG_DATA(cm));
					//printf("\n%s:%d sdl_index=%d bind_if_idx=%d!!!\n",__FUNCTION__,__LINE__,sdl->sdl_index,bind_if_idx);
					if((sdl!=NULL ) && (sdl->sdl_index==bind_if_idx))
					{
						match_flag=1;
						break;		
					}
				}
			}
			if(cm==NULL || match_flag==0)
			{
				*pstate = DHCPSTATE_INIT; // to retransmit
                		sleep(1);
                		break;
			}
		}
		#endif			

            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_SELECTING received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // XID and ESA matches?
                break;                  // listen again...

	     syslogAll_printk("DHCPC receive OFFER packet!\n");
//	     if(check_offerpacket(received, intf))
	     	{		 
	            if ( 0 == received->bp_siaddr.s_addr ) {
	                // then fill in from the options...
	                length = sizeof(received->bp_siaddr.s_addr);
	                get_bootp_option( received, TAG_DHCP_SERVER_ID,
	                                  &received->bp_siaddr.s_addr,
	                                  &length);
	            }

	            // see if it was a DHCP reply or a bootp reply; it could be
	            // either.
	            length = sizeof(msgtype);
	            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
	                                   &length) ) {
	                if ( DHCPOFFER == msgtype ) { // all is well
	                    // Save the good packet in *xmit                    
				#ifdef CONFIG_RTL_819X
				//exclude the padding option in receive packet
				rtl_exclude_pad(received, xmit);				
				#else
	                    bcopy( received, xmit, dhcp_size(received) );						
				#endif
	                    // we like the packet, so reset the timeout for next time
	                    reset_timeout( &tv, &timeout_scratch );
	                    *pstate = DHCPSTATE_REQUESTING;
	                    NEW_XID( xid ); // Happy to advance, so new XID

				if(((cyg_uint8)data) & 0x02)
				{
					length=sizeof(dns_addr);
					if(get_bootp_option( received, TAG_DOMAIN_SERVER, dns_addr, &length))						
						writeDnsAddrToFile(dns_addr, length);
				}
	                }
	            }
	            else // No TAG_DHCP_MESS_TYPE entry so it's a bootp reply
	                seen_bootp_reply = 1; // (keep the bootp packet in received)
	                
	            // If none of the above state changes occurred, we got a packet
	            // that "should not happen", OR we have a bootp reply in our
	            // hand; so listen again with the same timeout, without
	            // retrying the send, in the hope of getting a DHCP reply.
	     	}
            break;

        case DHCPSTATE_REQUESTING:
            // Just send what you got with a DHCPREQUEST in the message type.
            // then wait for an ACK in DHCPSTATE_REQUEST_RECV.

            // Fill in the BOOTP request - DHCPREQUEST packet
            xmit->bp_xid = htonl(xid);
            xmit->bp_op = BOOTREQUEST;
#ifdef BOOTP_COMPAT
            xmit->bp_flags = htons(0x0000); // UNICAST FLAG
#else
	     xmit->bp_flags = htons(0x8000); // BROADCAST FLAG
#endif

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPREQUEST, 1 );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
            // And this will be a new one:
            set_fixed_tag( xmit, TAG_DHCP_REQ_IP, ntohl(xmit->bp_yiaddr.s_addr), 4 );
            
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REQUESTING sending:\n" );
            show_bootp( intf, xmit );
#endif            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send( xmit );
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0, 
                      (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
 ////               *pstate = DHCPSTATE_FAILED;
 			sleep(1);
 			*pstate =DHCPSTATE_INIT;
                break;
            }
	     syslogAll_printk("DHCPC send REQUEST packet!\n");
            *pstate = DHCPSTATE_REQUEST_RECV;
            break;

        case DHCPSTATE_REQUEST_RECV:
            // wait for an ACK or a NACK - retry by going back to
            // DHCPSTATE_REQUESTING; NACK means go back to INIT.

		#if 0
            if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
            {
			perror("setsockopt SO_RCVTIMEO Error\n");
			goto out;
	     }
		#endif

            addrlen = sizeof(rx_addr);
		
#ifdef CHECK_RECV_INTERFACE_INDEX
		match_flag=0;
		memset(&iov, 0, sizeof(iov));
		memset(&mhdr, 0, sizeof(mhdr));
		
		iov.iov_base = received;
		iov.iov_len = sizeof(struct bootp);
		mhdr.msg_name = &from;
		mhdr.msg_namelen = sizeof(from);
		mhdr.msg_iov = &iov;
		mhdr.msg_iovlen = 1;
		mhdr.msg_control = (caddr_t)cmsgbuf;
		mhdr.msg_controllen = sizeof(cmsgbuf);
		
		if ((recv_len = recvmsg(s, &mhdr, 0)) < 0)		
#else	
		   if (recvfrom(s, received, sizeof(struct bootp), 0, (struct sockaddr *)&rx_addr, &addrlen) < 0) 
#endif		
             {
                // No packet arrived
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
////                    *pstate = DHCPSTATE_FAILED;
				*pstate =DHCPSTATE_INIT;
                    break;
                }
                *pstate = DHCPSTATE_REQUESTING;
                break;
            }
		
#ifdef CHECK_RECV_INTERFACE_INDEX
		if(wandhcpc_running)
		{
			for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) 
		 	{
				if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVIF) 
				{
		    			//printf("\n%s:%d recv control msg!!!\n",__FUNCTION__,__LINE__);
					sdl = (struct sockaddr_dl *)(CMSG_DATA(cm));
					//printf("\n%s:%d sdl_index=%d bind_if_idx=%d!!!\n",__FUNCTION__,__LINE__,sdl->sdl_index,bind_if_idx);
					if((sdl!=NULL ) && (sdl->sdl_index==bind_if_idx))
					{
						match_flag=1;
						break;
					}
				}
			}
			if(cm==NULL || match_flag==0)
			{
				if ( ! next_timeout( &tv, &timeout_scratch ) ) 
				 {
					*pstate =DHCPSTATE_INIT;
            					break;
        			}
                		*pstate = DHCPSTATE_REQUESTING;
                		break;
			}
		}
#endif

            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REQUEST_RECV received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // not the same transaction;
                break;                  // listen again...

            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                length = sizeof(received->bp_siaddr.s_addr );
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // check it was a DHCP reply
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPACK == msgtype // Same offer & server?
                     && received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr
                     && received->bp_siaddr.s_addr == xmit->bp_siaddr.s_addr) {
                     
                     syslogAll_printk("DHCPC receive ACK packet!\n");

			arp_client_ip.s_addr=received->bp_yiaddr.s_addr;
			arp_server_ip.s_addr=received->bp_siaddr.s_addr;
			
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    // Record the new lease and set up timers &c
			new_lease( received, lease );			
                    *pstate = DHCPSTATE_BOUND;		
			if(DHCPSTATE_BOUND != oldstate)
			{
				//check whether ip duplication	
				#ifdef SEND_ARP_TO_CHECK_IP_CONFLICT
				 struct in_addr sip;
	 			 sip.s_addr=htonl(INADDR_ANY);				
				if(!arp_check(intf, DHCPC_SEND_ARP, &sip, &received->bp_yiaddr, mac_addr))
				{
				     //diag_printf("%s:%d ####duplication IP address!\n",__FUNCTION__,__LINE__);					 
				     bzero(xmit, sizeof(*xmit));
			            xmit->bp_op = BOOTREQUEST;
			            xmit->bp_htype = HTYPE_ETHERNET;
			            xmit->bp_hlen = IFHWADDRLEN;
			            xmit->bp_xid = htonl(xid);
			            xmit->bp_secs = htons(cyg_current_time() / 100);
			            xmit->bp_flags = htons(0x8000); // BROADCAST FLAG
			            bcopy(mac_addr, xmit->bp_chaddr, xmit->bp_hlen);			
			            bcopy(mincookie, xmit->bp_vend, sizeof(mincookie));
			            // remove the next line to test ability to handle bootp packets.
			            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPDECLINE, 1 );
			            // Set all the tags we want to use when sending a packet
			            set_default_dhcp_tags( xmit );
				     set_fixed_tag( xmit, TAG_DHCP_REQ_IP, ntohl(received->bp_yiaddr.s_addr), 4 );
				     set_fixed_tag( xmit, TAG_DHCP_SERVER_ID, ntohl(received->bp_siaddr.s_addr), 4 );				 
				     msglen=dhcp_size_for_send(xmit);

				    sendto(s, xmit, msglen, 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)); 		     		    
				    recon_flag=1;
				    break;
				}
				#endif
				do_dhcp_down_net( intf, res, &oldstate, lease );
				if (!init_net(intf, res)) 
				{
					do_dhcp_down_net( intf, res, pstate, lease );
                        		*pstate = DHCPSTATE_FAILED;
                        		goto out;
				}
				oldstate=DHCPSTATE_BOUND;
			}

			if(((cyg_uint8)data) & 0x01)
			{		
#ifdef HOME_GATEWAY
				#ifdef CONFIG_RTL_DHCP_PPPOE
					if(!isFileExist("/etc/ppp_link"))
						writeInfoToFile(received, intf, "/etc/wan_info");
					else
						writeInfoToFile(received, intf, "/etc/wan_dhcp_info");
				#else
					writeInfoToFile(received, intf, "/etc/wan_info");
				#endif				
				kick_event(DHCP_EVENT);
				//sleep to avoid multiple fags (DHCP_EVENT and START_DHCP_PPP_EVENT) are setted at same time 
				//sleep(2);
				
				//diag_printf("%s:%d####calling set_dhcp_connect_flag()\n",__FUNCTION__,__LINE__);
				//set_event_flag(&dhcp_connected_flag, 1);
				kick_event(START_DHCP_PPP_EVENT);
				//diag_printf("%s:%d####DHCP_EVENT!\n",__FUNCTION__,__LINE__);
//				kick_event(WAN_EVENT);
				syslogAll_printk("WAN connected!\n");
#endif
			}
			else
			{
				kick_event(LAN_EVENT);
				writeInfoToFile(received, intf, "/etc/lan_info");	
#ifdef ULINK_DHCP_AUTO
				writeInfoToFile(received, intf, ULINK_DHCP_AUTO_CHECK_GETIP_FILE);
#endif
#ifdef DHCP_AUTO_SUPPORT
				writeInfoToFile(received, intf, DHCP_AUTO_CHECK_GETIP_FILE);
				extern void set_waitIPCount_value(int value);
				extern int get_waitIPCount_value();
				#define WAIT_IP_COUNT_MAX 15
			
				if(get_waitIPCount_value()>WAIT_IP_COUNT_MAX)
					set_waitIPCount_value(0);
#endif
			}
			
#ifdef SUPPORT_STATIC_ROUTE_OPTION 
			/* RFC3442 handle option 121 if needed */
			retval=OptionClasslessStaticRoute(received);

			/* RFC3442 handle option 249 if needed */
			retval=OptionMicroSoftClasslessStaticRoute(received);

			/* RFC2132 handle option 33 if needed */				
			retval=OptionStaticRoute(received);
#endif

#if defined(HAVE_TR069)
			//handle option 43
			OptionVendorSpecInfo(received);
#endif

#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
			addRouteForDnsServer(received, intf);
#endif
			if(((cyg_uint8)data) & 0x01)
			{
				sleep(3);			
				down_if_flag=0;
				wan_status_flag=0;
				
				#ifdef KLD_ENABLED
				sprintf(filename, "/etc/%s_status", intf);
				clearUpEvent(filename);
				#endif
			}
                    break;
                }
                if ( DHCPNAK == msgtype // Same server?
                     && received->bp_siaddr.s_addr == xmit->bp_siaddr.s_addr) {
                    // we're bounced!
                    syslogAll_printk("DHCPC receive NAK packet!\n");
                    *pstate = DHCPSTATE_INIT;  // So back the start of the rigmarole.
                    NEW_XID( xid ); // Unhappy to advance, so new XID
                    reset_timeout( &tv, &timeout_scratch );
                    break;
                }
                // otherwise it's something else, maybe another offer, or a bogus
                // NAK from someone we are not asking!
                // Just listen again, which implicitly discards it.
            }
            break;

        case DHCPSTATE_BOUND:

CHECK_EVENT:
		
		if(((cyg_uint8)data) & 0x01)
		{
			status_val=0;
			if(wan_status_flag)
			{
				//diag_printf("%s:%d wan_status_flag=%d\n",__FUNCTION__,__LINE__,wan_status_flag);
				status_val= wan_status_flag;
				wan_status_flag=0;
			}
			else
			{
				#ifdef KLD_ENABLED
				status_val=getUpEvent(filename);		
				#endif
			}
			
			if(status_val > DHCP_RENEW_FROM_WEBPAG)
			{   
			       //support dlink renew function from webpage
				down_if_flag=0;
				clearUpEvent(filename);
				recon_flag=1;
				*pstate=DHCPSTATE_DO_RELEASE;
				syslogAll_printk("WAN DHCP RENEW!\n");
			}
			else if(status_val>0)
			{
				if(status_val<DHCP_RELEASE_FROM_WEBPAG)
				{
					//wan interface status changes					
					if(status_val==DHCP_WAN_STATUS_DOWN)
					{
						//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
						*pstate=DHCPSTATE_FAILED;
					}
					else  //DHCP_WAN_STATUS_UP
					{
						//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
						////goto RE_CONNECT;
						*pstate=DHCPSTATE_DO_RELEASE;
					}
					//clearUpEvent(filename);
				}
				else
				{
					 //support dlink release function from webpage
					 if(down_if_flag>0)
					 {
						sleep(1);
						goto CHECK_EVENT;
					 }
					 *pstate=DHCPSTATE_DO_RELEASE;
				}			

				syslogAll_printk("WAN disconnect!\n");
			}
			else
		       	sleep(2);
		}
		else
		{	
			#ifdef CHECK_SERVER_ALIVE
			//send arp to check dhcp server is alive
			if(arp_check(intf, DHCPC_SEND_ARP, &arp_client_ip, &arp_server_ip, mac_addr))
			{
				//printf("\n%s:%d goto reconnect!!!\n",__FUNCTION__,__LINE__);
				recon_flag=1;
			}
			else	
	       		sleep(5);
			#else
				sleep(2);
			#endif
		}
		 
	     break;
            // Otherwise, nothing whatsoever to do...
           // return true;

        case DHCPSTATE_RENEWING:
            // Just send what you got with a DHCPREQUEST in the message
            // type UNICAST straight to the server.  Then wait for an ACK.

            // Fill in the BOOTP request - DHCPREQUEST packet
            xmit->bp_xid = htonl(xid);
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0); // No BROADCAST FLAG
            // Use the *client* address here:
            xmit->bp_ciaddr.s_addr = xmit->bp_yiaddr.s_addr;

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPREQUEST, 1 );
            // These must not be set in this context
            unset_tag( xmit, TAG_DHCP_REQ_IP );
            unset_tag( xmit, TAG_DHCP_SERVER_ID );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
            
            // Set unicast address to *server*
            server_addr.sin_addr.s_addr = res->bp_siaddr.s_addr;

#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_RENEWING sending:\n" );
            diag_printf( "UNICAST to family %d, addr %08x, port %d\n",
                         server_addr.sin_family,
                         server_addr.sin_addr.s_addr,
                         server_addr.sin_port );
            show_bootp( intf, xmit );
#endif            
            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send(xmit);
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0,
                       // UNICAST address of the server:
                      (struct sockaddr *)&server_addr,
                      sizeof(server_addr)) < 0) {
                      sleep(1);
                     *pstate = DHCPSTATE_FAILED;
////			*pstate =DHCPSTATE_INIT;
                break;
            }
	     syslogAll_printk("DHCPC send RENEWING REQUEST packet!\n");
            *pstate = DHCPSTATE_RENEW_RECV;
            break;

        case DHCPSTATE_RENEW_RECV:
            // wait for an ACK or a NACK - retry by going back to
            // DHCPSTATE_RENEWING; NACK means go to NOTBOUND.
            // No answer means just wait for T2, to broadcast.

	    #if 0
           if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
            {
			perror("setsockopt SO_RCVTIMEO Error\n");
			goto out;
	     }
	    #endif

            addrlen = sizeof(rx_addr);

	#ifdef CHECK_RECV_INTERFACE_INDEX
	match_flag=0;
	memset(&iov, 0, sizeof(iov));
	memset(&mhdr, 0, sizeof(mhdr));

	iov.iov_base = received;
	iov.iov_len = sizeof(struct bootp);
	mhdr.msg_name = &from;
	mhdr.msg_namelen = sizeof(from);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (caddr_t)cmsgbuf;
	mhdr.msg_controllen = sizeof(cmsgbuf);

	if ((recv_len = recvmsg(s, &mhdr, 0)) < 0)		
	#else	
       if (recvfrom(s, received, sizeof(struct bootp), 0, (struct sockaddr *)&rx_addr, &addrlen) < 0) 
	#endif		
            {
                // No packet arrived
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
                    // If we timed out completely, just give up until T2
                    // expires - retain the lease meanwhile.  The normal
                    // lease mechanism will invoke REBINDING as and when
                    // necessary.
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                *pstate = DHCPSTATE_RENEWING;
                break;
            }
	
	#ifdef CHECK_RECV_INTERFACE_INDEX
	if(wandhcpc_running)
	{
		for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) 
		{
			if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVIF) 
			{
				//printf("\n%s:%d recv control msg!!!\n",__FUNCTION__,__LINE__);
				sdl = (struct sockaddr_dl *)(CMSG_DATA(cm));
				//printf("\n%s:%d sdl_index=%d bind_if_idx=%d!!!\n",__FUNCTION__,__LINE__,sdl->sdl_index,bind_if_idx);
				if((sdl!=NULL ) && (sdl->sdl_index==bind_if_idx))
				{
					match_flag=1;
					break;
				}
			}
		}
		if(cm==NULL || match_flag==0)
		{
			if ( ! next_timeout( &tv, &timeout_scratch ) ) 
			{                  
				*pstate = DHCPSTATE_BOUND;
				break;
        		}
        		*pstate = DHCPSTATE_RENEWING;
        		break;
		}
	}
	#endif		
	
            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_RENEW_RECV received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // not the same transaction;
                break;                  // listen again...
	     syslogAll_printk("DHCPC receive RENEWING ACK packet!\n");
            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                length = sizeof(received->bp_siaddr.s_addr);
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // check it was a DHCP reply
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPACK == msgtype  // Same offer?
                     && received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr) {
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    // Record the new lease and set up timers &c
                    new_lease( received, lease );
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
               //// if ( DHCPNAK == msgtype ) { // we're bounced!
               if(DHCPNAK == msgtype && (received->bp_ciaddr.s_addr==xmit->bp_yiaddr.s_addr || received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr)){
                    *pstate = DHCPSTATE_NOTBOUND;  // So quit out.
                    break;
                }
                // otherwise it's something else, maybe another offer.
                // Just listen again, which implicitly discards it.
            }
            break;

        case DHCPSTATE_REBINDING:
            // Just send what you got with a DHCPREQUEST in the message type.
            // Then wait for an ACK.  This one is BROADCAST.

            // Fill in the BOOTP request - DHCPREQUEST packet
            xmit->bp_xid = htonl(xid);
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0x8000); // no BROADCAST FLAG            //****xmit->bp_flags = htons(0x8000);
            // Use the *client* address here:
            xmit->bp_ciaddr.s_addr = xmit->bp_yiaddr.s_addr;

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPREQUEST, 1 );
            // These must not be set in this context
            unset_tag( xmit, TAG_DHCP_REQ_IP );
            unset_tag( xmit, TAG_DHCP_SERVER_ID );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
            
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REBINDING sending:\n" );
            show_bootp( intf, xmit );
#endif            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send( xmit );
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0, 
                      (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                 sleep(1);
                *pstate = DHCPSTATE_FAILED;
////			*pstate =DHCPSTATE_INIT;
                break;
            }
	     syslogAll_printk("DHCPC send REBINDING REQUEST packet!\n");
            *pstate = DHCPSTATE_REBIND_RECV;
            break;

        case DHCPSTATE_REBIND_RECV:
            // wait for an ACK or a NACK - retry by going back to
            // DHCPSTATE_REBINDING; NACK means go to NOTBOUND.
            // No answer means just wait for expiry; we tried!

	    #if 0
            if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
            {
			perror("setsockopt SO_RCVTIMEO Error\n");
			goto out;
	     }	
	    #endif

            addrlen = sizeof(rx_addr);

	#ifdef CHECK_RECV_INTERFACE_INDEX
	match_flag=0;
	memset(&iov, 0, sizeof(iov));
	memset(&mhdr, 0, sizeof(mhdr));

	iov.iov_base = received;
	iov.iov_len = sizeof(struct bootp);
	mhdr.msg_name = &from;
	mhdr.msg_namelen = sizeof(from);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (caddr_t)cmsgbuf;
	mhdr.msg_controllen = sizeof(cmsgbuf);

	if ((recv_len = recvmsg(s, &mhdr, 0)) < 0)		
	#else	
       if (recvfrom(s, received, sizeof(struct bootp), 0, (struct sockaddr *)&rx_addr, &addrlen) < 0) 
	#endif	
             {
                // No packet arrived
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
                    // If we timed out completely, just give up until EX
                    // expires - retain the lease meanwhile.  The normal
                    // lease mechanism will invoke NOTBOUND state as and
                    // when necessary.
                    *pstate = DHCPSTATE_BOUND;
//			 diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
                    break;
                }
                *pstate = DHCPSTATE_REBINDING;
//		   diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
                break;
            }

		
#ifdef CHECK_RECV_INTERFACE_INDEX
		if(wandhcpc_running)
		{
			for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) 
			{
				if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVIF) 
				{
					//printf("\n%s:%d recv control msg!!!\n",__FUNCTION__,__LINE__);
					sdl = (struct sockaddr_dl *)(CMSG_DATA(cm));
					//printf("\n%s:%d sdl_index=%d bind_if_idx=%d!!!\n",__FUNCTION__,__LINE__,sdl->sdl_index,bind_if_idx);
					if((sdl!=NULL ) && (sdl->sdl_index==bind_if_idx))
					{
						match_flag=1;
						break;
					}
				}
			}
			if(cm==NULL || match_flag==0)
			{
				if ( ! next_timeout( &tv, &timeout_scratch ) ) 
				{                   
            				*pstate = DHCPSTATE_BOUND;
            				break;
        			}
                		*pstate = DHCPSTATE_REBINDING;
                		break;
			}
		}
#endif		
	
            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REBIND_RECV received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // not the same transaction;
                break;                  // listen again...

            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                unsigned int length = sizeof(received->bp_siaddr.s_addr );
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // check it was a DHCP reply
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPACK == msgtype  // Same offer?
                     && received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr) {
                     syslogAll_printk("DHCPC receive REBINDING ACK packet!\n");
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    // Record the new lease and set up timers &c
                    new_lease( received, lease );
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                else if ( DHCPNAK == msgtype ) { // we're bounced!
                    *pstate = DHCPSTATE_NOTBOUND;  // So back the start of the rigmarole.
                    break;
                }
                // otherwise it's something else, maybe another offer.
                // Just listen again, which implicitly discards it.
            }
            break;

        case DHCPSTATE_BOOTP_FALLBACK:
            // All done with socket
            close(s);
			
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

            s = -1;
            
            // And no lease should have become active, but JIC
            no_lease( lease );
            // Re-initialize the interface with the new state
            if ( DHCPSTATE_BOOTP_FALLBACK != oldstate ) {
                // Then need to go down and up
                do_dhcp_down_net( intf, res, &oldstate, lease ); // oldstate used
                if ( 0 != oldstate ) {
                    // Then not called from init_all_network_interfaces()
                    // so we must initialize the interface ourselves
                    if (!init_net(intf, res)) {
                        do_dhcp_down_net( intf, res, pstate, lease );
                        *pstate = DHCPSTATE_FAILED;
                        goto out;
                    }
                }
            }

            // Otherwise, nothing whatsoever to do...
            return true;

        case DHCPSTATE_NOTBOUND:
#if 0
            // All done with socket
            close(s);
            // No lease active
            no_lease( lease );
            // Leave interface up so app can tidy.
            diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
            return true;
 #endif
 		if(down_if_flag==0)
 		{
 			no_lease( lease );
 			do_dhcp_down_net( intf, res, &oldstate, lease );			
			down_if_flag = 1;

			if(((cyg_uint8)data) & 0x01)
				clearWanInfo(intf, "/etc/wan_info");
			
			if(((cyg_uint8)data) & 0x02)
				clearDnsInfo("/etc/resolv.conf");
 		}

		
// 		*pstate = DHCPSTATE_INIT; 		
		
		if(status_val>DHCP_RELEASE_FROM_WEBPAG && status_val<DHCP_RENEW_FROM_WEBPAG)
		{
			//support dlink release function from webpage
			sleep(1);
			goto CHECK_EVENT;
		}
		else    
			recon_flag=1;
//			goto RE_CONNECT;
//		return false;
 		break;

        case DHCPSTATE_FAILED:
            // All done with socket
//            close(s);
            // No lease active
            //diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
            if(down_if_flag==0)
            {
            		//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
            		down_if_flag=1;
            		no_lease( lease );
            		// Unconditionally down the interface.
            		do_dhcp_down_net( intf, res, &oldstate, lease );

			if(((cyg_uint8)data) & 0x01)
				clearWanInfo(intf, "/etc/wan_info");
			
			if(((cyg_uint8)data) & 0x02)
				clearDnsInfo("/etc/resolv.conf");
            }			
	     if(status_val==DHCP_WAN_STATUS_DOWN)
	     {
	     		//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
			sleep(1);
			goto CHECK_EVENT;
	     }
	     else
	     		goto RE_CONNECT;
//            return false;

        case DHCPSTATE_DO_RELEASE:
            // We have been forced here by external means, to release the
            // lease for graceful shutdown.

            // Just send what you got with a DHCPRELEASE in the message
            // type UNICAST straight to the server.  No ACK.  Then go to
            // NOTBOUND state.
            NEW_XID( xid );
            xmit->bp_xid = htonl(xid);
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0); // no BROADCAST FLAG
            // Use the *client* address here:
            xmit->bp_ciaddr.s_addr = xmit->bp_yiaddr.s_addr;

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPRELEASE, 1 );

            // Set unicast address to *server*
            server_addr.sin_addr.s_addr = res->bp_siaddr.s_addr;

#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_DO_RELEASE sending:\n" );
            diag_printf( "UNICAST to family %d, addr %08x, port %d\n",
                         server_addr.sin_family,
                         server_addr.sin_addr.s_addr,
                         server_addr.sin_port );
            show_bootp( intf, xmit );
#endif            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send( xmit );
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0, 
                       // UNICAST address of the server:
                      (struct sockaddr *)&server_addr,
                      sizeof(server_addr)) < 0) {
                      sleep(1);
                *pstate = DHCPSTATE_FAILED;
 ////			*pstate =DHCPSTATE_INIT;
                break;
            }
		syslogAll_printk("DHCPC send RELEASE packet!\n");
            *pstate = DHCPSTATE_NOTBOUND;
            break;

        default:
            no_lease( lease );
            close(s);
			
#ifdef ECOS_DBG_STAT
	     dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
            return false;
        }
    }

out:
    //diag_printf("#############%s:%d	dhcpc thread exit!\n", __FUNCTION__,__LINE__);
    if (s != -1) 
    {
        close (s);
		
#ifdef ECOS_DBG_STAT
       dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
    }
	
    return false;
}

// ------------------------------------------------------------------------
// Bring an interface down, failed to initialize it or lease is expired
// Also part of normal startup, bring down for proper reinitialization

int
do_dhcp_down_net(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease)
{
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    int s = -1;
    int retcode = false;

    // Ensure clean slate
//    cyg_route_reinit();  // Force any existing routes to be forgotten

#ifdef HOME_GATEWAY
#ifndef CONFIG_RTL_DHCP_PPPOE
	if(is_interface_up("ppp0"))
		ppp_disconnect();
#endif
#endif				
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    addrp = (struct sockaddr_in *) &ifr.ifr_addr;

    // Remove any existing address
    if ( DHCPSTATE_FAILED  == *pstate
         || DHCPSTATE_INIT == *pstate
         || 0              == *pstate ) {
        // it was configured for broadcast only, "half-up"
        memset(addrp, 0, sizeof(*addrp));
        addrp->sin_family = AF_INET;
        addrp->sin_len = sizeof(*addrp);
        addrp->sin_port = 0;
        addrp->sin_addr.s_addr = INADDR_ANY;
    }
    else {
        // get the specific address that was used
        strcpy(ifr.ifr_name, intf);
        if (ioctl(s, SIOCGIFADDR, &ifr)) {
            perror("SIOCGIFADDR 1");
            goto out;
        }
    }

    strcpy(ifr.ifr_name, intf);
    if (ioctl(s, SIOCDIFADDR, &ifr)) { /* delete IF addr */
        perror("SIOCDIFADDR1");
    }

	if(strcmp(intf, "eth0")==0)
	 {
	 	struct in_addr lan_addr, lan_mask;
		char str_lan_addr[16]={0}, str_lan_mask[16]={0};
		
		get_lan_ip_addr(&lan_addr, &lan_mask);
		inet_ntoa_r(lan_addr, str_lan_addr);
		inet_ntoa_r(lan_mask, str_lan_mask);

		RunSystemCmd(NULL_FILE, "ifconfig", "eth0", str_lan_addr, "netmask" , str_lan_mask, NULL_STR);
	 }
	
#ifdef DHCP_AUTO_SUPPORT
	if(isFileExist(DHCP_AUTO_CHECK_GETIP_FILE))
		unlink(DHCP_AUTO_CHECK_GETIP_FILE);
#endif

#ifdef INET6
    {
      int s6;
      struct if_laddrreq iflr;
      
      s6 = socket(AF_INET6, SOCK_DGRAM, 0);
      if (s6 < 0) {
        perror("socket AF_INET6");
//        close (s);
        return false;
      }
	  
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

      // Now delete the ipv6 addr
      memset(&iflr,0,sizeof(iflr));
      strcpy(iflr.iflr_name, intf);
      if (!ioctl(s6, SIOCGLIFADDR, &iflr)) {
      
	strcpy(iflr.iflr_name, intf);
	if (ioctl(s6, SIOCDLIFADDR, &ifr)) { /* delete IF addr */
	  perror("SIOCDIFADDR_IN61");
	}
      }
      close(s6);
	  
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
    }
#endif /* IP6 */

#if 0 /////////
    // Shut down interface so it can be reinitialized
    ifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
    if (ioctl(s, SIOCSIFFLAGS, &ifr)) { /* set ifnet flags */
        perror("SIOCSIFFLAGS down");
        goto out;
    }
#endif

    retcode = true;
 
    if ( 0 != *pstate ) // preserve initial state
        *pstate = DHCPSTATE_INIT;

    
 out:
    if (s != -1)
    {
        close(s);
		
#ifdef ECOS_DBG_STAT
   dbg_stat_add(dbg_dhcpc_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
    }

    return retcode;
}

// ------------------------------------------------------------------------
// Release (relinquish) a leased address - if we have one - and bring down
// the interface.
#if 0
int
do_dhcp_release(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease)
{
    if ( 0                           != *pstate
         && DHCPSTATE_INIT           != *pstate
         && DHCPSTATE_NOTBOUND       != *pstate
         && DHCPSTATE_FAILED         != *pstate
         && DHCPSTATE_BOOTP_FALLBACK != *pstate ) {
        *pstate = DHCPSTATE_DO_RELEASE;
        do_dhcp( intf, res, pstate, lease ); // to send the release packet
        cyg_thread_delay( 100 );             // to let it leave the building
    }
    return true;
}
#endif

// ------------------------------------------------------------------------

#if 0
void init_dhcpc_args(struct dhcpc_args *dargs, char *if_name, 
	struct bootp *bootp_data, cyg_sem_t dhcp_needs_attention)
{
	if(!dargs || !if_name)
		return;
	
	strcpy(dargs->if_name, if_name);
	dargs->bootp_data=bootp_data;
	dargs->dhcpstate=0;
	dargs->lease.needs_attention=&dhcp_needs_attention;
	dargs->lease.t1=0;
	dargs->lease.t2=0;
	dargs->lease.expiry=0;
	dargs->lease.next=0;
	dargs->lease.which=0;
	dargs->lease.alarm=0;
}
#endif

#if 0
void reinit_wandhcpc(void)
{
	if(wandhcpc_running)
		eth1_dhcpstate=DHCPSTATE_INIT;
}

void reinit_landhcpc(void)
{
	if(landhcpc_running)
		eth0_dhcpstate=DHCPSTATE_INIT;
}
#endif

void dhcpc_reconnect(cyg_uint8 flag)
{
	recon_flag=flag;
}

void dhcpc_set_wan_status(cyg_uint8 flag)
{
	wan_status_flag=flag;   //down:1,  up:2
}

#if 0
void dhcpc_stop(void)
{
	dhcpc_func_off=1;
}
#endif

//#ifdef HAVE_SYSTEM_REINIT
void kill_wandhcpc()
{
	if(wandhcpc_started && wandhcpc_running)
	{
		wandhcpc_quitting = 1;
		
		while(wandhcpc_quitting)
			cyg_thread_delay(5);
		
		cyg_thread_kill(wandhcpc_thread);
		cyg_thread_delete(wandhcpc_thread);
		//diag_printf("%s:%d####kill wandhcpc_thread!\n",__FUNCTION__,__LINE__);
		wandhcpc_running =0;
		wandhcpc_started=0;
	}
}

void kill_landhcpc()
{
	if(landhcpc_started && landhcpc_running)
	{
		landhcpc_quitting = 1;
		
		while(landhcpc_quitting)
			cyg_thread_delay(5);
		
		cyg_thread_kill(landhcpc_thread);
		cyg_thread_delete(landhcpc_thread);
		
		landhcpc_running =0;
		landhcpc_started=0;
	}
}
//#endif

void dhcpc_main(cyg_addrword_t data)
{
	while(1)
	{
		if(0 ==wandhcpc_quitting && (((cyg_uint8)data) & 0x01)!=0)
		{
			do_dhcp(data);	
			
			if(wandhcpc_quitting)
			{
				//diag_printf("%s:%d####\n",__FUNCTION__,__LINE__);
				wandhcpc_quitting=0;				
				return;
			}			
		}	
		
		if(0 ==landhcpc_quitting && (((cyg_uint8)data) & 0x01)==0) 
		{
			do_dhcp(data);	
			
			if(landhcpc_quitting)
			{				
				landhcpc_quitting=0;
				return;
			}			
		}	
		sleep(1);
	}
}

#if 0 //defined(CONFIG_8881A_UPGRADE_KIT)
void dhcpc_kill(void)
{
	cyg_thread_kill(landhcpc_thread);
}
#endif

__externC int wandhcpc_startup(char *if_name, int dns_mode)
{
	diag_printf("enter dhcpc_startup\n");
	if (wandhcpc_started==1)
	{
		/*if restarted alreadly. let dhcpc func on*/
		#if 0
		while(!dhcpc_cleaned) {
			/*wait to clean*/
			sleep(1);
		}		
		wan_dhcpstate=0;
		dhcpc_func_off=0;
		#endif
		diag_printf("DHCPC has already been startup\n");
		return(-1);
	}	
	unsigned char flag=0;
	flag |= 0x01;
	if(dns_mode==0) //auto dns
		flag |= 0x02;
	sprintf(wan_ifname, "dhcpc %s", if_name);
	if (wandhcpc_running==0)
	{
		cyg_thread_create(DHCPC_THREAD_PRIORITY,
		dhcpc_main,
		flag,
		wan_ifname,
		wandhcpc_stack,
		sizeof(wandhcpc_stack),
		&wandhcpc_thread,
		&wandhcpc_thread_object);
		
		diag_printf("Starting WAN DHCPC thread\n");
		cyg_thread_resume(wandhcpc_thread);
		wandhcpc_started=1;
		//dhcpc_func_off=0;
		return(0);
	}
	else
	{
		diag_printf("WAN DHCPC is already running\n");
		return(-1);
	}
}

__externC int landhcpc_startup(char *if_name, int dns_mode)
{	
	diag_printf("enter dhcpc_startup\n");
	if (landhcpc_started==1)
	{
		diag_printf("DHCPC has already been startup\n");
		return(-1);
	}
	unsigned char flag=0;
	if(dns_mode==0) //auto dns
		flag |= 0x02;
	sprintf(lan_ifname, "dhcpc %s", if_name);
	if (landhcpc_running==0)
	{
		cyg_thread_create(DHCPC_THREAD_PRIORITY,
		dhcpc_main,
		flag,
		lan_ifname,
		landhcpc_stack,
		sizeof(landhcpc_stack),
		&landhcpc_thread,
		&landhcpc_thread_object);
		
		diag_printf("Starting LAN DHCPC thread\n");
		cyg_thread_resume(landhcpc_thread);
		landhcpc_started=1;
		return(0);
	}
	else
	{
		diag_printf("LAN DHCPC is already running\n");
		return(-1);
	}
}


#endif // CYGPKG_NET_DHCP

// EOF dhcp_prot.c
