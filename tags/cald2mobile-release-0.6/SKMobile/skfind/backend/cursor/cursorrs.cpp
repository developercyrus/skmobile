/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorrs.cpp,v 1.8.2.11 2005/02/21 14:22:38 krys Exp $
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

#include <skfind/skfind.h>

#include "cursorrs.h"

SK_REFCOUNT_IMPL_DEFAULT(SKCursorRecordSet)
SK_REFCOUNT_IMPL_IID(SKCursorRecordSet, SK_SKCURSORRECORDSET_IID, SKIRecordSet);

SKERR SKCursorRecordSet::Init(PRUint32 iOffset, PRUint32 iCount,
                              SKIRecordSet *pRecordSet)
{
    SK_ASSERT(NULL != pRecordSet);
    SKERR err;

    *m_pCursor.already_AddRefed() = sk_CreateInstance(SKCursor)(iCount);
    if(!m_pCursor)
        return err_memory;

    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    PRUint32 *pData = m_pCursor->GetSharedCursorDataWrite();
    for(PRUint32 i = 0; i < iCount; ++i)
        pData[i] = iOffset + i;
    m_pCursor->ReleaseSharedCursorDataWrite();

    m_pRecordSet = pRecordSet;

    m_iSpeederCount = 0;
    m_ppSpeeders = NULL;

    return noErr;
}

SKERR SKCursorRecordSet::InitCursorRS(PRUint32 iOffset,
                                      PRUint32 iCount,
                                      SKIRecordSet *pRecordSet,
                                      SKRefCount *pFragment)
{
    SKERR err = Init(iOffset, iCount, pRecordSet);
    if (err == noErr)
        err = AppendSpeeder(pFragment);

    return err;
}

SKERR SKCursorRecordSet::Init(SKCursor *pCursor, SKIRecordSet *pRecordSet)
{
    SK_ASSERT(NULL != pRecordSet);

    m_pCursor = pCursor;
    m_pRecordSet = pRecordSet;

    m_iSpeederCount = 0;
    m_ppSpeeders = NULL;

    return noErr;
}

SKCursorRecordSet::~SKCursorRecordSet()
{
    if(m_ppSpeeders)
    {
        for(PRUint32 i = 0; i < m_iSpeederCount; ++i)
            m_ppSpeeders[i]->Release();
    }
    PR_Free(m_ppSpeeders);
}

SKERR SKCursorRecordSet::GetInternalCursor(SKCursor** ppCursor)
{
    if(!m_pCursor)
        return err_invalid;
    *ppCursor = m_pCursor;
    (*ppCursor)->AddRef();
    return noErr;
}

SKERR SKCursorRecordSet::GetCount(PRUint32 *piCount) const
{
    return m_pCursor->GetCount(piCount);
}

SKERR SKCursorRecordSet::GetRecord(PRUint32 id, SKIRecord** pRecord)
{
    SKERR err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pData = m_pCursor->GetSharedCursorDataRead();
    if(!pData)
        return err_failure;

    return m_pRecordSet->GetRecord(pData[id], pRecord);
}

SKERR SKCursorRecordSet::GetFldCollection(SKIFldCollection** pFldCol)
{
    return m_pRecordSet->GetFldCollection(pFldCol);
}

SKERR SKCursorRecordSet::LookupTextImp(const char * pszSearch, skfLookupMode mode,
                                    PRUint32 *piId)
{
    return err_failure;
}

SKERR SKCursorRecordSet::LookupUNumImp(PRUint32 lNum, skfLookupMode mode,
                                    PRUint32 *piId)
{
    return err_failure;
}

SKERR SKCursorRecordSet::LookupSNumImp(PRInt32 lNum, skfLookupMode mode,
                                    PRUint32 *piId)
{
    return err_failure;
}

SKERR SKCursorRecordSet::GetSubRecordSet(PRUint32 lOffset, PRUint32 lCount,
                                         SKIRecordSet** ppIRecordSet)
{
    SKERR err;
    *ppIRecordSet = NULL;

    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pData = m_pCursor->GetSharedCursorDataRead();
    if(!pData)
        return err_failure;

    PRUint32 iSelfCount = 0;
    err = m_pCursor->GetCount(&iSelfCount);
    if(err != noErr)
        return err;

    if(lOffset + lCount >= iSelfCount)
        return err_failure;

    skPtr<SKCursor> pCursor;
    *pCursor.already_AddRefed() = sk_CreateInstance(SKCursor)(lCount,
                                                              pData + lOffset);
    if(!pCursor)
        return err_memory;

    skPtr<SKCursorRecordSet> pRecordSet;
    *pRecordSet.already_AddRefed() = sk_CreateInstance(SKCursorRecordSet)();
    if(!pRecordSet)
        return err_memory;

    err = pRecordSet->Init(pCursor, m_pRecordSet);
    if(err != noErr)
        return err;

    *ppIRecordSet = pRecordSet;
    (*ppIRecordSet)->AddRef();

    return noErr;
}

SKERR SKCursorRecordSet::ExtractCursor(SKIField* pIField, PRUint32 iOffset,
                                       PRUint32 iCount, SKCursor** ppCursor)
{
    SKERR err;
    *ppCursor = NULL;

    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pSelfData = m_pCursor->GetSharedCursorDataRead();
    if(iCount && !pSelfData)
        return err_failure;

    PRUint32 iSelfCount = 0;
    err = m_pCursor->GetCount(&iSelfCount);
    if(err != noErr)
        return err;

    if(iOffset + iCount > iSelfCount)
        return err_failure;

    skPtr<SKCursor> pCursor;
    *pCursor.already_AddRefed() = sk_CreateInstance(SKCursor)(iCount);
    if(!pCursor)
        return err_memory;

    PRUint32 *pData = pCursor->GetSharedCursorDataWrite();
    if((iCount > 0) && !pData)
        return err_failure;

    for(PRUint32 i = 0; i < iCount; ++i)
    {
        skPtr<SKIRecord> pRec;
        err = m_pRecordSet->GetRecord(pSelfData[iOffset + i],
                                      pRec.already_AddRefed());
        if(err != noErr)
            return err;

        err = pRec->GetUNumFieldValue(pIField, pData + i);
        if(err != noErr)
            return err;
    }

    pCursor->ReleaseSharedCursorDataWrite();

    return pCursor.CopyTo(ppCursor);
}

