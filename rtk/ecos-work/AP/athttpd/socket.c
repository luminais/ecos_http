/* =================================================================
 *
 *      socket.c
 *
 *      Opens socket and starts the daemon.
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####                                     
 * -------------------------------------------                       
 * This file is part of eCos, the Embedded Configurable Operating System.
 * Copyright (C) 2005 Free Software Foundation, Inc.                 
 *
 * eCos is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 or (at your option) any later
 * version.                                                          
 *
 * eCos is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.                                                 
 *
 * You should have received a copy of the GNU General Public License 
 * along with eCos; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.     
 *
 * As a special exception, if other files instantiate templates or use
 * macros or inline functions from this file, or you compile this file
 * and link it with other works to produce a work based on this file,
 * this file does not by itself cause the resulting work to be covered by
 * the GNU General Public License. However the source code for this file
 * must still be made available in accordance with section (3) of the GNU
 * General Public License v2.                                        
 *
 * This exception does not invalidate any other reasons why a work based
 * on this file might be covered by the GNU General Public License.  
 * -------------------------------------------                       
 * ####ECOSGPLCOPYRIGHTEND####                                       
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    Anthony Tonizzo (atonizzo@gmail.com)
 *  Contributors: Sergei Gavrikov (w3sg@SoftHome.net), 
 *                Lars Povlsen    (lpovlsen@vitesse.com)
 *  Date:         2006-06-12
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/kernel/ktypes.h>         // base kernel types.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>                     // sprintf().
#include <time.h>                      // sprintf().

#include "http.h"
#include "socket.h"
#include "cgi.h"

/* HTTPS Support */
#ifdef SERVER_SSL
#include "syslog.h"
#include "certificate.h"
#include "privateKey.h"
#define SSL_KEYF                        "/etc/privateKey.key"
#define SSL_CERTF                       "/etc/certificate.crt"

int server_ssl;                         /*ssl socket */
SSL_CTX    *ctx;                        /*SSL context information*/
SSL_METHOD *meth;                       /*SSL method information*/

#endif /*end SERVER_SSL */

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define CYG_HTTPD_DAEMON_STACK_SIZE (CYGNUM_HAL_STACK_SIZE_MINIMUM + \
                                          CYGNUM_NET_ATHTTPD_THREADOPT_STACKSIZE)
static cyg_int32 cyg_httpd_initialized = 0;
#ifdef HAVE_SYSTEM_REINIT
static cyg_int32 cyg_httpd_cleaning =0;
#endif
cyg_thread   cyg_httpd_thread_object;
cyg_handle_t cyg_httpd_thread_handle;
cyg_uint8    cyg_httpd_thread_stack[CYG_HTTPD_DAEMON_STACK_SIZE]     
                                       __attribute__((__aligned__ (16)));
CYG_HTTPD_STATE httpstate;
                                         
__inline__ ssize_t
cyg_httpd_write(char* buf, int buf_len)
{
    // We are not going to write anything in case
#ifdef SERVER_SSL
    int do_ssl = httpstate.sockets[httpstate.client_index].do_ssl;
    ssize_t sent;
    if(do_ssl)
        sent = SSL_write(httpstate.sockets[httpstate.client_index].ssl,
            buf, 
            buf_len);
    else
        sent = send(httpstate.sockets[httpstate.client_index].descriptor, 
            buf, 
            buf_len,
            0);
#else
    ssize_t sent = send(httpstate.sockets[httpstate.client_index].descriptor, 
            buf, 
            buf_len,
            0);
#endif
    return sent;
}

__inline__ ssize_t
cyg_httpd_writev(cyg_iovec *iovec_bufs, int count)
{
    int i;
    ssize_t sent = writev(httpstate.sockets[httpstate.client_index].descriptor, 
                          iovec_bufs, 
                          count);
    ssize_t buf_len = 0;
    for (i = 0; i < count; i++)
        buf_len += iovec_bufs[i].iov_len;
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    if (sent != buf_len)
        diag_printf("writev() did not send out all bytes (%ld of %ld)\n", 
                    sent,
                    buf_len);
#endif    
    return sent;
}
    
