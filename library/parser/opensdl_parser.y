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
 *  V01.000    20-SEP-2018    Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001    04-OCT-2018    Jonathan D. Belanger
 *  This file has been updated quite often.  Most of the changes have been made
 *  to allow for parsing additional grammar constructs.  Also, there have been
 *  some changes coordinated with the flex file's use of start states.
 *
 *  V01.002    22-OCT-2018    Jonathan D. Belanger
 *  Added IF_LANGUAGE and IF_SYMBOL parsing.  Unlike the original SDL, we will
 *  allow conditionals to be properly nested.
 *
 *  V01.003    10-NOV-2018    Jonathan D. Belanger
 *  Added a macro to keep track of the line numbers so that they can be
 *  reported in error messages.
 */
%verbose
%define parse.lac    full
%define parse.error    verbose
%define api.pure    true

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
#include "library/common/opensdl_blocks.h"
#include "library/common/opensdl_message.h"
#include "library/utility/opensdl_actions.h"
#include "library/utility/opensdl_listing.h"
#include "opensdl/opensdl_main.h"

extern bool     listing;
extern FILE     *listingFP;

SDL_CONTEXT    context;
SDL_QUEUE    literal;

#define YYLLOC_DEFAULT(_cur, _rhs, _n)                \
do                                \
{                                \
    if (_n > 0)                            \
    {                                \
    (_cur).first_line   = YYRHSLOC(_rhs, 1).first_line;    \
    (_cur).first_column = YYRHSLOC(_rhs, 1).first_column;    \
    (_cur).last_line    = YYRHSLOC(_rhs, _n).last_line;    \
    (_cur).last_column  = YYRHSLOC(_rhs, _n).last_column;    \
    }                                \
    else                            \
    {                                \
    (_cur).first_line   = (_cur).last_line   =        \
        YYRHSLOC(_rhs, 0).last_line;            \
    (_cur).first_column = (_cur).last_column =        \
        YYRHSLOC(_rhs, 0).last_column;            \
    }                                \
} while (false)

void yyerror(YYLTYPE *locp, yyscan_t *scanner, char const *msg);
static char *bugchk = "%%SDL-F-BUGCHECK, Internal consistency failure "
            "[Line %d] - please submit a bug report\n";
static char *errexit = "-SDL-F-ERREXIT, Error exit\n";
                                    \

#define SDL_CALL(funct, loc)                    \
do                                \
{                                \
    uint32_t    status = (funct);                \
                                \
    if (SDL_GET_SEVERITY(status) != SDL_K_SUCCESS)        \
    {                                \
    char     *_msgTxt;                    \
                                \
    if ((status != SDL_ERREXIT) &&                \
        (sdl_get_message(msgVec, &_msgTxt) == SDL_NORMAL))    \
    {                            \
        if (listing == true)                \
        sdl_write_err(listingFP, _msgTxt);    \
        fprintf(stderr, "%s", _msgTxt);            \
        sdl_free(_msgTxt);                    \
    }                            \
    else                            \
    {                            \
        fprintf(stderr, bugchk, (loc).first_line);        \
        if (status == SDL_ERREXIT)                \
        fprintf(stderr, errexit);            \
        exit(0);                        \
    }                            \
    }                                \
} while (false)

%}

/*
 * Declarations
 */
