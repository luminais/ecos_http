/*========================================================================
//
//      diag.c
//
//      Infrastructure diagnostic output code
//
//========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg,gthomas,jlarmour
// Contributors: 
// Date:         1999-02-22
// Purpose:      Infrastructure diagnostic output
// Description:  Implementations of infrastructure diagnostic routines.
//
//####DESCRIPTIONEND####
//
//======================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/infra.h>

#include <cyg/infra/cyg_type.h>         // base types
  
#include <cyg/infra/diag.h>             // HAL polled output
#include <cyg/hal/hal_arch.h>           // architectural stuff for...
#include <cyg/hal/hal_intr.h>           // interrupt control
#include <cyg/hal/hal_diag.h>           // diagnostic output routines
#include <stdarg.h>
#include <limits.h>

#ifdef RTK_SYSLOG_SUPPORT
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
static int syslogType=0;
externC int syslog_printk(char * buffer);

#ifdef RTK_REMOTELOG_SUPPORT
#include <sys/uio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
static int remotelogEnable=0;
struct in_addr remoteSvrIp;
#endif
#endif  

#ifdef CYG_HAL_DIAG_LOCK_DATA_DEFN
CYG_HAL_DIAG_LOCK_DATA_DEFN;
#endif
  
/*----------------------------------------------------------------------*/

externC void diag_write_num(
    cyg_uint32  n,              /* number to write              */
    cyg_ucount8 base,           /* radix to write to            */
    cyg_ucount8 sign,           /* sign, '-' if -ve, '+' if +ve */
    cyg_bool    pfzero,         /* prefix with zero ?           */
    cyg_ucount8 width           /* min width of number          */
    );

class Cyg_dummy_diag_init_class {
public:
    Cyg_dummy_diag_init_class() {
        HAL_DIAG_INIT();
    }
};

// Initialize after HAL.
static Cyg_dummy_diag_init_class cyg_dummy_diag_init_obj 
                                      CYGBLD_ATTRIB_INIT_AFTER(CYG_INIT_HAL);

/*----------------------------------------------------------------------*/
/* Write single char to output                                          */

externC void diag_write_char(char c)
{    
    /* Translate LF into CRLF */
    
    if( c == '\n' )
    {
        HAL_DIAG_WRITE_CHAR('\r');        
    }

    HAL_DIAG_WRITE_CHAR(c);
}

// Default wrapper function used by diag_printf
static void
_diag_write_char(char c, void **param)
{
    diag_write_char(c);
}

/*----------------------------------------------------------------------*/
/* Initialize. Call to pull in diag initializing constructor            */

externC void diag_init(void)
{
}

//
// This routine is used to send characters during 'printf()' functions.
// It can be replaced by providing a replacement via diag_init_putc().
//
static void (*_putc)(char c, void **param) = _diag_write_char;

void
diag_init_putc(void (*putc)(char c, void **param))
{
    _putc = putc;
}

/*----------------------------------------------------------------------*/
/* Write zero terminated string                                         */
  
externC void diag_write_string(const char *psz)
{
    while( *psz ) diag_write_char( *psz++ );
}

/*----------------------------------------------------------------------*/
/* Write decimal value                                                  */

externC void diag_write_dec( cyg_int32 n)
{
    cyg_ucount8 sign;

    if( n < 0 ) n = -n, sign = '-';
    else sign = '+';
    
    diag_write_num( n, 10, sign, false, 0);
}

/*----------------------------------------------------------------------*/
/* Write hexadecimal value                                              */

externC void diag_write_hex( cyg_uint32 n)
{
    diag_write_num( n, 16, '+', false, 0);
}    

/*----------------------------------------------------------------------*/
/* Generic number writing function                                      */
/* The parameters determine what radix is used, the signed-ness of the  */
/* number, its minimum width and whether it is zero or space filled on  */
/* the left.                                                            */

