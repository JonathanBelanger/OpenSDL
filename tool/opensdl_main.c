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
#include "lib/parser/opensdl_parser.h"
#include "lib/parser/opensdl_lexical.h"
#include "lib/util/opensdl_blocks.h"
#include "lib/lang/opensdl_lang.h"
#include "lib/util/opensdl_message.h"

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
bool traceMemory;
bool listing;
FILE *listingFP = NULL;
int _verbose;

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
    (SDL_FUNC) NULL}};

static char *_extensions[] =
{"h", /* C */
NULL};

char *sdl_months[] =
{
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"};

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
        "A copyright header is included in the output file (see copyright.sdl "
            "for what is included).",
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
        0,
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
        "prefix|tag",
        OPTION_ARG_OPTIONAL,
        "Suppress outputting symbols with a prefix, tag, or both.",
        0
    },
    {
        "nosuppress",
        SDL_K_ARG_NOSUPPRESS,
        "prefix|tag",
        OPTION_ARG_OPTIONAL,
        "Do not suppress outputting symbols with a prefix, tag, or both. "
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
            }

            /*
             * TODO: Need to determine what we are going to do about multiple
             * input files.
             *
             * TODO: Check for the accessibility to the input file.  We should
             * have read access to it.
             */
            break;

        case SDL_K_ARG_NOCHECK:
            if (args[ArgCheckAlignment].present == false)
            {
                args[ArgCheckAlignment].present = true;
                args[ArgCheckAlignment].on = false;
            }

            /*
             * TODO: Need to determine what we are going to do about multiple
             * --nocheck or --check qualifiers.
             */
            break;

        case SDL_K_ARG_NOCOMMENT:
            if (args[ArgComments].present == false)
            {
                args[ArgComments].present = true;
                args[ArgComments].on = false;
            }

            /*
             * TODO: Need to determine error when multiple --nocomment  or
             * --comment on command line
             */
            break;

        case SDL_K_ARG_B32:
            if (args[ArgWordSize].present == false)
            {
                args[ArgWordSize].present = true;
                args[ArgWordSize].value = 32;
            }

            /*
             * TODO: Need to determine error when multiple --b32 or --b64 on
             * command line
             */
            break;

        case SDL_K_ARG_NOCOPY:
            if (args[ArgCopyright].present == false)
            {
                args[ArgCopyright].present = true;
                args[ArgCopyright].on = false;
            }

            /*
             * TODO: Need to determine error when multiple --nocopy  or --copy
             * on command line
             */
            break;

        case SDL_K_ARG_NOHEADER:
            if (args[ArgHeader].present == false)
            {
                args[ArgHeader].present = true;
                args[ArgHeader].on = false;
            }

            /*
             * TODO: Need to determine error when multiple --noheader  or
             * --header on command line
             */
            break;

        case SDL_K_ARG_B64:
            if (args[ArgWordSize].present == false)
            {
                args[ArgWordSize].present = true;
                args[ArgWordSize].value = 64;
            }

            /*
             * TODO: Need to determine error when multiple --b32 or --b64 on
             * command line
             */
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

            /*
             * TODO: Need to determine error when multiple --nomember or
             * --member on command line
             */
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
            if (args[ArgSuppressPrefix].present == false)
            {
                args[ArgSuppressPrefix].present = true;
                args[ArgSuppressPrefix].on = false;
            }

            /*
             * TODO: Need to determine error when multiple --nosuppress  or
             * --suppress on command line
             */
            if (args[ArgSuppressTag].present == false)
            {
                args[ArgSuppressTag].present = true;
                args[ArgSuppressTag].on = false;
            }

            /*
             * TODO: Need to determine error when multiple --nosuppress  or
             * --suppress on command line
             */
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
                    free(path);
                }
            }

            /*
             * TODO: Need to determine error when multiple --nocopy  or
             * --copy on command line.
             *
             * TODO: If a copyright file is specified, then we need to have
             * read access to it.
             *
             * TODO: This needs to be updated. We should always look for the
             * copyright file in the same place or make sure that it is in the
             * same directory with the opensdl image.
             */
            break;

        case 'H':
            if (args[ArgHeader].present == false)
            {
                args[ArgHeader].present = true;
                args[ArgHeader].on = true;
            }

            /*
             * TODO: Need to determine error when multiple --noheader  or
             * --header on command line
             */
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

            /*
             * TODO: This needs some work.
             */
            if (arg != NULL)
            {
                char *ptr;

                ptr = strchr(arg, ':');
                if (ptr != NULL)
                {
                    ptr++;
                    if (strncasecmp(ptr, "prefix", 6) == 0)
                    {
                        args[ArgSuppressPrefix].present = true;
                        args[ArgSuppressPrefix].on = true;
                    }
                    else if (strncasecmp(ptr, "tag", 3) == 0)
                    {
                        args[ArgSuppressTag].present = true;
                        args[ArgSuppressTag].on = true;
                    }
                    else
                    {
                        sdl_set_message(msgVec,
                                        1,
                                        SDL_INVQUAL,
                                        "-S|--suppress");
                        retVal = EINVAL;
                    }
                    if (retVal == 0)
                    {
                        ptr = strchr(arg, ',');
                        if (ptr != NULL)
                        {
                            ptr++;
                            if (strncasecmp(ptr, "prefix", 6) == 0)
                            {
                                args[ArgSuppressPrefix].present = true;
                                args[ArgSuppressPrefix].on = true;
                            }
                            else if (strncasecmp(ptr, "tag", 3) == 0)
                            {
                                args[ArgSuppressTag].present = true;
                                args[ArgSuppressTag].on = true;
                            }
                            else
                            {
                                sdl_set_message(msgVec,
                                                1,
                                                SDL_INVQUAL,
                                                "-S|--suppress");
                                retVal = ARGP_ERR_UNKNOWN;
                            }
                        }
                    }
                }
                else
                {
                    args[ArgSuppressPrefix].present = true;
                    args[ArgSuppressPrefix].on = true;
                    args[ArgSuppressTag].present = true;
                    args[ArgSuppressTag].on = true;
                }
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
                    args[ArgAlignment].value = strtol(arg,
                                                                   NULL,
                                                                   10);
                }

                /*
                 * TODO: Need to determine error when multiple -a on command line
                 */
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

            /*
             * TODO: Need to determine error when both -c and --nocomments on
             * command line
             */
            break;

        case 'k':
            if (args[ArgCheckAlignment].present == false)
            {
                args[ArgCheckAlignment].present = true;
                args[ArgCheckAlignment].on = true;
            }

            /*
             * TODO: Need to determine error when multiple -k on command line
             */
            break;

        case 'l':
            if (arg != NULL)
            {
                char *ptr;
                int jj = 0;

                while ((args[ArgLanguage].languages[jj].langStr !=
                            NULL) &&
                       (args[ArgLanguage].languages[jj].langVal !=
                            -1) &&
                       (retVal == 0))
                {
                    ptr = strchr(arg, '=');
                    if (ptr == NULL)
                    {
                        ptr = &arg[strlen(arg)];
                    }
                    if (strncasecmp(arg,
                          args[ArgLanguage].languages[jj].langStr,
                          (ptr - arg)) == 0)
                    {
                        int lang = args[ArgLanguage].languages[jj].langVal;

                        if (args[ArgLanguage].languages[lang].langSpec == false)
                        {
                            args[ArgLanguage].languages[lang].langSpec = true;
                            if (*ptr == '=')
                            {
                                args[ArgLanguage].languages[lang].outFileName =
                                    sdl_strdup(&ptr[1]);
                            }
                            args[ArgLanguage].present = true;
                        }
                        else
                        {
                            char lang[32];

                            strncpy(lang, arg, (ptr - arg));
                            sdl_set_message(msgVec, 1, SDL_DUPLANG, lang);
                            retVal = EINVAL;
                        }
                    }
                    jj++;
                }
                if ((args[ArgLanguage].present == false) && (retVal == 0))
                {
                    sdl_set_message(msgVec, 1, SDL_INVQUAL, "-l|-lang");
                    retVal = EINVAL;
                }
            }

            /* TODO: Need to error out that a language was NOT specified. */
            break;

        case 'm':
            if (args[ArgMemberAlign].present == false)
            {
                args[ArgMemberAlign].present = true;
                args[ArgMemberAlign].on = true;
            }

            /*
             * TODO: Need to determine error when multiple --nomember or
             * --member on command line
             */
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
                                        (sizeof(SDL_SYMBOL) * list->listSize));
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
    SDL_LANGUAGES   languages[SDL_K_LANG_MAX];
    SDL_SYMBOL_LIST symbols;
    time_t          localTime;
    uint32_t        status;
    int             ii, jj;

    /*
     * Turn off tracing
     */
    trace = false;
    traceMemory = false;
    _verbose = 0;

    /*
     * Turn off generating a list file, by default.  It may get set when
     * parsing the command line arguments.
     */
    listing = false;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
    {
        printf("%s:%d:main\n", __FILE__, __LINE__);
    }
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
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {
        languages[ii].langStr = strdup("cc");
        languages[ii].langVal = SDL_K_LANG_C;
        languages[ii].langSpec = false;
        languages[ii].langEna = true;
        languages[ii].outFileName = NULL;
        languages[ii].outFP = NULL;
    }
    args[ArgLanguage].languages = languages;
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

    /*
     * Set the message vector to a success value.
     */
    msgVec[0].msgCode.msgCode = SDL_NORMAL;
    msgVec[1].faoCount = 0;
    msgVec[1].faoInfo = 0;

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
    traceMemory = args[ArgTraceMemory].on;
    trace = args[ArgTrace].on;
    _verbose = args[ArgVerbose].on;

    /*
     * Initialize the parsing states.
     */
    context.state = Initial;
    context.condState.state = sdl_calloc(SDL_K_COND_STATE_SIZE,
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
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {

        /*
         * If we are going to generate an output file for the language, then we
         * have a number of things to do.
         */
        if (languages[ii].langSpec == true)
        {

            if (languages[ii].outFileName == NULL)
            {
                bool addDot = false;

                /*
                 * Go find the last '.' in the file name, this will be where
                 * the file extension starts.
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
                 * If we ended up at the beginning of the file, then there was
                 * no file extension specified.  Use the whole filename.
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
                        sdl_calloc((jj + strlen(_extensions[ii]) + 2), 1);

                /*
                 * Copy the extension for this language after the last '.' (or
                 * the one just added).
                 */
                strncpy(languages[ii].outFileName,
                        args[ArgInputFile].fileName,
                        jj);
                if (addDot == true)
                {
                    languages[ii].outFileName[jj++] = '.';
                }
                strcpy(
                    &languages[ii].outFileName[jj],
                    _extensions[ii]);
            }

            /*
             * Try and open the file for this language.  If it fails, we are
             * done.
             */
            if ((languages[ii].outFP =
                    fopen(
                        languages[ii].outFileName,
                        "w")) == NULL)
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
            else if (args[ArgHeader].on == true)
            {

                /*
                 * OK, we successfully opened this file.  Insert the header
                 * comments.  First starting with a row of '*'s.
                 */
                if ((*_outputFuncs[ii][SDL_K_STARS])(
                        languages[ii].outFP) ==
                            SDL_NORMAL)
                {

                    /*
                     * If we put the row of '*'s in, now put in information
                     * about OpenSDL.
                     */
                    if ((*_outputFuncs[ii][SDL_K_CREATED])(
                            languages[ii].outFP,
                            context.runTimeInfo) == SDL_NORMAL)
                    {
                        struct stat fileStats;

                        context.inputPath = realpath(
                                args[ArgInputFile].fileName,
                                NULL);
                        if (context.inputPath == NULL)
                        {
                            context.inputPath = strdup(
                                    args[ArgInputFile].fileName);
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
                        memcpy(&context.inputTimeInfo,
                               timeInfo,
                               sizeof(struct tm));
                        if ((*_outputFuncs[ii][SDL_K_FILEINFO])(
                                    languages[ii].outFP,
                                    context.inputTimeInfo,
                                    context.inputPath) == SDL_NORMAL)
                        {

                            /*
                             * Finally, if we get here, we just need to output
                             * another row of '*'s, and then can begin our
                             * actual input file parsing and output file
                             * generation.
                             */
                            if ((_outputFuncs[ii][SDL_K_STARS])(
                                    languages[ii].outFP) !=
                                        SDL_NORMAL)
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
        }
        else
        {
            languages[ii].outFP = NULL;
        }
    }
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
     * Go close all the files.
     */
    for (ii = 0; ii < SDL_K_LANG_MAX; ii++)
    {
        if ((languages[ii].langSpec == true) &&
            (languages[ii].outFP != NULL))
        {
            fclose(languages[ii].outFP);
        }
        if (languages[ii].outFileName != NULL)
        {
            sdl_free(languages[ii].outFileName);
        }
    }

    /*
     * Return the results back to the caller.
     */
    if (trace == true)
    {
        fprintf(stderr,
                "'%s' has been processed",
                args[ArgInputFile].fileName);
    }

    /*
     * Clean-up memory.
     */
    for (ii = 0; ii < context.langCondList.listUsed; ii++)
    {
        sdl_free(context.langCondList.lang);
    }
    for (ii = 0; ii < args[ArgSymbols].symbol->listUsed; ii++)
    {
        sdl_free(args[ArgSymbols].symbol->symbols->symbol);
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
        free(context.inputPath);
    }

    /*
     * Return back to the caller.
     */
    return (0);
}
