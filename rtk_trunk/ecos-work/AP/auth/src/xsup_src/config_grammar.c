
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "config_grammar.y"

/**
 * A client-side 802.1x implementation supporting EAP/TLS
 *
 * This code is released under both the GPL version 2 and BSD licenses.
 * Either license may be used.  The respective licenses are found below.
 *
 * Copyright (C) 2002 Bryan D. Payne & Nick L. Petroni Jr.
 * All Rights Reserved
 *
 * --- GPL Version 2 License ---
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * --- BSD License ---
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *       This product includes software developed by the University of
 *       Maryland at College Park and its contributors.
 *  - Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*******************************************************************
 * Grammar for configuration file
 * 
 * File: config_grammar.y
 *
 * Authors: bdpayne@cs.umd.edu, npetroni@cs.umd.edu
 *
 * $Id: config_grammar.y,v 1.1.1.1 2007/08/06 10:04:42 root Exp $
 * $Date: 2007/08/06 10:04:42 $
 * $Log: config_grammar.y,v $
 * Revision 1.1.1.1  2007/08/06 10:04:42  root
 * Initial import source to CVS
 *
 * Revision 1.1.1.1  2004/08/12 10:33:24  ysc
 *
 *
 * Revision 1.1  2004/07/24 00:52:56  kennylin
 *
 * Client mode TLS
 *
 * Revision 1.1  2004/07/24 00:40:55  kennylin
 *
 * Client mode TLS
 *
 * Revision 1.25  2004/03/26 03:52:47  chessing
 *
 * Fixed a bug in xsup_debug that would cause config-parse to crash.  Added new key word for session resumption.  Added code to attempt session resumption.  So far, testing has not succeeded, but it is attempting resume. (Four TTLS packets are exchanged, and then we get a failure.)  More testing is needed.
 *
 * Revision 1.24  2004/03/24 18:35:46  chessing
 *
 * Added a modified version of a patch from David Relson to fix a problem with some of the debug info in config_grammer.y.  Added some additional checks to eapol_key_type1 that will keep us from segfaulting under some *REALLY* strange conditions.  Changed the set key code in cardif_linux to double check that we aren't a wireless interface before returning an error.  This resolved a problem when XSupplicant was started when an interface was done.  Upon bringing up the interface, XSupplicant would sometimes think it wasn't wireless, and not bother trying to set keys.
 *
 * Revision 1.23  2004/03/22 00:41:00  chessing
 *
 * Added logfile option to the global config options in the config file.  The logfile is where output will go when we are running in daemon mode.  If no logfile is defined, output will go to the console that started xsupplicant.   Added forking to the code, so that when started, the process can daemonize, and run in the background.  If there is a desire to force running in the foreground (such as for debugging), the -f option was added.
 *
 * Revision 1.22  2004/03/15 16:23:24  chessing
 *
 * Added some checks to TLS using EAP types to make sure the root certificate isn't set to NULL.  (If it is, we can't authenticate, so we bail out.)  Changed the user certificate settings in the config file to all start with user_.  So, "cert" is now "user_cert", "key" is now "user_key", and "key_pass" is now "user_key_pass".  The structures and other related variables were also updated to reflect this change.  THIS WILL PROBABLY BREAK CONFIG FILES FOR SOME USERS!  (Be prepared for complaints on the list!)  ;)
 *
 * Revision 1.21  2004/03/06 03:53:54  chessing
 *
 * We now send logoffs when the process is terminated.  Added a new option to the config file "wireless_control" which will allow a user to disable non-EAPoL key changes.  Added an update to destination BSSID checking that will reset the wireless key to all 0s when the BSSID changes.  (This is what "wireless_control" disables when it is set to no.)  Roaming should now work, but because we are resetting keys to 128 bit, there may be issues with APs that use 64 bit keys.  I will test this weekend.
 *
 * Revision 1.20  2004/03/05 23:58:45  chessing
 *
 * Added CN (sometimes called server name) checking to TTLS and PEAP.  This resulted in two new config options in the eap-ttls, and eap-peap blocks.  cncheck should be the name (or partial name) to match in the CN.  cnexact should be yes/no depending on if we want to match the CN exactly, or just see if our substring is in the CN.
 *
 * Revision 1.19  2004/02/16 14:23:49  npetroni
 * updated config code to allow empty method fields in the config file. The format
 * is
 *
 * eap_method {
 *
 * }
 *
 * the semantics are to create a structure of that type and put it in the list for that network, but not to initialize any of the values (they remain NULL, 0, or whatever malloc gives us).
 *
 * Revision 1.18  2004/02/10 03:40:22  npetroni
 * updated config to include a phase 2 identity for PEAP
 *
 * Revision 1.17  2004/01/06 22:25:58  npetroni
 * added crl parameter to tls, ttls, and peap and user cert,key,key_pass to ttls,peap
 *
 * Revision 1.16  2003/12/31 16:16:35  npetroni
 * made some generalizations to the way config code works so that now
 * it is easy to let any method be put inside of PEAP with little effort.
 *
 * Added MD5, SIM to the PEAP config section.
 *
 * Added allow types for OTP and GTC- we still need configuration parameters
 *   for these methods though.
 *
 * this code is coming together I think.
 *
 * Revision 1.15  2003/12/31 07:03:48  npetroni
 * made a number of changes to the config code to generalize handling of EAP
 * methods and phase2. I still need to go back and make the parser work for
 * other phase2 type in PEAP, but the backend is there.
 *
 * Revision 1.14  2003/12/19 23:19:11  npetroni
 * updated config code and test example. Fixed a couple things
 *   1. added new variables to globals:
 *      startup_command
 *      first_auth_command
 *      reauth_command
 *      auth_period
 *      held_period
 *      max_starts
 *      allow_interfaces
 *      deny_ineterfaces
 *
 *   2. added new variables to network:
 *      dest_mac
 *
 *   3. added new variables to ttls:
 *      phase2_type
 *
 *   4. added new variables to peap:
 *      allow_types
 *
 *   5. layed the groundwork for "preferred types" to be sent in Nak
 *
 * Revision 1.13  2003/12/10 14:13:16  npetroni
 * updated configuration code to parse all types. example updated as well
 *
 * Revision 1.12  2003/11/29 01:11:30  npetroni
 * Added first round of configuration code.
 * Structural Changes:
 *    added examle config file and finished config-parser to test configuration
 *    files and optionally dump the output
 *
 * Current Status:
 *   Have not added parameters for any other method than TLS so we can discuss
 *   the changes before doing so.
 *
 *   Did not update config_build() so chris can keep testing as before.
 *
 *
 *******************************************************************/  
  
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
  
#include "config.h"
#include "xsup_err.h"
#include "xsup_debug.h"

// there has GOT to be a better way than this...
#include "eap_types/md5/eapmd5.h"
#include "eap_types/tls/eaptls.h"
#include "eap_types/ttls/eapttls.h"
#include "eap_types/mschapv2/eapmschapv2.h"
#include "eap_types/peap/eappeap.h"
#include "eap_types/leap/eapleap.h"
#ifndef EAP_SIM_ENABLE
#define EAP_SIM_ENABLE
#endif 
#include "eap_types/sim/eapsim.h"

#define CLEAN_EXIT cleanup_parse(); return XECONFIGPARSEFAIL

int yylex(void);  
int yyerror(char *err);
extern struct config_data *config_info;

extern int config_parse_debug;

struct config_data *tmp_config = NULL;

struct config_eap_tls *tmp_tls = NULL;
struct config_eap_md5 *tmp_md5 = NULL;
struct config_eap_ttls *tmp_ttls = NULL;
struct config_eap_leap *tmp_leap = NULL;
struct config_eap_mschapv2 *tmp_mschapv2 = NULL;
struct config_eap_peap *tmp_peap = NULL; 
struct config_eap_sim *tmp_sim = NULL;

struct config_pap *tmp_p2pap =NULL;
struct config_chap *tmp_p2chap = NULL;
struct config_mschap *tmp_p2mschap = NULL;
struct config_mschapv2 *tmp_p2mschapv2 = NULL;

struct config_network *tmp_network = NULL;



void set_current_tls() {
  if (tmp_tls == NULL) 
    initialize_config_eap_tls(&tmp_tls);
} 
void set_current_md5() {
  if (tmp_md5 == NULL) 
    initialize_config_eap_md5(&tmp_md5);
} 
void set_current_ttls() {
  if (tmp_ttls == NULL) 
    initialize_config_eap_ttls(&tmp_ttls);
} 
void set_current_leap() {
  if (tmp_leap == NULL) 
    initialize_config_eap_leap(&tmp_leap);
} 
void set_current_mschapv2() {
  if (tmp_mschapv2 == NULL) 
    initialize_config_eap_mschapv2(&tmp_mschapv2);
} 
void set_current_peap() {
  if (tmp_peap == NULL) 
    initialize_config_eap_peap(&tmp_peap);
} 
void set_current_sim() {
  if (tmp_sim == NULL) 
    initialize_config_eap_sim(&tmp_sim);
} 

