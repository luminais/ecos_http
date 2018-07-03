/*
 * CLI io include file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_io.h,v 1.2 2010-04-22 09:03:38 Exp $
 */
#ifndef	__CLI_IO_H__
#define	__CLI_IO_H__

#include <cli.h>

FILE *cli_io_init(int fd);
void cli_io_free(FILE *stream);
int cli_io_read(int fd, char *c);

#endif	/* __CLI_IO_H__ */
