/*
 * $ Copyright Open Broadcom Corporation $
 */

#ifndef _PCIDRV_H_
#define _PCIDRV_H_

#include <cyg/hal/hal_tables.h>
#include <cyg/hal/pci/bcmpci.h>

typedef struct pci_driver CYG_HAL_TABLE_TYPE pci_driver_t;

extern pci_driver_t __PCIDRVTAB__[], __PCIDRVTAB_END__;

#define PCIDRVTAB_ENTRY(_l, _name, _id_table, _probe, _remove, _save_state, _suspend, _resume, _enable_wake) \
pci_driver_t _l CYG_HAL_TABLE_ENTRY(pcidrv) = {		\
	name:			_name,				\
	id_table:		_id_table,			\
	probe:			_probe,				\
	remove:			_remove,			\
	save_state:		_save_state,		\
	suspend:		_suspend,			\
	resume:			_resume,			\
	enable_wake:	_enable_wake,		\
};


#endif /* _PCIDRV_H_ */
