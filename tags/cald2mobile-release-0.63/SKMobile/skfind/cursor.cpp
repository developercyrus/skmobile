/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursor.cpp,v 1.78.2.12 2005/03/04 10:20:54 kali Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
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

#include <skcore/skcore.h>

#include "sktable.h"

#include "recordcomparator.h"
#include "cursorcomparator.h"
#include "cursor.h"

#include "recordfilter.h"
#include "cursorfilter.h"

#include "mux.h"

#define BITISUP(k) ((m_pDataBitmap[(k)>>5] & 1<< ((k) & 0x1F)))
#define BITSETUP(k)                                                         \
    do                                                                      \
    {                                                                       \
        if(((k)>>5) >= m_iDataBitmapLongSize )                              \
        {                                                                   \
            SKERR err = GrowBitmap(((k)>>5) + 1);                           \
            if(err!=noErr)                                                  \
                return k;                                                   \
        }                                                                   \
        m_pDataBitmap[(k)>>5] |= 1<< ((k) & 0x1F);                          \
    } while(0)

static int g_sComparePRUint32(const void *p1, const void *p2)
{
    return *(PRUint32*)p1 - *(PRUint32*)p2;
}


class AndFilter : public SKICursorFilter
{
public:
    SK_REFCOUNT_INTF(AndFilter)
    SK_REFCOUNT_INTF_CREATOR(AndFilter)(SKCursor* cursor, PRBool bTruthValue,
            SKICursorComparator *pComparator);

    AndFilter(  SKCursor* cursor, PRBool bTruthValue,
                SKICursorComparator* pComparator)
    {
        m_bTruthValue = bTruthValue;
        m_pCursorFilter = cursor;
        m_iCurrent = 0;
        m_pCursorComparator = pComparator;
    }

    SKERR CheckRank(PRUint32 iRank, PRBool *pbKeepIt);

private:
    PRBool                      m_bTruthValue;
    skPtr<SKCursor>             m_pCursorFilter;
    PRUint32                    m_iCurrent;
    skPtr<SKICursorComparator>  m_pCursorComparator;
};

SK_REFCOUNT_IMPL(AndFilter)
SK_REFCOUNT_IMPL_CREATOR(AndFilter)(SKCursor* cursor, PRBool bTruthValue,
        SKICursorComparator* pComparator)
{
    AndFilter *filter = new AndFilter(cursor, bTruthValue, pComparator);
    return filter;
}

SKERR AndFilter::CheckRank(PRUint32 iRank, PRBool *pbKeepIt)
{
    SKERR err = noErr;
    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pData = m_pCursor->GetSharedCursorDataRead();
    err = m_pCursorFilter->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pDataFilter = m_pCursorFilter->GetSharedCursorDataRead();
    PRUint32 iSize = 0;
    m_pCursorFilter->GetCount(&iSize);
    PRInt32 iCmp = 0;

    if(m_pCursorComparator)
        while(m_iCurrent < iSize
                && ((err = m_pCursorComparator->CompareRanks(
                                                        pData[iRank],
                                                        pDataFilter[m_iCurrent],
                                                        &iCmp)) == noErr)
                && iCmp > 0)
              m_iCurrent ++;
    else
        while(m_iCurrent < iSize
                && ((iCmp = pData[iRank] - pDataFilter[m_iCurrent])) > 0)
              m_iCurrent ++;
    if(err != noErr)
        return err;
    *pbKeepIt =  (m_iCurrent < iSize) && !iCmp;
    if(!m_bTruthValue)
        *pbKeepIt = !(*pbKeepIt);
    return noErr;
}

// SKCursor
SK_REFCOUNT_IMPL(SKCursor);
SK_REFCOUNT_IMPL_CREATOR(SKCursor)(PRUint32 iCount, const PRUint32* pData)
{
    return new SKCursor(iCount, pData);
}
SK_REFCOUNT_IMPL_CREATOR(SKCursor)(PRUint32 iCount, const PRUint32* pData, PRBool bIsBitmap)
{
    return new SKCursor(iCount, pData, bIsBitmap);
}
SK_REFCOUNT_IMPL_IID(SKCursor, SK_SKCURSOR_IID, SKRefCount);

