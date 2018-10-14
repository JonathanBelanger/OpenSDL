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
 *  This source file contains the action routines called during the parsing of
 *  the input file..
 *
 * Revision History:
 *
 *  V01.000	25-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	08-SEP-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 *
 *  V01.002	10-OCT-2018	Jonathan D. Belanger
 *  Added a more complete definition of the possible data type keywords we can
 *  get from the parser.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <complex.h>
#include "opensdl_defs.h"
#include "opensdl_lang.h"
#include "opensdl_utility.h"

extern _Bool trace;

/*
 * Define the language specific entry-points.
 */
#define SDL_K_COMMENT_CB	0
#define SDL_K_MODULE_CB		1
#define SDL_K_END_MODULE_CB	2
#define SDL_K_ITEM_CB		3
#define SDL_K_CONSTANT_CB	4
#define SDL_K_AGGREGATE_CB	5
#define SDL_K_ENTRY_CB		6
#define SDL_K_ENUM_CB		7

static SDL_LANG_FUNC _outputFuncs[SDL_K_LANG_MAX] =
{

    /*
     * For the C/C++ languages.
     */
    {
	(SDL_FUNC) &sdl_c_comment,
	(SDL_FUNC) &sdl_c_module,
	(SDL_FUNC) &sdl_c_module_end,
	(SDL_FUNC) &sdl_c_item,
	(SDL_FUNC) &sdl_c_constant,
	(SDL_FUNC) &sdl_c_aggregate,
	(SDL_FUNC) &sdl_c_entry,
	(SDL_FUNC) &sdl_c_enumerate
    }
};

/*
 * The following defines the default tags for the various data types.
 */
static char *_defaultTag[] =
{
    "K",	/* CONSTANT */
    "B",	/* BYTE */
    "IB",	/* INTEGER_BYTE */
    "W",	/* WORD */
    "IW",	/* INTEGER_WORD */
    "L",	/* LONGWORD */
    "IL",	/* INTEGER_LONG */
    "IS",	/* INTEGER */
    "IH",	/* INTEGER_HW */
    "HI",	/* HARDWARE_INTEGER */
    "Q",	/* QUADWORD */
    "IQ",	/* INTEGER_QUAD */
    "O",	/* OCTAWORD */
    "T",	/* T_FLOATING */
    "TC",	/* T_FLOATING_COMPLEX */
    "S",	/* S_FLOATING */
    "SC",	/* S_FLOATING COMPLEX */
    "X",	/* X_FLOATING */
    "XC",	/* X_FLOATING COMPLEX */
    "F",	/* F_FLOATING */
    "FC",	/* F_FLOATING_COMPLEX */
    "D",	/* D_FLOATING */
    "DC",	/* D_FLOATING COMPLEX */
    "G",	/* G_FLOATING */
    "GC",	/* G_FLOATING_COMPLEX */
    "H",	/* H_FLOATING */
    "HC",	/* H_FLOATING COMPLEX */
    "P",	/* DECIMAL */
    "V",	/* BITFIELD		("M" for mask;	"S" for size) */
    "VB",	/* BITFIELD BYTE	("MB" for mask;	"SB" for size) */
    "VW",	/* BITFIELD WORD	("MW" for mask;	"SW" for size) */
    "VL",	/* BITFIELD LONGWORD 	("ML" for mask;	"SL" for size) */
    "VQ",	/* BITFIELD QUADWORD	("MQ" for mask;	"SQ" for size) */
    "VO",	/* BITFIELD OCTAWORD	("MO" for mask;	"SO" for size) */
    "C",	/* CHAR */
    "CV",	/* CHAR VARYING */
    "CS",	/* CHAR * */
    "A",	/* ADDRESS */
    "AL",	/* ADDRESS_LONG */
    "AQ",	/* ADDRESS QUAD */
    "AH",	/* ADDRESS_HW */
    "HA",	/* HARDWARE_ADDRESS */
    "PS",	/* POINTER */
    "PL",	/* POINTER_LONG */
    "PQ",	/* POINTER_QUAD */
    "PH",	/* POINTER_HW */
    "",		/* ANY */
    "Z",	/* VOID */
    "B",	/* BOOLEAN */
    "R",	/* STRUCTURE */
    "R",	/* UNION */
    "N",	/* ENUM */
    "E",	/* ENTRY */
};

/*
 * Local Prototypes (found at the end of this module).
 */
static int _sdl_aggregate_callback(
			SDL_CONTEXT *context,
			SDL_MEMBERS *member,
			bool ending,
			int depth);
static SDL_DECLARE *_sdl_get_declare(SDL_DECLARE_LIST *declare, char *name);
static SDL_ITEM *_sdl_get_item(SDL_ITEM_LIST *item, char *name);
static char *_sdl_get_tag(
        SDL_CONTEXT *context,
        char *tag,
        int datatype,
        bool lower);
static int64_t _sdl_sizeof(SDL_CONTEXT *context, int item);
static SDL_CONSTANT *_sdl_create_constant(
        char *id,
        char *prefix,
        char *tag,
        char *comment,
        char *typeName,
        int radix,
        int64_t value,
        char *string);
static int _sdl_queue_constant(SDL_CONTEXT *context, SDL_CONSTANT *myConst);
static SDL_ENUMERATE *_sdl_create_enum(
				SDL_CONTEXT *context,
				char *id,
				char *prefix,
				char *tag,
				bool typeDef);
static int _sdl_enum_compl(SDL_CONTEXT *context, SDL_ENUMERATE *myEnum);
static bool _sdl_all_lower(const char *str);
static void _sdl_reset_options(SDL_CONTEXT *context);
static int _sdl_iterate_members(
		SDL_CONTEXT *context,
		SDL_MEMBERS *member,
		void *qhead,
		int (*callback)(),
		int depth,
		int count);

/************************************************************************/
/* Functions called to create definitions from the Grammar file		*/
/************************************************************************/

/*
 * sdl_comment_line
 *  This function is called to output a line comment to the output file.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  comment:
 *  	A pointer to the comment string to be output.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_comment_line(SDL_CONTEXT *context, char *comment)
{
    int	retVal = 1;
    int ii;

    /*
     * Trim all trailing space characters.
     */
    sdl_trim_str(comment, SDL_M_TRAIL);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_comment_line\n", __FILE__, __LINE__);

    /*
     * Loop through all the possible languages and call the appropriate output
     * function for each of the enabled languages.
     */
    for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	if ((context->langSpec[ii] == true) &&
	    (context->langEna[ii] == true) &&
	    (context->commentsOff == false))
	    retVal = (*_outputFuncs[ii][SDL_K_COMMENT_CB])(
				context->outFP[ii],
				&comment[2],	/* Skip past comment token */
				true,
				false,
				false,
				false);

    /*
     * Return the results of this call back to the caller.
     */
    free(comment);
    return(retVal);
}

/*
 * sdl_comment_block
 *  This function is called to output a block comment to the output file.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  comment:
 *  	A pointer to the comment string to be output.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_comment_block(SDL_CONTEXT *context, char *comment)
{
    char	*ptr, *nl;
    int		retVal = 1;
    int 	ii;
    bool	start_comment, start_done = false;
    bool	middle_comment;
    bool	end_comment;

    /*
     * Trim all trailing space characters.
     */
    sdl_trim_str(comment, SDL_M_TRAIL);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_comment_block\n", __FILE__, __LINE__);

    /*
     * Loop through each line of the comment until we reach the end of the
     * comment string.
     */
    ptr = comment;
    while (*ptr != '\0')
    {
	start_comment = false;
	middle_comment = false;
	end_comment = false;
	nl = strchr(ptr, '\n');
	if (nl != NULL)
	{
	    *nl = '\0';
	    nl--;
	    if (*nl == '\r')
		*nl = '\0';
	    nl += 2;
	}
	else
	    nl = &ptr[strlen(ptr)];
	if (ptr[0] == '/')
	{
	    if ((ptr[1] == '+') && (start_done == false))
	    {
		ptr += 2;
		start_comment = true;
		start_done = true;
	    }
	    else if (ptr[1] == '/')
	    {
		ptr += 2;
		middle_comment = true;
	    }
	    else if (ptr[1] == '-')
	    {
		ptr += 2;
		end_comment = true;
	    }
	}
	if (strstr(ptr, "/-") != NULL)
	{
	    size_t	len = strlen(ptr);

	    end_comment = ((ptr[len - 2] == '/') && (ptr[len - 1] == '-'));
	    if (end_comment == true)
		ptr[len - 2] = '\0';
	}

	/*
	 * Loop through all the possible languages and call the appropriate
	 * output function for each of the enabled languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_COMMENT_CB])(
					context->outFP[ii],
					ptr,
					false,
					start_comment,
					middle_comment,
					end_comment);

	/*
	 * Move to the next line.
	 */
	ptr = nl;
    }

    /*
     * Return the results of this call back to the caller.
     */
    free(comment);
    return(retVal);
}

/*
 * sdl_set_local
 *  This function is called to set the value of a local variable.  If the local
 *  variable has not yet been declared, then a new one will be allocated.
 *  Otherwise, the current value will be changed.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the local variables.
 *  name:
 *	A pointer to the name of the local variable.
 *  value:
 *	A 64-bit integer value the local variable it to be set.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 *  -1:	Normal Successful Completion, local variable created.
 */
