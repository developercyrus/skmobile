/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: sort.cpp,v 1.6.4.7 2005/02/17 15:29:20 krys Exp $
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

#include <nspr/nspr.h>

#include "skcoreconfig.h"
#ifdef WIN32
#include <malloc.h>
#endif
#include "machine.h"
#include "error.h"
#include "refcount.h"
#include "sort.h"

//============================================================================
// free funcs
//============================================================================

//----------------------------------------------------------------------------
// SKLFindRecord
//----------------------------------------------------------------------------
SKERR SKLFindRecord(void* pKey, void* pBase, const PRUint32 lCount,
                    const PRUint32 lSize,
                    SKIComparator* pIComparator, PRUint32* pId,
                    PRBool *pbFound)
{
    *pbFound = PR_FALSE;
    PRUint32 i = lCount;
    while(--i != (PRUint32)-1)
    {
        PRInt32 iCmp = 0;
        SKERR err = pIComparator->Compare(pKey, pBase, &iCmp);
        if(err != noErr)
            return err;

        if(iCmp == 0)
        {
            if(pId)
                *pId = lCount - i - 1;
            *pbFound = PR_TRUE;
            return noErr;
        }
        pBase = (char*)pBase + lSize;
    }

    return noErr;
}

//----------------------------------------------------------------------------
// SKLFindSortedRecord
//----------------------------------------------------------------------------
SKERR SKLFindSortedRecord(void* pKey, void* pBase, const PRUint32 lCount,
                          const PRUint32 lSize,
                          SKIComparator* pIComparator, PRUint32* pId,
                          PRBool *pbFound)
{
    *pbFound = PR_FALSE;
    // Do strange things to avoid a multiplication like "pBase + i * lSize")
    PRUint32 i = lCount;
    while(--i != (PRUint32)-1)
    {
        PRInt32 iCmp = 0;
        SKERR err = pIComparator->Compare(pKey, pBase, &iCmp);
        if(err != noErr)
            return err;

        if(iCmp == 0)
        {
            if(pId)
                *pId = lCount - i - 1;
            *pbFound = PR_TRUE;
            return noErr;
        }
        else if(iCmp < 0)
        {
            if(pId)
                *pId = lCount - i - 1;
            *pbFound = PR_FALSE;
            return noErr;
        }
        pBase = (char*)pBase + lSize;
    }
    if(pId)
        *pId = lCount;

    return noErr;
}

#ifdef WIN32
#define alloca _alloca
#endif

#define PREPARE()                                                       \
    void *tmp = alloca(iSize);                                          \
    if(!tmp)                                                            \
        return err_memory;

#define SWAP(a,b)                                                       \
    {                                                                   \
        memmove(tmp, b, iSize);                                         \
        memmove(b, a, iSize);                                           \
        memmove(a, tmp, iSize);                                         \
    }

#define SWAP3(a, b, c)                                                  \
    {                                                                   \
        memmove(tmp, b, iSize);                                         \
        memmove(b, a, iSize);                                           \
        memmove(a, c, iSize);                                           \
        memmove(c, tmp, iSize);                                         \
    }


//----------------------------------------------------------------------------
// IsSorted
//----------------------------------------------------------------------------
#ifdef DEBUG
PRBool IsSorted(void* pBase, const PRUint32 lCount, const PRUint32 lSize,
                SKIComparator* pIComparator)
{
    if(lCount > 1)
    {
        for(PRUint32 i = 1; i < lCount; ++i)
        {
            PRInt32 iCmp = 0;
            SKERR err = pIComparator->Compare((char*)pBase + (i - 1) * lSize,
                                              (char*)pBase + i * lSize, &iCmp);
            SK_ASSERT(err == noErr);
            if(iCmp > 0)
                return PR_FALSE;
        }
    }
    return PR_TRUE;
}
#endif

//----------------------------------------------------------------------------
// SKQsortRecord
//----------------------------------------------------------------------------