SKCursor::SKCursor(PRUint32 iCount, const PRUint32* pData)
{
    SKERR err = noErr;
    m_pData = NULL;
    m_pDataBitmap = NULL;
    m_iCount = 0;
    m_iDataBitmapLongSize = 0;
    err = SetCursorForm(iCount, pData);
    SK_ASSERT(err == noErr);
    m_bMustBeSorted = PR_TRUE;
    m_pCursorComparator = NULL;
}

SKCursor::SKCursor(PRUint32 iCount, const PRUint32* pData, PRBool bIsBitmap)
{
    SKERR err = noErr;
    m_pData = NULL;
    m_pDataBitmap = NULL;
    m_iCount = 0;
    m_iDataBitmapLongSize = 0;
    if (bIsBitmap)
        err = SetBitmapForm(iCount, pData);
    else
        err = SetCursorForm(iCount, pData);

    SK_ASSERT(err == noErr);
    m_bMustBeSorted = PR_TRUE;
    m_pCursorComparator = NULL;
}

SKCursor::~SKCursor()
{
    if(m_pData)
    {
        PR_Free(m_pData);
        m_pData = NULL;
    }
    if(m_pDataBitmap)
    {
        PR_Free(m_pDataBitmap);
        m_pDataBitmap = NULL;
    }
}

SKERR SKCursor::InitStartCount(PRUint32 iStart, PRUint32 iCount)
{
    SKERR err = noErr;
    err = DestroyBitmapForm();
    if (err != noErr)
        return err;
    err = DestroyCursorForm();
    if (err != noErr)
        return err;

    m_iCount = 0;

    if(!iCount)
        return noErr;

    m_pData = (PRUint32*) PR_Malloc(iCount*sizeof(PRUint32));
    if(!m_pData)
        return err_memory;

    for(PRUint32 i = 0; i < iCount; ++i)
        m_pData[i] = i + iStart;

    m_iCount = iCount;

    *m_pCursorComparator.already_AddRefed() = sk_CreateInstance(SKCursorComparatorRank)();
    if (!m_pCursorComparator)
        return err_memory;

    return noErr;
}

SKERR SKCursor::SetElement(PRUint32 iRank, PRUint32 iValue)
{
    if (m_iCount == 0)
    {
        SKERR err = ComputeCursorForm();
        if (err != noErr)
            return err;
    }
    if(iRank > m_iCount || !m_pData)
        return err_failure;
    m_pData[iRank] = iValue;
    return noErr;
}

SKERR SKCursor::Lookup(PRUint32 iValue, PRBool* pbFound)
{
   if(m_pDataBitmap)
   {
        *pbFound = BITISUP(iValue);
        return noErr;
   }
   if(m_pData)
   {
       *pbFound = bsearch(&iValue, m_pData, m_iCount, sizeof(PRUint32),
               g_sComparePRUint32) != NULL;
       return noErr;
   }
   return err_invalid;
}

SKERR SKCursor::GetRecord(SKIRecordSet* pRecordSet, PRUint32 iId,
                          PRBool bNoLookup, SKIRecord** ppRecord)
{
    SKERR err = noErr;
    *ppRecord = NULL;

    if (!m_pData)
    {
        err = ComputeCursorForm();
        if (err != noErr)
            return err;
    }

    if(iId < m_iCount)
    {
        if(!bNoLookup)
        {
            err = pRecordSet->LookupUNum(iId, skflmEXACT, &iId);
            if(err != noErr)
                return err;
        }

        return pRecordSet->GetRecord(m_pData[iId], ppRecord);
    }
    else
        return err_failure;
}

SKERR SKCursor::Filter(SKICursorFilter* pFilter)
{
    SKERR err = noErr;
    if(!pFilter)
        return err_failure;

    if (!m_pData)
    {
        err = ComputeCursorForm();
        if (err != noErr)
            return err;
    }
    err = DestroyBitmapForm();
    if (err != noErr)
        return err;

    PRUint32 i;
    PRUint32 *pNextKept = m_pData;
    PRBool bKeepIt = PR_FALSE;

    // reinitializes the filter
    err = pFilter->SetCursor(this);
    if(err != noErr)
        return err;
    err = pFilter->Reset();
    if(err != noErr)
        return err;

    for(i = 0; i<m_iCount; i++)
    {
        err = pFilter->CheckRank(i, &bKeepIt);
        if(err != noErr)
            return err;

        if(bKeepIt)
        {
            *pNextKept = m_pData[i];
            pNextKept++;
        }
    }
    m_iCount = pNextKept-m_pData;
    return noErr;
}

