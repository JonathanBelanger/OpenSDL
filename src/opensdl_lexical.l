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

}

%option bison-bridge
%option bison-locations
%option ecs
%option noyywrap
%option reentrant
%option verbose
%option warn
%option yylineno
%option case-insensitive
%option 8bit
%option debug
%option perf-report

Names			[$_[:alpha:]][$_[:alnum:]]*
Local_comment	("{").*
Output_comment	("/*").*
Block_comment	("/+")(.|\n)*("/-")
Quoted_string	\"(\\.|[^"\\])*\"

%x ST_LIT
%x ST_INCL
%x ST_CONST
%x ST_CONST_NAME
%x ST_CONST_LIST
%x ST_CONST_RAD
%x ST_BLOCK

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
static bool _sdl_push_start_state(void);
static int _sdl_pop_start_state(void);

/*
 * These are external declarations for variables defined outside of this file.
 */
extern void *scanner;
extern _Bool trace;
%}

%%
INCLUDE					{
							if (_sdl_push_start_state() == false)
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
<*><<EOF>>				{
							if (_sdl_pop_file() == 0)
								yyterminate();
						}
LITERAL([:space:]*)[;]	{
							if (_sdl_push_start_state() == false)
								yyterminate();
							BEGIN(ST_LIT);
							return(SDL_K_LITERAL);
						}
<ST_LIT>.*\n			{
							int		len = yyleng;
							char	*ptr, *yycopy;
							int		retVal = t_literal_string;

							/*
							 * Make a duplication of yytext.
							 */
							yycopy = strdup(yytext);

							/*
							 * Upcase the entire string.
							 */
							for (ptr = yycopy; *ptr != '\0'; ptr++);
								*ptr = toupper(*ptr);

							/*
							 * Look for the uppercase END_LITERAL in the copy
							 * of yytext.
							 */
							ptr = strstr(yycopy, "END_LITERAL");

							/*
							 * Restore the copy of the copy of yytext to its
							 * case-original version.
							 */
							strcpy(yycopy, yytext);

							/*
							 * OK, if we found END_LITERAL, then we have some
							 * more processing to perform.
							 */
							if (ptr != NULL)
							{

								/*
								 * If END_LITERAL is at the very start of
								 * yytext, then we can swallow it and indicate
								 * that we are at the end of the literal
								 * section.
								 */
								if (ptr == yycopy)
								{

									/*
									 * TODO: We really need to allow for the
									 * situaltion where END_LITERAL does not
									 * have the ';' immediately after it.
									 * There may be some spaces between the end
									 * of END_LITERAL and the ';', which should
									 * be acceptable.  We may also what to
									 * consider where the ';' is on one or more
									 * lines after the line where the
									 * END_LITERAL is located.
									 */
									ptr += 12;
									BEGIN(_sdl_pop_start_state());
									retVal = SDL_K_END_LITERAL;
								}

								/*
								 * Unput everything, in reverse order, that
								 * remains in the buffer.  This will allow this
								 * data to be processed as part of the rest of
								 * the file.
								 */
								len--;
								while (&yycopy[len] != (ptr - 1))
									unput(yycopy[len--]);

								/*
								 * Move the null-termination to the point where
								 * the last unput byte was located.
								 */
								yycopy[len + 1] = '\0';
							}

							/*
							 * If we are at the end of the LITERAL section,
							 * then we do not need the copy of yytext anymore.
							 * Otherwise, store the copy of yytext to where
							 * Bison can find it.
							 */
							if (retVal == SDL_K_END_LITERAL)
								free(yycopy);
							else
								yylval->tval = yycopy;
							return(retVal);
						}
CONSTANT				{

							/*
							 * Push the current state onto the state stack.
							 */
							if (_sdl_push_start_state() == false)
								yyterminate();

							/*
							 * Change the start state to one specific for
							 * parsing the 2 forms of constant-names.
							 */
							BEGIN(ST_CONST_NAME);

							/*
							 * Return the value indicating that a CONSTANT
							 * declaration has started.
							 */
							return(SDL_K_CONSTANT);
						}
<ST_CONST_NAME>{Names}	{

							/*
							 * We have just a single name, so just duplicate
							 * it.
							 */
							yylval->tval = strdup(yytext);

							/*
							 * Change the start state to one for parsing the
							 * remaining CONSTANT declaration.  Note, we do not
							 * push the previous state onto the stack, we just
							 * change it.
							 */
							BEGIN(ST_CONST);

							/*
							 * Return the value indicating that we have a
							 * single constant-name.
							 */
							return t_constant_name;
						}
<ST_CONST_NAME>[(]		{

							/*
							 * We have just a multiple names being defined.
							 * Change the start state to one for parsing the
							 * parenthesized and comma separated list of
							 * constant-names.  Note, we do not push the
							 * previous state onto the stack, we just change
							 * it.
							 */
							BEGIN(ST_CONST_LIST);
						}
<ST_CONST_LIST>[^)]*	{
							int c;

							/*
							 * OK, we got everything up to, but not including,
							 * the close parenthesis.
							 */
							while ((c = input(scanner)) && (c != ')'))
								if (c == '\n')
									yylineno++;

							/*
							 * Trim leading and trailing spaces, compress
							 * multiple space characters to a single one, and
							 * convert control characters to a space character.
							 */
							sdl_trim_str(
								yytext,
								(SDL_M_CONVERT | SDL_M_TRIM | SDL_M_COMPRESS));

							/*
							 * Make a copy of the name string.
							 */
							yylval->tval = strdup(yytext);

							/*
							 * Put the close parenthesis back onto the input
							 * stream.
							 */
							unput(c);

							/*
							 * Return the value indicating that we have
							 * multiple constant-names.  Note, the open and
							 * close parentheses are not included.
							 */
							return(t_constant_names);
						}
<ST_CONST_LIST>[)]		{ 

							/*
							 * Change the start state to one for parsing the
							 * remaining CONSTANT declaration.  Note, we do not
							 * push the previous state onto the stack, we just
							 * change it.
							 */
							BEGIN(ST_CONST);
						}
