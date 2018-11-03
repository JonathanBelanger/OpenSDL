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
    {"NORMAL", "Normal successful completion", 0, 0},
    {"SYNTABOVR", "Symbol table overflow", 0, 0},
    {"ABORT", "Fatal internal error. Unable to continue execution", 0, 0},
    {"INFILOPN", "Unable to open input file %s" , 1, 0},
    {"BUGCHECK", "Internal consistency failure [Line %d] - please submit a bug report", 0, 1},
    {"REVCHECK", "Front-end / back-end version mismatch.  Check installation.", 0, 0},
    {"ERREXIT", "Error exit", 0, 0},
    {"INVSHRIMG", "Sharable language image not found %s", 1, 0},
    {"NULLSTRUCT", "Null structure %s has no members [Line %d]", 1, 1},
    {"DUPCONATT", "Item %s has duplicate or conflicting attributes [Line %d]", 1, 1},
    {"INVFLDSIZ", "Item %s has bitfield length or offset greater than, 3},2 [Line %d]", 1, 1},
    {"UNDEFSYM", "Undefined local symbol %s used in expression [Line %d]", 1, 1},
    {"UNDEFCON", "Undefined constant name %s used in expression [Line %d]", 1, 1},
    {"UNDEFUSER", "Undefined user type name %s referenced [Line %d]", 1, 1},
    {"INVDECL", "Invalid DECLARE for type %s [Line %d]", 1, 1},
    {"UNDEFFIL", "Unable to open include file %s [Line %d]", 1, 1},
    {"UNDEFORG", "Definition of ORIGIN name %s not found in aggregate [Line %d]", 1, 1},
    {"INVOUT", "Invalid attributes for output language %s [Line %d]", 1, 1},
    {"TOLOVF", "Token exceeds maximum size of %s [Line %d]", 1, 1},
    {"OUTFILOPN", "Unable to open output file %s", 1, 0},
    {"SYNTAXERR", "Syntax error [Line %d]", 0, 1},
    {"INTOVF", "Integer overflow in expression [Line %d]", 0, 1},
    {"ZERODIV", "Zero divide in expression [Line %d]", 0, 1},
    {"BYTSIZ", "Aggregate %s must be integral byte size [Line %d]", 1, 1},
    {"TOOMANYFIELDS", "Structure %s has too many fields [Line %d]", 1, 1},
    {"INVUNKLEN", "Unknown length attribute valid only for parameter type [Line %d]", 0, 1},
    {"SIZEREFDEF", "Size or type of item %s redefined [Line %d]", 1, 1},
    {"SIZEQUAL", "Item %s, an aggregate, cannot be qualified by SIZEOF [Line %d]", 1, 1},
    {"SIZENEST", "Illegal nesting of SIZEOF clauses (Item %s) [Line %d]", 1, 1},
    {"STRINGCONST", "String constant %s used in arithmetic expression [Line %d]", 1, 1},
    {"INVNAME", "Item name is invalid", 0, 0},
    {"ILLFORWREF", "Illegal forward reference for output language %s [Line %d]", 1, 1},
    {"INVSYMDEF", "Invalid symbol %s specified in /SYMBOLS qualifier", 1, 0},
    {"SYMALRDEF", "Symbol %s was already defined in command line", 1, 0},
    {"INVALIGN", "Illegal value for /ALIGNMENT qualifier in command line", 0, 0},
    {"INFILSDI", "File format error reading intermediate file %s.  Possible version mismatch", 1, 0},
    {"INFBITFLD", "Invalid bitfield %s -- bitfields must be aggregate members [Line %d]", 1, 1},
    {"ADROBJBAS", "Address object %s must have based storage class [Line %d]", 1, 1},
    {"INCDEFSTRUCT", "Incompletely defined structure -- %s [Line %d]", 1, 1},
    {"MULTDEFSYM", "Multiply defined symbol -- %s [Line %d]", 1, 1},
    {"INVPARMTYP", "Invalid parameter type for language %s [Line %d]", 1, 1},
    {"INVEXPR", "Invalid expression -- cannot be resolved to a constant as required, %s [Line %d]", 1, 1},
    {"INVLISTOPT", "Invalid use of LIST attribute -- LIST may only appear on the last parameter.  %s [Line %d]", 1, 1},
    {"BASEALIGN", "Invalid expression with BASEALIGN option.  Value must be in range 0 to 124. %s [Line %d]", 1, 1},
    {"LISFILOPN", "Unable to open listing file %s", 1, 0},
    {"NOOUTPUT", "No language output produced", 0, 0},
    {"ZEROLEN", "Item %s has 0 or negative length [Line %d]", 1, 1},
    {"MATCHEND", "End name does not match declaration name %s [Line %d]", 1, 1},
    {"WARNEXIT", "Warning exit", 0, 0},
    {"TYPNAM", "Aggregate type name not supported [Line %d]", 0, 1},
    {"INVREQPARAM", "Required parameter encountered after optional parameter %s", 1, 0},
    {"NAMTRUNC", "Generated name too long - truncated to 64 characters %s", 1, 0},
    {"TYPNOTSUP", "Output language does not support data type %s [Line %d]", 1, 1},
    {"IMMCTR32", "Cannot pass values larger than 32 bits by immediate mechanism [Line %d]", 0, 1},
    {"IDENTGTR31", "SDL-generated identifier longer than 31 characters exceeds capacity of %s compiler [Line %d]", 1, 1},
    {"LANGDUP", "Language name %s appears more than once in list [Line %d]", 1, 1},
    {"LANGMATCH", "Language %s does not appear in list of matching IF statement [Line %d]", 1, 1},
    {"LANGMISS", "Language %s in list of matching IF statement missing from END list [Line %d]", 1, 1},
    {"UNALIGNED", "%s does not align on its natural boundary [Line %d]", 1, 1},
    {"FILLNEGLEN", "Fill item %s has a negative length [Line %d]", 1, 1},
    {"OFFSETEXPR", "Offset or origin relative expression involves a forward or circular reference. %s [Line %d]", 1, 1},
    {"SYMNOTDEF", "Symbol %s was not defined in command line, value zero assumed [Line %d]", 1, 1},
    {"NEGORIGIN", "Aggregate %s has a negative origin - negative offset elements will be ignored [Line %d]", 1, 1},
    {"FILLZEROLEN", "Fill item %s has a zero length [Line %d]", 1, 1},
    {"BADNODETYPE", "internal node type is unknown for language %s", 1, 0},
    {"DIMENSIONSTAR", "DIMENSION * for MEMBER \"%s\" has no known discriminant [Line %d]", 1, 1},
    {"FIXUP", "temporary hard-coded list used to discriminate for MEMBER \"%s\" [Line %d]", 1, 1},
    {"POSSCIRC", "possible circular definition for type %s [Line %d]", 1, 1}
};

