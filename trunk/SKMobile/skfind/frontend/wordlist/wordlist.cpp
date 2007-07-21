/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: wordlist.cpp,v 1.55.2.4 2005/02/21 14:22:47 krys Exp $
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

#include <skfind/skfind.h>

#include "wordlist.h"

//#define DEBUG_WORDLIST

SK_REFCOUNT_IMPL_DEFAULT(SKWordList)
SK_REFCOUNT_IMPL_IID(SKWordList, SK_SKWORDLIST_IID, SKTextFile)

SKWordList::SKWordList()
{
    m_bInitialized = PR_FALSE;
    m_bInterrupt = PR_FALSE;
    SKEnvir *pEnv = NULL;
    SKEnvir::GetEnvir(&pEnv);

    char* pcSubRSSorted;
    pEnv->GetValue(SKF_FE_WORDLIST_PRESORTED, &pcSubRSSorted);
    m_bSubRSSorted =
            pcSubRSSorted && atol(pcSubRSSorted);
    PR_Free(pcSubRSSorted);
}

SKWordList::~SKWordList()
{
}

SKERR SKWordList::SetFileName(const char *pszFileName,
                              const char *pszDefaultFileName)
{
    SKERR   err;

    // inherited
    err = SKTextFile::SetFileName(pszFileName, pszDefaultFileName);
    if (err != noErr)
        return err;

    // open configuration file
    err = SetSimplifierURL("simplifier:unicode:uni_case,uni_comp");
    if(err != noErr)
        return err;

    m_bInitialized = PR_FALSE;

    err = ParseConfiguration();

    return err;
}

SKERR SKWordList::SetWordRecordSet(SKIRecordSet *pRecordSet)
{
    if(!pRecordSet)
        return err_failure;
    m_pWordRecordSet = pRecordSet;
    return m_pWordRecordSet->GetFldCollection(
                    m_pWordFldCollection.already_AddRefed());
}

SKERR SKWordList::GetWordRecordSet(SKIRecordSet **ppRecordSet)
{
    return m_pWordRecordSet.CopyTo(ppRecordSet);
}

SKERR SKWordList::SetLookupRecordSet(SKIRecordSet *pRecordSet,
                                     SKIField *pLookupField,
                                     SKIField *pLookupSubField,
                                     SKIField *pTextTokenField)
{
    if(!pRecordSet || !pTextTokenField)
        return err_failure;
    m_pLookupRecordSet = pRecordSet;
    m_pLookupField = pLookupField;
    m_pLookupSubField = pLookupSubField;
    m_pTextTokenField = pTextTokenField;
    return noErr;
}

SKERR SKWordList::GetLookupRecordSet(SKIRecordSet **ppRecordSet,
                                     SKIField **ppLookupField,
                                     SKIField **ppLookupSubField,
                                     SKIField **ppTextTokenField)
{
    SKERR err = noErr;
    if(ppRecordSet)
        err |= m_pLookupRecordSet.CopyTo(ppRecordSet);
    if(ppLookupField)
        err |= m_pLookupField.CopyTo(ppLookupField);
    if(ppLookupSubField)
        err |= m_pLookupSubField.CopyTo(ppLookupSubField);
    if(ppTextTokenField)
        err |= m_pTextTokenField.CopyTo(ppTextTokenField);
    return err;
}

SKERR SKWordList::SetRevLookupRecordSet(SKIRecordSet *pRecordSet,
                                        SKIField *pRevLookupField)
{
    if(!pRecordSet || !pRevLookupField)
        return err_failure;
    m_pRevLookupRecordSet = pRecordSet;
    m_pRevLookupField = pRevLookupField;
    return noErr;
}

