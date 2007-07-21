/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: simplifier.cpp,v 1.14.2.5 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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
#include "refcount.h"
#include "skptr.h"

#include "simplifier.h"

#define MAXSIMPSIZE 1024

SK_REFCOUNT_IMPL_IID(skIStringSimplifier,SK_SKISTRINGSIMPLIFIER_IID,SKRefCount)

skIStringSimplifier::~skIStringSimplifier()
{
    if(m_pcIdentity)
        PR_Free(m_pcIdentity);
}

SKERR skIStringSimplifier::SimplifyRealloc(char** ppc, PRUint32* plSize)
{
    SKERR err = noErr;
    if(!ppc)
        return err_invalid;

    PRUint32 iAvailable;
    // remaining bytes in the output buffer
    if(plSize)
        iAvailable = *plSize;
    else
        iAvailable = PL_strlen(*ppc) + 1;

    PRUint32 iToConvert = iAvailable - 1;

    if(!*ppc)
        return noErr;

    char * pcOut = *ppc;
    char * pcIn = *ppc;
    PRUint32 iDifference = 0;

    while(*pcIn && err != err_memory)
    {
        PRUint32 lOut = 15;
        PRUint32 lIn = iToConvert;
        char pcTmp[15];
        err = SimplifyFirstChar(pcIn, &lIn, pcTmp, &lOut);
        if(err != noErr && err != err_memory)
            return err;
        if(err == noErr && pcIn + lIn >= pcOut + lOut)
        {
            for(PRUint32 i = 0; i<lOut; i++) 
                *(pcOut++) = pcTmp[i];
            pcIn += lIn;
            iToConvert -= lIn;
            iDifference += lIn - lOut;
        }
        else
            err = err_memory;
    }
    if(err == noErr)
    {
        *pcOut = 0;
        return noErr;
    }
    // Can not do it in place

    // First, we may have to move the string suffix to the left
    // if iDifference > 0 (at least an input character was shortened
    // so pcIn is ahead of pcOut)
    if (iDifference)
    {
        PRUint32 iLeft = PL_strlen(pcIn);
        memmove (pcOut, pcIn, iLeft);
        pcOut[iLeft] = 0;
    }

    char *pcTmp;
    err = SimplifyToNew(*ppc, &pcTmp, &iAvailable);
    if(err != noErr)
        return err;
    if(plSize)
        *plSize = iAvailable;
    PR_Free(*ppc);
    *ppc = pcTmp;
    return noErr;
}

SKERR skIStringSimplifier::SimplifyOutputRealloc(const char* pcIn,
                                                 char** ppcOut,
                                                 PRUint32* plSize)
{
    SK_ASSERT (pcIn && ppcOut && plSize);
   
    if (!*ppcOut || !*plSize)
    {
        *ppcOut = (char*) PR_Malloc(16);
        if (!*ppcOut)
            return err_memory;
        *plSize = 16;
    }

    PRUint32 iInputLength = PL_strlen(pcIn);
    PRUint32 iOutputLength = *plSize;
    PRUint32 iTotalWritten = 0;
    SKERR err;
    while (*pcIn)
    {
        PRUint32 iWritten = iOutputLength;
        err = SimplifyFirstChar(pcIn, &iInputLength, *ppcOut + iTotalWritten,
            &iWritten);
        if (err != noErr)
        {
            if (err != err_memory)
                return err;

            // realloc the output buffer
            *ppcOut = (char*) PR_Realloc(*ppcOut, *plSize * 2);
            if (!*ppcOut)
                return err_memory;
            iOutputLength = *plSize;
            *plSize *= 2;
        }
        else
        {
            iTotalWritten += iWritten;
            iOutputLength -= iWritten;
            pcIn += iInputLength; // is the number of consumed input chars
        }
    }

    // add the final \0
    if (iTotalWritten == *plSize)
    {
        *ppcOut = (char*) PR_Realloc(*ppcOut, *plSize * 2);
        if (!*ppcOut)
            return err_memory;
        *plSize *= 2;
    }
    (*ppcOut)[iTotalWritten] = 0;
    return noErr;
}

SKERR skIStringSimplifier::SimplifyToNew(const char* pcSrc, char** ppcTarget,
        PRUint32 *piSize)
{
    SKERR err;
    char * pcTmp = 0;
    PRUint32 iTmpSize = (PRUint32) ((PL_strlen(pcSrc)+1) * 1.2);

    pcTmp = (char*) PR_Malloc(iTmpSize);
    const char *pcIn = pcSrc;
    PRInt32 iWritten = 0;
    while(*pcIn)
    {
        PRUint32 lIn = 0;
        PRUint32 lOut = iTmpSize - iWritten;
        err = SimplifyFirstChar(pcIn, &lIn, pcTmp + iWritten, &lOut);
        if(err == err_memory)
        {
            iTmpSize = iTmpSize * 2;
            pcTmp = (char*) PR_Realloc(pcTmp, iTmpSize);
            if(!pcTmp)
                return err_memory;
        }
        else if(err != noErr)
        {
            PR_Free(pcTmp);
            return err;
        }
        else
        {
            pcIn += lIn;
            iWritten += lOut;
        }
    }
    if((PRUint32) iWritten == iTmpSize)
    {
        pcTmp = (char*) PR_Realloc(pcTmp, iWritten+1);
        if(!pcTmp)
            return err_memory;
    }
    pcTmp[iWritten] = 0;
    *ppcTarget = pcTmp;
    if(piSize)
        *piSize = iTmpSize;
    return noErr;
}

