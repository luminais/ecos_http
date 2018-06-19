#include <stdlib.h>
#include <string.h>

#include <bcmnvram.h>
#include "pi_wan_err_info.h"
/*
说明：
	1.之前有需求：连接中直接提示联网中，获取到IP之后立马提示已联网。
		故AP模式下去掉连接中，拨号过程中提示正在尝试联网中…，
		考虑到桥接模式的特殊性，只处理桥接模式下获取到IP之后立马显示为已联网。
	2.有星空极速版PPPOE拨号验证时间提示时间为1-5分钟，没星空极速版提示为1-2分钟。
	3.WISP模式下不支持MAC地址克隆，故不提示
(一)AP模式：
	STATIC:
		1.	检测到WAN口网线未连接，请检查并连接好您的WAN口网线
		2.	未连接
		3.	正在尝试联网中…
		4.	未联网，请联系您的宽带运营商
		5.	已联网
		6.	系统检测到您的联网方式可能为静态IP，请手动选择并配置静态IP，尝试联网
	DHCP:
		1.	检测到WAN口网线未连接，请检查并连接好您的WAN口网线
		2.	未连接
		3.	正在尝试联网中…
		4.	未联网，请确认不使用路由器时是否可以正常上网，如不能，请联系您的宽带运营商
		5.	已联网
		6.	IP冲突，请修改LAN口IP
		7.	网络运营商远端无响应，请确认不接路由器时是否可以正常上网，如不能，请联系当地网络运营商解决
	PPPOE:
		1.	检测到WAN口网线未连接，请检查并连接好您的WAN口网线
		2.	未连接
		3.	正在诊断您输入的宽带用户名和宽带密码是否正确，请稍等，整个过程约1-5分钟
		4.	拨号成功，但无法连接至互联网。请确认不使用路由器时是否可以正常上网，如不能，请联系您的宽带运营商
		5.	已联网
		6.	用户名密码验证失败，请确认您的宽带用户名与宽带密码并重新输入
		7.	网络运营商远端无响应，请确认不接路由器时是否可以正常上网，如不能，请联系当地网络运营商解决
(二)WISP模式：
	STATIC:
		1.	热点信号放大模式未桥接
		2.	热点信号放大模式桥接中...
		3.	热点信号放大模式桥接成功，正在尝试联网...
		4.	未联网，请联系您的宽带运营商
		5.	已联网
		6.  无线密码验证失败，请重新输入上级无线密码！
	DHCP:
		1.	热点信号放大模式未桥接
		2.	热点信号放大模式桥接中...
		3.	热点信号放大模式桥接成功，正在尝试联网...
		4.	系统已获取到IP，但无法连接至互联网，请联系您的宽带运营商
		5.	已联网
		6.	IP冲突，请修改LAN口IP
		7.	网络运营商远端无响应，请确认不接路由器时是否可以正常上网，如不能，请联系当地网络运营商解决
		8.  无线密码验证失败，请重新输入上级无线密码！
	PPPOE:
		1.	热点信号放大模式未桥接
		2.	正在诊断您输入的宽带用户名和宽带密码是否正确，请稍等，整个过程约1-5分钟
		3.	拨号成功，但无法连接至互联网。请确认不使用路由器时是否可以正常上网，如不能，请联系您的宽带运营商
		4.	已联网
		5.	用户名密码验证失败，请确认您的宽带用户名与宽带密码并重新输入
		6.	网络运营商远端无响应，请确认不接路由器时是否可以正常上网，如不能，请联系当地网络运营商解决
		8.  无线密码验证失败，请重新输入上级无线密码！
(三)APCLIENT模式：
	1.	万能信号放大模式未桥接
	2.	万能信号放大模式桥接中...
	3.	万能信号放大模式桥接成功，正在尝试联网...
	4.	未联网，请联系您的宽带运营商
	5.	已联网
	6.  无线密码验证失败，请重新输入上级无线密码！
*/
#ifdef __CONFIG_APCLIENT_DHCPC__
extern int gpi_get_apclient_dhcpc_addr(char *ip,char *mask);
#endif
extern WLMODE tpi_get_wl_mode();