SKERR SKWordList::GetRevLookupRecordSet(SKIRecordSet **ppRecordSet,
                                        SKIField **ppRevLookupField)
{
    SKERR err = noErr;
    if(ppRecordSet)
        err |= m_pRevLookupRecordSet.CopyTo(ppRecordSet);
    if(ppRevLookupField)
        err |= m_pRevLookupField.CopyTo(ppRevLookupField);
    return err;
}

SKERR SKWordList::SetCapLookupRecordSet(SKIRecordSet *pRecordSet,
                                        SKIField *pCapLookupField,
                                        SKIField *pCapLookupSubField)
{
    if(!pRecordSet || !pCapLookupField || !pCapLookupSubField)
        return err_failure;
    m_pCapLookupRecordSet = pRecordSet;
    m_pCapLookupField = pCapLookupField;
    m_pCapLookupSubField = pCapLookupSubField;
    return noErr;
}

SKERR SKWordList::GetCapLookupRecordSet(SKIRecordSet **ppRecordSet,
                                        SKIField **ppCapLookupField,
                                        SKIField **ppCapLookupSubField)
{
    SKERR err = noErr;
    if(ppRecordSet)
        err |= m_pCapLookupRecordSet.CopyTo(ppRecordSet);
    if(ppCapLookupField)
        err |= m_pCapLookupField.CopyTo(ppCapLookupField);
    if(ppCapLookupSubField)
        err |= m_pCapLookupSubField.CopyTo(ppCapLookupSubField);
    return err;
}

SKERR SKWordList::SetRevCapLookupRecordSet(SKIRecordSet *pRecordSet,
                                           SKIField *pRevCapLookupField)
{
    if(!pRecordSet || !pRevCapLookupField)
        return err_failure;
    m_pRevCapLookupRecordSet = pRecordSet;
    m_pRevCapLookupField = pRevCapLookupField;
    return noErr;
}

SKERR SKWordList::GetRevCapLookupRecordSet(SKIRecordSet **ppRecordSet,
                                           SKIField **ppRevCapLookupField)
{
    SKERR err = noErr;
    if(ppRecordSet)
        err |= m_pRevCapLookupRecordSet.CopyTo(ppRecordSet);
    if(ppRevCapLookupField)
        err |= m_pRevCapLookupField.CopyTo(ppRevCapLookupField);
    return err;
}

