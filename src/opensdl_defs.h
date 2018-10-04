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
 *  This header file contains the definitions needed for the action routines to
 *  be able to interact with the parsing of an Open Structure Definition
 *  Language (OpenSDL) formatted definition file.
 *
 * Revision History:
 *
 *  V01.000	24-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	06-SEP-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 *
 *  V01.002	04-OCT-2018	Jonathan D. Belanger
 *  Reorganized and moved Queue definitions to their own header file.
 */
#ifndef _OPENSDL_DEFS_H_
#define _OPENSDL_DEFS_H_

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include "opensdl_queue.h"

#define SDL_K_VERSION_TYPE	'X'
#define SDL_K_VERSION_MAJOR	3
#define SDL_K_VERSION_MINOR	4
#define SDL_K_VERSION_LEVEL	10

/*
 * These are the base types supported by OpenSDL.
 */
#define SDL_K_TYPE_CONST	0
#define SDL_K_TYPE_NONE		0
#define	SDL_K_TYPE_BYTE		1
#define	SDL_K_TYPE_WORD		2
#define	SDL_K_TYPE_LONG		3
#define	SDL_K_TYPE_QUAD		4
#define	SDL_K_TYPE_OCTA		5
#define	SDL_K_TYPE_TFLT		6
#define	SDL_K_TYPE_SFLT		7
#define SDL_K_TYPE_DECIMAL	8
#define	SDL_K_TYPE_BITFLD	9
#define	SDL_K_TYPE_CHAR		10
#define	SDL_K_TYPE_ADDR		11
#define	SDL_K_TYPE_ADDRL	12
#define	SDL_K_TYPE_ADDRQ	13
#define	SDL_K_TYPE_ADDRHW	14
#define	SDL_K_TYPE_ANY		15
#define	SDL_K_TYPE_BOOL		16
#define	SDL_K_TYPE_SRUCT	17
#define	SDL_K_TYPE_UNION	18
#define SDL_K_BASE_TYPE_MIN	1
#define SDL_K_BASE_TYPE_MAX	63
#define SDL_K_DECLARE_MIN	64
#define SDL_K_DECLARE_MAX	255
#define SDL_K_ITEM_MIN		256
#define SDL_K_ITEM_MAX		511
#define SDL_K_AGGREGATE_MIN	512
#define SDL_K_AGGREGATE_MAX	1023

/*
 * Data type modifiers.
 */
#define	SDL_K_TYPE_MOD_SIGN	1
#define	SDL_K_TYPE_MOD_UNSIGN	2

/*
 * CONSTANT types
 */
#define SDL_K_CONST_NUM		1
#define SDL_K_CONST_STR		2

/*
 * ITEM types
 */
#define SDL_K_ITEM_DATA		1
#define SDL_K_ITEM_USER		2

/*
 * STORAGE types
 */
#define SDL_M_STOR_NONE		0
#define SDL_M_STOR_COMM		1
#define SDL_M_STOR_GLOB		2
#define SDL_M_STOR_TYPED	4

/*
 * DECLARE types
 */
#define SDL_K_DECL_DATA		SDL_K_ITEM_DATA
#define SDL_K_DECL_USER		SDL_K_ITEM_USER
#define SDL_K_DECL_EXPR		3

/*
 * ORIGIN types
 */
#define SDL_K_ORIGIN_DATA	SDL_K_ITEM_DATA
#define SDL_K_ORIGIN_USER	SDL_K_ITEM_USER
#define SDL_K_ORIGIN_AGGR	3

/*
 * PARAMETER types
 */
#define SDL_K_PARAM_DATA	SDL_K_ITEM_DATA
#define SDL_K_PARAM_AGGR	2

/*
 * RETURNS types
 */
#define SDL_K_RET_DATA		SDL_K_ITEM_DATA
#define SDL_K_RET_USER		SDL_K_ITEM_USER
#define SDL_K_RET_VOID		3

/*
 * ENTRY types
 */
#define SDL_K_PARAM_DSC		1
#define SDL_K_PARAM_SDESC	2
#define SDL_K_PARAM_VAL		3
#define SDL_K_PARAM_REF		4

/*
 * Radix input types
 */
#define	SDL_K_RADIX_DEF		0
#define	SDL_K_RADIX_DEC		0
#define	SDL_K_RADIX_BIN		1
#define	SDL_K_RADIX_OCT		2
#define	SDL_K_RADIX_HEX		3

/*
 * AGGREGATE member alignment options
 *
 * NOTE: A value greater than zero is used to calculate the alignment as
 * 2^value.
 */
#define	SDL_K_ALIGN		-1
#define	SDL_K_NOALIGN		0

/*
 * Operations that can be performed on local variables
 */
