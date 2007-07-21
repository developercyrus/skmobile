/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: tokens.cpp,v 1.22.2.5 2005/02/21 14:22:46 krys Exp $
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
#include "filters.h"

// IndexTokens
IndexTokens::IndexTokens()
{
    m_ppLinRecordSets = NULL;
    m_iTokenCount = 0;
    m_iTokenSize = 0;
    m_pTokens = NULL;
    m_piLastHiliteDocId = NULL;
    m_piLastHiliteNext = NULL;
    m_ppLinRecordSets = NULL;
}

IndexTokens::~IndexTokens()
{
    for(PRUint32 i = 0; i < m_iTokenCount; i++)
    {
        if(m_pTokens[i].m_pPrivateData)
            delete (IndexTokens*)m_pTokens[i].m_pPrivateData;
    }
    if(m_pTokens)
        PR_Free(m_pTokens);
    if(m_piLastHiliteDocId)
        PR_Free(m_piLastHiliteDocId);
    if(m_piLastHiliteNext)
        PR_Free(m_piLastHiliteNext);
    if(m_ppLinRecordSets)
    {
        for(PRUint32 i=0; i<m_iTokenCount; i++)
            if(m_ppLinRecordSets[i] != NULL)
                m_ppLinRecordSets[i]->Release();
        PR_Free(m_ppLinRecordSets);
    }
}

SKERR IndexTokens::GrowArrays()
{
    PRUint32 iTmpTokenSize = m_iTokenSize + 10;
    SearchToken* pTmpTokens = (SearchToken*) 
        PR_Realloc(m_pTokens, sizeof(SearchToken)*iTmpTokenSize);
    PRUint32* piTmpLastHiliteDocId = (PRUint32*)
        PR_Realloc(m_piLastHiliteDocId, sizeof(PRUint32)*iTmpTokenSize);
    PRUint32* piTmpLastHiliteNext = (PRUint32*)
        PR_Realloc(m_piLastHiliteNext, sizeof(PRUint32)*iTmpTokenSize);
    SKIRecordSet** ppTmpLinRecordSets = (SKIRecordSet**)
        PR_Realloc(m_ppLinRecordSets, sizeof(SKIRecordSet*)*iTmpTokenSize);
    if(pTmpTokens && piTmpLastHiliteDocId 
            && piTmpLastHiliteNext && ppTmpLinRecordSets)
    {
        m_pTokens = pTmpTokens;
        m_piLastHiliteDocId = piTmpLastHiliteDocId;
        m_piLastHiliteNext = piTmpLastHiliteNext;
        m_ppLinRecordSets = ppTmpLinRecordSets;
        memset(m_pTokens + m_iTokenSize, 0, 
                (iTmpTokenSize - m_iTokenSize) * sizeof(SearchToken));
        for(PRUint32 i = m_iTokenSize; i < iTmpTokenSize; i++)
        {
            m_piLastHiliteDocId[i] = (PRUint32)-1;
            m_piLastHiliteNext[i] = (PRUint32)-1;
            m_ppLinRecordSets[i] = NULL;
        }
        m_iTokenSize = iTmpTokenSize;
        return noErr;
    }
    /* error : we must free what we manage to allocate */
    if(pTmpTokens)
        PR_Free(pTmpTokens);
    if(piTmpLastHiliteDocId)
        PR_Free(piTmpLastHiliteDocId);
    if(piTmpLastHiliteNext)
        PR_Free(piTmpLastHiliteNext);
    if(ppTmpLinRecordSets)
        PR_Free(ppTmpLinRecordSets);
    return err_memory;
}

SKERR IndexTokens::Init(PRUint32 iNearThreshold, SKIndex *pIndex)
{
    m_iNearThreshold = iNearThreshold;
    m_pIndex = pIndex;

    return GrowArrays();
}

