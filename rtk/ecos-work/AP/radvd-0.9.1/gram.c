
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
#line 16 "gram.y"

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>

extern struct Interface *IfaceList;
struct Interface *iface = NULL;
struct AdvPrefix *prefix = NULL;
struct AdvRoute *route = NULL;
struct AdvRDNSS *rdnss = NULL;
struct AdvDNSSL *dnssl = NULL;

extern char *conf_file;
extern int num_lines;
extern char *yytext;
extern int sock;

static void cleanup(void);
static void yyerror(char *msg);

#if 0 /* no longer necessary? */
#ifndef HAVE_IN6_ADDR_S6_ADDR
# ifdef __FreeBSD__
#  define s6_addr32 __u6_addr.__u6_addr32
#  define s6_addr16 __u6_addr.__u6_addr16
# endif
#endif
#endif

#define ABORT	do { cleanup(); YYABORT; } while (0);
#define ADD_TO_LL(type, list, value) \
	do { \
		if (iface->list == NULL) \
			iface->list = value; \
		else { \
			type *current = iface->list; \
			while (current->next != NULL) \
				current = current->next; \
			current->next = value; \
		} \
	} while (0)



/* Line 189 of yacc.c  */
#line 119 "y.tab.c"

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
     T_INTERFACE = 258,
     T_PREFIX = 259,
     T_ROUTE = 260,
     T_RDNSS = 261,
     T_DNSSL = 262,
     STRING = 263,
     NUMBER = 264,
     SIGNEDNUMBER = 265,
     DECIMAL = 266,
     SWITCH = 267,
     IPV6ADDR = 268,
     INFINITY = 269,
     T_IgnoreIfMissing = 270,
     T_AdvSendAdvert = 271,
     T_MaxRtrAdvInterval = 272,
     T_MinRtrAdvInterval = 273,
     T_MinDelayBetweenRAs = 274,
     T_AdvManagedFlag = 275,
     T_AdvOtherConfigFlag = 276,
     T_AdvLinkMTU = 277,
     T_AdvReachableTime = 278,
     T_AdvRetransTimer = 279,
     T_AdvCurHopLimit = 280,
     T_AdvDefaultLifetime = 281,
     T_AdvDefaultPreference = 282,
     T_AdvSourceLLAddress = 283,
     T_AdvOnLink = 284,
     T_AdvAutonomous = 285,
     T_AdvValidLifetime = 286,
     T_AdvPreferredLifetime = 287,
     T_AdvRouterAddr = 288,
     T_AdvHomeAgentFlag = 289,
     T_AdvIntervalOpt = 290,
     T_AdvHomeAgentInfo = 291,
     T_Base6to4Interface = 292,
     T_UnicastOnly = 293,
     T_HomeAgentPreference = 294,
     T_HomeAgentLifetime = 295,
     T_AdvRoutePreference = 296,
     T_AdvRouteLifetime = 297,
     T_AdvRDNSSPreference = 298,
     T_AdvRDNSSOpenFlag = 299,
     T_AdvRDNSSLifetime = 300,
     T_FlushRDNSS = 301,
     T_AdvDNSSLLifetime = 302,
     T_FlushDNSSL = 303,
     T_AdvMobRtrSupportFlag = 304,
     T_BAD_TOKEN = 305
   };
