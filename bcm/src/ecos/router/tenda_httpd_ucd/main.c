/*
 * main.c -- Main program for the GoAhead WebServer (eCos version)
 *
 * Copyright (c) Go Ahead Software Inc., 1995-1999. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: main.c,v 1.3 2002/01/24 21:57:47 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer. This is a demonstration
 *	main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/
#include <autoconf.h>
#include <router_net.h>

#include  "uemf.h"
#include  "wsIntrn.h"

#ifdef __CONFIG_TC__
extern void tc_asp(void);
extern void tc_form(void);
#endif

/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */

static char_t	*password = T("");	/* Security password */
static char_t	*homePage = T("index.asp");	/* replace your home page here*/
static int		port = 80;			/* Server port */
static int		retries = 5;		/* Server port retries */
extern int		finished;			/* Finished flag */
int HTTPDebugLevel = HTTP_DEBUG_OFF;

/****************************** Forward Declarations **************************/

static int 	initWebs();
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t* url, char_t* path, char_t* query);
extern void defaultErrorHandler(int etype, char_t *msg);
extern void defaultTraceHandler(int level, char_t *buf);
extern void asp_define();
extern void goform_define();
extern void wireless_asp_define(void);
extern void wireless_form_define(void);
extern void firewall_asp_define();
extern void 		LoadManagelist();
#ifdef B_STATS
#error WARNING:  B_STATS directive is not supported in this OS!
#endif

extern int webs_Tenda_CGI_BIN_Handler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						  char_t *url, char_t *path, char_t* query);
#if 0
extern int TWL300NWebsSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query);
#else
extern int TWL300NWebsSecurityByCookieHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query);
#endif

#ifdef __CONFIG_APCLIENT_DHCPC__
extern int gpi_get_apclient_dhcpc_enable_by_mib();
#endif

/*********************************** Code *************************************/
/*
 *	Main loop
 */
void httpd_mainloop(void)
{
/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */

	if(bopen(NULL, (60 * 1024), B_USE_MALLOC)<0)
		diag_printf("%s: bopen failed.\n",__FUNCTION__);

/*
 *	Initialize the web server
 */
	if (initWebs() < 0) {
		return;
	}

	LoadManagelist();
	
	HTTPDebugLevel=HTTP_DEBUG_OFF;
/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	while (!finished) {
		int resultReady = socketReady(-1);
		int resultSelect = socketSelect(-1, 2000);
		if (resultReady || resultSelect) {
			HTTP_DBGPRINT(HTTP_DEBUG_ERROR, "%s: Return socketReady()=%d, socketSelect()=%d\n", __FUNCTION__, resultReady, resultSelect);
			socketProcess(-1);
		}
		emfSchedProcess();
	}

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
	bclose();
}


void HTTP_debug_level(int level)
{
    if (level > 0)
        HTTPDebugLevel = HTTP_DEBUG_ERROR;
    else
        HTTPDebugLevel = HTTP_DEBUG_OFF;

    diag_printf("HTTP debug level = %d\n", HTTPDebugLevel);
}


/******************************************************************************/
/*
 *	Initialize the web server.
 */
static int initWebs()
{
	char		host[128];
	char		*cp;
	char_t		wbuf[128];

/*
 *	Initialize networking.
 */

	//init_all_network_interfaces();

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir("/");
#if 0
	cp = CFG_get_s(s_CFG_LAN_IPSTR);
#else
	extern struct ip_set primary_lan_ip_set;
#ifdef __CONFIG_APCLIENT_DHCPC__
	if(1 == gpi_get_apclient_dhcpc_enable_by_mib())
	{
		cp = "0.0.0.0"; 	
	}
	else
#endif
	{
		cp = inet_ntoa(*(struct in_addr *)&primary_lan_ip_set.ip);		
	}
#endif
	diag_printf("%s: lanip=[%s]\n",__FUNCTION__,cp);
	
	//ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	//websSetIpaddr(wbuf);
	websSetIpaddr(cp);
	//ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	//websSetHost(wbuf);
	websSetHost(cp);
/*
 *	Configure the web server options before opening the web server
 */

#if defined(__CONFIG_DEFAULT_PAGE__)
	websSetDefaultPage(__CONFIG_DEFAULT_PAGE__);
#else
	websSetDefaultPage(homePage);
#endif


	websSetPassword(password);

/* 
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */

	websUrlHandlerDefine(T(""), NULL, 0, TWL300NWebsSecurityByCookieHandler,WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0,webs_Tenda_CGI_BIN_Handler, 0);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, WEBS_HANDLER_LAST); 

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);
*/
	asp_define();
	goform_define();
	wireless_asp_define();
	wireless_form_define();
	wirelessPwrAspGoformDefine();
	wirelessCtlAspGoformDefine();
	firewall_asp_define();
#ifdef __CONFIG_TC__
	tc_form();
	tc_asp();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0); 
	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}

	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith")); 
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t* url, char_t* path, char_t* query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, websGetDefaultPage());
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Default error handler.  The developer should insert code to handle
 *	error messages in the desired manner.
 */

void defaultErrorHandler(int etype, char_t *msg)
{
	diag_printf(msg);
}

/******************************************************************************/
/*
 *	Trace log. Customize this function to log trace output
 */

void defaultTraceHandler(int level, char_t *buf)
{
/*
 *	The following code would write all trace regardless of level
 *	to stdout.
 */
#if 0
	if (buf) {
		write(1, buf, gstrlen(buf));
	}
#endif
}


