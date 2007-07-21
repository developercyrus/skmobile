/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: filters.cpp,v 1.13.2.3 2005/02/21 14:22:46 krys Exp $
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

// IndexNearDocFilter
SK_REFCOUNT_IMPL_DEFAULT(IndexNearDocFilter)

SKERR IndexNearDocFilter::Init(SKIndex *pIndex,
                               PRUint32 iNearThreshold,
                               SKIRecordSet *pRS1,
                               SKIRecordSet *pRS2,
                               PRBool bAssertOrder)
{
    m_bAssertOrder = bAssertOrder;
    m_pIndex = pIndex;
    m_iNearThreshold = iNearThreshold;

    m_pRS1 = pRS1;
    m_pRS2 = pRS2;

    SKERR err;

    err = m_pRS1->GetCount(&m_iCount1);
    if(err != noErr)
        return err;

    err = m_pRS2->GetCount(&m_iCount2);
    if(err != noErr)
        return err;

    return Reset();
}

SKERR IndexNearDocFilter::CheckRank(PRUint32 iRank, PRBool* bKeepIt)
{
    SKERR err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    PRUint32 iDocId = m_pCursor->GetSharedCursorDataRead()[iRank];

    skPtr<SKCursor> pOcc1;
    err = m_pIndex->BuildOccurrenceList(m_pRS1, &m_iNext1, m_iCount1, iDocId,
                                        pOcc1.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKCursor> pOcc2;
    err = m_pIndex->BuildOccurrenceList(m_pRS2, &m_iNext2, m_iCount2, iDocId,
                                        pOcc2.already_AddRefed());
    if(err != noErr)
        return err;

    return CheckOcc(pOcc1, pOcc2, bKeepIt);
}

SKERR IndexNearDocFilter::Reset()
{
    m_iNext1 = m_iNext2 = 0;
    return noErr;
}

SKERR IndexNearDocFilter::CheckOcc(SKCursor *pOcc1, SKCursor *pOcc2,
                                   PRBool *bKeepIt)
{
    PRUint32 i1, i2, iCount1, iCount2;
    const PRUint32 *pData1, *pData2;
    SKERR err;
    err = pOcc1->GetCount(&iCount1);
    if(err != noErr)
        return err;
    err = pOcc2->GetCount(&iCount2);
    if(err != noErr)
        return err;

    if(iCount2 == 0 || iCount1 == 0)
    {
        bKeepIt = 0;
        return noErr;
    }

    PR_ASSERT( (iCount2>0) && (iCount1>0) );

    err = pOcc1->ComputeCursorForm();
    if (err != noErr)
        return err;
    pData1 = pOcc1->GetSharedCursorDataRead();

    err = pOcc2->ComputeCursorForm();
    if (err != noErr)
        return err;
    pData2 = pOcc2->GetSharedCursorDataRead();

    if((iCount1 && !pData1) || (iCount2 && !pData2))
        return err_failure;

    *bKeepIt = PR_FALSE;

    i1 = i2 = 0;
    PRUint32 pos1 = pData1[i1];
    PRUint32 pos2 = pData2[i2];

    while( true )
    {
        if (pos1<=pos2)
        {
            if ( (pos2-pos1) <= m_iNearThreshold )
            {
                *bKeepIt = PR_TRUE;
                return noErr;
            }
            else
            {
                ++i1; 
                if (i1==iCount1)  return noErr;
                pos1=pData1[i1];
            }
        }
        else
        {
            if (!m_bAssertOrder && (pos1-pos2) <= m_iNearThreshold)
            {
                *bKeepIt = PR_TRUE;
                return noErr;
            }
            else
            {
                ++i2;
                if (i2==iCount2)  return noErr;
                pos2 = pData2[i2];
            }
        }
    }
}

// IndexDocPhraseFilter
SK_REFCOUNT_IMPL_DEFAULT(IndexDocPhraseFilter);

IndexDocPhraseFilter::IndexDocPhraseFilter()
{
    m_pTokens = NULL;
}

SKERR IndexDocPhraseFilter::Init(IndexTokens *pTokens)
{
    m_pTokens = pTokens;
    return noErr;
}

SKERR IndexDocPhraseFilter::CheckRank(PRUint32 iRank, PRBool* bKeepIt)
{
    *bKeepIt = PR_FALSE;

    if(!m_pTokens)
        return err_failure;

    skPtr<SKCursor> pMatches;
    SKERR err;
    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    err = m_pTokens->ComputePhraseHiliteInfo(
        m_pCursor->GetSharedCursorDataRead()[iRank],
        pMatches.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 iCount;
    err = pMatches->GetCount(&iCount);
    if(err != noErr)
        return err;

    if(iCount > 0)
        *bKeepIt = PR_TRUE;

    return noErr;
}

// IndexNearOccFilter
SK_REFCOUNT_IMPL_DEFAULT(IndexNearOccFilter);

SKERR IndexNearOccFilter::Init(SKCursor *pExternalCursor,
                               PRUint32 iNearThreshold,
                               PRBool bForwardFiltering)
{
    m_pExternalCursor = pExternalCursor;
    m_iNearThreshold = iNearThreshold;
    m_bForwardFiltering = bForwardFiltering;

    SKERR err;
    err = m_pExternalCursor->GetCount(&m_iCount);
    if(err != noErr)
        return err;

    return Reset();
}

SKERR IndexNearOccFilter::CheckRank(PRUint32 iRank, PRBool* bKeepIt)
{
    SKERR err;
    *bKeepIt = PR_FALSE;

    err = m_pCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    PRUint32 iPos = m_pCursor->GetSharedCursorDataRead()[iRank];
    err = m_pExternalCursor->ComputeCursorForm();
    if (err != noErr)
        return err;
    const PRUint32 *pExternalData=m_pExternalCursor->GetSharedCursorDataRead();

    if(m_bForwardFiltering)
    {
        while(    (m_iNext < m_iCount)
               && (pExternalData[m_iNext] + m_iNearThreshold < iPos))
        {
            m_iNext++;
        }

        if((m_iNext < m_iCount) && (pExternalData[m_iNext] < iPos))
        {
            *bKeepIt = PR_TRUE;
        }
    }
    else
    {
        while((m_iNext < m_iCount) && (pExternalData[m_iNext] < iPos))
        {
            m_iNext++;
        }

        if(    (m_iNext < m_iCount)
            && (pExternalData[m_iNext] <= iPos + m_iNearThreshold))
        {
            *bKeepIt = PR_TRUE;
        }
    }

    return noErr;
}

SKERR IndexNearOccFilter::Reset()
{
    m_iNext = 0;
    return noErr;
}

