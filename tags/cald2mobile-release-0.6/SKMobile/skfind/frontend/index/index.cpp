/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: index.cpp,v 1.80.2.11 2005/02/21 14:22:46 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
 *          Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
 *          Marc Ariberti <ariberti @at@ idm .dot. fr>
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

#include <skfind/frontend/wordlist/wordlist.h>
#include <skfind/frontend/wordlist/wildcardwordlist.h>

#include "parse.h"
#include "index.h"
#include "tokens.h"

#define DEFAULT_NEAR_THRESHOLD  10

// SKIndex
SK_REFCOUNT_IMPL_DEFAULT(SKIndex)
SK_REFCOUNT_IMPL_IID(SKIndex, SK_SKINDEX_IID, SKTextFile)

SKIndex::SKIndex(void)
{
    m_bInitialized = PR_FALSE;
    m_bFileName = PR_FALSE;
    m_bSubResultsStoredSorted = PR_FALSE;

    m_lLinkCount = 0;
    m_ppszLinkNames = NULL;
    m_ppszSubLinkNames = NULL;
    m_pszLinks = NULL;
    m_pbMerge = NULL;
    m_pHardSepList = NULL;
    m_pSoftSepList = NULL;
    m_pSoftSepExcList = NULL;
    m_iNearThreshold = DEFAULT_NEAR_THRESHOLD;

    if(!m_pszOperators)
        m_pszOperators = PL_strdup("or(and(not(near(before");
}

#define __CleanPsz(pszName)                                             \
    if(pszName)                                                         \
    {                                                                   \
        PL_strfree(pszName);                                            \
        pszName = NULL;                                                 \
    }

#define __CleanPpsz(ppszName)                                           \
    if (ppszName != NULL)                                               \
    {                                                                   \
        PRUint32 __i;                                                   \
        for(__i = 0 ; __i < m_lLinkCount ; __i++)                       \
        {                                                               \
            __CleanPsz(ppszName[__i]);                                  \
        }                                                               \
        PR_Free(ppszName);                                              \
        ppszName = NULL;                                                \
    }

#define __CleanAllPsz()                                                 \
    __CleanPpsz(m_ppszLinkNames)                                        \
    __CleanPpsz(m_ppszSubLinkNames)                                     \
    if (m_pbMerge != NULL)                                              \
    {                                                                   \
        PR_Free(m_pbMerge);                                             \
        m_pbMerge = NULL;                                               \
    }                                                                   \
    if(m_pszLinks != NULL)                                              \
    {                                                                   \
        PR_smprintf_free(m_pszLinks);                                   \
        m_pszLinks = NULL;                                              \
    }

SKIndex::~SKIndex(void)
{
    __CleanAllPsz();
}

char* SKIndex::m_pszOperators = NULL;

SKERR SKIndex::SetOperators(const char* pszOperators)
{
    if(m_pszOperators)
        PL_strfree(m_pszOperators);
    m_pszOperators = PL_strdup(pszOperators);
    ::SetOperators(m_pszOperators);
    return noErr;
}

SKERR SKIndex::SetFileName(const char *pszFileName,
                           const char *pszDefaultFileName)
{
    SKERR   err;

    // inherited
    err = SKTextFile::SetFileName(pszFileName, pszDefaultFileName);
    if (err != noErr)
        return err;


    err = ParseConfiguration();
    if (err != noErr)
        return err;

    m_bInitialized = PR_FALSE;

    // if separators have not been specified in the configuration file,
    // initialize them with arbitrary default separators
    if (!m_pSoftSepList)
    {
        *m_pSoftSepList.already_AddRefed() = sk_CreateInstance(SKIntegerList)();
        if (m_pSoftSepList)
            err = m_pSoftSepList->SetListFromUTF8String("'");
        else
            err = err_memory;
    }
    if (err!= noErr)
        return err;

    if (!m_pHardSepList)
    {
        *m_pHardSepList.already_AddRefed() = sk_CreateInstance(SKIntegerList)();
        if (m_pHardSepList)
            err = m_pHardSepList->SetListFromUTF8String(
                            "!#()+,./:;<>^_`[]{|}~- ");
        else
            err = err_memory;
    }

    // ditto with the exceptions for the soft separators
    if (!m_pSoftSepExcList)
    {
        *m_pSoftSepExcList.already_AddRefed() =
            sk_CreateInstance(SKStringList)();
        if (m_pSoftSepExcList)
                err = m_pSoftSepExcList->SetListWithOneWord("aujourd'hui");
            else
                err = err_memory;
    }

    if (err == noErr)
        m_bFileName = PR_TRUE;

    return err;
}

