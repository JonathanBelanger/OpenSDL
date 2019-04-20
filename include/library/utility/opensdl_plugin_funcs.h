/*
 * Copyright (C) Jonathan D. Belanger 2019.
 *
 *  OpenSDL is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  OpenSDL is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with along with OpenSDL.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Description:
 *
 *  This header file contains the prototype definitions needed to OpenSDL to
 *  call the plugin interface functions that in turn call all the language
 *  specific functions.
 *
 * Revision History:
 *
 *  V01.000	14-Apr-2019 Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _OPENSDL_PLUGIN_FUNCS_H_
#define _OPENSDL_PLUGIN_FUNCS_H_

#include "../language/opensdl_lang.h"
#include "opensdl_defs.h"

uint32_t sdl_load_plugin(char *lang, char **fileExt, uint32_t *langId);
uint32_t sdl_load_fp(uint32_t langId, FILE *fp);
uint32_t sdl_call_commentStars(void);
uint32_t sdl_call_createdByInfo(struct tm *timeInfo);
uint32_t sdl_call_fileInfo(struct tm *timeInfo, char *filePath);
uint32_t sdl_call_comment(bool *langEna,
                          char *comment,
                          bool lineComment,
                          bool startComment,
                          bool middleComment,
                          bool endComment);
uint32_t sdl_call_module(bool *langEna, SDL_CONTEXT *context);
uint32_t sdl_call_moduleEnd(bool *langEna, SDL_CONTEXT *context);
uint32_t sdl_call_item(bool *langEna, SDL_ITEM *item, SDL_CONTEXT *context);
uint32_t sdl_call_constant(bool *langEna,
                           SDL_CONSTANT *constant,
                           SDL_CONTEXT *context);
uint32_t sdl_call_enumerate(bool *langEna,
                            SDL_ENUMERATE *_enum,
                            SDL_CONTEXT *context);
uint32_t sdl_call_aggregate(bool *langEna,
                            void *param,
                            SDL_LANG_AGGR_TYPE type,
                            bool ending,
                            int depth,
                            SDL_CONTEXT *context);
uint32_t sdl_call_entry(bool *langEna, SDL_ENTRY *entry, SDL_CONTEXT *context);
uint32_t sdl_call_literal(bool *langEna, char *line);
uint32_t sdl_call_close(void);

#endif /* _OPENSDL_PLUGIN_FUNCS_H_ */
