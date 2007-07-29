/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorscorer.cpp,v 1.12.2.4 2005/02/21 14:22:44 krys Exp $
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
#include "cursorcomparator.h"
#include "recordcomparator.h"
#include "cursor.h"
#include "mux.h"

#include "cursorscorer.h"

static int _CompareDocIdScore(const void *p1, const void *p2)
{
    // p[0] is the document ID
    // p[1] is the score
    // This code works even for signed scores
    PRUint32 i1 = ((PRUint32 *)p1)[1];
    PRUint32 i2 = ((PRUint32 *)p2)[1];
    // - decreasing order of scores
    // - increasing order of document ID
    if(i1 != i2)
        return i2 - i1;
    else
        return ((PRUint32 *)p1)[0] - ((PRUint32 *)p2)[0];
}

SK_REFCOUNT_IMPL_DEFAULT(SKCursorScorer);
SK_REFCOUNT_IMPL_IID(SKCursorScorer, SK_SKCURSORSCORER_IID, SKRefCount)

SKCursorScorer::SKCursorScorer()
{
    m_iSize = 0;
    m_bSignedScores = PR_FALSE;
    m_piUnsortedUnsignedScores = NULL;
    m_piSortedUnsignedScores = NULL;
    m_piUnsortedSignedScores = NULL;
    m_piSortedSignedScores = NULL;
}

SKCursorScorer::~SKCursorScorer()
{
#define _FreeArray(x) if(x) delete[] x;
    _FreeArray(m_piUnsortedUnsignedScores);
    _FreeArray(m_piSortedUnsignedScores);
    _FreeArray(m_piUnsortedSignedScores);
    _FreeArray(m_piSortedSignedScores);
#undef _FreeArray
}

SKERR SKCursorScorer::Init(PRUint32 iSize, PRBool bSignedScores)
{
    m_pUnsortedCursor = NULL;
    m_pSortedCursor = NULL;

#define _NullArray(x) if(x) { delete[] x; x = NULL; }
    _NullArray(m_piUnsortedUnsignedScores);
    _NullArray(m_piSortedUnsignedScores);
    _NullArray(m_piUnsortedSignedScores);
    _NullArray(m_piSortedSignedScores);
#undef _NullArray

    m_iSize = 0;
    m_bSignedScores = PR_FALSE;

    if(iSize)
    {
        if(!bSignedScores)
        {
            m_piUnsortedUnsignedScores = new PRUint32[iSize];
            if(!m_piUnsortedUnsignedScores)
                return err_memory;
            memset(m_piUnsortedUnsignedScores, 0, iSize * sizeof(PRUint32));
        }
        else
        {
            m_piUnsortedSignedScores = new PRInt32[iSize];
            if(!m_piUnsortedSignedScores)
                return err_memory;
            memset(m_piUnsortedSignedScores, 0, iSize * sizeof(PRUint32));
        }
    }

    m_iSize = iSize;
    m_bSignedScores = bSignedScores;

    return noErr;
}

