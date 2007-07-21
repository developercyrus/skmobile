/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorrs.h,v 1.7.2.7 2005/02/21 14:22:38 krys Exp $
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

#ifndef __SKF_CURSORRS_H_
#define __SKF_CURSORRS_H_

#define SK_SKCURSORRECORDSET_IID                                        \
{ 0x4caf4a81, 0x2d63, 0x49d7,                                           \
    { 0xa2, 0x41, 0xf2, 0x36, 0xb9, 0x82, 0xb6, 0x0a } }

class SKCursorRecordSet : public SKIRecordSet
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKCursorRecordSet)
    SK_REFCOUNT_INTF_IID(SKCursorRecordSet, SK_SKCURSORRECORDSET_IID);

    virtual SKERR Init(PRUint32 iOffset, PRUint32 iCount,
                       SKIRecordSet *pRecordSet);
    virtual SKERR Init(SKCursor *pCursor, SKIRecordSet *pRecordSet);
    virtual SKERR InitCursorRS(PRUint32 iOffset,
                               PRUint32 iCount,
                               SKIRecordSet *pRecordSet,
                               SKRefCount *pFragment);

    virtual SKERR SetCursorAndRecordSet(SKCursor* pCursor,
                                        SKIRecordSet* pRecordSet)
        { return Init(pCursor, pRecordSet); }
    
    virtual SKERR   GetInternalCursor(SKCursor**);

    virtual SKERR AppendSpeeder(SKRefCount *pSpeeder);

    virtual ~SKCursorRecordSet();

    // SKIRecordSet
    virtual SKERR   GetCount(PRUint32 *piCount) const;
    virtual SKERR   GetRecord(PRUint32 id, SKIRecord** pRecord);
    virtual SKERR   GetFldCollection(SKIFldCollection** pFldCol);

    virtual SKERR   LookupTextImp(const char * pszSearch, skfLookupMode mode,
                               PRUint32 *piId);
    virtual SKERR   LookupUNumImp(PRUint32 lNum, skfLookupMode mode,
                               PRUint32 *piId);
    virtual SKERR   LookupSNumImp(PRInt32 lNum, skfLookupMode mode,
                               PRUint32 *piId);

    virtual SKERR   GetSubRecordSet(PRUint32 lOffset, PRUint32 lCount,
                                    SKIRecordSet** ppIRecordSet);

    virtual SKERR   ExtractCursor(SKIField* pIField, PRUint32 iOffset,
                                  PRUint32 iCount, SKCursor** ppCursor);

    virtual SKERR   Filter(SKIRecordFilter *pFilter);
    virtual SKERR   Merge(SKIRecordSet *pRecordSet,
                          skfOperator oper,
                          SKIRecordComparator *pComparator,
                          PRBool bConsiderRank);
    virtual SKERR   Sort(SKIRecordComparator *pComparator,
                         PRBool bConsiderRank);

    virtual SKERR   FilterToNew(SKIRecordFilter *pFilter,
                                SKIRecordSet **ppNewRecordSet);
    virtual SKERR   MergeToNew(SKIRecordSet *pRecordSet,
                               skfOperator oper,
                               SKIRecordComparator *pComparator,
                               PRBool bConsiderRank,
                               SKIRecordSet **ppNewRecordSet);
    virtual SKERR   SortToNew(SKIRecordComparator *pComparator,
                              PRBool bConsiderRank,
                              SKIRecordSet **ppNewRecordSet);

    virtual SKERR   GetLookupField(SKIField ** ppField);

private:
    skPtr<SKCursor> m_pCursor;
    skPtr<SKIRecordSet> m_pRecordSet;

    PRUint32 m_iSpeederCount;
    SKRefCount ** m_ppSpeeders;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKCursorRecordSet, SK_SKCURSORRECORDSET_IID);

#else
#error "multiple inclusions of cursorrs.h"
#endif

