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
 *  This header file contains all the output function definitions for all the
 *  supported languages.
 *
 * Revision History:
 *
 *  V01.000	06-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	06-SEP-2018	Jonathan D. Belanger
 *  Updated the copyright to be GNUGPL V3 compliant.
 */
#ifndef _OPENSDL_LANG_H_
#define _OPENSDL_LANG_H_ 1

#include <time.h>

#define SDL_K_FUNC_PER_LANG	8
#define SDL_K_COMMENT_LEN	80+1

/*
 * Typedefs to define output function calling arrays.
 */
typedef	uint32_t (*SDL_FUNC)();
typedef SDL_FUNC SDL_LANG_FUNC[SDL_K_FUNC_PER_LANG];

/*
 * To simplify the parameter passing to the sdl_c_aggregate function, I'm
 * going to use a union, which contains the pointers to the structure that
 * could be supplied on the call.  The call will have just a single pointer
 * to void.
 */
typedef enum
{
    LangAggregate,
    LangSubaggregate,
    LangItem,
    LangComment
} SDL_LANG_AGGR_TYPE;
typedef struct
{
    union
    {
	void *parameter;
	SDL_AGGREGATE *aggr;
	SDL_SUBAGGR *subaggr;
	SDL_ITEM *item;
	SDL_COMMENT *comment;
    };
} SDL_LANG_AGGR;

#endif	/* _OPENSDL_LANG_H_ */
