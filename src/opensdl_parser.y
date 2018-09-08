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
 *  V01.000	Aug 23, 2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	Sep  6, 2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
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

#ifdef YYDEBUG
#undef YYDEBUG
#endif
#define	YYDEBUG			1
#define	YYERROR_VERBOSE	1
#define	YYLSP_NEEDED	1

SDL_CONTEXT			context;
SDL_QUEUE			literal;		/* A list of lines in the between LITERAL/END_LITERAL */

_Bool				literalState = false;

void yyerror(YYLTYPE *locp, yyscan_t *scanner, char const *msg);
%}

/*
 * Declarations
 */
%union
{
	__int64_t	bval;
	char		tval[512];
}

%start here

/* OpenSDL Directive Keywords */
%token SDL_K_LITERAL
%token SDL_K_END_LITERAL
%token SDL_K_DECLARE

/* OpenSDL Declaration Keywords */
%token SDL_K_AGGREGATE
%token SDL_K_CONSTANT
%token SDL_K_END
%token SDL_K_END_MODULE
%token SDL_K_ENTRY
%token SDL_K_ITEM
%token SDL_K_MODULE
%token SDL_K_STRUCTURE
%token SDL_K_UNION

/* OpenSDL Declaration Modifier Keywords */
/* MODULE Declaration */
%token SDL_K_IDENT

/* ITEM Declaration */
%token SDL_KWD_ALIGN
%token SDL_KWD_NOALIGN
%token SDL_K_BASEALIGN
%token SDL_K_COMMON
%token SDL_K_GLOBAL
%token SDL_K_DIMENSION
%token SDL_K_PREFIX
%token SDL_K_TAG
%token SDL_K_SIZEOF

/* AGGREGATE Declaration */
%token SDL_K_BASED
%token SDL_K_TYPEDEF
%token SDL_K_FILL
%token SDL_K_MARKER
%token SDL_K_ORIGIN

/* CONSTANT Declaration */
%token SDL_K_COUNTER
%token SDL_K_EQUALS
%token SDL_K_STRING
%token SDL_K_INCR
%token SDL_K_TYPENAME

/* ENTRY Declaration */
%token SDL_K_DSC
%token SDL_K_REF
%token SDL_K_STR_DSC
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

/* Data Type Keywords */
%token SDL_K_BYTE
%token SDL_K_WORD
%token SDL_K_LONG
%token SDL_K_QUAD
%token SDL_K_OCTA
%token SDL_K_SFLOAT
%token SDL_K_TFLOAT
%token SDL_K_DECIMAL
%token SDL_K_BITFIELD
%token SDL_K_CHAR
%token SDL_K_ADDR
%token SDL_K_ADDRL
%token SDL_K_ADDRQ
%token SDL_K_ADDR_HW
%token SDL_K_ANY
%token SDL_K_BOOL

/* Datatype Modifier Keywords */
%token SDL_K_SIGNED
%token SDL_K_UNSIGNED
%token SDL_K_COMPLEX
%token SDL_K_LENGTH
%token SDL_K_MASK
%token SDL_K_VARY
%token SDL_K_PRECISION

/* Operators */
%token SDL_K_PLUS
%token SDL_K_MINUS
%token SDL_K_MULT
%token SDL_K_DIVIDE
%token SDL_K_SHIFT
%token SDL_K_AND
%token SDL_K_OR
%token SDL_K_NOT
%token SDL_K_PERIOD
%token SDL_K_BITS
%token SDL_K_EQ

/* OpenSDL Delimiters */
%token SDL_K_EOD
%token SDL_K_OPEN
%token SDL_K_CLOSE
%token SDL_K_COMMA
%token SDL_K_COLON

/* Added Output Directives */
%token SDL_K_RADIX
%token SDL_K_DEC
%token SDL_K_HEX
%token SDL_K_OCT

%token <tval> ident
%token <tval> lit_val		/* Literal string					*/
%token <bval> decimal		/* Decimal digit					*/
%token <tval> hexadecimal	/* A digit in the hex notation		*/
%token <tval> octal			/* A digit in the octal notation	*/
%token <tval> binary		/* A digit in the binary notation	*/
%token <tval> ascii			/* ASCII character					*/
%token <tval> comment		/* A comment to be written out		*/
%token <tval> string		/* A string within double quotes	*/
%token <tval> variable		/* A local OpenSDL's variable		*/

%type <bval> expression
%type <bval> number
%type <bval> terminal
%type <bval> factor
%type <bval> bit_op
%type <bval> boolean
%type <bval> sizeof
%type <bval> datatypes
%type <bval> usertypes
%type <bval> basetypes
%type <bval> objecttype
%type <bval> address
%type <bval> basealign

%type <tval> tag
%type <tval> prefix
/*
%type <bval> inc
%type <bval> lbound
%type <bval> hbound
%type <bval> syntypes
%type <bval> signspec
%type <bval> align
%type <bval> radix
*/

/*
 * Grammar rules
 */
%%
here
	: %empty
	| here line
	| here error
	;

