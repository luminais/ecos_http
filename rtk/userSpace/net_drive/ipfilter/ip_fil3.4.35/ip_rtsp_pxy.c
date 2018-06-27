/*
 * rtsp proxy.
 *
 * Copyright (C) 2012, Tenda Corporation
 * All Rights Reserved.
 * 
 * $Id: ip_rtsp_pxy.c,v 1.1 2012-04-19 11:45:55 Exp $
 * 
 * author: liuzexin 
 */

 
//#define IPF_RTSP_PROXY //move to ipfilter/Makefile

extern int nataddrule(char *fmt, ...);

static frentry_t	rtsp_fr;
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"



int  ippr_rtsp_init __P((void));
int  ippr_rtsp_new __P((fr_info_t *, ip_t *, ap_session_t *, nat_t *));
int  ippr_rtsp_out __P((fr_info_t *, ip_t *, ap_session_t *, nat_t *));


/* Hook function */
int ippr_rtsp_init(void)
{
	bzero((char *)&rtsp_fr, sizeof(rtsp_fr));
	rtsp_fr.fr_ref = 1;
	rtsp_fr.fr_flags = FR_INQUE|FR_PASS|FR_QUICK|FR_KEEPSTATE;

	return 0;
}

/* New a pptp session */
int
ippr_rtsp_new
(
	fr_info_t *fin,
	ip_t *ip,
	ap_session_t *aps,
	nat_t *nat
)
{	
	aps->aps_data = NULL;
	aps->aps_psiz = 0;
	return 0;
}

/*	
*	the package format as follow:
*
*	SETUP rtsp://10.255.168.137/tv/40/ RTSP/1.0
*	CSeq: 2
*	Transport: RTP/AVP;unicast;client_port=23426-23427
*	User-Agent: CTC RTSP 1.0
*
*/
u_short ippr_rtsp_pickport( char *data )
{
    	char *portStr = NULL;
	char *port;
    	u_short sp = 0;
    
    	portStr = strstr( data, "client_port=" );
    	port = portStr + strlen("client_port=");

	while( *port >= '0' && *port <= '9' )
    	{
       	sp = sp * 10 + (*port-0x30);
       	port ++;
    	}

	return sp;
}

/*
*  function: modify the package port's string
*/
int  modify_pkt_port(char *olddata, char *newdata, u_short port)
{
	char *c=NULL, *p=NULL;
	int n=0;
	char buf[32]={0};
	p=olddata;
	c = strstr(olddata, "client_port=");
	
	if(!olddata || !c)
		return 0;
	c = c+ strlen("client_port=");

	sprintf(buf, "%d", port);
	memcpy(newdata, olddata, c-p);
	newdata += c-p;
	olddata += c-p;
	
	while(*olddata)
	{
		
		if((*olddata<='9')&&(*olddata>='0'))
		{      
			n++;
			olddata++;
		}
		else 
			break;
		
	}
	memcpy(newdata, buf, strlen(buf));
	strcat(newdata,"-");

	memset(buf, 0, 32);
	sprintf(buf, "%d", port+1);
	
	strcat(newdata, buf);

	olddata +=n+1;

	newdata += (strlen(buf)*2 +1);

	while(*olddata !='\0')
	{
		*newdata = *olddata;
		newdata++;
		olddata++;
	}

	return 0;	
}



