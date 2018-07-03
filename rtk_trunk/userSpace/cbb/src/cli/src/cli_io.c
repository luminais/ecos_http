/*
 * CLI command main entry.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_io.c,v 1.2 2010-04-22 09:04:56 Exp $
 */

#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/ttyio.h>

#include <stdio.h>
#include <unistd.h>
#include <cli.h>
#include <cli_io.h>


#define CLI_IO_DEV	"/dev/ser0"
static cyg_io_handle_t	cli_io_handle;

FILE *
cli_io_init(int fd)
{
	FILE *stream;

	/* Setup cli device handle */
	if (fd == -1) {
		cyg_io_lookup(CLI_IO_DEV, &cli_io_handle);
		stream = stdout;
	}
	else {
		stream = fdopen(fd, "wb");
	}

	return stream;
}

void
cli_io_free(FILE *stream)
{
	if (stream && stream != stdout) {
		fclose(stream);
	}
	return;
}

int
cli_io_read(int fd, char *c)
{
	int clen = 1;

	if (fd == -1) {
		cyg_io_read(cli_io_handle, c, (unsigned int *)&clen);
		return 1;
	}
	else {
		clen = read(fd, c, 1);
	}

	return clen;
}
