/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: table.cpp,v 1.82.2.11 2005/02/21 14:22:39 krys Exp $
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

#ifdef HAVE_CONFIG_H
#include "skbuildconfig.h"
#endif

#include <skfind/skfind.h>

#include "field.h"
#include "record.h"
#include "buf.h"
#include "table.h"
#include "lc.h"

#define DEFAULT_CONFIG_CFT "config.cft"
#define DEFAULT_DIRECTORY_SUFFIX ".skn"

#ifdef SK_USE_EXTERNAL_PROTECTION
extern "C" unsigned long SKFactoryExternalProtection(const char* pszUrl);
extern "C" unsigned long SKFactoryExternalProtectionLogout();
#endif

SK_REFCOUNT_IMPL_DEFAULT(SKTable)
SK_REFCOUNT_IMPL_IID(SKTable, SK_SKTABLE_IID, SKIRecordSet)

SKTable::SKTable()
{
    m_lFirstID = 0;
    m_bIsValid = PR_FALSE;
    m_lCount = 0;
    m_bIsProtected = PR_FALSE;
}

SKTable::~SKTable()
{
    Terminate();
#ifdef SK_USE_EXTERNAL_PROTECTION
    if (m_bIsProtected)
        SKFactoryExternalProtectionLogout();
#endif
    m_bIsValid = PR_FALSE;
}

SKERR SKTable::GetRecord(PRUint32 id, SKIRecord** ppIRecord)
{
    SKERR err;

    err = GetCachedRecord(id, ppIRecord);
    if(err == noErr)
        return noErr;

    err = m_pDatFile->GetRecord(id, ppIRecord);
    if(err == noErr)
    {
        ((SKRecord*)(*ppIRecord))->SetTable(this);
        InsertRecordInCache(*ppIRecord);
    }
    return err;
}

class TableDelegate : public SKConfigurationDelegate
{
public:
    SKERR ConfigureItem(char* pszSection, char* pszName, char* pszValue)
    {
        return m_pTable->ConfigureItem(pszSection, pszName, pszValue);
    }
    SKTable* m_pTable;
};

SKERR SKTable::SetFileName(const char* szFileName)
{
    SKERR   err;
    TableDelegate delegate;
    delegate.m_pTable = this;

    // SKRecordSet
    Terminate();

#ifdef SK_USE_EXTERNAL_PROTECTION
    err = SKFactoryExternalProtection(szFileName);
    if(err != noErr)
        return err;
    m_bIsProtected = PR_TRUE;
#endif

    SKEnvir *pEnvir = NULL;
    err = SKEnvir::GetEnvir(&pEnvir);
    if(err != noErr)
        return err;

    // inherited
    err = m_File.SetFileName(szFileName, DEFAULT_CONFIG_CFT);
    if(err != noErr)
    {
        char * pc = PR_smprintf("%s%s", szFileName, DEFAULT_DIRECTORY_SUFFIX);
        err = m_File.SetFileName(pc, DEFAULT_CONFIG_CFT);
        PR_smprintf_free(pc);
        if(err != noErr)
            return SKError(err_notfound, "[SKTable::SetFileName] Cannot find config file for table `%s'.",szFileName);
    }


    m_File.SetConfigurationDelegate(&delegate);

    err = m_File.ParseConfiguration();

    // check structure
    if (err != noErr || (err = Check()) != noErr)
        return err;

    // handle number of elements
    m_lCount = m_pDatFile->GetRecordCount();

    // ready to work
    m_bIsValid = PR_TRUE;

    return noErr;
}