SKERR skIStringSimplifier::CompareFirstChar(
        const char* pc1, PRUint32* plLen1,
        const char* pc2, PRUint32* plLen2,
        PRInt32* piCmp)
{
    SKERR err;
    PRUint32 iLenIn1, iLenIn2;
    iLenIn1 = 0;
    iLenIn2 = 0;

    PRUint32 iLenOut1, iLenOut2;
    char pcTmp1[MAXSIMPSIZE];
    char pcTmp2[MAXSIMPSIZE];
    iLenOut1 = MAXSIMPSIZE;
    iLenOut2 = MAXSIMPSIZE;

    err = SimplifyFirstChar(pc1, &iLenIn1, pcTmp1, &iLenOut1);
    if (err == err_memory)
        return SKError(err, "[skIStringSimplifier::CompareFirstChar] "
                "char returned by simplifier is more than %u bytes long.",
                MAXSIMPSIZE);

    err = SimplifyFirstChar(pc2, &iLenIn2, pcTmp2, &iLenOut2);
    if (err == err_memory)
        return SKError(err, "[skIStringSimplifier::CompareFirstChar] "
                "char returned by simplifier is more than %u bytes long.",
                MAXSIMPSIZE);

    PRBool bStalled = PR_FALSE;
    while(iLenOut1 != iLenOut2 && !bStalled)
    {
        bStalled = PR_TRUE;
        while(iLenOut1 < iLenOut2 && pc1[iLenIn1])
        {
            PRUint32 iAvailable = MAXSIMPSIZE - iLenOut1;
            PRUint32 iEaten = iLenIn1;
            err = SimplifyFirstChar(pc1+iLenIn1, &iEaten, pcTmp1+iLenOut1,
                    &iAvailable);
            if (err == err_memory)
                return SKError(err, "[skIStringSimplifier::CompareFirstChar] "
                     "char returned by simplifier is more than %u bytes long.",
                       MAXSIMPSIZE);
            iLenOut1 += iAvailable;
            iLenIn1 += iEaten;
            bStalled = PR_FALSE;
        }
        while(iLenOut2 < iLenOut1 && pc2[iLenIn2])
        {
            PRUint32 iAvailable = MAXSIMPSIZE - iLenOut2;
            PRUint32 iEaten = iLenIn2;
            err = SimplifyFirstChar(pc2+iLenIn2, &iEaten, pcTmp2+iLenOut2,
                    &iAvailable);
            if (err == err_memory)
                return SKError(err, "[skIStringSimplifier::CompareFirstChar] "
                     "char returned by simplifier is more than %u bytes long.",
                       MAXSIMPSIZE);
            iLenOut2 += iAvailable;
            iLenIn2 += iEaten;
            bStalled = PR_FALSE;
        }
    }

    pcTmp1[iLenOut1] = 0;
    pcTmp2[iLenOut2] = 0;

    if(piCmp)
        *piCmp = PL_strcmp(pcTmp1, pcTmp2);
    if(plLen1)
        *plLen1 = iLenIn1;
    if(plLen2)
        *plLen2 = iLenIn2;
    return noErr;
}

SKERR skIStringSimplifier::Compare(const char* pc1, PRUint32* plLen1,
        const char* pc2, PRUint32* plLen2,
        PRInt32* piCmp)
{
    SK_ASSERT(NULL != pc1);
    SK_ASSERT(NULL != pc2);
    PRInt32 iCmp = 0;
    SKERR err;
    PRUint32 iLen1 = 0;
    PRUint32 iLen2 = 0;
    PRUint32 iL1, iL2;

    while(!iCmp && (pc1[iLen1] || pc2[iLen2]))
    {
        iL1 = 0;
        iL2 = 0;
        err = CompareFirstChar(pc1 + iLen1, &iL1, pc2 + iLen2, &iL2, &iCmp);
        if(err != noErr)
            return err;
        iLen1 += iL1;
        iLen2 += iL2;
    }
    if(piCmp)
        *piCmp = iCmp;
    if(plLen1)
        *plLen1 = iLen1;
    if(plLen2)
        *plLen2 = iLen2;
    return noErr;
}