WANMODE tpi_get_wan_mode(WLMODE wl_mode)
{
    WANMODE ret = WAN_DHCP_MODE;
    int wan_mode = 0;

    switch(wl_mode)
    {
        case WL_AP_MODE:
        case WL_WISP_MODE:
            wan_mode = get_wan_type();
            if(wan_mode < WAN_NONE_MODE || wan_mode > WAN_PPPOE_MODE)
            {
                ret = WAN_NONE_MODE;
            }
            else
            {
                ret = wan_mode;
            }
            break;
        case WL_APCLIENT_MODE:
			ret = WAN_DHCP_MODE;
			break;
        case WL_WDS_MODE:
        default:
            ret = WAN_NONE_MODE;
            break;
    }

    return ret;
}

PI_INFO tpi_get_wan_status(WLMODE wl_mode,WANSTATUS * wan_status,WIFISTASTATUS * wifi_status)
{
    PI_INFO ret = PI_SUC;
    int wanLinkSta = 0;
    int conStat = 0;

	//这里不对wan_status和wifi_status进行空判断，因为AP模式不需要wifi_status，空判断放到switch里面进行
	
    switch(wl_mode)
    {
        case WL_AP_MODE:
			if(NULL == wan_status)
			{
				return PI_ERR;
			}
            wan_link_check();
            wanLinkSta = get_wan_linkstatus();
            if(0 == wanLinkSta)
            {
                (*wan_status) = WAN_NO_WIRE;
            }
            else
            {
                conStat = get_wan_connstatus();
                if(2 == conStat)
                    (*wan_status) = WAN_CONNECTED;
                else if(1 == conStat)
                    (*wan_status) = WAN_CONNECTING;
                else
                    (*wan_status) = WAN_DISCONNECTED;
            }
            break;
        case WL_WISP_MODE:
        case WL_APCLIENT_MODE:
			if(NULL == wan_status || NULL == wifi_status)
			{
				return PI_ERR;
			}
            gpi_get_wifi_status_info(wan_status,wifi_status);
            break;
        case WL_WDS_MODE:
        default:
            ret = PI_ERR;
            break;
    }

    return ret;
}

static WANERRINFO tpi_get_err_result(P_WAN_ERR_INFO_STRUCT p)
{
    WANERRINFO ret = INFO_ERROR;

    ret = ((p->code)%100) + 100 * ((p->network_check)%10) + 1000 * ((p->wan_mode)%10)
			+ 10000 * ((p->wl_mode)%10) + 100000 * ((p->time_action)%10)
          	+ 1000000 * ((p->color_action)%10) + 10000000 * ((p->button_action)%10);

    return ret;
}

DHCPERRCODE tpi_get_dhcp_err_result()
{
    DHCPERRCODE ret = COMMON_NONE;
    char dhcp_check[5] = {0};

    sprintf(dhcp_check,"%s",nvram_safe_get(ERR_CHECK));

    if(0 == strcmp(dhcp_check,"11"))
    {
        ret = DHCP_IP_CONFLLICT;
    }
	else if(0 == strcmp(dhcp_check,"5"))
	{
		ret = DHCP_NO_RESPOND;
	}

    return ret;
}

PPPOEERRCODE tpi_get_pppoe_err_result()
{
    PPPOEERRCODE ret = COMMON_NONE;
    char pppoe_check[5]= {0};

    sprintf(pppoe_check,"%s",nvram_safe_get(ERR_CHECK));

    if(!strcmp(pppoe_check,"5"))//无响应
    {
        ret = PPPOE_NO_RESPOND;
    }
    else if(!strcmp(pppoe_check,"7"))//正在验证
    {
        ret = COMMON_CONNECTING;
    }
    else if(!strcmp(pppoe_check,"2"))//用户名，密码错误
    {
        ret = PPPOE_CHECKED_PASSWORD_FAIL;
    }
    else if(!strcmp(pppoe_check,"3"))//成功
    {
        ret = COMMON_CONNECTED_ONLINEING;
    }

    return ret;
}

