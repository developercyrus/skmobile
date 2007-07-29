/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: regexpwordlist.cpp,v 1.24.2.3 2005/02/21 14:22:47 krys Exp $
 *
 * Authors: Alexis Seigneurin <seigneurin @at@ idm .dot. fr>
 *          Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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
#include <skpcre/pcre.h>

#include "wordlist.h"
#include "regexpwordlist.h"

//#define DEBUG_REGEXP
//#define DEBUG_REGEXP_FILTER

SK_REFCOUNT_IMPL_DEFAULT(SKRegExpWordList)
SK_REFCOUNT_IMPL_IID(SKRegExpWordList, SK_SKREGEXPWORDLIST_IID, SKWordList)

class RegExpFilter : public SKIRecordFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(RegExpFilter)

    RegExpFilter()
    : m_pSimp(NULL), m_RegExp(NULL), m_RegExpExtra(NULL),
      m_pcBuffer(NULL), m_iBufferLength(0)
#ifdef DEBUG_REGEXP_FILTER
        , m_pcPattern(NULL)
#endif
    {
    }

    ~RegExpFilter()
    {
        if(m_RegExp)
            pcre_free(m_RegExp);
        if(m_RegExpExtra)
            pcre_free(m_RegExpExtra);
        if(m_pcBuffer)
            PL_strfree(m_pcBuffer);
#ifdef DEBUG_REGEXP_FILTER
        if(m_pcPattern)
            PL_strfree(m_pcPattern);
#endif
    }

    virtual SKERR CheckRecord(SKIRecord* pRecord, PRBool* pbKeepIt)
    {
        *pbKeepIt = PR_FALSE;
        skPtr<SKBinary> pBinary;
        SKERR err = pRecord->GetDataFieldValue(m_pTokenField,
                                               pBinary.already_AddRefed());
        if(err != noErr)
            return err;
#ifdef DEBUG_REGEXP_FILTER
        fprintf(stderr, " filtering %s\n", (char*) pBinary->GetSharedData());
#endif
        *pbKeepIt = Match((char *)pBinary->GetSharedData(),
                          pBinary->GetSize()-1);
#ifdef DEBUG_REGEXP_FILTER
        fprintf(stderr, " Match returns : %d \n\n", *pbKeepIt);
#endif

        if(!*pbKeepIt || !m_pFilter)
            return noErr;

        return m_pFilter->CheckRecord(pRecord, pbKeepIt);
    };
    void SetField(SKIField * field)
    { m_pTokenField = field; }
    SKERR SetPattern(const char* pcPattern, skIStringSimplifier* pSimp)
    {
        SKERR err = noErr;
        m_pSimp = pSimp;
        char * pcSimplifiedPattern = NULL;

        if(m_pSimp)
        {
            // simplify the pattern if needed
            err = pSimp->SimplifyToNew(pcPattern, &pcSimplifiedPattern);
            if(err != noErr)
                return err;
        }

        // compile the regular expression
        const char * pError = NULL;
        PRInt32 iErrorOffset = 0;
        PRInt32 iOptions = PCRE_ANCHORED | PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
        
        m_RegExp = pcre_compile(
                pcSimplifiedPattern ? pcSimplifiedPattern : pcPattern,
                iOptions, &pError, &iErrorOffset, NULL);

        if(pError)
        {
            SKError(err_failure, "[SKRegExpWordList::RegExpFilter] "
                    "Failed compiling regular expression `%s' at offset %d, "
                    "with error `%s'", pcPattern, iErrorOffset, pError);
            return err_failure;
        }

        m_RegExpExtra = pcre_study(m_RegExp, 0, &pError);
        if(pError)
        {
            SKError(err_failure, "[SKRegExpWordList::RegExpFilter] "
                    "Failed studying regular expression `%s', "
                    "with error `%s'", pcPattern, pError);
            return err_failure;
        }

#ifdef DEBUG_REGEXP_FILTER
        m_pcPattern = PL_strdup(pcPattern);
#endif

        // free simplified string if allocated
        if(pcSimplifiedPattern)
            PL_strfree((char*)pcSimplifiedPattern);

        return noErr;
    }
    void SetFilter(SKIRecordFilter * filter) { m_pFilter = filter;  }
