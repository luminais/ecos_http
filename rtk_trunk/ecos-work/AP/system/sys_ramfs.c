//==========================================================================
//
//      ramfs1.c
//
//      Test fileio system
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-05-25
// Purpose:             Test fileio system
// Description:         This test uses the testfs to check out the initialization
//                      and basic operation of the fileio system
//                      
//                      
//                      
//                      
//                      
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/fs_ram.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <stdio.h>
#include <cyg/fileio/fileio.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>            // HAL polled output

//==========================================================================

#if 0
MTAB_ENTRY( ramfs_mte1,
                   "/",
                   "ramfs",
                   "",
                   0);
#endif

//==========================================================================

#define SHOW_RESULT( _fn, _res ) \
printf("<FAIL>: " #_fn "() returned %ld %s\n", (long)_res, _res<0?strerror(errno):"");

//==========================================================================

#define IOSIZE  100

//==========================================================================
#if 0
//==========================================================================
int count_size(char *name)
{
    int err,size;
    DIR *dirp;
    int num=0;
    char fullname[PATH_MAX];
    struct stat sbuf;
                
    diag_printf("reading directory %s\n",name);
    
    dirp = opendir( name );	

    size=0;
	
    if( dirp == NULL ) diag_printf("name NULL\n");

    for(;;)
    {
        struct dirent *entry = readdir( dirp );
        
        if( entry == NULL )
            break;

            if( name[0] )
            {
                strcpy(fullname, name );
                if( !(name[0] == '/' && name[1] == 0 ) )
                    strcat(fullname, "/" );
            }
            else fullname[0] = 0;
            
            strcat(fullname, entry->d_name );
	     diag_printf("fullname %s\n",fullname);
            
            err = stat( fullname, &sbuf );
            if( err < 0 )
            {
                if( errno == ENOSYS )
                    printf(" <no status available>");
                else SHOW_RESULT( stat, err );
            }
            else
            {
            	if (S_ISDIR(sbuf.st_mode) && entry->d_name[0]!='.')
            		size+=count_size(fullname);
            	else
			size += (unsigned long) sbuf.st_size;
            }

        printf("\n");
    }
    err = closedir( dirp );
    if( err < 0 ) SHOW_RESULT( stat, err );
    return size;
}
#endif

void list_dir(char *name)
{
    int err;
    DIR *dirp;
    int num=0;
    char fullname[PATH_MAX];
    struct stat sbuf;
                
    printf("reading directory %s\n",name);
    
    dirp = opendir( name );
    if( dirp == NULL ) SHOW_RESULT( opendir, -1 );
	
	
    if (!dirp)
		return;
	
    for(;;)
    {
        struct dirent *entry = readdir( dirp );
        
        if( entry == NULL )
            break;
        num++;

            if( name[0] )
            {
                strcpy(fullname, name );
                if( !(name[0] == '/' && name[1] == 0 ) )
                    strcat(fullname, "/" );
            }
            else fullname[0] = 0;
            
            strcat(fullname, entry->d_name );
            
            err = stat( fullname, &sbuf );
            if( err < 0 )
            {
                if( errno == ENOSYS )
                    printf(" <no status available>");
                else SHOW_RESULT( stat, err );
            }
            else
            {
            	if (S_ISDIR(sbuf.st_mode))
            		printf("%14s/",entry->d_name);
            	else
            		printf("%15s",entry->d_name);
                printf(" [mode %08x ino %08x nlink %d size %ld]",
                            sbuf.st_mode,sbuf.st_ino,sbuf.st_nlink,
                            (unsigned long) sbuf.st_size);
            }

        printf("\n");
    }

    err = closedir( dirp );
    if( err < 0 ) SHOW_RESULT( stat, err );
}

//==========================================================================
void cat_file(char *name, int hexdump)
{
    unsigned char buf[IOSIZE];
    int fd;
    ssize_t done;
    int i, j;
    int err;
    off_t pos = 0;

    err = access( name, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );

    fd = open( name, O_RDONLY );
    if( fd < 0 ) {
		SHOW_RESULT( open, fd );
		return;
    }
	
    j = 0;
    for(;;)
    {
        done = read( fd, buf, IOSIZE );
        if( done < 0 ) {
			SHOW_RESULT( read, done );
			break;
        }

        if( done == 0 ) break;

        for( i = 0; i < done; i++ ) {
        	if (hexdump) {
                	printf("%02x ", buf[i]);
                	j++;
                	if ((j%16) == 0)
                		printf("\n");
                }
                else {
                	printf("%c", buf[i]);
                }
        }
        pos += done;
    }
    printf("\n");
    err = close( fd );
    if( err < 0 ) SHOW_RESULT( close, err );
}

// -------------------------------------------------------------------------
int create_file(char *name, char *buf, int len)
{
	int fd;
	int wrote;

	fd = open(name, O_WRONLY|O_CREAT);
	if (fd < 0) {
		printf("open() fail: %s\n", name);
		return -1;
	}

        wrote = write(fd, buf, len);
        if(wrote != len)
		printf("write() fail\n");

	close(fd);
	return 0;
}

int write_line_to_file(char *filename, int mode, char *line_data)
{
	unsigned char tmpbuf[512];
	int fh=0;

	if(mode == 1) {/* write line data to file */
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
		if (fh < 0) {
			fprintf(stderr, "Create %s error!, the return value is %d\n", filename, fh);
			return 0;
		}
		
	}else if(mode == 2){/*append line data to file*/
		
		fh = open(filename, O_RDWR|O_APPEND);
		if (fh < 0) {
			fprintf(stderr, "open %s error!, the return value is %d\n", filename, fh);
			return 0;
		}
	}

	sprintf((char *)tmpbuf, "%s", line_data);
	write(fh, tmpbuf, strlen((char *)tmpbuf));

	close(fh);
	return 1;
}

int get_value_from_file(char *filename, char *variable, char *value)
{
	FILE *stream;
	char *strtmp;
	char line[128];
	char *token=NULL;
	
	stream = fopen(filename, "r");
	if (stream != NULL) {
		while (fgets(line, sizeof(line), stream))
		{
			strtmp = line;
			while(*strtmp == ' ')
				strtmp++;
			
			token = strtok (strtmp,":");
			if(token != NULL) 
			{
				if(!strcmp(token, variable))
				{
					token = strtok(NULL, ":");
					strcpy(value, token);
					break;	
				}
			}
		}
	}
	else
		return -1;
		
	fclose(stream);
	return 0;
}

//==========================================================================
void ramfs_init(void)
{
    int err;

    err = mount( "", "/", "ramfs" );
    if( err < 0 ) SHOW_RESULT( mount, err );

    err = chdir( "/" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    err = mkdir( "/var", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
    
    err = mkdir( "/tmp", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
	
    err = mkdir( "/tmp/usb", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
	
    err = mkdir( "/etc", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
#ifdef JFFS2_SUPPORT
	err = mount( "/dev/flash/0/0x10000,0x44000", "/web", "jffs2" );
    if( err < 0 ) SHOW_RESULT( mount, err );  
#else
    err = mkdir( "/web", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
#endif
}
