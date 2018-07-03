/*
 * Copyright (c) 2005, 2006
 *
 * James Hook (james@wmpp.com) 
 * Chris Zimman (chris@wmpp.com)
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/io.h>

#include <shell_thread.h>
#include <shell_err.h>
#include <commands.h>

#define CMD_BUF_USE_MALLOC 1
#define SHELLTASK_PRIORITY	28			/* This runs at a low priority */
#define SHELL_NAME		"#"		/* The default command line */
#define ERRORCOMMAND		255
#ifdef CMD_BUF_USE_MALLOC
static int com_buf_len = 128; /*current buffer len*/
#define COM_BUF_LEN_INIT    128 /*init buffer len*/
#else
#define COM_BUF_LEN		128
#endif
#define SHELL_HIST_SIZE		10

CYG_HAL_TABLE_BEGIN(__shell_CMD_TAB__, shell_commands);
CYG_HAL_TABLE_END(__shell_CMD_TAB_END__, shell_commands);

unsigned char count_argc(char *buf, shell_st_call *func);
void shelltask(cyg_addrword_t data);

unsigned char shell_stack[8*1024];


#if defined(CONFIG_RTL_8881A) || defined(CONFIG_SLOT_1_8812) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
#define ARGV_COUNT	200
#elif defined(CONFIG_RTL_819X)	//jwj:20120704
#define ARGV_COUNT	30
#else
#define ARGV_COUNT	30
#endif

unsigned char *argv[ARGV_COUNT];
unsigned char argc;

typedef struct _shell_hist_t {
#ifdef CMD_BUF_USE_MALLOC
    char command_buf[COM_BUF_LEN_INIT + 1];
#else
    char command_buf[COM_BUF_LEN + 1];
#endif
    unsigned int pos;
    struct _shell_hist_t *prev, *next;
} shell_hist_t;

shell_hist_t *
add_shell_history_node(shell_hist_t *history, unsigned int pos)
{
    shell_hist_t *s;

    if(!(s = (shell_hist_t *)malloc(sizeof(shell_hist_t))))
	return(NULL);

    memset(s, 0, sizeof(shell_hist_t));

    if(history) 
	history->next = s;

    s->prev = history;
    s->pos = pos;

    return s;
}

shell_hist_t *
build_hist_list(unsigned int size)
{
    int i = size;
    shell_hist_t *sh = NULL, *start = NULL;

    while(i) {
	if(i == size) {
	    if(!(start = sh = add_shell_history_node(NULL, i))) {
		printf("Aiee -- add_shell_history_node() returned NULL\n");
		return(NULL);
	    }
	}
	else {
	    if(!(sh = add_shell_history_node(sh, i))) {
		printf("Aiee -- add_shell_history_node() returned NULL\n");
		return(NULL);
	    }
	}
	i--;
    }

	if(start != NULL){
	    sh->next = start;
	    start->prev = sh;
	}
    return start;
}

void
create_shell_thread(void)
{
    if(shell_create_thread(NULL,
			  SHELLTASK_PRIORITY,
			  shelltask,
			  0,
			  "Shell",
			  &shell_stack,
			  sizeof(shell_stack),
			  NULL) != SHELL_OK) {
	SHELL_ERROR("Failed to create Shell thread\n");
	HAL_PLATFORM_RESET();
    }

    SHELL_DEBUG_PRINT("Created Shell thread\n");
}

