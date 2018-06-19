/*
 * BCM47XX runtime code entry point
 *
 * $ Copyright Open Broadcom Corporation $
 * 
 * $Id: start.S,v 1.1 2010-03-30 04:17:53 musterc Exp $
 *
 */
#include <pkgconf/hal.h>
#include <cyg/hal/arch.inc>

	.section ".jump_to_start","ax"

FUNC_START(start)
	.extern	_start
/*
	a small trick is here. we need to know the image storage location (a1)
	and the link address (a0). to get it at run time, the ra will save
	the return address when "bal" (branch and link). so, the "get_label" will
	is saved to ra, then (a1) is known. the link address (a0) is known
	at the linkage time.
*/
	bal	get_label		# !!! this line is not redundant , the pc+8 (1 nop) saved to ra
	nop
get_label:
	add	a1, ra, 0		# a1 is the current pc which is on the flash
	la	a0, get_label		# a0 is the link address
	beq	a0, a1, 2f
	nop				# to avoid warning message "Macro instruction expanded into multiple instructions in a branch delay slot"
	.global	_edata
	la	a2, _edata
1:
	lw	t0, (a1)
	lw	t1, 4(a1)
	lw	t2, 8(a1)
	lw	t3, 12(a1)
	sw	t0, (a0)
	sw	t1, 4(a0)
	sw	t2, 8(a0)
	sw	t3, 12(a0)
	addu	a1, 16
	addu	a0, 16
	ble	a0, a2, 1b
	nop
2:
	move    s0, zero
	mtc0    s0, $12
	nop
	sll	zero, zero, 3
	la	a0, _start
	jr	a0
	nop
FUNC_END(start)