void set_current_p2pap() {
  if (tmp_p2pap == NULL)
    initialize_config_pap(&tmp_p2pap);
}
void set_current_p2chap() {
  if (tmp_p2chap == NULL)
    initialize_config_chap(&tmp_p2chap);
}
void set_current_p2mschap() {
  if (tmp_p2mschap == NULL)
    initialize_config_mschap(&tmp_p2mschap);
}
void set_current_p2mschapv2() {
  if (tmp_p2mschapv2 == NULL)
    initialize_config_mschapv2(&tmp_p2mschapv2);
}

void set_current_config() {
  if (tmp_config == NULL) 
    initialize_config_data(&tmp_config);
} 

void set_current_globals() {
  set_current_config();
  if (!tmp_config->globals)
    initialize_config_globals(&(tmp_config->globals));
}   

void set_current_network() {
  if (tmp_network == NULL) 
    initialize_config_network(&tmp_network);
} 


void cleanup_parse()
{
  if (tmp_config)
    delete_config_data(&tmp_config);
  if (tmp_tls)
    delete_config_eap_tls(&tmp_tls);
  if (tmp_md5)
    delete_config_eap_md5(&tmp_md5);
  if (tmp_ttls)
    delete_config_eap_ttls(&tmp_ttls);
  if (tmp_leap)
    delete_config_eap_leap(&tmp_leap);
  if (tmp_mschapv2)
    delete_config_eap_mschapv2(&tmp_mschapv2);
  if (tmp_peap)
    delete_config_eap_peap(&tmp_peap);
  if (tmp_sim)
    delete_config_eap_sim(&tmp_sim);
  if (tmp_p2pap)
    delete_config_pap(&tmp_p2pap);
  if (tmp_p2chap)
    delete_config_chap(&tmp_p2chap);
  if (tmp_p2mschap)
    delete_config_mschap(&tmp_p2mschap);
  if (tmp_p2mschapv2)
    delete_config_mschapv2(&tmp_p2mschapv2);
  if (tmp_network)
    delete_config_network(&tmp_network);
}



/* function to check if debug is on and if so print the message */
void parameter_debug(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (!config_parse_debug) return;

  vprintf(fmt, ap);
  va_end(ap);
}



/* Line 189 of yacc.c  */
#line 404 "config_grammar.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TK_NETWORK_LIST = 258,
     TK_DEFAULT_NETNAME = 259,
     TK_NETNAME = 260,
     TK_STARTUP_COMMAND = 261,
     TK_FIRST_AUTH_COMMAND = 262,
     TK_REAUTH_COMMAND = 263,
     TK_LOGFILE = 264,
     TK_AUTH_PERIOD = 265,
     TK_HELD_PERIOD = 266,
     TK_MAX_STARTS = 267,
     TK_ALLOW_INTERFACES = 268,
     TK_DENY_INTERFACES = 269,
     TK_ALL = 270,
     TK_TYPE = 271,
     TK_ALLOW_TYPES = 272,
     TK_WIRELESS = 273,
     TK_WIRED = 274,
     TK_CONTROL_WIRELESS = 275,
     TK_IDENTITY = 276,
     TK_IDENTITY_VAL = 277,
     TK_DEST_MAC = 278,
     TK_MACADDRESS = 279,
     TK_SSID = 280,
     TK_SSID_VAL = 281,
     TK_EAP_TLS = 282,
     TK_USER_CERT = 283,
     TK_USER_KEY = 284,
     TK_USER_KEY_PASS = 285,
     TK_SESSION_RESUME = 286,
     TK_CNCHECK = 287,
     TK_CNEXACT = 288,
     TK_ROOT_CERT = 289,
     TK_ROOT_DIR = 290,
     TK_CRL_DIR = 291,
     TK_CHUNK_SIZE = 292,
     TK_RANDOM_FILE = 293,
     TK_EAP_MD5 = 294,
     TK_USERNAME = 295,
     TK_USERNAME_VAL = 296,
     TK_PASSWORD = 297,
     TK_EAP_LEAP = 298,
     TK_EAP_TTLS = 299,
     TK_PHASE2_TYPE = 300,
     TK_PAP = 301,
     TK_CHAP = 302,
     TK_MSCHAP = 303,
     TK_MSCHAPV2 = 304,
     TK_EAP_MSCHAPV2 = 305,
     TK_EAP_PEAP = 306,
     TK_EAP_SIM = 307,
     TK_AUTO_REALM = 308,
     TK_YES = 309,
     TK_NO = 310,
     TK_EAP_GTC = 311,
     TK_EAP_OTP = 312,
     TK_NUMBER = 313,
     TK_FNAME = 314,
     TK_PASS = 315,
     TK_COMMAND = 316
   };
#endif
/* Tokens.  */
#define TK_NETWORK_LIST 258
#define TK_DEFAULT_NETNAME 259
#define TK_NETNAME 260
#define TK_STARTUP_COMMAND 261
#define TK_FIRST_AUTH_COMMAND 262
#define TK_REAUTH_COMMAND 263
#define TK_LOGFILE 264
#define TK_AUTH_PERIOD 265
#define TK_HELD_PERIOD 266
#define TK_MAX_STARTS 267
#define TK_ALLOW_INTERFACES 268
#define TK_DENY_INTERFACES 269
#define TK_ALL 270
#define TK_TYPE 271
#define TK_ALLOW_TYPES 272
#define TK_WIRELESS 273
#define TK_WIRED 274
#define TK_CONTROL_WIRELESS 275
#define TK_IDENTITY 276
#define TK_IDENTITY_VAL 277
#define TK_DEST_MAC 278
#define TK_MACADDRESS 279
#define TK_SSID 280
#define TK_SSID_VAL 281
#define TK_EAP_TLS 282
#define TK_USER_CERT 283
#define TK_USER_KEY 284
#define TK_USER_KEY_PASS 285
#define TK_SESSION_RESUME 286
#define TK_CNCHECK 287
#define TK_CNEXACT 288
#define TK_ROOT_CERT 289
#define TK_ROOT_DIR 290
#define TK_CRL_DIR 291
#define TK_CHUNK_SIZE 292
#define TK_RANDOM_FILE 293
#define TK_EAP_MD5 294
#define TK_USERNAME 295
#define TK_USERNAME_VAL 296
#define TK_PASSWORD 297
#define TK_EAP_LEAP 298
#define TK_EAP_TTLS 299
#define TK_PHASE2_TYPE 300
#define TK_PAP 301
#define TK_CHAP 302
#define TK_MSCHAP 303
#define TK_MSCHAPV2 304
#define TK_EAP_MSCHAPV2 305
#define TK_EAP_PEAP 306
#define TK_EAP_SIM 307
#define TK_AUTO_REALM 308
#define TK_YES 309
#define TK_NO 310
#define TK_EAP_GTC 311
#define TK_EAP_OTP 312
#define TK_NUMBER 313
#define TK_FNAME 314
#define TK_PASS 315
#define TK_COMMAND 316




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 331 "config_grammar.y"

        char    *str;
        int     num;