SKERR SKIndex::ConfigureItem(char* pszSection, char* pszToken, char* pszValue)
{
    SKERR err = err_not_handled;
    if(!pszToken)
        return noErr;

    if(!PL_strcmp(pszSection, "WORDTABLE"))
    {
        if(!PL_strcmp(pszToken, "DOCLINKFIELD"))
        {
            if(m_pWordRecordSet)
            {
                skPtr<SKIFldCollection> pCol;
                err = m_pWordRecordSet->GetFldCollection(
                                pCol.already_AddRefed());
                if(err == noErr)
                    err = pCol->GetField(pszValue,
                                         m_pDocLinkField.already_AddRefed());
            }
            else
            {
                err = err_config;
            }
        }
        else if(!PL_strcmp(pszToken, "DOCIDFIELD"))
        {
            if(m_pDocLinkField)
            {
                skPtr<SKIRecordSet> pLinkRS;
                err = m_pDocLinkField->GetLinkSubRecordSet(
                                pLinkRS.already_AddRefed());
                if(err == noErr)
                {
                    skPtr<SKIFldCollection> pCol;
                    err = pLinkRS->GetFldCollection(
                                    pCol.already_AddRefed());
                    if(err == noErr)
                        err = pCol->GetField(pszValue,
                                        m_pDocIdField.already_AddRefed());
                }
            }
            else
            {
                err = err_config;
            }
        }
        else if(!PL_strcmp(pszToken, "STRUCTUREFIELD"))
        {
            if(!*pszValue)
                err = noErr;
            else if(m_pDocLinkField)
            {
                skPtr<SKIRecordSet> pLinkRS;
                err = m_pDocLinkField->GetLinkSubRecordSet(
                                pLinkRS.already_AddRefed());
                if(err == noErr)
                {
                    skPtr<SKIFldCollection> pCol;
                    err = pLinkRS->GetFldCollection(
                                    pCol.already_AddRefed());
                    if(err == noErr)
                        err = pCol->GetField(pszValue,
                                        m_pStructureField.already_AddRefed());
                }
            }
            else
            {
                err = err_config;
            }
        }
        else if(!PL_strcmp(pszToken, "OCCLINKFIELD"))
        {
            if(!*pszValue)
                err = noErr;
            else if(m_pDocLinkField)
            {
                skPtr<SKIRecordSet> pLinkRS;
                err = m_pDocLinkField->GetLinkSubRecordSet(
                                pLinkRS.already_AddRefed());
                if(err == noErr)
                {
                    skPtr<SKIFldCollection> pCol;
                    err = pLinkRS->GetFldCollection(
                                    pCol.already_AddRefed());
                    if(err == noErr)
                        err = pCol->GetField(pszValue,
                                        m_pOccLinkField.already_AddRefed());
                }
            }
            else
            {
                err = err_config;
            }
        }
        else if(!PL_strcmp(pszToken, "OCCFIELD"))
        {
            if(!*pszValue)
                err = noErr;
            else if(m_pOccLinkField)
            {
                skPtr<SKIRecordSet> pLinkRS;
                err = m_pOccLinkField->GetLinkSubRecordSet(
                                pLinkRS.already_AddRefed());
                if(err == noErr)
                {
                    skPtr<SKIFldCollection> pCol;
                    err = pLinkRS->GetFldCollection(
                                    pCol.already_AddRefed());
                    if(err == noErr)
                        err = pCol->GetField(pszValue,
                                             m_pOccField.already_AddRefed());
                }
            }
            else
            {
                err = err_config;
            }
        }
    }
    // [WILDCARDWORDLIST]
    else if(!PL_strcmp(pszSection, "WILDCARDWORDLIST"))
    {
        if(!PL_strcmp(pszToken, "PATH"))
        {
            *m_pWordList.already_AddRefed() =
                    sk_CreateInstance(SKWildCardWordList)();
            if(m_pWordList)
            {
                err = m_pWordList->SetFileName(pszValue);
                if(err == noErr)
                    err = m_pWordList->GetWordRecordSet(
                                    m_pWordRecordSet.already_AddRefed());
            }
            else
            {
                err = err_memory;
            }
        }
    }
    // [SEPARATORS]
    else if (!PL_strcmp(pszSection, "SEPARATORS"))
    {
        if(!PL_strcmp(pszToken, "HARD"))
        {
            *m_pHardSepList.already_AddRefed() =
                sk_CreateInstance(SKIntegerList)();
            if (m_pHardSepList)
                err = m_pHardSepList->SetListFromAsciiString(pszValue);
            else
                err = err_memory;
        } else
        if(!PL_strcmp(pszToken, "SOFT"))
        {
            *m_pSoftSepList.already_AddRefed() =
                sk_CreateInstance(SKIntegerList)();
            if (m_pSoftSepList)
                err = m_pSoftSepList->SetListFromAsciiString(pszValue);
            else
                err = err_memory;
        } else
        if (!PL_strcmp(pszToken, "EXCEPTIONS"))
        {
            *m_pSoftSepExcList.already_AddRefed() =
                sk_CreateInstance(SKStringList)();
            if (m_pSoftSepExcList)
                err = m_pSoftSepExcList->SetFileName(pszValue);
            else
                err = err_memory;
        } else err = err_config;
    }
    // [STOPWORDLIST]
    else if (!PL_strcmp(pszSection, "STOPWORDLIST"))
    {
        if(!PL_strcmp(pszToken, "PATH"))
        {
            *m_pStopWordList.already_AddRefed() =
                sk_CreateInstance(SKStringList)();
            if (m_pStopWordList)
                err = m_pStopWordList->SetFileName(pszValue);
            else
                err = err_memory;
        } else  err = err_config;
    }
    // [LINKS]
    else if (!PL_strcmp(pszSection, "LINKS"))
    {
        PRUint32 lLink = 0;
        if (!PL_strcmp(pszToken, "COUNT"))
        {
            PRUint32 i;
            m_lLinkCount = atoi(pszValue);
            __CleanPpsz(m_ppszLinkNames);
            err = noErr;
            m_ppszLinkNames =
                (char **)PR_Malloc(sizeof(char *) * m_lLinkCount);

            if (m_ppszLinkNames == NULL)
                err = SKError(err_idx_malloc, "[SKIndex::SetFileName] "
                        "Can not allocate memory");

            __CleanPpsz(m_ppszSubLinkNames);
            m_ppszSubLinkNames =
                (char **)PR_Malloc(sizeof(char *) * m_lLinkCount);
            if (m_ppszSubLinkNames == NULL)
                err = SKError(err_idx_malloc, "[SKIndex::SetFileName] "
                        "Can not allocate memory");

            m_pbMerge = (PRBool*) PR_Malloc(sizeof(PRBool) * m_lLinkCount);

            if (m_pbMerge == NULL)
                err = SKError(err_idx_malloc, "[SKIndex::SetFileName] "
                        "Can not allocate memory");

            for(i = 0 ; i < m_lLinkCount ; i++)
            {
                m_ppszLinkNames[i] = NULL;
                m_ppszSubLinkNames[i] = NULL;
                m_pbMerge[i] = PR_FALSE;
            }
        }
        else if(!PL_strncmp(pszToken, "FIELD", 5))
        {
            err = noErr;
            if (pszToken[5] != ',')
            {
                err = SKError(err_idx_invalid, "[SKIndex::SetFileName] "
                        "Missing comma after FIELD");
            }
            lLink = atoi(pszToken + 6)-1;
            if (lLink >= m_lLinkCount)
            {
                err = SKError(err_idx_invalid, "[SKIndex::SetFileName] "
                        "Bad link number value for FIELD: %d", lLink);
            }
            __CleanPsz(m_ppszLinkNames[lLink]);
            m_ppszLinkNames[lLink] = PL_strdup(pszValue);
        }
        else if(!PL_strncmp(pszToken, "SUBFIELD", 8))
        {
            err = noErr;
            if (pszToken[8] != ',')
            {
                err = SKError(err_idx_invalid, "[SKIndex::SetFileName] "
                        "Missing comma after SUBFIELD");
            }
            lLink = atoi(pszToken + 9)-1;
            if (lLink >= m_lLinkCount)
            {
                err = SKError(err_idx_invalid, "[SKIndex::SetFileName] "
                        "Bad link number value for SUBFIELD: %d", lLink);
            }
            __CleanPsz(m_ppszSubLinkNames[lLink]);
            m_ppszSubLinkNames[lLink] = PL_strdup(pszValue);
        }
        else if(!PL_strncmp(pszToken, "MERGE", 5))
        {
            err = noErr;
            if (pszToken[5] != ',')
            {
                err = SKError(err_idx_invalid, "[SKIndex::SetFileName] "
                        "Missing comma after MERGE");
            }
            lLink = atoi(pszToken + 6)-1;
            if (lLink >= m_lLinkCount)
            {
                err = SKError(err_idx_invalid, "[SKIndex::SetFileName] "
                        "Bad link number value for MERGE: %d", lLink);
            }
            m_pbMerge[lLink] = MAKE_BOOL(pszValue);
        }
    }

    return err;
}