// The need for chunked transfers arises from the fact that with persistent
//  connections it is not always easy to tell when a packet end. Also, with
//  dynamic pages it is not always possible to know the packet size upfront,
//  and thus the value of the 'Content-Length:' field in the header is not
//  known upfront.
// Today's web browser use 'Content-Length:' when present in the header and 
//  when not present they read everything that comes in up to the last 2 \r\n
//  and then figure it out. The HTTP standard _mandates_ 'Content-Length:' to
//  be present in the header with a correct value, and whenever that is not
//  possible, chunked transfers must be used.
//
// A chunked transer takes the form of:
// -----------------------------------------------------------------------------
//    cyg_httpd_start_chunked("html");
//    sprintf(phttpstate->payload, ...);             
//    cyg_httpd_write_chunked(phttpstate->payload, strlen(phttpstate->payload));
//    ...                         
//    cyg_httpd_end_chunked();
// -----------------------------------------------------------------------------
ssize_t
cyg_httpd_start_chunked(char *extension)
{
    httpstate.status_code = CYG_HTTPD_STATUS_OK;

#if defined(CYGOPT_NET_ATHTTPD_CLOSE_CHUNKED_CONNECTIONS)
    // I am not really sure that this is necessary, but even if it isn't, the
    //  added overhead is not such a big deal. In simple terms, I am not sure 
    //  how much I can rely on the client to understand that the frame has ended 
    //  with the last 5 bytes sent out. In an ideal world, the data '0\r\n\r\n'
    //  should be enough, but several posting on the subject I read seem to
    //  imply otherwise, at least with early generation browsers that supported
    //  the "Transfer-Encoding: chunked" mechanism. Things might be getting 
    //  better now but I snooped some sites that use the chunked stuff (Yahoo!
    //  for one) and all of them with no exception issue a "Connection: close" 
    //  on chunked frames even if there is nothing in the HTTP 1.1 spec that
    //  requires it.
    httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
#endif
    
    // We do not cache chunked frames. In case they are used to display dynamic
    //  data we want them to be executed every time they are requested.
    httpstate.mode |= 
              (CYG_HTTPD_MODE_TRANSFER_CHUNKED | CYG_HTTPD_MODE_NO_CACHE);
    
    httpstate.last_modified = -1;
    httpstate.mime_type = cyg_httpd_find_mime_string(extension);
    cyg_int32 header_length = cyg_httpd_format_header();
    return cyg_httpd_write(httpstate.outbuffer, header_length);
}

ssize_t
cyg_httpd_write_chunked(char* buf, int len)
{
    if (len == 0)
         return 0;

    char leader[16], trailer[] = {'\r', '\n'};
    cyg_iovec iovec_bufs[] = { {leader, 0}, {buf, len}, {trailer, 2} };
    iovec_bufs[0].iov_len = sprintf(leader, "%x\r\n", len);
    if (httpstate.mode & CYG_HTTPD_MODE_SEND_HEADER_ONLY)
        return (iovec_bufs[0].iov_len + len + 2);
#ifdef SERVER_SSL
    int do_ssl = httpstate.sockets[httpstate.client_index].do_ssl;
    if(do_ssl){
        int sent = 0;
        int i;
        for(i=0; i<3; i++){
            sent += cyg_httpd_write(iovec_bufs[i].iov_base, iovec_bufs[i].iov_len);
            if(sent<=0)
            {
                //diag_printf("[%s:%d] error!!!!!!!!!!!!!!!!!!!!!!!!i=%d, iov_len=%d, sent=%d\n", __FUNCTION__, __LINE__,i, iovec_bufs[i].iov_len, sent);
                break;
            }
        }
        return sent;
    }else
        return cyg_httpd_writev(iovec_bufs, 3);
#else
    return cyg_httpd_writev(iovec_bufs, 3);
#endif
}

void
cyg_httpd_end_chunked(void)
{
    httpstate.mode &= ~CYG_HTTPD_MODE_TRANSFER_CHUNKED;
    if ((httpstate.mode & CYG_HTTPD_MODE_SEND_HEADER_ONLY) != 0)
        return;
    strcpy(httpstate.outbuffer, "0\r\n\r\n");
    cyg_httpd_write(httpstate.outbuffer, 5);
}    

