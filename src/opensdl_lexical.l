%top{
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
 *  along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Description:
 *
 *  This source/header file is generated from the opensdl_lexicals.l file.
 *
 * Revision History:
 *
 *  V01.000	19-SEP-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include <stdio.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include "opensdl_parser.h"
#include "opensdl_actions.h"

extern bool literal_state;

}

%option bison-bridge
%option bison-locations
%option ecs
%option noyywrap
%option reentrant
%option verbose
%option warn
%option debug
%option yylineno
%option case-insensitive
%option 8bit

White_spaces	[:space:]+
Names			[$_[:alpha:]][$_[:alnum:]]*
Local_comment	("{").*
Output_comment	("/*").*
Block_comment	("/+").*("/-")
Quoted_string	\"(\\.|[^"\\])*\"
Literal_string	.*\n
Printable		[:print:]

%x ST_LIT
%x ST_INCL
%x ST_CONST
%x ST_CONST_NAME
%x ST_CONST_LIST
%x ST_CONST_RAD

%{

/*
 * The following definitions are used to maintain various aspects of the
 * parsing being performed.  The first is a Start State stack, which is used
 * to push and pop the Start State whenever it is changed from one to another.
 * The second is a file list, pushed any time an INCLUDE is detected and
 * popped at End-Of-File (EOF).
 *
 * The following structure is used to maintain the parsing information needed
 * when we hit an EOF and need to go back to where we left off in the previous
 * file (if there was one).
 */
typedef struct _file_list_
{
	struct _file_list_	*previous;
	FILE				*fp;
	char				*fileName;
	YY_BUFFER_STATE		bufferState;
	int					lineNumber;
} SDL_FILE_LIST;

static int 				*_sdl_start_state = NULL;
static int				_sdl_state_size = 0;
static int				_sdl_state_inuse = 0;
static SDL_FILE_LIST	*_sdl_file_list = NULL;

/*
 * This is for the currently being parsed file name.  It is used when
 * reporting errors back to the caller.
 */
static char *currentFileName = NULL;

/*
 * Now for some function prototypes.  These functions are defined un the user
 * code definition section.
 */
static bool _sdl_push_file(char *newFileName);
static bool _sdl_pop_file(void);
static void _sdl_str_unquote(char **dest, char *src);
static bool _sdl_push_start_state(int current_state);
static int _sdl_pop_start_state(void);

/*
 * These are external declarations for variables defined outside of this file.
 */
extern void *scanner;
extern _Bool trace;
%}

%%
INCLUDE					{
							if (_sdl_push_start_state(YY_START) == false)
								yyterminate();
							BEGIN(ST_INCL);
						}
<ST_INCL>[^ \t\n\"]+	{
							int c;

							while ((c = input(scanner)) && (c != '\n'))
								;
							yylineno++;
							if (_sdl_push_file(yytext) == 0)
								yyterminate();
							BEGIN(_sdl_pop_start_state());
						}
<ST_INCL>.|\n			{
							fprintf(
								stderr,
								"%4d bad include line\n",
								yylineno);
							yyterminate();
						}
<<EOF>>					{
							if (_sdl_pop_file() == 0)
								yyterminate();
						}
LITERAL[";"]			{
							int c;

							if (_sdl_push_start_state(YY_START) == false)
								yyterminate();
							BEGIN(ST_LIT);

							/*
							 * If the next character is a new-line or return, then eat it.
							 * Otherwise put it back.
							 */
							c = input(scanner);
							if ((c != '\n') && (c != '\r'))
								unput(c);
							return(SDL_K_LITERAL);
						}
<ST_LIT>{Literal_string}	{
							char *ptr, *yycopy;
							int ii;

							yycopy = calloc(1, yyleng);

							/*
							 * Upcase the copy of yytext.
							 */
							for (ii = 0; ii < yyleng; ii++)
								yycopy[ii] = toupper(yytext[ii]);

							/*
							 * Try and locate END_LITERAL in the text.
							 */
							ptr = strstr(yycopy, "END_LITERAL");

							/*
							 * If it is not found, then we are not terminating any thing.  Process
							 * the string as literal text.  Otherwise, there is an END_LITERAL in
							 * the string.
							 */
							if (ptr == NULL)
							{
								free(yycopy);
								yylval->tval = strdup(yytext);
								return(t_literal_string);
							}
							else
							{
								int putBackTo = ptr - yycopy;

								/*
								 * We have 1 of 2 options.  Either END_LITERAL was at byte 0 of
								 * the string or it was not.  If it is at the beginning, unput all
								 * the characters to and including the ;.  Otherwise, put
								 * everything back up to and including END_LITERAL.
								 */
								if (ptr == yycopy)
									putBackTo += 11;

								/*
								 * In order to do this correctly we have to recopy yytext and then
								 * put the copy chracters back.  This preserves the original case.
								 */
								strcpy(yycopy, yytext);
								for (ii = yyleng - 1; ii >= putBackTo; ii--)
									unput(yycopy[ii]);
								free(yycopy);
								BEGIN(_sdl_pop_start_state());
								return(SDL_K_END_LITERAL);
							}
						}
