/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: mux.h,v 1.2.2.4 2005/02/21 14:22:44 krys Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
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

#ifndef __MUX_H_
#define __MUX_H_

class SKIMuxAncillaryCallback
{
public:
    virtual SKERR Compute(PRUint32 iId,
            PRUint32* plRanks,
            PRBool* plMatch,
            void* pResult) = 0;
};

class SKMux
{
public:
    SKMux();
    ~SKMux();

    SKERR MuxCursors(SKCursor **ppCursors, PRUint32 iCount,
                     SKICursorComparator *pComparator,
                     PRBool *pbInterrupt);

    SKERR MuxCursorsWithAncillary(
                     SKCursor **ppCursors, PRUint32 iCount,
                     PRUint32 lAncillaryItemSize,
                     SKIMuxAncillaryCallback* pCallback,
                     SKICursorComparator *pComparator,
                     PRBool *pbInterrupt);
    
    SKERR RetrieveData(PRUint32 *piCount, PRUint32 **ppiData)
    {
        return RetrieveDataWithAncillary(piCount, ppiData, NULL);
    }

    SKERR RetrieveDataWithAncillary(PRUint32 *piCount, PRUint32 **ppiData, 
            void** pAncillaryData);

protected:
    void  InsertIndexWithoutComparator(PRUint32 iIndex);
    SKERR InsertIndexWithComparator(PRUint32 iIndex,
                                    SKICursorComparator *pComparator);

    const PRUint32 **   m_ppiData;
    PRUint32 *          m_piCount;
    PRUint32 *          m_piPosition;

    PRUint32 *          m_piInputData;

    PRUint32 *          m_piSortedIndexes;
    PRUint32            m_iSortedWidth;

    PRUint32 *          m_piFinalData;
    void*               m_pAncillary;
    PRUint32            m_iFinalCount;

    PRBool*             m_pbKept;
};

#else
#error "Multiple inclusions of mux.h"
#endif

