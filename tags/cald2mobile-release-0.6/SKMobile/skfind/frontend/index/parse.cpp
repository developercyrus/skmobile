/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: parse.cpp,v 1.31.2.8 2005/02/21 14:22:46 krys Exp $
 *
 * Authors: W.P. Dauchy
 *          Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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
#include <skfind/frontend/wordlist/wordlist.h>
#include <skfind/frontend/wordlist/wildcardwordlist.h>

#include "parse.h"
#include "index.h"
#include "tokens.h"

//----------------------------------------------------------------------
// special characters & operators
static char     szQuotes[] = "\"«»";
static char     szOper[64] = "or(and(not(near(before";
static char*    pszOper[NB_OPERATORS] = {0};

//----------------------------------------------------------------------
// macros
#define IsQuote(c)      ((c) && PL_strchr(szQuotes, (c)))

//----------------------------------------------------------------------
// operators
//----------------------------------------------------------------------
static skfOperator FindOperator(char* szWord)
{
    short i=0;

    // try to match an operator
    for(i=0; i<NB_OPERATORS; i++)
    {
        if(pszOper[i] && !PL_strcasecmp(pszOper[i], szWord))
        {
            return (skfOperator)(++i);
        }
    }

    return skfopNONE;
}

static skfOperator FindOperatorBegin(char* szWord)
{
    short i=0;

    // try to match an operator
    for(i=0; i<NB_OPERATORS; i++)
    {
        if(pszOper[i] 
                && !PL_strncasecmp(pszOper[i], szWord, PL_strlen(pszOper[i])))
        {
            return (skfOperator)(++i);
        }
    }

    return skfopNONE;
}


//----------------------------------------------------------------------
// SKIndex::IsSpace
//----------------------------------------------------------------------
PRBool SKIndex::IsSpace(char* pszString, char* pCurrent, char **ppszNext)
// Returns PR_TRUE if *pCurrent is a separator.
// Since *p can be a soft separator, we need to scan before and after
// the currently pointed character in order to check the word against
// m_pSoftSepExcList.
{
    PRBool bResult;

    PRUint32 iChar4;
    PRUint32 iLength;
    
    CharUTF8ToUCS4(&iChar4, pCurrent, &iLength);
    if(iLength == 0)
    {
        *ppszNext = pCurrent + 1;
        return PR_TRUE;
    }

    if (m_pHardSepList->IsPresent(iChar4, &bResult)!= noErr)
    {
        *ppszNext = pCurrent + 1;
        return PR_TRUE; // should an error occur (unlikely),
                        // return PR_TRUE
    }
    if (bResult == PR_TRUE)
    {
        *ppszNext = pCurrent + iLength;
        return PR_TRUE;
    }
    
    if (m_pSoftSepList->IsPresent(iChar4, &bResult)!= noErr)
    {
        *ppszNext = pCurrent + 1;
        return PR_TRUE;
    }
    if (bResult == PR_FALSE)
    {
        *ppszNext = pCurrent + iLength;
        return PR_FALSE;
    }

    // find the beginning and the end of the word
    char* pszBegin = pCurrent;
    while (pszBegin > pszString && *(pszBegin - 1) != 0)
    {
        PRUint32 iLength2 ;
        CharUTF8ToUCS4(&iChar4, pszBegin - 1, &iLength2);
        if(iLength2 > 0)
        {
            if (m_pHardSepList->IsPresent(iChar4, &bResult)!= noErr)
            {
                *ppszNext = pCurrent + 1;
                return PR_TRUE;
            }
            if (bResult == PR_TRUE)
                break;
        }
        pszBegin--;
    }
    
    char* pszEnd = pCurrent + iLength;
    while (*pszEnd != 0)
    {
        if(IsQuote(*pszEnd))
            break;
        PRUint32 iLength2;
        CharUTF8ToUCS4(&iChar4, pszEnd, &iLength2);
        if(iLength2 > 0)
        {
            if (m_pHardSepList->IsPresent(iChar4, &bResult) != noErr)
            {
                *ppszNext = pCurrent + 1;
                return PR_TRUE;
            }
            if (bResult == PR_TRUE)
                break;
        }
        pszEnd++;
    }

    char c = *pszEnd; // save the character
    *pszEnd = 0; // temporarily truncate the string
    if (m_pSoftSepExcList->IsPresent (pszBegin, &bResult)!= noErr)
    {
        *ppszNext = pCurrent + 1;
        bResult = PR_TRUE;
    }
    else 
    {
        *ppszNext = pCurrent + iLength;
        bResult = !bResult; // exception word => not a space
    }

    *pszEnd = c;
    
    // side note: what happens when there are several soft separators
    // in a word ?
    // example with the French word "aujourd'hui" as an exception
    // "d'aujourd'hui" will be split into "d" and "aujourd'hui"
    // "aujourd'hui'd" will not be split (unless "aujourd'hui'd" is
    // also an exception)
    
    // Generally speaking, all prefixing non-exceptions are removed.
    // If the soft separator pastes two exceptions together, they will
    // not be split.
    
    return bResult;
}


