/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
     IDENT = 258,
     FCONST = 259,
     SCONST = 260,
     BCONST = 261,
     XCONST = 262,
     Op = 263,
     ICONST = 264,
     PARAM = 265,
     TYPECAST = 266,
     DOT_DOT = 267,
     COLON_EQUALS = 268,
     T_WORD = 269,
     T_CWORD = 270,
     T_DATUM = 271,
     LESS_LESS = 272,
     GREATER_GREATER = 273,
     K_ABSOLUTE = 274,
     K_ALIAS = 275,
     K_ALL = 276,
     K_ARRAY = 277,
     K_BACKWARD = 278,
     K_BEGIN = 279,
     K_BY = 280,
     K_CASE = 281,
     K_CLOSE = 282,
     K_COLLATE = 283,
     K_COLUMN = 284,
     K_COLUMN_NAME = 285,
     K_CONSTANT = 286,
     K_CONSTRAINT = 287,
     K_CONSTRAINT_NAME = 288,
     K_CONTINUE = 289,
     K_CURRENT = 290,
     K_CURSOR = 291,
     K_DATATYPE = 292,
     K_DEBUG = 293,
     K_DECLARE = 294,
     K_DEFAULT = 295,
     K_DETAIL = 296,
     K_DIAGNOSTICS = 297,
     K_DUMP = 298,
     K_ELSE = 299,
     K_ELSIF = 300,
     K_END = 301,
     K_ERRCODE = 302,
     K_ERROR = 303,
     K_EXCEPTION = 304,
     K_EXECUTE = 305,
     K_EXIT = 306,
     K_FETCH = 307,
     K_FIRST = 308,
     K_FOR = 309,
     K_FOREACH = 310,
     K_FORWARD = 311,
     K_FROM = 312,
     K_GET = 313,
     K_HINT = 314,
     K_IF = 315,
     K_IN = 316,
     K_INFO = 317,
     K_INSERT = 318,
     K_INTO = 319,
     K_IS = 320,
     K_LAST = 321,
     K_LOG = 322,
     K_LOOP = 323,
     K_MESSAGE = 324,
     K_MESSAGE_TEXT = 325,
     K_MOVE = 326,
     K_NEXT = 327,
     K_NO = 328,
     K_NOT = 329,
     K_NOTICE = 330,
     K_NULL = 331,
     K_OPEN = 332,
     K_OPTION = 333,
     K_OR = 334,
     K_PERFORM = 335,
     K_PG_CONTEXT = 336,
     K_PG_DATATYPE_NAME = 337,
     K_PG_EXCEPTION_CONTEXT = 338,
     K_PG_EXCEPTION_DETAIL = 339,
     K_PG_EXCEPTION_HINT = 340,
     K_PRINT_STRICT_PARAMS = 341,
     K_PRIOR = 342,
     K_QUERY = 343,
     K_RAISE = 344,
     K_RELATIVE = 345,
     K_RESULT_OID = 346,
     K_RETURN = 347,
     K_RETURNED_SQLSTATE = 348,
     K_REVERSE = 349,
     K_ROWTYPE = 350,
     K_ROW_COUNT = 351,
     K_SCHEMA = 352,
     K_SCHEMA_NAME = 353,
     K_SCROLL = 354,
     K_SLICE = 355,
     K_SQLSTATE = 356,
     K_STACKED = 357,
     K_STRICT = 358,
     K_TABLE = 359,
     K_TABLE_NAME = 360,
     K_THEN = 361,
     K_TO = 362,
     K_TYPE = 363,
     K_USE_COLUMN = 364,
     K_USE_VARIABLE = 365,
     K_USING = 366,
     K_VARIABLE_CONFLICT = 367,
     K_WARNING = 368,
     K_WHEN = 369,
     K_WHILE = 370
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 116 "pl_gram.y"

		core_YYSTYPE			core_yystype;
		/* these fields must match core_YYSTYPE: */
		int						ival;
		char					*str;
		const char				*keyword;

		PLword					word;
		PLcword					cword;
		PLwdatum				wdatum;
		bool					boolean;
		Oid						oid;
		struct
		{
			char *name;
			int  lineno;
		}						varname;
		struct
		{
			char *name;
			int  lineno;
			PLpgSQL_datum   *scalar;
			PLpgSQL_rec		*rec;
			PLpgSQL_row		*row;
		}						forvariable;
		struct
		{
			char *label;
			int  n_initvars;
			int  *initvarnos;
		}						declhdr;
		struct
		{
			List *stmts;
			char *end_label;
			int   end_label_location;
		}						loop_body;
		List					*list;
		PLpgSQL_type			*dtype;
		PLpgSQL_datum			*datum;
		PLpgSQL_var				*var;
		PLpgSQL_expr			*expr;
		PLpgSQL_stmt			*stmt;
		PLpgSQL_condition		*condition;
		PLpgSQL_exception		*exception;
		PLpgSQL_exception_block	*exception_block;
		PLpgSQL_nsitem			*nsitem;
		PLpgSQL_diag_item		*diagitem;
		PLpgSQL_stmt_fetch		*fetch;
		PLpgSQL_case_when		*casewhen;



/* Line 2068 of yacc.c  */
#line 219 "pl_gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE plpgsql_yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE plpgsql_yylloc;