#endif
/* Tokens.  */
#define T_INTERFACE 258
#define T_PREFIX 259
#define T_ROUTE 260
#define T_RDNSS 261
#define T_DNSSL 262
#define STRING 263
#define NUMBER 264
#define SIGNEDNUMBER 265
#define DECIMAL 266
#define SWITCH 267
#define IPV6ADDR 268
#define INFINITY 269
#define T_IgnoreIfMissing 270
#define T_AdvSendAdvert 271
#define T_MaxRtrAdvInterval 272
#define T_MinRtrAdvInterval 273
#define T_MinDelayBetweenRAs 274
#define T_AdvManagedFlag 275
#define T_AdvOtherConfigFlag 276
#define T_AdvLinkMTU 277
#define T_AdvReachableTime 278
#define T_AdvRetransTimer 279
#define T_AdvCurHopLimit 280
#define T_AdvDefaultLifetime 281
#define T_AdvDefaultPreference 282
#define T_AdvSourceLLAddress 283
#define T_AdvOnLink 284
#define T_AdvAutonomous 285
#define T_AdvValidLifetime 286
#define T_AdvPreferredLifetime 287
#define T_AdvRouterAddr 288
#define T_AdvHomeAgentFlag 289
#define T_AdvIntervalOpt 290
#define T_AdvHomeAgentInfo 291
#define T_Base6to4Interface 292
#define T_UnicastOnly 293
#define T_HomeAgentPreference 294
#define T_HomeAgentLifetime 295
#define T_AdvRoutePreference 296
#define T_AdvRouteLifetime 297
#define T_AdvRDNSSPreference 298
#define T_AdvRDNSSOpenFlag 299
#define T_AdvRDNSSLifetime 300
#define T_FlushRDNSS 301
#define T_AdvDNSSLLifetime 302
#define T_FlushDNSSL 303
#define T_AdvMobRtrSupportFlag 304
#define T_BAD_TOKEN 305




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 127 "gram.y"

	unsigned int		num;
	int			snum;
	double			dec;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
	struct AdvRoute		*rinfo;
	struct AdvRDNSS		*rdnssinfo;
	struct AdvDNSSL		*dnsslinfo;



