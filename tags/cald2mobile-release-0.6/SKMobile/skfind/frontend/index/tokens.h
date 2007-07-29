/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: tokens.h,v 1.5.2.3 2005/02/21 14:22:46 krys Exp $
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

#ifndef __INDEXTOKENS_H_
#define __INDEXTOKENS_H_

// Misc structures
class IndexTokens
{
public:
    IndexTokens();
    ~IndexTokens();

    SKERR Init(PRUint32 iNearThreshold, SKIndex *pIndex);
    SKERR Parse(char *pszSearchString, PRBool bProx);
    SKERR ResolveLin(PRBool bUseFlex, skIStringSimplifier* pSimp,
                     SKIRecordFilter *pFilter);

    // DocumentList
    SKERR ComputeDocumentList(SKCursor **ppDocumentList);
    SKERR ComputePhraseDocumentList(SKCursor **ppDocumentList);
    SKERR ComputeTokenDocumentList(PRUint32 iToken, SKCursor **ppDocumentList);
    SKERR FilterNear(SKCursor *pReducedDocumentList, PRUint32 iPosition,
                     PRUint32 iNearThreshold, PRBool bAssertOrder);

    // HiliteInfo
    SKERR ComputeHiliteInfo(PRUint32 iDocId, SKCursor **ppHiliteInfo);
    SKERR ComputePhraseHiliteInfo(PRUint32 iDocId, SKCursor **ppHiliteInfo);
    SKERR ComputeTokenHiliteInfo(PRUint32 iToken, PRUint32 iDocId,
                                 SKCursor **ppHiliteInfo);
    SKERR FilterNearOccurrences(SKCursor *pReducedHiliteInfo,
                                SKCursor *pNextHiliteInfo,
                                PRUint32 iNearThreshold,
                                SKCursor **ppResult);

    // Scores
    SKERR ComputeScores(SKCursor *pDocumentList, SKIIndexScorer *pScorer);
    SKERR ComputePhraseScores(SKCursor *pDocumentList, SKIIndexScorer *pScorer);
    SKERR ComputeTokenScores(PRUint32 iToken, SKCursor *pDocumentList,
                             SKIIndexScorer *pScorer);

    skPtr<SKIndex> m_pIndex;
    PRUint32 m_iNearThreshold;

    SKERR GrowArrays();
    SearchToken *m_pTokens;
    PRUint32 *m_piLastHiliteDocId;
    PRUint32 *m_piLastHiliteNext;
    PRUint32 m_iTokenCount;
    PRUint32 m_iTokenSize;
    SKIRecordSet** m_ppLinRecordSets;
};

#else // __INDEXTOKENS_H_
#error "Multiple inclusions of tokens.h"
#endif // __INDEXTOKENS_H_

