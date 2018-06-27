/*rtldebug for rtldd*/
#ifndef __RTLDEBUG_H__
#define __RTLDEBUG_H__

#define ERROR_FLAG 0x1
#define WARN_FLAG  0x2
#define TRACE_FLAG 0x4
/*DEBUG*/
#define DEBUG

#ifdef DEBUG
	#define DEBUG_ERR(fmt,args...) \
		if(debug_flag & ERROR_FLAG) \
			diag_printf("Err:"fmt,##args); 

	#define DEUBG_WARN(fmt,args...) \
		if(debug_flag & WARN_FLAG) \
			diag_printf("WARN:"fmt,##args); 
		
	#define DEBUG_TRACE(fmt,args...) \
		if(debug_flag & TRACE_FLAG) \
			diag_printf("TRACE:"fmt,##args); 
	#define DEBUG_PRINT(fmt,args...) \
		diag_printf(fmt,##args)
#else
	#define DEBUG_ERR
	#define DEUBG_WARN
	#define DEBUG_TRACE
	#define DEBUG_PRINT
#endif
extern unsigned long debug_flag;

#endif
