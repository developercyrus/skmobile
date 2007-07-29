/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: recordcomparator.cpp,v 1.8.2.5 2005/02/21 14:22:44 krys Exp $
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

// SKIRecordComparator
SK_REFCOUNT_IMPL_IID(SKIRecordComparator, SK_SKIRECORDCOMPARATOR_IID,
                     SKRefCount)

SKIRecordComparator::SKIRecordComparator()
{
    m_sortingOrder = skfsoIncreasing;
}

SKERR SKIRecordComparator::SetRecordSet(SKIRecordSet *pRecordSet)
{
    m_pRecordSet = pRecordSet;

    return noErr;
}

SKERR SKIRecordComparator::GetRecordComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;

    return GetMyComparatorID(ppStrID);
}

SKERR SKIRecordComparator::GetRecordSet(SKIRecordSet **ppRecordSet)
{
    return m_pRecordSet.CopyTo(ppRecordSet);
}

SKERR SKIRecordComparator::SetSortingOrder(skfSortingOrder sortingOrder)
{
    m_sortingOrder = sortingOrder;
    return noErr;
}

// SKRecordComparatorBaseField
SK_REFCOUNT_IMPL_IID(SKRecordComparatorBaseField,
                     SK_SKRECORDCOMPARATORBASEFIELD_IID,
                     SKIRecordComparator)

SKRecordComparatorBaseField::SKRecordComparatorBaseField()
{
    m_pszLinkFieldName = NULL;
    m_bNoLinkLookup = PR_TRUE;

    m_pszFieldName = NULL;
}

SKRecordComparatorBaseField::~SKRecordComparatorBaseField()
{
    if(m_pszLinkFieldName)
        PL_strfree(m_pszLinkFieldName);
    if(m_pszFieldName)
        PL_strfree(m_pszFieldName);
}

SKERR SKRecordComparatorBaseField::SetOptionalLink(
                const char *pszLinkFieldName,
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

SKERR SKRecordComparatorBaseField::SetField(const char *pszFieldName)
{
    if(m_pszFieldName)
        PL_strfree(m_pszFieldName);
    m_pszFieldName = PL_strdup(pszFieldName);
    if(!m_pszFieldName)
        return err_memory;

    return noErr;
}

SKERR SKRecordComparatorBaseField::SetRecordSet(SKIRecordSet *pRecordSet)
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

    return SKIRecordComparator::SetRecordSet(pRecordSet);
}

SKERR SKRecordComparatorBaseField::GetResolvedRecord(SKIRecord** ppRecord)
{
    if (!ppRecord)
        return err_invalid;
    if (!*ppRecord)
        return err_invalid;

    // optionally resolve the link
    if(m_pLinkRecordSet && m_pLinkField)
    {
        SKERR err;

        PRUint32 iId;
        err = (*ppRecord)->GetUNumFieldValue(m_pLinkField, &iId);
        if(err != noErr)
            return err;

        if(!m_bNoLinkLookup)
        {
            err = m_pLinkRecordSet->LookupUNum(iId, skflmEXACT, &iId);
            if(err != noErr)
                return err;
        }

        err = m_pLinkRecordSet->GetRecord(iId, ppRecord);
        if(err != noErr)
            return err;
    }
    return noErr;
}

// SKRecordComparatorUNumField
SK_REFCOUNT_IMPL_DEFAULT(SKRecordComparatorUNumField);
SK_REFCOUNT_IMPL_IID(SKRecordComparatorUNumField,
                     SK_SKRECORDCOMPARATORUNUMFIELD_IID,
                     SKRecordComparatorBaseField)

SKERR SKRecordComparatorUNumField::CompareRecords(
                SKIRecord *pRecord1, SKIRecord *pRecord2, PRInt32 *piResult)
{
    SKERR err;

    PRUint32 iValue1 = 0;
    err = GetValue(pRecord1, &iValue1);
    if(err != noErr)
        return err;

    PRUint32 iValue2 = 0;
    err = GetValue(pRecord2, &iValue2);
    if(err != noErr)
        return err;

    if(iValue1 == iValue2)
        *piResult = 0;
    else if(iValue1 < iValue2)
        *piResult = -1;
    else
        *piResult = 1;

    if(m_sortingOrder == skfsoDecreasing)
        *piResult = -(*piResult);

    return noErr;
}

SKERR SKRecordComparatorUNumField::GetMyComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;

    SKERR err = noErr;

    skPtr<SKBinary> p;
    *p.already_AddRefed() = sk_CreateInstance(SKBinary)();
    if (!p)
        return err_memory;
    p.CopyTo(ppStrID);

    if ((m_pLinkField) && (m_pRecordSet))
    {
        //FIXME Add an identifier for SKIRecordSet
        err = (*ppStrID)->SetStringData("UNKNOWN");
    }
    else
    {
        char* pStrID =
            PR_sprintf_append(NULL, "UNumFieldRC(%s)", m_pszFieldName);
        (*ppStrID)->SetStringData(pStrID);
        PL_strfree(pStrID);
    }
    return err;
}

