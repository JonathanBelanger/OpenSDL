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
 *  This source file contains the the code used to load a plugin into OpenSDL
 *  to generate the output for a particular language.  The file to be open is a
 *  shared library (.so on Unix and .dll on Windows) with the name format of:
 *
 *      opensdl_<language>.so (.dll for Windows)
 *
 *  The <language> portion will be what is provided on the --lang=<language>
 *  qualifier.
 *
 * Revision History:
 *
 *  V01.000	103-Apr-2019 Jonathan D. Belanger
 *  Initially written.
 */
#include "opensdl_defs.h"
#include "library/common/opensdl_blocks.h"
#include "library/utility/opensdl_plugin.h"
#include "opensdl/opensdl_main.h"

typedef struct
{
    char *lang;
    char *fileExt;
    sdl_plugin_onload onLoad;
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
} SDL_PLUGIN_INFO;
SDL_PLUGIN_INFO *_sdl_plugin_info = NULL;
uint32_t _sdl_plugin_info_count = 0;

static char *_sdl_shared_image_name_fmt = "%s%c%sOpenSDL_%s.%s";
static char _sdl_image_path[PATH_MAX];
static bool _sdl_image_path_initialized = false;

/*
 * sdl_load_plugin
 *  This function is called with a language string and attempts to load the
 *  shared library for.  The shared library will be located in the same
 *  directory.  The plugin is loaded while processing the --lang command line
 *  argument(s).  This is prior to any output file being opened.  Therefore, a
 *  second call to the onLoad plugin function must be performed prior to doing
 *  any real processing.
 *
 * Input Parameters:
 *  lang:
 *      A pointer to a null-terminated string containing the language name that
 *      will be used to located and open the language shared library.
 *
 * Output Parameters:
 *  fileExt:
 *      A pointer to the address of a character string to receive the file
 *      extension to use for the output file.
 *  langId:
 *      A pointer to an unsigned 32-bit integer to receive the identifier for
 *      the loaded language.  This will be used on subsequent calls.
 *
 * Return Values:
 *  SDL_NORMAL      - Normal successful completion
 *  SDL_INVSHRIMG   - Output language not known
 *  SDL_ERREXIT     - Error exit
 *  SDL_ABORT       - Fatal internal error. Unable to continue execution
 */