%union
{
    __int64_t    ival;
    char        *tval;
    void        *aval;
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
%token SDL_K_ENUM
%token SDL_K_TYPENAME
%token SDL_K_RADIX
%token SDL_K_DEC
%token SDL_K_HEX
%token SDL_K_OCT

%token SDL_K_REF
%token SDL_K_VALUE
%token SDL_K_DESC
%token SDL_K_STR_DESC
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
%token SDL_K_LINKAGE

%token SDL_K_SIGNED
%token SDL_K_UNSIGNED
%token SDL_K_BYTE
%token SDL_K_INT_B
%token SDL_K_WORD
%token SDL_K_INT_W
%token SDL_K_LONG
%token SDL_K_INT_L
%token SDL_K_INT
%token SDL_K_INT_HW
%token SDL_K_HW_INT
%token SDL_K_QUAD
%token SDL_K_INT_Q
%token SDL_K_OCTA
%token SDL_K_VOID

%token SDL_K_ADDR
%token SDL_K_ADDR_L
%token SDL_K_ADDR_Q
%token SDL_K_ADDR_HW
%token SDL_K_HW_ADDR
%token SDL_K_PTR
%token SDL_K_PTR_L
%token SDL_K_PTR_Q
%token SDL_K_PTR_HW

%token SDL_K_SFLOAT
%token SDL_K_TFLOAT
%token SDL_K_FFLOAT
%token SDL_K_DFLOAT
%token SDL_K_GFLOAT
%token SDL_K_HFLOAT
%token SDL_K_XFLOAT
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
%token <tval> t_aggr_str
%token <tval> t_aggr_name

%type <ival> _v_expression
%type <ival> _v_number
%type <ival> _v_terminal
%type <ival> _v_factor
%type <ival> _v_bit_op
%type <ival> _v_boolean
%type <ival> _v_sizeof
%type <ival> _v_datatypes
%type <ival> _v_char_opt
%type <ival> _v_usertypes
%type <ival> _v_basetypes
%type <ival> _v_integer
%type <ival> _v_float
%type <ival> _v_signed
%type <ival> _v_address
%type <ival> _v_object
%type <ival> _v_aggtypes
%type <ival> _v_passing_option

%type <tval> _t_id
%type <tval> _t_module_name
%type <tval> _t_aggr_id

%type <aval> _a_returns_options
%type <aval> _a_named

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
        {
        SDL_CALL(
            sdl_comment_line(&context, $1, (SDL_YYLTYPE *) &@1),
            @$);
        }
    | t_block_comment
        {
        SDL_CALL(
            sdl_comment_block(&context, $1, (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

module_format
    : module
    | module_body
    | end_module
    ;

_t_id
    : t_name
        { $$ = $1; }
    | t_string
        { $$ = $1; }

module
    : SDL_K_MODULE t_name
        { 
        SDL_CALL(
            sdl_state_transition(
            &context,
            Module,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_module(&context, $2, NULL, (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_MODULE t_name SDL_K_IDENT t_string
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Module,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_module(&context, $2, $4, (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

_t_module_name
    : %empty
        { $$ = NULL; }
    | t_name
        { $$ = $1; }
    ;

end_module
    : SDL_K_END_MODULE _t_module_name
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            DefinitionEnd,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_module_end(
            &context,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

module_body
    : varset
    | literal
    | constant
    | declare
    | item
    | aggregate
    | prefix
    | marker
    | tag
    | counter
    | _typename
    | increment
    | enumerate
    | radix
    | dimension
    | storage
    | _typedef
    | origin
    | entry
    | definition_end
    | conditionals
    ;

definition_end
    : SDL_K_SEMI
        {
        switch (context.state)
        {
            case Local:
            case Declare:
            case Constant:
            case Item:
            case Entry:
            SDL_CALL(
                sdl_state_transition(
                &context,
                DefinitionEnd,
                (SDL_YYLTYPE *) &@1),
                @$);
            break;

            default:
            break;
        }
        }
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
        {
        if ($3 == 0)
        {
            if (sdl_set_message(
            msgVec,
            1,
            SDL_ZERODIV,
            @3.first_line) == SDL_NORMAL)
            {
            char *msgText;

            if (sdl_get_message(msgVec, &msgText) == SDL_NORMAL)
            {
                fprintf(stderr, "%s\n", msgText);
                sdl_free(msgText);
            }
            else
            {
                fprintf(
                stderr,
                "%%SDL-F-BUGCHECK, Internal consistency "
                    "failure [Line %d] - please submit a bug "
                    "report\n",
                    @3.first_line);
                exit(0);
            }
            }
            $$ = 0;
        }
        else
            $$ = $1 / $3;
        }
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
        { SDL_CALL(sdl_str2int(sdl_unquote_str($1), &$$), @$); }
    ;

_v_number
    : v_int
        { $$ = $1; }
    | t_variable
        { SDL_CALL(sdl_get_local(&context, $1, &$$), @$); }
    | t_hex
        { sscanf($1, "%lx", &$$); sdl_free($1); }
    | t_octal
        { sscanf($1, "%lo", &$$); sdl_free($1); }
    | t_binary
        { $$ = sdl_bin2int($1); }
    | t_ascii
        { $$ = (__int64_t) $1[0]; sdl_free($1); }
    | SDL_K_DOT
        {
        $$ = sdl_offset(
            &context,
            SDL_K_OFF_BYTE_REL,
            (SDL_YYLTYPE *) &@1);
        }
    | SDL_K_FULL
        {
        $$ = sdl_offset(
            &context,
            SDL_K_OFF_BYTE_BEG,
            (SDL_YYLTYPE *) &@1);
        }
    | SDL_K_CARAT
        { $$ = sdl_offset(&context, SDL_K_OFF_BIT, (SDL_YYLTYPE *) &@1); }
    ;

varset
    : t_variable SDL_K_EQ _v_expression
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Local,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_set_local(&context, $1, $3, (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

literal
    : SDL_K_LITERAL
    | t_literal_string
        {
        SDL_CALL(
            sdl_literal(&context, &literal, $1, (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_END_LITERAL
        {
        SDL_CALL(
            sdl_literal_end(&context, &literal, (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

declare
    : SDL_K_DECLARE _t_id SDL_K_SIZEOF _v_sizeof
        { 
        SDL_CALL(
            sdl_state_transition(
            &context,
            Declare,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_declare(&context, $2, $4, (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

_v_sizeof
    : SDL_K_OPENP _v_expression SDL_K_CLOSEP
        { $$ = $2 * SDL_K_SIZEOF_MIN; }
    | _v_datatypes
        { $$ = $1; }
    ;

prefix
    : SDL_K_PREFIX _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Prefix,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_PREFIX _t_aggr_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Prefix,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

marker
    : SDL_K_MARKER _t_aggr_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Marker,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

tag
    : SDL_K_TAG _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Tag,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_TAG _t_aggr_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Tag,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

origin
    : SDL_K_ORIGIN _t_aggr_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Origin,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

counter
    : SDL_K_COUNTER t_variable
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Counter,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

_typename
    : SDL_K_TYPENAME _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            TypeName,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

radix
    : SDL_K_RADIX v_int
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Radix,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

increment
    : SDL_K_INCR _v_expression
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Increment,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

enumerate
    : SDL_K_ENUM _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Enumerate,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

constant
    : SDL_K_CONSTANT _constant_body
    ;

_constant_body
    : _complex_clause
    | _clause_list
    ;

_clause_list
    : _clause SDL_K_COMMA _clause
    | _clause_list SDL_K_COMMA _clause
    ;

_clause
    : t_constant_name SDL_K_EQUALS _v_expression
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Constant,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_constant(
            &context,
            $1,
            $3,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | t_constant_names SDL_K_EQUALS _v_expression
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Constant,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_constant(
            &context,
            $1,
            $3,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

_complex_clause
    : t_constant_name SDL_K_EQUALS SDL_K_STRING t_string
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Constant,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_constant(
            &context,
            $1,
            0,
            $4,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | t_constant_name SDL_K_EQUALS _v_expression
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Constant,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_constant(
            &context,
            $1,
            $3,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | t_constant_names SDL_K_EQUALS _v_expression
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Constant,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_constant(
            &context,
            $1,
            $3,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

_v_datatypes
    : _v_usertypes
        { $$ = $1; }
    | _v_basetypes
        { $$ = $1; }
    ;

_v_usertypes
    : _t_id
        {
        $$ = sdl_usertype_idx(&context, $1);
        if ($$ == 0)
            $$ = sdl_aggrtype_idx(&context, $1);
        }
    ;

_v_basetypes
    : _v_integer
       { $$ = $1; }
    | _v_float
       { $$ = $1; }
    | SDL_K_CHAR
        { $$ = SDL_K_TYPE_CHAR; }
    | SDL_K_CHAR SDL_K_LENGTH _v_expression _v_char_opt
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Length,
            $3,
            NULL,
            (SDL_YYLTYPE *) &@3),
            @$);
        $$ = $4;
        }
    | SDL_K_CHAR SDL_K_LENGTH SDL_K_MULT
        { $$ = SDL_K_TYPE_CHAR_STAR; }
    | _v_address
        { $$ = $1; }
    | SDL_K_ANY
        { $$ = SDL_K_TYPE_ANY; }
    | SDL_K_BOOL
        { $$ = SDL_K_TYPE_BOOL; }
    ;

_v_char_opt
    : %empty
        { $$ = SDL_K_TYPE_CHAR; }
    | SDL_K_VARY
        { $$ = SDL_K_TYPE_CHAR_VARY; }
    ;

_v_integer
    : SDL_K_BYTE _v_signed
        { $$ = SDL_K_TYPE_BYTE * $2; }
    | SDL_K_INT_B _v_signed
        { $$ = SDL_K_TYPE_INT_B * $2; }
    | SDL_K_WORD _v_signed
        { $$ = SDL_K_TYPE_WORD * $2; }
    | SDL_K_INT_W _v_signed
        { $$ = SDL_K_TYPE_INT_W * $2; }
    | SDL_K_LONG _v_signed
        { $$ = SDL_K_TYPE_LONG * $2; }
    | SDL_K_INT _v_signed
        { $$ = SDL_K_TYPE_INT * $2; }
    | SDL_K_INT_L _v_signed
        { $$ = SDL_K_TYPE_INT_B * $2; }
    | SDL_K_INT_HW _v_signed
        { $$ = SDL_K_TYPE_INT_HW * $2; }
    | SDL_K_HW_INT _v_signed
        { $$ = SDL_K_TYPE_HW_INT * $2; }
    | SDL_K_QUAD _v_signed
        { $$ = SDL_K_TYPE_QUAD * $2; }
    | SDL_K_INT_Q _v_signed
        { $$ = SDL_K_TYPE_INT_Q * $2; }
    | SDL_K_OCTA _v_signed
        { $$ = SDL_K_TYPE_OCTA * $2; }
    ;

_v_float
    : SDL_K_TFLOAT
        { $$ = SDL_K_TYPE_TFLT; }
    | SDL_K_TFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_TFLT_C; }
    | SDL_K_SFLOAT
        { $$ = SDL_K_TYPE_SFLT; }
    | SDL_K_SFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_SFLT_C; }
    | SDL_K_XFLOAT
        { $$ = SDL_K_TYPE_XFLT; }
    | SDL_K_XFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_XFLT_C; }
    | SDL_K_FFLOAT
        { $$ = SDL_K_TYPE_FFLT; }
    | SDL_K_FFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_FFLT_C; }
    | SDL_K_DFLOAT
        { $$ = SDL_K_TYPE_DFLT; }
    | SDL_K_DFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_DFLT_C; }
    | SDL_K_GFLOAT
        { $$ = SDL_K_TYPE_GFLT; }
    | SDL_K_GFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_GFLT_C; }
    | SDL_K_HFLOAT
        { $$ = SDL_K_TYPE_HFLT; }
    | SDL_K_HFLOAT SDL_K_COMPLEX
        { $$ = SDL_K_TYPE_HFLT_C; }
    | SDL_K_DECIMAL SDL_K_PRECISION SDL_K_OPENP _v_expression SDL_K_COMMA
      _v_expression SDL_K_CLOSEP
        {
        SDL_CALL(sdl_precision(&context, $4, $6), @$);
        $$ = SDL_K_TYPE_DECIMAL;
        }
    ;

_v_signed
    : %empty
        { $$ = -1; }
    | SDL_K_SIGNED
        { $$ = -1; }
    | SDL_K_UNSIGNED
        { $$ = 1; }
    ;

_v_address
    : SDL_K_ADDR _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_ADDR;
        }
    | SDL_K_ADDR_L _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_ADDR_L;
        }
    | SDL_K_ADDR_Q _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_ADDR_Q;
        }
    | SDL_K_ADDR_HW _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_ADDR_HW;
        }
    | SDL_K_HW_ADDR _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_HW_ADDR;
        }
    | SDL_K_PTR _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_PTR;
        }
    | SDL_K_PTR_L _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_PTR_L;
        }
    | SDL_K_PTR_Q _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_PTR_Q;
        }
    | SDL_K_PTR_HW _v_object
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        $$ = -SDL_K_TYPE_PTR_HW;
        }
    ;

_v_object
    : %empty
        { $$ = 0; }
    | _v_datatypes
        { $$ = $1; }
    | SDL_K_OPENP SDL_K_ENTRY _basealign SDL_K_CLOSEP
        { $$ = SDL_K_TYPE_ENTRY; }
    | SDL_K_OPENP _v_datatypes _basealign SDL_K_CLOSEP
        { $$ = $2; }
    ;

_basealign
    : %empty
    | basealign
    ;

basealign
    : SDL_K_BASEALIGN SDL_K_OPENP _v_expression SDL_K_CLOSEP
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            BaseAlign,
            pow(2, $3),
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_BASEALIGN _v_datatypes
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            BaseAlign,
            sdl_sizeof(&context, $2),
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

item
    : SDL_K_ITEM _t_id _v_datatypes
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Item,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(sdl_item(&context, $2, $3, (SDL_YYLTYPE *) &@2), @$);
        }
    ;

_typedef
    : SDL_K_TYPEDEF
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Typedef,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

storage
    : SDL_K_COMMON
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Common,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_GLOBAL
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Global,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_BASED _t_aggr_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Based,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_FILL
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Fill,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_KWD_ALIGN
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Align,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_KWD_NOALIGN
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            NoAlign,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | basealign
    ;

dimension
    : SDL_K_DIMENSION _v_expression
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Dimension,
            sdl_dimension(&context, 1, $2),
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_DIMENSION _v_expression SDL_K_FULL _v_expression
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Dimension,
            sdl_dimension(&context, $2, $4),
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

_v_aggtypes
    : %empty
        { $$ = 0; }
    | _v_datatypes
        { $$ = $1; }
    ;

aggregate
    : SDL_K_AGGREGATE _t_id SDL_K_STRUCTURE _v_aggtypes
       {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Aggregate,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate(
            &context,
            $2,
            $4,
            SDL_K_TYPE_STRUCT,
            (SDL_YYLTYPE *) &@2),
            @$);
       }
    | SDL_K_AGGREGATE _t_id SDL_K_UNION _v_aggtypes
       {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Aggregate,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate(
            &context,
            $2,
            $4,
            SDL_K_TYPE_UNION,
            (SDL_YYLTYPE *) &@2),
            @$);
       }
    | aggregate_body
    | SDL_K_END _t_aggr_id SDL_K_SEMI
       {
        SDL_CALL(
            sdl_state_transition(
            &context,
            DefinitionEnd,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate_compl(&context, $2, (SDL_YYLTYPE *) &@2),
            @$);
       }
    | SDL_K_END SDL_K_SEMI
       {
        SDL_CALL(
            sdl_state_transition(
            &context,
            DefinitionEnd,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate_compl(&context, NULL, (SDL_YYLTYPE *) &@1),
            @$);
       }
    | SDL_K_END _t_id SDL_K_SEMI
       {
        SDL_CALL(
            sdl_state_transition(
            &context,
            DefinitionEnd,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate_compl(&context, $2, (SDL_YYLTYPE *) &@2),
            @$);
       }
    ;

_t_aggr_id
    : t_aggr_str
        { $$ = $1; }
    | t_aggr_name
        { $$ = $1; }
    ;

aggregate_body
    : _t_aggr_id _v_datatypes
        {
        SDL_CALL(
            sdl_aggregate_member(
            &context,
            $1,
            $2,
            SDL_K_TYPE_NONE,
            (SDL_YYLTYPE *) &@1,
            false,
            false,
            false,
            false),
            @$);
        }
    | _t_aggr_id _t_aggr_id
        {
        SDL_CALL(
            sdl_aggregate_member(
            &context,
            $1,
            sdl_aggrtype_idx(&context, $2),
            SDL_K_TYPE_NONE,
            (SDL_YYLTYPE *) &@1,
            false,
            false,
            false,
            false),
            @$);
        }
    | _t_aggr_id SDL_K_STRUCTURE _v_aggtypes
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Subaggregate,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate_member(
            &context,
            $1,
            $3,
            SDL_K_TYPE_STRUCT,
            (SDL_YYLTYPE *) &@1,
            false,
            false,
            false,
            false),
            @$);
        }
    | _t_aggr_id SDL_K_UNION _v_aggtypes
        {
        SDL_CALL(
            sdl_state_transition(
            &context,
            Subaggregate,
            (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(
            sdl_aggregate_member(
            &context,
            $1,
            $3,
            SDL_K_TYPE_UNION,
            (SDL_YYLTYPE *) &@1,
            false,
            false,
            false,
            false),
            @$);
        }
    | _t_aggr_id SDL_K_BITFIELD bitfield_options SDL_K_SEMI
        {
        SDL_CALL(
            sdl_aggregate_member(
            &context,
            $1,
            SDL_K_TYPE_BITFLD,
            SDL_K_TYPE_NONE,
            (SDL_YYLTYPE *) &@1,
            false,
            false,
            false,
            false),
            @$);
        }
    ;

bitfield_options
    : %empty
    | bitfield_options bitfield_choices
    ;

bitfield_choices
    : SDL_K_BYTE
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            SDL_K_TYPE_BYTE,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_WORD
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            SDL_K_TYPE_WORD,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_LONG
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            SDL_K_TYPE_LONG,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_QUAD
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            SDL_K_TYPE_QUAD,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_OCTA
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            SubType,
            SDL_K_TYPE_OCTA,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_LENGTH _v_expression
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Length,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_MASK
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Mask,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_SIGNED
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Signed,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

entry
    : SDL_K_ENTRY _t_id entry_options
        {
        SDL_CALL(
            sdl_state_transition(&context, Entry, (SDL_YYLTYPE *) &@1),
            @$);
        SDL_CALL(sdl_entry(&context, $2, (SDL_YYLTYPE *) &@2), @$);
        }
    ;

entry_options
    : %empty
    | entry_options entry_choices
    ;

entry_choices
    : SDL_K_ALIAS _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Alias,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_LINKAGE _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Linkage,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_VARIABLE
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Variable,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_RETURNS _v_datatypes _a_returns_options
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            ReturnsType,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        SDL_CALL(
            sdl_add_option(
            &context,
            ReturnsNamed,
            0,
            $3,
            (SDL_YYLTYPE *) &@3),
            @$);
        }
    | SDL_K_PARAM SDL_K_OPENP parameter_desc SDL_K_CLOSEP
    ;

parameter_desc
    : parameter_options
    | parameter_desc SDL_K_COMMA parameter_options

parameter_options
    : _v_datatypes _v_passing_option parameter_choices
        {
        SDL_CALL(
            sdl_add_parameter(&context, $1, $2, (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

_v_passing_option
    : %empty
       { $$ = SDL_K_PARAM_NONE; }
    | SDL_K_VALUE
       { $$ = SDL_K_PARAM_VAL; }
    | SDL_K_REF
       { $$ = SDL_K_PARAM_REF; }
    | SDL_K_DESC
       { $$ = SDL_K_PARAM_REF; }
    | SDL_K_STR_DESC
       { $$ = SDL_K_PARAM_REF; }
    ;

parameter_choices
    : %empty
    | parameter_choices parameter_loop
    ;

parameter_loop
    : SDL_K_IN
        {
        sdl_add_option(&context, In, 0, NULL, (SDL_YYLTYPE *) &@1);
        }
    | SDL_K_OUT
        {
        sdl_add_option(&context, Out, 0, NULL, (SDL_YYLTYPE *) &@1);
        }
    | _a_named
        {
        sdl_add_option(&context, Named, 0, $1, (SDL_YYLTYPE *) &@1);
        }
    | SDL_K_DIMENSION _v_expression
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Dimension,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_DEFAULT _v_expression
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Default,
            $2,
            NULL,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_TYPENAME _t_id
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            TypeName,
            0,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_OPT
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            Optional,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_LIST
        {
        SDL_CALL(
            sdl_add_option(
            &context,
            List,
            0,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

_a_returns_options
    : %empty
        { $$ = NULL; }
    | _a_named
        { $$ = $1; }
    ;

_a_named
    : SDL_K_NAMED _t_id
        { $$ = $2; }
    ;

conditionals
    : SDL_K_IFLANG language_list
        {
        SDL_CALL(
            sdl_conditional(
            &context,
            SDL_K_COND_LANG,
            sdl_get_language(&context, (SDL_YYLTYPE *) &@2),
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_ELSE
        {
        SDL_CALL(
            sdl_conditional(
            &context,
            SDL_K_COND_ELSE,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_END_IFLANG language_list
        {
        SDL_CALL(
            sdl_conditional(
            &context,
            SDL_K_COND_END_LANG,
            sdl_get_language(&context, (SDL_YYLTYPE *) &@2),
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    | SDL_K_IFSYMB _t_id
        {
        SDL_CALL(
            sdl_conditional(
            &context,
            SDL_K_COND_SYMB,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_ELSE_IFSYMB _t_id
        {
        SDL_CALL(
            sdl_conditional(
            &context,
            SDL_K_COND_ELSEIF,
            $2,
            (SDL_YYLTYPE *) &@2),
            @$);
        }
    | SDL_K_END_IFSYMB
        {
        SDL_CALL(
            sdl_conditional(
            &context,
            SDL_K_COND_END_SYMB,
            NULL,
            (SDL_YYLTYPE *) &@1),
            @$);
        }
    ;

language_list
    : %empty
    | language_list _t_id
        {
        SDL_CALL(
            sdl_add_language(&context, $2, (SDL_YYLTYPE *) &@2),
            @$);
        }
    ;

%%    /* End Grammar rules */
