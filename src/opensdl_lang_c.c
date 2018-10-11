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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include "opensdl_lang.h"
#include "opensdl_utility.h"

extern char *sdl_months[];
extern _Bool trace;

static char *_types[SDL_K_BASE_TYPE_MAX][2] =
{
	/* 32-bit	64-bit	*/
	{"void ",	"void "},	/* NONE */
	{"char ",	"char "},	/* BYTE */
	{"char ",	"char "},	/* INTEGER_BYTE */
	{"short ",	"short "},	/* WORD */
	{"short ",	"short "},	/* INTEGER_WORD */
	{"int ",	"int "},	/* LONGWORD */
	{"int ",	"int "},	/* INTEGER_LONG */
	{"int ",	"int "},	/* INTEGER */
	{"int ",	"__int64_t "},	/* INTEGER_HW */
	{"int ",	"__int64_t "},	/* HARDWARE_INTEGER */
	{"__int64_t ",	"int "},	/* QUADWORD */
	{"__int64_t ",	"int "},	/* INTEGER_QUAD */
	{"__int64_t ",	"__int128_t "},	/* OCTAWORD */
	{"float ",	"float "},	/* T_FLOATING */
	{"float ",	"float "},	/* T_FLOATING COMPLEX */
	{"double ",	"double "},	/* S_FLOATING */
	{"double ",	"double "},	/* S_FLOATING COMPLEX */
	{"float ",	"float "},	/* F_FLOATING */
	{"float ",	"float "},	/* F_FLOATING COMPLEX */
	{"double ",	"double "},	/* D_FLOATING */
	{"double ",	"double "},	/* D_FLOATING COMPLEX */
	{"double ",	"double "},	/* G_FLOATING */
	{"double ",	"double "},	/* G_FLOATING COMPLEX */
	{"__int64_t ",	"__int128_t "},	/* H_FLOATING */
	{"__int64_t ",	"__int128_t "},	/* H_FLOATING COMPLEX */
	{"char ",	"char "},	/* DECIMAL */
	{"int ",	"int "},	/* BITFIELD */
	{"char ",	"char "},	/* BITFIELD BYTE */
	{"short ",	"short "},	/* BITFIELD WORD */
	{"int ",	"int "},	/* BITFIELD LONGWORD */
	{"__int64_t ",	"__int64_t "},	/* BITFIELD QUADWORD */
	{"__int64_t ",	"__int128_t "},	/* BITFIELD OCTAWORD */
	{"char ",	"char "},	/* CHAR */
	{NULL,		NULL},		/* CHAR VARYING */
	{NULL,		NULL},		/* CHAR LENGTH(*) */
	{NULL,		NULL},		/* ADDRESS */
	{NULL,		"int "},	/* ADDRESS_LONG */
	{NULL,		"__int64_t "},	/* ADDRESS_QUAD */
	{NULL,		"__int64_t "},	/* ADDRESS_HW */
	{NULL,		"__int64_t "},	/* HARDWARE_ADDRESS*/
	{NULL,		NULL},		/* POINTER */
	{NULL,		"int "},	/* POINTER_LONG */
	{NULL,		"__int64_t "},	/* POINTER_QUAD */
	{NULL,		"__int64_t "},	/* POINTER_HW */
	{NULL,		NULL},		/* ANY */
	{"void ",	"void "},	/* VOID */
	{"bool ",	"bool "},	/* BOOLEAN */
	{"struct ",	"struct "},	/* STRUCTURE */
	{"union ",	"union "},	/* UNION */
	{NULL,		NULL}		/* ENTRY */
};

/*
 * Local Prototypes
 */
