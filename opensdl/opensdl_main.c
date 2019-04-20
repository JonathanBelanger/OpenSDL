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
 *		-a, --align:<value>
 *				The assumed alignment.  A value that is a power
 *				two and between 0 and 8.  A value of 0 is no
 *				alignment (the default).
 *		-b32|b64 The number of bits that represent a longword.
 *				(64 is the default)
 *		-k, --[no]check	Diagnostic messages are generated for items
 *				the do not fall on their natural alignment.
 *				(nocheck is the default)
 *		-c, --[no]comments
 *				Controls whether output comments are included
 *				in the output file(s). (Comments is the
 *				default)
 *		-C, --[no]copy	Controls whether the copyright header is
 *				included in the output file (see copyright.sdl
 *				for what is included). (nocopy is the default)
 *		-H, --[no]header
 *				Controls whether a header containing the date
 *				and the source filename is included at the
 *				beginning of the output file(s). (header is the
 *				default)
 *		-?, --help	Display the usage information.
 *		-l, --lang:<lang[=filespec]>
 *				Specifies one of the language options.  At
 *				least one needs to be specified on the command
 *				line.
 *		-L, --[no]list	This will cause a listing file to either be or
 *				not be generated.  No list is the default.
 *		-m, --[no]member
 *				Indicates that every item in an aggregate
 *				should be aligned. (nomember is the default)
 *		-M, --[no]module
 *				This has not yet been implemented. (module is
 *				the default)
 *		-p, --[no]parse	This has not yet been implemented. (parse is
 *				the default)
 *		-S, --[no]suppress[:prefix|tag]
 *				Suppress outputting symbols with a prefix, tag,
 *				or both. (nosupress is the default).
 *		-s, --symbol:<symbol=value>
 *				Used in conditional compilation where IFSYMBOL
 *				is specified in the input file.  A value of
 *				zero turns off the symbol and a non-zero value
 *				turns it on.
 *		-t, --trace	Trace memory allocations and deallocations.
 *		-v, --verbose	Verbose information during processing.  By
 *				default this is turned off.
 *		    --version	Display the version information for the OpenSDL
 *				utility.  By default the version information is
 *				not displayed.
 *
 * Revision History:
 *
 *  V01.000	23-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	06-SEP-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 *
 *  V01.002 30-MAR-2019 Jonathan D. Belanger
 *  Updated to use argp instead of a custom-rolled argument processor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <argp.h>
#include "opensdl_defs.h"
#include "library/language/opensdl_lang.h"
#include "library/utility/opensdl_plugin_funcs.h"
#include "library/common/opensdl_blocks.h"
#include "library/common/opensdl_message.h"
#include "library/parser/opensdl_parser.h"
#include "library/parser/opensdl_lexical.h"

/*
 * Function prototypes
 */
static error_t _sdl_parse_opt(int, char *, struct argp_state *);

/*
 * Defines and includes for enable extend trace and logging
 */
extern SDL_CONTEXT context;
extern SDL_QUEUE literal;

void *scanner = NULL;
bool trace;
bool listing;
FILE *listingFP = NULL;
int _verbose;

#define SDL_K_STARS	0
#define SDL_K_CREATED	1
#define SDL_K_FILEINFO	2

/*
 * argp declarations
 */
#define SDL_K_ARG_NOCHECK        1
#define SDL_K_ARG_NOCOMMENT      2
#define SDL_K_ARG_B32            3
#define SDL_K_ARG_NOCOPY         4
#define SDL_K_ARG_NOHEADER       5
#define SDL_K_ARG_B64            6
#define SDL_K_ARG_NOLIST         7
#define SDL_K_ARG_NOMEMBER       8
#define SDL_K_ARG_NOMODULE       9
#define SDL_K_ARG_NOPARSE       10
#define SDL_K_ARG_NOSUPPRESS    11
const char *argp_program_version = "OpenSDL V3.4.20181114";
const char *argp_program_bug_address =
    "https://github.com/JonathanBelanger/OpenSDL/issues";
