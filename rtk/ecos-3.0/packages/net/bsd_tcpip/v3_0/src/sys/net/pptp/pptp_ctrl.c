

#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "pptp_msg.h"
#include "pptp_ctrl.h"
#include "pptp_options.h"

#if 0
#define	M_DONTWAIT	1
#define	M_WAIT		0
#define M_TEMP          94 
#define Malloc(x,y) malloc((y),M_TEMP,M_DONTWAIT)
#define Freee(x)	free((x),M_TEMP)
#endif

/* control the number of times echo packets will be logged */
static int nlogecho = 10;

static struct thread_specific {
    struct sigaction old_sigaction; /* evil signals */
    PPTP_CONN * conn;
} global;

#define INITIAL_BUFSIZE 512 /* initial i/o buffer size. */


struct PPTP_CALL {
    /* Call properties */
    enum {
        PPTP_CALL_PAC, PPTP_CALL_PNS
    } call_type;
    union { 
        enum pptp_pac_state {
            PAC_IDLE, PAC_WAIT_REPLY, PAC_ESTABLISHED, PAC_WAIT_CS_ANS
        } pac;
        enum pptp_pns_state {
            PNS_IDLE, PNS_WAIT_REPLY, PNS_ESTABLISHED, PNS_WAIT_DISCONNECT 
        } pns;
    } state;
    u_int16_t call_id, peer_call_id;
    u_int16_t sernum;
    u_int32_t speed;
    /* For user data: */
    pptp_call_cb callback;
    void * closure;
};


/* (General Error Codes) */
static const struct {
    const char *name, *desc;
} pptp_general_errors[] = {
#define PPTP_GENERAL_ERROR_NONE                 0
    { "(None)", "No general error" },
#define PPTP_GENERAL_ERROR_NOT_CONNECTED        1
    { "(Not-Connected)", "No control connection exists yet for this "
        "PAC-PNS pair" },
#define PPTP_GENERAL_ERROR_BAD_FORMAT           2
    { "(Bad-Format)", "Length is wrong or Magic Cookie value is incorrect" },
#define PPTP_GENERAL_ERROR_BAD_VALUE            3
    { "(Bad-Value)", "One of the field values was out of range or "
            "reserved field was non-zero" },
#define PPTP_GENERAL_ERROR_NO_RESOURCE          4
    { "(No-Resource)", "Insufficient resources to handle this command now" },
#define PPTP_GENERAL_ERROR_BAD_CALLID           5
    { "(Bad-Call ID)", "The Call ID is invalid in this context" },
#define PPTP_GENERAL_ERROR_PAC_ERROR            6
    { "(PAC-Error)", "A generic vendor-specific error occured in the PAC" }
};

#define  MAX_GENERAL_ERROR ( sizeof(pptp_general_errors) / \
        sizeof(pptp_general_errors[0]) - 1)

/* Outgoing Call Reply Result Codes */
static const char *pptp_out_call_reply_result[] = {
/* 0 */	"Unknown Result Code",
/* 1 */	"Connected",
/* 2 */	"General Error",
/* 3 */	"No Carrier Detected",
/* 4 */	"Busy Signal",
/* 5 */	"No Dial Tone",
/* 6 */	"Time Out",
/* 7 */	"Not Accepted, Call is administratively prohibited" };

#define MAX_OUT_CALL_REPLY_RESULT 7

/* Call Disconnect Notify  Result Codes */
static const char *pptp_call_disc_ntfy[] = {
/* 0 */	"Unknown Result Code",
/* 1 */	"Lost Carrier",
/* 2 */	"General Error",
/* 3 */	"Administrative Shutdown",
/* 4 */	"(your) Request" };

#define MAX_CALL_DISC_NTFY 4

/* Call Disconnect Notify  Result Codes */
static const char *pptp_start_ctrl_conn_rply[] = {
/* 0 */	"Unknown Result Code",
/* 1 */	"Successful Channel Establishment",
/* 2 */	"General Error",
/* 3 */	"Command Channel Already Exists",
/* 4 */	"Requester is not Authorized" };

#define MAX_START_CTRL_CONN_REPLY 4

