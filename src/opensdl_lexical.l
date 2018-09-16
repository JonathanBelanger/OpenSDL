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
 *  V01.000	Aug 23, 2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	Sep  6, 2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.  Also, no need for two
 *  copyright sections (one should be sufficient, even for the generated file).
 */
#include <stdio.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include "opensdl_parser.h"

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

white_spaces	[ \t\n\r\v\f]+
names			[$_a-zA-Z][$_a-zA-Z0-9]*
local_comment	("{").*
output_comment	("/*"|"/+"|"//"|"/-").*
quoted_string	\"(\\.|[^"\\])*\"
literal_string	.*\n
printable		[[:print:]]

%x LITERAL
%x INCLUDE

%{
typedef struct _bs_
{
	struct _bs_		*prev;
	YY_BUFFER_STATE	bufState;
	int				lineno;
	char			*filename;
	FILE			*fp;
} SDL_BUFFER_STACK;

SDL_BUFFER_STACK *curbs = NULL;
char *curfilename;

int newfile(char *fn);
int popfile(void);
void str_unquote(char *, char *, size_t);

extern void *scanner;
extern _Bool trace;
%}

%%
<INITIAL>[iI][nN][cC][lL][uU][dD][eE]					{ BEGIN(INCLUDE); }
<INCLUDE>[^ \t\n\"]+									{
															int c;

															while ((c = input(scanner)) && (c != '\n'))
																;
															yylineno++;
															if (newfile(yytext) == 0)
																yyterminate();
															BEGIN(INITIAL);
														}
<INCLUDE>.|\n											{ fprintf(stderr, "%4d bad include line\n", yylineno); yyterminate(); }
<INITIAL><<EOF>>											{
															if (popfile() == 0)
																yyterminate();
														}
<INITIAL>[lL][iI][tT][eE][rR][aA][lL][";"]				{
															int c;

															BEGIN(LITERAL);

															/*
															 * If the next character is a new-line or return, then eat it.
															 * Otherwise put it back.
															 */
															c = input(scanner);
															if ((c != '\n') && (c != '\r'))
																unput(c);
															return(SDL_K_LITERAL);
														}
<LITERAL>{literal_string}								{
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
																strncpy(yylval->tval, yytext, sizeof(yylval->tval));
																return lit_val;
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
																BEGIN(INITIAL);
																return(SDL_K_END_LITERAL);
															}
														}

<INITIAL>[iI][fF][lL][aA][nN][gG][uU][aA][gG][eE]		{ return(SDL_K_IFLANG); }
<INITIAL>[eE][lL][sS][eE]								{ return(SDL_K_ELSE); }
<INITIAL>[eE][nN][dD]_[iI][fF][lL][aA][nN][gG][uU][aA][gG][eE]	{ return(SDL_K_END_IFLANG); }
<INITIAL>[iI][fF][sS][yY][mM][bB][oO][lL]				{ return(SDL_K_IFSYMB); }
<INITIAL>[eE][lL][sS][eE]_[iI][fF][sS][yY][mM][bB][oO][lL]	{ return(SDL_K_ELSE_IFSYMB); }
<INITIAL>[eE][nN][dD]_[iI][fF][sS][yY][mM][bB][oO][lL]	{ return(SDL_K_END_IFSYMB); }

<INITIAL>[dD][eE][cC][lL][aA][rR][eE]					{ return(SDL_K_DECLARE); }
<INITIAL>[aA][gG][gG][rR][eE][gG][aA][tT][eE]			{ return(SDL_K_AGGREGATE); }
<INITIAL>[cC][oO][nN][sS][tT][aA][nN][tT]				{ return(SDL_K_CONSTANT); }
<INITIAL>[eE][nN][dD]									{ return(SDL_K_END); }
<INITIAL>[eE][nN][dD]_[mM][oO][dD][uU][lL][eE]			{ return(SDL_K_END_MODULE); }
<INITIAL>[eE][nN][tT][rR][yY]							{ return(SDL_K_ENTRY); }
<INITIAL>[iI][tT][eE][mM]								{ return(SDL_K_ITEM); }
<INITIAL>[mM][oO][dD][uU][lL][eE]						{ return(SDL_K_MODULE); }
<INITIAL>[sS][tT][rR][uU][cC][tT][uU][rR][eE]			{ return(SDL_K_STRUCTURE); }
<INITIAL>[uU][nN][iI][oO][nN]							{ return(SDL_K_UNION); }