SKERR SKRecordComparatorUNumField::GetValue(SKIRecord *pRecord,
                                            PRUint32 *piValue)
{
    SKIRecord *pRec = pRecord;
    SKERR err = GetResolvedRecord(&pRec);
    if (err != noErr)
        return err;
    return pRec->GetUNumFieldValue(m_pField, piValue);
}

// SKRecordComparatorSNumField
SK_REFCOUNT_IMPL_DEFAULT(SKRecordComparatorSNumField);
SK_REFCOUNT_IMPL_IID(SKRecordComparatorSNumField,
                     SK_SKRECORDCOMPARATORSNUMFIELD_IID,
                     SKRecordComparatorBaseField)

SKERR SKRecordComparatorSNumField::CompareRecords(
                SKIRecord *pRecord1, SKIRecord *pRecord2, PRInt32 *piResult)
{
    SKERR err;

    PRInt32 iValue1 = 0;
    err = GetValue(pRecord1, &iValue1);
    if(err != noErr)
        return err;

    PRInt32 iValue2 = 0;
    err = GetValue(pRecord2, &iValue2);
    if(err != noErr)
        return err;

    *piResult = iValue1 - iValue2;

    if(m_sortingOrder == skfsoDecreasing)
        *piResult = -(*piResult);

    return noErr;
}

SKERR SKRecordComparatorSNumField::GetMyComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;

    SKERR err = noErr;

    skPtr<SKBinary> p;
    *p.already_AddRefed() = sk_CreateInstance(SKBinary)();
    if (!p)
        return err_memory;
    p.CopyTo(ppStrID);

    if ((m_pLinkField) && (m_pRecordSet))
    {
        //FIXME Add an identifier for SKIRecordSet
        err = (*ppStrID)->SetStringData("UNKNOWN");
    }
    else
    {
        char* pStrID =
            PR_sprintf_append(NULL, "SNumFieldRC(%s)", m_pszFieldName);
        (*ppStrID)->SetStringData(pStrID);
        PL_strfree(pStrID);
    }
    return err;
}

SKERR SKRecordComparatorSNumField::GetValue(SKIRecord *pRecord,
                                            PRInt32 *piValue)
{
    SKIRecord *pRec = pRecord;
    SKERR err = GetResolvedRecord(&pRec);
    if (err != noErr)
        return err;
    return pRec->GetSNumFieldValue(m_pField, piValue);
}

// SKRecordComparatorDataField
SK_REFCOUNT_IMPL_DEFAULT(SKRecordComparatorDataField);
SK_REFCOUNT_IMPL_IID(SKRecordComparatorDataField,
                     SK_SKRECORDCOMPARATORDATAFIELD_IID,
                     SKRecordComparatorBaseField)