static char doc[] = "Open Structure Definition Language";
static char args_doc[] = "[FILENAME]";
static struct argp_option options[] =
{
    {
        "align",
        'a',
        "value",
        0,
        "The assumed alignment.  A value that is a power two and between 0 "
            "and 8.  A value of 0 is no alignment (the default).",
        0
    },
    {
        "b32",
        SDL_K_ARG_B32,
        0,
        0,
        "The number of bits that represent a longword.",
        0
    },
    {
        "b64",
        SDL_K_ARG_B64,
        0,
        0,
        "The number of bits that represent a longword. (default)",
        0
    },
    {
        "check",
        'k',
        0,
        0,
        "Diagnostic messages are generated for items the do not fall on their "
            "natural alignment.",
        0
    },
    {
        "nocheck",
        SDL_K_ARG_NOCHECK,
        0,
        0,
        "Diagnostic messages are not generated for items the do not fall on "
            "their natural alignment. (the default)",
        0
    },
    {
        "comments",
        'c',
        0,
        0,
        "Output comments are included in the output file(s). (the default)",
        0
    },
    {
        "nocomments",
        SDL_K_ARG_NOCOMMENT,
        0,
        0,
        "Output comments are not included in the output file(s).",
        0
    },
    {
        "copy",
        'C',
        0,
        0,
        "A copyright header is included in the output file (see "
            "/usr/lib/OpenSDL/copyright.sdl for what is included).",
        0
    },
    {
        "nocopy",
        SDL_K_ARG_NOCOPY,
        0,
        0,
        "A copyright header is not included in the output file. (the default)",
        0
    },
    {
        "header",
        'H',
        0,
        0,
        "A header containing the date and the source filename is included at "
            "the top of the output file(s). (the default)",
        0
    },
    {
        "noheader",
        SDL_K_ARG_NOHEADER,
        0,
        0,
        "A header containing the date and the source filename is not included "
            "at the top of the output file(s)",
        0
    },
    {
        "lang",
        'l',
        "language[=filespec]",
        0,
        "Specifies one of the language options.  At least one needs to be "
            "specified on the command line.",
        0
    },
    {
        "list",
        'L',
        "[=filespec]",
        0,
        "Generate a listing file.",
        0
    },
    {
        "nolist",
        SDL_K_ARG_NOLIST,
        0,
        0,
        "Generate a listing file. (the default)",
        0
    },
    {
        "member",
        'm',
        0,
        0,
        "Align each item in an aggregate",
        0
    },
    {
        "nomember",
        SDL_K_ARG_NOMEMBER,
        0,
        0,
        "Do not align each item in an aggregate. (the default)",
        0
    },
    {
        "module",
        'M',
        0,
        OPTION_HIDDEN,
        "This has not yet been implemented. (the default)",
        0
    },
    {
        "nomodule",
        SDL_K_ARG_NOMODULE,
        0,
        OPTION_HIDDEN,
        "This has not yet been implemented.",
        0
    },
    {
        "parse",
        'p',
        0,
        OPTION_HIDDEN,
        "This has not yet been implemented. (the default)",
        0
    },
    {
        "noparse",
        SDL_K_ARG_NOPARSE,
        0,
        OPTION_HIDDEN,
        "This has not yet been implemented.",
        0
    },
    {
        "suppress",
        'S',
        "prefix[,tag]",
        OPTION_ARG_OPTIONAL,
        "Suppress generating symbols with a prefix, tag, or both. Specifying "
            "--suppress with no argument, will suppress both.",
        0
    },
    {
        "nosuppress",
        SDL_K_ARG_NOSUPPRESS,
        "prefix[,tag]",
        OPTION_ARG_OPTIONAL,
        "Do not suppress generating symbols with a prefix, tag, or both. "
            "Specifying --nosuppress with no argument, will not suppress both. "
            "(the default)",
        0
    },
    {
        "symbol",
        's',
        "symbol=value",
        0,
        "Used in conditional compilation where IFSYMBOL is specified in the "
            "input file.  A value of zero turns off the symbol and a non-zero "
            "value turns it on.",
        0
    },
    {
        "trace",
        't',
        0,
        0,
        "Trace memory allocations and deallocations.",
        0
    },
    {
        "verbose",
        'v',
        0,
        0,
        "Verbose information during processing.",
        0
    },
    {0}
};
static struct argp argp =
{options, _sdl_parse_opt, args_doc, doc, 0, 0, 0};

/*
 * Define the message vector to be used to report error messages throughout
 * the application.  Each entry in the message vector contains a 32-bit message
 * code, followed by a 16-bit Formatted ASCII Output (FAO) count, and a 16-bit
 * FAO information field.  The FAO information field contains an 8-bit type and
 * and 8-bit length field (256 character long strings should be long enough).
 * So, each message vector entry is 64-bits long.
 */
#define SDL_MSG_VEC_LEN	1024
SDL_MSG_VECTOR msgVec[SDL_MSG_VEC_LEN];
static char *errFmt = "\n%s";

/*
 * yyerror
 *  This is the error handler to be used by Bison when a syntax error has been
 *  detected.
 *
 * Input Parameters:
 *  locp:
 *      A pointer to a structure containing the information about the syntax
 *      error.  Specifically, we are interested in the first line number of the
 *      input file.
 *  scanner:
 *      A pointer to the Bison scanner structure.  This parameter is ignored.
 *  msg:
 *      A pointer to a string containing what specifically caused the parse
 *      error.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void yyerror(YYLTYPE *locp, yyscan_t *scanner, char const *msg)
{
    if (sdl_set_message(msgVec,
                        2,
                        SDL_SYNTAXERR,
                        locp->first_line,
                        SDL_PARSEERR,
                        msg) == SDL_NORMAL)
    {
        char *msgText;

        if (sdl_get_message(msgVec, &msgText) == SDL_NORMAL)
        {
            fprintf(stderr, "%s\n", msgText);
            sdl_free(msgText);
        }
    }
    return;
}

/*
 * _sdl_parse_opt
 *  This function is called repeatedly with individual command line options and
 *  zero or one associated option arguments.
 *
 * Input Parameters:
 *  key:
 *      A value indicating the command-line option being processed.
 *  arg:
 *      A pointer to an option argument.  This may be NULL if the option does
 *      not have a defined argument, or one was not specified.
 *
 * Output Parameters:
 *  state:
 *      A pointer to the parsing state structure.  This contains a pointer to
 *      the context structure where parsed argument information is placed.
 *
 * Return Values:
 *  0:                  Option and argument parsed successfully.
 *  ARGP_ERR_UNKNOWN:   Option or argument were not recognized or parsed
 *                      successfully.
 */
