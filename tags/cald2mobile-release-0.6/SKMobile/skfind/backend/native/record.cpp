/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: record.cpp,v 1.40.4.5 2005/03/16 13:58:24 bozo Exp $
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

#include <skfind/skfind.h>

#include "stream.h"
#include "field.h"
#include "record.h"
#include "buf.h"
#include "table.h"
#include "lc.h"

SK_REFCOUNT_IMPL(SKFixedRecordPool)
SK_REFCOUNT_IMPL_CREATOR(SKFixedRecordPool)(PRUint32 iPoolSize,
                                            PRUint32 iRecordSize)
{
    return new SKFixedRecordPool(iPoolSize, iRecordSize);
}

SKFixedRecordPool::SKFixedRecordPool(PRUint32 iPoolSize,
                                     PRUint32 iRecordSize) :
        m_iPoolSize(iPoolSize), m_iRecordSize(iRecordSize)
{
    m_pRecordBuffers = NULL;
    m_pRawRecords = NULL;
    m_ppRecords = NULL;
    m_iRecordPointer = 0;
}

SKFixedRecordPool::~SKFixedRecordPool()
{
    SK_ASSERT(m_iRecordPointer == 0);

    if(m_pRecordBuffers)
        PR_Free(m_pRecordBuffers);
    if(m_pRawRecords)
        delete[] m_pRawRecords;
    if(m_ppRecords)
        delete[] m_ppRecords;
}

SKERR SKFixedRecordPool::Init()
{
    m_pRecordBuffers = PR_Malloc(m_iPoolSize * m_iRecordSize);
    if(!m_pRecordBuffers)
        return err_memory;

    m_pRawRecords = new SKRecord[m_iPoolSize];
    if(!m_pRawRecords)
        return err_memory;

    m_ppRecords = new SKRecord*[m_iPoolSize];
    if(!m_ppRecords)
        return err_memory;

    for(PRUint32 i = 0; i < m_iPoolSize; ++i)
    {
        m_pRawRecords[i].SetPool(this);
        m_ppRecords[i] = m_pRawRecords + i;
    }

    return noErr;
}

SKERR SKFixedRecordPool::Terminate()
{
    for(PRUint32 i = 0; i < m_iPoolSize; ++i)
        m_pRawRecords[i].SetPool(NULL);

    return noErr;
}

SKERR SKFixedRecordPool::GetRecord(SKRecord **ppRecord, void *pBuffer,
                                   PRBool bVolatileBuffer)
{
    if(m_iRecordPointer >= m_iPoolSize)
        return err_notfound;

    *ppRecord = m_ppRecords[m_iRecordPointer++];

    SKERR err;
    if(bVolatileBuffer)
    {
        void *pRecordBuffer = (char *)m_pRecordBuffers +
                (*ppRecord - m_pRawRecords) * m_iRecordSize;
        memcpy(pRecordBuffer, pBuffer, m_iRecordSize);

        err = (*ppRecord)->SetBuffer(pRecordBuffer);
    }
    else
    {
        err = (*ppRecord)->SetBuffer(pBuffer);
    }
    if(err != noErr)
    {
        m_iRecordPointer--;
        *ppRecord = NULL;
        return err;
    }
    if(*ppRecord)
        (*ppRecord)->AddRef();

    return noErr;
}

SKERR SKFixedRecordPool::ReleaseRecord(SKRecord *pRecord)
{
    SK_ASSERT(    (pRecord >= m_pRawRecords)
            && (pRecord < m_pRawRecords + m_iPoolSize));

    m_ppRecords[--m_iRecordPointer] = pRecord;

    return noErr;
}

SK_REFCOUNT_IMPL(SKRecordPool);
SK_REFCOUNT_IMPL_CREATOR(SKRecordPool)(PRUint32 iPoolSize,
                                       PRUint32 iRecordSize)
{
    return new SKRecordPool(iPoolSize, iRecordSize);
}

SKRecordPool::SKRecordPool(PRUint32 iPoolSize, PRUint32 iRecordSize) :
        m_iPoolSize(iPoolSize), m_iRecordSize(iRecordSize)
{
    m_iPoolsCount = 0;
    m_ppPools = NULL;
}

