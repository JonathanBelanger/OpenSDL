/*
 * Copyright (C) Jonathan D. Belanger 2018.
 *
 *  OpenSDL is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  OpenSDL is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenSDL.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Description:
 *
 *  This source file contains the grammar definitions needed to support the
 *  parsing of an Open Structure Definition Language (OpenSDL) formatted
 *  definition file.
 *
 * Revision History:
 *
 *  V01.000	20-Sep-2018	Jonathan D. Belanger
 *  Initially written.
 */
%verbose
%define parse.lac	full
%define parse.error verbose
%define api.pure true

%locations
%token-table

/*
 * This generates the .output file.
 */
%verbose

%lex-param {void *scanner}
%parse-param {void *scanner}

/*
 * Prologue
 */
%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "opensdl_lexical.h"
#include "opensdl_parser.h"
#include "opensdl_actions.h"

SDL_CONTEXT			context;
SDL_QUEUE			literal;		/* A list of lines in the between LITERAL/END_LITERAL */

bool				literalState = false;

void yyerror(YYLTYPE *locp, yyscan_t *scanner, char const *msg);
%}

/*
 * Declarations
 */
%union
{
	__int64_t	ival;
	char		*tval;
	void		*aval;
}

%start beginning

/*
 * We get tokens from the flex side of the house.
 */
%token SDL_K_LITERAL
%token SDL_K_END_LITERAL
%token SDL_K_MODULE
%token SDL_K_IDENT
%token SDL_K_END_MODULE

%token SDL_K_IFLANG
%token SDL_K_ELSE
%token SDL_K_END_IFLANG
%token SDL_K_IFSYMB
%token SDL_K_ELSE_IFSYMB
%token SDL_K_END_IFSYMB

%token SDL_K_DECLARE
%token SDL_K_SIZEOF

%token SDL_K_AGGREGATE
%token SDL_K_CONSTANT
%token SDL_K_END_CONSTANT
%token SDL_K_ITEM
%token SDL_K_ENTRY

%token SDL_K_BITFIELD
%token SDL_K_LENGTH
%token SDL_K_MASK
%token SDL_K_STRUCTURE
%token SDL_K_UNION
%token SDL_K_BASED
%token SDL_K_TYPEDEF
%token SDL_K_FILL
%token SDL_K_MARKER
%token SDL_K_ORIGIN
%token SDL_K_END

%token SDL_KWD_ALIGN
%token SDL_KWD_NOALIGN
%token SDL_K_BASEALIGN
%token SDL_K_COMMON
%token SDL_K_GLOBAL
%token SDL_K_DIMENSION

%token SDL_K_PREFIX
%token SDL_K_TAG

%token SDL_K_COUNTER
%token SDL_K_EQUALS
%token SDL_K_STRING
%token SDL_K_INCR
%token SDL_K_TYPENAME
%token SDL_K_RADIX
%token SDL_K_DEC
%token SDL_K_HEX
%token SDL_K_OCT

%token SDL_K_REF
%token SDL_K_VALUE
%token SDL_K_IN
%token SDL_K_OUT
%token SDL_K_DEFAULT
%token SDL_K_LIST
%token SDL_K_NAMED
%token SDL_K_OPT
%token SDL_K_RETURNS
%token SDL_K_ALIAS
%token SDL_K_PARAM
%token SDL_K_VARIABLE

%token SDL_K_SIGNED
%token SDL_K_UNSIGNED
%token SDL_K_BYTE
%token SDL_K_WORD
%token SDL_K_LONG
%token SDL_K_QUAD
%token SDL_K_OCTA
%token SDL_K_INT_HW

%token SDL_K_ADDR
%token SDL_K_ADDRL
%token SDL_K_ADDRQ
%token SDL_K_ADDR_HW

%token SDL_K_SFLOAT
%token SDL_K_TFLOAT
%token SDL_K_COMPLEX
%token SDL_K_DECIMAL
%token SDL_K_PRECISION

%token SDL_K_CHAR
%token SDL_K_VARY
%token SDL_K_BOOL
%token SDL_K_ANY