externC void diag_write_long_num(
    cyg_uint64  n,              /* number to write              */
    cyg_ucount8 base,           /* radix to write to            */
    cyg_ucount8 sign,           /* sign, '-' if -ve, '+' if +ve */
    cyg_bool    pfzero,         /* prefix with zero ?           */
    cyg_ucount8 width           /* min width of number          */
    )
{
    char buf[32];
    cyg_count8 bpos;
    char bufinit = pfzero?'0':' ';
    char *digits = (char *)"0123456789ABCDEF";

    /* init buffer to padding char: space or zero */
    for( bpos = 0; bpos < (cyg_count8)sizeof(buf); bpos++ ) buf[bpos] = bufinit;

    /* Set pos to start */
    bpos = 0;

    /* construct digits into buffer in reverse order */
    if( n == 0 ) buf[bpos++] = '0';
    else while( n != 0 )
    {
        cyg_ucount8 d = n % base;
        buf[bpos++] = digits[d];
        n /= base;
    }

    /* set pos to width if less. */
    if( (cyg_count8)width > bpos ) bpos = width;

    /* set sign if negative. */
    if( sign == '-' )
    {
        if( buf[bpos-1] == bufinit ) bpos--;
        buf[bpos] = sign;
    }
    else bpos--;

    /* Now write it out in correct order. */
    while( bpos >= 0 )
        diag_write_char(buf[bpos--]);
}

externC void diag_write_num(
    cyg_uint32  n,              /* number to write              */
    cyg_ucount8 base,           /* radix to write to            */
    cyg_ucount8 sign,           /* sign, '-' if -ve, '+' if +ve */
    cyg_bool    pfzero,         /* prefix with zero ?           */
    cyg_ucount8 width           /* min width of number          */
    )
{
    diag_write_long_num((long long)n, base, sign, pfzero, width);
}

/*----------------------------------------------------------------------*/
/* perform some simple sanity checks on a string to ensure that it      */
/* consists of printable characters and is of reasonable length.        */

static cyg_bool diag_check_string( const char *str )
{
    cyg_bool result = true;
    const char *s;

    if( str == NULL ) return false;
    
    for( s = str ; result && *s ; s++ )
    {
        char c = *s;

        /* Check for a reasonable length string. */
        
        if( s-str > 2048 ) result = false;

        /* We only really support CR, NL, tab and backspace at present.
	 * If we want to use other special chars, this test will
         * have to be expanded.  */

        if( c == '\n' || c == '\r' || c == '\b' || c == '\t' || c == '\033' )
            continue;

        /* Check for printable chars. This assumes ASCII */
        
        if( c < ' ' || c > '~' )
            result = false;

    }

    return result;
}

/*----------------------------------------------------------------------*/

static int
_cvt(unsigned long long val, char *buf, long radix, char *digits)
{
    char temp[80];
    char *cp = temp;
    int length = 0;

    if (val == 0) {
        /* Special case */
        *cp++ = '0';
    } else {
        while (val) {
            *cp++ = digits[val % radix];
            val /= radix;
        }
    }
    while (cp != temp) {
        *buf++ = *--cp;
        length++;
    }
    *buf = '\0';
    return (length);
}

#define is_digit(c) ((c >= '0') && (c <= '9'))