SKERR SKRecordComparatorDataField::CompareRecords(
                SKIRecord *pRecord1, SKIRecord *pRecord2, PRInt32 *piResult)
{
    SKERR err;

    skPtr<SKBinary> pValue1;
    err = GetValue(pRecord1, pValue1.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKBinary> pValue2;
    err = GetValue(pRecord2, pValue2.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 iSize1 = pValue1->GetSize();
    PRUint32 iSize2 = pValue2->GetSize();

    PRUint32 iSize = (iSize1 < iSize2) ? iSize1 : iSize2;
    *piResult = memcmp(pValue1->GetSharedData(), pValue2->GetSharedData(),
                       iSize);

    if(*piResult == 0)
        *piResult = iSize1 - iSize2;

    if(m_sortingOrder == skfsoDecreasing)
        *piResult = -(*piResult);

    return noErr;
}

SKERR SKRecordComparatorDataField::GetMyComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;

    SKERR err = noErr;

    skPtr<SKBinary> p;
    *p.already_AddRefed() = sk_CreateInstance(SKBinary)();
    if (!p)
        return err_memory;
    p.CopyTo(ppStrID);
    if ((m_pLinkField) && (m_pRecordSet))
    {
        //FIXME Add an identifier for SKIRecordSet
        err = (*ppStrID)->SetStringData("UNKNOWN");
    }
    else
    {
        char* pStrID =
            PR_sprintf_append(NULL, "DataFieldRC(%s)", m_pszFieldName);
        (*ppStrID)->SetStringData(pStrID);
        PL_strfree(pStrID);
    }
    return err;
}

SKERR SKRecordComparatorDataField::GetValue(SKIRecord *pRecord,
                                            SKBinary **ppValue)
{
    SKIRecord *pRec = pRecord;
    SKERR err = GetResolvedRecord(&pRec);
    if (err != noErr)
        return err;
    return pRec->GetDataFieldValue(m_pField, ppValue);
}

// SKRecordComparatorChain
SK_REFCOUNT_IMPL_DEFAULT(SKRecordComparatorChain);
SK_REFCOUNT_IMPL_IID(SKRecordComparatorChain,
                     SK_SKRECORDCOMPARATORCHAIN_IID,
                     SKIRecordComparator)

SKRecordComparatorChain::SKRecordComparatorChain()
{
    m_iComparatorCount = 0;
    m_ppComparators = NULL;
}

SKRecordComparatorChain::~SKRecordComparatorChain()
{
    if(m_ppComparators != NULL)
    {
        for(PRUint32 i = 0; i < m_iComparatorCount; ++i)
        {
            if(m_ppComparators[i] != NULL)
                m_ppComparators[i]->Release();
        }
        PR_Free(m_ppComparators);
    }
}

SKERR SKRecordComparatorChain::AddComparator(SKIRecordComparator *pComparator)
{
    m_ppComparators = (SKIRecordComparator **)PR_Realloc(m_ppComparators,
                    (m_iComparatorCount + 1) * sizeof(SKIRecordComparator *));
    if(m_ppComparators == NULL)
        return err_memory;

    m_ppComparators[m_iComparatorCount] = pComparator;
    pComparator->AddRef();
    m_iComparatorCount++;

    return noErr;
}

SKERR SKRecordComparatorChain::CompareRecords(
                SKIRecord *pRecord1, SKIRecord *pRecord2, PRInt32 *piResult)
{
    for(PRUint32 i = 0; i < m_iComparatorCount; ++i)
    {
        if(m_ppComparators != NULL)
        {
            SKERR err = m_ppComparators[i]->CompareRecords(pRecord1, pRecord2,
                                                           piResult);
            if(err != noErr)
                return err;

            if(*piResult)
                break;
        }
    }

    if(m_sortingOrder == skfsoDecreasing)
        *piResult = -(*piResult);

    return noErr;
}

SKERR SKRecordComparatorChain::SetRecordSet(SKIRecordSet *pRecordSet)
{
    for(PRUint32 i = 0; i < m_iComparatorCount; ++i)
    {
        if(m_ppComparators != NULL)
        {
            SKERR err = m_ppComparators[i]->SetRecordSet(pRecordSet);
            if(err != noErr)
                return err;
        }
    }

    return SKIRecordComparator::SetRecordSet(pRecordSet);
}

SKERR SKRecordComparatorChain::GetMyComparatorID(SKBinary** ppStrID)
{
    if (!ppStrID)
        return err_invalid;

    //FIXME Add an identifier for SKRecordSet

    skPtr<SKBinary> pBinNxtElementID;
    char* strNxtElementID = NULL;
    char* pStrChainID;
    SKERR err = noErr;
    PRBool bIsUnknown = PR_FALSE;

    if (m_iComparatorCount)
    {
        err = m_ppComparators[0]->GetRecordComparatorID(
            pBinNxtElementID.already_AddRefed());
        if (err != noErr)
            return err;

        strNxtElementID = (char*) pBinNxtElementID->GetSharedData();
        if (!PL_strcmp(strNxtElementID, "UNKNOWN"))
            bIsUnknown = PR_TRUE;

        pStrChainID = PR_sprintf_append(NULL, "ChainRC{%s", strNxtElementID);

        PRUint32 i = 1;
        while ((!bIsUnknown) && (i < m_iComparatorCount))
        {
            err = m_ppComparators[i]->GetRecordComparatorID(
                pBinNxtElementID.already_AddRefed());
            if (err != noErr)
                return err;

            strNxtElementID = (char*) pBinNxtElementID->GetSharedData();
            if (!PL_strcmp(strNxtElementID, "UNKNOWN"))
                bIsUnknown = PR_TRUE;

            pStrChainID = 
                PR_sprintf_append(pStrChainID, "|%s", strNxtElementID);
            i++;
        }
        if (bIsUnknown)
        {
            if (pStrChainID)
                PR_smprintf_free(pStrChainID);
            pStrChainID = PR_sprintf_append(NULL, "UNKNOWN");
        }
        else
            pStrChainID = PR_sprintf_append(pStrChainID, "}");
    }
    else
        pStrChainID = PR_sprintf_append(NULL, "ChainRC{}");

    skPtr<SKBinary> pBinChainID;
    *pBinChainID.already_AddRefed() = sk_CreateInstance(SKBinary)();
    if (!pBinChainID)
        return err_memory;
    pBinChainID.CopyTo(ppStrID);
    (*ppStrID)->SetStringData(pStrChainID);
    if (pStrChainID)
        PR_smprintf_free(pStrChainID);
    return err;
}