line
	: comment
		{ sdl_comment(&context, $1); }
	| module
	| varset
	| literal
	| declare
/*	| item
	| declarations
*/	;

module
	: SDL_K_MODULE ident SDL_K_EOD
		{ sdl_module(&context, $2, NULL); }
	| SDL_K_MODULE ident SDL_K_IDENT string SDL_K_EOD
		{ sdl_module(&context, $2, sdl_unquote_str($4)); }
	| SDL_K_END_MODULE ident SDL_K_EOD
		{ sdl_module_end(&context, $2); }
	| SDL_K_END_MODULE SDL_K_EOD
		{ sdl_module_end(&context, NULL); }
	;

literal
	: SDL_K_LITERAL
		{ literalState = true; }
		literal
	| lit_val
		{ sdl_literal(&literal, $1); }
		literal
	| SDL_K_END_LITERAL SDL_K_EOD
		{ sdl_literal_end(&context, &literal); literalState = false; }
	;

declare
	: SDL_K_DECLARE ident SDL_K_SIZEOF sizeof prefix tag SDL_K_EOD
		{sdl_declare(&context, $2, $4, $5, $6); }
	;

sizeof
	: SDL_K_OPEN expression SDL_K_CLOSE
		{ $$ = $2; }
	| datatypes
		{ $$ = sdl_sizeof(&context, $1); }
	;

prefix
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_PREFIX ident
		{ strncpy($$, $2, SDL_K_NAME_MAX_LEN); }
	;

tag
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_TAG ident
		{ strncpy($$, $2, SDL_K_NAME_MAX_LEN); }
	;

varset
	: variable SDL_K_EQ expression SDL_K_EOD
		{ sdl_set_local(&context, $1, $3); }
	;

expression
	: expression SDL_K_PLUS terminal
		{ $$ = $1 + $3; }
	| expression SDL_K_MINUS terminal
		{ $$ = $1 - $3; }
	| terminal
	;

terminal
	: terminal SDL_K_MULT boolean
		{ $$ = $1 * $3; }
	| terminal SDL_K_DIVIDE boolean
		{ $$ = $1 / $3; }
	| boolean
	;

boolean
	: boolean SDL_K_AND bit_op
		{ $$ = $1 & $3; }
	| boolean SDL_K_OR bit_op
		{ $$ = $1 | $3; }
	| bit_op
	;
	
bit_op
	: bit_op SDL_K_SHIFT factor
		{ if ($3 >= 0) $$ = $1 >> $3; else $$ = $1 << abs($3); }
	| factor
	;

factor
	: SDL_K_MINUS factor
		{ $$ = -$2; }
	| SDL_K_NOT factor
		{ $$ = sdl_not($2); }
	| SDL_K_OPEN expression SDL_K_CLOSE
		{ $$ = $2; }
	| number
		{ $$ = $1; }
	| string
		{ sdl_str2int(sdl_unquote_str($1), &$$); }
	;

number
	: decimal
		{ $$ = $1; }
	| variable
		{ sdl_get_local(&context, $1, &$$); }
	| hexadecimal
		{ sscanf($1, "%lx", &$$); }
	| octal
		{ sscanf($1, "%lo", &$$); }
	| binary
		{ $$ = sdl_bin2int($1); }
	| ascii
		{ $$ = (__int64_t) $1[0]; }
	| SDL_K_PERIOD
		{ $$ = sdl_offset(context, SDL_K_OFF_BYTE_REL); }
	| SDL_K_COLON
		{ $$ = sdl_offset(context, SDL_K_OFF_BYTE_BEG); }
	| SDL_K_BITS
		{ $$ = sdl_offset(context, SDL_K_OFF_BIT); }
	;
/*
item
	: SDL_K_ITEM ident datatypes storage basealign dimension prefix tag SDL_K_EOD
		{ sdl_item(&context, $2, $3, $4, $5, $6, $7, $8}; }
	;
*/
datatypes
	: usertypes
		{ $$ = $1; }
	| basetypes
		{ $$ = $1; }
	;

usertypes
	: ident
		{ $$ = sdl_usertype_idx(&context, $1); }
	;

basetypes
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
	| address
		{ $$ = $1; }
	| SDL_K_ANY
		{ $$ = SDL_K_TYPE_ANY; }
	| SDL_K_BOOL
		{ $$ = SDL_K_TYPE_BOOL; }
	;

address
	: SDL_K_ADDR objecttype basealign
		{ $$ = SDL_K_TYPE_ADDR; }
	| SDL_K_ADDRL objecttype basealign
		{ $$ = SDL_K_TYPE_ADDRL; }
	| SDL_K_ADDRQ objecttype basealign
		{ $$ = SDL_K_TYPE_ADDRQ; }
	| SDL_K_ADDR_HW objecttype basealign
		{ $$ = SDL_K_TYPE_ADDRHW; }
	;

objecttype
	: %empty
		{ $$ = 0; }
	| SDL_K_OPEN datatypes SDL_K_CLOSE
		{ $$ = $2; }
	;