static int
_vprintf(void (*putc)(char c, void **param), void **param, const char *fmt, va_list ap)
{
    char buf[sizeof(long long)*8];
    char c, sign, *cp=buf;
    int left_prec, right_prec, zero_fill, pad, pad_on_right, 
        i, islong, islonglong;
    long long val = 0;
    int res = 0, length = 0;

    if (!diag_check_string(fmt)) {
        diag_write_string("<Bad format string: ");
        diag_write_hex((cyg_uint32)fmt);
        diag_write_string(" :");
        for( i = 0; i < 8; i++ ) {
            diag_write_char(' ');
            val = va_arg(ap, unsigned long);
            diag_write_hex(val);
        }
        diag_write_string(">\n");
        return 0;
    }
    while ((c = *fmt++) != '\0') {
        if (c == '%') {
            c = *fmt++;
            left_prec = right_prec = pad_on_right = islong = islonglong = 0;
            if (c == '-') {
                c = *fmt++;
                pad_on_right++;
            }
            if (c == '0') {
                zero_fill = true;
                c = *fmt++;
            } else {
                zero_fill = false;
            }
            while (is_digit(c)) {
                left_prec = (left_prec * 10) + (c - '0');
                c = *fmt++;
            }
            if (c == '.') {
                c = *fmt++;
                zero_fill++;
                while (is_digit(c)) {
                    right_prec = (right_prec * 10) + (c - '0');
                    c = *fmt++;
                }
            } else {
                right_prec = left_prec;
            }
            sign = '\0';
            if (c == 'l') {
                // 'long' qualifier
                c = *fmt++;
		islong = 1;
                if (c == 'l') {
                    // long long qualifier
                    c = *fmt++;
                    islonglong = 1;
                }
            }
            if (c == 'z') {
                c = *fmt++;
		islong = sizeof(size_t) == sizeof(long);
            }
            // Fetch value [numeric descriptors only]
            switch (c) {
            case 'p':
		islong = 1;
            case 'd':
            case 'D':
            case 'x':
            case 'X':
            case 'u':
            case 'U':
            case 'b':
            case 'B':
                if (islonglong) {
                    val = va_arg(ap, long long);
	        } else if (islong) {
                    val = (long long)va_arg(ap, long);
		} else{
                    val = (long long)va_arg(ap, int);
                }
                if ((c == 'd') || (c == 'D')) {
                    if (val < 0) {
                        sign = '-';
                        val = -val;
                    }
                } else {
                    // Mask to unsigned, sized quantity
                    if (islong) {
                        val &= ((long long)1 << (sizeof(long) * 8)) - 1;
                    } else if (!islonglong) { // no need to mask longlong
                        val &= ((long long)1 << (sizeof(int) * 8)) - 1;
                    }
                }
                break;
            default:
                break;
            }
            // Process output
            switch (c) {
            case 'p':  // Pointer
                (*putc)('0', param);
                (*putc)('x', param);
                zero_fill = true;
                left_prec = sizeof(unsigned long)*2;
                res += 2;  // Account for "0x" leadin
            case 'd':
            case 'D':
            case 'u':
            case 'U':
            case 'x':
            case 'X':
                switch (c) {
                case 'd':
                case 'D':
                case 'u':
                case 'U':
                    length = _cvt(val, buf, 10, (char *)"0123456789");
                    break;
                case 'p':
                case 'x':
                    length = _cvt(val, buf, 16, (char *)"0123456789abcdef");
                    break;
                case 'X':
                    length = _cvt(val, buf, 16, (char *)"0123456789ABCDEF");
                    break;
                }
                cp = buf;
                break;
            case 's':
            case 'S':
                cp = va_arg(ap, char *);
                if (cp == NULL) 
                    cp = (char *)"<null>";
                else if (!diag_check_string(cp)) {
                    diag_write_string("<Not a string: 0x");
                    diag_write_hex((cyg_uint32)cp);
                    cp = (char *)">";
                }
                length = 0;
                while (cp[length] != '\0') length++;
                break;
            case 'c':
            case 'C':
                c = va_arg(ap, int /*char*/);
                (*putc)(c, param);
                res++;
                continue;
            case 'b':
            case 'B':
                length = left_prec;
                if (left_prec == 0) {
                    if (islonglong)
                        length = sizeof(long long)*8;
                    else if (islong)
                        length = sizeof(long)*8;
                    else
                        length = sizeof(int)*8;
                }
                for (i = 0;  i < length-1;  i++) {
                    buf[i] = ((val & ((long long)1<<i)) ? '1' : '.');
                }
                cp = buf;
                break;
            case '%':
                (*putc)('%', param);
                res++;
                continue;
            default:
                (*putc)('%', param);
                (*putc)(c, param);
                res += 2;
                continue;
            }
            pad = left_prec - length;
            if (sign != '\0') {
                pad--;
            }
            if (zero_fill) {
                c = '0';
                if (sign != '\0') {
                    (*putc)(sign, param);
                    res++;
                    sign = '\0';
                }
            } else {
                c = ' ';
            }
            if (!pad_on_right) {
                while (pad-- > 0) {
                    (*putc)(c, param);
                    res++;
                }
            }
            if (sign != '\0') {
                (*putc)(sign, param);
                res++;
            }
            while (length-- > 0) {
                c = *cp++;
                (*putc)(c, param);
                res++;
            }
            if (pad_on_right) {
                while (pad-- > 0) {
                    (*putc)(' ', param);
                    res++;
                }
            }
        } else {
            (*putc)(c, param);
            res++;
        }
    }
    return (res);
}