SKERR SKWordList::ConfigureItem(    char* pszSection, char* pszToken, 
                                    char* pszValue)
{
    SKERR err;

    if((!pszToken) || (!pszSection))
        return noErr;
    
    if (!PL_strcmp(pszSection, "WORDTABLE"))
    {
        if (!PL_strcmp(pszToken, "RSURL"))
        {
            skPtr<SKIRecordSet> pRS;
            SKFactoryGetSubRecordSet(pszValue, pRS, err);
            if(err != noErr)
                return err;
            return SetWordRecordSet(pRS);
        }
    }
    else if (!PL_strcmp(pszSection, "LOOKUPTABLE"))
    {
        if (!PL_strcmp(pszToken, "RSURL"))
        {
            SKFactoryGetSubRecordSet(pszValue, m_pLookupRecordSet, err);
            if((err != noErr) || m_pWordRecordSet)
                return err;
            return SetWordRecordSet(m_pLookupRecordSet);
        }
        else if (!PL_strcmp(pszToken, "FIELD"))
        {
            if(!m_pLookupRecordSet)
                return err_invalid;
            skPtr<SKIFldCollection> pCol;
            err = m_pLookupRecordSet->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pLookupField.already_AddRefed());
            return err;
        }
        else if (!PL_strcmp(pszToken, "SUBFIELD"))
        {
            if(!m_pLookupField)
                return err_invalid;
            skPtr<SKIRecordSet> pRS;
            err = m_pLookupField->GetLinkSubRecordSet(pRS.already_AddRefed());
            if(err != noErr)
                return noErr;
            skPtr<SKIFldCollection> pCol;
            err = pRS->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pLookupSubField.already_AddRefed());
            return err;
        }
        else if (!PL_strcmp(pszToken, "TEXTTOKENFIELD"))
        {
            if(!m_pLookupRecordSet)
                return err_invalid;
            skPtr<SKIFldCollection> pCol;
            err = m_pLookupRecordSet->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pTextTokenField.already_AddRefed());
            return err;
        }
    }
    else if (!PL_strcmp(pszSection, "REVLOOKUPTABLE"))
    {
        if (!PL_strcmp(pszToken, "RSURL"))
        {
            SKERR err;
            SKFactoryGetSubRecordSet(pszValue, m_pRevLookupRecordSet, err);
            return err;
        }
        else if (!PL_strcmp(pszToken, "FIELD"))
        {
            if(!m_pRevLookupRecordSet)
                return err_invalid;
            skPtr<SKIFldCollection> pCol;
            err = m_pRevLookupRecordSet->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pRevLookupField.already_AddRefed());
            return err;
        }
    }
    else if (!PL_strcmp(pszSection, "CAPLOOKUPTABLE"))
    {
        if (!PL_strcmp(pszToken, "RSURL"))
        {
            SKERR err;
            SKFactoryGetSubRecordSet(pszValue, m_pCapLookupRecordSet, err);
            return err;
        }
        else if (!PL_strcmp(pszToken, "SIMPURL"))
            return SetSimplifierURL(pszValue);
        else if (!PL_strcmp(pszToken, "FIELD"))
        {
            if(!m_pCapLookupRecordSet)
                return err_invalid;
            skPtr<SKIFldCollection> pCol;
            err = m_pCapLookupRecordSet->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pCapLookupField.already_AddRefed());
            return err;
        }
        else if (!PL_strcmp(pszToken, "SUBFIELD"))
        {
            if(!m_pCapLookupField)
                return err_invalid;
            skPtr<SKIRecordSet> pRS;
            err = m_pCapLookupField->
                    GetLinkSubRecordSet(pRS.already_AddRefed());
            if(err != noErr) 
                return err;
            skPtr<SKIFldCollection> pCol;
            err = pRS->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pCapLookupSubField.already_AddRefed());
            return err;
        }
    }
    else if (!PL_strcmp(pszSection, "REVCAPLOOKUPTABLE"))
    {
        if (!PL_strcmp(pszToken, "RSURL"))
        {
            SKERR err;
            SKFactoryGetSubRecordSet(pszValue, m_pRevCapLookupRecordSet, err);
            return err;
        }
        else if (!PL_strcmp(pszToken, "FIELD"))
        {
            if(!m_pRevCapLookupRecordSet)
                return err_invalid;
            skPtr<SKIFldCollection> pCol;
            err = m_pRevCapLookupRecordSet
                ->GetFldCollection(pCol.already_AddRefed());
            if(err != noErr) 
                return err;
            err = pCol->GetField(pszValue, 
                                 m_pRevCapLookupField.already_AddRefed());
            return err;
        }
    }
    return err_not_handled;
}

