/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorscorer.h,v 1.4.2.3 2005/02/21 14:22:44 krys Exp $
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

#ifndef __CURSORSCORER_H_
#define __CURSORSCORER_H_

class SKAPI SKIMuxCursorScorer   
{   
public:    
    virtual SKERR Compute(  PRUint32 iId, PRUint32* plRanks, PRBool* plMatch, 
                            PRUint32* pResult) = 0;      
};

#define SK_SKCURSORSCORER_IID                                           \
{ 0xc946880f, 0xaeb2, 0x4994,                                           \
    { 0x94, 0x86, 0x3f, 0x17, 0xc0, 0x9f, 0x7d, 0x1e } }

class SKAPI SKCursorScorer : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKCursorScorer)
    SK_REFCOUNT_INTF_IID(SKCursorScorer, SK_SKCURSORSCORER_IID)

    SKCursorScorer();
    ~SKCursorScorer();

    // Initialization
    SKERR Init(PRUint32 iSize, PRBool bSignedScores);

    // Do the sorting
    SKERR SetUnsortedCursor(SKCursor *pUnsortedCursor);

    // Accesor to the size
    SKERR GetSize(PRUint32 *piSize);

    // Accessors to the cursors
    SKERR GetCursor(PRBool bSorted, SKCursor **ppCursor);

    // Accessors to the scores (getters)
    SKERR GetSharedUnsignedScores(PRBool bSorted, PRUint32 **ppiScores);
    SKERR GetSharedSignedScores(PRBool bSorted, PRInt32 **ppiScores);

    // Accessors to the scores (setters)
    SKERR SetUnsignedScore(PRUint32 iRank, PRUint32 iScore);
    SKERR SetSignedScore(PRUint32 iRank, PRInt32 iScore);

    SKERR AddToUnsignedScore(PRUint32 iRank, PRUint32 iScoreDiff);
    SKERR AddToSignedScore(PRUint32 iRank, PRInt32 iScoreDiff);

    SKERR MuxCursorScorer(  SKCursorScorer** ppCursorScorer, PRUint32 iCount,
                            SKIMuxCursorScorer* pCallback,
                            PRBool* pbInterrupt);

    SKERR GetUnsignedScore(PRBool bSorted, PRUint32 iRank, PRUint32 *piScore)
    {
        if(iRank >= m_iSize) 
            return err_invalid;
        PRUint32 * pData;
        SKERR err = GetSharedUnsignedScores(bSorted, &pData);
        if(err != noErr)
            return err;
        *piScore = pData[iRank];
        return noErr;
    }
    
    SKERR GetSignedScore(PRBool bSorted, PRUint32 iRank, PRInt32 *piScore)
    {
        return GetUnsignedScore(bSorted, iRank, (PRUint32*) piScore);
    }

private:
    PRUint32 m_iSize;
    PRBool m_bSignedScores;

    skPtr<SKCursor> m_pUnsortedCursor;
    skPtr<SKCursor> m_pSortedCursor;

    PRUint32 *m_piUnsortedUnsignedScores;
    PRUint32 *m_piSortedUnsignedScores;

    PRInt32 *m_piUnsortedSignedScores;
    PRInt32 *m_piSortedSignedScores;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKCursorScorer, SK_SKCURSORSCORER_IID)

#else // __CURSORSCORER_H_
#error "Multiple inclusions of cursorscorer.h"
#endif // __CURSORSCORER_H_