SKRecordPool::~SKRecordPool()
{
    if(m_ppPools)
    {
        for(PRUint32 i = 0; i < m_iPoolsCount; ++i)
        {
            if(m_ppPools[i])
            {
                m_ppPools[i]->Terminate();
                m_ppPools[i]->Release();
            }
        }
        PR_Free(m_ppPools);
    }
}

SKERR SKRecordPool::GetRecord(SKRecord **ppRecord, void *pBuffer,
                              PRBool bVolatileBuffer)
{
    SKERR err;

    for(PRUint32 i = 0; i < m_iPoolsCount; ++i)
    {
        err = m_ppPools[i]->GetRecord(ppRecord, pBuffer, bVolatileBuffer);
        // If succeeded then return
        if(err == noErr)
            return noErr;
    }

    // All the pools are full, create a new one
    SKFixedRecordPool * pPool =
        sk_CreateInstance(SKFixedRecordPool)(m_iPoolSize, m_iRecordSize);
    if(!pPool)
        return err_memory;

    err = pPool->Init();
    if(err != noErr)
        return err;

    m_ppPools = (SKFixedRecordPool **)
            PR_Realloc(m_ppPools,
                       (m_iPoolsCount + 1) * sizeof(SKFixedRecordPool *));
    SK_ASSERT(NULL != m_ppPools);
    m_ppPools[m_iPoolsCount++] = pPool;

    return m_ppPools[m_iPoolsCount - 1]->GetRecord(ppRecord, pBuffer,
                                                   bVolatileBuffer);
}


SK_REFCOUNT_IMPL_DEFAULT(SKRecord)

SKRecord::SKRecord()
{
    m_pBuffer = NULL;
    m_lId = (PRUint32)-1;
#ifdef FIELD_CACHE
    m_lFieldCount = 0;
    m_ppFieldCache = NULL;
#endif
    m_lWarnCounter = 1;
}

SKRecord::~SKRecord()
{
#ifdef FIELD_CACHE
    if(m_ppFieldCache)
    {
        skPtr<SKIFldCollection> pFldCol;
        SKERR err = m_pTable->GetFldCollection(pFldCol.already_AddRefed());
        SK_ASSERT(err == noErr);
        SK_ASSERT(NULL != pFldCol);

        skPtr<SKField> pField;
        for(PRUint32 i = 0; i < m_lFieldCount; ++i)
        {
            if(m_ppFieldCache[i])
            {
                err=pFldCol->GetField(i,(SKIField**)pField.already_AddRefed());
                SK_ASSERT(err == noErr);
                FieldType eType = pField->GetType();
                if(eType == SKFT_DATA)
                    ((SKBinary*)m_ppFieldCache[i])->Release();
                else
                    SK_ASSERT(NULL != PR_FALSE);
            }
        }
        delete[] m_ppFieldCache;
    }
#endif
}

void SKRecord::OnRefPostWarn()
{
    m_pPool->ReleaseRecord(this);
    m_pFragment = NULL;
    m_pTable = NULL;
}

SKERR SKRecord::GetUNumFieldValue(SKIField* pIField, PRUint32 *plValue)
{
    if(!m_pTable || !m_pBuffer || !pIField)
        return SKError(err_rec_invalid,"[SKRecord::GetUNumFieldValue] "
                "Invalid record");

    return ((SKField*)pIField)->GetUNumFieldValue(m_pBuffer, plValue);
}

SKERR SKRecord::GetSNumFieldValue(SKIField* pIField, PRInt32 *plValue)
{
    if(!m_pTable || !m_pBuffer || !pIField)
        return SKError(err_rec_invalid,"[SKRecord::GetSNumFieldValue] "
                "Invalid record");
    
    return ((SKField*)pIField)->GetSNumFieldValue(m_pBuffer, plValue);
}