static SKERR _MergeCursorDbl(PRUint32 *pBase, PRUint32 *piCount,
                             PRUint32 *pBase1, const PRUint32 iCount1,
                             PRUint32 *pBase2, const PRUint32 iCount2,
                             SKICursorComparator *pComparator)
{
    PRUint32 i = 0;
    PRUint32 i1 = 0;
    PRUint32 i2 = 0;

    while((i1 < iCount1) && (i2 < iCount2))
    {
        PRInt32 iCmp = 0;

        if(pComparator)
        {
            SKERR err;
            err = pComparator->CompareRanks(pBase1[i1], pBase2[i2], &iCmp);
            if(err != noErr)
                return err;
        }
        else
            iCmp = pBase1[i1] - pBase2[i2];

        if(iCmp < 0)
        {
            pBase[i++] = pBase1[i1++];
        }
        else if(iCmp > 0)
        {
            pBase[i++] = pBase2[i2++];
        }
        else
        {
            pBase[i++] = pBase1[i1++];
            i2++;
        }
    }

    if(i1 < iCount1)
    {
        memcpy(pBase + i, pBase1 + i1, (iCount1 - i1) * sizeof(PRUint32));
        i += iCount1 - i1;
    }
    else if(i2 < iCount2)
    {
        memcpy(pBase + i, pBase2 + i2, (iCount2 - i2) * sizeof(PRUint32));
        i += iCount2 - i2;
    }

    *piCount = i;

    return noErr;
}

