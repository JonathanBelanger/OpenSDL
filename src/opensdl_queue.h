/*
 * Copyright (C) Jonathan D. Belanger 2018.
 *
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
 *  This header file contains the definitions needed for the action routines to
 *  be able to interact with the parsing of an Open Structure Definition
 *  Language (OpenSDL) formatted definition file.
 *
 * Revision History:
 *
 *  V01.000	04-OCT-2018	Jonathan D. Belanger
 *  Initially written.
 */

#ifndef _OPENSDL_QUEUE_H_
#define _OPENSDL_QUEUE_H_

/*
 * The following definitions are used to define and maintain queues.
 */
typedef struct
{
    void		*flink;
    void		*blink;
} SDL_QUEUE;

#define SDL_Q_INIT(q)	((q)->flink = (q)->blink = (void *) (q))
#define SDL_Q_EMPTY(q)	(((q)->flink == (q)->blink) && ((q)->flink == (q)))

/*
 * Insert at the end of the queue (blink).
 */
#define SDL_INSQUE(q, e)				\
    SDL_QUEUE	*p = (SDL_QUEUE *) (q)->blink;		\
    (e)->flink = (void *) (q);				\
    (e)->blink = (void *) p;				\
    (q)->blink = p->flink = (void *) e

/*
 * Remove from the beginning of the queue (flink).
 */
#define SDL_REMQUE(q, e)				\
    if (SDL_Q_EMPTY((q)) == false)			\
    {							\
	SDL_QUEUE *n = (SDL_QUEUE *) (q)->flink;	\
	SDL_QUEUE *nn = (SDL_QUEUE *) n->flink;		\
	(e) = (q)->flink;				\
	(q)->flink = n->flink;				\
	nn->blink = (void *) (q);			\
    }							\
    else						\
	(e) = NULL


#endif	/* _OPENSDL_QUEUE_H_ */