CONSTANT				{
							if (_sdl_push_start_state(YY_START) == false)
								yyterminate();
							printf("%s:%d >>>> START STATE %d --> %d\n",
										__FILE__,
										__LINE__,
										YY_START,
										ST_CONST_NAME);
							BEGIN(ST_CONST_NAME);
							return(SDL_K_CONSTANT);
						}
<ST_CONST_NAME>{Names}	{
							yylval->tval = strdup(yytext);
							printf("%s:%d >>>> START STATE %d <-> %d\n",
										__FILE__,
										__LINE__,
										YY_START,
										ST_CONST);
							BEGIN(ST_CONST);
							return t_constant_name;
						}
<ST_CONST_NAME>[(]		{
							if (_sdl_push_start_state(YY_START) == false)
								yyterminate();
							printf("%s:%d >>>> START STATE %d --> %d\n",
										__FILE__,
										__LINE__,
										YY_START,
										ST_CONST_LIST);
							BEGIN(ST_CONST_LIST);
						}
<ST_CONST_LIST>[^)]*	{
							int c;

							while ((c = input(scanner)) && (c != ')'))
								if (c == '\n')
									yylineno++;
							sdl_trim_str(yytext, SDL_M_COLLAPSE);
							yylval->tval = strdup(yytext);
							BEGIN(_sdl_pop_start_state());
							return(t_constant_names);
						}
<ST_CONST_LIST>[)]		{ BEGIN(_sdl_pop_start_state()); }
<ST_CONST>EQUALS		{
							printf("\n????? AT EQUALS state = %d ?????\n\n", YY_START);
							return(SDL_K_EQUALS);
						}
<ST_CONST>STRING		{ return(SDL_K_STRING); }
<ST_CONST>COUNTER		{ return(SDL_K_COUNTER); }
<ST_CONST>INCREMENT		{ return(SDL_K_INCR); }
<ST_CONST>RADIX			{
							if (_sdl_push_start_state(YY_START) == false)
								yyterminate();
							printf("%s:%d >>>> START STATE %d --> %d\n",
										__FILE__,
										__LINE__,
										YY_START,
										ST_CONST_RAD);
							BEGIN(ST_CONST_RAD);
							return(SDL_K_RADIX);
						}
<ST_CONST_RAD>DEC		{
							BEGIN(_sdl_pop_start_state());
							return(SDL_K_DEC);
						}
<ST_CONST_RAD>HEX		{
							BEGIN(_sdl_pop_start_state());
							return(SDL_K_HEX);
						}
<ST_CONST_RAD>OCT		{
							BEGIN(_sdl_pop_start_state());
							return(SDL_K_OCT);
						}
<ST_CONST>[,]			{
							printf("%s:%d >>>> START STATE %d <-> %d\n",
										__FILE__,
										__LINE__,
										YY_START,
										ST_CONST_NAME);
							BEGIN(ST_CONST_NAME);
							return(SDL_K_COMMA);
						}
<ST_CONST>[;]			{
							BEGIN(_sdl_pop_start_state());
							return(SDL_K_END_CONSTANT);
						}
<ST_CONST>.\n			{ /* Eat unexpected characters */ }
IFLANGUAGE				{ return(SDL_K_IFLANG); }
ELSE					{ return(SDL_K_ELSE); }
END_IFLANGUAGE			{ return(SDL_K_END_IFLANG); }
IFSYMBOL				{ return(SDL_K_IFSYMB); }
ELSE_IFSYMBOL			{ return(SDL_K_ELSE_IFSYMB); }
END_IFSYMBOL			{ return(SDL_K_END_IFSYMB); }