/* timing options */
int idle_wait = PPTP_TIMEOUT;
int max_echo_wait = PPTP_TIMEOUT;

/* Local prototypes */
//static void pptp_reset_timer(void);
//static void pptp_handle_timer();
/* Write/read as much as we can without blocking. */
int pptp_write_some(PPTP_CONN * conn);
//int pptp_read_some(PPTP_CONN * conn);
/* Make valid packets from read_buffer */
int pptp_check_packet(PPTP_CONN * conn, void **buf, size_t *size);
/* Add packet to write_buffer */
int pptp_send_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size);
/* Dispatch packets (general) */
int pptp_dispatch_packet(PPTP_CONN * conn, void * buffer, size_t size);
/* Dispatch packets (control messages) */
int ctrlp_disp(PPTP_CONN * conn, void * buffer, size_t size);
/* Set link info, for pptp servers that need it.
   this is a noop, unless the user specified a quirk and
   there's a set_link hook defined in the quirks table
   for that quirk */
//void pptp_set_link(PPTP_CONN * conn, int peer_call_id);

extern struct PPTP_CONN * conn;

void CheckState()
{
	extern PPTP_CONN * conn;
	if( conn->conn_state == CONN_IDLE ||  conn->conn_state == CONN_WAIT_CTL_REPLY)
	{
		struct pptp_start_ctrl_conn packet = {
		    PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
		    hton16(PPTP_VERSION), 0, 0, 
		    hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
		    hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
		    PPTP_HOSTNAME, PPTP_VENDOR
		};
		if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet)))
		    conn->conn_state = CONN_WAIT_CTL_REPLY;        
	}
}

static const char *ctrl_msg_types[] = {
         "invalid control message type",
/*         (Control Connection Management) */
         "Start-Control-Connection-Request",            /* 1 */
         "Start-Control-Connection-Reply",              /* 2 */
         "Stop-Control-Connection-Request",             /* 3 */
         "Stop-Control-Connection-Reply",               /* 4 */
         "Echo-Request",                                /* 5 */
         "Echo-Reply",                                  /* 6 */
/*         (Call Management) */
         "Outgoing-Call-Request",                       /* 7 */
         "Outgoing-Call-Reply",                         /* 8 */
         "Incoming-Call-Request",                       /* 9 */
         "Incoming-Call-Reply",                        /* 10 */
         "Incoming-Call-Connected",                    /* 11 */
         "Call-Clear-Request",                         /* 12 */
         "Call-Disconnect-Notify",                     /* 13 */
/*         (Error Reporting) */
         "WAN-Error-Notify",                           /* 14 */
/*         (PPP Session Control) */
         "Set-Link-Info"                              /* 15 */
};
#define MAX_CTRLMSG_TYPE 15





void send_start_conn()
{
	struct pptp_start_ctrl_conn packet = {
		PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
		hton16(PPTP_VERSION), 0, 0, 
		hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
		hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
		PPTP_HOSTNAME, PPTP_VENDOR
	};
	pptp_send_ctrl_packet(conn, &packet, sizeof(packet));
}
/*** report a sent packet ****************************************************/
static void ctrlp_rep( void * buffer, int size, int isbuff)
{
    struct pptp_header *packet = buffer;
    unsigned int type;
    if(size < sizeof(struct pptp_header)) return;
    type = ntoh16(packet->ctrl_type);
    /* FIXME: do not report sending echo requests as long as they are
     * sent in a signal handler. This may dead lock as the syslog call
     * is not reentrant */
    if( type ==  PPTP_ECHO_RQST ) return;
    /* don't keep reporting sending of echo's */
    if( (type == PPTP_ECHO_RQST || type == PPTP_ECHO_RPLY) && nlogecho <= 0 ) return;
#if 0
	diag_printf("%s control packet type is %d '%s'\n",isbuff ? "Buffered" : "Sent", 
            type, ctrl_msg_types[type <= MAX_CTRLMSG_TYPE ? type : 0]);
#endif
}    

