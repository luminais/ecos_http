/*
 * sys_term.c
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

/*	$NetBSD: sys_term.c,v 1.44 2007/01/17 21:44:50 hubertf Exp $	*/

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
__RCSID("$NetBSD: sys_term.c,v 1.44 2007/01/17 21:44:50 hubertf Exp $");
#endif /* not lint */

#include "telnetd.h"
//#include "pathnames.h"

//#include <util.h>
//#include <vis.h>

//#include <utmp.h>
//struct	utmp wtmp;




#define VEOF            0       /* ICANON */
#define VEOL            1       /* ICANON */
#define VEOL2           2       /* ICANON */
#define VWERASE         4       /* ICANON */
#define VREPRINT        6       /* ICANON */
#define VDSUSP          11      /* ISIG */
#define VLNEXT          14      /* IEXTEN */
#define VDISCARD        15      /* IEXTEN */
#define VSTATUS         18      /* ICANON */
#define IXANY           0x00000800      /* any char will restart after stop */







#define SCPYN(a, b)	(void) strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))





struct termios termbuf, termbuf2;	/* pty control structure */

void getptyslave(void);
int cleanopen(char *);
char **addarg(char **, char *);
void scrub_env(void);
int getent(char *, char *);
char *getstr(const char *, char **);
#ifdef KRB5
extern void kerberos5_cleanup(void);
#endif

/*
 * init_termbuf()
 * copy_termbuf(cp)
 * set_termbuf()
 *
 * These three routines are used to get and set the "termbuf" structure
 * to and from the kernel.  init_termbuf() gets the current settings.
 * copy_termbuf() hands in a new "termbuf" to write to the kernel, and
 * set_termbuf() writes the structure into the kernel.
 */

void
init_termbuf(void)
{

/* TODO how to init termbuf ?? */
//	(void) tcgetattr(pty, &termbuf);



/* tty line mode */
termbuf.c_lflag |= EXTPROC;


/* tty_iscrnl ?? */
termbuf.c_iflag |= ICRNL;

/* always let client do the echoing */
termbuf.c_lflag |= ECHO;


/* tty_isediting && tty_israw ?? */
termbuf.c_lflag |= ICANON;

/* tty is trapsig */
termbuf.c_lflag |= ISIG;

/* Always set flow mode to 1 */
termbuf.c_iflag |= IXON;


/* restart any ?? */
termbuf.c_iflag |= IXANY;


/* always support binary in */
termbuf.c_iflag &= ~ISTRIP;

/* always support binary out */
termbuf.c_oflag &= ~OPOST;



	termbuf2 = termbuf;
}

#if	defined(LINEMODE) && defined(TIOCPKT_IOCTL)
void
copy_termbuf(char *cp, int len)
{
	if (len > sizeof(termbuf))
		len = sizeof(termbuf);
	memmove((char *)&termbuf, cp, len);
	termbuf2 = termbuf;
}
#endif	/* defined(LINEMODE) && defined(TIOCPKT_IOCTL) */

void
set_termbuf(void)
{
}


/*
 * spcset(func, valp, valpp)
 *
 * This function takes various special characters (func), and
 * sets *valp to the current value of that character, and
 * *valpp to point to where in the "termbuf" structure that
 * value is kept.
 *
 * It returns the SLC_ level of support for this function.
 */


