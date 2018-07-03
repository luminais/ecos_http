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

#ifndef __ALI_NETWORK_SECURITY_H__
#define __ALI_NETWORK_SECURITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ALI_NS_OK    0
#define ALI_NS_ERR  -1

/*
 * @desc：安全检查结果枚举。需要特别说明的是：
 *    1. ALI_NS_RESULT_unavailable与其他结果类型互斥，即若检查结果为ALI_NS_RESULT_unavailable，
 *    则不能同时包含其他任何类型。
 *    2. ALI_NS_RESULT_normal、ALI_NS_RESULT_phishing两者互斥，即检查结果不能同时包含此两者。
 *    3. ALI_NS_RESULT_hijacking与ALI_NS_RESULT_normal/ALI_NS_RESULT_phishing可共存，
 *    即检查结果可以既包含ALI_NS_RESULT_hijacking又包含此三者中的一个。
 */
typedef enum
{
    ALI_NS_RESULT_BIT_unavailable = 0,   // “阿里安全”服务尚未开启
    ALI_NS_RESULT_BIT_normal,            // 域名正常
    ALI_NS_RESULT_BIT_phishing,          // 域名为钓鱼网站
    ALI_NS_RESULT_BIT_hijacking          // DNS劫持
}ALI_NS_RESULT_BIT_E;

/*
 * @desc：安全检查对应结果掩码
 */
#define ALI_NS_RESULT_MASK_unavailable (1 << ALI_NS_RESULT_BIT_unavailable)
#define ALI_NS_RESULT_MASK_normal      (1 << ALI_NS_RESULT_BIT_normal)
#define ALI_NS_RESULT_MASK_phishing   (1 << ALI_NS_RESULT_BIT_phishing)
#define ALI_NS_RESULT_MASK_hijacking   (1 << ALI_NS_RESULT_BIT_hijacking)


/*************************************************************************
Function:      int ali_ns_open(void);
Description:   启动“阿里安全”服务
Input:         无
Output:        无
Return:        ALI_NS_OK: 成功
               其他:        失败
Others:
*************************************************************************/
extern int ali_ns_open(void);


/*************************************************************************
Function:      int ali_ns_close(void);
Description:   停止“阿里安全”服务
Input:         无
Output:        无
Return:        ALI_NS_OK: 成功
               其他:        失败
Others:
*************************************************************************/
extern int ali_ns_close(void);


/*************************************************************************
Function:      int ali_ns_update(void);
Description:   更新“阿里安全”服务的安全库
Input:         无
Output:        无
Return:        ALI_NS_OK: 更新成功
               其他:        更新失败
Others:
*************************************************************************/
extern int ali_ns_update(void);


/*************************************************************************
Description:   云端威胁查询结果回调函数类型。
Input:         domain：     域名，以'\0'结束。
               mac_addr： 接入设备的MAC地址
               dns_srv_ip：DNS服务器IP地址。
Output:        result：   阿里安全检查结果，共有如下取值可能：
                 ALI_NS_RESULT_MASK_unavailable： “阿里安全”功能未开启
                 ALI_NS_RESULT_MASK_normal:      正常域名
                 ALI_NS_RESULT_MASK_phishing:    命中钓鱼网站
                 ALI_NS_RESULT_MASK_hijacking:   仅被DNS劫持
                 ALI_NS_RESULT_MASK_phishing | ALI_NS_RESULT_MASK_hijacking: 
                  访问钓鱼网站且被DNS劫持
Return:        ALI_NS_OK： 检查成功，具体检查结果参见result参数。
               其他：      检查出错
导致该错误。       
Others:
*************************************************************************/
typedef void (*ali_ns_reply_callback)(unsigned int query_flag,
                                      const char *domain, 
                                      const unsigned char *mac, 
                                      unsigned int dns_srv_ip, 
                                      unsigned int result);


/*************************************************************************
Function:      int ali_ns_set_reply_cb(ali_ns_reply_callback cb);
Description:   设置云端威胁查询结果的回调函数。
Input:         host：     域名，必须非NULL,不要求以'\0'结束。
Output:        result：   阿里安全检查结果，共有如下取值可能：
Return:        ALI_NS_OK： 检查成功，具体检查结果参见result参数。
               其他：      检查出错
导致该错误。       
Others:
*************************************************************************/
extern int ali_ns_set_reply_cb(ali_ns_reply_callback cb);


/*************************************************************************
Function:      int ali_ns_query(const char *domain,
                         unsigned char mac_addr[6],
                         unsigned int dns_srv_ip);
Description:   检查是否命中白名单或者钓鱼网站（需要说明的是：若配置为白名
               单，即使访问的是钓鱼网站，该函数仍返回   
               ALI_NS_PHISHING_WHITELIST）。
Input:         domain：     域名，必须非NULL,不要求以'\0'结束。
               mac_addr： 接入设备的MAC地址，必须非空，用于检查是否命中
                           白名单。
               dns_srv_ip：DNS服务器IP地址。
Output:        
Return:        ALI_NS_OK： 检查成功，具体检查结果参见result参数。
               其他：      检查出错
导致该错误。       
Others:
*************************************************************************/
extern int ali_ns_query(
        unsigned int query_flag,
        const char *domain, 
        unsigned char mac_addr[6], 
        unsigned int dns_srv_ip);


/*************************************************************************
Function:      const char* ali_ns_get_version(void)
Description:   获取“阿里安全”特性版本号
Input:         无
Output:        无
Return:        版本字符串
Others:
*************************************************************************/
extern const char* ali_ns_get_version(void);


#ifdef __cplusplus
}
#endif

#endif // __ALI_NETWORK_SECURITY_H__

