/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: recordfilter.cpp,v 1.5.2.4 2005/02/21 14:22:44 krys Exp $
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

// SKIRecordFilter
SK_REFCOUNT_IMPL_IID(SKIRecordFilter, SK_SKIRECORDFILTER_IID, SKRefCount)

SKERR SKIRecordFilter::Reset()
{
    return noErr;
}

SKERR SKIRecordFilter::SetRecordSet(SKIRecordSet *pRecordSet)
{
    m_pRecordSet = pRecordSet;

    return noErr;
}

SKERR SKIRecordFilter::GetRecordSet(SKIRecordSet **ppRecordSet)
{
    return m_pRecordSet.CopyTo(ppRecordSet);
}

// SKRecordFilterJoinRSOnUNum
SK_REFCOUNT_IMPL_DEFAULT(SKRecordFilterJoinRSOnUNum);
SK_REFCOUNT_IMPL_IID(SKRecordFilterJoinRSOnUNum,
                     SK_SKRECORDFILTERJOINRSONUNUM_IID,
                     SKIRecordFilter)

SKRecordFilterJoinRSOnUNum::SKRecordFilterJoinRSOnUNum()
{
    m_pszFieldName = NULL;
    m_iLastRank = 0;
}

SKRecordFilterJoinRSOnUNum::~SKRecordFilterJoinRSOnUNum()
{
    if(m_pszFieldName)
        PL_strfree(m_pszFieldName);
}

SKERR SKRecordFilterJoinRSOnUNum::SetFieldName(const char *pszFieldName)
{
    if(m_pszFieldName)
        PL_strfree(m_pszFieldName);
    m_pField = NULL;

    m_pszFieldName = PL_strdup(pszFieldName);
    if(!m_pszFieldName)
        return err_memory;

    return noErr;
}

SKERR SKRecordFilterJoinRSOnUNum::SetFilterData(SKCursor* pFilterCursor,
                                                SKIRecordSet *pFilterRecordSet,
                                                const char *pszFilterFieldName)
{
    skPtr<SKIFldCollection> pCol;
    SKERR err = pFilterRecordSet->GetFldCollection(pCol.already_AddRefed());
    if(err != noErr)
        return err;

    err = pCol->GetField(pszFilterFieldName, m_pFilterField.already_AddRefed());
    if(err != noErr)
        return err;

    m_pFilterCursor = pFilterCursor;
    m_pFilterRecordSet = pFilterRecordSet;

    return noErr;
}

SKERR SKRecordFilterJoinRSOnUNum::SetRecordSet(SKIRecordSet *pRecordSet)
{
    skPtr<SKIFldCollection> pCol;
    SKERR err = pRecordSet->GetFldCollection(pCol.already_AddRefed());
    if(err != noErr)
        return err;

    err = pCol->GetField(m_pszFieldName, m_pField.already_AddRefed());
    if(err != noErr)
        return err;

    return SKIRecordFilter::SetRecordSet(pRecordSet);
}

SKERR SKRecordFilterJoinRSOnUNum::Reset()
{
    m_iLastRank = 0;
    return noErr;
}

SKERR SKRecordFilterJoinRSOnUNum::CheckRecord(SKIRecord *pRecord,
                                              PRBool *pbKeepIt)
{
    *pbKeepIt = PR_FALSE;

    SKERR err;
    PRUint32 iValue;
    err = pRecord->GetUNumFieldValue(m_pField, &iValue);
    if(err != noErr)
        return err;

    PRUint32 iFilterCount;
    err = m_pFilterCursor->GetCount(&iFilterCount);
    if(err != noErr)
        return err;

    if (m_iLastRank >= iFilterCount)
    {
        return noErr;
    }

    PRUint32 iFilterValue;
    do
    {
        skPtr<SKIRecord> pFilterRecord;
        err = m_pFilterCursor->GetRecord(m_pFilterRecordSet, m_iLastRank,
                                         PR_TRUE,
                                         pFilterRecord.already_AddRefed());
        if(err != noErr)
            return err;

        err = pFilterRecord->GetUNumFieldValue(m_pFilterField, &iFilterValue);
        if(err != noErr)
            return err;

        if(iValue > iFilterValue)
            m_iLastRank++;
    }
    while((iValue > iFilterValue) && (m_iLastRank < iFilterCount));

    *pbKeepIt = (iValue == iFilterValue);

    return noErr;
}