PI_INFO tpi_get_connecting_err_info(P_WAN_ERR_INFO_STRUCT p,WIFISTASTATUS wifi_status)
{
	PI_INFO ret = PI_SUC;

	if(NULL == p)
	{
		return PI_ERR;
	}
	
	p->button_action = BUTTON_DOWN;
	p->color_action = COLOR_TRY;
	p->time_action = TIME_NONE;
	p->code = COMMON_CONNECTING;

	if(WL_AP_MODE != p->wl_mode && (WIFI_AUTHENTICATED_FAIL == wifi_status || WIFI_AUTH_FAIL == wifi_status))
	{
		p->color_action = COLOR_ERR;
		switch(p->wan_mode)
		{
			case WAN_STATIC_MODE:
				p->code = STATIC_WL_CHECKED_PASSWORD_FAIL;
				break;
			case WAN_DHCP_MODE:
				p->code = DHCP_WL_CHECKED_PASSWORD_FAIL;
				break;
			case WAN_PPPOE_MODE:
				p->code = PPPOE_WL_CHECKED_PASSWORD_FAIL;
				break;
			default:
				ret = PI_ERR;
				break;
		}
		return ret;
	}
	
	switch(p->wl_mode)
	{
		case WL_AP_MODE:
		case WL_WISP_MODE:
            if(WAN_DHCP_MODE == p->wan_mode)
            {
                DHCPERRCODE dhcp_err = tpi_get_dhcp_err_result();
                if(dhcp_err == DHCP_IP_CONFLLICT)
                {
                    p->color_action = COLOR_ERR;
                    p->code = DHCP_IP_CONFLLICT;
                }
				else if(dhcp_err == DHCP_NO_RESPOND)
                {
                    p->color_action = COLOR_ERR;
                    p->code = DHCP_NO_RESPOND;
                }
            }
            else if(WAN_PPPOE_MODE == p->wan_mode)
            {
                PPPOEERRCODE pppoe_err = tpi_get_pppoe_err_result();

                if(pppoe_err == COMMON_CONNECTING)
                {
                    p->color_action = COLOR_TRY;
                    p->code = COMMON_CONNECTING;
                }
#if 0
				//此时虽然拨号成功，但是还没有获取到IP
                else if(pppoe_err == COMMON_CONNECTED_ONLINEING)
                {
                    p->color_action = COLOR_TRY;
                    p->code = COMMON_CONNECTED_ONLINEING;
                }
#endif
                else if(pppoe_err == PPPOE_CHECKED_PASSWORD_FAIL)
                {
                    p->color_action = COLOR_ERR;
                    p->code = PPPOE_CHECKED_PASSWORD_FAIL;
                }
                else if(pppoe_err == PPPOE_NO_RESPOND)
                {
                    p->color_action = COLOR_ERR;
                    p->code = PPPOE_NO_RESPOND;
                }
            }
			break;
		case WL_APCLIENT_MODE:
            p->time_action = TIME_NONE;
			break;
		default:
			break;
	}
	return ret;
}