%token SDL_K_OPENP
%token SDL_K_CLOSEP
%token SDL_K_EQ
%token SDL_K_PLUS
%token SDL_K_MINUS
%token SDL_K_MULT
%token SDL_K_DIVIDE
%token SDL_K_AND
%token SDL_K_OR
%token SDL_K_AT
%token SDL_K_NOT
%token SDL_K_DOT
%token SDL_K_FULL
%token SDL_K_CARAT
%token SDL_K_COMMA
%token SDL_K_SEMI

%token <ival> v_int

%token <tval> t_hex
%token <tval> t_octal
%token <tval> t_binary
%token <tval> t_ascii
%token <tval> t_literal_string
%token <tval> t_line_comment
%token <tval> t_block_comment
%token <tval> t_string
%token <tval> t_name
%token <tval> t_constant_name
%token <tval> t_constant_names
%token <tval> t_variable

%type <ival> v_expression
%type <ival> v_number
%type <ival> v_terminal
%type <ival> v_factor
%type <ival> v_bit_op
%type <ival> v_boolean
%type <ival> v_sizeof
%type <ival> v_datatypes
%type <ival> v_usertypes
%type <ival> v_basetypes
%type <ival> v_address
%type <ival> v_object
%type <ival> v_basealign

%type <tval> t_prefix
%type <tval> t_tag

/*
 * We have types on the bison side of the house.
 */

/*
 * Grammar rules
 */
%%
beginning
	: %empty
	| beginning file_layout
	| beginning error
	;

file_layout
	: comments
	| module_format
	;

comments
	: t_line_comment
	| t_block_comment
	;

module_format
	: module
	| module_body
	| end_module
	;

module
	: SDL_K_MODULE t_name SDL_K_SEMI
	| SDL_K_MODULE t_name SDL_K_IDENT t_string SDL_K_SEMI
	;

end_module
	: SDL_K_END_MODULE SDL_K_SEMI
	| SDL_K_END_MODULE t_name SDL_K_SEMI
	;

module_body
	: constant
	| varset
	| literal
	| declare
	;

constant
	: SDL_K_CONSTANT
	| t_constant_name
	| t_constant_names
	| SDL_K_COMMA
	| SDL_K_EQUALS v_expression
	| SDL_K_EQUALS SDL_K_STRING t_string
	| SDL_K_COUNTER t_variable
	| SDL_K_INCR v_expression
	| SDL_K_TYPENAME t_name
	| SDL_K_PREFIX t_name
	| SDL_K_TAG t_name
	| SDL_K_END_CONSTANT
	;

v_expression
	: v_expression SDL_K_PLUS v_terminal
		{ $$ = $1 + $3; }
	| v_expression SDL_K_MINUS v_terminal
		{ $$ = $1 - $3; }
	| v_terminal
	;

v_terminal
	: v_terminal SDL_K_MULT v_boolean
		{ $$ = $1 * $3; }
	| v_terminal SDL_K_DIVIDE v_boolean
		{ $$ = $1 / $3; }
	| v_boolean
	;

v_boolean
	: v_boolean SDL_K_AND v_bit_op
		{ $$ = $1 & $3; }
	| v_boolean SDL_K_OR v_bit_op
		{ $$ = $1 | $3; }
	| v_bit_op
	;
	
v_bit_op
	: v_bit_op SDL_K_AT v_factor
		{ if ($3 >= 0) $$ = $1 >> $3; else $$ = $1 << abs($3); }
	| v_factor
	;

v_factor
	: SDL_K_MINUS v_factor
		{ $$ = -$2; }
	| SDL_K_NOT v_factor
		{ $$ = ~$2; }
	| SDL_K_OPENP v_expression SDL_K_CLOSEP
		{ $$ = $2; }
	| v_number
		{ $$ = $1; }
	| t_string
		{ sdl_str2int(sdl_unquote_str($1), &$$); free($1); }
	;