#define RMS_MSG_BASE	79
static SDL_MSG_ARRAY rmsmsg[] =
{
    {"NORMAL", "Normal successful completion", 0, 0},
    {"EOF", "end of file detected", 0, 0},
    {NULL, NULL, 0, 0},
    {NULL, NULL, 0, 0},
    {"FNF", "file not found", 0, 0},
    {"PRV", "insufficient privilege or file protection violation", 0, 0}
};

#define SDL_K_FAC_SDL	0
#define SDL_K_FAC_RMS	1
#define SDL_K_FAC_SYM	2
static char *facilityName[] =
{
    "SDL",
    "RMS",
    "SYS"
};

static char severity[] = {'W', 'S', 'E', 'I', 'F'};

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
 *  SYS_NORMAL:	Normal successful completion.
 */
uint32_t sdl_set_message(SDL_MSG_VECTOR *msgVec, int msgCnt, ...)
{
    va_list		ap;
    SDL_MSG_VECTOR	*msgIdx = msgVec;
    SDL_MSG_FAO		*fao;
    char		*str, *ptr;
    int			ii, jj;
    uint32_t		retVal = SYS_NORMAL;
    uint16_t		strCnt, intCnt;

    va_start(ap, msgCnt);

    /*
     * Loop through the messages and message arguments provided on the call.
     */
    for (ii = 0; ii < msgCnt; ii++)
    {
	msgIdx->msgCode.msgCode = va_arg(ap, uint32_t);
	switch(msgIdx->msgCode.fac_no)
	{
	    case SDL_K_FACILITY:
		strCnt = sdlmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].faoStr;
		intCnt = sdlmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].faoInt;
		break;

	    case RMS_K_FACILITY:
		strCnt = rmsmsg[msgIdx->msgCode.msg_no - RMS_MSG_BASE].faoStr;
		intCnt = rmsmsg[msgIdx->msgCode.msg_no - RMS_MSG_BASE].faoInt;
		break;

	    case SYS_K_FACILITY:
	    default:
		strCnt = 0;
		intCnt = 0;
		break;
	}
	msgIdx->faoCount = strCnt + intCnt;
	fao = (SDL_MSG_FAO *) &msgIdx->faoInfo;
	for (jj = 0; jj < msgIdx->faoCount; jj++)
	{
	    ptr = (char *) fao + sizeof(SDL_MSG_FAO);
	    if (strCnt > 0)
	    {
		str = va_arg(ap, char *);
		strCnt--;
		fao->faoType = SDL_MSG_FAO_STRING;
		fao->faoLength = strlen(str);
		strncpy(ptr, str, fao->faoLength);
		ptr += fao->faoLength;
	    }
	    else
	    {
		fao->faoType = SDL_MSG_FAO_NUMBER;
		fao->faoLength = 0;
		*((uint32_t *) ptr) = va_arg(ap, uint32_t);
		ptr += sizeof(uint32_t);
	    }
	    fao = (SDL_MSG_FAO *) ptr;
	}
	msgIdx = (SDL_MSG_VECTOR *) ptr;
    }
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
 *  SYS_FATAL:	An error occurred trying to initialize the message vector.
 *  SYS_NORMAL:	Normal successful completion.
 */
