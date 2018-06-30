#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>
#include <string.h>
#include <netdb.h>
#include <arp_clients.h>
#include "tenda_arp.h"
#include "cJSON.h"
#include "wl_utility_rltk.h"
#include "debug.h"
#include "manufacturer.h"

static RET_INFO tpi_manufacturer_start();
RET_INFO tpi_manufacturer_first_init()
{
    return tpi_manufacturer_start();
}

RET_INFO tpi_manufacturer_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_manufacturer_start();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : connect_to_cloud_server
 功能描述  : 连接到服务器
 输入参数  : int *sock
             char *host
             unsigned short port
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int connect_to_cloud_server(int *sock, char *host, unsigned short port)
{
    struct sockaddr_in addr;
    int len;
    int result;
    struct hostent *hostinfo;

    if((*sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return(-1);
    }

    hostinfo = gethostbyname(host);

    if(!hostinfo)
    {
        RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"gethostbyname error\n");
        close(*sock);
        return(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
    addr.sin_port = htons(port);
    memset(addr.sin_zero,0,sizeof(addr.sin_zero));

    len = sizeof(addr);
    if((result=connect(*sock, (struct sockaddr *)&addr, len)) == -1)
    {
        perror("connect");
        close(*sock);
        return(-1);
    }

    return 0;
}
/*****************************************************************************
 函 数 名  : output_to_cloud
 功能描述  : 发送请求数据报文
 输入参数  : int fd
             void *buf
 输出参数  : 无
 返 回 值  : static

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static void output_to_cloud(int fd, void *buf)
{
    fd_set writefds;
    int max_fd;
    struct timeval tv;
    int ret;

    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);
    max_fd = fd;

    tv.tv_sec = 0;
    tv.tv_usec = 20000;

    ret = select(max_fd + 1, NULL, &writefds, NULL, &tv);

    if(ret == -1)
    {
        RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"select error ret:%d\n",ret);

    }
    else if(ret == 0)
    {
        RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"select timeout ret:%d\n",ret);
    }
    else
    {
        if(FD_ISSET(fd, &writefds))
        {
            if(send(fd, buf, strlen(buf), 0) == -1)
                RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"error send()ing request\n");
        }
        else
            RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"socket was not exist!!\n");
    }
}
/*****************************************************************************
 函 数 名  : read_input_by_cloud
 功能描述  : 获取服务器的回应报文
 输入参数  : int fd
             void *buf
             int len
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int read_input_by_cloud(int fd, void *buf, int len)
{
    fd_set readfds;
    int max_fd;
    struct timeval tv;
    int ret;
    int bread = -1;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    max_fd = fd;

    tv.tv_sec = 9;
    tv.tv_usec = 0;
    ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);

    if(ret == -1)
    {
        RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"[cloud]: select error\n");
    }
    else if(ret == 0)
    {
        RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"select timeout\n");
    }
    else
    {
        if(FD_ISSET(fd, &readfds))
        {
            if((bread=recv(fd, buf, len, 0)) == -1)
            {
                RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"error recv()ing reply\n");
            }
        }
        else
            RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"socket was not exist!!\n");
    }

    return(bread);
}
static void make_request(struct mac_node* client_mac, char *request)
{
    char mac_tmp[1024] = {0};
    char mac[6][4];
    struct mac_node* temp_node = client_mac;
    cJSON   *pJson = NULL;
    cJSON   *pArray = NULL;
    cJSON   *pItem = NULL;
    char*  p_send_buf;
    int first_node = 1;
    int request_mac_num = 0;
    pJson = cJSON_CreateObject();
    cJSON_AddItemToObject(pJson, "mac", pArray = cJSON_CreateArray());
    /* 一次请求多个客户端的设备厂商信息 */
    while(temp_node)
    {
        if (strlen(temp_node->mac))
        {
            if (sscanf(temp_node->mac, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%s", mac[0], mac[1],mac[2], mac[3],mac[4], mac[5]) == 6)
            {
            	if(request_mac_num > 49)
					break;
                sprintf(mac_tmp, "%s-%s-%s-%s-%s-%s", mac[0], mac[1],mac[2], mac[3],mac[4], mac[5]);
                cJSON_AddItemToArray(pArray, pItem = cJSON_CreateString(mac_tmp));
		  		request_mac_num++;
		  
            }
        }
        temp_node = temp_node->next;
    }
    p_send_buf = cJSON_Print(pJson);
    /* 组装请求信息 */
    sprintf(request, "POST %s HTTP/1.0\r\n"
            "Connection: Keep-Alive\r\n"
            "Accept: */*\r\n"
            "User-Agent: Mozilla/5.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
            "Content-Length: %d\r\n"
            "Host: %s\r\n\r\n"
            "%s\r\n",
            CLOUD_REQUST_URL, 2+strlen(p_send_buf), CLOUD_SERVER_NAME, p_send_buf);
    if(NULL != pJson)
    {
        cJSON_Delete(pJson);
        pJson = NULL;
    }

    FREE_P(&p_send_buf);
}

/*****************************************************************************
 函 数 名  : parse_response_string
 功能描述  : 解析数据包，并更新链表
 输入参数  : char* response
             struct mac_node* dev_mac
 输出参数  : 无
 返 回 值  : static

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
struct facture_name facture_name_head;
static int parse_response_string(char* response,struct mac_node* dev_mac)
{
    int         i = 0;
    int         nCount = 0;
    char      *pos = NULL;
    cJSON   *pJson = NULL;
    cJSON   *pArray = NULL;
    cJSON   *pItem = NULL;
    struct facture_name* temp = NULL;
    struct facture_name* head = &facture_name_head;
    unsigned char ether_mac[6] = {0};
    struct mac_node* dev_mac_temp = dev_mac;


    pos = strstr(response, "\r\n\r\n");

    if(NULL == pos)
    {
        return -1;
    }

    pJson   = cJSON_Parse(pos);
    if(pJson == NULL)
    {
        return -1;
    }

    pArray  = cJSON_GetObjectItem (pJson, "result" );
    if(pArray == NULL)
    {
    	 cJSON_Delete(pJson);
        return -1;
    }

    nCount  = cJSON_GetArraySize (pArray);
    for(i = 0; i < nCount; i++)
    {
        pItem = cJSON_GetArrayItem(pArray,i);
        if(pItem->valuestring!= NULL)
        {
            temp = (struct facture_name*)malloc(sizeof(struct facture_name));
            if(temp == NULL)
            {
                return -1;
            }
            memset(temp,0x0,sizeof(struct facture_name));
            ether_atoe(dev_mac_temp->mac, ether_mac);
            temp->mac[0] = ether_mac[0];
            temp->mac[1] = ether_mac[1];
            temp->mac[2] = ether_mac[2];
            if(strcmp(pItem->valuestring,"Null") == 0)
            {
                /* 如果查不到该MAC对应的设备厂商，则将该MAC记录为other，防止每一
                   次都要对该设备进行查询 */
                temp->name = (char*)malloc(strlen("other")+1);
                strcpy(temp->name,"other");
            }
            else
            {
                temp->name = (char*)malloc(strlen(pItem->valuestring)+1);
                strcpy(temp->name,pItem->valuestring);
            }
            temp->next = head->next;
            head->next = temp;
        }
        dev_mac_temp = dev_mac_temp->next;
    }
    cJSON_Delete(pJson);
}
int get_menufacture_by_cloud(struct mac_node* dev_mac,int fd)
{
    int bytes, btot, ret;
    char request[2048] = {0};
    char buf[2048] = {0};

    if(dev_mac == NULL)
    {
        close(fd);
        return 1;
    }
    /* 组装请求报文 */
    make_request(dev_mac,request);
    RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"%s\n",request);


    /* 发送请求报文 */
    output_to_cloud(fd, request);
    /* 接收报文 */
    if((bytes=read_input_by_cloud(fd, buf, 2048)) > 0)
    {
        /* 数据解析 */
        RC_MODULE_DEBUG(RC_MANUFACTURER_MODULE,TPI,"%s\n",buf);

        parse_response_string(buf,dev_mac);
    }
    close(fd);

    return 1;
}
/*****************************************************************************
 函 数 名  : router_mode_get_online_list
 功能描述  : 路由模式下获取在线客户端
 输入参数  : struct mac_node** mac_list
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int  router_mode_get_online_list(struct mac_node** mac_list)
{
    int client_num = 0;
    int index = 0;
    char facturer_name[16] = {0};
    time_t now_time = time(0);
    struct client_info  *clients_list = NULL;
    struct mac_node     *temp_node = NULL;
    struct mac_node     *mac_list_head = *mac_list;
    /* 获取在线客户端 */
    clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
    if(clients_list != NULL )
    {
        memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
        client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
    }
    else
    {
        client_num = 0;
    }

    for(index = 0; index < client_num; ++index)
    {
        /* 在当前列表中查询，查询不到则添加到询问链表中 */
        if(!get_menufacture_name(clients_list[index].mac,facturer_name))
        {
            temp_node = (struct mac_node*)malloc(sizeof(struct mac_node));
            if(temp_node == NULL)
            {
                goto exit;
            }
            memset(temp_node,0x0,sizeof(struct mac_node));
            strcpy(temp_node->mac,clients_list[index].mac);
            temp_node->next = mac_list_head;
            mac_list_head = temp_node;
        }
    }
