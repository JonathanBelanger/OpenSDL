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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include "opensdl_lang.h"
#include "opensdl_utility.h"
#include "opensdl_message.h"
#include "opensdl_main.h"

extern char *sdl_months[];
extern _Bool trace;

static char *_types[SDL_K_BASE_TYPE_MAX][2][2] =
{
	/* signed	unsigned */
    {					/*  0 */
	{"void",	"void"},	/* NONE 32-bit */
	{"void",	"void"}		/* NONE 64-bit */
    },
    {					/*  1 */
	{"int8_t",	"uint8_t"},	/* BYTE 32-bit */
	{"int8_t",	"uint8_t"}	/* BYTE 64-bit */
    },
    {					/*  2 */
	{"int8_t",	"uint8_t"},	/* INTEGER_BYTE 32-bit */
	{"int8_t",	"uint8_t"}	/* INTEGER_BYTE 64-bit */
    },
    {					/*  3 */
	{"int16_t",	"uint16_t"},	/* WORD 32-bit */
	{"int16_t",	"uint16_t"}	/* WORD 64-bit */
    },
    {					/*  4 */
	{"int16_t",	"uint16_t"},	/* INTEGER_WORD 32-bit */
	{"int16_t",	"uint16_t"}	/* INTEGER_WORD 64-bit */
    },
    {					/*  5 */
	{"int32_t",	"uint32_t"},	/* LONGWORD 32-bit */
	{"int32_t",	"uint32_t"}	/* LONGWORD 64-bit */
    },
    {					/*  6 */
	{"int32_t",	"uint32_t"},	/* INTEGER_LONG 32-bit */
	{"int32_t",	"uint32_t"}	/* INTEGER_LONG 64-bit */
    },
    {					/*  7 */
	{"int",		"unsigned int"},/* INTEGER 32-bit */
	{"int",		"unsigned int"}	/* INTEGER 64-bit */
    },
    {					/*  8 */
	{"int32_t",	"uint64_t"},	/* INTEGER_HW 32-bit */
	{"int32_t",	"uint64_t"}	/* INTEGER_HW 64-bit */
    },
    {					/*  9 */
	{"int32_t",	"uint64_t"},	/* HARDWARE_INTEGER 32-bit */
	{"int32_t",	"uint64_t"}	/* HARDWARE_INTEGER 64-bit */
    },
    {					/* 10 */
	{"int64_t",	"uint64_t"},	/* QUADWORD 32-bit */
	{"int64_t",	"uint64_t"}	/* QUADWORD 64-bit */
    },
    {					/* 11 */
	{"int64_t",	"uint64_t"},	/* INTEGER_QUAD 32-bit */
	{"int64_t",	"uint64_t"}	/* INTEGER_QUAD 64-bit */
    },
    {					/* 12 */
	{"__int128_t",	"__uint128_t"},	/* OCTAWORD 32-bit */
	{"__int128_t",	"__uint128_t"}	/* OCTAWORD 64-bit */
    },
    {					/* 13 */
	{"float",	"float"},	/* T_FLOATING 32-bit */
	{"float",	"float"}	/* T_FLOATING 64-bit */
    },
    {					/* 14 */
	{"float complex","float complex"},/* T_FLOATING COMPLEX 32-bit */
	{"float complex","float complex"} /* T_FLOATING COMPLEX 64-bit */
    },
    {					/* 15 */
	{"double",	"double"},	/* S_FLOATING 32-bit */
	{"double",	"double"}	/* S_FLOATING 64-bit */
    },
    {					/* 16 */
	{"double complex","double complex"},/* S_FLOATING COMPLEX 32-bit */
	{"double complex","double complex"} /* S_FLOATING COMPLEX 64-bit */
    },
    {					/* 17 */
	{"long double","long double"},	/* X_FLOATING 32-bit */
	{"long double","long double"}	/* X_FLOATING 64-bit */
    },
    {					/* 18 */
	{"long double complex","long double complex"},/* X_FLOATING COMPLEX 32-bit */
	{"long double complex","long double complex"} /* X_FLOATING COMPLEX 64-bit */
    },
    {					/* 19 */
	{"float",	"float"},	/* F_FLOATING 32-bit */
	{"float",	"float"}	/* F_FLOATING 64-bit */
    },
    {					/* 20 */
	{"float",	"float"},	/* F_FLOATING COMPLEX 32-bit */
	{"float",	"float"}	/* F_FLOATING COMPLEX 64-bit */
    },
    {					/* 21 */
	{"double",	"double"},	/* D_FLOATING 32-bit */
	{"double",	"double"}	/* D_FLOATING 64-bit */
    },
    {					/* 22 */
	{"double",	"double"},	/* D_FLOATING COMPLEX 32-bit */
	{"double",	"double"}	/* D_FLOATING COMPLEX 64-bit */
    },
    {					/* 23 */
	{"double",	"double"},	/* G_FLOATING 32-bit */
	{"double",	"double"}	/* G_FLOATING 64-bit */
    },
    {					/* 24 */
	{"double",	"double"},	/* G_FLOATING COMPLEX 32-bit */
	{"double",	"double"}	/* G_FLOATING COMPLEX 64-bit */
    },
    {					/* 25 */
	{"long double","long double"},	/* H_FLOATING 32-bit */
	{"long double","long double"}	/* H_FLOATING 64-bit */
    },
    {					/* 26 */
	{"long double complex","long double complex"},/* H_FLOATING COMPLEX 32-bit */
	{"long double complex","long double complex"} /* H_FLOATING COMPLEX 64-bit */
    },
    {					/* 27 */
	{"char",	"char"},	/* DECIMAL 32-bit */
	{"char",	"char"}		/* DECIMAL 64-bit */
    },
    {					/* 28 */
	{"int",		"unsigned int"},/* BITFIELD 32-bit */
	{"int",		"unsigned int"}	/* BITFIELD 64-bit */
    },
    {					/* 29 */
	{"int8_t",	"uint8_t"},	/* BITFIELD BYTE 32-bit */
	{"int8_t",	"uint8_t"}	/* BITFIELD BYTE 64-bit */
    },
    {					/* 30 */
	{"int16_t",	"uint16_t"},	/* BITFIELD WORD 32-bit */
	{"int16_t",	"uint16_t"}	/* BITFIELD WORD 64-bit */
    },
    {					/* 31 */
	{"int32_t",	"uint32_t"},	/* BITFIELD LONGWORD 32-bit */
	{"int32_t",	"uint32_t"}	/* BITFIELD LONGWORD 64-bit */
    },
    {					/* 32 */
	{"int64_t",	"uint64_t"},	/* BITFIELD QUADWORD 32-bit */
	{"int64_t",	"uint64_t"}	/* BITFIELD QUADWORD 64-bit */
    },
    {					/* 33 */
	{"__int128_t",	"__uint128_t"},	/* BITFIELD OCTAWORD 32-bit */
	{"__int128_t",	"__uint128_t"}	/* BITFIELD OCTAWORD 64-bit */
    },
    {					/* 34 */
	{"char",	"char"},	/* CHAR 32-bit */
	{"char",	"char"}		/* CHAR 64-bit */
    },
    {					/* 35 */
	{NULL,		NULL},		/* CHAR VARYING 32-bit */
	{NULL,		NULL},		/* CHAR VARYING 64-bit */
    },
    {					/* 36 */
	{"char",	"char"},	/* CHAR LENGTH(*) 32-bit */
	{"char",	"char"}		/* CHAR LENGTH(*) 64-bit */
    },
    {					/* 37 */
	{NULL,		NULL},		/* ADDRESS 32-bit */
	{NULL,		NULL}		/* ADDRESS 64-bit */
    },
    {					/* 38 */
	{"uint32_t",	"uint32_t"},	/* ADDRESS_LONG  32-bit*/
	{"uint32_t",	"uint32_t"}	/* ADDRESS_LONG 64-bit */
    },
    {					/* 39 */
	{"uint64_t",	"uint64_t"},	/* ADDRESS_QUAD 32-bit */
	{"uint64_t",	"uint64_t"}	/* ADDRESS_QUAD 64-bit */
    },
    {					/* 40 */
	{"uint32_t",	"uint64_t"},	/* ADDRESS_HW 32-bit */
	{"uint32_t",	"uint64_t"}	/* ADDRESS_HW 64-bit */
    },
    {					/* 41 */
	{"uint32_t",	"uint64_t"},	/* HARDWARE_ADDRESS 32-bit */
	{"uint32_t",	"uint64_t"}	/* HARDWARE_ADDRESS 64-bit */
    },
    {					/* 42 */
	{NULL,		NULL},		/* POINTER 32-bit */
	{NULL,		NULL}		/* POINTER 64-bit */
    },
    {					/* 43 */
	{"uint32_t",	"uint32_t"},	/* POINTER_LONG 32-bit */
	{"uint32_t",	"uint32_t"}	/* POINTER_LONG 64-bit */
    },
    {					/* 44 */
	{"uint64_t",	"uint64_t"},	/* POINTER_QUAD 32-bit */
	{"uint64_t",	"uint64_t"}	/* POINTER_QUAD 64-bit */
    },
    {					/* 45 */
	{"uint32_t",	"uint64_t"},	/* POINTER_HW 32-bit */
	{"uint32_t",	"uint64_t"}	/* POINTER_HW 64-bit */
    },
    {					/* 46 */
	{"void",	"void"},	/* ANY 32-bit */
	{"void",	"void"}		/* ANY 64-bit */
    },
    {					/* 47 */
	{"void",	"void"},	/* VOID 32-bit */
	{"void",	"void"}		/* VOID 64-bit */
    },
    {					/* 48 */
	{"bool",	"bool"},	/* BOOLEAN 32-bit */
	{"bool",	"bool"}		/* BOOLEAN 64-bit */
    },
    {					/* 49 */
	{"struct",	"struct"},	/* STRUCTURE 32-bit */
	{"struct",	"struct"}	/* STRUCTURE 64-bit */
    },
    {					/* 50 */
	{"union",	"union"},	/* UNION 32-bit */
	{"union",	"union"}	/* UNION 64-bit */
    },
    {					/* 51 */
	{"enum",	"enum"},	/* ENUM 32-bit */
	{"enum",	"enum"}		/* ENUM 64-bit */
    },
    {					/* 52 */
	{NULL,		NULL},		/* ENTRY 32-bit */
	{NULL,		NULL}		/* ENTRY 64-bit */
    }
};