SKERR SKTable::ConfigureItem(char* pszSection, char* pszName, char* pszValue)
{
#ifdef DEBUG_CONFIG
    fprintf(stderr, "SKTable:: [%s] %s = %s\n", pszSection, pszName, pszValue);
#endif
    // [GENERAL]
    if (!PL_strcmp (pszSection, "GENERAL"))
        return err_not_handled;
    // [RECORDCACHE]
    else if (!PL_strcmp (pszSection, "RECORDCACHE"))
        return SKRecordSet::ConfigureItem(pszSection, pszName, pszValue);
    // [DAT]
    else if (!PL_strcmp (pszSection, "DAT"))
    {
        if (!m_pDatFile)
            *m_pDatFile.already_AddRefed()=sk_CreateInstance(SKPageFile)();
        return m_pDatFile->ConfigureItem(pszSection, pszName, pszValue);
    }
    // [LC]
    else if (!PL_strcmp (pszSection, "LC"))
    {
        if(!m_pLcFile)
        {
            *m_pLcFile.already_AddRefed() =
                sk_CreateInstance(SKLCFile)();
        }
        if(pszName && !PL_strcmp(pszName, "FIELD"))
        {
            if(m_pSearchableField)
            {
                return  SKError(err_tbl_invalid, "[SKTable::SetFileName] "
                        "LC file record given twice or more.", pszValue);
            }
            else
            {
                m_pDatFile->GetWeakFldCollection()->GetField(pszValue,
                    (SKIField**)m_pSearchableField.already_AddRefed());
                if(!m_pSearchableField)
                {
                    return SKError(err_fld_unknown, "[SKTable::Configure] "
                            "Field %s is unknown.", pszValue);
                }
            }
            return noErr;
        }
        else
        {
            return m_pLcFile->ConfigureItem(pszSection, pszName, pszValue);
        }
    }
    // [$... -> configuration of a subfile
    else if (*pszSection == '$')
    {
        // get the file
        char * szPackid = PL_strstr(pszSection,",");
        long lPackId = 0;

        // separate the field
        if(szPackid != NULL && szPackid > pszSection)
        {
            // section is in the form of $FIELD,12
            *(szPackid) = 0;
        }
        skPtr<SKIField> pField;
        m_pDatFile->GetWeakFldCollection()->GetField(pszSection + 1,
            pField.already_AddRefed());
        if(pField == NULL)
        {
            return SKError(err_cnf_invalid, "[SKTable::SetFileName] "
                    "Field %s unknwon.", pszSection+1);
        }
        else if(szPackid != NULL && szPackid > pszSection)
        {
            // put the coma back in place
            *(szPackid) = ',';
            // compute the package id
            lPackId = atol(szPackid + 1);
        }

        // configure the field
        return ((SKField*)(SKIField*)pField)->
                ConfigureComposite(lPackId, pszName, pszValue);
    }
    return err_not_handled;
}

SKERR SKTable::Check()
{
    if(!m_pDatFile)
        return SKError(err_tbl_invalid, "[SKTable::Check] No DAT file (%s).",
                       m_File.GetSharedFileName());

    SKERR err = m_pDatFile->Check();
    if(err != noErr)
        return err;
    if(m_pLcFile)
    {
        err = m_pLcFile->Check();
        if(err != noErr)
            return err;
    }

    return noErr;
}

SKERR SKTable::GetLookupField(SKIField ** ppField)
{
    if(!m_pLcFile)
        return SKError(err_tbl_invalid, "[SKTable::GetLookupField] No LC file.");

    *ppField = m_pSearchableField;
    (*ppField)->AddRef();
    return noErr;
}

SKERR SKTable::LookupTextImp(const char* pszSearch, skfLookupMode mode,
                          PRUint32 *plId)
{
    SKERR       err;
    PRUint32    lPageNum;
    LcPagePtr   pPage;

    SK_ASSERT(NULL != m_pLcFile);
    if(!m_pLcFile)
        return SKError(err_tbl_invalid, "[SKTable::LookupTextImp] No LC file.");

    SK_ASSERT(NULL != pszSearch);
    if(!pszSearch)
        return SKError(err_tbl_invalid, "[SKTable::LookupTextImp]"
                    " Invalid search");

    SK_ASSERT(m_pSearchableField->GetType() == SKFT_DATA);

    err = m_pLcFile->Lookup(pszSearch, SKLCFile::STRING, &lPageNum);
    if(err == err_lc_notfound)
    {
        if(mode == skflmEXACT)
            return err_notfound;
        else if(lPageNum == (PRUint32)-1)
            return err_notfound;
        else
        {
            if(lPageNum + 1 < m_pLcFile->GetPageCount())
                *plId = (m_pDatFile->GetPageSize() * (lPageNum + 1)) - 1;
            else
                *plId = m_pDatFile->GetRecordCount() - 1;
            return noErr;
        }
    }

    if(err == noErr)
    {
        // gets the page
        pPage = m_pLcFile->GetPage(lPageNum);

        // load corresponding DAT page
        err = m_pDatFile->LoadPage(lPageNum);

        if(err == noErr)
        {
            // is it the last page ?
            PRBool IsLastPage = (lPageNum == m_pLcFile->GetPageCount() - 1);

            // how many record we must store
            PRUint32 lPhysicalPageSize = IsLastPage
                    ? m_pDatFile->GetRecordCount() % m_pDatFile->GetPageSize()
                    : m_pDatFile->GetPageSize();
            // the last page isn't empty
            if(!lPhysicalPageSize && IsLastPage)
                lPhysicalPageSize = m_pDatFile->GetPageSize();

            // load corresponding TDA page
            skPtr<SKPageFile> pTda;
            m_pSearchableField->GetFile(pTda.already_AddRefed());
            err = pTda->Load(pPage->offset, pPage->size);
			if(err != noErr)
			{
				return SKError(err,
					"[SKTable::LookupTextImp] "
					"Failed to load corresponding TDA page. offset = %d, size = %d, win32Error = %d",
					pPage->offset, pPage->size, ::GetLastError());
			}

            // lookup in the page
            err = SubLookupText(m_pDatFile->GetBufferPtr(), lPhysicalPageSize,
                                IsLastPage, pszSearch, mode, plId);
            if(err == err_notfound)
            {
                if((mode == skflmLASTBEFORE) && (lPageNum > 0))
                {
                    *plId = m_pDatFile->GetPageSize() * lPageNum -1;
                    err = noErr;
                }
                return err;
            }

            // correct id to right page
            (*plId) += m_pDatFile->GetPageSize() * lPageNum;
        }

        err = m_pDatFile->Unload();
    }

    return err ? err_failure : noErr;
}

