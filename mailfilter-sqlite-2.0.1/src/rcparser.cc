/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         RCSTYPE
/* Substitute the variable and function names.  */
#define yyparse         rcparse
#define yylex           rclex
#define yyerror         rcerror
#define yydebug         rcdebug
#define yynerrs         rcnerrs
#define yylval          rclval
#define yychar          rcchar

/* First part of user prologue.  */
#line 21 "rcfile.yy"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#ifdef yyalloc
#define rcalloc_ALREADY_DEFINED
#else
#define yyalloc rcalloc
#endif

#ifdef yysymbol_kind_t
#define rcsymbol_kind_t_ALREADY_DEFINED
#else
#define yysymbol_kind_t rcsymbol_kind_t
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mailfilter.hh"
#include "preferences.hh"
#include "rcfile.hh"

// We want to give the Preferences object as parameter for yyparse.
// #undef YYPARSE_PARAM
// #define YYPARSE_PARAM param

// Define P_DEBUG_MODE to get extra output from the parser.
#ifdef P_DEBUG_MODE
#undef P_DEBUG_MSG
#define P_DEBUG_MSG(msg)    \
   cout << "Parser: "       \
        << msg              \
        << endl             \
        << endl
#else
#define P_DEBUG_MSG(msg)    \
   ;
#endif

using namespace std;

FlexLexer*          rclexer;
extern int          num_lines;
extern string       sub_file;

extern string       int_to_string (int);
extern string       exec_shell  (const char*);
       void         strip_shell (char[]);

inline int yylex()
{  
  return rclexer->rclex();
}

extern "C"
{
  int rcparse(void*);
  // int rcparse(const char*);

  int rcwrap(void)
  {
    return 1;
  }

  void rcerror (const void* str, const void* NOT_USED)
  {
    cerr << PACKAGE_NAME << ": Error: " << str;
    if (sub_file.length ())
      cerr << " in file " << sub_file;
    else
      cerr << " in main rcfile";
    cerr << " in line " << (num_lines + 1);
    cerr << "." << endl;
    exit (-1);
  }
}


