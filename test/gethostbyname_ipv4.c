/******************************************************************************

                  版权所有 (C), 2017-2027, 深圳市吉祥腾达科技有限公司

 ******************************************************************************
  文 件 名   : gethostbyname_ipv4.c
  版 本 号   : 初稿
  作    者   : luminais
  生成日期   : 2017年5月9日
  最近修改   :
  功能描述   : 实现ipv4的gethostbyname函数，使其在解析失败时的超时时间为RES-
               OLV_TIMEOUT，而原库函数的实现超时时间为30秒
  函数列表   :
              gethostbyname_ipv4
              get_sys_dns_ip
  修改历史   :
  1.日    期   : 2017年5月9日
    作    者   : luminais
    修改内容   : 创建文件

******************************************************************************/

#include "network_check.h"

#define BUF_SIZE (1024) 
#define SRV_PORT (53) 
#define RESOLV_TIMEOUT (2)
#define DNS_FILE "/etc/resolv.conf"

typedef unsigned short U16; 
typedef unsigned int U32; 

typedef struct _DNS_HDR 
{ 
    U16 id; 
    U16 tag; 
    U16 numq; 
    U16 numa; 
    U16 numa1; 
    U16 numa2; 
}DNS_HDR; 

#pragma pack (1)
struct dns_answer_body
{
    U16 name;
    U16 type;
    U16 class;
    U32 ttl;
    U16 length;
};

/*****************************************************************************
 函 数 名  : get_sys_dns_ip
 功能描述  : 获取/etc/resolv.conf中第一个dns地址
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 成功:返回addr的地址
                        失败:返回NULL
 
 修改历史      :
  1.日    期   : 2017年5月9日
    作    者   : luminais
    修改内容   : 新生成函数

*****************************************************************************/
char* get_sys_dns_ip(void)
{
    FILE* fp = NULL;
    char buffer[UGW_TMP_STR_LEN] = {0};
    static char addr[32] = {0};
    int find = 0;

    fp = fopen(DNS_FILE, "r");
    if(fp == NULL)
        return NULL;

    while(fgets(buffer, UGW_TMP_STR_LEN, fp))
    {
        if((strstr(buffer, "nameserver")))
        {
            find = 1;
            break;
        }
        memset(buffer, 0x0, UGW_TMP_STR_LEN);
    }
    fclose(fp);

    if(find)
    {
        sscanf(buffer, "nameserver %s", addr);
        return addr;
    }

    return NULL;
}

/*****************************************************************************
 函 数 名  : get_dns_query_body
 功能描述  : 将域名转换成dns协议报文体
 输入参数  : unsigned char *query_msg   
             int *len                   
             unsigned char *query_name  
 输出参数  : 无
 返 回 值  : char
 
 修改历史      :
  1.日    期   : 2017年7月27日
    作    者   : luminais
    修改内容   : 新生成函数

*****************************************************************************/
char *get_dns_query_body(unsigned char *query_msg, int *len, unsigned char *query_name)
{
    unsigned char query_body_end[] = {
                     /* QNAME */
    0x00,0x01,  /* QTYPE A record */
    0x00,0x01   /* QCLASS: IN */
    };

    if(!query_name || !query_msg || !len)
        return NULL;

    int query_name_len = strlen(query_name), i;
    unsigned char *char_p = query_msg, j;

    char_p++;
    for(i=0, j=0; i<query_name_len; i++) {
        if(query_name[i] == '.') {
            *(char_p-1) = j;
            char_p += j;
            char_p++;
            *len += (j+1);
            j = 0;
        } else {
            *(char_p+j) = query_name[i];
            j++;
        }
    }
    *(char_p-1) = j;
    char_p += j;
    *len += (j+1);

    *char_p = 0;
    *len += 1;

    char_p++;
    memcpy(char_p, query_body_end, sizeof(query_body_end));
    *len += sizeof(query_body_end);
    char_p += sizeof(query_body_end);

    return char_p;
}

