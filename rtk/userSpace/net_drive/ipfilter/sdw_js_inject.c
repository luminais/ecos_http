#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <sys/stat.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/udp.h>
#include <netinet/ip_var.h>
#include <arpa/inet.h>
#include <ip_compat.h>
#include <ip_fil.h>
#include <ip_nat.h>
#include <ip_frag.h>
#include <netdb.h>
#include <router_net.h>
#include <bcmnvram.h>
#include <ctype.h>
#include <time.h>
#include "../../cbb/src/login_keep/lm_login_keep.h"
#include "sdw_filter.h"
#include "sdw_js_inject.h"
#include "jhash.h"

extern int ip_to_mac(struct in_addr ip, char *mac);

extern void (*js_inject_check)(struct ifnet *ifp, struct mbuf **mpp);

extern u_int ipf_nattable_sz;
char lan_ip_addr[IP_LEN_16] = {0};
uint32_t lan_mask_para = 0, lan_addr_para = 0;
char meta_names[][8] = {
    "author", "descri", "keywor", "genera", "revise", "others", "title\""
};

int ret_success = 0;
int ret_drop = -1;
int ret_notfound = 2;
int ret_failure = -2;

char *accept_header = "Accept:*/*;q=0";
char *accept_header_prefix = "\r\nAccept:*/*;q=0";

//char *acceptencoding_header = "\r\nAccept-Encoding:*/*;q=0";
//char *accept_combine_header = "\r\nAccept:*/*;q=0\r\nAccept-Encoding:*/*;q=0";
char *accept_part_header = "*/*;q=0";

//#define NULL 0
//#define bool int
//#define false 0
//#define true 1

#define USERAGENT_SHORTEST_LEN 40
char js_str[ARRAY_LEN_256]={0};

#define JS_RULE_HASH_LEN 30
struct rule_str *rule_str_hash[JS_RULE_HASH_LEN] = {0};

unsigned short tcp_checksum(struct ip *ip, struct tcphdr *tcp)
{
    int hlen = 0;
    int len = 0, count = 0;
    unsigned int sum = 0;
#if 1
    unsigned char odd[2] =
    {
        0, 0
    };
#endif
    unsigned short * ptr = NULL;

    hlen = (ip->ip_hl << 2);
    len = ntohs(ip->ip_len) - hlen;

    count = len;
    sum = 0;
    ptr = (unsigned short *)tcp;

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
    ptr = (unsigned short *) &(ip->ip_dst);
    sum += *ptr++;
    sum += *ptr;
    ptr = (unsigned short *) &(ip->ip_src);
    sum += *ptr++;
    sum += *ptr;
    sum += htons((unsigned short)len);
    sum += htons((unsigned short)ip->ip_p);
#if 1
    /* Roll over carry bits */
    sum = ( sum >> 16) + ( sum & 0xffff);
    sum += ( sum >> 16);

    // Take one's complement
    sum = ~sum & 0xffff;

	return sum;
#else
	while(sum >> 16)
		sum = (sum >> 16) + (sum & 0xffff);
	return (unsigned short)(~sum);
#endif
}

char* strstri_len(char * inBuffer, char * inSearchStr, int len)
{
    char*  currBuffPointer = inBuffer;

    while (*currBuffPointer != 0x00)
    {
        char* compareOne = currBuffPointer;
        char* compareTwo = inSearchStr;
        //统一转换为小写字符
        while (tolower((int)(*compareOne)) == tolower((int)(*compareTwo)))
        {
            compareOne++;
            compareTwo++;
            if (*compareTwo == 0x00)
            {
                return (char*) currBuffPointer;
            }

        }
        currBuffPointer++;
	len--;
	if(len<=0)
	{
		break;
	}
    }
    return NULL;
}

char* strstri(char * inBuffer, char * inSearchStr)
{
    char*  currBuffPointer = inBuffer;

    while (*currBuffPointer != 0x00)
    {
        char* compareOne = currBuffPointer;
        char* compareTwo = inSearchStr;
        //统一转换为小写字符
        while (tolower((int)(*compareOne)) == tolower((int)(*compareTwo)))
        {
            compareOne++;
            compareTwo++;
            if (*compareTwo == 0x00)
            {
                return (char*) currBuffPointer;
            }

        }
        currBuffPointer++;
    }
    return NULL;
}

char * strstr_len (const char * str1, const char * str2, int len)
{
	if(len>0)
	{
        char *cp = (char *) str1;
        char *s1, *s2;
 
        if ( !*str2 )
            return((char *)str1);
 
        while (*cp)
        {
                s1 = cp;
                s2 = (char *) str2;
 
                while ( *s1 && *s2 && !(*s1-*s2) )
                        s1++, s2++;
 
                if (!*s2)
                        return(cp);
 
                cp++; len--;
                if(len == 0)
                    break;
        }
 
        return NULL;
    }
    else
    {
    	return NULL;
	}
 
}

int locate_option_header_position(char *data, char *header_str, char **header_start, char **header_end, int data_len)
{
	*header_start = strstr_len(data, header_str, data_len);

	if(NULL != (*header_start))
	{
	       	if(((*header_start)-1)[0] != '\n')
       		{
        		return ret_notfound;
        	}
		
		(*header_end) = strchr((*header_start)+strlen(header_str), '\r');
		if(NULL == (*header_end))
		{
			return ret_drop;
		}
	}
	else
	{
		return ret_notfound;
	}
	return ret_success;
}

int locate_must_header_position(char *data, char *header_str, char **header_start, char **header_end, int data_len)
{
	*header_start = strstr_len(data, header_str, data_len);
	if(NULL != (*header_start))
	{
                if(((*header_start)-1)[0] != '\n')
                {
                        return ret_notfound;
                }

		(*header_end) = strchr((*header_start)+strlen(header_str), '\r');
		if(NULL == (*header_end))
		{
			return ret_drop;
		}
		else
		{
			return ret_success;
		}
	}

	return ret_drop;
}

