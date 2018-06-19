/*
 * Copyright (c) 2014-2015 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#ifndef _ALINKGW_API_H_
#define _ALINKGW_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/


#define ALINKGW_TRUE                1
#define ALINKGW_FALSE               0


#define ALINKGW_DISABLE				2
#define ALINKGW_UPDATE				1
#define ALINKGW_OK                  0
#define ALINKGW_ERR                 -1
#define ALINKGW_BUFFER_INSUFFICENT  -2

#define ALINKGW_SDK_VERSION         "v2.0.1"


/*************************************************************************
网关设备基本属性名称定义，设备厂商可以自定义新的属性
*************************************************************************/
#define ALINKGW_ATTR_WLAN_SWITCH_STATE      "wlanSwitchState"
#define ALINKGW_ATTR_WLAN_SWITCH_SCHEDULER  "wlanSwitchScheduler"
#define ALINKGW_ATTR_ALI_SECURITY           "aliSecurity"
#define ALINKGW_ATTR_PROBE_NUMBER           "probedNum"
#define ALINKGW_ATTR_PROBE_INFO             "proberInfo"
#define ALINKGW_ATTR_ACCESS_ATTACK_NUM		"accessAttackNum"
#define ALINKGW_ATTR_ACCESS_ATTACKR_INFO	"accessAttackerInfo"
#define ALINKGW_ATTR_TPSK                   "tpsk"
#define ALINKGW_ATTR_TPSK_LIST              "tpskList"

#define ALINKGW_SUBDEV_ATTR_DLSPEED         "dlSpeed"
#define ALINKGW_SUBDEV_ATTR_ULSPEED         "ulSpeed"
#define ALINKGW_SUBDEV_ATTR_WANDLSPEED         "wanDlSpeed"
#define ALINKGW_SUBDEV_ATTR_WANULSPEED        "wanUlSpeed"
#define ALINKGW_SUBDEV_ATTR_DLBWINFO         "dlBwInfo"
#define ALINKGW_SUBDEV_ATTR_ULBWINFO         "ulBwInfo"

#define ALINKGW_ATTR_PROBED_SWITCH_STATE     			 "probedSwitchState"
#define ALINKGW_ATTR_ACCESS_ATTACK_SWITCH_STATE      "accessAttackSwitchState"

#define ALINKGW_ATTR_WLAN_SETTING_24		"wlanSetting24g"
#define ALINKGW_ATTR_WLAN_SECURITY_24		"wlanSecurity24g"
#define ALINKGW_ATTR_WLAN_CHANNEL_24		"wlanChannelCondition24g"
#define ALINKGW_ATTR_WLAN_PAMODE			"wlanPaMode"
#define ALINKGW_ATTR_QOS_SETTING			"speedupSetting"

#define ALINKGW_ATTR_SUBDEVICE_DLSPEED		"dlSpeed"
#define ALINKGW_ATTR_SUBDEVICE_ULSPEED		"ulSpeed"

/*************************************************************************
网关设备基本service名称定义，设备厂商可以自定义新的service
*************************************************************************/
#define ALINKGW_SERVICE_AUTHDEVICE          "authDevice"
#define ALINKGW_SERVICE_CHANGEPASSWORD      "changePassword"

#define ALINKGW_SERVICE_BWCHECK				"bwCheck"
#define ALINKGW_SERVICE_FWUPGRADE 			"fwUpgrade"

