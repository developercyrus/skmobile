/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: pertinencescorer.h,v 1.3.2.3 2005/02/21 14:22:46 krys Exp $
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

#ifndef __PERTINENCESCORER_H_
#define __PERTINENCESCORER_H_

#define SK_SKPERTINENCESCORER_IID                                       \
{ 0x612f9d46, 0x28c8, 0x4e2e,                                           \
    { 0xb3, 0xc6, 0x67, 0x6d, 0x1a, 0x12, 0x9f, 0x34 } }

class SKAPI SKPertinenceScorer : public SKIIndexScorer
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKPertinenceScorer)
    SK_REFCOUNT_INTF_IID(SKPertinenceScorer, SK_SKPERTINENCESCORER_IID)

    SKPertinenceScorer();

    // What about floating point factors ? -- bozo
    SKERR SetBoldFactor(PRUint32 iBoldFactor);
    SKERR SetItalicFactor(PRUint32 iItalicFactor);
    SKERR SetElementaryScores(const PRUint32 *piElementaryScores);

    virtual SKERR Init(PRUint32 iCount);

    virtual SKERR AddScore(PRUint32 iRank, PRUint32 iDocId, PRUint32 iStructure,
                           PRUint32 iOccCount);

    virtual SKERR GetCursorScorer(SKCursorScorer **ppCursorScorer);

private:
    skPtr<SKCursorScorer> m_pCursorScorer;

    PRUint32 m_iBoldFactor;
    PRUint32 m_iItalicFactor;
    PRUint32 m_piElementaryScores[8];
};

SK_REFCOUNT_DECLARE_INTERFACE(SKPertinenceScorer, SK_SKPERTINENCESCORER_IID)

#else // __PERTINENCESCORER_H_
#error "Multiple inclusions of pertinencescorerh"
#endif // __PERTINENCESCORER_H_