SKERR SKWordList::GetWords(eSearchMode mode, PRBool bSensitive,
                           SKIRecordFilter* pFilter, const char* pszLinks,
                           const char* pszToken, SKCursor** ppCursor)
{
    if((mode != MODE_EXACT) && (mode != MODE_BEGIN) && (mode != MODE_END))
        return SKError(err_invalid, "[SKWordList::GetWords] Invalid arguments");


    if(!m_pWordRecordSet)
         return SKError(err_failure, "[SKWordList::GetWords] "
                                "Word table is missing");

#ifdef DEBUG_WORDLIST
    printf("WordList::GetWords(mode=%u, %s, %p, %s, %s)\n",
            mode, bSensitive ? "sensitive " : "not sensitive", pFilter,
            pszLinks, pszToken);
#endif
    
    // Reset the interrupt process
    m_bInterrupt = PR_FALSE;

    // if the token is empty and the mode isn't MODE_EXACT then doing a
    // sensitive query in MODE_BEGIN produces the same result without
    // the huge artillery of skResolveRSLink.
    if((!*pszToken) && (mode != MODE_EXACT))
    {
        bSensitive = PR_TRUE;
        mode = MODE_BEGIN;
    }

    skPtr<SKIRecordSet> pLookupRecordSet;
    skPtr<SKIField> pDirectField;
    PRUint32 iId1, iId2;
    *ppCursor = NULL;

    // Check we have the lookup table we're going to use
    if((mode == MODE_EXACT) || (mode == MODE_BEGIN))
    {
        if(bSensitive)
        {
            if(!m_pLookupRecordSet)
                return SKError(err_failure, "[SKWordList::GetWords] "
                               "Lookup table is missing");
#ifdef DEBUG_WORDLIST
            printf("Selecting exact direct table\n");
#endif
            pLookupRecordSet = m_pLookupRecordSet;
        }
        else
        {
            if(!m_pCapLookupRecordSet)
                return SKError(err_failure, "[SKWordList::GetWords] "
                               "CapLookup table is missing");
            pLookupRecordSet = m_pCapLookupRecordSet;
#ifdef DEBUG_WORDLIST
            printf("Selecting simplified direct table\n");
#endif
        }
    }
    else
    {
        if(bSensitive)
        {
            if(!m_pRevLookupRecordSet)
                return SKError(err_failure, "[SKWordList::GetWords] "
                               "RevLookup table is missing");
            pLookupRecordSet = m_pRevLookupRecordSet;
            pDirectField = m_pRevLookupField;
#ifdef DEBUG_WORDLIST
            printf("Selecting exact reverse table\n");
#endif
        }
        else
        {
            if(!m_pRevCapLookupRecordSet)
                return SKError(err_failure, "[SKWordList::GetWords] "
                               "RevCapLookup table is missing");
            pLookupRecordSet = m_pRevCapLookupRecordSet;
            pDirectField = m_pRevCapLookupField;
#ifdef DEBUG_WORDLIST
            printf("Selecting simplified reverse table\n");
#endif
        }
    }

    SKERR err = noErr;

    skPtr<SKCursor> pResult;
    // First step: token lookup
    if(mode == MODE_EXACT)
    {
        err = pLookupRecordSet->LookupText(pszToken, skflmEXACT, &iId1);
        if((err != noErr) && (err != err_notfound))
            return err;

        if(err == noErr)
            iId2 = iId1 + 1;
        else
            iId2 = iId1 = 0;
    }
    else if(!*pszToken)
    {
        iId1 = 0;
        err = pLookupRecordSet->GetCount(&iId2);
        if(err != noErr)
            return err;
    }
    else
    {
        char* pszToken2 = PL_strdup(pszToken);
        if(mode == MODE_END)
            sk_invertString(pszToken2);

#ifdef DEBUG_WORDLIST
        printf("Looking up last item before %s\n", pszToken2);
#endif
        err = pLookupRecordSet->LookupText(pszToken2, skflmLASTBEFORE, &iId1);
        if(err == err_notfound)
        {
            iId1 = 0;
            err = noErr;
        }
        else if(err != noErr)
        {
            PL_strfree(pszToken2);
            return err;
        }
        else
        {
            ++iId1;
        }

        sk_nextString(pszToken2);
#ifdef DEBUG_WORDLIST
        printf("Looking up last item before %s\n", pszToken2);
#endif
        err = pLookupRecordSet->LookupText(pszToken2, skflmLASTBEFORE, &iId2);
        if(err == err_notfound)
        {
            iId2 = 0;
            err = noErr;
        }
        else if(err != noErr)
        {
            PL_strfree(pszToken2);
            return err;
        }
        else
        {
            ++iId2;
        }
#ifdef DEBUG_WORDLIST
        printf("Interval : %u - %u\n", iId1, iId2);
#endif
        PL_strfree(pszToken2);

        SK_ASSERT(iId1 <= iId2);
    }

    *pResult.already_AddRefed() = sk_CreateInstance(SKCursor)();
    if(!pResult)
        return err_memory;

    err = pResult->InitStartCount(iId1, iId2 - iId1);
    if(err != noErr)
        return err;

    // reversed -> not reversed
    if(pDirectField)
    {
#ifdef DEBUG_WORDLIST
        printf("Get direct ids\n");
#endif
        skPtr<SKCursor> pNewResult;
        err = skResolveRSUNum(pResult, pLookupRecordSet, pDirectField,
                              PR_FALSE, &m_bInterrupt,
                              pNewResult.already_AddRefed());
        if(err != noErr)
            return err;
        pResult = pNewResult;
    }

    // Check the interrupt flag
    if(m_bInterrupt)
    {
        m_bInterrupt = PR_FALSE;
        return err_interrupted;
    }

    // simplified -> not simplified
    if(!bSensitive)
    {
        skPtr<SKCursor> pNewResult;
        err = skResolveRSLink(pResult, m_pCapLookupRecordSet,
                              m_pCapLookupField, m_pCapLookupSubField,
                              PR_FALSE, &m_bInterrupt,
                              pNewResult.already_AddRefed());
        if(err != noErr)
            return err;

        pResult = pNewResult;
    }

    // Check the interrupt flag
    if(m_bInterrupt)
    {
        m_bInterrupt = PR_FALSE;
        return err_interrupted;
    }

    // Filtering
    if(pFilter)
    {
        SKCursorFilterRecordWrapper cursorFilter;
        err = cursorFilter.SetCursor(pResult);
        if(err != noErr)
            return err;

        err = pFilter->SetRecordSet(m_pLookupRecordSet);
        if(err != noErr)
            return err;

        err = cursorFilter.SetRecordFilter(pFilter, PR_TRUE);
        if(err != noErr)
            return err;

        if(pResult != NULL)
        {
            err = pResult->Filter(&cursorFilter);
            if(err != noErr)
                return err;
        }
    }

    // Check the interrupt flag
    if(m_bInterrupt)
    {
        m_bInterrupt = PR_FALSE;
        return err_interrupted;
    }

    // token -> word
    if(m_pLookupField)
    {
        skPtr<SKCursor> pNewResult;

        if(m_pLookupSubField)
            err = skResolveRSLink(pResult, m_pLookupRecordSet,
                                  m_pLookupField, m_pLookupSubField,
                                  PR_FALSE, &m_bInterrupt,
                                  pNewResult.already_AddRefed());
        else
            err = skResolveRSUNum(pResult, m_pLookupRecordSet,
                                  m_pLookupField, PR_FALSE,
                                  &m_bInterrupt, pNewResult.already_AddRefed());

        if(err != noErr)
            return err;

        pResult = pNewResult;
    }

    // Check the interrupt flag
    if(m_bInterrupt)
    {
        m_bInterrupt = PR_FALSE;
        return err_interrupted;
    }

    // Second step: links resolution
    skPtr<SKCursor> pNewResult;
    err = ResolveLinks(pResult, pszLinks, pNewResult.already_AddRefed());
    if(err != noErr)
        return err;

    m_bInterrupt = PR_FALSE;

    return pNewResult.CopyTo(ppCursor);
}