struct _sputc_info {
    char *ptr;
    int max, len;
};

static void 
_sputc(char c, void **param)
{
    struct _sputc_info *info = (struct _sputc_info *)param;

    if (info->len < info->max) {
        *(info->ptr)++ = c;
        *(info->ptr) = '\0';
        info->len++;
    }
}

int
diag_sprintf(char *buf, const char *fmt, ...)
{        
    int ret;
    va_list ap;
    struct _sputc_info info;

    va_start(ap, fmt);
    info.ptr = buf;
    info.max = 1024;  // Unlimited
    info.len = 0;
    ret = _vprintf(_sputc, (void **)&info, fmt, ap);
    va_end(ap);
    return (info.len);
}

int
diag_snprintf(char *buf, size_t len, const char *fmt, ...)
{        
    int ret;
    va_list ap;
    struct _sputc_info info;

    va_start(ap, fmt);
    info.ptr = buf;
    info.len = 0;
    info.max = len-1;
    ret = _vprintf(_sputc, (void **)&info, fmt, ap);
    va_end(ap);
    return (info.len);
}

int 
diag_vsprintf(char *buf, const char *fmt, va_list ap)
{
    int ret;
    struct _sputc_info info;

    info.ptr = buf;
    info.max = 1024;  // Unlimited
    info.len = 0;
    ret = _vprintf(_sputc, (void **)&info, fmt, ap);
    return (info.len);
}

int
diag_printf(const char *fmt, ...)
{
    va_list ap;
    int ret;
    va_start(ap, fmt);
    ret = _vprintf(_putc, (void **)0, fmt, ap);
    va_end(ap);
    return (ret);
}

int
diag_vprintf(const char *fmt, va_list ap)
{
    int ret;

    ret = _vprintf(_putc, (void **)0, fmt, ap);
    return (ret);
}

void
diag_vdump_buf_with_offset(__printf_fun *pf,
                           cyg_uint8     *p, 
                           CYG_ADDRWORD   s, 
                           cyg_uint8     *base)
{
    int i, c;
    if ((CYG_ADDRWORD)s > (CYG_ADDRWORD)p) {
        s = (CYG_ADDRWORD)s - (CYG_ADDRWORD)p;
    }
    while ((int)s > 0) {
        if (base) {
            (*pf)("%08X: ", (CYG_ADDRWORD)p - (CYG_ADDRWORD)base);
        } else {
            (*pf)("%08X: ", p);
        }
        for (i = 0;  i < 16;  i++) {
            if (i < (int)s) {
                (*pf)("%02X ", p[i] & 0xFF);
            } else {
                (*pf)("   ");
            }
	    if (i == 7) (*pf)(" ");
        }
        (*pf)(" |");
        for (i = 0;  i < 16;  i++) {
            if (i < (int)s) {
                c = p[i] & 0xFF;
                if ((c < 0x20) || (c >= 0x7F)) c = '.';
            } else {
                c = ' ';
            }
            (*pf)("%c", c);
        }
        (*pf)("|\n");
        s -= 16;
        p += 16;
    }
}

void
diag_dump_buf_with_offset(cyg_uint8     *p, 
                          CYG_ADDRWORD   s, 
                          cyg_uint8     *base)
{
    diag_vdump_buf_with_offset(diag_printf, p, s, base);
}

