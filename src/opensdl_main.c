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
 *  This source file contains the lexical definitions needed to support the
 *  parsing of an Open Structure Definition Language (OpenSDL) formatted
 *  definition file.
 *
 * USAGE:
 *	$ ./opensdl <input_SDL_file>
 *		-align:<value>	The assumed alignment.  An integer greater than
 *				zero. (No alignment is the default)
 *		-32|64		The number of bits that represent a longword.
 *				(64 is the default)
 *		-[no]check	Diagnostic messages are generated for items
 *				the do not fall on their natural alignment.
 *				(nocheck is the default)
 *		-[no]comments	Controls whether output comments are
 *				included in the output file(s). (Comments is
 *				the default)
 *		-[no]copy	Controls whether the copyright header is
 *				included in the output file (see copyright.sdl
 *				for what is included). (nocopy is the default)
 *		-[no]header	Controls whether a header containing
 *				the date and the source filename is included at
 *				the beginning of the output file(s). (header is
 *				the default)
 *		-help		display the usage information.
 *		-lang:<lang[=filespec]>	Specifies one of the language options.
 *				At least one needs to be specified on the
 *				command line.
 *		-[no]list	This has not yet been implemented. (nolist is
 *				the default)
 *		-[no]member	Indicates that every item in an
 *				aggregate should be aligned. (nomember is the
 *				default)
 *		-[no]module	This has not yet been implemented.
 *				(module is the default)
 *		-[no]parse	This has not yet been implemented. (parse is
 *				the default)
 *		-[no]supress:prefix|tag
 *				Suppress outputting symbols with a prefix, tag,
 *				or both. (nosupress is the default).
 *		-symbol:<symbol=value>	Used in conditional compilation where
 *				IFSYMBOL is specified in the input file.  A
 *				value of zero turns off the symbol and a
 *				non-zero value turns it on.
 *
 * Revision History:
 *
 *  V01.000	23-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	06-SEP-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "opensdl_defs.h"
#include "opensdl_parser.h"
#include "opensdl_lexical.h"
#include "opensdl_actions.h"
#include "opensdl_lang.h"
#include "opensdl_message.h"

/*
 * Defines and includes for enable extend trace and logging
 */
extern int		yydebug;
extern SDL_CONTEXT	context;
extern SDL_QUEUE	literal;

void			*scanner = NULL;
_Bool			trace = false;

#define SDL_K_STARS	0
#define SDL_K_CREATED	1
#define SDL_K_FILEINFO	2

static SDL_LANG_FUNC _outputFuncs[SDL_K_LANG_MAX] =
{

    /*
     * For the C/C++ languages.
     */
    {
	(SDL_FUNC) &sdl_c_commentStars,
	(SDL_FUNC) &sdl_c_createdByInfo,
	(SDL_FUNC) &sdl_c_fileInfo,
	(SDL_FUNC) NULL,
	(SDL_FUNC) NULL,
	(SDL_FUNC) NULL,
	(SDL_FUNC) NULL,
	(SDL_FUNC) NULL
    }
};

static char *_extensions[] =
{
    "h",						/* C */
    NULL
};

