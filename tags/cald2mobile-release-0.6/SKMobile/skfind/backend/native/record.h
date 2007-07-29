/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: record.h,v 1.17.4.4 2005/03/16 13:58:24 bozo Exp $
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

#ifndef __RECORD_H_
#define __RECORD_H_

#define err_rec_invalid 2011

class SKFixedRecordPool;
class SKPageFileFragment;

class SKAPI SKRecord : public SKIRecord
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecord)

                  SKRecord();
                  ~SKRecord();
    virtual void OnRefPostWarn();

            SKERR SetId(PRUint32 id) { m_lId = id; return noErr; };
            SKERR GetId(PRUint32 *plId) { *plId = m_lId; return noErr; }
                    
    virtual SKERR GetUNumFieldValue(SKIField* pIField,
                                    PRUint32 *plValue);
    virtual SKERR GetSNumFieldValue(SKIField* pIField,
                                    PRInt32 *plValue);
    virtual SKERR GetDataFieldValue(SKIField* pIField,
                                    SKBinary** ppBinary);
    virtual SKERR GetStreamFieldValue(SKIField* pIField,
                                      SKIStream** ppStream);
    virtual SKERR GetLinkFieldCount(SKIField*, PRUint32*);
    virtual SKERR GetLinkFieldValue(SKIField* pIField,
                                    SKIRecordSet** result);

            SKERR SetPool(SKFixedRecordPool* pPool);
            SKERR SetBuffer(void *pBuffer);
            SKERR SetFragment(SKPageFileFragment *pFragment);
            SKERR GetTable(SKIRecordSet** ppIRecordSet) const;
            SKERR SetTable(SKIRecordSet* pTable);


            SKERR GetFldCollection(SKIFldCollection** pCol);
    
    const void*   GetSharedBuffer() const { return m_pBuffer; }

private:
    skPtr<SKIRecordSet> m_pTable;
    skPtr<SKFixedRecordPool>    m_pPool;
    skPtr<SKPageFileFragment>   m_pFragment;
    void*               m_pBuffer;
    PRUint32            m_lId;

#ifdef FIELD_CACHE
    PRUint32            m_lFieldCount;
    void**              m_ppFieldCache;
#endif
};

class SKAPI SKFixedRecordPool : public SKRefCount
{
public:
    SK_REFCOUNT_INTF(SKFixedRecordPool)
    SK_REFCOUNT_INTF_CREATOR(SKFixedRecordPool)(PRUint32 iPoolSize,
                                                PRUint32 iRecordSize);

    SKFixedRecordPool(PRUint32 iPoolSize, PRUint32 iRecordSize);
    ~SKFixedRecordPool();

    SKERR Init();
    SKERR Terminate();

    SKERR GetRecord(SKRecord **ppRecord, void *pBuffer, PRBool bVolatileBuffer);
    SKERR ReleaseRecord(SKRecord *pRecord);

private:
    SKFixedRecordPool();

    const PRUint32 m_iPoolSize;
    const PRUint32 m_iRecordSize;

    void *m_pRecordBuffers;
    SKRecord *m_pRawRecords;
    SKRecord **m_ppRecords;

    // first free record in m_ppRecords
    PRUint32 m_iRecordPointer;
};

class SKAPI SKRecordPool : public SKRefCount
{
public:
    SK_REFCOUNT_INTF(SKRecordPool)
    SK_REFCOUNT_INTF_CREATOR(SKRecordPool)(PRUint32 iPoolSize,
                                           PRUint32 iRecordSize);

    SKRecordPool(PRUint32 iPoolSize, PRUint32 iRecordSize);
    ~SKRecordPool();

    SKERR GetRecord(SKRecord **ppRecord, void *pBuffer, PRBool bVolatileBuffer);
    
private:
    SKRecordPool();

    const PRUint32 m_iPoolSize;
    const PRUint32 m_iRecordSize;

    PRUint32 m_iPoolsCount;
    SKFixedRecordPool **m_ppPools;
};

class SKAPI SKRecordCacheItem : public skPtr<SKIRecord>
{
public:
    SKRecordCacheItem() { m_lScore = 0; m_pPrev = m_pNext = NULL; };

    SKIRecord* operator=(SKIRecord* lp)
    {
        return skPtr<SKIRecord>::operator=(lp);
    }

    PRInt32             m_lScore;

    SKRecordCacheItem*  m_pPrev;
    SKRecordCacheItem*  m_pNext;
};

class SKAPI SKRecordSet : public SKIRecordSet
{
public:
    SKRecordSet();
    ~SKRecordSet();

    void Terminate();

    SKERR ConfigureItem(char* seSection, char* szToken, char* szValue);

    SKERR GetCachedRecord(PRUint32 lId, SKIRecord** ppIRecord);
    void InsertRecordInCache(SKIRecord* pIRecord);

protected:
    virtual void        OnRefPreWarn();

    SKRecordCacheItem*  m_pRecordCache;

    PRUint32            m_lCacheCount;

    PRInt32             m_lIncrement;
    PRInt32             m_lDecrement;

    SKRecordCacheItem*  m_pFirstItem;
    SKRecordCacheItem*  m_pInsertItem;
    SKRecordCacheItem*  m_pLastItem;
};

#else
#error "Multiple inclusions of record.h"
#endif