/* Open new pptp_connection. start pptp connect.......*/
PPTP_CONN * pptp_conn_open(int inet_sock, int isclient, pptp_conn_cb callback)
{
    PPTP_CONN *conn;
	
    if ((conn = malloc(sizeof(*conn))) == NULL) return NULL;

    conn->inet_sock = inet_sock;
    conn->conn_state = CONN_IDLE;
    conn->ka_state  = KA_NONE;
    conn->ka_id     = 1;
    conn->call_serial_number = 0;
    conn->callback  = callback;
    conn->read_size = conn->write_size = 0;
    conn->read_alloc = conn->write_alloc = INITIAL_BUFSIZE;
    conn->read_buffer =
        malloc(sizeof(*(conn->read_buffer)) * conn->read_alloc);
    conn->write_buffer =
        malloc(sizeof(*(conn->write_buffer)) * conn->write_alloc);
    if (conn->read_buffer == NULL || conn->write_buffer == NULL) {
        if (conn->read_buffer  != NULL) free(conn->read_buffer);
        if (conn->write_buffer != NULL) free(conn->write_buffer);
		free(conn); 
		return NULL;
    }
    /* non-blocking. */
    fcntl(conn->inet_sock, F_SETFL, O_NONBLOCK);

    if (isclient) {
        struct pptp_start_ctrl_conn packet = {
            PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
            hton16(PPTP_VERSION), 0, 0, 
            hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
            hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
            PPTP_HOSTNAME, PPTP_VENDOR
        };

        if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet)))
            conn->conn_state = CONN_WAIT_CTL_REPLY;
    }
    //global.conn = conn;
    return conn;
}

/**************This is client call requests****************/
PPTP_CALL * pptp_call_open(PPTP_CONN * conn, pptp_call_cb callback,
        char *phonenr)
{
    PPTP_CALL * call;
    int i = -1;
    int idx, rc;
	
    /* Send off the call request */
    struct pptp_out_call_rqst packet = {
        PPTP_HEADER_CTRL(PPTP_OUT_CALL_RQST),
        0,0, /*call_id, sernum */
        hton32(PPTP_BPS_MIN), hton32(PPTP_BPS_MAX),
        hton32(PPTP_BEARER_CAP), hton32(PPTP_FRAME_CAP), 
        hton16(PPTP_WINDOW), 0, 0, 0, {0}, {0}
    };
    assert(conn->conn_state == CONN_ESTABLISHED);
	
    if ((call = malloc(sizeof(*call))) == NULL) return NULL;
    call->call_type = PPTP_CALL_PNS;
    call->state.pns = PNS_IDLE;
    call->call_id   = (u_int16_t) i;
    call->sernum    = conn->call_serial_number++;
    call->callback  = callback;
    call->closure   = NULL;
    packet.call_id = htons(call->call_id);
    packet.call_sernum = htons(call->sernum);

    if (phonenr) {
        strncpy((char *)packet.phone_num, phonenr, sizeof(packet.phone_num));
        packet.phone_len = strlen(phonenr);
        if( packet.phone_len > sizeof(packet.phone_num))
            packet.phone_len = sizeof(packet.phone_num);
        packet.phone_len = hton16 (packet.phone_len);
    }
    if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet))) {
        call->state.pns = PNS_WAIT_REPLY;
        return call;
    } else {
        free(call);
        return NULL;
    }
}
void pptp_sessoin_close(PPTP_CONN * conn)
{
	extern u_int16_t pptp_gre_call_id, pptp_gre_peer_call_id;

    struct pptp_call_clear_rqst rqst = {
        PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_RQST), 0, 0
    };

    rqst.call_id = hton16(pptp_gre_call_id);
    pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
}

/*** Need to clear PPTP CONN INFO ************************/
void pptp_conn_close(PPTP_CONN * conn, u_int8_t close_reason)
{
    struct pptp_stop_ctrl_conn rqst = {
        PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RQST), 
        hton8(close_reason), 0, 0
    };	
    if (conn->conn_state == CONN_IDLE || conn->conn_state == CONN_WAIT_STOP_REPLY) 
        return;
	
    diag_printf("Closing PPTP connection");
    pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
    conn->conn_state = CONN_WAIT_STOP_REPLY;
    return;
}

