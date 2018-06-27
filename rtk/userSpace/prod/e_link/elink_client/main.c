/*
	E-link protocol implement main entry. A ball ache work begin.

	Two works need to done here.
		1. module initialize. Like timer/extra etc.
		2. Enter proto_elink_main_loop.
	
	Wangxinyu.yy@gmail.com 
	2016.8.8
*/


#include <stdio.h>
#include <libgen.h> // For basename.


#include "log_debug.h"
#include "extra.h"
#include "timerlib.h"


int main(int argc , char *argv[])
{
	/* Init log file. */
	log_init(basename(argv[0]));

	TRACE;
    //timer_init_all();
	//extra_init();

    proto_elink_init();

	/* Real work . */   
	proto_elink_main_loop();


	log_error("e-link protocol exit...\n");
    
	return 0;
}
