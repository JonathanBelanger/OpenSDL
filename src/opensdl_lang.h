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
typedef	int (*SDL_FUNC)();
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
    LangItem
} SDL_LANG_AGGR_TYPE;
typedef struct
{
    union
    {
	void		*parameter;
	SDL_AGGREGATE	*aggr;
	SDL_SUBAGGR	*subaggr;
	SDL_ITEM	*item;
    };
} SDL_LANG_AGGR;

/*
 * Define the C/C++ output function prototypes.
 */
int sdl_c_commentStars(FILE *fp);
int sdl_c_createdByInfo(FILE *fp, struct tm *timeInfo);
int sdl_c_fileInfo(FILE *fp, struct tm *timeInfo, char *fullFilePath);
int sdl_c_comment(
		FILE *fp,
		char *comment,
		bool lineComment,
		bool startComment,
		bool middleComment,
		bool endComment);
int sdl_c_module(FILE *fp, SDL_CONTEXT *context);
int sdl_c_module_end(FILE *fp, SDL_CONTEXT *context);
int sdl_c_item(FILE *fp, SDL_ITEM *item, SDL_CONTEXT *context);
int sdl_c_constant(FILE *fp, SDL_CONSTANT *constant, SDL_CONTEXT *context);
int sdl_c_aggregate(
		FILE *fp,
		void *param,
		SDL_LANG_AGGR_TYPE type,
		bool ending,
		int depth,
		SDL_CONTEXT *context);
int sdl_c_entry(FILE *fp, SDL_ENTRY *entry, SDL_CONTEXT *context);
int sdl_c_enumerate(FILE *fp, SDL_ENUMERATE *entry, SDL_CONTEXT *context);

#endif	/* _OPENSDL_LANG_H_ */
