/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_hash.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.28
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.28   1.0          leran from linux
************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tai_hash.h"
#include "manufacture.h"

unsigned int bkdrhash(const char *str)
{
	unsigned int hash = 0;
	hash = str[0];
	hash = hash*BKDR_SEED + str[1];
	hash = hash*BKDR_SEED + str[3];
	hash = hash*BKDR_SEED + str[4];
	hash = hash*BKDR_SEED + str[6];
	hash = hash*BKDR_SEED + str[7];	
	return (hash&0xFF);
}

static inline void hlist_add(struct hlist_head *head, struct hlist_node *info)
{	
	info->next = head->first ;
	if(head->first != NULL)
		head->first->prev = &info->next;
	head->first=info;
	info->prev = &head->first;
}

static inline void hlist_del(struct hlist_node *info)
{	
	*info->prev = info->next;
	if(info->next)
		info->next->prev = info->prev;
	info->next = NULL;
	info->prev = NULL;
}

int init_manu_table(void)
{
	struct man_hash *node ;
	int i = 0 ;
	unsigned int key ;
	
	memset(manutbl, 0, sizeof(manutbl));	
	
	while(macToTypeList[i].type != -1)
	{		
		node = malloc(sizeof(struct man_hash));
		if(node == NULL)
			return -1;
		node->info = &macToTypeList[i];
		key = bkdrhash(macToTypeList[i].oui);
		hlist_add(&manutbl[key], &node->node);
		manutbl[key].count++;
		i++;
	}
	printf("Table size %d\n", i);
	return 0;
}

void collision_dump(void)
{
	int i;
	int max = 0;
	printf("\n");
	for(i = 0; i < 255; i++)
	{
		if(max < manutbl[i].count)
		{
			max = manutbl[i].count;
		}
		printf("%d%s", manutbl[i].count,i%16==15?"\n":" ");
	}
	printf("\nMax collision : \033[40;31m%d\033[0m\n", max);
}