SKERR SKCursor::Merge(SKCursor* pCursor, skfOperator oper)
{
    if(!pCursor)
        return err_failure;

    SKERR err = noErr;

    PRUint32 iCount;
    PRUint32 *pData;

    if (oper == skfopAPPEND)
    {
        if (!m_pData)
        {
            err = ComputeCursorForm();
            if (err != noErr)
                return err;
        }
        if (!(pCursor->m_pData))
        {
            err = pCursor->ComputeCursorForm();
            if (err != noErr)
                return err;
        }
        if (m_pCursorComparator)
            m_pCursorComparator = NULL;

        if( !(m_iCount + pCursor->m_iCount) )
            return noErr;

        iCount = m_iCount + pCursor->m_iCount;
        pData = (PRUint32*) PR_Malloc(iCount*sizeof(PRUint32));
        if(!pData)
            return err_memory;

        memcpy(pData, m_pData, m_iCount * sizeof(PRUint32));
        memcpy(pData + m_iCount, pCursor->m_pData,
                pCursor->m_iCount * sizeof(PRUint32));
        err = DestroyCursorForm();
        if (err != noErr)
            return err;

        m_pData = pData;
        m_iCount = iCount;

        return noErr;
    }

    // make sure OR uses the bipmap form when available.
    if(m_pDataBitmap && oper == skfopOR && m_pData)
        if((err = DestroyCursorForm()) != noErr)
            return err;

    if (m_pData || ((!m_pData) && (!m_pDataBitmap)))
    {
        if (!m_pCursorComparator)
        {
            skPtr<SKICursorComparator> pCursorComparator = NULL;
            *pCursorComparator.already_AddRefed() =
                sk_CreateInstance(SKCursorComparatorRank)();
            if (!pCursorComparator)
                return err_memory;
            err = Sort(pCursorComparator);
        }

        pCursor->Sort(m_pCursorComparator);

        switch(oper)
        {
            case skfopOR:
                if( !(m_iCount + pCursor->m_iCount) )
                    break;

                pData = (PRUint32*) PR_Malloc(sizeof(PRUint32)*
                        (m_iCount + pCursor->m_iCount));
                if(!pData)
                {
                    err = err_failure;
                    break;
                }
                err = _MergeCursorDbl(pData, &iCount,
                                      m_pData, m_iCount,
                                      pCursor->m_pData,
                                      pCursor->m_iCount,
                                      m_pCursorComparator);
                if(err != noErr)
                {
                    delete[] pData;
                    pData = NULL;
                }
                err = DestroyCursorForm();
                if (err != noErr)
                    return err;
                m_pData = pData;
                m_iCount = iCount;
                break;
            case skfopAND:
                {
                    AndFilter f(pCursor, PR_TRUE, m_pCursorComparator);
                    f.SetCursor(this);
                    err = Filter(&f);
                }
                break;
            case skfopEXCEPT:
                {
                    AndFilter f(pCursor, PR_FALSE, m_pCursorComparator);
                    f.SetCursor(this);
                    err = Filter(&f);
                }
                break;
            default:
                    SK_ASSERT(false);
                    err = err_failure;
        }
        if (err != noErr)
            return err;
        err = DestroyBitmapForm();
        if (err != noErr)
            return err;
    }
    else
    {
        PRUint32 i;
        if(pCursor->m_pData && oper==skfopOR)
        {
            for(i = 0; i<pCursor->m_iCount; i++)
                BITSETUP(pCursor->m_pData[i]);
            return DestroyCursorForm();
        }
        if (!(*pCursor).m_pDataBitmap)
        {
            err = pCursor->ComputeBitmapForm();
            if (err != noErr)
                return err;
        }
        if (m_iDataBitmapLongSize < pCursor->m_iDataBitmapLongSize)
        {
            err = GrowBitmap(pCursor->m_iDataBitmapLongSize >> 5);
            if (err != noErr)
                return err;
        }

        switch(oper)
        {
            case skfopOR:
                // OR is done before
                SK_ASSERT(PR_FALSE);
                break;
            case skfopAND:
                for (i = 0; i < pCursor->m_iDataBitmapLongSize; ++i)
                    m_pDataBitmap[i] &= pCursor->m_pDataBitmap[i];
                break;
            case skfopEXCEPT:
                for (i = 0; i < pCursor->m_iDataBitmapLongSize; ++i)
                    m_pDataBitmap[i] &= ~pCursor->m_pDataBitmap[i];
                break;
            default:
                SK_ASSERT(PR_FALSE);
                err = err_failure;
        }
        err = DestroyCursorForm();
        if (err != noErr)
            return err;
    }

    return err;
}

SKERR SKCursor::MuxCursors(SKCursor **ppCursors, PRUint32 iCount,
                           PRBool *pbInterrupt)
{
    if(!iCount)
        return noErr;

    SK_ASSERT(NULL != ppCursors);

    SKMux mux;
    SKERR err = mux.MuxCursors(ppCursors, iCount, m_pCursorComparator, pbInterrupt);
    if(err != noErr)
        return err;

    err = DestroyCursorForm();
    if (err != noErr)
        return err;
    err = DestroyBitmapForm();
    if (err != noErr)
        return err;

    return mux.RetrieveData(&m_iCount, &m_pData);
}

