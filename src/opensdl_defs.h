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
 *
 *  V01.003	14-OCT-2018	Jonathan D. Belanger
 *  Added a block header definition to be used for all allocated blocks.
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
#define	SDL_K_TYPE_INT_B	2
#define	SDL_K_TYPE_WORD		3
#define	SDL_K_TYPE_INT_W	4
#define	SDL_K_TYPE_LONG		5
#define	SDL_K_TYPE_INT_L	6
#define	SDL_K_TYPE_INT		7
#define	SDL_K_TYPE_INT_HW	8
#define	SDL_K_TYPE_HW_INT	9
#define	SDL_K_TYPE_QUAD		10
#define	SDL_K_TYPE_INT_Q	11
#define	SDL_K_TYPE_OCTA		12
#define	SDL_K_TYPE_TFLT		13
#define	SDL_K_TYPE_TFLT_C	14
#define	SDL_K_TYPE_SFLT		15
#define	SDL_K_TYPE_SFLT_C	16
#define	SDL_K_TYPE_XFLT		17
#define	SDL_K_TYPE_XFLT_C	18
#define	SDL_K_TYPE_FFLT		19
#define	SDL_K_TYPE_FFLT_C	20
#define	SDL_K_TYPE_DFLT		21
#define	SDL_K_TYPE_DFLT_C	22
#define	SDL_K_TYPE_GFLT		23
#define	SDL_K_TYPE_GFLT_C	24
#define	SDL_K_TYPE_HFLT		25
#define	SDL_K_TYPE_HFLT_C	26
#define SDL_K_TYPE_DECIMAL	27
#define	SDL_K_TYPE_BITFLD	28
#define	SDL_K_TYPE_BITFLD_B	29
#define	SDL_K_TYPE_BITFLD_W	30
#define	SDL_K_TYPE_BITFLD_L	31
#define	SDL_K_TYPE_BITFLD_Q	32
#define	SDL_K_TYPE_BITFLD_O	33
#define	SDL_K_TYPE_CHAR		34
#define	SDL_K_TYPE_CHAR_VARY	35
#define	SDL_K_TYPE_CHAR_STAR	36
#define	SDL_K_TYPE_ADDR		37
#define	SDL_K_TYPE_ADDR_L	38
#define	SDL_K_TYPE_ADDR_Q	39
#define	SDL_K_TYPE_ADDR_HW	40
#define	SDL_K_TYPE_HW_ADDR	41
#define	SDL_K_TYPE_PTR		42
#define	SDL_K_TYPE_PTR_L	43
#define	SDL_K_TYPE_PTR_Q	44
#define	SDL_K_TYPE_PTR_HW	45
#define	SDL_K_TYPE_ANY		46
#define SDL_K_TYPE_VOID		47
#define	SDL_K_TYPE_BOOL		48
#define	SDL_K_TYPE_STRUCT	49
#define	SDL_K_TYPE_UNION	50
#define SDL_K_TYPE_IMPLICIT	51
#define SDL_K_TYPE_ENUM		52
#define SDL_K_TYPE_ENTRY	53	/* Always the last Base type */
#define SDL_K_BASE_TYPE_MIN	1
#define SDL_K_BASE_TYPE_MAX	SDL_K_TYPE_ENTRY + 1
#define SDL_K_DECLARE_MIN	64
#define SDL_K_DECLARE_MAX	255
#define SDL_K_ITEM_MIN		256
#define SDL_K_ITEM_MAX		511
#define SDL_K_AGGREGATE_MIN	512
#define SDL_K_AGGREGATE_MAX	1023
#define SDL_K_ENUM_MIN		1024
#define SDL_K_ENUM_MAX		1279
#define SDL_K_SIZEOF_MIN	1280

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
#define SDL_K_PARAM_NONE	0
#define SDL_K_PARAM_VAL		1
#define SDL_K_PARAM_REF		2

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
#define	SDL_K_NOALIGN		-1
#define	SDL_K_ALIGN		0

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
 * Conditionals
 */
