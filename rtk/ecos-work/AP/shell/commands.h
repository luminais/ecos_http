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

#if !defined(_COMMANDS_H_)
#define _COMMANDS_H_

#include <cyg/hal/hal_tables.h>
#include <shell.h>

#if defined(SHELL_TEST)
#define CMD_DECL(x) shell_return x(unsigned int argc, unsigned char *argv[])
#define CMD_DEF(x) x
#else
#define CMD_DECL(x)
#define CMD_DEF(x)
#endif	/* defined(SHELL_TEST) */

typedef CMD_DECL(cmd_func);

typedef shell_return (*shell_st_call)(unsigned int argc, unsigned char *argv[]);

typedef struct _ncommand_t {
    const char *name;	/* Function name */
    const char *descr;	/* Function description */
    const char *usage;	/* Function usage */
    cmd_func   *cfunc;	/* Command function pointer */
    struct _ncommand_t  *sub_cmds, *sub_cmds_end;    
} ncommand_t CYG_HAL_TABLE_TYPE;

extern ncommand_t __shell_CMD_TAB__[], __shell_CMD_TAB_END__;

ncommand_t  *cmd_search(ncommand_t *tab, ncommand_t *tabend, char *arg);
void        cmd_usage(ncommand_t *tab, ncommand_t *tabend, char *prefix);
#define shell_cmd(_s_,_h_,_u_,_f_) _cmd_entry(_s_,_h_,_u_,_f_,0,0,shell_commands)
#define shell_nested_cmd(_s_,_h_,_u_,_f_,_subs_,_sube_) _cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,shell_commands)
#define _cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,_n_)                                   \
cmd_func _f_;                                                     \
ncommand_t _cmd_tab_##_f_ CYG_HAL_TABLE_QUALIFIED_ENTRY(_n_,_f_) = {_s_, _h_, _u_, _f_, _subs_, _sube_};
#define cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,_n_)                                   \
extern _cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,_n_)
#define local_cmd_entry(_s_,_h_,_u_,_f_,_n_)                             \
static _cmd_entry(_s_,_h_,_u_,_f_,0,0,_n_)

#define SHELL_COMMAND_PARSER(arg, table, table_end, fptr) \
    {						\
    ncommand_t *shell_cmd = table;		\
    while(shell_cmd != &table_end) {		\
	if(!strcmp(arg, shell_cmd->name)) {	\
	    *fptr = shell_cmd->cfunc;		\
	    break;				\
	}					\
	shell_cmd++;				\
    }						\
    if(shell_cmd == &table_end)			\
       return -1;				\
    }

#endif	/* !#defined(_COMMANDS_H_) */
