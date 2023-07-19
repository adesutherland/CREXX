/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     TOKEN_CJUMP = 258,
     TOKEN_CNEXT = 259,
     TOKEN_CLIST = 260,
     TOKEN_CSETUP = 261,
     TOKEN_CZERO = 262,
     TOKEN_CLOSESIZE = 263,
     TOKEN_CODE = 264,
     TOKEN_CONF = 265,
     TOKEN_ID = 266,
     TOKEN_FID = 267,
     TOKEN_FID_END = 268,
     TOKEN_LINE_INFO = 269,
     TOKEN_REGEXP = 270,
     TOKEN_BLOCK = 271
   };
#endif
/* Tokens.  */
#define TOKEN_CJUMP 258
#define TOKEN_CNEXT 259
#define TOKEN_CLIST 260
#define TOKEN_CSETUP 261
#define TOKEN_CZERO 262
#define TOKEN_CLOSESIZE 263
#define TOKEN_CODE 264
#define TOKEN_CONF 265
#define TOKEN_ID 266
#define TOKEN_FID 267
#define TOKEN_FID_END 268
#define TOKEN_LINE_INFO 269
#define TOKEN_REGEXP 270
#define TOKEN_BLOCK 271




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 33 "../../crexx-f0049/re2c/src/parse/parser.ypp"
{
    const re2c::AST *regexp;
    re2c::SemAct    *semact;
    char             op;
    re2c::ASTBounds  bounds;
    std::string     *str;
    re2c::CondList  *clist;
}
/* Line 1529 of yacc.c.  */
#line 90 "src/parse/parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