SKERR IndexTokens::Parse(char *pszSearchString, PRBool bProx)
{
    SKERR err = m_pIndex->Parse_Request(pszSearchString, bProx, this);
    if(err != noErr)
        return err;

    PRUint32 i;

    if(bProx)
    {
        for(i = 0; i < m_iTokenCount; ++i)
        {
            if(m_pTokens[i].m_bPhrase)
            {
                IndexTokens *pSubTokens = new IndexTokens();
                PRUint32 j;

                if(!pSubTokens)
                    return err_memory;
                err = pSubTokens->Init(m_iNearThreshold, m_pIndex);
                if(err != noErr)
                    return err;
                m_pTokens[i].m_pPrivateData = pSubTokens;
                err = pSubTokens->Parse(m_pTokens[i].m_pszToken, PR_FALSE);
                if(err != noErr)
                    return err;
                for (j = 0; j< pSubTokens->m_iTokenCount &&
                            pSubTokens->m_pTokens[j].m_pszToken== NULL; j++) ;
                if (j== pSubTokens->m_iTokenCount)
                {
                    delete pSubTokens;
                    m_pTokens[i].m_pPrivateData = NULL;
                    m_pTokens[i].m_pszToken = NULL;
                }
            }
        }
    }
    
    PRBool bStopWord;
    for (i = 0; i < m_iTokenCount; ++i)
    {
        if (m_pTokens[i].m_bPhrase== PR_FALSE)
        {
            err= m_pIndex->IsStopWord(m_pTokens[i].m_pszToken, &bStopWord);
            if (err!= noErr)
                return err;
            if (bStopWord== PR_TRUE)
            {
                m_pTokens[i].m_pszToken = NULL;

                // Propagate the current operator on the next token.
                // This is essential because if the first word is a stopword,
                // the result document list will not be initialised, 
                // because the operator of the first token is OR.
                // This also implies that operators on stopwords are ignored.
                if (i < m_iTokenCount - 1)
                    m_pTokens[i+ 1].m_oper = m_pTokens[i].m_oper;
            }
        }
    }

    return noErr;
}

SKERR IndexTokens::ResolveLin(PRBool bUseFlex, skIStringSimplifier* pSimp,
                              SKIRecordFilter *pFilter)
{
    SKERR err;

    for(PRUint32 i = 0; i < m_iTokenCount; ++i)
    {
        if(!m_pTokens[i].m_pszToken && !m_pTokens[i].m_pPrivateData)
            continue; // stopwords do not have occurencies

        if(m_pTokens[i].m_bPhrase)
        {
            // After a shot discussion we decided to make this query
            // case sensitive. -- bozo
            err = ((IndexTokens *)m_pTokens[i].m_pPrivateData)->
                ResolveLin(PR_FALSE, NULL, pFilter);
        }
        else
        {
            err = m_pIndex->ResolveLin(m_pTokens[i].m_pszToken,
                                       m_ppLinRecordSets + i,
                                       bUseFlex, pSimp, pFilter);
        }
        if(err != noErr)
            return err;
    }

    return noErr;
}

SKERR IndexTokens::ComputeDocumentList(SKCursor **ppDocumentList)
{
    SKERR err;

    skPtr<SKCursor> pResult;
    *pResult.already_AddRefed() = sk_CreateInstance(SKCursor)();
    if(!pResult)
        return err_memory;

    for(PRUint32 i = 0; i < m_iTokenCount; ++i)
    {
        skfOperator oper = m_pTokens[i].m_oper;
        if(oper == skfopNEAR || oper == skfopNEARBEFORE)
            oper = skfopAND;

        skPtr<SKCursor> pTokenCursor;
        if(m_pTokens[i].m_bPhrase)
        {
            if (!m_pTokens[i].m_pszToken && !m_pTokens[i].m_pPrivateData)
                continue; // skip phrase containing only stopwords
            err = ((IndexTokens *)m_pTokens[i].m_pPrivateData)->
                ComputePhraseDocumentList(pTokenCursor.already_AddRefed());
        }
        else
        {
            if (!m_pTokens[i].m_pszToken)
                continue; // skip stopwords
            err = ComputeTokenDocumentList(i, pTokenCursor.already_AddRefed());
        }
        if(err != noErr)
            return err;

        err = pResult->Merge(pTokenCursor, oper);
        if(err != noErr)
            return err;

        if(m_pIndex->HasOccurencies())
        {
            if(    (m_pTokens[i].m_oper == skfopNEAR)
                || (m_pTokens[i].m_oper == skfopNEARBEFORE))
            {
                err = FilterNear(pResult, i, m_iNearThreshold, 
                        m_pTokens[i].m_oper == skfopNEARBEFORE);
                if(err != noErr)
                    return err;
            }
        }
    }

    return pResult.CopyTo(ppDocumentList);
}

