/*
 * telnetd.c
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id:
 */

/*	$NetBSD: telnetd.c,v 1.51 2008/07/20 01:09:07 lukem Exp $	*/

/*
 * Copyright (C) 1997 and 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__COPYRIGHT("@(#) Copyright (c) 1989, 1993 The Regents of the \
University of California.  All rights reserved.");
__RCSID("$NetBSD: telnetd.c,v 1.51 2008/07/20 01:09:07 lukem Exp $");
#endif /* not lint */

#include <telnetd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <telnet.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <limits.h>
#include <shared.h>
#include <unistd.h>

extern void *telnetd_osl_exec_shell(int (*shell)(int), char *name);
extern int telnetd_osl_is_shell_running(void *handle);
extern int telnetd_osl_getfd_shell(void *shell_handle);
extern void telnetd_osl_kill_shell(void *handle);
extern void telnetd_osl_th_shutdown(void *handle);


static int getterminaltype(char *, size_t);
static void _gettermname(void);
static int terminaltypeok(char *);
static void init_telnetd_handler(int sock_fd, int shell_fd);
static void protocol_negotiation(void);
static int telnetd_handler(int f, int p);


/*
 * I/O data buffers,
 * pointers, and counters.
 */
char	ptyibuf[BUFFER_SZ], *ptyip = ptyibuf;
char	ptyibuf2[BUFFER_SZ];

static size_t
telnetd_strlcpy(char *dest, const char *src, size_t len)
{
	size_t ret = strlen(src);

	if (len != 0) {
		if (ret < len)
			strcpy(dest, src);
		else {
			strncpy(dest, src, len - 1);
			dest[len-1] = 0;
		}
	}
	return ret;
}

static int
check_shell_running(struct login_info *linfo)
{
	if (!telnetd_osl_is_shell_running(linfo->shell_handle)) {
		TELNETD_PRINT("check_shell_running,shell dead: %s\n", linfo->shell_name);
		return -1;
	}
	return 0;
}

void
req_handler(struct telnetd_context *t_ctx)
{
	struct login_info *linfo;
	int shell_fd = -1;
	int  ret = 0;

	if (!t_ctx) {
		TELNETD_PRINT("reg_handler: got arg error\n");
		return;
	}

	t_ctx->status_flag = INITIATE_STATE;
	linfo = &t_ctx->ln_info;

	/* 
	 * If no login or authentication is needed, 
	 * executes default shell directly. 
	 */
	if (!linfo->need_login) {
		if (linfo->default_shell) {
			linfo->shell_handle = telnetd_osl_exec_shell(linfo->default_shell,
			linfo->shell_name);

			if (!linfo->shell_handle) {
				TELNETD_PRINT("Get shell handle error\n");
				goto out;
			}
		}
	}

	/* 
	 * Get file descriptor from shell, which will 
	 * be used to communicate between shell & server. 
	 */
	shell_fd = telnetd_osl_getfd_shell(linfo->shell_handle);
	if (shell_fd < 0) {
		TELNETD_PRINT("error, get fd failed\n");
		goto out;
	}

	/* Initialize data structures we need */
	init_telnetd_handler(t_ctx->conn_fd, shell_fd);


	/* Perform basic telnet protocol negotiation procedure */
	protocol_negotiation();


	while (1) {
		switch (t_ctx->status_flag) {

		case SHUTDOWN_STATE:
			TELNETD_PRINT("telnetd shutdown\n");
			ret = -1;
			break;

		default:
			/* main function of processing telnet cmd & data redirection */
			ret = telnetd_handler(t_ctx->conn_fd, shell_fd);
			if (ret < 0)
				break;

			/* 
			 * Check if shell is running or not; 
			 * The shell may exit by itself, in such case 
			 * we should finish our handler directly.
			 */
			ret = check_shell_running(linfo);
			break;
		}
		if (ret < 0)
			break;
	}

out:
	if (linfo->shell_handle) {
		telnetd_osl_kill_shell(linfo->shell_handle);
		free(linfo->shell_handle);
		linfo->shell_handle = NULL;
	}
	if (shell_fd >= 0)
		close(shell_fd);

	return;
}

