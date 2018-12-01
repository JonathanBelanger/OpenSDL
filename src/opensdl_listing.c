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
 *  This source file contains the routines to generate a listing file as the
 *  input file is parsed.  There are 4 interface functions:
 *
 *	1) sdl_open_list	- open a new listing file for write
 *	2) sdl_write_list	- write a string of input data to listing file
 *	3) sdl_write_err	- write an error message to the listing file
 *	4) sdl_close_list	- close the listing file
 *
 * Revision History:
 *
 *  V01.000	25-AUG-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opensdl_defs.h"
#include "opensdl_message.h"
#include "opensdl_blocks.h"
#include "opensdl_main.h"

/*
 * Local variables used while generating a listing file.
 */
typedef char		*SDL_MESSAGE_TXT;
static SDL_MESSAGE_TXT	*messages = NULL;
static int		messagesIndex = 0;
static int		messagesSize = 0;
#define SDL_PAGE_WIDTH	132
#define SDL_PAGE_LENGTH	66
#define SDL_PAGE_LOC	122
typedef char		SDL_HEADER_DEF[SDL_PAGE_WIDTH + 1];
static SDL_HEADER_DEF	sdl_listing_header[2];
static char		xBuf[SDL_PAGE_WIDTH + 1];
static int		xBufLoc = 0;
static uint32_t		listLine = 1;
static uint32_t		pageLine = 1;
static uint32_t		pageNo = 1;

/*
 * Local Prototypes.
 */
static void _sdl_end_page(FILE *fp);
static void _sdl_msg_list(FILE *fp);

/*
 * sdl_open_list
 *  This function is called to open the listing file for write, and initialize
 *  the listing header information (which is placed at the top of each page).
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context block where all the information about the
 *	processing of the input file is maintained, as well as the listing
 *	output file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:	An error occurred opening the listing file for write.
 *  !NULL:	A pointer to the listing file.
 */
