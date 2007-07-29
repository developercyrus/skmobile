/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: pertinencescorer.cpp,v 1.9.2.3 2005/02/21 14:22:46 krys Exp $
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
#include <skfind/frontend/wordlist/wordlist.h>
#include <skfind/frontend/wordlist/wildcardwordlist.h>

#include "index.h"
#include "pertinencescorer.h"

SK_REFCOUNT_IMPL_DEFAULT(SKPertinenceScorer)
SK_REFCOUNT_IMPL_IID(SKPertinenceScorer, SK_SKPERTINENCESCORER_IID,
                     SKIIndexScorer)

SKPertinenceScorer::SKPertinenceScorer()
{
    m_iBoldFactor = 1;
    m_iItalicFactor = 1;
    m_piElementaryScores[0] = 100;
    m_piElementaryScores[1] = 20;
    m_piElementaryScores[2] = 10;
    m_piElementaryScores[3] = 5;
    m_piElementaryScores[4] = 4;
    m_piElementaryScores[5] = 3;
    m_piElementaryScores[6] = 2;
    m_piElementaryScores[7] = 1;
}

SKERR SKPertinenceScorer::SetBoldFactor(PRUint32 iBoldFactor)
{
    m_iBoldFactor = iBoldFactor;
    return noErr;
}

SKERR SKPertinenceScorer::SetItalicFactor(PRUint32 iItalicFactor)
{
    m_iItalicFactor = iItalicFactor;
    return noErr;
}

SKERR SKPertinenceScorer::SetElementaryScores(const PRUint32*piElementaryScores)
{
    for(PRUint32 i = 0; i < 8; ++i)
        m_piElementaryScores[i] = piElementaryScores[i];
    return noErr;
}

SKERR SKPertinenceScorer::Init(PRUint32 iCount)
{
    *m_pCursorScorer.already_AddRefed() = sk_CreateInstance(SKCursorScorer)();
    if(!m_pCursorScorer)
        return err_memory;

    return m_pCursorScorer->Init(iCount, PR_FALSE);
}

SKERR SKPertinenceScorer::AddScore(PRUint32 iRank, PRUint32 iDocId,
                                   PRUint32 iStructure, PRUint32 iOccCount)
{
    PRUint32 iScoreDiff =
        m_piElementaryScores[(iStructure >> 2) & 0x7] * iOccCount;

    if(iStructure & 0x1)
        iScoreDiff = iScoreDiff * m_iBoldFactor;

    if(iStructure & 0x2)
        iScoreDiff = iScoreDiff * m_iItalicFactor;

    return m_pCursorScorer->AddToUnsignedScore(iRank, iScoreDiff);
}

SKERR SKPertinenceScorer::GetCursorScorer(SKCursorScorer **ppCursorScorer)
{
    return m_pCursorScorer.CopyTo(ppCursorScorer);
}

