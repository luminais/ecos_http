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

unsigned int My_SeqNum=1;
int My_RreqId=1;
int g_rawsocket_fd=-1;
int g_ioctrl_socket_fd=-1;



unsigned char BroadCast[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
int SeqGap=0;

//for the solution of Sequence Number overflow
/*
int CheckSeq(int StoredSeq,int RecvSeq)
{
	SeqGap=0;
	SeqGap = (int)(RecvSeq - StoredSeq);
	return SeqGap;
}
*/
//-----------------------------------------------------------------------


//int dump_table();   mark by chuangch 2007.09.14

static int is_my_stat(unsigned char addr[MACADDRLEN])
{

#ifndef _RTL8187B_ // 8187 not support it now

    struct iwreq          wrq;
    unsigned char * stat_addr;
#ifdef _RTL865X_	
    static unsigned char buf[MACADDRLEN] = {0}; 
#else
    static unsigned char buf[NUM_STAT*MACADDRLEN] = {0};
#endif


    memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
    strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);

    stat_addr = (unsigned char *)buf;
    memcpy(stat_addr, addr, MACADDRLEN);

    wrq.u.data.pointer = (caddr_t)buf;
    wrq.u.data.length = sizeof(buf);

    if(ioctl(g_ioctrl_socket_fd, GET_STA_LIST, &wrq) < 0) {
        printf("Get station list Error:%s\n\a",strerror(errno)); 
        return(-1);
    }

    if (memcmp(addr, stat_addr, MACADDRLEN) == 0) {
        return 1;
    }
#endif // not 8187
	
	return 0;
}



static int notify_path_found(unsigned char dest_addr[MACADDRLEN])
{	
    char buf[MACADDRLEN];
    struct iwreq          wrq;

    // Get wireless name 
    memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
    strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
    wrq.u.data.length = sizeof(buf);
    wrq.u.data.pointer = (caddr_t)buf;
    memcpy(wrq.u.data.pointer, dest_addr, MACADDRLEN);

    if(ioctl(g_ioctrl_socket_fd, SIOC_NOTIFY_PATH_CREATE, &wrq) < 0)
    {
        // If no wireless name : no wireless extensions
        // printf("Update_table  Error:%s\n\a",strerror(errno)); 
        return(-1);
    }

    return 0;
}


/*
int RERR_Check(unsigned char* NextHop)
{
	//char buf[PATH_SEL_TBL_SIZE*sizeof(struct path_sel_entry)];
	char buf[MACADDRLEN];
	
	struct iwreq          wrq;
	if(g_rawsocket_fd < 0) {
		// printf("***** socket fd in query_table is not ready (daemon msg)\n");
		return -1;
	}
	// Get wireless name 
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
 	
	wrq.u.data.pointer = (caddr_t) buf;
	wrq.u.data.length = sizeof(buf);
	
	memcpy(wrq.u.data.pointer, NextHop, MACADDRLEN);
	
	
	//clear conresponding path table entry
	if(ioctl(g_ioctrl_socket_fd, REMOVE_PATH_ENTRY_NEIGHBOR_DISPEAR, &wrq) < 0) {
		// If no wireless name : no wireless extensions
		// printf("dump_path_table  Error:%s\n\a",strerror(errno)); 
		return(-1);
	}

	return 0;
}
*/

