/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: unichar.h,v 1.1.2.2 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
 *          Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Alternatively you can contact IDM <skcontact @at@ idm> for other license
 * contracts concerning parts of the code owned by IDM.
 *
 *****************************************************************************/
/* END LICENSE */

#ifndef __SKC_UNICHAR_H_
#define __SKC_UNICHAR_H_

// Char conversion routines
SKAPI SKERR CharUTF8ToUCS4(PRUint32* plOut, const char* pcIn, PRUint32* iEaten);
SKAPI SKERR CharUCS4ToUTF8(char* pcOut, const PRUint32 lIn, PRUint32* iWritten);

// string conversion routines
SKAPI SKERR UTF8LenToUCS4Buffer(PRUint32** pplOutBuffer,
        PRUint32* plOutBufferLength, PRUint32* plOutWrittenLength,
        const char* pcIn, PRUint32 lInLength, PRBool bAddNullChar);

SKAPI SKERR UCS4LenToUTF8Buffer(char** ppcOutBuffer,
        PRUint32* plOutBufferLength, PRUint32* plOutWrittenLength,
        const PRUint32* plIn, PRUint32 lInLength, PRBool bAddNullChar);

SKAPI SKERR UTF8LenToUCS4Len(   PRUint32* plOut, PRUint32* plOutLength,
                                const char* pcIn, PRUint32* plInLength);

SKAPI SKERR UCS4LenToUTF8Len(   char* pcOut, PRUint32* plOutLength,
                                const PRUint32* plIn, PRUint32* plInLength);

SKAPI PRUint32 UTF8LenToNewUCS4(PRUint32** pplOut, const char* pcIn, 
                                    PRUint32 iLen);
SKAPI PRUint32 UCS4LenToNewUTF8(char ** ppcOut, const PRUint32 * plIn, 
                                    PRUint32 iLen);

SKAPI PRUint32 UTF8ToNewUCS4(PRUint32** pplOut, const char * pcIn);
SKAPI PRUint32 UCS4ToNewUTF8(char ** ppcOut, const PRUint32 * plIn);

SKAPI void UCS4Free(PRUint32* ptr);
SKAPI void UTF8Free(char* ptr);

// basic manipulation on UCS4 strings
SKAPI PRUint32 UCS4strlen(const PRUint32* pl);
SKAPI PRUint32* UCS4strdup(const PRUint32* pl);
SKAPI PRUint32* UCS4strndup(const PRUint32* pl, PRUint32 lSize);
SKAPI void UCS4strinvert(PRUint32* pl);
SKAPI void UTF8strinvert(char* pl);

SKAPI PRBool UCS4StartsWith(const PRUint32* plBig, const PRUint32* plSmall);

/*
SKAPI void UCS4EquivalentBeginning(PRUint32 lFlags, 
                                   const PRUint32* plString1, PRUint32* plLen1,
                                   const PRUint32* plString2, PRUint32* plLen2);

//SKAPI PRInt32 UCS4CompareChar(PRUint32 lFlags, PRUint32 l1, PRUint32 l2);

SKAPI PRInt32 UCS4CompareString(PRUint32 lFlags,
                                const PRUint32 *pl1, const PRUint32 *pl2);

*/
#else // __SKC_UNICHAR_H_
#error "Multiple inclusions of unichar.h"
#endif // __SKC_UNICHAR_H_

