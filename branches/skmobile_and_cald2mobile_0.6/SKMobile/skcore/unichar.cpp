/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: unichar.cpp,v 1.1.2.2 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
 *          Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
 *          Alexis Seigneurin <seigneurin @at@ idm .dot. fr>
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

#include <nspr/nspr.h>
#include <nspr/plstr.h>

#include "skcoreconfig.h"

#include "machine.h"
#include "error.h"

#include "unichar.h"

//PRInt32 UCS4CompareChar(PRUint32 lFlags, PRUint32 l1, PRUint32 l2);

SKERR CharUTF8ToUCS4(PRUint32* plOut, const char* pcIn, PRUint32* iEaten)
{
    PRUint32 lResult=0;
    PRUint32 lLength=0;
    const unsigned char* pcInput = (const unsigned char*) pcIn;

    if(pcInput[0] < 0x80)
        lLength = 1;
    else if(pcInput[0] < 0xc0)
    {
        lLength = 1;
        lResult = (PRUint32) 'X';
        *plOut = lResult;
        *iEaten = lLength;
        return err_failure;
    }
    else if(pcInput[0] < 0xe0)
        lLength = 2;
    else if(pcInput[0] < 0xf0)
        lLength = 3;
    else if(pcInput[0] < 0xf8)
        lLength = 4;
    else if(pcInput[0] < 0xfc)
        lLength = 5;
    else
        lLength = 6;

    if(lLength == 1)
        lResult = pcInput[0];
    else
    {
        PRUint32 i;
        // get high-order bits
        lResult = pcInput[0] & ((1 << (7 - lLength)) - 1);
        // get low-order bits
        for(i=1 ; i<lLength ; i++)
        {
            if((pcInput[i] & 0xc0) != 0x80)
            {
                *plOut = (PRUint32) 'X';
                *iEaten = i;
                return err_failure;
            }
            lResult <<= 6;
            lResult |= pcInput[i] & 0x3f;
        }
    }
    *plOut = lResult;
    *iEaten = lLength;
    return noErr;
}

SKERR CharUCS4ToUTF8(char* pcOut, const PRUint32 lIn, PRUint32* iWritten)
{
    PRUint32 lLength=0;
    PRUint32 lInput = lIn;

    // check output string
    if(pcOut == NULL)
        return err_invalid;

    // calaculates the number of bytes required
    if(lInput < 0x00000080)
        lLength = 1;
    else if(lInput < 0x00000800)
        lLength = 2;
    else if(lInput < 0x00010000)
        lLength = 3;
    else if(lInput < 0x00200000)
        lLength = 4;
    else if(lInput < 0x04000000)
        lLength = 5;
    else
        lLength = 6;

    // fills-in high-order bits of the bytes
    if(lLength == 1)
        pcOut[0] = 0;
    else
    {
        PRUint32 i;
        // first byte
        pcOut[0] = 0xFF - ((1 << (8 - lLength)) - 1);
        // following bytes
        for(i=1 ; i < lLength ; i++)
            pcOut[i] = (char) 0x80;
    }

    // fills-in low-order bits of the bytes
    if(lLength == 1)
        pcOut[0] |= lInput & 0x0000007F;
    else
    {
        PRUint32 i;
        // last bytes
        for(i=lLength-1 ; i>=1 ; i--)
        {
            pcOut[i] |= lInput & 0x3F;
            lInput >>= 6;
        }
        // first byte
        pcOut[0] |= lInput & ((1 << (7 - lLength)) - 1);
    }
    pcOut[lLength] = '\0';
    *iWritten = lLength;
    return noErr;
}

SKAPI SKERR UTF8LenToUCS4Len(   PRUint32* plOut, PRUint32* plOutLength,
                                const char* pcIn, PRUint32* plInLength)
{
    SKERR err = noErr;
    PRUint32 iWritten = 0;
    PRUint32 iRemaining = *plInLength;
    while(err == noErr && iRemaining && *pcIn)
    {
        PRUint32 iTmpChar;
        PRUint32 iCharLength = 0;
        err = CharUTF8ToUCS4(&iTmpChar, pcIn, &iCharLength);
        // FIXME : err is silently ignored. don't realy know what to do with it
        // probably need another parameter to know.
        if(iWritten == *plOutLength)
            return err_memory;
        plOut[iWritten++] = iTmpChar;
        iRemaining -= iCharLength;
        pcIn += iCharLength;
    }
    *plInLength = *plInLength - iRemaining;
    *plOutLength = iWritten;
    return noErr;
}

