/* $Id: upnpreplyparse.c,v 1.1.1.1 2007-08-06 10:04:43 root Exp $ */
/* Project : miniupnp
 * Website : http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * Author : Thomas Bernard
 * Copyright (c) 2005 Thomas Bernard
 * This software is subject to the conditions detailed in
 * the LICENCE file provided with this distribution. */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "upnpreplyparse.h"
#include "minixml.h"
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
#endif
/* *wps will use mini_upnp, mini_upnp alreadly has the code below. 
  * remember to keep sync with upnpreplyparse.c  upnpreplyparse.h minixml.h  
  * in mini_upnp directory
  */

static void
NameValueParserStartElt(void * d, const char * name, int l)
{
    struct NameValueParserData * data = (struct NameValueParserData *)d;
    if(l>63)
        l = 63;
    memcpy(data->curelt, name, l);
    data->curelt[l] = '\0';
}

static void
NameValueParserGetData(void * d, const char * datas, int l)
{
    struct NameValueParserData * data = (struct NameValueParserData *)d;
    struct NameValue * nv;
    nv = malloc(sizeof(struct NameValue));
	if(nv == NULL)
	{
		return;
	}
	memset(nv, 0, sizeof(struct NameValue));
	nv->value = (char *) malloc(l+1);
	memset(nv->value, 0, l+1);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD,sizeof(struct NameValue));
#endif
    strncpy(nv->name, data->curelt, 64);
    memcpy(nv->value, datas, l);
    LIST_INSERT_HEAD( &(data->head), nv, entries);
}

void ParseNameValue(const char * buffer, int bufsize,
                    struct NameValueParserData * data)
{
    struct xmlparser parser;
    LIST_INIT(&(data->head));
    /* init xmlparser object */
    parser.xmlstart = buffer;
    parser.xmlsize = bufsize;
    parser.data = data;
    parser.starteltfunc = NameValueParserStartElt;
    parser.endeltfunc = 0;
    parser.datafunc = NameValueParserGetData;
	parser.attfunc = 0;
    parsexml(&parser);
}

void ClearNameValueList(struct NameValueParserData * pdata)
{
    struct NameValue * nv;
    while((nv = pdata->head.lh_first) != NULL)
    {
    		if (nv->value)
			free(nv->value);
        LIST_REMOVE(nv, entries);
        free(nv);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,sizeof(struct NameValue));
#endif
    }
}

char * 
GetValueFromNameValueList(struct NameValueParserData * pdata,
                          const char * Name)
{
    struct NameValue * nv;
    char * p = NULL;
    for(nv = pdata->head.lh_first;
        (nv != NULL) && (p == NULL);
        nv = nv->entries.le_next)
    {
        if(strcmp(nv->name, Name) == 0)
            p = nv->value;
    }
    return p;
}

/* debug all-in-one function 
 * do parsing then display to stdout */
void DisplayNameValueList(char * buffer, int bufsize)
{
    struct NameValueParserData pdata;
    struct NameValue * nv;
    ParseNameValue(buffer, bufsize, &pdata);
    for(nv = pdata.head.lh_first;
        nv != NULL;
        nv = nv->entries.le_next)
    {
        printf("%s = %s\n", nv->name, nv->value);
    }
    ClearNameValueList(&pdata);
}