/*** this is a hard close *****************************************************/
void pptp_conn_destroy(PPTP_CONN * conn)
{
	if(!conn)	return ;
    if(conn->inet_sock >= 0 ) {close(conn->inet_sock);conn->inet_sock = -1;}
	if(conn->read_buffer)	{free(conn->read_buffer); conn->read_buffer =NULL;}
	if(conn->write_buffer)	{free(conn->write_buffer);conn->write_buffer = NULL;}
    if(conn!= NULL)  {free(conn);conn = NULL;}
}

/*** Non-blocking write *******************************************************/
int pptp_write_some(PPTP_CONN * conn) {
    ssize_t retval;
    retval = write(conn->inet_sock, conn->write_buffer, conn->write_size);
    if (retval < 0) { /* error. */
        if (errno == EAGAIN || errno == EINTR) { 
            return 0;
        } else { /* a real error */
            diag_printf("write error: %s", strerror(errno));
	    return -1;
        }
    }
    assert(retval <= conn->write_size);
    conn->write_size -= retval;
    memmove(conn->write_buffer, conn->write_buffer + retval, conn->write_size);
    ctrlp_rep(conn->write_buffer, retval, 0);
    return 0;
}

/*** Non-blocking read ********************************************************/
int pptp_read_some(int inet_sock,PPTP_CONN * conn)
{
    ssize_t retval;
	
    if (conn->read_size == conn->read_alloc) {
        char *new_buffer = realloc(conn->read_buffer, 
                sizeof(*(conn->read_buffer)) * conn->read_alloc * 2);
        if (new_buffer == NULL) {
            diag_printf("No memory"); return -1;
        }
        conn->read_alloc *= 2;
        conn->read_buffer = new_buffer;
    }
    retval = read(conn->inet_sock, conn->read_buffer + conn->read_size,
            conn->read_alloc  - conn->read_size);
	
    if (retval == 0) {
	 diag_printf("read returned zero********\n");	
	 sleep(1);	//jwj: fix redial pptp fail when dhcp+pptp, dns auto
        return -1;
    }
    if (retval < 0) {
        if (errno == EINTR || errno == EAGAIN)
	   		return 0;
        else { /* a real error */
            diag_printf("read error: %s", strerror(errno));
            return -1;
        }
    }
    conn->read_size += retval;
    return 0;
}
#define input_length 300
static unsigned char buffer_input[input_length];
/*** Packet formation *********************************************************/
int pptp_check_packet(PPTP_CONN * conn, void **buf, size_t *size)
{
    struct pptp_header *header;
    size_t bad_bytes = 0;

    while ((conn->read_size-bad_bytes) >= sizeof(struct pptp_header)) {
		
        header = (struct pptp_header *) (conn->read_buffer + bad_bytes);
        if (ntoh32(header->magic) != PPTP_MAGIC) goto throwitout;
        if (ntoh16(header->reserved0) != 0)
            diag_printf("reserved0 field is not zero! (0x%x) Cisco feature? \n",
                    ntoh16(header->reserved0));
        if (ntoh16(header->length) < sizeof(struct pptp_header)) goto throwitout;
        if (ntoh16(header->length) > PPTP_CTRL_SIZE_MAX) goto throwitout;

        if (ntoh16(header->length) > (conn->read_size-bad_bytes))
            goto flushbadbytes;
		
        if ((ntoh16(header->pptp_type) == PPTP_MESSAGE_CONTROL) &&
                (ntoh16(header->length) !=
                         PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))))
            goto throwitout;

        *size = ntoh16(header->length);
        //*buf = malloc(*size);
        memset(buffer_input,0,sizeof(buffer_input));
        *buf = buffer_input;
        if (*buf == NULL) 
		{
			diag_printf("Out of memory."); 
			return 0; 
			/* ack! */ 
		}
        memcpy(*buf, conn->read_buffer + bad_bytes, *size);
        /* Delete this packet from the read_buffer. */
        conn->read_size -= (bad_bytes + *size);
        memmove(conn->read_buffer, conn->read_buffer + bad_bytes + *size, 
                conn->read_size);
        if (bad_bytes > 0) 
            diag_printf("%lu bad bytes thrown away.", (unsigned long) bad_bytes);
        return 1;
throwitout:
        bad_bytes++;
    }
