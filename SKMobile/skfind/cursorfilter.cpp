/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorfilter.cpp,v 1.8.2.4 2005/02/21 14:22:44 krys Exp $
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
#include "cursorcomparator.h"
#include "recordcomparator.h"
#include "cursor.h"

#include "recordfilter.h"
#include "cursorfilter.h"

// SKICursorFilter
SK_REFCOUNT_IMPL_IID(SKICursorFilter, SK_SKICURSORFILTER_IID, SKRefCount)

SKERR SKICursorFilter::Reset()
{
    return noErr;
}

SKERR SKICursorFilter::SetCursor(SKCursor* pCursor)
{
    m_pCursor = pCursor;
    return noErr;
}

// SKCursorFilterRemoveDuplicated
SK_REFCOUNT_IMPL_DEFAULT(SKCursorFilterRemoveDuplicated);
SK_REFCOUNT_IMPL_IID(SKCursorFilterRemoveDuplicated,
                     SK_SKCURSORFILTERREMOVEDUPLICATED_IID,
                     SKICursorFilter)

SKERR SKCursorFilterRemoveDuplicated::CheckRank(PRUint32 iRank,PRBool *pbKeepIt)
{
    SKERR err = noErr;
    if (iRank == 0)
    {
        *pbKeepIt = PR_TRUE;
    }
    else
    {
        err = m_pCursor->ComputeCursorForm();
        if (err != noErr)
            return err;
        *pbKeepIt = (m_pCursor->GetSharedCursorDataRead()[iRank - 1] !=
                     m_pCursor->GetSharedCursorDataRead()[iRank]);
    }
    return err;
}

// SKCursorFilterRecordWrapper
SK_REFCOUNT_IMPL_DEFAULT(SKCursorFilterRecordWrapper);
SK_REFCOUNT_IMPL_IID(SKCursorFilterRecordWrapper,
                     SK_SKCURSORFILTERRECORDWRAPPER_IID,
                     SKICursorFilter)

SKCursorFilterRecordWrapper::SKCursorFilterRecordWrapper()
{
    m_bNoLookup = PR_TRUE;
    m_iLastId = (PRUint32)-1;
}

SKERR SKCursorFilterRecordWrapper::SetRecordFilter(
                SKIRecordFilter *pRecordFilter, PRBool bNoLookup)
{
    m_pRecordFilter = pRecordFilter;
    m_bNoLookup = bNoLookup;
    m_iLastId = (PRUint32)-1;
    m_pLastRecord = NULL;

    return m_pRecordFilter->GetRecordSet(m_pRecordSet.already_AddRefed());
}

SKERR SKCursorFilterRecordWrapper::Reset()
{
    return m_pRecordFilter->Reset();
}

SKERR SKCursorFilterRecordWrapper::CheckRank(PRUint32 iRank, PRBool* pbKeepIt)
{
    SKERR err;
    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pData = m_pCursor->GetSharedCursorDataRead();
    if(!pData)
        return err_failure;

    PRUint32 iCount = 0;
    err = m_pCursor->GetCount(&iCount);
    if(err != noErr)
        return err;

    if(iRank >= iCount)
        return err_failure;

    PRUint32 iId = pData[iRank];
    if(iId != m_iLastId)
    {
        m_iLastId = iId;
        m_pLastRecord = NULL;
    }

    // optional numerical lookup
    if(!m_bNoLookup && !m_pLastRecord)
    {
        err = m_pRecordSet->LookupUNum(iId, skflmEXACT, &iId);
        if(err != noErr)
            return err;
    }

    if(!m_pLastRecord)
    {
        // fetch the record
        err = m_pRecordSet->GetRecord(iId, m_pLastRecord.already_AddRefed());
        if(err != noErr)
            return err;
    }

    // record filtering
    return m_pRecordFilter->CheckRecord(m_pLastRecord, pbKeepIt);
}

SKERR SKCursorFilterRecordWrapper::SetCursor(SKCursor *pCursor)
{
    m_iLastId = (PRUint32)-1;
    m_pLastRecord = NULL;

    return SKICursorFilter::SetCursor(pCursor);
}