SKERR IndexTokens::ComputePhraseDocumentList(SKCursor **ppDocumentList)
{
    SKERR err;
    skPtr<SKCursor> pResult;

    PRUint32 iCurrentCount = (PRUint32)-1;
    PRUint32 iSmallest = (PRUint32)-1;

    for(PRUint32 i = 0; i < m_iTokenCount; ++i)
    {
        if (m_ppLinRecordSets[i] == NULL) // stopword
            continue;

        PRUint32 iCount;
        err = m_ppLinRecordSets[i]->GetCount(&iCount);
        if(err != noErr)
            return err;
        if((iSmallest == (PRUint32)-1) || (iCount < iCurrentCount))
        {
            iCurrentCount = iCount;
            iSmallest = i;
        }
    }

    err = ComputeTokenDocumentList(iSmallest, pResult.already_AddRefed());
    if(err != noErr)
        return err;

    IndexDocPhraseFilter filter;
    err = filter.Init(this);
    if(err != noErr)
        return err;

    err = filter.SetCursor(pResult);
    if(err != noErr)
        return err;

    err = pResult->Filter(&filter);
    if(err != noErr)
        return err;

    return pResult.CopyTo(ppDocumentList);
}

SKERR IndexTokens::ComputeTokenDocumentList(PRUint32 iToken,
                                            SKCursor **ppDocumentList)
{
    SKERR err;
    skPtr<SKIRecordSet> pRecordSet = m_ppLinRecordSets[iToken];

    skPtr<SKIField> pDocIdField;
    err = m_pIndex->GetDocIdField(pDocIdField.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 iCount = 0;
    err = pRecordSet->GetCount(&iCount);
    if(err != noErr)
        return err;

    skPtr<SKCursor> pResult;
    err = pRecordSet->ExtractCursor(pDocIdField, 0, iCount,
                                    pResult.already_AddRefed());
    if(err != noErr)
        return err;

    SKCursorFilterRemoveDuplicated filter;

    err = filter.SetCursor(pResult);
    if(err != noErr)
        return err;

    err = pResult->Filter(&filter);
    if(err != noErr)
        return err;

    return pResult.CopyTo(ppDocumentList);
}

SKERR IndexTokens::FilterNear(SKCursor *pReducedDocumentList,
                              PRUint32 iPosition, PRUint32 iNearThreshold,
                              PRBool bAssertOrder)
{
    if(iPosition < 1)
        return err_failure;

    SKERR err;
    IndexNearDocFilter filter;

    SK_ASSERT(iPosition > 0);

    skPtr<SKIRecordSet> pRS1, pRS2;
    pRS1 = m_ppLinRecordSets[iPosition - 1];
    if(!pRS1)
    {
        SearchToken *pTok = m_pTokens + iPosition - 1;
        IndexTokens *pToken = (IndexTokens *)pTok->m_pPrivateData;
        SK_ASSERT(pTok->m_bPhrase);
        SK_ASSERT(pToken->m_iTokenCount > 0);
        pRS1 = pToken->m_ppLinRecordSets[pToken->m_iTokenCount - 1];
    }
    pRS2 = m_ppLinRecordSets[iPosition];
    if(!pRS2)
    {
        SearchToken *pTok = m_pTokens + iPosition;
        IndexTokens *pToken = (IndexTokens *)pTok->m_pPrivateData;
        SK_ASSERT(pTok->m_bPhrase);
        SK_ASSERT(pToken->m_iTokenCount > 0);
        pRS2 = pToken->m_ppLinRecordSets[0];
    }
    
    err = filter.Init(m_pIndex,
                      iNearThreshold,
                      pRS1,
                      pRS2,
                      bAssertOrder);
    if(err != noErr)
        return err;

    err = filter.SetCursor(pReducedDocumentList);
    if(err != noErr)
        return err;

    err = pReducedDocumentList->Filter(&filter);

    return err;
}

SKERR IndexTokens::ComputeHiliteInfo(PRUint32 iDocId, SKCursor **ppHiliteInfo)
{
    SKERR err;

    skPtr<SKCursor> pResult;
    *pResult.already_AddRefed() = sk_CreateInstance(SKCursor)();
    if(!pResult)
        return err_memory;

    for(PRUint32 i = 0; i < m_iTokenCount; ++i)
    {
        skPtr<SKCursor> pTokenCursor;
        if(m_pTokens[i].m_bPhrase)
        {
            err = ((IndexTokens *)m_pTokens[i].m_pPrivateData)->
                ComputePhraseHiliteInfo(iDocId,
                                        pTokenCursor.already_AddRefed());
        }
        else
        {
            err = ComputeTokenHiliteInfo(i, iDocId,
                                         pTokenCursor.already_AddRefed());
        }
        if(err != noErr)
            return err;

        switch(m_pTokens[i].m_oper)
        {
        case skfopOR:
        case skfopAND:
            err = pResult->Merge(pTokenCursor, skfopOR);
            break;
        case skfopEXCEPT:
            err = pResult->Merge(pTokenCursor, skfopEXCEPT);
            break;
        case skfopNEAR:
        case skfopNEARBEFORE:
            {
                skPtr<SKCursor> pNewResult;
                err = FilterNearOccurrences(pResult, pTokenCursor,
                                            m_iNearThreshold,
                                            pNewResult.already_AddRefed());
                if(err == noErr)
                    pResult = pNewResult;
            }
            break;
        default:
            SK_ASSERT(false);
            err = err_failure;
            break;
        }
        if(err != noErr)
            return err;
    }

    return pResult.CopyTo(ppHiliteInfo);
}

#define _CheckError(err)                \
    if(err != noErr)                    \
    {                                   \
        delete[] ppOccurrences;         \
        return err;                     \
    }

SKERR IndexTokens::ComputePhraseHiliteInfo(PRUint32 iDocId,
                                           SKCursor **ppHiliteInfo)
{
    if(!m_iTokenCount)
        return err_failure;

    skPtr<SKCursor> *ppOccurrences = new skPtr<SKCursor>[m_iTokenCount];
    if(!ppOccurrences)
        return err_memory;

    SKERR err;
    PRInt32 i;
    PRInt32 iIndexFirst= 0;
    PRBool bFindFirst;
    //err = ComputeTokenHiliteInfo(m_ppLinRecordSets[0], iDocId,
    //                             ppOccurrences[0].already_AddRefed());
    //_CheckError(err);
    
    PRUint32 iDistance = 1;
    for(i = 0, bFindFirst = PR_FALSE; i < (PRInt32) m_iTokenCount; ++i)
    {
        if (m_pTokens[i].m_pszToken== NULL)
        {
            iDistance++;
            continue; // no highlight info is available for stopwords
            // it could be possible to create a cursor with additional
            // occ corresponding to the word N-1 if N-1 is a stopword
        }

        err = ComputeTokenHiliteInfo(i, iDocId,
                                     ppOccurrences[i].already_AddRefed());
        _CheckError(err);
        
        if (bFindFirst == PR_FALSE)
        {
            bFindFirst = PR_TRUE;
            iDistance = 1;
            iIndexFirst = i;
            continue; // nothing to filter with yet
        }
        
        IndexNearOccFilter filter;
        err = filter.Init(ppOccurrences[i - iDistance], iDistance, PR_TRUE);
        _CheckError(err);
        err = filter.SetCursor(ppOccurrences[i]);
        _CheckError(err);
        err = ppOccurrences[i]->Filter(&filter);
        _CheckError(err);
        
        iDistance = 1;
    }

    for(i = m_iTokenCount - 1, bFindFirst = PR_FALSE; i >= 0; --i)
    {
        if (m_pTokens[i].m_pszToken== NULL)
        {
            iDistance++;
            continue; // no highlight info is available for stopwords
        }

        if (bFindFirst == PR_FALSE)
        {
            bFindFirst = PR_TRUE;
            iDistance = 1;
            continue; // nothing to filter with yet
        }

        IndexNearOccFilter filter;
        err = filter.Init(ppOccurrences[i + iDistance], iDistance, PR_FALSE);
        _CheckError(err);
        err = filter.SetCursor(ppOccurrences[i]);
        _CheckError(err);
        err = ppOccurrences[i]->Filter(&filter);
        _CheckError(err);
        
        iDistance = 1;
    }

    for(i = iIndexFirst; i < (PRInt32) m_iTokenCount; ++i)
        if (m_pTokens[i].m_pszToken!= NULL)
        {
            err = ppOccurrences[iIndexFirst]->Merge(ppOccurrences[i], skfopOR);
            _CheckError(err);
        }

    err = ppOccurrences[iIndexFirst].CopyTo(ppHiliteInfo);

    delete[] ppOccurrences;

    return err;
}

#undef _CheckError

SKERR IndexTokens::ComputeTokenHiliteInfo(PRUint32 iToken,
                                          PRUint32 iDocId,
                                          SKCursor **ppHiliteInfo)
{
    skPtr<SKIRecordSet> pRecordSet = m_ppLinRecordSets[iToken];

    PRUint32 iCount;
    SKERR err = pRecordSet->GetCount(&iCount);
    if(err != noErr)
        return err;

    if(m_piLastHiliteDocId[iToken] < iDocId)
    {
        err = m_pIndex->BuildOccurrenceList(pRecordSet,
                                            m_piLastHiliteNext + iToken,
                                            iCount, iDocId, ppHiliteInfo);
        if(err != noErr)
        {
            m_piLastHiliteDocId[iToken] = (PRUint32)-1;
            m_piLastHiliteNext[iToken] = 0;
            return err;
        }

        m_piLastHiliteDocId[iToken] = iDocId;

        return noErr;
    }
    else
    {
        m_piLastHiliteNext[iToken] = 0;
        err = m_pIndex->BuildOccurrenceList(pRecordSet,
                                            m_piLastHiliteNext + iToken,
                                            iCount, iDocId, ppHiliteInfo);
        if(err != noErr)
        {
            m_piLastHiliteDocId[iToken] = (PRUint32)-1;
            m_piLastHiliteNext[iToken] = 0;
            return err;
        }

        m_piLastHiliteDocId[iToken] = iDocId;

        return noErr;
    }
}

SKERR IndexTokens::FilterNearOccurrences(SKCursor *pReducedHiliteInfo,
                                         SKCursor *pNextHiliteInfo,
                                         PRUint32 iNearThreshold,
                                         SKCursor **ppResult)
{
    PRUint32 i1, i2, iCount1, iCount2;
    const PRUint32 *pData1, *pData2;
    SKERR err;
    err = pReducedHiliteInfo->GetCount(&iCount1);
    if(err != noErr)
        return err;
    err = pNextHiliteInfo->GetCount(&iCount2);
    if(err != noErr)
        return err;

    err = pReducedHiliteInfo->ComputeCursorForm();
    if (err != noErr)
        return err;
    pData1 = pReducedHiliteInfo->GetSharedCursorDataRead();
    err = pNextHiliteInfo->ComputeCursorForm();
    if (err != noErr)
        return err;
    pData2 = pNextHiliteInfo->GetSharedCursorDataRead();
    if((iCount1 && !pData1) || (iCount2 && !pData2))
        return err_failure;

    PRUint32 *pResultData = new PRUint32[iCount1 + iCount2];
    if(!pResultData)
        return err_memory;
    PRUint32 iResultCount = 0;

#define INV ((PRUint32) -1)
    PRUint32 iLast1 = INV;
    PRUint32 iLast2 = INV;

    for(i1 = i2 = 0; i1 < iCount1 || i2 < iCount2 ; )
    {
        PRUint32 p1 = i1 < iCount1 ? pData1[i1] : INV;
        PRUint32 p2 = i2 < iCount2 ? pData2[i2] : INV;

        if(p1 > p2)
        {
            if( (p1 - p2 <= iNearThreshold) 
                    || ( (iLast1 != INV) && (p2 - iLast1) <= iNearThreshold) )
            {
                pResultData[iResultCount++] = p2;
                iLast2 = p2;
            }
            i2++;
        }
        else
        {
            if( (p2 - p1 <= iNearThreshold)
                    || ( (iLast2 != INV) && (p1 - iLast2) <= iNearThreshold) )
            {
                pResultData[iResultCount++] = p1;
                iLast1 = p1;
            }
            i1++;
        }
    }
#undef INV

    *ppResult = sk_CreateInstance(SKCursor)(iResultCount, pResultData);
    if(!*ppResult)
        return err_memory;
    else
        return noErr;
}

SKERR IndexTokens::ComputeScores(SKCursor *pDocumentList,
                                 SKIIndexScorer *pScorer)
{
    for(PRUint32 i = 0; i < m_iTokenCount; ++i)
    {
        SKERR err;
        if(m_pTokens[i].m_bPhrase)
        {
            err = ((IndexTokens *)m_pTokens[i].m_pPrivateData)->
                ComputePhraseScores(pDocumentList, pScorer);
        }
        else
        {
            err = ComputeTokenScores(i, pDocumentList, pScorer);
        }
        if(err != noErr)
            return err;
    }

    return noErr;
}

SKERR IndexTokens::ComputePhraseScores(SKCursor *pDocumentList,
                                       SKIIndexScorer *pScorer)
{
    for(PRUint32 i = 0; i < m_iTokenCount; ++i)
    {
        SKERR err = ComputeTokenScores(i, pDocumentList, pScorer);
        if(err != noErr)
            return err;
    }

    return noErr;
}

SKERR IndexTokens::ComputeTokenScores(PRUint32 iToken, SKCursor *pDocumentList,
                                      SKIIndexScorer *pScorer)
{
    skPtr<SKIRecordSet> pRecordSet = m_ppLinRecordSets[iToken];

    SKERR err;
    PRUint32 iRSNext = 0;
    PRUint32 iRSCount;
    err = pRecordSet->GetCount(&iRSCount);
    if(err != noErr)
        return err;

    skPtr<SKIField> pDocIdField;
    err = m_pIndex->GetDocIdField(pDocIdField.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIField> pStructureField;
    err = m_pIndex->GetStructureField(pStructureField.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIField> pOccLinkField;
    err = m_pIndex->GetOccLinkField(pOccLinkField.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 iCount;
    err = pDocumentList->GetCount(&iCount);
    if(err != noErr)
        return err;

    skPtr<SKIRecord> pRec;
    skPtr<SKIRecordSet> pRS;

    pDocumentList->ComputeCursorForm();
    if (err != noErr)
        return err;
    for(PRUint32 i = 0; i < iCount; ++i)
    {
        PRUint32 iDocId = pDocumentList->GetSharedCursorDataRead()[i];

        while(iRSNext < iRSCount)
        {
            err = pRecordSet->GetRecord(iRSNext, pRec.already_AddRefed());
            if(err != noErr)
                return err;

            PRUint32 iId;
            err = pRec->GetUNumFieldValue(pDocIdField, &iId);
            if(err != noErr)
                return err;

            if(iId > iDocId)
                break;

            iRSNext++;

            if(iId == iDocId)
            {
                PRUint32 iStructure = 0;
                if(pStructureField)
                {
                    err = pRec->GetUNumFieldValue(pStructureField, &iStructure);
                    if(err != noErr)
                        return err;
                }

                err = pRec->GetLinkFieldValue(pOccLinkField,
                                              pRS.already_AddRefed());
                if(err != noErr)
                    return err;

                PRUint32 iOccCount = 0;
                err = pRS->GetCount(&iOccCount);
                if(err != noErr)
                    return err;

                err = pScorer->AddScore(i, iDocId, iStructure, iOccCount);
                if(err != noErr)
                    return err;
            }
        }
    }

    return noErr;
}