flushbadbytes:
    /* no more packets.  Let's get rid of those bad bytes */
    conn->read_size -= bad_bytes;
    memmove(conn->read_buffer, conn->read_buffer + bad_bytes, conn->read_size);
    if (bad_bytes > 0) 
        diag_printf("%lu bad bytes thrown away.", (unsigned long) bad_bytes);
    return 0;
}

int pptp_send_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size)
{
    if( conn->write_size > 0) pptp_write_some(conn);
    if( conn->write_size == 0){
        ssize_t retval;
        retval = write(conn->inet_sock, buffer, size);
        if (retval < 0) { /* error. */
            if (errno == EAGAIN || errno == EINTR) { 
                /* ignore */;
                retval = 0;
            } else { /* a real error */
                diag_printf("write error: %s", strerror(errno));
                pptp_conn_destroy(conn); /* shut down fast. */
                return 0;
            }
        }
        ctrlp_rep( buffer, retval, 0);
        size -= retval;
        if( size <= 0) return 1;
    }

    if (conn->write_size + size > conn->write_alloc) {
        char *new_buffer = realloc(conn->write_buffer, 
                sizeof(*(conn->write_buffer)) * conn->write_alloc * 2);
        if (new_buffer == NULL) {
            diag_printf("Out of memory"); return 0;
        }
        conn->write_alloc *= 2;
        conn->write_buffer = new_buffer;
    }
    memcpy(conn->write_buffer + conn->write_size, buffer, size);
    conn->write_size += size;
    ctrlp_rep( buffer,size,1);
    return 1;
}

/*** Packet Dispatch **********************************************************/
int pptp_dispatch_packet(PPTP_CONN * conn, void * buffer, size_t size)
{
    int r = 0;	
    struct pptp_header *header = (struct pptp_header *)buffer;
    assert(ntoh32(header->magic) == PPTP_MAGIC);
    switch (ntoh16(header->pptp_type)) {
        case PPTP_MESSAGE_CONTROL:
            r = ctrlp_disp(conn, buffer, size);
            break;
        case PPTP_MESSAGE_MANAGE:
            /* MANAGEMENT messages aren't even part of the spec right now. */
            diag_printf("PPTP management message received, but not understood.");
            break;
        default:
            diag_printf("Unknown PPTP control message type received: %u", 
                    (unsigned int) ntoh16(header->pptp_type));
            break;
    }
    return r;
}

/*** pptp_dispatch_ctrl_packet ************************************************/
int ctrlp_disp(PPTP_CONN * conn, void * buffer, size_t size)
{
    struct pptp_header *header = (struct pptp_header *)buffer;
    u_int8_t close_reason = PPTP_STOP_NONE;
    assert(ntoh32(header->magic) == PPTP_MAGIC);
    assert(ntoh16(header->length) == size);
    assert(ntoh16(header->pptp_type) == PPTP_MESSAGE_CONTROL);
    if (size < PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))) {
        diag_printf("Invalid packet received [type: %d; length: %d].",
                (int) ntoh16(header->ctrl_type), (int) size);
        return 0;
    }
    switch (ntoh16(header->ctrl_type)) {
        /* ----------- STANDARD Start-Session MESSAGES ------------ */
#if 0		
        case PPTP_START_CTRL_CONN_RQST:
        {
            struct pptp_start_ctrl_conn *packet = 
                (struct pptp_start_ctrl_conn *) buffer;
            struct pptp_start_ctrl_conn reply = {
                PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RPLY),
                hton16(PPTP_VERSION), 0, 0,
                hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
                hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION),
                PPTP_HOSTNAME, PPTP_VENDOR };
            int idx, rc;
            diag_printf("Received Start Control Connection Request");
            /* fix this packet, if necessary */
			#if 0
			idx = get_quirk_index();
            if (idx != -1 && pptp_fixups[idx].start_ctrl_conn) {
                if ((rc = pptp_fixups[idx].start_ctrl_conn(&reply)))
                    warn("calling the start_ctrl_conn hook failed (%d)", rc);
            }
			#endif
            if (conn->conn_state == CONN_IDLE) {
                if (ntoh16(packet->version) < PPTP_VERSION) {
                    /* Can't support this (earlier) PPTP_VERSION */
                    reply.version = packet->version;
                    /* protocol version not supported */
                    reply.result_code = hton8(5);
                    pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
                    pptp_reset_timer(); /* give sender a chance for a retry */
                } else { /* same or greater version */
                    if (pptp_send_ctrl_packet(conn, &reply, sizeof(reply))) {
                        conn->conn_state = CONN_ESTABLISHED;
                        diag_printf("server connection ESTABLISHED.");
                        pptp_reset_timer();
                    }
                }
            }
            break;
        }
