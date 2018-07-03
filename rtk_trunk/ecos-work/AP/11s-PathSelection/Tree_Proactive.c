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


#include <time.h>
#include <sys/time.h>

#include "pathselection.h"
#include "Tree.h"
#include "util_PathUpdate.h"



#pragma pack(1)
//#define Tree_debug 1


#ifdef _RTL8187B_
int isPortal = 0;
int toBeRoot = 0;
#else
//int isPortal = 1;
int isPortal = 0;
int toBeRoot = 0;
#endif

int isRoot = 0;
static int rootDecision = 0;               // have made decision to root


typedef struct _timer_instance_{
	unsigned int c_count; //current count
	unsigned int p_count; //period count for restoring to current count
	void (*func)(void);
} ti_t;
ti_t i_timer[MAX_TIMER_INSTANCE] = INIT_TIMER_INSTANCE();

pann_mpp_t pann_mpp_pool[MAX_MPP_NUM];
pann_retx_t pann_retx_pool[MAX_MPP_NUM];
struct _rann_root_entry_ rann_root_entry;
struct _rann_retx_entry_ rann_retx_entry;



/****************************************************************************
 *
 * NAME: gen_rreq_ttl_1
 *
 * DESCRIPTION: generate RREQ with "ttl = 1" and broadcast it.
 *
 * PARAMETERS:      Name           Description
 *                  My_Address     my mac address
 *                  DesAdd         the destination's MAC address of RREQ
 *                  TTL            time to live
 *                  SeqNum         sequence number
 * RETURNS:         1
 *
 ****************************************************************************/
/*
int gen_rreq_ttl_1(unsigned char *My_Address,unsigned char *DesAdd,unsigned char TTL,unsigned short SeqNum)
{
	struct rreq  ptr;

	ptr.Category = 5;
	ptr.Action = 2;
	ptr.Id = 30;    //T.B.D
	ptr.Length = sizeof(struct rreq) - 2;
	ptr.ModeFlags = 1;
	ptr.HopCount = 0;
	ptr.ttl = 1;
	//ptr.DesCount     =1;
	ptr.RreqId = My_RreqId;
	memcpy(ptr.SourceAddress,My_Address,6);
	ptr.SourceSeqNum = htonl(My_SeqNum);
	ptr.Lifetime = htonl(255);
	ptr.Metric = htonl(0);
	ptr.first.DO = 0;           //initial DO=0; RF=1;
	ptr.first.RF = 1;
	ptr.first.Reserved = 0;
	memcpy(ptr.first.DesAddress,DesAdd,6);
	ptr.first.DesSeqNum = htonl(0);     //if maintain ,this is not 0

	My_SeqNum++;
	My_RreqId++;

	// @@@ Be careful, SeqNum is a two bytes number
	send_packet((unsigned char*)&ptr, My_Address, DesAdd, My_Address, 1, TTL, SeqNum);   //1 for broadcast

	return 1;
}
*/

void reset_timer()
{
	int i=0;
	for(i=0; i< MAX_TIMER_INSTANCE ; i++)
		if( i == 0 || i == 3 || i ==6 )
			i_timer[i].c_count = i_timer[i].p_count;
}


/****************************************************************************
 *
 * NAME: pann_handler
 *
 * DESCRIPTION: Per second execute func¡ABasis device (mpp, root, ap) execute other func
 *
 * PARAMETERS:      Name           Description
 * None.                 
 *
 * RETURNS:         1
 * None.
 ****************************************************************************/
