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
 *  This header file contains the function prototypes for the memory routines
 *  called during the parsing of the input file.
 *
 * Revision History:
 *
 *  V01.000	17-NOV-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _OPENSDL_BLOCKS_H_
#define _OPENSDL_BLOCKS_H_

void sdl_set_trace_memory(void);
void *sdl_allocate_block(
		SDL_BLOCK_ID blockID,
		SDL_HEADER *parent,
		SDL_YYLTYPE *loc);
void sdl_deallocate_block(SDL_HEADER *block);
char *sdl_strdup(const char *string);
void *sdl_calloc(size_t count, size_t size);
void *sdl_realloc(void *ptr, size_t newSize);
void sdl_free(void *ptr);

#endif /* _OPENSDL_BLOCKS_H_ */