#endif		
        case PPTP_START_CTRL_CONN_RPLY:
        {
            struct pptp_start_ctrl_conn *packet = 
                (struct pptp_start_ctrl_conn *) buffer;
            diag_printf("Received Start Control Connection Reply\n");
            if (conn->conn_state == CONN_WAIT_CTL_REPLY) {
                /* XXX handle collision XXX [see rfc] */
                if (ntoh16(packet->version) != PPTP_VERSION) {
                    if (conn->callback != NULL)
                        conn->callback(conn, CONN_OPEN_FAIL);
                    close_reason = PPTP_STOP_PROTOCOL;
                    goto pptp_conn_close;
                }
                if (ntoh8(packet->result_code) != 1 &&
                    /* J'ai change le if () afin que la connection ne se ferme
                     * pas pour un "rien" :p adel@cybercable.fr -
                     * 
                     * Don't close the connection if the result code is zero
                     * (feature found in certain ADSL modems)
                     */
                        ntoh8(packet->result_code) != 0) { 
                    diag_printf("Negative reply received to our Start Control "
                            "Connection Request\n");        
                    if (conn->callback != NULL)
                        conn->callback(conn, CONN_OPEN_FAIL);
                    close_reason = PPTP_STOP_PROTOCOL;
                    goto pptp_conn_close;
                }
                conn->conn_state = CONN_ESTABLISHED;
                /* log session properties */
                conn->version      = ntoh16(packet->version);
                conn->firmware_rev = ntoh16(packet->firmware_rev);
                memcpy(conn->hostname, packet->hostname, sizeof(conn->hostname));
                memcpy(conn->vendor, packet->vendor, sizeof(conn->vendor));
                //pptp_reset_timer(); /* 60 seconds until keep-alive */
                diag_printf("Client connection established.\n");
                if (conn->callback != NULL)
                    conn->callback(conn, CONN_OPEN_DONE);

				//start second stage connection
				pptp_call_open(conn, NULL, NULL);				
            } /* else goto pptp_conn_close; */
            break;
        }
            /* ----------- STANDARD Stop-Session MESSAGES ------------ */
        case PPTP_STOP_CTRL_CONN_RQST:
        {
            /* conn_state should be CONN_ESTABLISHED, but it could be 
             * something else */
            struct pptp_stop_ctrl_conn reply = {
                PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RPLY), 
                hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0
            };
            diag_printf("Received Stop Control Connection Request.\n");

			//stop ppp thread
			pptp_close();
			
            if (conn->conn_state == CONN_IDLE) break;
            if (pptp_send_ctrl_packet(conn, &reply, sizeof(reply))) {
                if (conn->callback != NULL)
                    conn->callback(conn, CONN_CLOSE_RQST);
                conn->conn_state = CONN_IDLE;
		return -1;
            }
            break;
        }
        case PPTP_STOP_CTRL_CONN_RPLY:
        {
            diag_printf("Received Stop Control Connection Reply.\n");
            /* conn_state should be CONN_WAIT_STOP_REPLY, but it 
             * could be something else */
            if (conn->conn_state == CONN_IDLE) break;
            conn->conn_state = CONN_IDLE;
	    return -1;
        }
            /* ----------- STANDARD Echo/Keepalive MESSAGES ------------ */
        case PPTP_ECHO_RPLY:
        {
            struct pptp_echo_rply *packet = 
                (struct pptp_echo_rply *) buffer;
            if ((conn->ka_state == KA_OUTSTANDING) && 
                    (ntoh32(packet->identifier) == conn->ka_id)) {
                conn->ka_id++;
                conn->ka_state = KA_NONE;
            }
            break;
        }
        case PPTP_ECHO_RQST:
        {
            struct pptp_echo_rqst *packet = 
                (struct pptp_echo_rqst *) buffer;
            struct pptp_echo_rply reply = {
                PPTP_HEADER_CTRL(PPTP_ECHO_RPLY), 
                packet->identifier, /* skip hton32(ntoh32(id)) */
                hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0
            };
            pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
            break;
        }
            /* ----------- OUTGOING CALL MESSAGES ------------ */
        case PPTP_OUT_CALL_RQST:
        {
            struct pptp_out_call_rqst *packet =
                (struct pptp_out_call_rqst *)buffer;
            struct pptp_out_call_rply reply = {
                PPTP_HEADER_CTRL(PPTP_OUT_CALL_RPLY),
                0 /* callid */, packet->call_id, 1, PPTP_GENERAL_ERROR_NONE, 0,
                hton32(PPTP_CONNECT_SPEED), 
                hton16(PPTP_WINDOW), hton16(PPTP_DELAY), 0 
            };
            diag_printf("Received Outgoing Call Request.\n");
            /* XXX PAC: eventually this should make an outgoing call. XXX */
            reply.result_code = hton8(7); /* outgoing calls verboten */
            pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
            break;
        }
        case PPTP_OUT_CALL_RPLY:
        {
			extern u_int16_t pptp_gre_call_id, pptp_gre_peer_call_id;
            struct pptp_out_call_rply *packet =
                (struct pptp_out_call_rply *)buffer;			
			pptp_gre_peer_call_id  = ntoh16(packet->call_id);
			pptp_gre_call_id = ntoh16(packet->call_id_peer);
				
            diag_printf("Received Outgoing Call Reply.\n");			
			//start request echo timer			
			//call_reply_timer();
			/*-----------------start pppd thread-----------------*/			
			wrapper_pptp_ppp();
			sleep(1);			
            break;
        }
            /* ----------- INCOMING CALL MESSAGES ------------ */
            /* XXX write me XXX */
            /* ----------- CALL CONTROL MESSAGES ------------ */
        case PPTP_CALL_CLEAR_RQST:
        {
            struct pptp_call_clear_rqst *packet =
                (struct pptp_call_clear_rqst *)buffer;
            struct pptp_call_clear_ntfy reply = {
                PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_NTFY), packet->call_id,
                1, PPTP_GENERAL_ERROR_NONE, 0, 0, {0}
            };
            diag_printf("Received Call Clear Request.\n");

			/* dzh add for clear ppp session info*/
			extern void pptp_close();
			pptp_close();
            break;
        }
        case PPTP_CALL_CLEAR_NTFY:
        {
            struct pptp_call_clear_ntfy *packet =
                (struct pptp_call_clear_ntfy *)buffer;
            diag_printf("Call disconnect notification received (call id %d)",
                    ntoh16(packet->call_id));
			break;
        }
        case PPTP_SET_LINK_INFO:
        {
            /* I HAVE NO CLUE WHAT TO DO IF send_accm IS NOT 0! */
            /* this is really dealt with in the HDLC deencapsulation, anyway. */
            struct pptp_set_link_info *packet =
                (struct pptp_set_link_info *)buffer;
            /* log it. */
            diag_printf("PPTP_SET_LINK_INFO received from peer_callid %u",
                    (unsigned int) ntoh16(packet->call_id_peer));
            diag_printf("  send_accm is %08lX, recv_accm is %08lX",
                    (unsigned long) ntoh32(packet->send_accm),
                    (unsigned long) ntoh32(packet->recv_accm));
            if (!(ntoh32(packet->send_accm) == 0 &&
                    ntoh32(packet->recv_accm) == 0))
                diag_printf("Non-zero Async Control Character Maps are not supported!");
            break;
        }
        default:
            diag_printf("Unrecognized Packet %d received.", 
                    (int) ntoh16(((struct pptp_header *)buffer)->ctrl_type));
            /* goto pptp_conn_close; */
            break;
    }
    return 0;
pptp_conn_close:
    diag_printf("pptp_conn_close(%d)", (int) close_reason);
    pptp_conn_close(conn, close_reason);
    return 0;
}
