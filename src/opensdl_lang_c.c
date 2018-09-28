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
 *  This source file contains all the code to generate an output file that is
 *  consistent with the C and C++ languages.
 *
 * Revision History:
 *
 *  V01.000	26-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	06-SEP-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
#include <stdio.h>
#include <string.h>
#include "opensdl_defs.h"
#include "opensdl_lang.h"
#include "opensdl_actions.h"

/*
 * TODO: We need to move away from static buffers.  Search for "512".
 */

extern char *sdl_months[];
extern _Bool trace;

static char	*_module_str[] =
{
    "\n/*** MODULE %s ",
    "IDENT = %s ",
    "***/",
    "#ifdef _%s_\n#define _%s_ 1\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n",
    "\n#ifdef __cplusplus\n}\n#endif\n\n#endif /* _%s_ */"
};
#define SDL_MODULE_ENT	0
#define SDL_IDENT_ENT	1
#define SDL_MODC_ENT	2
#define SDL_MODCOND_ENT	3
#define SDL_MODEND_ENT	4

static char	*_comments[] =
{
    "/*%s */",
    "\n/*\n *%s",
    "\n/*",
    " *%s",
    " *%s\n */",
    " */",
    "%s",
    "\n/*%s*/"
};
#define SDL_LINE_COMMENT	0
#define SDL_START1_COMMENT	1
#define SDL_START2_COMMENT	2
#define SDL_MID1_COMMENT	3
#define SDL_END1_COMMENT	4
#define SDL_END2_COMMENT	5
#define SDL_MID2_COMMENT	6
#define SDL_START_END_COMMENT	7

static char *_constant[] =
{"#define ", "%s%s_%s\t", "%s%s\t", "%d\t", "0x%x\t", "0%o\t", "\"%s\"\t"};
#define SDL_DEFINE_ENT	0
#define SDL_CONST_TAG	1
#define SDL_CONST_NOTAG	2
#define SDL_DEC_ENT	3
#define SDL_HEX_ENT	4
#define SDL_OCT_ENT	5
#define SDL_STR_ENT	6

static char	*_entry[] = {"%s %s(", ");"};
#define SDL_ENTRY_ENT	0
#define SDL_ENTRYC_ENT	1

static char	*_parameter[] = {"%s %s", "%s *%s", ","};
#define SDL_PARAM_VAL_ENT   0
#define SDL_PARAM_REF_ENT   1
#define SDL_PARAM_COM_ENT   3

static char	*_aggregates[] = {"struct", "union"};
#define SDL_AGGR_STR_ENT    0
#define SDL_AGGR_UNI_ENT    1

static char	*_typed = "typedef";

static char	*_names[] = {"%s%s_%.*s", "_%s%s_%.*s_"};
#define SDL_NAME_ENT	0
#define SDL_TYPED_ENT	1

static char	*_sign = "unsigned";

static char	*_types[] =
{
    "_Bool ",
    "char ",
    "short int ",
    "int ",
    "__int64 ",
    "__int128 ",
    "float ",
    "double float ",
    ": %d",
    "[%d]",
    "struct {int string_length; char string_text[%d];}",
    "*",
    "%s "
};
#define SDL_BOOL_ENT	0
#define SDL_BYTE_ENT	1
#define SDL_WORD_ENT	2
#define SDL_LONG_ENT	3
#define SDL_QUAD_ENT	4
#define SDL_OCTA_ENT	5
#define SDL_TFLT_ENT	6
#define SDL_SFLT_ENT	7
#define SDL_BITF_ENT	8
#define SDL_ARRAY_ENT	9
#define SDL_VARY_ENT	10
#define SDL_PTR_ENT	11
#define SDL_USER_ENT	12

static char	*_scope[] = {"extern ", "globalref", "globaldef"};
#define SDL_COMMON_ENT	0
#define SDL_GLOBAL_ENT	1
#define SDL_GDEF_ENT	2

static char	*_newLine = "\n";