// SKRecordFilterUNumBitField
SK_REFCOUNT_IMPL_DEFAULT(SKRecordFilterUNumBitField);
SK_REFCOUNT_IMPL_IID(SKRecordFilterUNumBitField,
                     SK_SKRECORDFILTERUNUMBITFIELD_IID,
                     SKIRecordFilter)

SKRecordFilterUNumBitField::SKRecordFilterUNumBitField()
{
    m_pszLinkFieldName = NULL;
    m_bNoLinkLookup = PR_TRUE;

    m_pszFieldName = NULL;

    m_bDefaultPolicy = PR_TRUE;
    m_iRuleSize = 0;
    m_iRuleCount = 0;
    m_pRules = NULL;
}

SKRecordFilterUNumBitField::~SKRecordFilterUNumBitField()
{
    if(m_pszLinkFieldName)
        PL_strfree(m_pszLinkFieldName);
    if(m_pszFieldName)
        PL_strfree(m_pszFieldName);

    if(m_pRules)
        PR_Free(m_pRules);
}

SKERR SKRecordFilterUNumBitField::SetOptionalLink(const char *pszLinkFieldName,
                                                  SKIRecordSet *pLinkRecordSet,
                                                  PRBool bNoLinkLookup)
{
    if(m_pszLinkFieldName)
        PL_strfree(m_pszLinkFieldName);
    m_pszLinkFieldName = PL_strdup(pszLinkFieldName);
    if(!m_pszLinkFieldName)
        return err_memory;

    m_pLinkRecordSet = pLinkRecordSet;
    m_bNoLinkLookup = bNoLinkLookup;

    return noErr;
}

SKERR SKRecordFilterUNumBitField::SetField(const char *pszFieldName)
{
    if(m_pszFieldName)
        PL_strfree(m_pszFieldName);
    m_pszFieldName = PL_strdup(pszFieldName);
    if(!m_pszFieldName)
        return err_memory;

    return noErr;
}

SKERR SKRecordFilterUNumBitField::Clear()
{
    m_bDefaultPolicy = PR_TRUE;
    m_iRuleCount = 0;
    return noErr;
}

SKERR SKRecordFilterUNumBitField::SetDefaultPolicy(PRBool bPolicy)
{
    m_bDefaultPolicy = bPolicy;
    return noErr;
}

SKERR SKRecordFilterUNumBitField::AddRule(PRBool bAction,
                  PRUint32 iBitShift, PRUint32 iBitMask,
                  PRUint32 iMinValue, PRUint32 iMaxValue)
{
    if(m_iRuleCount == m_iRuleSize)
    {
        m_iRuleSize += 10;
    m_pRules = (BitFieldRule*) PR_Realloc(m_pRules,
                    m_iRuleSize * sizeof(BitFieldRule));
    }
    SK_ASSERT(NULL != m_pRules);
    if(!m_pRules)
        return SKError(err_memory,
                "[SKCursorFilterUNumBitField::AddRule] "
                "Cannot allocate memory");

    m_pRules[m_iRuleCount].bAction = bAction;
    m_pRules[m_iRuleCount].iBitShift = iBitShift;
    m_pRules[m_iRuleCount].iBitMask = iBitMask;
    m_pRules[m_iRuleCount].iMin = iMinValue;
    m_pRules[m_iRuleCount].iMax = iMaxValue;
    m_iRuleCount++;
    return noErr;
}

SKERR SKRecordFilterUNumBitField::AddAcceptedValue(PRUint32 value)
{
    return AddRule(true, 0, 0xffffffff, value, value);
}