SKERR SKTable::SubLookupText(void *pBuffer, PRUint32 iCount,
                             PRBool bIsLastPage, const char* pszSearch,
                             skfLookupMode mode, PRUint32 *piId)
{
    PRUint32 iRecordSize = GetRecordSize();
    // inf is the id of the last record of the table known to be too small
    PRUint32 lInf = (PRUint32)-1;

    // sup is the id of the first record of the table known to be too big
    // if the "subtable" is in a page of a table (!bIsLastPage), the
    // last record is the first of the following page, so it is already too big
    // (we just need it to complete info for the last usefull record)
    PRUint32 lSup = iCount;

    PRUint32 lPos = (PRUint32)-1;
    long lCmp = 0;
    SKERR err = noErr;
    skPtr<SKBinary> pBinary;
    // dichotomize in the page
    while((err == noErr) && (lSup > lInf + 1))
    {
        lPos = (lInf + lSup) / 2;
        // get record
        const void * record = (char*) pBuffer + iRecordSize * lPos;
        // get next record (if such one exist)
        const void * nextRecord;
        if(bIsLastPage && (lPos + 1 == iCount))
            nextRecord = (void*)-1;
        else
            nextRecord = (char*)record + iRecordSize;
        // get data
        err = m_pSearchableField->GetDataFieldValue(record,
                        pBinary.already_AddRefed(), (const void*)nextRecord);
        // compare
        if(err == noErr)
        {
            lCmp = PL_strcmp(pszSearch, (char*)pBinary->GetSharedData());
            if(!lCmp)
                break;
            else if (lCmp > 0)
                lInf = lPos;
            else
                lSup = lPos;
        }
    }

    if(err == noErr)
    {
        if(!lCmp)
        {
            if(mode == skflmLASTBEFORE)
                --lPos;
        }
        else if(lCmp < 0)
        {
            --lPos;
            if(    (lPos != (PRUint32)-1)
                && (mode == skflmEXACT))
            {
                // get record
                const void * record = (char*) pBuffer + iRecordSize * lPos;
                // get next record (if such one exist)
                const void * nextRecord;
                if(bIsLastPage && (lPos + 1 == iCount))
                    nextRecord = (void*)-1;
                else
                    nextRecord = (char*)record + iRecordSize;
                // get data
                err = m_pSearchableField->GetDataFieldValue(record,
                                pBinary.already_AddRefed(),
                                (const void*)nextRecord);
                if(    (err == noErr)
                    && (PL_strcmp(pszSearch,
                                  (char*)pBinary->GetSharedData()) > 0))
                {
                    return err_notfound;
                }
            }
        }
        else if(lCmp > 0)
        {
            if(mode == skflmEXACT)
                return err_notfound;
        }

        if(err == noErr)
        {
            if(lPos >= iCount)
                return err_notfound;

            *piId = lPos;
            return noErr;
        }
    }

    return err_failure;
}