#undef __CleanPpsz
#undef __CleanAllPsz

SKERR SKIndex::Init()
{
    if(!m_bFileName)
        // SetFileName has not been called
        return err_invalid;

    if(!m_bInitialized)
    {
        skPtr<SKIFldCollection> pCol;
        skPtr<SKIFldCollection> pLinCol;
        skPtr<SKIField> pField, pSubField, pTokenField;
        skPtr<SKIRecordSet> pRs;

        // Links : builds the string : "LINK_1|...|LINK_N"
        PRUint32 lLink;
        for(lLink = 0 ; lLink < m_lLinkCount ; lLink++)
        {
            if(lLink != 0)
            {
                m_pszLinks = PR_sprintf_append(m_pszLinks, "|");
                if (m_pszLinks == NULL)
                    return err_idx_malloc;
            }
            m_pszLinks = PR_sprintf_append(m_pszLinks, "%s,%s,%s",
                                           m_ppszLinkNames[lLink],
                                           m_ppszSubLinkNames[lLink],
                                           m_pbMerge[lLink]?"1":"");

            if (m_pszLinks == NULL)
                return err_idx_malloc;
        }

        SKERR err;

        // m_pDocIdField
        err = AppendFieldComparator(m_pDocIdField);
        if(err != noErr)
            return err;

        // m_pStructureField
        if(m_pStructureField)
        {
            err = AppendFieldComparator(m_pStructureField);
            if(err != noErr)
                return err;
        }

        SKEnvir *pEnv = NULL;
        err = SKEnvir::GetEnvir(&pEnv);

        char* pcSubResultStoredSorted;
        pEnv->GetValue(SKF_FE_INDEX_PRESORTED, &pcSubResultStoredSorted);
        m_bSubResultsStoredSorted =
            pcSubResultStoredSorted && atol(pcSubResultStoredSorted);
        PR_Free(pcSubResultStoredSorted);

        m_bInitialized = PR_TRUE;
    }

    return noErr;
}