//===============================================================================================================
//mark gen_rrep_ack and recv_rrep_ack by chuangch 2007/04/10
/*
int gen_rrep_ack(struct rrep *input_rrep,unsigned char* My_Address,unsigned char TTL,unsigned short SeqNum)
{
	struct rrep_ack ptr; 
	ptr.Category =5;
	ptr.Action 	=5;
	ptr.Id			= input_rrep->Id;
	ptr.Length			=(sizeof(struct rrep_ack) - 2);
	memcpy(ptr.DesAddress, input_rrep->DesAddress, 6);
	ptr.DesSeqNum		= input_rrep->DesSeqNum;
	memcpy(ptr.SourceAddress, input_rrep->first.SourceAddress, 6);
	ptr.SourceSeqNum	= input_rrep->first.SourceSeqNum;
	send_packet((unsigned char*)&ptr, My_Address, ptr->DesAddress, My_Address, 0, TTL, SeqNum);
	return 1;

}

int recv_rrep_ack(struct rrep_ack *input_rrep_ack, unsigned char* My_Address,unsigned char TTL,unsigned short SeqNum)
{
	//printf("\n\ninput_rrep_ack.DesAddress=%2X-%2X-%2X-%2X-%2X-%2X-\n",input_rrep_ack->DesAddress[0],input_rrep_ack->DesAddress[1],input_rrep_ack->DesAddress[2],input_rrep_ack->DesAddress[3],input_rrep_ack->DesAddress[4],input_rrep_ack->DesAddress[5]);
	//printf("MyAddress=%2X-%2X-%2X-%2X-%2X-%2X-\n",My_Address[0],My_Address[1],My_Address[2],My_Address[3],My_Address[4],My_Address[5]);

	if((memcmp(input_rrep_ack->DesAddress,My_Address,6))==0) {
		// printf("***** Receive RREP-ACK for me (daemon msg)\n");
	} 
	else {
		// add by bc5678 2007.04.25
		if (is_my_stat(input_rrep_ack->DesAddress)) {
			// printf("This is my station\n");
			// printf("***** Receive RREP-ACK for me (daemon msg)\n");
		} else {
		send_packet((unsigned char*)input_rrep_ack,My_Address,input_rrep_ack->DesAddress,input_rrep_ack->SourceAddress,0,TTL,SeqNum);
		}
		
	}
	return 1;	
}
*/

// Recv Data frame but has no path information for nexthop,so generate a unicast rerr to inform the Datafrom-Sorce
// SourceAddr is my address and DesAddr is the data frame destination address
//===============================================================================================================

int gen_rerr(unsigned char* DesAddr,unsigned char* DataSrcAddr,unsigned char TTL,unsigned short SeqNum,unsigned char RERR_flag)   
{ 

    struct rerr ptr;
    // if(RERR_flag==2) { // don't have path to dest 

    ptr.Category    = _MESH_CATEGORY_ID_;
    ptr.Action      = _HWMP_MESH_PATHSEL_ACTION_ID_;
    ptr.Id          = _MESH_PERR_IE_;
    ptr.Length      = (sizeof(struct rerr)-2);
    ptr.ModeFlags   = 0;
    ptr.NumOfDes    = htonl(1); 
    memcpy(ptr.DesAddress, DesAddr, MACADDRLEN);
    ptr.DesSeqNum   = htonl(0); // htonl(Entry.dsn);
   	send_packet((unsigned char*)&ptr,my_address,DataSrcAddr,my_address,0,TTL,SeqNum);
    return 1;
    /*
    }
    else { //  neighbor node dispear
        RERR_Check(DesAddr);
        return 1;
    }
    */
}


int recv_rerr(struct rerr *input_rerr, unsigned char* PreHopAdd,unsigned char TTL,unsigned short SeqNum,unsigned char* DesAdd)
{
    struct path_sel_entry Entry;                    //clean the table entry

    /*
    {
        printf("@@@ recv_RERR : (can't find %X %X %X %X %X %X) from prehop : %x %x %x %x %x %x\n", 
        input_rerr->DesAddress[0], input_rerr->DesAddress[1], input_rerr->DesAddress[2], input_rerr->DesAddress[3], input_rerr->DesAddress[4], input_rerr->DesAddress[5], 
        PreHopAdd[0], PreHopAdd[1], PreHopAdd[2], PreHopAdd[3], PreHopAdd[4], PreHopAdd[5]);
    }
    */

    if(query_table(input_rerr->DesAddress,&Entry) > 0 ){
        if((memcmp(Entry.nexthopMAC,PreHopAdd, MACADDRLEN))==0) {
            // clear path selection table
            remove_table(input_rerr->DesAddress);



            //relay RERR ,NO really Destinaiton and Source
            // not sending the packet if I am the dest
            if(memcmp(my_address, DesAdd, MACADDRLEN))
                send_packet((unsigned char*)input_rerr, my_address, DesAdd, my_address, 0, TTL, SeqNum);
        }
        else {return 1;}
    }

    return 1;	
}