SKERR SKTable::LookupUNumImp(PRUint32 lNum, skfLookupMode mode, PRUint32 *plId)
{
    SKERR       err;
    PRUint32    lPageNum;
    LcPagePtr   pPage;

    if(!m_pLcFile)
        return SKError(err_tbl_invalid, "[SKTable::LookupUNumImp] No LC file.");

    SK_ASSERT(IsAnUnsignedFieldType(m_pSearchableField->GetType()));

    err = m_pLcFile->Lookup((const char*)&lNum, SKLCFile::UINT, &lPageNum);
    if(err == err_lc_notfound)
    {
        if(mode == skflmEXACT)
            return err_notfound;
        else if(lPageNum == (PRUint32)-1)
            return err_notfound;
        else
        {
            if(lPageNum + 1 < m_pLcFile->GetPageCount())
                *plId = (m_pDatFile->GetPageSize() * (lPageNum + 1)) - 1;
            else
                *plId = m_pDatFile->GetRecordCount() - 1;
            return noErr;
        }
    }

    if(err == noErr)
    {
        // gets the page
        pPage = m_pLcFile->GetPage(lPageNum);

        // load corresponding DAT page
        err = m_pDatFile->LoadPage(lPageNum);

        if(err == noErr)
        {
            // is it the last page ?
            PRBool IsLastPage = (lPageNum == m_pLcFile->GetPageCount() - 1);

            // how many record we must store
            PRUint32 lPhysicalPageSize = IsLastPage
                    ? m_pDatFile->GetRecordCount() % m_pDatFile->GetPageSize()
                    : m_pDatFile->GetPageSize();
            // the last page isn't empty
            if(!lPhysicalPageSize && IsLastPage)
                lPhysicalPageSize = m_pDatFile->GetPageSize();

            // lookup in the page
            err = SubLookupUNum(m_pDatFile->GetBufferPtr(), lPhysicalPageSize,
                                lNum, mode, plId);
            if(err == err_notfound)
            {
                if((mode == skflmLASTBEFORE) && (lPageNum > 0))
                {
                    *plId = m_pDatFile->GetPageSize() * lPageNum -1;
                    err = noErr;
                }
                return err;
            }

            // correct id to right page
            (*plId) += m_pDatFile->GetPageSize() * lPageNum;
        }

        err = m_pDatFile->Unload();
    }

    return err ? err_failure : noErr;
}

SKERR SKTable::SubLookupUNum(void *pBuffer, PRUint32 iCount,
                             PRUint32 lNum, skfLookupMode mode, PRUint32 *plId)
{
    PRUint32 iRecordSize = GetRecordSize();
    // inf is the id of the last record of the table known to be too small
    PRUint32 lInf = (PRUint32)-1;

    // sup is the id of the first record of the table known to be too big
    // if the "subtable" is in a page of a table (!bIsLastPage), the
    // last record is the first of the following page, so it is already too big
    // (we just need it to complete info for the last usefull record)
    PRUint32 lSup = iCount;

    PRUint32 lPos = (PRUint32)-1;
    long lCmp = 0;
    SKERR err = noErr;
    PRUint32 lValue;
    // dichotomize in the page
    while((err == noErr) && (lSup > lInf + 1))
    {
        lPos = (lInf + lSup) / 2;
        // get record
        const void * record = (char*) pBuffer + iRecordSize * lPos;
        // get data
        err = m_pSearchableField->GetUNumFieldValue(record, &lValue);
        // compare
        if(err == noErr)
        {
            lCmp = (lNum > lValue) - (lNum < lValue);
            if(!lCmp)
                break;
            else if (lCmp > 0)
                lInf = lPos;
            else
                lSup = lPos;
        }
    }

    if(err == noErr)
    {
        if(!lCmp)
        {
            if(mode == skflmLASTBEFORE)
                --lPos;
        }
        else if(lCmp < 0)
        {
            --lPos;
            if(    (lPos != (PRUint32)-1)
                && (mode == skflmEXACT))
            {
                // get record
                const void * record = (char*) pBuffer + iRecordSize * lPos;
                // get data
                err = m_pSearchableField->GetUNumFieldValue(record, &lValue);
                if((err == noErr) && (lNum - lValue > 0))
                    return err_notfound;
            }
        }
        else if(lCmp > 0)
        {
            if(mode == skflmEXACT)
                return err_notfound;
        }

        if(err == noErr)
        {
            if(lPos >= iCount)
                return err_notfound;

            *plId = lPos;
            return noErr;
        }
    }

    return err_failure;
}