PI_INFO tpi_get_connected_err_info(P_WAN_ERR_INFO_STRUCT p)
{
	PI_INFO ret = PI_SUC;
	int internetStat = 0;
	
	if(NULL == p)
	{
		return PI_ERR;
	}
	
	p->button_action = BUTTON_DOWN;
	p->color_action = COLOR_TRY;
	p->time_action = TIME_SHOW;
	p->code = COMMON_CONNECTED_ONLINEING;
	
	switch(p->wl_mode)
	{
		case WL_AP_MODE:
		case WL_WISP_MODE:
			internetStat = get_wan_onln_connstatus();

			if(0 == internetStat)
			{
				p->color_action = COLOR_ERR;
				p->code = COMMON_NOT_ONLINE;
			}
#if 0
			else if(1 == internetStat)
			{
				p->color_action = COLOR_TRY;
				p->code = COMMON_CONNECTED_ONLINEING;
			}
#endif
			else
			{
				p->color_action = COLOR_SUC;
				p->code = COMMON_ONLINEED;
			}
		break;
		case WL_APCLIENT_MODE:
            p->time_action = TIME_NONE;
            p->color_action = COLOR_SUC;
#ifdef __CONFIG_APCLIENT_DHCPC__
			char ip[17] = {0},mask[17] = {0};
			gpi_get_apclient_dhcpc_addr(ip,mask);
			if(0 == strcmp(ip,"") || 0 == strcmp(ip,""))
			{
				p->color_action = COLOR_TRY;
				p->code = COMMON_CONNECTING;
			}
#endif		
			break;
		default:
			ret = PI_ERR;
			break;
	}

	return ret;
}

WANERRINFO tpi_get_err_info(WLMODE wl_mode)
{
	WANERRINFO ret = INFO_ERROR;
    WANSTATUS wan_status = WAN_NO_WIRE;
    int internetStat = 0;
	WIFISTASTATUS wifi_status = WIFI_INIT_FAIL;
    WAN_ERR_INFO_STRUCT p_info;
    P_WAN_ERR_INFO_STRUCT p;

    p = &p_info;

	memset(p,0x0,sizeof(WAN_ERR_INFO_STRUCT));
	
    p->wl_mode = wl_mode;
    p->wan_mode = tpi_get_wan_mode(wl_mode);

    if(WAN_NONE_MODE == p->wan_mode || PI_ERR == tpi_get_wan_status(p->wl_mode,&wan_status,&wifi_status))
    {
    	goto err_return;
    }
	
	if(WL_AP_MODE == p->wl_mode && WAN_NO_WIRE != wan_status && WAN_DISCONNECTED != wan_status)
	{
		if(0 == network_tpye)
		{
			p->network_check = NETWORK_CHECK_STATIC_MODE;
		}
		else if(1 == network_tpye)
		{
			p->network_check = NETWORK_CHECK_DHCP_MODE;
		}
		else if(2 == network_tpye)
		{
			p->network_check = NETWORK_CHECK_PPPOE_MODE;
		}
		else
		{
			p->network_check = NETWORK_CHECK_NONE_MODE;
		}
	}
	
	switch(wan_status)
	{
		case WAN_NO_WIRE:
			if(WL_AP_MODE != wl_mode)
			{
				goto err_return;
			}
			p->button_action = BUTTON_CONNECTED;
			p->color_action = COLOR_ERR;
			p->time_action = TIME_NONE;
			p->code = COMMON_NO_WIRE;
			break;
		case WAN_DISCONNECTED:
			p->button_action = BUTTON_CONNECTED;
			p->color_action = COLOR_ERR;
			p->time_action = TIME_NONE;
			p->code = COMMON_NOT_CONNECT;
			break;
		case WAN_CONNECTING:
			if(PI_ERR == tpi_get_connecting_err_info(p,wifi_status))
			{
				goto err_return;
			}
			break;
		case WAN_CONNECTED:
			if(PI_ERR == tpi_get_connected_err_info(p))
			{
				goto err_return;
			}
			break;
		default:
			goto err_return;
	}
	
	ret = tpi_get_err_result(p);
	
	return ret;	

err_return:
	return INFO_ERROR;
}

WANERRINFO tpi_get_wan_err_info()
{
    WANERRINFO ret = INFO_ERROR;
    WLMODE wl_mode = WL_AP_MODE;

    wl_mode = tpi_get_wl_mode();

    switch(wl_mode)
    {
        case WL_AP_MODE:
        case WL_WISP_MODE:
        case WL_APCLIENT_MODE:
			ret = tpi_get_err_info(wl_mode);
            break;
		case WL_WDS_MODE:
        default:
            break;
    }

    return ret;
}
