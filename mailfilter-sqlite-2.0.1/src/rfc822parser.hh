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

#ifndef YY_RFC_RFC822PARSER_HH_INCLUDED
# define YY_RFC_RFC822PARSER_HH_INCLUDED
/* Debug traces.  */
#ifndef RFCDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define RFCDEBUG 1
#  else
#   define RFCDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define RFCDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined RFCDEBUG */
#if RFCDEBUG
extern int rfcdebug;
#endif

/* Token kinds.  */
#ifndef RFCTOKENTYPE
# define RFCTOKENTYPE
  enum rfctokentype
  {
    RFCEMPTY = -2,
    RFCEOF = 0,                    /* "end of file"  */
    RFCerror = 256,                /* error  */
    RFCUNDEF = 257,                /* "invalid token"  */
    HEADER_END = 258,              /* HEADER_END  */
    TOPLINE = 259,                 /* TOPLINE  */
    TAG = 260,                     /* TAG  */
    BODY = 261,                    /* BODY  */
    BODY_MULTI_LINE = 262          /* BODY_MULTI_LINE  */
  };
  typedef enum rfctokentype rfctoken_kind_t;
#endif
/* Token kinds.  */
#define RFCEMPTY -2
#define RFCEOF 0
#define RFCerror 256
#define RFCUNDEF 257
#define HEADER_END 258
#define TOPLINE 259
#define TAG 260
#define BODY 261
#define BODY_MULTI_LINE 262

/* Value type.  */
#if ! defined RFCSTYPE && ! defined RFCSTYPE_IS_DECLARED
#line 80 "rfc822.yy"
union rfc
{
#line 81 "rfc822.yy"

  char* sval;

#line 94 "rfc822parser.hh"

};
#line 80 "rfc822.yy"
typedef union rfc RFCSTYPE;
# define RFCSTYPE_IS_TRIVIAL 1
# define RFCSTYPE_IS_DECLARED 1
#endif


extern RFCSTYPE rfclval;


int rfcparse (const char* param);


#endif /* !YY_RFC_RFC822PARSER_HH_INCLUDED  */
