/***********************************************************
	Copyright (C), 1998-2014, Tenda Tech. Co., Ltd.
	FileName: ifbatchupgrade_usrreq.c
	Description: eCosv1.0 ÅúÁ¿Éý¼¶, kernel mode
	Version : 1.0
	Date: 2014.09.25
	Function List:
	History:	<author>   <time>     <version >   <desc>
	wzs        2014.09.25   1.0        new
************************************************************/
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <kdev.h>

#include <sys/socket.h>
#include <sys/types.h>

#ifndef	FALSE
#define	FALSE	0
#endif
#ifndef TRUE
#define	TRUE	1
#endif

#define MAC_PACKET_LEN          		1514
#define dataLen                 			1024
#define ETH_UPGRADE_PROTOCOL		0x5151
#define ETH_UPGRADE_PROTOCOL2	0x5252

/* Request type */
#define UPG (u_short)	1 //upgrade broadcast
#define DAT (u_short)	2 //upgrade data
#define ACK (u_short)	3 //upgrade ack
#define SUC (u_short)	4 //upgrade success
#define ERR (u_short)	5 

#define TENDA_UPGRADE		"TENDA_UPGRADE"
#define TENDA_DATA			"TENDA_DATA"

#define TENDA_PROTO		"0x5151"           
#define TENDA_PROTO_DATA	"0x5252"
#define FILE_BEGIN_FLAG		(u_short)1
#define FILE_END_FLAG		(u_short)2

/* upgrade data */
struct upgrade_data
{
    u_short fileFlag;			//upgrade data start/end flag
    u_short restoreFlag;
    char version[36];
    char  data[dataLen];
    u_short dataLength;
};

/* Ethernet trastmit packet */
struct eth_tmt_pkt
{
    char flag[32];				// TENDA_UPGRADE e.g.
    u_short index;				// upgrade data index
    u_short rq_type;			// request type
    struct upgrade_data  upg_data;
};

/* UPG_BDCT_FLAG:
 * 0: initial
 * 1: broadcast received
 * 2: restore defaults flag set
 * 3: server reply:success
 * 4: validate fail, return
*/
int UPG_BDCT_FLAG = 0;

/* UPG_DATA_FLAG:
 * 0 : initial
 * 1 : first data_packet received 
 * 2 : last data_packet received
 * 3 : validate ok, upgrade
*/
int UPG_DATA_FLAG = 0;

/*restore defaults flag set*/
int UPG_RESTORE_FLAG = 0;

extern struct upgrade_mem *imagePtr;
extern int imageLen;
extern int batch_upgrade_cache(char *data, unsigned int len);
extern int validate_image_format(char *stream, int httpd_offset);

int ifupgrade_input(struct ifnet *, struct ether_header *, struct mbuf *);
int (*upgrade_input_hook)(struct ifnet *, struct ether_header *, struct mbuf *) = ifupgrade_input;

void 
ifupgrade_deinit(void)
{
	upgrade_input_hook = NULL;
}

/*
 * Send packet to ethernet
 */
static void
ifupgrade_output(struct ifnet *eth_ifp, u_char *dhost, struct eth_tmt_pkt *upghdr, u_short type)
{
	struct mbuf *m;
	struct ether_header *eh;
	struct sockaddr dst;
	struct eth_tmt_pkt *upg_hdr;
	int s;

	/* generate a packet of that type */
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
	{
		return;
	}
	
	MCLGET(m, M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) 
	{
		m_freem(m);
		return;
	}
	
	/* Reserved room for ether header */
	m->m_data += sizeof(*eh) + 2;

	/* Build upgrade header */
	upg_hdr = mtod(m, struct eth_tmt_pkt *);
	memset(upg_hdr->flag, 0, sizeof(upg_hdr->flag));
	memcpy(upg_hdr->flag, upghdr->flag, strlen(upghdr->flag));
	upg_hdr->index = ntohs(upghdr->index);
	upg_hdr->rq_type = ntohs(upghdr->rq_type);

	/* Adjust sizes */
	m->m_len = sizeof(*upg_hdr) + strlen(upg_hdr->flag);
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.rcvif = 0;

	/*
	* Set destination to send and tell
	* the ether_output() to do raw send
	* without routing for us.
	*/
	memset(&dst, 0, sizeof(dst));
	dst.sa_len = 16;
	dst.sa_family = AF_UNSPEC;

	eh = (struct ether_header *)dst.sa_data;
	eh->ether_type = ntohs(type);
	memcpy(eh->ether_dhost, dhost, sizeof(eh->ether_dhost));

	/* Raw send */
	s = splimp();
	ether_output(eth_ifp, m, &dst, 0);
	splx(s);

	return;
}

/*
 * Batch Upgrade input routine, called from
 * ether_demux().
 */
int
ifupgrade_input(struct ifnet *eth_ifp,
	struct ether_header *eh, struct mbuf *m)
{
	struct eth_tmt_pkt rep_hdr;		// for replying packet.
	struct eth_tmt_pkt *upg_hdr;		// for parasing packet.
//	static u_short cur_index = 0;
	u_short type, new_index;
	