int move_header_inject(char *data, char *header_str, char *insert_str, char *position_fix, char **position_tail, char **user_agent_p,int data_len)
{
	char *str_start = NULL, *str_end = NULL;
	
	int header_len = 0;
	int insert_len = strlen(insert_str);
	
	int ret_value = locate_option_header_position(data, header_str,&str_start,&str_end,data_len);

	if(ret_success == ret_value)
	{
		if(str_end > position_fix)
		{
			PRINTF("!!!move_header_inject str_end>position_fix!!!\n");
			return ret_drop;
		}
		
		header_len = (int)(str_end-str_start)+1;
		
		if(header_len<insert_len) //overwrite, move memory
		{
		/*	memcpy(str_start,str_end+2,(((int)(*position_tail))-(int)str_end-2));
			(*position_tail) -= (header_len -2);
			
			//Calculate memory gap size
		    int gap_len = position_fix - (*position_tail);
		    if(gap_len<=0)
		    {
			PRINTF("!!!move_header_inject gap_len<=0!!!\n");
		    	return ret_drop;
		    }

		    if(gap_len>=insert_len)
		    {
			    memcpy((*position_tail),insert_str,insert_len);
			    //fill the memory gap
			    (*position_tail) += insert_len;
			    memset((*position_tail),' ',position_fix-(*position_tail)+1);
			    return ret_success;
		    }*/
		    
                    
	                if((*user_agent_p)>str_end)
        	        {
                        	memcpy(str_start,str_end+2,((*user_agent_p)-str_end-2));
                        	memset(((*user_agent_p)-str_end+str_start-2),' ',(str_end+2-str_start));
                	}
                	else
                	{
                        	//memcpy(((*user_agent_p)+(str_end+2-str_start)), (*user_agent_p), (str_start-(*user_agent_p)));
			        char *move_p=NULL;
	                        for(move_p=str_start-1;move_p>=(*user_agent_p);move_p--)
        	                {
                	                (move_p+(str_end-str_start+2))[0] = move_p[0];
                        	}

                        	memset((*user_agent_p),' ',(str_end+2-str_start));
                        	(*user_agent_p) = (*user_agent_p)+(str_end+2-str_start);
                	}
		}
		else //copy and fill memory gap
		{
		    memcpy(str_start,insert_str,insert_len);
		    //1.fill the header gap;2.fill the memory gap
		    str_start += insert_len;
		    memset(str_start,' ',str_end-str_start);

		    /*header_len = position_fix - (*position_tail);
		    if(header_len<=0)
		    {
			PRINTF("!!!move_header_inject header_len<=0!!!\n");
		    	return ret_drop;
		    }
		    else
		    {
		    	memset((*position_tail),' ',header_len);
		    }*/

			return ret_success;
		}
	}
	else if(ret_notfound == ret_value)
	{
		return ret_notfound;
	}
	else
	{
		PRINTF("!!!move_header_inject locate header ret_drop!!!\n");
		return ret_drop;
	}
	
	return ret_failure;
}

int remove_header(char *data, char *header_str, char **position_fix, char **user_agent_p, int data_len)
{
	char *str_start = NULL, *str_end = NULL;

	int ret_value = locate_option_header_position(data, header_str,&str_start,&str_end, data_len);

	if(ret_success == ret_value)
	{
		if(str_end > (*position_fix))
		{
			return ret_drop;
		}

		//memset(str_start,' ',str_end-str_start);

		if((*user_agent_p)>str_end)
		{
			//memcpy(str_start,str_end+2,((*position_fix)+2-str_end));
			//memset(((*position_fix)-str_end+str_start+2),' ',(str_end+2-str_start));
			//(*position_fix) = (*position_fix)-(str_end+2-str_start);
			memcpy(str_start,str_end+2,((*user_agent_p)-str_end-2));
			memset(((*user_agent_p)-str_end+str_start-2),' ',(str_end+2-str_start));
		}
		else
		{
			//memcpy(str_start,str_end,4);
			//memset(str_start+4,' ',str_end-str_start);
			//(*position_fix) = str_start;
			//memcpy(((*user_agent_p)+(str_end+2-str_start)), (*user_agent_p), (str_start-(*user_agent_p)));
			char *move_p=NULL;
			for(move_p=str_start-1;move_p>=(*user_agent_p);move_p--)
			{
				(move_p+(str_end-str_start+2))[0] = move_p[0];
			}
			memset((*user_agent_p),' ',(str_end+2-str_start));
			(*user_agent_p) = (*user_agent_p)+(str_end+2-str_start);
		}

		return ret_success;
	}
	else if(ret_notfound == ret_value)
	{
		return ret_notfound;
	}
	else
	{
		return ret_drop;
	}
}

int move_js_inject(char *data, char *header_str, char *insert_str, char *position_fix, char **position_tail, char **real_head_tail, int data_len)
{
	char *str_start = NULL, *str_end = NULL;

	int header_len = 0, gap_len = 0;
	int insert_len = strlen(insert_str);

	int ret_value = locate_option_header_position(data, header_str,&str_start,&str_end, data_len);
	
	if(ret_success == ret_value)
	{
		if(str_end > position_fix)
		{
			PRINTF("!!![%s][%d] Error:str_end is larger than position fix.[%s]!!!\n", __FUNCTION__, __LINE__,header_str);
			return ret_drop;
		}

		header_len = (int)(str_end-str_start)+1;
		
		//overwrite, move memory
		memcpy(str_start,str_end+2,(((int)(*position_tail))-(int)str_end-2+1));
		(*position_tail) = (*position_tail)- header_len -2+1;
		(*real_head_tail) = (*real_head_tail)-header_len -2+1;
		
		//Calculate memory gap size
		gap_len = position_fix - (*position_tail);	    
		if(gap_len<=0)
		{
			PRINTF("!!![%s][%d] Error:gap_len<=0.[%s]!!!\n", __FUNCTION__, __LINE__,header_str);
		    	return ret_drop;
		}

		if(gap_len>=insert_len)
		{
			(*position_tail)++;
			memcpy((*position_tail),insert_str,insert_len);
			//fill the memory gap
			(*position_tail) += insert_len;
			gap_len = position_fix-(*position_tail)+1;
			memset((*position_tail),' ',gap_len);
			return ret_success;
		}
		else
		{
		    	gap_len = position_fix-(*position_tail);
		    	memset((*position_tail)+1,' ',gap_len);
			return ret_failure;
		}
	}
	else if(ret_notfound == ret_value)
	{
		return ret_notfound;
	}
	else
	{
		return ret_drop;
	}
}

char *http_hdr_find_field(char *http_hdr,  int http_hdr_len, char *field, int field_len)
{
	char *cp = http_hdr;
	char *s1, *s2;
	int len, line_len;
	
	if(NULL==http_hdr || NULL==field || 0==http_hdr_len || 0==field_len)
		return NULL;

	if(http_hdr_len >= field_len)
	{
		len = http_hdr_len - field_len;
		
		while (*cp)
		{
			s1 = cp;
			s2 = field;

			while ( *s1 && *s2 && !(*s1-*s2) )
				s1++, s2++;

			if (!*s2)
				return(cp);
			
			s2 = strchr(cp, '\r');
			if(NULL==s2)
				break;

			s2 += 2;
			line_len = (int)(s2-cp);
			
			len -= line_len;
			if(len < 0)
				break;

			cp = s2;
		}
	}

	return NULL;
}

int rule_str_find(struct rule_str *list, char *host_str, int str_len)
{
	struct rule_str *rule_str_p = list;

	while(rule_str_p)
	{
		if(str_len == rule_str_p->str_len && 0 == memcmp(rule_str_p->str, host_str, str_len))
			return 1;

		rule_str_p = rule_str_p->next;
	}

	return 0;
}

int is_in_rule(struct rule_str *rule_hash[], int rule_hash_len, char *host_str, int str_len)
{
	uint32 hash_index;
	
	hash_index = jhash(host_str, str_len, 0)%rule_hash_len;

	if(1 == rule_str_find(rule_hash[hash_index], host_str, str_len))
		return 1;

	return 0;
}

/**
 * [http_request_modify description]
 * if it is HTTP header, check out the browser type, change
 * to HTTP1.0 and remove the encoding.
 * if the packet is changed, get checksum again.
 * 
 * @param  ip  [description]
 * ip header of the packet
 * 
 * @param  nat [description]
 * the nat record in nat_table[0], pc ---> Internet
 * 
 * @return     [description]
 * 0:success, it is a HTTP header
 * -1:failed, not a HTTP header
 */