#define	SDL_K_OPER_SET		1
#define	SDL_K_OPER_SUB		2
#define	SDL_K_OPER_ADD		3
#define	SDL_K_OPER_MUL		4
#define	SDL_K_OPER_DIV		5
#define	SDL_K_OPER_SHIFT	6
#define	SDL_K_OPER_AND		7
#define	SDL_K_OPER_OR		8

/*
 * Offset calculations.
 */
#define	SDL_K_OFF_BYTE_REL	1	/* from 1st member or ORIGIN */
#define SDL_K_OFF_BYTE_BEG	2	/* from 1st member only */
#define	SDL_K_OFF_BIT		3	/* bit offset from previous */

/*
 * The following definitions are used to maintain a list of zero or more local
 * variables
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*id; /* Variable name	*/
    __int64_t		value; /* Variable value	*/
} SDL_LOCAL_VARIABLE;

/*
 * The following definition is used to hold a single line of literal text.
 * There is no attempt to interpret what is in the line, it is written, as is
 * to the output file.
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*line;
} SDL_LITERAL;

/*
 * The following definitions are used to maintain single CONSTANT declaration
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*id;
    char		*prefix;
    char		*tag;
    char		*comment;
    char		*typeName;
    int			radix;
    int			type; /* Numeric or String */
    union
    {
	char		*string;
	__int64_t	value;
    };
} SDL_CONSTANT;

/*
 * The following definitions are used to declare a set of DECLARE items.
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*id;
    char		*prefix;
    char		*tag;
    int			typeID;
    int			type;
    __int64_t		size;
} SDL_DECLARE;

typedef struct
{
    SDL_QUEUE		header;
    int			nextID;
} SDL_DECLARE_LIST;

/*
 * The following definition is used to declare a single ITEM.
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*id;
    char		*prefix;
    char		*tag;
    char		*comment;
    int			typeID;
    int			alignment;
    bool		commonDef;
    bool		globalDef;
    bool		typeDef;
    bool		dimension;
    bool		_unsigned;
    __int64_t		precision;
    __int64_t		scale;
    __int64_t		lbound;
    __int64_t		hbound;
    __int64_t		size;
    __int64_t		memSize;	/* Actual space used in memory	*/
    int			type;		/* data or user type	*/
} SDL_ITEM;

typedef struct
{
    SDL_QUEUE		header;
    int			nextID;
} SDL_ITEM_LIST;

/*
 * The following definitions are used to declare a single ENTRY, with zero or
 * one parameter lists.
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*name;
    char		*comment;
    char		*typeName;
    int			type;
    union
    {
	int		data;
	char		*aggrName;
    };
    bool		in;
    bool		out;
    bool		optional;
    bool		list;
    bool		defaultPresent;
    bool		dimension;
    __int64_t		bound;
    __int64_t		defaultValue;
} SDL_PARAMETER;

typedef struct
{
    SDL_QUEUE		header;
    char		*id;
    char		*comment;
    char		*alias;
    char		*linkage;
    char		*typeName;
    bool		variable;
    bool		returns;
    SDL_QUEUE		parameters;
    int			type;
} SDL_ENTRY;

/*
 * The following definitions are used to declare a single AGGREGATE.
 */
typedef struct
{
    char 		*id;
    char		*prefix;
    char		*tag;
    int			type;
    union
    {
	int		data;
	char		*name;
    };
    int			alignment;
    bool		fill;
    bool		dimension;
    __int64_t		lbound;
    __int64_t		hbound;
    __int64_t		offset;
} SDL_ORIGIN;

typedef struct
{
    char		*id;
    char		*prefix;
    char		*tag;
    char		*marker;
    char		*comment;
    char		*basedPtrName;
    int			typeID;
    int			unionType;	/* for implied unions 		*/
    int			alignment;
    int			type;
    SDL_QUEUE		members;
    bool		typeDef;
    bool		fill;
    bool		dimension;
    __int64_t		lbound;
    __int64_t		hbound;
    __int64_t		currentOffset;
    __int64_t		size;
    __int64_t		memSize;	/* Actual space used in memory	*/
    int			currentBitOffset;
} SDL_SUBAGGR;

typedef struct
{
    SDL_QUEUE		header;
    int			type;
    union
    {
	SDL_ITEM    	item;
	SDL_SUBAGGR	subaggr;
    };
} SDL_MEMBERS;

typedef struct
{
    SDL_QUEUE		header;
    char		*id;
    char		*prefix;
    char		*tag;
    char		*marker;
    char		*comment;
    char		*basedPtrName;
    SDL_ORIGIN		origin;
    int			typeID;
    int			unionType;	/* for implied unions 		*/
    int			alignment;
    int			type;
    SDL_QUEUE		members;
    bool		originPresent;
    bool		commonDef;
    bool		globalDef;
    bool		typeDef;
    bool		fill;
    bool		dimension;
    __int64_t		lbound;
    __int64_t		hbound;
    __int64_t		currentOffset;
    __int64_t		size;
    __int64_t		memSize;	/* Actual space used in memory	*/
    int			currentBitOffset;
} SDL_AGGREGATE;