SKERR SKTable::LookupSNumImp(PRInt32 lNum, skfLookupMode mode, PRUint32 *plId)
{
    SKERR       err;
    PRUint32    lPageNum;
    LcPagePtr   pPage;

    SK_ASSERT(NULL != m_pLcFile);
    if(!m_pLcFile)
        return SKError(err_tbl_invalid, "[SKTable::LookupSNumImp] No LC file.");

    SK_ASSERT(IsASignedFieldType(m_pSearchableField->GetType()));

    err = m_pLcFile->Lookup((const char*)&lNum, SKLCFile::SINT, &lPageNum);
    if(err == err_lc_notfound)
    {
        if(mode == skflmEXACT)
            return err_notfound;
        else if(lPageNum == (PRUint32)-1)
            return err_notfound;
        else
        {
            if(lPageNum + 1 < m_pLcFile->GetPageCount())
                *plId = (m_pDatFile->GetPageSize() * (lPageNum + 1)) - 1;
            else
                *plId = m_pDatFile->GetRecordCount() - 1;
            return noErr;
        }
    }

    if(err == noErr)
    {
        // gets the page
        pPage = m_pLcFile->GetPage(lPageNum);

        // load corresponding DAT page
        err = m_pDatFile->LoadPage(lPageNum);

        if(err == noErr)
        {
            // is it the last page ?
            PRBool IsLastPage = (lPageNum == m_pLcFile->GetPageCount() - 1);

            // how many record we must store
            PRUint32 lPhysicalPageSize = IsLastPage
                    ? m_pDatFile->GetRecordCount() % m_pDatFile->GetPageSize()
                    : m_pDatFile->GetPageSize();
            // the last page isn't empty
            if(!lPhysicalPageSize && IsLastPage)
                lPhysicalPageSize = m_pDatFile->GetPageSize();

            // lookup in the page
            err = SubLookupSNum(m_pDatFile->GetBufferPtr(), lPhysicalPageSize,
                                lNum, mode, plId);
            if(err == err_notfound)
            {
                if((mode == skflmLASTBEFORE) && (lPageNum > 0))
                {
                    *plId = m_pDatFile->GetPageSize() * lPageNum -1;
                    err = noErr;
                }
                return err;
            }

            // correct id to right page
            (*plId) += m_pDatFile->GetPageSize() * lPageNum;
        }

        err = m_pDatFile->Unload();
    }

    return err ? err_failure : noErr;
}

SKERR SKTable::SubLookupSNum(void *pBuffer, PRUint32 iCount,
                             PRInt32 lNum, skfLookupMode mode, PRUint32 *plId)
{
    PRUint32 iRecordSize = GetRecordSize();
    // inf is the id of the last record of the table known to be too small
    PRUint32 lInf = (PRUint32)-1;

    // sup is the id of the first record of the table known to be too big
    // if the "subtable" is in a page of a table (!bIsLastPage), the
    // last record is the first of the following page, so it is already too big
    // (we just need it to complete info for the last usefull record)
    PRUint32 lSup = iCount;

    PRUint32 lPos = (PRUint32)-1;
    long lCmp = 0;
    SKERR err = noErr;
    PRInt32 lValue;
    // dichotomize in the page
    while((err == noErr) && (lSup > lInf + 1))
    {
        lPos = (lInf + lSup) / 2;
        // get record
        const void * record = (char*) pBuffer + iRecordSize * lPos;
        // get data
        err = m_pSearchableField->GetSNumFieldValue(record, &lValue);
        // compare
        if(err == noErr)
        {
            lCmp = (lNum > lValue) - (lNum < lValue);
            if(!lCmp)
                break;
            else if (lCmp > 0)
                lInf = lPos;
            else
                lSup = lPos;
        }
    }

    if(err == noErr)
    {
        if(!lCmp)
        {
            if(mode == skflmLASTBEFORE)
                --lPos;
        }
        else if(lCmp < 0)
        {
            --lPos;
            if(    (lPos != (PRUint32)-1)
                && (mode == skflmEXACT))
            {
                // get record
                const void * record = (char*) pBuffer + iRecordSize * lPos;
                // get data
                err = m_pSearchableField->GetSNumFieldValue(record, &lValue);
                if((err == noErr) && (lNum - lValue > 0))
                    return err_notfound;
            }
        }
        else if(lCmp > 0)
        {
            if(mode == skflmEXACT)
                return err_notfound;
        }

        if(err == noErr)
        {
            if(lPos >= iCount)
                return err_notfound;

            *plId = lPos;
            return noErr;
        }
    }

    return err_failure;
}

