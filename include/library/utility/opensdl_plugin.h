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
 *  This header file contains the definitions required for a language plugin to
 *  be added to OpenSDL without having to recompile the tool.  The language
 *  plugins are created as a shared library (.so on Unix and .dll on Windows).
 *  The name of these shared libraries will have a specific format.  This
 *  format is:
 *
 *      opensdl_<language>.so (.dll for Windows)
 *
 *  The <language> portion will be what is provided on the --lang=<language>
 *  qualifier.
 *
 * Revision History:
 *
 *  V01.000 13-Apr-2019 Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _OPENSDL_PLUGIN_H_
#define _OPENSDL_PLUGIN_H_

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN64
#include <Windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif
#include "opensdl_defs.h"
#include "library/language/opensdl_lang.h"
#include "library/common/opensdl_message.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * This is the version information passed to the plugin to indicate the version
 * that the OpenSDL utility will be speaking.  If the plugin cannot support it,
 * it should return a SDL_REVCHECK error.
 */
typedef struct
{
    uint8_t patch;
    uint8_t minor;
    uint8_t major;
    uint8_t type;
} SDL_API_VERSION;

#define SDL_VER_TYPE_TEST       'T'
#define SDL_VER_TYPE_RC         'R'
#define SDL_VER_TYPE_RELEASE    'V'
#define SDL_API_VERSION_MAJOR   1
#define SDL_API_VERSION_MINOR   0
#define SDL_API_VERSION_PATCH   0

typedef uint32_t (*sdl_plugin_commentStars)(void);
typedef uint32_t (*sdl_plugin_createdByInfo)(struct tm *timeInfo);
typedef uint32_t (*sdl_plugin_fileInfo)(struct tm *timeInfo, char *filePath);
typedef uint32_t (*sdl_plugin_comment)(char *comment,
                                       bool lineComment,
                                       bool startComment,
                                       bool middleComment,
                                       bool endComment);
typedef uint32_t (*sdl_plugin_module)(SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_moduleEnd)(SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_item)(SDL_ITEM *item, SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_constant)(SDL_CONSTANT *constant,
                                        SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_enumerate)(SDL_ENUMERATE *_enum,
                                         SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_aggregate)(void *param,
                                         SDL_LANG_AGGR_TYPE type,
                                         bool ending,
                                         int depth,
                                         SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_entry)(SDL_ENTRY *entry, SDL_CONTEXT *context);
typedef uint32_t (*sdl_plugin_literal)(char *line);
typedef uint32_t (*sdl_plugin_close)(void);

typedef enum
{
    SDL_API_NULL = 0,       /* Last entry in Transfer Vector */
    SDL_API_PROTOCOL_VER,
    SDL_API_MESSAGE_VECTOR,
    SDL_API_TRACE_PTR,
    SDL_API_OUTPUT_FP,
    SDL_API_COMMENT_STAR,
    SDL_API_CREATED_BY,
    SDL_API_FILE_INFO,
    SDL_API_COMMENT,
    SDL_API_MODULE,
    SDL_API_MODULE_END,
    SDL_API_ITEM,
    SDL_API_CONSTANT,
    SDL_API_ENUMERATE,
    SDL_API_AGGREGATE,
    SDL_API_ENTRY,
    SDL_API_FILE_EXTENSION,
    SDL_API_LITERAL,
    SDL_API_CLOSE,
    SDL_API_MAX
} SDL_API_TAG;

typedef struct
{
    SDL_API_TAG tag;
    union
    {
        uint32_t sdl_tv_value;
        SDL_API_VERSION sdl_tv_version;
        bool *sdl_tv_boolean;
        char *sdl_tv_string;
        FILE *sdl_tv_fp;
        SDL_MSG_VECTOR *sdl_tv_msgVec;
        sdl_plugin_commentStars sdl_tv_commentStars;
        sdl_plugin_createdByInfo sdl_tv_createdByInfo;
        sdl_plugin_fileInfo sdl_tv_fileInfo;
        sdl_plugin_comment sdl_tv_comment;
        sdl_plugin_module sdl_tv_module;
        sdl_plugin_moduleEnd sdl_tv_moduleEnd;
        sdl_plugin_item sdl_tv_item;
        sdl_plugin_constant sdl_tv_constant;
        sdl_plugin_enumerate sdl_tv_enumerate;
        sdl_plugin_aggregate sdl_tv_aggregate;
        sdl_plugin_entry sdl_tv_entry;
        sdl_plugin_literal sdl_tv_literal;
        sdl_plugin_close sdl_tv_close;
    };
} SDL_API_TV;

typedef uint32_t (*sdl_plugin_onload)(SDL_API_TV *tv);

#ifdef __cplusplus
}
#endif

/*
 * The following macros defined O/S specific ways to get the path to ourselves.
 */
#ifdef _WIN64
#define SHARED_LIBRARY_PREFIX ""
#define SHARED_LIBRARY_SUFFIX "dll"
#define GET_IMAGE_PATH(buf, bufLen) GetModuleFileNameA(NULL,                \
                                                       buf,                 \
                                                       bufLen)
#elif defined(__APPLE__) && defined(__MACH__)
#define SHARED_LIBRARY_PREFIX ""
#define SHARED_LIBRARY_SUFFIX "so"
#define GET_IMAGE_PATH(buf, bufLen)                                         \
            uint32_ t size = bufLen;                                        \
                                                                            \
            _NSGetExecutablePath(buf, &size)
#elif defined(__DragonFly__)
#define SHARED_LIBRARY_PREFIX "lib"
#define SHARED_LIBRARY_SUFFIX "so"
#define GET_IMAGE_PATH(buf, bufLen) readlink("/proc/curproc/file",          \
                                             buf,                           \
                                             bufLen)
#elif defined(__NetBSD__)
#define SHARED_LIBRARY_PREFIX "lib"
#define SHARED_LIBRARY_SUFFIX "so"
#define GET_IMAGE_PATH(buf, bufLen) readlink("/proc/curproc/exe",           \
                                             buf,                           \
                                             bufLen)
#elif defined(__CYGWIN__)
#define SHARED_LIBRARY_PREFIX "cyg"
#define SHARED_LIBRARY_SUFFIX "dll"
#define GET_IMAGE_PATH(buf, bufLen) readlink("/proc/self/exe",              \
                                             buf,                           \
                                             bufLen)
#else
#define SHARED_LIBRARY_PREFIX "lib"
#define SHARED_LIBRARY_SUFFIX "so"
#define GET_IMAGE_PATH(buf, bufLen) readlink("/proc/self/exe",              \
                                             buf,                           \
                                             bufLen)
#endif

#endif /* _OPENSDL_PLUGIN_H_ */