SKERR SKQsortRecord(void* pBase, const PRUint32 iCount, const PRUint32 iSize,
                    SKIComparator* pIComparator)
{
    if(iCount > 1)
    {
        PREPARE();

        char* pLeft = (char*)pBase + iSize;
        char* pRight = (char*)pBase + (iCount-1) * iSize;

        SWAP(pBase, (char*)pBase + iSize * (iCount / 2));

        SKERR err = noErr;
        PRInt32 iCmp = 0;
        while(pLeft < pRight)
        {
            err = noErr;
            if(pLeft < pRight)
                err = pIComparator->Compare(pBase, pLeft, &iCmp);
            while((err == noErr) && (pLeft < pRight) && (iCmp > 0))
            {
                pLeft = pLeft + iSize;
                if(pLeft < pRight)
                    err = pIComparator->Compare(pBase, pLeft, &iCmp);
            }
            if(err != noErr)
                return err;

            err = noErr;
            if(pLeft < pRight)
                err = pIComparator->Compare(pRight, pBase, &iCmp);
            while((err == noErr) && (pLeft < pRight) && (iCmp >= 0))
            {
                pRight = pRight - iSize;
                if(pLeft < pRight)
                    err = pIComparator->Compare(pRight, pBase, &iCmp);
            }
            if(err != noErr)
                return err;

            if(pLeft < pRight) SWAP(pLeft, pRight);
        }

        SK_ASSERT(pLeft == pRight);
        SK_ASSERT(pBase != pRight);
        err = pIComparator->Compare(pRight, pBase, &iCmp);
        if(err != noErr)
            return err;

        if(iCmp >= 0)
            pRight -= iSize;

        if(pRight != pBase)
            SWAP(pRight, pBase)
        PRUint32 lLeftCount = (pRight - (char*)pBase) / iSize;
        PRUint32 lRightCount = iCount - lLeftCount - 1;

        err = SKQsortRecord(pBase, lLeftCount,  iSize, pIComparator);
        if(err != noErr)
            return err;
        SK_ASSERT(IsSorted(pBase, lLeftCount,  iSize, pIComparator));

        err = SKQsortRecord(pRight + iSize, lRightCount,  iSize, pIComparator);
        if(err != noErr)
            return err;
        SK_ASSERT(IsSorted(pRight + iSize, lRightCount,  iSize, pIComparator));
    }

    return noErr;
}

//----------------------------------------------------------------------------
// SKQ3sortRecord
//----------------------------------------------------------------------------