//gen_rrep(My_Address,My_SeqNum,Entry1.destMAC,Entry1.dsn,0, 0,My_Address);
//DesAdd is who product RREP, Source is Who product RREQ
//=========================================================================
//modify by chuangch 2007/04/10
int gen_rrep(unsigned char* DesAdd,unsigned int DesSeq,unsigned char* Source,unsigned int SourceSeq,unsigned int NewMetric,unsigned char hopcount,unsigned char TTL,unsigned short SeqNum, unsigned char Is6AddrFormat,unsigned char* Addr5,unsigned char* Addr6)
{
	
    struct rrep  ptr;	

    ptr.Category    = _MESH_CATEGORY_ID_;
    ptr.Action      = _HWMP_MESH_PATHSEL_ACTION_ID_;
    ptr.Id          = _MESH_PREP_IE_;
    ptr.Length      = (sizeof(struct rrep)-2);
    ptr.ModeFlags   = 0;
    ptr.HopCount    = hopcount;
    ptr.ttl         = TTL;
    //ptr.SrcCount			=1;
    memcpy(ptr.DesAddress,DesAdd, MACADDRLEN);

    ptr.DesSeqNum   = htonl(DesSeq);
    ptr.LifeTime    = htonl(255);

    ptr.Metric      = htonl(NewMetric);

    memcpy(ptr.first.SourceAddress,Source, MACADDRLEN);
    ptr.first.SourceSeqNum	= htonl(SourceSeq);
    ptr.first.DepDesSeqNum	= htonl(0); //?
	if(Is6AddrFormat)
        send_packet_with_6addr((unsigned char*)&ptr,my_address,Source,my_address,0,ptr.ttl,SeqNum,Addr5,Addr6);
    else
        send_packet((unsigned char*)&ptr,my_address,Source,my_address,0,ptr.ttl, SeqNum);
    return 1;

}
//=========================================================================

static void forward_rrep(struct rrep *ptr,
									unsigned char TTL,
									unsigned short SeqNum,
									 unsigned char Is6AddrFormat,
									unsigned char *addr5 ,
									unsigned char *addr6 
									 )
{

    if(ptr->ttl <=0) {
        return;
    }    
    ptr->DesSeqNum = htonl(ptr->DesSeqNum);
    ptr->LifeTime = htonl(ptr->LifeTime);
    ptr->Metric = htonl(ptr->Metric);
    ptr->first.SourceSeqNum = htonl(ptr->first.SourceSeqNum);
    ptr->first.DepDesSeqNum = htonl(ptr->first.DepDesSeqNum);
   
    if(Is6AddrFormat)
        send_packet_with_6addr((unsigned char*)ptr, my_address, ptr->first.SourceAddress, ptr->DesAddress, 0, TTL, SeqNum, addr5, addr6);
    else
        send_packet((unsigned char*)ptr, my_address, ptr->first.SourceAddress, ptr->DesAddress, 0, TTL, SeqNum);

    return;	
}


//DesAdd is who product RREP, Source is Who product RREQ
int recv_rrep(struct rrep *input_rrep,						
							unsigned char* My_Prehop,
							unsigned int My_Prehop_metric,
							int My_interFace,
							unsigned char TTL,
							unsigned short SeqNum, 
							unsigned char Is6AddrFormat,
							unsigned char *addr5 ,
							unsigned char *addr6  )
{ 
    if(memcmp(input_rrep->DesAddress, my_address, MACADDRLEN)==0) // for e.g., the situation is caused by rann's delay
        return 0;

    input_rrep->LifeTime = htonl(input_rrep->LifeTime);
    input_rrep->DesSeqNum = htonl(input_rrep->DesSeqNum);
    input_rrep->first.SourceSeqNum = htonl(input_rrep->first.SourceSeqNum);
    input_rrep->first.DepDesSeqNum = htonl(input_rrep->first.DepDesSeqNum);
    input_rrep->Metric = htonl(input_rrep->Metric) + My_Prehop_metric;
    --input_rrep->ttl;
    ++input_rrep->HopCount;


    updatePathByFreshness(input_rrep->DesAddress, input_rrep->DesSeqNum,
                input_rrep->Metric, input_rrep->HopCount, My_Prehop, 
                My_interFace);    

	/*
	//Check the entry for the RREP Destination who product the RREP
	temp_result=query_table(input_rrep->DesAddress,&Entry1);
	if(temp_result<=0) {       //need to create table entry for rrep.desnation
		create_table_entryA(input_rrep->DesAddress,input_rrep->DesSeqNum,My_Prehop,My_interFace,(input_rrep->HopCount),(input_rrep->Metric));	
		temp_result=query_table(input_rrep->DesAddress,&Entry1);
	}
	else {

		
		// updatePendingPath_by_src(input_rrep->DesAddress, input_rrep->Metric, input_rrep->DesSeqNum, input_rrep->HopCount, My_Prehop);
			
		//check seq#
		// *
		if(CheckSeq(Entry1.dsn,input_rrep->DesSeqNum)< 0) {
			;
		} * //
		// *
		if( CheckSeq(Entry1.dsn,input_rrep->DesSeqNum) > 0 
			||(Entry1.metric > input_rrep->Metric && !memcmp(Entry1.nexthopMAC,My_Prehop,6))
			|| Entry1.metric > input_rrep->Metric + pathSwitchThreshold )
		{			
			Entry1.dsn=input_rrep->DesSeqNum;
			memcpy(Entry1.nexthopMAC,My_Prehop,6);
			Entry1.hopcount=(input_rrep->HopCount);
			Entry1.metric=(input_rrep->Metric);
			modify_table(&Entry1);
		
		}
		else {return 0;}
		* ///

	}
	*/

    if((memcmp(input_rrep->first.SourceAddress,my_address, MACADDRLEN))==0) {	
        // close by popen printf("***** REEP is for me,and I will send back RREP_ACK (daemon msg)\n");

        // fix:Initial ping several timeout (bug num: Productlization Phase 2 29, 2007/10/16)
        if (1 == Is6AddrFormat)
            notify_path_found(addr6);
        else
            notify_path_found(input_rrep->DesAddress);
        // end of fix:Initial ping several timeout.


        return 1;
    }

    //	input_rrep->Metric = input_rrep->Metric + My_Prehop_metric;
    // close by popen printf("***** REEP is not for me,relay the RREP (daemon msg)\n");

    forward_rrep(input_rrep, TTL,SeqNum, Is6AddrFormat, addr5, addr6);
    return 1;
}