v_number
	: v_int
		{ $$ = $1; }
	| t_variable
		{ sdl_get_local(&context, $1, &$$); free($1); }
	| t_hex
		{ sscanf($1, "%lx", &$$); free($1); }
	| t_octal
		{ sscanf($1, "%lo", &$$); free($1); }
	| t_binary
		{ $$ = sdl_bin2int($1); free($1); }
	| t_ascii
		{ $$ = (__int64_t) $1[0]; free($1); }
	| SDL_K_DOT
		{ $$ = sdl_offset(&context, SDL_K_OFF_BYTE_REL); }
	| SDL_K_FULL
		{ $$ = sdl_offset(&context, SDL_K_OFF_BYTE_BEG); }
	| SDL_K_CARAT
		{ $$ = sdl_offset(&context, SDL_K_OFF_BIT); }
	;

varset
	: t_variable SDL_K_EQ v_expression SDL_K_SEMI
		{ sdl_set_local(&context, $1, $3); }
	;

literal
	: SDL_K_LITERAL
		{ literalState = true; }
		literal
	| t_literal_string
		{ sdl_literal(&literal, $1); }
		literal
	| SDL_K_END_LITERAL SDL_K_SEMI
		{ sdl_literal_end(&context, &literal); literalState = false; }
	;

declare
	: SDL_K_DECLARE t_name SDL_K_SIZEOF v_sizeof t_prefix t_tag SDL_K_SEMI
		{sdl_declare(&context, $2, $4, $5, $6); }
	;

v_sizeof
	: SDL_K_OPENP v_expression SDL_K_CLOSEP
		{ $$ = $2; }
	| v_datatypes
		{ $$ = $1; }
	;

t_prefix
	: %empty
		{ $$ = NULL; }
	| SDL_K_PREFIX t_name
		{ $$ = $2; }
	;

t_tag
	: %empty
		{ $$ = NULL; }
	| SDL_K_TAG t_name
		{ $$ = $2; }
	;

v_datatypes
	: v_usertypes
		{ $$ = $1; }
	| v_basetypes
		{ $$ = $1; }
	;

v_usertypes
	: t_name
		{ $$ = sdl_usertype_idx(&context, $1); }
	;


v_basetypes
	: SDL_K_BYTE
		{ $$ = SDL_K_TYPE_BYTE; }
	| SDL_K_WORD
		{ $$ = SDL_K_TYPE_WORD; }
	| SDL_K_LONG
		{ $$ = SDL_K_TYPE_LONG; }
	| SDL_K_QUAD
		{ $$ = SDL_K_TYPE_QUAD; }
	| SDL_K_OCTA
		{ $$ = SDL_K_TYPE_OCTA; }
	| SDL_K_SFLOAT
		{ $$ = SDL_K_TYPE_SFLT; }
	| SDL_K_TFLOAT
		{ $$ = SDL_K_TYPE_TFLT; }
	| SDL_K_DECIMAL
		{ $$ = SDL_K_TYPE_DECIMAL; }
	| SDL_K_BITFIELD
		{ $$ = SDL_K_TYPE_BITFLD; }
	| SDL_K_CHAR
		{ $$ = SDL_K_TYPE_CHAR; }
	| v_address
		{ $$ = $1; }
	| SDL_K_ANY
		{ $$ = SDL_K_TYPE_ANY; }
	| SDL_K_BOOL
		{ $$ = SDL_K_TYPE_BOOL; }
	;

v_address
	: SDL_K_ADDR v_object v_basealign
		{ $$ = SDL_K_TYPE_ADDR; }
	| SDL_K_ADDRL v_object v_basealign
		{ $$ = SDL_K_TYPE_ADDRL; }
	| SDL_K_ADDRQ v_object v_basealign
		{ $$ = SDL_K_TYPE_ADDRQ; }
	| SDL_K_ADDR_HW v_object v_basealign
		{ $$ = SDL_K_TYPE_ADDRHW; }
	;

v_object
	: %empty
		{ $$ = 0; }
	| SDL_K_OPENP v_datatypes SDL_K_CLOSEP
		{ $$ = $2; }
	;

v_basealign
	: %empty
		{ $$ = 0; }
	| SDL_K_BASEALIGN SDL_K_OPENP v_expression SDL_K_CLOSEP
		{ $$ = pow(2, $3); }
	| SDL_K_BASEALIGN v_datatypes
		{ $$ = $2; }
	;

%%	/* End Grammar rules */
 