SKERR SKWordList::ResolveLinks(SKCursor* pFrom, const char* pszLinks,
                               SKCursor** ppTo)
{
    pFrom->AddRef();
    *ppTo = pFrom;

    char* pszLinksCopy = PL_strdup(pszLinks);
    if(pszLinks && !pszLinksCopy)
    {
        (*ppTo)->Release();
        *ppTo = NULL;
        return SKError(err_failure, "[SKWordList::ResolveLinks] "
                       "Out of memory");
    }

    char* pszPointer = pszLinksCopy;

    while(pszPointer && (*pszPointer != '\0'))
    {
        char* pszNext = PL_strchr(pszPointer, '|');
        if(pszNext)
            *pszNext = '\0';

        char* pszCut1 = PL_strchr(pszPointer, ',');
        if(!pszCut1)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            PL_strfree(pszLinksCopy);
            return SKError(err_invalid, "[SKWordList::ResolveLinks] "
                           "Invalid link field '%s'", pszLinks);
        }
        *pszCut1 = '\0';

        char* pszCut2 = PL_strchr(pszCut1 + 1, ',');
        if(!pszCut2)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            PL_strfree(pszLinksCopy);
            return SKError(err_invalid, "[SKWordList::ResolveLinks] "
                           "Invalid link field '%s'", pszLinks);
        }
        *pszCut2 = '\0';

        skPtr<SKCursor> pTmpCursor;
        SKERR err;
        if(*(pszCut1 + 1))
        {
            err = ResolveLink(*ppTo, pszPointer, pszCut1 + 1,
                              pTmpCursor.already_AddRefed());
        }
        else
        {
            err = ResolveUNum(*ppTo, pszPointer,
                              pTmpCursor.already_AddRefed());
        }
        if(err != noErr)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            PL_strfree(pszLinksCopy);
            return err;
        }

        // Check the interrupt flag
        if(m_bInterrupt)
        {
            (*ppTo)->Release();
            *ppTo = NULL;
            PL_strfree(pszLinksCopy);
            m_bInterrupt = PR_FALSE;
            return err_interrupted;
        }

        if(pszCut2[1] != '\0')
        {
            err = pTmpCursor->Merge(*ppTo, skfopOR);
            if(err != noErr)
            {
                (*ppTo)->Release();
                *ppTo = NULL;
                PL_strfree(pszLinksCopy);
                return err;
            }
        }

        (*ppTo)->Release();
        pTmpCursor.CopyTo(ppTo);

        pszPointer = pszNext;
        if(pszPointer)
            pszPointer++;
    }

    PL_strfree(pszLinksCopy);

    return noErr;
}