SKERR SKCursor::Sort(SKICursorComparator * pComparator)
{
    if (!m_bMustBeSorted)
        return noErr;

    if(!m_pData && !pComparator)
        return noErr;

    SKERR err = noErr;
    m_bMustBeSorted = PR_FALSE;
    if (!m_pData)
    {
        err = ComputeCursorForm();
        if (err != noErr)
        {
            m_bMustBeSorted = PR_TRUE;
            return err;
        }
    }
    m_bMustBeSorted = PR_TRUE;
    if (!pComparator)
    {
        qsort(m_pData, m_iCount, sizeof(PRUint32), g_sComparePRUint32);
        m_pCursorComparator = pComparator;
    }
    else
    {
        if (m_pCursorComparator)
        {
            skPtr<SKBinary> pBinNewComparatorID;
            skPtr<SKBinary> pBinOldComparatorID;
            char* pStrNewComparatorID = NULL;
            char* pStrOldComparatorID = NULL;
            err = pComparator->GetCursorComparatorID(
                    pBinNewComparatorID.already_AddRefed());
            if (err != noErr)
                return err;
            //FIXME
            //Add an identifier for SKIRecordSet to identify SKICursorComparator
            pStrNewComparatorID = (char*) pBinNewComparatorID->GetSharedData();
            if (PL_strcmp(pStrNewComparatorID, "UNKNOWN"))
            {
                err = m_pCursorComparator->GetCursorComparatorID(
                            pBinOldComparatorID.already_AddRefed());
                if (err != noErr)
                    return err;
                pStrOldComparatorID =
                    (char*) pBinOldComparatorID->GetSharedData();
                if (PL_strcmp(pStrOldComparatorID, "UNKNOWN"))
                    if (!PL_strcmp(pStrNewComparatorID, pStrOldComparatorID))
                        return noErr;
            }
        }

        m_pCursorComparator = pComparator;
        //m_pCursorComparator->AddRef();

//#define _SORT_FUNCTION SKQsortRecord
#define _SORT_FUNCTION SKQ3sortRecord
//#define _SORT_FUNCTION SKHsortRecord
        err = _SORT_FUNCTION(m_pData, m_iCount, sizeof(PRUint32),
                             m_pCursorComparator);

        if (err != noErr)
            return err;
#ifdef SKC_DEBUG
        SK_ASSERT(IsSorted(m_pData, m_iCount, sizeof(PRUint32), m_pCursorComparator));
#endif
    }
    return noErr;
}

SKERR SKCursor::Clone(SKCursor **ppClone)
{
    if(!ppClone)
        return err_invalid;

    if (m_iCount)
        *ppClone = sk_CreateInstance(SKCursor)(m_iCount, m_pData);
    else
        *ppClone = sk_CreateInstance(SKCursor)(0, NULL);
    if(*ppClone == NULL)
        return err_memory;

    if (m_iDataBitmapLongSize)
    {
        SKERR err = (*ppClone)->GrowBitmap(m_iDataBitmapLongSize);
        if(err != noErr)
            return err;
        memcpy((*ppClone)->m_pDataBitmap, m_pDataBitmap,
                    sizeof(PRUint32) * m_iDataBitmapLongSize);
    }
    if (m_pCursorComparator)
        (**ppClone).m_pCursorComparator = m_pCursorComparator;

    return noErr;
}

SKERR SKCursor::Extract(PRUint32 iOffset, PRUint32 iCount, SKCursor** ppClone)
{
    if(!ppClone)
        return err_invalid;
    if (!m_pData)
    {
        SKERR err = ComputeCursorForm();
        if (err != noErr)
            return err;
    }

    *ppClone = sk_CreateInstance(SKCursor)(iCount, m_pData + iOffset);
    if(*ppClone == NULL)
        return err_memory;
    return noErr;
}

SKERR SKCursor::ComputeBitmapForm()
{
    if (!m_bCursorCanChange && (m_pDataBitmap != NULL))
        return noErr;

    SKERR err = CleanBitmap();
    if (err != noErr)
        return err;

    if (!m_iCount)
        return noErr;

    PRUint32 k;
    PRUint32 iMaxSize = 0;
    for (k = 0; k < m_iCount; ++k)
        if (m_pData[k] > iMaxSize)
            iMaxSize = m_pData[k];

    if((iMaxSize >> 5) > m_iDataBitmapLongSize)
    {
        err = GrowBitmap(m_iDataBitmapLongSize);
        if(err != noErr)
            return err;
    }

    for (k = 0; k < m_iCount; ++k)
        BITSETUP(m_pData[k]);

    return noErr;
}

SKERR SKCursor::ComputeCursorForm()
{
    if (m_pData)
        return noErr;

    if (!m_iDataBitmapLongSize)
        return DestroyCursorForm();

    // i'd prefer one single pass with a realloc.
    PRUint32 k;
    for (k = 0; k < m_iDataBitmapLongSize * 32 ; k++)
    {
        if (BITISUP(k))
            m_iCount++;
    }
    if (m_iCount == 0)
        return noErr;

    m_pData = (PRUint32*) PR_Malloc(m_iCount * sizeof(PRUint32));

    if (!m_pData)
        return err_memory;

    PRUint32 iRow = 0;
    for (k = 0; k < m_iDataBitmapLongSize * 32; k++)
        if (BITISUP(k))
            m_pData[iRow++] = k;

    return Sort(m_pCursorComparator);
}

