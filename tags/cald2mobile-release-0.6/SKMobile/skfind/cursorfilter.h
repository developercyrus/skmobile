/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursorfilter.h,v 1.5.4.3 2005/02/21 14:22:44 krys Exp $
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

#ifndef __SKF_CURSORFILTER_H_
#define __SKF_CURSORFILTER_H_

class SKIRecordFilter;

// SKICursorFilter

#define SK_SKICURSORFILTER_IID                                          \
{ 0x0e84d693, 0xc706, 0x45b1,                                           \
    { 0x80, 0x9f, 0x96, 0xb5, 0xbf, 0x95, 0x09, 0xcf } }

class SKAPI SKICursorFilter : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKICursorFilter, SK_SKICURSORFILTER_IID)

    virtual SKERR Reset();
    virtual SKERR CheckRank(PRUint32 iRank, PRBool* pbKeepIt) = 0;

    virtual SKERR SetCursor(SKCursor *pCursor);

protected:
    skPtr<SKCursor> m_pCursor;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKICursorFilter, SK_SKICURSORFILTER_IID)

// SKCursorFilterRemoveDuplicated

#define SK_SKCURSORFILTERREMOVEDUPLICATED_IID                           \
{ 0x7d482bef, 0x4f59, 0x429c,                                           \
    { 0x91, 0xdd, 0x93, 0x52, 0xf2, 0x7d, 0xd8, 0x44 } }

class SKAPI SKCursorFilterRemoveDuplicated : public SKICursorFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKCursorFilterRemoveDuplicated)
    SK_REFCOUNT_INTF_IID(SKCursorFilterRemoveDuplicated,
                         SK_SKCURSORFILTERREMOVEDUPLICATED_IID)

    virtual SKERR CheckRank(PRUint32 iRank, PRBool* pbKeepIt);
};

SK_REFCOUNT_DECLARE_INTERFACE(SKCursorFilterRemoveDuplicated,
                              SK_SKCURSORFILTERREMOVEDUPLICATED_IID)

// SKCursorFilterRecordWrapper

#define SK_SKCURSORFILTERRECORDWRAPPER_IID                              \
{ 0xb1ea60b1, 0xb3a6, 0x4760,                                           \
    { 0xb9, 0xc4, 0x74, 0x91, 0x3d, 0x7c, 0x0b, 0xde } }

class SKAPI SKCursorFilterRecordWrapper : public SKICursorFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKCursorFilterRecordWrapper)
    SK_REFCOUNT_INTF_IID(SKCursorFilterRecordWrapper,
                         SK_SKCURSORFILTERRECORDWRAPPER_IID)

    SKCursorFilterRecordWrapper();

    SKERR SetRecordFilter(SKIRecordFilter *pRecordFilter, PRBool bNoLookup);

    virtual SKERR Reset();
    virtual SKERR CheckRank(PRUint32 iRank, PRBool* pbKeepIt);
    virtual SKERR SetCursor(SKCursor *pCursor);

protected:
    skPtr<SKIRecordFilter> m_pRecordFilter;
    PRBool m_bNoLookup;

    skPtr<SKIRecordSet> m_pRecordSet;
    PRUint32 m_iLastId;
    skPtr<SKIRecord> m_pLastRecord;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKCursorFilterRecordWrapper,
                              SK_SKCURSORFILTERRECORDWRAPPER_IID)

#else
#error "Multiple inclusions of cursorfilter.h"
#endif

