/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_hash.h
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.28
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.28   1.0        	Learn from linux
************************************************************/


#ifndef __TAI_HASH_H__

#define __TAI_HASH_H__
#include "tai.h"

#define BKDR_SEED	131

struct hlist_node{
	struct hlist_node *next;
	struct hlist_node **prev;
};
struct hlist_head{
	struct hlist_node *first;
	int count;
};

	
struct man_hash{
	struct hlist_node node;
	struct Code_to_Str_t *info;	
};

struct hlist_head manutbl[256] ;

unsigned int bkdrhash(const char *str);

int init_manu_table(void) ;


#endif