SKERR SKCursor::DestroyBitmapForm()
{
    if (m_pDataBitmap)
    {
        PR_Free(m_pDataBitmap);
        m_pDataBitmap = NULL;
    }
    m_iDataBitmapLongSize = 0;
    return noErr;
}

SKERR SKCursor::DestroyCursorForm()
{
    if (m_pData)
    {
        PR_Free(m_pData);
        m_pData = NULL;
    }
    m_iCount = 0;
    return noErr;
}

SKERR SKCursor::SetBitmapForm(PRUint32 iDataBitmapLongSize,
                              const PRUint32* piDataBitmap)
{
    SKERR err = DestroyCursorForm();
    if (err != noErr)
        return err;
    err = CleanBitmap();
    if (err != noErr)
        return err;

    err = GrowBitmap(iDataBitmapLongSize);
    if(err != noErr)
        return err;
    memcpy(m_pDataBitmap, piDataBitmap,
                sizeof(PRUint32) * m_iDataBitmapLongSize);
    m_iDataBitmapLongSize = iDataBitmapLongSize;

    m_bCursorCanChange = PR_FALSE;
    return noErr;
}

SKERR SKCursor::SetCursorForm(PRUint32 iDataCount,
                              const PRUint32* piData)
{
    SKERR err = DestroyBitmapForm();
    if (err != noErr)
        return err;

    if(iDataCount)
    {
        if(iDataCount > m_iCount)
            m_pData = (PRUint32*)
                PR_Realloc(m_pData, iDataCount*sizeof(PRUint32));
        if (!m_pData)
            return err_memory;

        if(piData)
            memcpy(m_pData, piData, iDataCount * sizeof(PRUint32));
    }
    else
        m_pData = NULL;

    m_iCount = iDataCount;

    m_bCursorCanChange = PR_FALSE;
    return noErr;
}

SKERR SKCursor::GrowBitmap(PRUint32 iDataBitmapLongSize)
{
    if(iDataBitmapLongSize < m_iDataBitmapLongSize)
        return noErr;

    m_pDataBitmap = (PRUint32*) PR_Realloc(m_pDataBitmap,
            iDataBitmapLongSize * sizeof(PRUint32));

    for(PRUint32 i = m_iDataBitmapLongSize; i<iDataBitmapLongSize; i++)
        m_pDataBitmap[i] = 0;

    m_iDataBitmapLongSize = iDataBitmapLongSize;
    return m_pDataBitmap ? noErr : err_memory;
}

SKERR SKCursor::CleanBitmap()
{
    for(PRUint32 i = 0; i<m_iDataBitmapLongSize; i++)
        m_pDataBitmap[i] = 0;
    return noErr;
}

#ifdef DEBUG
SKERR SKCursor::Dump()
{
    if (m_iCount)
    {
        printf("\nCursor Form\n");
        for (PRUint32 i = 0; i < m_iCount; ++i)
            printf("\tm_pData[%u] = %u\n", i, m_pData[i]);
    }
    else
        printf("\nCursor Form not ready\n");

    if (m_iDataBitmapLongSize)
    {
        printf("Bitmap Form\n");
        for (PRUint32 i = 0; i < m_iDataBitmapLongSize; ++i)
            if(m_pDataBitmap[i])
                printf("\tm_pDataBitmap[%u] = %u\n", i, m_pDataBitmap[i]);
    }
    else
        printf("Bitmap Form not ready\n");

    return noErr;
}
#endif
SKERR skResolveRSUNum(SKCursor *pFrom, SKIRecordSet *pRS,
                      SKIField *pField, PRBool bFieldSorted,
                      PRBool *pbInterrupt, SKCursor **ppTo)
{
    SKERR err;

    PRUint32 iCount;
    err = pFrom->GetCount(&iCount);
    if(err != noErr)
        return err;

    *ppTo = sk_CreateInstance(SKCursor)(iCount);
    if(!*ppTo)
        return err_memory;

    err = pFrom->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pFromData = pFrom->GetSharedCursorDataRead();
    err = (*ppTo)->ComputeCursorForm();
    if (err != noErr)
        return err;
    PRUint32 *pToData = (*ppTo)->GetSharedCursorDataWrite();
    if(iCount && (!pFromData || !pToData))
        return err_memory;

    for(PRUint32 i = 0; i < iCount; ++i)
    {
        // Check interruption flag
        if(pbInterrupt && *pbInterrupt)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            return err_interrupted;
        }

        skPtr<SKIRecord> pRec;
        err = pRS->GetRecord(pFromData[i], pRec.already_AddRefed());
        if(err != noErr)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            return err;
        }

        err = pRec->GetUNumFieldValue(pField, pToData + i);
        if(err != noErr)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            return err;
        }
    }

    if(!bFieldSorted)
    {
        err = (*ppTo)->Sort(NULL);
        if(err != noErr)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            return err;
        }
    }

    (*ppTo)->ReleaseSharedCursorDataWrite();

    return noErr;
}