DECLARE					{ return(SDL_K_DECLARE); }
AGGREGATE				{ return(SDL_K_AGGREGATE); }
END						{ return(SDL_K_END); }
END_MODULE				{ return(SDL_K_END_MODULE); }
ENTRY					{ return(SDL_K_ENTRY); }
ITEM					{ return(SDL_K_ITEM); }
MODULE					{ 
							printf("\n\tStart  = Value\n\t-----    -----\n");
							printf("\tST_LIT = %d\n", ST_LIT);
							printf("\tST_INCL = %d\n", ST_INCL);
							printf("\tST_CONST = %d\n", ST_CONST);
							printf("\tST_CONST_NAME = %d\n", ST_CONST_NAME);
							printf("\tST_CONST_LIST = %d\n", ST_CONST_LIST);
							printf("\tST_CONST_RAD = %d\n\n", ST_CONST_RAD);
							return(SDL_K_MODULE);
						}
STRUCTURE				{ return(SDL_K_STRUCTURE); }
UNION					{ return(SDL_K_UNION); }

IDENT					{ return(SDL_K_IDENT); }

ALIGN					{ return(SDL_KWD_ALIGN); }
NOALIGN					{ return(SDL_KWD_NOALIGN); }
BASEALIGN				{ return(SDL_K_BASEALIGN); }
COMMON					{ return(SDL_K_COMMON); }
GLOBAL					{ return(SDL_K_GLOBAL); }
DIMENSION				{ return(SDL_K_DIMENSION); }
<*>PREFIX				{ return(SDL_K_PREFIX); }
<*>TAG					{ return(SDL_K_TAG); }
SIZEOF					{ return(SDL_K_SIZEOF); }

BASED					{ return(SDL_K_BASED); }
TYPEDEF					{ return(SDL_K_TYPEDEF); }
FILL					{ return(SDL_K_FILL); }
MARKER					{ return(SDL_K_MARKER); }
ORIGIN					{ return(SDL_K_ORIGIN); }
<*>TYPENAME				{ return(SDL_K_TYPENAME); }

REFERENCE				{ return(SDL_K_REF); }
VALUE					{ return(SDL_K_VALUE); }
IN						{ return(SDL_K_IN); }
OUT						{ return(SDL_K_OUT); }
DEFAULT					{ return(SDL_K_DEFAULT); }
LIST					{ return(SDL_K_LIST); }
NAMED					{ return(SDL_K_NAMED); }
OPTIONAL				{ return(SDL_K_OPT); }
RETURNS					{ return(SDL_K_RETURNS); }
ALIAS					{ return(SDL_K_ALIAS); }
PARAMETER				{ return(SDL_K_PARAM); }
VARIABLE				{ return(SDL_K_VARIABLE); }

BYTE					{ return(SDL_K_BYTE); }
WORD					{ return(SDL_K_WORD); }
LONGWORD				{ return(SDL_K_LONG); }
QUADWORD				{ return(SDL_K_QUAD); }
OCTAWORD				{ return(SDL_K_OCTA); }
S_FLOAT					{ return(SDL_K_SFLOAT); }
T_FLOAT					{ return(SDL_K_TFLOAT); }
DECIMAL					{ return(SDL_K_DECIMAL); }
BITFIELD				{ return(SDL_K_BITFIELD); }
CHARACTER				{ return(SDL_K_CHAR); }
ADDRESS					{ return(SDL_K_ADDR); }
POINTER					{ return(SDL_K_ADDR); }
POINTER_LONG			{ return(SDL_K_ADDRL); }
POINTER_QUAD			{ return(SDL_K_ADDRQ); }
POINTER_HW				{ return(SDL_K_ADDR_HW); }
HARDWARE_ADDRESS		{ return(SDL_K_ADDR_HW); }
ANY						{ return(SDL_K_ANY); }
BOOLEAN					{ return(SDL_K_BOOL); }

SIGNED					{ return(SDL_K_SIGNED); }
UNSIGNED				{ return(SDL_K_UNSIGNED); }
COMPLEX					{ return(SDL_K_COMPLEX); }
LENGTH					{ return(SDL_K_LENGTH); }
MASK					{ return(SDL_K_MASK); }
VARYING					{ return(SDL_K_VARY); }
PRECISION				{ return(SDL_K_PRECISION); }

";"						{ return(SDL_K_SEMI); }
<*>"("					{ return(SDL_K_OPENP); }
<*>")"					{ return(SDL_K_CLOSEP); }
","						{ return(SDL_K_COMMA); }
":"						{ return(SDL_K_COLON); }