/*
 * _sdl_c_typeidStr
 *  This function is called to convert a typeID to a string to be used to
 *  declare am item.
 *
 * Input Parameters:
 *  typeID:
 *	A value indicating the type to be converted.
 *  context:
 *	A pointer to the context block to be used for converting a type into
 *	a string.
 */
static char *_sdl_c_typeidStr(int typeID, SDL_CONTEXT *context)
{
    char	*retVal = NULL;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_c_typeidStr\n", __FILE__, __LINE__);

    if ((typeID >= SDL_K_BASE_TYPE_MIN) && (typeID <= SDL_K_BASE_TYPE_MAX))
	switch (typeID)
	{
	    case SDL_K_TYPE_BYTE:
	    case SDL_K_TYPE_CHAR:
		retVal = _types[SDL_BYTE_ENT];
		break;

	    case SDL_K_TYPE_WORD:
		retVal = _types[SDL_WORD_ENT];
		break;

	    case SDL_K_TYPE_LONG:
		retVal = _types[SDL_LONG_ENT];
		break;

	    case SDL_K_TYPE_QUAD:
		retVal = _types[SDL_QUAD_ENT];
		break;

	    case SDL_K_TYPE_OCTA:
		retVal = _types[SDL_OCTA_ENT];
		break;

	    case SDL_K_TYPE_TFLT:
		retVal = _types[SDL_TFLT_ENT];
		break;

	    case SDL_K_TYPE_SFLT:
		retVal = _types[SDL_SFLT_ENT];
		break;

	    /*
	     * TODO: We need more here (the base type).
	     */
	    case SDL_K_TYPE_DECIMAL:
		retVal = _types[SDL_BYTE_ENT];
		break;

	    /*
	     * TODO: We need more here (the base type).
	     */
	    case SDL_K_TYPE_BITFLD:
		retVal = _types[SDL_BYTE_ENT];
		break;

	    /*
	     * TODO: We need more here (the base type).
	     */
	    case SDL_K_TYPE_ADDR:
	    case SDL_K_TYPE_ADDRL:
	    case SDL_K_TYPE_ADDRQ:
	    case SDL_K_TYPE_ADDRHW:
		retVal = _types[SDL_PTR_ENT];
		break;

	    case SDL_K_TYPE_BOOL:
		retVal = _types[SDL_BOOL_ENT];
		break;

	    /*
	     * TODO: We need more here (the base type).
	     */
	    case SDL_K_TYPE_SRUCT:
		retVal = _types[SDL_BYTE_ENT];
		break;

	    /*
	     * TODO: We need more here (the base type).
	     */
	    case SDL_K_TYPE_UNION:
		retVal = _types[SDL_BYTE_ENT];
		break;

	    default:
		break;
	}
    else if ((typeID >= SDL_K_DECLARE_MIN) && (typeID <= SDL_K_DECLARE_MAX))
    {
	SDL_DECLARE *myDeclare = sdl_get_declare(&context->declares, typeID);

	/*
	 * TODO: We need more here (the base type).
	 */
    }
    else if ((typeID >= SDL_K_ITEM_MIN) && (typeID <= SDL_K_ITEM_MAX))
    {
	SDL_ITEM *myItem = sdl_get_item(&context->items, typeID);

	/*
	 * TODO: We need more here (the base type).
	 */
    }
    else if ((typeID >= SDL_K_AGGREGATE_MIN) && (typeID <= SDL_K_AGGREGATE_MAX))
    {
	SDL_AGGREGATE *myAggregate =
	    sdl_get_aggregate(&context->aggregates, typeID);

	/*
	 * TODO: We need more here (the base type).
	 */
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_comment_stars
 *  This function is called to output a comment at the start of the output file
 *  that is composed of all stars, except the first and last, which start and
 *  end the comment, respectively.  This is the first and fourth comment lines.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_commentStars(FILE *fp)
{
    char	str[SDL_K_COMMENT_LEN];
    int		retVal = 1;
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_commentStars\n", __FILE__, __LINE__);

    /*
     * This is specifically for the C language.  When and if we support more,
     * this will need to be language specific.
     *
     * The first and last characters are a '/' with '*' in between.
     */
    for (ii = 0; ii < SDL_K_COMMENT_LEN;ii++)
	str[ii] = '*';
    str[SDL_K_COMMENT_LEN - 1] = '\0';
    str[0] = str[SDL_K_COMMENT_LEN - 2] = '/';

    /*
     * Write out the string to the output file.
     */
    if (fprintf(fp, "%s\n", str) < 0)
	retVal = 0;

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_createdByInfo
 *  This function is called to output a comment at the start of the output file
 *  that is of the time the parsing began and the utility that created the
 *  file.  This is the second comment line.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  timeInfo:
 *	A pointer to the time information associated when the utility was
 *	executed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_createdByInfo(FILE *fp, struct tm *timeInfo)
{
    char	str[SDL_K_COMMENT_LEN];
    char	*timeFmt = "/* Created %02d-%s-%04d %02d:%02d:%02d by OpenSDL "
			   "%c%d.%d-%d";
    int		retVal = 0;
    int		len, ii, jj;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_createdByInfo\n", __FILE__, __LINE__);

    /*
     * Let's write out the beginning of the comment.
     */
    len = sprintf(
		str,
		timeFmt,
		timeInfo->tm_mday,
		sdl_months[timeInfo->tm_mon],
		(1900 + timeInfo->tm_year),
		timeInfo->tm_hour,
		timeInfo->tm_min,
		timeInfo->tm_sec,
		SDL_K_VERSION_TYPE,
		SDL_K_VERSION_MAJOR,
		SDL_K_VERSION_MINOR,
		SDL_K_VERSION_LEVEL);
    jj = SDL_K_COMMENT_LEN - 1;
    str[jj--] = '\0';
    str[jj--] = '/';
    str[jj--] = '*';
    str[jj] = ' ';
    for (ii = len; ii < jj; ii++)
	str[ii] = ' ';

    /*
     * Write out the string to the output file.
     */
    retVal = fprintf(fp, "%s\n", str);
    retVal = (retVal < 0) ? 0 : 1;


    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_fileInfo
 *  This function is called to output a comment at the start of the output file
 *  that is the modify date and full file path of the input file being
 *  processed.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  timeInfo:
 *	A pointer to the time information associated when the utility was
 *	executed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_fileInfo(FILE *fp, struct tm *timeInfo, char *fullFilePath)
{
    char	str[SDL_K_COMMENT_LEN];
    char	newFilename[SDL_K_COMMENT_LEN];
    char	*sourceFmt = "/* Source: %02d-%s-%04d %02d:%02d:%02d ";
    int		retVal = 1;
    int		newLen, len, ii, jj, kk = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_fileInfo\n", __FILE__, __LINE__);

    len = sprintf(
		str,
		sourceFmt,
		timeInfo->tm_mday,
		sdl_months[timeInfo->tm_mon],
		(1900 + timeInfo->tm_year),
		timeInfo->tm_hour,
		timeInfo->tm_min,
		timeInfo->tm_sec);
    strncpy(newFilename, fullFilePath, SDL_K_COMMENT_LEN - 1);
    newFilename[SDL_K_COMMENT_LEN - 1] = '\0';
    newLen = strlen(newFilename);
    jj = SDL_K_COMMENT_LEN - 1;
    str[jj--] = '\0';
    str[jj--] = '/';
    str[jj--] = '*';
    str[jj] = ' ';
    for (ii = len; ii < jj; ii++)
	str[ii] = ' ';
    if (newLen > (jj - len))
	for (ii = (newLen - jj + len); ii < newLen; ii++)
	    newFilename[kk++] = newFilename[ii];
    sprintf(&str[len], "%s", newFilename);
    str[len + newLen] = ' ';

    /*
     * Write out the string to the output file.
     */
    if (fprintf(fp, "%s\n", str) < 0)
	retVal = 0;

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_comment
 *  This function is called to output a C/C++ language style comment.  NOTE:
 *  C++ supports using '//' as a line-comment, but not all versions of C
 *  support this.  Therefore, we will not be generating the line-comment in
 *  this implementation.
  *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  comment:
 *	A pointer to the comment string to be output.
 *  lineComment:
 *	A boolean value indicating that the comment should start with a start
 *	comment and end with an end comment.
 *  startComment:
 *	A boolean value indicating that the comment should start with a start
 *	comment.
 *  endComment:
 *	A boolean value indicating that the comment should end with an end
 *	comment.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_comment(
		FILE *fp,
		char *comment,
		bool lineComment,
		bool startComment,
		bool middleComment,
		bool endComment)
{
    int	retVal = 1;
    char *whichComment;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_comment\n", __FILE__, __LINE__);

    /*
     * Determine the type of comment being used.
     */
    if (lineComment == true)
	whichComment = _comments[SDL_LINE_COMMENT];
    else if (startComment == true)
    {
	if (endComment == true)
	    whichComment = _comments[SDL_START_END_COMMENT];
	else if (strlen(comment) == 0)
	    whichComment = _comments[SDL_START2_COMMENT];
	else
	    whichComment = _comments[SDL_START1_COMMENT];
    }
    else if (endComment == true)
    {
	if (strlen(comment) == 0)
	    whichComment = _comments[SDL_END2_COMMENT];
	else
	    whichComment = _comments[SDL_END1_COMMENT];
    }
    else if (middleComment == true)
	whichComment = _comments[SDL_MID1_COMMENT];
    else
	whichComment = _comments[SDL_MID2_COMMENT];

    /*
     * If the file has been opened, then write out the comment.
     */
    if (fp != NULL)
    {
	if (fprintf(fp, whichComment, comment) < 0)
	    retVal = 0;
	else if (fprintf(fp, _newLine) < 0)
	    retVal = 0;
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_module
 *  This function is called to output a C/C++ language style comment associated
 *  with the MODULE statement.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  context:
 *	A pointer to the parser context where the module name and ident
 *	information are stored.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_module(FILE *fp, SDL_CONTEXT *context)
{
    int	retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_module\n", __FILE__, __LINE__);

    /*
     * OK, time to write out the OpenSDL Parser's MODULE header.
     */
    if (fp != NULL)
    {
	if (fprintf(fp, _module_str[SDL_MODULE_ENT], context->module) < 0)
	    retVal = 0;
	else if ((context->ident != NULL) && (strlen(context->ident) > 0))
	{
	    if (fprintf(fp, _module_str[SDL_IDENT_ENT], context->ident) < 0)
		retVal = 0;
	}
	if ((retVal == 1) &&
	    (fprintf(fp, _module_str[SDL_MODC_ENT], context->ident) < 0))
	    retVal = 0;
	if (retVal == 1)
	{
	    if (fprintf(fp, _newLine) < 0)
		retVal = 0;
	    else if (fprintf(
			fp,
			_module_str[SDL_MODCOND_ENT],
			strupr(context->module),
			strupr(context->module)) < 0)
		retVal = 0;
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
 * sdl_c_module_end
 *  This function is called when it gets to the END_MODULE keyword.  It end the
 *  definitions.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  context:
 *	A pointer to the parser context where the module name information is
 *	stored.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_module_end(FILE *fp, SDL_CONTEXT *context)
{
    int	retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_cmodule_end\n", __FILE__, __LINE__);

    /*
     * OK, time to write out the OpenSDL Parser's MODULE footer.
     */
    if (fp != NULL)
    {
	if (fprintf(fp, _module_str[SDL_MODEND_ENT], strupr(context->module)) < 0)
	    retVal = 0;
	else if (fprintf(fp, _newLine) < 0)
	    retVal = 0;
    }
    else
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_item
 *  This function is called when it gets an ITEM keyword.  It writes out a
 *  declaration consistent with the information provided on the ITEM.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  item:
 *  	A pointer to the ITEM record.
 *  context:
 *	A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_item(FILE *fp, SDL_ITEM *item, SDL_CONTEXT *context)
{
    char firstName[512];
    char secondName[512];
    size_t	copyLen;
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_item\n", __FILE__, __LINE__);

    /*
     * If we have a typedef, then we need to declare this ITEM to have 2 names.
     * The first name will be _[<prefix>]<tag>_<id>_ and the second name will
     * be [<prefix>]<tag>_<id>.
     */
    if (item->typeDef)
    {
	copyLen =
	    4 + strlen(item->prefix) + strlen(item->tag) + strlen(item->id);
	if (copyLen > 512)
	    copyLen = strlen(item->id) - (copyLen - 512);
	else
	    copyLen = strlen(item->id);
	sprintf(
	    firstName,
	    _names[SDL_TYPED_ENT],
	    item->prefix,
	    item->tag,
	    copyLen, item->id);
	copyLen =
	    2 + strlen(item->prefix) + strlen(item->tag) + strlen(item->id);
	if (copyLen > 512)
	    copyLen = strlen(item->id) - (copyLen - 512);
	else
	    copyLen = strlen(item->id);
	sprintf(
	    secondName,
	    _names[SDL_NAME_ENT],
	    item->prefix,
	    item->tag,
	    copyLen, item->id);
    }
    else
    {
	copyLen =
	    2 + strlen(item->prefix) + strlen(item->tag) + strlen(item->id);
	if (copyLen > 512)
	    copyLen = strlen(item->id) - (copyLen - 512);
	else
	    copyLen = strlen(item->id);
	sprintf(
	    firstName,
	    _names[SDL_NAME_ENT],
	    item->prefix,
	    item->tag,
	    copyLen, item->id);
	secondName[0] = '\0';
    }

    /*
     * TODO: Complete this code.
     */

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_constant
 *  This function is called when it gets a CONSTANT keyword.  It writes out a
 *  declaration consistent with the information provided on the CONSTANT.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  constant:
 *  	A pointer to the CONSTANT record.
 *  context:
 *	A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
int sdl_c_constant(FILE *fp, SDL_CONSTANT *constant, SDL_CONTEXT *context)
{
    char *prefix = (constant->prefix ? constant->prefix : "");
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_constant\n", __FILE__, __LINE__);

    /*
     * Now let's do some output.
     *
     * First the #define portion.
     */
    if (fprintf(fp, _constant[SDL_DEFINE_ENT]) < 0)
	retVal = 0;

    /*
     * Next the <name> portion.
     */
    if (strlen(constant->tag) > 0)
    {
	if (fprintf(
	        fp,
	        _constant[SDL_CONST_TAG],
	        prefix,
	        constant->tag,
	        constant->id) < 0)
	    retVal = 0;
    }
    else
    {
	if (fprintf(fp, _constant[SDL_CONST_NOTAG], prefix, constant->id) < 0)
	    retVal = 0;
    }

    /*
     * If name was successful and this is a string constant, then output the
     * string value.
     */
    if ((retVal == 1) && (constant->type == SDL_K_CONST_STR))
    {
	if (fprintf(fp, _constant[SDL_STR_ENT], constant->string) < 0)
	    retVal = 0;
    }

    /*
     * It isn't a string, so it much be a value.  Output the value based on the
     * request RADIX.
     */
    else if (retVal == 1)
    {
	switch (constant->radix)
	{
	    case SDL_K_RADIX_DEC:
		if (fprintf(fp, _constant[SDL_DEC_ENT], constant->value) < 0)
		    retVal = 0;
		break;

	    case SDL_K_RADIX_OCT:
		if (fprintf(fp, _constant[SDL_OCT_ENT], constant->value) < 0)
		    retVal = 0;
		break;

	    case SDL_K_RADIX_HEX:
		if (fprintf(fp, _constant[SDL_HEX_ENT], constant->value) < 0)
		    retVal = 0;
		break;

	    default:
		retVal = 0;
		break;
	}
    }

    /*
     * If there was a comment associated with this constant, then output that
     * as well.
     */
    if ((retVal == 1) && (constant->comment != NULL))
	if (fprintf(fp, _comments[SDL_LINE_COMMENT], constant->comment) < 0)
	    retVal = 0;

    /*
     * Move to the next line in the output file.
     */
    if ((retVal == 1) && (fprintf(fp, _newLine) < 0))
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}