void
telnetd_shutdown(struct telnetd_context *t_ctx)
{
	t_ctx->status_flag = SHUTDOWN_STATE;
	telnetd_osl_th_shutdown(t_ctx->thread_handle);
}

/*
 * getterminaltype
 *
 * Ask the other end to send along its terminal type and speed.
 * Output is the variable terminaltype filled in.
 */
static unsigned char ttytype_sbbuf[] = {
	IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE
};

static int
getterminaltype(char *name, size_t l)
{
	int retval = -1;

	settimer(baseline);
#ifdef AUTHENTICATION
	/*
	 * Handle the Authentication option before we do anything else.
	 */
	send_do(TELOPT_AUTHENTICATION, 1);
	while (his_will_wont_is_changing(TELOPT_AUTHENTICATION))
		ttloop();
	if (his_state_is_will(TELOPT_AUTHENTICATION)) {
		retval = auth_wait(name, l);
	}
#endif

#ifdef	ENCRYPTION
	send_will(TELOPT_ENCRYPT, 1);
#endif	/* ENCRYPTION */
	send_do(TELOPT_TTYPE, 1);
	send_do(TELOPT_TSPEED, 1);
	send_do(TELOPT_XDISPLOC, 1);
	send_do(TELOPT_NEW_ENVIRON, 1);
	send_do(TELOPT_OLD_ENVIRON, 1);
	while (
#ifdef	ENCRYPTION
			his_do_dont_is_changing(TELOPT_ENCRYPT) ||
#endif	/* ENCRYPTION */
			his_will_wont_is_changing(TELOPT_TTYPE) ||
			his_will_wont_is_changing(TELOPT_TSPEED) ||
			his_will_wont_is_changing(TELOPT_XDISPLOC) ||
			his_will_wont_is_changing(TELOPT_NEW_ENVIRON) ||
			his_will_wont_is_changing(TELOPT_OLD_ENVIRON)) {
		ttloop();
	}
	if (his_state_is_will(TELOPT_TSPEED)) {
		static unsigned char sb[] =
		{ IAC, SB, TELOPT_TSPEED, TELQUAL_SEND, IAC, SE };

		output_datalen((const char *)sb, sizeof(sb));
		DIAG(TD_OPTIONS, printsub('>', sb + 2, sizeof(sb - 2)); );
	}
#ifdef	ENCRYPTION
	/*
	 * Wait for the negotiation of what type of encryption we can
	 * send with.  If autoencrypt is not set, this will just return.
	 */
	if (his_state_is_will(TELOPT_ENCRYPT)) {
		encrypt_wait();
	}
#endif	/* ENCRYPTION */
	if (his_state_is_will(TELOPT_XDISPLOC)) {
		static unsigned char sb[] =
		{ IAC, SB, TELOPT_XDISPLOC, TELQUAL_SEND, IAC, SE };

		output_datalen((const char *)sb, sizeof(sb));
		DIAG(TD_OPTIONS, printsub('>', sb + 2, sizeof(sb - 2)); );
	}
	if (his_state_is_will(TELOPT_NEW_ENVIRON)) {
		static unsigned char sb[] =
		{ IAC, SB, TELOPT_NEW_ENVIRON, TELQUAL_SEND, IAC, SE };

		output_datalen((const char *)sb, sizeof(sb));
		DIAG(TD_OPTIONS, printsub('>', sb + 2, sizeof(sb - 2)); );
	}
	else if (his_state_is_will(TELOPT_OLD_ENVIRON)) {
		static unsigned char sb[] =
		{ IAC, SB, TELOPT_OLD_ENVIRON, TELQUAL_SEND, IAC, SE };

		output_datalen((const char *)sb, sizeof(sb));
		DIAG(TD_OPTIONS, printsub('>', sb + 2, sizeof(sb - 2)); );
	}
	if (his_state_is_will(TELOPT_TTYPE)) {

		output_datalen((const char *)ttytype_sbbuf, sizeof(ttytype_sbbuf));
		DIAG(TD_OPTIONS, printsub('>', ttytype_sbbuf + 2, sizeof(ttytype_sbbuf - 2)); );
	}
	if (his_state_is_will(TELOPT_TSPEED)) {
		while (sequenceIs(tspeedsubopt, baseline))
			ttloop();
	}
	if (his_state_is_will(TELOPT_XDISPLOC)) {
		while (sequenceIs(xdisplocsubopt, baseline))
			ttloop();
	}
	if (his_state_is_will(TELOPT_NEW_ENVIRON)) {
		while (sequenceIs(environsubopt, baseline))
			ttloop();
	}
	if (his_state_is_will(TELOPT_OLD_ENVIRON)) {
		while (sequenceIs(oenvironsubopt, baseline))
			ttloop();
	}
	if (his_state_is_will(TELOPT_TTYPE)) {
		char first[256], last[256];

		while (sequenceIs(ttypesubopt, baseline))
			ttloop();

		/*
		 * If the other side has already disabled the option, then
		 * we have to just go with what we (might) have already gotten.
		 */
		if (his_state_is_will(TELOPT_TTYPE) && !terminaltypeok(terminaltype)) {
			(void) telnetd_strlcpy(first, terminaltype, sizeof(first));
			for (;;) {
				/*
				 * Save the unknown name, and request the next name.
				 */
				(void) telnetd_strlcpy(last, terminaltype, sizeof(last));
				_gettermname();
				if (terminaltypeok(terminaltype))
					break;
				if ((strncmp(last, terminaltype, sizeof(last)) == 0) ||
				his_state_is_wont(TELOPT_TTYPE)) {
					/*
					 * We've hit the end.  If this is the same as
					 * the first name, just go with it.
					 */
					if (strncmp(first, terminaltype, sizeof(first)) == 0)
						break;
					/*
					 * Get the terminal name one more time, so that
					 * RFC1091 compliant telnets will cycle back to
					 * the start of the list.
					 */
					_gettermname();
					if (strncmp(first, terminaltype, sizeof(first)) != 0) {
						(void) telnetd_strlcpy(terminaltype,
						first, sizeof(terminaltype));
					}
					break;
				}
			}
		}
	}
	return retval;
}  /* end of getterminaltype */