#define SDL_K_COND_NONE		0
#define SDL_K_COND_SYMB		1
#define SDL_K_COND_LANG		2
#define SDL_K_COND_ELSEIF	3
#define SDL_K_COND_ELSE		4
#define SDL_K_COND_END_SYMB	5
#define SDL_K_COND_END_LANG	6

/*
 * The following definitions are going to be used for the allocation,
 * processing, and deallocation of the various OpenSDL blocks.
 */
typedef enum
{
    NotABlock,
    LocalBlock,
    LiteralBlock,
    ConstantBlock,
    EnumMemberBlock,
    EnumerateBlock,
    DeclareBlock,
    ItemBlock,
    AggrMemberBlock,
    AggregateBlock,
    ParameterBlock,
    EntryBlock
} SDL_BLOCK_ID;

typedef struct _sdl_header
{
    SDL_QUEUE		queue;
    void 		*parent;
    SDL_BLOCK_ID	blockID;
    bool		top;
} SDL_HEADER;

/*
 * The following definitions are used to maintain a list of zero or more local
 * variables
 */
typedef struct
{
    SDL_HEADER		header;
    char		*id; /* Variable name	*/
    int64_t		value; /* Variable value	*/
    int			srcLineNo;
} SDL_LOCAL_VARIABLE;

/*
 * The following definition is used to hold a single line of literal text.
 * There is no attempt to interpret what is in the line, it is written, as is
 * to the output file.
 */
typedef struct
{
    SDL_HEADER		header;
    char		*line;
    int			srcLineNo;
} SDL_LITERAL;

/*
 * The following definitions are used to maintain single CONSTANT declaration
 */
typedef struct
{
    SDL_HEADER		header;
    char		*comment;
    char		*id;
    char		*prefix;
    char		*tag;
    char		*typeName;
    union
    {
	char	*string;
	int64_t	value;
    };
    int			radix;
    int			type;	/* Numeric or String */
    int			size;	/* Number of bytes to be output (for masks) */
    int			srcLineNo;
} SDL_CONSTANT;

/*
 * The following definitions are used to declare a CONSTANT that is indicated
 * as an enumeration.
 */
typedef struct
{
    SDL_HEADER		header;
    char		*comment;
    char		*id;
    int64_t		value;
    int			srcLineNo;
    bool		valueSet;
} SDL_ENUM_MEMBER;

typedef struct
{
    SDL_HEADER		header;
    SDL_QUEUE		members;
    char		*id;
    char		*prefix;
    char		*tag;
    int64_t		size;
    int			typeID;
    int			alignment;
    int			srcLineNo;
    bool		typeDef;
} SDL_ENUMERATE;

typedef struct
{
    SDL_QUEUE		header;
    int			nextID;
} SDL_ENUM_LIST;

/*
 * The following definitions are used to declare a set of DECLARE items.
 */
typedef struct
{
    SDL_HEADER		header;
    char		*id;
    char		*prefix;
    char		*tag;
    int64_t		size;
    int			srcLineNo;
    int			type;
    int			typeID;
    bool		_unsigned;
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
    SDL_HEADER		header;
    char		*comment;
    char		*id;
    char		*prefix;
    char		*tag;
    int64_t		hbound;
    int64_t		lbound;
    int64_t		length;		/* for BITFIELDs only */
    int64_t		subType;	/* For BITFILEDs and ADDRESSes only */
    int64_t		offset;
    int64_t		precision;
    int64_t		scale;
    int64_t		size;
    int			alignment;
    int			bitOffset;	/* for BITFIELDs only */
    int			srcLineNo;
    int			type;		/* data or user type	*/
    int			typeID;
    bool		commonDef;
    bool		dimension;
    bool		fill;
    bool		globalDef;
    bool		mask;		/* For BITFIELDs only*/
    bool		parentAlignment;
    bool		typeDef;
    bool		_unsigned;
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
    SDL_HEADER		header;
    char		*comment;
    char		*name;
    char		*typeName;
    int			data;
    int64_t		bound;
    int64_t		defaultValue;
    int			passingMech;
    int			srcLineNo;
    int			type;
    bool		defaultPresent;
    bool		dimension;
    bool		in;
    bool		list;
    bool		optional;
    bool		out;
    bool		_unsigned;
} SDL_PARAMETER;