#line 164 "rc.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "rcparser.hh"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ALLOW = 3,                      /* ALLOW  */
  YYSYMBOL_ALLOW_CASE = 4,                 /* ALLOW_CASE  */
  YYSYMBOL_ALLOW_NOCASE = 5,               /* ALLOW_NOCASE  */
  YYSYMBOL_DEL_DUPLICATES = 6,             /* DEL_DUPLICATES  */
  YYSYMBOL_DENY_NOCASE = 7,                /* DENY_NOCASE  */
  YYSYMBOL_DENY_CASE = 8,                  /* DENY_CASE  */
  YYSYMBOL_DENY = 9,                       /* DENY  */
  YYSYMBOL_HIGHSCORE = 10,                 /* HIGHSCORE  */
  YYSYMBOL_LOGFILE = 11,                   /* LOGFILE  */
  YYSYMBOL_MAXLENGTH = 12,                 /* MAXLENGTH  */
  YYSYMBOL_MAXSIZE_ALLOW = 13,             /* MAXSIZE_ALLOW  */
  YYSYMBOL_MAXSIZE_DENY = 14,              /* MAXSIZE_DENY  */
  YYSYMBOL_MAXSIZE_SCORE = 15,             /* MAXSIZE_SCORE  */
  YYSYMBOL_NORMAL = 16,                    /* NORMAL  */
  YYSYMBOL_SERVER = 17,                    /* SERVER  */
  YYSYMBOL_USER = 18,                      /* USER  */
  YYSYMBOL_PASS = 19,                      /* PASS  */
  YYSYMBOL_PROTOCOL = 20,                  /* PROTOCOL  */
  YYSYMBOL_PORT = 21,                      /* PORT  */
  YYSYMBOL_REG_CASE = 22,                  /* REG_CASE  */
  YYSYMBOL_REG_TYPE = 23,                  /* REG_TYPE  */
  YYSYMBOL_SHOW_HEADERS = 24,              /* SHOW_HEADERS  */
  YYSYMBOL_SCORE = 25,                     /* SCORE  */
  YYSYMBOL_SCORE_CASE = 26,                /* SCORE_CASE  */
  YYSYMBOL_SCORE_NOCASE = 27,              /* SCORE_NOCASE  */
  YYSYMBOL_TIMEOUT = 28,                   /* TIMEOUT  */
  YYSYMBOL_TEST = 29,                      /* TEST  */
  YYSYMBOL_VERBOSE = 30,                   /* VERBOSE  */
  YYSYMBOL_EXP = 31,                       /* EXP  */
  YYSYMBOL_YES_NO_ID = 32,                 /* YES_NO_ID  */
  YYSYMBOL_TEXT_ID = 33,                   /* TEXT_ID  */
  YYSYMBOL_NUM_ID = 34,                    /* NUM_ID  */
  YYSYMBOL_SHELL_CMD = 35,                 /* SHELL_CMD  */
  YYSYMBOL_ENV_VAR = 36,                   /* ENV_VAR  */
  YYSYMBOL_CTRL_CHAR = 37,                 /* CTRL_CHAR  */
  YYSYMBOL_NOT_EQUAL = 38,                 /* NOT_EQUAL  */
  YYSYMBOL_39_ = 39,                       /* '='  */
  YYSYMBOL_40_ = 40,                       /* '"'  */
  YYSYMBOL_YYACCEPT = 41,                  /* $accept  */
  YYSYMBOL_commands = 42,                  /* commands  */
  YYSYMBOL_command = 43,                   /* command  */
  YYSYMBOL_exps = 44,                      /* exps  */
  YYSYMBOL_exp = 45,                       /* exp  */
  YYSYMBOL_str_arg = 46,                   /* str_arg  */
  YYSYMBOL_yes_no_arg = 47,                /* yes_no_arg  */
  YYSYMBOL_num_arg = 48,                   /* num_arg  */
  YYSYMBOL_allow_rule = 49,                /* allow_rule  */
  YYSYMBOL_del_duplicates = 50,            /* del_duplicates  */
  YYSYMBOL_deny_rule = 51,                 /* deny_rule  */
  YYSYMBOL_highscore = 52,                 /* highscore  */
  YYSYMBOL_logfile = 53,                   /* logfile  */
  YYSYMBOL_maxlength = 54,                 /* maxlength  */
  YYSYMBOL_maxsize_allow = 55,             /* maxsize_allow  */
  YYSYMBOL_maxsize_deny = 56,              /* maxsize_deny  */
  YYSYMBOL_maxsize_score = 57,             /* maxsize_score  */
  YYSYMBOL_normal = 58,                    /* normal  */
  YYSYMBOL_server = 59,                    /* server  */
  YYSYMBOL_user = 60,                      /* user  */
  YYSYMBOL_pass = 61,                      /* pass  */
  YYSYMBOL_protocol = 62,                  /* protocol  */
  YYSYMBOL_port = 63,                      /* port  */
  YYSYMBOL_reg_case = 64,                  /* reg_case  */
  YYSYMBOL_reg_type = 65,                  /* reg_type  */
  YYSYMBOL_show_headers = 66,              /* show_headers  */
  YYSYMBOL_score = 67,                     /* score  */
  YYSYMBOL_test = 68,                      /* test  */
  YYSYMBOL_timeout = 69,                   /* timeout  */
  YYSYMBOL_verbose = 70                    /* verbose  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined RCSTYPE_IS_TRIVIAL && RCSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   118

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  71
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  145

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   293


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    39,     2,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38
};

#if RCDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   160,   160,   161,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   188,   190,   207,   212,
     213,   214,   217,   220,   223,   227,   231,   235,   239,   243,
     247,   254,   261,   265,   269,   273,   277,   281,   288,   295,
     302,   309,   316,   323,   331,   338,   345,   352,   359,   371,
     378,   385,   392,   399,   404,   409,   414,   419,   424,   431,
     438,   445
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if RCDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ALLOW", "ALLOW_CASE",
  "ALLOW_NOCASE", "DEL_DUPLICATES", "DENY_NOCASE", "DENY_CASE", "DENY",
  "HIGHSCORE", "LOGFILE", "MAXLENGTH", "MAXSIZE_ALLOW", "MAXSIZE_DENY",
  "MAXSIZE_SCORE", "NORMAL", "SERVER", "USER", "PASS", "PROTOCOL", "PORT",
  "REG_CASE", "REG_TYPE", "SHOW_HEADERS", "SCORE", "SCORE_CASE",
  "SCORE_NOCASE", "TIMEOUT", "TEST", "VERBOSE", "EXP", "YES_NO_ID",
  "TEXT_ID", "NUM_ID", "SHELL_CMD", "ENV_VAR", "CTRL_CHAR", "NOT_EQUAL",
  "'='", "'\"'", "$accept", "commands", "command", "exps", "exp",
  "str_arg", "yes_no_arg", "num_arg", "allow_rule", "del_duplicates",
  "deny_rule", "highscore", "logfile", "maxlength", "maxsize_allow",
  "maxsize_deny", "maxsize_score", "normal", "server", "user", "pass",
  "protocol", "port", "reg_case", "reg_type", "show_headers", "score",
  "test", "timeout", "verbose", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-60)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -60,    82,   -60,   -21,   -19,     9,   -31,    11,    13,    17,
     -17,   -12,    -7,    -3,    14,    28,    27,    34,    35,    36,
      37,    38,    39,    40,    41,    28,    28,    28,    42,    44,
      74,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,     3,     3,     3,     3,     3,     3,
      75,     3,     3,     3,     3,     3,     3,    28,     3,    28,
      28,    28,   -60,    77,    75,     3,     3,     3,     3,    28,
      75,     3,     3,    19,    21,    26,    28,    75,    28,     4,
     -60,   -60,   -60,   -60,   -60,   -60,    85,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,    28,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,     3,
       3,     3,     3,     3,     3,   -60,   -60,   -60,   -60,   -60,
     -60,   -60,    -2,   -60,    78,   -60,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    35,    38,    37,    40,    39,     0,    41,    47,    46,
      45,    44,    43,    42,    48,    49,    50,    51,    52,     0,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,    70,    69,    71,    30,    28,
      29,    31,     0,    26,     0,    53,    64,    63,    66,    65,
      68,    67,    32,    27,    33
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -60,   -60,   -60,   -60,   -18,   -52,   -59,   -25,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,    31,   132,   133,    90,    97,    73,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      83,    84,    85,    91,    92,    93,    94,    95,    60,    98,
      99,   100,   101,   102,   103,   110,   105,    54,    55,    56,
      57,   116,    67,   111,   112,   113,   114,    68,   126,   117,
     118,   128,    69,   129,   130,   131,    70,   128,   142,   129,
     130,   131,   104,    89,   106,   107,   108,    58,    59,    61,
      62,    63,    64,    71,   115,    65,    66,   119,   120,   121,
     122,   125,    72,   127,   123,   124,    74,   136,   137,   138,
     139,   140,   141,    75,    76,    77,    78,    79,    80,    81,
      82,    86,     2,    87,   135,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    88,   143,    96,   109,   134,   144
};

static const yytype_uint8 yycheck[] =
{
      25,    26,    27,    55,    56,    57,    58,    59,    39,    61,
      62,    63,    64,    65,    66,    74,    68,    38,    39,    38,
      39,    80,    39,    75,    76,    77,    78,    39,    87,    81,
      82,    33,    39,    35,    36,    37,    39,    33,    40,    35,
      36,    37,    67,    40,    69,    70,    71,    38,    39,    38,
      39,    38,    39,    39,    79,    38,    39,    38,    39,    38,
      39,    86,    34,    88,    38,    39,    39,   119,   120,   121,
     122,   123,   124,    39,    39,    39,    39,    39,    39,    39,
      39,    39,     0,    39,   109,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    39,   132,    40,    39,    32,    40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    42,     0,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    43,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    38,    39,    38,    39,    38,    39,
      39,    38,    39,    38,    39,    38,    39,    39,    39,    39,
      39,    39,    34,    48,    39,    39,    39,    39,    39,    39,
      39,    39,    39,    48,    48,    48,    39,    39,    39,    40,
      46,    46,    46,    46,    46,    46,    40,    47,    46,    46,
      46,    46,    46,    46,    48,    46,    48,    48,    48,    39,
      47,    46,    46,    46,    46,    48,    47,    46,    46,    38,
      39,    38,    39,    38,    39,    48,    47,    48,    33,    35,
      36,    37,    44,    45,    32,    48,    46,    46,    46,    46,
      46,    46,    40,    45,    40
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    41,    42,    42,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    44,    44,    45,    45,
      45,    45,    46,    47,    48,    49,    49,    49,    49,    49,
      49,    50,    51,    51,    51,    51,    51,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    67,    67,    67,    67,    67,    68,
      69,    70
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     1,     3,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     4,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     4,     4,     4,     4,     4,     4,     3,
       3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = RCEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == RCEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (param, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use RCerror or RCUNDEF. */
