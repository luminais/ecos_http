/* =================================================================
 *
 *      fmget.h
 *
 *      Handles the asp requests.
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####                                     
 * -------------------------------------------                       
 * This file is part of eCos, the Embedded Configurable Operating System.
 * Copyright (C) 2005, 2007 Free Software Foundation, Inc.           
 *
 * eCos is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 or (at your option) any later
 * version.                                                          
 *
 * eCos is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.                                                 
 *
 * You should have received a copy of the GNU General Public License 
 * along with eCos; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.     
 *
 * As a special exception, if other files instantiate templates or use
 * macros or inline functions from this file, or you compile this file
 * and link it with other works to produce a work based on this file,
 * this file does not by itself cause the resulting work to be covered by
 * the GNU General Public License. However the source code for this file
 * must still be made available in accordance with section (3) of the GNU
 * General Public License v2.                                        
 *
 * This exception does not invalidate any other reasons why a work based
 * on this file might be covered by the GNU General Public License.  
 * -------------------------------------------                       
 * ####ECOSGPLCOPYRIGHTEND####                                       
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    hf_shi
 *  Contributors: 
 *                
 *  Date:         2012-06-01
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */

#ifndef _CYG_ATHHTTPD_FMGET_H_
#define _CYG_ATHHTTPD_FMGET_H_

#define TMP_BUF_SIZE 512

int getIndex(int argc, char ** argv);
int getInfo(int argc, char ** argv);
int getVirtualInfo(int argc, char ** argv);
int getVirtualIndex(int argc, char ** argv);
int getModeCombobox(int argc, char **argv);
int ipFilterList(int argc, char **argv);
int portFilterList(int argc, char **argv);
int macFilterList(int argc, char **argv);
int urlFilterList(int argc, char **argv);
int getLangInfo(int argc, char ** argv);
int isConnectPPP(void);
int isDhcpClientExist(char *interface);
int getWanLink(char *interface);
#endif /*_CYG_ATHHTTPD_FMGET_H_*/