//----------------------------------------------------------------------
//wildcards
// - Check that the request is valid
//              . even number of quotes
//              . wildcards allowed at the end of a word only (one wildcard per word max)
//              . no wildcard inside an expression if prox is on
//
// - Split the request into tokens
//              . a token is a word if proximity is off
//              . or an expression between quotes if proximity is on
//              . if pTokens is NULL then only check request
//
//      returns noErr if the request is valid, or an error code
//
//      NOTE : if the string is limited to an operator, assume the caller wants
//      to search for that operator and consider it a token. The caller may
//      find that the operator is _also_ a stopword (usually it is)
//      but this is none of our business.
//
//----------------------------------------------------------------------
SKERR SKIndex::Parse_Request(char* pszString, PRBool bProx,
                             IndexTokens* pIndexTokens)
{
    bool                bQuoteOpen = false, bDone = false;
    char                *s, *t;
    skfOperator         oper;

    // load on first time through
    if (!pszOper[0])
         SetOperators(szOper);

    // bad arguments
    if(!pszString || !pIndexTokens)
        return err_prs_invalid;
    
    const char* pcSingleWC = m_pWordList->GetSharedSingleWildCard();
    PRUint32 iSingleWCLength = PL_strlen(pcSingleWC);
    const char* pcMultipleWC = m_pWordList->GetSharedMultipleWildCard();
    PRUint32 iMultipleWCLength = PL_strlen(pcMultipleWC);

    // trim left
    for(s = pszString; 
        *s && IsSpace(pszString, s, &t)
        && !IsQuote(*s)
        && PL_strncmp(pcMultipleWC, s, iMultipleWCLength)
        && PL_strncmp(pcSingleWC, s, iSingleWCLength);
        s = t)
        ;

    // if the request contains only spaces : invalid request
    if (!*s)
        return err_invalid;

    // first operator is always skfopOR
    pIndexTokens->m_pTokens[0].m_oper = skfopOR;

    // walks the string only once
    // s is the current character
    // t always points to the beginning of the current token
#define CURRENTTOKEN (pIndexTokens->m_pTokens[pIndexTokens->m_iTokenCount])
    for (pIndexTokens->m_iTokenCount=0, t=s; !bDone;)
    {
        char *pszNext;
        // will stop at next iteration
        if (*s)
        {
            // quote
            if (IsQuote(*s) && HasOccurencies())
            {
                // a quote always signals the end of a token
                // even an opening quote if there's no space before
                *s = 0;
                // toggle quote state
                CURRENTTOKEN.m_bPhrase = bQuoteOpen;
                bQuoteOpen = !bQuoteOpen;
                pszNext = s + 1;
            }
            else if (s==t && (oper = FindOperatorBegin(s)) 
                    && IsSpace(pszString, s + PL_strlen(pszOper[oper-1]), 
                            &pszNext))
            {
                s += PL_strlen(pszOper[oper-1]);
                *s = 0;
            }
            // space and other conventional separators
            // including parentheses
            else if (IsSpace(pszString, s, &pszNext)
                && PL_strncmp(pcMultipleWC, s, iMultipleWCLength)
                && PL_strncmp(pcSingleWC, s, iSingleWCLength))
            {
                // kills space unless prox is on and
                // we are between quots
                for(char *psz = s; psz < pszNext; ++psz)
                    *psz = (bProx && bQuoteOpen) ? ' ' : '\0';
            }
        }
        else
        {
            bDone = true;
        }

        // end of token
        if (*s == '\0')
        {
            // filter out runs of spaces or delimiters
            // also, we may not need to parse the request, just verify
            // that it is correct. In this case, pass NULL for pIndexTokens
            if (t < s && pIndexTokens)
            {
                // is the token an operator ?
                oper = FindOperator((char*)t);
                if (oper != skfopNONE)
                {
                    CURRENTTOKEN.m_oper = oper;
                    // the operator _is_ the token until there is a token
                    CURRENTTOKEN.m_pszToken = (char*)t;
                }
                else
                {
                    // too many tokens
                    if (pIndexTokens->m_iTokenCount + 1
                            >= pIndexTokens->m_iTokenSize)
                    {
                        SKERR err = pIndexTokens->GrowArrays();
                        if(err != noErr)
                            return err;
                    }

                    // stores pointer to token
                    CURRENTTOKEN.m_pszToken = (char*)t;

                    pIndexTokens->m_iTokenCount++;
                    // sets default operator for next token
                    CURRENTTOKEN.m_oper = skfopAND;
                }
            }

            // advance token pointer
            t = pszNext;
        }

        s = pszNext;
    }

    // only one operator, no token
    if (pIndexTokens->m_iTokenCount == 0 && CURRENTTOKEN.m_oper != skfopNONE)
        pIndexTokens->m_iTokenCount++;

    // all open quotes must be closed
    // regardless of proximity
    if (bQuoteOpen)
        return SKError(err_prs_syntax,
                       "[Parse_Request] Unmatched quotes were found");
    else
        return noErr;
}

//----------------------------------------------------------------------
// SetOperators
//----------------------------------------------------------------------
SKAPI SKERR SetOperators (char *pszOps)
{
    // clean out the operator list
    short i=0;
    memset (pszOper, 0, sizeof(pszOper));

    if (pszOps)
    {
        // copy to the internal buffer, if meaningful
        if (pszOps!= szOper)
            PL_strncpy (szOper, pszOps, sizeof(szOper));

        // parse the operator string
        for (i=0, pszOps=szOper; *pszOps && i<NB_OPERATORS; i++)
        {
            pszOper[i] = pszOps;
            pszOps = PL_strchr(pszOps, '(');
            if (!pszOps)
                break;
            *pszOps++ = 0;
        }

        // found the right number of tokens
        if (i == NB_OPERATORS - 1)
            return noErr;
    }

    return SKError(err_prs_invalid,
                   "[SetOperators] Expected %d operators, got %d instead",
                   NB_OPERATORS, i+1);
}

//----------------------------------------------------------------------
// GetOperator
//----------------------------------------------------------------------
/*
SKAPI char* GetOperator (Operator op)
{
    if (op >= 0 && op < NB_OPERATORS)
        return pszOper[op];
    return NULL;
}
*/

