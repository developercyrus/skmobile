/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: wildcardwordlist.cpp,v 1.36.2.4 2005/02/21 14:22:47 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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
#include "wildcardwordlist.h"

//#define DEBUG_WILDCARD
//#define DEBUG_WILDCARD_FILTER

SK_REFCOUNT_IMPL_DEFAULT(SKWildCardWordList)
SK_REFCOUNT_IMPL_IID(SKWildCardWordList, SK_SKWILDCARDWORDLIST_IID, SKWordList)

class WildCardFilter : public SKIRecordFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(WildCardFilter)

    WildCardFilter() : m_pcSingleWildCard(NULL), m_pcMultipleWildCard(NULL),
                       m_pSimp(NULL),
                       m_pcPattern(NULL)
    {
    };

    ~WildCardFilter()
    {
        if(m_pcPattern)
            PL_strfree(m_pcPattern);
    }

    virtual SKERR CheckRecord(SKIRecord* pRecord, PRBool* pbKeepIt)
    {
        *pbKeepIt = PR_FALSE;

        skPtr<SKBinary> pBinary;
        SKERR err = pRecord->GetDataFieldValue(m_pTokenField,
                                               pBinary.already_AddRefed());
        if(err != noErr)
            return err;

#ifdef DEBUG_WILDCARD
        printf("filtering %s\n", (char*) pBinary->GetSharedData());
#endif
        *pbKeepIt = Match(m_pcPattern, (char*) pBinary->GetSharedData());
#ifdef DEBUG_WILDCARD
        printf("Match returns : %d \n", *pbKeepIt);
#endif

        if(!*pbKeepIt || !m_pFilter)
            return noErr;

        return m_pFilter->CheckRecord(pRecord, pbKeepIt);
    };
    void SetField(SKIField * field)
    { 
        m_pTokenField = field; 
    }
    void SetPattern(char* p)
    {
        m_pcPattern = PL_strdup(p);
    }
    void SetSingleWildCard(char* p)
    {
        m_pcSingleWildCard =  p;
    }
    void SetMultipleWildCard(char *p)
    {
        m_pcMultipleWildCard =  p;
    }
    void SetSimplifier(skIStringSimplifier* pSimplifier) 
    {
        m_pSimp = pSimplifier;
    }
    void SetFilter(SKIRecordFilter * filter) { m_pFilter = filter;  }
private:
    PRBool Match(char* plPattern, char* plStream);
    skPtr<SKIField>     m_pTokenField;
    char* m_pcSingleWildCard;
    char* m_pcMultipleWildCard;
    skPtr<skIStringSimplifier> m_pSimp;
    char* m_pcPattern;
    skPtr<SKIRecordFilter> m_pFilter;

    char * m_pcBuffer;
    PRUint32 m_iBufferLength;
};

SK_REFCOUNT_IMPL_DEFAULT(WildCardFilter)

SKWildCardWordList::SKWildCardWordList() :
    m_pcSingleWildCard(NULL), m_pcMultipleWildCard(NULL)
{
}

SKWildCardWordList::~SKWildCardWordList()
{
    if(m_pcSingleWildCard)
        PL_strfree(m_pcSingleWildCard);
    if(m_pcMultipleWildCard)
        PL_strfree(m_pcMultipleWildCard);
}

SKERR SKWildCardWordList::ConfigureItem(char* pszSection,
                                        char* pszToken, char* pszValue)
{
    if(!pszToken)
        return noErr;

    if (!PL_strcmp(pszToken, "SINGLEWILDCARD"))
    {
        PRUint32 pcCar[2];
        if (sscanf(pszValue, "0x%x", pcCar) == 1)
        {
            pcCar[1] = 0;
            UCS4ToNewUTF8(&m_pcSingleWildCard, pcCar);
            return noErr;
        }
        else
        {
            return err_invalid;
        }
    }
    else if (!PL_strcmp(pszToken, "MULTIPLEWILDCARD"))
    {
        PRUint32 pcCar[2];
        if (sscanf(pszValue, "0x%x", pcCar) == 1)
        {
            pcCar[1] = 0;
            UCS4ToNewUTF8(&m_pcMultipleWildCard, pcCar);
            return noErr;
        }
        else
        {
            return err_invalid;
        }
    }

    return SKWordList::ConfigureItem(pszSection, pszToken, pszValue);
}

