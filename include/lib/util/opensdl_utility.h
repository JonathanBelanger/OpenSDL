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
 *  This header file contains the function prototypes for the routines called
 *  during the parsing of the input file.
 *
 * Revision History:
 *
 *  V01.000	04-OCT-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _OPENSDL_UTILITY_H_
#define _OPENSDL_UTILITY_H_

uint32_t sdl_state_transition(
		SDL_CONTEXT *context,
		SDL_STATE action,
		SDL_YYLTYPE *loc);
char *sdl_unquote_str(char *str);
SDL_LOCAL_VARIABLE *sdl_find_local(SDL_CONTEXT *context, char *name);
SDL_DECLARE *sdl_get_declare(SDL_DECLARE_LIST *declare, int typeID);
SDL_ITEM *sdl_get_item(SDL_ITEM_LIST *item, int typeID);
SDL_AGGREGATE *sdl_get_aggregate(SDL_AGGREGATE_LIST *aggregate, int typeID);
SDL_ENUMERATE *sdl_get_enum(SDL_ENUM_LIST *enums, int typeID);
uint32_t sdl_get_local(SDL_CONTEXT *context, char *name, __int64_t *value);
int sdl_usertype_idx(SDL_CONTEXT *context, char *usertype);
int sdl_aggrtype_idx(SDL_CONTEXT *context, char *aggregateName);
int64_t sdl_bin2int(char *binStr);
uint32_t sdl_str2int(char *strVal, int64_t *val);
int64_t sdl_offset(SDL_CONTEXT *context, int offsetType, SDL_YYLTYPE *loc);
int sdl_dimension(SDL_CONTEXT *context, size_t lbound, size_t hbound);
uint32_t sdl_add_option(
		SDL_CONTEXT *context,
		SDL_OPTION_TYPE option,
		__int64_t value,
		char *string,
		SDL_YYLTYPE *loc);
uint32_t sdl_precision(SDL_CONTEXT *context, int64_t precision, int64_t scale);
bool sdl_all_lower(const char *str);

#define SDL_M_LEAD	0x00000001	/* remove leading spaces */
#define SDL_M_TRAIL	0x00000002	/* remove trailing spaces */
#define SDL_M_COMPRESS	0x00000004	/* remove duplicate spaces */
#define SDL_M_COLLAPSE	0x00000008	/* remove all spaces */
#define SDL_M_CONVERT	0x00000010	/* make sure only spaces are used */
#define SDL_M_KEEP_NL	0x00000020	/* keep new-lines, if present */
#define SDL_M_TRIM	(SDL_M_LEAD | SDL_M_TRAIL)
void sdl_trim_str(char *str, int type);
int64_t *sdl_increment(int64_t value);
int64_t sdl_sizeof(SDL_CONTEXT *context, int item);
bool sdl_isUnsigned(SDL_CONTEXT *context, int64_t *datatype);
bool sdl_isItem(SDL_MEMBERS *member);
bool sdl_isComment(SDL_MEMBERS *member);
bool sdl_isBitfield(SDL_MEMBERS *member);
bool sdl_isAddress(int type);

#endif	/* _OPENSDL_UTILITY_H_ */