#define ALINKGW_SERVICE_WL_CHANNEL			"refineWlanChannel"
/*************************************************************************
* @desc: attribute类型，分为两种：

一种是其值可以通过单个字符串/布尔值/数字表示的属性，如属性wlanSwitchState，
其值可以表示为：

"wlanSwitchState":{
    "value"："1",  //无线开关，bool类型，"1"代表打开，"0"代表关闭
    "when":"1404443369"
}；

另一种是其值需要多个<name, value>对组合表示的属性，如属性wlanSwitchScheduler，
其值为：
"wlanSwitchScheduler":
{
   "set":{
        "enabled":"1",  //定时器开关，bool类型，"1"代表打开，"0"代表关闭
        "offTime":[  //关闭wifi的定时时间
            "UTC+08:00 0 0 22 * * 1-5",
            "UTC+08:00 0 0 23 * * 6-7"
        ],
        "onTime":[  //打开wifi的定时时间
            "UTC+08:00 0 0 8 * * 1-5",
            "UTC+08:00 0 0 10 * * 6-7"
        ]
    },
    "when":"1404443369"
}

数组类型是多个集合表示的属性，如tpsklist:
"tpskList":
[
    {
        "set":{
            "tpsk":"12345678",   //hash(secret(model))
            "mac":"11:22:33:44:55:66",  //station的mac地址
            "duration":"0"  //有效期,0表示长期有效
        },
        "when":"1404443369"
    },
    {
        "set":{
            "tpsk":"12345678",   //hash(secret(model))
            "mac":"12:34:56:78:90:12"
            "duration":"0"
        },
        "when":"1404443369"
    }
]

*************************************************************************/
typedef enum {
    ALINKGW_ATTRIBUTE_simple = 0,  //值可由单个字符串/布尔值/数字表示的属性
    ALINKGW_ATTRIBUTE_complex,     //值必须由多个<name, value>对组合表示的属性
    ALINKGW_ATTRIBUTE_array,       //值必须由一个或多个集合数组表示的属性
    ALINKGW_ATTRIBUTE_MAX          //属性类型分类最大数，该值不表示具体类型
}ALINKGW_ATTRIBUTE_TYPE_E;


typedef enum {
    ALINKGW_STATUS_INITAL = 0,  //初始状态，
    ALINKGW_STATUS_INITED,      //alinkgw初始化完成
    ALINKGW_STATUS_REGISTERED,  //注册成功
    ALINKGW_STATUS_LOGGED       //登录服务器成功，正常情况下处于该状态
}ALINKGW_CONN_STATUS_E;

typedef enum{
    ALINKGW_LL_NONE = 0,
    ALINKGW_LL_ERROR,
    ALINKGW_LL_WARN,
    ALINKGW_LL_INFO,
    ALINKGW_LL_DEBUG
}ALINKGW_LOGLEVEL_E;

/*************************************************************************
Function:       int (*ALINKGW_KVP_save_cb)(const char *key,
                                           const char *value)
Description:    函数类型，表示厂商提供的alinkgw连接状态更新通知函数原型
Input:          new_status:    更新后的链接状态
Output:         无
Return:         无
Others:
*************************************************************************/
typedef void (*ALINKGW_STATUS_cb)(ALINKGW_CONN_STATUS_E new_status);



/*************************************************************************
Function:       int (*ALINKGW_KVP_save_cb)(const char *key,
                                           const char *value)
Description:    函数类型，表示厂商提供的保存<Key,Value>字符串Pair到非易失
                性存储器的函数原型
Input:          key:    value值对于的字符串关键字，带结束字符'\0'
                value:  存放保存的字符串值，带结束字符'\0'
Output:         无
Return:         0:      保存成功
                -1:     缓冲区不足
                其他:   其他错误
Others:
*************************************************************************/
typedef int (*ALINKGW_KVP_save_cb)(const char *key, \
                                   const char *value);



/*************************************************************************
Function:       int (*ALINKGW_KVP_load_cb)(const char *key,
                                           char *value,
                                           unsigned int buf_sz)
Description:    函数类型，表示厂商提供的从非易失性存储器中读取<Key, Value>
                字符串Pair的函数原型
Input:          key:    value值对于的字符串关键字，带结束字符'\0'
                buff_size:  value缓冲区长度
Output:         value:  存放读取的字符串值，带结束字符'\0'
Return:         0:      读取成功
                -1:     缓冲区不足
                其他:   读取失败
Others:
*************************************************************************/
typedef int (*ALINKGW_KVP_load_cb)(const char *key, \
                                   char *value, \
                                   unsigned int buf_sz);