#define YYERRCODE RCUNDEF


/* Enable debugging if requested.  */
#if RCDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, param); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void* param)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (param);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void* param)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, param);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void* param)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], param);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, param); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !RCDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !RCDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void* param)
{
  YY_USE (yyvaluep);
  YY_USE (param);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void* param)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = RCEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == RCEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= RCEOF)
    {
      yychar = RCEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == RCerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = RCUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = RCEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 26: /* exps: exp  */
#line 189 "rcfile.yy"
                { (yyval.sval) = (yyvsp[0].sval); }
#line 1313 "rc.tab.c"
    break;

  case 27: /* exps: exps exp  */
#line 191 "rcfile.yy"
                { char* tmp = (char*)malloc ((strlen ((yyvsp[-1].sval)) + strlen ((yyvsp[0].sval)) + 2)
						* sizeof (char));
		  if (tmp)
		    {
		      strcpy (tmp, (yyvsp[-1].sval));
                      (yyval.sval) = strcat (tmp, (yyvsp[0].sval));
		      free ((yyvsp[-1].sval)); free ((yyvsp[0].sval));
		    }
		  else
                    {
		      cerr << "Out of memory error in rcparser." << endl;
		      exit (-1);
                    }
		}
#line 1332 "rc.tab.c"
    break;

  case 28: /* exp: SHELL_CMD  */
#line 208 "rcfile.yy"
                { strip_shell ((yyvsp[0].sval));
		  (yyval.sval) = strdup (exec_shell ((yyvsp[0].sval)).c_str ());
		  free ((yyvsp[0].sval));
		}
#line 1341 "rc.tab.c"
    break;

  case 29: /* exp: ENV_VAR  */
#line 212 "rcfile.yy"
                                  { (yyval.sval) = (yyvsp[0].sval); }
#line 1347 "rc.tab.c"
    break;

  case 30: /* exp: TEXT_ID  */
#line 213 "rcfile.yy"
                                  { (yyval.sval) = (yyvsp[0].sval); }
#line 1353 "rc.tab.c"
    break;

  case 31: /* exp: CTRL_CHAR  */
#line 214 "rcfile.yy"
                                  { (yyval.sval) = (yyvsp[0].sval); }
#line 1359 "rc.tab.c"
    break;

  case 32: /* str_arg: '"' exps '"'  */
#line 217 "rcfile.yy"
                                  { (yyval.sval) = (yyvsp[-1].sval); }
#line 1365 "rc.tab.c"
    break;

  case 33: /* yes_no_arg: '"' YES_NO_ID '"'  */
#line 220 "rcfile.yy"
                                  { (yyval.sval) = (yyvsp[-1].sval); }
#line 1371 "rc.tab.c"
    break;

  case 34: /* num_arg: NUM_ID  */
#line 223 "rcfile.yy"
                                  { (yyval.ival) = (yyvsp[0].ival); }
#line 1377 "rc.tab.c"
    break;

  case 35: /* allow_rule: ALLOW '=' str_arg  */
#line 228 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_allow_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval)); 
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1385 "rc.tab.c"
    break;

  case 36: /* allow_rule: ALLOW NOT_EQUAL str_arg  */
#line 232 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_allow_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1393 "rc.tab.c"
    break;

  case 37: /* allow_rule: ALLOW_CASE '=' str_arg  */
#line 236 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_allow_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1401 "rc.tab.c"
    break;

  case 38: /* allow_rule: ALLOW_CASE NOT_EQUAL str_arg  */
#line 240 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
	          Preferences :: Instance ().add_allow_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1409 "rc.tab.c"
    break;

  case 39: /* allow_rule: ALLOW_NOCASE '=' str_arg  */
#line 244 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_allow_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1417 "rc.tab.c"
    break;

  case 40: /* allow_rule: ALLOW_NOCASE NOT_EQUAL str_arg  */
#line 248 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_allow_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1425 "rc.tab.c"
    break;

  case 41: /* del_duplicates: DEL_DUPLICATES '=' yes_no_arg  */
#line 255 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_del_duplicates ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1433 "rc.tab.c"
    break;

  case 42: /* deny_rule: DENY '=' str_arg  */
#line 262 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_deny_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
 		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1441 "rc.tab.c"
    break;

  case 43: /* deny_rule: DENY NOT_EQUAL str_arg  */
#line 266 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_deny_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1449 "rc.tab.c"
    break;

  case 44: /* deny_rule: DENY_CASE '=' str_arg  */
#line 270 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_deny_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1457 "rc.tab.c"
    break;

  case 45: /* deny_rule: DENY_CASE NOT_EQUAL str_arg  */
#line 274 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_deny_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1465 "rc.tab.c"
    break;

  case 46: /* deny_rule: DENY_NOCASE '=' str_arg  */
#line 278 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_deny_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1473 "rc.tab.c"
    break;

  case 47: /* deny_rule: DENY_NOCASE NOT_EQUAL str_arg  */
#line 282 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().add_deny_rule ((yyvsp[-2].sval), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1481 "rc.tab.c"
    break;

  case 48: /* highscore: HIGHSCORE '=' num_arg  */
#line 289 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_highscore ((yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1489 "rc.tab.c"
    break;

  case 49: /* logfile: LOGFILE '=' str_arg  */
#line 296 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().set_log_file ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval));  }
#line 1497 "rc.tab.c"
    break;

  case 50: /* maxlength: MAXLENGTH '=' num_arg  */
#line 303 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_maxlength ((yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1505 "rc.tab.c"
    break;

  case 51: /* maxsize_allow: MAXSIZE_ALLOW '=' num_arg  */
#line 310 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_max_size_allow ((yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1513 "rc.tab.c"
    break;

  case 52: /* maxsize_deny: MAXSIZE_DENY '=' num_arg  */
#line 317 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_max_size_deny ((yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1521 "rc.tab.c"
    break;

  case 53: /* maxsize_score: MAXSIZE_SCORE num_arg '=' num_arg  */
#line 324 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
                              + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_max_size_score ((yyvsp[-2].ival), (yyvsp[0].ival));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); }