<INITIAL>[iI][dD][eE][nN][tT]							{ return(SDL_K_IDENT); }

<INITIAL>[aA][lL][iI][gG][nN]							{ return(SDL_KWD_ALIGN); }
<INITIAL>[nN][oO][aA][lL][iI][gG][nN]					{ return(SDL_KWD_NOALIGN); }
<INITIAL>[bB][aA][sS][eE][aA][lL][iI][gG][nN]			{ return(SDL_K_BASEALIGN); }
<INITIAL>[cC][oO][mM][mM][oO][nN]						{ return(SDL_K_COMMON); }
<INITIAL>[gG][lL][oO][bB][aA][lL]						{ return(SDL_K_GLOBAL); }
<INITIAL>[dD][iI][mM][eE][nN][sS][iI][oO][nN]			{ return(SDL_K_DIMENSION); }
<INITIAL>[pP][rR][eE][fF][iI][xX]						{ return(SDL_K_PREFIX); }
<INITIAL>[tT][aA][gG]									{ return(SDL_K_TAG); }
<INITIAL>[sS][iI][zZ][eE][oO][fF]						{ return(SDL_K_SIZEOF); }

<INITIAL>[bB][aA][sS][eE][dD]							{ return(SDL_K_BASED); }
<INITIAL>[tT][yY][pP][eE][dD][eE][fF]					{ return(SDL_K_TYPEDEF); }
<INITIAL>[fF][iI][lL][lL]								{ return(SDL_K_FILL); }
<INITIAL>[mM][aA][rR][kK][eE][rR]						{ return(SDL_K_MARKER); }
<INITIAL>[oO][rR][iI][gG][iI][nN]						{ return(SDL_K_ORIGIN); }
<INITIAL>[cC][oO][uU][nN][tT][eE][rR]					{ return(SDL_K_COUNTER); }
<INITIAL>[eE][qQ][uU][aA][lL][sS]						{ return(SDL_K_EQUALS); }
<INITIAL>[sS][tT][rR][iI][nN][gG]						{ return(SDL_K_STRING); }
<INITIAL>[iI][nN][cC][rR][eE][mM][eE][nN][tT]			{ return(SDL_K_INCR); }
<INITIAL>[tT][yY][pP][eE][nN][aA][mM][eE]				{ return(SDL_K_TYPENAME); }

<INITIAL>[dD][eE][sS][cC][rR][iI][pP][tT][oO][rR]		{ return(SDL_K_DSC); }
<INITIAL>[rR][eE][fF][eE][rR][eE][nN][cC][eE]			{ return(SDL_K_REF); }
<INITIAL>[rR][tT][lL]_[sS][tT][rR]_[dD][eE][sS][cC]		{ return(SDL_K_STR_DSC); }
<INITIAL>[vV][aA][lL][uU][eE]							{ return(SDL_K_VALUE); }
<INITIAL>[iI][nN]										{ return(SDL_K_IN); }
<INITIAL>[oO][uU][tT]									{ return(SDL_K_OUT); }
<INITIAL>[dD][eE][fF][aA][uU][lL][tT]					{ return(SDL_K_DEFAULT); }
<INITIAL>[lL][iI][sS][tT]								{ return(SDL_K_LIST); }
<INITIAL>[nN][aA][mM][eE][dD]							{ return(SDL_K_NAMED); }
<INITIAL>[oO][pP][tT][iI][oO][nN][aA][lL]				{ return(SDL_K_OPT); }
<INITIAL>[rR][eE][tT][uU][rR][nN][sS]					{ return(SDL_K_RETURNS); }
<INITIAL>[aA][lL][iI][aA][sS]							{ return(SDL_K_ALIAS); }
<INITIAL>[pP][aA][rR][aA][mM][eE][tT][eE][rR]			{ return(SDL_K_PARAM); }
<INITIAL>[vV][aA][rR][iI][aA][bB][lL][eE]				{ return(SDL_K_VARIABLE); }