SKERR SKWildCardWordList::GetWildCardWords( skIStringSimplifier* pSimp,
                                            const char* pszLinks,
                                            const char* pcPattern,
                                            SKIRecordFilter* pFilter,
                                            SKCursor** ppCursor)
{
#ifdef DEBUG_WILDCARD
    fprintf(stderr, "GetWildCardWords: input pattern=%s\n",
            pcPattern);
#endif
    SK_ASSERT(m_pcSingleWildCard && m_pcMultipleWildCard);

    const char* pcWC = pcPattern;

    // try to find a wild card in the pattern
    while(     PL_strncmp(pcWC, m_pcSingleWildCard, 
                                PL_strlen(m_pcSingleWildCard))
            && PL_strncmp(pcWC, m_pcMultipleWildCard,
                                PL_strlen(m_pcMultipleWildCard))
            && *pcWC) pcWC++;

    char* pcToSearch = NULL;
    // The mode we are going to use
    SKWordList::eSearchMode lMode = MODE_INVALID;
    if(!*pcWC)
    {
        // no wild card -> we perform a search on the whole word
        lMode = SKWordList::MODE_EXACT;
        pcToSearch = PL_strdup(pcPattern);
    }
    else if(pcWC > pcPattern)
    {
        // we have a fixed beginning
        lMode = SKWordList::MODE_BEGIN;
        pcToSearch = PL_strndup(pcPattern, (PRUint32) (pcWC - pcPattern));
    }
    else
    {
        if (PL_strlen(pcPattern) == 1)
            return SKError(err_invalid, "[SKWildCardWordList::GetWildCardWords]"
                                 " Invalid request contains only a wildcard");

        // so we look for a fixed ending
        char* pcRevSingleWildCard = PL_strdup(m_pcSingleWildCard);
        char* pcRevMultipleWildCard = PL_strdup(m_pcMultipleWildCard);
        char* pcRevPattern = PL_strdup(pcPattern);
        UTF8strinvert(pcRevSingleWildCard);
        UTF8strinvert(pcRevMultipleWildCard);
        UTF8strinvert(pcRevPattern);

        pcWC = pcRevPattern;
        // try to find a wildcard again
        while(     PL_strncmp(pcWC, pcRevSingleWildCard,
                                    PL_strlen(m_pcSingleWildCard))
                && PL_strncmp(pcWC, pcRevMultipleWildCard,
                                    PL_strlen(m_pcMultipleWildCard))
                && *pcWC) pcWC++;

        SK_ASSERT(*pcWC);  // plWC must point on the beginning of the reversed
                        // wildcard

        if(pcWC>pcRevPattern) // there is a fixed ending
        {
            lMode = SKWordList::MODE_END;
            pcToSearch = PL_strndup(pcRevPattern, pcWC - pcRevPattern);
            UTF8strinvert(pcToSearch);
        }
        else
        {
            lMode = SKWordList::MODE_BEGIN;
            pcToSearch = PL_strdup("");
        }


        PL_strfree(pcRevSingleWildCard);
        PL_strfree(pcRevMultipleWildCard);
        PL_strfree(pcRevPattern);
    }

    // Do we need an additional filter ?
    WildCardFilter filter;
    if(pSimp || lMode != SKWordList::MODE_EXACT)
    {
#ifdef DEBUG_WILDCARD_FILTER
        fprintf(stderr, "WildCardWordList defines a filter\n");
#endif
        // prepare the filter
        filter.SetField(m_pTextTokenField);
        filter.SetSimplifier(pSimp);
        filter.SetPattern((char*)pcPattern);
        filter.SetSingleWildCard(m_pcSingleWildCard);
        filter.SetMultipleWildCard(m_pcMultipleWildCard);
        filter.SetFilter(pFilter);
        pFilter = &filter;
    }

    // now prepare the string to be searched in the simplified table
    if(pSimp)
    {
        SKERR err = m_pTableSimplifier->SimplifyRealloc(&pcToSearch);
        if(err != noErr)
        {
            if(pcToSearch)
                PL_strfree(pcToSearch);
            return err;
        }
#ifdef DEBUG_WILDCARD
        fprintf(stderr, "wordlist simplifier: %s\noperation simplifier: %s\n",
                m_pTableSimplifier->GetSharedIdentity(),
                pSimp->GetSharedIdentity());
    }
    else
    {
        fprintf(stderr, "No simplifier !\n");
#endif
    }
    

#ifdef DEBUG_WILDCARD
    fprintf(stderr, "Trigger is : `%s'\n", pcToSearch);
#endif

    // performs the search
    SKERR err = GetWords(lMode, pSimp ? 0 : 1,
                        pFilter, pszLinks, pcToSearch, ppCursor);

    PL_strfree(pcToSearch);

    return err;
}

