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
 */
#include "opensdl_defs.h"
#include "opensdl_lang.h"
#include "opensdl_actions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern _Bool trace;

/*
 * Define the language specific entry-points.
 */
#define SDL_K_COMMENT_CB	0
#define SDL_K_MODULE_CB		1
#define SDL_K_END_MODULE_CB	2
#define SDL_K_ITEM_CB		3
#define SDL_K_CONSTANT_CB	4

static SDL_LANG_FUNC _outputFuncs[SDL_K_LANG_MAX] =
{

    /*
     * For the C/C++ languages.
     */
    {
	(SDL_FUNC) &sdl_c_comment,
	&sdl_c_module,
	&sdl_c_module_end,
	&sdl_c_item,
	&sdl_c_constant
    }
};

/*
 * The following defines the default tags for the various data types.
 */
static char *_defaultTag[] =
{
    "K",	/* CONSTANT */
    "B",	/* BYTE [UNSIGNED] */
    "W",	/* WORD [UNSIGNED] */
    "L",	/* LONGWORD [UNSIGNED] */
    "Q",	/* QUADWORD [UNSIGNED] */
    "O",	/* OCTAWORD [UNSIGNED] */
    "F",	/* TFLOAT */
    "D",	/* SFLOAT */
    "P",	/* DECIMAL */
    "V",	/* BITFIELD 'S' and 'M' for size and mask, respectively */
    "T",	/* CHARACTER */
    "A",	/* ADDRESS/POINTER */
    "A",	/* POINTER_LONG */
    "A",	/* POINTER_QUAD */
    "A",	/* POINTER_HW/ADDRESS_HARDWARE */
    "",		/* ANY */
    "C",	/* BOOLEAN (Conditional) */
    "R",	/* STRUCTURE */
    "R"		/* UNION */
};

/*
 * Local Prototypes
 */
static SDL_DECLARE *_sdl_get_declare(SDL_DECLARE_LIST *declare, char *name);
static SDL_LOCAL_VARIABLE *_sdl_get_local(SDL_CONTEXT *context, char *name);
static SDL_ITEM *_sdl_get_item(SDL_ITEM_LIST *item, char *name);
static SDL_AGGREGATE *_sdl_get_aggregate(SDL_AGGREGATE_LIST *aggregate, char *name);
static char *_sdl_get_tag(SDL_CONTEXT *context, char *tag, int datatype);
static void _sdl_trim_tag(char *tag);
static __int64_t _sdl_sizeof(SDL_CONTEXT *context, int item);
static SDL_CONSTANT *_sdl_create_constant(
        char *id,
        char *prefix,
        char *tag,
        char *comment,
        char *typeName,
        int radix,
        __int64_t value,
        char *string);
static int _sdl_queue_constant(SDL_CONTEXT *context, SDL_CONSTANT *myConst);

/*
 * sdl_unquote_str
 *  This function is called to remove the leading and trailing double-quotes
 *  from the supplied string.  The call does the update in place.
 *
 * Input Parameters:
 *  str:
 *	A pointer to a string containing leading and trailing double quotes.
 *
 * Output Parameters:
 *  str:
 *	A pointer to a string no longer containing leading and trailing double
 *	quotes.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
char *sdl_unquote_str(char *str)
{
    char	*retVal = str;
    int		ii;
    size_t	len = strlen(str);

    if (str[0] == '"')
    {
	for (ii = 0; ii < len; ii++)
	    str[ii] = str[ii+1];
	len--;
    }
    if (str[len - 1] == '"')
	str[len - 1] = '\0';

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_declare
 *  This function is called to get the record of a previously defined local
 *  variable.
 *
 * Input Parameters:
 *  declare:
 *	A pointer to the list containing all currently defined DECLAREs.
 *  typeID:
 *	A value assigned to the declared user-type.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
SDL_DECLARE *sdl_get_declare(SDL_DECLARE_LIST *declare, int typeID)
{
    SDL_DECLARE	*retVal = (SDL_DECLARE *) declare->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_declare\n", __FILE__, __LINE__);

    while (retVal != (SDL_DECLARE *) &declare->header)
	if (retVal->typeID == typeID)
	    break;
	else
	    retVal = (SDL_DECLARE *) retVal->header.flink;
    if (retVal == (SDL_DECLARE *) &declare->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_item
 *  This function is called to get the record of a previously defined ITEM.
 *
 * Input Parameters:
 *  item:
 *	A pointer to the list containing all currently defined ITEMs.
 *  typeID:
 *	A value assigned to the declared item.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing ITEM.
 */
