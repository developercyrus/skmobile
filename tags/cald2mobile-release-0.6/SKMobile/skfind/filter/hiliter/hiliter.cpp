/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: hiliter.cpp,v 1.15.4.7 2005/02/21 14:22:44 krys Exp $
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

#include "hiliter.h"

SK_REFCOUNT_IMPL_DEFAULT(SKHiliter)

SKERR SKHiliter::SetValues(const PRUint32* pValues, PRUint32 lValuesCount)
{
    if(m_pValues)
        delete[] m_pValues;

    if((m_lValuesCount = lValuesCount))
    {
        m_pValues = new PRUint32[m_lValuesCount];
        if(m_pValues && pValues)
            memcpy(m_pValues, pValues, m_lValuesCount * sizeof(PRUint32));
        else
            return SKError(err_failure,
                           "[SKHiliter::SetValues] Invalid arguments.");
    }

    return noErr;
}

SKHiliter::~SKHiliter()
{
    if(m_pValues)
        delete[] m_pValues;
    if(m_pszStartFrom)
        PL_strfree(m_pszStartFrom);
    if(m_pszStartTo)
        PL_strfree(m_pszStartTo);
    if(m_pszStopFrom)
        PL_strfree(m_pszStopFrom);
    if(m_pszStopTo)
        PL_strfree(m_pszStopTo);
}

SKERR SKHiliter::SetStartStrings(const char* pszStartFrom,
                                 const char* pszStartTo,
                                 PRInt32 iStartOccPos)
{
    if(m_pszStartFrom)
        PL_strfree(m_pszStartFrom);
    if(m_pszStartTo)
        PL_strfree(m_pszStartTo);

    m_pszStartFrom = PL_strdup(pszStartFrom);
    m_pszStartTo = PL_strdup(pszStartTo);
    m_lStartFromLen = PL_strlen(pszStartFrom);
    m_lStartToLen = PL_strlen(pszStartTo);
    if(iStartOccPos <= (PRInt32)m_lStartToLen)
        m_iStartOccPos = iStartOccPos;
    else
        m_iStartOccPos = -1;

    return (m_pszStartFrom && m_pszStartTo) ? noErr : err_failure;
}

SKERR SKHiliter::SetStopStrings(const char* pszStopFrom,
                                const char* pszStopTo,
                                PRInt32 iStopOccPos)
{
    if(m_pszStopFrom)
        PL_strfree(m_pszStopFrom);
    if(m_pszStopTo)
        PL_strfree(m_pszStopTo);

    m_pszStopFrom = PL_strdup(pszStopFrom);
    m_pszStopTo = PL_strdup(pszStopTo);
    m_lStopFromLen = PL_strlen(pszStopFrom);
    m_lStopToLen = PL_strlen(pszStopTo);
    if(iStopOccPos <= (PRInt32)m_lStopToLen)
        m_iStopOccPos = iStopOccPos;
    else
        m_iStopOccPos = -1;

    return (m_pszStopFrom && m_pszStopTo) ? noErr : err_failure;
}