uint32_t sdl_load_plugin(char *lang, char **fileExt, uint32_t *langId)
{
    uint32_t retVal = SDL_NORMAL;
    char pathToSharedLib[PATH_MAX];
    char *envPath = NULL;
    void *dlFP;
    uint32_t ii;

    /*
     * If we already loaded this language, then there is not really any more to
     * to.  Just return the same information as if we had to perform the load.
     */
    for (ii = 0; ii < _sdl_plugin_info_count; ii++)
    {
        if (strcmp((const char*) _sdl_plugin_info[ii].lang, lang) == 0)
        {
            *langId = ii;
            *fileExt = _sdl_plugin_info[ii].fileExt;
            return SDL_NORMAL;
        }
    }

    /*
     * If this is the first time we've been called, then go get the path to
     * ourselves.  The shared library will be in the same directory.  If the
     * environment variable SDL_SHARED_LIBRARY_PATH is defined, then this will
     * be used instead of the path to the opensdl.exe image.
     */
    envPath = getenv("SDL_SHARED_LIBRARY_PATH");
    if (_sdl_image_path_initialized == false)
    {
        if (envPath == NULL)
        {
            char *ptr;

            GET_IMAGE_PATH(_sdl_image_path, PATH_MAX) ;
            ptr = strrchr(_sdl_image_path, PATH_SEP);
            *ptr = '\0';
        }
        else
        {
            strcpy(_sdl_image_path, envPath);
        }
        _sdl_image_path_initialized = true;
    }

    /*
     * Generate the path to the shared library for this language.
     */
    sprintf(pathToSharedLib,
            _sdl_shared_image_name_fmt,
            _sdl_image_path,
            PATH_SEP,
            SHARED_LIBRARY_PREFIX,
            lang,
            SHARED_LIBRARY_SUFFIX);

    /*
     * Let's make sure we have execute access to the shared library.
     */
    if (access(pathToSharedLib, X_OK) != 0)
    {
        retVal = SDL_INVSHRIMG;
        if (sdl_set_message(msgVec,
                            2,
                            retVal,
                            pathToSharedLib,
                            errno) != SDL_NORMAL)
        {
            retVal = SDL_ERREXIT;
        }
        return retVal;
    }

    /*
     * So, everything looks good.  Open the shared library so that we can get
     * the onLoad function address.
     */
    dlFP = dlopen(pathToSharedLib, RTLD_LAZY);
    if (dlFP != NULL)
    {
        sdl_plugin_onload onLoad = NULL;

        dlerror();  /* Clear any errors */

        /*
         * Get the address of the onLoad function.
         */
        *(void **) &onLoad = dlsym(dlFP, "onLoad");
        if (dlerror() != NULL)
        {
            retVal = SDL_INVSHRIMG;
            if (sdl_set_message(msgVec,
                                2,
                                retVal,
                                pathToSharedLib,
                                errno) != SDL_NORMAL)
            {
                retVal = SDL_ERREXIT;
            }
            return retVal;
        }
        else
        {
            SDL_API_TV tv[SDL_API_MAX - 1];
            int ii = 0;

            /*
             * Setup the transfer vector that will be passed to the share
             * library's onLoad function, which will parse it and return the
             * information we need to call into it as we perform our real
             * processing.
             */
            tv[ii].tag = SDL_API_PROTOCOL_VER;
            tv[ii].sdl_tv_version.type = SDL_VER_TYPE_TEST;
            tv[ii].sdl_tv_version.major = SDL_API_VERSION_MAJOR;
            tv[ii].sdl_tv_version.minor = SDL_API_VERSION_MINOR;
            tv[ii++].sdl_tv_version.patch = SDL_API_VERSION_PATCH;
            tv[ii].tag = SDL_API_MESSAGE_VECTOR;
            tv[ii++].sdl_tv_msgVec = msgVec;
            tv[ii].tag = SDL_API_TRACE_PTR;
            tv[ii++].sdl_tv_boolean = &trace;
            tv[ii].tag = SDL_API_COMMENT_STAR;
            tv[ii++].sdl_tv_commentStars = NULL;
            tv[ii].tag = SDL_API_CREATED_BY;
            tv[ii++].sdl_tv_createdByInfo = NULL;
            tv[ii].tag = SDL_API_FILE_INFO;
            tv[ii++].sdl_tv_fileInfo = NULL;
            tv[ii].tag = SDL_API_COMMENT;
            tv[ii++].sdl_tv_comment = NULL;
            tv[ii].tag = SDL_API_MODULE;
            tv[ii++].sdl_tv_module = NULL;
            tv[ii].tag = SDL_API_MODULE_END;
            tv[ii++].sdl_tv_moduleEnd = NULL;
            tv[ii].tag = SDL_API_ITEM;
            tv[ii++].sdl_tv_item = NULL;
            tv[ii].tag = SDL_API_CONSTANT;
            tv[ii++].sdl_tv_constant = NULL;
            tv[ii].tag = SDL_API_ENUMERATE;
            tv[ii++].sdl_tv_enumerate = NULL;
            tv[ii].tag = SDL_API_AGGREGATE;
            tv[ii++].sdl_tv_aggregate = NULL;
            tv[ii].tag = SDL_API_ENTRY;
            tv[ii++].sdl_tv_entry = NULL;
            tv[ii].tag = SDL_API_FILE_EXTENSION;
            tv[ii++].sdl_tv_string = NULL;
            tv[ii].tag = SDL_API_LITERAL;
            tv[ii++].sdl_tv_literal = NULL;
            tv[ii].tag = SDL_API_CLOSE;
            tv[ii++].sdl_tv_close = NULL;
            tv[ii].tag = SDL_API_NULL;

            /*
             * Call the onLoad function.
             */
            retVal = (*onLoad)(tv);
            if (retVal == SDL_NORMAL)
            {

                /*
                 * So far, everything looks great.  Allocate a record to hold
                 * this plugin's information.
                 */
                _sdl_plugin_info = sdl_realloc(_sdl_plugin_info,
                                               ((_sdl_plugin_info_count + 1) *
                                                sizeof(SDL_PLUGIN_INFO)));
                if (_sdl_plugin_info != NULL)
                {
                    uint32_t index = _sdl_plugin_info_count++;

                    _sdl_plugin_info[index].lang = sdl_strdup(lang);
                    _sdl_plugin_info[index].onLoad = onLoad;
                    ii = 0;
                    while (tv[ii].tag != SDL_API_NULL)
                    {
                        switch (tv[ii].tag)
                        {
                            case SDL_API_COMMENT_STAR:
                                _sdl_plugin_info[index].sdl_tv_commentStars =
                                        tv[ii].sdl_tv_commentStars;
                                break;

                            case SDL_API_CREATED_BY:
                                _sdl_plugin_info[index].sdl_tv_createdByInfo =
                                    tv[ii].sdl_tv_createdByInfo;
                                break;

                            case SDL_API_FILE_INFO:
                                _sdl_plugin_info[index].sdl_tv_fileInfo =
                                    tv[ii].sdl_tv_fileInfo;
                                break;

                            case SDL_API_COMMENT:
                                _sdl_plugin_info[index].sdl_tv_comment =
                                    tv[ii].sdl_tv_comment;
                                break;

                            case SDL_API_MODULE:
                                _sdl_plugin_info[index].sdl_tv_module =
                                    tv[ii].sdl_tv_module;
                                break;

                            case SDL_API_MODULE_END:
                                _sdl_plugin_info[index].sdl_tv_moduleEnd =
                                    tv[ii].sdl_tv_moduleEnd;
                                break;

                            case SDL_API_ITEM:
                                _sdl_plugin_info[index].sdl_tv_item =
                                    tv[ii].sdl_tv_item;
                                break;

                            case SDL_API_CONSTANT:
                                _sdl_plugin_info[index].sdl_tv_constant =
                                    tv[ii].sdl_tv_constant;
                                break;

                            case SDL_API_ENUMERATE:
                                _sdl_plugin_info[index].sdl_tv_enumerate =
                                    tv[ii].sdl_tv_enumerate;
                                break;

                            case SDL_API_AGGREGATE:
                                _sdl_plugin_info[index].sdl_tv_aggregate =
                                    tv[ii].sdl_tv_aggregate;
                                break;

                            case SDL_API_ENTRY:
                                _sdl_plugin_info[index].sdl_tv_entry =
                                    tv[ii].sdl_tv_entry;
                                break;

                            case SDL_API_FILE_EXTENSION:
                                _sdl_plugin_info[index].fileExt =
                                   tv[ii].sdl_tv_string;
                                break;

                            case SDL_API_LITERAL:
                                _sdl_plugin_info[index].sdl_tv_literal=
                                   tv[ii].sdl_tv_literal;
                                break;

                            case SDL_API_CLOSE:
                                _sdl_plugin_info[index].sdl_tv_close =
                                    tv[ii].sdl_tv_close;
                                break;

                            default:
                                break;
                        }
                        ii++;
                    }

                    /*
                     * Now to initialize the output parameters.
                     */
                    *langId = index;
                    *fileExt = _sdl_plugin_info[index].fileExt;
                }
                else
                {
                    retVal = SDL_ABORT;
                    if (sdl_set_message(msgVec,
                                        2,
                                        retVal,
                                        ENOMEM) != SDL_NORMAL)
                    {
                        retVal = SDL_ERREXIT;
                    }
                }
            }
            else
            {
                if (sdl_set_message(msgVec,
                                    1,
                                    retVal) != SDL_NORMAL)
                {
                    retVal = SDL_ERREXIT;
                }
            }
        }
    }
    else
    {
        retVal = SDL_INVSHRIMG;
        if (sdl_set_message(msgVec,
                            2,
                            retVal,
                            pathToSharedLib,
                            errno) != SDL_NORMAL)
        {
            retVal = SDL_ERREXIT;
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_load_fp
 *  This function is called to give the plugin the file pointer to which it
 *  will write its generated output.  This needs to be done after the
 *  sdl_load_plugin call and before any of the other calls to request the
 *  plugin to generate its output.
 *
 * Input Parameters:
 *  langId:
 *      A value associated with this particular language plugin.
 *  fp:
 *      The file pointer to pass to the plugin.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL      - Normal successful completion
 */
uint32_t sdl_load_fp(uint32_t langId, FILE *fp)
{
    uint32_t retVal = SDL_NORMAL;
    SDL_API_TV tv[2];

    /*
     * We only provide one thing to the plugin, the output file pointer, so
     * initialize the transfer vector with this information and call the
     * plugin's onLoad function.
     */
    tv[0].tag = SDL_API_OUTPUT_FP;
    tv[0].sdl_tv_fp = fp;
    tv[1].tag = SDL_API_NULL;
    retVal = (*_sdl_plugin_info[langId].onLoad)(tv);
    if (retVal != SDL_NORMAL)
    {
        if (sdl_set_message(msgVec,
                            1,
                            retVal) != SDL_NORMAL)
        {
            retVal = SDL_ERREXIT;
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_commentStars
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_commentStars(void)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if (_sdl_plugin_info[ii].sdl_tv_commentStars != NULL)
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_commentStars)();
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_createdByInfo
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  timeInfo:
 *      A pointer to a time structure containing the start time of this
 *      compile.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_createdByInfo(struct tm *timeInfo)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if (_sdl_plugin_info[ii].sdl_tv_createdByInfo != NULL)
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_createdByInfo)(timeInfo);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_fileInfo
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  timeInfo:
 *      A pointer to a time structure containing the modify time of the input
 *      file.
 *  filePath:
 *      A pointer to a null terminated string for the full path of the input
 *      file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_fileInfo(struct tm *timeInfo, char *filePath)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if (_sdl_plugin_info[ii].sdl_tv_fileInfo != NULL)
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_fileInfo)(timeInfo,
                                                             filePath);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_comment
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  comment:
 *    A pointer to the comment string to be output.
 *  lineComment:
 *    A boolean value indicating that the comment should start with a start
 *    comment and end with an end comment.
 *  startComment:
 *    A boolean value indicating that the comment should start with a start
 *    comment.
 *  endComment:
 *    A boolean value indicating that the comment should end with an end
 *    comment.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_comment(bool *langEna,
                          char *comment,
                          bool lineComment,
                          bool startComment,
                          bool middleComment,
                          bool endComment)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_comment != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_comment)(comment,
                                                            lineComment,
                                                            startComment,
                                                            middleComment,
                                                            endComment);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_module
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  context:
 *    A pointer to the parser context where the module name and ident
 *    information are stored.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_module(bool *langEna, SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_module != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_module)(context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_moduleEnd
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  context:
 *    A pointer to the parser context where the module name information is
 *    stored.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_moduleEnd(bool *langEna, SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_moduleEnd != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_moduleEnd)(context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_item
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  item:
 *      A pointer to the ITEM record.
 *  context:
 *    A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_item(bool *langEna, SDL_ITEM *item, SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_item != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_item)(item, context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_constant
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  constant:
 *      A pointer to the CONSTANT record.
 *  context:
 *    A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_constant(bool *langEna,
                           SDL_CONSTANT *constant,
                           SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_constant != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_constant)(constant, context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_enumerate
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  _enum:
 *    A pointer to the ENUMERATE record
 *  context:
 *    A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_enumerate(bool *langEna,
                            SDL_ENUMERATE *_enum,
                            SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_enumerate != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_enumerate)(_enum, context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_aggregate
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  param:
 *    A pointer to the AGGREGATE, subaggregate, or ITEM record
 *  type:
 *    A value indicating the type of structure the param parameter
 *    represents.
 *  ending:
 *    A boolean value indicating that we are ending a definition.  This flag
 *    is used in the following way:
 *      if (ending == true)
 *          if (type == LangAggregate)
 *              <we are ending an aggregate>
 *          else if (type == LangSubaggregate)
 *              <we are ending a subaggregate>
 *          else if (type == LangAggregate)
 *              <we are starting an aggregate>
 *          else if (type == LangSubaggregate)
 *              <we are starting a subaggregate>
 *          else
 *              <we are defining a single member item>
 *  depth:
 *      A value indicating the depth at which we are defining the next field.
 *      This is used for indenting purposes.
 *  context:
 *    A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_aggregate(bool *langEna,
                            void *param,
                            SDL_LANG_AGGR_TYPE type,
                            bool ending,
                            int depth,
                            SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_aggregate!= NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_aggregate)(param,
                                                              type,
                                                              ending,
                                                              depth,
                                                              context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_entry
 *  This function is called to call the plugin specific version of this call,
 *  if it was returned by the plugin.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  entry:
 *    A pointer to the ENTRY record
 *  context:
 *    A pointer to the context block to be used to determine the type string.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_entry(bool *langEna, SDL_ENTRY *entry, SDL_CONTEXT *context)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_entry != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_entry)(entry, context);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_call_literal
 *  This function is called to request that a literal line be written out to
 *  the output file.  There is no parsing or interpretation of this literal
 *  string.
 *
 * Input Parameters:
 *  langEna:
 *      An array containing the language enabled flag for each of the
 *      languages.
 *  line:
 *    A pointer to a null terminated string to be written to the output files.
 *
 * Output Parameters:
 *  None:
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_literal(bool *langEna, char *line)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        if ((_sdl_plugin_info[ii].sdl_tv_literal != NULL) &&
            (langEna[ii] == true))
        {
            retVal = (*_sdl_plugin_info[ii].sdl_tv_literal)(line);
        }
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * sdl_close_all
 *  This function is called to close all the language files.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  SDL_NORMAL  - Normal successful completion
 */
uint32_t sdl_call_close(void)
{
    uint32_t retVal = SDL_NORMAL;
    uint32_t ii;

    /*
     * Loop through each of the plugins, calling their equivalent function.
     */
    for (ii = 0;
         ((ii < _sdl_plugin_info_count) && (retVal == SDL_NORMAL));
         ii++)
    {
        retVal = (*_sdl_plugin_info[ii].sdl_tv_close)();
        sdl_free(_sdl_plugin_info[ii].lang);
        sdl_free(_sdl_plugin_info[ii].fileExt);
    }
    sdl_free(_sdl_plugin_info);

    /*
     * Return back to the caller.
     */
    return(retVal);
}