uint32_t sdl_get_message(SDL_MSG_VECTOR *msgVec, char **msgStr)
{
    SDL_MSG_VECTOR	*msgIdx = msgVec;
    SDL_MSG_FAO		*fao;
    char		*msgFmt;
    char		*ptr, string, *facStr, *symbol;
    char		localMsgStr[SDL_LOCAL_MSG_LEN];
    int			ii;
    uint32_t		retVal = SYS_NORMAL;
    uint32_t		value;
    uint16_t		faoStr, faoInt, length;
    uint8_t		msgStrLen = 0;
    uint8_t		msgStrSize = 0;
    uint8_t		msgRemLen = 0;
    uint8_t		localMsgLen;
    char		first = '%';
    bool		done = false;

    *msgStr = NULL;

    /*
     * Loop through the message vector starting at the message code.
     */
    while ((done == false) && (retVal == 1))
    {

	/*
	 * Get the message text associated with the message code.
	 */
	switch (msgIdx->msgCode.fac_no)
	{
	    case SDL_K_FACILITY:
		symbol = sdlmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].msgSymb;
		msgFmt = sdlmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].msgText;
		faoStr = sdlmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].faoStr;
		faoInt = sdlmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].faoInt;
		facStr = facilityName[SDL_K_FAC_SDL];
		break;

	    case RMS_K_FACILITY:
		symbol = rmsmsg[msgIdx->msgCode.msg_no - SDL_MSG_BASE].msgSymb;
		msgFmt = rmsmsg[msgIdx->msgCode.msg_no - RMS_MSG_BASE].msgText;
		faoStr = rmsmsg[msgIdx->msgCode.msg_no - RMS_MSG_BASE].faoStr;
		faoInt = rmsmsg[msgIdx->msgCode.msg_no - RMS_MSG_BASE].faoInt;
		facStr = facilityName[SDL_K_FAC_RMS];
		break;

	    default:
		msgFmt = NULL;
		break;
	}

	/*
	 * Now we need to loop through each of the messages FAO arguments.
	 */
	fao = &msgIdx->faoCount;
	ptr = (char *) fao + sizeof(SDL_MSG_FAO);
	string = NULL;
	for (ii = 0;  ii < msgIdx->faoCount; ii++)
	{
	    switch(fao->faoType)
	    {
		case SDL_MSG_FAO_NUMBER:
		    value = *((uint32_t *) ptr);
		    ptr += sizeof(uint32_t);
		    break;

		case SDL_MSG_FAO_STRING:
		    length = fao->faoLength;
		    ptr += sizeof(uint8_t);
		    string = ptr;
		    ptr += length;
		    break;
	    }
	}

	/*
	 * If we have a string value, then the string is always first and the
	 * value may or may not be needed, but putting it here does no harm.
	 * If we do not have a string value, then we either have a value or
	 * don't need it, again no harm in supplying it.
	 */
	if (string != NULL)
	    localMsgLen = sprintf(localMsgStr, msgFmt, string, value);
	else
	    localMsgLen = sprintf(localMsgStr, msgFmt, value);

	/*
	 * Determine if there is enough space in the message string we are
	 * generating.  If not, we reallocate some more.
	 */
	msgRemLen = msgStrSize -
		    msgStrLen -
		    (strlen(facStr) + strlen(symbol) + localMsgLen + 7);
	if (msgRemLen < 0)
	{
	    msgStrSize += SDL_LOCAL_MSG_LEN;
	    *msgStr = realloc(*msgStr, msgStrSize);
	}

	/*
	 * If we have a place to put the string, then do so now.
	 */
	if (*msgStr != NULL)
	    msgStrLen += sprintf(
			    &msgStr[msgStrLen],
			    "%c%s-%c-%s, %s\n",
			    first,
			    facStr,
			    severity[msgIdx->msgCode.severity],
			    symbol,
			    localMsgStr);
	else
	    retVal = SYS_FATAL;
	msgIdx = (SDL_MSG_VECTOR *) ptr;
	done = (msgIdx->msgCode.msgCode == 0);
	first = '-';
    }

    /*
     * Return the results of this call back to the user.
     */
    return(retVal);
}