SKERR SKTable::GetSubRecordSet(PRUint32 lOffset, PRUint32 lCount,
                               SKIRecordSet** ppIRecordSet)
{
    if(lOffset + lCount > m_pDatFile->GetRecordCount())
        return SKError(err_tbl_invalid, "[SKTable::GetSubRecordSet] "
                       "Invalid record boundaries (%s: %u <= %u)",
                       m_File.GetSharedFileName(),
                       lOffset + lCount, m_pDatFile->GetRecordCount());

    *ppIRecordSet = NULL;

    SKERR err;

    skPtr<SKPageFileFragment> pFragment;
    if(lOffset + lCount == m_pDatFile->GetRecordCount())
        err = m_pDatFile->LoadStaticRecord(lOffset, lCount,
                                           pFragment.already_AddRefed());
    else
        err = m_pDatFile->LoadStaticRecord(lOffset, lCount + 1,
                                           pFragment.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIRecordSet> pRecordSet;
    SKFactoryGetRecordSet("cursor:", pRecordSet, err);
    if(err != noErr)
        return err;

    err = pRecordSet->InitCursorRS(lOffset, lCount, this, pFragment);
    if(err != noErr)
        return err;

    *ppIRecordSet = pRecordSet;
    (*ppIRecordSet)->AddRef();

    return noErr;
}

SKERR SKTable::ExtractCursor(SKIField* pIField, PRUint32 iOffset,
                             PRUint32 iCount, SKCursor** ppCursor)
{
    if(iOffset + iCount > m_pDatFile->GetRecordCount())
        return SKError(err_tbl_invalid, "[SKTable::GetCursor] "
                       "Invalid record boundaries");

    SKERR err = m_pDatFile->LoadRecord(iOffset, iCount);
    if(err != noErr)
        return err;

    char* pBuffer = (char*)m_pDatFile->GetBufferPtr();
    PRUint32 iRecordSize = m_pDatFile->GetWeakFldCollection()->GetRecordSize();

    *ppCursor = sk_CreateInstance(SKCursor)(iCount, NULL);
    if(!*ppCursor)
        return err_failure;

    err = (*ppCursor)->ComputeCursorForm();
    if (err != noErr)
        return err;
    PRUint32* pData = (*ppCursor)->GetSharedCursorDataWrite();

    for(PRUint32 i = 0; i < iCount; ++i)
    {
        err = ((SKField*)pIField)->GetUNumFieldValue(pBuffer, pData++);
        if(err != noErr)
        {
            sk_DeleteInstance(*ppCursor);
            *ppCursor = NULL;
            break;
        }
        pBuffer += iRecordSize;
    }

    m_pDatFile->Unload();

    (*ppCursor)->ReleaseSharedCursorDataWrite();

    return err;
}

SKERR SKTable::Filter(SKIRecordFilter *pFilter)
{
    return err_failure;
}

SKERR SKTable::Merge(SKIRecordSet *pRecordSet, skfOperator oper,
                     SKIRecordComparator *pComparator,
                     PRBool bConsiderRank)
{
    return err_failure;
}

SKERR SKTable::Sort(SKIRecordComparator *pComparator,
                    PRBool bConsiderRank)
{
    return err_failure;
}

SKERR SKTable::FilterToNew(SKIRecordFilter *pFilter,
                           SKIRecordSet **ppNewRecordSet)
{
    SKERR err = GetSubRecordSet(0, m_lCount, ppNewRecordSet);
    if(err != noErr)
        return err;

    return (*ppNewRecordSet)->Filter(pFilter);
}

SKERR SKTable::MergeToNew(SKIRecordSet *pRecordSet, skfOperator oper,
                          SKIRecordComparator *pComparator,
                          PRBool bConsiderRank,
                          SKIRecordSet **ppNewRecordSet)
{
    SKERR err = GetSubRecordSet(0, m_lCount, ppNewRecordSet);
    if(err != noErr)
        return err;

    return (*ppNewRecordSet)->Merge(pRecordSet,
        oper, pComparator, bConsiderRank);
}

SKERR SKTable::SortToNew(SKIRecordComparator *pComparator,
                         PRBool bConsiderRank,
                         SKIRecordSet **ppNewRecordSet)
{
    SKERR err = GetSubRecordSet(0, m_lCount, ppNewRecordSet);
    if(err != noErr)
        return err;

    return (*ppNewRecordSet)->Sort(pComparator, bConsiderRank);
}