SKAPI SKERR UCS4LenToUTF8Len(   char* pcOut, PRUint32* plOutLength,
                                const PRUint32* plIn, PRUint32* plInLength)
{
    SKERR err = noErr;
    PRUint32 iWritten = 0;
    PRUint32 iRemaining = *plInLength;
    while(err == noErr && iRemaining && *plIn)
    {
        char pcTmp[10];
        PRUint32 iCharLength = 9;
        err = CharUCS4ToUTF8(pcTmp, *plIn++, &iCharLength);
        // Conversion cannot fail this way
        SK_ASSERT(err == noErr);
        char * pc = pcTmp;
        if(iWritten + iCharLength > *plOutLength)
            return err_memory;
        while(*pc)
            pcOut[iWritten++] = *pc++;
        iRemaining--;
    }
    *plInLength = *plInLength - iRemaining;
    *plOutLength = iWritten;
    return noErr;
}

PRUint32 UTF8LenToNewUCS4(PRUint32 ** pplOut, const char * pcIn,
                                    PRUint32 lInputSize)
{
    PRUint32 lBufferSize = lInputSize + 1;
    PRUint32 lBufferUsed = lInputSize;
    *pplOut = (PRUint32*) PR_Malloc((lBufferSize) * sizeof(PRUint32));
    SK_ASSERT(*pplOut);
    UTF8LenToUCS4Len(*pplOut, &lBufferUsed, pcIn, &lInputSize);
    (*pplOut)[lBufferUsed] = 0;
    SK_ASSERT(UCS4strlen(*pplOut) == lBufferUsed);
    return lBufferUsed;
}

PRUint32 UCS4LenToNewUTF8(char ** ppcOut, const PRUint32 * plIn,
                                    PRUint32 lInputSize)
{
    // This function could be optimized in two ways :
    // - Better first evaluation for lBufferSize (this one is too big)
    // - playing with pointer after Realloc in order to avoid to retranslate
    //   the whole string
    SKERR err = err_memory;
    PRUint32 lBufferSize = lInputSize + 1;
    PRUint32 lWritten;
    *ppcOut = 0;
    while(err == err_memory)
    {
        if(*ppcOut)
            lBufferSize = (PRUint32)((lBufferSize + lBufferSize / 5) + 1);
        *ppcOut = (char*) PR_Realloc(*ppcOut, (lBufferSize) * sizeof(char));
        lWritten = lBufferSize - 1;
        PRUint32 lRead = lInputSize;
        err = UCS4LenToUTF8Len(*ppcOut, &lWritten, plIn, &lRead);
        (*ppcOut)[lWritten] = 0;
    }
    return lWritten;
}

SKAPI PRUint32 UTF8ToNewUCS4(PRUint32** pplOut, const char* pcIn)
{
    return UTF8LenToNewUCS4(pplOut, pcIn, PL_strlen(pcIn));
}

PRUint32 UCS4ToNewUTF8(char ** ppcOut, const PRUint32 * plIn)
{
    return UCS4LenToNewUTF8(ppcOut, plIn, UCS4strlen(plIn));
}

void UCS4Free(PRUint32* ptr)
{
    PR_Free(ptr);
}

void UTF8Free(char * ptr)
{
    PR_Free(ptr);
}

PRUint32 UCS4strlen(const PRUint32* pl)
{
    PRUint32 l = 0;
    while(*pl) { pl++; l++; }
    return l;
}

PRUint32* UCS4strdup(const PRUint32* pl)
{
    return UCS4strndup(pl, UCS4strlen(pl));
}

PRUint32* UCS4strndup(const PRUint32* pl, PRUint32 lSize)
{
    PRUint32 *plResult = (PRUint32*)
        PR_Malloc(sizeof(PRUint32) * (lSize + 1));
    if(!plResult)
        return plResult;
    PRUint32 *plPos=plResult;
    while(lSize--)
        *(plPos++) = *(pl++);
    *plPos = 0;
    return plResult;
}

void UTF8strinvert(char* pcString)
{
    PRUint32 lSize = PL_strlen(pcString);
    PRUint32 i;
    for (i = 0; i < lSize / 2; i++)
    {
        char cTmp = pcString[i];
        pcString[i] = pcString[lSize - (i+1)];
        pcString[lSize - (i+1)] = cTmp;
    }
}

void UCS4strinvert(PRUint32* lString)
{
    PRUint32 lSize = UCS4strlen(lString);
    PRUint32 i;
    for (i = 0; i < lSize / 2; i++)
    {
        PRUint32 lTmp = lString[i];
        lString[i] = lString[lSize - (i+1)];
        lString[lSize - (i+1)] = lTmp;
    }
}

PRBool UCS4StartsWith(const PRUint32* plBig, const PRUint32* plSmall)
{
    while(*plSmall)
    {
        if(!(*plBig) || *(plBig++) != *(plSmall++))
            return PR_FALSE;
    }
    return PR_TRUE;
}

