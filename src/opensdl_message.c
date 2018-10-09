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
 *  This source file contains all the functions required to support reporting
 *  error messages.
 *
 * Revision History:
 *
 *  V01.000	09-OCT-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "opensdl_message.h"

#define SDL_LOCAL_MSG_LEN	256

/*
 * Message definitions, by facility.
 */
#define SDL_MSG_BASE	0
static SDL_MSG_ARRAY sdlmsg[] =
{
    {"Normal successful completion", 0, 0},
    {"Symbol table overflow", 0, 0},
    {"Fatal internal error. Unable to continue execution", 0, 0},
    {"Unable to open input file %s" , 1, 0},
    {"Internal consistency failure [Line %d] - please submit a bug report", 0, 1},
    {"Front-end / back-end version mismatch.  Check installation.", 0, 0},
    {"Error exit", 0, 0},
    {"Shareable language image not found %s", 1, 0},
    {"Null structure %s has no members [Line %d]", 1, 1},
    {"Item %s has duplicate or conflicting attributes [Line %d]", 1, 1},
    {"Item %s has bitfield length or offset greater than, 3},2 [Line %d]", 1, 1},
    {"Undefined local symbol %s used in expression [Line %d]", 1, 1},
    {"Undefined constant name %s used in expression [Line %d]", 1, 1},
    {"Undefined user type name %s referenced [Line %d]", 1, 1},
    {"Invalid DECLARE for type %s [Line %d]", 1, 1},
    {"Unable to open include file %s [Line %d]", 1, 1},
    {"Definition of ORIGIN name %s not found in aggregate [Line %d]", 1, 1},
    {"Invalid attributes for output language %s [Line %d]", 1, 1},
    {"Token exceeds maximum size of %s [Line %d]", 1, 1},
    {"Unable to open output file %s", 1, 0},
    {"Syntax error [Line %d]", 0, 1},
    {"Integer overflow in expression [Line %d]", 0, 1},
    {"Zero divide in expression [Line %d]", 0, 1},
    {"Aggregate %s must be integral byte size [Line %d]", 1, 1},
    {"Structure %s has too many fields [Line %d]", 1, 1},
    {"Unknown length attribute valid only for parameter type [Line %d]", 0, 1},
    {"Size or type of item %s redefined [Line %d]", 1, 1},
    {"Item %s, an aggregate, cannot be qualified by SIZEOF [Line %d]", 1, 1},
    {"Illegal nesting of SIZEOF clauses (Item %s) [Line %d]", 1, 1},
    {"String constant %s used in arithmetic expression [Line %d]", 1, 1},
    {"Item name is invalid", 0, 0},
    {"Illegal forward reference for output language %s [Line %d]", 1, 1},
    {"Invalid symbol %s specified in /SYMBOLS qualifier", 1, 0},
    {"Symbol %s was already defined in command line", 1, 0},
    {"Illegal value for /ALIGNMENT qualifier in command line", 0, 0},
    {"File format error reading intermediate file %s.  Possible version mismatch", 1, 0},
    {"Invalid bitfield %s -- bitfields must be aggregate members [Line %d]", 1, 1},
    {"Address object %s must have based storage class [Line %d]", 1, 1},
    {"Incompletely defined structure -- %s [Line %d]", 1, 1},
    {"Multiply defined symbol -- %s [Line %d]", 1, 1},
    {"Invalid parameter type for language %s [Line %d]", 1, 1},
    {"Invalid expression -- cannot be resolved to a constant as required, %s [Line %d]", 1, 1},
    {"Invalid use of LIST attribute -- LIST may only appear on the last parameter.  %s [Line %d]", 1, 1},
    {"Invalid expression with BASEALIGN option.  Value must be in range 0 to 124. %s [Line %d]", 1, 1},
    {"Unable to open listing file %s", 1, 0},
    {"No language output produced", 0, 0},
    {"Item %s has 0 or negative length [Line %d]", 1, 1},
    {"End name does not match declaration name %s [Line %d]", 1, 1},
    {"Warning exit", 0, 0},
    {"Aggregate type name not supported [Line %d]", 0, 1},
    {"Required parameter encountered after optional parameter %s", 1, 0},
    {"Generated name too long - truncated to 64 characters %s", 1, 0},
    {"Output language does not support data type %s [Line %d]", 1, 1},
    {"Cannot pass values larger than 32 bits by immediate mechanism [Line %d]", 0, 1},
    {"SDL-generated identifier longer than 31 characters exceeds capacity of %s compiler [Line %d]", 1, 1},
    {"Language name %s appears more than once in list [Line %d]", 1, 1},
    {"Language %s does not appear in list of matching IF statement [Line %d]", 1, 1},
    {"Language %s in list of matching IF statement missing from END list [Line %d]", 1, 1},
    {"%s does not align on its natural boundry [Line %d]", 1, 1},
    {"Fill item %s has a negative length [Line %d]", 1, 1},
    {"Offset or origin relative expression involves a forward or circular reference. %s [Line %d]", 1, 1},
    {"Symbol %s was not defined in command line, value zero assumed [Line %d]", 1, 1},
    {"Aggregate %s has a negative origin - negative offset elements will be ignored [Line %d]", 1, 1},
    {"Fill item %s has a zero length [Line %d]", 1, 1},
    {"internal node type is unknown for language %s", 1, 0},
    {"DIMENSION * for MEMBER \"%s\" has no known discriminant [Line %d]", 1, 1},
    {"temporary hardcoded list used to discriminate for MEMBER \"%s\" [Line %d]", 1, 1},
    {"possible circular definition for type %s [Line %d]", 1, 1}
};