static error_t _sdl_parse_opt(int key, char *arg, struct argp_state *state)
{
    SDL_CONTEXT *context = (SDL_CONTEXT *) state->input;
    SDL_ARGUMENTS *args = context->argument;
    error_t retVal = 0;
    int ii;

    printf("key: ");
    if ((key < 256) && (isprint(key) != 0))
        printf("%c", key);
    else
        printf("%08x", key);
    printf(" arg: ");
    if (arg == NULL)
        printf("(null)");
    else
        printf("%s", arg);
    printf("\n");

    switch (key)
    {
        case ARGP_KEY_ARG:
            if (args[ArgInputFile].present == false)
            {
                args[ArgInputFile].present = true;
                args[ArgInputFile].fileName = sdl_strdup(arg);

                /*
                 * We at least need read access to the file.
                 */
                if (access(arg, R_OK) != 0)
                {
                    sdl_set_message(msgVec, 1, SDL_INFILOPN, arg);
                    retVal = EACCES;
                }
            }
            break;

        case SDL_K_ARG_NOCHECK:
            if (args[ArgCheckAlignment].present == false)
            {
                args[ArgCheckAlignment].present = true;
                args[ArgCheckAlignment].on = false;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--check|--nocheck");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_NOCOMMENT:
            if (args[ArgComments].present == false)
            {
                args[ArgComments].present = true;
                args[ArgComments].on = false;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--comment|--nocomment");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_B32:
            if (args[ArgWordSize].present == false)
            {
                args[ArgWordSize].present = true;
                args[ArgWordSize].value = 32;
            }
            else
            {
                sdl_set_message(msgVec, 1, SDL_CONFLDUPLQ, "--b32|--b64");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_NOCOPY:
            if (args[ArgCopyright].present == false)
            {
                args[ArgCopyright].present = true;
                args[ArgCopyright].on = false;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--copy|--nocopy");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_NOHEADER:
            if (args[ArgHeader].present == false)
            {
                args[ArgHeader].present = true;
                args[ArgHeader].on = false;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--header|--noheader");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_B64:
            if (args[ArgWordSize].present == false)
            {
                args[ArgWordSize].present = true;
                args[ArgWordSize].value = 64;
            }
            else
            {
                sdl_set_message(msgVec, 1, SDL_CONFLDUPLQ, "--b32|--b64");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_NOLIST:
            if (args[ArgListing].present == false)
            {
                args[ArgListing].present = true;
                args[ArgListing].on = false;
            }
            else
            {
                sdl_set_message(msgVec, 1, SDL_DUPLISTQUAL);
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_NOMEMBER:
            if (args[ArgMemberAlign].present == false)
            {
                args[ArgMemberAlign].present = true;
                args[ArgMemberAlign].on = false;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--member|--nomember");
                retVal = EINVAL;
            }
            break;

        case SDL_K_ARG_NOMODULE:
            sdl_set_message(msgVec, 1, SDL_INVQUAL, "--nomodule");
            retVal = ARGP_ERR_UNKNOWN;
            break;

        case SDL_K_ARG_NOPARSE:
            sdl_set_message(msgVec, 1, SDL_INVQUAL, "--noparse");
            retVal = ARGP_ERR_UNKNOWN;
            break;

        case SDL_K_ARG_NOSUPPRESS:
            if (arg != NULL)
            {
                char *ptr = strchr(arg, ',');
                char *first = arg;
                char *second = NULL;
                size_t firstLen = strlen(first);
                size_t secondLen = 0;

                if (ptr != NULL)
                {
                    second = ptr + 1;
                    secondLen = strlen(second);
                    firstLen = ptr - arg;
                }
                if (strncmp(first, "prefix", firstLen) == 0)
                {
                    args[ArgSuppressPrefix].present = true;
                    args[ArgSuppressPrefix].on = false;
                }
                else if (strncmp(first, "tag", firstLen) == 0)
                {
                    args[ArgSuppressTag].present = true;
                    args[ArgSuppressTag].on = false;
                }
                else
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_INVQUAL,
                                    "--nosuppress");
                    retVal = EINVAL;
                }
                if ((second != NULL) &&
                    (strncmp(second, "prefix", secondLen) == 0))
                {
                    args[ArgSuppressPrefix].present = true;
                    args[ArgSuppressPrefix].on = false;
                }
                else if ((second != NULL) &&
                         (strncmp(second, "tag", secondLen) == 0))
                {
                    args[ArgSuppressTag].present = true;
                    args[ArgSuppressTag].on = false;
                }
                else if (second != NULL)
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_INVQUAL,
                                    "--nosuppress");
                    retVal = EINVAL;
                }
            }
            else
            {
                args[ArgSuppressPrefix].present = true;
                args[ArgSuppressPrefix].on = false;
                args[ArgSuppressTag].present = true;
                args[ArgSuppressTag].on = false;
            }
            break;

        case 'C':
            if (args[ArgCopyright].present == false)
            {
                char *path = realpath(state->argv[0], NULL);
                int jj;

                args[ArgCopyright].present = true;
                args[ArgCopyright].on = true;
                if (path != NULL)
                {
                    jj = strlen(path) - 1;
                    while ((jj > 0) && (path[jj - 1] != '/'))
                    {
                        jj--;
                    }
                    args[ArgCopyrightFile].present = true;
                    args[ArgCopyrightFile].fileName =
                        sdl_calloc(1, jj + 14);
                    strncpy(args[ArgCopyrightFile].fileName,
                            path,
                            jj);
                    strcpy(&args[ArgCopyrightFile].fileName[jj],
                           "copyright.sdl");
                    sdl_free(path);
                    if (access(args[ArgCopyrightFile].fileName, R_OK) != 0)
                    {
                        sdl_set_message(msgVec,
                                        1,
                                        SDL_NOCOPYFIL);
                        retVal = EACCES;
                    }
                }
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--copy|--nocopy");
                retVal = EINVAL;
            }
            break;

        case 'H':
            if (args[ArgHeader].present == false)
            {
                args[ArgHeader].present = true;
                args[ArgHeader].on = true;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--header|--noheader");
                retVal = EINVAL;
            }
            break;

        case 'L':
            if (args[ArgListing].present == false)
            {
                args[ArgListing].present = true;
                args[ArgListing].on = true;
                if (arg != NULL)
                {
                    args[ArgListingFile].present = true;
                    args[ArgListingFile].fileName =
                        sdl_strdup(&arg[1]);
                }
            }
            else
            {
                sdl_set_message(msgVec, 1, SDL_DUPLISTQUAL);
                retVal = EINVAL;
            }
            break;

        case 'M':
            sdl_set_message(msgVec, 1, SDL_INVQUAL, "-M|--module");
            retVal = ARGP_ERR_UNKNOWN;
            break;

        case 'S':
            if (arg != NULL)
            {
                char *ptr = strchr(arg, ',');
                char *first = arg;
                char *second = NULL;
                size_t firstLen = strlen(first);
                size_t secondLen = 0;

                if (ptr != NULL)
                {
                    second = ptr + 1;
                    secondLen = strlen(second);
                    firstLen = ptr - arg;
                }
                if (strncmp(first, "prefix", firstLen) == 0)
                {
                    args[ArgSuppressPrefix].present = true;
                    args[ArgSuppressPrefix].on = true;
                }
                else if (strncmp(first, "tag", firstLen) == 0)
                {
                    args[ArgSuppressTag].present = true;
                    args[ArgSuppressTag].on = true;
                }
                else
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_INVQUAL,
                                    "--suppress");
                    retVal = EINVAL;
                }
                if ((second != NULL) &&
                    (strncmp(second, "prefix", secondLen) == 0))
                {
                    args[ArgSuppressPrefix].present = true;
                    args[ArgSuppressPrefix].on = true;
                }
                else if ((second != NULL) &&
                         (strncmp(second, "tag", secondLen) == 0))
                {
                    args[ArgSuppressTag].present = true;
                    args[ArgSuppressTag].on = true;
                }
                else if (second != NULL)
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_INVQUAL,
                                    "--suppress");
                    retVal = EINVAL;
                }
            }
            else
            {
                args[ArgSuppressPrefix].present = true;
                args[ArgSuppressPrefix].on = true;
                args[ArgSuppressTag].present = true;
                args[ArgSuppressTag].on = true;
            }
            break;

        case 'a':
            if (arg != NULL)
            {
                if ((strlen(arg) != 1) ||
                    ((arg[0] != '0') &&
                     (arg[0] != '1') &&
                     (arg[0] != '2') &&
                     (arg[0] != '4') &&
                     (arg[0] != '8')))
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_INVALIGN);
                    retVal = EINVAL;
                }
                if (args[ArgAlignment].present == false)
                {
                    args[ArgAlignment].present = true;
                    args[ArgAlignment].value = strtol(arg, NULL, 10);
                }
                else
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_CONFLDUPLQ,
                                    "--align|--noalign");
                    retVal = EINVAL;
                }
            }
            else
            {
                sdl_set_message(msgVec, 1, SDL_INVQUAL, "-a|--align");
                retVal = EINVAL;
            }
            break;

        case 'c':
            if (args[ArgComments].present == false)
            {
                args[ArgComments].present = true;
                args[ArgComments].on = true;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--comments|--nocomments");
                retVal = EINVAL;
            }
            break;

        case 'k':
            if (args[ArgCheckAlignment].present == false)
            {
                args[ArgCheckAlignment].present = true;
                args[ArgCheckAlignment].on = true;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--check|--nocheck");
                retVal = EINVAL;
            }
            break;

        case 'l':
            if (arg != NULL)
            {
                SDL_LANGUAGES *langs = args[ArgLanguage].languages;
                char *ptr;
                int jj = 0;
                bool found = false;

                /*
                 * Find out if there was a output file specified with the
                 * argument.  The file can be preceded by either an equals sign
                 * or a colon.
                 */
                ptr = strchr(arg, '=');
                if (ptr == NULL)
                {
                    ptr = strchr(arg, ':');
                }

                /*
                 * If the pointer is still NULL, then set it to the end of the
                 * argument (which is the null character).  Otherwise, change
                 * the character we found to a null character and move the
                 * pointer to the first letter in the output file.
                 */
                if (ptr == NULL)
                {
                    ptr = &arg[strlen(arg)];
                }
                else
                {
                    *ptr = '\0';
                    ptr++;
                }

                /*
                 * Loop through the list of languages already specified and see
                 * if we have already seen this one.  If we have, we return an
                 * error.
                 */
                for (jj = 0;
                     ((jj < context->languagesSpecified) && (found == false));
                     jj++)
                {
                    if (strcasecmp(arg, langs[jj].langStr) == 0)
                    {
                        found = true;
                    }
                }

                /*
                 * If we didn't find the language already specified, then we
                 * have some work to do.
                 */
                if (found == false)
                {
                    uint32_t status = SDL_NORMAL;

                    /*
                     * First, let's allocate a language item for this guy.
                     */
                    langs = sdl_realloc(langs,
                                        ((context->languagesSpecified + 1) *
                                         sizeof(SDL_LANGUAGES)));
                    if (langs != NULL)
                    {
                        uint32_t index = context->languagesSpecified;

                        context->languagesSpecified++;
                        args[ArgLanguage].languages = langs;

                        /*
                         * Now let's do a bit of initialization of the new
                         * entry.
                         */
                        langs[index].langStr = strdup(arg);

                        /*
                         * Try and load the shared library for this language.
                         */
                        status = sdl_load_plugin(arg,
                                                 &langs[index].extension,
                                                 &langs[index].langVal);
                        if (status == SDL_NORMAL)
                        {
                            if (*ptr != '\0')
                            {
                                langs[index].outFileName = sdl_strdup(ptr);
                            }
                            else
                            {
                                langs[index].outFileName = NULL;
                            }
                        }
                        else
                        {
                            retVal = EINVAL;
                        }
                    }
                    args[ArgLanguage].present = true;
                }
                else
                {
                    sdl_set_message(msgVec, 1, SDL_DUPLANG, arg);
                    retVal = EINVAL;
                }
            }
            break;

        case 'm':
            if (args[ArgMemberAlign].present == false)
            {
                args[ArgMemberAlign].present = true;
                args[ArgMemberAlign].on = true;
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_CONFLDUPLQ,
                                "--member|--nomember");
                retVal = EINVAL;
            }
            break;

        case 'p':
            sdl_set_message(msgVec, 1, SDL_INVQUAL, "-p|--parse");
            retVal = ARGP_ERR_UNKNOWN;
            break;

        case 's':
            if (arg != NULL)
            {
                SDL_SYMBOL_LIST *list = args[ArgSymbols].symbol;
                char *ptr;

                ptr = strchr(arg, ':');
                if (ptr != NULL)
                {
                    char *ptr2 = strchr(arg, '=');
                    char *symbol;
                    int value;
                    bool noValue = false;

                    ptr++;
                    if (ptr2 == NULL)
                    {
                        ptr2 = &arg[strlen(arg)];
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
                        bool found = false;
                        int jj;

                        for (jj = 0;
                             ((jj < list->listUsed) && (found == false));
                             jj++)
                        {
                            if (strcmp(list->symbols[jj].symbol, symbol) == 0)
                            {
                                found = true;
                            }
                        }
                        if (found == false)
                        {
                            if (list->listUsed >= list->listSize)
                            {
                                list->listSize++;
                                list->symbols = sdl_realloc(list->symbols,
                                                            (sizeof(SDL_SYMBOL) *
                                                             list->listSize));
                            }
                            list->symbols[list->listUsed].symbol = symbol;
                            list->symbols[list->listUsed++].value = value;
                        }
                        else
                        {
                            sdl_set_message(msgVec,
                                            1,
                                            SDL_SYMALRDEF,
                                            "-s||--symbol");
                            retVal = EINVAL;
                        }
                    }
                    else
                    {
                        sdl_set_message(msgVec,
                                        1,
                                        SDL_ABORT,
                                        "-s||--symbol");
                        retVal = EINVAL;
                    }
                }
                else
                {
                    sdl_set_message(msgVec,
                                    1,
                                    SDL_INVQUAL,
                                    "-s||--symbol");
                    retVal = EINVAL;
                }
            }
            else
            {
                sdl_set_message(msgVec,
                                1,
                                SDL_INVQUAL,
                                "-s||--symbol");
                retVal = EINVAL;
            }
            break;

        case 't':
            if (args[ArgTraceMemory].present == false)
            {
                args[ArgTraceMemory].present = true;
                args[ArgTraceMemory].on = true;
            }
            break;

        case 'v':
            if ((args[ArgTrace].present == false) ||
                (args[ArgVerbose].present == false))
            {
                args[ArgTrace].present = true;
                args[ArgTrace].on = true;
                args[ArgVerbose].present = true;
                args[ArgVerbose].value = 1;
            }
            break;

        case ARGP_KEY_END:
            if (args[ArgLanguage].present == false)
            {
                sdl_set_message(msgVec, 1, SDL_NOOUTPUT);
                retVal = ARGP_ERR_UNKNOWN;
            }
            break;

        case ARGP_KEY_NO_ARGS:
            sdl_set_message(msgVec, 1, SDL_NOOUTPUT);
            retVal = ARGP_ERR_UNKNOWN;
            break;

        case ARGP_KEY_INIT:
            args[ArgAlignment].present = false;
            args[ArgAlignment].value = 0;
            args[ArgCheckAlignment].present = false;
            args[ArgCheckAlignment].on = false;
            args[ArgComments].present = false;
            args[ArgComments].on = true;
            args[ArgCopyright].present = false;
            args[ArgCopyright].on = false;
            args[ArgCopyrightFile].present = false;
            args[ArgCopyrightFile].fileName = NULL;
            args[ArgHeader].present = false;
            args[ArgHeader].on = true;
            args[ArgInputFile].present = false;
            args[ArgInputFile].fileName = NULL;
            args[ArgLanguage].present = false;
            args[ArgListing].present = false;
            args[ArgListing].on = false;
            args[ArgListingFile].present = false;
            args[ArgListingFile].fileName = NULL;
            args[ArgMemberAlign].present = false;
            args[ArgMemberAlign].on = true;
            args[ArgSymbols].present = false;
            args[ArgSymbols].symbol->symbols = NULL;
            args[ArgSymbols].symbol->listSize = 0;
            args[ArgSymbols].symbol->listUsed = 0;
            args[ArgSuppressPrefix].present = false;
            args[ArgSuppressPrefix].on = false;
            args[ArgSuppressTag].present = false;
            args[ArgSuppressTag].on = false;
            args[ArgTraceMemory].present = false;
            args[ArgTraceMemory].on = false;
            args[ArgTrace].present = false;
            args[ArgTrace].on = false;
            args[ArgVerbose].present = false;
            args[ArgVerbose].on = false;
            args[ArgWordSize].present = false;
            args[ArgWordSize].value = 64;
            break;

        case ARGP_KEY_SUCCESS:
        case ARGP_KEY_ERROR:
            break;

        case ARGP_KEY_FINI:
            break;

        default:
            sdl_set_message(msgVec, 1, SDL_INVQUAL, &key);
            retVal = ARGP_ERR_UNKNOWN;
            break;
    }

    /*
     * Return back to the caller.
     */
    return retVal;
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
    FILE            *cfp = NULL;
    FILE            *fp;
    char            *msgTxt = NULL;
    struct tm       *timeInfo;
    SDL_ARGUMENTS   *args = context.argument;
    SDL_LANGUAGES   *languages;
    SDL_SYMBOL_LIST symbols;
    time_t          localTime;
    uint32_t        status;
    int             ii, jj;

    /*
     * Turn off generating a list file, by default.  It may get set when
     * parsing the command line arguments.
     */
    listing = false;
    memset(&context, 0, sizeof(SDL_CONTEXT));
    SDL_Q_INIT(&literal);

    /*
     * Get the current time as the start time.
     */
    localTime = time(NULL);
    timeInfo = localtime(&localTime);
    memcpy(&context.runTimeInfo, timeInfo, sizeof(struct tm));

    /*
     * Initialize the parsing context.
     */
    args[ArgLanguage].languages = NULL;
    args[ArgSymbols].symbol = &symbols;
    context.processingEnabled = true;

    /*
     * Initialize the dimension array and the options index.
     */
    for (ii = 0; ii < SDL_K_MAX_DIMENSIONS; ii++)
    {
        context.dimensions[ii].inUse = false;
    }

    /*
     * Set the options, parameters and languages workspace.
     */
    context.options = NULL;
    context.optionsSize = 0;
    context.optionsIdx = 0;
    context.parameters = NULL;
    context.parameterSize = 0;
    context.parameterIdx = 0;
    context.stateStack = NULL;
    context.stateSize = 0;
    context.stateIdx = 0;
    context.langCondList.lang = NULL;
    context.langCondList.listSize = 0;
    context.langCondList.listUsed = 0;
    context.langEnableVec = NULL;
    context.languagesSpecified = 0;

    /*
     * Set the message vector to a success value.
     */
    sdl_set_message(msgVec, 1, SDL_NORMAL);

    /*
     * Parse out the command line arguments.
     */
    status = argp_parse(&argp, argc, argv, 0, 0, &context);
    if ((status != 0) ||
        (errno != 0) ||
        (msgVec[0].msgCode.msgCode != SDL_NORMAL))
    {
        status = sdl_get_message(msgVec, &msgTxt);
        if (status == SDL_NORMAL)
        {
            fprintf(stderr, errFmt, msgTxt);
        }
        sdl_free(msgTxt);
        return (-1);
    }

    /*
     * Set some global variables that all the other code needs to be aware.
     */
    listing = args[ArgListing].on;
    if (args[ArgTraceMemory].on == true)
    {
        sdl_set_trace_memory();
    }
    trace = args[ArgTrace].on;
    _verbose = (args[ArgVerbose].on ? 1 : 0);

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
        printf("%s:%d:main\n", __FILE__, __LINE__);
    }

    /*
     * We now know the languages for which we are going to be generating output
     * files.
     */
    languages = args[ArgLanguage].languages;

    /*
     * Initialize the parsing states.
     */
    context.state = Initial;
    context.condState.state = sdl_calloc(SDL_K_COND_STATE_SIZE,
                                         sizeof(SDL_COND_STATES));
    context.condState.state[0] = CondNone;
    context.condState.top = 0;
    context.condState.bottom = SDL_K_COND_STATE_SIZE;
    context.langEnableVec = sdl_calloc(context.languagesSpecified, sizeof(bool));

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

    if (args[ArgInputFile].present == false)
    {
        status = sdl_set_message(msgVec,
                                 1,
                                 SDL_NOINPFIL);
        if (status == SDL_NORMAL)
        {
            status = sdl_get_message(msgVec, &msgTxt);
        }
        if (status == SDL_NORMAL)
        {
            fprintf(stderr, errFmt, msgTxt);
        }
        sdl_free(msgTxt);
        return (-1);
    }

    /*
     * Open the input file or reading.
     */
    if ((fp = fopen(args[ArgInputFile].fileName, "r")) == NULL)
    {
        status = sdl_set_message(msgVec,
                                 2,
                                 SDL_INFILOPN,
                                 args[ArgInputFile].fileName,
                                 errno);
        if (status == SDL_NORMAL)
        {
            status = sdl_get_message(msgVec, &msgTxt);
        }
        if (status == SDL_NORMAL)
        {
            fprintf(stderr, errFmt, msgTxt);
        }
        sdl_free(msgTxt);
        return (-1);
    }

    /*
     * If the user indicated that they wanted the copyright information at the
     * start of the file, then open it for read.
     */
    if (args[ArgCopyright].on == true)
    {
        if (args[ArgCopyrightFile].present == false)
        {
            status = sdl_set_message(msgVec, 1, SDL_NOCOPYFIL);
            if (status == SDL_NORMAL)
            {
                status = sdl_get_message(msgVec, &msgTxt);
            }
            if (status == SDL_NORMAL)
            {
                fprintf(stderr, errFmt, msgTxt);
            }
        }
        else if ((cfp = fopen(args[ArgCopyrightFile].fileName,
                              "r")) == NULL)
        {
            status = sdl_set_message(msgVec,
                                     2,
                                     SDL_INFILOPN,
                                     args[ArgCopyrightFile].fileName,
                                     errno);
            if (status == SDL_NORMAL)
            {
                status = sdl_get_message(msgVec, &msgTxt);
            }
            if (status == SDL_NORMAL)
            {
                fprintf(stderr, errFmt, msgTxt);
            }
        }
        if (msgTxt != NULL)
        {
            sdl_free(msgTxt);
            return (-1);
        }
    }

    /*
     * Loop through each of the supported languages.
     */
    for (ii = 0; ii < context.languagesSpecified; ii++)
    {
        FILE *outFP;

        if (languages[ii].outFileName == NULL)
        {
            bool addDot = false;

            /*
             * Go find the last '.' in the file name, this will be where the
             * file extension starts.
             */
            for (jj = strlen(context.argument[ArgInputFile].fileName);
                 jj >= 0;
                 jj--)
            {
                if (context.argument[ArgInputFile].fileName[jj] == '.')
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
            {
                jj = strlen(args[ArgInputFile].fileName);
                addDot = true;
            }

            /*
             * Now allocate a buffer large enough for the file name,
             * extension, and null terminator.
             */
            languages[ii].outFileName =
                    sdl_calloc((jj + strlen(languages[ii].extension) + 2), 1);

            /*
             * Copy the extension for this language after the last '.' (or
             * the one just added).
             */
            strncpy(languages[ii].outFileName, args[ArgInputFile].fileName, jj);
            if (addDot == true)
            {
                languages[ii].outFileName[jj++] = '.';
            }
            strcpy(&languages[ii].outFileName[jj],
                   languages[ii].extension);
        }

        /*
         * Try and open the file for this language.  If it fails, we are
         * done.
         */
        if ((outFP = fopen(languages[ii].outFileName, "w")) == NULL)
        {
            status = sdl_set_message(msgVec,
                                     2,
                                     SDL_OUTFILOPN,
                                     languages[ii].outFileName,
                                     errno);
            if (status == SDL_NORMAL)
            {
                status = sdl_get_message(msgVec, &msgTxt);
            }
            if (status == SDL_NORMAL)
            {
                fprintf(stderr, errFmt, msgTxt);
            }
            sdl_free(msgTxt);
            return (-1);
        }
        else
        {
            status = sdl_load_fp(ii, outFP);
            if (status != SDL_NORMAL)
            {
                status = sdl_get_message(msgVec, &msgTxt);
                if (status == SDL_NORMAL)
                {
                    fprintf(stderr, errFmt, msgTxt);
                }
                sdl_free(msgTxt);
                return (-1);
            }
            context.langEnableVec[ii] = true;
        }
    }
    if (args[ArgHeader].on == true)
    {

        /*
         * OK, we successfully opened the files.  Insert the header
         * comments.  First starting with a row of '*'s.
         */
        if (sdl_call_commentStars() == SDL_NORMAL)
        {

            /*
             * If we put the row of '*'s in, now put in information
             * about OpenSDL.
             */
            if (sdl_call_createdByInfo(&context.runTimeInfo) == SDL_NORMAL)
            {
                struct stat fileStats;

                context.inputPath = realpath(args[ArgInputFile].fileName,
                                             NULL);
                if (context.inputPath == NULL)
                {
                    context.inputPath = strdup(args[ArgInputFile].fileName);
                }
                if (stat(context.inputPath, &fileStats) != 0)
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
                memcpy(&context.inputTimeInfo, timeInfo, sizeof(struct tm));
                if (sdl_call_fileInfo(&context.inputTimeInfo,
                                      context.inputPath) == SDL_NORMAL)
                {

                    /*
                     * Finally, if we get here, we just need to output
                     * another row of '*'s, and then can begin our
                     * actual input file parsing and output file
                     * generation.
                     */
                    if (sdl_call_commentStars() != SDL_NORMAL)
                    {
                        status = sdl_get_message(msgVec, &msgTxt);
                        if (status == SDL_NORMAL)
                        {
                            fprintf(stderr, errFmt, msgTxt);
                        }
                        sdl_free(msgTxt);
                        return (-1);
                    }
                }
                else
                {
                    status = sdl_get_message(msgVec, &msgTxt);
                    if (status == SDL_NORMAL)
                    {
                        fprintf(stderr, errFmt, msgTxt);
                    }
                    sdl_free(msgTxt);
                    return (-1);
                }
            }
            else
            {
                status = sdl_get_message(msgVec, &msgTxt);
                if (status == SDL_NORMAL)
                {
                    fprintf(stderr, errFmt, msgTxt);
                }
                sdl_free(msgTxt);
                return (-1);
            }
        }
        else
        {
            status = sdl_get_message(msgVec, &msgTxt);
            if (status == SDL_NORMAL)
            {
                fprintf(stderr, errFmt, msgTxt);
            }
            sdl_free(msgTxt);
            return (-1);
        }
    }

    /*
     * If we get this far, then go and write out the header information to each
     * languages output file.
     */

    SDL_Q_INIT(&context.locals);
    context.module = NULL;
    context.ident = NULL;

    /*
     * If the copyright needs to be put into the file, then do so now.
     */
    if (cfp != NULL)
    {
        bool deferListing = listing;

        listing = false;
        yylex_init(&scanner);
        yyset_debug(_verbose, scanner);
        yyset_in(cfp, scanner);
        yyparse(scanner);
        yylex_destroy(scanner);
        listing = deferListing;
    }

    /*
     * If we are being asked to create a listing file, then do so now.
     */
    if (listing == true)
    {

        /*
         * If the listing file name was not specified by the user, then
         * generate one from the input file, using '.lis' as the file
         * extension.
         */
        if (args[ArgListingFile].present == false)
        {
            for (ii = strlen(args[ArgInputFile].fileName);
                 ii >= 0;
                 ii--)
            {
                if (args[ArgInputFile].fileName[ii] == '.')
                {
                    break;
                }
            }
            if (ii <= 0)
            {
                ii = strlen(args[ArgInputFile].fileName);
            }
            args[ArgListingFile].fileName = sdl_calloc(ii + 5, 1);
            strncpy(args[ArgListingFile].fileName,
                    args[ArgInputFile].fileName, ii);
            strcpy(&args[ArgListingFile].fileName[ii], ".lis");
        }

        /*
         * Before we begin parsing the input file, open the listing file.
         */
        listingFP = sdl_open_list(&context);
    }

    /*
     * Start parsing the real input file
     */
    yylex_init(&scanner);
    yyset_debug(_verbose, scanner);

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
     * If were asked to create a listing file, then close it now.
     */
    if (listing == true)
    {
        sdl_close_list(&context);
        listingFP = NULL;
        listing = false;
    }

    /*
     * Go close all the output files.
     */
    sdl_call_close();

    /*
     * Return the results back to the caller.
     */
    if (trace == true)
    {
        fprintf(stderr,
                "'%s' has been processed\n",
                args[ArgInputFile].fileName);
    }

    /*
     * Clean-up memory.
     */
    for (ii = 0; ii < context.langCondList.listUsed; ii++)
    {
        sdl_free(context.langCondList.lang[ii]);
    }
    for (ii = 0; ii < args[ArgSymbols].symbol->listUsed; ii++)
    {
        if (args[ArgSymbols].symbol->symbols[ii].symbol != NULL)
        {
            sdl_free(args[ArgSymbols].symbol->symbols[ii].symbol);
        }
    }
    for (ii = 0; ii < context.languagesSpecified; ii++)
    {
        if (languages[ii].langStr != NULL)
        {
            sdl_free(languages[ii].langStr);
        }
        if (languages[ii].outFileName != NULL)
        {
            sdl_free(languages[ii].outFileName);
        }
    }
    if (languages != NULL)
    {
        sdl_free(languages);
    }
    if (args[ArgSymbols].symbol->symbols != NULL)
    {
        sdl_free(args[ArgSymbols].symbol->symbols);
    }
    if (args[ArgInputFile].fileName != NULL)
    {
        sdl_free(args[ArgInputFile].fileName);
    }
    if (context.inputPath != NULL)
    {
        sdl_free(context.inputPath);
    }

    /*
     * Return back to the caller.
     */
    return (0);
}