SKERR SKRecordFilterUNumBitField::AddAcceptedRange(PRUint32 min, PRUint32 max)
{
    return AddRule(true, 0, 0xffffffff, min, max);
}

SKERR SKRecordFilterUNumBitField::AddRejectedValue(PRUint32 value)
{
    return AddRule(false, 0, 0xffffffff, value, value);
}

SKERR SKRecordFilterUNumBitField::AddRejectedRange(PRUint32 min, PRUint32 max)
{
    return AddRule(false, 0, 0xffffffff, min, max);
}

SKERR SKRecordFilterUNumBitField::CheckRecord(SKIRecord *pRecord,
                                              PRBool *pbKeepIt)
{
    SKERR err;
    PRUint32 lValue, lRId, i, lBits;

    *pbKeepIt = m_bDefaultPolicy;

    skPtr<SKIRecord> pRec(pRecord);

    // optionally resolve the link
    if(m_pLinkRecordSet && m_pLinkField)
    {
        err = pRec->GetUNumFieldValue(m_pLinkField, &lRId);
        if(err != noErr)
            return err;
        if (!m_bNoLinkLookup)
        {
            err = m_pLinkRecordSet->LookupUNum(lRId, skflmEXACT, &lRId);
            if(err != noErr)
                return err;
        }
        err = m_pLinkRecordSet->GetRecord(lRId, pRec.already_AddRefed());
        if(err != noErr)
            return err;
    }

    err = pRec->GetUNumFieldValue(m_pField, &lValue);
    if(err != noErr)
        return err;

    for(i = 0 ; i < m_iRuleCount ; i++)
    {
        lBits = (lValue >> m_pRules[i].iBitShift) & m_pRules[i].iBitMask;
        if (lBits >= m_pRules[i].iMin && lBits <= m_pRules[i].iMax)
        {
            *pbKeepIt = m_pRules[i].bAction;
            break;
        }
    }

    return noErr;
}

SKERR SKRecordFilterUNumBitField::SetRecordSet(SKIRecordSet *pRecordSet)
{
    SK_ASSERT(NULL != pRecordSet);
    if(!pRecordSet)
        return err_invalid;

    SKERR err;

    skPtr<SKIFldCollection> pCol;
    err = pRecordSet->GetFldCollection(pCol.already_AddRefed());
    if(err != noErr)
        return err;

    if(m_pszLinkFieldName && m_pLinkRecordSet)
    {
        err = pCol->GetField(m_pszLinkFieldName,
                             m_pLinkField.already_AddRefed());
        if(err != noErr)
            return err;

        err = m_pLinkRecordSet->GetFldCollection(pCol.already_AddRefed());
        if(err != noErr)
            return err;

        err = pCol->GetField(m_pszFieldName, m_pField.already_AddRefed());
        if(err != noErr)
            return err;
    }
    else
    {
        err = pCol->GetField(m_pszFieldName, m_pField.already_AddRefed());
        if(err != noErr)
            return err;
    }

    return SKIRecordFilter::SetRecordSet(pRecordSet);
}

SK_REFCOUNT_IMPL_DEFAULT(skfRecordFilterUNumInCursor);
SK_REFCOUNT_IMPL_IID(skfRecordFilterUNumInCursor,
                     SK_SKFRECORDFILTERUNUMINCURSOR_IID,
                     SKIRecordFilter)

SKERR skfRecordFilterUNumInCursor::SetCursor(SKCursor* pCursor)
{
    m_pCursor = pCursor;
    return noErr;
}

SKERR skfRecordFilterUNumInCursor::CheckRecord(SKIRecord* pRecord,
        PRBool* pbKeepIt)
{
    if(!m_pField)
        return err_invalid;
    if(!pRecord)
        return err_invalid;
    SKERR err;
    PRUint32 iValue;
    err = pRecord->GetUNumFieldValue(m_pField, &iValue);
    if(err != noErr)
        return err;
    return m_pCursor->Lookup(iValue, pbKeepIt);
}