SKERR SKIndex::AppendFieldComparator(SKIField *pField)
{
    skPtr<SKRecordComparatorUNumField> pFieldComparator;
    char *pszFieldName = NULL;

    *pFieldComparator.already_AddRefed() =
            sk_CreateInstance(SKRecordComparatorUNumField)();
    if(!pFieldComparator)
        return err_memory;

    SKERR err;

    err = pField->GetName(&pszFieldName);
    if(err != noErr)
        return err;

    err = pFieldComparator->SetField(pszFieldName);
    PL_strfree(pszFieldName);
    if(err != noErr)
        return err;

    return m_docComparator.AddComparator(pFieldComparator);
}

SKERR SKIndex::SearchExpression(const char *pszSearchString,
                                PRBool bUseFlex /*= PR_FALSE*/,
                                skIStringSimplifier *pSimp /*= NULL */,
                                SKIRecordFilter *pFilter /*= NULL */,
                                SKIndexResult **ppResult)
{
    SK_ASSERT(NULL != pszSearchString);
    SK_ASSERT(NULL != ppResult);

    // check input variables
    if (!pszSearchString || !ppResult)
        return SKError(err_idx_invalid, "[SKIndex::SearchExpression] "
                       "Invalid arguments");
    *ppResult = NULL;

    if(PL_strlen(pszSearchString) == 0)
        return SKError(err_idx_invalid, "[SKIndex::SearchExpression] "
                       "Empty request");

    // Instanciate the result object (all our garbage will automatically
    // be freed if an error occurs)
    skPtr<SKIndexResult> pResult;
    *pResult.already_AddRefed() = sk_CreateInstance(SKIndexResult)();
    if(!pResult)
        return SKError(err_memory, "[SKIndex::SearchExpression] "
                       "Unable to allocate the result");

    SKERR err = Init();
    if(err != noErr)
        return err;

    err = pResult->DoSearch(this, pszSearchString, bUseFlex, pSimp,
                            pFilter);
    if(err != noErr)
        return err;

    return pResult.CopyTo(ppResult);
}