/*************************************************************************
Function:       int (*ALINKGW_ATTRIBUTE_get_cb)(char *json_out_buf,
                                                unsigned int buf_sz)
Description:    函数类型，表示厂商提供的获取指定属性的值的函数原型
Input:          无
Output:         buf:    存放输出属性值的buf指针
                        属性值类型为simple，buf中存放单一值字符串,如:
						1或false等
                        属性值类型为complex，buf中存放json格式字符串，如:
						{"enabled":"1","offTime":[] "onTime":[]}
                buf_sz: 存放输出json串的buffer大小
Return:         0:      设置成功
                -2:     用于存放输出json串的buffer空间不够
                其他:   设置失败
Others:
*************************************************************************/
typedef int (*ALINKGW_ATTRIBUTE_get_cb)(char *buf, \
                                        unsigned int buf_sz);



/*************************************************************************
Function:       int (*ALINKGW_ATTRIBUTE_set_cb)(const char *sz_json_in)
Description:    函数类型，表示厂商提供的配置指定属性的函数原型
Input:          json_in: 输入字符串，结束字符'\0'
                        属性值类型为simple，json_in中存放单一值字符串,如:
						1或false等
                        属性值类型为complex，json_in中存放json格式字符串，如:
						{"enabled":"1","offTime":[] "onTime":[]}
Output:         无
Return:         0:      设置成功
                其他:   设置失败
Others:
*************************************************************************/
typedef int (*ALINKGW_ATTRIBUTE_set_cb)(const char *json_in);



/*************************************************************************
Function:       int (*ALINKGW_SERVICE_execute_cb)(const char *json_in,
                                                  char *json_out_buf,
                                                  unsigned int buf_sz);
Description:    函数类型，表示厂商提供的执行远端服务请求的函数原型
Input:          json_in:       输入字符串，结束字符'\0'
Output:         json_out_buf:  存放服务执行结果json串的buffer，没有结果输
                               出则无需存放
                buf_sz:        存放服务执行结果json串的buffer大小
Return:         0:      服务调用成功
                -1：    用于存放输出json串的buf空间不够
                其他:   服务调用失败
Others:
*************************************************************************/
typedef int (*ALINKGW_SERVICE_execute_cb)(const char *json_in, \
                                          char *json_out_buf, \
                                          unsigned int buf_sz);

/*************************************************************************
Description:    子设备属性上报结构体定义，表示子设备的结构体实例占内存空间长度非固定，
                 视属性个数而定
Mmenber:    mac[18]:        子设备mac地址，格式为小写字母表示的17Byte长冒号隔开的字符串,
                                eg: "00:11:22:cc:bb:dd"
            attr_name[]:    属性名称字符串数组，以NULL指针结尾
Others:
*************************************************************************/
typedef struct subdevice_attr_
{
    char mac[18];
    const char *attr_name[];
}subdevice_attr_t;


/*************************************************************************
Function:       int (*ALINKGW_ATTRIBUTE_subdevice_get_cb)(const char *subdev_mac, \
                                                  char *buf, \
                                                  unsigned int buf_sz);
Description:    函数类型，表示厂商提供的获取子设备指定属性的值的函数原型
Input:          subdev_mac:子设备mac地址，格式:xx:xx:xx:xx:xx:xx
Output:         buf:    存放输出属性值的buf指针
                        属性值类型为simple，buf中存放单一值字符串,如:
						1或false等
                        属性值类型为complex，buf中存放json格式字符串，如:
						{"enabled":"1","offTime":[] "onTime":[]}
                buf_sz: 存放输出json串的buffer大小
Return:         0:      设置成功
                -2:     用于存放输出json串的buffer空间不够
                其他:   设置失败
Others:
*************************************************************************/
typedef int (*ALINKGW_ATTRIBUTE_subdevice_get_cb)(const char *subdev_mac, \
                                                  char *buf, \
                                                  unsigned int buf_sz);


/*************************************************************************
Function:       int (*ALINKGW_ATTRIBUTE_subdevice_set_cb)(const char *subdev_mac, \
                                                  const char *json_in);
Description:    函数类型，表示厂商提供的配置子设备指定属性的函数原型
Input:          subdev_mac:子设备mac地址，字符串长度17Byte，结束字符'\0'
                        格式:xx:xx:xx:xx:xx:xx
                json_in: 输入字符串，结束字符'\0'
                        属性值类型为simple，json_in中存放单一值字符串,如:
						1或false等
                        属性值类型为complex，json_in中存放json格式字符串，如:
						{"enabled":"1","offTime":[] "onTime":[]}
Output:         无
Return:         0:      设置成功
                其他:   设置失败
Others:
*************************************************************************/
typedef int (*ALINKGW_ATTRIBUTE_subdevice_set_cb)(const char *subdev_mac, \
                                                  const char *json_in);