/*
 * Local Prototypes
 */
static uint32_t _sdl_c_output_alignment(
		FILE *fp,
		int alignment,
		SDL_CONTEXT *context);
static char *_sdl_c_generate_name(
		char *name,
		char *prefix,
		char *tag,
		SDL_CONTEXT *context);
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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_commentStars(FILE *fp)
{
    char	str[SDL_K_COMMENT_LEN];
    uint32_t	retVal = SDL_NORMAL;
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
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_createdByInfo(FILE *fp, struct tm *timeInfo)
{
    char	str[SDL_K_COMMENT_LEN];
    char	*timeFmt = "/* Created %02d-%s-%04d %02d:%02d:%02d by OpenSDL "
			   "%c%d.%d-%d";
    uint32_t	retVal = SDL_NORMAL;
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
    if (fprintf(fp, "%s\n", str) < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }


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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_fileInfo(FILE *fp, struct tm *timeInfo, char *fullFilePath)
{
    char	str[SDL_K_COMMENT_LEN];
    char	*sourceFmt = "/* Source: %02d-%s-%04d %02d:%02d:%02d ";
    char	*ptr;
    uint32_t	retVal = SDL_NORMAL;
    int		len, remLen, fileLen = strlen(fullFilePath);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_fileInfo\n", __FILE__, __LINE__);

    /*
     * Set the output string to all spaces and the closing comment.  This will
     * help later on.
     */
    memset(str, ' ', SDL_K_COMMENT_LEN);
    str[SDL_K_COMMENT_LEN - 3] = '*';
    str[SDL_K_COMMENT_LEN - 2] = '/';
    str[SDL_K_COMMENT_LEN - 1] = '\0';

    /*
     * Generate the file time information string.
     */
    len = sprintf(
		str,
		sourceFmt,
		timeInfo->tm_mday,
		sdl_months[timeInfo->tm_mon],
		(1900 + timeInfo->tm_year),
		timeInfo->tm_hour,
		timeInfo->tm_min,
		timeInfo->tm_sec);

    /*
     * New determine how much of the full file path will fit into the remaining
     * length of the file portion of the comment string.
     */
    remLen = SDL_K_COMMENT_LEN - len - 4;
    if (remLen < fileLen)
    {
	str[len++] = '.';
	str[len++] = '.';
	str[len++] = '.';
	ptr = &fullFilePath[fileLen - remLen + 3];
    }
    else
	ptr = fullFilePath;

    /*
     * Now copy the filename information that will fit into the comment string.
     * This will be the last part of the filename.
     */
    while (*ptr != '\0')
    {
	str[len++] = *ptr;
	ptr++;
    }

    /*
     * Write out the string to the output file.
     */
    if (fprintf(fp, "%s\n", str) < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_comment(
		FILE *fp,
		char *comment,
		bool lineComment,
		bool startComment,
		bool middleComment,
		bool endComment)
{
    uint32_t	retVal = SDL_NORMAL;
    char *	whichComment;

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

    if (fprintf(fp, whichComment, comment) < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }
    else if (fprintf(fp, "\n") < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_module(FILE *fp, SDL_CONTEXT *context)
{
    uint32_t	retVal = SDL_NORMAL;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_module\n", __FILE__, __LINE__);

    /*
     * Write out the MODULE comment at near the top of the file.
     */
    if (fprintf(fp, "\n/*** MODULE %s ", context->module) < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }
    else if ((context->ident != NULL) && (strlen(context->ident) > 0))
    {
	if (fprintf(fp, "IDENT = %s ", context->ident) < 0)
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}
    }
    if ((retVal == SDL_NORMAL) && (fprintf(fp, "***/\n") < 0))
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

    /*
     * We include some standard C headers that will simplify the rest of
     * the definitions.
     */
    if ((retVal == SDL_NORMAL) &&
	(fprintf(
		fp,
		"#include <ctype.h>\n"
		"#include <stdint.h>\n"
		"#include <stdbool.h>\n"
		"#include <complex.h>\n") < 0))
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

    /*
     * We now put in the "if not defined" statements to make sure that this
     * header file, itself, can only be included once.
     */
    if (retVal == SDL_NORMAL)
    {
	if (fprintf(
		fp,
		"\n#ifndef _%s_\n#define _%s_ 1\n",
		strupr(context->module),
		strupr(context->module)) < 0)
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}
    }

    /*
     * Finally, put in the items to allow C++ to be able to include this
     * header file.
     */
    if ((retVal == SDL_NORMAL) &&
	(fprintf(
		fp,
		"#ifdef __cplusplus\nextern \"C\" {\n"
		"#endif\n") < 0))
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_module_end(FILE *fp, SDL_CONTEXT *context)
{
    uint32_t	retVal = SDL_NORMAL;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_cmodule_end\n", __FILE__, __LINE__);

    /*
     * Finally, close of the C++ mode and the close to only allow this
     * header file to be included once.
     */
    if (fprintf(
		fp,
		"\n#ifdef __cplusplus\n}\n#endif\n#endif /* _%s_ */\n",
		strupr(context->module)) < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_INVNAME:	Invalid item name specified.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_item(FILE *fp, SDL_ITEM *item, SDL_CONTEXT *context)
{
    SDL_MEMBERS	dummy = { .type = item->type };
    char	*type;
    char	*name = _sdl_c_generate_name(
					item->id,
					item->prefix,
					item->tag,
					context);
    uint32_t	retVal = SDL_NORMAL;
    bool	freeMe = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_item\n", __FILE__, __LINE__);

    /*
     * Output the ITEM declaration.
     */
    if (name != NULL)
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
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
	    }
	}
	else if (item->commonDef == true)
	{
	    if (fprintf(fp, "extern ") < 0)
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
	    }
	}

	/*
	 * Now we need to output the type and name.
	 */
	if (retVal == SDL_NORMAL)
	{
	    if (item->type == SDL_K_TYPE_CHAR_VARY)
	    {
		if (fprintf(
			fp,
			"struct {short string_length; "
			"char string_text[%ld];} %s",
			item->length,
			name) < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
	    }
	    else
	    {
		char *addr = ((item->type == SDL_K_TYPE_ADDR) ||
			      (item->type == SDL_K_TYPE_PTR)) ? "*" : "";

		if (fprintf(fp, "%s %s%s", type, addr, name) < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
	    }
	}

	/*
	 * If there is a dimension specified, then generate that.
	 */
	if (retVal == SDL_NORMAL)
	{
	    if (sdl_isBitfield(&dummy) == true)
	    {
		if (fprintf(fp, " : %ld", item->length) < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
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
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
	    }
	    else if ((item->length > 0) && (item->type == SDL_K_TYPE_CHAR))
	    {
		if (fprintf(fp, "[%ld]", item->length) < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
	    }
	}

	/*
	 * Next, if there is an alignment need, then we add an attribute
	 * statement.
	 */
	if ((retVal == SDL_NORMAL) && (item->parentAlignment == false))
	    retVal = _sdl_c_output_alignment(fp, item->alignment, context);

	/*
	 * Finally, we close off the declaration.
	 */
	if (retVal == SDL_NORMAL)
	    if (fprintf(fp, ";\n") < 0)
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
	    }
    }
    else
    {
	retVal = SDL_INVNAME;
	if (sdl_set_message(
			msgVec,
			1,
			retVal) != SDL_NORMAL)
	    retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_UNKCONSTTYP:	Unknown constant type specified.
 *  SDL_UNKRADIX:	Unknown radix specified.
 *  SDL_ABORT:		An unexpected error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_constant(FILE *fp, SDL_CONSTANT *constant, SDL_CONTEXT *context)
{
    char 	*prefix = constant->prefix;
    char	*name = _sdl_c_generate_name(
					constant->id,
					prefix,
					constant->tag,
					context);
    uint32_t	retVal = SDL_NORMAL;
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
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
	    retVal = SDL_ERREXIT;
    }

    /*
     * If first part was successful and this is a string constant, then output
     * the string value.
     */
    if (retVal == SDL_NORMAL)
    {
	switch (constant->type)
	{
	    case SDL_K_CONST_STR:
		if (fprintf(fp, "\"%s\"\t", constant->string) < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
		break;

	    case SDL_K_CONST_NUM:
		switch (constant->radix)
		{
		    case SDL_K_RADIX_DEC:
			if (fprintf(fp, "%ld\t", constant->value) < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
			break;

		    case SDL_K_RADIX_OCT:
			if (fprintf(
				fp,
				"0%0*lo\t",
				(size / 3) + 1,
				constant->value) < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
			break;

		    case SDL_K_RADIX_HEX:
			if (fprintf(
				fp,
				"0x%0*lx\t",
				(size / 4),
				constant->value) < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
			break;

		    default:
			retVal = SDL_UNKRADIX;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					constant->radix,
					constant->srcLineNo) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
			break;
		}
		break;

	    default:
		retVal = SDL_UNKCONSTTYP;
		if (sdl_set_message(
				msgVec,
				1,
				retVal,
				constant->type,
				constant->srcLineNo) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
		break;
	}
    }

    /*
     * If there was a comment associated with this constant, then output that
     * as well.
     */
    if ((retVal == SDL_NORMAL) && (constant->comment != NULL))
	if (fprintf(fp, "/*%s */", constant->comment) < 0)
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}

    /*
     * Move to the next line in the output file.
     */
    if ((retVal == SDL_NORMAL) && (fprintf(fp, "\n") < 0))
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
	    retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_INVAGGRNAM:	Invalid aggregate name.
 *  SDL_ABORT:		An error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_aggregate(
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
    uint32_t		retVal = SDL_NORMAL;
    bool		defVariable = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_c_aggregate\n", __FILE__, __LINE__);

    if (fprintf(fp, spaces) < 0)
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
	    retVal = SDL_ERREXIT;
    }

    switch (type)
    {
	case LangAggregate:
	    name = _sdl_c_generate_name(
				my.aggr->id,
				my.aggr->marker,
				my.aggr->tag,
				context);
	    if ((name != NULL) && (retVal == SDL_NORMAL))
	    {

		/*
		 * Are we starting or ending an AGGREGATE?
		 */
		if (ending == false)
		{
		    if (my.aggr->commonDef == true)
			if (fprintf(fp, "extern ") < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
		    if ((retVal == SDL_NORMAL) && (my.aggr->typeDef == true))
		    {
			if (fprintf(fp, "typedef ") < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
		    }
		    defVariable = my.aggr->commonDef || my.aggr->typeDef;
		    if (retVal == SDL_NORMAL)
		    {
			char *which =
				_types[my.aggr->aggType][bits][my.aggr->_unsigned];
			char *td = (defVariable == true ? "_" : "");

			if (fprintf(fp, "%s", which) < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
			if ((retVal == SDL_NORMAL) &&
			    (my.aggr->alignmentPresent == true))
			    retVal = _sdl_c_output_alignment(
						    fp,
						    my.aggr->alignment,
						    context);
			if ((retVal == SDL_NORMAL) &&
			    (fprintf(fp, " %s%s\n%s{\n", td, name, spaces) < 0))
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
		    }
		}
		else
		{
		    defVariable = my.aggr->commonDef || my.aggr->typeDef;
		    if (defVariable == true)
		    {
			if (fprintf(fp, "} %s", name) < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
			else if (my.aggr->dimension == true)
			{
			    int64_t dimension = my.aggr->hbound -
						my.aggr->lbound + 1;

			    if (fprintf(fp, "[%ld]", dimension) < 0)
			    {
				retVal = SDL_ABORT;
				if (sdl_set_message(
						msgVec,
						2,
						retVal,
						errno) != SDL_NORMAL)
				    retVal = SDL_ERREXIT;
			    }
			}
		    }
		    else
		    {
			if (fprintf(fp, "}") < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
		    }
		    if ((retVal == SDL_NORMAL) && (fprintf(fp, ";\n") < 0))
		    {
			retVal = SDL_ABORT;
			if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
		    }
		}
	    }
	    else
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				ENOMEM) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	    break;

	case LangSubaggregate:
	    name = _sdl_c_generate_name(
				my.subaggr->id,
				my.subaggr->prefix,
				my.subaggr->tag,
				context);
	    if ((name != NULL) && (retVal == SDL_NORMAL))
	    {

		/*
		 * Are we starting or ending an AGGREGATE?
		 */
		if (ending == false)
		{
		    char *which =
			    _types[my.subaggr->aggType][bits][my.subaggr->_unsigned];

		    if (fprintf(fp, "%s ", which) < 0)
		    {
			retVal = SDL_ABORT;
			if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
		    }
		    if ((retVal == SDL_NORMAL) &&
			(my.subaggr->parentAlignment == false))
			retVal = _sdl_c_output_alignment(
						    fp,
						    my.subaggr->alignment,
						    context);
		    if ((retVal == SDL_NORMAL) &&
			(fprintf(fp, "\n%s{\n", spaces) < 0))
		    {
			retVal = SDL_ABORT;
			if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
		    }
		}
		else
		{
		    if (fprintf(fp, "} %s", name) < 0)
		    {
			retVal = SDL_ABORT;
			if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
		    }
		    else if (my.subaggr->dimension == true)
		    {
			int64_t dimension = my.subaggr->hbound -
					    my.subaggr->lbound + 1;

			if (fprintf(fp, "[%ld]", dimension) < 0)
			{
			    retVal = SDL_ABORT;
			    if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
				retVal = SDL_ERREXIT;
			}
		    }
		    if ((retVal == SDL_NORMAL) && fprintf(fp, ";\n") < 0)
		    {
			retVal = SDL_ABORT;
			if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
		    }
		}
	    }
	    else
	    {
		retVal = SDL_INVAGGRNAM;
		if (sdl_set_message(
				msgVec,
				1,
				retVal) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	    break;

	case LangItem:
	    retVal = sdl_c_item(fp, my.item, context);
	    break;

	case LangComment:
	    if (context->commentsOff == false)
		retVal = sdl_c_comment(
				fp,
				my.comment->comment,
				my.comment->lineComment,
				my.comment->startComment,
				my.comment->middleComment,
				my.comment->endComment);
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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ABORT:		An error occurred.
 *  SDL_ERREXIT:	Error exit.
 */
uint32_t sdl_c_entry(FILE *fp, SDL_ENTRY *entry, SDL_CONTEXT *context)
{
    SDL_PARAMETER	*param = (SDL_PARAMETER *) entry->parameters.flink;
    char		*type;
    char		outBuf[256];
    size_t		outLen = 0;
    uint32_t		retVal = SDL_NORMAL;
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
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}
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
	    outLen += sprintf(&outBuf[outLen], "unsigned ");
	outLen += sprintf(&outBuf[outLen], "%s ", type);
	if ((entry->returns.type == SDL_K_TYPE_ADDR) ||
	    (entry->returns.type == SDL_K_TYPE_PTR))
	    outLen += sprintf(&outBuf[outLen], "%s", "*");
	outLen += sprintf(&outBuf[outLen], "%s(", entry->id);
	if (fprintf(fp, outBuf) < 0)
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}
	if ((freeMe == true) && (type != NULL))
	    free(type);
    }

    /*
     * OK, parameter passing can be rather complex, especially when you have
     * an address passed by reference.  So, we are going to need to break the
     * parameter output into multiple steps.
     */
    outLen = 0;
    while ((retVal == SDL_NORMAL) &&
	   (param != (SDL_PARAMETER *) &entry->parameters))
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
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	}
	else if ((firstLine == true) &&
		(param == (SDL_PARAMETER *) &entry->parameters))
	{
	    if (fprintf(fp, "\n\t%s", outBuf) < 0)	/* last parameter */
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	}
	else
	{
	    if (fprintf(fp, "\n\t%s,", outBuf) < 0)	/* other parameter */
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	    firstLine = true;
	}
    }
    if ((retVal == SDL_NORMAL) && (fprintf(fp, ");\n") < 0))
    {
	retVal = SDL_ABORT;
	if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
	    retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_INVENUMNAM	Invalid enumeration name.
 *  SDL_ERREXIT:	Error exit.
 *  SDL_ABORT:		An unexpected error occurred.
 */
uint32_t sdl_c_enumerate(FILE *fp, SDL_ENUMERATE *_enum, SDL_CONTEXT *context)
{
    SDL_ENUM_MEMBER	*myMem = (SDL_ENUM_MEMBER *) _enum->members.flink;
    char		*name = _sdl_c_generate_name(
					_enum->id,
					_enum->prefix,
					_enum->tag,
					context);
    uint32_t		retVal = SDL_NORMAL;

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
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	}
	else
	{
	    if (fprintf(fp, "enum %s\n{\n", name) < 0)
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	}

	/*
	 * Loop through each of the entries in the member list.
	 */
	while((retVal == SDL_NORMAL) &&
	      (myMem != (SDL_ENUM_MEMBER *) &_enum->members))
	{
	    if (fprintf(fp, "    %s", myMem->id) < 0)
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	    else
	    {
		if (myMem->valueSet == true)
		{
		    if (fprintf(fp, " = %ld,\n", myMem->value) < 0)
		    {
			retVal = SDL_ABORT;
			if (sdl_set_message(
					msgVec,
					2,
					retVal,
					errno) != SDL_NORMAL)
			    retVal = SDL_ERREXIT;
		    }
		}
		else if (fprintf(fp, ",\n") < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
	    }
	}

	/*
	 * Finally, we need to close of the enum definition.
	 */
	if ((retVal == SDL_NORMAL) && (fprintf(fp, "}") < 0))
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}
	if ((retVal == SDL_NORMAL) && (_enum->typeDef == true))
	{
	    if (fprintf(fp, " %s", name) < 0)
	    {
		retVal = SDL_ABORT;
		if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
		    retVal = SDL_ERREXIT;
	    }
	}
	if (retVal == SDL_NORMAL)
	    retVal = _sdl_c_output_alignment(
					fp,
					_enum->alignment,
					context);
	if ((retVal == SDL_NORMAL) && (fprintf(fp, ";\n") < 0))
	{
	    retVal = SDL_ABORT;
	    if (sdl_set_message(
			msgVec,
			2,
			retVal,
			errno) != SDL_NORMAL)
		retVal = SDL_ERREXIT;
	}
	free(name);
    }
    else
    {
	retVal = SDL_INVENUMNAM;
	if (sdl_set_message(
			msgVec,
			1,
			retVal,
			_enum->srcLineNo) != SDL_NORMAL)
	    retVal = SDL_ERREXIT;
    }

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
 *  SDL_NORMAL:		Normal Successful Completion.
 *  SDL_ERREXIT:	Error exit.
 *  SDL_ABORT:		An unexpected error occurred.
 */
static uint32_t _sdl_c_output_alignment(
		FILE *fp,
		int alignment,
		SDL_CONTEXT *context)
{
    uint32_t	retVal = SDL_NORMAL;

    if (context->memberAlign == true)
    {
	switch(alignment)
	{
	    case SDL_K_NOALIGN:
		if (fprintf(fp, " __attribute__ ((__packed__))") < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
		break;

	    case SDL_K_ALIGN:
		if (fprintf(fp, " __attribute__ ((aligned))") < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
		break;

	    default:
		if (fprintf(
			fp,
			" __attribute__ ((aligned (%d)))",
			alignment) < 0)
		{
		    retVal = SDL_ABORT;
		    if (sdl_set_message(
				msgVec,
				2,
				retVal,
				errno) != SDL_NORMAL)
			retVal = SDL_ERREXIT;
		}
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
static char *_sdl_c_generate_name(
		char *name,
		char *prefix,
		char *tag,
		SDL_CONTEXT *context)
{
    char	*retVal = NULL;
    size_t	len = 1;
    size_t	tagLen = 0;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_c_generate_name\n", __FILE__, __LINE__);

    /*
     * First, if we have a prefix, let's get it's length.  Also, the presents
     * of the prefix indicates that we'll also add the tag.  If the prefix is
     * not present, then the tag will not be added either.
     */
    if (prefix != NULL)
    {
	if (context->suppressPrefix == false)
	    len += strlen(prefix);

	/*
	 * Next look at the tag.  If is it null or zero length, then it is not
	 * concatenated to the current string.  Otherwise, it is (with an
	 * underscore between the prefix/tag and the name.
	 */
	if ((tag != NULL) && (context->suppressTag == false))
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
	if (context->suppressPrefix == false)
	{
	    strcpy(&retVal[len], prefix);
	    len += strlen(prefix);
	}
	if ((tag != NULL) && (tagLen > 0) && (context->suppressTag == false))
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
    int		sign = (_unsigned ? 1 : 0);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_c_typeidStr\n", __FILE__, __LINE__);

    *freeMe = false;
    if (typeID == SDL_K_TYPE_NONE)
	retVal = _types[SDL_K_TYPE_VOID][bits][sign];
    else if ((typeID >= SDL_K_BASE_TYPE_MIN) && (typeID <= SDL_K_BASE_TYPE_MAX))
    {
	if ((typeID == SDL_K_TYPE_ADDR) || (typeID == SDL_K_TYPE_PTR))
	    retVal = _sdl_c_typeidStr(subType, 0, _unsigned, context, freeMe);
	else
	    retVal = _types[typeID][bits][sign];
    }
    else if ((typeID >= SDL_K_DECLARE_MIN) && (typeID <= SDL_K_DECLARE_MAX))
    {
	SDL_DECLARE *myDeclare = sdl_get_declare(&context->declares, typeID);

	if (myDeclare != NULL)
	    retVal = _sdl_c_generate_name(
				myDeclare->id,
				myDeclare->prefix,
				myDeclare->tag,
				context);
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
	printf("%s:%d:_sdl_c_leading_spaces(%d)\n", __FILE__, __LINE__, depth);

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
