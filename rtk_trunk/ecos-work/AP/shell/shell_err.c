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
#include <string.h>

#include <shell.h>

#define DEFINES_SHELL_ERROR_T
#include <shell_err.h>

/*
 * A list of string representations of the error codes
 * Note, these must match the order of the ennumerated error
 * codes in the list
 */

const shell_error_t shell_errors[] = {
    /* OK */
    { SHELL_OK,				"OK" },
    
    /* General Errors */
    { SHELL_ALLOC_ERROR,		"Allocation failure" },
    { SHELL_INVALID_PTR,		"Invalid pointer value" },
    { SHELL_INVALID_ARGUMENT,		"Invalid function argument"},
    
    /* Thread Errors */
    { SHELL_THREAD_CREATE_FAIL,		"Thread creation failed" },
    { SHELL_PRIORITY_ERROR,		"Thread priority out of range" },
    { SHELL_INVALID_THREAD_NAME,	"Invalid thread name" },
    { SHELL_INVALID_FUNC_PTR,		"Invalid function pointer" },
    { SHELL_INVALID_STACK_SIZE,		"Invalid stack size" },
};

const char *
shell_error_str(int err)
{
    if((err >= SHELL_OK) || (err <= SHELL_LAST_ERROR))
	return shell_errors[err].err_msg;
}