void
diag_dump_buf(void *p, CYG_ADDRWORD s)
{
   diag_dump_buf_with_offset((cyg_uint8 *)p, s, 0);
}

void
diag_dump_buf_with_offset_32bit(cyg_uint32   *p, 
				CYG_ADDRWORD  s, 
				cyg_uint32   *base)
{
    int i;
    if ((CYG_ADDRWORD)s > (CYG_ADDRWORD)p) {
        s = (CYG_ADDRWORD)s - (CYG_ADDRWORD)p;
    }
    while ((int)s > 0) {
        if (base) {
            diag_printf("%08X: ", (CYG_ADDRWORD)p - (CYG_ADDRWORD)base);
        } else {
            diag_printf("%08X: ", (CYG_ADDRWORD)p);
        }
        for (i = 0;  i < 4;  i++) {
            if (i < (int)s/4) {
                diag_printf("%08X ", p[i] );
            } else {
                diag_printf("         ");
            }
        }
        diag_printf("\n");
        s -= 16;
        p += 4;
    }
}

externC void
diag_dump_buf_32bit(void *p, CYG_ADDRWORD s)
{
   diag_dump_buf_with_offset_32bit((cyg_uint32 *)p, s, 0);
}

void
diag_dump_buf_with_offset_16bit(cyg_uint16   *p, 
				CYG_ADDRWORD  s, 
				cyg_uint16   *base)
{
    int i;
    if ((CYG_ADDRWORD)s > (CYG_ADDRWORD)p) {
        s = (CYG_ADDRWORD)s - (CYG_ADDRWORD)p;
    }
    while ((int)s > 0) {
        if (base) {
            diag_printf("%08X: ", (CYG_ADDRWORD)p - (CYG_ADDRWORD)base);
        } else {
            diag_printf("%08X: ", (CYG_ADDRWORD)p);
        }
        for (i = 0;  i < 8;  i++) {
            if (i < (int)s/2) {
	      diag_printf("%04X ", p[i] );
	      if (i == 3) diag_printf(" ");
            } else {
	      diag_printf("     ");
            }
        }
        diag_printf("\n");
        s -= 16;
        p += 8;
    }
}

externC void
diag_dump_buf_16bit(void *p, CYG_ADDRWORD s)
{
   diag_dump_buf_with_offset_16bit((cyg_uint16 *)p, s, 0);
} 
#ifdef RTK_SYSLOG_SUPPORT
/************************system log**********************/
////////////////////////////////////////////////////////

#define CONFIG_LOG_BUF_SHIFT 12
#define __LOG_BUF_LEN	(1 << CONFIG_LOG_BUF_SHIFT)

/* printk's without a loglevel use this.. */
#define DEFAULT_MESSAGE_LOGLEVEL 4 /* KERN_WARNING */

/* We show everything that is MORE important than this.. */
#define MINIMUM_CONSOLE_LOGLEVEL 1 /* Minimum loglevel we let people use */
#define DEFAULT_CONSOLE_LOGLEVEL 7 /* anything MORE serious than KERN_DEBUG */
#define default_message_loglevel DEFAULT_MESSAGE_LOGLEVEL
#define LOG_BUF_MASK (log_buf_len-1)
#define LOG_BUF(idx) (log_buf[(idx) & LOG_BUF_MASK])


 
/*
 * The indices into log_buf are not constrained to log_buf_len - they
 * must be masked before subscripting
 */

static unsigned log_start;	/* Index into log_buf: next char to be read by syslog() */
static unsigned con_start;	/* Index into log_buf: next char to be sent to consoles */
static unsigned log_end;	/* Index into log_buf: most-recently-written-char + 1 */

static int printk_time = 1;



//#ifdef CONFIG_PRINTK_FUNC
static char __log_buf[__LOG_BUF_LEN];
static char *log_buf = __log_buf;
static int log_buf_len = __LOG_BUF_LEN;
static unsigned logged_chars; /* Number of chars produced since last read+clear operation */

