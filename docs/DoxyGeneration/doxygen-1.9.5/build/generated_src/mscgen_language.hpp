/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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

#ifndef YY_YY_C_USERS_RUFOB_DOCUMENTS_GITHUB_COLLAPSEDFRAGMENT_DOCS_DOXYGENERATION_DOXYGEN_1_9_5_BUILD_GENERATED_SRC_MSCGEN_LANGUAGE_HPP_INCLUDED
# define YY_YY_C_USERS_RUFOB_DOCUMENTS_GITHUB_COLLAPSEDFRAGMENT_DOCS_DOXYGENERATION_DOXYGEN_1_9_5_BUILD_GENERATED_SRC_MSCGEN_LANGUAGE_HPP_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_STRING = 258,
     TOK_QSTRING = 259,
     TOK_EQUAL = 260,
     TOK_COMMA = 261,
     TOK_SEMICOLON = 262,
     TOK_OCBRACKET = 263,
     TOK_CCBRACKET = 264,
     TOK_OSBRACKET = 265,
     TOK_CSBRACKET = 266,
     TOK_MSC = 267,
     TOK_ATTR_LABEL = 268,
     TOK_ATTR_URL = 269,
     TOK_ATTR_ID = 270,
     TOK_ATTR_IDURL = 271,
     TOK_ATTR_LINE_COLOUR = 272,
     TOK_ATTR_TEXT_COLOUR = 273,
     TOK_ATTR_TEXT_BGCOLOUR = 274,
     TOK_ATTR_ARC_LINE_COLOUR = 275,
     TOK_ATTR_ARC_TEXT_COLOUR = 276,
     TOK_ATTR_ARC_TEXT_BGCOLOUR = 277,
     TOK_REL_LOSS_TO = 278,
     TOK_REL_LOSS_FROM = 279,
     TOK_REL_SIG_BI = 280,
     TOK_REL_SIG_TO = 281,
     TOK_REL_SIG_FROM = 282,
     TOK_REL_METHOD_BI = 283,
     TOK_REL_METHOD_TO = 284,
     TOK_REL_METHOD_FROM = 285,
     TOK_REL_RETVAL_BI = 286,
     TOK_REL_RETVAL_TO = 287,
     TOK_REL_RETVAL_FROM = 288,
     TOK_REL_DOUBLE_BI = 289,
     TOK_REL_DOUBLE_TO = 290,
     TOK_REL_DOUBLE_FROM = 291,
     TOK_REL_CALLBACK_BI = 292,
     TOK_REL_CALLBACK_TO = 293,
     TOK_REL_CALLBACK_FROM = 294,
     TOK_REL_BOX = 295,
     TOK_REL_ABOX = 296,
     TOK_REL_RBOX = 297,
     TOK_REL_NOTE = 298,
     TOK_SPECIAL_ARC = 299,
     TOK_OPT_HSCALE = 300,
     TOK_OPT_WIDTH = 301,
     TOK_OPT_ARCGRADIENT = 302,
     TOK_OPT_WORDWRAPARCS = 303,
     TOK_ASTERISK = 304,
     TOK_UNKNOWN = 305,
     TOK_REL_SIG = 306,
     TOK_REL_METHOD = 307,
     TOK_REL_RETVAL = 308,
     TOK_REL_DOUBLE = 309,
     TOK_ATTR_ARC_SKIP = 310
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 251 "mscgen_language.y"

    char         *string;
    Msc           msc;
    MscOpt        opt;
    MscOptType    optType;
    MscArc        arc;
    MscArcList    arclist;
    MscArcType    arctype;
    MscEntity     entity;
    MscEntityList entitylist;
    MscAttrib     attrib;
    MscAttribType attribType;


/* Line 2058 of yacc.c  */
#line 127 "C:/Users/rufob/Documents/GitHub/CollapsedFragment/docs/DoxyGeneration/doxygen-1.9.5/build/generated_src/mscgen_language.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_C_USERS_RUFOB_DOCUMENTS_GITHUB_COLLAPSEDFRAGMENT_DOCS_DOXYGENERATION_DOXYGEN_1_9_5_BUILD_GENERATED_SRC_MSCGEN_LANGUAGE_HPP_INCLUDED  */
