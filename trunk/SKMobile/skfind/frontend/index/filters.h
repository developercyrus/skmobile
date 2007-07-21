/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: filters.h,v 1.4.4.2 2005/02/21 14:22:46 krys Exp $
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

#ifndef __INDEXFILTERS_H_
#define __INDEXFILTERS_H_

// IndexNearDocFilter
class IndexNearDocFilter : public SKICursorFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(IndexNearDocFilter)

    SKERR Init(SKIndex *pIndex,
               PRUint32 iNearThreshold,
               SKIRecordSet *pRS1,
               SKIRecordSet *pRS2,
               PRBool bAssertOrder);

    virtual SKERR CheckRank(PRUint32 iRank, PRBool* bKeepIt);
    virtual SKERR Reset();
private:
    SKERR CheckOcc(SKCursor *pOcc1, SKCursor *pOcc2, PRBool *bKeepIt);

    skPtr<SKIndex> m_pIndex;
    PRUint32 m_iNearThreshold;
    PRBool  m_bAssertOrder;

    skPtr<SKIField> m_pDocIdField;
    skPtr<SKIField> m_pOccOffsetField;
    skPtr<SKIField> m_pOccCountField;

    skPtr<SKIRecordSet> m_pOccRecordSet;
    skPtr<SKIField> m_pOccIdField;

    skPtr<SKIRecordSet> m_pRS1;
    skPtr<SKIRecordSet> m_pRS2;
    PRUint32 m_iCount1;
    PRUint32 m_iCount2;
    PRUint32 m_iNext1;
    PRUint32 m_iNext2;
};

// IndexDocPhraseFilter
class IndexDocPhraseFilter : public SKICursorFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(IndexDocPhraseFilter)

    IndexDocPhraseFilter();

    SKERR Init(IndexTokens *pTokens);
    virtual SKERR CheckRank(PRUint32 iRank, PRBool* bKeepIt);

private:
    IndexTokens *m_pTokens;
};

// IndexNearOccFilter
class IndexNearOccFilter : public SKICursorFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(IndexNearOccFilter)

    SKERR Init(SKCursor *pExternalCursor,
               PRUint32 iNearThreshold,
               PRBool bForwardFiltering);

    virtual SKERR CheckRank(PRUint32 iRank, PRBool* bKeepIt);
    virtual SKERR Reset();
private:
    skPtr<SKCursor> m_pExternalCursor;
    PRUint32 m_iNearThreshold;
    PRBool m_bForwardFiltering;

    PRUint32 m_iNext;
    PRUint32 m_iCount;
};

#else // __INDEXFILTERS_H_
#error "Multiple inclusions of filters.h"
#endif // __INDEXFILTERS_H_