SKERR SKWordList::ResolveLink(SKCursor *pFrom, const char *pszField,
                              const char *pszSubField, SKCursor **ppTo)
{
    SKERR err;

    skPtr<SKIField> pField;
    err = m_pWordFldCollection->GetField(pszField,
                                         pField.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIRecordSet> pSubRS;
    err = pField->GetLinkSubRecordSet(pSubRS.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIFldCollection> pSubCol;
    err = pSubRS->GetFldCollection(pSubCol.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIField> pSubField;
    err = pSubCol->GetField(pszSubField, pSubField.already_AddRefed());
    if(err != noErr)
        return err;

    return skResolveRSLink(pFrom, m_pWordRecordSet, pField, pSubField,
                           m_bSubRSSorted, &m_bInterrupt, ppTo);
}

SKERR SKWordList::ResolveUNum(SKCursor *pFrom, const char *pszField,
                              SKCursor **ppTo)
{
    skPtr<SKIField> pField;
    SKERR err = m_pWordFldCollection->GetField(pszField,
                                               pField.already_AddRefed());
    if(err != noErr)
        return err;

    return skResolveRSUNum(pFrom, m_pWordRecordSet, pField, PR_FALSE,
                           &m_bInterrupt, ppTo);
}

SKERR SKWordList::SetSimplifierURL(const char* pcUrl)
{
    SKERR err;
    SKFactory* pFactory;
    err = SKFactory::GetFactory(&pFactory);
    SK_ASSERT(err == noErr);

    err = pFactory->CreateInstance(pcUrl,
            (SKRefCount**) m_pTableSimplifier.already_AddRefed());
    return err;
}

void SKWordList::Interrupt()
{
    m_bInterrupt = PR_TRUE;
}

