/* $Id: upnpreplyparse.h,v 1.1.1.1 2007-08-06 10:04:43 root Exp $ */
/* miniupnp project
 * see http://miniupnp.free.fr/
 * (c) 2005-2006 Thomas Bernard
 * This software is subjects to the conditions detailed in the LICENCE
 * file provided with this distribution */
#ifndef __UPNPREPLYPARSE_H__
#define __UPNPREPLYPARSE_H__

#ifndef WIN32
#include <sys/queue.h>
#else
#include "bsdqueue.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct NameValue {
    LIST_ENTRY(NameValue) entries;
    char name[64];
    char *value;
};

struct NameValueParserData {
    LIST_HEAD(listhead, NameValue) head;
    char curelt[64];
};

void ParseNameValue(const char * buffer, int bufsize,
                    struct NameValueParserData * data);

void ClearNameValueList(struct NameValueParserData * pdata);

char * GetValueFromNameValueList(struct NameValueParserData * pdata, const char * Name);

void DisplayNameValueList(char * buffer, int bufsize);

#ifdef __cplusplus
}
#endif

#endif