#line 1530 "rc.tab.c"
    break;

  case 54: /* normal: NORMAL '=' yes_no_arg  */
#line 332 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_normal ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1538 "rc.tab.c"
    break;

  case 55: /* server: SERVER '=' str_arg  */
#line 339 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_server ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1546 "rc.tab.c"
    break;

  case 56: /* user: USER '=' str_arg  */
#line 346 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_usr ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1554 "rc.tab.c"
    break;

  case 57: /* pass: PASS '=' str_arg  */
#line 353 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_passwd ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1562 "rc.tab.c"
    break;

  case 58: /* protocol: PROTOCOL '=' str_arg  */
#line 360 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_protocol ((yyvsp[0].sval));

                  /* See account.cc for further information about
                     the following (rather misplaced) call:  */

                  Preferences :: Instance ().set_connection ();
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1575 "rc.tab.c"
    break;

  case 59: /* port: PORT '=' num_arg  */
#line 372 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_port ((unsigned int)(yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1583 "rc.tab.c"
    break;

  case 60: /* reg_case: REG_CASE '=' yes_no_arg  */
#line 379 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_default_case ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1591 "rc.tab.c"
    break;

  case 61: /* reg_type: REG_TYPE '=' str_arg  */
#line 386 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_reg_type ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1599 "rc.tab.c"
    break;

  case 62: /* show_headers: SHOW_HEADERS '=' str_arg  */