SKERR SKWildCardWordList::SetSingleWildCard(const char * pszWC)
{
    if(m_pcSingleWildCard)
        PL_strfree(m_pcSingleWildCard);
    m_pcSingleWildCard = PL_strdup(pszWC);
    return noErr;
}

SKERR SKWildCardWordList::SetMultipleWildCard(const char * pszWC)
{
    if(m_pcMultipleWildCard)
        PL_strfree(m_pcMultipleWildCard);
    m_pcMultipleWildCard = PL_strdup(pszWC);
    return noErr;
}

PRBool WildCardFilter::Match(char* pattern, char* stream)
{
#ifdef DEBUG_WILDCARD_FILTER
    fprintf(stderr, "  Match(pattern='%s', stream='%s')\n",pattern, stream);
#endif

    PRBool bStop = PR_FALSE;
    PRUint32 lLen1=0;
    PRUint32 lLen2=0;
    
    if(!m_pSimp)
    {
        SKERR err;
        SKFactory * pFactory;
        err = SKFactory::GetFactory(&pFactory);
        SK_ASSERT(err == noErr);

        err = pFactory->CreateInstance("simplifier:unicode:",
            (SKRefCount**) m_pSimp.already_AddRefed());

        if(err != noErr)
            return PR_FALSE;

    }

    PRInt32 iCmp;
    while(!bStop)
    {
#ifdef DEBUG_WILDCARD_FILTER
       fprintf(stderr, "  pattern=%s stream=%s\n", pattern, stream);
#endif
        if(*stream && !PL_strncmp(pattern, m_pcSingleWildCard,
                    PL_strlen(m_pcSingleWildCard)))
        {
#ifdef DEBUG_WILDCARD_FILTER
            fprintf(stderr, "  single\n");
#endif
            pattern += PL_strlen(m_pcSingleWildCard);
            m_pSimp->SimplifyFirstChar(stream, &lLen2, NULL, NULL);
            stream += lLen2;
        }
       else if(!PL_strncmp(pattern, m_pcMultipleWildCard,
                   PL_strlen(m_pcMultipleWildCard)))
        {
#ifdef DEBUG_WILDCARD_FILTER
            fprintf(stderr, "  multiple\n");
#endif
            pattern += PL_strlen(m_pcMultipleWildCard);
            while(*stream)
            {
                if(Match(pattern, stream))
                    return PR_TRUE;
                stream++;
            }
        }
        else if(*pattern && *stream &&
          (noErr == m_pSimp->CompareFirstChar(pattern, &lLen1, 
                                              stream, &lLen2, &iCmp))
           && (lLen1 != 0 || lLen2 != 0) && !iCmp )
        {
#ifdef DEBUG_WILDCARD_FILTER
            fprintf(stderr, "  match\n");
#endif
            pattern += lLen1;
            stream += lLen2;
        }
        else
        {
            bStop = PR_TRUE;
        }
    }

#ifdef DEBUG_WILDCARD_FILTER
    fprintf(stderr, "  exiting while\n");
#endif
    // if we are at the end of both buffer, it's fine.
    if(!*pattern && !*stream)
        return PR_TRUE;

    return PR_FALSE;
}