/*************************************************************************
Function:       int ALINKGW_report_attr_subdevices(subdevice_attr_t *subdev_attrs[]);
Description:    多个子设备的属性间接上报接口，支持多属性上报，需要注意：
                - 该函数需要在ALINKGW_start()后调用
Input:          subdev_attrs:  上报的子设备和属性结构体指针数组，以NULL结束
Output:         无
Return:         0:      上报成功;
                其他:   上报失败
Others:
*************************************************************************/
int ALINKGW_report_attr_subdevices(subdevice_attr_t *subdev_attrs[]);



/*************************************************************************
Function:       int ALINKGW_register_attribute_subdevice(const char *attr_name,\
                                       ALINKGW_ATTRIBUTE_TYPE_E type,\
                                       ALINKGW_ATTRIBUTE_subdevice_get_cb get_cb,\
                                       ALINKGW_ATTRIBUTE_subdevice_set_cb set_cb);
Description:    注册子设备属性
Input:          name:   属性名称
                type:   属性值类型
                get_cb: 获取属性值的回调函数
                set_cb: 设置属性值的回调函数
Output:         无
Return:         0:      注册成功;
                其他:   注册失败
Others:
*************************************************************************/
int ALINKGW_register_attribute_subdevice(const char *attr_name,\
                                           ALINKGW_ATTRIBUTE_TYPE_E type,\
                                           ALINKGW_ATTRIBUTE_subdevice_get_cb get_cb,\
                                           ALINKGW_ATTRIBUTE_subdevice_set_cb set_cb);



/*************************************************************************
Function:       ALINKGW_register_attribute(const char *name,
                                           ALINKGW_ATTRRIBUTE_TYPE_E type,
                                           ALINKGW_ATTRIBUTE_get_cb get_cb,
ALINKGW_ATTRIBUTE_set_cb set_cb);
Description:    注册厂商指定的设备属性
Input:          name:   属性名称
                type:   属性值类型
                get_cb: 获取属性值的回调函数
                set_cb: 设置属性值的回调函数
Output:         无
Return:         0:      注册成功;
                其他:   注册失败
Others:
*************************************************************************/
int ALINKGW_register_attribute(const char *name,
                               ALINKGW_ATTRIBUTE_TYPE_E type,
                               ALINKGW_ATTRIBUTE_get_cb get_cb,
                               ALINKGW_ATTRIBUTE_set_cb set_cb);



/*************************************************************************
Function:       ALINKGW_register_service(const char *name,
                                         ALINKGW_SERVICE_EXEC_cb exec_cb)
Description:    注册厂商指定的可被远端物联平台调用的RPC服务
Input:          name:      服务名称
                exec_bc:   远程调用服务回调函数
Output:         无
Return:         0:      注册成功;
                其他:   注册失败
Others:
*************************************************************************/
int ALINKGW_register_service(const char *name,
                             ALINKGW_SERVICE_execute_cb exec_cb);


/*************************************************************************
Function:       ALINKGW_unregister_service(const char *name)
Description:    注销厂商指定的可被远端物联平台调用的RPC服务
Input:          name:      服务名称
Output:         无
Return:         0:      注销成功;
                其他:   注册失败
Others:
*************************************************************************/
int ALINKGW_unregister_service(const char *name);


/*************************************************************************
Function:       ALINKGW_set_kvp_cb(ALINKGW_KVP_save_cb save_cb,
                                   ALINKGW_KVP_load_cb load_cb)
Description:    设置厂商提供的<Key,Value>字符串Pair的save/load的回调函数
Input:          save_cb:    保存<Key,Value>字符串Pair的回调函数，需非空
                load_cb:    恢复<Key,Value>字符串Pair的回调函数，需非空
Output:         无
Return:         无
Others:
*************************************************************************/
void ALINKGW_set_kvp_cb(ALINKGW_KVP_save_cb save_cb,
                        ALINKGW_KVP_load_cb load_cb);