static void emit_log_char(char c,int prt_to_con)
{
	LOG_BUF(log_end) = c;
	log_end++;
	if (log_end - log_start > log_buf_len)
		log_start = log_end - log_buf_len;
	if (log_end - con_start > log_buf_len)
		con_start = log_end - log_buf_len;
	if (logged_chars < log_buf_len)
		logged_chars++;
}



static int recursion_bug;
static int new_text_line = 1;
static char printk_buf[1024];

externC int scrlog_vprintk(const char *fmt, va_list args)
{
	int printed_len = 0;
	unsigned long flags;
	int this_cpu;
	char *p;




	/* Emit the output into the temporary buffer */
	printed_len += vsnprintf(printk_buf,1024,fmt, args);


	/*
	 * Copy the output into log_buf.  If the caller didn't provide
	 * appropriate log level tags, we insert them here
	 */
	 if(printk_buf[0]&&printk_time)
	 {
		/* Follow the token with the time */
		char tbuf[50],*strtime = NULL, *tp;
		unsigned tlen;
		unsigned long long t;
		unsigned long nanosec_rem;
		time_t time_now={0};
		if(time(&time_now)<0)
		{
			return -1;	
		}
		strtime=ctime(&time_now) + 4;//* skip day of week */
		strtime[15]='\0';
		tlen = sprintf(tbuf,"%s\t",strtime);

		for (tp = tbuf; tp < tbuf + tlen; tp++)
			emit_log_char(*tp,0);
		printed_len += tlen;
			
	 }
	for (p = printk_buf; *p; p++) {
		if (!*p)
			break;
		emit_log_char(*p,0);
	}

	return printed_len;
}

externC int scrlog_printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = scrlog_vprintk(fmt, args);
	va_end(args);

	return r;
}
externC int do_syslog(int type, char *buf, int len)
{
	unsigned i, j, limit, count;
	int do_clear = 0;
	char c;
	int error = 0;

	switch (type) {
	case SYS_LOG_CLOSE:		/* Close log */
		break;
	case SYS_LOG_OPEN:		/* Open log */
		break;
	case SYS_LOG_READ:		/* Read from log */
		error = -EINVAL;
		if (!buf || len < 0)
			goto out;
		error = 0;
		if (!len)
			goto out;
		if (error)
			goto out;
		i = 0;
		while (!error && (log_start != log_end) && i < len) {
			c = LOG_BUF(log_start);
			log_start++;
			*buf = c;
			buf++;
			i++;
		}
		if (!error)
			error = i;
		buf[i] = 0;
		break;
#if 0	
	case SYS_LOG_CLEAR:		/* Read/clear last kernel messages */
		do_clear = 1;
		/* FALL THRU */

	case 3:		/* Read last kernel messages */
		error = -EINVAL;
		if (!buf || len < 0)
			goto out;
		error = 0;
		if (!len)
			goto out;
		count = len;
		if (count > log_buf_len)
			count = log_buf_len;
		if (count > logged_chars)
			count = logged_chars;
		if (do_clear)
			logged_chars = 0;
		limit = log_end;
		/*
		 * __put_user() could sleep, and while we sleep
		 * printk() could overwrite the messages
		 * we try to copy to user space. Therefore
		 * the messages are copied in reverse. <manfreds>
		 */
		for (i = 0; i < count && !error; i++) {
			j = limit-1-i;
			if (j + log_buf_len < log_end)
				break;
			c = LOG_BUF(j);
			buf[count-1-i] = c;
		}
		if (error)
			break;
		error = i;
		if (i != count) {	
					error = -EFAULT;
					break;	
			}
		}
		break;

	case 5:		/* Clear ring buffer */
		logged_chars = 0;
		break;
#endif		
	case SYS_LOG_NUMBER:		/* Number of chars in the log buffer */
		error = log_end - log_start;
		break;
	case SYS_LOG_BUF_LEN:	/* Size of the log buffer */
		error = log_buf_len;
		break;
	default:
		error = -EINVAL;
		break;
	}
out:
	return error;
}

///////////////////////////////////////////////////////////////////


externC void setSyslogType(int syslog_type)
{
	syslogType=syslog_type;
}