void UCS4EquivalentBeginning(   PRUint32 lFlags,
                                const PRUint32* plString1, PRUint32* plLen1,
                                const PRUint32* plString2, PRUint32* plLen2)
{
    *plLen1 = 0;
    *plLen2 = 0;
    return;
}

#if 0



PRInt32 UCS4CompareString(PRUint32 lFlags,
                          const PRUint32 *pl1, const PRUint32 *pl2)
{
    while(*pl1 && *pl2)
    {
        PRUint32 res = UCS4CompareChar(lFlags, *pl1, *pl2);
        if(!res)
        {
            pl1++;
            pl2++;
        }
        else
        {
            return res;
        }
    }
    // make sure the strings are of same length
    if(*pl1==0 && *pl2==0)
        return 0;
    else
    if(*pl1==0)
        return -1;
    else
        return 1;
}
#endif

SKERR UTF8LenToUCS4Buffer(PRUint32** pplOutBuffer, PRUint32* plOutBufferLength,
                          PRUint32* plOutWrittenLength, const char* pcIn,
                          PRUint32 lInLength, PRBool bAddNullChar)
{
    SKERR err;

    // check the buffer and its size are sensible
    SK_ASSERT((*plOutBufferLength > 0) ^ (!*pplOutBuffer));
    if(        (*pplOutBuffer == NULL && *plOutBufferLength != 0)
            || (*pplOutBuffer != NULL && *plOutBufferLength == 0))
        return err_invalid;

    // create a buffer if needed
    if(!*plOutBufferLength)
    {
        *plOutBufferLength = 32;
        *pplOutBuffer = (PRUint32*)
            PR_Malloc(sizeof(PRUint32) * (*plOutBufferLength));
        if(!*pplOutBuffer)
            return err_memory;
    }

    PRUint32 iUCS4Size;
    do
    {
        // try to convert UTF8 to UCS4
        PRUint32 iMaxLength;
        iMaxLength = lInLength;
        iUCS4Size = *plOutBufferLength;
        if(bAddNullChar)
            iUCS4Size -= 1;
        err = UTF8LenToUCS4Len(*pplOutBuffer, &iUCS4Size, pcIn, &iMaxLength);

        // increase buffer size if needed
        if(err == err_memory)
        {
            *plOutBufferLength <<= 2;
            PR_Free(*pplOutBuffer);
            *pplOutBuffer = (PRUint32*)
                PR_Malloc(sizeof(PRUint32) * (*plOutBufferLength));
            if(!*pplOutBuffer)
                return err_memory;
        }
    } while(err == err_memory);

    if(bAddNullChar)
        (*pplOutBuffer)[iUCS4Size] = 0;

    if(plOutWrittenLength)
        *plOutWrittenLength = iUCS4Size;

    return noErr;
}

SKERR UCS4LenToUTF8Buffer(char** ppcOutBuffer, PRUint32* plOutBufferLength,
                          PRUint32* plOutWrittenLength, const PRUint32* plIn,
                          PRUint32 lInLength, PRBool bAddNullChar)
{
    SKERR err;

    // check the buffer and its size are sensible
    SK_ASSERT((*plOutBufferLength > 0) ^ (!*ppcOutBuffer));
    if(        (*ppcOutBuffer == NULL && *plOutBufferLength != 0)
            || (*ppcOutBuffer != NULL && *plOutBufferLength == 0))
        return err_invalid;

    // create a buffer if needed
    if(!*plOutBufferLength)
    {
        *plOutBufferLength = 32;
        *ppcOutBuffer = (char*)
            PR_Malloc(sizeof(char) * (*plOutBufferLength));
        if(!*ppcOutBuffer)
            return err_memory;
    }

    PRUint32 iUTF8Size;
    do
    {
        // try to convert UTF8 to UCS4
        PRUint32 iMaxLength;
        iMaxLength = lInLength;
        iUTF8Size = *plOutBufferLength;
        if(bAddNullChar)
            iUTF8Size -= 1;
        err = UCS4LenToUTF8Len(*ppcOutBuffer, &iUTF8Size, plIn, &iMaxLength);

        // increase buffer size if needed
        if(err == err_memory)
        {
            *plOutBufferLength <<= 2;
            PR_Free(*ppcOutBuffer);
            *ppcOutBuffer = (char*)
                PR_Malloc(sizeof(char) * (*plOutBufferLength));
            if(!*ppcOutBuffer)
                return err_memory;
        }
    } while(err == err_memory);

    if(bAddNullChar)
        (*ppcOutBuffer)[iUTF8Size] = 0;

    if(plOutWrittenLength)
        *plOutWrittenLength = iUTF8Size;

    return noErr;
}