/*************************************************************************
Function:       ALINKGW_set_conn_status_cb(int *new_status)
Description:    设置厂商提供的alinkgw连接server端状态变化回调函数
Input:          无
Output:         new_status: 链接状态更新回调函数，必须非空
Return:         无
Others:
*************************************************************************/
void ALINKGW_set_conn_status_cb(ALINKGW_STATUS_cb status_cb);


/*************************************************************************
Function:       int ALINKGW_report_attr(const char *attr_name)
Description:    设备属性并更，间接上报接口,需要注意：
            - 该函数需要在ALINKGW_start()后调用
Input:          attr_name:  上报的属性名称
Output:         无
Return:         0:      上报成功;
                其他:   上报失败
Others:
*************************************************************************/
int ALINKGW_report_attr(const char *attr_name);


/*************************************************************************
Function:       int ALINKGW_report_attr(const char *attr_name)
Description:    设备多个属性，间接上报接口,需要注意：
            - 该函数需要在ALINKGW_start()后调用
Input:          attr_name:  上报的属性名称指针数组，以NULL结束
Output:         无
Return:         0:      上报成功;
                其他:   上报失败
Others:
*************************************************************************/
int ALINKGW_report_attrs(const char *attr_name[]);



/*************************************************************************
Function:       int ALINKGW_report_attr_direct(const char *name, )
Description:    设备属性直接上报接口,需要注意：
            - 该函数需要在ALINKGW_start()后调用
Input:          attr_name:  上报的属性名称
                type:       上报属性的类型
                data:       上报属性的值
Output:         无
Return:         0:      上报成功;
                其他:   上报失败
Others:
*************************************************************************/
int ALINKGW_report_attr_direct(const char *attr_name,\
                               ALINKGW_ATTRIBUTE_TYPE_E type,\
                               const char *data);


/*************************************************************************
Function:       ALINKGW_attach_sub_device(const char *name,
                                          const char *type,
                                          const char *category,
                                          const char *manufacturer,
                                          const char *mac)
Description:    接入网关的子设备上线上报
Input:          name:           设备名称
                type:           设备类型
                category:       设备分类
                manufacturer:   设备厂商，未知厂商统一为:unknown
                mac:            设备mac地址，必须非空串，格式:xx:xx:xx:xx:xx:xx
Output:         无
Return:         0:     上报成功;
                其他:  上报失败
Others:
*************************************************************************/
int ALINKGW_attach_sub_device(const char *name,
                              const char *type,
                              const char *category,
                              const char *manufacturer,
                              const char *mac);


/*************************************************************************
Function:       ALINKGW_detach_sub_device(const char *dev_mac)
Description:    接入网关的子设备离线上报
Input:          dev_mac:    子设备mac地址，格式:xx:xx:xx:xx:xx:xx
Output:         无
Return:         0:     上报成功;
                其他:  上报失败
Others:
*************************************************************************/
int ALINKGW_detach_sub_device(const char *dev_mac);



/*************************************************************************
Function:       int ALINKGW_wait_connect(int timeout)
Description:    等待alinkgw连接服务器，直到登录成功或超时返回
Input:          timeout:    等待返回超时时间，单位秒，-1表示一直等待
Output:         无
Return:         0:      登录成功
                其他:   未登录成功
Others:
*************************************************************************/
int ALINKGW_wait_connect(int timeout);



/*************************************************************************
Function:       int ALINKGW_set_sandbox_mode()
Description:    设置沙箱模式（若调用该函数，路由器连接内部测试用的物联平台
                环境；否则，路由器连接正式对外发布版本的物联平台环境），需要注意：
            - 该函数需要在ALINKGW_start()前调用
Input:
Output:         无
Return:         0:      设置成功
                其他:   设置失败
Others:
*************************************************************************/
int ALINKGW_set_sandbox_mode();

/***********************************************************************
Function:       int ALINKGW_enable_asec(unsigned int bEnabled)
Description:    开启（或关闭）“阿里安全”插件功能，需要注意：
            - “阿里安全”功能默认开启
            - 当用户通过UI界面关闭/开启“阿里安全”时，调用本接口
Input:          bEnabled： 1 - 开启， 0 - 关闭
Output:         无
Return:         0:      设置成功
                其他:   设置失败
Others:
*************************************************************************/
int ALINKGW_enable_asec(unsigned int bEnabled);

