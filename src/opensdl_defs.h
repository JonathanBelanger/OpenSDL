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
 *  V01.000	Aug 24, 2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	Sep  6, 2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
#ifndef _DEFSDL_H_
#define _DEFSDL_H_

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>

#define SDL_K_VERSION_TYPE	'X'
#define SDL_K_VERSION_MAJOR	1
#define SDL_K_VERSION_MINOR	0
#define SDL_K_VERSION_LEVEL	0

/*
 * The following definitions are used to define and maintain queues.
 */
typedef struct
{
    void	*flink;
    void	*blink;
} SDL_QUEUE;

#define SDL_Q_INIT(q)	((q)->flink = (q)->blink = (void *) (q))
#define SDL_Q_EMPTY(q)	(((q)->flink == (q)->blink) && ((q)->flink == (q)))

/*
 * Insert at the end of the queue (blink).
 */
#define SDL_INSQUE(q, e)				\
    SDL_QUEUE	*p = (SDL_QUEUE *) (q)->blink;		\
    (e)->flink = (void *) (q);				\
    (e)->blink = (void *) p;				\
    (q)->blink = p->flink = (void *) e

/*
 * Remove from the beginning of the queue (flink).
 */
#define SDL_REMQUE(q, e)				\
    if (SDL_Q_EMPTY((q)) == false)			\
    {							\
	SDL_QUEUE *n = (SDL_QUEUE *) (q)->flink;	\
	SDL_QUEUE *nn = (SDL_QUEUE *) n->flink;		\
	(e) = (q)->flink;				\
	(q)->flink = n->flink;				\
	nn->blink = (void *) (q);			\
    }							\
    else						\
	(e) = NULL

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
    SDL_QUEUE	header;
    char *id; /* Variable name	*/
    __int64_t value; /* Variable value	*/
} SDL_LOCAL_VARIABLE;

/*
 * The following definition is used to hold a single line of literal text.
 * There is no attempt to interpret what is in the line, it is written, as is
 * to the output file.
 */
typedef struct
{
    SDL_QUEUE	header;
    char	*line;
} SDL_LITERAL;

/*
 * The following definitions are used to maintain single CONSTANT declaration
 */
typedef struct
{
    SDL_QUEUE	header;
    char *id;
    char *prefix;
    char *tag;
    char *comment;
    char *typeName;
    int		radix;
    int type; /* Numeric or String */
    union
    {
	char *string;
	__int64_t	value;
    };
    _Bool	valueSet;
} SDL_CONSTANT;

/*
 * The following definitions are used to declare a set of DECLARE items.
 */
typedef struct
{
    SDL_QUEUE	header;
    char *id;
    char *prefix;
    char *tag;
    int		typeID;
    int		type;
    __int64_t	size;
} SDL_DECLARE;

typedef struct
{
    SDL_QUEUE	header;
    int		nextID;
} SDL_DECLARE_LIST;

/*
 * The following definition is used to declare a single ITEM.
 */
typedef struct
{
    SDL_QUEUE	header;
    char *id;
    char *prefix;
    char *tag;
    char *comment;
    int		typeID;
    int		alignment;
    _Bool	commonDef;
    _Bool	globalDef;
    _Bool	typeDef;
    _Bool	dimension;
    __int64_t	lbound;
    __int64_t	hbound;
    __int64_t	size;
    __int64_t	memSize;		/* Actual space used in memory	*/
    int		type;				/* data or user type	*/
} SDL_ITEM;

/*
 * The following definitions are used to declare a single ENTRY, with zero or
 * one parameter lists.
 */
typedef struct
{
    SDL_QUEUE	header;
    char *name;
    char *comment;
    char *typeName;
    int		type;
    union
    {
	int	data;
	char *aggrName;
    };
    _Bool	in;
    _Bool	out;
    _Bool	optional;
    _Bool	list;
    _Bool	defaultPresent;
    _Bool	dimension;
    __int64_t	bound;
    __int64_t	defaultValue;
    int		radix;
} SDL_PARAMETER;

typedef struct
{
    SDL_QUEUE	header;
    char *id;
    char *comment;
    char *alias;
    char *linkage;
    char *typeName;
    _Bool	variable;
    _Bool	returns;
    SDL_QUEUE	parameters;
    int		type;
} SDL_ENTRY;