/* Line 214 of yacc.c  */
#line 569 "config_grammar.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 581 "config_grammar.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  31
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   454

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  183
/* YYNRULES -- Number of states.  */
#define YYNSTATES  362

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   316

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    63,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,     2,    65,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     8,    10,    12,    15,    17,    20,
      22,    26,    30,    34,    38,    42,    46,    50,    54,    58,
      62,    66,    70,    74,    76,    80,    82,    86,    88,    93,
      96,    99,   101,   103,   105,   107,   109,   111,   113,   115,
     119,   123,   127,   131,   135,   139,   143,   147,   151,   153,
     155,   157,   159,   161,   163,   165,   169,   171,   173,   175,
     177,   179,   181,   183,   185,   187,   189,   194,   198,   201,
     203,   207,   211,   215,   219,   223,   227,   231,   235,   239,
     243,   248,   252,   255,   257,   261,   265,   270,   274,   277,
     279,   283,   287,   291,   295,   299,   303,   307,   311,   315,
     319,   323,   327,   331,   335,   339,   343,   347,   349,   351,
     353,   355,   357,   362,   365,   367,   371,   375,   380,   383,
     385,   389,   393,   398,   401,   403,   407,   411,   416,   419,
     421,   425,   429,   434,   438,   441,   443,   447,   451,   456,
     460,   463,   465,   469,   473,   478,   482,   485,   487,   491,
     495,   499,   503,   507,   511,   515,   519,   523,   527,   531,
     535,   539,   543,   545,   547,   551,   555,   559,   561,   563,
     565,   567,   569,   571,   573,   575,   577,   582,   586,   589,
     591,   595,   599,   603
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      67,     0,    -1,    68,    69,    -1,    68,    -1,    69,    -1,
       1,    -1,    68,    70,    -1,    70,    -1,    69,    74,    -1,
      74,    -1,     3,    62,    15,    -1,     3,    62,    71,    -1,
       4,    62,     5,    -1,     6,    62,    61,    -1,     7,    62,
      61,    -1,     8,    62,    61,    -1,     9,    62,     5,    -1,
      10,    62,    58,    -1,    11,    62,    58,    -1,    12,    62,
      58,    -1,    13,    62,    72,    -1,    14,    62,    73,    -1,
      71,    63,     5,    -1,     5,    -1,    72,    63,     5,    -1,
       5,    -1,    73,    63,     5,    -1,     5,    -1,     5,    64,
      75,    65,    -1,    75,    76,    -1,    75,    83,    -1,    76,
      -1,    83,    -1,    77,    -1,    79,    -1,    81,    -1,    80,
      -1,    82,    -1,    78,    -1,    16,    62,    18,    -1,    16,
      62,    19,    -1,    20,    62,    54,    -1,    20,    62,    55,
      -1,    21,    62,    22,    -1,    25,    62,    26,    -1,    23,
      62,    24,    -1,    17,    62,    15,    -1,    17,    62,    84,
      -1,    86,    -1,    89,    -1,    92,    -1,   108,    -1,   111,
      -1,   114,    -1,   121,    -1,    84,    63,    85,    -1,    85,
      -1,    27,    -1,    39,    -1,    44,    -1,    43,    -1,    50,
      -1,    51,    -1,    52,    -1,    56,    -1,    57,    -1,    27,
      64,    87,    65,    -1,    27,    64,    65,    -1,    87,    88,
      -1,    88,    -1,    28,    62,    59,    -1,    29,    62,    59,
      -1,    30,    62,    60,    -1,    31,    62,    54,    -1,    31,
      62,    55,    -1,    34,    62,    59,    -1,    35,    62,    59,
      -1,    36,    62,    59,    -1,    37,    62,    58,    -1,    38,
      62,    59,    -1,    39,    64,    90,    65,    -1,    39,    64,
      65,    -1,    90,    91,    -1,    91,    -1,    40,    62,    41,
      -1,    42,    62,    60,    -1,    44,    64,    93,    65,    -1,
      44,    64,    65,    -1,    93,    94,    -1,    94,    -1,    28,
      62,    59,    -1,    29,    62,    59,    -1,    30,    62,    60,
      -1,    34,    62,    59,    -1,    35,    62,    59,    -1,    36,
      62,    59,    -1,    37,    62,    58,    -1,    38,    62,    59,
      -1,    31,    62,    54,    -1,    31,    62,    55,    -1,    32,
      62,    59,    -1,    33,    62,    54,    -1,    33,    62,    55,
      -1,    45,    62,    46,    -1,    45,    62,    47,    -1,    45,
      62,    48,    -1,    45,    62,    49,    -1,    95,    -1,    96,
      -1,    99,    -1,   102,    -1,   105,    -1,    46,    64,    97,
      65,    -1,    97,    98,    -1,    98,    -1,    40,    62,    41,
      -1,    42,    62,    60,    -1,    47,    64,   100,    65,    -1,
     100,   101,    -1,   101,    -1,    40,    62,    41,    -1,    42,
      62,    60,    -1,    48,    64,   103,    65,    -1,   103,   104,
      -1,   104,    -1,    40,    62,    41,    -1,    42,    62,    60,
      -1,    49,    64,   106,    65,    -1,   106,   107,    -1,   107,
      -1,    40,    62,    41,    -1,    42,    62,    60,    -1,    43,
      64,   109,    65,    -1,    43,    64,    65,    -1,   109,   110,
      -1,   110,    -1,    40,    62,    41,    -1,    42,    62,    60,
      -1,    50,    64,   112,    65,    -1,    50,    64,    65,    -1,
     112,   113,    -1,   113,    -1,    40,    62,    41,    -1,    42,
      62,    60,    -1,    51,    64,   115,    65,    -1,    51,    64,
      65,    -1,   115,   116,    -1,   116,    -1,    21,    62,    22,
      -1,    28,    62,    59,    -1,    29,    62,    59,    -1,    30,
      62,    60,    -1,    34,    62,    59,    -1,    35,    62,    59,
      -1,    36,    62,    59,    -1,    31,    62,    54,    -1,    31,
      62,    55,    -1,    37,    62,    58,    -1,    32,    62,    59,
      -1,    33,    62,    54,    -1,    33,    62,    55,    -1,    38,
      62,    59,    -1,   117,    -1,   120,    -1,    17,    62,    15,
      -1,    17,    62,   118,    -1,   118,    63,   119,    -1,   119,
      -1,    50,    -1,    39,    -1,    52,    -1,    57,    -1,    56,
      -1,   111,    -1,    89,    -1,   121,    -1,    52,    64,   122,
      65,    -1,    52,    64,    65,    -1,   122,   123,    -1,   123,
      -1,    40,    62,    41,    -1,    42,    62,    60,    -1,    53,
      62,    54,    -1,    53,    62,    55,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   403,   403,   407,   411,   416,   421,   422,   425,   426,
     429,   434,   437,   445,   453,   461,   469,   480,   487,   494,
     501,   504,   509,   519,   531,   547,   565,   581,   599,   620,
     621,   622,   623,   627,   628,   629,   630,   631,   632,   635,
     641,   649,   655,   663,   673,   683,   709,   714,   717,   729,
     740,   751,   762,   773,   784,   797,   798,   801,   806,   811,
     816,   821,   826,   831,   836,   841,   848,   849,   854,   855,
     858,   866,   874,   882,   888,   894,   902,   910,   918,   924,
     934,   935,   940,   941,   944,   952,   962,   963,   968,   969,
     972,   980,   988,   996,  1004,  1012,  1020,  1026,  1034,  1040,
    1046,  1054,  1059,  1064,  1074,  1084,  1094,  1104,  1107,  1108,
    1109,  1110,  1113,  1126,  1127,  1130,  1138,  1148,  1161,  1162,
    1165,  1173,  1183,  1196,  1197,  1200,  1208,  1219,  1232,  1233,
    1236,  1244,  1254,  1255,  1260,  1261,  1264,  1272,  1282,  1283,
    1288,  1289,  1292,  1300,  1310,  1311,  1316,  1317,  1320,  1328,
    1336,  1344,  1352,  1360,  1368,  1376,  1382,  1388,  1394,  1402,
    1407,  1412,  1420,  1421,  1424,  1429,  1432,  1433,  1436,  1441,
    1446,  1451,  1456,  1464,  1475,  1486,  1499,  1500,  1505,  1506,
    1509,  1517,  1525,  1530
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TK_NETWORK_LIST", "TK_DEFAULT_NETNAME",
  "TK_NETNAME", "TK_STARTUP_COMMAND", "TK_FIRST_AUTH_COMMAND",
  "TK_REAUTH_COMMAND", "TK_LOGFILE", "TK_AUTH_PERIOD", "TK_HELD_PERIOD",
  "TK_MAX_STARTS", "TK_ALLOW_INTERFACES", "TK_DENY_INTERFACES", "TK_ALL",
  "TK_TYPE", "TK_ALLOW_TYPES", "TK_WIRELESS", "TK_WIRED",
  "TK_CONTROL_WIRELESS", "TK_IDENTITY", "TK_IDENTITY_VAL", "TK_DEST_MAC",
  "TK_MACADDRESS", "TK_SSID", "TK_SSID_VAL", "TK_EAP_TLS", "TK_USER_CERT",
  "TK_USER_KEY", "TK_USER_KEY_PASS", "TK_SESSION_RESUME", "TK_CNCHECK",
  "TK_CNEXACT", "TK_ROOT_CERT", "TK_ROOT_DIR", "TK_CRL_DIR",
  "TK_CHUNK_SIZE", "TK_RANDOM_FILE", "TK_EAP_MD5", "TK_USERNAME",
  "TK_USERNAME_VAL", "TK_PASSWORD", "TK_EAP_LEAP", "TK_EAP_TTLS",
  "TK_PHASE2_TYPE", "TK_PAP", "TK_CHAP", "TK_MSCHAP", "TK_MSCHAPV2",
  "TK_EAP_MSCHAPV2", "TK_EAP_PEAP", "TK_EAP_SIM", "TK_AUTO_REALM",
  "TK_YES", "TK_NO", "TK_EAP_GTC", "TK_EAP_OTP", "TK_NUMBER", "TK_FNAME",
  "TK_PASS", "TK_COMMAND", "'='", "','", "'{'", "'}'", "$accept",
  "configfile", "global_section", "network_section", "global_statement",
  "network_list", "allow_interface_list", "deny_interface_list",
  "network_entry", "network_statements", "network_parameter",
  "network_type_parameter", "network_control_wireless",
  "network_identity_parameter", "network_ssid_parameter",
  "network_dest_mac_parameter", "network_allow_parameter",
  "eap_type_statement", "eap_type_list", "eap_type", "eap_tls_statement",
  "eap_tls_params", "eap_tls_param", "eap_md5_statement", "eap_md5_params",
  "eap_md5_param", "eap_ttls_statement", "eap_ttls_params",
  "eap_ttls_param", "eap_ttls_phase2_statement", "phase2_pap_statement",
  "phase2_pap_params", "phase2_pap_param", "phase2_chap_statement",
  "phase2_chap_params", "phase2_chap_param", "phase2_mschap_statement",
  "phase2_mschap_params", "phase2_mschap_param",
  "phase2_mschapv2_statement", "phase2_mschapv2_params",
  "phase2_mschapv2_param", "eap_leap_statement", "eap_leap_params",
  "eap_leap_param", "eap_mschapv2_statement", "eap_mschapv2_params",
  "eap_mschapv2_param", "eap_peap_statement", "eap_peap_params",
  "eap_peap_param", "eap_peap_allow_parameter",
  "eap_peap_phase2_type_list", "eap_peap_phase2_type",
  "eap_peap_phase2_statement", "eap_sim_statement", "eap_sim_params",
  "eap_sim_param", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,    61,    44,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    66,    67,    67,    67,    67,    68,    68,    69,    69,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    71,    71,    72,    72,    73,    73,    74,    75,
      75,    75,    75,    76,    76,    76,    76,    76,    76,    77,
      77,    78,    78,    79,    80,    81,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    84,    84,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    86,    86,    87,    87,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      89,    89,    90,    90,    91,    91,    92,    92,    93,    93,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    95,    95,
      95,    95,    96,    97,    97,    98,    98,    99,   100,   100,
     101,   101,   102,   103,   103,   104,   104,   105,   106,   106,
     107,   107,   108,   108,   109,   109,   110,   110,   111,   111,
     112,   112,   113,   113,   114,   114,   115,   115,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   117,   117,   118,   118,   119,   119,
     119,   119,   119,   120,   120,   120,   121,   121,   122,   122,
     123,   123,   123,   123
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     2,     1,     2,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     3,     1,     3,     1,     4,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     3,     2,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     3,     2,     1,     3,     3,     4,     3,     2,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     4,     2,     1,     3,     3,     4,     2,     1,
       3,     3,     4,     2,     1,     3,     3,     4,     2,     1,
       3,     3,     4,     3,     2,     1,     3,     3,     4,     3,
       2,     1,     3,     3,     4,     3,     2,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     3,     2,     1,
       3,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     5,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     4,     7,     9,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     2,     6,     8,    23,    10,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    31,    33,    38,    34,    36,    35,    37,
      32,    48,    49,    50,    51,    52,    53,    54,    13,    14,
      15,    16,    17,    18,    19,    25,    20,    27,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    30,     0,     0,    22,    39,
      40,    46,    57,    58,    60,    59,    61,    62,    63,    64,
      65,    47,    56,    41,    42,    43,    45,    44,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    67,     0,    69,
       0,     0,    81,     0,    83,     0,     0,   133,     0,   135,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,     0,    89,   107,
     108,   109,   110,   111,     0,     0,   139,     0,   141,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   145,   174,   173,     0,   147,   162,   163,   175,
       0,     0,     0,   177,     0,   179,    24,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    66,    68,
       0,     0,    80,    82,     0,     0,   132,   134,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,    88,     0,     0,   138,   140,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,   146,     0,     0,     0,   176,   178,
      55,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    84,    85,   136,   137,    90,    91,    92,    98,    99,
     100,   101,   102,    93,    94,    95,    96,    97,   103,   104,
     105,   106,     0,     0,     0,   114,     0,     0,     0,   119,
       0,     0,     0,   124,     0,     0,     0,   129,   142,   143,
     164,   169,   168,   170,   172,   171,   165,   167,   148,   149,
     150,   151,   155,   156,   158,   159,   160,   152,   153,   154,
     157,   161,   180,   181,   182,   183,     0,     0,   112,   113,
       0,     0,   117,   118,     0,     0,   122,   123,     0,     0,
     127,   128,     0,   115,   116,   120,   121,   125,   126,   130,
     131,   166
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    14,    15,    16,    17,    37,    76,    78,    18,    52,
      53,    54,    55,    56,    57,    58,    59,    60,   111,   112,
      61,   128,   129,    62,   133,   134,    63,   157,   158,   159,
     160,   294,   295,   161,   298,   299,   162,   302,   303,   163,
     306,   307,    64,   138,   139,    65,   167,   168,    66,   185,
     186,   187,   316,   317,   188,    67,   194,   195
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -92
static const yytype_int16 yypact[] =
{
     242,   -92,   -49,   -42,   -32,    -6,    -2,    18,    23,    60,
      75,    79,    98,   101,   112,   263,   113,   -92,   -92,    10,
     169,   175,   124,   136,   140,   198,   115,   146,   151,   207,
     208,   -92,   113,   -92,   -92,   -92,   -92,   157,   -92,   159,
     182,   197,   201,   216,   217,   167,   194,   218,   219,   220,
     221,   222,   -13,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   224,   -92,   225,   257,
       9,    64,   -19,   258,   265,   255,   141,    13,    48,    97,
      96,    12,   -31,   -92,   -92,   -92,   285,   286,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   229,   -92,   -92,   -92,   -92,   -92,   -92,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   -92,   152,   -92,
     240,   241,   -92,   165,   -92,   243,   244,   -92,   168,   -92,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     256,   259,   260,   261,   262,   264,   -92,   119,   -92,   -92,
     -92,   -92,   -92,   -92,   267,   268,   -92,   192,   -92,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   -92,   -92,   -92,    37,   -92,   -92,   -92,   -92,
     282,   283,   284,   -92,    44,   -92,   -92,   -92,   172,   288,
     289,   290,    38,   292,   293,   294,   291,   295,   -92,   -92,
     314,   296,   -92,   -92,   316,   298,   -92,   -92,   300,   301,
     302,    50,   304,    56,   305,   306,   307,   303,   308,    52,
     -24,    17,    21,    41,   -92,   -92,   327,   309,   -92,   -92,
      67,   297,   311,   312,   313,    85,   315,   104,   317,   318,
     319,   321,   322,   -92,   -92,   331,   320,   139,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92,   323,   324,   195,   -92,   325,   326,   196,   -92,
     328,   329,   199,   -92,   330,   332,   200,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   333,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92,   -92,   -92,   -92,   -92,   334,   335,   -92,   -92,
     341,   337,   -92,   -92,   342,   338,   -92,   -92,   343,   339,
     -92,   -92,   -33,   -92,   -92,   -92,   -92,   -92,   -92,   -92,
     -92,   -92
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -92,   -92,   -92,   374,   378,   -92,   -92,   -92,   -11,   -92,
     348,   -92,   -92,   -92,   -92,   -92,   -92,   349,   -92,   106,
     -92,   -92,   189,   -91,   -92,   187,   -92,   -92,   166,   -92,
     -92,   -92,    28,   -92,   -92,    29,   -92,   -92,   100,   -92,
     -92,    99,   -92,   -92,   266,   -90,   -92,   287,   -92,   -92,
     223,   -92,   -92,    51,   -92,   -89,   -92,   212
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     183,   184,   189,    39,    40,    34,   311,    41,    42,   190,
      43,   191,    44,    19,    45,    35,   292,   312,   293,   313,
      20,    34,   192,   314,   315,    36,    46,    99,   100,   169,
      47,    48,    21,   170,   193,   113,   114,    49,    50,    51,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,    46,    93,   130,   169,   131,    22,   296,   170,   297,
      23,   300,    49,   301,    51,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,    46,   182,   132,   101,
      24,   304,   310,   305,   190,    25,   191,    49,   135,    51,
     136,   102,   264,   265,   183,   184,   189,   192,   288,   289,
     290,   291,   253,   103,   278,   279,   311,   104,   105,   258,
     281,   282,    31,   137,   106,   107,   108,   312,     4,   313,
     109,   110,    26,   314,   315,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   164,    27,   165,   322,
     323,    28,   151,   152,   153,   154,   155,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   325,   326,
      29,   166,   156,    30,   151,   152,   153,   154,   155,   118,
     119,   120,   121,    72,    38,   122,   123,   124,   125,   126,
     118,   119,   120,   121,   234,    68,   122,   123,   124,   125,
     126,    39,    40,   334,   335,    41,    42,    69,    43,   102,
      44,    70,    45,    71,    73,   130,   127,   131,   135,    74,
     136,   103,    75,    77,    46,   104,   105,   208,    47,    48,
      79,    80,   106,   107,   108,    49,    50,    51,   109,   110,
     212,    86,   164,   216,   165,   292,   296,   293,   297,   300,
     304,   301,   305,     1,    81,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,   238,    87,    82,
     338,   342,    98,    83,   346,   350,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    84,    85,
     115,   117,    88,    89,    90,    91,    92,    96,    97,   116,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   210,   211,   260,   214,   215,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   209,   228,   318,
     213,   229,   339,   235,   230,   231,   232,   343,   233,   236,
     237,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   255,   256,   257,   261,   262,   269,
     263,   266,   267,   268,   270,   271,   272,   273,   274,   275,
     276,   286,   277,   280,   283,   284,   285,   287,   308,   309,
     319,   320,   332,   321,   324,   353,   327,   328,   329,   330,
     333,   331,   355,   357,   359,   336,   337,   340,   341,    32,
     344,   345,   348,    33,   349,   354,   352,   356,   358,   360,
      94,    95,   347,   361,   217,   351,   259,     0,   254,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239
};

