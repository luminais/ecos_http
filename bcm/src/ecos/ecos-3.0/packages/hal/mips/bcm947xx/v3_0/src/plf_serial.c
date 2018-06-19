// Copyright (c) 2004-2005 RealNet Solutions, Inc., All Rights Reserved
//=============================================================================
//
//      ns16552.c
//
//      Simple driver for the NS16652 serial controller
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   
// Contributors:
// Date:        
// Description: Simple HAL driver for the NS16552 serial controller
// Note:        To drop into a new HAL, you should only have to change the
//              few parameters here at the top.
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions

//-----------------------------------------------------------------------------

//#define NS_SERIAL_A_BASE 0xbd000020
//#define NS_SERIAL_A_IRQ  CYGNUM_HAL_INTERRUPT_UART1
//#define NS_SERIAL_B_BASE 0xbd000000
//#define NS_SERIAL_B_IRQ  CYGNUM_HAL_INTERRUPT_UART2
#define NS_SERIAL_A_BASE 0xb8000300
#define NS_SERIAL_A_IRQ  CYGNUM_HAL_INTERRUPT_UART1
#define NS_SERIAL_B_BASE 0xb8000400
#define NS_SERIAL_B_IRQ  CYGNUM_HAL_INTERRUPT_UART2

#define NS_CLOCK 1843200

int num_tty  = 0;

#define CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD  115200

// Fill in extra code as required
#define NS_EXTRA_INIT()                         \
    CYG_MACRO_START                             \
    CYG_MACRO_END

//-----------------------------------------------------------------------------


#if 0
#define CYG_DEVICE_SERIAL_BAUD_MSB (((NS_CLOCK / (16*(CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD))) >> 8) & 0xff)
#define CYG_DEVICE_SERIAL_BAUD_LSB  ((NS_CLOCK / (16*(CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD))) & 0xff)
#else
#define BRTC(base, baud) ((base + (8 * baud)) / (16*(baud)))

static unsigned int serial_baud_msb = BRTC(NS_CLOCK, CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD) >> 8;
static unsigned int serial_baud_lsb = BRTC(NS_CLOCK, CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD) & 0xff;
#endif

#if 0
// Define the serial registers.
#define CYG_DEV_RBR 0x00   // receiver buffer register, read, dlab = 0
#define CYG_DEV_THR 0x00   // transmitter holding register, write, dlab = 0
#define CYG_DEV_DLL 0x00   // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_IER 0x04   // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_DLM 0x04   // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_IIR 0x08   // interrupt identification register, read, dlab = 0
#define CYG_DEV_FCR 0x08   // fifo control register, write, dlab = 0
#define CYG_DEV_LCR 0x0C   // line control register, read/write
#define CYG_DEV_MCR 0x10   // modem control register, read/write
#define CYG_DEV_LSR 0x14   // line status register, read
#define CYG_DEV_MSR 0x18   // modem status register, read
#else
#define CYG_DEV_RBR 0x00   // receiver buffer register, read, dlab = 0
#define CYG_DEV_THR 0x00   // transmitter holding register, write, dlab = 0
#define CYG_DEV_DLL 0x00   // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_IER 0x01   // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_DLM 0x01   // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_IIR 0x02   // interrupt identification register, read, dlab = 0
#define CYG_DEV_FCR 0x02   // fifo control register, write, dlab = 0
#define CYG_DEV_LCR 0x03   // line control register, read/write
#define CYG_DEV_MCR 0x04   // modem control register, read/write
#define CYG_DEV_LSR 0x05   // line status register, read
#define CYG_DEV_MSR 0x06   // modem status register, read
#endif

// Interrupt Enable Register
#define SIO_IER_RCV 0x01
#define SIO_IER_XMT 0x02
#define SIO_IER_LS  0x04
#define SIO_IER_MS  0x08

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// Modem Control Register
#define SIO_MCR_DTR 0x01
#define SIO_MCR_RTS 0x02
#define SIO_MCR_INT 0x08   // Enable interrupts