static char *_sdl_c_generate_name(char *name, char *prefix, char *tag);
static char *_sdl_c_typeidStr(int typeID, SDL_CONTEXT *context);
static char *_sdl_c_leading_spaces(int depth);

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
	whichComment = "/*%s */";
    else if (startComment == true)
    {
	if (endComment == true)
	    whichComment = "\n/*%s*/";
	else if (strlen(comment) == 0)
	    whichComment = "\n/*";
	else
	    whichComment = "\n/*\n *%s";
    }
    else if (endComment == true)
    {
	if (strlen(comment) == 0)
	    whichComment = " */";
	else
	    whichComment = " *%s\n */";
    }
    else if (middleComment == true)
	whichComment = " *%s";
    else
	whichComment = "%s";

    /*
     * If the file has been opened, then write out the comment.
     */
    if (fp != NULL)
    {
	if (fprintf(fp, whichComment, comment) < 0)
	    retVal = 0;
	else if (fprintf(fp, "\n") < 0)
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
    int		retVal = 1;

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
	if (fprintf(fp, "\n/*** MODULE %s ", context->module) < 0)
	    retVal = 0;
	else if ((context->ident != NULL) && (strlen(context->ident) > 0))
	{
	    if (fprintf(fp, "IDENT = %s ", context->ident) < 0)
		retVal = 0;
	}
	if ((retVal == 1) && (fprintf(fp, "***/\n") < 0))
	    retVal = 0;
	if ((retVal == 1) &&
	    (fprintf(
		fp,
		"#include <stdint.h>\n"
		"#include <ctype.h>\n"
		"#include <sysbool.h>\n") < 0))
	    retVal = 0;
	if (retVal == 1)
	{
	    if (fprintf(
			fp,
			"\n#ifdef _%s_\n#define _%s_ 1\n"
			"#ifdef __cplusplus\nextern \"C\" {\n"
			"#endif\n",
			strupr(context->module),
			strupr(context->module)) < 0)
		retVal = 0;
	}

	/*
	 * Only do alignment pragmas if we are building for a 64-bit
	 * environment.
	 */
	if ((context->wordSize == 64) && (retVal == 1))
	{
	    if (fprintf(fp, "#pragma __member_alignment __save\n") < 0)
		retVal = 0;
	    else
	    {
		if (fprintf(
			fp,
			"#pragma __%smember_alignment\n",
			(context->memberAlign == true ? "" : "no")) < 0)
		    retVal = 0;
	    }
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
	if (fprintf(
		fp,
		"\n#ifdef __cplusplus\n}\n#endif\n\n#endif /* _%s_ */\n",
		strupr(context->module)) < 0)
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
    char	*sign = (item->_unsigned == true ? "unsigned " : "");
    char	*format = _sdl_c_typeidStr(item->type, context);
    int		bits = (context->wordSize / 32) - 1;	/* 0 = 32, 1 = 64 */
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_item\n", __FILE__, __LINE__);

    /*
     * Output the ITEM declaration.
     * TODO: We need to be able to handle typedefs and user defined types.
     * TODO: Varying
     * TODO: CHARACTER LENGTH X
     */
    if (fp != NULL)
    {
	switch (item->type)
	{
	    case SDL_K_TYPE_BYTE:
	    case SDL_K_TYPE_WORD:
	    case SDL_K_TYPE_LONG:
	    case SDL_K_TYPE_QUAD:
	    case SDL_K_TYPE_OCTA:
		if (fprintf(fp, format, sign) < 0)
		    retVal = 0;
		break;

	    case SDL_K_TYPE_BOOL:
	    case SDL_K_TYPE_TFLT:
	    case SDL_K_TYPE_SFLT:
	    case SDL_K_TYPE_DECIMAL:
	    case SDL_K_TYPE_CHAR:
	    case SDL_K_TYPE_ADDR:
	    case SDL_K_TYPE_ADDR_L:
	    case SDL_K_TYPE_ADDR_Q:
	    case SDL_K_TYPE_ADDR_HW:
	    case SDL_K_TYPE_HW_ADDR:
	    case SDL_K_TYPE_PTR:
	    case SDL_K_TYPE_PTR_L:
	    case SDL_K_TYPE_PTR_Q:
	    case SDL_K_TYPE_PTR_HW:
		if (fprintf(fp, format) < 0)
		    retVal = 0;
		break;

	    case SDL_K_TYPE_BITFLD:
		format = _sdl_c_typeidStr(item->bitfieldType, context);
		if (fprintf(fp, format, sign) < 0)
		    retVal = 0;
		break;

	    default:
		break;
	}
	if (retVal == 1)
	{
	    char *name = _sdl_c_generate_name(
				item->id,
				item->prefix,
				item->tag);

	    if (name != NULL)
	    {
		retVal = (fprintf(fp, name) < 0) ? 0 : 1;
		free(name);
	    }
	    else
		retVal = 0;
	}
	if (retVal == 1)
	{
	    if ((item->dimension == true) ||
		(item->type == SDL_K_TYPE_DECIMAL) ||
		(item->type == SDL_K_TYPE_CHAR))
	    {
		int	dim;

		if (item->dimension == true)
		    dim = item->hbound - item->lbound + 1;
		else if (item->type == SDL_K_TYPE_CHAR)
		    dim = item->length;
		else
		    dim = item->precision / 2  + 1;

		/*
		 * TODO:Need to handle CHARACTER LENGTH * definitions.
		 */
		if (dim > 0)
		{
		    if (fprintf(fp, "[%d]", dim) < 0)
			retVal = 0;
		}
	    }
	    else if ((item->type == SDL_K_TYPE_BITFLD) ||
		     (item->type == SDL_K_TYPE_BITFLD_B) ||
		     (item->type == SDL_K_TYPE_BITFLD_W) ||
		     (item->type == SDL_K_TYPE_BITFLD_L) ||
		     (item->type == SDL_K_TYPE_BITFLD_Q) ||
		     (item->type == SDL_K_TYPE_BITFLD_O))
	    {
		if (fprintf(
			fp,
			_types[item->type][bits],
			item->length) < 0)
		    retVal = 0;
	    }
	}
	if ((retVal == 1) && (fprintf(fp, ";\n") < 0))
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
    char 	*prefix = (constant->prefix ? constant->prefix : "");
    char	*name = _sdl_c_generate_name(constant->id, prefix, constant->tag);
    int		retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_constant\n", __FILE__, __LINE__);

    /*
     * Now let's do some output.
     *
     * First the #define <name> portion.
     */
    if (fprintf(fp, "#define %s\t", name) < 0)
	retVal = 0;

    /*
     * If first part was successful and this is a string constant, then output
     * the string value.
     */
    if (retVal == 1)
    {
	switch (constant->type)
	{
	    case SDL_K_CONST_STR:
		if (fprintf(fp, "\"%s\"\t", constant->string) < 0)
		    retVal = 0;
		break;

	    case SDL_K_CONST_NUM:
		switch (constant->radix)
		{
		    case SDL_K_RADIX_DEC:
			if (fprintf(fp, "%ld\t", constant->value) < 0)
			    retVal = 0;
			break;

		    case SDL_K_RADIX_OCT:
			if (fprintf(fp, "0%lo\t", constant->value) < 0)
			    retVal = 0;
			break;

		    case SDL_K_RADIX_HEX:
			if (fprintf(fp, "0x%lx\t", constant->value) < 0)
			    retVal = 0;
			break;

		    default:
			retVal = 0;
			break;
		}
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
	if (fprintf(fp, "/*%s */", constant->comment) < 0)
	    retVal = 0;

    /*
     * Move to the next line in the output file.
     */
    if ((retVal == 1) && (fprintf(fp, "\n") < 0))
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_aggregate
 *  This function is called after all the fields for an AGGREGATE keyword have
 *  been processed.  It is called to start the struct/union, as well as for
 *  each member in the struct/union.  It is also called after all the members
 *  have been written in order to close out the definition.  It writes out a
 *  single item (struct/union or member declaration) for each call to this.
 *  function.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  param:
 *	A pointer to the AGGREGATE, subaggregate, or ITEM record
 *  type:
 *	A value indicating the type of structure the param parameter
 *	represents.
 *  ending:
 *	A boolean value indicating that we are ending a definition.  This flag
 *	is used in the following way:
 *	    if (ending == true)
 *		if (type == LangAggregate)
 *		    <we are ending an aggregate>
 *		else if (type == LangSubaggregate)
 *		    <we are ending a subaggregate>
 *	    else if (type == LangAggregate)
 *		<we are starting an aggregate>
 *	    else if (type == LangSubaggregate)
 *		<we are starting a subaggregate>
 *	    else
 *		<we are defining a single member item>
 *  depth:
 *  	A value indicating the depth at which we are defining the next field.
 *  	This is used for indenting purposes.
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
int sdl_c_aggregate(
		FILE *fp,
		void *param,
		SDL_LANG_AGGR_TYPE type,
		bool ending,
		int depth,
		SDL_CONTEXT *context)
{
    char		*name = NULL;
    char		*spaces = _sdl_c_leading_spaces(depth);
    SDL_LANG_AGGR	aggr = { .parameter = param };
    int			bits = (context->wordSize / 32) - 1;  /* 0=32, 1=64 */
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_aggregate\n", __FILE__, __LINE__);

    if (fprintf(fp, spaces) < 0)
	retVal = 0;

    switch (type)
    {
	case LangAggregate:
	    name = _sdl_c_generate_name(
				aggr.aggr->id,
				aggr.aggr->prefix,
				aggr.aggr->tag);

	    /*
	     * Are we starting or ending an AGGREGATE?
	     */
	    if ((ending == false) && (retVal == 1) && (name != NULL))
	    {
		if (aggr.aggr->typeDef == true)
		{
		    if (fprintf(fp, "typedef ") < 0)
			retVal = 0;
		}
		if (retVal == 1)
		{
		    char *which = (aggr.aggr->structUnion == Structure ?
				_types[SDL_K_TYPE_STRUCT][bits] :
				_types[SDL_K_TYPE_UNION][bits]);
		    char *fmt = (aggr.aggr->typeDef == true ?
				"%s _%s\n%s{\n" :
				"%s %s\n%s{\n");

		    if (fprintf(fp, fmt, which, name, spaces) < 0)
			retVal = 0;
		}
	    }
	    else if ((retVal == 1) && (name != NULL))
	    {
		char *fmt = (aggr.aggr->typeDef == true ?
					"%s} %s;\n" :
					"%s};\n");
		if (fprintf(fp, fmt, spaces, name) < 0)
		    retVal = 0;
	    }
	    else if (name == NULL)
		retVal = 0;
	    break;

	case LangSubaggregate:
	    name = _sdl_c_generate_name(
				aggr.subaggr->id,
				aggr.subaggr->prefix,
				aggr.subaggr->tag);

	    /*
	     * Are we starting or ending an AGGREGATE?
	     */
	    if ((ending == false) && (retVal == 1) && (name != NULL))
	    {
		if (aggr.subaggr->typeDef == true)
		{
		    if (fprintf(fp, "%stypedef ", spaces) < 0)
			retVal = 0;
		}
		if (retVal == 1)
		{
		    char *which = (aggr.subaggr->structUnion == Structure ?
				_types[SDL_K_TYPE_STRUCT][bits] :
				_types[SDL_K_TYPE_UNION][bits]);
		    char *fmt = (aggr.subaggr->typeDef == true ?
				"%s _%s\n%s{\n" :
				"%s %s\n%s{\n");

		    if (fprintf(fp, fmt, which, name, spaces) < 0)
			    retVal = 0;
		}
	    }
	    else if ((retVal == 1) && (name != NULL))
	    {
		char *fmt = (aggr.subaggr->typeDef == true ?
					"%s} %s;\n" :
					"%s};\n");

		if (fprintf(fp, fmt, spaces, name) < 0)
		    retVal = 0;
	    }
	    else if (name == NULL)
		retVal = 0;
	    break;

	case LangItem:
	    if (retVal == 1)
		retVal = sdl_c_item(fp, aggr.item, context);
	    break;
    }

    /*
     * Deallocate any allocated memory.
     */
    if (name != NULL)
	free(name);
    if (spaces != NULL)
	free(spaces);

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_entry
 *  This function is called after all the fields for an ENTRY keyword have
 *  been processed.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  entry:
 *	A pointer to the ENTRY record
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
int sdl_c_entry(FILE *fp, SDL_ENTRY *entry, SDL_CONTEXT *context)
{
    SDL_PARAMETER	*param = (SDL_PARAMETER *) entry->parameters.flink;
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_entry\n", __FILE__, __LINE__);

    /*
     * If there is no type assigned to this ENTRY, then this is a procedure.
     * Otherwise, it is a function and we'll have to determine what the type
     * string should be for this.
     */
    if (entry->returns.type == SDL_K_TYPE_NONE)
    {
	if (fprintf(fp, "void %s(", entry->id) < 0)
	    retVal = 0;
    }
    else
    {
	char *typeStr = _sdl_c_typeidStr(entry->returns.type, context);

	if ((entry->returns.type >= SDL_K_BASE_TYPE_MIN) &&
	    (entry->returns.type <= SDL_K_BASE_TYPE_MAX))
	    switch (entry->returns.type)
	    {
		case SDL_K_TYPE_BYTE:
		case SDL_K_TYPE_WORD:
		case SDL_K_TYPE_LONG:
		case SDL_K_TYPE_QUAD:
		case SDL_K_TYPE_OCTA:
		    if (fprintf(
			    fp,
			    typeStr,
			    (entry->returns._unsigned ? "unsigned " : "")) < 0)
			retVal = 0;
		    break;

		case SDL_K_TYPE_BOOL:
		case SDL_K_TYPE_TFLT:
		case SDL_K_TYPE_SFLT:
		case SDL_K_TYPE_DECIMAL:
		case SDL_K_TYPE_CHAR:
		case SDL_K_TYPE_ADDR:
		case SDL_K_TYPE_ADDR_L:
		case SDL_K_TYPE_ADDR_Q:
		case SDL_K_TYPE_ADDR_HW:
		case SDL_K_TYPE_HW_ADDR:
		case SDL_K_TYPE_PTR:
		case SDL_K_TYPE_PTR_L:
		case SDL_K_TYPE_PTR_Q:
		case SDL_K_TYPE_PTR_HW:
		    if (fprintf(fp, typeStr) < 0)
			retVal = 0;
		    break;

		default:
		    break;
	    }
	else if ((entry->returns.type >= SDL_K_AGGREGATE_MIN) &&
	         (entry->returns.type <= SDL_K_AGGREGATE_MIN))
	{
	    if (fprintf(fp, "%s ", typeStr) < 0)
		retVal = 0;
	    else
		typeStr = entry->returns.name;
	}
	if ((retVal == 1) &&
	    (fprintf(fp, "%s %s(", typeStr, entry->id) < 0))
	    retVal = 0;
    }
    while ((retVal == 1) && (param != (SDL_PARAMETER *) &entry->parameters))
    {
	param = param->header.flink;
    }
    if ((retVal == 1) &&
	(fprintf(fp, ");\n") < 0))
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/************************************************************************
 * Local Functions							*
 ************************************************************************/

/*
 * _sdl_c_generate_name
 *  This function is called to take the name, prefix and tag, and generate a
 *  proper name.
 *
 * Input Parameters:
 *  name:
 *	A pointer to the name portion to have the prefix and tag information
 *	prepended to it.
 *  prefix:
 *	A pointer to the prefix portion of the name.  This parameter may be
 *	NULL.
 *  tag:
 *	A pointer to the tag portion of the name.  This parameter may not be
 *	NULL, but may be zero length.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:	An error occurred trying to allocate memory for the string.
 *  !NULL:	A pointer to the generated name.
 */
static char *_sdl_c_generate_name(char *name, char *prefix, char *tag)
{
    char	*retVal = NULL;
    size_t	len = 1;
    size_t	tagLen = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
	printf("%s:%d:_sdl_c_generate_name\n", __FILE__, __LINE__);
	if (prefix != NULL)
	    printf("\tprefix: %s\n", prefix);
	if (tag != NULL)
	    printf("\ttag: %s\n", tag);
	printf("\tname: %s\n", name);
    }

    /*
     * First, if we have a prefix, let's get it's length.
     */
    if (prefix != NULL)
	len += strlen(prefix);

    /*
     * Next look at the tag.  If is it null or zero length, then it is not
     * concatenated to the current string.  Otherwise, it is (with an
     * underscore between the prefix/tag and the name.
     */
    if (tag != NULL)
    {
	tagLen = strlen(tag);

	len += tagLen;
	if (tagLen > 0)
	    len++;
    }

    /*
     * Finally, allocate a buffer large enough and put each of the stings into
     * it.
     */
    len += strlen(name);
    retVal = calloc(1, len);
    len = 0;
    if (prefix != NULL)
    {
	strcpy(&retVal[len], prefix);
	len += strlen(prefix);
    }
    if ((tag != NULL) && (tagLen > 0))
    {
	strcpy(&retVal[len], tag);
	len += tagLen;
	retVal[len++] = '_';
    }
    strcpy(&retVal[len], name);

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

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
    int		bits = (context->wordSize / 32) - 1;	/* 0 = 32, 1 = 64 */

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_c_typeidStr\n", __FILE__, __LINE__);

    if ((typeID >= SDL_K_BASE_TYPE_MIN) && (typeID <= SDL_K_BASE_TYPE_MAX))
	retVal = _types[typeID][bits];
    else if ((typeID >= SDL_K_DECLARE_MIN) && (typeID <= SDL_K_DECLARE_MAX))
    {
	SDL_DECLARE *myDeclare = sdl_get_declare(&context->declares, typeID);

	if (myDeclare != NULL)
	    retVal = _sdl_c_typeidStr(myDeclare->type, context);
    }
    else if ((typeID >= SDL_K_ITEM_MIN) && (typeID <= SDL_K_ITEM_MAX))
    {
	SDL_ITEM *myItem = sdl_get_item(&context->items, typeID);

	if (myItem != NULL)
	    retVal = _sdl_c_typeidStr(myItem->type, context);
    }
    else if ((typeID >= SDL_K_AGGREGATE_MIN) && (typeID <= SDL_K_AGGREGATE_MAX))
    {
	SDL_AGGREGATE *myAggregate =
	    sdl_get_aggregate(&context->aggregates, typeID);

	if (myAggregate!= NULL)
	    retVal = _sdl_c_typeidStr(myAggregate->type, context);
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * _sdl_c_leading_spaces
 *  This function is called to determine and generate the number of leading
 *  spaces to be used while writing out a struct/union declaration.  For each
 *  depth, 4 spaces are added.  A tab will be used instead of 4 spaces when it
 *  make sense.
 *
 * Input Parameters:
 *  depth:
 *	A value indicating the struct/union depth we are currently.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:	An error occurred trying to allocate memory for the string.
 *  !NULL:	A pointer to the spaces to use for the output.
 */
static char *_sdl_c_leading_spaces(int depth)
{
    char	*retVal = NULL;
    int		spaces = depth * 4;
    int		tabs = spaces / 8;
    int		remaining = spaces - (tabs * 8);
    int		ii;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
	printf("%s:%d:_sdl_c_leading_spaces(%d)\n", __FILE__, __LINE__, depth);
	printf("\tspaces: %d\n\ttabs: %d\n\tremaining: %d\n", spaces, tabs, remaining);
    }

    /*
     * Allocate enough memory for the return string.
     */
    retVal = calloc(1, (tabs + remaining + 1));
    if (retVal != NULL)
    {

	/*
	 * First, insert zero or more tabs.
	 */
	for (ii = 0; ii < tabs; ii++)
	    retVal[ii] = '\t';

	/*
	 * Next, insert zero to four spaces.
	 */
	for (ii = tabs; ii < (tabs + remaining); ii++)
	    retVal[ii] = ' ';

	/*
	 * Finally, null-terminate the string.
	 */
	retVal[tabs + remaining] = '\0';
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}


