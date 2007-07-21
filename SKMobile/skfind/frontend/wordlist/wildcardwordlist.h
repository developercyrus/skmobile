/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: wildcardwordlist.h,v 1.12.2.4 2005/02/21 14:22:47 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#ifndef __WILDCARDWORDLIST_H_
#define __WILDCARDWORDLIST_H_

#define SK_SKWILDCARDWORDLIST_IID                                       \
{ 0x2f59c5a2, 0xe2bc, 0x4ef3,                                           \
    { 0x9f, 0xd4, 0xc5, 0x2e, 0xec, 0x04, 0x46, 0x06 } }

class SKAPI SKWildCardWordList : public SKWordList
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKWildCardWordList)
    SK_REFCOUNT_INTF_IID(SKWildCardWordList, SK_SKWILDCARDWORDLIST_IID)

    SKWildCardWordList();
    ~SKWildCardWordList();
    
    SKERR GetWildCardWords(skIStringSimplifier* pSimp, const char* pszLinks, 
                       const char* pszToken, SKIRecordFilter* filter,
                       SKCursor** ppCursor);
    
    SKERR SetSingleWildCard(const char* pszWC);
    SKERR SetMultipleWildCard(const char* pszWC);
    
    const char* GetSharedSingleWildCard() { return m_pcSingleWildCard; }
    const char* GetSharedMultipleWildCard() { return m_pcMultipleWildCard; }
    
protected:
    virtual SKERR ConfigureItem(char* pszSection,char* pszToken,char* pszValue);

    char *              m_pcSingleWildCard;
    char *              m_pcMultipleWildCard;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKWildCardWordList, SK_SKWILDCARDWORDLIST_IID)

#define WCWL_DEFAULT_SINGLE_WILDCARD "?"
#define WCWL_DEFAULT_MULTIPLE_WILDCARD "*"

#endif