#define	IIR_FIFO_MASK	0xc0	/* set if FIFOs are enabled */

//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

//-----------------------------------------------------------------------------

static void serial_out8(unsigned long addr, cyg_uint8 value)
{
	HAL_WRITE_UINT8(addr, value);
	/* PR13509 WAR: Read SB location after UART write */
	*((volatile unsigned int *) CYGARC_UNCACHED_ADDRESS(0x18000000));
}

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lcr;
    cyg_uint32 i;
    
#if 1
    lcr = SIO_LCR_DLAB;
    serial_out8(base+CYG_DEV_LCR, lcr);
    
    serial_out8(base+CYG_DEV_DLL, serial_baud_lsb);
    serial_out8(base+CYG_DEV_DLM, serial_baud_msb);
    
    // 8-1-no parity.
    serial_out8(base+CYG_DEV_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);
    
    serial_out8(base+CYG_DEV_IER, 0);

    serial_out8(base+CYG_DEV_FCR, 0x01);  // Enable
    for (i= 0; i < 1000000; i++);
    serial_out8(base+CYG_DEV_FCR, 0x07);  // Enable & clear FIFO
    for (i= 0; i < 1000000; i++);
    HAL_READ_UINT8(base+CYG_DEV_IIR, lcr);
    if ((lcr & IIR_FIFO_MASK) != IIR_FIFO_MASK)
        serial_out8(base+CYG_DEV_FCR, 0x00);  // Enable        
    
#endif
    NS_EXTRA_INIT();
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();
#if 1
    do {
        HAL_READ_UINT8(base+CYG_DEV_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    serial_out8(base+CYG_DEV_THR, c);
#endif
    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
#if 1
    HAL_READ_UINT8(base+CYG_DEV_LSR, lsr);
    if ((lsr & SIO_LSR_DR) == 0)
        return false;

    HAL_READ_UINT8(base+CYG_DEV_RBR, *ch);
#endif
    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static channel_data_t ns_ser_channels[2]
#if 1
= {
    { (cyg_uint8*)NS_SERIAL_A_BASE, 1000, NS_SERIAL_A_IRQ },
    { (cyg_uint8*)NS_SERIAL_B_BASE, 1000, NS_SERIAL_B_IRQ }
};
#endif



static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        serial_out8(chan->base+CYG_DEV_IER, SIO_IER_RCV);
        serial_out8(chan->base+CYG_DEV_MCR, SIO_MCR_INT|SIO_MCR_DTR|SIO_MCR_RTS);

        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        serial_out8(chan->base+CYG_DEV_IER, 0);

        HAL_INTERRUPT_MASK(chan->isr_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    char c;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    HAL_READ_UINT8(chan->base+CYG_DEV_LSR, lsr);
    if ( (lsr & SIO_LSR_DR) != 0 ) {

        HAL_READ_UINT8(chan->base+CYG_DEV_RBR, c);
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Disable interrupts.
    HAL_INTERRUPT_MASK(ns_ser_channels[0].isr_vector);
#if 0
    HAL_INTERRUPT_MASK(ns_ser_channels[1].isr_vector);
#endif
    // Init channels
    cyg_hal_plf_serial_init_channel(&ns_ser_channels[0]);
#if 0
    cyg_hal_plf_serial_init_channel(&ns_ser_channels[1]);
#endif
    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ns_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

#if 0    
    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ns_ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
#endif
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void
cyg_hal_plf_serial_add(void *regs, cyg_uint32 irq, cyg_uint32 baud_base, cyg_uint32 reg_shift)
{
	unsigned int brtc;
	if (num_tty == 0)
	{
		brtc = BRTC(baud_base, CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD);
		serial_baud_msb = brtc >> 8;
		serial_baud_lsb = brtc & 0xff;
	}
    ns_ser_channels[num_tty].base = regs;
    ns_ser_channels[num_tty].isr_vector = irq;
    num_tty++;
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// End of ns16552.c