/* Line 214 of yacc.c  */
#line 270 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 282 "y.tab.c"

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
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   164

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  93
/* YYNRULES -- Number of states.  */
#define YYNSTATES  192

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   305

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    53,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    51,     2,    52,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     8,    14,    17,    19,    25,    26,
      28,    29,    31,    32,    34,    35,    37,    38,    40,    43,
      45,    49,    53,    57,    61,    65,    69,    73,    77,    81,
      85,    89,    93,    97,   101,   105,   109,   113,   117,   121,
     125,   129,   133,   137,   141,   143,   146,   152,   157,   158,
     160,   163,   165,   169,   173,   177,   181,   185,   189,   191,
     194,   200,   205,   206,   208,   211,   213,   217,   221,   227,
     230,   232,   234,   237,   238,   240,   243,   245,   249,   253,
     257,   261,   267,   270,   272,   274,   277,   278,   280,   283,
     285,   289,   293,   295
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      56,     0,    -1,    56,    57,    -1,    57,    -1,    58,    51,
      60,    52,    53,    -1,     3,    59,    -1,     8,    -1,    61,
      62,    63,    64,    65,    -1,    -1,    66,    -1,    -1,    68,
      -1,    -1,    74,    -1,    -1,    80,    -1,    -1,    87,    -1,
      66,    67,    -1,    67,    -1,    18,     9,    53,    -1,    17,
       9,    53,    -1,    19,     9,    53,    -1,    18,    11,    53,
      -1,    17,    11,    53,    -1,    19,    11,    53,    -1,    15,
      12,    53,    -1,    16,    12,    53,    -1,    20,    12,    53,
      -1,    21,    12,    53,    -1,    22,     9,    53,    -1,    23,
       9,    53,    -1,    24,     9,    53,    -1,    26,     9,    53,
      -1,    27,    10,    53,    -1,    25,     9,    53,    -1,    28,
      12,    53,    -1,    35,    12,    53,    -1,    36,    12,    53,
      -1,    34,    12,    53,    -1,    39,     9,    53,    -1,    40,
       9,    53,    -1,    38,    12,    53,    -1,    49,    12,    53,
      -1,    69,    -1,    68,    69,    -1,    70,    51,    71,    52,
      53,    -1,     4,    13,    54,     9,    -1,    -1,    72,    -1,
      72,    73,    -1,    73,    -1,    29,    12,    53,    -1,    30,
      12,    53,    -1,    33,    12,    53,    -1,    31,    94,    53,
      -1,    32,    94,    53,    -1,    37,    59,    53,    -1,    75,
      -1,    74,    75,    -1,    76,    51,    77,    52,    53,    -1,
       5,    13,    54,     9,    -1,    -1,    78,    -1,    78,    79,
      -1,    79,    -1,    41,    10,    53,    -1,    42,    94,    53,
      -1,    83,    51,    84,    52,    53,    -1,    81,    82,    -1,
      82,    -1,    13,    -1,     6,    81,    -1,    -1,    85,    -1,
      85,    86,    -1,    86,    -1,    43,     9,    53,    -1,    44,
      12,    53,    -1,    45,    94,    53,    -1,    46,    12,    53,
      -1,    90,    51,    91,    52,    53,    -1,    88,    89,    -1,
      89,    -1,     8,    -1,     7,    88,    -1,    -1,    92,    -1,
      92,    93,    -1,    93,    -1,    47,    94,    53,    -1,    48,
      12,    53,    -1,     9,    -1,    14,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   142,   142,   143,   146,   190,   205,   212,   219,   220,
     224,   227,   231,   234,   237,   238,   241,   242,   244,   245,
     248,   252,   256,   260,   264,   268,   272,   276,   280,   284,
     288,   292,   296,   300,   304,   308,   312,   316,   320,   324,
     328,   332,   336,   340,   346,   350,   357,   388,   411,   412,
     415,   416,   419,   423,   427,   431,   435,   439,   447,   451,
     458,   466,   490,   491,   494,   495,   499,   503,   509,   516,
     517,   520,   555,   564,   565,   568,   569,   573,   577,   581,
     592,   598,   605,   606,   609,   651,   660,   661,   664,   665,
     669,   680,   686,   690
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_INTERFACE", "T_PREFIX", "T_ROUTE",
  "T_RDNSS", "T_DNSSL", "STRING", "NUMBER", "SIGNEDNUMBER", "DECIMAL",
  "SWITCH", "IPV6ADDR", "INFINITY", "T_IgnoreIfMissing", "T_AdvSendAdvert",
  "T_MaxRtrAdvInterval", "T_MinRtrAdvInterval", "T_MinDelayBetweenRAs",
  "T_AdvManagedFlag", "T_AdvOtherConfigFlag", "T_AdvLinkMTU",
  "T_AdvReachableTime", "T_AdvRetransTimer", "T_AdvCurHopLimit",
  "T_AdvDefaultLifetime", "T_AdvDefaultPreference", "T_AdvSourceLLAddress",
  "T_AdvOnLink", "T_AdvAutonomous", "T_AdvValidLifetime",
  "T_AdvPreferredLifetime", "T_AdvRouterAddr", "T_AdvHomeAgentFlag",
  "T_AdvIntervalOpt", "T_AdvHomeAgentInfo", "T_Base6to4Interface",
  "T_UnicastOnly", "T_HomeAgentPreference", "T_HomeAgentLifetime",
  "T_AdvRoutePreference", "T_AdvRouteLifetime", "T_AdvRDNSSPreference",
  "T_AdvRDNSSOpenFlag", "T_AdvRDNSSLifetime", "T_FlushRDNSS",
  "T_AdvDNSSLLifetime", "T_FlushDNSSL", "T_AdvMobRtrSupportFlag",
  "T_BAD_TOKEN", "'{'", "'}'", "';'", "'/'", "$accept", "grammar",
  "ifacedef", "ifacehead", "name", "ifaceparams", "optional_ifacevlist",
  "optional_prefixlist", "optional_routelist", "optional_rdnsslist",
  "optional_dnssllist", "ifacevlist", "ifaceval", "prefixlist",
  "prefixdef", "prefixhead", "optional_prefixplist", "prefixplist",
  "prefixparms", "routelist", "routedef", "routehead",
  "optional_routeplist", "routeplist", "routeparms", "rdnssdef",
  "rdnssaddrs", "rdnssaddr", "rdnsshead", "optional_rdnssplist",
  "rdnssplist", "rdnssparms", "dnssldef", "dnsslsuffixes", "dnsslsuffix",
  "dnsslhead", "optional_dnsslplist", "dnsslplist", "dnsslparms",
  "number_or_infinity", 0
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
     305,   123,   125,    59,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    55,    56,    56,    57,    58,    59,    60,    61,    61,
      62,    62,    63,    63,    64,    64,    65,    65,    66,    66,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    68,    68,    69,    70,    71,    71,
      72,    72,    73,    73,    73,    73,    73,    73,    74,    74,
      75,    76,    77,    77,    78,    78,    79,    79,    80,    81,
      81,    82,    83,    84,    84,    85,    85,    86,    86,    86,
      86,    87,    88,    88,    89,    90,    91,    91,    92,    92,
      93,    93,    94,    94
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     5,     2,     1,     5,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     2,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     2,     5,     4,     0,     1,
       2,     1,     3,     3,     3,     3,     3,     3,     1,     2,
       5,     4,     0,     1,     2,     1,     3,     3,     5,     2,
       1,     1,     2,     0,     1,     2,     1,     3,     3,     3,
       3,     5,     2,     1,     1,     2,     0,     1,     2,     1,
       3,     3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     3,     0,     6,     5,     1,     2,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,     9,    19,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    11,    44,     0,    18,    26,    27,    21,    24,
      20,    23,    22,    25,    28,    29,    30,    31,    32,    35,
      33,    34,    36,    39,    37,    38,    42,    40,    41,    43,
       4,     0,     0,    14,    13,    58,     0,    45,    48,     0,
       0,     0,    16,    15,     0,    59,    62,     0,     0,     0,
       0,     0,     0,     0,    49,    51,    47,     0,    71,    72,
      70,     0,     7,    17,     0,    73,     0,     0,     0,    63,
      65,     0,     0,    92,    93,     0,     0,     0,     0,     0,
      50,    61,    69,    84,    85,    83,    86,     0,     0,     0,
       0,     0,    74,    76,     0,     0,     0,    64,    52,    53,
      55,    56,    54,    57,    46,    82,     0,     0,     0,    87,
      89,     0,     0,     0,     0,     0,    75,    66,    67,    60,
       0,     0,     0,    88,    77,    78,    79,    80,    68,    90,
      91,    81
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     6,    31,    32,    61,    93,   102,
     122,    33,    34,    62,    63,    64,   113,   114,   115,    94,
      95,    96,   128,   129,   130,   103,   119,   120,   104,   151,
     152,   153,   123,   144,   145,   124,   168,   169,   170,   135
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -111
static const yytype_int8 yypact[] =
{
      12,    19,    16,  -111,    -2,  -111,  -111,  -111,  -111,   -14,
      40,    41,    35,    36,    39,    42,    43,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    62,    63,
      61,    10,    47,   -14,  -111,    11,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      37,    38,    44,    45,    46,    59,    60,    64,    65,    66,
      75,    84,    47,  -111,    14,  -111,  -111,  -111,  -111,  -111,
    -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,
    -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,
    -111,    67,    79,    87,    84,  -111,    69,  -111,    -1,    85,
      68,    82,    89,  -111,    72,  -111,    -8,    88,    90,     9,
       9,    91,    19,    73,    -1,  -111,  -111,    92,  -111,    82,
    -111,    96,  -111,  -111,    76,    -3,    95,     9,    74,    -8,
    -111,    71,    77,  -111,  -111,    78,    80,    81,    83,    86,
    -111,  -111,  -111,  -111,    96,  -111,   -10,    97,    98,     9,
      99,    93,    -3,  -111,    94,   100,   101,  -111,  -111,  -111,
    -111,  -111,  -111,  -111,  -111,  -111,     9,   102,   103,   -10,
    -111,   104,   105,   106,   107,   108,  -111,  -111,  -111,  -111,
     109,   110,   111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,
    -111,  -111
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -111,  -111,   113,  -111,    -5,  -111,  -111,  -111,  -111,  -111,
    -111,  -111,   115,  -111,    70,  -111,  -111,  -111,    -6,  -111,
      15,  -111,  -111,  -111,   -13,  -111,  -111,    18,  -111,  -111,
    -111,   -24,  -111,  -111,   -15,  -111,  -111,  -111,   -34,  -110
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
     136,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     1,     7,   155,   133,     1,
      24,    25,    26,   134,    27,    28,    29,     5,   107,   108,
     109,   110,   111,   126,   127,    30,   112,   166,   167,   173,
     147,   148,   149,   150,    37,    39,    38,    40,    41,     9,
      42,    60,    35,    36,    43,    44,   180,    45,    46,    47,
      48,    49,    59,    50,    66,    98,    51,    52,    53,    54,
      55,    56,    57,    58,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    91,    92,
      81,    82,   100,   101,   116,   118,   121,    83,    84,    85,
     131,   141,   132,   137,   143,   154,   171,   138,   140,   105,
     172,   174,    86,    87,   181,     8,   157,    88,    89,    90,
     106,    99,   117,   125,   158,   139,   156,   146,   176,   165,
     159,   160,    97,   161,   162,   183,   163,   142,     0,   164,
       0,     0,     0,     0,     0,   175,     0,   177,    65,     0,
       0,     0,     0,   178,   179,   182,     0,   184,   185,   186,
     187,   188,   189,   190,   191
};