// This function builds and sends out a standard header. It is likely going to
//  be used by a c language callback function, and thus followed by one or
//  more calls to cyg_httpd_write(). Unlike cyg_httpd_start_chunked(), this
//  call requires prior knowledge of the final size of the frame (browsers
//  _will_trust_ the "Content-Length:" field when present!), and the user 
//  is expected to make sure that the total number of bytes (octets) sent out
//  via 'cyg_httpd_write()' matches the number passed in the len parameter.
// Its use is thus more limited, and the more flexible chunked frames should 
//  be used whenever possible.
void
cyg_httpd_create_std_header(char *extension, int len)
{
    httpstate.status_code = CYG_HTTPD_STATUS_OK;
    httpstate.mode |= CYG_HTTPD_MODE_NO_CACHE;

    // We do not want to send out a "Last-Modified:" field for c language
    //  callbacks.
    httpstate.last_modified = -1;
    httpstate.mime_type = cyg_httpd_find_mime_string(extension);
    httpstate.payload_len = len;
    cyg_int32 header_length = cyg_httpd_format_header();
    cyg_httpd_write(httpstate.outbuffer, header_length);
}

void
cyg_httpd_process_request(cyg_int32 index)
{
    httpstate.client_index = index;
    cyg_int32 descr = httpstate.sockets[index].descriptor;
#ifdef SERVER_SSL
    int do_ssl = httpstate.sockets[index].do_ssl;
#endif

    // By placing a terminating '\0' not only we have a safe stopper point
    //  for our parsing, but also we can detect if we have a split header.
    // Since headers always end with an extra '\r\n', if we find a '\0'
    //  before the terminator than we can safely assume that the header has
    //  not been received completely and more is following (i.e. split headers.)
    httpstate.inbuffer[0] = '\0';
    httpstate.inbuffer_len = 0;

    cyg_bool done = false;
    do
    {   
        // At this point we know we have data pending because the corresponding
        //  bit in the fd_set structure was set.
        int len;
#ifdef SERVER_SSL
        //diag_printf("[%s:%d] do_ssl=%d\n", __FUNCTION__, __LINE__, do_ssl);
        if(do_ssl){
            //diag_printf("[%s:%d] descriptor=%d, index=%d\n", __FUNCTION__, __LINE__, httpstate.sockets[index].descriptor, index);
            //sleep(2);
            if(httpstate.sockets[index].ssl==NULL){
                //diag_printf("[%s:%d] ====================ssl is NULL!====================\n", __FUNCTION__, __LINE__);
                continue;
            }
            len = SSL_read(httpstate.sockets[index].ssl,
                httpstate.inbuffer + httpstate.inbuffer_len,
                CYG_HTTPD_MAXINBUFFER - httpstate.inbuffer_len);
        }
        else
            len = recv(descr,
                httpstate.inbuffer + httpstate.inbuffer_len,
                CYG_HTTPD_MAXINBUFFER - httpstate.inbuffer_len,
                0);
#else
        len = recv(descr,
                httpstate.inbuffer + httpstate.inbuffer_len,
                CYG_HTTPD_MAXINBUFFER - httpstate.inbuffer_len,
                0);
#endif
        //diag_printf("[%s:%d]len=%d\n", __FUNCTION__, __LINE__, len);
        if (len == 0)
        {
            // This is the client that has closed its TX socket, possibly as
            //  a response from a shutdown() initiated by the server. Another
            //  possibility is that the client was closed altogether, in
            //  which case the client sent EOFs on each open sockets before 
            //  dying.
            close(descr);
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

            FD_CLR(descr, &httpstate.rfds);
            httpstate.sockets[index].descriptor = 0;
#ifdef SERVER_SSL
            if(NULL!=httpstate.sockets[index].ssl)
                SSL_free(httpstate.sockets[index].ssl);
            //httpstate.sockets[index].ssl = NULL;
#endif
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
            diag_printf("EOF received on descriptor: %d. Closing it.\n", descr);
#endif    
            return;
        }    
        
        if (len < 0)
        {
            // There was an error reading from this socket. Play it safe and
            //  close it. This will force the client to generate a shutdown
            //  and we will read a len = 0 the next time around.
           // perror(errno);
           //diag_printf("errno=%d %d\n",errno,__LINE__);
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
            diag_printf("ERROR reading from socket. read() returned: %d\n", 
                        httpstate.inbuffer_len);
#endif    
            close(descr);
            FD_CLR(descr, &httpstate.rfds);
            httpstate.sockets[index].descriptor = 0;
#ifdef SERVER_SSL
            if(NULL!=httpstate.sockets[index].ssl)
                SSL_free(httpstate.sockets[index].ssl);
            //httpstate.sockets[index].ssl = NULL;
#endif

            //shutdown(descr, SHUT_RDWR);
            return;
        }  

        httpstate.inbuffer[httpstate.inbuffer_len + len] = '\0';

        // It is always possible to receive split headers, in which case a
        //  header is only partially sent on one packet, with the rest on
        //  following packets. We can tell when a full packet is in the buffer
        //  by scanning for a header terminator ('\r\n\r\n'). Be smart and
        //  scan only the data received in the last read() operation, and not
        //  the full buffer each time.
        httpstate.request_end = 
               strstr(&httpstate.inbuffer[httpstate.inbuffer_len], "\r\n\r\n");
        httpstate.inbuffer_len += len;

        // Go through all the requests that were received in this packet.
        while (httpstate.request_end != 0)
        {
#if defined(HTTP_FILE_SERVER_SUPPORTED)
			http_file_server_req_init();
#endif
        
            httpstate.request_end += 4; // Include the terminator.
            
            // Timestamp the socket. 
            httpstate.sockets[index].timestamp = time(NULL);
                
            // This is where it all happens.
            cyg_httpd_process_method();
                
            if (httpstate.mode & CYG_HTTPD_MODE_CLOSE_CONN)
            {
                // There are 2 cases we can be here:
                // 1) chunked frames close their connection by default
                // 2) The client requested the connection be terminated with a
                //     "Connection: close" in the header
                // In any case, we close the TX pipe and wait for the client to
                //  send us an EOF on the receive pipe. This is a more graceful
                //  way to handle the closing of the socket, compared to just
                //  calling close() without first asking the opinion of the
                //  client, and  running the risk of stray data lingering 
                //  around.
                shutdown(descr, SHUT_WR);
            }
            
            // Move back the next request (if any) to the beginning of inbuffer.
            //  This way we avoid inching towards the end of inbuffer with
            //  consecutive requests.
            strcpy(httpstate.inbuffer, httpstate.request_end);
            httpstate.inbuffer_len -= (int)(httpstate.request_end - 
                                                       httpstate.inbuffer);

#if defined(HTTP_FILE_SERVER_SUPPORTED)
			http_file_server_req_free();
#endif
                                                       
            // If there is no data left over we are done processing all
            //  requests.
            if (httpstate.inbuffer_len == 0)
            {
                done = true;
                break;
            }    

            // Any other fully formed request pending?                                           
            httpstate.request_end = strstr(httpstate.inbuffer, "\r\n\r\n");
           
        }        
    }
    while (done == false);
}