#define RMS_MSG_BASE	79
static SDL_MSG_ARRAY rmsmsg[] =
{
   {"end of file detected", 0, 0},
   {NULL, 0, 0},
   {NULL, 0, 0},
   {"file not found", 0, 0},
   {"insufficient privilege or file protection violation", 0, 0}
};

/*
 * sdl_set_message
 *  This function is called to initialize a message vector with the supplied
 *  information.  Note, this function may be called with a variable number of
 *  arguments.
 *
 * Input Parameters
 *  msgVec:
 *	A pointer to the message vector, which is large enough to receive all
 *	the values needed to report a complete error.
 *  msgCnt:
 *	A value indicating the number of messages supplied on this function.
 *	NOTE: This is not necessarily the number of arguments on the call, as
 *	the FAO arguments are not counted in this value.
 *  ...:
 *	A variable number of message FAO arguments or the next message to be
 *	inserted into the message vector.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  0:	An error occurred trying to initialize the message vector.
 *  1:	Normal successful completion.
 */
int sdl_set_message(SDL_MSG_VECTOR *msgVec, int msgCnt, ...)
{
    va_list		ap;
    SDL_MSG_VECTOR	*msgIdx = msgVec;
    unsigned int	*faoPtr;
    int			ii, jj;
    int			retVal = 1;

    va_start(ap, msgCnt);

    /*
     * Loop through the messages and message arguments provided on the call.
     * TODO: This code needs help with message arguments that are strings.  We
     * TODO: may want to insert a counted string into the message vector (so
     * TODO: that we don't have issues with deallocated memory).
     */
    for (ii = 0; ii < msgCnt; ii++)
    {
	msgIdx->msgCode.msgCode = va_arg(ap, unsigned int);
	switch(msgIdx->msgCode.facility)
	{
	    case SDL_K_FACILITY:
		msgIdx->faoCount =
			sdlmsg[msgIdx->msgCode.message - SDL_MSG_BASE].faoStr +
			sdlmsg[msgIdx->msgCode.message - SDL_MSG_BASE].faoInt;
		break;

	    case RMS_K_FACILITY:
		msgIdx->faoCount =
			rmsmsg[msgIdx->msgCode.message - RMS_MSG_BASE].faoStr +
			rmsmsg[msgIdx->msgCode.message - RMS_MSG_BASE].faoInt;
		break;

	    default:
		msgIdx->faoCount = 0;
		break;
	}
	faoPtr = &msgIdx->faoCount;
	faoPtr++;
	for (jj = 0; jj < msgIdx->faoCount; jj++)
	{
	    faoPtr = va_arg(ap, unsigned int);
	    faoPtr++;
	}
	msgIdx = (SDL_MSG_VECTOR *) faoPtr;
    }
    msgIdx->faoCount = 0;
    msgIdx->msgCode.msgCode = 0;
    va_end(ap);

    /*
     * Return the results of this call back to the user.
     */
    return(retVal);
}

