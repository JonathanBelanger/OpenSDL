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
 *  V01.000	Aug 28, 2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	Sep  6, 2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
#ifndef _OPENSDL_ACTIONS_H_
#define _OPENSDL_ACTIONS_H_	1

/*
 * Get and set functions.
 */
char *sdl_unquote_str(char *str);
SDL_DECLARE *sdl_get_declare(SDL_DECLARE_LIST *declare, int typeID);
SDL_ITEM *sdl_get_item(SDL_ITEM_LIST *item, int typeID);
SDL_AGGREGATE *sdl_get_aggregate(SDL_AGGREGATE_LIST *aggregate, int typeID);
int sdl_get_local(SDL_CONTEXT *context, char *name, __int64_t *value);
int sdl_set_local(SDL_CONTEXT *context, char *name, __int64_t value);

/*
 * Parsing functions.
 */
int sdl_comment(SDL_CONTEXT *context, char *comment);
int sdl_module(SDL_CONTEXT *context, char *moduleName, char *identName);
int sdl_module_end(SDL_CONTEXT *context, char *moduleName);
int sdl_literal(SDL_QUEUE *literals, char *line);
int sdl_literal_end(SDL_CONTEXT *context, SDL_QUEUE *literals);
int sdl_usertype_idx(SDL_CONTEXT *context, char *usertype);
int sdl_declare(
	SDL_CONTEXT *context,
	char *name,
	int size,
	char *prefix,
	char *tag);
__int64_t sdl_bin2int(char *binStr);
int sdl_str2int(char *strVal, __int64_t *val);
int sdl_offset(SDL_CONTEXT *context, int offsetType);
SDL_IDENT *sdl_constant_eq(__int64_t value, _Bool present);
int sdl_constant_add(SDL_CONTEXT *context, char *name, SDL_IDENT *initVal);
int sdl_constant(SDL_CONTEXT *context, __int64_t value, char *valueStr);
int sdl_constant_name(SDL_CONTEXT *context, char *name);
int sdl_constant_names(
		SDL_CONTEXT *context,
		__int64_t value,
		char *prefix,
		char *tag,
		char *counter,
		__int64_t incr,
		char *typeName,
		int radix);
int sdl_constant_opts(
		SDL_CONTEXT *context,
		char *prefix,
		char *tag,
		char *counter,
		__int64_t incr,
		char *typeName,
		int radix);
int sdl_constant_val(
		SDL_CONTEXT *context,
		char *prefix,
		char *tag,
		char *counter,
		char *typeName,
		int radix);
int sdl_constant_done(SDL_CONTEXT *context, _Bool alreadyQd);

#define SDL_M_LEAD	0x00000001	/* remove leading spaces */
#define SDL_M_TRAIL	0x00000002	/* remove trailing spaces */
#define SDL_M_COMPRESS	0x00000004	/* remove duplicate spaces */
#define SDL_M_COLLAPSE	0x00000008	/* remove all spaces */
#define SDL_M_CONVERT	0x00000010	/* make sure only spaces are used */
#define SDL_M_TRIM	(SDL_M_LEAD | SDL_M_TRAIL)
void sdl_trim_str(char *str, int type);

#endif	/* _OPENSDL_ACTIONS_H_ */