SKERR skResolveRSLink(SKCursor *pFrom, SKIRecordSet *pRS,
                      SKIField *pField, SKIField *pSubField,
                      PRBool bSubFieldSorted, PRBool *pbInterrupt,
                      SKCursor **ppTo)
{
    SKERR err;

    PRUint32 iCount;
    err = pFrom->GetCount(&iCount);
    if(err != noErr)
        return err;

    // CD: next call will return NULL on mac
    if(!iCount)
    {
        *ppTo = sk_CreateInstance(SKCursor)();
        return noErr;
    }

    SKCursor **ppCursors = new SKCursor *[iCount];
    if(!ppCursors)
        return err_memory;

    memset(ppCursors, 0, iCount * sizeof(SKCursor *));

    for(PRUint32 i = 0; i < iCount; ++i)
    {
        // Check interruption flag
        if(pbInterrupt && *pbInterrupt)
        {
            err = err_interrupted;
            break;
        }

        skPtr<SKIRecord> pRec;
        err = pFrom->GetRecord(pRS, i, PR_TRUE, pRec.already_AddRefed());

        if(err != noErr)
            break;

        skPtr<SKIRecordSet> pLink;
        err = pRec->GetLinkFieldValue(pField, pLink.already_AddRefed());
        if(err != noErr)
            break;

        PRUint32 j;
        err = pLink->GetCount(&j);
        if(err != noErr)
            break;

        err = pLink->ExtractCursor(pSubField, 0, j, ppCursors + i);
        if(err != noErr)
            break;

        if(!bSubFieldSorted)
        {
            err = ppCursors[i]->Sort(NULL);
            if(err != noErr)
                break;
        }
    }
    if(err != noErr)
    {
        for(PRUint32 i = 0; i < iCount; ++i)
            if(ppCursors[i])
                ppCursors[i]->Release();

        delete[] ppCursors;
        ppCursors = NULL;
        return err;
    }

    // Remove NULL pointers.
    PRUint32 iNonEmpty = 0;
    for(PRUint32 j = 0; j < iCount; ++j)
    {
        // Check interruption flag
        if(pbInterrupt && *pbInterrupt)
        {
            err = err_interrupted;
            break;
        }

        if(ppCursors[j])
        {
            PRUint32 k = 0;
            err = ppCursors[j]->GetCount(&k);
            if(err != noErr)
                break;
            if(k)
            {
                if(iNonEmpty != j)
                {
                    ppCursors[iNonEmpty] = ppCursors[j];
                    ppCursors[j] = NULL;
                }
                ++iNonEmpty;
            }
            else
            {
                ppCursors[j]->Release();
                ppCursors[j] = NULL;
            }
        }
    }

    if(err == noErr)
    {
        *ppTo = sk_CreateInstance(SKCursor)();
        if(*ppTo)
            err = (*ppTo)->MuxCursors(ppCursors, iNonEmpty, pbInterrupt);
        else
            err = err_memory;
    }

    for(PRUint32 k = 0; k < iCount; ++k)
        if(ppCursors[k])
            ppCursors[k]->Release();

    delete[] ppCursors;
    ppCursors = NULL;
    return err;
}