static const yytype_int16 yycheck[] =
{
     110,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     3,     0,   127,     9,     3,
      34,    35,    36,    14,    38,    39,    40,     8,    29,    30,
      31,    32,    33,    41,    42,    49,    37,    47,    48,   149,
      43,    44,    45,    46,     9,     9,    11,    11,     9,    51,
      11,     4,    12,    12,    12,    12,   166,     9,     9,     9,
       9,     9,    52,    10,    53,    51,    12,    12,    12,    12,
      12,     9,     9,    12,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    13,     5,
      53,    53,    13,     6,     9,    13,     7,    53,    53,    53,
      12,     9,    12,    12,     8,    10,     9,   112,   114,    94,
      12,    12,    53,    53,    12,     2,   129,    53,    53,    53,
      51,    54,    54,    51,    53,    52,    52,    51,   152,   144,
      53,    53,    62,    53,    53,   169,    53,   119,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    53,    33,    -1,
      -1,    -1,    -1,    53,    53,    52,    -1,    53,    53,    53,
      53,    53,    53,    53,    53
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    56,    57,    58,     8,    59,     0,    57,    51,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    34,    35,    36,    38,    39,    40,
      49,    60,    61,    66,    67,    12,    12,     9,    11,     9,
      11,     9,    11,    12,    12,     9,     9,     9,     9,     9,
      10,    12,    12,    12,    12,    12,     9,     9,    12,    52,
       4,    62,    68,    69,    70,    67,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    13,     5,    63,    74,    75,    76,    69,    51,    54,
      13,     6,    64,    80,    83,    75,    51,    29,    30,    31,
      32,    33,    37,    71,    72,    73,     9,    54,    13,    81,
      82,     7,    65,    87,    90,    51,    41,    42,    77,    78,
      79,    12,    12,     9,    14,    94,    94,    12,    59,    52,
      73,     9,    82,     8,    88,    89,    51,    43,    44,    45,
      46,    84,    85,    86,    10,    94,    52,    79,    53,    53,
      53,    53,    53,    53,    53,    89,    47,    48,    91,    92,
      93,     9,    12,    94,    12,    52,    86,    53,    53,    53,
      94,    12,    52,    93,    53,    53,    53,    53,    53,    53,
      53,    53
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
        case 4:

/* Line 1455 of yacc.c  */
#line 147 "gram.y"
    {
			struct Interface *iface2;

			iface2 = IfaceList;
			while (iface2)
			{
				if (!strcmp(iface2->Name, iface->Name))
				{
					flog(LOG_ERR, "duplicate interface "
						"definition for %s", iface->Name);
					ABORT;
				}
				iface2 = iface2->next;
			}			

			if (check_device(sock, iface) < 0) {
				if (iface->IgnoreIfMissing) {
					dlog(LOG_DEBUG, 4, "interface %s did not exist, ignoring the interface", iface->Name);
					goto skip_interface;
				}
				else {
					flog(LOG_ERR, "interface %s does not exist", iface->Name);
					ABORT;
				}
			}
			if (setup_deviceinfo(sock, iface) < 0)
				ABORT;
			if (check_iface(iface) < 0)
				ABORT;
			if (setup_linklocal_addr(sock, iface) < 0)
				ABORT;
			if (setup_allrouters_membership(sock, iface) < 0)
				ABORT;

			iface->next = IfaceList;
			IfaceList = iface;

			dlog(LOG_DEBUG, 4, "interface definition for %s is ok", iface->Name);

skip_interface:
			iface = NULL;
		}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 191 "gram.y"
    {
			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			strncpy(iface->Name, (yyvsp[(2) - (2)].str), IFNAMSIZ-1);
			iface->Name[IFNAMSIZ-1] = '\0';
		}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 206 "gram.y"
    {
			/* check vality */
			(yyval.str) = (yyvsp[(1) - (1)].str);
		}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 213 "gram.y"
    {
			iface->AdvPrefixList = (yyvsp[(2) - (5)].pinfo);
			iface->AdvRouteList = (yyvsp[(3) - (5)].rinfo);
		}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 224 "gram.y"
    {
			(yyval.pinfo) = NULL;
		}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 231 "gram.y"
    {
			(yyval.rinfo) = NULL;
		}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 238 "gram.y"
    { ADD_TO_LL(struct AdvRDNSS, AdvRDNSSList, (yyvsp[(1) - (1)].rdnssinfo)); }
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 242 "gram.y"
    { ADD_TO_LL(struct AdvDNSSL, AdvDNSSLList, (yyvsp[(1) - (1)].dnsslinfo)); }
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 249 "gram.y"
    {
			iface->MinRtrAdvInterval = (yyvsp[(2) - (3)].num);
		}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 253 "gram.y"
    {
			iface->MaxRtrAdvInterval = (yyvsp[(2) - (3)].num);
		}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 257 "gram.y"
    {
			iface->MinDelayBetweenRAs = (yyvsp[(2) - (3)].num);
		}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 261 "gram.y"
    {
			iface->MinRtrAdvInterval = (yyvsp[(2) - (3)].dec);
		}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 265 "gram.y"
    {
			iface->MaxRtrAdvInterval = (yyvsp[(2) - (3)].dec);
		}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 269 "gram.y"
    {
			iface->MinDelayBetweenRAs = (yyvsp[(2) - (3)].dec);
		}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 273 "gram.y"
    {
			iface->IgnoreIfMissing = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 277 "gram.y"
    {
			iface->AdvSendAdvert = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 281 "gram.y"
    {
			iface->AdvManagedFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 285 "gram.y"
    {
			iface->AdvOtherConfigFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 289 "gram.y"
    {
			iface->AdvLinkMTU = (yyvsp[(2) - (3)].num);
		}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 293 "gram.y"
    {
			iface->AdvReachableTime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 297 "gram.y"
    {
			iface->AdvRetransTimer = (yyvsp[(2) - (3)].num);
		}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 301 "gram.y"
    {
			iface->AdvDefaultLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 305 "gram.y"
    {
			iface->AdvDefaultPreference = (yyvsp[(2) - (3)].snum);
		}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 309 "gram.y"
    {
			iface->AdvCurHopLimit = (yyvsp[(2) - (3)].num);
		}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 313 "gram.y"
    {
			iface->AdvSourceLLAddress = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 317 "gram.y"
    {
			iface->AdvIntervalOpt = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 321 "gram.y"
    {
			iface->AdvHomeAgentInfo = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 325 "gram.y"
    {
			iface->AdvHomeAgentFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 329 "gram.y"
    {
			iface->HomeAgentPreference = (yyvsp[(2) - (3)].num);
		}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 333 "gram.y"
    {
			iface->HomeAgentLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 337 "gram.y"
    {
			iface->UnicastOnly = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 341 "gram.y"
    {
			iface->AdvMobRtrSupportFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 347 "gram.y"
    {
			(yyval.pinfo) = (yyvsp[(1) - (1)].pinfo);
		}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 351 "gram.y"
    {
			(yyvsp[(2) - (2)].pinfo)->next = (yyvsp[(1) - (2)].pinfo);
			(yyval.pinfo) = (yyvsp[(2) - (2)].pinfo);
		}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 358 "gram.y"
    {
			unsigned int dst;

			if (prefix->AdvPreferredLifetime >
			    prefix->AdvValidLifetime)
			{
				flog(LOG_ERR, "AdvValidLifeTime must be "
					"greater than AdvPreferredLifetime in %s, line %d", 
					conf_file, num_lines);
				ABORT;
			}

			if( prefix->if6to4[0] )
			{
				if (get_v4addr(prefix->if6to4, &dst) < 0)
				{
					flog(LOG_ERR, "interface %s has no IPv4 addresses, disabling 6to4 prefix", prefix->if6to4 );
					prefix->enabled = 0;
				} else
				{
					*((uint16_t *)(prefix->Prefix.s6_addr)) = htons(0x2002);
					memcpy( prefix->Prefix.s6_addr + 2, &dst, sizeof( dst ) );
				}
			}

			(yyval.pinfo) = prefix;
			prefix = NULL;
		}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 389 "gram.y"
    {
			prefix = malloc(sizeof(struct AdvPrefix));
			
			if (prefix == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(prefix);

			if ((yyvsp[(4) - (4)].num) > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			prefix->PrefixLen = (yyvsp[(4) - (4)].num);

			memcpy(&prefix->Prefix, (yyvsp[(2) - (4)].addr), sizeof(struct in6_addr));
		}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 420 "gram.y"
    {
			prefix->AdvOnLinkFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 424 "gram.y"
    {
			prefix->AdvAutonomousFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 428 "gram.y"
    {
			prefix->AdvRouterAddr = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 432 "gram.y"
    {
			prefix->AdvValidLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 436 "gram.y"
    {
			prefix->AdvPreferredLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 440 "gram.y"
    {
			dlog(LOG_DEBUG, 4, "using interface %s for 6to4", (yyvsp[(2) - (3)].str));
			strncpy(prefix->if6to4, (yyvsp[(2) - (3)].str), IFNAMSIZ-1);
			prefix->if6to4[IFNAMSIZ-1] = '\0';
		}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 448 "gram.y"
    {
			(yyval.rinfo) = (yyvsp[(1) - (1)].rinfo);
		}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 452 "gram.y"
    {
			(yyvsp[(2) - (2)].rinfo)->next = (yyvsp[(1) - (2)].rinfo);
			(yyval.rinfo) = (yyvsp[(2) - (2)].rinfo);
		}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 459 "gram.y"
    {
			(yyval.rinfo) = route;
			route = NULL;
		}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 467 "gram.y"
    {
			route = malloc(sizeof(struct AdvRoute));
			
			if (route == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			route_init_defaults(route, iface);

			if ((yyvsp[(4) - (4)].num) > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid route prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			route->PrefixLen = (yyvsp[(4) - (4)].num);

			memcpy(&route->Prefix, (yyvsp[(2) - (4)].addr), sizeof(struct in6_addr));
		}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 500 "gram.y"
    {
			route->AdvRoutePreference = (yyvsp[(2) - (3)].snum);
		}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 504 "gram.y"
    {
			route->AdvRouteLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 510 "gram.y"
    {
			(yyval.rdnssinfo) = rdnss;
			rdnss = NULL;
		}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 521 "gram.y"
    {
			if (!rdnss) {
				/* first IP found */
				rdnss = malloc(sizeof(struct AdvRDNSS));

				if (rdnss == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				rdnss_init_defaults(rdnss, iface);
			}

			switch (rdnss->AdvRDNSSNumber) {
				case 0:
					memcpy(&rdnss->AdvRDNSSAddr1, (yyvsp[(1) - (1)].addr), sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				case 1:
					memcpy(&rdnss->AdvRDNSSAddr2, (yyvsp[(1) - (1)].addr), sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				case 2:
					memcpy(&rdnss->AdvRDNSSAddr3, (yyvsp[(1) - (1)].addr), sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				default:
					flog(LOG_CRIT, "Too many addresses in RDNSS section");
					ABORT;
			}

		}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 556 "gram.y"
    {
			if (!rdnss) {
				flog(LOG_CRIT, "No address specified in RDNSS section");
				ABORT;
			}
		}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 574 "gram.y"
    {
			flog(LOG_WARNING, "Ignoring deprecated RDNSS preference.");
		}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 578 "gram.y"
    {
			flog(LOG_WARNING, "Ignoring deprecated RDNSS open flag.");
		}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 582 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) < iface->MaxRtrAdvInterval && (yyvsp[(2) - (3)].num) != 0) {
				flog(LOG_ERR, "AdvRDNSSLifetime must be at least MaxRtrAdvInterval");
				ABORT;
			}
			if ((yyvsp[(2) - (3)].num) > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "Warning: AdvRDNSSLifetime <= 2*MaxRtrAdvInterval would allow stale DNS servers to be deleted faster");

			rdnss->AdvRDNSSLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 593 "gram.y"
    {
			rdnss->FlushRDNSSFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 599 "gram.y"
    {
			(yyval.dnsslinfo) = dnssl;
			dnssl = NULL;
		}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 610 "gram.y"
    {
			char *ch;
			for (ch = (yyvsp[(1) - (1)].str);*ch != '\0';ch++) {
				if (*ch >= 'A' && *ch <= 'Z')
					continue;
				if (*ch >= 'a' && *ch <= 'z')
					continue;
				if (*ch >= '0' && *ch <= '9')
					continue;
				if (*ch == '-' || *ch == '.')
					continue;

				flog(LOG_CRIT, "Invalid domain suffix specified");
				ABORT;
			}

			if (!dnssl) {
				/* first domain found */
				dnssl = malloc(sizeof(struct AdvDNSSL));

				if (dnssl == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				dnssl_init_defaults(dnssl, iface);
			}

			dnssl->AdvDNSSLNumber++;
			dnssl->AdvDNSSLSuffixes =
				realloc(dnssl->AdvDNSSLSuffixes,
					dnssl->AdvDNSSLNumber * sizeof(char*));
			if (dnssl->AdvDNSSLSuffixes == NULL) {
				flog(LOG_CRIT, "realloc failed: %s", strerror(errno));
				ABORT;
			}

			dnssl->AdvDNSSLSuffixes[dnssl->AdvDNSSLNumber - 1] = strdup((yyvsp[(1) - (1)].str));
		}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 652 "gram.y"
    {
			if (!dnssl) {
				flog(LOG_CRIT, "No domain specified in DNSSL section");
				ABORT;
			}
		}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 670 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) < iface->MaxRtrAdvInterval && (yyvsp[(2) - (3)].num) != 0) {
				flog(LOG_ERR, "AdvDNSSLLifetime must be at least MaxRtrAdvInterval");
				ABORT;
			}
			if ((yyvsp[(2) - (3)].num) > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "Warning: AdvDNSSLLifetime <= 2*MaxRtrAdvInterval would allow stale DNS suffixes to be deleted faster");

			dnssl->AdvDNSSLLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 681 "gram.y"
    {
			dnssl->FlushDNSSLFlag = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 687 "gram.y"
    {
                                (yyval.num) = (yyvsp[(1) - (1)].num); 
                        }
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 691 "gram.y"
    {
                                (yyval.num) = (uint32_t)~0;
                        }
    break;



/* Line 1455 of yacc.c  */
#line 2407 "y.tab.c"
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
#line 696 "gram.y"


static
void cleanup(void)
{
	if (iface)
		free(iface);
	
	if (prefix)
		free(prefix);
}

static void
yyerror(char *msg)
{
	cleanup();
	flog(LOG_ERR, "%s in %s, line %d: %s", msg, conf_file, num_lines, yytext);
}