//following modify by chuangch 2007/04/10
//=========================================================================
int gen_rreq(unsigned char *DesAdd,unsigned char TTL,unsigned short SeqNum)
{
    struct rreq ptr;

    ptr.Category    = _MESH_CATEGORY_ID_;
    ptr.Action      = _HWMP_MESH_PATHSEL_ACTION_ID_;
    ptr.Id          = _MESH_PREQ_IE_;
    ptr.Length      = sizeof(struct rreq) - 2;
    ptr.ModeFlags   = 1;
    ptr.HopCount    =0;
    ptr.ttl         = TTL;
    //ptr->DesCount		=1;
    ptr.RreqId      = htonl(My_RreqId);
    memcpy(ptr.SourceAddress,my_address, MACADDRLEN);
    ptr.SourceSeqNum	= htonl(My_SeqNum);
    ptr.Lifetime    = htonl(255);
    ptr.Metric      = htonl(0);
    ptr.first.DO    =1;           //initial DO=1; RF=1;
    ptr.first.RF    =1;
    ptr.first.Reserved	=0;
    memcpy(ptr.first.DesAddress,DesAdd, MACADDRLEN);
    ptr.first.DesSeqNum	= htonl(0);     //if maintain ,this is not 0

    My_SeqNum++;
    My_RreqId++;
	send_packet((unsigned char*)&ptr,my_address,DesAdd,my_address,1,TTL,SeqNum);   //1 for broadcast
    return 1;          

}
//=========================================================================



static void forward_rreq(struct rreq *for_out,unsigned char TTL,unsigned short SeqNum)
{
    unsigned char DesAdd[6]; // just an unused parameter prepreared for send_packet
    if(for_out->ttl <=0) {
        return;
    }

    for_out->RreqId = htonl(for_out->RreqId);
    for_out->Metric = htonl(for_out->Metric);
    for_out->Lifetime = htonl(for_out->Lifetime);
    for_out->first.DesSeqNum = htonl(for_out->first.DesSeqNum);
    for_out->SourceSeqNum = htonl(for_out->SourceSeqNum);
    send_packet((unsigned char*)for_out, my_address, DesAdd, for_out->SourceAddress, 1, TTL, SeqNum);
}



