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
#define SDL_K_TYPE_ENUM		51
#define SDL_K_TYPE_ENTRY	52	/* Always the last Base type */
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
    int64_t		value; /* Variable value	*/
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
	char	*string;
	int64_t	value;
    };
    int			radix;
    int			type; /* Numeric or String */
} SDL_CONSTANT;

/*
 * The following definitions are used to declare a CONSTANT that is indicated
 * as an enumeration.
 */
typedef struct
{
    SDL_QUEUE		header;
    char		*comment;
    char		*id;
    int64_t		value;
    bool		valueSet;
} SDL_ENUM_MEMBER;

typedef struct
{
    SDL_QUEUE		header;
    SDL_QUEUE		members;
    char		*id;
    char		*prefix;
    char		*tag;
    int64_t		size;
    int64_t		memSize;	/* Actual space used in memory	*/
    int			typeID;
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
    SDL_QUEUE		header;
    char		*id;
    char		*prefix;
    char		*tag;
    int64_t		size;
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
    int64_t		hbound;
    int64_t		lbound;
    int64_t		length;		/* for BITFIELDs only */
    int64_t		subType;	/* For BITFILEDs and ADDRESSes only */
    int64_t		memSize;	/* Actual space used in memory	*/
    int64_t		precision;
    int64_t		scale;
    int64_t		size;
    int			alignment;
    int			type;		/* data or user type	*/
    int			typeID;
    bool		commonDef;
    bool		dimension;
    bool		fill;
    bool		globalDef;
    bool		mask;		/* For BITFIELDs only*/
    bool		typeDef;
    bool		_signed;	/* For BITFIELDs only */
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
    SDL_QUEUE		header;
    char		*comment;
    char		*name;
    char		*typeName;
    union
    {
	int	data;
	char	*aggrName;
    };
    int64_t		bound;
    int64_t		defaultValue;
    int			type;
    int			passingMech;
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
    int			type;
    bool		_unsigned;
} SDL_RETURNS;

typedef struct
{
    SDL_QUEUE		header;
    char		*alias;
    char		*comment;
    char		*id;
    char		*linkage;
    char		*typeName;
    SDL_QUEUE		parameters;
    SDL_RETURNS		returns;
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
    int64_t		currentOffset;
    int64_t		hbound;
    int64_t		lbound;
    int64_t		memSize;	/* Actual space used in memory	*/
    int64_t		size;
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
    int64_t		currentOffset;
    int64_t		hbound;
    int64_t		lbound;
    int64_t		memSize;	/* Actual space used in memory	*/
    int64_t		size;
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
	int64_t value;
	char	*valueStr;
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
    int64_t		precision;
    int64_t		scale;
    int			aggregateDepth;
    int			optionsIdx;
    int			optionsSize;
    int			parameterIdx;
    int			parameterSize;
    int			wordSize;	/* 32 or 64 */
    bool		memberAlign;
    bool		commentsOff;
} SDL_CONTEXT;

/*
 *
 */
struct nod$_node
{
    void *nod$a_flink;
    void *nod$a_blink;
    void *nod$a_paren;
    void *nod$a_child;
    void *nod$a_comment;
    union
    {
	int nod$l_typeinfo;
	void *nod$a_typeinfo;
    } nod$r_info;
    union
    {
	int nod$l_typeinfo2;
	void *nod$a_typeinfo2;
	void *nod$a_symtab;
    } nod$r_info2;
    char nod$b_type;
    char nod$b_boundary;
    short nod$w_datatype;
    int nod$l_offset;
    union
    {
	int nod$l_fldsiz;
	void *nod$a_fldsiz;
    } nod$r_fldsiz;

    /*
     * Flags nod$v_%%dim indicate module SDLACTION.PLI
     * has cached a pointer to an expression that cannot
     * be evaluated yet.  Those flags will be cleared as
     * SDLACTION.PLI finishes parsing the aggregate that
     * contains the item.
     */
    union	/* A single longword used for two purposes */
    {
	int nod$l_hidim;	/* generally used as integer, but */
	void *hidim;		/* SDLACTION.PLI caches a pointer. */
    } mod$r_hidim;
    union	/* A single longword used for two purposes */
    {
	int nod$l_lodim;	/* generally used as integer, but */
	void *nod$a_lodim;	/* SDLACTION.PLI caches a pointer. */
    } nof$r_lodim;