<ST_CONST>EQUALS		{ return(SDL_K_EQUALS); }
<ST_CONST>STRING		{ return(SDL_K_STRING); }
<ST_CONST>COUNTER		{ return(SDL_K_COUNTER); }
<ST_CONST>INCREMENT		{ return(SDL_K_INCR); }
<ST_CONST>RADIX			{
							if (_sdl_push_start_state() == false)
								yyterminate();
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
MODULE					{ return(SDL_K_MODULE); }
STRUCTURE				{ return(SDL_K_STRUCTURE); }
UNION					{ return(SDL_K_UNION); }

IDENT					{ return(SDL_K_IDENT); }

ALIGN					{ return(SDL_KWD_ALIGN); }
NOALIGN					{ return(SDL_KWD_NOALIGN); }
BASEALIGN				{ return(SDL_K_BASEALIGN); }
COMMON					{ return(SDL_K_COMMON); }
GLOBAL					{ return(SDL_K_GLOBAL); }
DIMENSION				{ return(SDL_K_DIMENSION); }
SIZEOF					{ return(SDL_K_SIZEOF); }

BASED					{ return(SDL_K_BASED); }
TYPEDEF					{ return(SDL_K_TYPEDEF); }
FILL					{ return(SDL_K_FILL); }
MARKER					{ return(SDL_K_MARKER); }
ORIGIN					{ return(SDL_K_ORIGIN); }
<*>PREFIX				{ return(SDL_K_PREFIX); }
<*>TAG					{ return(SDL_K_TAG); }
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

<*>"("					{ return(SDL_K_OPENP); }
<*>")"					{ return(SDL_K_CLOSEP); }
<*>"="					{ return(SDL_K_EQ); }
<*>"+"					{ return(SDL_K_PLUS); }
<*>"-"					{ return(SDL_K_MINUS); }
<*>"*"					{ return(SDL_K_MULT); }
<*>"/"					{ return(SDL_K_DIVIDE); }
<*>"&"					{ return(SDL_K_AND); }
<*>"!"					{ return(SDL_K_OR); }
<*>"@"					{ return(SDL_K_AT); }
<*>"~"					{ return(SDL_K_NOT); }
<*>"."					{ return(SDL_K_DOT); }
<*>":"					{ return(SDL_K_FULL); }
<*>"^"					{ return(SDL_K_CARAT); }
";"						{ return(SDL_K_SEMI); }
","						{ return(SDL_K_COMMA); }

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
							char	*ptr, *yycopy;
							int		ii;

							yycopy = strdup(yytext);

							/*
							 * Try and locate "/-" in the text.
							 */
							ptr = strstr(yycopy, "/-");
							
							/*
							 * If we didn't find the "/-" in the string, the we
							 * must have hit the EOF.  For now, we'll do
							 * nothing.  We may have to come back and revisit
							 * this.
							 */
							if (ptr != NULL)
							{
								int putBackTo = ptr - yycopy + 2;
								int len = yyleng;

								/*
								 * Return each of the characters back
								 */
								for (ii = len - 1; ii >= putBackTo; ii--)
									unput(yycopy[ii]);

								/*
								 * Add a null character to just after the "/-".
								 */
								ptr[2] = '\0';

								/*
								 * No need to duplicate the string, we already
								 * did.
								 */
								yylval->tval = yycopy;
							}
							return(t_block_comment);
						}
{Local_comment}			{ /* eat local comments... */ }
<*>[:space:]+			{ /* eat white spaces... */ }
<*>"%"D[0-9]{1,20}		{
							yylval->ival = strtol(&yytext[2], NULL, 10);
							return(v_int);
						}
<*>([0])|([1-9][0-9]{0,19})	{
							yylval->ival = strtol(yytext, NULL, 10);
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
<*>"%"A[\x20-\x7E]		{
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
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	Normal Successful Completion.
 *  false:	Failed to allocate enough memory to save the state.
 */
static bool _sdl_push_start_state(void)
{
	bool	retVal = true;
	int		index = _sdl_state_size - _sdl_state_inuse - 1;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%s:%d:_sdl_push_start_state\n", __FILE__, __LINE__);

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
		_sdl_start_state[index] = YY_START;
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
		printf("%s:%d:_sdl_pop_start_state\n", __FILE__, __LINE__);

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
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}
