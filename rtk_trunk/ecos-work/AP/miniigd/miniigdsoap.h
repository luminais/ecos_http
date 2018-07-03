/* $Id: upnpsoap.h,v 1.1.1.1 2007-08-06 10:04:43 root Exp $ */
/* miniupnp project
 * http://miniupnp.free.fr/
 * (c) 2006 Thomas Bernard 
 * This software is subject to the condition detailed in
 * the LICENCE file included in the distribution */
#ifndef __UPNPSOAP_H__
#define __UPNPSOAP_H__

/* ExecuteSoapAction() :
 * This method execute the requested Soap Action */
void ExecuteSoapAction(struct upnphttp *, const char *, int);

/* Sends a correct SOAP error with an UPNPError code and 
 * description */
void
SoapError(struct upnphttp * h, int errCode, const char * errDesc);

#endif