exit:
    *mac_list = mac_list_head;
    FREE_P(&clients_list);
}

/*****************************************************************************
 函 数 名  : ap_mode_get_online_list
 输入参数  : struct mac_node** mac_list
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int ap_mode_get_online_list(struct mac_node** mac_list)
{
    unsigned int i;
    char dev_mac[18] = {0};
    char facturer_name[16] = {0};
    struct mac_node     *temp_node = NULL;
    struct mac_node     *mac_list_head = *mac_list;
    WLAN_STA_INFO_T pInfo[MAX_STA_NUM + 1];
    unsigned char mac[18] = {0};

    memset(pInfo,0x0,sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM + 1));
    /* 获取接入路由器的无线客户端 */
    getWlStaInfo( "wlan0", pInfo);

    for (i=1; i<=MAX_STA_NUM; i++)
    {

        if (pInfo[i].aid && (pInfo[i].flag & STA_INFO_FLAG_ASOC))
        {
            memset(mac,0x0,18);
            sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",
                    pInfo[i].addr[0],pInfo[i].addr[1],pInfo[i].addr[2],
                    pInfo[i].addr[3],pInfo[i].addr[4],pInfo[i].addr[5]);
            /* 在当前列表中查询，查询不到则添加到询问链表中 */
            if(!get_menufacture_name(mac,facturer_name))
            {
                temp_node = (struct mac_node*)malloc(sizeof(struct mac_node));
                if(temp_node == NULL)
                {
                    goto exit;
                }
                memset(temp_node,0x0,sizeof(struct mac_node));
                strcpy(temp_node->mac,mac);
                temp_node->next = mac_list_head;
                mac_list_head = temp_node;
            }
        }
    }