char *sdl_months[] =
{
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

/*
 * Define the message vector to be used to report error messages throughout
 * the application.  Each entry in the message vector contains a 32-bit message
 * code, followed by a 16-bit Formatted ASCII Output (FAO) count, and a 16-bit
 * FAO information field.  The FAO information field contains an 8-bit type and
 * and 8-bit length field (256 character long strings should be long enough).
 * So, each message vector entry is 64-bits long.
 */
#define SDL_MSG_VEC_LEN	1024
SDL_MSG_VECTOR	msgVec[SDL_MSG_VEC_LEN];
static char	*errFmt = "\n%s";

void yyerror(YYLTYPE *locp, yyscan_t *scanner, char const *msg)
{
    fprintf(stderr,
	"[%d:%d] -> [%d:%d], %s",
	locp->first_line,
	locp->first_column,
	locp->last_line,
	locp->last_column,
	msg);
    return;
}

/*
 * usage
 *  This function is called to display the usage information to the user and
 *  returns back to the caller.
 */
static void _sdl_usage(void)
{
    printf("Usage: opensdl [options] file...\nOptions:\n");
    printf("  -align:<value>\t    The assumed alignment.  An integer greater than\n");
    printf("\t\t\t    zero. (No alignment is the default)\n");
    printf("  -32|64\t\t    The number of bits that represent a longword.\n");
    printf("\t\t\t    (64 is the default)\n");
    printf("  -[no]check\t\t    Diagnostic messages are generated for items\n");
    printf("\t\t\t    the do not fall on their natural alignment.\n");
    printf("\t\t\t    (nocheck is the default)\n");
    printf("  -[no]comments\t\t    Controls whether output comments are\n");
    printf("\t\t\t    included in the output file(s). (Comments is\n");
    printf("\t\t\t    the default)\n");
    printf("  -[no]copy\t\t    Controls whether the copyright header is\n");
    printf("\t\t\t    included in the output file (see copyright.sdl\n");
    printf("\t\t\t    for what is included). (nocopy is the default)\n");
    printf("  -[no]header\t\t    Controls whether a header containing\n");
    printf("\t\t\t    the date and the source filename is included at\n");
    printf("\t\t\t    the beginning of the output file(s). (header is\n");
    printf("\t\t\t    the default)\n");
    printf("  -help\t\t\t    Displays this usage information.\n");
    printf("  -lang:<lang[=filespec]>   Specifies one of the language options.\n");
    printf("\t\t\t    At least one needs to be specified on the\n");
    printf("\t\t\t    command line.\n");
    printf("  -[no]list\t\t    This has not yet been implemented. (nolist is\n");
    printf("\t\t\t    the default)\n");
    printf("  -[no]member\t\t    Indicates that every item in an\n");
    printf("\t\t\t    aggregate should be aligned. (nomember is the\n");
    printf("\t\t\t    default)\n");
    printf("  -[no]module\t\t    This has not yet been implemented.\n");
    printf("\t\t\t    (module is the default)\n");
    printf("  -[no]parse\t\t    This has not yet been implemented. (parse is\n");
    printf("\t\t\t    the default)\n");
    printf("  -[no]supress:prefix|tag   Suppress outputting symbols with a prefix, tag,\n");
    printf("\t\t\t    or both. (nosupress is the default).\n");
    printf("  -symbol:<symbol=value>    Used in conditional compilation where\n");
    printf("\t\t\t    IFSYMBOL is specified in the input file.  A\n");
    printf("\t\t\t    value of zero turns off the symbol and a\n");
    printf("\t\t\t    non-zero value turns it on.\n");

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * _sdl_parse_args
 *  This function is called to parse the command line arguments and update the
 *  context variable accordingly.
 *
 * Input Parameters:
 *  argc:
 *	A value indicating the number of arguments specified in the 'argv'
 *	parameter.
 *  argv:
 *	A pointer to an array of strings containing the arguments provided on
 *	the command line.  The first argument is always the executable
 *	filename.
 *
 * Output Parameters:
 *  context:
 *	A pointer to the context variable to be initialized from the command
 *	line arguments.
 *
 * Return Values:
 *  SDL_NORMAL:		Normal successful completion.
 *  !SDL_NORMAL:	An error occurred.
 */
static uint32_t _sdl_parse_args(int argc, char *argv[], SDL_CONTEXT *context)
{
    char	*ptr;
    int		ii = 1;
    uint32_t	retVal = SDL_NORMAL;

    /*
     * Initialize all the defaults.
     */
    context->inputFile = NULL;
    context->symbCondList.symbols = NULL;
    context->symbCondList.listSize = 0;
    context->symbCondList.listUsed = 0;
    context->alignment = 0;		/* default to no align */
    context->wordSize = 64;		/* default to 64-bits */
    context->checkAlignment = false;	/* default to no-check */
    context->commentsOff = false;	/* default to comments on */
    context->copyright = false;		/* default to no-copyright */
    context->header = true;		/* default to write out header */
    context->memberAlign = true;	/* default to member alignment */
    context->suppressPrefix = false;	/* default to displaying prefix */
    context->suppressTag = false;	/* default to displaying tag */

    /*
     * Loop through each of the arguments extracting the relevant information
     * and storing it into the context block.
     */
    while ((ii < argc) && (retVal == SDL_NORMAL))
    {
	if (argv[ii][0] == '-')
	{
	    switch (argv[ii][1])
	    {
		/*
		 * 32-bit length for longwords.
		 */
		case '3':
		    if ((argv[ii][2] == '2') && (argv[ii][3] == '\0'))
			context->wordSize = 32;
		    else
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * 64-bit length for longwords.
		 */
		case '6':
		    if ((argv[ii][2] == '4') && (argv[ii][3] == '\0'))
			context->wordSize = 64;
		    else
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * align:<value>, value must be greater than zero.
		 */
		case 'a':
		    if (strncmp(argv[ii], "-align", 5) == 0)
		    {
			if (argv[ii][5] == ':')
			{
			    context->alignment = strtol(
						    &argv[ii][6],
						    NULL,
						    10);
			    if (context->alignment < 0)
				retVal = SDL_INVALIGN;
			}
			else
			    retVal = SDL_INVALIGN;
			if (retVal == SDL_INVALIGN)
			{
			    if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
				retVal = SS_FATAL;
			}
		    }
		    else
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * check for member alignment on natural boundaries.
		 * comments, include comments in output file(s).
		 * copy, include copyright comment at start of output file(s).
		 */
		case 'c':
		    if (strcmp(argv[ii], "-check") == 0)
			context->checkAlignment = true;
		    else if (strcmp(argv[ii], "-comments") == 0)
			context->commentsOff = false;
		    else if (strcmp(argv[ii], "-copy") == 0)
		    {
			char	*path = realpath(argv[0], NULL);
			int	jj;

			if (path != NULL)
			{
			    context->copyright = true;
			    jj = strlen(path) - 1;
			    while ((jj > 0) && (path[jj - 1] != '/'))
				jj--;
			    context->copyrightFile = calloc(1, jj + 14);
			    strncpy(context->copyrightFile, path, jj);
			    strcpy(
				&context->copyrightFile[jj],
				"copyright.sdl");
			    free(path);
			}
			else
			    retVal = SS_FATAL;
		    }
		    else
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * header, output OpenSDL header information.
		 * help, display usage information.
		 */
		case 'h':
		    if (strcmp(argv[ii], "-header") == 0)
			context->header = true;
		    else if (strcmp(argv[ii], "-help") == 0)
			_sdl_usage();
		    else
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * lang:<lang>[=<filespec>], specify output language and file.
		 * list[:<filespec>], generate a listing file.
		 */
		case 'l':
		    if (strncmp(argv[ii], "-lang", 4) == 0)
		    {
			if (argv[ii][5] == ':')
			{
			    int		jj = 0;
			    bool	langSet = false;

			    while ((context->languages[jj].langStr != NULL) &&
				   (context->languages[jj].langVal != -1) &&
				   (retVal == SDL_NORMAL))
			    {
				ptr = strchr(argv[ii], '=');
				if (ptr == NULL)
				    ptr = &argv[ii][strlen(argv[ii])];
				if (strncasecmp(
					&argv[ii][6],
					context->languages[jj].langStr,
					(ptr - &argv[ii][6])) == 0)
				{
				    int	lang = context->languages[jj].langVal;

				    if (context->langSpec[lang] == false)
				    {
					context->langSpec[lang] = true;
					if (*ptr == '=')
					    context->outFileName[lang] =
						strdup(&ptr[1]);
					langSet = true;
				    }
				    else
				    {
					char	lang[32];

					retVal = SDL_DUPLANG;
					strncpy(lang,
						&argv[ii][6],
						(ptr - &argv[ii][6]));
					if (sdl_set_message(
							msgVec,
							1,
							retVal,
							lang) != SS_NORMAL)
					    retVal = SS_FATAL;
				    }
				}
				jj++;
			    }
			    if ((langSet == false) && (retVal == SDL_NORMAL))
			    {
				retVal = SS_INVQUAL;
				if (sdl_set_message(
						msgVec,
						1,
						retVal,
						argv[ii]) != SS_NORMAL)
				    retVal = SS_FATAL;
			    }
			}
		    }
		    else if (strncmp(argv[ii], "-list", 4) == 0)
		    {	/* ignore for now */	}
		    else
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * member, force member alignment
		 * module, ?
		 */
		case 'm':
		    if (strncasecmp(argv[ii], "-member", 6) == 0)
			context->memberAlign = true;
		    else if (strncasecmp(argv[ii], "-module", 7) != 0)
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * nocheck, don't check member alignment.
		 * nocomments, don't include comments.
		 * nocopy, don't include copyright information.
		 * noheader, don't include OpenSDL header.
		 * nolist, don't generate a listing file.
		 * nomember, don't force member alignment.
		 * nomodule, ?
		 * noparse, don't generate an intermediate file.
		 * nosuppress, don't suppress prefix or tag.
		 */
		case 'n':
		    if (strcmp(argv[ii], "-nocheck") == 0)
			context->checkAlignment = false;
		    else if (strcmp(argv[ii], "-nocomments") == 0)
			context->commentsOff = true;
		    else if (strcmp(argv[ii], "-nocopy") == 0)
			context->copyright = false;
		    else if (strcmp(argv[ii], "-noheader") == 0)
			context->header = false;
		    else if (strcmp(argv[ii], "-nomember") == 0)
			context->memberAlign = false;
		    else if (strcmp(argv[ii], "-nosuppress") == 0)
		    {
			context->suppressPrefix = false;
			context->suppressTag = false;
		    }
		    else if ((strcmp(argv[ii], "-nolist") != 0) &&
			     (strcmp(argv[ii], "-nomodule") != 0) &&
		    	     (strcmp(argv[ii], "-noparse") != 0))
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		/*
		 * parse, generate an intermediate file.
		 */
		case 'p':
		    /* Ignore */
		    break;

		/*
		 * suppress:[prefix][,tag], suppress usage or prefix and/or tag
		 * symbol:<symbol>=<value>, symbol used in conditionals
		 */
		case 's':
		    if (strncmp(argv[ii], "-suppress", 8) == 0)
		    {
			ptr = strchr(argv[ii], ':');
			if (ptr != NULL)
			{
			    ptr++;
			    if (strncasecmp(ptr, "prefix", 6) == 0)
				context->suppressPrefix = true;
			    else if (strncasecmp(ptr, "tag", 3) == 0)
				context->suppressTag= true;
			    else
				retVal = SS_INVQUAL;
			    if (retVal == SDL_NORMAL)
			    {
				ptr = strchr(argv[ii], ',');
				if (ptr != NULL)
				{
				    ptr++;
				    if (strncasecmp(ptr, "prefix", 6) == 0)
					context->suppressPrefix = true;
				    else if (strncasecmp(ptr, "tag", 3) == 0)
					context->suppressTag= true;
				    else
					retVal = SS_INVQUAL;
				}
			    }
			}
			else
			    retVal = SS_INVQUAL;
		    }
		    else if (strncmp(argv[ii], "-symbol", 6) == 0)
		    {
			SDL_SYMBOL_LIST	*list = &context->symbCondList;

			ptr = strchr(argv[ii], ':');
			if (ptr != NULL)
			{
			    char	*ptr2 = strchr(argv[ii], '=');
			    char	*symbol;
			    int		value;
			    bool	noValue = false;

			    ptr++;
			    if (ptr2 == NULL)
			    {
				ptr2 = &argv[ii][strlen(argv[ii])];
				noValue = true;
			    }
			    symbol = strndup(ptr, (ptr2 - ptr));
			    if (noValue == false)
			    {
				ptr2++;
				value = strtol(ptr2, NULL, 10);
			    }
			    if (symbol != NULL)
			    {
				int	jj;
				bool	found = false;

				for (jj = 0;
				     ((jj < list->listUsed) &&
				      (found == false));
				     jj++)
				{
				    if (strcmp(
					    list->symbols[jj].symbol,
					    symbol) == 0)
					found = true;
				}
				if (found == false)
				{
				    if (list->listUsed >= list->listSize)
				    {
					list->listSize++;
					list->symbols = realloc(
						    list->symbols,
						    (sizeof(SDL_SYMBOL) *
						     list->listSize));
				    }
				    list->symbols[list->listUsed].symbol =
					symbol;
				    list->symbols[list->listUsed++].value =
					value;
				}
				else
				    retVal = SDL_SYMALRDEF;
			    }
			    else
				retVal = SDL_ABORT;
			}
			else
			    retVal = SS_INVQUAL;
		    }
		    else
			retVal = SS_INVQUAL;
		    if (retVal == SS_INVQUAL)
		    {
			retVal = SS_INVQUAL;
			if (sdl_set_message(
					msgVec,
					1,
					retVal,
					argv[ii]) != SS_NORMAL)
			    retVal = SS_FATAL;
		    }
		    break;

		default:
		    break;
	    }
	}
	else if (context->inputFile == NULL)
	    context->inputFile = strdup(argv[ii++]);
	else
	{
	    retVal = SS_INVQUAL;
	    if (sdl_set_message(
			    msgVec,
			    1,
			    retVal,
				argv[ii]) != SS_NORMAL)
		    retVal = SS_FATAL;
	}
	ii++;
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * main
 *  This is the main function called by the image activator.  It initializes
 *  the context, other data, and parses the command line arguments into the
 *  context.
 *
 * Input Parameters:
 *  argc:
 *	A value indicating the number of arguments specified in argv.
 *  argv:
 *	A pointer to an array or strings containing the command line arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  0: for success.
 *  1: for failure.
 */
int main(int argc, char *argv[])
{
    FILE	*cfp = NULL;
    FILE	*fp;
    char	*msgTxt = NULL;
    struct tm	*timeInfo;
    time_t	localTime;
    uint32_t	status;
    int		ii, jj;

    /*
     * Turn on tracing
     */
    trace = true;
    yydebug = 1;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:main\n", __FILE__, __LINE__);
    memset(&context, 0, sizeof(SDL_CONTEXT));
    SDL_Q_INIT(&literal);

    /*
     * Get the current time as the start time.
     */
    localTime = time(NULL);
    timeInfo = localtime(&localTime);

    /*
     * Initialize the parsing context.
     */
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {
	context.langSpec[ii] = false;
	context.langEna[ii] = true;
	context.outFileName[ii] = NULL;
    }
    context.processingEnabled = true;

    /*
     * Initialize the dimension array and the options index.
     */
    for (ii = 0; ii < SDL_K_MAX_DIMENSIONS; ii++)
	context.dimensions[ii].inUse = false;

    /*
     * Set the options, parameters and languages workspace.
     */
    context.options = NULL;
    context.optionsSize = 0;
    context.optionsIdx = 0;
    context.parameters = NULL;
    context.parameterSize = 0;
    context.parameterIdx = 0;
    context.langCondList.lang = NULL;
    context.langCondList.listSize = 0;
    context.langCondList.listUsed = 0;

    context.languages[0].langStr = "cc";
    context.languages[0].langVal = SDL_K_LANG_C;
    context.languages[1].langStr = NULL;
    context.languages[1].langVal = -1;

    /*
     * Initialize the parsing states.
     */
    context.state = Initial;
    context.condState.state = calloc(
				SDL_K_COND_STATE_SIZE,
				sizeof(SDL_COND_STATES));
    context.condState.state[0] = CondNone;
    context.condState.top = 0;
    context.condState.bottom = SDL_K_COND_STATE_SIZE;

    /*
     * Initialize the context queues.
     */
    SDL_Q_INIT(&context.locals);
    SDL_Q_INIT(&context.constants);
    SDL_Q_INIT(&context.declares.header);
    context.declares.nextID = SDL_K_DECLARE_MIN;
    SDL_Q_INIT(&context.items.header);
    context.items.nextID = SDL_K_ITEM_MIN;
    SDL_Q_INIT(&context.aggregates.header);
    context.aggregates.nextID = SDL_K_AGGREGATE_MIN;
    SDL_Q_INIT(&context.enums.header);
    context.enums.nextID = SDL_K_ENUM_MIN;
    SDL_Q_INIT(&context.entries);

    /*
     * Parse out the command line arguments.
     */
    status = _sdl_parse_args(argc, argv, &context);
    if ((status != SDL_NORMAL) && (status != SS_FATAL))
    {
	status = sdl_get_message(msgVec, &msgTxt);
	if (status == SS_NORMAL)
	    fprintf(stderr, errFmt, msgTxt);
	free(msgTxt);
	return(-1);
    }

    if (context.inputFile == NULL)
    {
	status = sdl_set_message(
			msgVec,
			1,
			SDL_NOINPFIL);
	if (status == SS_NORMAL)
	    status = sdl_get_message(msgVec, &msgTxt);
	if (status == SS_NORMAL)
	    fprintf(stderr, errFmt, msgTxt);
	free(msgTxt);
	return(-1);
    }

    /*
     * Open the input file or reading.
     */
    if ((fp = fopen(context.inputFile, "r")) == NULL)
    {
	status = sdl_set_message(
			msgVec,
			2,
			SDL_INFILOPN,
			context.inputFile,
			RMS_FNF);
	if (status == SS_NORMAL)
	    status = sdl_get_message(msgVec, &msgTxt);
	if (status == SS_NORMAL)
	    fprintf(stderr, errFmt, msgTxt);
	free(msgTxt);
	return(-1);
    }

    /*
     * If the user indicated that they wanted the copyright information at the
     * start of the file, then open it for read.
     */
    if (context.copyright == true)
    {
	if (context.copyrightFile == NULL)
	{
	    status = sdl_set_message(
				msgVec,
				1,
				SDL_NOCOPYFIL);
	    if (status == SS_NORMAL)
		status = sdl_get_message(msgVec, &msgTxt);
	    if (status == SS_NORMAL)
		fprintf(stderr, errFmt, msgTxt);
	}
	else if ((cfp = fopen(context.copyrightFile, "r")) == NULL)
	{
	    status = sdl_set_message(
				msgVec,
				2,
				SDL_INFILOPN,
				context.copyrightFile,
				RMS_FNF);
	    if (status == SS_NORMAL)
		status = sdl_get_message(msgVec, &msgTxt);
	    if (status == SS_NORMAL)
		fprintf(stderr, errFmt, msgTxt);
	}
	if (msgTxt != NULL)
	{
	    free(msgTxt);
	    return(-1);
	}
    }

    /*
     * Loop through each of the supported languages.
     */
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {

	/*
	 * If we are going to generate an output file for the language, then we
	 * have a number of things to do.
	 */
	if (context.langSpec[ii] == true)
	{

	    /*
	     * First we need to copy the filename specified on the command
	     * line.  NOTE: We may need to truncate the name to fit the field
	     * we have for it (32 characters lone).
	     */
	    context.outFileName[ii] = strdup(context.inputFile);

	    /*
	     * Go find the last '.' in the file name, this will be where the
	     * file extension starts.
	     */
	    for (jj = strlen(context.outFileName[ii]); jj >= 0; jj--)
	    {
		if (context.outFileName[ii][jj] == '.')
		{
		    jj++;
		    break;
		}
	    }

	    /*
	     * If we ended up at the beginning of the file, then there was no
	     * file extension specified.  Use the whole filename.
	     */
	    if (jj <= 0)
		jj = strlen(context.outFileName[ii]);

	    /*
	     * Copy the extension for this language after the last '.' (or the
	     * one just added).
	     */
	    strcpy(&context.outFileName[ii][jj], _extensions[ii]);

	    /*
	     * Try and open the file for this language.  If it fails, we are
	     * done.
	     */
	    if ((context.outFP[ii] = fopen(context.outFileName[ii], "w")) == NULL)
	    {
		status = sdl_set_message(
				msgVec,
				2,
				SDL_OUTFILOPN,
				context.outFileName[ii],
				RMS_FNF);
		if (status == SS_NORMAL)
		    status = sdl_get_message(msgVec, &msgTxt);
		if (status == SS_NORMAL)
		    fprintf(stderr, errFmt, msgTxt);
		free(msgTxt);
		return(-1);
	    }
	    else if (context.header == true)
	    {

		/*
		 * OK, we successfully opened this file.  Insert the header
		 * comments.  First starting with a row of '*'s.
		 */
		if ((*_outputFuncs[ii][SDL_K_STARS])(
				context.outFP[ii]) == 1)
		{

		    /*
		     * If we put the row of '*'s in, now put in information
		     * about OpenSDL.
		     */
		    if ((*_outputFuncs[ii][SDL_K_CREATED])(
				context.outFP[ii],
				timeInfo) == 1)
		    {
			char		*path = realpath(context.inputFile, NULL);
			struct stat	fileStats;
			_Bool		freePath = false;

			if (path == NULL)
			    path = context.inputFile;
			else
			    freePath = true;
			if (stat(path, &fileStats) != 0)
			{
			    timeInfo->tm_year = -42;
			    timeInfo->tm_mon = 10;
			    timeInfo->tm_mday = 17;
			    timeInfo->tm_hour = 0;
			    timeInfo->tm_min = 0;
			    timeInfo->tm_sec = 0;
			}
			else
			    timeInfo = localtime(&fileStats.st_mtime);

			/*
			 * OK, if we put in the row of information about
			 * OpenSDL, let's put in information about the file we
			 * are about to parse.
			 */
			if ((*_outputFuncs[ii][SDL_K_FILEINFO])(
					context.outFP[ii],
					timeInfo,
					path) == 1)
			{
			    if (freePath == true)
				free(path);

			    /*
			     * Finally, if we get here, we just need to output
			     * another row of '*'s, and then can begin our
			     * actual input file parsing and output file
			     * generation.
			     */
			    if ((_outputFuncs[ii][SDL_K_STARS])(
					context.outFP[ii]) == 0)
				return(-1);
			}
			else
			{
			    if (freePath == true)
				free(path);
			    return(-1);
			}
		    }
		    else
			return(-1);
		}
		else
		    return(-1);
	    }
	}
	else
	    context.outFP[ii] = NULL;
    }
    SDL_Q_INIT(&context.locals);
    context.module = NULL;
    context.ident = NULL;

    /*
     * If the copyright needs to be put into the file, then do so now.
     */
    if (cfp != NULL)
    {
	yylex_init(&scanner);
	yyset_debug(1, scanner);
	yydebug = 0;
	yyset_in(cfp, scanner);
	yyparse(scanner);
	yylex_destroy(scanner);
    }

    /*
     * Start parsing the real input file
     */
    yylex_init(&scanner);
    yyset_debug(1, scanner);
    yydebug = 0;

    /*
     * Associate the input file to the parser.
     */
    yyset_in(fp, scanner);

    /*
     * Do the parsing.
     */
    yyparse(scanner);

    /*
     * All done parsing, do the clean-up.
     */
    yylex_destroy(scanner);

    /*
     * Go close all the files.
     */
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {
	if ((context.langSpec[ii] == true) && (context.outFP[ii] != NULL))
	    fclose(context.outFP[ii]);
	if (context.outFileName[ii] != NULL)
	    free(context.outFileName[ii]);
    }

    /*
     * Return the results back to the caller.
     */
    fprintf(stderr, "'%s' has been processed", context.inputFile);

    /*
     * Clean-up memory.
     */
    for (ii =0; ii < context.langCondList.listUsed; ii++)
	free(context.langCondList.lang);
    for (ii = 0; ii < context.symbCondList.listUsed; ii++)
	free(context.symbCondList.symbols->symbol);
    if (context.symbCondList.symbols != NULL)
	free(context.symbCondList.symbols);
    free(context.inputFile);

    /*
     * Return back to the caller.
     */
    return(0);
}