SKERR SKHiliter::FilterData(const void* pData, PRUint32 lSize)
{
    if(    !m_pszStartFrom || !m_pszStartFrom || !m_pszStopTo || !m_pszStopFrom
        || (((char*)pData)[lSize - 1] != '\0')
        || (PL_strlen((char*)pData) + 1 != lSize))
    {
        return err_failure;
    }

    PRInt32 lDiff = 0;
    if((m_lStartToLen - m_lStartFromLen) > 0)
        lDiff += m_lStartToLen - m_lStartFromLen;
    if((m_lStopToLen - m_lStopFromLen) > 0)
        lDiff += m_lStopToLen - m_lStopFromLen;
    if(m_iStartOccPos >= 0) lDiff += 10;
    if(m_iStopOccPos >= 0) lDiff += 10;

    SKERR err;
    if(lDiff > 0)
        err = Realloc(lSize + lDiff * m_lValuesCount);
    else
        err = Realloc(lSize);
    if(err != noErr)
        return err;

    m_lSize = 0;
    char* pszFrom = (char*)pData;
    char* pszTo = (char*)m_pData;

    char* psz = PL_strstr(pszFrom, m_pszStartFrom);
    PRUint32 i = 0, lPos = 0;
    while(psz)
    {
        // Copy the string before
        PRUint32 len = psz - pszFrom;
        PL_strncpy(pszTo, pszFrom, len);
        pszTo += len;
        pszFrom = psz + m_lStartFromLen;
        m_lSize += len;
        // Copy m_pszStartTo if needed
        if((i < m_lValuesCount) && (lPos == m_pValues[i]))
        {
            if(m_iStartOccPos >= 0)
            {
                SK_ASSERT(m_iStartOccPos <= (PRInt32)m_lStartToLen);
                PL_strncpy(pszTo, m_pszStartTo, m_iStartOccPos);
                pszTo += m_iStartOccPos;
                len = sprintf(pszTo, "%u", lPos);
                pszTo += len;
                PL_strncpy(pszTo, m_pszStartTo + m_iStartOccPos,
                           m_lStartToLen - m_iStartOccPos);
                pszTo += m_lStartToLen - m_iStartOccPos;
                m_lSize += len + m_lStartToLen;
            }
            else
            {
                PL_strncpy(pszTo, m_pszStartTo, m_lStartToLen);
                pszTo += m_lStartToLen;
                m_lSize += m_lStartToLen;
            }
        }
        // Find m_pszStopFrom
        psz = PL_strstr(psz, m_pszStopFrom);
        if(!psz)
            break;

        // Copy the content
        len = psz - pszFrom;
        PL_strncpy(pszTo, pszFrom, len);
        pszTo += len;
        pszFrom = psz + m_lStopFromLen;
        m_lSize += len;

        // Copy m_pszStopTo if needed
        if((i < m_lValuesCount) && (lPos == m_pValues[i]))
        {
            if(m_iStopOccPos >= 0)
            {
                SK_ASSERT(m_iStopOccPos <= (PRInt32)m_lStopToLen);
                PL_strncpy(pszTo, m_pszStopTo, m_iStopOccPos);
                pszTo += m_iStopOccPos;
                len = sprintf(pszTo, "%u", lPos);
                pszTo += len;
                PL_strncpy(pszTo, m_pszStopTo + m_iStopOccPos,
                           m_lStopToLen - m_iStopOccPos);
                pszTo += m_lStopToLen - m_iStopOccPos;
                m_lSize += len + m_lStopToLen;
            }
            else
            {
                PL_strncpy(pszTo, m_pszStopTo, m_lStopToLen);
                pszTo += m_lStopToLen;
                m_lSize += m_lStopToLen;
            }
            ++i;
        }

        // Find m_pszStartFrom
        psz = PL_strstr(psz, m_pszStartFrom);
        ++lPos;
    }

    PRUint32 len = lSize - (pszFrom - (char*)pData);
    SK_ASSERT(pszFrom[len - 1] == '\0');
    PL_strncpy(pszTo, pszFrom, len);
    m_lSize += len;
    return (i == m_lValuesCount) ? noErr : err_failure;
}

SKERR skFilterHilite(SKBinary *pContent, SKCursor *pPositions,
                     const char *pcszStartTag, const char *pcszStopTag,
                     SKBinary **ppTo)
{
    if( pContent == NULL || pPositions == NULL || ppTo == NULL )
        return err_invalid;

    PRUint32 iCount;
    SKERR err = pPositions->GetCount(&iCount);
    if( err != noErr )
        return err;
    
    SKHiliter* pHlt = sk_CreateInstance(SKHiliter)();
    if( pHlt == NULL )
        return err_invalid;

    // Get the strings right
    char* pcStartTag = PR_smprintf("<a name=\"skhlt_\"></a>%s", pcszStartTag);
    pHlt->SetStartStrings("<?SK W?>", pcStartTag, 15);
    PR_smprintf_free(pcStartTag);
    pHlt->SetStopStrings("<?SK /W?>", pcszStopTag, -1);

    err = pHlt->SetValues( pPositions->GetSharedCursorDataRead(), iCount );
    if( err != noErr )
    {
        sk_DeleteInstance( pHlt );
        return err;
    }

    // pass the data to the hiliter
    if( ((char *)pContent->GetSharedData())[pContent->GetSize() - 1] == '\0' )
    {
        err = pHlt->FilterData( pContent->GetSharedData(), pContent->GetSize() );
    }
    else
    {
        char *pBinData = (char *)PR_Malloc(   (pContent->GetSize() + 1) 
                                            * sizeof(char) );
        if( pBinData != NULL )
        {
            memcpy(pBinData, pContent->GetSharedData(),
                   pContent->GetSize() * sizeof(char));
            pBinData[pContent->GetSize()] = '\0';

            err = pHlt->FilterData(pBinData, pContent->GetSize() + 1);

            PR_Free(pBinData);
        }
        else
            err = err_memory;
    }

    if( err != noErr )
    {
        sk_DeleteInstance( pHlt );
        return err;
    }

    // create a new binary to return (I know this copy could be avoided)
    //*ppTo = sk_CreateInstance(SKBinary)(hlt);
    *ppTo = pHlt;

    return noErr;
}
