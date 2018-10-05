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
    char		*comment;
    char		*id;
    char		*prefix;
    char		*tag;
    char		*typeName;
    union
    {
	char		*string;
	__int64_t	value;
    };
    int			radix;
    int			type; /* Numeric or String */
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
    __int64_t		size;
    int			type;
    int			typeID;
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
    char		*comment;
    char		*id;
    char		*prefix;
    char		*tag;
    int			alignment;
    int			type;		/* data or user type	*/
    int			typeID;
    bool		commonDef;
    bool		dimension;
    bool		fill;
    bool		globalDef;
    bool		mask;		/* for BITFIELDs only*/
    bool		typeDef;
    bool		_signed;	/* for BITFIELDs only */
    bool		_unsigned;
    __int64_t		hbound;
    __int64_t		lbound;
    __int64_t		length;		/* for BITFIELDs only */
    __int64_t		memSize;	/* Actual space used in memory	*/
    __int64_t		precision;
    __int64_t		scale;
    __int64_t		size;
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
    char		*comment;
    char		*name;
    char		*typeName;
    union
    {
	int		data;
	char		*aggrName;
    };
    __int64_t		bound;
    __int64_t		defaultValue;
    int			type;
    bool		defaultPresent;
    bool		dimension;
    bool		in;
    bool		list;
    bool		optional;
    bool		out;
} SDL_PARAMETER;

typedef struct
{
    SDL_QUEUE		header;
    char		*alias;
    char		*comment;
    char		*id;
    char		*linkage;
    char		*typeName;
    SDL_QUEUE		parameters;
    int			type;
    bool		returns;
    bool		variable;
} SDL_ENTRY;

/*
 * The following definitions are used to declare a single AGGREGATE.
 */

typedef enum
{
    Unknown,
    Structure,
    Union
} SDL_AGGR_TYPE;

typedef struct
{
    char		*basedPtrName;
    char		*comment;
    char		*id;
    char		*marker;
    char		*prefix;
    char		*tag;
    void		*parent;	/* aggregateDepth determines level */
    SDL_QUEUE		members;
    SDL_AGGR_TYPE	structUnion;
    __int64_t		currentOffset;
    __int64_t		hbound;
    __int64_t		lbound;
    __int64_t		memSize;	/* Actual space used in memory	*/
    __int64_t		size;
    int			alignment;
    int			currentBitOffset;
    int			type;
    int			typeID;
    bool		dimension;
    bool		fill;
    bool		typeDef;
    bool		_unsigned;
} SDL_SUBAGGR;

typedef struct
{
    SDL_QUEUE		header;
    union
    {
	SDL_ITEM    	item;
	SDL_SUBAGGR	subaggr;
    };
    SDL_AGGR_TYPE	type;
} SDL_MEMBERS;

typedef struct
{
    char 		*id;
    SDL_MEMBERS		*origin;
} SDL_ORIGIN;

typedef struct
{
    SDL_QUEUE		header;
    char		*comment;
    char		*basedPtrName;
    char		*id;
    char		*marker;
    char		*prefix;
    char		*tag;
    SDL_QUEUE		members;
    SDL_ORIGIN		origin;
    __int64_t		currentOffset;
    __int64_t		hbound;
    __int64_t		lbound;
    __int64_t		memSize;	/* Actual space used in memory	*/
    __int64_t		size;
    SDL_AGGR_TYPE	structUnion;
    int			alignment;
    int			currentBitOffset;
    int			type;
    int			typeID;
    bool		commonDef;
    bool		dimension;
    bool		fill;
    bool		globalDef;
    bool		originPresent;
    bool		typeDef;
    bool		_unsigned;
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
    Alias,
    Align,
    BaseAlign,
    Based,
    Counter,
    Default,
    Dimension,
    Fill,
    Common,
    Global,
    In,
    Increment,
    Length,
    Linkage,
    List,
    Marker,
    Mask,
    Optional,
    Origin,
    Out,
    Named,
    NoAlign,
    Parameter,
    Prefix,
    Radix,
    Reference,
    Returns,
    Signed,
    Tag,
    Typedef,
    TypeName,
    Value,
    Variable
} SDL_OPTION_TYPE;
#define SDL_K_MAX_OPTIONS	16
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
    Aggregate,
    Comment,
    Constant,
    Declare,
    Entry,
    IfLanguage,
    IfSymbol,
    Item,
    Literal,
    Local,
    Module,
    Subaggregate,
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
    union
    {
	__int64_t 	value;
	char		*valueStr;
    };
    bool		string;
} SDL_CONSTANT_DEF;

/*
 * This is the context data structure.  It maintains everything about what has
 * been parsed and is being parsed.  It is initialized when a MODULE has been
 * parsed and is cleared when an END_MODULE has been parsed.
 */
typedef struct
{
    char 		*ident;
    char 		*module;
    char		*outFileName[SDL_K_LANG_MAX];
    FILE		*outFP[SDL_K_LANG_MAX];
    void		*currentAggr;
    bool		langEna[SDL_K_LANG_MAX];
    bool		langSpec[SDL_K_LANG_MAX];
    SDL_DIMENSION	dimensions[SDL_K_MAX_DIMENSIONS];
    SDL_OPTION		options[SDL_K_MAX_OPTIONS];
    SDL_DECLARE_LIST	declares;
    SDL_ITEM_LIST	items;
    SDL_AGGREGATE_LIST	aggregates;
    SDL_STATE		state;
    SDL_QUEUE		locals;
    SDL_QUEUE		constants;
    SDL_QUEUE		entries;
    SDL_CONSTANT_DEF	constDef;
    __int64_t		precision;
    __int64_t		scale;
    int			aggregateDepth;
    int			optionsIdx;
} SDL_CONTEXT;

#endif /* _OPENSDL_DEFS_H_ */
