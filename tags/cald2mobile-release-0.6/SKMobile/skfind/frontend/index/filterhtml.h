/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: filterhtml.h,v 1.6.4.3 2005/02/21 14:22:46 krys Exp $
 *
 * Authors: Marc Ariberti <ariberti @at@ idm .dot. fr>
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

#ifndef __FILTERHTML_H
#define __FILTERHTML_H

#define SK_SKRECORDFILTERHTML_IID                                       \
{ 0xe38b6d56, 0x3616, 0x482a,                                           \
    { 0x92, 0x6f, 0x95, 0xa8, 0x10, 0x89, 0x4b, 0x41 } }

class SKAPI SKRecordFilterHtml : public SKIRecordFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordFilterHtml)
    SK_REFCOUNT_INTF_IID(SKRecordFilterHtml, SK_SKRECORDFILTERHTML_IID)

    SKRecordFilterHtml();
    ~SKRecordFilterHtml();

    SKERR SetStructureFieldName(const char *pszStructureFieldName);

    SKERR SetCheckMainTitle(PRBool bTitle);
    SKERR SetCheckTitleLevel(PRUint32 iTitleLevel);

    virtual SKERR SetRecordSet(SKIRecordSet* pRecordSet);

    virtual SKERR CheckRecord(SKIRecord *pRecord, PRBool *pbKeepIt);

private:
    char *              m_pszStructureFieldName;
    skPtr<SKIField>     m_pStructureField;

    PRBool              m_bTitle;
    PRUint32            m_iTitleLevel;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordFilterHtml, SK_SKRECORDFILTERHTML_IID)

#else
#error "Multiple inclusions of filterhtml.h"
#endif

