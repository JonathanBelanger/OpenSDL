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
 *  This source file contains the memory routines called during the parsing of
 *  the input file.
 *
 * Revision History:
 *
 *  V01.000 17-NOV-2018 Jonathan D. Belanger
 *  Initially written.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include <library/common/opensdl_blocks.h>
/* #include <library/utility/opensdl_utility.h> */

static bool traceMemory = false;

/*
 * Local Variables
 */
static uint64_t _bytes_allocated = 0;
static uint64_t _bytes_deallocated = 0;
static uint64_t _allocate_calls = 0;
static uint64_t _deallocate_calls = 0;
static uint64_t _strdup_calls = 0;
static uint64_t _calloc_calls = 0;    /* Not the system calloc */
static uint64_t _realloc_calls = 0;
static uint64_t _free_calls = 0;

/*
 * sdl_set_trace_memory
 *  This function is called to set the traceMemory flag.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void sdl_set_trace_memory(void)
{
    traceMemory = true;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * sdl_allocate_blk
 *  This function is called to allocate one of the blocks needed to maintain
 *  the information parsed from the input file(s).
 *
 * Input Parameters:
 *  blockID:
 *    A value indicating the type of block to be allocated.
 *  parent:
 *    A pointer to the parent block for this block.  If this is NULL, then
 *    there is no particular parent with which we need to concern ourselves.
 *  loc:
 *    A pointer to the start and end locations for this item in the input
 *    file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:    Error in either block ID or the attempt to allocate memory
 *        failed.
 *  !NULL:    Normal successful completion.
 */
