#ifndef CYGONCE_INFRA_DIAG_H
#define CYGONCE_INFRA_DIAG_H

/*=============================================================================
//
//      diag.h
//
//      Diagnostic Routines for Infra Development
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg, gthomas
// Date:        1998-03-02
// Purpose:     Diagnostic Routines for Infra Development
// Description: Diagnostic routines for use during infra development.
// Usage:       #include <cyg/infra/diag.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================*/

#include <pkgconf/infra.h>
#include <cyg/infra/cyg_type.h>
#include <stdarg.h>

/*---------------------------------------------------------------------------*/
/* Diagnostic routines                                                       */

externC void diag_init(void);         /* Initialize, call before any others*/

externC void diag_write_char(char c); /* Write single char to output       */

externC void diag_write_string(const char *psz); /* Write zero terminated string */

externC void diag_write_dec( cyg_int32 n);    /* Write decimal value       */

externC void diag_write_hex( cyg_uint32 n);   /* Write hexadecimal value   */

externC void diag_dump_buf(void *buf, CYG_ADDRWORD len);
externC void diag_dump_buf_32bit(void *buf, CYG_ADDRWORD len);
externC void diag_dump_buf_16bit(void *buf, CYG_ADDRWORD len);
typedef int __printf_fun(const char *fmt, ...);
externC void diag_vdump_buf_with_offset(__printf_fun *pf,
                                        cyg_uint8     *p, 
                                        CYG_ADDRWORD   s, 
                                        cyg_uint8     *base);
externC void diag_dump_buf_with_offset(cyg_uint8     *p, 
                                       CYG_ADDRWORD   s, 
                                       cyg_uint8     *base);

externC void diag_dump_buf_with_offset_32bit(cyg_uint32 *p, 
                                             CYG_ADDRWORD     s, 
                                             cyg_uint32      *base);

externC void diag_dump_buf_with_offset_16bit(cyg_uint16 *p, 
                                             CYG_ADDRWORD     s, 
                                             cyg_uint16      *base);

/* Formatted print      */
externC int  diag_printf( const char *fmt, ... ) CYGBLD_ATTRIB_PRINTF_FORMAT(1,2);  

externC void diag_init_putc(void (*putc)(char c, void **param));
externC int  diag_sprintf(char *buf, const char *fmt, ...) 
     CYGBLD_ATTRIB_PRINTF_FORMAT(2,3);
externC int  diag_snprintf(char *buf, size_t len, const char *fmt, ...)
     CYGBLD_ATTRIB_PRINTF_FORMAT(3,4);
externC int  diag_vsprintf(char *buf, const char *fmt, va_list ap)
     CYGBLD_ATTRIB_PRINTF_FORMAT(2,0);
externC int  diag_vprintf(const char *fmt, va_list ap)
     CYGBLD_ATTRIB_PRINTF_FORMAT(1,0);

#ifdef RTK_SYSLOG_SUPPORT
#define SYS_LOG_OPEN	1
#define SYS_LOG_CLOSE	2
#define SYS_LOG_READ	3
//#define SYS_LOG_CLEAR	4
#define SYS_LOG_NUMBER		5
#define	SYS_LOG_BUF_LEN	6

#define SYSLOG_BUFFER_SIZE 512
#define SYSLOG_FILE_SIZE_MAX 65536//set log max 64K
#define SYSLOG_FILE_NAME "/tmp/syslog.log"
#define SYSLOG_FILE_BAK_NAME "/tmp/syslog.bak"

typedef enum
{
	SYSLOG_DISABLE=0,
	SYSLOG_ENABLE=1,
	SYSLOG_ALL_ENABLE=2,
	SYSLOG_WLANLOG_ENABLE=4,
	SYSLOG_DOSLOG_ENABLE=8,
	SYSLOG_MESHLOG_ENABLE=16
} SYSLOG_T;
externC int syslogAll_printk(const char *fmt, ...);
externC int wlanlog_printk(const char *fmt, ...);
externC int doslog_printk(const char *fmt, ...);
externC void setSyslogType(int syslog_type);


#ifdef RTK_REMOTELOG_SUPPORT
externC void setRemotelogEnable(int remotelog_enable,void* remotelog_ip);
#endif 

#else
#define syslogAll_printk(fmt, args...)
#define wlanlog_printk(fmt, args...)
#define doslog_printk(fmt, args...)
#endif

/*---------------------------------------------------------------------------*/
/* Internal Diagnostic MACROS                                                */

#define DIAG_DEVICE_START_SYNC()
#define DIAG_DEVICE_END_SYNC()

/*---------------------------------------------------------------------------*/
#endif /* CYGONCE_INFRA_DIAG_H */
/* EOF diag.h */