SKERR SKRecord::GetDataFieldValue(SKIField* pIField, SKBinary** ppBinary)
{
    SKERR err;
    *ppBinary = NULL;
    
    if(!m_pTable || !m_pBuffer)
        return SKError(err_rec_invalid,"[SKRecord::GetDataFieldValue] "
                        "Invalid record");

    if(!pIField)
        return SKError(err_rec_invalid,"[SKRecord::GetDataFieldValue] "
                        "Invalid field");
    PRBool bCheckType;
    err = pIField->IsData(&bCheckType);
    SK_ASSERT(err == noErr);
    if(!bCheckType)
        return SKError(err_rec_invalid,"[SKRecord::GetDataFieldValue] "
                        "Not called with a data field");

#ifdef FIELD_CACHE
    if(m_ppFieldCache[((SKField*)pIField)->GetPosition()])
    {
        SK_ASSERT(((SKField*)pIField)->GetType() == SKFT_DATA);
        *ppBinary = (SKBinary*)
            m_ppFieldCache[((SKField*)pIField)->GetPosition()];
        (*ppBinary)->AddRef();
        return noErr;
    }
#endif

    if(m_lId != (PRUint32)-1)
    {
        PRUint32 lCount;
        m_pTable->GetCount(&lCount);
        if(m_lId == lCount-1)
        {
            err = ((SKField*)pIField)
                ->GetDataFieldValue(m_pBuffer, ppBinary, (const void*) -1);
        }
        else
        {
            skPtr<SKIRecord> next;
            m_pTable->GetRecord(m_lId+1, next.already_AddRefed());
            err = ((SKField*)pIField)->
                GetDataFieldValue(m_pBuffer, ppBinary,
                                  ((SKRecord*)(SKIRecord*)next)->
                                      GetSharedBuffer());
        }
    }
    else
    {
        err = ((SKField*)pIField)->GetDataFieldValue(m_pBuffer,ppBinary);
    }

    if(*ppBinary)
    {
#ifdef FIELD_CACHE
        (*ppBinary)->AddRef();
        m_ppFieldCache[((SKField*)pIField)->GetPosition()] = *ppBinary;
#endif
    }

    return err;
}

SKERR SKRecord::GetStreamFieldValue(SKIField* pIField,
                                    SKIStream** ppStream)
{
    SKERR err;
    *ppStream = NULL;
    
    if(!m_pTable || !m_pBuffer)
        return SKError(err_rec_invalid,"[SKRecord::GetStreamFieldValue] "
                        "Invalid record");

    if(m_lId != (PRUint32)-1)
    {
        PRUint32 lCount;
        m_pTable->GetCount(&lCount);
        if(m_lId == lCount-1)
        {
            err = ((SKField*)pIField)
                ->GetStreamFieldValue(m_pBuffer, ppStream, (const void*) -1);
        }
        else
        {
            skPtr<SKIRecord> next;
            m_pTable->GetRecord(m_lId+1, next.already_AddRefed());
            err = ((SKField*)pIField)->
                GetStreamFieldValue(m_pBuffer, ppStream,
                                    ((SKRecord*)(SKIRecord*)next)->
                                    GetSharedBuffer());
        }
    }
    else
    {
        err = ((SKField*)pIField)->GetStreamFieldValue(m_pBuffer, ppStream);
    }

    return err;
}

SKERR SKRecord::GetLinkFieldCount(SKIField* pIField, PRUint32 *piCount)
{
    SKERR err = noErr;
    if(!m_pTable || !m_pBuffer)
        return SKError(err_rec_invalid,"[SKRecord::GetLinkFieldCount] "
                        "Invalid record");
    // get offset
    skPtr<SKField> pOffsetField;
    ((SKField*)pIField)->GetOffsetField(pOffsetField.already_AddRefed());
    SK_ASSERT(NULL != pOffsetField);
    PRUint32 lOffset = 0;
    ((SKField*) pOffsetField)->GetUNumFieldValue(m_pBuffer, &lOffset);

    // is there a length ?
    skPtr<SKField> pCountField;
    ((SKField*)pIField)->GetCountField(pCountField.already_AddRefed());
    PRUint32 lCount = 0;
    if(pCountField)
    {
        pCountField->GetUNumFieldValue(m_pBuffer, &lCount);
    }
    else
    {
        // if we do not know the id of the record, we can not get the data.
        if(m_lId == (PRUint32)-1)
        {
            err = SKError(err_rec_invalid, "[SKRecord::GetDataFieldValue] "
                    "Not count for data and record detached from table.");
        }
        // is it the last record ?
        PRUint32 lTableCount = 0;
        m_pTable->GetCount(&lTableCount);
        if(m_lId == lTableCount - 1)
        {
            // read all the end of the file
            PRUint32 iLinCount = 0;
            skPtr<SKIRecordSet> pLinkRS;
            err = pIField->GetLinkSubRecordSet(pLinkRS.already_AddRefed());
            if(err != noErr)
            {
                return SKError(err, "[SKRecord::GetLinkFieldCount] "
                               "LIN not initialized.");
            }
            err = pLinkRS->GetCount(&iLinCount);
            if(err != noErr)
                return err;
            lCount = iLinCount - lOffset;
        }
        else
        {
            // stop where next record begins.
            PRUint32 lNextOffset;
            skPtr<SKIRecord> next;
            m_pTable->GetRecord(m_lId+1, next.already_AddRefed());
            pOffsetField->GetUNumFieldValue(
                ((SKRecord*)(SKIRecord*)next)->GetSharedBuffer(), &lNextOffset);
            lCount = lNextOffset - lOffset;
        }
    }
    *piCount = lCount;
    return noErr;
};