void *sdl_allocate_block(
        SDL_BLOCK_ID blockID,
        SDL_HEADER *parent,
        SDL_YYLTYPE *loc)
{
    void    *retVal = NULL;
    uint64_t    size = 0;

    /*
     * Increment the call counter.
     */
    _allocate_calls++;

    /*
     * Determine which block to allocate.
     */
    switch(blockID)
    {
        case LocalBlock:
            retVal = calloc(1, sizeof(SDL_LOCAL_VARIABLE));
            if (retVal != NULL)
            {
                SDL_LOCAL_VARIABLE *local = (SDL_LOCAL_VARIABLE *) retVal;

                local->header.parent = parent;
                local->header.blockID = blockID;
                local->header.top = false;
                SDL_COPY_LOC(local->loc, loc);
                size = sizeof(SDL_LOCAL_VARIABLE);
            }
            break;

        case LiteralBlock:
            retVal = calloc(1, sizeof(SDL_LITERAL));
            if (retVal != NULL)
            {
                SDL_LITERAL *literal = (SDL_LITERAL *) retVal;

                literal->header.parent = parent;
                literal->header.blockID = blockID;
                literal->header.top = false;
                SDL_COPY_LOC(literal->loc, loc);
                size = sizeof(SDL_LITERAL);
            }
            break;

        case ConstantBlock:
            retVal = calloc(1, sizeof(SDL_CONSTANT));
            if (retVal != NULL)
            {
                SDL_CONSTANT *constBlk = (SDL_CONSTANT *) retVal;

                constBlk->header.parent = parent;
                constBlk->header.blockID = blockID;
                constBlk->header.top = false;
                SDL_COPY_LOC(constBlk->loc, loc);
                size = sizeof(SDL_CONSTANT);
            }
            break;

        case EnumMemberBlock:
            retVal = calloc(1, sizeof(SDL_ENUM_MEMBER));
            if (retVal != NULL)
            {
                SDL_ENUM_MEMBER *member = (SDL_ENUM_MEMBER *) retVal;

                member->header.parent = parent;
                member->header.blockID = blockID;
                member->header.top = false;
                SDL_COPY_LOC(member->loc, loc);
                size = sizeof(SDL_ENUM_MEMBER);
            }
            break;

        case EnumerateBlock:
            retVal = calloc(1, sizeof(SDL_ENUMERATE));
            if (retVal != NULL)
            {
                SDL_ENUMERATE *myEnum  = (SDL_ENUMERATE *) retVal;

                myEnum->header.parent = parent;
                myEnum->header.blockID = blockID;
                myEnum->header.top = false;
                SDL_COPY_LOC(myEnum->loc, loc);
                SDL_Q_INIT(&myEnum->members);
                size = sizeof(SDL_ENUMERATE);
            }
            break;

        case DeclareBlock:
            retVal = calloc(1, sizeof(SDL_DECLARE));
            if (retVal != NULL)
            {
                SDL_DECLARE *decl = (SDL_DECLARE *) retVal;

                decl->header.parent = parent;
                decl->header.blockID = blockID;
                decl->header.top = false;
                SDL_COPY_LOC(decl->loc, loc);
                size = sizeof(SDL_DECLARE);
            }
            break;

        case ItemBlock:
            retVal = calloc(1, sizeof(SDL_ITEM));
            if (retVal != NULL)
            {
                SDL_ITEM *item = (SDL_ITEM *) retVal;

                item->header.parent = parent;
                item->header.blockID = blockID;
                item->header.top = false;
                SDL_COPY_LOC(item->loc, loc);
                size = sizeof(SDL_ITEM);
            }
            break;

        case AggrMemberBlock:
            retVal = calloc(1, sizeof(SDL_MEMBERS));
            if (retVal != NULL)
            {
                SDL_MEMBERS *member = (SDL_MEMBERS *) retVal;

                member->header.parent = parent;
                member->header.blockID = blockID;
                member->header.top = false;
                SDL_COPY_LOC(member->loc, loc);
                size = sizeof(SDL_MEMBERS);
            }
            break;

        case AggregateBlock:
            retVal = calloc(1, sizeof(SDL_AGGREGATE));
            if (retVal != NULL)
            {
                SDL_AGGREGATE *aggr= (SDL_AGGREGATE *) retVal;

                aggr->header.parent = parent;
                aggr->header.blockID = blockID;
                aggr->header.top = false;
                SDL_COPY_LOC(aggr->loc, loc);
                SDL_Q_INIT(&aggr->members);
                size = sizeof(SDL_AGGREGATE);
            }
            break;

        case ParameterBlock:
            retVal = calloc(1, sizeof(SDL_PARAMETER));
            if (retVal != NULL)
            {
                SDL_PARAMETER *param = (SDL_PARAMETER *) retVal;

                param->header.parent = parent;
                param->header.blockID = blockID;
                param->header.top = false;
                SDL_COPY_LOC(param->loc, loc);
                size = sizeof(SDL_PARAMETER);
            }
            break;

        case EntryBlock:
            retVal = calloc(1, sizeof(SDL_ENTRY));
            if (retVal != NULL)
            {
                SDL_ENTRY *entry = (SDL_ENTRY *) retVal;

                entry->header.parent = parent;
                entry->header.blockID = blockID;
                entry->header.top = false;
                SDL_COPY_LOC(entry->loc, loc);
                SDL_Q_INIT(&entry->parameters);
                size = sizeof(SDL_ENTRY);
            }
            break;

        default:
            break;
    }

    /*
     * Count the newly allocated bytes (size will be zero if nothing was
     * allocated).
     */
    _bytes_allocated += size;

    /*
     * Trace the newly allocated block, plus additional relevant information.
     */
    if (traceMemory == true)
    {
        printf("%s:%d:sdl_allocate_block: ID = %d, size = %ld, "
               "address: 0x%016lx\n\tsdl_allocate_blk Calls: %ld\n"
               "\tsdl_deallocate_blk Calls: %ld\n\tsdl_strdup Calls: %ld\n"
               "\tsdl_calloc Calls: %ld\n\tsdl_realloc Calls: %ld\n"
               "\tsdl_free Calls: %ld\nBytes Allocated: %ld, "
               "Bytes Deallocated: %ld, Bytes Remaining %ld\n",
               __FILE__,
               __LINE__,
               blockID,
               size,
               (uint64_t) retVal,
               _allocate_calls,
               _deallocate_calls,
               _strdup_calls,
               _calloc_calls,
               _realloc_calls,
               _free_calls,
               _bytes_allocated,
               _bytes_deallocated,
               _bytes_allocated - _bytes_deallocated);
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * sdl_deallocate_blk
 *  This function is called to deallocate one of the blocks allocated to
 *  maintain the information parsed from the input file(s).
 *
 * Input Parameters:
 *  block:
 *    A pointer to the block to be deallocated.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void sdl_deallocate_block(SDL_HEADER *block)
{
    SDL_BLOCK_ID blockID = block->blockID;
    uint64_t size = 0;

    /*
     * Increment the call counter.
     */
    _deallocate_calls++;

    /*
     * Determine which block to allocate.
     */
    switch(blockID)
    {
        case LocalBlock:
            if (block != NULL)
            {
                SDL_LOCAL_VARIABLE *local = (SDL_LOCAL_VARIABLE *) block;

                if (local->id != NULL)
                {
                    sdl_free(local->id);
                }
                size = sizeof(SDL_LOCAL_VARIABLE);
            }
            break;

        case LiteralBlock:
            if (block != NULL)
            {
                SDL_LITERAL *literal = (SDL_LITERAL *) block;

                if (literal->line != NULL)
                {
                    sdl_free(literal->line);
                }
                size = sizeof(SDL_LITERAL);
            }
            break;

        case ConstantBlock:
            if (block != NULL)
            {
                SDL_CONSTANT *constBlk = (SDL_CONSTANT *) block;

                if (constBlk->comment != NULL)
                {
                    sdl_free(constBlk->comment);
                }
                if (constBlk->id != NULL)
                {
                    sdl_free(constBlk->id);
                }
                if (constBlk->prefix != NULL)
                {
                    sdl_free(constBlk->prefix);
                }
                if (constBlk->tag != NULL)
                {
                    sdl_free(constBlk->tag);
                }
                if (constBlk->typeName != NULL)
                {
                    sdl_free(constBlk->typeName);
                }
                if ((constBlk->type == SDL_K_CONST_STR) &&
                    (constBlk->string != NULL))
                {
                    sdl_free(constBlk->string);
                }
                size = sizeof(SDL_CONSTANT);
            }
            break;

        case EnumMemberBlock:
            if (block != NULL)
            {
                SDL_ENUM_MEMBER *member = (SDL_ENUM_MEMBER *) block;

                if (member->comment != NULL)
                {
                    sdl_free(member->comment);
                }
                if (member->id != NULL)
                {
                    sdl_free(member->id);
                }
                size = sizeof(SDL_ENUM_MEMBER);
            }
            break;

        case EnumerateBlock:
            if (block != NULL)
            {
                SDL_ENUMERATE *myEnum = (SDL_ENUMERATE *) block;
                SDL_ENUM_MEMBER *member;

                while (SDL_Q_EMPTY(&myEnum->members) == false)
                {
                    SDL_REMQUE(&myEnum->members, member);
                    sdl_deallocate_block(&member->header);
                }
                if (myEnum->id != NULL)
                {
                    sdl_free(myEnum->id);
                }
                if (myEnum->prefix != NULL)
                {
                    sdl_free(myEnum->prefix);
                }
                if (myEnum->tag != NULL)
                {
                    sdl_free(myEnum->tag);
                }
                size = sizeof(SDL_ENUMERATE);
            }
            break;

        case DeclareBlock:
            if (block != NULL)
            {
                SDL_DECLARE *decl = (SDL_DECLARE *) block;

                if (decl->id != NULL)
                {
                    sdl_free(decl->id);
                }
                if (decl->prefix != NULL)
                {
                    sdl_free(decl->prefix);
                }
                if (decl->tag != NULL)
                {
                    sdl_free(decl->tag);
                }
                size = sizeof(SDL_DECLARE);
            }
            break;

        case ItemBlock:
            if (block != NULL)
            {
                SDL_ITEM *item = (SDL_ITEM *) block;

                if (item->id != NULL)
                {
                    sdl_free(item->id);
                }
                if (item->prefix != NULL)
                {
                    sdl_free(item->prefix);
                }
                if (item->tag != NULL)
                {
                    sdl_free(item->tag);
                }
                size = sizeof(SDL_ITEM);
            }
            break;

        case AggrMemberBlock:
            if (block != NULL)
            {
                SDL_MEMBERS *member = (SDL_MEMBERS *) block;

                if ((member->type == SDL_K_TYPE_STRUCT) ||
                    (member->type == SDL_K_TYPE_UNION))
                {
                    if (member->subaggr.basedPtrName != NULL)
                    {
                        sdl_free(member->subaggr.basedPtrName);
                    }
                    if (member->subaggr.id != NULL)
                    {
                        sdl_free(member->subaggr.id);
                    }
                    if (member->subaggr.marker != NULL)
                    {
                        sdl_free(member->subaggr.marker);
                    }
                    if (member->subaggr.prefix != NULL)
                    {
                        sdl_free(member->subaggr.prefix);
                    }
                    if (member->subaggr.tag != NULL)
                    {
                        sdl_free(member->subaggr.tag);
                    }
                }
                else if (member->type == SDL_K_TYPE_COMMENT)
                {
                    if (member->item.id != NULL)
                    {
                        sdl_free(member->item.id);
                    }
                    if (member->item.prefix != NULL)
                    {
                        sdl_free(member->item.prefix);
                    }
                    if (member->item.tag != NULL)
                    {
                        sdl_free(member->item.tag);
                    }
                }
                else
                {
                    if (member->comment.comment != NULL)
                    {
                        sdl_free(member->comment.comment);
                    }
                }
                size = sizeof(SDL_MEMBERS);
                }
            break;

        case AggregateBlock:
            if (block != NULL)
            {
                SDL_AGGREGATE *aggr = (SDL_AGGREGATE *) block;
                SDL_MEMBERS *member;

                while (SDL_Q_EMPTY(&aggr->members) == false)
                {
                    SDL_REMQUE(&aggr->members, member);
                    sdl_deallocate_block(&member->header);
                }
                if (aggr->basedPtrName != NULL)
                {
                    sdl_free(aggr->basedPtrName);
                }
                if (aggr->id != NULL)
                {
                    sdl_free(aggr->id);
                }
                if (aggr->marker != NULL)
                {
                    sdl_free(aggr->marker);
                }
                if (aggr->prefix != NULL)
                {
                    sdl_free(aggr->prefix);
                }
                if (aggr->tag != NULL)
                {
                    sdl_free(aggr->tag);
                }
                size = sizeof(SDL_AGGREGATE);
            }
            break;

        case ParameterBlock:
            if (block != NULL)
            {
                SDL_PARAMETER *param = (SDL_PARAMETER *) block;

                if (param->comment != NULL)
                {
                    sdl_free(param->comment);
                }
                if (param->name != NULL)
                {
                    sdl_free(param->name);
                }
                if (param->typeName != NULL)
                {
                    sdl_free(param->typeName);
                }
                size = sizeof(SDL_PARAMETER);
            }
            break;

        case EntryBlock:
            if (block != NULL)
            {
                SDL_ENTRY *entry = (SDL_ENTRY *) block;
                SDL_PARAMETER *param;

                while (SDL_Q_EMPTY(&entry->parameters) == false)
                {
                    SDL_REMQUE(&entry->parameters, param);
                    sdl_deallocate_block(&param->header);
                }
                if (entry->alias != NULL)
                {
                    sdl_free(entry->alias);
                }
                if (entry->id != NULL)
                {
                    sdl_free(entry->id);
                }
                if (entry->linkage != NULL)
                {
                    sdl_free(entry->linkage);
                }
                if (entry->typeName != NULL)
                {
                    sdl_free(entry->typeName);
                }
                size = sizeof(SDL_ENTRY);
            }
            break;

        default:
            break;
    }
    free(block);
    _bytes_deallocated += size;

    /*
     * Trace the deallocated block, plus additional relevant information.
     */
    if (traceMemory == true)
    {
        printf("%s:%d:sdl_deallocate_block: ID = %d, size = %ld, "
                "address: 0x%016lx\n\tsdl_allocate_blk Calls: %ld\n"
                "\tsdl_deallocate_blk Calls: %ld\n\tsdl_strdup Calls: %ld\n"
                "\tsdl_calloc Calls: %ld\n\tsdl_realloc Calls: %ld\n"
                "\tsdl_free Calls: %ld\nBytes Allocated: %ld, "
                "Bytes Deallocated: %ld, Bytes Remaining %ld\n",
                __FILE__,
                __LINE__,
                blockID,
                size,
                (uint64_t) block,
                _allocate_calls,
                _deallocate_calls,
                _strdup_calls,
                _calloc_calls,
                _realloc_calls,
                _free_calls,
                _bytes_allocated,
                _bytes_deallocated,
                _bytes_allocated - _bytes_deallocated);
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * sdl_strdup
 *  This function is called to duplicate the memory associated with a string.
 *  We have this function so that we can keep track of allocated and
 *  deallocated memory.
 *
 * Input Parameters:
 *  string:
 *    A pointer to the string to be duplicated.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:           An error occurred allocating the buffer for the duplicated
 *                  string.
 *  !NULL:          A pointer to the duplicated string.
 */
char *sdl_strdup(const char *string)
{
    char *retVal;
    size_t length = sizeof(uint64_t) + 1;

    /*
     * Increment the call counter.
     */
    _strdup_calls++;

    /*
     * If the pointer to the string is NULL, then we are just going to create
     * a buffer to hold a zero length, null-terminated string.
     */
    if (string != NULL)
    length += strlen(string);

    /*
     * Allocate a buffer large enough for the supplied string to be copied.
     */
    retVal = calloc(length, 1);

    /*
     * If the allocate was successful, then copy the string into the new
     * buffer.
     */
    if (retVal != NULL)
    {
        uint64_t *bufLen = (uint64_t *) retVal;

        /*
         * Put the length of the allocated buffer in the buffer at the
         * beginning, then move the address we are returning to be just after
         * the length information.  We use a 64-bit length so that we do not
         * run into any alignment issues (we assume calloc always returns an
         * aligned buffer).
         */
        *bufLen = length;
        retVal += sizeof(uint64_t);

        /*
         * If a string was supplied, then duplicate the string into the return
         * buffer.  Otherwise, set the first byte to a null-terminator.
         */
        if (string != NULL)
        {
            strcpy(retVal, string);
        }
        else
        {
            retVal[0] = '\0';
        }
        _bytes_allocated += length;
    }

    /*
     * Trace the strdup, plus additional relevant information.
     */
    if (traceMemory == true)
    {
        printf("%s:%d:sdl_strdup: size = %ld, address: 0x%016lx\n"
               "\tsdl_allocate_blk Calls: %ld\n\tsdl_deallocate_blk Calls: %ld\n"
               "\tsdl_strdup Calls: %ld\n\tsdl_calloc Calls: %ld\n"
               "\tsdl_realloc Calls: %ld\n\tsdl_free Calls: %ld\n"
               "Bytes Allocated: %ld, Bytes Deallocated: %ld, "
               "Bytes Remaining %ld\n",
               __FILE__,
               __LINE__,
               length,
               (uint64_t) retVal,
               _allocate_calls,
               _deallocate_calls,
               _strdup_calls,
               _calloc_calls,
               _realloc_calls,
               _free_calls,
               _bytes_allocated,
               _bytes_deallocated,
               _bytes_allocated - _bytes_deallocated);
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * sdl_calloc
 *  This function is called to allocate a buffer of memory.  We have this
 *  function so that we can keep track of allocated and deallocated memory.
 *
 * Input Parameters:
 *  count:
 *    A value indicating the number of items to be allocated.
 *  size:
 *    A value indicating the size of each item to be allocated.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:    An error occurred allocating the buffer.
 *  !NULL:    A pointer to the buffer.
 */
void *sdl_calloc(size_t count, size_t size)
{
    void *retVal;
    size_t length = sizeof(uint64_t) + (count * size);

    /*
     * Increment the call counter.
     */
    _calloc_calls++;

    /*
     * Allocate a buffer large enough for the requested size plus the bit of
     * overhead we have.
     */
    retVal = calloc(length, 1);

    /*
     * If the allocate was successful, then set the length information, and
     * change the returned pointer to just after the length field.
     */
    if (retVal != NULL)
    {
        uint64_t *bufLen = (uint64_t *) retVal;

        /*
         * Put the length of the allocated buffer in the buffer at the
         * beginning, then move the address we are returning to be just after
         * the length information.  We use a 64-bit length so that we do not
         * run into any alignment issues (we assume calloc always returns an
         * aligned buffer).
         */
        *bufLen = length;
        retVal += sizeof(uint64_t);
        _bytes_allocated += length;
    }

    /*
     * Trace the calloc, plus additional relevant information.
     */
    if (traceMemory == true)
    {
        printf("%s:%d:sdl_calloc: size = %ld, address: 0x%016lx\n"
               "\tsdl_allocate_blk Calls: %ld\n\tsdl_deallocate_blk Calls: %ld\n"
               "\tsdl_strdup Calls: %ld\n\tsdl_calloc Calls: %ld\n"
               "\tsdl_realloc Calls: %ld\n\tsdl_free Calls: %ld\n"
               "Bytes Allocated: %ld, Bytes Deallocated: %ld, "
               "Bytes Remaining %ld\n",
               __FILE__,
               __LINE__,
               length,
               (uint64_t) retVal,
               _allocate_calls,
               _deallocate_calls,
               _strdup_calls,
               _calloc_calls,
               _realloc_calls,
               _free_calls,
               _bytes_allocated,
               _bytes_deallocated,
               _bytes_allocated - _bytes_deallocated);
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * sdl_realloc
 *  This function is called to reallocate a larger buffer of memory.  We have
 *  this function so that we can keep track of allocated and deallocated
 *  memory.  NOTE: If ptr is not NULL, it is returned to the free memory pool.
 *
 * Input Parameters:
 *  ptr:
 *    A pointer to the buffer to be reallocated.
 *  newSize:
 *    A value indicating the new size that should be allocated.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:           An error occurred allocating the buffer.
 *  !NULL:          A pointer to the buffer.
 */
void *sdl_realloc(void *ptr, size_t newSize)
{
    void *retVal;
    size_t length = sizeof(uint64_t) + newSize;

    /*
     * Increment the call counter.
     */
    _realloc_calls++;

    /*
     * Allocate a buffer large enough for the requested size plus the bit of
     * overhead we have.
     */
    retVal = calloc(length, 1);

    /*
     * If the allocate was successful, then set the length information, and
     * change the returned pointer to just after the length field.
     */
    if (retVal != NULL)
    {
        uint64_t *newBufLen = (uint64_t *) retVal;
        char *oldPtr = NULL;
        uint64_t *oldBufLen = NULL;
        size_t copyLen = 0;

        /*
         * If ptr is NULL, then there is nothing to copy, so set things up so
         * that we don't try.
         */
        if (ptr != NULL)
        {
            oldPtr = (char *) ptr - sizeof(uint64_t);
            oldBufLen = (uint64_t *) oldPtr;
            copyLen = *oldBufLen - sizeof(uint64_t);
        }

        /*
         * Put the length of the allocated buffer in the buffer at the
         * beginning, then move the address we are returning to be just after
         * the length information.  We use a 64-bit length so that we do not
         * run into any alignment issues (we assume calloc always returns an
         * aligned buffer).
         */
        *newBufLen = length;
        retVal += sizeof(uint64_t);

        /*
         * Now copy the contents of the old buffer to the new one, but only if
         * there is an old one.
         */
        if (ptr != NULL)
        {
            memcpy(retVal, ptr, copyLen);

            /*
             * Free the old buffer, as we no longer need it.
             */
            free(oldPtr);
            _bytes_deallocated += *oldBufLen;
        }
        _bytes_allocated += length;
    }

    /*
     * Trace the realloc, plus additional relevant information.
     */
    if (traceMemory == true)
    {
        printf("%s:%d:sdl_realloc: size = %ld, address: 0x%016lx\n"
               "\tsdl_allocate_blk Calls: %ld\n\tsdl_deallocate_blk Calls: %ld\n"
               "\tsdl_strdup Calls: %ld\n\tsdl_calloc Calls: %ld\n"
               "\tsdl_realloc Calls: %ld\n\tsdl_free Calls: %ld\n"
               "Bytes Allocated: %ld, Bytes Deallocated: %ld, "
               "Bytes Remaining %ld\n",
               __FILE__,
               __LINE__,
               length,
               (uint64_t) retVal,
               _allocate_calls,
               _deallocate_calls,
               _strdup_calls,
               _calloc_calls,
               _realloc_calls,
               _free_calls,
               _bytes_allocated,
               _bytes_deallocated,
               _bytes_allocated - _bytes_deallocated);
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * sdl_free
 *  This function is called to free a buffer of memory.  We have this function
 *  so that we can keep track of allocated and deallocated memory.
 *
 * Input Parameters:
 *  ptr:
 *    A pointer to a buffer to be deallocated.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void sdl_free(void *ptr)
{
    char *bufPtr;
    size_t length;

    /*
     * Increment the call counter.
     */
    _free_calls++;

    /*
     * If there is a buffer supplied on the call, then get the real beginning
     * of the buffer, where the length information is stored.  Then free the
     * buffer.
     */
    if (ptr != NULL)
    {
        bufPtr = (char *) ptr - sizeof(uint64_t);
        length = *((uint64_t *) bufPtr);
        free(bufPtr);
        _bytes_deallocated += length;
    }
    else
    {
        length = 0;
    }

    /*
     * Trace the free, plus additional relevant information.
     */
    if (traceMemory == true)
    {
        printf("%s:%d:sdl_free: size = %ld, address: 0x%016lx\n"
               "\tsdl_allocate_blk Calls: %ld\n\tsdl_deallocate_blk Calls: %ld\n"
               "\tsdl_strdup Calls: %ld\n\tsdl_calloc Calls: %ld\n"
               "\tsdl_realloc Calls: %ld\n\tsdl_free Calls: %ld\n"
               "Bytes Allocated: %ld, Bytes Deallocated: %ld, "
               "Bytes Remaining %ld\n",
               __FILE__,
               __LINE__,
               length,
               (uint64_t) ptr,
               _allocate_calls,
               _deallocate_calls,
               _strdup_calls,
               _calloc_calls,
               _realloc_calls,
               _free_calls,
               _bytes_allocated,
               _bytes_deallocated,
               _bytes_allocated - _bytes_deallocated);
    }

    /*
     * Return back to the caller.
     */
    return;
}
