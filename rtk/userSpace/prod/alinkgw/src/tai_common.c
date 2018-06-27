/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_common.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.27
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.30   1.0          new
************************************************************/
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include "tai.h"
#include "alinkgw_api.h"



#if 0
#define  DEVNAME       	   		"F456"
#define  DEVMODEL	 	   		"TENDA_NETWORKING_ROUTER_FH456"
#define  DEVMANUFACTURE  	   	"Tenda"
#ifndef NKGW_RELEASE_VERSION
	#define  DEVKEY		       		"05skP7pHf8FJEPcMd3P3"                      
	#define  DEVSECRET	       	   	"CiBIIj5Ev2dVzEArtAkp4HvSQOgm5FxF1B7Bgyei"  
#else
	#define  DEVKEY		       		"o3JF99FaJQG7icApRFe3"
	#define  DEVSECRET	       	   	"uBRnIATi0MgCXeJOMrv45AH8DvS2jtfCwYCWrkGi"
#endif

#define  DEVTYPE		  			"ROUTER"
#define  DEVCATEGORY	   		"NETWORKING"
#define  DEVSN		           		"12345678"
#define  DEVVERSION		   		"1.0.4"
#define  DEVBRAND		       	"Tenda"
#define  DEVCID			   		"107AFF1F147AFF1F187AFF1F"	//后期要换
#define DEV_MAC_NAME			"et0macaddr"
#endif

#if 1
#define  DEVNAME       	   		"FH456"
#define  DEVMODEL	 	   		"TENDA_NETWORKING_ROUTER_FH456"
#define  DEVMANUFACTURE  	   	"Tenda"
#ifndef NKGW_RELEASE_VERSION
	#define  DEVKEY		       		"UtPDBr8GLJHpUicQmqVt"                      
	#define  DEVSECRET	       	   	"40PhlIFzkrTSrvj5Nfm5qcSLcDgHpqmdoE3ksBTw"  
#else
	#define  DEVKEY		       		"fvAvBRLmB5h7NzetWW2P"
	#define  DEVSECRET	       	   	"zxa3g6gaEzmRxn7CTtetmxFnZKWX11snh79jxKsx"
#endif

#define  DEVTYPE		  		"ROUTER"
#define  DEVCATEGORY	   		"NETWORKING"
#define  DEVSN		           	"12345678"
#define  DEVVERSION		   		"1.0.0; APP2.0"
#define  DEVBRAND		       	"Tenda"
#define  DEVCID			   		"107AFF1F147AFF1F187AFF1F"	
#define  DEV_MAC_NAME			"et0macaddr"
#endif



#if 0
typedef enum {
    ALINKGW_STATUS_INITAL = 0,  //初始状态，
    ALINKGW_STATUS_INITED,      //alinkgw初始化完成
    ALINKGW_STATUS_REGISTERED,  //注册成功
    ALINKGW_STATUS_LOGGED       //阻塞不可用
}ALINKGW_CONN_STATUS_E;
#endif


int save_kvp_value(const char* name , const char* value)
{

	if(name == NULL || value == NULL)
	{
		return ALINKGW_ERR ;
	}

	LLDEBUG("[name:%s][value:%s]\n" , name , value);
	
	nvram_set(name , value) ;

	nvram_commit();
	
	return ALINKGW_OK ;
}


int get_kvp_value(const char* name , char* value , unsigned int buf_sz)
{
	if(name == NULL || value == NULL)
	{
		return ALINKGW_ERR ;
	}

	strncpy(value , nvram_safe_get(name) , buf_sz) ;
	
	LLDEBUG("[name:%s][value:%s]\n" , name , value);

	return ALINKGW_OK ;

}

//add by z10312 20150401
#define  NKGW_RESET_BIND "ali_reset_bind"
#define ALINKGW_KEY		"alikey"
#define ALINKGW_SECRET		"alisecret"
#define ALINKGW_MODEL		"alimodel"

ALINKGW_CONN_STATUS_E last_status = ALINKGW_STATUS_INITAL ;
extern int tenda_wan_link_status(void);
int ali_get_connection_status()
{
	printf("wan_link_status:%d last_status:%d\n",tenda_wan_link_status(), last_status);
	if (tenda_wan_link_status() && ALINKGW_STATUS_LOGGED == last_status)
		return 1;
	else
		return 0;	
}

