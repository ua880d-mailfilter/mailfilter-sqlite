/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_RC_RCPARSER_HH_INCLUDED
# define YY_RC_RCPARSER_HH_INCLUDED
/* Debug traces.  */
#ifndef RCDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define RCDEBUG 1
#  else
#   define RCDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define RCDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined RCDEBUG */
#if RCDEBUG
extern int rcdebug;
#endif

/* Token kinds.  */
#ifndef RCTOKENTYPE
# define RCTOKENTYPE
  enum rctokentype
  {
    RCEMPTY = -2,
    RCEOF = 0,                     /* "end of file"  */
    RCerror = 256,                 /* error  */
    RCUNDEF = 257,                 /* "invalid token"  */
    ALLOW = 258,                   /* ALLOW  */
    ALLOW_CASE = 259,              /* ALLOW_CASE  */
    ALLOW_NOCASE = 260,            /* ALLOW_NOCASE  */
    DEL_DUPLICATES = 261,          /* DEL_DUPLICATES  */
    DENY_NOCASE = 262,             /* DENY_NOCASE  */
    DENY_CASE = 263,               /* DENY_CASE  */
    DENY = 264,                    /* DENY  */
    HIGHSCORE = 265,               /* HIGHSCORE  */
    LOGFILE = 266,                 /* LOGFILE  */
    LOG_HEADERS_SQLITE3 = 267,     /* LOG_HEADERS_SQLITE3  */
    MAXLENGTH = 268,               /* MAXLENGTH  */
    MAXSIZE_ALLOW = 269,           /* MAXSIZE_ALLOW  */
    MAXSIZE_DENY = 270,            /* MAXSIZE_DENY  */
    MAXSIZE_SCORE = 271,           /* MAXSIZE_SCORE  */
    NORMAL = 272,                  /* NORMAL  */
    SERVER = 273,                  /* SERVER  */
    USER = 274,                    /* USER  */
    PASS = 275,                    /* PASS  */
    PROTOCOL = 276,                /* PROTOCOL  */
    PORT = 277,                    /* PORT  */
    REG_CASE = 278,                /* REG_CASE  */
    REG_TYPE = 279,                /* REG_TYPE  */
    SHOW_HEADERS = 280,            /* SHOW_HEADERS  */
    SCORE = 281,                   /* SCORE  */
    SCORE_CASE = 282,              /* SCORE_CASE  */
    SCORE_NOCASE = 283,            /* SCORE_NOCASE  */
    TIMEOUT = 284,                 /* TIMEOUT  */
    TEST = 285,                    /* TEST  */
    VERBOSE = 286,                 /* VERBOSE  */
    EXP = 287,                     /* EXP  */
    YES_NO_ID = 288,               /* YES_NO_ID  */
    TEXT_ID = 289,                 /* TEXT_ID  */
    NUM_ID = 290,                  /* NUM_ID  */
    SHELL_CMD = 291,               /* SHELL_CMD  */
    ENV_VAR = 292,                 /* ENV_VAR  */
    CTRL_CHAR = 293,               /* CTRL_CHAR  */
    NOT_EQUAL = 294                /* NOT_EQUAL  */
  };
  typedef enum rctokentype rctoken_kind_t;
#endif
/* Token kinds.  */
#define RCEMPTY -2
#define RCEOF 0
#define RCerror 256
#define RCUNDEF 257
#define ALLOW 258
#define ALLOW_CASE 259
#define ALLOW_NOCASE 260
#define DEL_DUPLICATES 261
#define DENY_NOCASE 262
#define DENY_CASE 263
#define DENY 264
#define HIGHSCORE 265
#define LOGFILE 266
#define LOG_HEADERS_SQLITE3 267
#define MAXLENGTH 268
#define MAXSIZE_ALLOW 269
#define MAXSIZE_DENY 270
#define MAXSIZE_SCORE 271
#define NORMAL 272
#define SERVER 273
#define USER 274
#define PASS 275
#define PROTOCOL 276
#define PORT 277
#define REG_CASE 278
#define REG_TYPE 279
#define SHOW_HEADERS 280
#define SCORE 281
#define SCORE_CASE 282
#define SCORE_NOCASE 283
#define TIMEOUT 284
#define TEST 285
#define VERBOSE 286
#define EXP 287
#define YES_NO_ID 288
#define TEXT_ID 289
#define NUM_ID 290
#define SHELL_CMD 291
#define ENV_VAR 292
#define CTRL_CHAR 293
#define NOT_EQUAL 294

/* Value type.  */
#if ! defined RCSTYPE && ! defined RCSTYPE_IS_DECLARED
#line 106 "rcfile.yy"
union rc
{
#line 107 "rcfile.yy"

  int   ival;
  char* sval;

#line 159 "rcparser.hh"

};
#line 106 "rcfile.yy"
typedef union rc RCSTYPE;
# define RCSTYPE_IS_TRIVIAL 1
# define RCSTYPE_IS_DECLARED 1
#endif


extern RCSTYPE rclval;


int rcparse (void* param);


#endif /* !YY_RC_RCPARSER_HH_INCLUDED  */