typedef struct
{
    char		*name;
    int64_t		type;
    int			srcLineNo;
    bool		_unsigned;
} SDL_RETURNS;

typedef struct
{
    SDL_HEADER		header;
    char		*alias;
    char		*comment;
    char		*id;
    char		*linkage;
    char		*typeName;
    SDL_QUEUE		parameters;
    SDL_RETURNS		returns;
    int			srcLineNo;
    bool		variable;
} SDL_ENTRY;

/*
 * The following definitions are used to declare a single AGGREGATE.
 */
typedef struct
{
    char		*basedPtrName;
    char		*comment;
    char		*id;
    char		*marker;
    char		*prefix;
    char		*tag;
    struct _sdl_member	*parent;
    SDL_QUEUE		members;
    int64_t		currentOffset;
    int64_t		hbound;
    int64_t		lbound;
    int64_t		offset;
    int64_t		size;
    int			alignment;
    int			currentBitOffset;
    int			aggType;
    int			type;
    int			typeID;
    bool		dimension;
    bool		fill;
    bool		parentAlignment;
    bool		typeDef;
    bool		_unsigned;
} SDL_SUBAGGR;

typedef struct _sdl_member
{
    SDL_HEADER		header;
    union
    {
	SDL_ITEM    	item;
	SDL_SUBAGGR	subaggr;
    };
    int64_t		offset;
    int			srcLineNo;
    int			type;
} SDL_MEMBERS;

typedef struct
{
    char 		*id;
    SDL_MEMBERS		*origin;
    int			srcLineNo;
} SDL_ORIGIN;

typedef struct
{
    SDL_HEADER		header;
    char		*comment;
    char		*basedPtrName;
    char		*id;
    char		*marker;
    char		*prefix;
    char		*tag;
    SDL_QUEUE		members;
    SDL_ORIGIN		origin;
    int64_t		currentOffset;
    int64_t		hbound;
    int64_t		lbound;
    int64_t		size;
    int			aggType;
    int			alignment;
    int			currentBitOffset;
    int			srcLineNo;
    int			type;
    int			typeID;
    bool		alignmentPresent;
    bool		commonDef;
    bool		dimension;
    bool		fill;
    bool		globalDef;
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
    Enumerate,
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
    ReturnsNamed,
    ReturnsType,
    Signed,
    Tag,
    Typedef,
    TypeName,
    Value,
    Variable,
    SubType
} SDL_OPTION_TYPE;
#define SDL_K_OPTIONS_INCR	8
typedef struct
{
    SDL_OPTION_TYPE	option;
    union
    {
	int64_t	value;
	char	*string;
    };
    int			srcLineNo;
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
    ElseLanguage,
    IfSymbol,
    ElseSymbol,
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
	int64_t value;
	char	*valueStr;
    };
    bool		string;
    int			srcLineNo;
} SDL_CONSTANT_DEF;

/*
 * This structure is used to manage one or more language specifiers on a
 * conditional expression.
 */
typedef char	SDL_LANG_STR[16];
typedef struct
{
    SDL_LANG_STR	*lang;
    int			listUsed;
    int			listSize;
} SDL_LANGUAGE_LIST;

/*
 * This structure is used to manage zero or more symbol specific conditional
 * expressions.
 */
typedef struct
{
    char	*symbol;
    int		value;
} SDL_SYMBOL;
typedef struct
{
    SDL_SYMBOL		*symbols;
    int			listUsed;
    int			listSize;
} SDL_SYMBOL_LIST;

/*
 * This is the context data structure.  It maintains everything about what has
 * been parsed and is being parsed.  It is initialized when a MODULE has been
 * parsed and is cleared when an END_MODULE has been parsed.
 */