typedef struct
{
    SDL_QUEUE	header;
} SDL_ENTRY_LIST;

/*
 * The following definitions are used to declare a single AGGREGATE.
 */
typedef struct
{
    char *id;
    char *prefix;
    char *tag;
    int		type;
    union
    {
	int	data;
	char *name;
    };
    int		alignment;
    _Bool	fill;
    _Bool	dimension;
    __int64_t	lbound;
    __int64_t	hbound;
    __int64_t	offset;
} SDL_ORIGIN;

typedef struct
{
    SDL_QUEUE	header;
    char *id;
    char *prefix;
    char *tag;
    char *marker;
    char *comment;
    char *basedPtrName;
    SDL_ORIGIN	origin;
    int		typeID;
    int		unionType;		/* for implied unions 		*/
    int		alignment;
    int		type;
    SDL_QUEUE	members;
    _Bool	originPresent;
    _Bool	commonDef;
    _Bool	globalDef;
    _Bool	typeDef;
    _Bool	fill;
    _Bool	dimension;
    __int64_t	lbound;
    __int64_t	hbound;
    __int64_t	currentOffset;
    __int64_t	size;
    __int64_t	memSize;		/* Actual space used in memory	*/
    int		currentBitOffset;
} SDL_AGGREGATE;

typedef struct
{
    SDL_QUEUE	header;
    int		type;
    union
    {
	SDL_ITEM    	item;
	SDL_AGGREGATE	subaggr;
    };
} SDL_MEMBERS;

typedef struct
{
    SDL_QUEUE	header;
    int		nextID;
} SDL_AGGREGATE_LIST;

typedef struct
{
    SDL_QUEUE	header;
    int		nextID;
} SDL_ITEM_LIST;

/*
 * The following structure is used to hold dimension data (low and high), as
 * well as a status.
 */
#define SDL_K_MAX_DIMENSIONS	16
typedef struct
{
    size_t	lbound;
    size_t	hbound;
    bool	inUse;
} SDL_DIMENSION;

/*
 * Supported languages
 */
#define SDL_K_LANG_C		0
#define SDL_K_LANG_MAX		1

/*
 * The following definition is used to maintain context during the parsing.
 */
typedef struct
{
    __int64_t	value;
    _Bool	present;
} SDL_IDENT;

typedef struct
{
    char *counter; /* Variable name	*/
    char *prefix;
    char *tag;
    char *typeName;
    __int64_t	value;
    __int64_t	increment;
    int		radix;
} SDL_CONSTANT_LIST;

#define SDL_TIMESTR_LEN		20+1	/* dd-MMM-yyyy hh:mm:ss		*/
#define SDL_K_SUBAGG_MAX	8+1

typedef struct
{
    SDL_AGGREGATE	*aggStack[SDL_K_SUBAGG_MAX];
    _Bool		langSpec[SDL_K_LANG_MAX];
    _Bool		langEna[SDL_K_LANG_MAX];
    char *outFileName[SDL_K_LANG_MAX];
    FILE		*outFP[SDL_K_LANG_MAX];
    SDL_DIMENSION	dimensions[SDL_K_MAX_DIMENSIONS];
    SDL_CONSTANT	**constStack;
    SDL_CONSTANT_LIST	constList;
    SDL_QUEUE		locals;
    SDL_QUEUE		constants;
    SDL_DECLARE_LIST	declares;
    SDL_ITEM_LIST	items;
    SDL_AGGREGATE_LIST	aggregates;
    SDL_ENTRY_LIST	entries;
    char *module;
    char *ident;
    int			aggStackPtr;
    int			constEntries;
} SDL_CONTEXT;

#define SDL_AGGSTACK_EMPTY(p)	((p) == SDL_K_SUBAGG_MAX)
#define SDL_AGGSTACK_FULL(p)	((p) == 0)
#define SDL_AGGSTACK_PUSH(a, s, p)		\
    if (SDL_AGGSTACK_FULL(p) == false)		\
    {						\
	(s)[(p)--] = (a);			\
	1;					\
    }						\
    else					\
	0
#define SDL_AGGSTACK_POP(s, p)			\
    if (SDL_AGGSTACK_EMPTY(p) == false)		\
	(s)[(p)++];				\
    else					\
	NULL

#endif /* __DEFSDL_H__ */