void
shelltask(cyg_addrword_t data)
{   
    shell_hist_t *sh;

    cyg_io_handle_t handle;
    cyg_uint32 len;
    Cyg_ErrNo err;

#ifdef CMD_BUF_USE_MALLOC
	unsigned int i = 0; //char will limit max 256
	char *command_buf = NULL;
	com_buf_len = COM_BUF_LEN_INIT; 
	command_buf = malloc(com_buf_len + 1); 
	if(!command_buf){
		printf("shell out of memory\n");
        return -1;
	}
	bzero(command_buf,com_buf_len + 1);
#else
    unsigned char i = 0, num;
    
    char command_buf[COM_BUF_LEN + 1];	/* store '\0' */
#endif

    char ch;

    len = sizeof(ch);

    if(!(sh = build_hist_list(SHELL_HIST_SIZE))) {
	printf("build_hist_list() failed\nResetting\n");
	HAL_PLATFORM_RESET();
    }

    err = cyg_io_lookup("/dev/ser0", &handle); 
    //err = cyg_io_lookup("/dev/ttydiag", &handle);

    if(err != ENOERR) {
	printf("Unable to open /dev/ser0\nHalting\n");
	HAL_PLATFORM_RESET();
    }

    /* Unbuffer stdout */
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    command_buf[0] = '\0';

    printf("\n"SHELL_NAME" ");

    while(1) {
	do{ 
	    cyg_io_read(handle, &ch, &len);
	} while(!isprint(ch) && 
		(ch != '\b') &&		/* Backspace */ 
		(ch != 0x7F) &&		/* Backspace */
		(ch != 0x0A) &&		/* LF */
		(ch != 0x0D) &&		/* CR */
		(ch != 0x01) &&		/* Ctrl-A */
		(ch != 0x02) &&		/* Ctrl-B */
		(ch != 0x15) &&		/* Ctrl-U */
		(ch != 0x1b));
	switch(ch) {
	case 0x01:		/* Ctrl-A */
	    if(i) {
		int x = i;
		while(x) {
		    putchar('\b');
		    x--;
		    i--;
		}
	    }
	    break;
	case 0x02:		/* Ctrl-B */	
	    if(i) {
		i--;
		putchar('\b');
	    }
	    break;
		
	case 0x0A:				/* LF */
	case 0x0D:				/* CR */
	    if(!i) {				/* commandbuf is NULL, begin a new line */
		printf("\n"SHELL_NAME" ");
	    }
	    else {
		/* Get rid of the end space */
		if(command_buf[i - 1]==' ') 
		    i--;

		command_buf[i] = '\0';
		
		/* Add to the shell history */
#ifdef CMD_BUF_USE_MALLOC
		if(i <= COM_BUF_LEN_INIT) /*if cmd too big,do not add history*/
		{
		    strncpy(sh->command_buf, command_buf, sizeof(sh->command_buf) - 1);
            sh->command_buf[COM_BUF_LEN_INIT] = '\0';
		    sh = sh->next;
		}
#else
		strncpy(sh->command_buf, command_buf, sizeof(sh->command_buf) - 1);
		sh = sh->next;
#endif

		run_clicmd(command_buf);

#ifdef CMD_BUF_USE_MALLOC
		if((i < com_buf_len /2) && (com_buf_len > COM_BUF_LEN_INIT)){
		    com_buf_len = com_buf_len/2;
		    char *ptr = NULL;
			ptr = malloc(com_buf_len + 1);
			if(!ptr){
			//if(ptr){ //for test
				printf("shell outof memory\n");
				com_buf_len = 2*com_buf_len;
				i = 0;
				command_buf[i] = '\0';
				printf(SHELL_NAME" ");
                //free(ptr); //for test
				break;
			}
			bzero(ptr,com_buf_len+1);
			free(command_buf);
			command_buf = ptr;
		}
#endif

		i = 0;
		command_buf[i] = '\0';
		printf(SHELL_NAME" ");
	    }
	    break;
	    
	case 0x15:			/* Ctrl-U */
	    if(i) {
		while(i) {
		    putchar('\b');
		    putchar(' ');
		    putchar('\b');
		    i--;
		}
	    }
	    break;

	case 0x7F:
	case '\b':		/* Backspace */
	    if(i) {
		i--;		/* Pointer back once */
		putchar('\b');	/* Cursor back once */
		putchar(' ');	/* Erase last char in screen */
		putchar('\b');	/* Cursor back again */
	    }
	    break;
	
	case ' ':
	    /* Don't allow continuous or begin space(' ') */
	    if((!i) || 
#ifdef CMD_BUF_USE_MALLOC
	       (i > com_buf_len) || 
#else
	       (i > COM_BUF_LEN) || 
#endif
	       (command_buf[i - 1] == ' ')) {
		/* do nothing */
	    }
	    else {
		command_buf[i] = ch;
		i++;
		putchar(ch);	/* display and store ch */
	    }
	    break;

	case 0x41:
	case 0x42:
		if (i >= 2) {
			//check if users presses Up arrow or Down arrow
			if ((command_buf[i-1]==0x5b) && (command_buf[i-2]==0x1b)) {
				int x;
				/* If there's any input on the line already, erase it */
				if(i) {
					x = i+2;
					putchar('\r');
					while(x) {
				    		putchar(' ');
				    		x--;
					}
					printf("\r"SHELL_NAME" ");
				}
				
				if (ch == 0x41)
					sh = sh->prev;
				else
					sh = sh->next;
#ifdef CMD_BUF_USE_MALLOC
                strncpy(command_buf, sh->command_buf, sizeof(sh->command_buf) - 1);
                command_buf[COM_BUF_LEN_INIT] = '\0'; //for strlen can get the end
#else
				strncpy(command_buf, sh->command_buf, sizeof(command_buf) - 1);
#endif
				i = strlen(command_buf);
				//printf("%s", command_buf);
				for (x=0; x<i; x++) {
					if (isprint(command_buf[x]))
						putchar(command_buf[x]);
				}
				break;
			}
		}
		//It's a Normal key. Fall through!!
	default:			/* Normal key */
#ifdef CMD_BUF_USE_MALLOC
		if(i >= com_buf_len ){
		    char *ptr = NULL;
		    com_buf_len = 2 * com_buf_len;
			ptr = malloc(com_buf_len+1);
			if(!ptr){
			//if(ptr){  //for test
				printf("cmd may too long, shell outof memory\n");
				com_buf_len = com_buf_len/2;
                //free(ptr); //for test
				break;
			}
			bzero(ptr,com_buf_len+1);
			memcpy(ptr,command_buf,i);
			free(command_buf);
			command_buf = ptr;
		}
		command_buf[i] = ch;
		i++;
		if (isprint(ch))
			putchar(ch);  /* Display and store ch */
		break;
#else
	    if(i < COM_BUF_LEN) {	/* The buf is less than MAX length */
		command_buf[i] = ch;
		i++;
		if (isprint(ch))
			putchar(ch);  /* Display and store ch */
	    }
	    break;
#endif
	}
    }
}

