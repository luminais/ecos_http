#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>


#ifndef __ECOS
#include <linux/wireless.h>
#else
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif


#ifndef __ECOS
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif
#else
#include <net/if_dl.h>
#endif



#include <time.h>
#include <sys/time.h>

#ifdef _RTL8187B_
#include "1x_pathsel.h"
#endif

#ifdef _RTL8186_
#include "../auth/include/1x_ioctl.h"
#endif

#include "common.h"
#include "pathselection.h"
#include "util_PathUpdate.h"

#pragma pack(1)  

	
// betterPath: return better  when x.metric < y.metric or (x.metric = y.metric but x.hopcount < y.hopcount)
#define betterPath(xmetric, xhop, ymetric, yhop) ( ((xmetric + pathSwitchThreshold) < ymetric) || ( (xmetric <= ymetric) && (xhop < yhop) ) )
const unsigned char broadcast_mac_addr[MACADDRLEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#ifndef __ECOS
static int get_iface_index(int fd, const char* interface_name)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy (ifr.ifr_name,interface_name);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        return (-1);
    }
    return ifr.ifr_ifindex;
}

static void send_rawsocket(char *meshframe, int len, char* src_mac_addr)
{
    struct sockaddr_ll sll; // target address
    int send_result = 0;
    memset(&sll, 0, sizeof(sll));
    sll.sll_ifindex = get_iface_index(g_rawsocket_fd, interface_rawsocket);
    // address length
    sll.sll_halen = ETH_ALEN; // 6
    sll.sll_family = PF_PACKET;

    // MAC - begin
    memset(sll.sll_addr, 0, 8);
    memcpy(sll.sll_addr, src_mac_addr, 6);
    // MAC - end

    sll.sll_protocol = 0;
    sll.sll_hatype = 0;
    sll.sll_pkttype = 0;

    // send the packet
    if ((send_result = sendto(g_rawsocket_fd, meshframe, len, 0, (struct sockaddr*)&sll, sizeof(sll))) == -1) {
        // fprintf(stderr, "Error: can't sendto()\n");
    } else {
        //printf(".");
    }
    //  signal(SIGALRM, test_rawsocket_send);
    return;
}
#endif

static int modify_table( struct path_sel_entry *pEntry_in )
{
	struct iwreq          wrq;
	
#ifndef _RTL865X_
	char buf[MACADDRLEN + sizeof(struct path_sel_entry)];
	struct path_sel_entry Entry;
	memcpy(Entry.destMAC, pEntry_in->destMAC, MACADDRLEN);
	memcpy(Entry.nexthopMAC, pEntry_in->nexthopMAC, MACADDRLEN);
	Entry.dsn = pEntry_in->dsn; 
	Entry.metric = pEntry_in->metric; 
	Entry.hopcount = pEntry_in->hopcount; 
#endif
	
	/* Get wireless name */
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
 
#ifdef _RTL865X_
	wrq.u.data.length = sizeof(struct path_sel_entry);
	wrq.u.data.pointer = (caddr_t)pEntry_in;
#else
	wrq.u.data.length = sizeof(buf);
	wrq.u.data.pointer = (caddr_t)buf;
	memcpy(buf, Entry.destMAC, MACADDRLEN);
	memcpy(buf+MACADDRLEN, &Entry, sizeof(struct path_sel_entry));
#endif

	if(ioctl(g_ioctrl_socket_fd, SIOCUPATHTABLE, &wrq) < 0)
	{
		// If no wireless name : no wireless extensions
#ifdef GALLARDO_TEST
		printf("Update_table  Error:%s\t\a",strerror(errno)); 
		printMac(pEntry_in->destMAC);
		printf("\n"); 
#endif		
		return(-1);
	}
	
	return 1;
}


static int create_table_entry(struct path_sel_entry *pEntry_in)
{
    struct iwreq          wrq;
    /* Get wireless name */
    memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
    strncpy(wrq.ifr_name, interface_ioctl , IFNAMSIZ);

    wrq.u.data.pointer = (caddr_t)pEntry_in;
    wrq.u.data.length = sizeof(struct path_sel_entry);
    if(ioctl(g_ioctrl_socket_fd, SIOCAPATHTABLE, &wrq) < 0)
    {
		return(-1);
	}
	
	return 1;
}


//write a new destination entry in the path selection table
static void create_table_entryA(unsigned char* desAddress,unsigned int desSeq,unsigned char* nexhop, int interfaceNum,unsigned char hopcount,unsigned int metric)
{ 
	struct path_sel_entry Entry;
	memset((void*)&Entry,0,sizeof(struct path_sel_entry));
	memcpy(Entry.destMAC, desAddress, MACADDRLEN);

//	Entry.isvalid=1;
	Entry.dsn=desSeq;
	memcpy(Entry.nexthopMAC, nexhop, MACADDRLEN);
	Entry.metric=metric;
	Entry.hopcount= hopcount;
//	memset(Entry.modify_time,0,8);

	if(create_table_entry(&Entry) < 0) {
		// printf("at path_selec.c create table error\n");
	}

	return;	
}