<*>{Quoted_string}		{
							_sdl_str_unquote(&yylval->tval, yytext);\
							return(t_string);
						}
<*>{Names}				{
							yylval->tval = strdup(yytext);
							return t_name;
						}
<*>"#"{Names}			{

							yylval->tval = strdup(yytext);
							return(t_variable);
						}
{Output_comment}		{
							yylval->tval = strdup(yytext);
							return(t_line_comment);
						}
{Block_comment}			{
							yylval->tval = strdup(yytext);
							return(t_block_comment);
						}
{Local_comment}			{ /* eat local comments... */ }
<*>{White_spaces}		{ /* eat white spaces... */ }
<*>"%"D[0-9]{1,20}		{
							yylval->ival = strtol(&yytext[2], NULL, 10);
							return(v_int);
						}
<*>([0])|([1-9][0-9]{0,19})	{
							yylval->ival = strtol(yytext, NULL, 10);
							printf("\n????????  Here 1 (%ld) state = %d ????????\n", yylval->ival, YY_START);
							return(v_int);
						}
<*>"%"X[0-9a-fA-F]{1,16}	{
							yylval->tval = strdup(&yytext[2]);
							return(t_hex);
						}
<*>[0]X[0-9a-fA-F]{1,16}	{
							yylval->tval = strdup(&yytext[2]);
							return(t_hex);
						}
<*>"%"O[0-7]{1,22}		{
							yylval->tval = strdup(&yytext[2]);
							return(t_octal);
						}
<*>[0][0-7]{1,22}		{
							yylval->tval = strdup(&yytext[1]);
							return(t_octal);
						}
<*>"%"B[0-1]{1,64}		{
							yylval->tval = strdup(&yytext[2]);
							return(t_binary);
						}
<*>"%"A{Printable}		{
							yylval->tval = strdup(&yytext[2]);
							return(t_ascii);
						}
<*>"%"A[0-9]{3}			{
							yylval->ival = strtol(&yytext[2], NULL, 10);
							return(v_int);
						}
<*>"%"A[0-9a-fA-F]{2}	{
							yylval->tval = strdup(&yytext[2]);
							return(t_hex);
						}

%%

/*
 * _sdl_push_file
 *  This function is called when an include statement has been parsed and the
 *  include filename extracted from the input stream.  This function will push
 *  the current file parsing state onto a list, open the new file and set it
 *  up to be parsed.
 *
 * Input Parameters:
 *  newFileName:
 *		A pointer to a string specifying the next file to be opened for
 *		parsing.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	Normal Successful Completion.
 *	false:	An error occurred.
 */
#define yyg ((struct yyguts_t *) scanner)
static bool _sdl_push_file(char *newFileName)
{
	FILE			*fp = fopen(newFileName, "r");
	SDL_FILE_LIST	*entry = calloc(1, sizeof(SDL_FILE_LIST));
	bool			retVal = true;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%s:%d:_sdl_push_file\n", __FILE__, __LINE__);

	/*
	 * Get out if there is no file or memory allocation did not allocate
	 * anything.
	 */
	if (fp == NULL)
	{
		perror(newFileName);
		retVal = false;
	}
 	if ((entry == NULL) && (retVal == true))
 	{
 		perror("malloc");
 		exit(1);
 	}

	/*
	 * OK, if we get here and we still have a success, go initialize the new
	 * current entry.
	 */
	if (retVal == true)
	{

	 	/*
	 	 * Remember the line number we left off of in the current file.
	 	 */
		if (_sdl_file_list != NULL)
			_sdl_file_list->lineNumber = yylineno;

		/*
		 * Insert this entry onto the beginning of the file list.
		 */
		entry->previous = _sdl_file_list;
		_sdl_file_list = entry;

		/*
		 * Set up the new current entry
		 */
		entry->bufferState = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
		entry->fp = fp;
		entry->fileName = strdup(newFileName);
		yy_switch_to_buffer(entry->bufferState, scanner);
		yylineno = 1;
		currentFileName = newFileName;
	}

	/*
	 * Return the result back to the caller
	 */
	return(retVal);
}

/*
 * _sdl_pop_file
 *  This function is called when an EOF has been parsed from the input stream.
 *  This function will pop the previous file off of the list, and set it up so
 *  that it can continue to be parsed.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	Normal Successful Completion.
 *	false:	EOF on the top file was detected.
 */
