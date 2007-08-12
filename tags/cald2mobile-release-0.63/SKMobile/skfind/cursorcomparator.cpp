/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorcomparator.cpp,v 1.12.2.7 2005/02/21 14:22:44 krys Exp $
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

// SKICursorComparator
SK_REFCOUNT_IMPL_IID(SKICursorComparator, SK_SKICURSORCOMPARATOR_IID,
                     SKIComparator)

SKICursorComparator::SKICursorComparator()
{
    m_sortingOrder = skfsoIncreasing;
    m_bConsiderRank = PR_TRUE;
}

SKICursorComparator::~SKICursorComparator()
{
}

PRBool SKICursorComparator::GetConsiderRank()
{
    return m_bConsiderRank;
}

SKERR SKICursorComparator::GetCursorComparatorID(SKBinary** ppBinID)
{
    if (!ppBinID)
        return err_invalid;

    skPtr<SKBinary> pBinComparatorID;
    SKERR err = GetMyComparatorID(pBinComparatorID.already_AddRefed());
    if (err != noErr)
        return err;

    //FIXME Add an identifier for SKIRecordSet, to identify SKIRecordComparator
    char* pStrID = (char*) pBinComparatorID->GetSharedData();
    if (!pStrID)
        return err_failure;

    if (err != noErr)
        return err;
    if (PL_strcmp(pStrID, "UNKNOWN"))
        if (m_sortingOrder == skfsoIncreasing)
            pStrID = PR_sprintf_append(NULL, "+%s", pStrID);
        else
            pStrID = PR_sprintf_append(NULL, "-%s", pStrID);

    err = pBinComparatorID->SetStringData(pStrID);
    PL_strfree(pStrID);
    if (err != noErr)
        return err;
    pBinComparatorID.CopyTo(ppBinID);
    return noErr;
}

SKERR SKICursorComparator::SetConsiderRank(PRBool bConsiderRank)
{
    m_bConsiderRank = bConsiderRank;
    return noErr;
}

SKERR SKICursorComparator::SetSortingOrder(skfSortingOrder sortingOrder)
{
    m_sortingOrder = sortingOrder;
    return noErr;
}

SKERR SKICursorComparator::Compare(const void *p1, const void *p2,
                                   PRInt32 *piResult)
{
    *piResult = 0;
    if(!p1 || !p2)
        return err_failure;

    SKERR err = CompareRanks(*(PRUint32*)p1, *(PRUint32*)p2, piResult);

    if(m_sortingOrder == skfsoDecreasing)
        *piResult = -*piResult;

    return err;
}

// SKCursorComparatorRank
SK_REFCOUNT_IMPL_DEFAULT(SKCursorComparatorRank);

SKERR SKCursorComparatorRank::CompareRanks(PRUint32 iRank1, PRUint32 iRank2,
                                           PRInt32 *piResult)
{
    *piResult = iRank1 - iRank2;

    return noErr;
}

SKERR SKCursorComparatorRank::GetMyComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;
    skPtr<SKBinary> p;
    *p.already_AddRefed() = sk_CreateInstance(SKBinary)();
    if (!p)
        return err_memory;
    p.CopyTo(ppStrID);

    SKERR err = (*ppStrID)->SetStringData("RankCC");
    return err;
}

// SKCursorFilterRecordWrapper
SK_REFCOUNT_IMPL_DEFAULT(SKCursorComparatorRecordWrapper);
SK_REFCOUNT_IMPL_IID(SKCursorComparatorRecordWrapper,
                     SK_SKCURSORCOMPARATORRECORDWRAPPER_IID,
                     SKICursorComparator)

SKCursorComparatorRecordWrapper::SKCursorComparatorRecordWrapper()
{
    m_bNoLookup = PR_TRUE;
    m_iLastRanks[0] = (PRUint32)-1;
    m_iLastRanks[1] = (PRUint32)-1;
    m_bConsiderRank = PR_FALSE;
}

SKERR SKCursorComparatorRecordWrapper::SetRecordComparator(
                SKIRecordComparator *pRecordComparator, PRBool bNoLookup)
{
    m_pRecordComparator = pRecordComparator;
    m_bNoLookup = bNoLookup;
    m_iLastRanks[0] = (PRUint32)-1;
    m_iLastRanks[1] = (PRUint32)-1;
    m_pLastRecords[0] = NULL;
    m_pLastRecords[1] = NULL;

    return m_pRecordComparator->GetRecordSet(m_pRecordSet.already_AddRefed());
}

SKERR SKCursorComparatorRecordWrapper::CompareRanks(
                PRUint32 iRank1, PRUint32 iRank2, PRInt32 *piResult)
{
    // This SK_ASSERTion is invalid in a sorting process
    //SK_ASSERT((m_iLastRanks[0] != iRank1) || (m_iLastRanks[1] != iRank2));
    if(m_iLastRanks[0] != iRank1)
    {
        m_iLastRanks[0] = iRank1;
        m_pLastRecords[0] = NULL;
    }
    if(m_iLastRanks[1] != iRank2)
    {
        m_iLastRanks[1] = iRank2;
        m_pLastRecords[1] = NULL;
    }

    SKERR err;

    if(!m_pRecordSet)
        return err_invalid;

    // optional numerical lookups
    if(!m_bNoLookup)
    {
        if(!m_pLastRecords[0])
        {
            err = m_pRecordSet->LookupUNum(iRank1, skflmEXACT, &iRank1);
            if(err != noErr)
                return err;
        }

        if(!m_pLastRecords[1])
        {
            err = m_pRecordSet->LookupUNum(iRank2, skflmEXACT, &iRank2);
            if(err != noErr)
                return err;
        }
    }

    // fetch the records
    if(!m_pLastRecords[0])
    {
        err = m_pRecordSet->GetRecord(iRank1,
                                      m_pLastRecords[0].already_AddRefed());
        if(err != noErr)
            return err;
    }

    if(!m_pLastRecords[1])
    {
        err = m_pRecordSet->GetRecord(iRank2,
                                      m_pLastRecords[1].already_AddRefed());
        if(err != noErr)
            return err;
    }

    // records comparison
    err = m_pRecordComparator->CompareRecords(m_pLastRecords[0],
                                              m_pLastRecords[1],
                                              piResult);
    if(err != noErr)
        return err;

    // Deterministic or stable sort?
    if (m_bConsiderRank)
      if(!*piResult)
          *piResult = iRank1 - iRank2;

    return noErr;
}

SKERR SKCursorComparatorRecordWrapper::GetMyComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;
    SKERR err = noErr;
    if (m_pRecordComparator)
        err = m_pRecordComparator->GetRecordComparatorID(ppStrID);
    else
    {
        skPtr<SKBinary> p;
        *p.already_AddRefed() = sk_CreateInstance(SKBinary)();
        if (!p)
            return err_memory;
        p.CopyTo(ppStrID);
        err = (*ppStrID)->SetStringData("RecordWrapperCC");
    }
    return err;
}