typedef struct
{
    char 		*ident;
    char 		*module;
    char		*inputFile;
    char		*outFileName[SDL_K_LANG_MAX];
    FILE		*outFP[SDL_K_LANG_MAX];
    void		*currentAggr;
    bool		langEna[SDL_K_LANG_MAX];
    bool		langSpec[SDL_K_LANG_MAX];
    SDL_DIMENSION	dimensions[SDL_K_MAX_DIMENSIONS];
    SDL_OPTION		*options;
    SDL_PARAMETER	**parameters;
    SDL_DECLARE_LIST	declares;
    SDL_ITEM_LIST	items;
    SDL_AGGREGATE_LIST	aggregates;
    SDL_ENUM_LIST	enums;
    SDL_STATE		state;
    SDL_STATE		constantPrevState;
    SDL_QUEUE		locals;
    SDL_QUEUE		constants;
    SDL_QUEUE		entries;
    SDL_CONSTANT_DEF	constDef;
    SDL_LANGUAGE_LIST	langCondList;
    SDL_SYMBOL_LIST	symbCondList;
    int64_t		precision;
    int64_t		scale;
    int			aggregateDepth;
    int			alignment;
    int			fillerCount;
    int			optionsIdx;
    int			optionsSize;
    int			parameterIdx;
    int			parameterSize;
    int			modEndSrcLineNo;
    int			modSrcLineNo;
    int			wordSize;	/* 32 or 64 */
    bool		checkAlignment;
    bool		commentsOff;
    bool		copyright;
    bool		header;
    bool		memberAlign;
    bool		processingEnabled;
    bool		suppressPrefix;
    bool		suppressTag;
} SDL_CONTEXT;

/*
 *
 */
#define sdl_m_value		0x00000001
#define sdl_m_mask		0x00000002
#define sdl_m_unsigned		0x00000004
#define sdl_m_common		0x00000008
#define sdl_m_global		0x00000010
#define sdl_m_varying		0x00000020
#define sdl_m_variable		0x00000040
#define sdl_m_based		0x00000080
#define sdl_m_desc 		0x00000100
#define sdl_m_dimen 		0x00000200
#define sdl_m_in		0x00000400
#define sdl_m_out		0x00000800
#define sdl_m_bottom 		0x00001000
#define sdl_m_bound 		0x00002000
#define sdl_m_ref		0x00004000
#define sdl_m_userfill 		0x00008000
#define sdl_m_alias		0x00010000
#define sdl_m_default		0x00020000
#define sdl_m_vardim		0x00040000
#define sdl_m_link		0x00080000
#define sdl_m_optional		0x00100000
#define sdl_m_signed		0x00200000
#define sdl_m_fixed_fldsiz	0x00400000
#define sdl_m_generated		0x00800000
#define sdl_m_module		0x01000000
#define sdl_m_list		0x02000000
#define sdl_m_rtl_str_desc	0x04000000
#define sdl_m_complex		0x08000000
#define sdl_m_typedef		0x10000000
#define sdl_m_declared		0x20000000
#define sdl_m_forward		0x40000000
#define sdl_m_align		0x80000000

/*
 *
 */