static bool _sdl_pop_file(void)
{
	SDL_FILE_LIST	*entry = _sdl_file_list;
	SDL_FILE_LIST	*prev;
	bool			retVal = true;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%s:%d:_sdl_pop_file\n", __FILE__, __LINE__);

	/*
	 * If there is nothing to free, we return an error.
	 */
	if (entry == NULL)
		retVal = false;
	else
	{

		/*
		 * Get rid of current entry and all its associated memory.
		 */
		fclose(entry->fp);
		yy_delete_buffer(entry->bufferState, scanner);

		/*
		 * Before we free up the current file entry, get the pointer to the
		 * previous one, then free up current one.
		 */
		prev = entry->previous;
		free(entry->fileName);
		free(entry);

		/*
		 * If there is no previous entry, then return an error.  Otherwise,
		 * make the previous one the current one.
		 */
		if(prev == NULL)
			retVal = false;
		else
		{
			yy_switch_to_buffer(prev->bufferState, scanner);
			_sdl_file_list = prev;
			yylineno = prev->lineNumber;
			currentFileName = prev->fileName;
		}
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * _sdl_str_unquote
 *  This function is called to remove the leading an trailing double quote
 *  characters from a string.  This is very simplified, in that the
 *  parsing will place the double quotes as the first and last characters
 *  in the string.
 *
 * Input Parameters:
 *  src:
 *		A pointer to the string to have its leading and trailing double quotes
 *		removed.
 *  maxLen:
 *		A value indicating the maximum possible length for the string.
 *
 * Output Parameters:
 *  dest:
 *		A pointer to the address to receive the allocated buffer.
 *
 * Return Values:
 *  None.
 */
static void _sdl_str_unquote(char **dest, char *src)
{
	char	*ptr;

	/*
	 * Copy everything but the first character.
	 */
	*dest = strdup(&src[1]);

	/*
	 * Now find the last double-quote character.
	 */
	ptr = *dest + strlen(*dest);
	while ((*ptr != '\"') && (ptr != *dest))
		ptr--;

	/*
	 * If we found the last double-quote character, then convert it to a null
	 * character.
	 */
	if (*ptr == '\"')
		*ptr = '\0';

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * _sdl_push_start_state
 *  This function is called to push the current start state onto a stack
 *  because we are changing start states and want to come back to the current
 *  one.
 *
 * Input Parameters:
 *  current_state:
 *		A value indicating the current start state to be pushed onto the
 *      stack.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	Normal Successful Completion.
 *  false:	Failed to allocate enough memory to save the state.
 */
static bool _sdl_push_start_state(int current_state)
{
	bool	retVal = true;
	int		index = _sdl_state_size - _sdl_state_inuse - 1;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf(
			"%s:%d:_sdl_push_start_state(%d)\n",
			__FILE__,
			__LINE__,
			current_state);

	/*
	 * If the index is negative, there is no more room on the stack.  Allocate
	 * a new stack, copy the items below, making room for a new entry at the
	 * top, free the old stack, and finally make the new stack the current
	 * one.
	 */
	if (index < 0)
	{
		int	*newStack = calloc((_sdl_state_size + 1), sizeof(int));

		if (newStack != NULL)
		{
			int	ii;

			_sdl_state_size++;
			for (ii = 1; ii < _sdl_state_size; ii++)
				newStack[ii] = _sdl_start_state[ii - 1];
			free(_sdl_start_state);
			_sdl_start_state = newStack;
			index = 0;
		}
		else
		{
	 		perror("calloc");
			retVal = false;
 		}
	}
	if (retVal == true)
	{
		_sdl_start_state[index] = current_state;
		_sdl_state_inuse++;
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * _sdl_pop_start_state
 *  This function is called to pop the previous start state off of a stack
 *  because we are done with the current start state and need to go back to
 *  the previous one.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  0:		The INITIAL state (returned when there is nothing on the stack).
 *  >=0:	The previous start state (which may be INITIAL).
 */
static int _sdl_pop_start_state(void)
{
	int		retVal = 0;
	int		index = _sdl_state_size - _sdl_state_inuse;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%s:%d:_sdl_pop_start_state(", __FILE__, __LINE__);

	/*
	 * If any of the entries are in-use, then return the top one and
	 * decrement the in-use counter;
	 */
	if (_sdl_state_inuse > 0)
	{
		retVal = _sdl_start_state[index];
		_sdl_state_inuse--;
	}

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%d) from %d\n", retVal, YY_START);

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}