SDL_ITEM *sdl_get_item(SDL_ITEM_LIST *item, int typeID)
{
    SDL_ITEM	*retVal = (SDL_ITEM *) item->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_item\n", __FILE__, __LINE__);

    while (retVal != (SDL_ITEM *) &item->header)
	if (retVal->typeID == typeID)
	    break;
	else
	    retVal = (SDL_ITEM *) retVal->header.flink;
    if (retVal == (SDL_ITEM *) &item->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_aggregate
 *  This function is called to get the record of a previously defined
 *  AGGREGATE.
 *
 * Input Parameters:
 *  aggregate:
 *	A pointer to the list containing all currently defined AGGREGATEs.
 *  typeID:
 *	A value assigned to the declared aggregate.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
SDL_AGGREGATE *sdl_get_aggregate(SDL_AGGREGATE_LIST *aggregate, int typeID)
{
    SDL_AGGREGATE *retVal = (SDL_AGGREGATE *) aggregate->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_aggregate\n", __FILE__, __LINE__);

    while (retVal != (SDL_AGGREGATE *) &aggregate->header)
	if (retVal->typeID == typeID)
	    break;
	else
	    retVal = (SDL_AGGREGATE *) retVal->header.flink;
    if (retVal == (SDL_AGGREGATE *) &aggregate->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_get_local
 *  This function is called to get the value associated with a local definition
 *  currently defined with the indicated name.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the local variables.
 *  name:
 *	A pointer to the name of the local variable.
 *
 * Output Parameters:
 *  value:
 *	A pointer to the location to receive the value of the local variable.
 *
 * Return Value
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_get_local(SDL_CONTEXT *context, char *name, __int64_t *value)
{
    SDL_LOCAL_VARIABLE	*local = _sdl_get_local(context, name);
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_get_local", __FILE__, __LINE__);

    /*
     * We should have already found the local variable definition
     */
    if ((local != NULL) && (value != NULL))
	*value = local->value;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
	printf(
	    "%s, %ld (%016lx - %4.4s)\n",
	    name,
	    *value,
	    *value,
	    (char *) value);
    }

    /*
     * Return the results of this call back to the caller.
     */
    free(name);
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
 */
int sdl_set_local(SDL_CONTEXT *context, char *name, __int64_t value)
{
    SDL_LOCAL_VARIABLE	*local = _sdl_get_local(context, name);
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
	local = calloc(1, sizeof(SDL_LOCAL_VARIABLE));
	if (local != NULL)
	{
	    local->id = name;
	    SDL_INSQUE(&context->locals, &local->header);
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
 * sdl_comment
 *  This function is called to output a comment to the output file.
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
int sdl_comment(SDL_CONTEXT *context, char *comment)
{
    int	retVal = 1;
    int ii;
    int startOfComment = 2;	/* Need to skip past the '/[*+-/]'.	*/
    _Bool lineComment = false;
    _Bool startComment = false;
    _Bool endComment = false;

    /*
     * Trim all trailing space characters.
     */
    sdl_trim_str(comment, SDL_M_TRAIL);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_comment('%s' - after trimming\n", __FILE__, __LINE__, comment);

    /*
     * Determine the type of comment by the second character:
     *	'*' = lineComment
     *	'+' = startComment
     *	'-' = endComment
     *	'/' = a middle comment (all the flags are false).
     */
    switch (comment[1])
    {
	case '*':
	    lineComment = true;
	    break;

	case '+':
	    startComment = true;
	    break;

	case '-':
	    endComment = true;
	    break;
    }

    /*
     * Loop through all the possible languages and call the appropriate output
     * function for each of the enabled languages.
     */
    for (ii = 0; ((ii < SDL_K_LANG_MAX) && (retVal == 1)); ii++)
	if ((context->langSpec[ii] == true) && (context->langEna[ii] == true))
	    retVal = (*_outputFuncs[ii][SDL_K_COMMENT_CB])(
				context->outFP[ii],
				&comment[startOfComment],
				lineComment,
				startComment,
				endComment);

    /*
     * Return the results of this call back to the caller.
     */
    free(comment);
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
     * Clean-out any unaccounted for aggregates in the aggregate stack.
     */
    for (ii = context->aggStackPtr; ii < SDL_K_SUBAGG_MAX; ii++)
    {
	free(context->aggStack[ii]->id);
	if (context->aggStack[ii]->prefix != NULL)
	    free(context->aggStack[ii]->prefix);
	if (context->aggStack[ii]->tag != NULL)
	    free(context->aggStack[ii]->tag);
	if (context->aggStack[ii]->marker != NULL)
	    free(context->aggStack[ii]->marker);
	if (context->aggStack[ii]->comment != NULL)
	    free(context->aggStack[ii]->comment);
	free(context->aggStack[ii]);
    }
    context->aggStackPtr = SDL_K_SUBAGG_MAX;

    /*
     * Reset all the dimension entries.
     */
    for (ii = 0; ii < SDL_K_MAX_DIMENSIONS; ii++)
	context->dimensions[ii].inUse = 0;

    /*
     * Clean out all the local variables.
     */
    while(SDL_Q_EMPTY(&context->locals) == false)
    {
	SDL_LOCAL_VARIABLE *local;

	SDL_REMQUE(&context->locals, local);
	free(local->id);
	free(local);
    }

    /*
     * Clean out all the constant definitions.
     */
    ii = 1;
    while(SDL_Q_EMPTY(&context->constants) == false)
    {
	SDL_CONSTANT *constant;

	SDL_REMQUE(&context->constants, constant);
	printf(
	        "\t%2d: name: %s\n\t    prefix: %s\n\t    tag: %s\n\t    typeName: %s\n\t    type: %s\n",
		ii,
		constant->id,
		constant->prefix,
		constant->tag,
		constant->typeName,
		(constant->type == SDL_K_CONST_STR ? "String" : "Number"));
	if (constant->type == SDL_K_CONST_STR)
	{
	    printf(
		"\t    value: %s\n",
		constant->string);

	}
	else
	{
	    printf(
		"\t    value: %ld (%s)\n",
		constant->value,
		(constant->radix <= SDL_K_RADIX_DEC ? "Decimal" :
		    (constant->radix == SDL_K_RADIX_OCT ? "Octal" :
			(constant->radix == SDL_K_RADIX_HEX ? "Hexidecimal" :
			    "Invalid"))));
	}
	ii++;
	free(constant->id);
	if (constant->prefix != NULL)
	    free(constant->prefix);
	if (constant->tag != NULL)
	    free(constant->tag);
	if (constant->comment != NULL)
	    free(constant->comment);
	if (constant->typeName != NULL)
	    free(constant->typeName);
	free(constant);
    }

    /*
     * Clean out all the declares.
     */
    ii = 1;
    while (SDL_Q_EMPTY(&context->declares.header) == false)
    {
	SDL_DECLARE *declare;

	SDL_REMQUE(&context->declares.header, declare);
	printf(
	        "\t%2d: name: %s\n\t    prefix: %s\n\t    tag: %s\n\t    typeID: %d\n\t    type: %d\n\t    size: %ld\n",
	        ii++,
	        declare->id,
	        declare->prefix,
	        declare->tag,
	        declare->typeID,
	        declare->type,
	        declare->size);
	free(declare->id);
	if (declare->prefix != NULL)
	    free(declare->prefix);
	if (declare->tag != NULL)
	    free(declare->tag);
	free(declare);
    }

    /*
     * Clean out all the items.
     */
    while (SDL_Q_EMPTY(&context->items.header) == false)
    {
	SDL_ITEM *item;

	SDL_REMQUE(&context->items.header, item);
	free(item->id);
	if (item->prefix != NULL)
	    free(item->prefix);
	if (item->tag != NULL)
	    free(item->tag);
	if (item->comment != NULL)
	    free(item->comment);
	free(item);
    }

    /*
     * Clean out all the aggregates.
     */
    while (SDL_Q_EMPTY(&context->aggregates.header) == false)
    {
	SDL_AGGREGATE *aggregate;

	SDL_REMQUE(&context->aggregates.header, aggregate);
	free(aggregate->id);
	if (aggregate->prefix != NULL)
	    free(aggregate->prefix);
	if (aggregate->tag != NULL)
	    free(aggregate->tag);
	if (aggregate->marker != NULL)
	    free(aggregate->marker);
	if (aggregate->comment != NULL)
	    free(aggregate->comment);
	if (aggregate->basedPtrName != NULL)
	    free(aggregate->basedPtrName);
	free(aggregate);
    }

    /*
     * Clean out all the entries.
     */
    while (SDL_Q_EMPTY(&context->entries.header) == false)
    {
	SDL_ENTRY *entry;

	SDL_REMQUE(&context->entries.header, entry);
	free(entry->id);
	if (entry->comment != NULL)
	    free(entry->comment);
	if (entry->alias != NULL)
	    free(entry->alias);
	if (entry->linkage != NULL)
	    free(entry->linkage);
	if (entry->typeName != NULL)
	    free(entry->typeName);
	free(entry);
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

    literalLine = (SDL_LITERAL *) calloc(1, sizeof(SDL_LITERAL));
    if (literalLine != NULL)
    {
	literalLine->line = line;
	SDL_INSQUE(literals, &literalLine->header);
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
	free(literalLine->line);
	free(literalLine);
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_usertype_idx
 *  This function is called to return the user type id associated with a
 *  particular user type.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  usertype:
 *	A pointer to a string containing the name of the type.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  A value greater than or equal to SDL_K_USER_MIN, representing the type ID
 *  of the indicated usertype.
 */
int sdl_usertype_idx(SDL_CONTEXT *context, char *usertype)
{
    int		retVal = 0;
    SDL_DECLARE *myDeclare = (SDL_DECLARE *) context->declares.header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_usertype_idx\n", __FILE__, __LINE__);

    while (myDeclare != (SDL_DECLARE *) &context->declares.header)
	if (strcmp(myDeclare->id, usertype) == 0)
	{
	    retVal = myDeclare->typeID;
	    break;
	}
	else
	    myDeclare = (SDL_DECLARE *) myDeclare->header.flink;

    /*
     * Return the results of this call back to the caller.
     */
    free(usertype);
    return(retVal);
}

/*
 * sdl_declare
 *  This function is called to create a DECLARE record, if one does not already
 *  exist.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  name:
 *	A pointer to a string containing the name of the type.
 *  sizeType:
 *	A value indicating the size or datatype of the declaration.
 *  prefix:
 *	A pointer to a string to be used for prefixing definitions utilizing
 *	this declaration.
 *  tag:
 *	A pointer to a string to be used for tagging definitions utilizing
 *	this declaration.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_declare(
	SDL_CONTEXT *context,
	char *name,
	int sizeType,
	char *prefix,
	char *tag)
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
	myDeclare = (SDL_DECLARE *) calloc(1, sizeof(SDL_DECLARE));

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
	    myDeclare->prefix = prefix;
	    myDeclare->tag = _sdl_get_tag(context, tag, myDeclare->type);
	    _sdl_trim_tag(myDeclare->tag);
	    SDL_INSQUE(&context->declares.header, &myDeclare->header);
	}
	else
	{
	    retVal = 0;
	    free(name);
	    if (prefix != NULL)
		free(prefix);
	    if (tag != NULL)
		free(tag);
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_bin2int
 *  This function is called to convert a binary number in to its equivalent
 *  integer value.
 *
 * Input Parameters:
 *  binVal:
 *  	A numeric value containing just 1's and 0's.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  A number representing the integer value converted from binary.
 */
__int64_t sdl_bin2int(char *binStr)
{
    __int64_t	retVal = 0;
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_bin2int\n", __FILE__, __LINE__);

    while (binStr[ii] != '\0')
    {
	retVal *= 2;
	retVal += binStr[ii++] - '0';
    }

    /*
     * Return the results of this call back to the caller.
     */
    free(binStr);
    return(retVal);
}

/*
 * sdl_str2int
 *  This function is called to convert a string to a value.  The string can be
 *  at most 8 characters long and the ASCII values are combined into a numeric
 *  value.  There is no attempt to interpret the contents of the string itself.
 *
 * Input Parameters:
 *  strVal:
 *	A pointer to the string to be converted.
 *
 * Output Parameters:
 *  val:
 *  	A pointer to a long integer location to receive the value of the
 *	converted string.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_str2int(char *strVal, __int64_t *val)
{
    int		retVal = 1;
    size_t	len = strlen(strVal);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_str2int\n", __FILE__, __LINE__);

    /*
     * If the input string is less than or equal to the maximum size of a long
     * word (64-bit word) and we have a place to store the results, then we can
     * proceed.  Otherwise, we have an error.
     */
    if ((val != NULL) && (len <= sizeof(__int64_t)))
    {
	char	*ptr = (char *) val;
	int		ii;

	/*
	 * Set the resulting value to zero.  This will set each of the bytes in
	 * the value to be returned to all zeros.
	 */
	*val = 0;

	/*
	 * Loop through each byte in the put string and store it in the
	 * corresponding byte of the returned value.  This goes from the lowest
	 * to the highest bytes.  NOTE: We can simply cast this because we have
	 * no idea of how long the input string may be.
	 */
	for (ii = 0; ii < len; ii++)
	    ptr[ii] = strVal[ii];
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    free(val);
    return(retVal);
}

/*
 * sdl_offset
 *  This function is called to get the current offset within the current
 *  AGGREGATE definition.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  offsetType:
 *	A value indicating the type of offset to return.  This can be one of the
 *	following values:
 *	    SDL_K_OFF_BYTE_REL	- Relative from the beginning or ORIGIN.
 *	    SDL_K_OFF_BYTE_BEG	- Relative from the beginning.
 *	    SDL_K_OFF_BIT		- bit offset from the beginning or most recent
 *							  element.
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *  0:		if we did not find anything.
 *  >=0:	if we did find something to return.
 */
int sdl_offset(SDL_CONTEXT *context, int offsetType)
{
    SDL_AGGREGATE	*aggregate = NULL;
    int			retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_offset\n", __FILE__, __LINE__);

    if (SDL_AGGSTACK_EMPTY(context->aggStackPtr) == false)
    {
	aggregate = context->aggStack[context->aggStackPtr];
	switch (offsetType)
	{
	    case SDL_K_OFF_BYTE_REL:
		retVal = aggregate->origin.offset - aggregate->currentOffset;
		break;

	    case SDL_K_OFF_BYTE_BEG:
		retVal = aggregate->currentOffset;
		break;

	    case SDL_K_OFF_BIT:
		retVal = aggregate->currentBitOffset;
		break;
	}
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_dimension
 *  This function is called when a DIMENSION modifier is process.  It is called
 *  with 2 values, the lower and higher bounds of the dimension.  For languages
 *  that do not support lower bounds, the range will be appropriately adjusted
 *  to that languages needs, while maintaining the number of requested items.
 *  This section of code makes no assumptions about these two values.  The
 *  index of the entry used is returned back to the caller, which will, in turn
 *  be supplied on the processing of the declaration where the DIMENSION
 *  modifier was specified.
 *
 * Input Parameter:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  lbound:
 *	A value indicating the lower bound value for the dimension.
 *  hbound:
 *	A value indicating the upper bound value for the dimension.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  The entry in the dimension array used to store the DIMENSION information
 *  until needed.
 */
int sdl_dimension(SDL_CONTEXT *context, size_t lbound, size_t hbound)
{
    int		retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_dimension\n", __FILE__, __LINE__);

    /*
     * Loop through all the dimension array entries looking for an available
     * slot.
     */
    while ((retVal < SDL_K_MAX_DIMENSIONS) &&
	   (context->dimensions[retVal].inUse == false))
	retVal++;

    /*
     * If we found one, then save the dimension information and set the inUse
     * flag.  Otherwise, we'tt return a value outside the array dimensions to
     * indicate that we ran out of space.
     */
    if (retVal < SDL_K_MAX_DIMENSIONS)
    {
	context->dimensions[retVal].lbound = lbound;
	context->dimensions[retVal].hbound = hbound;
	context->dimensions[retVal].inUse = true;
    }

    /*
     * Return the dimension array location utilized back to the caller.
     */
    return(retVal);
}

/*
 * sdl_item
 *  This function is called to create an item, save it into the context and
 *  write out the item to the output file.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  TODO
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_item(
	SDL_CONTEXT *context,
	char *name,
	int datatype,
	int storage,
	int basealign,
	int dimension,
	char *prefix,
	char *tag)
{
    SDL_ITEM	*myItem = _sdl_get_item(&context->items, name);
    int		ii, retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_item\n", __FILE__, __LINE__);

    /*
     * We only create, never update.
     */
    if (myItem == NULL)
    {
	myItem = (SDL_ITEM *) calloc(1, sizeof(SDL_ITEM));

	if (myItem != NULL)
	{
	    myItem->id = name;
	    myItem->typeID = context->items.nextID++;
	    myItem->type = datatype;
	    myItem->size = _sdl_sizeof(context, datatype);
	    myItem->commonDef = (storage & SDL_M_STOR_COMM) == SDL_M_STOR_COMM;
	    myItem->globalDef = (storage & SDL_M_STOR_GLOB) == SDL_M_STOR_GLOB;
	    myItem->typeDef = (storage & SDL_M_STOR_TYPED) == SDL_M_STOR_TYPED;
	    myItem->alignment = basealign;
	    myItem->dimension = dimension >= 0;
	    if (myItem->dimension)
	    {
		myItem->lbound = context->dimensions[dimension].lbound;
		myItem->hbound = context->dimensions[dimension].hbound;
		context->dimensions[dimension].inUse = false;
	    }
	    myItem->prefix = prefix;
	    myItem->tag = _sdl_get_tag(context, tag, datatype);
	    _sdl_trim_tag(myItem->tag);
	    SDL_INSQUE(&context->items.header, &myItem->header);

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
	else
	    retVal = 0;
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_constant
 *  This function is called to create a definition record for a CONSTANT.  If
 *  one is already there, then the existing one is re-initialized.
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
int sdl_constant(SDL_CONTEXT *context)
{
    int retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_constant\n", __FILE__, __LINE__);

    /*
     * If we don't have a CONSTANT definition record handy, then go allocate
     * one now.
     */
    if (context->constDef == NULL)
    {
	context->constDef = calloc(1, sizeof(SDL_CONSTANT_DEF));
	if (context->constDef == NULL)
	    retVal = 0;
    }
    else
	retVal = sdl_constant_end(context, true);

    /*
     * Return the results of this call back to the caller.
     */
    return (retVal);
}

/*
 * sdl_constant_str
 *  This function is called to add a string definition information to a
 *  constant declaration.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  what:
 *	A value indicating what the information provided in the str parameter
 *	represents.
 *  str:
 *	A pointer to the string to store someplace.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_constant_str(SDL_CONTEXT *context, int what, char *str)
{
    int retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_constant_str\n", __FILE__, __LINE__);

    /*
     * If we don't have a place to store this, then we need to return an error.
     */
    if (context->constDef != NULL)
    {
	switch (what)
	{
	case SDL_K_CONST_NAME:
	case SDL_K_CONST_NAMES:
	    context->constDef->name_s = str;
	    break;

	case SDL_K_CONST_STRING:
	    context->constDef->type = SDL_K_CONST_STR;
	    context->constDef->strValue = str;
	    break;

	case SDL_K_CONST_COUNTER:
	    context->constDef->counter = str;
	    break;

	case SDL_K_CONST_TYPENAME:
	    context->constDef->typeName = str;
	    break;

	case SDL_K_CONST_PREFIX:
	    context->constDef->prefix = str;
	    break;

	case SDL_K_CONST_TAG:
	    context->constDef->tag = str;
	    break;

	default:
	    retVal = 0;
	    break;
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
 * sdl_constant_num
 *  This function is called to add a numeric definition information to a
 *  constant declaration.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  what:
 *	A value indicating what the information provided in the value parameter
 *	represents.
 *  value:
 *	A value to be stored someplace.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_constant_num(SDL_CONTEXT *context, int what, __int64_t value)
{
    int retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_constant_num\n", __FILE__, __LINE__);

    /*
     * If we don't have a place to store this, then we need to return an error.
     */
    if (context->constDef != NULL)
    {
	switch (what)
	{
	case SDL_K_CONST_EQUALS:
	    context->constDef->type = SDL_K_CONST_NUM;
	    context->constDef->value = value;
	    break;

	case SDL_K_CONST_INCR:
	    context->constDef->increment = value;
	    context->constDef->incrPresent = true;
	    break;

	case SDL_K_CONST_RADIX:
	    context->constDef->radix = value;
	    break;

	default:
	    retVal = 0;
	    break;
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
 * sdl_constant_end
 *  This function is called to perform the processing required to actually
 *  create one or more constant values.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the current parsing.
 *  keep:
 *	A boolean value to indicate if the constant definition block should be
 *	kept and initialized or deallocated (true == keep and initialize; false
 *	== deallocate).
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_constant_end(SDL_CONTEXT *context, bool keep)
{
    SDL_CONSTANT_DEF *constDef = context->constDef;
    int retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_constant_end\n", __FILE__, __LINE__);

    /*
     * If we don't have a CONSTANT definition record handy, then return an
     * error.
     */
    if (constDef != NULL)
    {
	SDL_CONSTANT *myConst;
	char *comma = strchr(constDef->name_s, ',');

	/*
	 * Before we go too far, we need to determine what kind of definition
	 * we have.  First let's see if there is one or multiple names needing
	 * to be created.
	 */
	if (comma == NULL)
	{
	    myConst = _sdl_create_constant(
		    constDef->name_s,
		    constDef->prefix,
		    constDef->tag,
		    NULL,
		    constDef->typeName,
		    constDef->radix,
		    constDef->value,
		    constDef->strValue);
	    retVal = _sdl_queue_constant(context, myConst);
	}
	else
	{
	    char *ptr = constDef->name_s;
	    __int64_t value = constDef->value;

	    /*
	     * We have multiple names to process through.  First we need
	     * go through the name_s string and pull out the individual
	     * name/comment data.
	     */
	    while (((comma = strchr(ptr, ',')) != NULL) && (retVal == 1))
	    {
		char *name;
		char *comment = NULL;

		*comma = '\0';
		if (strlen(ptr) > 0)
		{
		    name = strdup(ptr);
		    if (name != NULL)
		    {
			char *commentStart = strstr(ptr, "/*");

			if (commentStart != NULL)
			    comment = strdup(commentStart);
			else
			{
			    commentStart = strstr(ptr, "{");
			    if (commentStart != NULL)
				*commentStart = '\0';
			}
			myConst = _sdl_create_constant(
			        name,
			        constDef->prefix,
			        constDef->tag,
			        comment,
			        constDef->typeName,
			        constDef->radix,
			        value,
			        NULL);
			retVal = _sdl_queue_constant(context, myConst);
			free(name);
			if (comment != NULL)
			    free(comment);
			ptr = comma + 1;
		    }
		    else
			retVal = 0;
		}
		value += (constDef->incrPresent ? constDef->increment : 0);
	    }
	}

	/*
	 * Either re-initialize or deallocate all the memory.
	 */
	free(constDef->name_s);
	constDef->name_s = NULL;
	if (constDef->prefix != NULL)
	    free(constDef->prefix);
	constDef->prefix = NULL;
	if (constDef->tag != NULL)
	    free(constDef->tag);
	constDef->tag = NULL;
	if (constDef->typeName != NULL)
	    free(constDef->typeName);
	constDef->typeName = NULL;
	constDef->type = 0;
	constDef->increment = 0;
	constDef->incrPresent = false;
	constDef->radix = 0;
	if (keep == false)
	{
	    free(constDef);
	    context->constDef = NULL;
	}
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
	    retVal = (SDL_DECLARE *) retVal->header.flink;
    if (retVal == (SDL_DECLARE *) &declare->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_get_local
 *  This function is called to get the local definition currently defined with
 *  the indicated name.  If none if found a NULL is returned.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context structure where we maintain information about
 *	the local variables.
 *  name:
 *	A pointer to the name of the local variable.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	Local variable not found.
 *  !NULL:	An existing local variable.
 */
static SDL_LOCAL_VARIABLE *_sdl_get_local(SDL_CONTEXT *context, char *name)
{
    SDL_LOCAL_VARIABLE	*retVal = (SDL_LOCAL_VARIABLE *) context->locals.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_get_local\n", __FILE__, __LINE__);

    /*
     * Search through the list of local variables and see if one with the same
     * name already exists.
     */
    while (retVal != (SDL_LOCAL_VARIABLE *) &context->locals)
	if (strcmp(retVal->id, name) == 0)
	    break;
	else
	    retVal = (SDL_LOCAL_VARIABLE *) retVal->header.flink;

    /*
     * If we ended up at the head of the queue, then we did not find what we
     * were asked to locate.
     */
    if (retVal == (SDL_LOCAL_VARIABLE *) &context->locals)
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
	    retVal = (SDL_ITEM *) retVal->header.flink;
    if (retVal == (SDL_ITEM *) &item->header)
	retVal = NULL;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_get_aggregate
 *  This function is called to get the record of a previously defined
 *  AGGREGATE.  If the AGGREGATE has not yet been defined, then a NULL will be
 *  returned. Otherwise, the a pointer to the found AGGREGATE will be returned
 *  to the caller.
 *
 * Input Parameters:
 *  item:
 *	A pointer to the list containing all currently defined AGGREGATEs.
 *  name:
 *	A pointer to the name of the declared aggregate.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value
 *  NULL:	AGGREGATE not found.
 *  !NULL:	An existing AGGREGATE.
 */
static SDL_AGGREGATE *_sdl_get_aggregate(SDL_AGGREGATE_LIST *aggregate, char *name)
{
    SDL_AGGREGATE	*retVal = (SDL_AGGREGATE *) aggregate->header.flink;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_get_aggregate\n", __FILE__, __LINE__);

    while (retVal != (SDL_AGGREGATE *) &aggregate->header)
	if (strcmp(retVal->id, name) == 0)
	    break;
	else
	    retVal = (SDL_AGGREGATE *) retVal->header.flink;
    if (retVal == (SDL_AGGREGATE *) &aggregate->header)
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
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  A pointer to a user specified tag or a default tag.
 */
static char *_sdl_get_tag(SDL_CONTEXT *context, char *tag, int datatype)
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
	if (datatype == SDL_K_TYPE_NONE)
	    retVal = strdup(_defaultTag[SDL_K_TYPE_CHAR]);
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
		SDL_DECLARE *myDeclare = sdl_get_declare(&context->declares, datatype);

		if (myDeclare != NULL)
		{
		    if (strlen(myDeclare->tag) > 0)
			retVal = strdup(myDeclare->tag);
		    else
			retVal = _sdl_get_tag(context, tag, myDeclare->typeID);
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
			retVal = _sdl_get_tag(context, tag, myItem->typeID);
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
					myAggregate->typeID);
		}
		else
		    retVal = strdup(_defaultTag[SDL_K_TYPE_ANY]);
	    }
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_trim_tag
 *  This function is called to remove all trailing underscores from the tag
 *  string.  A single underscore will be used when generating the symbol.
 *
 * Input Parameters:
 *  tag:
 *	A pointer to the tag to be trimmed.
 *
 * Output Parameters:
 *  tag:
 *	A pointer to the potentially updated string.
 *
 * Return Values:
 *  None.
 */
static void _sdl_trim_tag(char *tag)
{
    size_t	len = strlen(tag);
    size_t	ii;
    _Bool	done = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_trim_tag\n", __FILE__, __LINE__);

    /*
     * Start at the end of the tag string and if the last character is an
     * underscore, then change it to a null character.  Then check the next
     * until we come across something other than an underscore or we run out
     * of characters to check.
     */
    for (ii = len - 1; ((ii > 0) && (done == false)); ii--)
    {
	if (tag[ii] == '_')
	    tag[ii] = '\0';
	else
	    done = true;
    }

    /*
     * Return the results of this call back to the caller.
     */
    return;
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
static __int64_t _sdl_sizeof(SDL_CONTEXT *context, int item)
{
    __int64_t	retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_sizeof\n", __FILE__, __LINE__);

    if ((item >= SDL_K_BASE_TYPE_MIN) && (item <= SDL_K_BASE_TYPE_MAX))
	switch (item)
	{
	    case SDL_K_TYPE_BYTE:
		retVal = sizeof(char);
		break;

	    case SDL_K_TYPE_WORD:
		retVal = sizeof(short int);
		break;

	    case SDL_K_TYPE_LONG:
		retVal = sizeof(int);
		break;

	    case SDL_K_TYPE_QUAD:
		retVal = sizeof(__int64_t);
		break;

	    case SDL_K_TYPE_OCTA:
		retVal = sizeof(__int128_t);
		break;

	    case SDL_K_TYPE_TFLT:
		retVal = sizeof(float);
		break;

	    case SDL_K_TYPE_SFLT:
		retVal = sizeof(double);
		break;

	    case SDL_K_TYPE_DECIMAL:
		retVal = sizeof(float);
		break;

	    case SDL_K_TYPE_CHAR:
		retVal = sizeof(char);
		break;

	    case SDL_K_TYPE_ADDR:
		retVal = sizeof(void *);
		break;

	    case SDL_K_TYPE_ADDRL:
		retVal = sizeof(long int);
		break;

	    case SDL_K_TYPE_ADDRQ:
		retVal = sizeof(long long int);
		break;

	    case SDL_K_TYPE_ADDRHW:
		retVal = sizeof(void *);
		break;

	    case SDL_K_TYPE_BOOL:
		retVal = sizeof(_Bool);
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

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_trim_str
 *  This function is called to remove space characters from a string.  If can
 *  perform 4 kinds of space removal (any one or all at the same time).
 *
 *	SDL_M_LEAD	Remove all leading spaces
 *	SDL_M_TRAIL	Remove all trailing spaces
 *	SDL_M_COMPRESS	Convert repeating space characters to a single one
 *	SDL_M_COLLAPSE	Remove all space characters
 *
 * Input Parameters:
 *  str:
 *	A pointer to the string to be trimmed.
 *  type:
 *	A value mask indicating the type of space trimming to be performed.
 *
 * Output Parameters:
 *  str:
 *	A pointer to the updated string (modified in place)
 *
 * Return Values:
 *  None.
 */
void sdl_trim_str(char *str, int type)
{
    int		srcIdx = 0;
    int		destIdx = 0;

    /*
     * If we are to convert control characters to spaces, do so now.
     */
    if ((type & SDL_M_CONVERT) != 0)
    {
	while (str[srcIdx] != '\0')
	{
	    if (iscntrl(str[srcIdx]) != 0)
		str[srcIdx] = ' ';
	    srcIdx++;
	}
	srcIdx = 0;
    }

    /*
     * If we are stripping leading or compressing multiple or removing all
     * spaces, then we start from the beginning of the string and work out way
     * to the end.  Otherwise, we are probably stripping trailing spaces, in
     * which case, we are stripping from the end of the string forward.
     */
    if ((type & (SDL_M_LEAD | SDL_M_COMPRESS | SDL_M_COLLAPSE)) != 0)
	while (str[srcIdx] != '\0')
	{

	    /*
	     * Collapsing removes all spaces, so it automatically includes
	     * leading, trailing, and repeated space removal.
	     */
	    if ((type & SDL_M_COLLAPSE) != 0)
	    {
		if (isspace(str[srcIdx]))
		    srcIdx++;
		else
		    str[destIdx++] = str[srcIdx++];
	    }
	    else
	    {
		bool skipCompress = false;

		if (((type & SDL_M_LEAD) != 0) && (destIdx == 0))
		{
		    skipCompress = true;
		    if (isspace(str[srcIdx]))
			srcIdx++;
		    else
			str[destIdx++] = str[srcIdx++];
		}
		if (((type & SDL_M_COMPRESS) != 0) && (skipCompress == false))
		{
		    if (destIdx == 0)
			str[destIdx++] = str[srcIdx++];
		    else if (isspace(str[destIdx - 1]) && isspace(str[srcIdx]))
			srcIdx++;
		    else
			str[destIdx++] = str[srcIdx++];
		}
	    }
	}
    else
	destIdx = strlen(str);

    /*
     * Make sure we terminate the string with a null-character.
     */
    str[destIdx] = '\0';

    /*
     * If we are stripping spaces from the end of the string, then we will just
     * convert the space characters to a null-character.
     */
    if (((type & SDL_M_TRAIL) != 0) && ((type & SDL_M_COLLAPSE) == 0))
    {
	while (isspace(str[--destIdx]))
	    str[destIdx] = '\0';
    }

    /*
     * Return back to the caller.
     */
    return;
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
 *	A pointer to the typename associated to this constant.
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
        __int64_t value,
        char *string)
{
    SDL_CONSTANT *retVal = calloc(1, sizeof(SDL_CONSTANT));

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
	if (tag != NULL)
	    retVal->tag = strdup(prefix);
	else
	    retVal->tag = NULL;
	if (comment != NULL)
	    retVal->comment = strdup(prefix);
	else
	    retVal->comment = NULL;
	if (typeName != NULL)
	    retVal->typeName = strdup(prefix);
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
 *	A pointer to the constant structure to be queued upo and put put.
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
    int retVal = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_queue_constant\n", __FILE__, __LINE__);

    if (myConst != NULL)
    {
	int ii;

	SDL_INSQUE(&context->constants, &myConst->header);

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