int sdl_set_local(SDL_CONTEXT *context, char *name, int64_t value)
{
    SDL_LOCAL_VARIABLE	*local = sdl_find_local(context, name);
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
	printf("%s:%d:sdl_set_local(", __FILE__, __LINE__);
	printf(
	    "%s, %ld (%016lx - %4.4s)\n",
	    name,
	    value,
	    value,
	    (char *) &value);
    }

    /*
     * OK, if we did not find a local variable with the same name, then we need
     * to go get one.
     */
    if (local == NULL)
    {
	local = sdl_allocate_block(LocalBlock, NULL);
	if (local != NULL)
	{
	    local->id = name;
	    SDL_INSQUE(&context->locals, &local->header.queue);
	    retVal = -1;
	}
	else
	{
	    free(name);
	    retVal = 0;
	}
    }

    /*
     * If we still have a success, then set the value.
     */
    local->value = value;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_module
 *  This function is called when it gets to the MODULE keyword.  It starts the
 *  definitions.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  moduleName:
 *  	A pointer to the MODULE module-name string to be output.
 *  identString:
 *  	A pointer to the IDENT "ident-string" string to be output.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_module(SDL_CONTEXT *context, char *moduleName, char *identName)
{
    int	retVal = 1;
    int ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_module\n", __FILE__, __LINE__);

    /*
     * Save the MODULE's module-name
     */
    context->module = moduleName;

    /*
     * Save the IDENT's indent-name.
     */
    context->ident = identName;

    /*
     * Loop through all the possible languages and call the appropriate output
     * function for each of the enabled languages.
     */
    for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
	    retVal = (*_outputFuncs[ii][SDL_K_MODULE_CB])(
				context->outFP[ii],
				context);

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_module_end
 *  This function is called when it gets to the END_MODULE keyword.  It end the
 *  definitions.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  moduleName:
 *  	A pointer to the MODULE module-name string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_module_end(SDL_CONTEXT *context, char *moduleName)
{
    int			retVal = 1;
    int			ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_module_end\n", __FILE__, __LINE__);

    /*
     * OK, time to write out the OpenSDL Parser's MODULE footer.
     *
     * Loop through all the possible languages and call the appropriate output
     * function for each of the enabled languages.
     */
    for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
	    retVal = (*_outputFuncs[ii][SDL_K_END_MODULE_CB])(
				context->outFP[ii],
				context);

    /*
     * Reset all the dimension entries.
     */
    for (ii = 0; ii < SDL_K_MAX_DIMENSIONS; ii++)
	context->dimensions[ii].inUse = 0;

    /*
     * Clean out all the local variables.
     */
    ii = 1;
    while(SDL_Q_EMPTY(&context->locals) == false)
    {
	SDL_LOCAL_VARIABLE *local;

	SDL_REMQUE(&context->locals, local);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    Local Variables:\n");
	    printf(
	        "\t%2d: name: %s\n\t    value: %ld\n",
		ii++,
		local->id,
		local->value);
	}
	sdl_deallocate_block(&local->header);
    }

    /*
     * Clean out all the constant definitions.
     */
    ii = 1;
    while(SDL_Q_EMPTY(&context->constants) == false)
    {
	SDL_CONSTANT *constant;

	SDL_REMQUE(&context->constants, constant);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    CONSTANTs:\n");
	    printf(
	        "\t%2d: name: %s\n\t    prefix: %s\n\t    tag: %s\n"
		"\t    typeName: %s\n\t    type: %s\n",
		ii++,
		constant->id,
		(constant->prefix == NULL ? "" : constant->prefix),
		(constant->tag == NULL ? "" : constant->tag),
		(constant->typeName == NULL ? "" : constant->typeName),
		(constant->type == SDL_K_CONST_STR ? "String" : "Number"));
	    if (constant->type == SDL_K_CONST_STR)
		printf("\t    value: %s\n", constant->string);
	    else
	    {
		printf(
		    "\t    value: %ld (%s)\n",
		    constant->value,
		    (constant->radix <= SDL_K_RADIX_DEC ? "Decimal" :
			(constant->radix == SDL_K_RADIX_OCT ? "Octal" :
				(constant->radix == SDL_K_RADIX_HEX ?
					"Hexidecimal" : "Invalid"))));
	    }
	    if (constant->comment != NULL)
		printf(
		    "\t    comment: %s\n",
		    constant->comment);
	}
	sdl_deallocate_block(&constant->header);
    }

    /*
     * Clean out all the enumeration definitions.
     */
    ii = 1;
    while(SDL_Q_EMPTY(&context->enums.header) == false)
    {
	SDL_ENUMERATE	*_enum;
	SDL_ENUM_MEMBER *member;
	int		jj;

	SDL_REMQUE(&context->enums.header, _enum);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    ENUMs:\n");
	    printf(
	        "\t%2d: name: %s\n\t    prefix: %s\n\t    tag: %s\n"
		"\t    _typeDef: %s\n",
		ii++,
		_enum->id,
		(_enum->prefix == NULL ? "" : _enum->prefix),
		(_enum->tag == NULL ? "" : _enum->tag),
		(_enum->typeDef == true ? "True" : "False"));
	}
	jj = 1;
	member = (SDL_ENUM_MEMBER *) _enum->members.flink;
	while (&member->header.queue != &_enum->members)
	{
	    if (trace == true)
	    {
		if (jj == 1)
		    printf("    ENUM_MEMBERs:\n");
		printf(
		    "\t%2d: name: %s\n\t    value: %ld\n\t    valueSet: %s\n",
		    jj++,
		    member->id,
		    member->value,
		    (member->valueSet == true ? "True" : "False"));
		    if (member->comment != NULL)
			printf(
			    "\t    comment: %s\n",
			    member->comment);
	    }
	    member = (SDL_ENUM_MEMBER *) member->header.queue.flink;
	}
	sdl_deallocate_block(&_enum->header);
    }

    /*
     * Clean out all the declares.
     */
    ii = 1;
    while (SDL_Q_EMPTY(&context->declares.header) == false)
    {
	SDL_DECLARE *declare;

	SDL_REMQUE(&context->declares.header, declare);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    DECLAREs:\n");
	    printf(
	        "\t%2d: name: %s\n\t    prefix: %s\n\t    tag: %s\n"
		"\t    typeID: %d\n\t    type: %d\n\t    size: %ld\n",
	        ii++,
	        declare->id,
	        (declare->prefix != NULL ? declare->prefix : ""),
	        (declare->tag != NULL ? declare->tag : ""),
	        declare->typeID,
	        declare->type,
	        declare->size);
	}
	sdl_deallocate_block(&declare->header);
    }

    /*
     * Clean out all the items.
     */
    ii = 1;
    while (SDL_Q_EMPTY(&context->items.header) == false)
    {
	SDL_ITEM *item;

	SDL_REMQUE(&context->items.header, item);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    ITEMs:\n");
	    printf(
	        "\t%2d: name: %s\n\t    prefix: %s\n\t    tag: %s\n"
		"\t    typeID: %d\n\t    alignment: %d\n\t    type: %d\n"
		"\t    size: %ld\n\t    memSize: %ld\n\t    commonDef: %s\n"
		"\t    globalDef: %s\n\t    typeDef: %s\n",
	        ii++,
	        item->id,
	        (item->prefix != NULL ? item->prefix : ""),
	        (item->tag != NULL ? item->tag : ""),
	        item->typeID,
	        item->alignment,
	        item->type,
	        item->size,
	        item->memSize,
	        (item->commonDef == true ? "True" : "False"),
	        (item->globalDef == true ? "True" : "False"),
	        (item->typeDef == true ? "True" : "False"));
	    if (item->dimension == true)
		printf(
		    "\t    dimension: [%ld:%ld]\n",
		    item->lbound,
		    item->hbound);
	}
	sdl_deallocate_block(&item->header);
    }

    /*
     * Clean out all the aggregates.
     */
    ii = 1;
    while (SDL_Q_EMPTY(&context->aggregates.header) == false)
    {
	SDL_AGGREGATE *aggregate;

	SDL_REMQUE(&context->aggregates.header, aggregate);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    AGGREGATEs:\n");
	    printf(
	        "\t%2d: name: %s\n\t    structUnion: %s\n\t    prefix: %s\n"
		"\t    marker: %s\n\t    tag: %s\n\t    origin: %s\n"
		"\t    typeID: %d\n\t    alignment: %d\n\t    type: %d\n"
		"\t    bitOffset: %d\n\t    byteOffset: %ld\n\t    size: %ld\n"
		"\t    memSize: %ld\n\t    commonDef: %s\n"
		"\t    globalDef: %s\n\t    typeDef: %s\n\t    fill: %s\n"
		"\t    _unsigned: %s\n",
	        ii++,
	        aggregate->id,
	        (aggregate->aggType == SDL_K_TYPE_UNION ? "UNION" : "STRUCTURE"),
	        (aggregate->prefix != NULL ? aggregate->prefix : ""),
	        (aggregate->marker != NULL ? aggregate->marker : ""),
	        (aggregate->tag != NULL ? aggregate->tag : ""),
	        (aggregate->origin.id != NULL ? aggregate->origin.id : ""),
	        aggregate->typeID,
	        aggregate->alignment,
	        aggregate->type,
	        aggregate->currentBitOffset,
	        aggregate->currentOffset,
	        aggregate->size,
	        aggregate->memSize,
	        (aggregate->commonDef == true ? "True" : "False"),
	        (aggregate->globalDef == true ? "True" : "False"),
	        (aggregate->typeDef == true ? "True" : "False"),
	        (aggregate->fill == true ? "True" : "False"),
	        (aggregate->_unsigned == true ? "True" : "False"));
	    if (aggregate->dimension == true)
		printf(
		    "\t    dimension: [%ld:%ld]\n",
		    aggregate->lbound,
		    aggregate->hbound);
	}
	if (SDL_Q_EMPTY(&aggregate->members) == false)
	    _sdl_iterate_members(
			context,
			aggregate->members.flink,
			&aggregate->members,
			NULL,
			1,
			1);
	sdl_deallocate_block(&aggregate->header);
    }

    /*
     * Clean out all the entries.
     */
    ii = 1;
    while (SDL_Q_EMPTY(&context->entries) == false)
    {
	SDL_ENTRY	*entry;
	SDL_PARAMETER 	*param;
	int		jj;

	SDL_REMQUE(&context->entries, entry);
	if (trace == true)
	{
	    printf("--------------------------------\n");
	    if (ii == 1)
		printf("    ENTRYs:\n");

	    printf("\t%2d: name: %s\n", ii++, entry->id);
	    if (entry->alias != NULL)
		printf("\t    alias: %s\n", entry->alias);
	    if (entry->typeName != NULL)
		printf("\t    typeName: %s\n", entry->typeName);
	    if (entry->linkage != NULL)
		printf("\t    linkage: %s\n", entry->linkage);
	    printf(
		"\t    returns.type: %d\n\t    returns._unsigned: %s\n",
		entry->returns.type,
		(entry->returns._unsigned ? "True" : "False"));
	    if (entry->returns.name != NULL)
		printf("\t    returns.named: %s\n", entry->returns.name);
	}
	jj = 1;
	param = (SDL_PARAMETER *) entry->parameters.flink;
	while (&param->header.queue != &entry->parameters)
	{
	    if (trace == true)
	    {
		if (jj == 1)
		    printf("    PARAMETERs:\n");
		printf(
		    "\t%2d: name: %s\n\t    type: %d\n\t    typeName: %s\n"
		    "\t    bound: %ld\n\t    defaultValue: %ld\n"
		    "\t    defaultPresent: %s\n\t    dimension: %s\n"
		    "\t    in: %s\n\t    out: %s\n\t    list: %s\n"
		    "\t    optional: %s\n\t    _unsigned: %s\n",
		    jj++,
		    param->name,
		    param->type,
		    param->typeName,
		    param->bound,
		    param->defaultValue,
		    (param->defaultPresent == true ? "True" : "False"),
		    (param->dimension== true ? "True" : "False"),
		    (param->in == true ? "True" : "False"),
		    (param->out == true ? "True" : "False"),
		    (param->list == true ? "True" : "False"),
		    (param->optional == true ? "True" : "False"),
		    (param->_unsigned == true ? "True" : "False"));
	    }
	    param = (SDL_PARAMETER *) param->header.queue.flink;
	}
	sdl_deallocate_block(&entry->header);
    }

    /*
     * Reset the module-name and ident-string to zero length.
     */
    free(context->module);
    context->module = NULL;
    if (context->ident != NULL)
	free(context->ident);
    context->ident = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_literal
 *  This function is called when it gets a line of text within a LITERAL...
 *  END_LITERAL pair.  When the END_LITERAL is reached all the lines will be
 *  dumped out to the file as necessary.
 *
 * Input Parameters:
 *  literals:
 *	A pointer to the queue containing the current set of literal lines.
 *  line:
 *  	A pointer to the line of text from the input file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_literal(SDL_QUEUE *literals, char *line)
{
    SDL_LITERAL	*literalLine;
    int		retVal = 1;
    size_t	len = strlen(line);

    /*
     * Before we go too far, strip ending control characters.
     */
    while (((line[len-1] == '\n') ||
	    (line[len-1] == '\f') ||
	    (line[len-1] == '\r')) &&
	   (len > 0))
	line[--len] = '\0';

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_literal(%.*s)\n", __FILE__, __LINE__, (int) len, line);

    literalLine = sdl_allocate_block(LiteralBlock, NULL);
    if (literalLine != NULL)
    {
	literalLine->line = line;
	SDL_INSQUE(literals, &literalLine->header.queue);
    }
    else
    {
	retVal = 0;
	free(line);
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_literal_end
 *  This function is called when it gets an END_LITERAL.  All the lines saved
 *  since the LITERAL statement will be dumped out to the file(s).
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  literals:
 *	A pointer to the queue containing the current set of literal lines.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_literal_end(SDL_CONTEXT *context, SDL_QUEUE *literals)
{
    SDL_LITERAL	*literalLine;
    int		retVal = 1;
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_literal_end\n", __FILE__, __LINE__);

    /*
     * Keep pulling off literal lines until there are no more.
     */
    while ((SDL_Q_EMPTY(literals) == false) && (retVal == 1))
    {
	SDL_REMQUE(literals, literalLine);

	/*
	 * Loop through each of the supported languages and if we are
	 * generating a file for the language, write out the line to it.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
	    {
		if (fprintf(context->outFP[ii], "%s\n", literalLine->line) < 0)
		    retVal = 0;
	    }

	/*
	 * Free up all the memory we allocated for this literal line.
	 */
	sdl_deallocate_block(&literalLine->header);
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_declare
 *  This function is called to start the creation of a DECLARE record, if one
 *  does not already exist.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer to a string containing the name of the type.
 *  sizeType:
 *	A value indicating the size or datatype of the declaration.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_declare(SDL_CONTEXT *context, char *name, int sizeType)
{
    SDL_DECLARE	*myDeclare = _sdl_get_declare(&context->declares, name);
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_declare\n", __FILE__, __LINE__);

    /*
     * We only create, never update.
     */
    if (myDeclare == NULL)
    {
	myDeclare = sdl_allocate_block(DeclareBlock, NULL);
	if (myDeclare != NULL)
	{
	    myDeclare->id = name;
	    myDeclare->typeID = context->declares.nextID++;
	    if (sizeType < 0)
	    {
		myDeclare->size = -sizeType;
		myDeclare->type = SDL_K_TYPE_CHAR;
	    }
	    else
	    {
		myDeclare->size = _sdl_sizeof(context, sizeType);
		myDeclare->type = sizeType;
	    }
	    SDL_INSQUE(&context->declares.header, &myDeclare->header.queue);
	}
	else
	{
	    retVal = 0;
	    free(name);
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_declare_compl
 *  This function is called to finish creating a DECLARE record.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_declare_compl(SDL_CONTEXT *context)
{
    SDL_DECLARE	*myDeclare = (SDL_DECLARE *) context->declares.header.blink;
    char 	*prefix = NULL;
    char 	*tag = NULL;
    int		ii, retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_declare_compl\n", __FILE__, __LINE__);

    /*
     * Go find our options
     */
    for (ii = 0; ii < context->optionsIdx; ii++)
	if (context->options[ii].option == Prefix)
	{
	    prefix = context->options[ii].string;
	    context->options[ii].string = NULL;
	}
	else if (context->options[ii].option == Tag)
	{
	    tag = context->options[ii].string;
	    context->options[ii].string = NULL;
	}

    /*
     * We only finish creating, never starting new.
     */
    if (myDeclare != NULL)
    {
	myDeclare->prefix = prefix;
	myDeclare->tag = _sdl_get_tag(
		    context,
		    tag,
		    myDeclare->type,
		    _sdl_all_lower(myDeclare->id));
    }

    /*
     * Return the results of this call back to the caller.
     */
    _sdl_reset_options(context);
    return(retVal);
}

/*
 * sdl_item
 *  This function is called to start the creation of an ITEM, save it into the
 *  context.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer the the name of the item to be defined.
 *  datatype:
 *	A value to be associated with the datatype for this item.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_item(SDL_CONTEXT *context, char *name, int datatype)
{
    SDL_ITEM	*myItem = _sdl_get_item(&context->items, name);
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_item\n", __FILE__, __LINE__);

    /*
     * We only complete, never create.
     * TODO: Need to check to make sure we are not defining yet another ITEM
     * TODO: with the same name (NOTE: It has to be completely the same).
     */
    if (myItem == NULL)
    {
	myItem = sdl_allocate_block(ItemBlock, NULL);
	if (myItem != NULL)
	{
	    myItem->id = name;
	    myItem->typeID = context->items.nextID++;
	    if (datatype < 0)
	    {
		myItem->_unsigned = false;
		datatype = -datatype;
	    }
	    else
		myItem->_unsigned = true;
	    myItem->type = datatype;
	    if (datatype == SDL_K_TYPE_DECIMAL)
	    {
		myItem->precision = context->precision;
		myItem->scale = context->scale;
	    }
	    myItem->size = _sdl_sizeof(context, datatype);
	    SDL_INSQUE(&context->items.header, &myItem->header.queue);
	}
    }
    else
    {
	retVal = 0;
	free(name);
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_item_compl
 *  This function is called to finish the creation of an ITEM, and write out
 *  the item to the output file.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_item_compl(SDL_CONTEXT *context)
{
    SDL_ITEM	*myItem = context->items.header.blink;
    char	*prefix = NULL;
    char	*tag = NULL;
    int64_t	addrType = SDL_K_TYPE_NONE;
    int		ii, retVal = 1;
    int		storage = 0;
    int		basealign = 0;
    int		dimension;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_item_compl\n", __FILE__, __LINE__);

    /*
     * Go find our options
     */
    for (ii = 0; ii < context->optionsIdx; ii++)
	switch(context->options[ii].option)
	{
	    case Prefix:
		prefix = context->options[ii].string;
		context->options[ii].string = NULL;
		break;

	    case Tag:
		tag = context->options[ii].string;
		context->options[ii].string = NULL;
		break;

	    case BaseAlign:
		basealign= context->options[ii].value;
		break;

	    case Dimension:
		dimension = context->options[ii].value;
		myItem->dimension = true;
		break;

	    case Common:
		storage |= SDL_M_STOR_COMM;
		break;

	    case Global:
		storage |= SDL_M_STOR_GLOB;
		break;

	    case Typedef:
		storage |= SDL_M_STOR_TYPED;
		break;

	    case SubType:
		addrType= context->options[ii].value;
		break;

	    default:
		break;
	}

    /*
     * We only update, never create.
     */
    if (myItem != NULL)
    {
	myItem->commonDef = (storage & SDL_M_STOR_COMM) == SDL_M_STOR_COMM;
	myItem->globalDef = (storage & SDL_M_STOR_GLOB) == SDL_M_STOR_GLOB;
	myItem->typeDef = (storage & SDL_M_STOR_TYPED) == SDL_M_STOR_TYPED;
	myItem->alignment = basealign;
	if (myItem->dimension == true)
	{
	    myItem->lbound = context->dimensions[dimension].lbound;
	    myItem->hbound = context->dimensions[dimension].hbound;
	    context->dimensions[dimension].inUse = false;
	}
	myItem->prefix = prefix;
	myItem->tag = _sdl_get_tag(
		    context,
		    tag,
		    myItem->type,
		    _sdl_all_lower(myItem->id));

	/*
	 * Addresses can have sub-types.
	 */
	if ((myItem->type == SDL_K_TYPE_ADDR) ||
	    (myItem->type == SDL_K_TYPE_ADDR_L) ||
	    (myItem->type == SDL_K_TYPE_ADDR_Q) ||
	    (myItem->type == SDL_K_TYPE_ADDR_HW) ||
	    (myItem->type == SDL_K_TYPE_HW_ADDR) ||
	    (myItem->type == SDL_K_TYPE_PTR) ||
	    (myItem->type == SDL_K_TYPE_PTR_L) ||
	    (myItem->type == SDL_K_TYPE_PTR_Q) ||
	    (myItem->type == SDL_K_TYPE_PTR_HW))
	    myItem->subType = addrType;

	/*
	 * Loop through all the possible languages and call the appropriate
	 * output function for each of the enabled languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_ITEM_CB])(
					context->outFP[ii],
					myItem,
					context);
    }

    /*
     * Return the results of this call back to the caller.
     */
    _sdl_reset_options(context);
    return(retVal);
}

/*
 * sdl_constant
 *  This function is called to start the definition of one or more constant
 *  values.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  id:
 *	A pointer the the name/names of the constants to be defined.
 *  value:
 *	A value to be associated with the constant.
 *  valueStr:
 *	A pointer to a string to be associated with the constant.  If this is
 *	NULL, then value is used.  Otherwise, this is used.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_constant(
		SDL_CONTEXT *context,
		char *id,
		int64_t value,
		char *valueStr)
{
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_constant\n", __FILE__, __LINE__);

    /*
     * Set up the information needed when we get around to completing the
     * constant definition(s).
     */
    context->constDef.id = id;
    if (valueStr != NULL)
    {
	context->constDef.valueStr = valueStr;
	context->constDef.string = true;
    }
    else
    {
	context->constDef.value = value;
	context->constDef.string = false;
    }

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * sdl_constant_compl
 *  This function is called to complete the definition of one or more constant
 *  values.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
#define _SDL_OUTPUT_COMMENT	0
#define _SDL_COMMA_		2
#define _SDL_COMMENT_LIST_NULL	3
int sdl_constant_compl(SDL_CONTEXT *context)
{
    SDL_CONSTANT	*myConst;
    SDL_ENUMERATE	*myEnum = NULL;
    char 		*id = context->constDef.id;
    int64_t		value = context->constDef.value;
    char		*valueStr = context->constDef.string ?
					context->constDef.valueStr :
					NULL;
    char		*comma = strchr(id, ',');
    static char		*commentList[] = {"/*", "{", ",", NULL};
    int			ii, retVal = 1;
    char		*prefix = NULL;
    char		*tag = NULL;
    char		*counter = NULL;
    char		*typeName = NULL;
    char		*enumName = NULL;
    int64_t		increment = 0;
    int64_t		radix = SDL_K_RADIX_DEF;
    int			datatype = SDL_K_TYPE_CONST;
    bool		incrementPresent = false;
    bool		localCreated = false;
    bool		typeDef = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_constant_compl\n", __FILE__, __LINE__);

    /*
     * Go find our options
     */
    for (ii = 0; ii < context->optionsIdx; ii++)
    {
	switch(context->options[ii].option)
	{
	    case Prefix:
		prefix = context->options[ii].string;
		context->options[ii].string = NULL;
		break;

	    case Tag:
		tag = context->options[ii].string;
		context->options[ii].string = NULL;
		break;

	    case Counter:
		counter = context->options[ii].string;
		context->options[ii].string = NULL;
		localCreated = sdl_set_local(context, counter, value) < 0;
		break;

	    case TypeName:
		typeName = context->options[ii].string;
		context->options[ii].string = NULL;
		break;

	    case Increment:
		increment = context->options[ii].value;
		incrementPresent = true;
		break;

	    case Radix:
		radix = context->options[ii].value;
		break;

	    case Enumerate:
		enumName = context->options[ii].string;
		context->options[ii].string = NULL;
		break;

	    case Typedef:
		typeDef = true;
		break;

	    default:
		break;
	}
    }

    /*
     * Before we go too far, we need to determine what kind of definition
     * we have.  First let's see if there is one or multiple names needing
     * to be created.
     */
    if (comma == NULL)	/* single CONSTANT or ENUM */
    {

	/*
	 * If we are not declaring a string constant and an enum name was
	 * specified, then we are not creating a CONSTANT, but an ENUM.  Adjust
	 * the data-type accordingly.
	 */
	if ((valueStr == NULL) && (enumName != NULL))
	    datatype = SDL_K_TYPE_ENUM;
	if (tag == NULL)
	    tag = _sdl_get_tag(
			context,
			NULL,
			datatype,
			_sdl_all_lower(id));

	/*
	 * OK, we have the tag we need, so if we are really creating a
	 * CONSTANT, then do so now.
	 */
	if ((valueStr != NULL) || (enumName == NULL))
	{
	    myConst = _sdl_create_constant(
				id,
				prefix,
				tag,
				NULL,
				typeName,
				radix,
				value,
				valueStr);
	    if (myConst != NULL)
		retVal = _sdl_queue_constant(context, myConst);
	    else
		retVal = 0;
	}

	/*
	 * Otherwise, we are creating an enumeration (just one).
	 */
	else
	{
	    myEnum = _sdl_create_enum(context, enumName, prefix, tag, typeDef);
	    if (myEnum != NULL)
	    {
		SDL_ENUM_MEMBER *myMem = sdl_allocate_block(
						EnumMemberBlock,
						&myEnum->header);

		if (myMem != NULL)
		{

		    /*
		     * Initialize the lone enumeration member and queue it up.
		     */
		    myMem->id = id;
		    myMem->value = value;
		    myMem->valueSet = value != 0;
		    SDL_INSQUE(&myEnum->members, &myMem->header.queue);
		}
		else
		{
		    free(myEnum);
		    myEnum = NULL;
		    retVal = 0;
		}
	    }
	    else
		retVal = 0;
	}
    }
    else	/* list of CONSTANTs or ENUMs */
    {
	char 		*ptr = id;
	char		*nl;
	int64_t	prevValue = value;
	bool		freeTag = tag == NULL;
	bool		done = false;

	/*
	 * If an enum name was specified, then we are not creating a CONSTANT,
	 * but an ENUM.  Adjust the data-type accordingly.  Also, allocate and
	 * initialize the ENUM parent.
	 */
	if (enumName != NULL)
	{
	    datatype = SDL_K_TYPE_ENUM;
	    myEnum = _sdl_create_enum(context, enumName, prefix, tag, typeDef);
	    if (myEnum == NULL)
		retVal = 0;
	}

	sdl_trim_str(ptr, SDL_M_LEAD);
	while ((done == false) && (retVal == 1))
	{
	    char	*name = ptr;
	    char	*comment;

	    ii = 0;
	    while (commentList[ii] != NULL)
	    {
		comment = strstr(name, commentList[ii]);
		if ((comment != NULL) || (ii == _SDL_COMMA_))
		{
		    if ((comment != NULL) && (*comment != ','))
		    {
			comment += strlen(commentList[ii]);
			nl = strchr(comment, '\n');
			if (nl != NULL)
			{
			    ptr = nl + 1;
			    if (ii == _SDL_OUTPUT_COMMENT)
				*nl = '\0';	/* Null-terminate comment */
			    else
				comment = NULL; /* Local comment, ignore */
			}
			else
			    ptr = strchr(comment, '\0');
		    }
		    else
			comment = NULL;
		    nl = name;
		    while (isalnum(*nl) || (*nl == '_') || (*nl == '$'))
			nl++;
		    if (ii == _SDL_COMMA_)
			ptr = (*nl == '\0') ? nl : (nl + 1);
		    *nl = '\0';		/* Null-terminate name */
		    ii = _SDL_COMMENT_LIST_NULL;
		}
		else
		    ii++;
	    }
	    if (strlen(name) > 0)
	    {
		if (myEnum == NULL)
		{
		    if (freeTag == true)
			tag = _sdl_get_tag(
				    context,
				    NULL,
				    datatype,
				    _sdl_all_lower(id));
		    myConst = _sdl_create_constant(
					    name,
					    prefix,
					    tag,
					    comment,
					    typeName,
					    radix,
					    value,
					    NULL);
		    if (myConst != NULL)
			retVal = _sdl_queue_constant(context, myConst);
		    else
			retVal = 0;
		    if (freeTag == true)
		    {
			free(tag);
			tag = NULL;
		    }
		}
		else
		{
		    SDL_ENUM_MEMBER *myMem = sdl_allocate_block(
							EnumMemberBlock,
							&myEnum->header);

		    if (myMem != NULL)
		    {
			myMem->id = id;
			myMem->value = value;
			myMem->valueSet = (value - prevValue) != 1;
			SDL_INSQUE(&myEnum->members, &myMem->header.queue);
		    }
		}
	    }
	    if ((retVal == 1) && (counter != NULL) && (prevValue != value))
	    {
		retVal = sdl_set_local(context, counter, value);
		prevValue = value;
	    }
	    if (incrementPresent == true)
		value += increment;
	    sdl_trim_str(ptr, SDL_M_LEAD);
	    done = *ptr == '\0';
	}
    }

    /*
     * If we created an enumeration, then go complete it (this will output it
     * to the language output files).
     */
    if ((retVal == 1) && (myEnum != NULL))
	retVal = _sdl_enum_compl(context,myEnum);

    /*
     * Deallocate all the memory.
     */
    if (id != NULL)
	free(id);
    if (prefix != NULL)
	free(prefix);
    if (tag != NULL)
	free(tag);
    if ((counter != NULL) && (localCreated == false))
	free(counter);
    if (typeName != NULL)
	free(typeName);
    if (enumName != NULL)
	free(enumName);

    /*
     * Return the results of this call back to the caller.
     */
    _sdl_reset_options(context);
    return (retVal);
}

/*
 * sdl_aggregate
 *  This function is called to create the AGGREGATE structure that will contain
 *  all the information about the aggregate being defined.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer to the string to be associated with this aggregate
 *	definition.
 *  datatype:
 *	A value to be associated with the datatype for this item.
 *  aggType:
 *	A value indicating if this AGGREGATE is for a UNION or STRUCTURE
 *	definition.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_aggregate(
	SDL_CONTEXT *context,
	char *name,
	int64_t datatype,
	int aggType)
{
    SDL_AGGREGATE	*myAggr = sdl_allocate_block(AggregateBlock, NULL);
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_aggregate\n", __FILE__, __LINE__);

    if (myAggr != NULL)
    {
	myAggr->id = name;
	myAggr->typeID = context->aggregates.nextID++;
	if (datatype < 0)
	{
	    myAggr->_unsigned = false;
	    datatype = -datatype;
	}
	else if (datatype > 0)
	    myAggr->_unsigned = true;
	myAggr->type = datatype;
	myAggr->aggType = aggType;
	myAggr->tag = _sdl_get_tag(
				context,
				NULL,
				SDL_K_TYPE_STRUCT,
				_sdl_all_lower(name));
	SDL_Q_INIT(&myAggr->members);
	SDL_INSQUE(&context->aggregates.header, &myAggr->header.queue);
	context->currentAggr = myAggr;
	context->aggregateDepth++;
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * sdl_aggregate_member
 *  This function is called to define a member in an AGGREGATE definition.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer to the string containing the name for the AGGREGATE being
 *	defined.
 *  datatype:
 *	An integer indicating either a base type, user type, or another
 *	aggregate.
 *  aggType:
 *	A value indicating the type of subaggregate that is being requested to
 *	be created.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_aggregate_member(
		SDL_CONTEXT *context,
		char *name,
		int64_t datatype,
		int aggType)
{
    SDL_MEMBERS		*myMember = NULL;
    SDL_AGGREGATE	*myAggr = (context->aggregateDepth > 1 ?
					NULL :
					(SDL_AGGREGATE *) context->currentAggr);
    SDL_SUBAGGR		*mySubAggr = (context->aggregateDepth > 1 ?
					(SDL_SUBAGGR *) context->currentAggr :
					NULL);
    int64_t		subType = SDL_K_TYPE_BYTE;
    int64_t		length = 0;
    int			retVal = 1;
    bool		mask = false;
    bool		_signed = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_aggregate_member(%d)\n", __FILE__, __LINE__, aggType);

    /*
     * Before we go too far, there may have been one or more options defined to
     * the previous member/aggregate/subaggregate.
     */
    if (context->optionsIdx > 0)
    {
	int64_t	dimension;
	int		ii;

	/*
	 * Determine if the previous item we worked on was an ITEM, an
	 * AGGREGATE, or a subaggregate.  If ITEM, that is the one that needs
	 * to be updated with any saved options.  Otherwise, we already have
	 * the thing that needs to have these options applied to it.
	 */
	if (mySubAggr != NULL)
	{
	    if (SDL_Q_EMPTY(&mySubAggr->members) == false)
		myMember = (SDL_MEMBERS *) mySubAggr->members.blink;
	}
	else
	{
	    if (SDL_Q_EMPTY(&myAggr->members) == false)
		myMember = (SDL_MEMBERS *) myAggr->members.blink;
	}

	/*
	 * If the current member is not a STRUCTURE or UNION, then it is
	 * the current aggregate, of which we already have the address.
	 */
	if ((myMember != NULL) &&
	    ((myMember->type == SDL_K_TYPE_STRUCT) ||
	     (myMember->type == SDL_K_TYPE_UNION)))
	    myMember = NULL;

	/*
	 * Go find our options
	 */
	for (ii = 0; ii < context->optionsIdx; ii++)
	    switch (context->options[ii].option)
	    {

		/*
		 * Present only options.
		 */
		case Align:
		    if (myMember != NULL)
			myMember->item.alignment = SDL_K_ALIGN;
		    else if (mySubAggr != NULL)
			mySubAggr->alignment = SDL_K_ALIGN;
		    else
			myAggr->alignment = SDL_K_ALIGN;
		    break;

		case Common:
		    if ((myAggr != NULL) && (myMember == NULL))
			myAggr->commonDef = true;
		    break;

		case Fill:
		    if (myMember != NULL)
			myMember->item.fill = true;
		    else if (mySubAggr != NULL)
			mySubAggr->fill = true;
		    else
			myAggr->fill = true;
		    break;

		case Global:
		    if ((myAggr != NULL) && (myMember == NULL))
			myAggr->globalDef = true;
		    break;

		case Mask:
			mask = true;
		    break;

		case NoAlign:
		    if (myMember != NULL)
			myMember->item.alignment = SDL_K_NOALIGN;
		    else if (mySubAggr != NULL)
			mySubAggr->alignment = SDL_K_NOALIGN;
		    else
			myAggr->alignment = SDL_K_NOALIGN;
		    break;

		case Typedef:
		    if (myMember != NULL)
			myMember->item.typeDef = true;
		    else if (mySubAggr != NULL)
			mySubAggr->typeDef = true;
		    else
			myAggr->typeDef = true;
		    break;

		case Signed:
			_signed = true;
		    break;

		/*
		 * String options.
		 */
		case Based:
		    if ((myAggr != NULL) && (myMember == NULL))
			myAggr->basedPtrName = context->options[ii].string;
		    else
			free(context->options[ii].string);
		    context->options[ii].string = NULL;
		    break;

		case Marker:
		    if ((mySubAggr != NULL) && (myMember == NULL))
			mySubAggr->marker = context->options[ii].string;
		    else if (myMember == NULL)
			myAggr->marker = context->options[ii].string;
		    context->options[ii].string = NULL;
		    break;

		case Origin:
		    if ((myAggr != NULL) && (myMember == NULL))
		    {
			myAggr->origin.id = context->options[ii].string;
			myAggr->originPresent = true;
		    }
		    else
			free(context->options[ii].string);
		    context->options[ii].string = NULL;
		    break;

		case Prefix:
		    if (myMember != NULL)
			myMember->item.prefix = context->options[ii].string;
		    else if (mySubAggr != NULL)
			mySubAggr->prefix = context->options[ii].string;
		    else
			myAggr->prefix = context->options[ii].string;
		    context->options[ii].string = NULL;
		    break;

		case Tag:
		    if (myMember != NULL)
		    {
			if (myMember->item.tag != NULL)
			    free(myMember->item.tag);
			myMember->item.tag = context->options[ii].string;
		    }
		    else if (mySubAggr != NULL)
		    {
			if (mySubAggr->tag != NULL)
			    free(mySubAggr->tag);
			mySubAggr->tag = context->options[ii].string;
		    }
		    else
		    {
			if (myAggr->tag != NULL)
			    free(myAggr->tag);
			myAggr->tag = context->options[ii].string;
		    }
		    context->options[ii].string = NULL;
		    break;

		/*
		 * Numeric options.
		 */
		case BaseAlign:
		    if (myMember != NULL)
			myMember->item.alignment = context->options[ii].value;
		    else if (mySubAggr != NULL)
			mySubAggr->alignment = context->options[ii].value;
		    else
			myAggr->alignment = context->options[ii].value;
		    break;

		case Dimension:
		    dimension = context->options[ii].value;
		    if (myMember != NULL)
		    {
			myMember->item.lbound =
				context->dimensions[dimension].lbound;
			myMember->item.hbound =
				context->dimensions[dimension].hbound;
			myMember->item.dimension = true;
		    }
		    else if (mySubAggr != NULL)
		    {
			mySubAggr->lbound =
				context->dimensions[dimension].lbound;
			mySubAggr->hbound =
				context->dimensions[dimension].hbound;
			mySubAggr->dimension = true;
		    }
		    else
		    {
			myAggr->lbound = context->dimensions[dimension].lbound;
			myAggr->hbound = context->dimensions[dimension].hbound;
			myAggr->dimension = true;
		    }
		    context->dimensions[dimension].inUse = false;
		    break;

		case Length:
		    length = context->options[ii].value;
		    break;

		case SubType:
		    subType = context->options[ii].value;
		    break;

		default:
		    break;
	    }

	/*
	 * We have what we want, reset the options list for the member we are
	 * about to start.
	 */
	_sdl_reset_options(context);
    }

    /*
     * OK, we took care of adding the options to our predecessor, so now we
     * need to start a new member.
     */
    myMember = sdl_allocate_block(
			AggrMemberBlock,
			(myAggr != NULL ?
				&myAggr->header :
				&mySubAggr->parent->header));
    if (myMember != NULL)
    {

	/*
	 * Determine the type of member we are being asked to create.
	 */
	myMember->type = aggType;
	switch (aggType)
	{
	    case SDL_K_TYPE_STRUCT:
	    case SDL_K_TYPE_UNION:
		myMember->subaggr.id = name;
		myMember->subaggr.aggType = aggType;
		if (datatype < 0)
		{
		    myMember->subaggr._unsigned = false;
		    datatype = -datatype;
		}
		else if (datatype > 0)
		    myMember->subaggr._unsigned = true;
		myMember->subaggr.type = datatype;
		myMember->subaggr.parent = context->currentAggr;
		myMember->subaggr.tag = _sdl_get_tag(
						context,
						NULL,
						SDL_K_TYPE_STRUCT,
						_sdl_all_lower(name));
		SDL_Q_INIT(&myMember->subaggr.members);
		context->aggregateDepth++;
		context->currentAggr = &myMember->subaggr;
		break;

	    default:
		myMember->item.id = name;
		if (datatype < 0)
		{
		    myMember->item._unsigned = false;
		    datatype = -datatype;
		}
		else if (datatype > 0)
		    myMember->item._unsigned = true;
		myMember->item.type = datatype;
		switch (datatype)
		{
		    case SDL_K_TYPE_DECIMAL:
			myMember->item. precision = context->precision;
			myMember->item.scale = context->scale;
			break;

		    /*
		     * TODO: When mask is set, we need to define a constant
		     * TODO: with the correct bit(s) set.
		     */
		    case SDL_K_TYPE_BITFLD:
			myMember->item.length = (length == 0 ? 1 : length);
			myMember->item.mask = mask;
			myMember->item._signed = _signed;
			myMember->item.subType = subType;
			break;

		    case SDL_K_TYPE_ADDR:
		    case SDL_K_TYPE_ADDR_L:
		    case SDL_K_TYPE_ADDR_Q:
		    case SDL_K_TYPE_ADDR_HW:
		    case SDL_K_TYPE_HW_ADDR:
		    case SDL_K_TYPE_PTR:
		    case SDL_K_TYPE_PTR_L:
		    case SDL_K_TYPE_PTR_Q:
		    case SDL_K_TYPE_PTR_HW:
			myMember->item.subType = subType;
			break;
		}
		myMember->item.tag = _sdl_get_tag(
						context,
						NULL,
						datatype,
						_sdl_all_lower(name));
		myMember->item.size = _sdl_sizeof(context, datatype);
		break;
	}
	if (mySubAggr != NULL)
	{
	    SDL_INSQUE(&mySubAggr->members, &myMember->header.queue);
	}
	else
	{
	    SDL_INSQUE(&myAggr->members, &myMember->header.queue);
	}
    }
    else
	retVal = 0;

   /*
    * Return the results of this call back to the caller.
    */
   return (retVal);
}

/*
 * sdl_aggregate_compl
 *  This function is called to complete the definition of an aggregate
 *  definition.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer to the string to be associated with this aggregate
 *	definition.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_aggregate_compl(SDL_CONTEXT *context, char *name)
{
    SDL_AGGREGATE	*myAggr = (SDL_AGGREGATE *) context->currentAggr;
    SDL_SUBAGGR		*mySubAggr = (SDL_SUBAGGR *) context->currentAggr;
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_aggregate_compl\n", __FILE__, __LINE__);

    /*
     * If we have any options that had been processed, they are for the most
     * recent ITEM member.
     */
    if (context->optionsIdx > 0)
    {
	SDL_MEMBERS	*myMember = NULL;
	int64_t		dimension;
	int		ii;

	/*
	 * OK, the issue is where is the most recent ITEM member.  It is in the
	 * current aggregate.  Don't assume that the queues actually contain
	 * anything, and make sure that the member is actually not a
	 * subaggregate.
	 */
	if (context->aggregateDepth == 1)
	{
	    if (SDL_Q_EMPTY(&myAggr->members) == false)
		myMember = myAggr->members.blink;
	}
	else
	{
	    if (SDL_Q_EMPTY(&mySubAggr->members) == false)
		myMember = mySubAggr->members.blink;
	}
	if ((myMember != NULL) &&
	    ((myMember->type != SDL_K_TYPE_STRUCT) ||
	     (myMember->type != SDL_K_TYPE_UNION)))
	    myMember = NULL;
	for (ii = 0; ii < context->optionsIdx; ii++)
	    switch (context->options[ii].option)
	    {

		/*
		 * Present only options.
		 */
		case Align:
		    if (myMember != NULL)
			myMember->item.alignment = SDL_K_ALIGN;
		    break;

		case Fill:
		    if (myMember != NULL)
			myMember->item.fill = true;
		    break;

		case Mask:
		    if (myMember != NULL)
			myMember->item.mask = true;
		    break;

		case NoAlign:
		    if (myMember != NULL)
			myMember->item.alignment = SDL_K_NOALIGN;
		    break;

		case Signed:
		    if (myMember != NULL)
			myMember->item._signed = true;
		    break;

		case Typedef:
		    if (myMember != NULL)
			myMember->item.typeDef = true;
		    break;

		/*
		 * String options.
		 */
		case Prefix:
		    if (myMember != NULL)
			myMember->item.prefix = context->options[ii].string;
		    else
			free(context->options[ii].string);
		    context->options[ii].string = NULL;
		    break;

		case Tag:
		    if (myMember != NULL)
			myMember->item.tag = context->options[ii].string;
		    else
			free(context->options[ii].string);
		    context->options[ii].string = NULL;
		    break;

		/*
		 * Numeric options.
		 */
		case BaseAlign:
		    if (myMember != NULL)
			myMember->item.alignment = context->options[ii].value;
		    break;

		case Dimension:
		    dimension = context->options[ii].value;
		    if (myMember != NULL)
		    {
			myMember->item.lbound =
				context->dimensions[dimension].lbound;
			myMember->item.hbound =
				context->dimensions[dimension].hbound;
			myMember->item.dimension = true;
		    }
		    context->dimensions[dimension].inUse = false;
		    break;

		case Length:
		    if (myMember != NULL)
			myMember->item.length = context->options[ii].value;
		    break;

		default:
		    break;
	    }

	/*
	 * We have what we want, reset the options list for the member we are
	 * about to start.
	 */
	_sdl_reset_options(context);
    }

    /*
     * We are completing either a subaggregate or an AGGREGATE.  Decrement the
     * depth.
     */
    context->aggregateDepth--;

    /*
     * If we are at zero, then we have completed the entire AGGREGATE
     * definition and can now write out the results.
     */
    if (context->aggregateDepth == 0)
    {
	SDL_AGGREGATE	*myAggr =
			    (SDL_AGGREGATE *) context->aggregates.header.blink;
	int ii;

	context->currentAggr = NULL;

	/*
	 * Loop through all the possible languages and call the appropriate
	 * output function for each of the enabled languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_AGGREGATE_CB])(
					context->outFP[ii],
					myAggr,
					LangAggregate,
					false,
					0,
					context);
	if (SDL_Q_EMPTY(&myAggr->members) == false)
	    retVal = _sdl_iterate_members(
			context,
			myAggr->members.flink,
			&myAggr->members,
			(int (*)()) &_sdl_aggregate_callback,
			1,
			1);

	/*
	 * Loop through all the possible languages and call the appropriate
	 * output function for each of the enabled languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_AGGREGATE_CB])(
					context->outFP[ii],
					myAggr,
					LangAggregate,
					true,
					0,
					context);
    }

    /*
     * We just closed a subaggregate.  Make the previous subaggregate the
     * current one.
     */
    else
	context->currentAggr = mySubAggr->parent;

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * sdl_entry
 *  This function is called to create the ENTRY structure that will contain
 *  all the information about the function/procedure definition being defined.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer to the string to be associated with this entry definition.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_entry(SDL_CONTEXT *context, char *name)
{
    SDL_ENTRY	*myEntry = sdl_allocate_block(EntryBlock, NULL);
    int		retVal = 1;
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_entry\n", __FILE__, __LINE__);

    if (myEntry != NULL)
    {
	myEntry->id = name;
	SDL_Q_INIT(&myEntry->parameters);
	for (ii = 0; ii < context->optionsIdx; ii++)
	{
	    switch(context->options[ii].option)
	    {
		case Alias:
		    myEntry->alias = context->options[ii].string;
		    context->options[ii].string = NULL;
		    break;

		case Linkage:
		    myEntry->linkage = context->options[ii].string;
		    context->options[ii].string = NULL;
		    break;

		case TypeName:
		    myEntry->typeName = context->options[ii].string;
		    context->options[ii].string = NULL;
		    break;

		case Variable:
		    myEntry->variable = true;
		    break;

		case ReturnsType:
		    myEntry->returns.type = context->options[ii].value;
		    if (myEntry->returns.type >= 0)
			myEntry->returns._unsigned = true;
		    else
			myEntry->returns.type = -myEntry->returns.type;
		    break;

		case ReturnsNamed:
		    myEntry->returns.name = context->options[ii].string;
		    context->options[ii].string = NULL;
		    break;

		default:
		    break;
	    }
	}
	for (ii = 0; ii < context->parameterIdx; ii++)
	{
	    SDL_PARAMETER *myParam = context->parameters[ii];

	    context->parameters[ii] = NULL;
	    myParam->header.parent = &myEntry->header;
	    SDL_INSQUE(&myEntry->parameters, &myParam->header.queue);
	}
	context->parameterIdx = 0;
	SDL_INSQUE(&context->entries, &myEntry->header.queue);

	/*
	 * Loop through all the possible languages and call the appropriate
	 * output function for each of the enabled languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true) &&
		(context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_ENTRY_CB])(
					context->outFP[ii],
					myEntry,
					context);
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    _sdl_reset_options(context);
    return (retVal);
}

/*
 * sdl_add_parameter
 *  This function is called when a parameter needs to be added to an entry.
 *  We do this in two passes.  The first creates the parameter records, then
 *  when creating the ENTRY record, these are copied into there and the array
 *  cleaned up.  The list in the context block is used as an array of pointers
 *  to PARAMETER definitions.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  datatype:
 *	An integer indicating either a base type or a user type.  If a base,
 *	get the default.  If a user, we may need to call ourselves again to
 *	get what we came to get.
 *  passing:
 *	A value indicating how a parameter is passed (by reference or value).
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_add_parameter(SDL_CONTEXT *context, int datatype, int passing)
{
    int		retVal = 1;
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_add_parameter\n", __FILE__, __LINE__);

    /*
     * If the stack is full, reallocate a larger stack.
     */
    if (context->parameterIdx >= context->parameterSize)
    {
	size_t	size;

	context->parameterSize += SDL_K_OPTIONS_INCR;
	size = context->parameterSize * sizeof(SDL_PARAMETER *);
	context->parameters = realloc(context->parameters, size);
    }

    if (context->parameters != NULL)
    {
	SDL_PARAMETER	*param = sdl_allocate_block(ParameterBlock, NULL);

	if (datatype < 0)
	    param->type = -datatype;
	else
	{
	    param->type = datatype;
	    param->_unsigned = true;
	}
	param->passingMech = passing;

	for (ii = 0; ii < context->optionsIdx; ii++)
	{
	    switch (context->options[ii].option)
	    {
		case In:
		    param->in = true;
		    context->options[ii].option = None;;
		    break;

		case Out:
		    param->out = true;
		    context->options[ii].option = None;;
		    break;

		case Named:
		    param->name = context->options[ii].string;
		    context->options[ii].option = None;;
		    context->options[ii].string = NULL;
		    break;

		case Dimension:
		    param->bound = context->options[ii].value;
		    param->dimension = true;
		    context->options[ii].option = None;;
		    break;

		case Default:
		    param->defaultValue = context->options[ii].value;
		    param->defaultPresent = true;
		    context->options[ii].option = None;;
		    break;

		case TypeName:
		    param->typeName = context->options[ii].string;
		    context->options[ii].option = None;;
		    context->options[ii].string = NULL;
		    break;

		case Optional:
		    param->optional = true;
		    context->options[ii].option = None;;
		    break;

		case List:
		    param->list = true;
		    context->options[ii].option = None;;
		    break;

		default:
		    break;
	    }
	}
	context->parameters[context->parameterIdx++] = param;
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/************************************************************************/
/* Local Functions							*/
/************************************************************************/

/*
 * _sdl_aggregate_callback
 *  This function is called to iterate through each of the languages for a
 *  particular member of an AGGREGATE.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  member:
 *	A pointer to the member item needing to be written out.
 *  ending:
 *      A boolean value indicating that we are ending a subaggregate.
 *  depth:
 *	A value indicating the depth of the member.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
static int _sdl_aggregate_callback(
			SDL_CONTEXT *context,
			SDL_MEMBERS *member,
			bool ending,
			int depth)
{
    void		*param;
    SDL_LANG_AGGR_TYPE	type;
    int			retVal = 1;
    int			ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_aggregate_callback\n", __FILE__, __LINE__);

    if ((member->type == SDL_K_TYPE_STRUCT) ||
	(member->type == SDL_K_TYPE_UNION))
    {
	param = (void *) &member->subaggr;
	type = LangSubaggregate;
    }
    else
    {
	param = (void *) &member->item;
	type = LangItem;
    }

    /*
     * Loop through all the possible languages and call the appropriate
     * output function for each of the enabled languages.
     */
    for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
	    retVal = (*_outputFuncs[ii][SDL_K_AGGREGATE_CB])(
					context->outFP[ii],
					param,
					type,
					ending,
					depth,
					context);

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * _sdl_get_declare
 *  This function is called to get the record of a previously defined local
 *  variable.  If the local variable has not yet been declared, then a NULL
 *  will be returned.  Otherwise, the a pointer to the found DECLARE will be
 *  returned to the caller.
 *
 * Input Parameters:
 *  declare:
 *	A pointer to the list containing all currently defined DECLAREs.
 *  name:
 *	A pointer to the name of the declared user-type.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
static SDL_DECLARE *_sdl_get_declare(SDL_DECLARE_LIST *declare, char *name)
{
    SDL_DECLARE	*retVal = (SDL_DECLARE *) declare->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_get_declare\n", __FILE__, __LINE__);

    while (retVal != (SDL_DECLARE *) &declare->header)
	if (strcmp(retVal->id, name) == 0)
	    break;
	else
	    retVal = (SDL_DECLARE *) retVal->header.queue.flink;
    if (retVal == (SDL_DECLARE *) &declare->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_get_item
 *  This function is called to get the record of a previously defined ITEM.  If
 *  the ITEM has not yet been defined, then a NULL will be returned.
 *  Otherwise, the a pointer to the found ITEM will be returned to the caller.
 *
 * Input Parameters:
 *  item:
 *	A pointer to the list containing all currently defined ITEMs.
 *  name:
 *	A pointer to the name of the declared item.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	ITEM not found.
 *  !NULL:	An existing ITEM.
 */
static SDL_ITEM *_sdl_get_item(SDL_ITEM_LIST *item, char *name)
{
    SDL_ITEM	*retVal = (SDL_ITEM *) item->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_get_item\n", __FILE__, __LINE__);

    while (retVal != (SDL_ITEM *) &item->header)
	if (strcmp(retVal->id, name) == 0)
	    break;
	else
	    retVal = (SDL_ITEM *) retVal->header.queue.flink;
    if (retVal == (SDL_ITEM *) &item->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_get_tag
 *  This function is used to determine the tag that should be used for a
 *  particular definition.  The user can specify a tag, which will be used
 *  instead of the default.  If one is not specified, then we need to determine
 *  what default tag should be used.  This is not necessarily as
 *  straightforward as you'd think.  Because a definition can be based off of a
 *  previous definition, which, in turn, can be based off of yet another
 *  definition, we need to loop until we find a base type to be utilized.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  tag:
 *	A pointer to a string containing the user specified tag (or NULL).
 *  datatype:
 *	An integer indicating either a base type or a user type.  If a base,
 *	get the default.  If a user, we may need to call ourselves again to
 *	get what we came to get.
 *  lower:
 *	A boolean value to indicate that the defaulted tag should be lower
 *	case.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  A pointer to a user specified tag or a default tag.
 */
static char *_sdl_get_tag(
		SDL_CONTEXT *context,
		char *tag,
		int datatype,
		bool lower)
{
    char	*retVal = tag;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_get_tag\n", __FILE__, __LINE__);

    /*
     * If the user specified one, then we are done here.
     */
    if (tag == NULL)
    {

	/*
	 * If the datatype is a base type, then return then get the default
	 * tag for this type.
	 */
	if (datatype == SDL_K_TYPE_CONST)
	    retVal = strdup(_defaultTag[SDL_K_TYPE_CONST]);
	else if ((datatype >= SDL_K_BASE_TYPE_MIN) &&
		 (datatype <= SDL_K_BASE_TYPE_MAX))
	    retVal = strdup(_defaultTag[datatype]);
	else
	{

	    /*
	     * OK, the datatype is not a base type.  So it is either a DECLARE,
	     * ITEM, or AGGREGATE type.  Therefore we need to go figure out
	     * which it is and see if there is either a TAG defined there or
	     * it is a base type.
	     */
	    if ((datatype >= SDL_K_DECLARE_MIN) &&
		(datatype <= SDL_K_DECLARE_MAX))
	    {
		SDL_DECLARE *myDeclare = sdl_get_declare(
		        &context->declares,
		        datatype);

		if (myDeclare != NULL)
		{
		    if (strlen(myDeclare->tag) > 0)
			retVal = strdup(myDeclare->tag);
		    else
			retVal = _sdl_get_tag(
			        context,
			        tag,
			        myDeclare->typeID,
			        lower);
		}
		else
		    retVal = strdup(_defaultTag[SDL_K_TYPE_ANY]);
	    }
	    else if ((datatype >= SDL_K_ITEM_MIN) &&
		     (datatype <= SDL_K_ITEM_MAX))
	    {
		SDL_ITEM *myItem = sdl_get_item(&context->items, datatype);

		if (myItem != NULL)
		{
		    if (strlen(myItem->tag) > 0)
			retVal = strdup(myItem->tag);
		    else
			retVal = _sdl_get_tag(
			        context,
			        tag,
			        myItem->typeID,
			        lower);
		}
		else
		    retVal = strdup(_defaultTag[SDL_K_TYPE_ANY]);
	    }
	    else if ((datatype >= SDL_K_AGGREGATE_MIN) &&
		     (datatype <= SDL_K_AGGREGATE_MAX))
	    {
		SDL_AGGREGATE *myAggregate =
			sdl_get_aggregate(&context->aggregates, datatype);

		if (myAggregate != NULL)
		{
		    if (strlen(myAggregate->tag) > 0)
			retVal = strdup(myAggregate->tag);
		    else
			retVal = _sdl_get_tag(
					context,
					tag,
					myAggregate->typeID,
					lower);
		}
		else
		    retVal = strdup(_defaultTag[SDL_K_TYPE_ANY]);
	    }
	}
	if (lower == true)
	{
	    int ii, len = strlen(retVal);

	    for (ii = 0; ii < len; ii++)
		retVal[ii] = tolower(retVal[ii]);
	}
    }
    else
    {
	size_t len = strlen(retVal);
	size_t ii;
	_Bool done = false;

	/*
	 * Start at the end of the tag string and if the last character is an
	 * underscore, then change it to a null character.  Then check the next
	 * until we come across something other than an underscore or we run out
	 * of characters to check.
	 */
	for (ii = len - 1; ((ii > 0) && (done == false)); ii--)
	{
	    if (retVal[ii] == '_')
		retVal[ii] = '\0';
	    else
		done = true;
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * _sdl_sizeof
 *  This function is called to return the length of an item, based on the item
 *  indicated.  The item can be either a base type, or a user type.
 *
 * Input Parameters:
 *  item:
 *	A value indicating the item type for which we need the size.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  A value greater than or equal to zero, representing the size of the
 *  indicated type.
 */
static int64_t _sdl_sizeof(SDL_CONTEXT *context, int item)
{
    int64_t	retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_sizeof\n", __FILE__, __LINE__);

    if ((item >= SDL_K_BASE_TYPE_MIN) && (item <= SDL_K_BASE_TYPE_MAX))
	switch (item)
	{
	    case SDL_K_TYPE_NONE:
		retVal = 0;
		break;

	    case SDL_K_TYPE_BYTE:
	    case SDL_K_TYPE_INT_B:
	    case SDL_K_TYPE_BITFLD_B:
		retVal = sizeof(int8_t);
		break;

	    case SDL_K_TYPE_CHAR:
		retVal = sizeof(char);
		break;

	    case SDL_K_TYPE_WORD:
	    case SDL_K_TYPE_INT_W:
	    case SDL_K_TYPE_BITFLD_W:
		retVal = sizeof(int16_t);
		break;

	    case SDL_K_TYPE_LONG:
	    case SDL_K_TYPE_INT_L:
	    case SDL_K_TYPE_BITFLD_L:
	    case SDL_K_TYPE_ADDR_L:
	    case SDL_K_TYPE_PTR_L:
		retVal = sizeof(int32_t);
		break;

	    case SDL_K_TYPE_INT:
	    case SDL_K_TYPE_BITFLD:
		retVal = sizeof(int);
		break;

	    case SDL_K_TYPE_INT_HW:
	    case SDL_K_TYPE_HW_INT:
		if (context->wordSize == 32)
		    retVal = sizeof(int32_t);
		else
		    retVal = sizeof(int64_t);
		break;

	    case SDL_K_TYPE_QUAD:
	    case SDL_K_TYPE_INT_Q:
	    case SDL_K_TYPE_BITFLD_Q:
	    case SDL_K_TYPE_ADDR_Q:
	    case SDL_K_TYPE_PTR_Q:
		retVal = sizeof(int64_t);
		break;

	    case SDL_K_TYPE_OCTA:
	    case SDL_K_TYPE_BITFLD_O:
		retVal = sizeof(__int128_t);
		break;

	    case SDL_K_TYPE_HFLT:
	    case SDL_K_TYPE_XFLT:
		retVal = sizeof(long double);
		break;

	    case SDL_K_TYPE_HFLT_C:
	    case SDL_K_TYPE_XFLT_C:
		retVal = sizeof(long double _Complex);
		break;

	    case SDL_K_TYPE_TFLT:
	    case SDL_K_TYPE_FFLT:
		retVal = sizeof(float);
		break;

	    case SDL_K_TYPE_TFLT_C:
	    case SDL_K_TYPE_FFLT_C:
		retVal = sizeof(float _Complex);
		break;

	    case SDL_K_TYPE_SFLT:
	    case SDL_K_TYPE_DFLT:
	    case SDL_K_TYPE_GFLT:
		retVal = sizeof(double);
		break;

	    case SDL_K_TYPE_SFLT_C:
	    case SDL_K_TYPE_DFLT_C:
	    case SDL_K_TYPE_GFLT_C:
		retVal = sizeof(double _Complex);
		break;

	    case SDL_K_TYPE_DECIMAL:
		retVal = 2;		/* (2 * precision) + 1 */
		break;

	    case SDL_K_TYPE_CHAR_VARY:
		retVal = sizeof(char);	/* length + 2 bytes for stored length */
		break;

	    case SDL_K_TYPE_CHAR_STAR:
		retVal = sizeof(char);
		break;

	    case SDL_K_TYPE_ADDR:
	    case SDL_K_TYPE_PTR:
	    case SDL_K_TYPE_ENTRY:
		retVal = context->wordSize / 8;
		break;

	    case SDL_K_TYPE_ADDR_HW:
	    case SDL_K_TYPE_HW_ADDR:
	    case SDL_K_TYPE_PTR_HW:
		retVal = context->wordSize / 8;
		break;

	    case SDL_K_TYPE_ANY:
	    case SDL_K_TYPE_VOID:
	    case SDL_K_TYPE_STRUCT:
	    case SDL_K_TYPE_UNION:
		retVal = 0;
		break;

	    case SDL_K_TYPE_BOOL:
		retVal = sizeof(bool);
		break;

	    case SDL_K_TYPE_ENUM:
		retVal = sizeof(int);
		break;

	    default:
		break;
	}
    else if ((item >= SDL_K_DECLARE_MIN) && (item <= SDL_K_DECLARE_MAX))
    {
	SDL_DECLARE *myDeclare = sdl_get_declare(&context->declares, item);

	if (myDeclare != NULL)
	    retVal = myDeclare->size;
    }
    else if ((item >= SDL_K_ITEM_MIN) && (item <= SDL_K_ITEM_MAX))
    {
	SDL_ITEM *myItem = sdl_get_item(&context->items, item);

	if (myItem != NULL)
	    retVal = myItem->size;
    }
    else if ((item >= SDL_K_AGGREGATE_MIN) && (item <= SDL_K_AGGREGATE_MAX))
    {
	SDL_AGGREGATE *myAggregate =
	    sdl_get_aggregate(&context->aggregates, item);

	if (myAggregate != NULL)
	    retVal = myAggregate->size;
    }
    else if ((item >= SDL_K_ENUM_MIN) && (item <= SDL_K_ENUM_MAX))
	retVal = sizeof(int);

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_create_constant
 *  This function is called to create a constant record and return it back to
 *  the caller.
 *
 * Input Parameters:
 *  id:
 *  	A pointer to the constant identifier string.
 *  prefix:
 *	A pointer to the prefix to be prepended before the tag and id.
 *  tag:
 *	A pointer to the tag to be added between the prefix and the id.
 *  comment:
 *	A pointer to a comment string to be associated with this constant.
 *  typeName:
 *	A pointer to the type-name associated to this constant.
 *  radix:
 *	A value indicating the radix the value is to be displayed.
 *  value:
 *	A value for the actual constant.
 *  string:
 *	A pointer to a string value for the actual constant.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:	Failed to allocate the memory required.
 *  !NULL:	Normal Successful Completion.
 */
static SDL_CONSTANT *_sdl_create_constant(
        char *id,
        char *prefix,
        char *tag,
        char *comment,
        char *typeName,
        int radix,
        int64_t value,
        char *string)
{
    SDL_CONSTANT *retVal = sdl_allocate_block(ConstantBlock, NULL);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_create_constant\n", __FILE__, __LINE__);

    /*
     * Initialize the constant information.
     */
    if (retVal != NULL)
    {
	retVal->id = strdup(id);
	if (prefix != NULL)
	    retVal->prefix = strdup(prefix);
	else
	    retVal->prefix = NULL;
	retVal->tag = strdup(tag);
	if (comment != NULL)
	    retVal->comment = strdup(comment);
	else
	    retVal->comment = NULL;
	if (typeName != NULL)
	    retVal->typeName = strdup(typeName);
	else
	    retVal->typeName = NULL;
	retVal->radix = radix;
	if (string == NULL)
	{
	    retVal->type = SDL_K_CONST_NUM;
	    retVal->value = value;
	}
	else
	{
	    retVal->type = SDL_K_CONST_STR;
	    retVal->string = string;
	}
    }

    /*
     * Return the results back to the caller.
     */
    return (retVal);
}

/*
 * _sdl_queue_constant
 *  This function is called to queue a newly defined CONSTANT into the list in
 *  the context and then call the language specific routines to write out the
 *  language specific definition.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  myConst:
 *	A pointer to the constant structure to be queued up and put put.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
static int _sdl_queue_constant(SDL_CONTEXT *context, SDL_CONSTANT *myConst)
{
    int retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_queue_constant\n", __FILE__, __LINE__);

    if (myConst != NULL)
    {
	int ii;

	SDL_INSQUE(&context->constants, &myConst->header.queue);

	/*
	 * Loop through all the possible languages and call the
	 * appropriate output function for each of the enabled
	 * languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true)
		    && (context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_CONSTANT_CB])(
		        context->outFP[ii],
		        myConst,
		        context);
    }
    else
	retVal = 0;

    /*
     * Return the results back to the caller.
     */
    return (retVal);
}

/*
 * _sdl_create_enum
 *  This function is called to create an enumerate record and return it back to
 *  the caller.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  id:
 *  	A pointer to the enum identifier string.
 *  prefix:
 *	A pointer to the prefix to be prepended before the tag and id.
 *  tag:
 *	A pointer to the tag to be added between the prefix and the id.
 *  typeDef:
 *	A boolean value indicating that this enumeration will be declared as a
 *	typedef.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:	Failed to allocate the memory required.
 *  !NULL:	Normal Successful Completion.
 */
static SDL_ENUMERATE *_sdl_create_enum(
				SDL_CONTEXT *context,
				char *id,
				char *prefix,
				char *tag,
				bool typeDef)
{
    SDL_ENUMERATE	*retVal = sdl_allocate_block(EnumerateBlock, NULL);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_create_enum\n", __FILE__, __LINE__);

    if (retVal != NULL)
    {
	SDL_Q_INIT(&retVal->members);
	retVal->id = strdup(id);
	if (prefix != NULL)
	    retVal->prefix = strdup(prefix);
	else
	    retVal->prefix = NULL;
	retVal->tag = strdup(tag);
	retVal->typeDef = typeDef;
	retVal->size = _sdl_sizeof(context, SDL_K_TYPE_ENUM);
	retVal->typeID = context->enums.nextID++;
	SDL_INSQUE(&context->enums.header, &retVal->header.queue);
    }

    /*
     * Return the results back to the caller.
     */
    return (retVal);
}

/*
 * _sdl_enum_compl
 *  This function is called to call the language specific output routines to
 *  output and enumeration.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  myEnum:
 *	A pointer to the ENUMERATE structure to be output.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
static int _sdl_enum_compl(SDL_CONTEXT *context, SDL_ENUMERATE *myEnum)
{
    int retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_enum_compl\n", __FILE__, __LINE__);

    if (myEnum != NULL)
    {
	int ii;

	/*
	 * Loop through all the possible languages and call the
	 * appropriate output function for each of the enabled
	 * languages.
	 */
	for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	    if ((context->langSpec[ii] == true)
		    && (context->langEna[ii] == true))
		retVal = (*_outputFuncs[ii][SDL_K_ENUM_CB])(
		        context->outFP[ii],
		        myEnum,
		        context);
    }
    else
	retVal = 0;

    /*
     * Return the results back to the caller.
     */
    return (retVal);
}

/*
 * _sdl_all_lower
 *  This function is called to determine if the supplied string is all lower
 *  case.
 *
 * Input Parameters:
 *  str:
 *	A pointer to the string to check.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	'str' is all lower case.
 *  false:	'str' is either all uppercase or mixed case.
 */
static bool _sdl_all_lower(const char *str)
{
    int ii, len = (str ? strlen(str) : 0);
    bool retVal = true;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_all_lower\n", __FILE__, __LINE__);

    for (ii = 0; ((ii < len) && (retVal == true)); ii++)
    {
	if (isalpha(str[ii]) == true)
	    retVal = islower(str[ii]);
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_reset_options
 *  This function is called to reset the options array in the context block.
 *  If any of the saved options is a string and it is not NULL, then free it.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void _sdl_reset_options(SDL_CONTEXT *context)
{
    int	ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_reset_options\n", __FILE__, __LINE__);

    /*
     * Loop through each of the options, and if we have a string option and it
     * is not NULL, then free it.
     */
    for (ii = 0; ii < context->optionsIdx; ii++)
	if (((context->options[ii].option == Alias) ||
	     (context->options[ii].option == Based) ||
	     (context->options[ii].option == Counter) ||
	     (context->options[ii].option == Linkage) ||
	     (context->options[ii].option == Marker) ||
	     (context->options[ii].option == Named) ||
	     (context->options[ii].option == Origin) ||
	     (context->options[ii].option == Prefix) ||
	     (context->options[ii].option == ReturnsNamed) ||
	     (context->options[ii].option == Tag) ||
	     (context->options[ii].option == TypeName)) &&
	    (context->options[ii].string != NULL))
	    free(context->options[ii].string);

    /*
     * Reset the option index back to the beginning.
     */
    context->optionsIdx = 0;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * _sdl_iterate_members
 *  This function is called to iterate through each of the members and if that
 *  member is a subaggregate, call itself to iterate through that level.  If
 *  callback is a NULL, then we are just cleaning up the member tree.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current state of the parsing.
 *  member:
 *	A pointer to the current member are are iterating through.
 *  qhead:
 *	The address of the queue header.  Used to determine when we have
 *	search through each of the member entries in the AGGREGATE or
 *	subaggregate.
 *  callback:
 *	A pointer to the entry point to call to do something with the member
 *	information.  If this is NULL, then we are just deallocating all the
 *	member memory.  If tracing is turned on, we will display the records
 *	as we iterate through.
 *  depth:
 *	A value indicating the aggregate depth we are currently iterating.
 *  count
 *	A value indicating the number of members at the current depth.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
static int _sdl_iterate_members(
		SDL_CONTEXT *context,
		SDL_MEMBERS *member,
		void *qhead,
		int (*callback)(),
		int depth,
		int count)
{
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_iterate_members\n", __FILE__, __LINE__);

    if ((member->type == SDL_K_TYPE_STRUCT) ||
	(member->type == SDL_K_TYPE_UNION))
    {
	SDL_SUBAGGR *subaggr = &member->subaggr;

	if ((trace == true) && (callback == NULL))
	{
	    printf(
		"\t%d: SUBAGGREGATE:\n\t    name: %s\n\t    prefix: %s\n"
		"\t    tag: %s\n\t    marker: %s\n\t    arrgType: %s\n"
		"\t    typeID: %d\n\t    alignment: %d\n\t    type: %d\n"
		"\t    _unsigned: %s\n\t    size: %ld\n\t    memSize: %ld\n"
		"\t    typeDef: %s\n\t    fill: %s\n\t    basedPtrName: %s\n"
		"\t    currentBitOffset: %d\n",
		count,
		subaggr->id,
		(subaggr->prefix != NULL ? subaggr->prefix : ""),
		(subaggr->tag != NULL ? subaggr->tag : ""),
		(subaggr->marker != NULL ? subaggr->marker : ""),
		(subaggr->aggType == SDL_K_TYPE_STRUCT ? "STRUCTURE" : "UNION"),
		subaggr->typeID,
		subaggr->alignment,
		subaggr->type,
		(subaggr->_unsigned == true ? "True" : "False"),
		subaggr->size,
		subaggr->memSize,
		(subaggr->typeDef == true ? "True" : "False"),
		(subaggr->fill == true ? "True" : "False"),
		(subaggr->basedPtrName!= NULL ? subaggr->basedPtrName: ""),
		subaggr->currentBitOffset);
	    if (subaggr->dimension == true)
		printf(
		    "\t    dimension[%ld:%ld]\n",
		    subaggr->lbound,
		    subaggr->hbound);
	}
	if (callback != NULL)
	    (*callback)(context, member, false, depth);
	if (SDL_Q_EMPTY(&subaggr->members) == false)
	    retVal = _sdl_iterate_members(
				context,
				(SDL_MEMBERS *) subaggr->members.flink,
				&subaggr->members,
				callback,
				depth + 1,
				1);
	if (callback != NULL)
	    (*callback)(context, member, true, depth);
	if (member->header.queue.flink != qhead)
	    retVal = _sdl_iterate_members(
				context,
				member->header.queue.flink,
				qhead,
				callback,
				depth, count + 1);
    }
    else	/* Unknown */
    {
	if ((trace == true) && (callback == NULL))
	{
	    printf(
		"\t%d: ITEM:\n\t    name: %s\n\t    prefix: %s\n"
		"\t    tag: %s\n\t    typeID: %d\n\t    alignment: %d\n"
		"\t    type: %d\n\t    _unsigned: %s\n\t    size: %ld\n"
		"\t    memSize: %ld\n\t    typeDef: %s\n\t    fill: %s\n"
		"\t    length: %ld\n\t    mask: %s\n\t    signed: %s\n"
		"\t    bitfieldType: %ld\n",
		count,
	        member->item.id,
	        (member->item.prefix != NULL ? member->item.prefix : ""),
	        (member->item.tag != NULL ? member->item.tag : ""),
	        member->item.typeID,
	        member->item.alignment,
	        member->item.type,
	        (member->item._unsigned == true ? "True" : "False"),
	        member->item.size,
	        member->item.memSize,
	        (member->item.typeDef == true ? "True" : "False"),
	        (member->item.fill == true ? "True" : "False"),
	        member->item.length,
	        (member->item.mask == true ? "True" : "False"),
	        (member->item._signed == true ? "True" : "False"),
	        member->item.subType);
	    if (member->item.dimension == true)
		printf(
		    "\t    dimension: [%ld:%ld]\n",
		    member->item.lbound,
		    member->item.hbound);
	}
	if (callback != NULL)
	    (*callback)(context, member, false, depth);
	if (member->header.queue.flink != qhead)
	    retVal = _sdl_iterate_members(
				context,
				member->header.queue.flink,
				qhead,
				callback,
				depth,
				count + 1);
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}