SKERR SKIndex::ResolveLin(const char *pszToken,
                          SKIRecordSet **ppResult,
                          PRBool bUseFlex,
                          skIStringSimplifier* pSimp,
                          SKIRecordFilter *pFilter)
{
    *ppResult = NULL;

    const char *pszLinks = bUseFlex ? m_pszLinks : "";

    skPtr<SKCursor> pCursor;
    SKERR err = m_pWordList->GetWildCardWords(pSimp, pszLinks,
                                              pszToken, NULL,
                                              pCursor.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 iCount;
    pCursor->GetCount(&iCount);

    skPtr<SKIRecord> pRecord;

    if(iCount == 0)
    {
        // generate an empty recordset
        skPtr<SKIRecordSet> pTmpRS;
        err = m_pWordRecordSet->GetRecord(0, pRecord.already_AddRefed());
        if(err != noErr)
            return err;
        err = pRecord->GetLinkFieldValue(m_pDocLinkField,
                                         pTmpRS.already_AddRefed());
        if(err != noErr)
            return err;

        return pTmpRS->GetSubRecordSet(0, 0, ppResult);
    }
    else
    {
#define _ReleaseResults(iCount) {               \
    PRUint32 i;                                 \
    for (i = 0; i < iCount; i++)                \
    {                                           \
        if (ppResults[i])                       \
            ppResults[i]->Release();            \
    }                                           \
    PR_Free (ppResults);                        \
}

        SKIRecordSet** ppResults = (SKIRecordSet**)
            PR_Malloc(sizeof(SKIRecordSet*) * iCount);
        if (!ppResults)
            return err_memory;

        PRUint32 i;
        for(i = 0; i < iCount; ++i)
        {
            if ((err = pCursor->GetRecord(m_pWordRecordSet, i, PR_TRUE,
                pRecord.already_AddRefed())) != noErr)
            {
                _ReleaseResults(i);
                return err;
            }

            if ((err = pRecord->GetLinkFieldValue(m_pDocLinkField,
                ppResults + i)) != noErr)
            {
                _ReleaseResults(i);
                return err;
            }

            if(!m_bSubResultsStoredSorted)
            {
                err = ppResults[i]->Sort(&m_docComparator, PR_TRUE);
                if(err != noErr)
                {
                    _ReleaseResults(i);
                    return err;
                }
            }
        }

        if((err = MultiMerge(ppResults, iCount) != noErr))
        {
            _ReleaseResults(iCount);
            return err;
        }

        skPtr<SKIRecordSet> pResult = ppResults[0];
        _ReleaseResults(iCount);
#undef _ReleaseResults

        if(pResult && pFilter)
        {
            err = pResult->Filter(pFilter);
            if(err != noErr)
                return err;
        }

        return pResult.CopyTo(ppResult);
    }
}

