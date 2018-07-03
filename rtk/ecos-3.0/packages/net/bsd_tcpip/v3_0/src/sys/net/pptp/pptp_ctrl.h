/* pptp_ctrl.h ... handle PPTP control connection.
 *                 C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp_ctrl.h,v 1.6 2008/02/19 05:05:03 quozl Exp $
 */

#ifndef INC_PPTP_CTRL_H
#define INC_PPTP_CTRL_H
#include <sys/types.h>
//#include "pptp_compat.h"

typedef struct PPTP_CONN PPTP_CONN;
typedef struct PPTP_CALL PPTP_CALL;

enum call_state { CALL_OPEN_RQST,  CALL_OPEN_DONE, CALL_OPEN_FAIL,
		  CALL_CLOSE_RQST, CALL_CLOSE_DONE };
enum conn_state { CONN_OPEN_RQST,  CONN_OPEN_DONE, CONN_OPEN_FAIL,
		  CONN_CLOSE_RQST, CONN_CLOSE_DONE };

typedef void (*pptp_call_cb)(PPTP_CONN*, PPTP_CALL*, enum call_state);
typedef void (*pptp_conn_cb)(PPTP_CONN*, enum conn_state);

struct PPTP_CONN {
    int inet_sock;
    /* Connection States */
    enum { 
        CONN_IDLE, CONN_WAIT_CTL_REPLY, CONN_WAIT_STOP_REPLY, CONN_ESTABLISHED 
    } conn_state; /* on startup: CONN_IDLE */
    /* Keep-alive states */
    enum { 
        KA_NONE, KA_OUTSTANDING 
    } ka_state;  /* on startup: KA_NONE */
    /* Keep-alive ID; monotonically increasing (watch wrap-around!) */
    u_int32_t ka_id; /* on startup: 1 */
    /* Other properties. */
    u_int16_t version;
    u_int16_t firmware_rev;
    u_int8_t  hostname[64], vendor[64];
    /* XXX these are only PNS properties, currently XXX */
    /* Call assignment information. */
    u_int16_t call_serial_number;
    void * closure;
    pptp_conn_cb callback;
    /******* IO buffers ******/
    char * read_buffer, *write_buffer;
    size_t read_alloc,   write_alloc;
    size_t read_size,    write_size;
};

/* if 'isclient' is true, then will send 'conn open' packet to other host.
 * not necessary if this is being opened by a server process after
 * receiving a conn_open packet from client. 
 */
PPTP_CONN * pptp_conn_open(int inet_sock, int isclient, 
			   pptp_conn_cb callback);
PPTP_CALL * pptp_call_open(PPTP_CONN * conn, 
			   pptp_call_cb callback, char *phonenr);
/* soft close.  Will callback on completion. */
void pptp_conn_close(PPTP_CONN * conn, u_int8_t close_reason);
/* hard close */
void pptp_conn_destroy(PPTP_CONN * conn);

#endif /* INC_PPTP_CTRL_H */
