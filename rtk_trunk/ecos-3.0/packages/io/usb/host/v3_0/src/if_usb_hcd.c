#include <cyg/io/usb_sys/types.h>
#include <cyg/io/devtab.h>
#include <pkgconf/io_usb_host.h>
#include <cyg/io/usb_sys/bus.h>
#include <cyg/io/usb_sys/linker_set.h>

#include <cyg/io/usb_sys/param.h>
#include <cyg/io/usb_sys/kernel.h>
#include <cyg/kernel/kapi.h>

cyg_mutex_t Giant;
cyg_mutex_t	kenv_lock;
int cold = 1;
unsigned char usb_hcd_flag = 0;

/*for kobj mutex*/

static bool if_usb_hcd_init(struct cyg_devtab_entry *tab);
struct sysinit **sysinit = NULL, **sysinit_end = NULL;

CYG_HAL_TABLE_BEGIN(__start_set_sysinit_set,sysinit_set);
CYG_HAL_TABLE_END(__stop_set_sysinit_set,sysinit_set);

extern struct sysinit __start_set_sysinit_set;
extern struct sysinit __stop_set_sysinit_set;

DEVTAB_ENTRY(rltk_usb_hcd, 
			CYGDAT_RLTK_USB_HCD, 
			0, 
			NULL, 
			if_usb_hcd_init, 
			NULL, 
			NULL
);

void
mi_startup(void)
{

	struct sysinit **sipp;		/* system initialization*/
	struct sysinit **xipp;		/* interior loop of sort*/
	struct sysinit *save;		/* bubble*/

	struct sysinit **saves,**saves_free;
	int num;
	struct sysinit *tmp;
	struct sysinit *tmp_end;
	struct sysinit *sip;

	if (sysinit == NULL) {
		tmp = &__start_set_sysinit_set;
		tmp_end = &__stop_set_sysinit_set;

		num = ((int)(tmp_end) - (int)(tmp))/(sizeof(struct sysinit)) ;
		saves = (struct sysinit**)malloc(num*4);
		saves_free = saves;
		sysinit = saves;

		for(sip = tmp;sip < tmp_end;sip++)
		{
			*saves = sip;
			saves++;
		}
		sysinit_end = saves;	
	}

restart:
	/*
	 * Perform a bubble sort of the system initialization objects by
	 * their subsystem (primary key) and order (secondary key).
	 */
	 
	for (sipp = sysinit; sipp < sysinit_end; sipp++) {
			for (xipp = sipp + 1; xipp < sysinit_end; xipp++) {
				if ((*sipp)->subsystem < (*xipp)->subsystem ||
					 ((*sipp)->subsystem == (*xipp)->subsystem &&
					  (*sipp)->order < (*xipp)->order))
					continue;	/* skip*/
				save = *sipp;
				*sipp = *xipp;
				*xipp = save;
			}
		}


	/*
	 * Traverse the (now) ordered list of system initialization tasks.
	 * Perform each task, and continue on to the next task.
	 *
	 * The last item on the list is expected to be the scheduler,
	 * which will not return.
	 */
	
	for (sipp = sysinit; sipp < sysinit_end; sipp++) {		
		if ((*sipp)->subsystem == SI_SUB_DUMMY)
			continue;	/* skip dummy task(s)*/

		if ((*sipp)->subsystem == SI_SUB_DONE)
			continue;

#if 0
#if defined(VERBOSE_SYSINIT)
		if ((*sipp)->subsystem > last) {
			verbose = 1;
			last = (*sipp)->subsystem;
			printf("subsystem %x\n", last);
		}
		if (verbose) {
#if defined(DDB)
			const char *name;
			c_db_sym_t sym;
			db_expr_t  offset;

			sym = db_search_symbol((vm_offset_t)(*sipp)->func,
			    DB_STGY_PROC, &offset);
			db_symbol_values(sym, &name, NULL);
			if (name != NULL)
				printf("   %s(%p)... ", name, (*sipp)->udata);
			else
#endif
				printf("   %p(%p)... ", (*sipp)->func,
				    (*sipp)->udata);
		}
#endif
#endif

		/* Call function */
		(*((*sipp)->func))((*sipp)->udata);

		/* Check off the one we're just done */
		(*sipp)->subsystem = SI_SUB_DONE;

		/* Check if we've installed more sysinit items via KLD */
	}

	if(saves_free)
		free(saves_free);
}


static void
configure_first(dummy)
	void *dummy;
{
	/* nexus0 is the top of the mips device tree */
#ifdef CONFIG_RTL_8196C
	usb_hcd_flag = 1;
	device_add_child(root_bus, "ehci", 0);
	//device_add_child(root_bus, "ohci", 0);
#endif

#ifdef CONFIG_RTL_8196E
	usb_hcd_flag = 3;//otg
	device_add_child(root_bus, "dwcotg", 0);
#endif
}


static void
configure(dummy)
	void *dummy;
{
	/* initialize new bus architecture */
	root_bus_configure();
}

static void
configure_final(dummy)
	void *dummy;
{
#if 0
	intr_enable();

	cninit_finish(); 

	if (bootverbose)
		printf("Device configuration finished.\n");

	cold = 0;
#endif
}


SYSINIT(configure1, SI_SUB_CONFIGURE, SI_ORDER_FIRST, configure_first, NULL);
SYSINIT(configure2, SI_SUB_CONFIGURE, SI_ORDER_THIRD, configure, NULL);
SYSINIT(configure3, SI_SUB_CONFIGURE, SI_ORDER_ANY, configure_final, NULL);

static bool if_usb_hcd_init(struct cyg_devtab_entry *tab)
{
	cyg_mutex_init(&Giant);
	mi_startup();
}