unsigned char 
count_argc(char *buf, shell_st_call *func)
{
    ncommand_t *shell_cmd = __shell_CMD_TAB__;
    unsigned char num;
	#if defined(CONFIG_RTL_8881A) || defined(CONFIG_SLOT_1_8812) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	unsigned int pointer;
	#else
	unsigned char pointer;
	#endif
    char name[40];
    argc = 0;			/* argc is global  */
    num = 0;
    pointer = 0;
    int i;

    /* clean argv */
	for(i = 0; i < ARGV_COUNT;i++)
		argv[i] = NULL;
    

    while((buf[pointer] != ' ') && 
	  (buf[pointer] != '\0') && 
	  (pointer < 20)) {
		name[pointer] = buf[pointer];
		pointer++;
    }
    name[pointer] = '\0';

	#if defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER)	
		if(!strstr(name,"wlan"))
			printf("\n\r");
	#else
		printf("\n\r");
	#endif

    /* 
     * Now got the command name, and pointer 
     * is to the first space in the buf
     */

    /* Check the dynamic function tables as well */
    while(shell_cmd != &__shell_CMD_TAB_END__) {
		if(!strcmp(name, shell_cmd->name)) {
	    	*func = shell_cmd->cfunc;
	    	break;
		}
		shell_cmd++;
    }

    if(shell_cmd == &__shell_CMD_TAB_END__)
		return ERRORCOMMAND;
	
    while(buf[pointer] != '\0') {
		if(buf[pointer] == '"') {
		    unsigned char tmp_pointer = pointer;
		    while(buf[++pointer] != '\0');
		    while((tmp_pointer+1) != pointer){
			if(buf[--pointer] == '"'){
			    buf[pointer++] = '\0';
			    unsigned char tmp_buf[32];
			    strcpy(tmp_buf,&buf[tmp_pointer+1]);
			    strcpy(&buf[tmp_pointer],tmp_buf);
			    //argv[argc-1] = (unsigned char *)&buf[tmp_pointer+1]; 
			    argv[argc++] = (unsigned char *)&buf[pointer]; 
			    tmp_pointer = pointer;
			    break;
			}
		    }
		    pointer = tmp_pointer;
		}
		if(buf[pointer] == ' ') {

		    /* End of last argv */
		    buf[pointer++] = '\0';
		    /* argv[0] = program name */
		    if (argc == 0) {
		        argv[argc++] = (unsigned char *)&buf[0];
		    }

		    /* Add a parameter for every space */
		    argv[argc++] = (unsigned char *)&buf[pointer];
		}
		else 
			pointer++;
		
    }

    /* if no any argument, setup default argv[0] */
    if (argc == 0)
	    argv[argc++] = (unsigned char *)&buf[0];

    return num;
}
#ifdef CONFIG_TENDA_ATE_REALTEK
extern int run_tenda_cmd(char *command_buf);
#endif
int run_clicmd(char *command_buf)
{
	shell_st_call func;
	unsigned char num;
   
	/* Count argv entries in the command_buf */
	num = count_argc(command_buf, &func);
	
	if(num == ERRORCOMMAND) {
#ifdef CONFIG_TENDA_ATE_REALTEK
        if(run_tenda_cmd(command_buf)){
            return 0;
        } else 
#endif
        {
		/* Error or none exist command */
		printf("Unknown command '%s'\n", (const char *)command_buf);
		return -1;
        }
	}
	else{
		/* Call corresponding function */
		func(argc-1, &argv[1]); // argc--, argv++ for backward compatable
		return 0;
	}
}
