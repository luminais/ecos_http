/*
 * $ Copyright Open Broadcom Corporation $
 */

//==========================================================================
//
//      devs/serial/mips/bcm47xx/include/mips_bcm47xx_ser.inl
//
//      BCM47XX Serial I/O definitions.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:gthomas, jskov
// Date:        2000-05-24
// Purpose:     REF4955 Serial definitions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>

//-----------------------------------------------------------------------------
// There are two serial ports.

//-----------------------------------------------------------------------------
#if 0
#define BAUD_DIVISOR(_x_) (1843200+(8*(_x_)))/(16*(_x_))

static unsigned short select_baud[] = {
    0,    // Unused
    0,    // 50
    0,    // 75
    0,    // 110
    0,    // 134.5
    0,    // 150
    0,    // 200
    0,    // 300
    0,    // 600
    BAUD_DIVISOR(1200),
    0,    // 1800
    BAUD_DIVISOR(2400),
    0,    // 3600
    BAUD_DIVISOR(4800),
    0 ,   // 7200
    BAUD_DIVISOR(9600),
    BAUD_DIVISOR(14400),
    BAUD_DIVISOR(19200),
    BAUD_DIVISOR(38400),
    BAUD_DIVISOR(57600),
    BAUD_DIVISOR(115200),
    0,    // 230400
};
#else
#define BRTC(base, baud) ((base + (8 * baud)) / (16*(baud)))

static unsigned int select_baud[] = {
    0,    // Unused
    0,    // 50
    0,    // 75
    0,    // 110
    0,    // 134.5
    0,    // 150
    0,    // 200
    0,    // 300
    0,    // 600
    1200,
    0,    // 1800
    2400,
    0,    // 3600
    4800,
    0,   // 7200
    9600,
    14400,
    19200,
    38400,
    57600,
    115200,
    0,    // 230400
};
#endif

//-----------------------------------------------------------------------------
// Port 0 descriptors

#ifdef CYGPKG_IO_SERIAL_MIPS_BCM47XX_SERIAL0
pc_serial_info pc_serial_info0 = {0xb8000300, CYGNUM_HAL_INTERRUPT_UART1};

void
pc_serial_info0_add(void *regs, cyg_uint32 irq, cyg_uint32 baud_base, cyg_uint32 reg_shift)
{
	int i, num = sizeof(select_baud)/sizeof(unsigned int);

	pc_serial_info0.base = (int)regs;
	pc_serial_info0.int_num = irq;

	for (i = 0; i < num; i++) {
		if (select_baud[i] != 0)
			select_baud[i] = BRTC(baud_base, select_baud[i]);
	}
}

#if CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL0_BUFSIZE > 0
static unsigned char pc_serial_out_buf0[CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL0_BUFSIZE];
static unsigned char pc_serial_in_buf0[CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(pc_serial_channel0,
                                       pc_serial_funs, 
                                       pc_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &pc_serial_out_buf0[0], 
                                       sizeof(pc_serial_out_buf0),
                                       &pc_serial_in_buf0[0], 
                                       sizeof(pc_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(pc_serial_channel0,
                      pc_serial_funs, 
                      pc_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(pc_serial_io0, 
             CYGDAT_IO_SERIAL_MIPS_BCM47XX_SERIAL0_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &pc_serial_channel0
    );
#endif

//-----------------------------------------------------------------------------
// Port 1 descriptors

#ifdef CYGPKG_IO_SERIAL_MIPS_BCM47XX_SERIAL1
pc_serial_info pc_serial_info1;

void
pc_serial_info1_add(void *regs, cyg_uint32 irq, cyg_uint32 baud_base, cyg_uint32 reg_shift)
{

        pc_serial_info1.base = regs;
	pc_serial_info1.int_num = irq;
}

#if CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL1_BUFSIZE > 0
static unsigned char pc_serial_out_buf1[CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL1_BUFSIZE];
static unsigned char pc_serial_in_buf1[CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(pc_serial_channel1,
                                       pc_serial_funs, 
                                       pc_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &pc_serial_out_buf1[0], 
                                       sizeof(pc_serial_out_buf1),
                                       &pc_serial_in_buf1[0], 
                                       sizeof(pc_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(pc_serial_channel1,
                      pc_serial_funs, 
                      pc_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_BCM47XX_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(pc_serial_io1, 
             CYGDAT_IO_SERIAL_MIPS_BCM47XX_SERIAL1_NAME,
             0,                   // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &pc_serial_channel1
    );
#endif

// EOF mips_bcm47xx_ser.inl