<INITIAL>[bB][yY][tT][eE]								{ return(SDL_K_BYTE); }
<INITIAL>[wW][oO][rR][dD]								{ return(SDL_K_WORD); }
<INITIAL>[lL][oO][nN][gG][wW][oO][rR][dD]				{ return(SDL_K_LONG); }
<INITIAL>[qQ][uU][aA][dD][wW][oO][rR][dD]				{ return(SDL_K_QUAD); }
<INITIAL>[oO][cC][tT][aA][wW][oO][rR][dD]				{ return(SDL_K_OCTA); }
<INITIAL>[sS]_[fF][lL][oO][aA][tT]						{ return(SDL_K_SFLOAT); }
<INITIAL>[tT]_[fF][lL][oO][aA][tT]						{ return(SDL_K_TFLOAT); }
<INITIAL>[dD][eE][cC][iI][mM][aA][lL]					{ return(SDL_K_DECIMAL); }
<INITIAL>[bB][iI][tT][fF]i[eE][lL][dD]					{ return(SDL_K_BITFIELD); }
<INITIAL>[cC][hH][aA][rR][aA][cC][tT][eE][rR]			{ return(SDL_K_CHAR); }
<INITIAL>[aA][dD][dD][rR][eE][sS][sS]					{ return(SDL_K_ADDR); }
<INITIAL>[pP][oO][iI][nN][tT][eE][rR]					{ return(SDL_K_ADDR); }
<INITIAL>[pP][oO][iI][nN][tT][eE][rR]_[lL][oO][nN][gG]	{ return(SDL_K_ADDRL); }
<INITIAL>[pP][oO][iI][nN][tT][eE][rR]_[qQ][uU][aA][dD]	{ return(SDL_K_ADDRQ); }
<INITIAL>[pP][oO][iI][nN][tT][eE][rR]_[hH][wW]			{ return(SDL_K_ADDR_HW); }
<INITIAL>[hH][aA][rR][dD][wW][aA][rR][eE]_[aA][dD][dD][rR][eE][sS][sS]	{ return(SDL_K_ADDR_HW); }
<INITIAL>[aA][nN][yY]									{ return(SDL_K_ANY); }
<INITIAL>[bB][oO][oO][lL][eE][aA][nN]					{ return(SDL_K_BOOL); }

<INITIAL>[sS][iI][gG][nN][eE][dD]						{ return(SDL_K_SIGNED); }
<INITIAL>[uU][nN][sS][iI][gG][nN][eE][dD]				{ return(SDL_K_UNSIGNED); }
<INITIAL>[cC][oO][mM][pP][lL][eE][xX]					{ return(SDL_K_COMPLEX); }
<INITIAL>[lL][eE][nN][gG][tT][hH]						{ return(SDL_K_LENGTH); }
<INITIAL>[mM][aA][sS][kK]								{ return(SDL_K_MASK); }
<INITIAL>[vV][aA][rR][yY][iI][nN][gG]					{ return(SDL_K_VARY); }
<INITIAL>[pP][rR][eE][cC][iI][sS]i[oO][nN]				{ return(SDL_K_PRECISION); }

<INITIAL>"+"											{ return(SDL_K_PLUS); }
<INITIAL>"-"											{ return(SDL_K_MINUS); }
<INITIAL>"*"											{ return(SDL_K_MULT); }
<INITIAL>"/"											{ return(SDL_K_DIVIDE); }
<INITIAL>"@"											{ return(SDL_K_SHIFT); }
<INITIAL>"&"											{ return(SDL_K_AND); }
<INITIAL>"!"											{ return(SDL_K_OR); }
<INITIAL>"~"											{ return(SDL_K_NOT); }
<INITIAL>"."											{ return(SDL_K_PERIOD); }
<INITIAL>"^"											{ return(SDL_K_BITS); }
<INITIAL>"="											{ return(SDL_K_EQ); }

<INITIAL>";"											{ return(SDL_K_EOD); }
<INITIAL>"("											{ return(SDL_K_OPEN); }
<INITIAL>")"											{ return(SDL_K_CLOSE); }
<INITIAL>","											{ return(SDL_K_COMMA); }
<INITIAL>":"											{ return(SDL_K_COLON); }

<INITIAL>[rR][aA][dD][iI][xX]							{ return(SDL_K_RADIX); }
<INITIAL>[dD][eE][cC]									{ return(SDL_K_DEC); }
<INITIAL>[hH][eE][xX]									{ return(SDL_K_HEX); }
<INITIAL>[oO][cC][tT]									{ return(SDL_K_OCT); }