#ifdef SERVER_SSL
    int
cyg_httpd_handle_new_connection(cyg_int32 listener, int do_ssl)
#else
    void
cyg_httpd_handle_new_connection(cyg_int32 listener)
#endif
{
    cyg_int32 i;
    struct	 timeval tv;
	cyg_int32 rc=0;

    int fd_client = accept(listener, NULL, NULL);
    CYG_ASSERT(listener != -1, "accept() failed");
    if (fd_client == -1) 
#ifdef SERVER_SSL
        return -1;
#else
        return;
#endif

    //diag_printf("[%s:%d]fd_client=%d\n", __FUNCTION__, __LINE__, fd_client);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    rc = setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (rc == -1) {
        close(fd_client);
        diag_printf("%s %d setsockopt failed\n",__FUNCTION__,__LINE__);
#ifdef ECOS_DBG_STAT
	   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
#ifdef SERVER_SSL
        return -1;
#else
        return;
#endif
    }


   rc = setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
   if (rc == -1) {
	   close(fd_client);
	   diag_printf("%s %d setsockopt failed\n",__FUNCTION__,__LINE__);
#ifdef ECOS_DBG_STAT
	   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
#ifdef SERVER_SSL
        return -1;
#else
        return;
#endif
    }

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

#ifdef SERVER_SSL
    SSL *ssl;
    if(do_ssl){
        // inital SSL
        ssl = SSL_new(ctx);
        if(ssl == NULL){
            //diag_printf("!!!!!!!!!!Couldn't create ssl connection!!!!!!!!!!!!\n");
            return ;
        }    
        SSL_set_fd(ssl, fd_client);
        if(SSL_accept(ssl) <= 0){  
            ERR_print_errors_fp(stderr);
            //diag_printf("!!!!!!!!!!Couldn't SSL_accept!!!!!!!!!!!\n");
            return;
        }
        else{
            //diag_printf("[%s:%d]SSL_ACCEPTED\n", __FUNCTION__, __LINE__);
        }
    }
