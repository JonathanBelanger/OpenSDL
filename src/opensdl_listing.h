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
 *  This header file contains the routine definitions called by other modules.
 *
 * Revision History:
 *
 *  V01.000	25-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _OPENSDL_LISTING_H_
#define _OPENSDL_LISTING_H_

FILE *sdl_open_listing(SDL_CONTEXT *context);
void sdl_write_list(FILE *fp, char *buf, size_t len);
void sdl_write_err(FILE *fp, SDL_MSG_VECTOR *msgVector);
void sdl_close_listing(SDL_CONTEXT *context);

#endif /* _OPENSDL_LISTING_H_ */
