
#ifdef TENDA_WLJB
#define IPF_HTTP_PROXY

//extern nat_t   *g_nat ;
int ack_delta = 0;

extern unsigned short chsum_fixup_short(unsigned short sum, unsigned short old_data,
						unsigned short new_data);

//extern unsigned short tenda_mss;

unsigned short tcp_checksum(struct ip *ip, struct tcphdr *tcp)
{
    int hlen;
    int len, count;
    int sum;
    char odd[2] =
    {
        0, 0
    };
    unsigned short * ptr;

    hlen = ( ip->ip_hl << 2);
    len = ip->ip_len - hlen;

    count = len;
    sum = 0;
    ptr = ( unsigned short *) tcp;

    while (count > 1){
        sum += *ptr++;
        count -= 2;
    }
    if (count){
        odd[0] = *( unsigned char *) ptr;

        ptr = ( unsigned short *) odd;
        sum += *ptr;
    }

    /* Pseudo-header */
    ptr = ( unsigned short *) &( ip->ip_dst);
    sum += *ptr++;
    sum += *ptr;
    ptr = ( unsigned short *) &( ip->ip_src);
    sum += *ptr++;
    sum += *ptr;
    sum += htons( ( unsigned short) len);
    sum += htons( ( unsigned short) ip->ip_p);

    /* Roll over carry bits */
    sum = ( sum >> 16) + ( sum & 0xffff);
    sum += ( sum >> 16);

    // Take one's complement
    sum = ~sum & 0xffff;

    return sum;
}

unsigned short chsum(unsigned short *buf, int nwords)
{
	unsigned long sum;

	for(sum=0; nwords>0; nwords--)
		sum += *buf++;
	sum = (sum>>16) + (sum & 0xFFFF);
	sum += (sum>>16);

	return ~(unsigned short)sum;
}

static char http_get[] = "GET / HTTP/1.0\r\nUser-Agent:";
									
int tenda_http_request_packet2(fr_info_t *fin,struct mbuf *m0, ip_t *ip)
{
	char *data = NULL,*pdata = NULL,*pline=0;
	
	//下面是windows上最常用到的User-Agent形式，这样可以减少修改报文
	char new_ua[]="User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\r\n"; 
	char newbuf[1492]={0},*pnewbuf = NULL;

	int data_len0,data_len1,find_get;
	int old_ua_len,new_ua_len;
	int inc,off;

	tcphdr_t *tcph;

	old_ua_len = 0;
	new_ua_len = sizeof(new_ua) - 1;
	
	tcph = (tcphdr_t *)fin->fin_dp;
	data = ( char *) tcph + ( tcph->th_off << 2);
	data_len0 = ip->ip_len - (tcph->th_off << 2) - (ip->ip_hl << 2);
	off = fin->fin_hlen + (tcph->th_off << 2);

	if(data_len0 < sizeof(http_get) || data_len0 > (1492 - new_ua_len))
		return 0;

	find_get = 0;
	switch (data[0])
	{
	case 'G':
		if (memcmp(data, "GET ", 4) == 0) {
			find_get = 1;
		}
		break;

	case 'P':
		if (memcmp(data, "POST ", 5) == 0) {
			find_get = 1;
		}
		break;

	case 'H':
		if (memcmp(data, "HEAD ", 5) == 0) {
			find_get = 1;
		}
		break;

	default:
		break;
	}

	if(! find_get) return 0;

	inc = 0;
	pnewbuf = newbuf;
	pdata = data;
	data_len1 = data_len0;
	while(data_len1>0){
		*pnewbuf = *pdata;
		if(*pdata == 'U' && memcmp(pdata, "User-Agent: ", 12) == 0){
			pline = pdata;
			for(;*pline != '\n' && (pline - data) < data_len0;pline++ )
				old_ua_len++;
			if((pline - data) == data_len0){
				return 0;
			}
			if(*pline == '\n'){
				old_ua_len++;
				/*pline 这时指向data中剩下的数据*/
				pline++;
			}
			/*拷贝'U'字符以后的数据*/
			memcpy(pnewbuf,new_ua,new_ua_len);
			pnewbuf+=new_ua_len;
			
			inc = new_ua_len - old_ua_len;

			/*把剩下的数据拷到pnewbuf中*/
			data_len1 = data_len0 - (pline - data);

			while(data_len1>0 &&(pnewbuf - newbuf) < 1492){
				*pnewbuf++ = *pline++;
				data_len1--;
			}

			if(data_len1 != 0){
				return 0;
			}

			break;
			
		}
		pnewbuf++;
		pdata++;
		data_len1--;
	}

	data_len0 += inc; 

	if (inc < 0)
		m_adj(m0, inc);

	m_copyback(m0, off, data_len0, newbuf);
#  ifdef	M_PKTHDR
	if (!(m0->m_flags & M_PKTHDR))
		m0->m_pkthdr.len += inc;
#  endif
	if (inc != 0) {
		ip->ip_len += inc;
		fin->fin_dlen += inc;
		fin->fin_plen += inc;
	}

	return inc;
}

//-------------------------------------------------------------------------

int ippr_http_init(void)
{
    return 0;
}

//-------------------------------------------------------------------------

int ippr_http_out(fr_info_t *fin, ip_t *ip, ap_session_t *aps, nat_t *nat)
{
	tcphdr_t *tcp;
	mb_t *m;
	int inc = 0;
	
#if	SOLARIS && defined(_KERNEL)
	m = fin->fin_qfm;
#else
	m = *fin->fin_mp;
#endif
#if 0
	if(!(m->m_flags & M_EXT))
		return APR_INC(inc);
	if (!(m->m_flags & M_PKTHDR))
		return APR_INC(inc);
#endif	
	tcp = (tcphdr_t *)fin->fin_dp;

	if(tcp->th_flags & TH_SYN) return 0;


	inc = tenda_http_request_packet2(fin,m,ip);

    	return APR_INC(inc);
}
#endif