#endif

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Opening descriptor: %d\n", fd_client);
#endif    
    // Timestamp the socket and process the frame immediately, since the accept
    //  guarantees the presence of valid data on the newly opened socket.
    for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i++)
        if (httpstate.sockets[i].descriptor == 0)
        {
            httpstate.sockets[i].descriptor = fd_client;
            httpstate.sockets[i].timestamp  = time(NULL);
            //diag_printf("[%s:%d] descriptor=%d\n", __FUNCTION__, __LINE__, httpstate.sockets[i].descriptor);
#ifdef SERVER_SSL
            if(do_ssl){
                httpstate.sockets[i].ssl  = ssl;
                httpstate.sockets[i].do_ssl  = 1;
            }else{
                httpstate.sockets[i].ssl  = NULL;
                httpstate.sockets[i].do_ssl  = 0;
            }
#endif
            cyg_httpd_process_request(i);
#ifdef SERVER_SSL
            return fd_client;
#else
            return;
#endif
        }    
}

#ifdef SERVER_SSL
    void
cyg_httpd_handle_new_ssl_connection(cyg_int32 listener, int do_ssl)
{
    cyg_httpd_handle_new_connection(listener, do_ssl);
}
#endif

// This is the "garbage collector" (or better, the "garbage disposer") of
//  the server. It closes any socket that has been idle for a time period
//  of CYG_HTTPD_SELECT_TIMEOUT seconds.
void
cyg_httpd_close_unused_sockets(cyg_int32 listener)
{
    cyg_int32 i;
    
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Garbage collector called\r\n");
#endif    
    httpstate.fdmax = listener;
    for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i++)
    {
        if (httpstate.sockets[i].descriptor != 0)
        {
            if (time(NULL) - httpstate.sockets[i].timestamp > 
                                          CYG_HTTPD_SOCKET_IDLE_TIMEOUT)
            {           
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
                diag_printf("Closing descriptor: %d\n", 
                            httpstate.sockets[i].descriptor);
#endif    
                shutdown(httpstate.sockets[i].descriptor, SHUT_WR);
#ifdef SERVER_SSL
                if(NULL!=httpstate.sockets[i].ssl)
                    SSL_free(httpstate.sockets[i].ssl);
                httpstate.sockets[i].ssl = NULL;
#endif
            }
            else
                httpstate.fdmax = MAX(httpstate.fdmax, 
                                      httpstate.sockets[i].descriptor);
        }                              
    }
}

void
cyg_httpd_daemon(cyg_addrword_t data)
{
    cyg_int32 rc=0;
    cyg_int32 listener=0;
	struct timeval tv = {1, 0};
	int time_passed=0;
    init_all_network_interfaces();

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up)
    {
        struct bootp* bps = &eth0_bootp_data;
        diag_printf("ETH0 is up. IP address: %s\n", inet_ntoa(bps->bp_yiaddr));
    }
#endif
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_CGIBIN_TCL
    cyg_httpd_init_tcl_interpreter();
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Tcl interpreter has been initialized...\n");
#endif
#endif    

#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
    cyg_httpd_auth_init();
#endif
    cyg_httpd_initialize();

    // Get the network going. This is benign if the application has
    //  already done this.
#ifdef SERVER_SSL
    /* With HTTPS Support*/
    int do_ssl = -1;
    rc = InitSSLStuff();
    if(rc!=1){
        // failed
        diag_printf("[%s:%d] initial HTTPS failed!!!\n", __FUNCTION__, __LINE__);
        return ;
    }
#endif

    /* HTTP Support */
#ifdef ATHTTPD_IPV6
    listener = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
