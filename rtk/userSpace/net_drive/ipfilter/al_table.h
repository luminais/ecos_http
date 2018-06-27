/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: al_table.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.2.6
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.2.6   1.0          new
************************************************************/
#define ETHER_ADDR_LEN 		6

#define NAME_LEN_256		256

#define BLACK_LIST_LEN	200

#define WHITE_LIST_LEN	50

#if 0
struct b_node
{
	char host_url[NAME_LEN_256] ;
};
#endif

struct b_list
{
	int position ;
	char  host_url[BLACK_LIST_LEN][NAME_LEN_256] ;
};
#if 0
struct w_node
{
	char mac[ETHER_ADDR_LEN] ;
	char host_url[NAME_LEN_256] ;
};
#endif

struct w_list
{
	int position;
	unsigned char mac[WHITE_LIST_LEN][ETHER_ADDR_LEN] ;
	char host_url[WHITE_LIST_LEN][NAME_LEN_256] ;
};


struct b_list black_list ;

struct w_list white_list ;

