
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
#line 331 "config_grammar.y"

        char    *str;
        int     num;



/* Line 1676 of yacc.c  */
#line 181 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