/*
storage
	: %empty
		{ $$ = SDL_M_STOR_NONE; }
	| SDL_K_COMMON
		{ $$ = SDL_M_STOR_COMM; }
	| SDL_K_GLOBAL
		{ $$ = SDL_M_STOR_GLOB; }
	| SDL_K_COMMON SDL_K_TYPEDEF
		{ $$ = SDL_M_STOR_COMM | SDL_M_STOR_TYPED; }
	| SDL_K_GLOBAL SDL_K_TYPEDEF
		{ $$ = SDL_M_STOR_GLOB | SDL_M_STOR_TYPED; }
	| SDL_K_TYPEDEF
		{ $$ = SDL_M_STOR_TYPED; }
	;

dimension
	: %empty
		{ $$ = -1; }
	| SDL_K_DIMENSION hbound
		{ $$ = sdl_dimension(1, $2); }
	| SDL_K_DIMENSION lbound SDL_K_COLON hbound
		{ $$ = sdl_dimension($2, $4); }
	;

declarations
	: aggregate
	| constant
	| entry
	| item
	| struct
	| union
	;

aggregate
	: SDL_K_AGGREGATE ident aggregate_type datatypes storage based alignment dimension marker prefix tag origin fill SDL_K_EOD
		{ sdl_aggregate(&context, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13); }
	| aggregate_body
	| SDL_K_END SDL_K_EOD
		{ sdl_aggregate_end(&context); }
	;

aggregate_type
	: SDL_K_STRUCT
		{ $$ = SDL_K_TYPE_STRUCT; };
	| SDL_K_UNION
		{ $$ = SDL_K_TYPE_UNION; };
	| error
	;

based
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_BASED ident
		{ $$ = $2; }
	;

alignment
	: SDL_KWD_ALIGN
		{ $$ = -1; }
	| SDL_K_NOALIGN
		{ $$ = 0; }
	| basealign
		{ $$ = $1; }
	;
*/
basealign
	: %empty
		{ $$ = 0; }
	| SDL_K_BASEALIGN SDL_K_OPEN expression SDL_K_CLOSE
		{ $$ = pow(2, $3); }
	| SDL_K_BASEALIGN datatypes
		{ $$ = sdl_sizeof(&context, $2); }
	;
/*
marker
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_MARKER ident
		{ $$ = $2; }
	;

origin
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_ORIGIN member_name
		{ $$ = $2; }
	;

fill
	: %empty
		{ $$ = false; }
	| SDL_K_FILL
		{ $$ = true; }
	;

member_name
	: datatype alignment dimension prefix tag fill SDL_K_EOD
	| aggregate_name alignment dimension prefix tag fill SDL_K_EOD
	| usertype alignment dimension prefix tag fill SDL_K_EOD
	;

aggretage_body
	: %empty
	| ident datatypes storage dimension prefix tag SDL_K_EOD
	| subaggregate
	| 

*
 * TODO: How to do implicit UNIONS?
 *
subaggregate
	: ident SDL_K_STRUCT agg_opt SDL_K_EOD
	| ident SDL_K_UNION agg_opt SDL_K_EOD
	;

agg_opt
	: %empty
	: datatypes alignment dimension marker prefix tag fill SDL_K_EOD
	;

constant
	: SDL_K_CONSTANT ident SDL_K_EQUALS value prefix tag counter typename SDL_K_EOD
	| SDL_K_CONSTANT ident SDL_K_EQUALS SDL_K_STRING ident prefix tag SDL_K_EOD
	| SDL_K_CONSTANT const_list SDL_K_EOD
	| SDL_K_CONSTANT SDL_K_OPEN list SDL_K_CLOSE SDL_K_EQUALS increment prefix tag counter typename SDL_K_EOD
	;

entry
	: SDL_K_ENTRY ident alias parameter linkage variable returns typename SDL_K_EOD
	;

alias
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| ALIAS ident
		{ $$ = $2; }
	;

parameter
	: %empty
	| SDL_K_PARAM SDL_K_OPEN param_desc SDL_K_CLOSE
	;

linkage
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_LINKAGE ident
		{ $$ = $2; }
	;

variable
	: %empty
		{ $$ = false; }
	| SDL_K_VARIABLE
		{ $$ = true; }
	;

returns
	: %empty
	| SDL_K_RETURNS datatype named
	;

typename
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_TYPENAME ident
		{ $$ = $2; }
	;

*
 * TODO: Need to make this into a list.
 *
param_desc
	: datatype pass_mech param_opt
	| aggtype pass_mech param_opt
	;

named
	: %empty
		{ $$[0] = SDL_K_NOT_PRESENT; }
	| SDL_K_NAMED ident
		{ $$ = $2; }
	;

param_opt
	: %empty
	| SDL_K_IN
	| SDL_K_OUT
	| named
	| SDL_K_DIMENSION value
	| SDL_K_DEFAULT value
	| typename
	| SDL_K_OPTIONAL
	| SDL_K_LIST
	;
*/
%%	/* End Grammar rules */