/*****************************************************************************
 函 数 名  : gethostbyname_ipv4
 功能描述  : 向/etc/resolv.conf中第一个dns地址请求域名[name]的ipv4类型地址
 输入参数  : const char *name  :需要解析的域名
             char *ip          :存储解析得到的ip字符串
             int ip_len        :ip字符串数组的长度
 输出参数  : 无
 返 回 值  :  成功:返回0
                        失败:返回-1
 
 修改历史      :
  1.日    期   : 2017年5月9日
    作    者   : luminais
    修改内容   : 新生成函数

*****************************************************************************/
int gethostbyname_ipv4(const char *name, char *ip, int ip_len)
{
    int clifd, len = 0, i; 
    int query_body_len = 0;
    int offset_cnt = 0;
    socklen_t socklen = 0;
    char buf[BUF_SIZE] = {0};
    char *p = NULL;
    char *dns_ipaddr = NULL;
    char ipaddr[32] = {0};
    struct sockaddr_in servaddr; 
    struct timeval tv;

    UGW_FuncEntry(UGW_MODULE_NETCHECK);
    memset(&tv, 0x0, sizeof(tv));
    dns_ipaddr = get_sys_dns_ip();
    if(!dns_ipaddr)
    {
        UGW_DEBUG(UGW_ERROR, UGW_MODULE_NETCHECK, "get_sys_dns_ip failed\n");
        return -1;
    }
    clifd = socket(AF_INET, SOCK_DGRAM, 0 ); 
    if (clifd < 0)
    {
        UGW_DEBUG(UGW_ERROR, UGW_MODULE_NETCHECK, "create socket error!\n");
        return -1; 
    }
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(SRV_PORT); 
    inet_pton(AF_INET, dns_ipaddr, &servaddr.sin_addr); 
    
    DNS_HDR *dnshdr = (DNS_HDR *)buf; 
    memset(buf, 0, BUF_SIZE); 
    dnshdr->id = (U16)1; 
    dnshdr->tag = htons(0x0100); 
    dnshdr->numq = htons(1); 
    dnshdr->numa = 0; 

    p = buf + sizeof(DNS_HDR);
    if(NULL == get_dns_query_body(p, &query_body_len, name))
    {
        printf("[%s][%d][luminais] get_dns_query_body failed\n", __FUNCTION__, __LINE__);
        close(clifd);
        return -1;
    }

    tv.tv_sec = RESOLV_TIMEOUT;
    tv.tv_usec = 0;
    if(setsockopt(clifd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("[%s][%d][luminais] setsockopt SO_SNDTIMEO failed. %d(%m)\n", __FUNCTION__, __LINE__, errno);
    }
    if(setsockopt(clifd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("[%s][%d][luminais] setsockopt SO_RCVTIMEO failed. %d(%m)\n", __FUNCTION__, __LINE__, errno);
    }
    socklen = sizeof(struct sockaddr_in); 

    len = sendto(clifd, buf, sizeof(DNS_HDR) + query_body_len, 0, (struct sockaddr *)&servaddr, socklen);
    if (len < 0)
    {
        UGW_DEBUG(UGW_ERROR, UGW_MODULE_NETCHECK, "send error\n");
        close(clifd);
        return -1; 
    }
    len = recvfrom(clifd, buf, BUF_SIZE, 0, (struct sockaddr *)&servaddr, &socklen);
    close(clifd);
    if (len < 0) 
    { 
        if (errno == EAGAIN)
        {
            UGW_DEBUG(UGW_ERROR, UGW_MODULE_NETCHECK, "recvfrom timeout!\n");
        }
        return -1;
    }
    
    if (dnshdr->numa == 0) 
    {
        UGW_DEBUG(UGW_ERROR, UGW_MODULE_NETCHECK, "ack error\n");
        return -1; 
    }

    p = buf;
    /* answer_header */
    p += 12;
    offset_cnt += 12;
    /* query body len */
    p += query_body_len;
    offset_cnt += query_body_len;
    struct dns_answer_body *dns_answer_body_p;
    U16 i_len;
    dns_answer_body_p = (struct dns_answer_body *)p;
    while(ntohs(dns_answer_body_p->type) != 1 && offset_cnt <=len)
    {
        p += sizeof(struct dns_answer_body);
        offset_cnt += sizeof(struct dns_answer_body);
        i_len = ntohs(dns_answer_body_p->length);
        p += i_len;
        offset_cnt += i_len;
        dns_answer_body_p = (struct dns_answer_body *)p;
    }
    p += sizeof(struct dns_answer_body);
    offset_cnt += sizeof(struct dns_answer_body);
    if(offset_cnt >= len)
    {
        printf("[%s][%d][luminais] %s : offset_cnt >= len \n", __FUNCTION__, __LINE__, name);
        return -1;
    }
    snprintf(ipaddr, sizeof(ipaddr)-1, "%u.%u.%u.%u", (unsigned char)*p, (unsigned char)*(p + 1),(unsigned char)*(p + 2), (unsigned char)*(p + 3));
    strncpy(ip, ipaddr, ip_len);
    UGW_DEBUG(UGW_DBG, UGW_MODULE_NETCHECK, "%s --> %s[%s]", name, ipaddr, ip); 
    
    UGW_FuncExit(UGW_MODULE_NETCHECK);
    return 0; 
}