#else
    listener = socket(AF_INET, SOCK_STREAM, 0);
#endif
    CYG_ASSERT(listener > 0, "Socket create failed");
    if (listener < 0) {
        return;
    }
#ifdef ECOS_DBG_STAT
	   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    cyg_int32 yes = 1;
    rc = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1) {
	close(listener);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
        return;
    }

#ifdef ATHTTPD_IPV6
    memset(&(httpstate.server_conn), 0, sizeof(struct sockaddr_in6));
    httpstate.server_conn.sin6_family = AF_INET6;
    httpstate.server_conn.sin6_len = sizeof(struct sockaddr_in6);
    httpstate.server_conn.sin6_addr = in6addr_any;
    httpstate.server_conn.sin6_port = htons(CYGNUM_NET_ATHTTPD_SERVEROPT_PORT);
    rc = bind(listener,
              (struct sockaddr *)&httpstate.server_conn, 
              		sizeof(struct sockaddr_in6));
#else
    memset(&(httpstate.server_conn), 0, sizeof(struct sockaddr_in));
    httpstate.server_conn.sin_family = AF_INET;
    httpstate.server_conn.sin_addr.s_addr = INADDR_ANY;
    httpstate.server_conn.sin_port = htons(CYGNUM_NET_ATHTTPD_SERVEROPT_PORT);
    rc = bind(listener,
              (struct sockaddr *)&httpstate.server_conn, 
              sizeof(struct sockaddr)); 
#endif
    CYG_ASSERT(rc == 0, "bind() returned error");
    if (rc != 0) {
	close(listener);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
        return;
    }

    rc = listen(listener, SOMAXCONN);
    CYG_ASSERT(rc == 0, "listen() returned error");
    if (rc != 0) {
	close(listener);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
        return;
    }

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Web server Started and listening...\n");
#endif
    cyg_int32 i;
    for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i++)
    {
        httpstate.sockets[i].descriptor  = 0;
        httpstate.sockets[i].timestamp   = (time_t)0;
#ifdef SERVER_SSL
        httpstate.sockets[i].ssl   = NULL;
#endif
    }

    FD_ZERO(&httpstate.rfds);
#ifdef SERVER_SSL
    httpstate.fdmax = MAX(listener, server_ssl);
#else
    httpstate.fdmax = MAX(listener, httpstate.fdmax);
#endif

    //diag_printf("[%s:%d] socket setup succeed. \n", __FUNCTION__, __LINE__);
    //diag_printf("[%s:%d] listener=%d\n", __FUNCTION__, __LINE__, listener);
    //diag_printf("[%s:%d] server_ssl=%d\n", __FUNCTION__, __LINE__, server_ssl);
    while (1)
    {    
#ifdef HAVE_SYSTEM_REINIT
		if(cyg_httpd_cleaning)
		{
			break;
		}
#endif
        // The listener is always added to the select() sensitivity list.
        FD_SET(listener, &httpstate.rfds); 
#ifdef SERVER_SSL
        FD_SET(server_ssl, &httpstate.rfds); 
#endif
        //struct timeval tv = {CYG_HTTPD_SOCKET_IDLE_TIMEOUT, 0};
        
        rc = select(httpstate.fdmax + 1, &httpstate.rfds, NULL, NULL, &tv);
        if (rc > 0)
        {
            if (FD_ISSET(listener, &httpstate.rfds)){
                // If the request is from the listener socket, then 
                //  this must be a new connection.
#ifdef SERVER_SSL
                do_ssl=0;
                cyg_httpd_handle_new_connection(listener, do_ssl);
#else
                cyg_httpd_handle_new_connection(listener);
#endif
            }

#ifdef SERVER_SSL
            if (FD_ISSET(server_ssl, &httpstate.rfds)){
                do_ssl=1;
                cyg_httpd_handle_new_ssl_connection(server_ssl, do_ssl);
            }
#endif

#ifdef SERVER_SSL
            httpstate.fdmax = MAX(server_ssl, listener);
#else
            httpstate.fdmax = MAX(listener, httpstate.fdmax);
#endif
            //diag_printf("[%s:%d]server_ssl=%d, listener=%d, fdmax=%d\n", __FUNCTION__, __LINE__, server_ssl, listener, httpstate.fdmax);

            // The sensitivity list returned by select() can have multiple
            //  socket descriptors that need service. Loop through the whole
            //  descriptor list to see if one or more need to be served.
            for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i ++)
            {
                cyg_int32 descr = httpstate.sockets[i].descriptor;
                if (descr != 0)
                {
                    // If the descriptor is set in the descriptor list, we
                    //  service it. Otherwise, we add it to the descriptor list
                    //  to listen for. The rfds list gets rewritten each time
                    //  select() is called and after the call it contains only
                    //  the descriptors that need be serviced. Before calling
                    //  select() again we must repopulate the list with all the
                    //  descriptors that must be listened for.
                    if (FD_ISSET(descr, &httpstate.rfds)){
                        cyg_httpd_process_request(i);
                    }
                    else{
                        FD_SET(descr, &httpstate.rfds); 
                    }
                    if (httpstate.sockets[i].descriptor != 0)
                        httpstate.fdmax = MAX(httpstate.fdmax, descr);
                }
            }
        }
        else if (rc == 0)
        {
        	time_passed++;
			if(time_passed>=CYG_HTTPD_SOCKET_IDLE_TIMEOUT)
			{
            	cyg_httpd_close_unused_sockets(listener);
				time_passed=0;
			}
        }
        else
        {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
            cyg_int8 *ptr = (cyg_int8*)&httpstate.rfds;
            diag_printf("rfds: %x %x %x %x\n", ptr[0], ptr[1], ptr[2], ptr[3] );
            for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i++)
                if (httpstate.sockets[i].descriptor != 0)
                     diag_printf("Socket in list: %d\n", 
                                 httpstate.sockets[i].descriptor);
