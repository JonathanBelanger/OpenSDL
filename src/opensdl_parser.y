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
 *  V01.000	20-SEP-2018	Jonathan D. Belanger
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

%type <ival> _v_expression
%type <ival> _v_number
%type <ival> _v_terminal
%type <ival> _v_factor
%type <ival> _v_bit_op
%type <ival> _v_boolean
%type <ival> _v_sizeof
%type <ival> _v_datatypes
%type <ival> _v_usertypes
%type <ival> _v_basetypes
%type <ival> _v_address
%type <ival> _v_object
%type <ival> _v_basealign

%type <tval> _t_prefix
%type <tval> _t_tag

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
	    { free($1); }
	| t_block_comment
	    { free($1); }
	;

module_format
	: module
	| module_body
	| end_module
	;

module
	: SDL_K_MODULE t_name SDL_K_SEMI
	    { sdl_module(&context, $2, NULL); }
	| SDL_K_MODULE t_name SDL_K_IDENT t_string SDL_K_SEMI
	    { sdl_module(&context, $2, $4); }
	;

end_module
	: SDL_K_END_MODULE SDL_K_SEMI
	    { sdl_module_end(&context, NULL); }
	| SDL_K_END_MODULE t_name SDL_K_SEMI
	    { sdl_module_end(&context, $2); }
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
	    { free($1); }
	| t_constant_names
	    { free($1); }
	| SDL_K_COMMA
	| SDL_K_EQUALS _v_expression
	| SDL_K_EQUALS SDL_K_STRING t_string
	    { free($3); }
	| SDL_K_COUNTER t_variable
	    { free($2); }
	| SDL_K_INCR _v_expression
	| SDL_K_TYPENAME t_name
	    { free($2); }
	| SDL_K_PREFIX t_name
	    { free($2); }
	| SDL_K_TAG t_name
	    { free($2); }
	| SDL_K_END_CONSTANT
	;

_v_expression
	: _v_expression SDL_K_PLUS _v_terminal
	    { $$ = $1 + $3; }
	| _v_expression SDL_K_MINUS _v_terminal
	    { $$ = $1 - $3; }
	| _v_terminal
	;

_v_terminal
	: _v_terminal SDL_K_MULT _v_boolean
	    { $$ = $1 * $3; }
	| _v_terminal SDL_K_DIVIDE _v_boolean
	    { $$ = $1 / $3; }
	| _v_boolean
	;

_v_boolean
	: _v_boolean SDL_K_AND _v_bit_op
	    { $$ = $1 & $3; }
	| _v_boolean SDL_K_OR _v_bit_op
	    { $$ = $1 | $3; }
	| _v_bit_op
	;
	
_v_bit_op
	: _v_bit_op SDL_K_AT _v_factor
	    { $$ = ($3 >= 0) ? ($1 >> $3) : ($1 << abs($3)); }
	| _v_factor
	;

_v_factor
	: SDL_K_MINUS _v_factor
	    { $$ = -$2; }
	| SDL_K_NOT _v_factor
	    { $$ = ~$2; }
	| SDL_K_OPENP _v_expression SDL_K_CLOSEP
	    { $$ = $2; }
	| _v_number
	    { $$ = $1; }
	| t_string
	    { sdl_str2int(sdl_unquote_str($1), &$$); }
	;

_v_number
	: v_int
	    { $$ = $1; }
	| t_variable
	    { sdl_get_local(&context, $1, &$$); }
	| t_hex
	    { sscanf($1, "%lx", &$$); free($1); }
	| t_octal
	    { sscanf($1, "%lo", &$$); free($1); }
	| t_binary
	    { $$ = sdl_bin2int($1); }
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
	: t_variable SDL_K_EQ _v_expression SDL_K_SEMI
	    { sdl_set_local(&context, $1, $3); }
	;

literal
	: SDL_K_LITERAL
	| t_literal_string
	    { sdl_literal(&literal, $1); }
	| SDL_K_END_LITERAL
	    { sdl_literal_end(&context, &literal); }
	;

declare
	: SDL_K_DECLARE t_name SDL_K_SIZEOF _v_sizeof _t_prefix _t_tag SDL_K_SEMI
	    { sdl_declare(&context, $2, $4, $5, $6); }
	;

_v_sizeof
	: SDL_K_OPENP _v_expression SDL_K_CLOSEP
	    { $$ = $2; }
	| _v_datatypes
	    { $$ = $1; }
	;

_t_prefix
	: %empty
	    { $$ = NULL; }
	| SDL_K_PREFIX t_name
	    { $$ = $2; }
	;

_t_tag
	: %empty
	    { $$ = NULL; }
	| SDL_K_TAG t_name
	    { $$ = $2; }
	;

_v_datatypes
	: _v_usertypes
	    { $$ = $1; }
	| _v_basetypes
	    { $$ = $1; }
	;

_v_usertypes
	: t_name
	    { $$ = sdl_usertype_idx(&context, $1); }
	;


_v_basetypes
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
	| _v_address
	    { $$ = $1; }
	| SDL_K_ANY
	    { $$ = SDL_K_TYPE_ANY; }
	| SDL_K_BOOL
	    { $$ = SDL_K_TYPE_BOOL; }
	;

_v_address
	: SDL_K_ADDR _v_object _v_basealign
	    { $$ = SDL_K_TYPE_ADDR; }
	| SDL_K_ADDRL _v_object _v_basealign
	    { $$ = SDL_K_TYPE_ADDRL; }
	| SDL_K_ADDRQ _v_object _v_basealign
	    { $$ = SDL_K_TYPE_ADDRQ; }
	| SDL_K_ADDR_HW _v_object _v_basealign
	    { $$ = SDL_K_TYPE_ADDRHW; }
	;

_v_object
	: %empty
	    { $$ = 0; }
	| SDL_K_OPENP _v_datatypes SDL_K_CLOSEP
	    { $$ = $2; }
	;

_v_basealign
	: %empty
	    { $$ = 0; }
	| SDL_K_BASEALIGN SDL_K_OPENP _v_expression SDL_K_CLOSEP
	    { $$ = pow(2, $3); }
	| SDL_K_BASEALIGN _v_datatypes
	    { $$ = $2; }
	;

%%	/* End Grammar rules */