SKERR SKRecord::GetLinkFieldValue(SKIField* pIField, SKIRecordSet** ppRecordSet)
{
    SKERR err = noErr;
    *ppRecordSet = NULL;
    if(!m_pTable || !m_pBuffer)
        return SKError(err_rec_invalid,"[SKRecord::GetLinkFieldValue] "
                        "Invalid record");

    // get file
    skPtr<SKIRecordSet> pLinkRS;
    err = pIField->GetLinkSubRecordSet(pLinkRS.already_AddRefed());
    if(err != noErr)
    {
        return SKError(err, "[SKRecord::GetLinkFieldValue] "
                       "LIN not initialized.");
    }

    // get offset
    skPtr<SKField> pOffsetField;
    ((SKField*)pIField)->GetOffsetField(pOffsetField.already_AddRefed());
    SK_ASSERT(NULL != pOffsetField);
    PRUint32 lOffset = 0;
    ((SKField*) pOffsetField)->GetUNumFieldValue(m_pBuffer, &lOffset);

    // get count
    PRUint32 lCount = 0;
    err = GetLinkFieldCount(pIField, &lCount);

    return pLinkRS->GetSubRecordSet(lOffset, lCount, ppRecordSet);
}

SKERR SKRecord::GetFldCollection(SKIFldCollection** fldcol)
{
    SK_ASSERT(NULL != m_pTable);
    return m_pTable->GetFldCollection(fldcol);
}

SKERR SKRecord::SetPool(SKFixedRecordPool *pPool)
{
    m_pPool = pPool;
    return noErr;
}

SKERR SKRecord::SetBuffer(void *pBuffer)
{
    m_pBuffer = pBuffer;
    SK_ASSERT(NULL != m_pBuffer);
    return noErr;
}

SKERR SKRecord::SetFragment(SKPageFileFragment *pFragment)
{
    m_pFragment = pFragment;
    return noErr;
}

SKERR SKRecord::GetTable(SKIRecordSet** ppIRecordSet) const
{
    *ppIRecordSet = m_pTable;
    if(*ppIRecordSet)
        (*ppIRecordSet)->AddRef();
    return (*ppIRecordSet == NULL);
}

