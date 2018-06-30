#ifndef __TAI_H__
#define __TAI_H__

#define NKGW_RELEASE_VERSION

#define ALIBABADEBUG

#ifdef ALIBABADEBUG

#define AL_2_DEBUG(fmt , args...)	printf("[debug][function:%s][line:%d]"fmt , __FUNCTION__ , __LINE__ , args); 

#define AL_SHOW(args)					printf("[debug][function:%s][line:%d][%s]\n" , __FUNCTION__ , __LINE__ , args); 

#define LLDEBUG(fmt ,args...) 		printf("[debug][line:%d][function:%s]"fmt , __LINE__ , __FUNCTION__ ,args ) ;  

#define SHOWDEBUG(args) 		printf("[debug][line:%d][function:%s][%s]\n" , __LINE__ , __FUNCTION__, args) ;

#endif

#define ROUTER_ATTRIBUTE_LEN_8		8
#define ROUTER_ATTRIBUTE_LEN_16		16
#define ROUTER_ATTRIBUTE_LEN_20		20
#define ROUTER_ATTRIBUTE_LEN_32		32
#define ROUTER_ATTRIBUTE_LEN_64		64
#define ROUTER_ATTRIBUTE_LEN_80		80

struct DEV_ATTRIBUTE
{
	char sn[ROUTER_ATTRIBUTE_LEN_64];
	char name[ROUTER_ATTRIBUTE_LEN_32] ;
	char brand[ROUTER_ATTRIBUTE_LEN_32] ;
	char type[ROUTER_ATTRIBUTE_LEN_32];
	char category[ROUTER_ATTRIBUTE_LEN_32] ;
	char manufacturer[ROUTER_ATTRIBUTE_LEN_32];
	char version[ROUTER_ATTRIBUTE_LEN_32];
	char mac[ROUTER_ATTRIBUTE_LEN_20] ;
	char model[ROUTER_ATTRIBUTE_LEN_80] ;
	char cid[ROUTER_ATTRIBUTE_LEN_64] ;
	char key[ROUTER_ATTRIBUTE_LEN_64] ;
	char secret[ROUTER_ATTRIBUTE_LEN_64] ;
};

struct Code_to_Str_t{
	int	type;
	char	*oui;
	char	*manufacture;
};





#define ARRAY_LEN_4				4
#define ARRAY_LEN_8				8
#define ARRAY_LEN_12			12
#define ARRAY_LEN_16			16
#define ARRAY_LEN_20			20
#define ARRAY_LEN_32			32
#define ARRAY_LEN_64			64
#define ARRAY_LEN_128			128
#define ARRAY_LEN_256			256
#define ARRAY_LEN_512			512
#define ARRAY_LEN_1024			1024





#endif