FILE *sdl_open_list(SDL_CONTEXT *context)
{
    FILE	*retVal;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_open_list\n", __FILE__, __LINE__);

    /*
     * Open the listing file for write.
     */
    retVal = fopen(context->listingFileName, "w");

    /*
     * If the listing file was successfully opened, then perform some
     * additional initialization.
     */
    if (retVal != NULL)
    {
	int	ii, len;

	/*
	 * Save the file pointer for the listing file in the context block.
	 */
	context->listingFP = retVal;

	/*
	 * Initialize the first line for the header written at the start of
	 * each listing page.  The first line contains the date and time the
	 * opensdl utility was executed, the name and version of the utility
	 * itself, and the page number.
	 */
	sprintf(sdl_listing_header[0],
		"%*s%02d-%s-%04d %02d:%02d:%02d OpenSDL %c%d.%d-%d",
		58,
		"",
		context->runTimeInfo.tm_mday,
		sdl_months[context->runTimeInfo.tm_mon],
		(1900 + context->runTimeInfo.tm_year),
		context->runTimeInfo.tm_hour,
		context->runTimeInfo.tm_min,
		context->runTimeInfo.tm_sec,
		SDL_K_VERSION_TYPE,
		SDL_K_VERSION_MAJOR,
		SDL_K_VERSION_MINOR,
		SDL_K_VERSION_LEVEL);
	len = strlen(sdl_listing_header[0]);
	for (ii = len; ii < SDL_PAGE_LOC; ii++)
	    sdl_listing_header[0][ii] = ' ';
	strcpy(&sdl_listing_header[0][SDL_PAGE_LOC], "Page ");

	/*
	 * Initialize the second line for the header written at the start of
	 * each listing page.  The second line contains the modify date and
	 * time for the input file and the fill path of the input file.
	 */
	sprintf(sdl_listing_header[1],
		"%*s%02d-%s-%04d %02d:%02d:%02d %.*s",
		58,
		"",
		context->inputTimeInfo.tm_mday,
		sdl_months[context->inputTimeInfo.tm_mon],
		(1900 + context->inputTimeInfo.tm_year),
		context->inputTimeInfo.tm_hour,
		context->inputTimeInfo.tm_min,
		context->inputTimeInfo.tm_sec,
		53,
		context->inputPath);
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * sdl_write_list
 *  This function is called by the Flex lexical analyzer any time a rule is
 *  about to be called.  This allows the read in code to be written out to the
 *  listing file.  Note, we may receive multiple lines to be written out to
 *  the listing file, or not a complete line.  We use the newline character
 *  to determine this, ignore the carriage-return character, and use the
 *  form-feed character to move to the next page.
 *
 * Input Parameters:
 *  fp:
 *	The address of the file pointer associated with the listing file.
 *  buf:
 *	A pointer to the buffer of characters to be written out to the listing
 *	file.
 *  len:
 *	A value indicating the size of the buf parameter.  When this is set to
 *	0, then get it from the strlen call.  Otherwise, we have a buffer that
 *	may not be null terminated, so don't assume it is.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void sdl_write_list(FILE *fp, char *buf, size_t len)
{
    int		ii;
    size_t	myLen = (len == 0) ? strlen(buf) : len;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_write_list\n", __FILE__, __LINE__);

    /*
     * Scan through the buffer, outputting lines as we go.  NOTE: We may end up
     * with a partial output line.  If this is the case, then we'll be called
     * again with more to output.
     */
    for (ii = 0; ii < myLen; ii++)
    {

	/*
	 * If we are on the first line of a page, then display the listing headers.
	 */
	if (pageLine == 1)
	    _sdl_end_page(fp);

	/*
	 * If we are starting a new line, then insert the line number.
	 */
	if (xBufLoc == 0)
	    xBufLoc = sprintf(xBuf, " %6d ", listLine);

	/*
	 * If the character is a carriage-return, then just ignore it.
	 */
	if (buf[ii] == '\r')
	    continue;

	/*
	 * If the character is a new-line, then null-terminate the output
	 * buffer and output the line.
	 */
	if (buf[ii] == '\n')
	{
	    xBuf[xBufLoc] = '\0';
	    fprintf(fp, "%s\n", xBuf);
	    listLine++;
	    pageLine++;
	    xBufLoc = 0;
	    if (messagesIndex > 0)
		_sdl_msg_list(fp);
	    continue;
	}

	/*
	 * If the character is a form-feed, or we have exceeded the number of
	 * lines in a page, then insert a form-feed and reset the pageLine.
	 */
	if ((buf[ii] == '\f') || (pageLine > SDL_PAGE_LENGTH))
	{
	    pageLine = 1;
	    continue;
	}

	/*
	 * We have just a regular character.  If there is room, then add it to
	 * the output buffer.  Otherwise, just swallow it.
	 */
	if (xBufLoc < SDL_PAGE_WIDTH)
	    xBuf[xBufLoc++] = buf[ii];
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * sdl_write_err
 *  This function is called by the Bison parser any time an error is returned.
 *  This allows errors to be emitted to the output file immediately after the
 *  line of code that generated it.  Note:, if there is a partial line waiting
 *  to be written, then the error text is held until the new-line or form-feed
 *  is detected.  This also means that it is possible to get more than one
 *  error for any particular line of code.
 *
 * Input Parameters:
 *  fp:
 *	The address of the file pointer associated with the listing file.
 *  msgText:
 *	A pointer to the string containing all the information needed to
 *	display the error text.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void sdl_write_err(FILE *fp, char *msgText)
{

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_write_err\n", __FILE__, __LINE__);

    /*
     * If we are in the process of producing a listing line, then store the
     * message text to be used later.
     */
    if (messagesIndex >= messagesSize)
    {
	size_t	size = sizeof(SDL_MESSAGE_TXT) * (messagesSize + 1);

	messages = sdl_realloc(messages, size);;
	if (messages != NULL)
	    messagesSize++;
    }
    messages[messagesIndex++] = sdl_strdup(msgText);

    /*
     * If we are already at the beginning of a line, then we can display the
     * message text now.
     */
    if (xBufLoc <= 8)
	_sdl_msg_list(fp);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * sdl_close_list
 *  This function is called to close the listing file.  If there is any output
 *  that has not yet been written out, then do so before closing the file.
 *
 * Input Parameters:
 *  context:
 *	A pointer to the context block where all the information about the
 *	processing of the input file is maintained, as well as the listing
 *	output file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void sdl_close_list(SDL_CONTEXT *context)
{

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:sdl_close_list\n", __FILE__, __LINE__);

    /*
     * If there is anything in the output buffer, write it out now.
     */
    if (xBufLoc > 0)
    {
	xBuf[xBufLoc] = '\0';
	fprintf(context->listingFP, "%s\n", xBuf);
    }

    /*
     * All that is left to do is close the file, if it was opened.
     */
    if (context->listingFP != NULL)
    {
	fclose(context->listingFP);
	context->listingFP = NULL;
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * _sdl_end_page
 *  This function is called to end the current page and start the next.
 *
 * Input Parameters:
 *  fp:
 *	The address of the file pointer associated with the listing file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void _sdl_end_page(FILE *fp)
{

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_end_page\n", __FILE__, __LINE__);

    /*
     * For the first line of the file, we do not write a form-feed.  All other
     * times, we need a form-feed, so that we can move to the next page.
     */
    if (listLine > 1)
	fprintf(fp, "\f\n");

    /*
     * Print the header at the top of each page.
     */
    fprintf(fp, "%s%4d\n", sdl_listing_header[0], pageNo++);
    pageLine++;
    fprintf(fp, "%s\n", sdl_listing_header[1]);
    pageLine++;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * _sdl_msg_list
 *  This function is called to display the message text associated with an
 *  input file where we are also generating a listing file.  We want to display
 *  the error message as close to the line where it was detected as possible.
 *  We also need to take into account where we are on the page of the listing
 *  file, and insert a page break along with the page headers, as needed.
 *
 * Input Parameters:
 *  fp:
 *	The address of the file pointer associated with the listing file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void _sdl_msg_list(FILE *fp)
{
    int		ii, jj, len, pageRoom, msgLines;

    /*
     * If tracing is turned on, write out this call (calls only, no returns).
     */
    if (trace == true)
	printf("%s:%d:_sdl_msg_list\n", __FILE__, __LINE__);

    /*
     * Determine if there is enough room to display the message in it's
     * entirety.  Each message in the messages array is considered separately.
     */
    pageRoom = SDL_PAGE_LENGTH - pageLine + 1;

    /*
     * Loop through each of the messages.
     */
    for (ii = 0; ii < messagesIndex; ii++)
    {

	/*
	 * Get the length of this specific message text.
	 */
	len = strlen(messages[ii]);

	/*
	 * Determine the number of lines this message occupies by counting the
	 * new-line characters.
	 */
	msgLines = 0;
	for (jj = 0; jj < len; jj++)
	    if (messages[ii][jj] == '\n')
		msgLines++;

	/*
	 * If there is not enough room on the current page for the message
	 * text, then move to the next page.  This will reset the pageLine, so
	 * we need to recalculate the pageRoom.
	 */
	if (pageRoom < msgLines)
	{
	    _sdl_end_page(fp);
	    pageRoom = SDL_PAGE_LENGTH - pageLine + 1;
	}

	/*
	 * One way or another, we have enough room for the message, so write it
	 * out to the listing file, and take into consideration the number of
	 * lines just written out.
	 */
	fprintf(fp, "%s", messages[ii]);
	pageRoom -= msgLines;
	pageLine += msgLines;

	/*
	 * We no longer need the memory associated with this message text, so
	 * return it.
	 */
	sdl_free(messages[ii]);
	messages[ii] = 0;
    }

    /*
     * Reset the message index.
     */
    messagesIndex = 0;

    /*
     * Return back to the caller.
     */
    return;
}
