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
    MAXLENGTH = 267,               /* MAXLENGTH  */
    MAXSIZE_ALLOW = 268,           /* MAXSIZE_ALLOW  */
    MAXSIZE_DENY = 269,            /* MAXSIZE_DENY  */
    MAXSIZE_SCORE = 270,           /* MAXSIZE_SCORE  */
    NORMAL = 271,                  /* NORMAL  */
    SERVER = 272,                  /* SERVER  */
    USER = 273,                    /* USER  */
    PASS = 274,                    /* PASS  */
    PROTOCOL = 275,                /* PROTOCOL  */
    PORT = 276,                    /* PORT  */
    REG_CASE = 277,                /* REG_CASE  */
    REG_TYPE = 278,                /* REG_TYPE  */
    SHOW_HEADERS = 279,            /* SHOW_HEADERS  */
    SCORE = 280,                   /* SCORE  */
    SCORE_CASE = 281,              /* SCORE_CASE  */
    SCORE_NOCASE = 282,            /* SCORE_NOCASE  */
    TIMEOUT = 283,                 /* TIMEOUT  */
    TEST = 284,                    /* TEST  */
    VERBOSE = 285,                 /* VERBOSE  */
    EXP = 286,                     /* EXP  */
    YES_NO_ID = 287,               /* YES_NO_ID  */
    TEXT_ID = 288,                 /* TEXT_ID  */
    NUM_ID = 289,                  /* NUM_ID  */
    SHELL_CMD = 290,               /* SHELL_CMD  */
    ENV_VAR = 291,                 /* ENV_VAR  */
    CTRL_CHAR = 292,               /* CTRL_CHAR  */
    NOT_EQUAL = 293                /* NOT_EQUAL  */
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
#define MAXLENGTH 267
#define MAXSIZE_ALLOW 268
#define MAXSIZE_DENY 269
#define MAXSIZE_SCORE 270
#define NORMAL 271
#define SERVER 272
#define USER 273
#define PASS 274
#define PROTOCOL 275
#define PORT 276
#define REG_CASE 277
#define REG_TYPE 278
#define SHOW_HEADERS 279
#define SCORE 280
#define SCORE_CASE 281
#define SCORE_NOCASE 282
#define TIMEOUT 283
#define TEST 284
#define VERBOSE 285
#define EXP 286
#define YES_NO_ID 287
#define TEXT_ID 288
#define NUM_ID 289
#define SHELL_CMD 290
#define ENV_VAR 291
#define CTRL_CHAR 292
#define NOT_EQUAL 293

/* Value type.  */
#if ! defined RCSTYPE && ! defined RCSTYPE_IS_DECLARED
#line 106 "rcfile.yy"
union rc
{
#line 107 "rcfile.yy"

  int   ival;
  char* sval;

#line 157 "rcparser.hh"

};
#line 106 "rcfile.yy"
typedef union rc RCSTYPE;
# define RCSTYPE_IS_TRIVIAL 1
# define RCSTYPE_IS_DECLARED 1
#endif


extern RCSTYPE rclval;


int rcparse (void* param);


#endif /* !YY_RC_RCPARSER_HH_INCLUDED  */
