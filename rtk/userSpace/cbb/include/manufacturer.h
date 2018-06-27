#ifndef __MANUFACTURER_H__
#define __MANUFACTURER_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define CLOUD_SERVER_NAME       "api.cloud.tenda.com.cn"
#define CLOUD_REQUST_URL          "/route/mac/v1"
#define CLOUD_SERVER_PORT       80
#ifndef MAX_STA_NUM
#define MAX_STA_NUM  64
#endif

#define FREE_P(p) if(*p) \
    { \
        free(*p); \
        *p=NULL; \
    }

struct facture_mac
{
    unsigned char mac[3];
    char flag;
};

struct facture_name
{
    unsigned char mac[3];
    char* name;
    struct facture_name* next;
};

struct mac_node
{
    char mac[20];
    struct mac_node* next;
};


/*TPI*/
extern RET_INFO tpi_manufacturer_first_init();
extern RET_INFO tpi_manufacturer_action(RC_MODULES_COMMON_STRUCT *var);
#endif/*__MANUFACTURER_H__*/