// Main idea: The decision of whether forwarding or droping a rreq shall depends on
//            the "freshness" of the DSN and the "goodness" of the metric. The freshness
//            is to compare with previous rreq's {issuer, dest} pair. Thus, one SRC could
//            request two hosts, says A and B, and their SEQ NOs are 1 and 2 respectively.
//            If current MP have received rreq for B, it should NOT drop the rreq for A, 
//            because rreq for A is still a "fresh" RREQ for current MP.
// Design logic:
// Step 1: reject rreq from myself
// Step 2: always update path to pre-hop ('cos it is almost a near real-time metric)
// Step 3: check the freshness of tuple {src, dest, DSN} and the goodness of metric
//         if not fresh enough and not good enough, drop it
// Step 4: forward & reply
//         4a: reply rrep if current node is destination or destination is my station
//         4b: forward rreq with DO=1
//         4c: when DO=0 and having a path to desition which go through src, drop it
//         4d: reply rrep and forward rreq with DO=1, when DO=0 and having a path to destination which does not go throuth src
//         4e: forward rreq for 3b~3e
// 

int recv_rreq(struct rreq *input,unsigned char* My_Prehop,unsigned int My_Prehop_metric,int My_interFace,unsigned char TTL,unsigned short SeqNum)
{

    ///// Step 1: reject rreq from myself
    if(memcmp(input->SourceAddress, my_address, MACADDRLEN)==0) {
        return 0;            //get the RREQ from my_sent RREQ
    }

    input->Lifetime = ntohl(input->Lifetime);	
    input->RreqId = ntohl(input->RreqId);
    input->SourceSeqNum = ntohl(input->SourceSeqNum);
    input->first.DesSeqNum = ntohl(input->first.DesSeqNum);		 
    input->Metric = ntohl(input->Metric)+My_Prehop_metric;
    ++input->HopCount;
    --input->ttl;
	
    //// Step 3: check the freshness of tuple {src, dest, DSN} and the goodness of metric
    if(updatePathByFreshness(input->SourceAddress, input->SourceSeqNum,
                            input->Metric, input->HopCount, My_Prehop, 
                            My_interFace)==0 ) {                          
        return 0;
    }

    ///// Step 4: forward & reply

    // case 4a
    if((memcmp(input->first.DesAddress,my_address, MACADDRLEN))==0) {	
        gen_rrep(my_address,My_SeqNum, input->SourceAddress, input->SourceSeqNum, 0, 0, _MESH_HEADER_TTL_, SeqNum,0,0,0);
        My_SeqNum++;
        return 1;
    }

    if (is_my_stat(input->first.DesAddress)>0) {
        gen_rrep(my_address, My_SeqNum, input->SourceAddress, input->SourceSeqNum, 0, 0, _MESH_HEADER_TTL_, SeqNum, 1,input->SourceAddress,input->first.DesAddress);
        My_SeqNum++;
        return 1;
    }

    // case 4b
    if(input->first.DO==1) {         //DO=1
        forward_rreq(input, TTL, SeqNum);
        return 1;
    }
    
#if 0 //never enter this section, mark out
	result = query_table(input->first.DesAddress,&Entry2);
	//DO=0
	if(result >=0) {     //I have the information to destination 
	
		// case 4c
		if(memcmp(Entry2.nexthopMAC, Entry1.nexthopMAC, MACADDRLEN)==0)
		{
			/// Stanley
			// case 1:  nexthop = prehop, abort!
			// case 2: Happens when DO=0
			//    C: rreq issuer, X: rreq target
			//    A (current node): rrep issuer (DO=0, auto reply)
			//       A-----B (best)----C-----X 
			//       |-------(worse)---|
			//    A has to check whether Path_to_X->next == nexthop of reponse, if true, abort
			return 0;
		}
		if(idx_pending>=0) 
		{
			// in the future (when pending update), case 4c will happen, so abort it.
			if(memcmp(Entry2.nexthopMAC, pending_path[idx_pending].nexthopMAC, MACADDRLEN)==0)
			{
				return 0;
			}
		}			
		
		// case 4d
		gen_rrep(Entry2.destMAC,Entry2.dsn,Entry1.destMAC,input->SourceSeqNum,(Entry2.metric),(Entry2.hopcount),My_Address,TTL,SeqNum,0,0,0);
			
		if(input->first.RF==1) {  //RF=1
			// close by popen printf("***** send back RREP and generatie a RREQ with DO=1,RF=1,and broadcast it (daemon msg)\n");
			input->first.DO=1;
			forward_rreq(input, My_Address, TTL, SeqNum);
		}  		
	}
	else {                    //I have no information to destination
		// case 4e
		forward_rreq(input, My_Address, TTL, SeqNum);
	}
#endif

	return 1;

}

