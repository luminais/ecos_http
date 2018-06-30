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

#include <pkgconf/system.h>
#include <shell.h>
#include <shell_err.h>

/*#define BOOT_BANNER	"***********************************\n" \
			"* Insert your clever message here *\n" \
			"***********************************\n"
*/			
extern unsigned char *fwVersion;

unsigned char build_time[] = BUILD_TIME;

void
print_build_tag(void)
{
    //SHELL_PRINT(BOOT_BANNER);
    //SHELL_PRINT("Built "BUILD_TAG"\n");
    SHELL_PRINT("%s\n", fwVersion);
    SHELL_PRINT("Built "BUILD_TIME""BUILD_HOST"\n");
    SHELL_PRINT("eCos Release: %d.%d.%d\n", 
	       CYGNUM_HAL_VERSION_MAJOR,
	       CYGNUM_HAL_VERSION_MINOR,
	       CYGNUM_HAL_VERSION_RELEASE);
    SHELL_PRINT("Using "COMPILER_RELEASE"\n");
}
