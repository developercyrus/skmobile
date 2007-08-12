/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: bucket.cpp,v 1.3.4.2 2005/02/21 14:22:44 krys Exp $
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
#include "bucket.h"

class skfRealBucket : public skfBucket
{
public:
    SK_REFCOUNT_INTF_DEFAULT(skfRealBucket)

    skfRealBucket();
    virtual ~skfRealBucket();
    SKERR Init();

    virtual SKERR GetRecordSet(const char *pszRSURL,
                               SKIRecordSet **ppRecordSet);

private:
    static void * PR_CALLBACK AllocTable(void *pool, PRSize size);
    static void PR_CALLBACK FreeTable(void *pool, void *item);
    static PLHashEntry * PR_CALLBACK AllocEntry(void *pool, const void *key);
    static void PR_CALLBACK FreeEntry(void *pool, PLHashEntry *he,PRUintn flag);
    static PLHashAllocOps m_sAllocOps;

    PLHashTable *m_pRecordSetHash;
};

SK_REFCOUNT_IMPL_DEFAULT(skfRealBucket)

PLHashAllocOps skfRealBucket::m_sAllocOps =
    { skfRealBucket::AllocTable, skfRealBucket::FreeTable,
      skfRealBucket::AllocEntry, skfRealBucket::FreeEntry };

static skPtr<skfBucket> g_pTheBucket;


skfRealBucket::skfRealBucket()
{
    m_pRecordSetHash = NULL;
}

skfRealBucket::~skfRealBucket()
{
    if(m_pRecordSetHash)
        PL_HashTableDestroy(m_pRecordSetHash);
}

SKERR skfRealBucket::Init()
{
    m_pRecordSetHash = PL_NewHashTable(32, PL_HashString,
                                       PL_CompareStrings, PL_CompareValues,
                                       &m_sAllocOps, this);
    if(!m_pRecordSetHash)
        return err_memory;

    return noErr;
}

SKERR skfRealBucket::GetRecordSet(const char *pszRSURL,
                                  SKIRecordSet **ppRecordSet)
{
    *ppRecordSet = (SKIRecordSet *)PL_HashTableLookup(m_pRecordSetHash,
                                                      pszRSURL);
    if(*ppRecordSet)
    {
        (*ppRecordSet)->AddRef();
        return noErr;
    }

    char *pszComponent = (char *)PR_Malloc(PL_strlen(pszRSURL) + 11);
    if(!pszComponent)
        return err_memory;

    PL_strcpy(pszComponent, "recordset:");
    PL_strcpy(pszComponent + 10, pszRSURL);

    SKERR err;

    SKFactory* pFactory;
    err = SKFactory::GetFactory(&pFactory);
    if(err != noErr || pFactory == NULL)
    {
        PR_Free(pszComponent);
        return err;
    }

    err = pFactory->CreateInstance(pszComponent, (SKRefCount **)ppRecordSet);
    PR_Free(pszComponent);
    if(err != noErr)
        return err;

    char *pszKey = PL_strdup(pszRSURL);
    if(!pszKey)
    {
        (*ppRecordSet)->Release();
        *ppRecordSet = NULL;
        return err_memory;
    }

    (*ppRecordSet)->AddRef();
    PL_HashTableAdd(m_pRecordSetHash, pszKey, *ppRecordSet);

    return noErr;
}

void * PR_CALLBACK skfRealBucket::AllocTable(void *pool, PRSize size)
{
    return PR_MALLOC(size);
}

void PR_CALLBACK skfRealBucket::FreeTable(void *pool, void *item)
{
    PR_Free(item);
}

PLHashEntry * PR_CALLBACK skfRealBucket::AllocEntry(void *pool, const void *key)
{
    return PR_NEW(PLHashEntry);
}

void PR_CALLBACK skfRealBucket::FreeEntry(void *pool, PLHashEntry *he,
                                          PRUintn flag)
{
    if(flag == HT_FREE_ENTRY)
    {
        PL_strfree((char*)he->key);
        ((SKIRecordSet*)he->value)->Release();
        PR_Free(he);
    }
    else if(flag == HT_FREE_VALUE)
    {
        PL_strfree((char*)he->key);
        ((SKIRecordSet*)he->value)->Release();
    }
}

SKERR skfBucket::GetBucket(skfBucket **ppBucket)
{
    if(!g_pTheBucket)
    {
        *g_pTheBucket.already_AddRefed() = sk_CreateInstance(skfRealBucket)();
        SKERR err = ((skfRealBucket*)(skfBucket *)g_pTheBucket)->Init();
        if(err != noErr)
            return err;
    }
    return g_pTheBucket.CopyTo(ppBucket);
}