	/*
	 * Note: The caller has to make sure there is
	 *       a ethernet header before the m_data
	 *       and the at least contains a upgrade
	 *       header of data.
	 */
	if (m->m_len < (sizeof(rep_hdr)))
	{
		goto drop_it;
	}

	//printf("==> src_addr__: %02x%02x%02x%02x%02x%02x\n", 
	//	eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2], eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5]);

	/* Distiguish upgrade packet */
	upg_hdr = mtod(m, struct eth_tmt_pkt *);
	new_index = ntohs(upg_hdr->index);
	type = ntohs(upg_hdr->rq_type);

	/* Initilize reply packet */
	memset(&rep_hdr, 0, sizeof(rep_hdr));
	memset(rep_hdr.flag, 0, sizeof(rep_hdr.flag));
	strcpy(rep_hdr.flag, upg_hdr->flag);
	rep_hdr.rq_type = ACK;
	
	/* Check packet type */
	switch(ntohs(eh->ether_type))
	{
		case ETH_UPGRADE_PROTOCOL:
			if(type == UPG && !strcmp(upg_hdr->flag,TENDA_UPGRADE) && UPG_BDCT_FLAG == 0)
			{
				diag_printf("[Batch upgrade]: Broadcast received+_+\n");
				rep_hdr.index = 0;
				ifupgrade_output(eth_ifp, eh->ether_shost, &rep_hdr, ETH_UPGRADE_PROTOCOL);
				UPG_BDCT_FLAG = 1;

				/* Need to restore */
				if(ntohs(upg_hdr->upg_data.restoreFlag) == 1)
				{
					diag_printf("[Batch upgrade]: Need to restore!!!!\n");
					UPG_RESTORE_FLAG = 1;
				}
			}
			break;
		case ETH_UPGRADE_PROTOCOL2:
			/* Upgrade sucess. This packet will be received after reboot */
			if(type == SUC && !strcmp(upg_hdr->flag,TENDA_UPGRADE))
			{
				diag_printf("[Batch upgrade]: Server reply: success+_+\n");
				/* ...
					led ctrl
				     ...
				*/
				ifupgrade_deinit();
				UPG_BDCT_FLAG = 3;
			}
			/* Error, it turns out to be an ACK missing or an invalid ACK */
			else if(type == ERR && !strcmp(upg_hdr->flag,TENDA_DATA))
			{
				ifupgrade_deinit();
				UPG_DATA_FLAG = 4;
			}
			/* Determine the data packet 
			     if received data packet,  return err.
			*/
			else if(type == DAT && !strcmp(upg_hdr->flag,TENDA_DATA))
			{
				diag_printf("[Batch upgrade]: Received data packet: return err.\n");
				ifupgrade_deinit();
				UPG_DATA_FLAG = 4;
			}
		#if 0
			else if(type == DAT && !strcmp(upg_hdr->flag,TENDA_DATA))
			{
				if(imagePtr == NULL)
				{
					goto drop_it;
				}
			
				if(cur_index == 0 && ntohs(upg_hdr->upg_data.fileFlag) == FILE_BEGIN_FLAG)
				{
					UPG_DATA_FLAG = 1;

				}

				/* Determine the data end flag */
				if(ntohs(upg_hdr->upg_data.fileFlag) == FILE_END_FLAG)
				{
					UPG_DATA_FLAG = 2;
				}

				/* Catch the upgrade data.
				 * Once the sequence num is wrong, re-request the packet again directly
				*/
				if(new_index == (cur_index+1))
				{
					cur_index = new_index;
					if( (new_index > 0) && (new_index < 2000) )
					{
						int len = ntohs(upg_hdr->upg_data.dataLength);
						/* No enough mem... */
						if(batch_upgrade_cache(upg_hdr->upg_data.data, len) < 0)
						{
							ifupgrade_deinit();
							UPG_DATA_FLAG = 4;
						}
					}
				}

				rep_hdr.index = cur_index;
				ifupgrade_output(eth_ifp, eh->ether_shost, &rep_hdr, ETH_UPGRADE_PROTOCOL2);

				/* FILE_END_FLAG found, validate the image and do upgrade*/
				if(UPG_DATA_FLAG == 2)
				{
					diag_printf("\n[Batch upgrade]: FILE_END_FLAG received+_+\n");

					/* read upgrade_data over. */
					ifupgrade_deinit();
					
					rep_hdr.index = 0;
					if(validate_image_format((char *)imagePtr, 0) > 0)
					{
						diag_printf("[Batch upgrade]: Validate success+_+\n");
						rep_hdr.rq_type = SUC;
						UPG_DATA_FLAG = 3;
						ifupgrade_output(eth_ifp, eh->ether_shost, &rep_hdr, ETH_UPGRADE_PROTOCOL);
					}
					else
					{
						diag_printf("[Batch upgrade]: Validate fail+_+#\n");
						rep_hdr.rq_type = ERR;
						UPG_DATA_FLAG = 4;
						ifupgrade_output(eth_ifp, eh->ether_shost, &rep_hdr, ETH_UPGRADE_PROTOCOL);
					}
				}
			}
		#endif
			break;
		default:
			break;
	}

drop_it:
	/* Free the packet anyway */
	m_freem(m);
	return FALSE;
}