void pann_handler(){
	int i = 0;
	if(isRoot == 0 && isPortal == 0){
		for(i = 0; i < MAX_TIMER_INSTANCE - 1; i++){
			if(i == 0 || i == 3) continue;
			i_timer[i].c_count--;
			if(i_timer[i].c_count == 0){
				i_timer[i].func();
				i_timer[i].c_count = i_timer[i].p_count;
			}
		}
	}
	if(isRoot == 0 && isPortal == 1){ 
		// portal
		for(i = 0; i < MAX_TIMER_INSTANCE; i++){
			if(i == 3) continue;          // portal don't gen RANN
			if(i == 6 && rootDecision == 1) continue; // have made decision to root
			i_timer[i].c_count--;
			if(i_timer[i].c_count == 0){
				i_timer[i].func();
				i_timer[i].c_count = i_timer[i].p_count;
			}
		}
	}
	else if(isRoot == 1 && isPortal == 1){			 // root
		for(i = 0; i < 4; i++){         // root don't retx RANN, tbl RANN, root decision
			i_timer[i].c_count--;
			if(i_timer[i].c_count == 0){
				i_timer[i].func();
				i_timer[i].c_count = i_timer[i].p_count;
			}
		}
	}
}

#if defined(CONFIG_RTL_MESH_AUTOPORTAL_SUPPORT)

/****************************************************************************
 *
 * NAME: init_pann
 *
 * DESCRIPTION: tree-base initialization.
 *
 * PARAMETERS:      Name           Description
 *                  My_Address     my mac address
 *                  DesAdd         the destination's MAC address of RREQ
 *                  TTL            time to live
 *                  SeqNum         sequence number
 * RETURNS:         1
 *
 ****************************************************************************/
int reset_pann_from_driver(){
	
	unsigned char z6[]={0,0,0,0,0,0};

#ifdef Tree_debug
	printf("init_pann!!!!!!!!!!!!!!!!!!!!!\n");
#endif
    memset(pann_retx_pool, 0, sizeof(pann_retx_pool));
    memset(pann_mpp_pool, 0, sizeof(pann_mpp_pool));
	ini_route_setting(&isPortal,&toBeRoot);
	update_root_info(z6);
    return 0;
}
#endif
/****************************************************************************
 *
 * NAME: root_decision
 *
 * DESCRIPTION: portals decide a root base on mac address in PANNs when mesh has no root.
 *
 * PARAMETERS:      Name           Description
 * None.
 *                 
 * RETURNS:         
 * None.
 *
 ****************************************************************************/
void root_decision(){

#ifdef Tree_debug	
    printf("root_decision!!!!!!!!!!><!!!!!!!\n");
    fflush(stdout);
#endif

    int i;
    int n = 0;                  // check if any portal wanna be root
    unsigned char root[MACADDRLEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};  // record who is the root

    if(rootDecision == 1) {
        return;
    }

    /*for(i = 0; i < portal_n; ){
    	count++;
    	if(pann_mpp_pool[count].flag == 0){
    		continue;
    	}
    	if(pann_mpp_pool[count].beRoot == 1) {
    		n++;
    	}
    	i++;
        }*/
    for(i = 0; i < MAX_MPP_NUM; i++){
        if(pann_mpp_pool[i].flag == 0){
            continue;
        }
        if(pann_mpp_pool[i].beRoot == 1) {
            n++;
        }
    }
    if(toBeRoot == 0){

            /*if(n == 0){             // no portal wanna be root, enter AODV mode.
            }
            else */ 
        if (n > 0){         // root is not me, keep route to root
            // Chris: unnecessary decision. removed
            /*if(n == 1){
                    //for(i = 0; i < portal_n; ){
                    for(i = 0; i <MAX_MPP_NUM ; i++){
                        if(pann_mpp_pool[i].flag == 0){
                            continue;
                        }
                        if(pann_mpp_pool[i].beRoot == 1){
                            memcpy(root, pann_mpp_pool[i].mac , MACADDRLEN);
                        }
                        }
                    }
                    else{*/
            //for(i = 0; i < portal_n; ){
            for(i = 0; i <MAX_MPP_NUM ; i++){
                if(pann_mpp_pool[i].flag == 0){
                    continue;
                }
                if(pann_mpp_pool[i].beRoot != 1){
                    continue;
                }
                if(memcmp(pann_mpp_pool[i].mac, root, MACADDRLEN) < 0){
                    memcpy(root, pann_mpp_pool[i].mac, MACADDRLEN);
                }
            }
        //}
        }
    }
    else if(toBeRoot == 1){
        if(n == 0){             //I am the root
            isRoot = 1;
#ifdef Tree_debug
            printf("2. I am ROOT!!!!!!!!!!!!!!!\n");
#endif
        }
        else if(n > 0){
            if(memcmp(my_address, root, MACADDRLEN) < 0){
                memcpy(root, my_address, MACADDRLEN);
            }
            for(i = 0; i < MAX_MPP_NUM; i++){
                if(pann_mpp_pool[i].flag == 0){
                    continue;
                }
                if(pann_mpp_pool[i].beRoot != 1) {
                    continue;
                }
                if(memcmp(pann_mpp_pool[i].mac, root, MACADDRLEN) < 0){
                    memcpy(root, pann_mpp_pool[i].mac, MACADDRLEN);
                }
            }
            if(memcmp(root, my_address, MACADDRLEN) == 0){
                isRoot = 1;
#ifdef Tree_debug
                printf("3. I am ROOT!!!!!!!!!!!!\n");
#endif
            }
        }
    }
    rootDecision = 1;
    if(isRoot == 1){
        memcpy(rann_root_entry.mac, my_address, MACADDRLEN);
        rann_root_entry.timeout = ROOT_ANNOUNCEMENT_TIMEOUT;
        rann_root_entry.flag = 1;

        update_root_info(my_address);
    }
}