SKERR SKRecord::SetTable(SKIRecordSet* pTable)
{
    if(!pTable)
        return err_tbl_invalid;

    if(pTable == m_pTable)
        return noErr;

#ifdef FIELD_CACHE
    skPtr<SKIFldCollection> pFldCol;
    SKERR err;

    if(m_ppFieldCache)
    {
        err = m_pTable->GetFldCollection(pFldCol.already_AddRefed());
        if(err != noErr)
            return err_tbl_invalid;
        SK_ASSERT(NULL != pFldCol);

        skPtr<SKField> pField;
        for(PRUint32 i = 0; i < m_lFieldCount; ++i)
        {
            if(m_ppFieldCache[i])
            {
                err=pFldCol->GetField(i,(SKIField**)pField.already_AddRefed());
                SK_ASSERT(err == noErr);
                FieldType eType = pField->GetType();
                if(eType == SKFT_DATA)
                    ((SKBinary*)m_ppFieldCache[i])->Release();
                else if(eType == SKFT_LINK)
                    ((SKIRecordSet*)m_ppFieldCache[i])->Release();
                else
                    SK_ASSERT(PR_FALSE);
            }
        }
        delete[] m_ppFieldCache;
        m_ppFieldCache = NULL;
    }
#endif

    m_pTable = pTable;

#ifdef FIELD_CACHE
    err = m_pTable->GetFldCollection(pFldCol.already_AddRefed());
    if(err != noErr)
        return err_tbl_invalid;
    SK_ASSERT(NULL != pFldCol);

    err = pFldCol->GetFieldCount(&m_lFieldCount);
    if(err != noErr)
        return err_tbl_invalid;

    m_ppFieldCache = new void*[m_lFieldCount];
    for(PRUint32 i = 0; i < m_lFieldCount; ++i)
        m_ppFieldCache[i] = NULL;
#endif

    return pTable ? noErr : err_tbl_invalid;
}

SKRecordSet::SKRecordSet()
{
    m_pRecordCache = NULL;
    m_lCacheCount = 0;
    m_lIncrement = 5;
    m_lDecrement = -1;
}

SKRecordSet::~SKRecordSet()
{
    if(m_pRecordCache)
    {
        for(PRUint32 i = 0; i < m_lCacheCount; ++i)
            if(m_pRecordCache[i])
                delete m_pRecordCache[i];
        delete[] m_pRecordCache;
    }
}

void SKRecordSet::Terminate()
{
    m_lWarnCounter = -1;
    if(m_pRecordCache)
    {
        delete[] m_pRecordCache;
        m_pRecordCache = NULL;
    }
    m_lCacheCount = 0;
    m_lIncrement = 5;
    m_lDecrement = -1;
}

SKERR SKRecordSet::ConfigureItem(char* szSection, char* szToken, char* szValue)
{
    if(!szToken)
        return err_invalid;
    if(!strcmp(szToken, "SIZE"))
    {
        if(m_pRecordCache)
        {
            delete[] m_pRecordCache;
            m_pRecordCache = NULL;
        }

        PRInt32 lCacheSize = atol(szValue);
        if(lCacheSize < 0)
            return SKError(err_rec_invalid, "[SKRecordSet::Configure] "
                           "Cache size not valid (%d)", lCacheSize);

        if(lCacheSize)
        {
            m_pRecordCache = new SKRecordCacheItem[lCacheSize];

            for(PRInt32 i = 1; i < lCacheSize; i++)
            {
                m_pRecordCache[i - 1].m_pNext = m_pRecordCache + i;
                m_pRecordCache[i].m_pPrev = m_pRecordCache + i - 1;
            }

            m_pFirstItem = m_pInsertItem = m_pRecordCache;
            m_pLastItem = m_pRecordCache + lCacheSize - 1;
        }
        return noErr;
    }
    else if(!strcmp(szToken, "SCOREINCREMENT"))
        m_lIncrement = atol(szValue);
    else if(!strcmp(szToken, "SCOREDECREMENT"))
        m_lDecrement = atol(szValue);
    else
        return err_not_handled;
    return noErr;
}