    /*
     * Flag nod$v_initial indicate module SDLACTION.PLI
     * has cached a pointer to an expression that cannot
     * be evaluated yet.  That flag will be cleared as
     * SDLACTION.PLI finishes parsing the aggregate that
     * contains the item.
     */
    union	/* A single longword used for two purposes */
    {
	int nod$l_initial; 	/* generally used as integer, but */
	void *nod$a_initial;  	/* SDLACTION.PLI caches a pointer. */
    } nod$r_initial;
    int nod$l_srcline;
    int nod$l_nodeid;
    union
    {
	unsigned int nod$l_flags;
	int nod$l_fixflags;
	struct
	{
	    unsigned int nod$v_value : 1;
#define nod$m_value	0x00000001
	    unsigned int nod$v_mask : 1;
#define nod$m_mask	0x00000002
	    unsigned int nod$v_unsigned : 1;
#define nod$m_unsigned	0x00000004
	    unsigned int nod$v_common : 1;
#define nod$m_common	0x00000008
	    unsigned int nod$v_global : 1;
#define nod$m_global	0x00000010
	    unsigned int nod$v_varying : 1;
#define nod$m_varying	0x00000020
	    unsigned int nod$v_variable : 1;
#define nod$m_variable	0x00000040
	    unsigned int nod$v_based : 1;
#define nod$m_based	0x00000080
	    unsigned int nod$v_desc : 1;
#define nod$m_desc 	0x00000100
	    unsigned int nod$v_dimen : 1;	/* is dimensioned */
#define nod$m_dimen 	0x00000200
	    unsigned int nod$v_in : 1;
#define nod$m_in	0x00000400
	    unsigned int nod$v_out : 1;
#define nod$m_out	0x00000800
	    unsigned int nod$v_bottom : 1;
#define nod$m_bottom 	0x00001000
	    unsigned int nod$v_bound : 1;
#define nod$m_bound 	0x00002000
	    unsigned int nod$v_ref : 1;
#define nod$m_ref	0x00004000
	    unsigned int nod$v_userfill : 1;
#define nod$m_userfill 	0x00008000
	    unsigned int nod$v_alias : 1;
#define nod$m_alias	0x00010000
	    unsigned int nod$v_default : 1;	/* DEFAULT */
#define nod$m_default	0x00020000
	    unsigned int nod$v_vardim : 1;	/* "DIMENSION *" */
#define nod$m_vardim	0x00040000
	    unsigned int nod$v_link : 1;
#define nod$m_link	0x00080000
	    unsigned int nod$v_optional : 1;
#define nod$m_optional	0x00100000
	    unsigned int nod$v_signed : 1;
#define nod$m_signed	0x00200000
	    unsigned int nod$v_fixed_fldsiz : 1;
#define nod$m_fixed_fldsiz	0x00400000
	    unsigned int nod$v_generated : 1;
#define nod$m_generated	0x00800000
	    unsigned int nod$v_module : 1;
#define nod$m_module	0x01000000
	    unsigned int nod$v_list : 1;
#define nod$m_list	0x02000000
	    unsigned int nod$v_rtl_str_desc : 1;
#define nod$m_rtl_str_desc	0x04000000
	    unsigned int nod$v_complex : 1;
#define nod$m_complex	0x08000000
	    unsigned int nod$v_typedef : 1;
#define nod$m_typedef	0x10000000
	    unsigned int nod$v_declared : 1;
#define nod$m_declared	0x20000000
	    unsigned int nod$v_forward : 1;
#define nod$m_forward	0x40000000
	    unsigned int nod$v_align : 1;
#define nod$m_align	0x80000000
	} nod$r_flagstruc;
    } nod$r_flagunion;
    union
    {
	unsigned int nod$l_flags2;
	int nod$l_fixflags2;
	struct
	{
	    unsigned int nod$v_has_object : 1;
	    unsigned int nod$v_offset_fixed : 1;
	    unsigned int nod$v_length : 1;
	    unsigned int nod$v_hidim : 1;
	    unsigned int nod$v_lodim : 1;
	    unsigned int nod$v_initial : 1;
	    unsigned int nod$v_base_align : 1;
	    unsigned int nod$v_offset_ref : 1;
	} nod$r_flags2struc;
    } nod$r_flags2union;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_naked;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_name;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_return_name;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_prefix;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_marker;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_tag;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_typename;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_maskstr;
#define nod$k_nodesize 346
};

#endif /* _OPENSDL_DEFS_H_ */