/****************************************************************************
 *
 * NAME: gen_pann
 *
 * DESCRIPTION: generate PANN (portal or root) and broadcast it.
 *
 * PARAMETERS:      Name           Description
 * None.                 
 * 
 * RETURNS:         
 * None.
 *
 ****************************************************************************/
void gen_pann(){
#ifdef Tree_debug
	printf("GEN_PANN!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif

	struct pann ptr;
	unsigned char nexthop_mac_addr[MACADDRLEN] = {0};

	ptr.category       = _MESH_CATEGORY_ID_;
	ptr.action         = _GANN_ACTION_ID_;
	ptr.eid            = _MESH_GANN_IE_;
	ptr.length         = (sizeof(struct pann) - 2);
	if(toBeRoot == 1){
		ptr.flag = 1;
	}
	else{
		ptr.flag = 0;
	}
	ptr.hopCount = 0;
	ptr.ttl = _MESH_HEADER_TTL_;
	memcpy(ptr.origAddr, my_address, MACADDRLEN);
        
	ptr.seqNum = htonl(My_SeqNum);

	ptr.metric = htonl(0);
	
	send_packet((unsigned char*)&ptr, my_address, nexthop_mac_addr, my_address, 1, ptr.ttl, htons(My_SeqNum));  // broadcast -> flag = 1

	My_SeqNum++;

	return;
}

static void updateMppPool(struct pann *pann)
{
    int i;
    int first = -1;

    // check whether the mac of the pann (parameter) exist , and reset the timeout time if it really does.
    for(i = 0; i < MAX_MPP_NUM; i++){
        if(pann_mpp_pool[i].flag == 1){
            if(memcmp(pann_mpp_pool[i].mac, pann->origAddr, MACADDRLEN) == 0){
                first = -1;
                pann_mpp_pool[i].timeout = PORTAL_ANNOUNCEMENT_TIMEOUT;
                pann_mpp_pool[i].seqNum = pann->seqNum;	
                pann_mpp_pool[i].flag = 1;
                pann_mpp_pool[i].beRoot = pann->flag;
                break;
            }
        }
        else if(first == -1){
            first = i;
        }
    }

    // record the the mac of the pann(parameter), if it doesn't exist
    if(first >= 0){
        memcpy(pann_mpp_pool[first].mac, pann->origAddr, MACADDRLEN);
        pann_mpp_pool[first].timeout = PORTAL_ANNOUNCEMENT_TIMEOUT;
        pann_mpp_pool[first].seqNum = pann->seqNum;
        pann_mpp_pool[first].flag = 1;
        pann_mpp_pool[first].beRoot = pann->flag;
    }
}

static void updatePannRetx(struct pann *pann)
{
    int i;
    int first = -1;

    for(i = 0; i < MAX_MPP_NUM; i++){
        if(pann_retx_pool[i].flag == 1){
            if(memcmp(pann_retx_pool[i].pann.origAddr, pann->origAddr, MACADDRLEN) == 0){    //existence
                first = -1;
                if(ntohl(pann_retx_pool[i].pann.seqNum) < pann->seqNum){
                    memcpy((void *)&pann_retx_pool[i].pann, pann, sizeof(pann_retx_pool[i].pann));
                    pann_retx_pool[i].pann.seqNum = htonl(pann->seqNum);
                    pann_retx_pool[i].pann.metric = htonl(pann->metric);
                    pann_retx_pool[i].timeout = PORTAL_PROPAGATION_DELAY;
                    pann_retx_pool[i].flag = 1;
                }

                break;
            }
        }
        else if(first == -1){
            first = i;
        }
    }
			
    if(first >= 0){
        memcpy((void *)&pann_retx_pool[first].pann, pann, sizeof(pann_retx_pool[first].pann));
        pann_retx_pool[first].pann.seqNum = htonl(pann->seqNum);
        pann_retx_pool[first].pann.metric = htonl(pann->metric);
        pann_retx_pool[first].timeout = PORTAL_PROPAGATION_DELAY;
        pann_retx_pool[first].flag = 1;
#ifdef Tree_debug
        printf("put into retx[%d] = %02X-%02X-%02X-%02X-%02X-%02X\n", first,
        pann->origAddr[0], pann->origAddr[1], pann->origAddr[2],
        pann->origAddr[3], pann->origAddr[4], pann->origAddr[5]);
#endif
    }
	
}

/****************************************************************************
 *
 * NAME: recv_pann
 *
 * DESCRIPTION: update portal info, retransmit table and keep route to portal when receive PANN.
 *
 * PARAMETERS:      Name           Description
 *                  pann           the pann you received
 *                  My_Prehop      recore who relayed pann to you
 *                  metric         
 *                  My_interFace
 *                  TTL
 *                  SeqNum
 *
 * RETURNS:       0  
 *
 ****************************************************************************/
int recv_pann(struct pann *pann, unsigned char* My_Prehop, unsigned int metric, int My_interFace, int TTL, int SeqNum){
    pann->metric = ntohl(pann->metric)+ metric;
    pann->seqNum = ntohl(pann->seqNum);
    ++pann->hopCount;
    --pann->ttl;
	if((memcmp(pann->origAddr, my_address ,MACADDRLEN)) ==0) {
        return 0;            // get the packet from myself
    }

    /* check the freshness and the goodness of metric*/
    if(updatePathByFreshness(pann->origAddr, pann->seqNum,
                            pann->metric, pann->hopCount, My_Prehop, 
                            My_interFace)==0 ) {
        return 0;
    }

    updateMppPool(pann);
    updatePannRetx(pann);			
    
    return 3;
}
/****************************************************************************
 *
 * NAME: retx_pann
 *
 * DESCRIPTION: rebroadcast the pann produced by the portals
 *
 * PARAMETERS:      Name           Description
 * None.
 *
 * RETURNS:         
 * None.
 *
 ****************************************************************************/
void retx_pann(){
   int i;
	unsigned char nexthop_mac_addr[MACADDRLEN] = {0};

#ifdef Tree_debug
    printf("\n    --retransmit package---\n");
#endif
    for(i = 0; i < MAX_MPP_NUM; i++){

        if(pann_retx_pool[i].flag == 1){
            pann_retx_pool[i].timeout--;
#ifdef Tree_debug
            printf("        %d %02X-%02X-%02X-%02X-%02X-%02X\n", pann_retx_pool[i].timeout,
                pann_retx_pool[i].pann.origAddr[0], pann_retx_pool[i].pann.origAddr[1], pann_retx_pool[i].pann.origAddr[2],
                pann_retx_pool[i].pann.origAddr[3], pann_retx_pool[i].pann.origAddr[4], pann_retx_pool[i].pann.origAddr[5]);
#endif
            if(pann_retx_pool[i].timeout == 0){
                pann_retx_pool[i].flag = 0;

            if(pann_retx_pool[i].pann.ttl <= 0){  // ttl <= 0, discard PANN
                return;
            }

#ifdef Tree_debug				
                printf("PANN_RETX\n");
#endif
				send_packet( (unsigned char *)&pann_retx_pool[i].pann , my_address, nexthop_mac_addr, my_address, 1, pann_retx_pool[i].pann.ttl, htons(ntohl(pann_retx_pool[i].pann.seqNum)));
            }
        }
    }
}


//  add by mcinnis 20070414
void tbl_pann(){
    int i;
    int query_result = 0;
    struct iwreq          wrq;
#ifdef Tree_debug
    printf("\n    --print mac table---\n");
#endif
    for(i = 0; i < MAX_MPP_NUM; i++){
        if(pann_mpp_pool[i].flag == 1){
            pann_mpp_pool[i].timeout--;

            if(pann_mpp_pool[i].timeout == 0){
                struct path_sel_entry Entry;
                pann_mpp_pool[i].flag = 0;
                memset((void*)&Entry,0,sizeof(struct path_sel_entry));
                query_result = query_table(pann_mpp_pool[i].mac, &Entry);
                if(query_result > 0) {
                    /*
                                    Entry.isvalid = 0;
                                    memcpy(Entry.destMAC, pann_mpp_pool[i].mac, MACADDRLEN);
                                    if(modify_table(Entry.destMAC, &Entry) < 0) {
                                    // printf("at tree_ondemand.c modify table error\n");
                                    }
                                    */
                    remove_table(pann_mpp_pool[i].mac);
                }
            }
#ifdef Tree_debug
            printf("        %d %02X-%02X-%02X-%02X-%02X-%02X\n", pann_mpp_pool[i].timeout,
            pann_mpp_pool[i].mac[0], pann_mpp_pool[i].mac[1], pann_mpp_pool[i].mac[2],
            pann_mpp_pool[i].mac[3], pann_mpp_pool[i].mac[4], pann_mpp_pool[i].mac[5]);
#endif
        }
    }
		
    /* Get wireless name */
    memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
    strncpy(wrq.ifr_name, interface_ioctl , IFNAMSIZ);

    wrq.u.data.length =  sizeof( pann_mpp_t) * MAX_MPP_NUM;
    if((wrq.u.data.pointer = (caddr_t)malloc(wrq.u.data.length))==NULL)
    {
        // printf("Create_table_Entry  Error:%s\n\a",strerror(errno)); 
        return;
    }

    memcpy( wrq.u.data.pointer, pann_mpp_pool, sizeof( pann_mpp_t) * MAX_MPP_NUM );

    if(ioctl(g_ioctrl_socket_fd, SET_PORTAL_POOL, &wrq) < 0)
    {
        // If no wireless name : no wireless extensions
        free(wrq.u.data.pointer);
        // printf("Create_table_Entry  Error:%s\n\a",strerror(errno)); 
        return;
    }
    free(wrq.u.data.pointer);

}


/****************************************************************************
 *
 * NAME: gen_rann
 *
 * DESCRIPTION: generate rann (root) and broadcast it.
 *
 * PARAMETERS:      Name           Description
 * None.
 *
 * RETURNS:         
 * None.
 *
 ****************************************************************************/
void gen_rann(){
#ifdef Tree_debug
    printf("GEN_RANN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif

	unsigned char nexthop_mac_addr[MACADDRLEN] = {0};
	struct rann ptr;
   
    ptr.category     = _MESH_CATEGORY_ID_;
    ptr.action       = _HWMP_MESH_PATHSEL_ACTION_ID_;
    ptr.Id           = _MESH_RANN_IE_;
    ptr.length       = (sizeof(struct rann) - 2);
    ptr.flag         = 0;
    ptr.hopCount     = 0;
    ptr.ttl          = _MESH_HEADER_TTL_;
    memcpy(ptr.origAddr, my_address, MACADDRLEN);	
    ptr.seqNum = htonl(My_SeqNum);        
    ptr.metric = htonl(0);
	send_packet((unsigned char*)&ptr, my_address, nexthop_mac_addr, my_address, 1, ptr.ttl, htons(My_SeqNum));	// flag = 1, for broadcast

	rann_root_entry.timeout = ROOT_ANNOUNCEMENT_TIMEOUT;
	rann_root_entry.seqNum = My_SeqNum;

	My_SeqNum++;
	
    return;
}
/****************************************************************************
 *
 * NAME: retx_rann
 *
 * DESCRIPTION: rebroadcast the rann produced by the portals or root
 *
 * PARAMETERS:      Name           Description
 * None.
 *
 * RETURNS:         
 * None.
 *
 ****************************************************************************/
void retx_rann(){
    unsigned char nexthop_mac_addr[MACADDRLEN] = {0};

#ifdef Tree_debug
    printf("\n    --retransmit RANN package---\n");
#endif
    if(rann_retx_entry.flag == 1){
        rann_retx_entry.timeout--;
#ifdef Tree_debug
        printf("        %d %02X-%02X-%02X-%02X-%02X-%02X\n", rann_retx_entry.timeout,
        rann_retx_entry.rann.origAddr[0], rann_retx_entry.rann.origAddr[1], rann_retx_entry.rann.origAddr[2],
        rann_retx_entry.rann.origAddr[3],rann_retx_entry.rann.origAddr[4],rann_retx_entry.rann.origAddr[5]);
#endif
        if(rann_retx_entry.timeout == 0){
            rann_retx_entry.flag = 0;
#ifdef Tree_debug
            printf("RANN RETX\n");
#endif
            if(rann_retx_entry.rann.ttl <= 0){ // ttl <= 0, discard RANN
                return;
            }
			    printf("\n\n\n\n*************%s %s %d ******************\n",__FILE__,__FUNCTION__,__LINE__);	
            send_packet((unsigned char*)&rann_retx_entry.rann, my_address, nexthop_mac_addr, my_address, 1, rann_retx_entry.rann.ttl, htons(ntohl(rann_retx_entry.rann.seqNum)));
        }
    }
}
/****************************************************************************
 *
 * NAME: tbl_rann
 *
 * DESCRIPTION: manage the root info.
 *
 * PARAMETERS:      Name           Description
 * None.
 *
 * RETURNS:         
 * None.
 *
 ****************************************************************************/
void tbl_rann(){
#ifdef Tree_debug
    printf("\n    --print RANN mac table---\n");
#endif
    // int query_result = 0;
    if(rann_root_entry.flag == 1){
        rann_root_entry.timeout--;
        if(rann_root_entry.timeout == 0){

            remove_table(rann_root_entry.mac);

            rann_root_entry.flag = 0;
            //if(isPortal == 1){
            rootDecision = 0;
            //}
            memset(rann_root_entry.mac, 0, MACADDRLEN);
            update_root_info(rann_root_entry.mac);   // clear root mac info to driver chuangch 2007/07/10 
#ifdef Tree_debug
            printf("tbl_rann reset root mac,rootDecision,portal_n done\n");
#endif
        }
#ifdef Tree_debug
        printf(">>>        %d %02X-%02X-%02X-%02X-%02X-%02X\n", rann_root_entry.timeout,
        rann_root_entry.mac[0], rann_root_entry.mac[1], rann_root_entry.mac[2],
        rann_root_entry.mac[3], rann_root_entry.mac[4], rann_root_entry.mac[5]);
#endif
    }
}
/****************************************************************************
 *
 * NAME: recv_pann
 *
 * DESCRIPTION: update root info, retransmit table and keep route to root when receive RANN.
 *
 * PARAMETERS:      Name           Description
 *                  pann           the pann you received
 *                  My_Prehop      recore who relayed rann to you
 *                  metric         
 *                  My_interFace
 *                  TTL
 *                  SeqNum
 *
 * RETURNS:       0  
 *
 ****************************************************************************/
int recv_rann(struct rann *rann, unsigned char* My_Prehop, unsigned int metric, int My_interFace, int TTL, int SeqNum){
    int update_retx = 0;
    rann->metric = ntohl(rann->metric)+ metric;
    rann->seqNum = ntohl(rann->seqNum);
    rann->lifetime = ntohl(rann->lifetime);
    ++rann->hopCount;
    --rann->ttl;
        
    if((memcmp(rann->origAddr, my_address ,MACADDRLEN)) == 0) {
        //  printf("get self RANN packet!!!");
        return 0;            // get the packet from myself
    }           

    if(isRoot == 1) {
        if(memcmp(rann->origAddr, my_address, MACADDRLEN) < 0){ // has more apropriate ROOT in mesh
            printf("WARNING - There are more than one root in mesh, I quit.");
            isRoot = 0;
        }
        else{
            printf("WARNING - There are more than one root in mesh, I'm an apropriate root.\n");
            return 0;
        }
    }   
    
    // check whether the mac of the rann (parameter) exist , and reset the timeout time if it really does.
    if(rann_root_entry.flag == 1){              // already has root info
        if( memcmp( rann->origAddr, rann_root_entry.mac, MACADDRLEN ) < 0 ){ // sender of RANN is different from my root info
            memcpy(rann_root_entry.mac, rann->origAddr, MACADDRLEN);                
            rann_root_entry.timeout = ROOT_ANNOUNCEMENT_TIMEOUT;
            rann_root_entry.seqNum = rann->seqNum;
            rann_root_entry.flag = 1;           
            rootDecision = 1;
            update_retx = 1;
            update_root_info(rann_root_entry.mac); // update root mac into driver

        } // memcmp < 0
        else if( memcmp( rann_root_entry.mac, rann->origAddr, MACADDRLEN ) == 0 )
        {
            update_retx = 1;
            if(rann_root_entry.seqNum < rann->seqNum){
                rann_root_entry.timeout = ROOT_ANNOUNCEMENT_TIMEOUT;
                rann_root_entry.seqNum = rann->seqNum;
                rann_root_entry.flag = 1; 
            }    
        } // memcmp = 0
    } // flag = 1
    else if(rann_root_entry.flag == 0){ // doesn't has root info
        memcpy(rann_root_entry.mac, rann->origAddr, MACADDRLEN);            
        rann_root_entry.timeout = ROOT_ANNOUNCEMENT_TIMEOUT;
        rann_root_entry.seqNum = rann->seqNum;
        rann_root_entry.flag = 1;         
        rootDecision = 1;   
        update_retx = 1;
        update_root_info(rann_root_entry.mac); // update root mac into driver
    } // flag = 0


    if(update_retx > 0) {
        if(updatePathByFreshness(rann->origAddr, rann->seqNum,
                                rann->metric, rann->hopCount, My_Prehop, 
                                My_interFace)==0 ) {
            return 0;
        }

        memcpy((void *)&rann_retx_entry.rann, rann, sizeof(rann_retx_entry.rann));
        rann_retx_entry.rann.seqNum = htonl(rann->seqNum);
        rann_retx_entry.rann.metric = htonl(rann->metric);
        rann_retx_entry.rann.lifetime = htonl(rann->lifetime);
        rann_retx_entry.timeout = ROOT_PROPAGATION_DELAY;
        rann_retx_entry.flag = 1;
    }
    return 0;
}