SKERR SKCursorScorer::SetUnsortedCursor(SKCursor *pUnsortedCursor)
{
    m_pUnsortedCursor = NULL;
    m_pSortedCursor = NULL;
#define _NullArray(x) if(x) { delete[] x; x = NULL; }
    _NullArray(m_piSortedUnsignedScores);
    _NullArray(m_piSortedSignedScores);
    // Here we have only m_iSize valid and m_piUnsorted(Uns/S)ignedScores is
    // an array of m_iSize integers

    if(!pUnsortedCursor)
        return err_invalid;

    PRUint32 iCount;
    SKERR err;

    err = pUnsortedCursor->GetCount(&iCount);
    if(err != noErr)
        return err;

    if(m_iSize != iCount)
        return err_invalid;

    if(m_iSize)
    {
        // Signedness...
        PRUint32 *piUnsortedScores;
        PRUint32 *piSortedScores;
        if(!m_bSignedScores)
        {
            piUnsortedScores = m_piUnsortedUnsignedScores;
            m_piSortedUnsignedScores = new PRUint32[m_iSize];
            if(!m_piSortedUnsignedScores)
                return err_memory;
            piSortedScores = m_piSortedUnsignedScores;
        }
        else
        {
            piUnsortedScores = (PRUint32 *)m_piUnsortedSignedScores;
            m_piSortedSignedScores = new PRInt32[m_iSize];
            if(!m_piSortedSignedScores)
                return err_memory;
            piSortedScores = (PRUint32 *)m_piSortedSignedScores;
        }

        err = pUnsortedCursor->ComputeCursorForm();
        if (err != noErr)
            return err;
        const PRUint32 *pUnsortedCursorData =
            pUnsortedCursor->GetSharedCursorDataRead();
        if(!pUnsortedCursorData)
            return err_failure;

        // Memory allocation
        skPtr<SKCursor> pSortedCursor;
        *pSortedCursor.already_AddRefed() =
                sk_CreateInstance(SKCursor)(m_iSize, NULL);
        if(!pSortedCursor)
        {
            _NullArray(m_piSortedUnsignedScores);
            _NullArray(m_piSortedSignedScores);
            return err_memory;
        }
        err = pSortedCursor->ComputeCursorForm();
        if (err != noErr)
            return err;
        PRUint32 *pSortedCursorData = pSortedCursor->GetSharedCursorDataWrite();
        if(!pSortedCursorData)
        {
             _NullArray(m_piSortedUnsignedScores);
             _NullArray(m_piSortedSignedScores);
             return err_memory;
        }

        PRUint32 *pTmpData = new PRUint32[2 * m_iSize];
        if(!pTmpData)
        {
            _NullArray(m_piSortedUnsignedScores);
            _NullArray(m_piSortedSignedScores);
            return err_memory;
        }
#undef _NullArray

        // No error will happen now.
        m_pUnsortedCursor = pUnsortedCursor;
        m_pSortedCursor = pSortedCursor;

        // Prepare sorting...
        for(PRUint32 i = 0; i < m_iSize; ++i)
        {
            pTmpData[i << 1] = pUnsortedCursorData[i];
            pTmpData[(i << 1) + 1] = piUnsortedScores[i];
        }

        // Sort...
        qsort(pTmpData, m_iSize, 2 * sizeof(PRUint32), _CompareDocIdScore);

        // Finish sorting...
        for(PRUint32 j = 0; j < m_iSize; ++j)
        {
            pSortedCursorData[j] = pTmpData[j << 1];
            piSortedScores[j] = pTmpData[(j << 1) + 1];
        }

        m_pSortedCursor->ReleaseSharedCursorDataWrite();

        delete[] pTmpData;

        return noErr;
    }
    else
    {
        *m_pSortedCursor.already_AddRefed() =
                sk_CreateInstance(SKCursor)(0, NULL);
        if(!m_pSortedCursor)
            return err_memory;

        m_pUnsortedCursor = pUnsortedCursor;

        return noErr;
    }
}

SKERR SKCursorScorer::GetSize(PRUint32 *piSize)
{
    *piSize = m_iSize;
    return noErr;
}

SKERR SKCursorScorer::GetCursor(PRBool bSorted, SKCursor **ppCursor)
{
    if(!ppCursor)
        return err_invalid;
    if(bSorted)
        return m_pSortedCursor.CopyTo(ppCursor);
    else
        return m_pUnsortedCursor.CopyTo(ppCursor);
}

SKERR SKCursorScorer::GetSharedUnsignedScores(PRBool bSorted,
                                              PRUint32 **ppiScores)
{
    if(!ppiScores || m_bSignedScores)
        return err_invalid;
    if(bSorted)
    {
        *ppiScores = m_piSortedUnsignedScores;
        return noErr;
    }
    else
    {
        *ppiScores = m_piUnsortedUnsignedScores;
        return noErr;
    }
}