static void
_gettermname(void)
{
	/*
	 * If the client turned off the option,
	 * we can't send another request, so we
	 * just return.
	 */
	if (his_state_is_wont(TELOPT_TTYPE))
		return;
	settimer(baseline);
	output_datalen((const char *)ttytype_sbbuf, sizeof(ttytype_sbbuf));
	DIAG(TD_OPTIONS, printsub('>', ttytype_sbbuf + 2, sizeof(ttytype_sbbuf) - 2); );
	while (sequenceIs(ttypesubopt, baseline))
		ttloop();
}

static int
terminaltypeok(char *s)
{
	if (terminaltype == NULL)
		return 1;

	/*
	 * tgetent() will return 1 if the type is known, and
	 * 0 if it is not known.  If it returns -1, it couldn't
	 * open the database.  But if we can't open the database,
	 * it won't help to say we failed, because we won't be
	 * able to verify anything else.  So, we treat -1 like 1.
	 */
	/* since ecos has no tgetent implemented, ignore it */
	return 1;
}

static void
init_telnetd_handler(int sock_fd, int shell_fd)
{
	net = sock_fd;
	pty = shell_fd;
	pfrontp = pbackp = ptyobuf;
	netip = netibuf;
	nfrontp = nbackp = netobuf;
	ptyip = ptyibuf;
	ncc = pcc = 0;

	memset(ptyobuf, 0, BUFFER_SZ+NETSLOP);
	memset(netobuf, 0, BUFFER_SZ+NETSLOP);
	memset(netibuf, 0, BUFFER_SZ);
	memset(ptyibuf, 0, BUFFER_SZ);
	memset(options, 0, ARRAY_SZ);
	memset(do_dont_resp, 0, ARRAY_SZ);
	memset(will_wont_resp, 0, ARRAY_SZ);

}

/*
 * Negotiate detail settings with the other end, which includes 
 * terminal type, ECHO mode, LINE mode, STATUS...
 */