int query_table(unsigned char *destaddr, struct path_sel_entry *pEntry_out)
{

    struct iwreq          wrq;

    /* Get wireless name */
    memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
    strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);

    wrq.u.data.pointer = (caddr_t)pEntry_out;
    wrq.u.data.length = sizeof(struct path_sel_entry);

    memset((void*)pEntry_out, 0, sizeof(struct path_sel_entry));
    memcpy(wrq.u.data.pointer, destaddr, MACADDRLEN);

    if(ioctl(g_ioctrl_socket_fd, SIOCQPATHTABLE, &wrq) < 0)
    {
        // If no wireless name : no wireless extensions
        return(-2);
    }

    return 1;
}

int remove_table(unsigned char* invalid_addr)
{
    char buf[MACADDRLEN];
    struct iwreq          wrq;

    /* Get wireless name */
    memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
    strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
    wrq.u.data.pointer = (caddr_t) buf;
    wrq.u.data.length = sizeof(buf);			
    memcpy(wrq.u.data.pointer, invalid_addr, MACADDRLEN);
    		

    //clear conresponding path table entry
    if(ioctl(g_ioctrl_socket_fd, REMOVE_PATH_ENTRY, &wrq) < 0) {
        // If no wireless name : no wireless extensions
        // printf("dump_path_table  Error:%s\n\a",strerror(errno)); 
        return(1);
    }

    return 0;
}


// Design logic:
//    Check the freshness of tuple {src, dest, DSN} and the goodness of metric
//         3a: if not fresh or equal freshness with worse metric, drop
//         3b: if equal freshness with better metric, (pending) update path to src
//         3c: if fresher record and better metric, (pending) update path to src
//         3d: if fresher record but worse metric with the same prehop, (pending) update path to src
//         3e: if fresher record but worse metric with different prehop, (pending) update path to src
//         3f: if no entry existed in the driver, immediately update path to src
//
//     Exception 1: an MP has rebooted and driver's table was cleaned, so the dsn is smaller than current stored one
//     Exception 2: an mpp/root has just rebooted and driver's table contains an entry inserted by PANN/RANN 
//     Exception 3: (not handled now) dsn is larger than max value of unsigned int
// 
// Parameters:
//   src: for RREQ, src is rreq's issuer;  for RREP, src is the rrep's issuer. 
//   dest always the other end opposed to src
//   dsn: related to src  (so, it can be shared by rreq, rrep, ...)
//   My_Prehop: pre-hop to src
//
int updatePathByFreshness(unsigned char *destMAC, unsigned int dsn, 
	   unsigned int metric, unsigned char hopcount, unsigned char *My_Prehop,
	   int My_interFace)
{

    int qResult;
    struct path_sel_entry pathEntry;

    #if 0
    /*always update path to pre-hop ('cos it is almost a near real-time metric) */
    if(memcmp(destMAC, My_Prehop, MACADDRLEN) != 0) {
        qResult = query_table(My_Prehop, &pathEntry);
        if(qResult == -2) {            //need to create table entry for pre-hop
            create_table_entryA(My_Prehop, 0, My_Prehop, My_interFace, 1, metric);
        }
    }
    #endif
    
   	qResult = query_table(destMAC, &pathEntry);
    if(qResult > 0 && pathEntry.dsn <= dsn) {      
        if(memcmp(My_Prehop, pathEntry.nexthopMAC, MACADDRLEN) == 0 || 
            betterPath(metric, hopcount, pathEntry.metric, pathEntry.hopcount)) {   
            
            memcpy(pathEntry.destMAC, destMAC, MACADDRLEN);
            memcpy(pathEntry.nexthopMAC, My_Prehop, MACADDRLEN);	    
            pathEntry.dsn        = dsn;
            pathEntry.metric       = metric;
            pathEntry.hopcount = hopcount;
            modify_table(&pathEntry);          
            return 1;
        }        
    }
    else if(qResult == -2) {/*query table command succeed, but the pathEntry is empty*/
        create_table_entryA(destMAC, dsn, My_Prehop, My_interFace, hopcount, metric);
        return 1;
    }

    return 0;   
}