SKERR SKIndex::MultiMerge(SKIRecordSet** ppResults, PRUint32 iCount)
{
    SK_ASSERT (NULL != ppResults);

    while (iCount > 1)
    {
        PRUint32 iLast = iCount & 0xFFFFFFFE;
        PRUint32 i;
        SKERR err;

        // One loop iteration merges the 2N cursors [0..2N-1]
        // (or 2N+1 cursors [0..2N]) into [0..N-1].

        for (i = 0; i < iLast; i+= 2)
        {
            SK_ASSERT (NULL != ppResults[i]);
            SK_ASSERT (NULL != ppResults[i + 1]);

            // Merge each couple of cursors together
            err = ppResults[i]->Merge(ppResults[i + 1], skfopOR,
                &m_docComparator, PR_TRUE);
            if (err != noErr)
                return err;

            // Move the result to the left
            if (i != 0)
            {
                SK_ASSERT (!ppResults[i / 2]);
                ppResults[i / 2] = ppResults[i];
                ppResults[i] = NULL;
            }

            ppResults[i + 1]->Release();
            ppResults[i + 1] = NULL;
        }

        // Merge the last, unpaired cursor
        if (iLast != iCount)
        {
            SK_ASSERT (iCount == iLast + 1);
            SK_ASSERT (NULL != ppResults[iLast]);
            SK_ASSERT (NULL != ppResults[0]);

            err = ppResults[0]->Merge(ppResults[iLast], skfopOR,
                &m_docComparator, PR_TRUE);
            if (err != noErr)
                return err;

            // For a very large value of iCount, it may be interesting
            // to choose a random cursor, instead of the first one.
        }

        iCount = iLast / 2;
        // Iterate until we have only one cursor.
    }

    return noErr;
}

SKERR SKIndex::BuildOccurrenceList(SKIRecordSet *pRS, PRUint32 *piNext,
                                   PRUint32 iMax, PRUint32 iDocId,
                                   SKCursor **ppOccList)
{
    *ppOccList = NULL;

    skPtr<SKCursor> pResult;
    *pResult.already_AddRefed() = sk_CreateInstance(SKCursor)();
    if(!pResult)
        return err_memory;

    if(m_pOccLinkField)
    {

        SKERR err;
        skPtr<SKIRecord> pRec;
        PRUint32 i, iId;
        for(i = 0; i < iMax; ++i)
        {
            err = pRS->GetRecord(i, pRec.already_AddRefed());
            if(err != noErr)
                return err;
            err = pRec->GetUNumFieldValue(m_pDocIdField, &iId);
            if(err != noErr)
                return err;
            if(iId == iDocId)
            {
                break;
            }
        }

        if(i < iMax)
        {
            SK_ASSERT(iId == iDocId);
            *piNext = i;

            skPtr<SKIRecordSet> pOccRS;
            skPtr<SKCursor> pNewCursor;
            do
            {
                err = pRec->GetLinkFieldValue(m_pOccLinkField,
                                              pOccRS.already_AddRefed());
                if(err != noErr)
                    return err;

                PRUint32 iCount = 0;
                err = pOccRS->GetCount(&iCount);
                if(err != noErr)
                    return err;

                err = pOccRS->ExtractCursor(m_pOccField, 0, iCount,
                                            pNewCursor.already_AddRefed());
                if(err != noErr)
                    return err;

                err = pResult->Merge(pNewCursor, skfopOR);
                if(err != noErr)
                    return err;

                ++i;
                if (i < iMax)
                {
                    err = pRS->GetRecord(i, pRec.already_AddRefed());
                    if(err != noErr)
                        return err;

                    err = pRec->GetUNumFieldValue(m_pDocIdField, &iId);
                    if(err != noErr)
                        return err;
                }
            } while((i < iMax) && (iId == iDocId));
        }
        *piNext = i;
    }

    return pResult.CopyTo(ppOccList);
}

SKERR SKIndex::IsStopWord(char* pszToken, PRBool* pbResult)
{
    if (m_pStopWordList == NULL)
    {
        *pbResult = false;
        return noErr;
    }

    return m_pStopWordList->IsPresent (pszToken, pbResult);
}

SKERR SKIndex::GetWordRecordSet(SKIRecordSet **ppRecordSet)
{
    if (!m_pWordList)
        return SKError(err_invalid, "[SKIndex::GetWordRecordSet] "
                                    "index not initialized");

    return m_pWordList->GetWordRecordSet(ppRecordSet);
}

SKERR SKIndex::GetDocLinkField(SKIField **ppField)
{
    return m_pDocLinkField.CopyTo(ppField);
}

SKERR SKIndex::GetDocIdField(SKIField **ppField)
{
    return m_pDocIdField.CopyTo(ppField);
}