/*
 * sdl_get_message
 *  This function is called to scan a message vector and generate a string
 *  with the message text.
 *
 * Input Parameters
 *  msgVec:
 *	A pointer to the message vector.
 *
 * Output Parameters:
 *  msgStr:
 *	A pointer to a location to receive the message text.
 *
 * Return Values:
 *  0:	An error occurred trying to initialize the message vector.
 *  1:	Normal successful completion.
 */
int sdl_get_message(SDL_MSG_VECTOR *msgVec, char **msgStr)
{
    SDL_MSG_VECTOR	*msgIdx = msgVec;
    unsigned int	*faoPtr;
    char		*msgFmt;
    char		localMsgStr[SDL_LOCAL_MSG_LEN];
    int			retVal = 1;
    int			faoStr, faoInt;
    int			msgStrLen = 0;
    int			msgStrSize = 0;
    int			msgRemLen = 0;
    int			localMsgLen;
    bool		done = false;

    *msgStr = NULL;

    while ((done == false) && (retVal == 1))
    {
	switch (msgIdx->msgCode.facility)
	{
	    case SDL_K_FACILITY:
		msgFmt = sdlmsg[msgIdx->msgCode.message - SDL_MSG_BASE].msgText;
		faoStr = sdlmsg[msgIdx->msgCode.message - SDL_MSG_BASE].faoStr;
		faoInt = sdlmsg[msgIdx->msgCode.message - SDL_MSG_BASE].faoInt;
		break;

	    case RMS_K_FACILITY:
		msgFmt = rmsmsg[msgIdx->msgCode.message - RMS_MSG_BASE].msgText;
		faoStr = rmsmsg[msgIdx->msgCode.message - RMS_MSG_BASE].faoStr;
		faoInt = rmsmsg[msgIdx->msgCode.message - RMS_MSG_BASE].faoInt;
		break;

	    default:
		msgFmt = NULL;
		break;
	}
	faoPtr = &msgIdx->faoCount;
	faoPtr++;
	switch(msgIdx->faoCount)
	{
	    case 1:
		if (faoStr == 1)
		{
		    char *string = *(faoPtr++);

		    sprintf(localMsgStr, msgFmt, string);
		}
		else
		{
		    unsigned int value = *(faoPtr++);

		    sprintf(localMsgStr, msgFmt, value);
		}
		break;

	    case 2:
		if (faoStr == 2)
		{
		    char *string1 = *(faoPtr++);
		    char *string2 = *(faoPtr++);

		    sprintf(localMsgStr, msgFmt, string1, string2);
		}
		else if (faoStr == 1)
		{
		    char *string = *(faoPtr++);
		     unsigned int value = *(faoPtr++);

		    sprintf(localMsgStr, msgFmt, string, value);
		}
		else
		{
		    unsigned int value1 = *(faoPtr++);
		    unsigned int value2 = *(faoPtr++);

		    sprintf(localMsgStr, msgFmt, value1, value2);
		}
		break;

	    default:
		localMsgStr[0] = '\0';
		faoPtr++;
		break;
	}
	localMsgLen = strlen(localMsgStr);
	msgRemLen = msgStrSize - msgStrLen - localMsgLen - 1;

	if (msgRemLen < 0)
	{
	    msgStrSize += SDL_LOCAL_MSG_LEN;
	    *msgStr = realloc(*msgStr, msgStrSize);
	}
	if (*msgStr != NULL)
	{
	    strcpy(msgStr[msgStrLen], localMsgStr);
	    msgStrLen = strlen(*msgStr);
	}
	else
	    retVal = 0;
	msgIdx = (SDL_MSG_VECTOR *) faoPtr;
	done = ((msgIdx->faoCount == 0) && (msgIdx->msgCode.msgCode == 0));
    }

    /*
     * Return the results of this call back to the user.
     */
    return(retVal);
}