#line 393 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
                  Preferences :: Instance ().set_headers_file ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval));  }
#line 1607 "rc.tab.c"
    break;

  case 63: /* score: SCORE num_arg '=' str_arg  */
#line 400 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
                              + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().add_score ((yyvsp[-3].sval), (yyvsp[-2].ival), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1616 "rc.tab.c"
    break;

  case 64: /* score: SCORE num_arg NOT_EQUAL str_arg  */
#line 405 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
	                      + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().add_score ((yyvsp[-3].sval), (yyvsp[-2].ival), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1625 "rc.tab.c"
    break;

  case 65: /* score: SCORE_CASE num_arg '=' str_arg  */
#line 410 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
			      + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().add_score ((yyvsp[-3].sval), (yyvsp[-2].ival), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1634 "rc.tab.c"
    break;

  case 66: /* score: SCORE_CASE num_arg NOT_EQUAL str_arg  */
#line 415 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
			      + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().add_score ((yyvsp[-3].sval), (yyvsp[-2].ival), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1643 "rc.tab.c"
    break;

  case 67: /* score: SCORE_NOCASE num_arg '=' str_arg  */
#line 420 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
			      + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().add_score ((yyvsp[-3].sval), (yyvsp[-2].ival), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1652 "rc.tab.c"
    break;

  case 68: /* score: SCORE_NOCASE num_arg NOT_EQUAL str_arg  */
#line 425 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-3].sval) + int_to_string (rclval.ival)
			      + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().add_score ((yyvsp[-3].sval), (yyvsp[-2].ival), (yyvsp[-1].sval), (yyvsp[0].sval));
		  free ((yyvsp[-3].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1661 "rc.tab.c"
    break;

  case 69: /* test: TEST '=' yes_no_arg  */
#line 432 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + (yyvsp[0].sval));
		  Preferences :: Instance ().set_test_mode ((yyvsp[0].sval));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); free ((yyvsp[0].sval)); }
#line 1669 "rc.tab.c"
    break;

  case 70: /* timeout: TIMEOUT '=' num_arg  */
#line 439 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_time_out ((unsigned int)(yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1677 "rc.tab.c"
    break;

  case 71: /* verbose: VERBOSE '=' num_arg  */
#line 446 "rcfile.yy"
                { P_DEBUG_MSG((yyvsp[-2].sval) + (string)(yyvsp[-1].sval) + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_verbose_level ((yyvsp[0].ival));
		  free ((yyvsp[-2].sval)); free ((yyvsp[-1].sval)); }
#line 1685 "rc.tab.c"
    break;


#line 1689 "rc.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == RCEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (param, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= RCEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == RCEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, param);
          yychar = RCEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, param);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (param, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != RCEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, param);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, param);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 450 "rcfile.yy"


/* This function strips the leading, and trailing quotation marks
   around a shell `command' to be executed by the POSIX shell.  */

void strip_shell(char s[])
{
  string buf = s;
  for (unsigned int i = 1; i < buf.length () - 1; i++) {
    s[i-1] = buf[i];
    s[i] = '\0';
  }
}

/* The class declarations can be found in rcfile.hh.  */

RCParser :: RCParser(istream* ip, ostream* op)
  : isp(ip), osp(op)
{
  try {
    rclexer = new rcFlexLexer(isp, osp);
  }
  catch (...) {  
    throw;  
  } 
}

RCParser :: ~RCParser()
{ 
  delete rclexer;
}

void RCParser :: parse(void* val)
{
  rclexer->yyrestart(isp);
  rcparse(val);
}
