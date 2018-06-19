#ifndef _GPI_WAN_ERR_INFO_
#define _GPI_WAN_ERR_INFO_
#include "pi_common.h"
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
#ifndef WANERRINFO
#define WANERRINFO unsigned int
#define INFO_ERROR 0
#endif

/*
由低到高的开始计算
第1位和第2位表示错误代码编号
第3为表示WAN口环境检测结果(0表示不检测,1表示static IP,2表示DHCP,3表示PPPOE)
第4位表示WAN口类型(1表示static IP,2表示DHCP,3表示PPPOE)
第5位表示工作模式(0表示AP,1表示WISP,2表示APClient)
第6位传给页面显示连接时间/IP信息等(0.不显示1.显示)
第7位传给页面显示颜色(1表示错误颜色、2表示尝试颜色、3表示成功颜色)
第8位传给页面判断是否有断开操作(1,断开2连接)(为了兼容需要断开按钮的软件需求)
这里面是尽量罗列出所有的状态，不将连接中去掉，具体在上层接口中实现
*/
typedef struct wan_err_info_struct
{
	unsigned char code;/*0-99表示错误代码*/
	unsigned char network_check;/*0-3表示WAN口环境检测结果,0无结果或者不检测,1STATIC,2DHCP,3PPPOE*/
	unsigned char wan_mode;/*1-3表示WAN口类型,1STATIC,2DHCP,3PPPOE*/
	unsigned char wl_mode;/*0-2表示工作模式,0AP,1WISP,2APCLIENT*/
	unsigned char time_action;/*0-1表示显示IP信息,0不显示,1显示*/
	unsigned char color_action;/*1-3表示页面颜色,1错误,2尝试,3成功*/
	unsigned char button_action;/*1-2表示按钮类型,1断开,2连接*/
} WAN_ERR_INFO_STRUCT,*P_WAN_ERR_INFO_STRUCT;

typedef enum
{
    COMMON_NONE                     = 0,
    COMMON_NO_WIRE                  = 1,
    COMMON_NOT_CONNECT              = 2,
    COMMON_CONNECTING               = 3,
    COMMON_CONNECTED_ONLINEING      = 4,
    COMMON_NOT_ONLINE               = 5,
    COMMON_ONLINEED                 = 6,
} COMMONERRCODE;

typedef enum
{
    STATIC_WL_CHECKED_PASSWORD_FAIL = (COMMON_ONLINEED + 1),
	STATIC_NETWORK_CHECK			= (COMMON_ONLINEED + 2),
} STATICERRCODE;

typedef enum
{
    DHCP_IP_CONFLLICT               = (COMMON_ONLINEED + 1),
	DHCP_NO_RESPOND               	= (COMMON_ONLINEED + 2),
	DHCP_WL_CHECKED_PASSWORD_FAIL 	= (COMMON_ONLINEED + 3),
	DHCP_NETWORK_CHECK				= (COMMON_ONLINEED + 4),
} DHCPERRCODE;

typedef enum
{
    PPPOE_CHECKED_PASSWORD_FAIL     = (COMMON_ONLINEED + 1),
    PPPOE_NO_RESPOND                = (COMMON_ONLINEED + 2),
	PPPOE_WL_CHECKED_PASSWORD_FAIL	= (COMMON_ONLINEED + 3),
	PPPOE_NETWORK_CHECK				= (COMMON_ONLINEED + 4),
} PPPOEERRCODE;

#define ERR_CHECK       "err_check"
/*
ERR_CHECK:
0:默认值
7:正在验证用户名和密码
2:用户名和密码错误
3:拨号成功
5:远程客户端无响应
11:DHCP冲突
*/
#define WAN_ISONLN      "wan_isonln"
/*
WAN_ISONLN:
0:未联网
1:已联网
*/
#define WAN_CHECK       "wan_check"
/*
WAN_CHECK:
0:检测中
1:检测完成
*/
extern int network_tpye;
#endif