int ippr_rtsp_setuppkt( fr_info_t *fin, ip_t *ip, nat_t *nat, char *pdata, int data_len, int off )
{
	mb_t *m;
	char newbuf[1280]={0};
	char *newdata = newbuf, *olddata = NULL;
	int inc = 0;
	struct in_addr swip; 
	u_short newport=0;
	u_short sp = 0;
	u_int newoip = 0;
	fr_info_t fi;
	nat_t *ipn;
	char *data = pdata;
    
#if	SOLARIS && defined(_KERNEL)
	m = fin->fin_qfm;
#else
	m = *fin->fin_mp;
#endif
	KMALLOCS(olddata, char *, data_len + 1);
	memset(olddata, 0, data_len+1);
	memcpy(olddata, pdata, data_len);

	sp = ippr_rtsp_pickport( data );
	
	bcopy((char *)fin, (char *)&fi, sizeof(fi));
	fi.fin_data[0] = sp;
	fi.fin_data[1] = fin->fin_data[1];
	
 	ipn = nat_outlookup(&fi, IPN_UDP|FI_W_DPORT, IPPROTO_UDP, nat->nat_inip, ip->ip_dst, NAT_OUTBOUND);

	if(!ipn)
	{
		
	/*
	 * Add skeleton NAT entry for connection which will come back the
	 * other way.
	 */
		int slen;
		struct udphdr udp;            
		slen = ip->ip_len;
		ip->ip_len = fin->fin_hlen + sizeof(udp);
		       
		bzero(&udp, sizeof(udp));

		udp.uh_sport = htons(sp);
		fi.fin_fi.fi_p = IPPROTO_UDP;
		fi.fin_dp = (char *)&udp;
		swip = ip->ip_src;
		        
		fi.fin_fi.fi_saddr = nat->nat_inip.s_addr;
		fi.fin_fi.fi_daddr = nat->nat_oip.s_addr;       
		ip->ip_src = nat->nat_inip;

		ipn = nat_new(&fi, ip, nat->nat_ptr, NULL, IPN_UDP|FI_W_DPORT, nat->nat_dir);

		if (ipn != NULL) {
		
			ipn->nat_age = fr_defnatage;
			fi.fin_fr = &rtsp_fr;
			newport = ipn->nat_outport;
			newoip = ipn->nat_outip.s_addr;
			(void) fr_addstate(ip, &fi, NULL,
					   FI_W_DPORT|FI_IGNOREPKT);
			
		}
		ip->ip_len = slen;
		ip->ip_src = swip;
	
		modify_pkt_port(olddata, newdata, ntohs(newport));
		
		inc = strlen(newbuf) - data_len;
		
	}
	else
	{
		ipn->nat_age = fr_defnatage;
    		newport = ipn->nat_outport;
		newoip = ipn->nat_outip.s_addr;
	}

	char wanipstr[16] = {0};
    	char inipstr[16] = {0};
	sprintf(wanipstr,"%u.%u.%u.%u",NIPQUAD(newoip));
	u_int inip = nat->nat_inip.s_addr;
	sprintf(inipstr,"%u.%u.%u.%u",NIPQUAD(inip));

	nataddrule("rdr %s %s/32 port %u -> %s port %u %s\n",
	   	nat->nat_ifname,
       	wanipstr,
       	ntohs(newport),
       	inipstr,
	      	sp,
	      	"udp");
      	
    	if( inc < 0 )
		m_adj(m, inc);
    
	m_copyback(m, off, strlen(newbuf), newbuf);


#ifdef M_PKTHDR
	if (!(m->m_flags & M_PKTHDR))
		m->m_pkthdr.len += inc;
#endif
	if (inc != 0) 
    	{
		ip->ip_len += inc;
		fin->fin_dlen += inc;
		fin->fin_plen += inc;
	}

    	KFREES(olddata, data_len + 1);
    	return APR_INC(inc);
    
}




/*rtsp ALG output */
int
ippr_rtsp_out
(
	fr_info_t *fin,
	ip_t *ip,
	ap_session_t *aps,
	nat_t *nat
)
{
	tcphdr_t *tcp;
	int  tcph_len = 0, iph_len = 0, data_len = 0, off = 0;
	char *data = NULL;
	int inc = 0;

	   
	tcp = (tcphdr_t *)fin->fin_dp; 
	tcph_len = (tcp->th_off << 2);
	iph_len = ip->ip_hl << 2;
	data_len = ip->ip_len - tcph_len - iph_len;
	off = fin->fin_hlen + tcph_len;
	data = (char *)tcp + tcph_len;

	if( data_len != 0 )
	{
		if( !memcmp(data,"SETUP", 5))
		{	
			inc = ippr_rtsp_setuppkt( fin, ip, nat, data, data_len, off );
		}
	}
	
	return APR_INC(inc);
	
}