typedef struct
{
    SDL_QUEUE		header;
    int			nextID;
} SDL_AGGREGATE_LIST;

/*
 * The following structure is used to hold dimension data (low and high), as
 * well as a status.
 */
#define SDL_K_MAX_DIMENSIONS	16
typedef struct
{
    size_t		lbound;
    size_t		hbound;
    bool		inUse;
} SDL_DIMENSION;

/*
 * Options can come in any order, so we are never quite sure which ones we are
 * getting when.  So, we will create a structure to store them as they come and
 * when we reach the end of a definition, they will be used for that
 * definition.
 */

/*
 * Available options allowed on declarations.
 */
typedef enum
{
    None,
    Align,
    NoAlign,
    BaseAlign,
    Common,
    Global,
    Prefix,
    Tag,
    Based,
    Typedef,
    Fill,
    Marker,
    Origin,
    Counter,
    Increment,
    TypeName,
    Reference,
    Value,
    In,
    Out,
    Default,
    List,
    Named,
    Optional,
    Returns,
    Alias,
    Linkage,
    Parameter,
    Variable,
    Radix,
    Dimension,
    Length,
    Mask
} SDL_OPTION_TYPE;
#define SDL_K_MAX_OPTIONS	8
typedef struct
{
    SDL_OPTION_TYPE	option;
    union
    {
	__int64_t	value;
	char		*string;
    };
} SDL_OPTION;

/*
 * Supported languages and other useful definitions.
 */
#define SDL_K_LANG_C		0
#define SDL_K_LANG_MAX		1
#define SDL_TIMESTR_LEN		20 + 1	/* dd-MMM-yyyy hh:mm:ss		*/
#define SDL_K_SUBAGG_MAX	8 + 1

/*
 * State values used to determine what is being parsed and how to interpret
 * the information.
 */
typedef enum
{
    Initial,
    Module,
    Comment,
    Literal,
    Local,
    Declare,
    Constant,
    Item,
    Aggregate,
    Subaggregate,
    Entry,
    IfLanguage,
    IfSymbol,
    DefinitionEnd
} SDL_STATE;

/*
 * This is used to save information about a CONSTANT that is being defined as
 * the parsing progresses.  This information will eventually be used to define
 * one or more actual CONSTANT values.
 */
typedef struct
{
    char		*id;
    bool		string;
    union
    {
	__int64_t 	value;
	char		*valueStr;
    };
} SDL_CONSTANT_DEF;

/*
 * This is the context data structure.  It maintains everything about what has
 * been parsed and is being parsed.  It is initialized when a MODULE has been
 * parsed and is cleared when an END_MODULE has been parsed.
 */
typedef struct
{

    /*
     * The following fields are set when processing the MODULE statement and
     * checked in the processing for the END_MODULE statement (if the module-id
     * is supplied).
     */
    char 		*module;
    char 		*ident;

    /*
     * The following fields are set in the main function as part of processing
     * the command-line qualifiers.
     */
    bool		langSpec[SDL_K_LANG_MAX];
    bool		langEna[SDL_K_LANG_MAX];
    char		*outFileName[SDL_K_LANG_MAX];
    FILE		*outFP[SDL_K_LANG_MAX];

    /*
     * The following fields are used while parsing various statement
     * constructs.  In most cases, these fields are fleeting and can be
     * continuously reused.
     */
    SDL_DIMENSION	dimensions[SDL_K_MAX_DIMENSIONS];
    SDL_OPTION		options[SDL_K_MAX_OPTIONS];
    SDL_CONSTANT_DEF	constDef;
    __int64_t		precision;
    __int64_t		scale;
    int			optionsIdx;

    /*
     * The following field is used to maintain the current state of the
     * processing.  NOTE: There is no history associated with states in this
     * implementation.  So, there is no ability to go from one state to another
     * and then back to the previous state, where this previous state has more
     * than one possibility.
     */
    SDL_STATE		state;

    /*
     * The following fields are why we are here.  These fields are created as
     * part of the processing.  They are also passed to the language specific
     * output functions.
     */
    SDL_QUEUE		locals;
    SDL_QUEUE		constants;
    SDL_DECLARE_LIST	declares;
    SDL_ITEM_LIST	items;
    SDL_AGGREGATE_LIST	aggregates;
    SDL_QUEUE		entries;
} SDL_CONTEXT;

#endif /* _OPENSDL_DEFS_H_ */