SKERR SKCursorRecordSet::Filter(SKIRecordFilter *pFilter)
{
    SKCursorFilterRecordWrapper cursorFilter;
    SKERR err;

    err = cursorFilter.SetCursor(m_pCursor);
    if(err != noErr)
        return err;

    // we must call SetRecordSet before SetRecordComparator
    err = pFilter->SetRecordSet(m_pRecordSet);
    if(err != noErr)
        return err;

    err = cursorFilter.SetRecordFilter(pFilter, PR_TRUE);
    if(err != noErr)
        return err;

    return m_pCursor->Filter(&cursorFilter);
}

SKERR SKCursorRecordSet::Merge(SKIRecordSet *pRecordSet,
                               skfOperator oper,
                               SKIRecordComparator *pComparator,
                               PRBool bConsiderRank)
{
    SKCursorRecordSet *pRS = (SKCursorRecordSet *) pRecordSet;
    SKCursor *pCursor = pRS->m_pCursor;

    skPtr<SKCursorComparatorRecordWrapper> pCursorComparator;
    *pCursorComparator.already_AddRefed() =
        sk_CreateInstance(SKCursorComparatorRecordWrapper)();
    if (!pCursorComparator)
        return err_memory;
    pCursorComparator->SetConsiderRank(bConsiderRank);

    // we must call SetRecordSet before SetRecordComparator
    SKERR err = pComparator->SetRecordSet(m_pRecordSet);
    if(err != noErr)
        return err;

    err = pCursorComparator->SetRecordComparator(pComparator, PR_TRUE);
    if(err != noErr)
        return err;

    for(PRUint32 i = 0; i < pRS->m_iSpeederCount; ++i)
        AppendSpeeder(pRS->m_ppSpeeders[i]);

    err = m_pCursor->Sort(pCursorComparator);
    if (err != noErr)
        return err;
    return m_pCursor->Merge(pCursor, oper);
}

SKERR SKCursorRecordSet::Sort(SKIRecordComparator *pComparator,
                              PRBool bConsiderRank)
{
    skPtr<SKCursorComparatorRecordWrapper> pCursorComparator;
    *pCursorComparator.already_AddRefed() =
        sk_CreateInstance(SKCursorComparatorRecordWrapper)();
    if (!pCursorComparator)
        return err_memory;
    pCursorComparator->SetConsiderRank(bConsiderRank);

    // we must call SetRecordSet before SetRecordComparator
    SKERR err = pComparator->SetRecordSet(m_pRecordSet);
    if(err != noErr)
        return err;

    err = pCursorComparator->SetRecordComparator(pComparator, PR_TRUE);
    if(err != noErr)
        return err;

    return m_pCursor->Sort(pCursorComparator);
}

SKERR SKCursorRecordSet::FilterToNew(SKIRecordFilter *pFilter,
                                     SKIRecordSet **ppNewRecordSet)
{
    SKERR err;

    PRUint32 iCount = 0;
    err = m_pCursor->GetCount(&iCount);
    if(err != noErr)
        return err;

    err = GetSubRecordSet(0, iCount, ppNewRecordSet);
    if(err != noErr)
        return err;

    return (*ppNewRecordSet)->Filter(pFilter);
}

SKERR SKCursorRecordSet::MergeToNew(SKIRecordSet *pRecordSet, skfOperator oper,
                                    SKIRecordComparator *pComparator,
                                    PRBool bConsiderRank,
                                    SKIRecordSet **ppNewRecordSet)
{
    SKERR err;

    PRUint32 iCount = 0;
    err = m_pCursor->GetCount(&iCount);
    if(err != noErr)
        return err;

    err = GetSubRecordSet(0, iCount, ppNewRecordSet);
    if(err != noErr)
        return err;

    return (*ppNewRecordSet)->Merge(pRecordSet,
        oper, pComparator, bConsiderRank);
}

SKERR SKCursorRecordSet::SortToNew(SKIRecordComparator *pComparator,
                                   PRBool bConsiderRank,
                                   SKIRecordSet **ppNewRecordSet)
{
    SKERR err;

    PRUint32 iCount = 0;
    err = m_pCursor->GetCount(&iCount);
    if(err != noErr)
        return err;

    err = GetSubRecordSet(0, iCount, ppNewRecordSet);
    if(err != noErr)
        return err;

    return (*ppNewRecordSet)->Sort(pComparator, bConsiderRank);
}

SKERR SKCursorRecordSet::GetLookupField(SKIField ** ppField)
{
    *ppField = NULL;
    return err_failure;
}

SKERR SKCursorRecordSet::AppendSpeeder(SKRefCount *pSpeeder)
{
    m_ppSpeeders = (SKRefCount **)
        PR_Realloc(m_ppSpeeders, (m_iSpeederCount + 1) * sizeof(SKRefCount *));
    SK_ASSERT(NULL != m_ppSpeeders);
    m_ppSpeeders[m_iSpeederCount++] = pSpeeder;
    pSpeeder->AddRef();
    return noErr;
}