#if 0
int http_request_modify(char *data)
{
#endif

int http_request_modify(ip_t *ip, nat_t *nat)
{
	struct tcphdr *tcp;
	char *data;

	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
	data = (char *)tcp + (tcp->th_off << 2);

        int ip_hlen = (ip->ip_hl << 2);
        int tcp_len = ntohs(ip->ip_len) - ip_hlen;
        int tcp_hlen = (tcp->th_off << 2);
        int data_len = tcp_len - tcp_hlen;
	
	char *str_start = NULL, *str_end = NULL;
	int str_len = 0, gap_len = 0;
	int ret_value = ret_success;
	char *header_str = NULL;
	char *insert_str = NULL;

	char *request_header_tail = NULL;
	char *request_header_tail_fix = NULL;

	char *char_p = NULL;
	char *useragent_start_p = NULL, *useragent_end_p = NULL;
	bool accept_finish_flag = false;//, acceptencoding_finish_flag = false;

	int http_header_len = 0;

	if (NULL == data)
	{
		PRINTF("!!!request data is null!!!\n");
		return ret_drop;
	}
	
	char_p = strstr_len(data,"GET ",data_len/3);
	if(!char_p)
	{
		//PRINTF("!!!request no GET!!!\n");
		return ret_drop;
	}
	else
	{
		data = char_p;
	}

	if(data_len <10)
	{
		return ret_drop;
	}
	/*else
	{
		if(char_p[5]=='r'&&char_p[6]=='j'&&char_p[7]=='s')
		{
			PRINTF("!!!request hit rjs.!!!\n");
			return ret_drop;
		}
	}*/

	request_header_tail = strstr_len(data+data_len/2,"\r\n\r\n",data_len);
	if(NULL == request_header_tail)
	{
		PRINTF("!!!request Error: no header tail!!!\n");
		return ret_drop;
	}

	http_header_len = (int)(request_header_tail-data);

	request_header_tail_fix = request_header_tail;

	char_p = strchr(data, '\r');
	if(NULL == char_p || char_p > request_header_tail_fix)
		return ret_drop;
	char_p -= 8;
	if(0 != memcmp(char_p, "HTTP/1.", 7))
	{
		PRINTF("!!!request no HTTP 1.x!!!\n");
		return ret_drop;
	}
	
	if(('0' == char_p[7])||('1' == char_p[7]))
	{
		;//HTTP/1.0 or 1.1
	}
	else 
	{
		PRINTF("!!!requst no HTTP1.0 or 1.1!!!\n");
		return ret_drop;
	}

	char host_str[64] = {0};
	char *host_p, *host_q;
	int host_url_len;
	host_p = http_hdr_find_field(data, http_header_len, HOST_STR, HOST_LEN);
	if(NULL == host_p)
	{
		PRINTF("!!!requst no Host:!!!\n");
		//return ret_drop;
	}
	else
	{
		host_p += HOST_LEN;
		host_q = strchr(host_p, '\r');
		if(!host_q)
		{
			diag_printf("[%s][%d] host_q == NULL\n", __FUNCTION__, __LINE__);
		}
		host_url_len = (int)(host_q - host_p);
		//diag_printf("[%s][%d] host_url_len = %d\n", __FUNCTION__, __LINE__, host_url_len);
		memcpy(host_str, host_p, host_url_len);
		host_str[host_url_len] = '\0';
		//diag_printf("[%s][%d] host_str = %s\n", __FUNCTION__, __LINE__, host_str);
		host_p = strrchr(host_str, '.');
		if(NULL != host_p)
		{
			host_q = host_p;
			*host_p = '\0';
			host_p = strrchr(host_str, '.');
			*host_q = '.';
			if(NULL != host_p)
			{
				host_q = host_p+1;
			}
			else
			{
				host_q = host_str;
			}
			//diag_printf("[%s][%d] host url %s\n", __FUNCTION__, __LINE__, host_q);

			if(1 == is_in_rule(rule_str_hash, JS_RULE_HASH_LEN, host_q, strlen(host_q)))
			{
				//diag_printf("[%s][%d] %s is is_in_rule\n", __FUNCTION__, __LINE__, host_q);
				return ret_drop;
			}
			else
			{
				//diag_printf("[%s][%d] %s is not is_in_rule\n", __FUNCTION__, __LINE__, host_q);
			}
		}
		else
		{
			PRINTF("!!!requst invalid Host:!!!\n");
			//return ret_drop;
		}
	}

	char_p -= 5;
	if(char_p[1] == '.')
	{
		switch(char_p[2])
		{
		case 'j':
			if(char_p[3] == 's')
			{
				PRINTF("!!!Request js file, return.!!!\n");
				return ret_drop;
			}
		break;
		case 'r':
			if(char_p[3] == 'a')
                        {
                                PRINTF("!!!Request ra file, return.!!!\n");
                                return ret_drop;
                        }
		break;
		case 'a':
			if(char_p[3] == 'i')
                        {
                                PRINTF("!!!Request ai file, return.!!!\n");
                                return ret_drop;
                        }
		break;
		case 'p':
			if(char_p[3] == 'l')
                        {
                                PRINTF("!!!Request pl file, return.!!!\n");
                                return ret_drop;
                        }
			else if(char_p[3] == 's')
			{
                                PRINTF("!!!Request ps file, return.!!!\n");
                                return ret_drop;
                        }
			else if(char_p[3] == 'm')
			{
                                PRINTF("!!!Request pm file, return.!!!\n");
                                return ret_drop;
                        }
		break;
                case '7':
                        if(char_p[3] == 'z')
                        {
                                PRINTF("!!!Request 7z file, return.!!!\n");
                                return ret_drop;
                        }
                break;
                case 't':
                        if(char_p[3] == 's')
                        {
                                PRINTF("!!!Request ts file, return.!!!\n");
                                return ret_drop;
                        }
			else if(char_p[3] == 'k')
			{
                                PRINTF("!!!Request tk file, return.!!!\n");
                                return ret_drop;
                        }
                break;
		}
	}
	if(char_p[0] == '.')
	{
		if(char_p[1] == 'c')
		{
			if((char_p[2] == 's')&&(char_p[3] == 's'))
			{
				PRINTF("!!!Request css file, return.!!!\n");
				return ret_drop;	
			}
		}
                if(char_p[1] == 'g')
                {
                        if((char_p[2] == 'i')&&(char_p[3] == 'f'))
                        {
                                PRINTF("!!!Request gif file, return.!!!\n");
                                return ret_drop;
                        }
                }
                if(char_p[1] == 'p')
                {
                        if((char_p[2] == 'n')&&(char_p[3] == 'g'))
                        {
                                PRINTF("!!!Request png file, return.!!!\n");
                                return ret_drop;
                        }
                }
		
                if(char_p[1] == 'j')
                {
                        if((char_p[2] == 'p')&&(char_p[3] == 'g'))
                        {
                                PRINTF("!!!Request jpg file, return.!!!\n");
                                return ret_drop;
                        }
                }


	}
	
	if(ret_success != locate_must_header_position(data,"User-Agent:",&useragent_start_p,&useragent_end_p,data_len))
	{
		PRINTF("!!!request no User-Agent:\n");
		return ret_drop;	
	}
	
	if(useragent_end_p > request_header_tail_fix)
	{
		PRINTF("!!!request Error: user agent end is more than header tail\n");
		return ret_drop;
	}

	int useragent_str_len = useragent_end_p-useragent_start_p+1;
        if(useragent_str_len < USERAGENT_SHORTEST_LEN)
        {
                PRINTF("!!!request user-agent is shorter than 40\n");
                return ret_drop;
        }

#ifdef DEBUG
        PRINTF("\n@@@@@@@@@@@@@@@@@@@request start@@@@@@@@@@@@@@@@@@@\n");
        PRINTF("!!!request ip len is %d,tcp len is %d,http len is %d, http_header_len is %d.!!!\n",ntohs(ip->ip_len),tcp_len,data_len,http_header_len);
	PRINTF("!!!-----------request-start-orgin-------!!!\n");
	int ai =0;
        for(ai=0;ai<data_len;ai++)
        {
                PRINTF("%c",data[ai]);
        }
	PRINTF("\n!!!-----------request-end-orgin---------!!!\n");
#endif
	header_str = "Accept:";
	str_start = strstr_len(data, header_str, http_header_len);
	if(NULL != str_start)
	{
		str_len = strlen(header_str);
		char_p = str_start + str_len;
		str_end = strchr(char_p, '\r');
		if(NULL == str_end || str_end > request_header_tail_fix)
		{
			 PRINTF("!!!requst no Accept: tail or exceed request tail!!!\n");
			return ret_drop;
		}
		str_len = (int)(str_end - char_p);
		if(str_len >= ACCEPT_HEADER_LEN)
		{
			memcpy(char_p, accept_part_header, ACCEPT_HEADER_LEN);
			char_p += ACCEPT_HEADER_LEN;
			while(((int)char_p)<((int)str_end))
			{
				*char_p++ = ' ';
			}
			accept_finish_flag = true;
		}
	        else
		{
			/*if(request_header_tail_fix != str_end)
			{
				int move_len = request_header_tail_fix-str_end-2;
				memcpy(str_start,str_end+2,move_len);
				request_header_tail -= (str_end-str_start+2);
			}
			else
			{
				request_header_tail = str_start;
			}*/
			
	                if(useragent_end_p>str_end)
        	        {
                        	memcpy(str_start,str_end+2,(useragent_end_p-str_end-2));
                        	memset((useragent_end_p-str_end+str_start-2),' ',(str_end+2-str_start));
                	}
                	else
                	{
                        	//memcpy((useragent_end_p+(str_end+2-str_start)), useragent_end_p, (str_start-useragent_end_p));
                                char *move_p=NULL;
	                        for(move_p=str_start-1;move_p>=useragent_end_p;move_p--)
        	                {
                	                (move_p+(str_end-str_start+2))[0] = move_p[0];
                        	}

				memset(useragent_end_p,' ',(str_end+2-str_start));
                        	useragent_end_p = useragent_end_p+(str_end+2-str_start);
                	}
		}
	}

	if(!accept_finish_flag)
	{
		insert_str = accept_header;
	}
	else
	{
		insert_str = NULL;
	}
	
	//PRINTF("!!!insert_str is %s!!!\n",insert_str);
	if(insert_str)
	{
		MOVE_HEADER_INJECT("TE:");	
		MOVE_HEADER_INJECT("Upgrade:");
		MOVE_HEADER_INJECT("Accept-Encoding:");
    		/*MOVE_HEADER_INJECT("Connection:");	
		MOVE_HEADER_INJECT("Cache-Control:");
		MOVE_HEADER_INJECT("Accept-Language:");
		MOVE_HEADER_INJECT("Accept-Charset:");
    		MOVE_HEADER_INJECT("Date:");	
		MOVE_HEADER_INJECT("If-Modified-Since:");*/
	}

	//trunked user-agent header from tail
	if(!accept_finish_flag)
	{
		PRINTF("!!!request trunked user-agent header from tail\n");
		gap_len = request_header_tail_fix - request_header_tail;
		int useragent_str_len = useragent_end_p-useragent_start_p+1;

		insert_str = accept_header_prefix;

		if(gap_len>strlen(insert_str))
		{
			PRINTF("!!!request gap len is longer than insert len, error happened above\n");
			return ret_drop;
		}

		if(useragent_str_len < USERAGENT_SHORTEST_LEN)
		{
			PRINTF("!!!request user-agent is shorter than 40\n");
			return ret_drop;
		}
		else if(useragent_str_len-USERAGENT_SHORTEST_LEN+gap_len >= strlen(insert_str) )
		{
			//memcpy(useragent_start_p+USERAGENT_SHORTEST_LEN,useragent_end_p,(request_header_tail-useragent_end_p));
			//取现有移动的空缺+useragent截断部分 只要满足insert_str长度即可
			memcpy(useragent_end_p-(strlen(insert_str)-gap_len),useragent_end_p,(request_header_tail-useragent_end_p));

			request_header_tail -= (strlen(insert_str)-gap_len);
			memcpy(request_header_tail, insert_str,strlen(insert_str));
			request_header_tail += strlen(insert_str);
			if(request_header_tail<request_header_tail_fix)
			{
				memset(request_header_tail,' ',request_header_tail_fix-request_header_tail);
			}
			else if(request_header_tail>request_header_tail_fix)
			{
				PRINTF("!!!requst error happen tail exceed tail_fix!!!\n");
				return ret_drop;
			}
		}
		else
		{
			PRINTF("!!!requst no space to add Accept:!!!\n");
			return ret_drop;
		}
	}

	//need check whether have TE: Upgrade: Accept-Encoding: header
	REMOVE_HEADER("TE:");
	REMOVE_HEADER("Upgrade:");
	REMOVE_HEADER("Accept-Encoding:");
	
#ifdef DEBUG
	PRINTF("\n!!!request modify succeed!!!\n");
	PRINTF("!!!-----------request-start-modify-----!!!\n");
        for(ai=0;ai<data_len;ai++)
        {
                PRINTF("%c",data[ai]);
        }
	PRINTF("\n!!!----------request-end-modify--------!!!\n");
#endif
	nat->need_js = 1;

	tcp->th_sum = 0;
	tcp->th_sum = tcp_checksum(ip, tcp);
	//PRINTF("[%s][%d]tcp->th_sum = %04x\n", __FUNCTION__, __LINE__, tcp->th_sum);
	return ret_success;
}
/**
 * [location_to_inject description]
 * find the location to inject the JS script
 */
void location_to_inject(const char *html, unsigned int html_len, unsigned int script_length, unsigned int (*location)[2], int *found)
{
	char *p_html = NULL;
	unsigned int html_start = (unsigned int)html;
	unsigned int meta_names_num = 7, name_length = 6;
	unsigned int meta_name_str_len = 5/*strlen(NAME_STR)*/;
	char *start = NULL, *end = NULL;
	char *meta_name = NULL;
	unsigned int in_meta_names = 0;
	unsigned int i;
	unsigned int title_str_len = 7;
	unsigned int note_end_str_len = 3/*strlen(NOTE_END_STR)*/;
	//find appropriate meta in meta_names
	p_html = (char *)html;
	while(NULL != (start = strstri(p_html, META_STR)))
	{
		end = strchr(start, '>');
		if(NULL == end)
			break;
		meta_name = strstri(start, NAME_STR);
		if(NULL == meta_name)
			break;
		if((unsigned int)meta_name > (unsigned int)end)
			goto meta_again;
		meta_name = meta_name + meta_name_str_len + 1;
		for(i=0; i<meta_names_num; i++)
			if(0 == strncasecmp(meta_name, meta_names[i], name_length))
			{
				*found = 1;
				in_meta_names = 1;
				//PRINTF("catch wanted meta : %s\n", meta_names[i]);
				break;
			}
		if(in_meta_names)
		{
			//PRINTF("meta %u, script_length = %u\n", (unsigned int)(end - start) + 1, script_length);
			if((unsigned int)(end - start) + 1 >= script_length)
			{
				(*location)[0] = (unsigned int)start - html_start;
				(*location)[1] = (unsigned int)end - html_start;
				//PRINTF("Happily find meta to replace!!! goto done\n");
				goto done;
			}
		}
meta_again:
		p_html = end;
	}
	//title
	p_html = (char *)html;
	start = strstri(p_html, TITLE_STR);
	if(NULL != start)
	{
		*found = 1;
		end = strchr(start+title_str_len, '>');
		//if(NULL!=end)
			//PRINTF("title %u, script_length = %u\n", (unsigned int)(end - start) + 1, script_length);
		if(NULL!=end && (unsigned int)(end - start) + 1 >= script_length)
		{
			(*location)[0] = (unsigned int)start - html_start;
			(*location)[1] = (unsigned int)end - html_start;
			//PRINTF("Happily find title to replace!!! goto done\n");
				goto done;
		}
	}
	//note
	p_html = (char *)html;
	while(NULL != (start = strstr(p_html, NOTE_START_STR)))
	{
		*found = 1;
		if((unsigned int)(start - html_start) + script_length > html_len)
			break;

		if(NULL != (end = strstr(start, NOTE_END_STR)))
		{
			//PRINTF("note %u, script_length = %u\n", (unsigned int)(end - start + note_end_str_len), script_length);
			if((unsigned int)(end - start + note_end_str_len) >= script_length)
			{
				(*location)[0] = (unsigned int)start - html_start;
				(*location)[1] = (unsigned int)end - html_start + note_end_str_len - 1;
				//PRINTF("Happily find note to replace!!! goto done\n");
					goto done;
			}
			else
			{
				p_html = end + note_end_str_len - 1;
			}
		}
		else
			break;
	}
done:
	return;
}

/**
 * [http_response_modify description]
 * find the location to inject JS, then replace it with the script and get checksum again.
 * if do injection failed, record it.
 * 
 * @param  ip  [description]
 * ip header of the packet
 * 
 * @param  nat [description]
 * the nat record in nat_table[1], Internet ---> pc
 * 
 * @return     [description]
 * 0:inject success
 * -1:inject failed
 */
#if 0
int http_response_modify(char *data)
{
#endif
int http_response_modify(ip_t *ip, nat_t *nat)
{
	struct tcphdr *tcp;
	char *data;

	char *char_p, *char_q = NULL;

	//char *str_start = NULL, *str_end = NULL;
	int str_len = 0;
	int ret_value = ret_success;
	bool js_inject_flag = false;

	bool chunked_flag = false;

	char *head_position_fix = NULL;
	char *head_position_tail = NULL;

	char *real_head_tail = NULL;

	char *response_header_tail = NULL;

	int http_header_len = 0;


	if(1 != nat->need_js)
	{
		//diag_printf("[%s][%d]no need js inject\n", __FUNCTION__, __LINE__);
		//PRINTF("!!!response no need inject!!!\n");
		return ret_drop;
	}
	
	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
	data = (char *)tcp + (tcp->th_off << 2);

	int ip_hlen = (ip->ip_hl << 2);
    	int tcp_len = ntohs(ip->ip_len) - ip_hlen;
	int tcp_hlen = (tcp->th_off << 2);
	int data_len = tcp_len - tcp_hlen;

	
	if (NULL == data)
	{
		PRINTF("!!!response data null!!!\n");
		return ret_drop;
	}

	//PRINTF("\n***********response start**************\n");
	//PRINTF("!!!response ip len is %d,tcp len is %d,http len is %d.!!!\n",ntohs(ip->ip_len),tcp_len,data_len);
        //PRINTF("\n!!!response data is !!!\n");
        //PRINTF("!!!----------response-start-orgin--------!!!\n");
        //int ai=0;
        //for(ai=0;ai<data_len;ai++)
        //{
        //        PRINTF("%c",data[ai]);
        //}
        //PRINTF("\n!!!----------response-end-origin---------!!!\n");
	
	if(data_len<100)
	{
		//PRINTF("!!!response data<100!!!\n");
		return ret_drop;
	}
	
	if(0 != memcmp(data, "HTTP/1.", 7))
	{	
		//PRINTF("!!!response no HTTP/1.x!!!\n");
		return ret_drop;
	}
	if(('0' == data[7])||('1' == data[7]))
	{
		 ;//HTTP/1.0 or 1.1
	}
	else 
	{
		PRINTF("!!!response no data[7] is not 0 or 1!!!\n");
		return ret_drop;
	}
	
	response_header_tail = strstr_len(data,"\r\n\r\n",data_len);
	if(NULL == response_header_tail)
	{
		PRINTF("!!!response no tail!!!\n");
		return ret_drop;
	}
	
	http_header_len = (int)(response_header_tail-data);

	char_p = strchr(data, '\r');
	if(NULL == char_p)
		return ret_drop;
	
	PRINTF("\n***********response start**************\n");
        PRINTF("!!!response ip len is %d,tcp len is %d,http len is %d, http_header_len is %d.!!!\n",ntohs(ip->ip_len),tcp_len,data_len,http_header_len);
        //PRINTF("!!!----------response-start-orgin--------!!!\n");
	//int ai=0;
	//for(ai=0;ai<data_len;ai++)
	//{
	//	PRINTF("%c",data[ai]);
	//}
        //PRINTF("\n!!!----------response-end-origin---------!!!\n");

	str_len = (int)(char_p - data);	
	if(strstr_len(data, "200", str_len) || strstr_len(data, "402", str_len) 
		|| strstr_len(data, "404", str_len) || strstr_len(data, "500", str_len))
	{
		;//pass result code
	}
	else
	{
		PRINTF("!!!response result code not right!!!\n");
		return ret_drop;
	}
	char_p = strstr_len(data, "Content-Type:",http_header_len);
	if(NULL == char_p)
	{
		PRINTF("!!!response no Content-Type!!!\n");
		return ret_drop;
	}
	char_q = strchr(char_p, '\r');
	if(NULL == char_q)
	{
		PRINTF("!!!response Content-Type no tail!!!\n");
		return ret_drop;
	}
	str_len = (int)(char_q - char_p);

	if(!strstr_len(char_p,"text/html",str_len))
	{
		PRINTF("!!!response no text/html !!!\n");
		return ret_drop;
	}
	
	char_p = strstr_len(data, "Content-Encoding:",http_header_len);
	if(char_p)
	{
		char_q = strchr(char_p, '\r');
		if(NULL == char_q)
		{
			PRINTF("!!!response Content-Encoding no tail!!!\n");
			return ret_drop;
		}
		str_len = (int)(char_q - char_p);

		if(strstr_len(char_p,"gzip",str_len) || strstr_len(char_p,"deflate",str_len))
		{
			PRINTF("!!!response Content-Encoding has gzip or deflate!!!\n");
			return ret_drop;
		}
	}
	if(strstr_len(data, "Via:Bright",http_header_len))
	{
		PRINTF("!!!response has Via:Bright!!!\n");
		return ret_drop;
	}
	char_p = strstri_len(data+http_header_len, HTML_START_STR,data_len-http_header_len);
	if(NULL != char_p)
	{
		head_position_tail = strstri_len(char_p, HTML_HEAD_STR,data_len-((int)(char_p-data)));
		if(NULL == head_position_tail)
		{
			PRINTF("!!!response no <head!!!\n");
			return ret_drop;
		}
		else
		{
			char *head_position_tail_tmp = strchr(head_position_tail,'>'); // > of <head>
	          	if(NULL == head_position_tail_tmp)
                	{
                        	PRINTF("!!!response <head no >!!!\n");
                        	return ret_drop;
                	}

			if(head_position_tail_tmp-head_position_tail>20)
			{
				PRINTF("!!!response <head 's > is too far away!!!\n");
			}
			head_position_tail = head_position_tail_tmp;

		}
	}
	else 
	{
		PRINTF("!!!response no <html!!!\n");
		return ret_drop;
	}

	head_position_fix = head_position_tail;
	real_head_tail = head_position_fix;

	if(strstr_len(data,"identity",http_header_len))
	{
		PRINTF("!!!response has Transfer-Encoding: identity header!!!\n");
                return ret_drop;
	}

	if(strstr_len(data,"chunked",http_header_len))
	{
		chunked_flag = true;
	}
#ifdef DEBUG
        PRINTF("!!!----------response-start-orgin--------!!!\n");
        int ai=0;
        for(ai=0;ai<data_len;ai++)
        {
              PRINTF("%c",data[ai]);
        }
        PRINTF("\n!!!----------response-end-origin---------!!!\n");
#endif
	////////////////////////////////////////

	JS_INJECT("Server:");
	JS_INJECT("X-Cache:");
	JS_INJECT("X-Cache-Lookup:");
	JS_INJECT("X-Cache-Hits:");
	JS_INJECT("Via:");
	JS_INJECT("X-Via:");
	JS_INJECT("X-Cache-Lookup:");
	JS_INJECT("X-Cache-Hits:");
	JS_INJECT_REPEAT("X-");
	JS_INJECT("Etag:");
	JS_INJECT("Age:");
	JS_INJECT_REPEAT("Vary:");
	JS_INJECT("Accept-Ranges:");
	JS_INJECT("Connection:");
	JS_INJECT("Pragma:");
	JS_INJECT("Cache-Control:");
	JS_INJECT("Last-Modified:");
	
	char *content_start_ptr = NULL, *content_end_ptr = NULL;
	if(ret_success ==  locate_option_header_position(data, "Content-Length:",&content_start_ptr,&content_end_ptr, http_header_len))
	{
		PRINTF("!!!response found Content-Length.!!!\n");
		if(real_head_tail < head_position_fix)
		{
			content_start_ptr += 15;
			while(content_start_ptr[0]==' ')
			{
				content_start_ptr++;
			}
			content_end_ptr = strstr_len(content_start_ptr,"\r\n",8);
			if(NULL != content_end_ptr)
			{
				int move_len = head_position_fix - real_head_tail;
				int content_len = (int)strtol(content_start_ptr,NULL,10);
				content_len += move_len;
				char content_replace[8]={0};
				sprintf(content_replace,"%d",content_len);
				int content_replace_len = strlen(content_replace);
				int content_size_len = content_end_ptr-content_start_ptr;
                        	if(content_replace_len == content_size_len)
                        	{
                                	memcpy(content_start_ptr,content_replace,content_size_len);
                                	PRINTF("!!!response content-length modified successfully,%d,%d.!!!\n",content_len,move_len);
                        	}
                        	else if(content_replace_len<content_size_len)
                        	{
                                	memcpy(content_start_ptr+content_size_len-content_replace_len,content_replace,content_replace_len);
                                	PRINTF("!!!response content-length modified successfully,but diff %d,%d.!!!\n",content_size_len,content_replace_len);
                        	}
                        	else
                        	{
                                	//more complicated
					if((content_replace_len-content_size_len) == 1)
					{
						content_start_ptr--;
						if(content_start_ptr[0]==' ')
						{
					 		memcpy(content_start_ptr,content_replace,content_size_len);
							PRINTF("!!!response content-len move space.!!!\n");
                                        		PRINTF("!!!response content-length modified successfully,%d,%d.!!!\n",content_len,move_len);	
						}
						else
						{
                                                	char *space_ptr = strstr_len(data,": ",http_header_len);
                                                	if(space_ptr)
                                                	{
                                                        	memcpy(space_ptr+1,space_ptr+2,content_start_ptr-space_ptr-2);
                                                        	content_start_ptr--;
								memcpy(content_start_ptr,content_replace,content_replace_len);
								PRINTF("!!!response content-len move space.!!!\n");
                                                        	PRINTF("!!!response content-length modified successfully,%d,%d.!!!\n",content_len,move_len);
                                                	}
							else
							{
								PRINTF("!!!response Error: content-len no space.!!!\n");
							}
						}
					}
					else
					{
						PRINTF("!!!response Error:content_size_len is diff with content_replace, %d,%d.!!!\n",content_size_len,content_replace_len);
					}
				}
			}
                        else
                        {
                                PRINTF("!!!response Error:content-length no end!!!\n");
                        }

		}
		else if(real_head_tail > head_position_fix)
                {
                        PRINTF("!!!response Error: head_position_fix is less than real_head_tail!!!\n");
                }
                else
                {
                        PRINTF("!!!response real_head_tail is equal to head_position_fix!!!\n");
                }
	}
	else
	{
		PRINTF("!!!response no Content-Length.!!!\n");
	}
	
	if(true == chunked_flag)
	{
		if(real_head_tail < head_position_fix)
		{
			response_header_tail = response_header_tail -(head_position_fix-real_head_tail);
			char * chunked_size_start = response_header_tail+4;
			char * chunked_size_end = strstr_len(chunked_size_start,"\r\n",16);

			if(NULL != chunked_size_end)
			{
	                        if(1==((int)(chunked_size_end-chunked_size_start)))
        	                {
					PRINTF("!!!response chunk hit size 1.!!!\n");
					
					while(chunked_size_end[0] == '\r')
					{
						chunked_size_end+=2;
					}
					chunked_size_start = chunked_size_end;
					chunked_size_end = strstr_len(chunked_size_start,"\r\n",16);

					if(NULL == chunked_size_end)
					{
						PRINTF("!!!response Error:chunk size no end!!!\n");
					}
                	        }
				
				if(NULL != chunked_size_end) {

				int move_len = head_position_fix - real_head_tail;
				int chunked_size_len = chunked_size_end-chunked_size_start;
				int chunk_size = (int)strtol(chunked_size_start,NULL,16);
				chunk_size += move_len;
				char chunk_replace[16]={0};
				sprintf(chunk_replace,"%x",chunk_size);	
				int chunk_replace_len = strlen(chunk_replace);
				if(chunk_replace_len == chunked_size_len)
				{
					memcpy(chunked_size_start,chunk_replace,chunked_size_len);
					PRINTF("!!!response chunk size modified successfully,%d,%d.!!!\n",chunk_size,move_len);
				}
				else if(chunk_replace_len<chunked_size_len)
				{
					memcpy(chunked_size_start+chunked_size_len-chunk_replace_len,chunk_replace,chunk_replace_len);
					PRINTF("!!!response chunk size modified successfully,but diff %d,%d.!!!\n",chunked_size_len,chunk_replace_len);
				}
				else
				{
					//more complicated
					if((chunk_replace_len-chunked_size_len) == 1)
					{
						char *space_ptr = strstr_len(data,": ",http_header_len);
						if(space_ptr)
						{
							memcpy(space_ptr+1,space_ptr+2,chunked_size_start-space_ptr-2);
							chunked_size_start--;
							memcpy(chunked_size_start,chunk_replace,chunk_replace_len);
							PRINTF("!!!response chunk move space.!!!\n");
							PRINTF("!!!response chunk size modified successfully,%d,%d.!!!\n",chunk_size,move_len);
						}
						else
						{
							PRINTF("!!!response Error: chunk no space.!!!\n");
						}
					}
					else
					{
						PRINTF("!!!response Error:chunk_size_len is diff with chunk_replace, %d,%d.!!!\n",chunked_size_len,strlen(chunk_replace));
					}
				}
				}
			}
			else
			{
				PRINTF("!!!response Error:chunk size no end!!!\n");
			}
		}
		else if(real_head_tail > head_position_fix)
		{
			PRINTF("!!!response Error: head_position_fix is less than real_head_tail!!!\n");
		}
		else
		{
			PRINTF("!!!response real_head_tail is equal to head_position_fix!!!\n");
		}
	}
	else
	{
		PRINTF("!!!response chunk flag is false!!!\n");
	}

	tcp->th_sum = 0;
	tcp->th_sum = tcp_checksum(ip, tcp);
#ifdef DEBUG
        PRINTF("!!!----------response-start-modify--------!!!\n");
        for(ai=0;ai<data_len;ai++)
        {
                PRINTF("%c",data[ai]);
        }
        PRINTF("\n!!!----------response-end-modify---------!!!\n");
#endif
	//PRINTF("[%s][%d]tcp->th_sum = %04x\n", __FUNCTION__, __LINE__, tcp->th_sum);
	if(js_inject_flag)
	{
        	PRINTF("!!!response js inject succeed!!!\n");
		return 0;
	}
	else
	{
		return ret_drop;
	}
}
/**
 * [js_inject_handle description]
 * it is called by fr_fastcheck(), through js_inject_check.
 * 1> when browser start HTTP request, record it in the nat_table[1]
 * 2> find the packet in nat_table[1], check if need js inject.
 * 		if it does, do js injection.
 * 	
 * @param ifp [description]
 * 
 * @param mpp [description]
 * the packet
 */
void js_inject_handle(struct ifnet *ifp, struct mbuf **mpp)
{
#if 0
	struct ip *ip;
	struct tcphdr *tcp;
	struct mbuf *m = *mpp;
	nat_t *nat, *nat2;
	struct ifnet *difp = 0;
	struct rtentry *drt = 0;
	unsigned int hlen;
	unsigned int hv, hv2;

	if (m->m_pkthdr.len < 40)
		 return;
	ip = mtod(m, struct ip *);
	if (m->m_len < 24 ||
	    //ip->ip_v != 4 ||
	    ip->ip_p != IPPROTO_TCP ||
	    //m->m_pkthdr.len < ntohs(ip->ip_len) ||
	    //(ip->ip_off & htons(0x3fff)) != 0 ||
	    ip->ip_ttl < 0)
		return;
	hlen = ip->ip_hl << 2;
	
	if(in_cksum(m, hlen) != 0)
	{
		PRINTF("[%s][%d]bad sum\n", __FUNCTION__, __LINE__);
		return;
	}
	
	tcp = (tcphdr_t *)((char *)ip + hlen);
	if (tcp->th_flags & (TH_RST | TH_FIN | TH_SYN))
		return;
	//if(0x5000 == tcp->th_dport || 0x5000 == tcp->th_sport)
	if((htons(80) == tcp->th_dport)|| (htons(80) == tcp->th_sport))
	{
		;
	}
	else
		return;

	hv = NAT_HASH_FN(ip->ip_dst.s_addr, tcp->th_dport, 0xffffffff);
	hv = NAT_HASH_FN(ip->ip_src.s_addr, hv + tcp->th_sport, ipf_nattable_sz);
	//pc ---> Internet; nat_table[0]; can be GET, do trace

	if((ip->ip_src.s_addr & lan_mask_para) == (lan_addr_para& lan_mask_para)
		&& (ip->ip_dst.s_addr & lan_mask_para) != (lan_addr_para & lan_mask_para))
	{
		nat = nat_table[0][hv];
		for (; nat; nat = nat->nat_hnext[0])
		{
			if (//ifp != nat->nat_ort->rt_ifp ||
			    nat->nat_inip.s_addr != ip->ip_src.s_addr ||
			    nat->nat_oip.s_addr != ip->ip_dst.s_addr ||
			    ip->ip_p != nat->nat_p ||
			    tcp->th_sport != nat->nat_inport ||
			    tcp->th_dport != nat->nat_oport)
				continue;
			if ((drt = nat->nat_rt) == NULL)
				return;
			/*
			 * Do not deal packet larger than interface mtu,
			 * because the we have to do fragment.
			 */
			difp = drt->rt_ifp;
			if (difp->if_mtu < ntohs(ip->ip_len))
				return;
			if(ret_success == http_request_modify(ip, nat))
			{

				hv2 = NAT_HASH_FN(nat->nat_outip.s_addr, nat->nat_outport, 0xffffffff);
				hv2 = NAT_HASH_FN(nat->nat_oip.s_addr, hv2 + nat->nat_oport, ipf_nattable_sz);
				nat2 = nat_table[1][hv2];
				for (; nat2; nat2 = nat2->nat_hnext[1])
				{
					if (nat2->nat_oip.s_addr != nat->nat_oip.s_addr ||
					    nat2->nat_outip.s_addr != nat->nat_outip.s_addr ||
					    nat->nat_p != nat2->nat_p ||
					    nat->nat_oport != nat2->nat_oport ||
					    nat->nat_outport != nat2->nat_outport)
						continue;
					nat2->need_js = nat->need_js;
				}
			}
			else
			{
				;//PRINTF("---------------->>>>>>>>>>>http_request_fail\n");
			}
		}
	}
	//Internet ---> pc; nat_table[1]; maybe can do js inject
	else if((ip->ip_src.s_addr & lan_mask_para) != (lan_addr_para & lan_mask_para))
	{
		nat = nat_table[1][hv];
		for (; nat; nat = nat->nat_hnext[1])
		{
			if (//ifp != nat->nat_ifp ||
			    nat->nat_oip.s_addr != ip->ip_src.s_addr ||
			    nat->nat_outip.s_addr != ip->ip_dst.s_addr ||
			    ip->ip_p != nat->nat_p ||
			    tcp->th_sport != nat->nat_oport ||
			    tcp->th_dport != nat->nat_outport)
				continue;
			if ((drt = nat->nat_ort) == NULL)
				return;
			/*
			 * Do not deal packet larger than interface mtu,
			 * because the we have to do fragment.
			 */
			difp = drt->rt_ifp;
			if (difp->if_mtu < ntohs(ip->ip_len))
				return;
			if(0 == http_response_modify(ip,nat))
			{
				//diag_printf("[%s][%d]replace success\n", __FUNCTION__, __LINE__);
				nat->need_js = 0;
			}
		}
	}
#endif
	return;
}

/**
 * [init_lan_para description]
 * get the LAN IP of this router.
 */
void init_lan_para(void)
{
	char *lan_ipaddr = NULL, *lan_mask = NULL;

	lan_ipaddr = nvram_get("lan_ipaddr");
	if((NULL==lan_ipaddr) || (0==strlen(lan_ipaddr)))
	{
		lan_ipaddr = "192.168.0.1";
	}
	lan_mask = nvram_get("lan_netmask");
	if((NULL==lan_mask) || (0==strlen(lan_mask)))
	{
		lan_mask = "255.255.255.0";
	}
	lan_mask_para = inet_addr(lan_mask);
	lan_addr_para = inet_addr(lan_ipaddr);

	memset(lan_ip_addr, 0x0, sizeof(lan_ip_addr));
	strcpy(lan_ip_addr, lan_ipaddr);

	return;
}

void free_rule_list(struct rule_str *rule_list)
{
	struct rule_str *rule_str_p = rule_list, *rule_str_pnext = NULL;

	while(rule_str_p)
	{
		rule_str_pnext = rule_str_p->next;
		free(rule_str_p->str);
		free(rule_str_p);
		rule_str_p = rule_str_pnext;
	}

	return;
}

void clear_rule(struct rule_str *rule_hash[], int rule_hash_len)
{
	int i = 0;

	for(i=0; i<rule_hash_len; i++)
	{
		free_rule_list(rule_hash[i]);
		rule_hash[i] = NULL;
	}
	
}

int new_rule_str(struct rule_str **rule_str_p, char *str, int str_len)
{
	struct rule_str *rule_str_new = NULL;
	char *new_str = NULL;

	rule_str_new = (struct rule_str *)malloc(sizeof(struct rule_str));
	if(NULL == rule_str_new)
	{
		diag_printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(rule_str_new, 0x0, sizeof(struct rule_str));

	new_str = (char *)malloc(str_len);
	if(NULL == new_str)
	{
		diag_printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		free(rule_str_new);
		return -1;
	}
	memset(new_str, 0x0, str_len);
	memcpy(new_str, str, str_len);
	
	rule_str_new->next = NULL;
	rule_str_new->str_len = str_len;
	rule_str_new->str = new_str;

	*rule_str_p = rule_str_new;

	return 0;
}

int rule_str_append(struct rule_str *list, struct rule_str *rule_str_add)
{
	struct rule_str *rule_str_p = list;

	if(NULL == list)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return -1;
	}

	while(rule_str_p->next)
		rule_str_p = rule_str_p->next;

	rule_str_p->next = rule_str_add;

	return 0;
}

int init_rule_hash(char *rule_p, struct rule_str *rule_hash[], int rule_hash_len)
{
	uint32 hash_index;
	char *char_p;
	int str_len, ret;
	struct rule_str *rule_str_new = NULL;
	
	char_p = strtok(rule_p, SAME_RULE_DELIM);
	str_len = strlen(char_p);
	ret = new_rule_str(&rule_str_new, char_p, str_len);
	if(0 != ret)
	{
		goto init_failed;
	}
	hash_index = jhash(char_p, str_len, 0)%rule_hash_len;
	if(NULL == rule_hash[hash_index])
		rule_hash[hash_index] = rule_str_new;
	else
		rule_str_append(rule_hash[hash_index], rule_str_new);

	while((char_p = strtok(NULL, SAME_RULE_DELIM)))
	{
		str_len = strlen(char_p);
		ret = new_rule_str(&rule_str_new, char_p, str_len);
		if(0 != ret)
		{
			goto init_failed;
		}
		hash_index = jhash(char_p, str_len, 0)%rule_hash_len;
		if(NULL == rule_hash[hash_index])
			rule_hash[hash_index] = rule_str_new;
		else
			rule_str_append(rule_hash[hash_index], rule_str_new);
	}

	return 0;

init_failed:
	clear_rule(rule_hash, rule_hash_len);
	return -1;
}

void print_rule_list(struct rule_str *rule_list)
{
	struct rule_str *rule_str_p = rule_list, *rule_str_pnext = NULL;
	int i;

	while(rule_str_p)
	{
		rule_str_pnext = rule_str_p->next;
		for(i=0; i<rule_str_p->str_len; i++)
			printf("%c", rule_str_p->str[i]);
		rule_str_p = rule_str_pnext;
		if(rule_str_p)
			printf("|");
	}

	return;
}

void print_rule_hash(struct rule_str *rule_hash[], int rule_hash_len)
{
	int i = 0;

	for(i=0; i<rule_hash_len; i++)
	{
		printf("hash[%d] : ", i);
		print_rule_list(rule_hash[i]);
		printf("\n");
	}

	return;
}

int init_js_str(char *js_p)
{
	char *char_p = js_p;

	if(NULL == js_p)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if(0 == memcmp(js_p, "http://", strlen("http://")))
	{
		char_p += strlen("http://");
	}

	sprintf(js_str, JS_PATH_FORMAT, char_p);

	diag_printf("[%s][%d] js_str = %s\n", __FUNCTION__, __LINE__, js_str);

	return 0;
}

int init_js_rule(char *js_rule)
{
	char *url_p, *js_p;
	int ret;
    
	url_p = strtok(js_rule, DIFF_RULE_DELIM);
	js_p = strtok(NULL, DIFF_RULE_DELIM);
	diag_printf("[%s][%d] js_p = %s\n", __FUNCTION__, __LINE__, js_p);

	ret = init_rule_hash(url_p, rule_str_hash, JS_RULE_HASH_LEN);
	if(0 != ret)
	{
		diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = init_js_str(js_p);

	return 0;
}

void print_js_inject_para(void)
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("rule_str_hash : \n");
	print_rule_hash(rule_str_hash, JS_RULE_HASH_LEN);
	printf("js_str : %s\n", js_str);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);

	return;
}

void init_js_inject_para(char *js_rule)
{
	clear_rule(rule_str_hash, JS_RULE_HASH_LEN);
	memset(js_str, 0x0, sizeof(js_str));
	init_lan_para();
	init_js_rule(js_rule);
	print_js_inject_para();

	return;
}

int js_inject_enable(void)
{
	js_inject_check = js_inject_handle;
	return 0;
}

int js_inject_disable(void)
{
	js_inject_check = NULL;
	return 0;
}