void connection_status_update(ALINKGW_CONN_STATUS_E status)
{
    
    int ret;
    char mib_val[32];

    if(status != last_status)
    {
        LLDEBUG("connect status update , [status:%d]\n" , status);

        
    }
    //add by z10312 20150401  添加解除绑定接口
    //网络链接恢复，在线clients重新attatch前先解除绑定
    if(ALINKGW_STATUS_LOGGED == status)
    {
        sprintf(mib_val, "%s", nvram_safe_get(NKGW_RESET_BIND));
        
        if (strcmp("1", mib_val) )
        {
            ret = ALINKGW_reset_binding();
            if(ret == 0)
            {
                nvram_set(NKGW_RESET_BIND, "1");
                nvram_commit();
                printf("Reset binding info Ok\n");
            }else{
                printf("Reset binding info Failed with ret:%d\n",ret);
            }	
        }
    }
    //end by z10312 20150401
    
    last_status = status;
    return ;	
}


int get_router_common_attribute(struct DEV_ATTRIBUTE* r_attribute)
{
	char model[ROUTER_ATTRIBUTE_LEN_80] ={0};
	char key[ROUTER_ATTRIBUTE_LEN_64] ={0};
	char secret[ROUTER_ATTRIBUTE_LEN_64] ={0};
	char *devname = NULL;
	
	if(r_attribute == NULL )
	{
		return ALINKGW_ERR ;
	}
	//sn
	strncpy(r_attribute->sn , DEVSN, ROUTER_ATTRIBUTE_LEN_64);	

	//brand
	strncpy(r_attribute->brand , DEVBRAND , ROUTER_ATTRIBUTE_LEN_32) ;

	//type
	strncpy(r_attribute->type , DEVTYPE , ROUTER_ATTRIBUTE_LEN_32) ;
	
	//category
	strncpy(r_attribute->category, DEVCATEGORY, ROUTER_ATTRIBUTE_LEN_32) ;
	
	//manufacturer
	strncpy(r_attribute->manufacturer, DEVMANUFACTURE, ROUTER_ATTRIBUTE_LEN_32) ;
	
	//version
	strncpy(r_attribute->version, DEVVERSION, ROUTER_ATTRIBUTE_LEN_32) ;
	
	//mac
	strncpy(r_attribute->mac , nvram_safe_get(DEV_MAC_NAME) , ROUTER_ATTRIBUTE_LEN_20 ) ;

	//model
	snprintf(model, sizeof(model), "%s", nvram_safe_get(ALINKGW_MODEL));
	if (strcmp("", model) == 0)
        	strncpy(r_attribute->model, DEVMODEL, ROUTER_ATTRIBUTE_LEN_80) ;
	else
		strncpy(r_attribute->model, model, ROUTER_ATTRIBUTE_LEN_80) ;

	//cid
	strncpy(r_attribute->cid, DEVCID, ROUTER_ATTRIBUTE_LEN_64) ;

	//key	
	snprintf(key, sizeof(key), "%s", nvram_safe_get(ALINKGW_KEY));
	if (strcmp("", key) == 0)
        	strncpy(r_attribute->key, DEVKEY, ROUTER_ATTRIBUTE_LEN_64) ;
	else
		strncpy(r_attribute->key, key, ROUTER_ATTRIBUTE_LEN_64) ;


	//secret
	snprintf(secret, sizeof(secret), "%s", nvram_safe_get(ALINKGW_SECRET));
	if (strcmp("", secret) == 0)
        	strncpy(r_attribute->secret, DEVSECRET , ROUTER_ATTRIBUTE_LEN_64) ;
	else
		strncpy(r_attribute->secret, secret , ROUTER_ATTRIBUTE_LEN_64) ;

	//name
	 devname = strstr(r_attribute->model, "ROUTER_");	 
	 if(devname == NULL)
	 {
		strncpy(r_attribute->name , DEVNAME , ROUTER_ATTRIBUTE_LEN_32) ;
	 }else{
		strncpy(r_attribute->name , devname + strlen("ROUTER_") , ROUTER_ATTRIBUTE_LEN_32) ;
	 }
	 
	return 0 ;
	
}