exit:
    *mac_list = mac_list_head;
    return 0;
}
/*****************************************************************************
 函 数 名  : cloud_manufacturer_main_loop
 功能描述  : 更新设备厂商的主循环
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static

 修改历史      :
  1.日    期   : 2016年12月2日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static void cloud_manufacturer_main_loop()
{
    int fd = 0;
    struct mac_node *mac_list = NULL;
    struct mac_node *temp_mac_node = NULL;
    struct mac_node *next_mac_node = NULL;
    while(1)
    {
        if(nvram_match(SYSCONFIG_WORKMODE, "client+ap") 
		|| nvram_match(SYSCONFIG_WORKMODE, "bridge"))
        {
            /* ap或者apclient模式下获取在线客户端 */
            ap_mode_get_online_list(&mac_list);
        }
        else
        {
            /* 路由模式下获取在线客户端 */
            router_mode_get_online_list(&mac_list);
        }

        if(mac_list != NULL)
        {
             /* 链接到服务器 */
            if(connect_to_cloud_server(&fd,CLOUD_SERVER_NAME, CLOUD_SERVER_PORT) != 0)
            {
            	goto wait_next;
            }
            /* 更新本地不能识别的设备 */
            get_menufacture_by_cloud(mac_list,fd);
            /* 清理内存 */
wait_next:
            temp_mac_node = mac_list;
            while(temp_mac_node)
            {
                next_mac_node = temp_mac_node->next;
                free(temp_mac_node);
                temp_mac_node = next_mac_node;
            }
            mac_list = NULL;
        }
        cyg_thread_delay(500);
    }
}

static int cloud_manufacturer_thread_type = 0;
static char cloud_manufacturer_daemon_stack[10240];
static cyg_handle_t cloud_manufacturer_daemon_handle;
static cyg_thread cloud_manufacturer_daemon_thread;
static RET_INFO tpi_manufacturer_start()
{
    if(cloud_manufacturer_thread_type == 0)
    {
        cyg_thread_create( 9,
                           (cyg_thread_entry_t *)cloud_manufacturer_main_loop,
                           0,
                           "cloud_manufacturer",
                           &cloud_manufacturer_daemon_stack,
                           sizeof(cloud_manufacturer_daemon_stack),
                           &cloud_manufacturer_daemon_handle,
                           &cloud_manufacturer_daemon_thread);
        cyg_thread_resume(cloud_manufacturer_daemon_handle);
        cloud_manufacturer_thread_type = 1;
    }
    return RET_SUC;
}
