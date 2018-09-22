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
static char			__outBuf[512];
static int			__offset = 0;

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

%token SDL_K_AND
%token SDL_K_OPENP
%token SDL_K_CLOSEP
%token SDL_K_MULT
%token SDL_K_PLUS
%token SDL_K_COMMA
%token SDL_K_MINUS
%token SDL_K_PERIOD
%token SDL_K_DIVIDE
%token SDL_K_COLON
%token SDL_K_SEMI
%token SDL_K_EQ
%token SDL_K_SHIFT
%token SDL_K_BITS
%token SDL_K_OR
%token SDL_K_NOT

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

/*
 * We have types on the bison side of the house.
 */

/*
 * Grammar rules
 */
%%
beginning
	: %empty
	| beginning line
	| beginning error
	;

line
	: module
	| module_body
	| end_module
	;

module
	: SDL_K_MODULE t_name SDL_K_SEMI
		{ printf("\nMODULE %s;\n", $2); free($2); }
	| SDL_K_MODULE t_name SDL_K_IDENT t_string SDL_K_SEMI
		{ printf("\nMODULE %s IDENT \"%s\";\n", $2, $4); free($2); free($4);}
	;

end_module
	: SDL_K_END_MODULE SDL_K_SEMI
		{ printf("\nEND_MODULE;\n"); }
	| SDL_K_END_MODULE t_name SDL_K_SEMI
		{ printf("\nEND_MODULE %s ;\n", $2); free($2); }
	;

module_body
	: constant
	;

constant
	: SDL_K_CONSTANT
		{ __offset = sprintf(__outBuf, "**** CONSTANT "); }
	| t_constant_name
		{ __offset += sprintf(&__outBuf[__offset], "%s", $1); free($1); }
	| t_constant_names
		{ __offset += sprintf(&__outBuf[__offset], "(%s)", $1); free($1); }
	| SDL_K_COMMA
		{ printf("\n%s;\n\n", __outBuf); __offset = sprintf(__outBuf, "**** CONSTANT "); }
	| SDL_K_EQUALS v_expression
		{ __offset += sprintf(&__outBuf[__offset], " EQUALS %ld", $2); printf("\n????????  Here 2 ????????\n");}
	| SDL_K_EQUALS SDL_K_STRING t_string
		{ __offset += sprintf(&__outBuf[__offset], " STRING \"%s\"", $3); free($3); }
	| SDL_K_COUNTER t_variable
		{ __offset += sprintf(&__outBuf[__offset], " COUNTER %s", $2); free($2); }
	| SDL_K_INCR v_expression
		{ __offset += sprintf(&__outBuf[__offset], " INCREMENT %ld", $2); }
	| SDL_K_TYPENAME t_name
		{ __offset += sprintf(&__outBuf[__offset], " TYPENAME %s", $2); free($2); }
	| SDL_K_PREFIX t_name
		{ __offset += sprintf(&__outBuf[__offset], " PREFIX %s", $2); free($2); }
	| SDL_K_TAG t_name
		{ __offset += sprintf(&__outBuf[__offset], " TAG %s", $2); free($2); }
	| SDL_K_END_CONSTANT
		{ printf("\n%s;\n\n", __outBuf); }
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
	: v_bit_op SDL_K_SHIFT v_factor
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
	| SDL_K_PERIOD
		{ $$ = sdl_offset(&context, SDL_K_OFF_BYTE_REL); }
	| SDL_K_COLON
		{ $$ = sdl_offset(&context, SDL_K_OFF_BYTE_BEG); }
	| SDL_K_BITS
		{ $$ = sdl_offset(&context, SDL_K_OFF_BIT); }
	;

%%	/* End Grammar rules */
 