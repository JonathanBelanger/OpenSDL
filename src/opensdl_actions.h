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
 *  This header file contains the prototypes for the action routines called
 *  during the parsing of the input file..
 *
 * Revision History:
 *
 *  V01.000	28-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	06-Sep-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
#ifndef _OPENSDL_ACTIONS_H_
#define _OPENSDL_ACTIONS_H_	1

/*
 * Parsing functions.
 */
int sdl_set_local(SDL_CONTEXT *context, char *name, int64_t value, int srcLineNo);
int sdl_comment_line(SDL_CONTEXT *context, char *comment);
int sdl_comment_block(SDL_CONTEXT *context, char *comment);
int sdl_module(SDL_CONTEXT *context, char *moduleName, char *identName);
int sdl_module_end(SDL_CONTEXT *context, char *moduleName);
int sdl_literal(SDL_QUEUE *literals, char *line, int srcLineNo);
int sdl_literal_end(SDL_CONTEXT *context, SDL_QUEUE *literals, int srcLineNo);
int sdl_declare(SDL_CONTEXT *context, char *name, int64_t size, int srcLineNo);
int sdl_declare_compl(SDL_CONTEXT *context, int srcLineNo);
int sdl_item(SDL_CONTEXT *context, char *name, int64_t datatype, int srcLineNo);
int sdl_item_compl(SDL_CONTEXT *context, int srcLineNo);
int sdl_constant(
		SDL_CONTEXT *context,
		char *id,
		int64_t value,
		char *valueStr,
		int size,
		int srcLineNo);
int sdl_constant_compl(SDL_CONTEXT *context, int srcLineNo);
int sdl_aggregate(
		SDL_CONTEXT *context,
		char *name,
		int64_t datatype,
		int aggType,
		int srcLineNo);
int sdl_aggregate_member(
		SDL_CONTEXT *context,
		char *name,
		int64_t datatype,
		int aggType,
		int srcLineNo);
int sdl_aggregate_compl(SDL_CONTEXT *context, char *name, int srcLineNo);
int sdl_entry(SDL_CONTEXT *context, char *name, int srcLineNo);
int sdl_add_parameter(
		SDL_CONTEXT *context,
		int64_t datatype,
		int passing,
		int srcLineNo);

/*
 * These are used to specify the constant information being saved.
 */
#define SDL_K_CONST_NAME	1
#define SDL_K_CONST_NAMES	2
#define SDL_K_CONST_STRING	3
#define SDL_K_CONST_COUNTER	4
#define SDL_K_CONST_TYPENAME	5
#define SDL_K_CONST_PREFIX	6
#define SDL_K_CONST_TAG		7
#define SDL_K_CONST_EQUALS	8
#define SDL_K_CONST_INCR	9
#define SDL_K_CONST_RADIX	10

#endif	/* _OPENSDL_ACTIONS_H_ */