int
spcset(int func, cc_t *valp, cc_t **valpp)
{

#define	setval(a, b)	*valp = termbuf.c_cc[a]; \
			*valpp = &termbuf.c_cc[a]; \
			return(b);
#define	defval(a) *valp = ((cc_t)a); *valpp = (cc_t *)0; return(SLC_DEFAULT);

	switch(func) {
	case SLC_EOF:
		setval(VEOF, SLC_VARIABLE);
	case SLC_EC:
		setval(VERASE, SLC_VARIABLE);
	case SLC_EL:
		setval(VKILL, SLC_VARIABLE);
	case SLC_IP:
		setval(VINTR, SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_ABORT:
		setval(VQUIT, SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_XON:
		setval(VSTART, SLC_VARIABLE);
	case SLC_XOFF:
		setval(VSTOP, SLC_VARIABLE);
	case SLC_EW:
		setval(VWERASE, SLC_VARIABLE);
	case SLC_RP:
		setval(VREPRINT, SLC_VARIABLE);
	case SLC_LNEXT:
		setval(VLNEXT, SLC_VARIABLE);
	case SLC_AO:
		setval(VDISCARD, SLC_VARIABLE|SLC_FLUSHOUT);
	case SLC_SUSP:
		setval(VSUSP, SLC_VARIABLE|SLC_FLUSHIN);
	case SLC_FORW1:
		setval(VEOL, SLC_VARIABLE);
	case SLC_FORW2:
		setval(VEOL2, SLC_VARIABLE);
	case SLC_AYT:
		TELNETD_PRINT("SLC_AYT: array out of range..\n");
//		setval(VSTATUS, SLC_VARIABLE);

	case SLC_BRK:
	case SLC_SYNCH:
	case SLC_EOR:
		defval(0);

	default:
		*valp = 0;
		*valpp = 0;
		return(SLC_NOSUPPORT);
	}
}


/*
 * getpty()
 *
 * Allocate a pty.  As a side effect, the external character
 * array "line" contains the name of the slave side.
 *
 * Returns the file descriptor of the opened pty.
 */
#ifndef	__GNUC__
char *line = NULL16STR;
#else
static char Xline[] = NULL16STR;
char *line = Xline;
#endif


static int ptyslavefd; /* for cleanopen() */


#ifdef	LINEMODE
/*
 * tty_flowmode()	Find out if flow control is enabled or disabled.
 * tty_linemode()	Find out if linemode (external processing) is enabled.
 * tty_setlinemod(on)	Turn on/off linemode.
 * tty_isecho()		Find out if echoing is turned on.
 * tty_setecho(on)	Enable/disable character echoing.
 * tty_israw()		Find out if terminal is in RAW mode.
 * tty_binaryin(on)	Turn on/off BINARY on input.
 * tty_binaryout(on)	Turn on/off BINARY on output.
 * tty_isediting()	Find out if line editing is enabled.
 * tty_istrapsig()	Find out if signal trapping is enabled.
 * tty_setedit(on)	Turn on/off line editing.
 * tty_setsig(on)	Turn on/off signal trapping.
 * tty_issofttab()	Find out if tab expansion is enabled.
 * tty_setsofttab(on)	Turn on/off soft tab expansion.
 * tty_islitecho()	Find out if typed control chars are echoed literally
 * tty_setlitecho()	Turn on/off literal echo of control chars
 * tty_tspeed(val)	Set transmit speed to val.
 * tty_rspeed(val)	Set receive speed to val.
 */


int
tty_linemode(void)
{
	return(termbuf.c_lflag & EXTPROC);
}

void
tty_setlinemode(int on)
{
	set_termbuf();
//	(void) ioctl(pty, TIOCEXT, (char *)&on);
	init_termbuf();
}
#endif	/* LINEMODE */

int
tty_isecho(void)
{
	return (termbuf.c_lflag & ECHO);
}

int
tty_flowmode(void)
{
	return((termbuf.c_iflag & IXON) ? 1 : 0);
}

int
tty_restartany(void)
{
	return((termbuf.c_iflag & IXANY) ? 1 : 0);
}

void
tty_setecho(int on)
{
	if (on)
		termbuf.c_lflag |= ECHO;
	else
		termbuf.c_lflag &= ~ECHO;
}

int
tty_israw(void)
{
	return(!(termbuf.c_lflag & ICANON));
}

void
tty_binaryin(int on)
{
	if (on) {
		termbuf.c_iflag &= ~ISTRIP;
	} else {
		termbuf.c_iflag |= ISTRIP;
	}
}

void
tty_binaryout(int on)
{
	if (on) {
		termbuf.c_cflag &= ~(CSIZE|PARENB);
		termbuf.c_cflag |= CS8;
		termbuf.c_oflag &= ~OPOST;
	} else {
		termbuf.c_cflag &= ~CSIZE;
		termbuf.c_cflag |= CS7|PARENB;
		termbuf.c_oflag |= OPOST;
	}
}

int
tty_isbinaryin(void)
{
	/* supposed we support binary in */
	return(!(termbuf.c_iflag & ISTRIP));
}

int
tty_isbinaryout(void)
{
	/* supposed we support binary out */
	return(!(termbuf.c_oflag&OPOST));
	//return 1;
}

#ifdef	LINEMODE
int
tty_isediting(void)
{
	return(termbuf.c_lflag & ICANON);
}

int
tty_istrapsig(void)
{
	return(termbuf.c_lflag & ISIG);
}

void
tty_setedit(int on)
{
	if (on)
		termbuf.c_lflag |= ICANON;
	else
		termbuf.c_lflag &= ~ICANON;
}

void
tty_setsig(int on)
{
	if (on)
		termbuf.c_lflag |= ISIG;
	else
		termbuf.c_lflag &= ~ISIG;
}
#endif	/* LINEMODE */

int
tty_issofttab(void)
{
# ifdef	OXTABS
	return (termbuf.c_oflag & OXTABS);
# endif
# ifdef	TABDLY
	return ((termbuf.c_oflag & TABDLY) == TAB3);
# endif
	return 0;
}

void
tty_setsofttab(int on)
{
	if (on) {
# ifdef	OXTABS
		termbuf.c_oflag |= OXTABS;
# endif
# ifdef	TABDLY
		termbuf.c_oflag &= ~TABDLY;
		termbuf.c_oflag |= TAB3;
# endif
	} else {
# ifdef	OXTABS
		termbuf.c_oflag &= ~OXTABS;
# endif
# ifdef	TABDLY
		termbuf.c_oflag &= ~TABDLY;
		termbuf.c_oflag |= TAB0;
# endif
	}
}

int
tty_islitecho(void)
{
# ifdef	ECHOCTL
	return (!(termbuf.c_lflag & ECHOCTL));
# endif
# ifdef	TCTLECH
	return (!(termbuf.c_lflag & TCTLECH));
# endif
# if	!defined(ECHOCTL) && !defined(TCTLECH)
	return (0);	/* assumes ctl chars are echoed '^x' */
# endif
}

void
tty_setlitecho(int on)
{
# ifdef	ECHOCTL
	if (on)
		termbuf.c_lflag &= ~ECHOCTL;
	else
		termbuf.c_lflag |= ECHOCTL;
# endif
# ifdef	TCTLECH
	if (on)
		termbuf.c_lflag &= ~TCTLECH;
	else
		termbuf.c_lflag |= TCTLECH;
# endif
}

int
tty_iscrnl(void)
{
	//return (termbuf.c_iflag & ICRNL);


	if(!(termbuf.c_iflag & ICRNL))
		TELNETD_PRINT("Error ,tty_iscrnl no ICRNL\n");
	return 1;
}


void
tty_tspeed(int val)
{
//	cfsetospeed(&termbuf, val);
}

void
tty_rspeed(int val)
{
//	cfsetispeed(&termbuf, val);
}




extern int def_tspeed, def_rspeed;
	extern int def_row, def_col;
/*
 * Open the specified slave side of the pty,
 * making sure that we have a clean tty.
 */
int
cleanopen(char *ttyline)
{
	return ptyslavefd;
}


char	*envinit[3];

void
init_env(void)
{
	char **envp;

	envp = envinit;
	if ((*envp = getenv("TZ")))
		*envp++ -= 3;
	*envp = 0;
	environ = envinit;
}



char **
addarg(char **argv, char *val)
{
	char **cpp;
	char **nargv;

	if (argv == NULL) {
		/*
		 * 10 entries, a leading length, and a null
		 */
		argv = (char **)malloc(sizeof(*argv) * 12);
		if (argv == NULL)
			return(NULL);
		*argv++ = (char *)10;
		*argv = (char *)0;
	}
	for (cpp = argv; *cpp; cpp++)
		;
	if (cpp == &argv[(long)argv[-1]]) {
		--argv;
		nargv = (char **)realloc(argv,
		    sizeof(*argv) * ((long)(*argv) + 10 + 2));
		if (argv == NULL) {
			fatal(net, "not enough memory");
			/*NOTREACHED*/
		}
		argv = nargv;
		*argv = (char *)((long)(*argv) + 10);
		argv++;
		cpp = &argv[(long)argv[-1] - 10];
	}
	*cpp++ = val;
	*cpp = 0;
	return(argv);
}

/*
 * scrub_env()
 *
 * We only accept the environment variables listed below.
 */

void
scrub_env(void)
{
	static const char *reject[] = {
		"TERMCAP=/",
		NULL
	};

	static const char *acceptstr[] = {
		"XAUTH=", "XAUTHORITY=", "DISPLAY=",
		"TERM=",
		"EDITOR=",
		"PAGER=",
		"LOGNAME=",
		"POSIXLY_CORRECT=",
		"TERMCAP=",
		"PRINTER=",
		NULL
	};

	char **cpp, **cpp2;
	const char **p;

	for (cpp2 = cpp = environ; *cpp; cpp++) {
		int reject_it = 0;

		for(p = reject; *p; p++)
			if(strncmp(*cpp, *p, strlen(*p)) == 0) {
				reject_it = 1;
				break;
			}
		if (reject_it)
			continue;

		for(p = acceptstr; *p; p++)
			if(strncmp(*cpp, *p, strlen(*p)) == 0)
				break;
		if(*p != NULL)
			*cpp2++ = *cpp;
	}
	*cpp2 = NULL;
}