SKERR SKCursorScorer::GetSharedSignedScores(PRBool bSorted,
                                              PRInt32 **ppiScores)
{
    if(!ppiScores || !m_bSignedScores)
        return err_invalid;
    if(bSorted)
    {
        *ppiScores = m_piSortedSignedScores;
        return noErr;
    }
    else
    {
        *ppiScores = m_piUnsortedSignedScores;
        return noErr;
    }
}

SKERR SKCursorScorer::SetUnsignedScore(PRUint32 iRank, PRUint32 iScore)
{
    if(!m_iSize || m_bSignedScores || (iRank >= m_iSize))
        return err_failure;
    m_piUnsortedUnsignedScores[iRank] = iScore;
    return noErr;
}

SKERR SKCursorScorer::SetSignedScore(PRUint32 iRank, PRInt32 iScore)
{
    if(!m_iSize || !m_bSignedScores || (iRank >= m_iSize))
        return err_failure;
    m_piUnsortedSignedScores[iRank] = iScore;
    return noErr;
}


SKERR SKCursorScorer::AddToUnsignedScore(PRUint32 iRank, PRUint32 iScoreDiff)
{
    if(!m_iSize || m_bSignedScores || (iRank >= m_iSize))
        return err_failure;
    m_piUnsortedUnsignedScores[iRank] += iScoreDiff;
    return noErr;
}


SKERR SKCursorScorer::AddToSignedScore(PRUint32 iRank, PRInt32 iScoreDiff)
{
    if(!m_iSize || !m_bSignedScores || (iRank >= m_iSize))
        return err_failure;
    m_piUnsortedSignedScores[iRank] += iScoreDiff;
    return noErr;
}

class SKCursorScorerWrapper : public SKIMuxAncillaryCallback
{
public:
    SKCursorScorerWrapper(SKIMuxCursorScorer* pWrapped) 
        : m_pWrapped(pWrapped) {};
    virtual SKERR Compute(PRUint32 iId,
            PRUint32* plRanks,
            PRBool* plMatch,
            void* pResult)
    {
        return m_pWrapped->Compute(iId, plRanks, plMatch, (PRUint32*) pResult);
    }
private:
    SKIMuxCursorScorer* m_pWrapped;
};

SKERR SKCursorScorer::MuxCursorScorer(  SKCursorScorer** ppCursorScorer, 
                                        PRUint32 iWidth,
                                        SKIMuxCursorScorer* pCallback,
                                        PRBool* pbInterrupt)
{
    if(!iWidth)
        return noErr;
    
    SK_ASSERT(ppCursorScorer && *ppCursorScorer);
    if( !ppCursorScorer || !*ppCursorScorer )
        return err_invalid;

    SKMux mux;
    SKCursor** ppCursors = new SKCursor*[iWidth];
    for(PRUint32 i = 0; i<iWidth; i++)
        ppCursors[i] = ppCursorScorer[i]->m_pUnsortedCursor;
    SKCursorScorerWrapper wrapper(pCallback);
    SKERR err = mux.MuxCursorsWithAncillary(ppCursors, iWidth,
            sizeof(PRUint32), &wrapper, 
            NULL, pbInterrupt);
    delete [] ppCursors;

    if(err != noErr)
        return err;
    
    PRUint32 iCount;
    PRUint32* piData;
    PRUint32* piScores;
    
    err = mux.RetrieveDataWithAncillary(&iCount, &piData, (void**) &piScores);
    if(err != noErr)
        return err;

    err = Init(iCount, ppCursorScorer[0]->m_bSignedScores);
    if(err != noErr)
        return err;

    memcpy(m_bSignedScores  ? (PRUint32*)m_piUnsortedSignedScores 
                            : m_piUnsortedUnsignedScores, 
           piScores, iCount*sizeof(PRUint32));

    skPtr<SKCursor> pCursor;
    *pCursor.already_AddRefed() = sk_CreateInstance(SKCursor)(iCount, piData);
    err = SetUnsortedCursor(pCursor);
    if(err != noErr)
        return err;

    return noErr;
}