<INITIAL>{output_comment}			{ strncpy(yylval->tval, yytext, sizeof(yylval->tval)); return comment; }
<INITIAL>{quoted_string}			{ str_unquote(yylval->tval, yytext, sizeof(yylval->tval)); return string; }
<INITIAL>{names}					{ strncpy(yylval->tval, yytext, sizeof(yylval->tval)); return ident; }
<INITIAL>"#"{names}					{ strncpy(yylval->tval, yytext, sizeof(yylval->tval)); return variable; }
<INITIAL>{local_comment}			{ /* eat local comments... */ }
<INITIAL>{white_spaces}				{ /* eat white spaces... */ }
<INITIAL>"%"[dD][0-9]{1,20}			{ yylval->bval = strtol(&yytext[2], NULL, 10); return decimal; }
<INITIAL>([0])|([1-9][0-9]{0,19})	{ yylval->bval = strtol(yytext, NULL, 10); return decimal; }
<INITIAL>"%"[xX][0-9a-fA-F]{1,16}	{ strcpy(yylval->tval, &yytext[2]); return hexadecimal; }
<INITIAL>[0][xX][0-9a-fA-F]{1,16}	{ strcpy(yylval->tval, &yytext[2]); return hexadecimal; }
<INITIAL>"%"[oO][0-7]{1,22}			{ strcpy(yylval->tval, &yytext[2]); return octal; }
<INITIAL>[0][0-7]{1,22}				{ strcpy(yylval->tval, &yytext[1]); return octal; }
<INITIAL>"%"[bB][0-1]{1,64}			{ strcpy(yylval->tval, &yytext[2]); return binary; }
<INITIAL>"%"[aA]{printable}			{ strcpy(yylval->tval, &yytext[2]); return ascii; }
<INITIAL>"%"[aA][0-9]{3}			{ yylval->bval = strtol(&yytext[2], NULL, 10); return decimal; }
<INITIAL>"%"[aA][0-9a-fA-F]{2}		{ strcpy(yylval->tval, &yytext[2]); return hexadecimal; }

%%

#define yyg ((struct yyguts_t *) scanner)

int newfile(char *fn)
{
	FILE				*fp = fopen(fn, "r");
	SDL_BUFFER_STACK	*bs = malloc(sizeof(SDL_BUFFER_STACK));
	int					retVal = 1;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%s:%d:newfile\n", __FILE__, __LINE__);

	/*
	 * Get out if there is no file or memory allocation did not allocate
	 * anything.
	 */
	if (fp == NULL)
	{
		perror(fn);
		retVal = 0;
	}
 	if ((bs == NULL) && (retVal == 1))
 	{
 		perror("malloc");
 		exit(1);
 	}

	/*
	 * OK, if we get here and we still have a success, go initialize the new
	 * current entry.
	 */
	if (retVal == 1)
	{

	 	/*
	 	 * Remember the state
	 	 */
		if (curbs != NULL)
			curbs->lineno = yylineno;
		bs->prev = curbs;

		/*
		 * Set up the new current entry
		 */
		bs->bufState = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
		bs->fp = fp;
		bs->filename = fn;
		yy_switch_to_buffer(bs->bufState, scanner);
		curbs = bs;
		yylineno = 1;
		curfilename = fn;
	}

	/*
	 * Return the result back to the caller
	 */
	return(1);
}

int popfile(void)
{
	SDL_BUFFER_STACK	*bs = curbs;
	SDL_BUFFER_STACK	*prev;
	int					retVal = 1;

	/*
	 * If tracing is turned on, write out this call (calls only, no returns).
	 */
	if (trace == true)
		printf("%s:%d:popfile\n", __FILE__, __LINE__);

	/*
	 * If there is nothing to free, we return an error.
	 */
	if (bs == NULL)
		retVal = 0;
	else
	{

		/*
		 * Get rid of current entry and all its associated memory.
		 */
		fclose(bs->fp);
		yy_delete_buffer(bs->bufState, scanner);

		/*
		 * Before we free up the current bufstack, get the pointer to the
		 * previous one, then free up current one.
		 */
		prev = bs->prev;
		free(bs);

		/*
		 * If there is no previous bufstack, then return the error.  Otherwise,
		 * make the previous one the current one.
		 */
		if(prev == NULL)
			retVal = 0;
		else
		{
			yy_switch_to_buffer(prev->bufState, scanner);
			curbs = prev;
			yylineno = curbs->lineno;
			curfilename = curbs->filename;
		}
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

void str_unquote(char *dest, char *src, size_t maxLen)
{
	char	*ptr;

	/*
	 * Copy everything but the first character.
	 */
	strncpy(dest, &src[1], maxLen);

	/*
	 * Now find the last double-quote character.
	 */
	ptr = dest + strlen(dest);
	while ((*ptr != '\"') && (ptr != dest))
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