#ifdef RTK_REMOTELOG_SUPPORT
externC void setRemotelogEnable(int remotelog_enable,void* remotelog_ip)
{
	remotelogEnable = remotelog_enable;
	remoteSvrIp = *((struct in_addr*)remotelog_ip);
}

externC void remote_log_snd(char * datBuff)
{
	if(!remotelogEnable)
		return;
	
	int fd,send_succ,len;

	//open socket and send
	struct sockaddr_in remoteAddr;
	int one =1;
	fd= socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == fd)
		return;
	len = strlen(datBuff);
	memset(&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_port = htons(514);
	remoteAddr.sin_family = AF_INET;
	memcpy(&remoteAddr.sin_addr,&remoteSvrIp,4);
	send_succ = (sendto(fd, datBuff, len, 0,(struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) >= 0);
	//printf("%s,%d,: len=%d ip=%s,send_succ=%d,errno=%d\n",__FUNCTION__,__LINE__,len,inet_ntoa(remoteSvrIp),send_succ,errno);
	close(fd);
	return;
}
#endif

externC int syslog_printk(char * buffer)
{
	int logFile=-1;
	int writeCount=0,writeTotal=0;
	struct stat fileStat={0};
	char timeBuff[32]={0};
	char *strTime=NULL;
	time_t time_now={0};
	//diag_printf("%s:%d buffer=%s\n",__FILE__,__LINE__,buffer);
	stat(SYSLOG_FILE_NAME,&fileStat);
	if(fileStat.st_size>SYSLOG_FILE_SIZE_MAX)
	{
		unlink(SYSLOG_FILE_BAK_NAME);
		rename(SYSLOG_FILE_NAME,SYSLOG_FILE_BAK_NAME);
	}
	
	logFile=open(SYSLOG_FILE_NAME,O_WRONLY|O_APPEND|O_CREAT,0644);
	if (logFile<0) {
		diag_printf("open log file fail!Can't show system log!\n");
		return -1;
	}
	writeCount=write(logFile,buffer,strlen(buffer));
	if(writeCount<0)
	{
		diag_printf("write log file fail!\n");
		close(logFile);
		return -1;
	}
	writeTotal+=writeCount;
	close(logFile);
#ifdef RTK_REMOTELOG_SUPPORT
	remote_log_snd(buffer);
#endif	
	return writeTotal;
}
externC int syslogAll_printk(const char *fmt, ...)
{
	int r;

	if((syslogType&SYSLOG_ENABLE)&&
		(syslogType&SYSLOG_ALL_ENABLE))
	{	
		char buffer[SYSLOG_BUFFER_SIZE]={0};
		va_list args;
		va_start(args, fmt);
		r = scrlog_vprintk(fmt,args);
		va_end(args);
		return r;
	}
	else
		return 0;
}
externC int wlanlog_printk(const char *fmt, ...)
{
	int r;
	if((syslogType&SYSLOG_ENABLE)&&
		((syslogType&SYSLOG_WLANLOG_ENABLE)
		||(syslogType&SYSLOG_ALL_ENABLE)))
	{
		va_list args;
		va_start(args, fmt);
		r = scrlog_vprintk(fmt,args);
		va_end(args);
		//diag_printf("%s:%d buffer=%s\n",__FILE__,__LINE__,buffer);
		return r;
	}
	else
	{
		return 0;
	}
}

externC int doslog_printk(const char *fmt, ...)
{
	int r;
	if((syslogType&SYSLOG_ENABLE)&&
		((syslogType&SYSLOG_DOSLOG_ENABLE)
		||(syslogType&SYSLOG_ALL_ENABLE)))
	{
		va_list args;
		va_start(args, fmt);
		r = scrlog_vprintk(fmt,args);
		va_end(args);
		//diag_printf("%s:%d buffer=%s\n",__FILE__,__LINE__,buffer);
		return r;
	}
	else
	{
		return 0;
	}
}
#endif

/*-----------------------------------------------------------------------*/
/* EOF infra/diag.c */