SKERR SKQ3sortRecord(void* pBase, const PRUint32 iCount,
                     const PRUint32 iSize,
                     SKIComparator *pComparator
                     //,PRBool bDeterministic
                     )
{
    if(iCount > 1)
    {
        PREPARE();

        char* pLeft = (char*)pBase + iSize;
        char* pMiddle = pLeft;
        char* pRight = (char*)pBase + (iCount - 1) * iSize;

        // Un-comment to enable the non-deterministic argument
        /*
        if(bDeterministic)
        {
        */
            SWAP(pBase, (char*)pBase + (iCount / 2) * iSize);
        /*
        }
        else
            if(iCount > 3)
            {
                PRUint32 iPivot =
                    (((PRUint32) pBase ^ iCount ^ (PRUint32)pComparator)
                     % (iCount / 3)) + iCount / 3;
                SWAP(pBase, (char*)pBase + iPivot * iSize);
            }
        */

        SKERR err = noErr;
        PRInt32 iCmp = 0;
        while(pMiddle < pRight)
        {
            err = noErr;
            if(pMiddle < pRight)
                err = pComparator->Compare(pBase, pMiddle, &iCmp);
            while((err == noErr) && (pMiddle < pRight) && (iCmp >= 0))
            {
                if(iCmp > 0)
                {
                    if(pMiddle > pLeft)
                    {
                        SWAP(pMiddle, pLeft);
                    }
                    pLeft += iSize;
                }

                pMiddle += iSize;
                if(pMiddle < pRight)
                    err = pComparator->Compare(pBase, pMiddle, &iCmp);
            }
            if(err != noErr)
                return err;

            err = noErr;
            if(pMiddle < pRight)
                err = pComparator->Compare(pRight, pBase, &iCmp);
            while((err == noErr) && (pMiddle < pRight) && (iCmp > 0))
            {
                pRight-= iSize;
                if(pMiddle < pRight)
                    err = pComparator->Compare(pRight, pBase, &iCmp);
            }
            if(err != noErr)
                return err;

            if(pMiddle < pRight)
            {
                SK_ASSERT(iCmp <= 0);
                if(iCmp < 0)
                {
                    SWAP3(pLeft, pMiddle, pRight);
                    pLeft+= iSize;
                    pMiddle+= iSize;
                }
                else
                {
                    SWAP(pMiddle, pRight);
                    pMiddle+= iSize;
                }
            }
        }
        SK_ASSERT(pMiddle == pRight);

        err = pComparator->Compare(pBase, pMiddle, &iCmp);
        if(err != noErr)
            return err;

        if(iCmp > 0)
        {
            SWAP(pMiddle, pLeft);
            pLeft+= iSize;
            pMiddle+= iSize;
            pRight+= iSize;
        }
        else if(iCmp == 0)
        {
            pRight+= iSize;
            pMiddle+= iSize;
        }

        if(pLeft - iSize != pBase)
        {
            SWAP(pLeft - iSize, pBase);
        }

        PRUint32 iLeftCount = ((PRUint32) (pLeft - (char*)pBase) / iSize)
            - 1;
        PRUint32 iRightCount = iCount -
            ((PRUint32) (pRight - (char*)pBase) / iSize);

        if (iLeftCount > 1)
        {
            err = SKQ3sortRecord(pBase, iLeftCount, iSize, pComparator
                /*, bDeterministic*/);
            if(err != noErr)
                return err;
        }

        if (iRightCount > 1)
        {
            err = SKQ3sortRecord(pRight, iRightCount, iSize, pComparator
                /*, bDeterministic*/);
            if(err != noErr)
                return err;
        }

        SK_ASSERT(IsSorted(pBase, iCount, iSize, pComparator));
    }

    return noErr;
}


//----------------------------------------------------------------------------
// SKHsortRecord
//----------------------------------------------------------------------------

static inline SKERR _SKHsortDownHeap(void* pBase,
                                     PRUint32 iCount, PRUint32 iSize,
                                     SKIComparator* pIComparator, PRUint32 i)
{
    PREPARE();
    PRUint32 j = 2 * i + 1; // first descendant of i
    while(j < iCount)
    {
        PRInt32 iCmp;
        char *q = (char *)pBase + iSize * j;
        if(j + 1 < iCount) // is there a second descendant?
        {
            SKERR err = pIComparator->Compare(q + iSize, q, &iCmp);
            if(err != noErr)
                return err;
            j += (iCmp > 0);
            q += iSize * (iCmp > 0);
            // j is the descendant of i with maximum label
        }

        char *p = (char *)pBase + iSize * i;
        SKERR err = pIComparator->Compare(p, q, &iCmp);
        if(err != noErr)
            return err;
        if(iCmp >= 0)
            return noErr;   // v has heap property
        // otherwise
        SWAP(p, q);         // exchange labels of i and j
        i = j;              // continue
        j = 2 * i + 1;
    }
    return noErr;
}

SKERR SKHsortRecord(void* pBase, PRUint32 iCount, PRUint32 iSize,
                    SKIComparator* pIComparator)
{
    // Build heap
    for(PRUint32 i = iCount / 2 - 1; i != (PRUint32) -1; --i)
    {
        SKERR err = _SKHsortDownHeap(pBase, iCount, iSize, pIComparator, i);
        if(err != noErr)
            return err;
    }

    PREPARE();

    while(iCount > 1)
    {
        --iCount;
        SWAP(pBase, (char *)pBase + iSize * iCount);
        SKERR err =_SKHsortDownHeap(pBase, iCount, iSize, pIComparator, 0);
        if(err != noErr)
            return err;
    }

    SK_ASSERT(IsSorted(pBase, iCount, iSize, pIComparator));
    return noErr;
}

#undef SWAP

