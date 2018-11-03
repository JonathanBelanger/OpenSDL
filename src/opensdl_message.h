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
 *  This header file contains all the definitions required to display error
 *  messages.
 *
 * Revision History:
 *
 *  V01.000	08-OCT-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _OPENSDL_MESSAGES_H_
#define _OPENSDL_MESSAGES_H_

#define SYS_K_FACILITY	0
#define RMS_K_FACILITY	1
#define SDL_K_FACILITY	186

#define SDL_GET_SEVERITY(msg)	((msg)->severity)
#define SDL_GET_MESSAGE(msg)	((msg)->msg_no)
#define SDL_GET_FACILITY(msg)	((msg)->fac_no)
#define SDL_GET_INHIBIT(msg)	((msg)->inhib_msg)
#define SDL_GET_CONTROL(msg)	((msg)->control)

#define SDL_K_WARNING	0
#define SDL_K_SUCCESS	1
#define SDL_K_ERROR	2
#define SDL_K_INFO	3
#define SDL_K_SEVERE	4

/*
 * Successful SDL Error.
 */
#define SDL_NORMAL		0x00ba0001

/*
 * Fatal SDL Errors.
 */
#define SDL_SYNTABOVR		0x00ba000c
#define SDL_ABORT		0x00ba0014
#define SDL_INFILOPN		0x00ba001c
#define SDL_BUGCHECK		0x00ba0024
#define SDL_REVCHECK		0x00ba002c
#define SDL_ERREXIT		0x00ba0034

/*
 * Error SDL Errors.
 */
#define SDL_INVSHRIMG		0x00ba003a
#define SDL_NULLSTRUCT		0x00ba0042
#define SDL_DUPCONATT		0x00ba004a
#define SDL_INVFLDSIZ		0x00ba0052
#define SDL_UNDEFSYM		0x00ba005a
#define SDL_UNDEFCON		0x00ba0062
#define SDL_UNDEFUSER		0x00ba006a
#define SDL_INVDECL		0x00ba0072
#define SDL_UNDEFFIL		0x00ba007a
#define SDL_UNDEFORG		0x00ba0082
#define SDL_INVOUT		0x00ba008a
#define SDL_TOKOVF		0x00ba0092
#define SDL_OUTFILOPN		0x00ba009a
#define SDL_SYNTAXERR		0x00ba00a2
#define SDL_INTOVF		0x00ba00aa
#define SDL_ZERODIV		0x00ba00b2
#define SDL_BYTSIZ		0x00ba00ba
#define SDL_TOOMANYFIELDS	0x00ba00c2
#define SDL_INVUNKLEN		0x00ba00ca
#define SDL_SIZEREDEF		0x00ba00d2
#define SDL_SIZEQUAL		0x00ba00da
#define SDL_SIZENEST		0x00ba00e2
#define SDL_STRINGCONST		0x00ba00ea
#define SDL_INVNAME		0x00ba00f2
#define SDL_ILLFORWREF		0x00ba00fa
#define SDL_INVSYMDEF		0x00ba0102
#define SDL_SYMALRDEF		0x00ba010a
#define SDL_INVALIGN		0x00ba0112
#define SDL_INFILSDI		0x00ba011a
#define SDL_INVBITFLD		0x00ba0122
#define SDL_ADROBJBAS		0x00ba012a
#define SDL_INCDEFSTRUCT	0x00ba0132
#define SDL_MULTDEFSYM		0x00ba013a
#define SDL_INVPARMTYP		0x00ba0142
#define SDL_INVEXPR		0x00ba014a
#define SDL_INVLISTOPT  	0x00ba0152
#define SDL_BASEALIGN   	0x00ba015a

/*
 * Warning SDL Errors.
 */
#define SDL_LISFILOPN		0x00ba0160
#define SDL_NOOUTPUT		0x00ba0168
#define SDL_ZEROLEN		0x00ba0170
#define SDL_MATCHEND		0x00ba0178
#define SDL_WARNEXIT		0x00ba0180
#define SDL_TYPNAM		0x00ba0188
#define SDL_INVREQPARAM		0x00ba0190
#define SDL_NAMTRUNC		0x00ba0198
#define SDL_TYPNOTSUP		0x00ba01a0
#define SDL_IMMGTR32		0x00ba01a8
#define SDL_IDENTGTR31		0x00ba01b0
#define SDL_LANGDUP		0x00ba01b8
#define SDL_LANGMATCH		0x00ba01c0
#define SDL_LANGMISS		0x00ba01c8
#define SDL_UNALIGNED		0x00ba01d0
#define SDL_FILLNEGLEN		0x00ba01d8
#define SDL_OFFSETEXPR		0x00ba01e0
#define SDL_SYMNOTDEF		0x00ba01e8
#define SDL_BADNODETYPE		0x00ba0200
#define SDL_DIMENSIONSTAR	0x00ba0208

/*
 * Informational SDL Errors.
 */
#define SDL_NEGORIGIN		0x00ba01f3
#define SDL_FILLZEROLEN		0x00ba01fb
#define SDL_FIXUP		0x00ba0213
#define SDL_POSSCIRC		0x00ba021b

/*
 * Useful RMS Errors.
 */
#define RMS_EOF			0x0001827a
#define RMS_FNF			0x00018292
#define RMS_PRV			0x0001829a
#define RMS_NORMAL		0x00010001

/*
 * Useful SYS Errors.
 */
#define SYS_NORMAL		0x00000001
#define SYS_FATAL		0x00000004

typedef union
{
    uint32_t		msgCode;
    struct
    {
	uint32_t	severity : 3;
	uint32_t	cond_id : 25;
	uint32_t	inhib_msg: 1;
	uint32_t	control : 3;
    };
    struct
    {
	uint32_t	fill_3a : 3;
	uint32_t	msg_no : 13;
	uint32_t	fac_no : 12;
	uint32_t	fill_3b : 4;
    };
    struct
    {
	uint32_t	fill_4a : 3;
	uint32_t	code : 12;
	uint32_t	fac_sp : 1;
	uint32_t	fill_4 : 11;
	uint32_t	cust_def : 1;
	uint32_t	fill_4b : 4;
    };
} SDL_MESSAGE;

typedef struct
{
    char		*msgText;
    char		*msgSymb;
    uint16_t		faoStr;
    uint16_t		faoInt;
} SDL_MSG_ARRAY;

typedef struct
{
    SDL_MESSAGE		msgCode;
    uint16_t		faoCount;
    uint16_t		faoInfo;
} SDL_MSG_VECTOR;

typedef struct
{
    uint8_t		faoType;
    uint8_t		faoLength;
} SDL_MSG_FAO;

#define SDL_MSG_FAO_NUMBER	0
#define SDL_MSG_FAO_STRING	1

#endif	/* _OPENSDL_MESSAGES_H_ */