SKERR SKRecordSet::GetCachedRecord(PRUint32 lId, SKIRecord** ppIRecord)
{
    *ppIRecord = NULL;
    if(m_pRecordCache)
    {
        SKRecordCacheItem* pItem = m_pFirstItem;
        m_pInsertItem = NULL;
        while(pItem && (SKIRecord*)*(skPtr<SKIRecord>*)pItem)
        {
            PRUint32 l;
            SKERR err = (*pItem)->GetId(&l);
            SK_ASSERT(err == noErr);
            if(err != noErr)
                return err;
            if(l == lId)
            {
                // This is the result
                *ppIRecord = *pItem;
                // Move up this item
                pItem->m_lScore += m_lIncrement;
                SKRecordCacheItem* pUpItem = pItem;
                while(    pUpItem->m_pPrev
                       && (pUpItem->m_pPrev->m_lScore <= pItem->m_lScore))
                {
                    pUpItem = pUpItem->m_pPrev;
                }
                if(pUpItem != pItem)
                {
                    SKRecordCacheItem* pTmpItem = pItem->m_pPrev;

                    pItem->m_pPrev->m_pNext = pItem->m_pNext;
                    if(pItem->m_pNext)
                        pItem->m_pNext->m_pPrev = pItem->m_pPrev;
                    pItem->m_pPrev = pUpItem->m_pPrev;
                    pItem->m_pNext = pUpItem;
                    if(pItem->m_pPrev)
                        pItem->m_pPrev->m_pNext = pItem;
                    pUpItem->m_pPrev = pItem;

                    if(m_pFirstItem == pUpItem)
                        m_pFirstItem = pItem;
                    if(!pUpItem->m_pNext)
                        m_pLastItem = pUpItem;

                    pItem = pTmpItem;
                }
                pItem = pItem->m_pNext;
                break;
            }
            else
            {
                // Release obsolete data
                if((pItem->m_lScore += m_lDecrement) <= 0)
                {
                    pItem->m_lScore = 0;
                    if(--m_lCacheCount > 0)
                        --m_lWarnCounter;
                    else
                        m_lWarnCounter = -1;
                    *pItem = NULL;
                }
                if(    (pItem->m_lScore <= m_lIncrement)
                    && (    !pItem->m_pPrev
                         || (    pItem->m_pPrev
                              && (pItem->m_pPrev->m_lScore > m_lIncrement))))
                {
                    m_pInsertItem = pItem;
                }
            }
            pItem = pItem->m_pNext;
        }

        while(pItem && (SKIRecord*)*(skPtr<SKIRecord>*)pItem)
        {
            // Release obsolete data
            if((pItem->m_lScore += m_lDecrement) <= 0)
            {
                pItem->m_lScore = 0;
                if(--m_lCacheCount > 0)
                    --m_lWarnCounter;
                else
                    m_lWarnCounter = -1;
                *pItem = NULL;
            }
            if(    (pItem->m_lScore <= m_lIncrement)
                && (    !pItem->m_pPrev
                     || (    pItem->m_pPrev
                          && (pItem->m_pPrev->m_lScore > m_lIncrement))))
            {
                m_pInsertItem = pItem;
            }
            pItem = pItem->m_pNext;
        }
        if(!m_pInsertItem && pItem)
            m_pInsertItem = pItem;
    }
    if(*ppIRecord)
        (*ppIRecord)->AddRef();
    return (*ppIRecord == NULL);
}

void SKRecordSet::InsertRecordInCache(SKIRecord* pIRecord)
{
    SK_ASSERT(NULL != pIRecord);
    if(m_pRecordCache && m_pInsertItem)
    {
        SK_ASSERT(!m_pLastItem->m_pNext);

        SKRecordCacheItem* pNextLastItem = m_pLastItem;

        // Detach the last item
        if(m_pInsertItem != m_pLastItem)
        {
            SK_ASSERT(NULL != m_pLastItem->m_pPrev);
            m_pLastItem->m_pPrev->m_pNext = NULL;
            pNextLastItem = pNextLastItem->m_pPrev;
        }

        // Init the item
        if(!*m_pLastItem)
            ++m_lCacheCount;
        m_lWarnCounter = m_lCacheCount;
        *m_pLastItem = pIRecord;
        m_pLastItem->m_lScore = m_lIncrement;

        if(m_pInsertItem != m_pLastItem)
        {
            // Link it
            m_pLastItem->m_pPrev = m_pInsertItem->m_pPrev;
            m_pLastItem->m_pNext = m_pInsertItem;

            // Back-link it
            if(m_pLastItem->m_pPrev)
                m_pLastItem->m_pPrev->m_pNext = m_pLastItem;
            if(m_pLastItem->m_pNext)
                m_pLastItem->m_pNext->m_pPrev = m_pLastItem;

            // Update the first item
            if(m_pFirstItem == m_pInsertItem)
                m_pFirstItem = m_pLastItem;
            // Update the insert item
            m_pInsertItem = m_pLastItem;
            // Update the last item
            m_pLastItem = pNextLastItem;
        }
    }
}

void SKRecordSet::OnRefPreWarn()
{
    if(m_lReferenceCounter == (PRInt32)(1 + m_lCacheCount))
        Terminate();
}