typedef struct sdl_r_node
{
    struct sdl_r_node *sdl_a_flink;
    struct sdl_r_node *sdl_a_blink;
    struct sdl_r_node *sdl_a_parent;
    struct sdl_r_node *sdl_a_child;
    struct sdl_r_node *sdl_a_comment;
    union
    {
	int sdl_l_typeinfo;
	void *sdl_a_typeinfo;
    } sdl_r_info;
    union
    {
	int sdl_l_typeinfo2;
	void *sdl_a_typeinfo2;
	void *sdl_a_symtab;
    } sdl_r_info2;
    char sdl_b_type;
    char sdl_b_boundary;
    short sdl_w_datatype;
    int sdl_l_offset;
    union
    {
	int sdl_l_fldsiz;
	void *sdl_a_fldsiz;
    } sdl_r_fldsiz;

    /*
     * Flags sdl_v_%%dim indicate module SDLACTION.PLI
     * has cached a pointer to an expression that cannot
     * be evaluated yet.  Those flags will be cleared as
     * SDLACTION.PLI finishes parsing the aggregate that
     * contains the item.
     */
    union	/* A single longword used for two purposes */
    {
	int sdl_l_hidim;	/* generally used as integer, but */
	void *hidim;		/* SDLACTION.PLI caches a pointer. */
    } mod$r_hidim;
    union	/* A single longword used for two purposes */
    {
	int sdl_l_lodim;	/* generally used as integer, but */
	void *sdl_a_lodim;	/* SDLACTION.PLI caches a pointer. */
    } nof$r_lodim;

    /*
     * Flag sdl_v_initial indicate module SDLACTION.PLI
     * has cached a pointer to an expression that cannot
     * be evaluated yet.  That flag will be cleared as
     * SDLACTION.PLI finishes parsing the aggregate that
     * contains the item.
     */
    union	/* A single longword used for two purposes */
    {
	int sdl_l_initial; 	/* generally used as integer, but */
	void *sdl_a_initial;  	/* SDLACTION.PLI caches a pointer. */
    } sdl_r_initial;
    int sdl_l_srcline;
    int sdl_l_nodeid;
    union
    {
	unsigned int sdl_l_flags;
	int sdl_l_fixflags;
	struct
	{
	    unsigned int sdl_v_value : 1;
	    unsigned int sdl_v_mask : 1;
	    unsigned int sdl_v_unsigned : 1;
	    unsigned int sdl_v_common : 1;
	    unsigned int sdl_v_global : 1;
	    unsigned int sdl_v_varying : 1;
	    unsigned int sdl_v_variable : 1;
	    unsigned int sdl_v_based : 1;
	    unsigned int sdl_v_desc : 1;
	    unsigned int sdl_v_dimen : 1;	/* is dimensioned */
	    unsigned int sdl_v_in : 1;
	    unsigned int sdl_v_out : 1;
	    unsigned int sdl_v_bottom : 1;
	    unsigned int sdl_v_bound : 1;
	    unsigned int sdl_v_ref : 1;
	    unsigned int sdl_v_userfill : 1;
	    unsigned int sdl_v_alias : 1;
	    unsigned int sdl_v_default : 1;	/* DEFAULT */
	    unsigned int sdl_v_vardim : 1;	/* "DIMENSION *" */
	    unsigned int sdl_v_link : 1;
	    unsigned int sdl_v_optional : 1;
	    unsigned int sdl_v_signed : 1;
	    unsigned int sdl_v_fixed_fldsiz : 1;
	    unsigned int sdl_v_generated : 1;
	    unsigned int sdl_v_module : 1;
	    unsigned int sdl_v_list : 1;
	    unsigned int sdl_v_rtl_str_desc : 1;
	    unsigned int sdl_v_complex : 1;
	    unsigned int sdl_v_typedef : 1;
	    unsigned int sdl_v_declared : 1;
	    unsigned int sdl_v_forward : 1;
	    unsigned int sdl_v_align : 1;
	} sdl_r_flagstruc;
    } sdl_r_flagunion;
    union
    {
	unsigned int sdl_l_flags2;
	int sdl_l_fixflags2;
	struct
	{
	    unsigned int sdl_v_has_object : 1;
	    unsigned int sdl_v_offset_fixed : 1;
	    unsigned int sdl_v_length : 1;
	    unsigned int sdl_v_hidim : 1;
	    unsigned int sdl_v_lodim : 1;
	    unsigned int sdl_v_initial : 1;
	    unsigned int sdl_v_base_align : 1;
	    unsigned int sdl_v_offset_ref : 1;
	} sdl_r_flags2struc;
    } sdl_r_flags2union;
    struct
    {
	short string_length;
	char string_text[34];
    } sdl_t_naked;
    struct
    {
	short string_length;
	char string_text[34];
    } sdl_t_name;
    struct
    {
	short string_length;
	char string_text[34];
    } sdl_t_return_name;
    struct
    {
	short string_length;
	char string_text[32];
    } sdl_t_prefix;
    struct
    {
	short string_length;
	char string_text[32];
    } sdl_t_marker;
    struct
    {
	short string_length;
	char string_text[32];
    } sdl_t_tag;
    struct
    {
	short string_length;
	char string_text[32];
    } sdl_t_typename;
    struct
    {
	short string_length;
	char string_text[32];
    } sdl_t_maskstr;
} SDL_R_NODE;
#define sdl_k_nodesize sizeof(SDL_R_NODE)

#endif /* _OPENSDL_DEFS_H_ */