#endif                                 
            CYG_ASSERT(rc != -1, "Error during select()");                 
        }
    }
	
	close(listener);
	for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i++)
    {
        if (httpstate.sockets[i].descriptor != 0)
        {
        	 shutdown(httpstate.sockets[i].descriptor, SHUT_WR);
        }
	}
#ifdef HAVE_SYSTEM_REINIT
	cyg_httpd_cleaning=0;
#endif
}

void
cyg_httpd_start(void)
{
    if (cyg_httpd_initialized)
        return;
    cyg_httpd_initialized = 1;
    
    cyg_thread_create(CYGNUM_NET_ATHTTPD_THREADOPT_PRIORITY,
                      cyg_httpd_daemon,
                      (cyg_addrword_t)0,
                      "HTTPD Thread",
                      (void *)cyg_httpd_thread_stack,
                      CYG_HTTPD_DAEMON_STACK_SIZE,
                      &cyg_httpd_thread_handle,
                      &cyg_httpd_thread_object);
    cyg_thread_resume(cyg_httpd_thread_handle);
}                       
#ifdef HAVE_SYSTEM_REINIT
void clean_httpd()
{
	if(cyg_httpd_initialized)
	{
		cyg_httpd_cleaning=1;
		while(cyg_httpd_cleaning){
			cyg_thread_delay(20);
		}
		cyg_httpd_initialized=0;
		bzero(&httpstate,sizeof(httpstate));
		cyg_thread_kill(cyg_httpd_thread_handle);
		cyg_thread_delete(cyg_httpd_thread_handle);
	}
}
#endif

#ifdef SERVER_SSL
    int