/*************************************************************************
Function:       int ALINKGW_set_loglevel(ALINKGW_LOGLEVEL_E e)
Description:    设置日志信息级别（默认级别为ALINKGW_LL_ERROR）
Input:
Output:         无
Return:         无
Others:
*************************************************************************/
void ALINKGW_set_loglevel(ALINKGW_LOGLEVEL_E e);

/*************************************************************************
Function:       int ALINKGW_reset_binding()
Description:    解除路由器设备和手机APP用户的绑定关系。需要注意：
                - 该函数仅适用于恢复路由器出厂设置场景，请勿在其他场景下调用
				- 该函数只有在设备联网环境才能成功
                - 该函数调用需在ALINKGW_start()之后、ALINKGW_end()之前调用
Input:
Output:         无
Return:         0:      解绑成功
                其他:   解绑失败
Others:
*************************************************************************/
int ALINKGW_reset_binding();

/*************************************************************************
Function:       ALINKGW_start(const char *sn, ....)
Description:    启动alink服务，调用该接口前，要配置平台信息、注册属性和服务
Input:          sn:        设备序列号, 最大64字节
                name:      设备名称，最大32字节
                brand:     设备品牌，最大32字节
                type:      设备类型，最大32字节
                category:  设备分类，最大32字节
                manufacturer:   设备制造商, 最大32字节
                version:   固件版本号,最大32字节
                mac:       设备mac地址，格式11:22:33:44:55:66，建议使用lan
				           接口mac地址,避免mac克隆引起设备mac地址变更
                model:     设备型号，由物联平台授权，最大80字节
                cid:       设备芯片ID，同类型不同设备的cid必须不同，最大64字节
                key:       设备链接云端key，由物联平台颁发
                secret:    设备链接云端秘钥，由物联平台颁发
Output:         无
Return:         0:     启动成功;
                其他:  启动失败
Others:
*************************************************************************/
int ALINKGW_start(
    const char *sn,
    const char *name,
    const char *brand,
    const char *type,
    const char *category,
    const char *manufacturer,
    const char *version,
    const char *mac,
    const char *model,
    const char *cid,
    const char *key,
    const char *secret);

/*************************************************************************
Function:       ALINKGW_end()
Description:    停止alink服务，并释放资源
Input:          无
Output:         无
Return:         0:     停止成功;
                其他:  停止失败
Others:
*************************************************************************/
int ALINKGW_end();


/*************************************************************************
Function:       ALINKGW_cloud_save()
Description:    云端存储数据
Input:          name: 数据名称字符串（以'\0'结尾）
                val_buf: 数据值buffer
                val_len: 数据值buffer length
Output:         无
Return:         0:     保存成功;
                其他:  保存失败
Others:
*************************************************************************/
int ALINKGW_cloud_save(
    const char *name,
    unsigned char *val_buf,
    unsigned int val_len);

/*************************************************************************
Function:       ALINKGW_cloud_restore()
Description:    从云端恢复数据
Input:          name: 数据名称字符串（以'\0'结尾）
Output:         val_buf: 存放从云端获取的数据值的buffer
                val_len: value-result参数，用作value时，存放buffer的size；用作result时，存放value length
                        用作result时，存放value length
Return:         0:     恢复成功;
                其他:  恢复失败
Others:
*************************************************************************/
int ALINKGW_cloud_restore(
    const char *name,
    unsigned char *val_buf,
    unsigned int *val_len);


/*************************************************************************
Function:       ALINKGW_get_cloud_attrs()
Description:    从云端获取最近一次上报的属性值接口
Input:          attr_name:  获取的属性名称数组，以NULL结束
Output:         val_buf: 存放从云端获取的属性值的buffer
                val_len: value-result参数，用作value时，存放buffer的size；
                        用作result时，存放value length
Return:         0:     恢复成功;
                其他:  恢复失败
Others:
*************************************************************************/
int ALINKGW_get_cloud_attrs(
    const char *attr_name[],
    unsigned char *val_buf,
    unsigned int *val_len);



/************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _ALINKGW_API_H_ */

