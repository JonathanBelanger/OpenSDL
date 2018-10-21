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

static char *_types[SDL_K_BASE_TYPE_MAX][2][2] =
{
	/* 32-bit	64-bit	*/
    {
	{"void",	"void"},	/* NONE */
	{"void",	"void"}		/* NONE */
    },
    {
	{"int8_t",	"int8_t"},	/* BYTE */
	{"uint8_t",	"uint8_t"}	/* BYTE */
    },
    {
	{"int8_t",	"int8_t"},	/* INTEGER_BYTE */
	{"uint8_t",	"uint8_t"}	/* INTEGER_BYTE */
    },
    {
	{"int16_t",	"int16_t"},	/* WORD */
	{"uint16_t",	"uint16_t"}	/* WORD */
    },
    {
	{"int16_t",	"int16_t"},	/* INTEGER_WORD */
	{"uint16_t",	"uint16_t"}	/* INTEGER_WORD */
    },
    {
	{"int32_t",	"int32_t"},	/* LONGWORD */
	{"uint32_t",	"uint32_t"}	/* LONGWORD */
    },
    {
	{"int32_t",	"int32_t"},	/* INTEGER_LONG */
	{"uint32_t",	"uint32_t"}	/* INTEGER_LONG */
    },
    {
	{"int",		"int"},		/* INTEGER */
	{"unsigned int","unsigned int"}	/* INTEGER */
    },
    {
	{"int32_t",	"int64_t"},	/* INTEGER_HW */
	{"uint32_t",	"uint64_t"}	/* INTEGER_HW */
    },
    {
	{"int32_t",	"int64_t"},	/* HARDWARE_INTEGER */
	{"uint32_t",	"uint64_t"}	/* HARDWARE_INTEGER */
    },
    {
	{"int64_t",	"int64_t"},	/* QUADWORD */
	{"uint64_t",	"uint64_t"}	/* QUADWORD */
    },
    {
	{"int64_t",	"int64_t"},	/* INTEGER_QUAD */
	{"uint64_t",	"uint64_t"}	/* INTEGER_QUAD */
    },
    {
	{"__int128_t",	"__int128_t"},	/* OCTAWORD */
	{"__uint128_t",	"__uint128_t"}	/* OCTAWORD */
    },
    {
	{"float",	"float"},	/* T_FLOATING */
	{"float",	"float"}	/* T_FLOATING */
    },
    {
	{"float complex","float complex"},/* T_FLOATING COMPLEX */
	{"float complex","float complex"} /* T_FLOATING COMPLEX */
    },
    {
	{"double",	"double"},	/* S_FLOATING */
	{"double",	"double"}	/* S_FLOATING */
    },
    {
	{"double complex","double complex"},/* S_FLOATING COMPLEX */
	{"double complex","double complex"} /* S_FLOATING COMPLEX */
    },
    {
	{"long double","long double"},	/* X_FLOATING */
	{"long double","long double"}	/* X_FLOATING */
    },
    {
	{"long double complex","long double complex"},/* X_FLOATING COMPLEX */
	{"long double complex","long double complex"} /* X_FLOATING COMPLEX */
    },
    {
	{"float",	"float"},	/* F_FLOATING */
	{"float",	"float"}	/* F_FLOATING */
    },
    {
	{"float",	"float"},	/* F_FLOATING COMPLEX */
	{"float",	"float"}	/* F_FLOATING COMPLEX */
    },
    {
	{"double",	"double"},	/* D_FLOATING */
	{"double",	"double"}	/* D_FLOATING */
    },
    {
	{"double",	"double"},	/* D_FLOATING COMPLEX */
	{"double",	"double"}	/* D_FLOATING COMPLEX */
    },
    {
	{"double",	"double"},	/* G_FLOATING */
	{"double",	"double"}	/* G_FLOATING */
    },
    {
	{"double",	"double"},	/* G_FLOATING COMPLEX */
	{"double",	"double"}	/* G_FLOATING COMPLEX */
    },
    {
	{"long double","long double"},	/* H_FLOATING */
	{"long double","long double"}	/* H_FLOATING */
    },
    {
	{"long double complex","long double complex"},/* H_FLOATING COMPLEX */
	{"long double complex","long double complex"} /* H_FLOATING COMPLEX */
    },
    {
	{"char",	"char"},	/* DECIMAL */
	{"char",	"char"}		/* DECIMAL */
    },
    {
	{"int",		"int"},		/* BITFIELD */
	{"unsigned int","unsigned int"}	/* BITFIELD */
    },
    {
	{"int8_t",	"int8_t"},	/* BITFIELD BYTE */
	{"uint8_t",	"uint8_t"}	/* BITFIELD BYTE */
    },
    {
	{"int16_t",	"int16_t"},	/* BITFIELD WORD */
	{"uint16_t",	"uint16_t"}	/* BITFIELD WORD */
    },
    {
	{"int32_t",	"int32_t"},	/* BITFIELD LONGWORD */
	{"uint32_t",	"uint32_t"}	/* BITFIELD LONGWORD */
    },
    {
	{"int64_t",	"int64_t"},	/* BITFIELD QUADWORD */
	{"uint64_t",	"uint64_t"}	/* BITFIELD QUADWORD */
    },
    {
	{"__int128_t",	"__int128_t"},	/* BITFIELD OCTAWORD */
	{"__uint128_t",	"__uint128_t"}	/* BITFIELD OCTAWORD */
    },
    {
	{"char",	"char"},	/* CHAR */
	{"char",	"char"}		/* CHAR */
    },
    {
	{NULL,		NULL},		/* CHAR VARYING */
	{NULL,		NULL},		/* CHAR VARYING */
    },
    {
	{"char",	"char"},	/* CHAR LENGTH(*) */
	{"char",	"char"}		/* CHAR LENGTH(*) */
    },
    {
	{NULL,		NULL},		/* ADDRESS */
	{NULL,		NULL}		/* ADDRESS */
    },
    {
	{"uint32_t",	"uint32_t"},	/* ADDRESS_LONG */
	{"uint32_t",	"uint32_t"}	/* ADDRESS_LONG */
    },
    {
	{"uint64_t",	"uint64_t"},	/* ADDRESS_QUAD */
	{"uint64_t",	"uint64_t"}	/* ADDRESS_QUAD */
    },
    {
	{"uint32_t",	"uint64_t"},	/* ADDRESS_HW */
	{"uint32_t",	"uint64_t"}	/* ADDRESS_HW */
    },
    {
	{"uint32_t",	"uint64_t"},	/* HARDWARE_ADDRESS*/
	{"uint32_t",	"uint64_t"}	/* HARDWARE_ADDRESS*/
    },
    {
	{NULL,		NULL},		/* POINTER */
	{NULL,		NULL}		/* POINTER */
    },
    {
	{"uint32_t",	"uint32_t"},	/* POINTER_LONG */
	{"uint32_t",	"uint32_t"}	/* POINTER_LONG */
    },
    {
	{"uint64_t",	"uint64_t"},	/* POINTER_QUAD */
	{"uint64_t",	"uint64_t"}	/* POINTER_QUAD */
    },
    {
	{"uint32_t",	"uint64_t"},	/* POINTER_HW */
	{"uint32_t",	"uint64_t"}	/* POINTER_HW */
    },
    {
	{"void",	"void"},	/* ANY */
	{"void",	"void"}		/* ANY */
    },
    {
	{"void",	"void"},	/* VOID */
	{"void",	"void"}		/* VOID */
    },
    {
	{"bool",	"bool"},	/* BOOLEAN */
	{"bool",	"bool"}		/* BOOLEAN */
    },
    {
	{"struct",	"struct"},	/* STRUCTURE */
	{"struct",	"struct"}	/* STRUCTURE */
    },
    {
	{"union",	"union"},	/* UNION */
	{"union",	"union"}	/* UNION */
    },
    {
	{"enum",	"enum"},	/* ENUM */
	{"enum",	"enum"}		/* ENUM */
    },
    {
	{NULL,		NULL},		/* ENTRY */
	{NULL,		NULL}		/* ENTRY */
    }
};

