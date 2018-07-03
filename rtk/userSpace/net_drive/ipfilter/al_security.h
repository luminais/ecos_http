/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: al_security.h
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.2.6
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.27   1.0          new
************************************************************/

#define ALIBABADEBUG

#ifdef ALIBABADEBUG
#define LLDEBUG(fmt ,args...) 		printf("[debug][line:%d][function:%s]"fmt , __LINE__ , __FUNCTION__ ,args ) ;  

#define SHOWDEBUG(args) 		printf("[debug][line:%d][function:%s][%s]\n" , __LINE__ , __FUNCTION__, args) ;

#endif


int (*al_security_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);

int al_security_handle_hook(struct ifnet *ifp, char *head, struct mbuf *m) ;

int al_security_handle_prepare(void) ;

int al_security_handle_after(void) ;

int get_lan_ip_addr(void) ;


