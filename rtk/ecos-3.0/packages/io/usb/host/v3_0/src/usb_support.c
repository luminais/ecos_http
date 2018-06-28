#include <cyg/kernel/kapi.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/libc/string/string.h>

#define CYGNUM_USB_EXPLORER_THREADOPT_STACKSIZE	0x00008000
cyg_thread   cyg_usb_explore_thread_object;
cyg_handle_t cyg_usb_explore_thread_handle;
cyg_uint8 	 usb_explor_stack[CYGNUM_USB_EXPLORER_THREADOPT_STACKSIZE];

#define CYGNUM_USB_CONTROL_XFER_THREADOPT_STACKSIZE	0x00008000
cyg_thread   cyg_usb_control_xfer_thread_object;
cyg_handle_t cyg_usb_control_xfer_thread_handle;
cyg_uint8 	 usb_control_xfer_stack[CYGNUM_USB_CONTROL_XFER_THREADOPT_STACKSIZE];

#define CYGNUM_USB_GIANT_THREADOPT_STACKSIZE	0x00008000
cyg_thread   cyg_usb_giant_thread_object;
cyg_handle_t cyg_usb_giant_thread_handle;
cyg_uint8 	 usb_giant_stack[CYGNUM_USB_GIANT_THREADOPT_STACKSIZE];

#define CYGNUM_USB_NON_GIANT_THREADOPT_STACKSIZE	0x00008000
cyg_thread   cyg_usb_non_giant_thread_object;
cyg_handle_t cyg_usb_non_giant_thread_handle;
cyg_uint8 	 usb_non_giant_stack[CYGNUM_USB_NON_GIANT_THREADOPT_STACKSIZE];

int kthread_create(void (*threadfn)(void *data),
                                   void *data,
                                   cyg_thread* thread,
                                   char *pmesg, 
                                   unsigned char prio)
{
	if(!strcmp(pmesg,"Explore Thread")){
		diag_printf("1\n");
		cyg_thread_create(
		prio,
		threadfn,
		(cyg_addrword_t)data,
		"Usb Explore Thread",
		usb_explor_stack,
		sizeof(usb_explor_stack),
		&cyg_usb_explore_thread_handle,
		&cyg_usb_explore_thread_object);

		cyg_thread_resume(cyg_usb_explore_thread_handle);
	}else if(!strcmp(pmesg,"Control Xfer Thread")){
		diag_printf("1\n");
		cyg_thread_create(
		prio,
		threadfn,
		(cyg_addrword_t)data,
		"Usb Control_Xfer Thread",
		usb_control_xfer_stack,
		sizeof(usb_control_xfer_stack),
		&cyg_usb_control_xfer_thread_handle,
		&cyg_usb_control_xfer_thread_object);

		cyg_thread_resume(cyg_usb_control_xfer_thread_handle);
	}else if(!strcmp(pmesg,"Giant Thread")){
		diag_printf("1\n");
		cyg_thread_create(
		prio,
		threadfn,
		(cyg_addrword_t)data,
		"Usb Giant Thread",
		usb_giant_stack,
		sizeof(usb_giant_stack),
		&cyg_usb_giant_thread_handle,
		&cyg_usb_giant_thread_object);

		cyg_thread_resume(cyg_usb_giant_thread_handle);
	}else if(!strcmp(pmesg,"Non Giant Thread")){
		diag_printf("1\n");
		cyg_thread_create(
		prio,
		threadfn,
		(cyg_addrword_t)data,
		"Usb Non_Giant Thread",
		usb_non_giant_stack,
		sizeof(usb_non_giant_stack),
		&cyg_usb_non_giant_thread_handle,
		&cyg_usb_non_giant_thread_object);

		cyg_thread_resume(cyg_usb_non_giant_thread_handle);
	}
	
	return 0;
}