static void
protocol_negotiation(void)
{

	char user_name[256];

	/*
	 * get terminal type.
	 */
	*user_name = 0;
	getterminaltype(user_name, sizeof(user_name));
#ifdef LINEMODE
	/*
	 * Initialize the slc mapping table.
	 */
	get_slc_defaults();
#endif
	/*
	 * Do some tests where it is desireable to wait for a response.
	 * Rather than doing them slowly, one at a time, do them all
	 * at once.
	 */
	if (my_state_is_wont(TELOPT_SGA))
		send_will(TELOPT_SGA, 1);
	/*
	 * Is the client side a 4.2 (NOT 4.3) system?  We need to know this
	 * because 4.2 clients are unable to deal with TCP urgent data.
	 *
	 * To find out, we send out a "DO ECHO".  If the remote system
	 * answers "WILL ECHO" it is probably a 4.2 client, and we note
	 * that fact ("WILL ECHO" ==> that the client will echo what
	 * WE, the server, sends it; it does NOT mean that the client will
	 * echo the terminal input).
	 */
	send_do(TELOPT_ECHO, 1);

#ifdef	LINEMODE
	if (his_state_is_wont(TELOPT_LINEMODE)) {
		/* Query the peer for linemode support by trying to negotiate
		 * the linemode option.
		 */
		linemode = 0;
		editmode = 0;
		send_do(TELOPT_LINEMODE, 1);  /* send do linemode */
	}
#endif	/* LINEMODE */

	/*
	 * Send along a couple of other options that we wish to negotiate.
	 */
	send_do(TELOPT_NAWS, 1);
	send_will(TELOPT_STATUS, 1);
	flowmode = 1;		/* default flow control state */
	restartany = -1;	/* uninitialized... */
	send_do(TELOPT_LFLOW, 1);

	/*
	 * Spin, waiting for a response from the DO ECHO.  However,
	 * some REALLY DUMB telnets out there might not respond
	 * to the DO ECHO.  So, we spin looking for NAWS, (most dumb
	 * telnets so far seem to respond with WONT for a DO that
	 * they don't understand...) because by the time we get the
	 * response, it will already have processed the DO ECHO.
	 * Kludge upon kludge.
	 */
	while (his_will_wont_is_changing(TELOPT_NAWS))
		ttloop();

	/*
	 * But...
	 * The client might have sent a WILL NAWS as part of its
	 * startup code; if so, we'll be here before we get the
	 * response to the DO ECHO.  We'll make the assumption
	 * that any implementation that understands about NAWS
	 * is a modern enough implementation that it will respond
	 * to our DO ECHO request; hence we'll do another spin
	 * waiting for the ECHO option to settle down, which is
	 * what we wanted to do in the first place...
	 */
	if (his_want_state_is_will(TELOPT_ECHO) && his_state_is_will(TELOPT_NAWS)) {
		while (his_will_wont_is_changing(TELOPT_ECHO))
			ttloop();
	}
	/*
	 * On the off chance that the telnet client is broken and does not
	 * respond to the DO ECHO we sent, (after all, we did send the
	 * DO NAWS negotiation after the DO ECHO, and we won't get here
	 * until a response to the DO NAWS comes back) simulate the
	 * receipt of a will echo.  This will also send a WONT ECHO
	 * to the client, since we assume that the client failed to
	 * respond because it believes that it is already in DO ECHO
	 * mode, which we do not want.
	 */
	if (his_want_state_is_will(TELOPT_ECHO)) {
		DIAG(TD_OPTIONS, output_data("td: simulating recv\r\n"); );
		willoption(TELOPT_ECHO);
	}

	/*
	 * Finally, to clean things up, we turn on our echo.  This
	 * will break stupid 4.2 telnets out of local terminal echo.
	 */

	if (my_state_is_wont(TELOPT_ECHO))
		send_will(TELOPT_ECHO, 1);

#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
	/*
	 * Continuing line mode support.  If client does not support
	 * real linemode, attempt to negotiate kludge linemode by sending
	 * the do timing mark sequence.
	 */
	if (lmodetype < REAL_LINEMODE)
		send_do(TELOPT_TM, 1);
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */

	/*
	 * Call telrcv() once to pick up anything received during
	 * terminal type negotiation, 4.2/4.3 determination, and
	 * linemode negotiation.
	 */
	telrcv();

#ifdef	LINEMODE
	/*
	 * Last check to make sure all our states are correct.
	 */
	init_termbuf();
	localstat();
#endif	/* LINEMODE */


}