/*
 * Local Prototypes
 */
static int _sdl_c_output_alignment(
			FILE *fp,
			int alignment,
			SDL_CONTEXT *context);
static char *_sdl_c_generate_name(char *name, char *prefix, char *tag);
static char *_sdl_c_typeidStr(
			int typeID,
			int subType,
			bool _unsigned,
			SDL_CONTEXT *context,
			bool *freeMe);
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

	/*
	 * Write out the MODULE comment at near the top of the file.
	 */
	if (fprintf(fp, "\n/*** MODULE %s ", context->module) < 0)
	    retVal = 0;
	else if ((context->ident != NULL) && (strlen(context->ident) > 0))
	{
	    if (fprintf(fp, "IDENT = %s ", context->ident) < 0)
		retVal = 0;
	}
	if ((retVal == 1) && (fprintf(fp, "***/\n") < 0))
	    retVal = 0;

	/*
	 * We include some standard C headers that will simplify the rest of
	 * the definitions.
	 */
	if ((retVal == 1) &&
	    (fprintf(
		fp,
		"#include <ctype.h>\n"
		"#include <stdint.h>\n"
		"#include <stdbool.h>\n"
		"#include <complex.h>\n") < 0))
	    retVal = 0;

	/*
	 * We now put in the "if not defined" statements to make sure that this
	 * header file, itself, can only be included once.
	 */
	if (retVal == 1)
	{
	    if (fprintf(
			fp,
			"\n#ifndef _%s_\n#define _%s_ 1\n",
			strupr(context->module),
			strupr(context->module)) < 0)
		retVal = 0;
	}

	/*
	 * Finally, put in the items to allow C++ to be able to include this
	 * header file.
	 */
	if ((retVal == 1) && (fprintf(
				fp,
				"#ifdef __cplusplus\nextern \"C\" {\n"
				"#endif\n") < 0))
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

	/*
	 * Finally, close of the C++ mode and the close to only allow this
	 * header file to be included once.
	 */
	if (fprintf(
		fp,
		"\n#ifdef __cplusplus\n}\n#endif\n#endif /* _%s_ */\n",
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
    SDL_MEMBERS	dummy = { .type = item->type };
    char	*sign = (item->_unsigned == true ? "unsigned " : "");
    char	*type;
    char	*name = _sdl_c_generate_name(item->id, item->prefix, item->tag);
    int		retVal = 1;
    bool	freeMe = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_item\n", __FILE__, __LINE__);

    /*
     * Output the ITEM declaration.
     */
    if ((fp != NULL) && (name != NULL))
    {
	type = _sdl_c_typeidStr(
			item->type,
			item->subType,
			item->_unsigned,
			context,
			&freeMe);

	/*
	 * If typedef is indicated, then let's start with that.
	 */
	if (item->typeDef == true)
	{
	    if (fprintf(fp, "typedef ") < 0)
		retVal = 0;
	}
	else if (item->commonDef == true)
	{
	    if (fprintf(fp, "extern ") < 0)
		retVal = 0;
	}

	/*
	 * Now we need to output the type and name.
	 * TODO: We need to change the way unsigned is written (u for intxx_t,
	 * TODO: and __uint128_t for OCTAWORDs, and "unsigned " in front of
	 * TODO: others.
	 */
	if (retVal == 1)
	{
	    if (item->type == SDL_K_TYPE_CHAR_VARY)
	    {
		if (fprintf(
			fp,
			"struct {short string_length; "
			"char string_text[%ld] %s;",
			item->length,
			name) < 0)
		    retVal = 0;
	    }
	    else
	    {
		char *addr = ((item->type == SDL_K_TYPE_ADDR) ||
			      (item->type == SDL_K_TYPE_PTR)) ? "*" : "";
		if (fprintf(fp, "%s%s %s%s", sign, type, addr, name) < 0)
		    retVal = 0;
	    }
	}

	/*
	 * If there is a dimension specified, then generate that.
	 */
	if (retVal == 1)
	{
	    if (sdl_isBitfield(&dummy) == true)
	    {
		if (fprintf(fp, " : %ld", item->length) < 0)
		    retVal = 0;
	    }
	    else if ((item->dimension == true) ||
		     (item->type == SDL_K_TYPE_DECIMAL))
	    {
		int64_t	len;

		if (item->type == SDL_K_TYPE_DECIMAL)
		    len = (item->precision / 2) + 1;
		else
		    len = item->hbound - item->lbound + 1;
		if (fprintf(fp, "[%ld]", len) < 0)
		    retVal = 0;
	    }
	}

	/*
	 * Next, if there is an alignment need, then we add an attribute
	 * statement.
	 */
	if ((retVal == 1) && (item->parentAlignment == false))
	    retVal = _sdl_c_output_alignment(fp, item->alignment, context);

	/*
	 * Finally, we close off the declaration.
	 */
	if (retVal == 1)
	    if (fprintf(fp, ";\n") < 0)
		retVal = 0;
    }
    else
	retVal = 0;

    if (freeMe == true)
	free(type);

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
    char 	*prefix = constant->prefix;
    char	*name = _sdl_c_generate_name(constant->id, prefix, constant->tag);
    int		retVal = 1;
    int		size = constant->size * 8;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_constant\n", __FILE__, __LINE__);

    /*
     * Make sure we have the size set correctly.
     */
    if (size == 0)
	size = context->wordSize;

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
			if (fprintf(
				fp,
				"0%0*lo\t",
				(size / 3) + 1,
				constant->value) < 0)
			    retVal = 0;
			break;

		    case SDL_K_RADIX_HEX:
			if (fprintf(
				fp,
				"0x%0*lx\t",
				(size / 4),
				constant->value) < 0)
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
    SDL_LANG_AGGR	my = { .parameter = param };
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
				my.aggr->id,
				my.aggr->prefix,
				my.aggr->tag);

	    /*
	     * Are we starting or ending an AGGREGATE?
	     */
	    if ((ending == false) && (retVal == 1) && (name != NULL))
	    {
		if (my.aggr->commonDef == true)
		    if (fprintf(fp, "extern ") < 0)
			retVal = 0;
		if ((retVal == 1) && (my.aggr->typeDef == true))
		{
		    if (fprintf(fp, "typedef ") < 0)
			retVal = 0;
		}
		if (retVal == 1)
		{
		    char *which =
			    _types[my.aggr->aggType][bits][my.aggr->_unsigned];
		    char *td = (my.aggr->typeDef == true ? "_" : "");

		    if (fprintf(fp, "%s", which) < 0)
			retVal = 0;
		    if ((retVal == 1) && (my.aggr->alignmentPresent == true))
			retVal = _sdl_c_output_alignment(
						fp,
						my.aggr->alignment,
						context);
		    if ((retVal == 1) &&
			(fprintf(fp, " %s%s\n%s{\n", td, name, spaces) < 0))
			retVal = 0;
		}
	    }
	    else if ((retVal == 1) && (name != NULL))
	    {
		if (my.aggr->typeDef == true)
		{
		    if (fprintf(fp, "} %s", name) < 0)
			retVal = 0;
		}
		else
		{
		    if (fprintf(fp, "}") < 0)
			retVal = 0;
		}
		if ((retVal == 1) && (fprintf(fp, ";\n") < 0))
		    retVal = 0;
	    }
	    else if (name == NULL)
		retVal = 0;
	    break;

	case LangSubaggregate:
	    name = _sdl_c_generate_name(
				my.subaggr->id,
				my.subaggr->prefix,
				my.subaggr->tag);

	    /*
	     * Are we starting or ending an AGGREGATE?
	     */
	    if ((ending == false) && (retVal == 1) && (name != NULL))
	    {
		char *which =
			_types[my.subaggr->aggType][bits][my.subaggr->_unsigned];

		if (fprintf(fp, "%s ", which) < 0)
		    retVal = 0;
		if ((retVal == 1) && (my.subaggr->parentAlignment == false))
		    retVal = _sdl_c_output_alignment(
						fp,
						my.subaggr->alignment,
						context);
		if ((retVal == 1) && (fprintf(fp, "\n%s{\n", spaces) < 0))
		    retVal = 0;
	    }
	    else if ((retVal == 1) && (name != NULL))
	    {
		if (fprintf(fp, "} %s;\n", name) < 0)
		    retVal = 0;
	    }
	    else if (name == NULL)
		retVal = 0;
	    break;

	case LangItem:
	    if (retVal == 1)
		retVal = sdl_c_item(fp, my.item, context);
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
    char		*type;
    char		outBuf[256];
    size_t		outLen = 0;
    int			retVal = 1;
    bool		freeMe = false;
    bool		firstLine = false;

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
	type = _sdl_c_typeidStr(
			entry->returns.type,
			0,
			entry->returns._unsigned,
			context,
			&freeMe);
	if (entry->returns._unsigned == true)
	    outLen += sprintf(&outBuf[outLen], "unsigend ");
	outLen += sprintf(&outBuf[outLen], "%s ", type);
	if ((entry->returns.type == SDL_K_TYPE_ADDR) ||
	    (entry->returns.type == SDL_K_TYPE_PTR))
	    outLen += sprintf(&outBuf[outLen], "%s", "*");
	outLen += sprintf(&outBuf[outLen], "%s(", entry->id);
	if (fprintf(fp, outBuf) < 0)
	    retVal = 0;
	if ((freeMe == true) && (type != NULL))
	    free(type);
    }

    /*
     * OK, parameter passing can be rather complex, especially when you have
     * an address passed by reference.  So, we are going to need to break the
     * parameter output into multiple steps.
     */
    outLen = 0;
    while ((retVal == 1) && (param != (SDL_PARAMETER *) &entry->parameters))
    {
	type = _sdl_c_typeidStr(
			param->type,
			0,
			param->_unsigned,
			context,
			&freeMe);
	if (param->_unsigned == true)
	    outLen += sprintf(&outBuf[outLen], "unsigned ");
	outLen += sprintf(&outBuf[outLen], "%s", type);
	if ((param->type == SDL_K_TYPE_ADDR) ||
	    (param->type == SDL_K_TYPE_PTR) ||
	    (param->passingMech == SDL_K_PARAM_REF) ||
	    (param->name != NULL))
	    outLen += sprintf(&outBuf[outLen], " ");
	if ((param->type == SDL_K_TYPE_ADDR) ||
	    (param->type == SDL_K_TYPE_PTR) ||
	    (param->type == SDL_K_TYPE_CHAR_STAR))
	    outLen += sprintf(&outBuf[outLen], "%s", "*");
	if (param->passingMech == SDL_K_PARAM_REF)
	    outLen += sprintf(&outBuf[outLen], "%s", "*");
	if (param->name != NULL)
	    outLen += sprintf(&outBuf[outLen], param->name);
	if ((freeMe == true) && (type != NULL))
	    free(type);

	/*
	 * Move to the next parameter, if there is one.
	 */
	param = param->header.queue.flink;
	outLen = 0;

	if ((firstLine == false) &&
	    (param == (SDL_PARAMETER *) &entry->parameters))
	{
	    if (fprintf(fp, outBuf) < 0)		/* only parameter */
		retVal = 0;
	}
	else if ((firstLine == true) &&
		(param == (SDL_PARAMETER *) &entry->parameters))
	{
	    if (fprintf(fp, "\n\t%s", outBuf) < 0)	/* last parameter */
		retVal = 0;
	}
	else
	{
	    if (fprintf(fp, "\n\t%s,", outBuf) < 0)	/* other parameter */
		retVal = 0;
	    firstLine = true;
	}
    }
    if ((retVal == 1) && (fprintf(fp, ");\n") < 0))
	retVal = 0;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_c_enumerate
 *  This function is called after all the fields for an CONSTANT keyword with
 *  an ENUMERATE argument has been processed.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  _enum:
 *	A pointer to the ENUMERATE record
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
int sdl_c_enumerate(FILE *fp, SDL_ENUMERATE *_enum, SDL_CONTEXT *context)
{
    SDL_ENUM_MEMBER	*myMem = (SDL_ENUM_MEMBER *) _enum->members.flink;
    char		*name = _sdl_c_generate_name(
					_enum->id,
					_enum->prefix,
					_enum->tag);
    int			retVal = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_enumerate\n", __FILE__, __LINE__);

    if (name != NULL)
    {
	/*
	 * If typedef has been requested, then we need to start with that.
	 * Otherwise, we just start with an enum statement.
	 */
	if (_enum->typeDef == true)
	{
	    if (fprintf(fp, "typedef enum _%s\n{\n", name) < 0)
		retVal = 0;
	}
	else
	{
	    if (fprintf(fp, "enum %s\n{\n", name) < 0)
		retVal = 0;
	}

	/*
	 * Loop through each of the entries in the member list.
	 */
	while((retVal == 1) && (myMem != (SDL_ENUM_MEMBER *) &_enum->members))
	{
	    if (fprintf(fp, "    %s", myMem->id) < 0)
		retVal = 0;
	    else
	    {
		if (myMem->valueSet == true)
		{
		    if (fprintf(fp, " = %ld,\n", myMem->value) < 0)
			retVal = 0;
		}
		else if (fprintf(fp, ",\n") < 0)
		    retVal = 0;
	    }
	}

	/*
	 * Finally, we need to close of the enum definition.
	 */
	if ((retVal == 1) && (fprintf(fp, "}") < 0))
	    retVal = 0;
	if ((retVal == 1) && (_enum->typeDef == true))
	{
	    if (fprintf(fp, " %s", name) < 0)
		retVal = 0;
	}
	if (retVal == 1)
	    retVal = _sdl_c_output_alignment(
					fp,
					_enum->alignment,
					context);
	if ((retVal == 1) && (fprintf(fp, ";\n") < 0))
	    retVal = 0;
	free(name);
    }
    else
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
 * _sdl_c_output_alignment
 *  This function is called to output the alignment definitions for ITEMs,
 *  AGGREGATEs,and subaggregates.
 *
 * Input Parameters:
 *  fp:
 *	A pointer to the file pointer to write out the information.
 *  alignment:
 *  	A value indicating the alignment requirement.
 *  context:
 *	A pointer to the context block to be used for converting a type into
 *	a string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  1:	Normal Successful Completion.
 *  0:	An error occurred.
 */
static int _sdl_c_output_alignment(
			FILE *fp,
			int alignment,
			SDL_CONTEXT *context)
{
    int		retVal = 1;

    if (context->memberAlign == true)
    {
	switch(alignment)
	{
	    case SDL_K_NOALIGN:
		if (fprintf(fp, " __attribute__ ((__packed__))") < 0)
		    retVal = 0;
		break;

	    case SDL_K_ALIGN:
		if (fprintf(fp, " __attribute__ ((aligned))") < 0)
		    retVal = 0;
		break;

	    default:
		if (fprintf(
			fp,
			" __attribute__ ((aligned (%d)))",
			alignment) < 0)
		    retVal = 0;
		break;
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

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
     * First, if we have a prefix, let's get it's length.  Also, the presents
     * of the prefix indicates that we'll also add the tag.  If the prefix is
     * not present, then the tag will not be added either.
     */
    if (prefix != NULL)
    {
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
	if ((tag != NULL) && (tagLen > 0))
	{
	    strcpy(&retVal[len], tag);
	    len += tagLen;
	    retVal[len++] = '_';
	}
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
 *  subType:
 *	A value indicating the subType to be converted, when the type is
 *	ADDRESS or POINTER.
 *  _unsigned:
 *	A boolean value indicating whether we should get the signed or unsigned
 *	version of the datatype (only really valid for INTEGERs).
 *  context:
 *	A pointer to the context block to be used for converting a type into
 *	a string.
 *
 * Output Parameters:
 *  freeMe:
 *	A pointer to a boolean to receive an indicator that the returned string
 *	needs to be freed.
 *
 * Return Values:
 *  NULL:	Failed to convert the type to a string.
 *  !NULL:	A pointer to the type string to be written out to the output
 *		file.
 */
static char *_sdl_c_typeidStr(
			int typeID,
			int subType,
			bool _unsigned,
			SDL_CONTEXT *context,
			bool *freeMe)
{
    char	*retVal = NULL;
    int		bits = (context->wordSize / 32) - 1;	/* 0 = 32, 1 = 64 */

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_c_typeidStr\n", __FILE__, __LINE__);

    *freeMe = false;
    if (typeID == SDL_K_TYPE_NONE)
	retVal = _types[SDL_K_TYPE_VOID][bits][_unsigned];
    else if ((typeID >= SDL_K_BASE_TYPE_MIN) && (typeID <= SDL_K_BASE_TYPE_MAX))
    {
	if ((typeID == SDL_K_TYPE_ADDR) || (typeID == SDL_K_TYPE_PTR))
	    retVal = _sdl_c_typeidStr(subType, 0, _unsigned, context, freeMe);
	else
	    retVal = _types[typeID][bits][_unsigned];
    }
    else if ((typeID >= SDL_K_DECLARE_MIN) && (typeID <= SDL_K_DECLARE_MAX))
    {
	SDL_DECLARE *myDeclare = sdl_get_declare(&context->declares, typeID);

	if (myDeclare != NULL)
	    retVal = _sdl_c_generate_name(
				myDeclare->id,
				myDeclare->prefix,
				myDeclare->tag);
	if (retVal != NULL)
	    *freeMe = true;
    }
    else if ((typeID >= SDL_K_ITEM_MIN) && (typeID <= SDL_K_ITEM_MAX))
    {
	SDL_ITEM *myItem = sdl_get_item(&context->items, typeID);

	if (myItem != NULL)
	    retVal = _sdl_c_typeidStr(
				myItem->type,
				subType,
				_unsigned,
				context,
				freeMe);
    }
    else if ((typeID >= SDL_K_AGGREGATE_MIN) && (typeID <= SDL_K_AGGREGATE_MAX))
    {
	SDL_AGGREGATE *myAggregate =
	    sdl_get_aggregate(&context->aggregates, typeID);

	if (myAggregate!= NULL)
	    retVal = _sdl_c_typeidStr(
				myAggregate->type,
				subType,
				_unsigned,
				context,
				freeMe);
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
