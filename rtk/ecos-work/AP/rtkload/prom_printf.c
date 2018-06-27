/* 
 * Borrowed from arch/mips/r39xx/prom/init.c
 *
 */

/*
 * init.c: early initialisation code for R39XX Class PDAs
 *
 * Copyright (C) 1999 Harald Koerfgen
 *
 * $Id: prom_printf.c,v 1.2 2008/08/04 08:54:44 michael Exp $
 */



#define CONFIG_SERIAL

#include <stdarg.h>

#include "config.h"
//#include <linux/autoconf.h>

#ifdef CONFIG_RTL_EB8186
#include <asm/rtl8181.h>
#endif

#ifdef CONFIG_RTL_8197F
#define BSP_UART0_BASE		0xB8147000
#define BSP_UART0_THR        (BSP_UART0_BASE + 0x024)
#define BSP_UART0_LSR       (BSP_UART0_BASE + 0x014)
#define REG8(reg)		(*(volatile unsigned char  *)(reg))
#else
#define UART_THR	0xb8002000	
#define UART_LSR	0xb8002014
#endif
#define REG32(reg)   (*(volatile unsigned int   *)((unsigned int)reg))

extern int vsprintf(char *buf, const char *fmt, va_list args);

void serial_outc(char c)
{
	int i=0;
	
	while (1)
	{
		i++;
		if (i >=0x6000)
			break;

#ifdef CONFIG_RTL_8197F
		if (REG8(BSP_UART0_LSR) & 0x20)
			break;
#else
		if (REG32(UART_LSR) & 0x20000000)
        		break;
#endif
	}
#ifdef CONFIG_RTL_8197F
	REG8(BSP_UART0_THR) = c;
#else
	REG32(UART_THR) = (c << 24);
#endif
}
 

/*
 * Helpful for debugging :-)
 */
int prom_printf(const char * fmt, ...)
{
#ifdef CONFIG_SERIAL
	//extern void serial_outc(char);
	static char buf[1024];
	va_list args;
	char c;
	int i = 0;

	/*
	 * Printing messages via serial port
	 */
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	for (i = 0; buf[i] != '\0'; i++) {
		c = buf[i];
		if (c == '\n')
			serial_outc('\r');
		serial_outc(c);
	}

	return i;
#else
	return 0;
#endif
}
