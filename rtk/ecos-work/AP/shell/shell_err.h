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

#if !defined(_SHELL_ERROR_H_)
#define _SHELL_ERROR_H_

#include <cyg/infra/diag.h>	/* diag_printf() */

#include <stdio.h>
#include <shell.h>

#if !defined(DEFINES_SHELL_DEBUG)
extern unsigned int shell_debug;
#endif  /* defined(DEFINES_SHELL_DEBUG) */

#define SHELL_DEBUG_STR           "Shell: "
#define SHELL_DIAG_STR            "Shell(Diag): "

#define SHELL_PRINT(x, args...) printf(x, ##args)
#define SHELL_PRINTD(x, args...) printf(SHELL_DEBUG_STR x, ##args)

#if defined(SHELL_DEBUG)

#define SHELL_DEBUG_PRINT(x, args...) if(shell_debug) SHELL_PRINTD(x, ##args)
#define SHELL_DEBUG_PRINT_CONT(x, args...) SHELL_PRINT(x, ##args)
#define SHELL_DEBUG_PRINT_FUNC(x, args...) if(shell_debug) SHELL_PRINTD("[%s] " x, __func__, ##args)
#define SHELL_DEBUG_DEFINE(x) x;
#define SHELL_DEBUG_CALL(x) if(shell_debug) x
#define SHELL_TRACE(x, args...) if(shell_debug) \
	SHELL_PRINTD("TRACE:%s:%s:%d\n", __FILE__, __func__, __LINE__); \
	SHELL_PRINTD("TRACE:|\n"); \
	SHELL_PRINTD("TRACE:|-->%s\n", #x); \
	SHELL_PRINTD("TRACE:|--> " x, ##args);
#define SHELL_TRACE_CALL(x) if(shell_debug) SHELL_PRINTD("TRACE:%s:%d:%s\n", __FILE__, __LINE__, #x); x;

#else

#define SHELL_DEBUG_PRINT(x, args...)
#define SHELL_DEBUG_PRINT_CONT(x, args...)
#define SHELL_DEBUG_DEFINE(x)
#define SHELL_DEBUG_CALL(x)
#define SHELL_TRACE(x)
#define SHELL_TRACE_CALL(x) x;

#endif  /* defined(SHELL_DEBUG) */

#define SHELL_DIAG_PRINT(x, args...) diag_printf(SHELL_DIAG_STR x, ##args)
#define SHELL_ERROR(x, args...) { printf("%s%s", SHELL_DEBUG_STR, __func__); printf(x, ##args); }
#define SHELL_ASSERT(func, err) if(!(func)) { SHELL_ERROR(" :%s\n", shell_errors[err].err_msg); return err; }

typedef struct {
    unsigned int err_code;
    const char *err_msg;
} shell_error_t;

enum _SHELL_ERRORS {
    /* OK */
    SHELL_OK =	0,

    /* General errors */
    SHELL_ALLOC_ERROR,
    SHELL_INVALID_PTR,
    SHELL_INVALID_ARGUMENT,
    
    /* Thread errors */
    SHELL_THREAD_CREATE_FAIL,
    SHELL_PRIORITY_ERROR,
    SHELL_INVALID_THREAD_NAME,
    SHELL_INVALID_FUNC_PTR,
    SHELL_INVALID_STACK_SIZE,

    /* Make sure this one is last */
    SHELL_LAST_ERROR
};

#if !defined(DEFINES_SHELL_ERROR_T)
extern shell_error_t shell_errors[];
#endif	/* !defined(DEFINES_EXTERN_SHELL_ERROR_T) */

const char *shell_error_str(int err);

#endif	/* defined(_SHELL_ERROR_H_) */
