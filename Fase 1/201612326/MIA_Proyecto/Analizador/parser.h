/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 45 "sintactico.y" /* yacc.c:1909  */

    /*Este fragmento de código aparecerá en el parser.h.... y por alguna razón, también en el parser.cpp*/
    
    #include <map>
    #include "comando.h"
    #include <iostream>
    #include <math.h>
    #include "string.h"

#line 54 "parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    guion = 258,
    igual = 259,
    comentario = 260,
    mkdisk = 261,
    rmdisk = 262,
    fdisk = 263,
    mount = 264,
    unmount = 265,
    exec = 266,
    rep = 267,
    size = 268,
    fit = 269,
    unit = 270,
    path = 271,
    type = 272,
    deleteParam = 273,
    name = 274,
    add = 275,
    r_id = 276,
    mkfs = 277,
    login = 278,
    logout = 279,
    mkgrp = 280,
    rmgrp = 281,
    mkusr = 282,
    rmusr = 283,
    r_chmod = 284,
    mkfile = 285,
    cat = 286,
    rem = 287,
    edit = 288,
    ren = 289,
    r_mkdir = 290,
    cp = 291,
    mv = 292,
    find = 293,
    r_chown = 294,
    chgrp = 295,
    r_pause = 296,
    loss = 297,
    recovery = 298,
    fs = 299,
    usr = 300,
    pwd = 301,
    grp = 302,
    ugo = 303,
    r = 304,
    p = 305,
    cont = 306,
    file = 307,
    dest = 308,
    ruta = 309,
    valor = 310,
    rutaComilla = 311,
    valorComilla = 312
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 38 "sintactico.y" /* yacc.c:1909  */

    char TEXT[256];
    struct parametro PARAM;
    struct comando COM;
    std::map<TipoParametro, struct parametro>* PARAMS;

#line 131 "parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