private:
    PRBool Match(const char * pcCandidate, PRUint32 iLength);

    skPtr<SKIField>     m_pTokenField;
    skPtr<SKIRecordFilter> m_pFilter;
    skPtr<skIStringSimplifier> m_pSimp;

    pcre * m_RegExp;
    pcre_extra * m_RegExpExtra;

    char * m_pcBuffer;
    PRUint32 m_iBufferLength;

#ifdef DEBUG_REGEXP_FILTER
    char * m_pcPattern;
#endif
};

SK_REFCOUNT_IMPL_DEFAULT(RegExpFilter)

SKRegExpWordList::SKRegExpWordList()
{
}

SKRegExpWordList::~SKRegExpWordList()
{
}

#define ISPATTERN(c)                                                    \
    (   c=='.' || c=='+' || c=='*' || c=='?' || c=='[' || c==']'        \
     || c=='('  || c== ')') 
#define IsZeroOrMoreWildcard(c) (c=='?' || c=='*')

SKERR SKRegExpWordList::GetRegExpWords(skIStringSimplifier* pSimp,
                                       const char* pcLinks,
                                       const char* pcPattern,
                                       SKIRecordFilter* pFilter,
                                       SKCursor** ppCursor)
{
    SKERR err;
#ifdef DEBUG_REGEXP
    printf("GetRegExpWords: pattern=%s\n", pcPattern);
#endif

    // look for a fixed beginning
    const char * pc;
    for(pc = pcPattern ; *pc;)
    {
        if(pc[0] == '\\')              // "\..."
        {
            if(pc[1])                  // "\pc[1]..."
            {
                if(IsZeroOrMoreWildcard(pc[2]))
                    break;
                else
                    pc += 2;
            }
            else                        // "\" (error)
                break;
        }
        else                            // "..."
        {
            if(pc[0])                  // "pc[0]..."
            {
                if(!ISPATTERN(pc[0]))
                {
                    if(IsZeroOrMoreWildcard(pc[1]))
                        break;
                    else
                        pc++;
                }
                else
                    break;
            }
            else
                break;                  // ""
        }
    }
    PRUint32 iFixedBeginning = pc - pcPattern;
    PRUint32 iFixedEnding = 0;

    PRUint32 iPatternLen = PL_strlen(pcPattern);
    if(iFixedBeginning != iPatternLen)
    {
        // look for a fixed ending
        pc = pcPattern + iPatternLen - 1;
        while((pc >= pcPattern) && !ISPATTERN(*pc))
            pc--;
        iFixedEnding = pcPattern + iPatternLen - pc - 1;
    }

    // Configuration
    SKWordList::eSearchMode lMode = MODE_INVALID;
    char * pcToSearch = NULL;
    if(iFixedBeginning == iPatternLen)
    {
        lMode = MODE_EXACT;
        pcToSearch = PL_strdup(pcPattern);
    }
    else if(iFixedBeginning >= iFixedEnding)
    {
        lMode = MODE_BEGIN;
        pcToSearch = PL_strndup(pcPattern, iFixedBeginning);
    }
    else
    {
        lMode = MODE_END;
        pcToSearch = PL_strndup(pcPattern + iPatternLen - iFixedEnding,
                                  iFixedEnding);
    }

    // additional filter
    RegExpFilter filter;

    if(    (    (lMode == MODE_BEGIN)
             && (    (iFixedBeginning + 2 != iPatternLen)
                  || (pcPattern[iFixedBeginning] != '.')
                  || (pcPattern[iFixedBeginning + 1] != '*')))
        || (    (lMode == MODE_END)
             && (    (iFixedEnding + 2 != iPatternLen)
                  || (pcPattern[0] != '.')
                  || (pcPattern[1] != '*')))
        || pSimp)
    {
#ifdef DEBUG_REGEXP
        fprintf(stderr, "Need a regexp filter.\n");
#endif
        // prepare the filter
        filter.SetField(m_pTextTokenField);
        err = filter.SetPattern(pcPattern, pSimp);
        if(err != noErr)
            return err;

        filter.SetFilter(pFilter);
        pFilter = &filter;
    }

    if(pSimp)
    {
        char * pcTmp;
        err = m_pTableSimplifier->SimplifyToNew(pcToSearch, &pcTmp);
        PR_Free(pcToSearch);
        pcToSearch = pcTmp;
    }

    // discards escapement
    char * pc1 = pcToSearch;
    char * pc2 = pc1;
    while(*pc1)
    {
        if(*pc1 == '\\')
            pc1++;
        *pc2++ = *pc1++;
    }
    *pc2 = 0;
    
#ifdef DEBUG_REGEXP
    fprintf(stderr, "TOSEARCH='%s'\n", pcToSearch);
#endif

    // perform the search
    err = GetWords(lMode, pSimp ? PR_FALSE : PR_TRUE,
                        pFilter, pcLinks, pcToSearch, ppCursor);

    PL_strfree(pcToSearch);

    return err;
}

