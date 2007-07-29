/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorcomparator.h,v 1.9.2.5 2005/02/21 14:22:44 krys Exp $
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

#ifndef __SKF_CURSORCOMPARATOR_H_
#define __SKF_CURSORCOMPARATOR_H_

// SKICursorComparator

#define SK_SKICURSORCOMPARATOR_IID                                      \
{ 0x33b1e251, 0x46a9, 0x4004,                                           \
    { 0x84, 0x88, 0xd4, 0xbe, 0x00, 0xb4, 0x84, 0xdc } }

class SKAPI SKICursorComparator : public SKIComparator
{
public:
    SK_REFCOUNT_INTF_IID(SKICursorComparator, SK_SKICURSORCOMPARATOR_IID)

    SKICursorComparator();
    virtual ~SKICursorComparator();

    virtual SKERR       Compare(const void *p1, const void *p2,
                                PRInt32 *piResult);

    virtual SKERR       CompareRanks(PRUint32 iRank1, PRUint32 iRank2,
                                     PRInt32 *piResult) = 0;

            SKERR       GetCursorComparatorID(SKBinary** ppStrID);

            SKERR       SetSortingOrder(skfSortingOrder sortingOrder);

            PRBool      GetConsiderRank();
            SKERR       SetConsiderRank(PRBool ConsiderRank);

protected:
    virtual SKERR       GetMyComparatorID(SKBinary** ppStrId) = 0;
    PRBool              m_bConsiderRank;

private:
    // This value is handled by Compare() itself, that's why it is private
    skfSortingOrder     m_sortingOrder;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKICursorComparator, SK_SKICURSORCOMPARATOR_IID)

// SKCursorComparatorRank
class SKAPI SKCursorComparatorRank : public SKICursorComparator
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKCursorComparatorRank)

    virtual SKERR CompareRanks(PRUint32 iRank1, PRUint32 iRank2,
                               PRInt32 *piResult);
protected:
    virtual SKERR GetMyComparatorID(SKBinary** ppStrId);
};

// SKCursorComparatorRecordWrapper

#define SK_SKCURSORCOMPARATORRECORDWRAPPER_IID                          \
{ 0xa343de7d, 0xf025, 0x472e,                                           \
    { 0x99, 0xe2, 0x4a, 0xbd, 0x95, 0x28, 0x36, 0x39 } }

class SKAPI SKCursorComparatorRecordWrapper : public SKICursorComparator
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKCursorComparatorRecordWrapper)
    SK_REFCOUNT_INTF_IID(SKCursorComparatorRecordWrapper,
                         SK_SKCURSORCOMPARATORRECORDWRAPPER_IID)
    SKCursorComparatorRecordWrapper();

    virtual SKERR CompareRanks(PRUint32 iRank1, PRUint32 iRank2,
                               PRInt32 *piResult);
    SKERR SetRecordComparator(SKIRecordComparator *pRecordComparator,
                              PRBool bNoLookup);

protected:
    virtual SKERR GetMyComparatorID(SKBinary** ppStrId);
    skPtr<SKIRecordComparator> m_pRecordComparator;
    PRBool m_bNoLookup;

    skPtr<SKIRecordSet> m_pRecordSet;
    PRUint32 m_iLastRanks[2];
    skPtr<SKIRecord> m_pLastRecords[2];
};

SK_REFCOUNT_DECLARE_INTERFACE(SKCursorComparatorRecordWrapper,
                              SK_SKCURSORCOMPARATORRECORDWRAPPER_IID)

#else
#error "Multiple inclusions of cursor.h"
#endif