SKERR SKIndex::GetStructureField(SKIField **ppField)
{
    return m_pStructureField.CopyTo(ppField);
}

SKERR SKIndex::GetOccLinkField(SKIField **ppField)
{
    return m_pOccLinkField.CopyTo(ppField);
}

SKERR SKIndex::SetNearThreshold(PRUint32 iNearThreshold)
{
    m_iNearThreshold = iNearThreshold;
    return noErr;
}

SKERR SKIndex::GetNearThreshold(PRUint32 *piNearThreshold)
{
    if(!piNearThreshold)
        return err_failure;
    *piNearThreshold = m_iNearThreshold;
    return noErr;
}

// SKIndexResult
SK_REFCOUNT_IMPL_DEFAULT(SKIndexResult)
SK_REFCOUNT_IMPL_IID(SKIndexResult, SK_SKINDEXRESULT_IID, SKRefCount)

SKIndexResult::SKIndexResult()
{
    m_pszSearchString = NULL;
    m_pTokens = NULL;
}

SKIndexResult::~SKIndexResult()
{
    if(m_pszSearchString)
        PL_strfree(m_pszSearchString);
    if(m_pTokens)
        delete (IndexTokens*)m_pTokens;
}

SKERR SKIndexResult::GetDocumentList(SKCursor **ppDocumentList)
{
    *ppDocumentList = NULL;
    if(!m_pDocumentList)
        return noErr;

    // Don't clone the cursor because the application may filter it and then
    // run a scorer.
#if 0
    SKERR err;
    PRUint32 iCount;
    err = m_pDocumentList->GetCount(&iCount);
    if(err != noErr)
        return err;

    *ppDocumentList = sk_CreateInstance(SKCursor)
        (iCount, m_pDocumentList->GetSharedData());
    if(!*ppDocumentList)
        return err_memory;

    return noErr;
#else
    return m_pDocumentList.CopyTo(ppDocumentList);
#endif
}

SKERR SKIndexResult::RunScorer(SKIIndexScorer *pScorer)
{
    if(!pScorer)
        return err_invalid;

    SKERR err;

    PRUint32 iCount;
    err = m_pDocumentList->GetCount(&iCount);
    if(err != noErr)
        return err;

    err = pScorer->Init(iCount);
    if(err != noErr)
        return err;

    return ((IndexTokens *)m_pTokens)->ComputeScores(m_pDocumentList, pScorer);
}

SKERR SKIndexResult::GetHiliteInfo(PRUint32 iDocId, SKCursor **ppHiliteInfo)
{
    IndexTokens *pTokens = (IndexTokens *)m_pTokens;

    SKERR err = noErr;
    if( pTokens == NULL )
        return err_failure;

    err = pTokens->ComputeHiliteInfo(iDocId, ppHiliteInfo);

    return err;
}

SKERR SKIndexResult::DoSearch(SKIndex *pIndex, const char *pszSearchString,
                              PRBool bUseFlex, skIStringSimplifier* pSimp,
                              SKIRecordFilter* pFilter)
{
    // Initialization
    m_pIndex = pIndex;

    m_pszSearchString  = PL_strdup(pszSearchString);
    if(!m_pszSearchString)
        return err_memory;

    m_pTokens = new IndexTokens();
    if(!m_pTokens)
        return err_memory;

    SKERR err;
    PRUint32 iNearThreshold;
    err = m_pIndex->GetNearThreshold(&iNearThreshold);
    if(err != noErr)
        return err;

    IndexTokens *pTokens = (IndexTokens *)m_pTokens;
    err = pTokens->Init(iNearThreshold, m_pIndex);
    if(err != noErr)
        return err;

    // Parsing
    err = pTokens->Parse(m_pszSearchString, PR_TRUE);
    if(err != noErr)
        return err;

    // LIN resolution
    err = pTokens->ResolveLin(bUseFlex, pSimp, pFilter);
    if(err != noErr)
        return err;

    // Document list build
    err = pTokens->ComputeDocumentList(m_pDocumentList.already_AddRefed());

    return err;
}

SK_REFCOUNT_IMPL_IID(SKIIndexScorer, SK_SKIINDEXSCORER_IID, SKRefCount)