PRBool RegExpFilter::Match(const char * pcCandidate, PRUint32 iLength)
{
    SKERR err;

    // simplify the candidate word if needed
    if(m_pSimp)
    {
        if(m_pcBuffer && iLength < m_iBufferLength)
        {
            PL_strcpy(m_pcBuffer, pcCandidate);
            err = m_pSimp->SimplifyRealloc(&m_pcBuffer, &m_iBufferLength);
        }
        else
        {
            if(m_pcBuffer)
                PL_strfree(m_pcBuffer);
            err = m_pSimp->SimplifyToNew(pcCandidate, 
                    &m_pcBuffer, &m_iBufferLength);
        }
        if(err != noErr)
        {
            SKError(err, "Simplification failed");
            return PR_FALSE;
        }
        pcCandidate = m_pcBuffer;
        iLength = PL_strlen(m_pcBuffer);
    }

#ifdef DEBUG_REGEXP_FILTER
    fprintf(stderr, "Match(pattern='%s', candidate='%s' of length %d)\n",
            m_pcPattern, pcCandidate, iLength);
#endif

    PRInt32 iVector[3];
    PRInt32 iStatus = pcre_exec(m_RegExp, m_RegExpExtra,
            pcCandidate, iLength, 0, 0, iVector, 3);
#ifdef DEBUG_REGEXP_FILTER
    fprintf(stderr, "pcre_exec return=%d iVector[0]=%d iVector[1]=%d\n",
            iStatus, iVector[0], iVector[1]);
#endif

    // check if the pattern matched
    if(iStatus < 0)
    {
        if(iStatus != PCRE_ERROR_NOMATCH)
            SKError(err_failure, "[SKRegExpWordList::RegExpFilter::Match] "
                    "pcre_exec failed with error code %d\n", iStatus);
        return PR_FALSE;
    }
    // check the pattern matched on the entire word
    if(iVector[0] == 0 && iVector[1] == (PRInt32)iLength)
        return PR_TRUE;

    return PR_FALSE;
}

SKERR SKRegExpWordList::ConfigureItem(char* pcSection,
                                        char* pcToken, char* pcValue)
{
    if(!pcToken)
        return noErr;

    if (   !PL_strcmp(pcToken, "SINGLEWILDCARD")
        || !PL_strcmp(pcToken, "MULTIPLEWILDCARD"))
        return noErr;

    return SKWordList::ConfigureItem(pcSection, pcToken, pcValue);
}