static const yytype_int16 yycheck[] =
{
      91,    91,    91,    16,    17,    16,    39,    20,    21,    40,
      23,    42,    25,    62,    27,     5,    40,    50,    42,    52,
      62,    32,    53,    56,    57,    15,    39,    18,    19,    17,
      43,    44,    64,    21,    65,    54,    55,    50,    51,    52,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    65,    40,    17,    42,    62,    40,    21,    42,
      62,    40,    50,    42,    52,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    65,    65,    15,
      62,    40,    15,    42,    40,    62,    42,    50,    40,    52,
      42,    27,    54,    55,   185,   185,   185,    53,    46,    47,
      48,    49,    65,    39,    54,    55,    39,    43,    44,    65,
      54,    55,     0,    65,    50,    51,    52,    50,     5,    52,
      56,    57,    62,    56,    57,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    40,    62,    42,    54,
      55,    62,    45,    46,    47,    48,    49,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    54,    55,
      62,    65,    65,    62,    45,    46,    47,    48,    49,    28,
      29,    30,    31,    58,     5,    34,    35,    36,    37,    38,
      28,    29,    30,    31,    65,    61,    34,    35,    36,    37,
      38,    16,    17,    54,    55,    20,    21,    61,    23,    27,
      25,    61,    27,     5,    58,    40,    65,    42,    40,    58,
      42,    39,     5,     5,    39,    43,    44,    65,    43,    44,
      63,    62,    50,    51,    52,    50,    51,    52,    56,    57,
      65,    64,    40,    65,    42,    40,    40,    42,    42,    40,
      40,    42,    42,     1,    62,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    65,    64,    62,
      65,    65,     5,    62,    65,    65,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    62,    62,
      22,    26,    64,    64,    64,    64,    64,    63,    63,    24,
       5,     5,    63,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,   198,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    62,    62,   128,    62,    22,
     133,    62,   294,   157,    64,    64,    64,   298,    64,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    59,    59,    58,
      60,    59,    59,    59,    59,    41,    60,    41,    60,    59,
      59,    58,    60,    59,    59,    59,    59,    59,    41,    60,
      59,    59,    41,    60,    59,    41,    59,    59,    59,    58,
      60,    59,    41,    41,    41,    62,    62,    62,    62,    15,
      62,    62,    62,    15,    62,    60,    63,    60,    60,    60,
      52,    52,   302,   352,   138,   306,   194,    -1,   185,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   167
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    67,    68,    69,    70,    74,    62,
      62,    64,    62,    62,    62,    62,    62,    62,    62,    62,
      62,     0,    69,    70,    74,     5,    15,    71,     5,    16,
      17,    20,    21,    23,    25,    27,    39,    43,    44,    50,
      51,    52,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    86,    89,    92,   108,   111,   114,   121,    61,    61,
      61,     5,    58,    58,    58,     5,    72,     5,    73,    63,
      62,    62,    62,    62,    62,    62,    64,    64,    64,    64,
      64,    64,    64,    65,    76,    83,    63,    63,     5,    18,
      19,    15,    27,    39,    43,    44,    50,    51,    52,    56,
      57,    84,    85,    54,    55,    22,    24,    26,    28,    29,
      30,    31,    34,    35,    36,    37,    38,    65,    87,    88,
      40,    42,    65,    90,    91,    40,    42,    65,   109,   110,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    45,    46,    47,    48,    49,    65,    93,    94,    95,
      96,    99,   102,   105,    40,    42,    65,   112,   113,    17,
      21,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    65,    89,   111,   115,   116,   117,   120,   121,
      40,    42,    53,    65,   122,   123,     5,     5,    63,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    65,    88,
      62,    62,    65,    91,    62,    62,    65,   110,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      64,    64,    64,    64,    65,    94,    62,    62,    65,   113,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    65,   116,    62,    62,    62,    65,   123,
      85,    59,    59,    60,    54,    55,    59,    59,    59,    58,
      59,    41,    60,    41,    60,    59,    59,    60,    54,    55,
      59,    54,    55,    59,    59,    59,    58,    59,    46,    47,
      48,    49,    40,    42,    97,    98,    40,    42,   100,   101,
      40,    42,   103,   104,    40,    42,   106,   107,    41,    60,
      15,    39,    50,    52,    56,    57,   118,   119,    22,    59,
      59,    60,    54,    55,    59,    54,    55,    59,    59,    59,
      58,    59,    41,    60,    54,    55,    62,    62,    65,    98,
      62,    62,    65,   101,    62,    62,    65,   104,    62,    62,
      65,   107,    63,    41,    60,    41,    60,    41,    60,    41,
      60,   119
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 403 "config_grammar.y"
    {
		     config_info = tmp_config; 
		     tmp_config = NULL;
                  }
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 407 "config_grammar.y"
    { 
                      debug_printf(DEBUG_NORMAL, "Error: No networks defined.\n"); 
		      CLEAN_EXIT;
		    }
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 411 "config_grammar.y"
    {
		      debug_printf(DEBUG_NORMAL, "Error: No globals defined.\n"); 
		      cleanup_parse();
		      return XECONFIGPARSEFAIL;
                    }
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 416 "config_grammar.y"
    {
 		      cleanup_parse();
		      return XECONFIGPARSEFAIL; }
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 429 "config_grammar.y"
    {
                      set_current_globals();
                      parameter_debug("network_list: all\n");
		      // do nothing. leave null
                    }
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 434 "config_grammar.y"
    {
		    // done below. nothing to do here
  		    }
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 437 "config_grammar.y"
    {
 		     set_current_globals();
		     parameter_debug("Default network: \"%s\"\n", (yyvsp[(3) - (3)].str));
		     if (tmp_config->globals->default_net)
		       free((yyvsp[(3) - (3)].str));
		     else
		       tmp_config->globals->default_net = (yyvsp[(3) - (3)].str);
		  }
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 445 "config_grammar.y"
    {
 		     set_current_globals();
		     parameter_debug("Startup command: \"%s\"\n", (yyvsp[(3) - (3)].str));
		     if (tmp_config->globals->startup_command)
		       free((yyvsp[(3) - (3)].str));
		     else
		       tmp_config->globals->startup_command = (yyvsp[(3) - (3)].str);
		    }
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 453 "config_grammar.y"
    {
 		     set_current_globals();
		     parameter_debug("First_Auth command: \"%s\"\n", (yyvsp[(3) - (3)].str));
		     if (tmp_config->globals->first_auth_command)
		       free((yyvsp[(3) - (3)].str));
		     else
		       tmp_config->globals->first_auth_command = (yyvsp[(3) - (3)].str);
		    }
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 461 "config_grammar.y"
    {
 		     set_current_globals();
		     parameter_debug("Reauth command: \"%s\"\n", (yyvsp[(3) - (3)].str));
		     if (tmp_config->globals->reauth_command)
		       free((yyvsp[(3) - (3)].str));
		     else
		       tmp_config->globals->reauth_command = (yyvsp[(3) - (3)].str);
		    }
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 469 "config_grammar.y"
    {
		     set_current_globals();
		     parameter_debug("Logfile: \"%s\"\n", (yyvsp[(3) - (3)].str));
		     if (tmp_config->globals->logfile)
		       {
			 free((yyvsp[(3) - (3)].str));
			 tmp_config->globals->logfile = NULL;
		       }
		     else
		       tmp_config->globals->logfile = (yyvsp[(3) - (3)].str);
		    }
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 480 "config_grammar.y"
    {
		     set_current_globals();
		     if (!TEST_FLAG(tmp_config->globals->flags, CONFIG_GLOBALS_AUTH_PER)) {
		       SET_FLAG(tmp_config->globals->flags, CONFIG_GLOBALS_AUTH_PER);
		       tmp_config->globals->auth_period = (yyvsp[(3) - (3)].num);
		     }
                    }
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 487 "config_grammar.y"
    {
		     set_current_globals();
		     if (!TEST_FLAG(tmp_config->globals->flags, CONFIG_GLOBALS_HELD_PER)) {
		       SET_FLAG(tmp_config->globals->flags, CONFIG_GLOBALS_HELD_PER);
		       tmp_config->globals->held_period = (yyvsp[(3) - (3)].num);
		     }
                    }
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 494 "config_grammar.y"
    {
		     set_current_globals();
		     if (!TEST_FLAG(tmp_config->globals->flags, CONFIG_GLOBALS_MAX_STARTS)) {
		       SET_FLAG(tmp_config->globals->flags, CONFIG_GLOBALS_MAX_STARTS);
		       tmp_config->globals->max_starts = (yyvsp[(3) - (3)].num);
		     }
                    }
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 501 "config_grammar.y"
    {
                     // nothing to do here
                    }
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 504 "config_grammar.y"
    {
                     // nothing to do here
                    }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 509 "config_grammar.y"
    {
                       parameter_debug("network_list: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_globals();
		       if (config_string_list_contains_string(tmp_config->globals->allowed_nets,
							    (yyvsp[(3) - (3)].str)))
			 free((yyvsp[(3) - (3)].str));
		       else 
			 config_string_list_add_string(&tmp_config->globals->allowed_nets,
						     (yyvsp[(3) - (3)].str));
                    }
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 519 "config_grammar.y"
    { 
                       parameter_debug("network_list: \"%s\"\n", (yyvsp[(1) - (1)].str));
		       set_current_globals();
		       if (config_string_list_contains_string(tmp_config->globals->allowed_nets,
							    (yyvsp[(1) - (1)].str)))
			 free((yyvsp[(1) - (1)].str));
		       else 
			 config_string_list_add_string(&tmp_config->globals->allowed_nets,
						     (yyvsp[(1) - (1)].str));
                    }
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 531 "config_grammar.y"
    {
                       parameter_debug("allow_interface_list: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_globals();
		       if (config_string_list_contains_string(tmp_config->globals->allow_interfaces,
							      (yyvsp[(3) - (3)].str)))
			 free((yyvsp[(3) - (3)].str));
		       else if (config_string_list_contains_string(tmp_config->globals->deny_interfaces,
								   (yyvsp[(3) - (3)].str))) {
			 debug_printf(DEBUG_NORMAL,
				      "Interface \"%s\" both allowed and denied\n", (yyvsp[(3) - (3)].str));
			 CLEAN_EXIT;
		       }
		       else 
			 config_string_list_add_string(&tmp_config->globals->allow_interfaces,
						     (yyvsp[(3) - (3)].str));
                    }
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 547 "config_grammar.y"
    { 
                       parameter_debug("allow_interface_list: \"%s\"\n", (yyvsp[(1) - (1)].str));
		       set_current_globals();
		       if (config_string_list_contains_string(tmp_config->globals->allow_interfaces,
							    (yyvsp[(1) - (1)].str)))
			 free((yyvsp[(1) - (1)].str));
		       else if (config_string_list_contains_string(tmp_config->globals->deny_interfaces,
								   (yyvsp[(1) - (1)].str))) {
			 debug_printf(DEBUG_NORMAL,
				      "Interface \"%s\" both allowed and denied\n", (yyvsp[(1) - (1)].str));
			 CLEAN_EXIT;
		       }
		       else 
			 config_string_list_add_string(&tmp_config->globals->allow_interfaces,
						     (yyvsp[(1) - (1)].str));
                    }
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 565 "config_grammar.y"
    {
                       parameter_debug("deny_interface_list: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_globals();
		       if (config_string_list_contains_string(tmp_config->globals->deny_interfaces,
							      (yyvsp[(3) - (3)].str)))
			 free((yyvsp[(3) - (3)].str));
		       else if (config_string_list_contains_string(tmp_config->globals->allow_interfaces,
								   (yyvsp[(3) - (3)].str))) {
			 debug_printf(DEBUG_NORMAL,
				      "Interface \"%s\" both allowed and denied\n", (yyvsp[(3) - (3)].str));
			 CLEAN_EXIT;
		       }
		       else 
			 config_string_list_add_string(&tmp_config->globals->deny_interfaces,
						     (yyvsp[(3) - (3)].str));
                    }
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 581 "config_grammar.y"
    { 
                       parameter_debug("deny_interface_list: \"%s\"\n", (yyvsp[(1) - (1)].str));
		       set_current_globals();
		       if (config_string_list_contains_string(tmp_config->globals->deny_interfaces,
							    (yyvsp[(1) - (1)].str)))
			 free((yyvsp[(1) - (1)].str));
		       else if (config_string_list_contains_string(tmp_config->globals->allow_interfaces,
								   (yyvsp[(1) - (1)].str))) {
			 debug_printf(DEBUG_NORMAL,
				      "Interface \"%s\" both allowed and denied\n", (yyvsp[(1) - (1)].str));
			 CLEAN_EXIT;
		       }
		       else 
			 config_string_list_add_string(&tmp_config->globals->deny_interfaces,
						     (yyvsp[(1) - (1)].str));
                    }
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 599 "config_grammar.y"
    {
                      set_current_config();
		      tmp_network->name = (yyvsp[(1) - (4)].str);
		      // check if there is a networks field and that 
		      // the current tmp is not already listed
		      if ((!tmp_config->networks ||
			  !config_network_contains_net(tmp_config->networks,
						       tmp_network->name)) &&
			  config_allows_network(tmp_config, tmp_network->name))
		      {
			config_network_add_net(&(tmp_config->networks),
					       tmp_network);
		      }
		      // if we don't need it, delete it
		      else {
			delete_config_network(&tmp_network);
		      }
		      tmp_network = NULL;
                    }
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 635 "config_grammar.y"
    {
                           parameter_debug("Type: Wireless\n");
			   set_current_network();
			   if (tmp_network->type == UNSET)
			     tmp_network->type = WIRELESS;
                         }
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 641 "config_grammar.y"
    {
                           parameter_debug("Type: Wired\n");
			   set_current_network();
			   if (tmp_network->type == UNSET)
			     tmp_network->type = WIRED;
                         }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 649 "config_grammar.y"
    {
                           parameter_debug("Control Wireless = YES\n");
			   set_current_network();
			   if (tmp_network->wireless_ctrl == CTL_UNSET)
			     tmp_network->wireless_ctrl = CTL_YES;
                         }
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 655 "config_grammar.y"
    {
			   parameter_debug("Control Wireless = NO\n");
			   set_current_network();
			   if (tmp_network->wireless_ctrl == CTL_UNSET)
			     tmp_network->wireless_ctrl = CTL_NO;
			 }
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 663 "config_grammar.y"
    {
                            parameter_debug("ID: \"%s\"\n", (yyvsp[(3) - (3)].str));
			    set_current_network();
			    if (!tmp_network->identity)
			      tmp_network->identity = (yyvsp[(3) - (3)].str);
			    else
			      free((yyvsp[(3) - (3)].str));
                          }
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 673 "config_grammar.y"
    {
                            parameter_debug("SSID: \"%s\"\n", (yyvsp[(3) - (3)].str));
			    set_current_network();
			    if (!tmp_network->ssid)
			      tmp_network->ssid = (yyvsp[(3) - (3)].str);
			    else
			      free((yyvsp[(3) - (3)].str));
                         }
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 683 "config_grammar.y"
    {
                            parameter_debug("Dest Mac: %s\n", (yyvsp[(3) - (3)].str));
			    set_current_network();
			    if (TEST_FLAG(tmp_network->flags, CONFIG_NET_DEST_MAC)) {
			      free((yyvsp[(3) - (3)].str));
			    }
			    else {
			      int tmp_dst_mac[CONFIG_MAC_LEN];
			      SET_FLAG(tmp_network->flags, CONFIG_NET_DEST_MAC);
			      sscanf((yyvsp[(3) - (3)].str), "%2x:%2x:%2x:%2x:%2x:%2x", 
				     &tmp_dst_mac[0], 
				     &tmp_dst_mac[1], 
				     &tmp_dst_mac[2], 
				     &tmp_dst_mac[3], 
				     &tmp_dst_mac[4], 
				     &tmp_dst_mac[5]);
			      tmp_network->dest_mac[0] = tmp_dst_mac[0];
			      tmp_network->dest_mac[1] = tmp_dst_mac[1];
			      tmp_network->dest_mac[2] = tmp_dst_mac[2];
			      tmp_network->dest_mac[3] = tmp_dst_mac[3];
			      tmp_network->dest_mac[4] = tmp_dst_mac[4];
			      tmp_network->dest_mac[5] = tmp_dst_mac[5];
			    }
                         }
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 709 "config_grammar.y"
    {
                           parameter_debug("Allow Types: ALL\n");
			   set_current_network();
			   SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_ALL);
                       }
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 717 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_TLS)) {
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_TLS,
					       tmp_tls);
		       }
		       else 
			 delete_config_eap_tls(&tmp_tls);
		       tmp_tls = NULL;
                      }
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 729 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_MD5))
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_MD5,
					       tmp_md5);
		       else 
			 delete_config_eap_md5(&tmp_md5);
		       tmp_md5 = NULL;
                      }
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 740 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_TTLS))
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_TTLS,
					       tmp_ttls);
		       else 
			 delete_config_eap_ttls(&tmp_ttls);
		       tmp_ttls = NULL;
                      }
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 751 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_LEAP))
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_LEAP,
					       tmp_leap);
		       else 
			 delete_config_eap_leap(&tmp_leap);
		       tmp_leap = NULL;
                      }
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 762 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_MSCHAPV2))
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_MSCHAPV2,
					       tmp_mschapv2);
		       else 
			 delete_config_eap_mschapv2(&tmp_mschapv2);
		       tmp_mschapv2 = NULL;
                      }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 773 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_PEAP))
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_PEAP,
					       tmp_peap);
		       else 
			 delete_config_eap_peap(&tmp_peap);
		       tmp_peap = NULL;
                      }
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 784 "config_grammar.y"
    {
                       set_current_network(); 
		       if (!config_eap_method_contains_method(tmp_network->methods,
							      EAP_TYPE_SIM))
			 add_config_eap_method(&(tmp_network->methods),
					       EAP_TYPE_SIM,
					       tmp_sim);
		       else 
			 delete_config_eap_sim(&tmp_sim);
		       tmp_sim = NULL;
                      }
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 801 "config_grammar.y"
    {
                        parameter_debug("Allow Type: TLS\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_TLS);
                      }
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 806 "config_grammar.y"
    {
                        parameter_debug("Allow Type: MD5\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_MD5);
                      }
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 811 "config_grammar.y"
    {
                        parameter_debug("Allow Type: TTLS\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_TTLS);
                      }
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 816 "config_grammar.y"
    {
                        parameter_debug("Allow Type: LEAP\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_LEAP);
                      }
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 821 "config_grammar.y"
    {
                        parameter_debug("Allow Type: MSCHAPV2\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_MSCV2);
                      }
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 826 "config_grammar.y"
    {
                        parameter_debug("Allow Type: PEAP\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_PEAP);
                      }
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 831 "config_grammar.y"
    {
                        parameter_debug("Allow Type: SIM\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_SIM);
                      }
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 836 "config_grammar.y"
    {
                        parameter_debug("Allow Type: GTC\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_GTC);
                      }
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 841 "config_grammar.y"
    {
                        parameter_debug("Allow Type: OTP\n");
			set_current_network();
			SET_FLAG(tmp_network->flags, CONFIG_NET_ALLOW_OTP);
                      }
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 849 "config_grammar.y"
    {
                        set_current_tls(); /* define an empty tls struct*/
                      }
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 858 "config_grammar.y"
    {
                        parameter_debug("tls user cert: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->user_cert)
			  tmp_tls->user_cert = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
                      }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 866 "config_grammar.y"
    {
	 	        parameter_debug("tls user key: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->user_key)
			  tmp_tls->user_key = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 874 "config_grammar.y"
    {
	 	        parameter_debug("tls user pass: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->user_key_pass)
			  tmp_tls->user_key_pass = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 882 "config_grammar.y"
    {
		        parameter_debug("Session Resumption = YES\n");
		        set_current_tls();
		        if (tmp_tls->session_resume == RES_UNSET)
			  tmp_tls->session_resume = RES_YES;
		      }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 888 "config_grammar.y"
    {
			parameter_debug("Session Resumption = NO\n");
			set_current_tls();
			if (tmp_tls->session_resume == RES_UNSET)
			  tmp_tls->session_resume = RES_NO;
		      }
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 894 "config_grammar.y"
    {
	 	        parameter_debug("tls root_cert: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->root_cert)
			  tmp_tls->root_cert = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 902 "config_grammar.y"
    {
	 	        parameter_debug("tls root_dir: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->root_dir)
			  tmp_tls->root_dir = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 910 "config_grammar.y"
    {
	 	        parameter_debug("tls crl_dir: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->crl_dir)
			  tmp_tls->crl_dir = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 918 "config_grammar.y"
    {
 		        parameter_debug("tls chunk: %d\n", (yyvsp[(3) - (3)].num));
			set_current_tls();
			if (tmp_tls->chunk_size == 0)
			  tmp_tls->chunk_size = (yyvsp[(3) - (3)].num);
  		      }
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 924 "config_grammar.y"
    {
	 	        parameter_debug("tls rand: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_tls();
			if (!tmp_tls->random_file)
			  tmp_tls->random_file = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 935 "config_grammar.y"
    {
                        set_current_md5(); /* define an empty md5 struct*/
                      }
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 944 "config_grammar.y"
    {
                       parameter_debug("md5 username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_md5();
		       if (!tmp_md5->username)
			 tmp_md5->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 952 "config_grammar.y"
    {
		       parameter_debug("md5 password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_md5();
		       if (!tmp_md5->password)
			 tmp_md5->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 963 "config_grammar.y"
    {
                        set_current_ttls(); /* define an empty ttls struct*/
                      }
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 972 "config_grammar.y"
    {
                        parameter_debug("ttls user cert: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->user_cert)
			  tmp_ttls->user_cert = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
                      }
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 980 "config_grammar.y"
    {
	 	        parameter_debug("ttls user key: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->user_key)
			  tmp_ttls->user_key = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 988 "config_grammar.y"
    {
	 	        parameter_debug("ttls user pass: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->user_key_pass)
			  tmp_ttls->user_key_pass = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 996 "config_grammar.y"
    {
	 	        parameter_debug("ttls root_cert: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->root_cert)
			  tmp_ttls->root_cert = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1004 "config_grammar.y"
    {
	 	        parameter_debug("ttls root_dir: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->root_dir)
			  tmp_ttls->root_dir = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1012 "config_grammar.y"
    {
	 	        parameter_debug("ttls crl_dir: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->crl_dir)
			  tmp_ttls->crl_dir = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1020 "config_grammar.y"
    {
 		        parameter_debug("ttls chunk: %d\n", (yyvsp[(3) - (3)].num));
			set_current_ttls();
			if (tmp_ttls->chunk_size == 0)
			  tmp_ttls->chunk_size = (yyvsp[(3) - (3)].num);
  		      }
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1026 "config_grammar.y"
    {
	 	        parameter_debug("ttls rand: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_ttls();
			if (!tmp_ttls->random_file)
			  tmp_ttls->random_file = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1034 "config_grammar.y"
    {
		        parameter_debug("Session Resumption = YES\n");
		        set_current_ttls();
		        if (tmp_ttls->session_resume == RES_UNSET)
			  tmp_ttls->session_resume = RES_YES;
		      }
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1040 "config_grammar.y"
    {
			parameter_debug("Session Resumption = NO\n");
			set_current_ttls();
			if (tmp_ttls->session_resume == RES_UNSET)
			  tmp_ttls->session_resume = RES_NO;
		      }
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1046 "config_grammar.y"
    {
		        parameter_debug("ttls CN check : \"%s\"\n", (yyvsp[(3) - (3)].str));
                        set_current_ttls();
                        if (!tmp_ttls->cncheck)
                          tmp_ttls->cncheck = (yyvsp[(3) - (3)].str);
                        else
                          free((yyvsp[(3) - (3)].str));
		      }
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1054 "config_grammar.y"
    {
  		        parameter_debug("match CN exactly : \"yes\"\n");
		        set_current_ttls();
		        tmp_ttls->cnexact = 1;
		    }
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1059 "config_grammar.y"
    {
  		        parameter_debug("match CN exactly : \"no\"\n");
		        set_current_ttls();
		        tmp_ttls->cnexact = 0;
		    }
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1064 "config_grammar.y"
    {
	 	        parameter_debug("ttls phase2_type 'pap'\n");
			if (tmp_ttls && 
			    tmp_ttls->phase2_type != TTLS_PHASE2_UNDEFINED) {
			  cleanup_parse();
			  return XECONFIGPARSEFAIL;  
			}
			set_current_ttls();
			tmp_ttls->phase2_type = TTLS_PHASE2_PAP;
        	      }
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1074 "config_grammar.y"
    {
	 	        parameter_debug("ttls phase2_type 'chap'\n");
			if (tmp_ttls && 
			    tmp_ttls->phase2_type != TTLS_PHASE2_UNDEFINED) {
			  cleanup_parse();
			  return XECONFIGPARSEFAIL;  
			}
			set_current_ttls();
			tmp_ttls->phase2_type = TTLS_PHASE2_CHAP;
        	      }
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1084 "config_grammar.y"
    {
	 	        parameter_debug("ttls phase2_type 'mschap'\n");
			if (tmp_ttls && 
			    tmp_ttls->phase2_type != TTLS_PHASE2_UNDEFINED) {
			  cleanup_parse();
			  return XECONFIGPARSEFAIL;  
			}
			set_current_ttls();
			tmp_ttls->phase2_type = TTLS_PHASE2_MSCHAP;
        	      }
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1094 "config_grammar.y"
    {
	 	        parameter_debug("ttls phase2_type 'mschapv2'\n");
			if (tmp_ttls && 
			    tmp_ttls->phase2_type != TTLS_PHASE2_UNDEFINED) {
			  cleanup_parse();
			  return XECONFIGPARSEFAIL;  
			}
			set_current_ttls();
			tmp_ttls->phase2_type = TTLS_PHASE2_MSCHAPV2;
        	      }
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1113 "config_grammar.y"
    {
                       set_current_ttls(); 
		       if (!config_ttls_phase2_contains_phase2(tmp_ttls->phase2,
							       TTLS_PHASE2_PAP))
			 add_config_ttls_phase2(&(tmp_ttls->phase2), 
						TTLS_PHASE2_PAP,
						tmp_p2pap);
		       else
			 delete_config_pap(&tmp_p2pap);
		       tmp_p2pap = NULL;
                      }
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1130 "config_grammar.y"
    {
                       parameter_debug("pap username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2pap();
		       if (!tmp_p2pap->username)
			 tmp_p2pap->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1138 "config_grammar.y"
    {
		       parameter_debug("pap password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2pap();
		       if (!tmp_p2pap->password)
			 tmp_p2pap->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1148 "config_grammar.y"
    {
                       set_current_ttls(); 
		       if (!config_ttls_phase2_contains_phase2(tmp_ttls->phase2,
							       TTLS_PHASE2_CHAP))
			 add_config_ttls_phase2(&(tmp_ttls->phase2), 
						TTLS_PHASE2_CHAP,
						tmp_p2chap);
		       else
			 delete_config_chap(&tmp_p2chap);
		       tmp_p2chap = NULL;
                      }
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1165 "config_grammar.y"
    {
                       parameter_debug("chap username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2chap();
		       if (!tmp_p2chap->username)
			 tmp_p2chap->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1173 "config_grammar.y"
    {
		       parameter_debug("chap password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2chap();
		       if (!tmp_p2chap->password)
			 tmp_p2chap->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1183 "config_grammar.y"
    {
                       set_current_ttls(); 
		       if (!config_ttls_phase2_contains_phase2(tmp_ttls->phase2,
							       TTLS_PHASE2_MSCHAP))
			 add_config_ttls_phase2(&(tmp_ttls->phase2), 
						TTLS_PHASE2_MSCHAP,
						tmp_p2mschap);
		       else
			 delete_config_mschap(&tmp_p2mschap);
		       tmp_p2mschap = NULL;
                      }
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1200 "config_grammar.y"
    {
                       parameter_debug("mschap username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2mschap();
		       if (!tmp_p2mschap->username)
			 tmp_p2mschap->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1208 "config_grammar.y"
    {
		       parameter_debug("mschap password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2mschap();
		       if (!tmp_p2mschap->password)
			 tmp_p2mschap->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1219 "config_grammar.y"
    {
                       set_current_ttls(); 
		       if (!config_ttls_phase2_contains_phase2(tmp_ttls->phase2,
							       TTLS_PHASE2_MSCHAPV2))
			 add_config_ttls_phase2(&(tmp_ttls->phase2), 
						TTLS_PHASE2_MSCHAPV2,
						tmp_p2mschapv2);
		       else
			 delete_config_mschapv2(&tmp_p2mschapv2);
		       tmp_p2mschapv2 = NULL;
                      }
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1236 "config_grammar.y"
    {
                       parameter_debug("mschapv2 username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2mschapv2();
		       if (!tmp_p2mschapv2->username)
			 tmp_p2mschapv2->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1244 "config_grammar.y"
    {
		       parameter_debug("mschapv2 password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_p2mschapv2();
		       if (!tmp_p2mschapv2->password)
			 tmp_p2mschapv2->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1255 "config_grammar.y"
    {
                        set_current_leap(); /* define an empty leap struct*/
                      }
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1264 "config_grammar.y"
    {
                       parameter_debug("leap username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_leap();
		       if (!tmp_leap->username)
			 tmp_leap->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1272 "config_grammar.y"
    {
		       parameter_debug("leap password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_leap();
		       if (!tmp_leap->password)
			 tmp_leap->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1283 "config_grammar.y"
    {
                        set_current_mschapv2(); /* define an empty mschapv2 struct*/
                      }
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1292 "config_grammar.y"
    {
                       parameter_debug("mschapv2 username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_mschapv2();
		       if (!tmp_mschapv2->username)
			 tmp_mschapv2->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1300 "config_grammar.y"
    {
		       parameter_debug("mschapv2 password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_mschapv2();
		       if (!tmp_mschapv2->password)
			 tmp_mschapv2->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1311 "config_grammar.y"
    {
                        set_current_peap(); /* define an empty peap struct*/
                      }
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1320 "config_grammar.y"
    {
                            parameter_debug("ID: \"%s\"\n", (yyvsp[(3) - (3)].str));
			    set_current_peap();
			    if (!tmp_peap->identity)
			      tmp_peap->identity = (yyvsp[(3) - (3)].str);
			    else
			      free((yyvsp[(3) - (3)].str));
                          }
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1328 "config_grammar.y"
    {
                        parameter_debug("peap user cert: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->user_cert)
			  tmp_peap->user_cert = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
                      }
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1336 "config_grammar.y"
    {
	 	        parameter_debug("peap user key: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->user_key)
			  tmp_peap->user_key = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1344 "config_grammar.y"
    {
	 	        parameter_debug("peap user pass: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->user_key_pass)
			  tmp_peap->user_key_pass = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1352 "config_grammar.y"
    {
	 	        parameter_debug("peap root_cert: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->root_cert)
			  tmp_peap->root_cert = (yyvsp[(3) - (3)].str);
			else
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1360 "config_grammar.y"
    {
	 	        parameter_debug("peap root_dir: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->root_dir)
			  tmp_peap->root_dir = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1368 "config_grammar.y"
    {
	 	        parameter_debug("peap crl_dir: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->crl_dir)
			  tmp_peap->crl_dir = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1376 "config_grammar.y"
    {
		        parameter_debug("Session Resumption = YES\n");
		        set_current_peap();
		        if (tmp_peap->session_resume == RES_UNSET)
			  tmp_peap->session_resume = RES_YES;
		      }
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1382 "config_grammar.y"
    {
			parameter_debug("Session Resumption = NO\n");
			set_current_peap();
			if (tmp_peap->session_resume == RES_UNSET)
			  tmp_peap->session_resume = RES_NO;
		      }
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1388 "config_grammar.y"
    {
 		        parameter_debug("peap chunk: %d\n", (yyvsp[(3) - (3)].num));
			set_current_peap();
			if (tmp_peap->chunk_size == 0)
			  tmp_peap->chunk_size = (yyvsp[(3) - (3)].num);
  		      }
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1394 "config_grammar.y"
    {
		        parameter_debug("peap CN check : \"%s\"\n", (yyvsp[(3) - (3)].str));
                        set_current_peap();
                        if (!tmp_peap->cncheck)
                          tmp_peap->cncheck = (yyvsp[(3) - (3)].str);
                        else
                          free((yyvsp[(3) - (3)].str));
		      }
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1402 "config_grammar.y"
    {
  		        parameter_debug("match CN exactly : \"yes\"\n");
		        set_current_peap();
		        tmp_peap->cnexact = 1;
		    }
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1407 "config_grammar.y"
    {
  		        parameter_debug("match CN exactly : \"no\"\n");
		        set_current_peap();
		        tmp_peap->cnexact = 0;
       		    }
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1412 "config_grammar.y"
    {
	 	        parameter_debug("peap rand: \"%s\"\n", (yyvsp[(3) - (3)].str));
			set_current_peap();
			if (!tmp_peap->random_file)
			  tmp_peap->random_file = (yyvsp[(3) - (3)].str);
			else 
			  free((yyvsp[(3) - (3)].str));
        	      }
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1420 "config_grammar.y"
    {}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1421 "config_grammar.y"
    {}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1424 "config_grammar.y"
    {
                           parameter_debug("PEAP Allow Types: ALL\n");
			   set_current_peap();
			   SET_FLAG(tmp_peap->flags, CONFIG_PEAP_ALLOW_ALL);
                       }
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1436 "config_grammar.y"
    {
                          parameter_debug("PEAP Allow Type: MSCHAPV2\n");
	  	  	  set_current_peap();
			  SET_FLAG(tmp_peap->flags, CONFIG_PEAP_ALLOW_MSCV2);
                        }
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1441 "config_grammar.y"
    {
                          parameter_debug("PEAP Allow Type: MD5\n");
	  	  	  set_current_peap();
			  SET_FLAG(tmp_peap->flags, CONFIG_PEAP_ALLOW_MD5);
                        }
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1446 "config_grammar.y"
    {
                          parameter_debug("PEAP Allow Type: SIM\n");
	  	  	  set_current_peap();
			  SET_FLAG(tmp_peap->flags, CONFIG_PEAP_ALLOW_SIM);
                        }
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1451 "config_grammar.y"
    {
                          parameter_debug("PEAP Allow Type: OTP\n");
	  	  	  set_current_peap();
			  SET_FLAG(tmp_peap->flags, CONFIG_PEAP_ALLOW_OTP);
                        }
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1456 "config_grammar.y"
    {
                          parameter_debug("PEAP Allow Type: GTC\n");
	  	  	  set_current_peap();
			  SET_FLAG(tmp_peap->flags, CONFIG_PEAP_ALLOW_GTC);
                        }
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1464 "config_grammar.y"
    {
                             set_current_peap(); 
	   	             if (!config_eap_method_contains_method(tmp_peap->phase2,
								    EAP_TYPE_MSCHAPV2))
			       add_config_eap_method(&(tmp_peap->phase2),
						     EAP_TYPE_MSCHAPV2,
						     tmp_mschapv2);
			     else
			       delete_config_eap_mschapv2(&tmp_mschapv2);
			     tmp_mschapv2 = NULL;
                            }
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1475 "config_grammar.y"
    {
                             set_current_peap(); 
	   	             if (!config_eap_method_contains_method(tmp_peap->phase2,
								    EAP_TYPE_MD5))
			       add_config_eap_method(&(tmp_peap->phase2),
						     EAP_TYPE_MD5,
						     tmp_md5);
			     else
			       delete_config_eap_md5(&tmp_md5);
			     tmp_md5 = NULL;
                            }
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1486 "config_grammar.y"
    {
                             set_current_peap(); 
	   	             if (!config_eap_method_contains_method(tmp_peap->phase2,
								    EAP_TYPE_SIM))
			       add_config_eap_method(&(tmp_peap->phase2),
						     EAP_TYPE_SIM,
						     tmp_sim);
			     else
			       delete_config_eap_sim(&tmp_sim);
			     tmp_sim = NULL;
                            }
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1500 "config_grammar.y"
    {
                        set_current_sim(); /* define an empty sim struct*/
                      }
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1509 "config_grammar.y"
    {
                       parameter_debug("sim username: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_sim();
		       if (!tmp_sim->username)
			 tmp_sim->username = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
                     }
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1517 "config_grammar.y"
    {
		       parameter_debug("sim password: \"%s\"\n", (yyvsp[(3) - (3)].str));
		       set_current_sim();
		       if (!tmp_sim->password)
			 tmp_sim->password = (yyvsp[(3) - (3)].str);
		       else
			 free((yyvsp[(3) - (3)].str));
		     }
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1525 "config_grammar.y"
    {
  		       parameter_debug("sim auto_realm: \"yes\"\n");
		       set_current_sim();
		       tmp_sim->auto_realm = 1;
		   }
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1530 "config_grammar.y"
    {
  		       parameter_debug("sim auto_realm: \"no\"\n");
		       set_current_sim();
		       tmp_sim->auto_realm = 0;
		   }
    break;



/* Line 1455 of yacc.c  */
#line 3875 "config_grammar.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1536 "config_grammar.y"