InitSSLStuff(void)
{
    int sock_opt = 1;

    /* write certificate and private key to file */
    FILE *fd_key = fopen(SSL_KEYF, "w+");
    if(0==fwrite(privateKey, strlen(privateKey), 1, fd_key)){
        diag_printf("[%s:%d] key file write failed!!!\n", __FUNCTION__, __LINE__);
        return 0;
    }else
        fclose(fd_key);

    FILE *fd_cert = fopen(SSL_CERTF, "w+");
    if(0==fwrite(certificate, strlen(certificate), 1, fd_cert)){
        diag_printf("[%s:%d] certificate file write failed!!!\n", __FUNCTION__, __LINE__);
        return 0;
    }else
        fclose(fd_cert);

    sleep(3);
    syslog(LOG_NOTICE, "Enabling SSL security system");
#ifdef ATHTTPD_IPV6
    if ((server_ssl = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        //die(NO_CREATE_SOCKET);
        diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        return 0;
    }
#else
    if ((server_ssl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        syslog(LOG_ALERT,"Couldn't create socket for ssl");
        diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        //die(NO_CREATE_SOCKET);
        return 0;
    }
#endif 

    /* server socket is nonblocking */
#if 0
    if (fcntl(server_ssl, F_SETFL, flags | O_NONBLOCK) == -1){
        syslog(LOG_ALERT, "%s, %i:Couldn't fcntl", __FILE__, __LINE__);
        diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        //die(NO_FCNTL);
        return 0;
    }
#endif

    if ((setsockopt(server_ssl, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
                    sizeof(sock_opt))) == -1){
        syslog(LOG_ALERT,"%s, %i:Couldn't sockopt", __FILE__,__LINE__);
        diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        //die(NO_SETSOCKOPT);
        return 0;
    }

    /* internet socket */
    memset(&(httpstate.server_conn), 0, sizeof(struct sockaddr_in));
#ifdef ATHTTPD_IPV6
    httpstate.server_conn.sin6_family = AF_INET6;
    httpstate.server_conn.sin6_len = sizeof(struct sockaddr_in6);
    memcpy(&httpstate.server_conn.sin6_addr,&in6addr_any,sizeof(in6addr_any));
    httpstate.server_conn.sin6_port = htons(443);
#else
    httpstate.server_conn.sin_family = AF_INET;
    httpstate.server_conn.sin_addr.s_addr = INADDR_ANY;
    httpstate.server_conn.sin_port = htons(443);
#endif

    if (bind(server_ssl, (struct sockaddr *) &(httpstate.server_conn),
                sizeof(httpstate.server_conn)) == -1){
        syslog(LOG_ALERT, "Couldn't bind ssl to port %d", ntohs(server_sockaddr.sin_port));
        //die(NO_BIND);
        diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        return 0;
    }

    /* listen: large number just in case your kernel is nicely tweaked */
    if (listen(server_ssl, SOMAXCONN) == -1){
        //die(NO_LISTEN);
        diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        return 0;       
    }

    //if (server_ssl > httpstate.fdmax)
    //    httpstate.fdmax = server_ssl;

    /*Init all of the ssl stuff*/
    //  i don't know why this line is commented out... i found it like that - damion may-02 
    /*  SSL_load_error_strings();*/
    SSLeay_add_ssl_algorithms();
    meth = SSLv23_server_method();
    if(meth == NULL){
        ERR_print_errors_fp(stderr);
        syslog(LOG_ALERT, "Couldn't create the SSL method");
        //diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        //die(NO_SSL);
        return 0;
    }
    ctx = SSL_CTX_new(meth);
    if(!ctx){
        syslog(LOG_ALERT, "Couldn't create a connection context\n");
        ERR_print_errors_fp(stderr);
        //diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        //die(NO_SSL);
        return 0;
    }

    if (SSL_CTX_use_certificate_file(ctx, SSL_CERTF, SSL_FILETYPE_PEM) <= 0) {
        syslog(LOG_ALERT, "Failure reading SSL certificate file: %s",SSL_CERTF);fflush(NULL);
        //diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        close(server_ssl);
        return 0;
    }
    syslog(LOG_DEBUG, "Loaded SSL certificate file: %s",SSL_CERTF);fflush(NULL);

    if (SSL_CTX_use_PrivateKey_file(ctx, SSL_KEYF, SSL_FILETYPE_PEM) <= 0) {
        syslog(LOG_ALERT, "Failure reading private key file: %s",SSL_KEYF);fflush(NULL);
        //diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        close(server_ssl);
        return 0;
    }
    syslog(LOG_DEBUG, "Opened private key file: %s",SSL_KEYF);fflush(NULL);

    if (!SSL_CTX_check_private_key(ctx)) {
        syslog(LOG_ALERT, "Private key does not match the certificate public key");fflush(NULL);
        //diag_printf("[%s:%d] failed!!!\n", __FUNCTION__, __LINE__);
        close(server_ssl);
        return 0;
    }

    /*load and check that the key files are appropriate.*/
    syslog(LOG_NOTICE,"SSL security system enabled");
    return 1;
}
#endif /*SERVER_SSL*/