static int
telnetd_handler(int f, int p)
{
	fd_set rfdset, wfdset;
	int max_fd, n;
	int c;
	struct timeval tval = {1, 0};


	if (ncc < 0 && pcc < 0) {
		TELNETD_PRINT("ncc: %d, pcc: %d\n", ncc, pcc);
		return -1;
	}
	FD_ZERO(&rfdset);
	FD_ZERO(&wfdset);

	FD_SET(f, &rfdset);
	FD_SET(f, &wfdset);
	FD_SET(p, &rfdset);
	FD_SET(p, &wfdset);

	max_fd = f > p ? f+1:p+1;

	n = select(max_fd, &rfdset, NULL, NULL, &tval);

	if (n < 1) {
		if (n < 0) {
			TELNETD_PRINT("select error\n");
			return -1;
		}

		return 0;
	}

	/*
	 * Something to read from the network...
	 */
	if (FD_ISSET(f, &rfdset)) {

		ncc = read(f, netibuf, sizeof(netibuf));
		if (ncc < 0 && errno == EWOULDBLOCK)
			ncc = 0;
		else {
			if (ncc <= 0) {
				TELNETD_PRINT("ncc <0\n");
				return -1;
			}
			netip = netibuf;
		}
	}

	/*
	 * Something to read from the pty...
	 */
	if (FD_ISSET(p, &rfdset)) {

		pcc = read(p, ptyibuf, BUFFER_SZ);

		/*
		 * On some systems, if we try to read something
		 * off the master side before the slave side is
		 * opened, we get EIO.
		 */
		if (pcc < 0 && (errno == EWOULDBLOCK ||
		errno == EAGAIN ||
		errno == EIO)) {
			pcc = 0;
		} else {
			if (pcc <= 0) {
				TELNETD_PRINT("pcc <0\n");
				return -1;
			}
			ptyip = ptyibuf;
		} /* End else */

	} /* End if(FD_ISSET) */


	/* Redirect the pty data to client */
	while (pcc > 0) {

		/* Check net buff is avalible or not */
		if ((&netobuf[BUFFER_SZ] - nfrontp) < 2)
		{
			TELNETD_PRINT("resource not avaliable\n");
			break;
		}
		c = *ptyip++ & 0377, pcc--;
		if (c == IAC)
			output_data("%c", c);

		/* 
		 * Since the new line is represented as only '\n', 
		 * to meet the telnet protocol, it needed to be preceded by \r.
		 */
		if (c == '\n')
			output_datalen("\r", 1);

		output_data("%c", c);
		if ((c == '\r') && (my_state_is_wont(TELOPT_BINARY))) {
			if (pcc > 0 && ((*ptyip & 0377) == '\n')) {
				output_data("%c", *ptyip++ & 0377);
				pcc--;
			} else
				output_datalen("\0", 1);
		}
	}

	/* Check if the net fd is avaliable to write */
	if (FD_ISSET(f, &wfdset) && (nfrontp - nbackp) > 0)
		netflush();

	if (ncc > 0)
		telrcv();

	/* Check if cli is avaliable to write */
	if (FD_ISSET(p, &wfdset) && (pfrontp - pbackp) > 0)
		ptyflush();
	return 0;
}
void
doeof(void)
{
	init_termbuf();

#if	defined(LINEMODE) && (VEOF == VMIN)
	if (!tty_isediting()) {
		extern char oldeofc;
		*pfrontp++ = oldeofc;
		return;
	}
#endif
	*pfrontp++ = slctab[SLC_EOF].sptr ?
		(unsigned char)*slctab[SLC_EOF].sptr : '\004';
}