void send_packet_with_6addr( unsigned char *RBody ,
							 unsigned char *self_mac_addr ,unsigned char *dest_mac_addr ,
							 unsigned char *src_mac_addr ,
							 int flag,unsigned char TTL,
							 unsigned short SeqNum,
							 unsigned char *addr5 ,
							 unsigned char *addr6 
														)
{

    unsigned char *ra, *ta, *da, *sa;
    int zero_offset = 14;
    unsigned char meshframe[256]; // buffer for mesh frame

    if (flag==1) {
        memcpy(dest_mac_addr, broadcast_mac_addr, MACADDRLEN);
    }

    memset(meshframe, 0, sizeof(meshframe));
    meshframe[zero_offset+30] = 0x01;
    meshframe[zero_offset+31] = TTL;
    memcpy(meshframe+32+zero_offset, &SeqNum, 2);

    // Type & Subtype
    SetFrameSubType(meshframe+zero_offset, WIFI_11S_MESH_ACTION);
    SetToDs(meshframe+zero_offset);
    SetFrDs(meshframe+zero_offset);

    // next hop
    ra = GetAddr1Ptr(meshframe+zero_offset);
    memcpy(ra, dest_mac_addr, MACADDRLEN);  /*driver would check next hop later*/

    // transmitter = self
    ta = GetAddr2Ptr(meshframe+zero_offset);
    memcpy(ta, self_mac_addr, MACADDRLEN);

    // destination addr
    da = GetAddr3Ptr(meshframe+zero_offset);
    memcpy(da, dest_mac_addr, MACADDRLEN);

    // source addr
    sa = GetAddr4Ptr(meshframe+zero_offset);
    memcpy(sa, src_mac_addr, MACADDRLEN);

    // add by Jason 2007.04.26
    memcpy(meshframe+zero_offset+WLAN_HDR_A4_MESH_MGT_LEN, addr5, MACADDRLEN);
    memcpy(meshframe+zero_offset+WLAN_HDR_A4_MESH_MGT_LEN+MACADDRLEN, addr6, MACADDRLEN);
    // body[3] is the length field of [RREQ|RREP|RERR|RREP-ACK] IE, but not include the length of Category field and Action field
    memcpy(meshframe + zero_offset + WLAN_HDR_A6_MESH_MGT_LEN, RBody, 2 + RBody[3]);

    int frm_len = zero_offset + WLAN_HDR_A6_MESH_MGT_LEN + 2 + RBody[3];
#ifndef __ECOS
    send_rawsocket((char*)meshframe, frm_len, (char *)src_mac_addr);
#endif
}

void send_packet( unsigned char *RBody ,unsigned char *self_mac_addr ,unsigned char *dest_mac_addr ,unsigned char *src_mac_addr ,int flag,unsigned char TTL,unsigned short SeqNum)
{
    unsigned char *ra, *ta, *da, *sa;
    int zero_offset = 14;
    unsigned char meshframe[256]; // buffer for mesh frame
    unsigned short ether_type;
	unsigned char tmp_type;

    if (flag==1) {
        memcpy(dest_mac_addr, broadcast_mac_addr, MACADDRLEN);
    }


    memset(meshframe, 0, sizeof(meshframe));

    meshframe[zero_offset+31] = TTL;
    memcpy(meshframe+32+zero_offset, &SeqNum, 2);

    // Type & Subtype
    SetFrameSubType(meshframe+zero_offset, WIFI_11S_MESH_ACTION);
    SetToDs(meshframe+zero_offset);
    SetFrDs(meshframe+zero_offset);
	tmp_type = meshframe[zero_offset];
	meshframe[zero_offset] = meshframe[zero_offset+1];
	meshframe[zero_offset+1] = tmp_type;

    // next hop
    ra = GetAddr1Ptr(meshframe+zero_offset);
    memcpy(ra, dest_mac_addr, MACADDRLEN); /*driver would check next hop later*/

    // transmitter = self
    ta = GetAddr2Ptr(meshframe+zero_offset);
    memcpy(ta, self_mac_addr, MACADDRLEN);

    // destination addr
    da = GetAddr3Ptr(meshframe+zero_offset);
    memcpy(da, dest_mac_addr, MACADDRLEN);

    // source addr
    sa = GetAddr4Ptr(meshframe+zero_offset);
    memcpy(sa, src_mac_addr, MACADDRLEN);

    // body[3] is the length field of [RREQ|RREP|RERR|RREP-ACK] IE, but not include the length of Category field and Action field
    memcpy(meshframe + zero_offset + WLAN_HDR_A4_MESH_MGT_LEN, RBody, 2 + RBody[3]);

    int frm_len = zero_offset + WLAN_HDR_A4_MESH_MGT_LEN + 2 + RBody[3];
#ifndef __ECOS
    send_rawsocket((char *)meshframe, frm_len, (char *)src_mac_addr);
#else
	/*printf(" %s %d**************\n",__FUNCTION__,__LINE__);
	int i;
	for(i=0;i< frm_len;i++)
	 	printf("%2X ",*(meshframe+i));
	printf("\n");*/

	extern struct eth_drv_sc rltk819x_wlan_mesh_sc10;
	rltk819x_send_wlan_mesh(&rltk819x_wlan_mesh_sc10, meshframe, frm_len);
#endif
}

