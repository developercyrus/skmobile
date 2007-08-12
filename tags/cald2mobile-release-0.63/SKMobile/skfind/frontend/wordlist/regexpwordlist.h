/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: regexpwordlist.h,v 1.4.2.3 2005/02/21 14:22:47 krys Exp $
 *
 * Authors: Alexis Seigneurin <seigneurin @at@ idm .dot. fr>
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

#ifndef __REGEXPWORDLIST_H_
#define __REGEXPWORDLIST_H_

#define SK_SKREGEXPWORDLIST_IID                                         \
{ 0x48e5d7df, 0xba2a, 0x4c86,                                           \
    { 0x93, 0x40, 0x7a, 0x60, 0xac, 0x05, 0x4a, 0x41 } }

class SKAPI SKRegExpWordList : public SKWordList
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRegExpWordList)
    SK_REFCOUNT_INTF_IID(SKRegExpWordList, SK_SKREGEXPWORDLIST_IID)

    SKRegExpWordList();
    ~SKRegExpWordList();
    
    SKERR GetRegExpWords(skIStringSimplifier* pSimp, const char* pszLinks, 
                         const char* pszToken, SKIRecordFilter* filter,
                         SKCursor** ppCursor);
    
protected:
    virtual SKERR ConfigureItem(char* pszSection,char* pszToken,char* pszValue);
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRegExpWordList, SK_SKREGEXPWORDLIST_IID)

#endif
