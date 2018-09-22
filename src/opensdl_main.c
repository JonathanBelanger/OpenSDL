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
 *
 * Revision History:
 *
 *  V01.000	Aug 23, 2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	Sept  6, 2018	Jonathan D. Belanger
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
    {(SDL_FUNC) &sdl_c_commentStars, &sdl_c_createdByInfo, &sdl_c_fileInfo, NULL}
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

int main(int argc, char *argv[])
{
    FILE	*fp;
    int		ii, jj;
    time_t	localTime;
    struct tm	*timeInfo;

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

    if (argc < 2)
    {
	fprintf(stderr, "No input specified");
	return(-1);
    }

    if ((fp = fopen(argv[1], "r")) == NULL)
    {
	fprintf(
	    stderr,
	    "Cannot open input file '%s', errno = %d",
	    argv[1],
	    errno);
	return(-1);
    }

    /*
     * Initialize the parsing context.
     */
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {
	context.langSpec[ii] = false;
	context.langEna[ii] = true;
    }

    /*
     * Initialize the aggregate stack.
     */
    for (ii = 0; ii < SDL_K_SUBAGG_MAX; ii++)
	context.aggStack[ii] = NULL;
    context.aggStackPtr = SDL_K_SUBAGG_MAX;
    context.langSpec[SDL_K_LANG_C] = true;

    /*
     * Initialize the dimension array.
     */
    for (ii = 0; ii < SDL_K_MAX_DIMENSIONS; ii++)
	context.dimensions[ii].inUse = false;

    /*
     * Initialize the constant stack.
     */
    context.constEntries = 0;
    context.constStack = NULL;

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
    SDL_Q_INIT(&context.entries.header);

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
	    strncpy(context.outFileName[ii], argv[1], SDL_K_NAME_MAX_LEN);

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
	     * If adding the extension will exceed the buffer length, then
	     * truncate to accommodate.
	     */
	    if ((jj + strlen(_extensions[ii]) + 1) > SDL_K_NAME_MAX_LEN)
	    {
		jj = SDL_K_NAME_MAX_LEN - strlen(_extensions[ii]) - 2;
		context.outFileName[ii][jj++] = '.';
	    }

	    /*
	     * Copy the extension for this language after the last '.' (or the
	     * one just added).
	     */
	    strcpy(&context.outFileName[ii][jj], _extensions[ii]);

	    /*
	     * Try and open the file for this language.  If ti fails, we are
	     * done.
	     */
	    if ((context.outFP[ii] = fopen(context.outFileName[ii], "w")) == NULL)
	    {
		fprintf(
		    stderr,
		    "Cannot open output file '%s', errno = %d",
		    context.outFileName[ii],
		    errno);
		return(-1);
	    }
	    else
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
			char		*path = realpath(argv[1], NULL);
			struct stat	fileStats;
			_Bool		freePath = false;

			if (path == NULL)
			    path = argv[1];
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
    context.module[0] = '\0';
    context.ident[0] = '\0';

    /*
     * Start parsing ...
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
	if ((context.langSpec[ii] == true) && (context.outFP[ii] != NULL))
	    fclose(context.outFP[ii]);

    /*
     * Return the results back to the caller.
     */
    fprintf(stderr, "'%s' has been processed", argv[1]);
    return(0);
}
