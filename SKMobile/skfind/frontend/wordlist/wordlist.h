/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: wordlist.h,v 1.23.2.4 2005/02/21 14:22:47 krys Exp $
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

#ifndef __WORDLIST_H_
#define __WORDLIST_H_

#define SK_SKWORDLIST_IID                                               \
{ 0xd3bb92fe, 0x54a4, 0x4981,                                           \
    { 0xae, 0x87, 0x3b, 0x04, 0xc8, 0x12, 0xe1, 0xf1 } }

#define SKF_FE_WORDLIST_PRESORTED "SKF_FE_WORDLIST_PRESORTED"

class SKAPI SKWordList: public SKTextFile
{
public:
    enum  eSearchMode { MODE_INVALID = 0L, MODE_EXACT = 1L,
                        MODE_BEGIN = 2L, MODE_END = 3L};

    SK_REFCOUNT_INTF_DEFAULT(SKWordList)
    SK_REFCOUNT_INTF_IID(SKWordList, SK_SKWORDLIST_IID)

    SKWordList();
    ~SKWordList();

    virtual SKERR SetFileName(const char *pszFileName,
                              const char *pszDefaultFileName = NULL);

    SKERR SetWordRecordSet(SKIRecordSet *pRecordSet);
    SKERR GetWordRecordSet(SKIRecordSet **ppRecordSet);

    SKERR SetLookupRecordSet(SKIRecordSet *pRecordSet,
                             SKIField *pLookupField,
                             SKIField *pLookupSubField,
                             SKIField *pTextTokenField);
    SKERR GetLookupRecordSet(SKIRecordSet **ppRecordSet,
                             SKIField **ppLookupField,
                             SKIField **ppLookupSubField,
                             SKIField **ppTextTokenField);

    SKERR SetRevLookupRecordSet(SKIRecordSet *pRecordSet,
                                SKIField *pRevLookupField);
    SKERR GetRevLookupRecordSet(SKIRecordSet **ppRecordSet,
                                SKIField **ppRevLookupField);

    SKERR SetCapLookupRecordSet(SKIRecordSet *pRecordSet,
                                SKIField *pCapLookupField,
                                SKIField *pCapLookupSubField);
    SKERR GetCapLookupRecordSet(SKIRecordSet **ppRecordSet,
                                SKIField **ppCapLookupField,
                                SKIField **ppCapLookupSubField);

    SKERR SetRevCapLookupRecordSet(SKIRecordSet *pRecordSet,
                                   SKIField *pRevCapLookupField);
    SKERR GetRevCapLookupRecordSet(SKIRecordSet **ppRecordSet,
                                   SKIField **ppRevCapLookupField);

    SKERR GetWords(eSearchMode mode, PRBool bSensitive,
                   SKIRecordFilter* pFilter, const char* pszLinks,
                   const char* pszToken, SKCursor** ppCursor);

    SKERR ResolveLinks(SKCursor* pFrom, const char* pszLinks, SKCursor** ppTo);

    void Interrupt();

protected:
    virtual SKERR SetSimplifierURL(const char* pcUrl);

    virtual SKERR ConfigureItem( char* pszSection, char* pszToken,
                                 char* pszValue);

    SKERR ResolveLink(SKCursor *pFrom, const char *pszField,
                      const char *pszSubField, SKCursor **ppTo);

    SKERR ResolveUNum(SKCursor *pFrom, const char *pszField,
                      SKCursor **ppTo);

    skPtr<SKIRecordSet>         m_pWordRecordSet;
    skPtr<SKIFldCollection>     m_pWordFldCollection;

    skPtr<SKIRecordSet>         m_pLookupRecordSet;
    skPtr<SKIField>             m_pLookupField;
    skPtr<SKIField>             m_pLookupSubField;
    skPtr<SKIField>             m_pTextTokenField;

    skPtr<SKIRecordSet>         m_pRevLookupRecordSet;
    skPtr<SKIField>             m_pRevLookupField;

    skPtr<SKIRecordSet>         m_pCapLookupRecordSet;
    skPtr<SKIField>             m_pCapLookupField;
    skPtr<SKIField>             m_pCapLookupSubField;

    skPtr<SKIRecordSet>         m_pRevCapLookupRecordSet;
    skPtr<SKIField>             m_pRevCapLookupField;

    PRPackedBool                m_bInitialized;
    PRPackedBool                m_bSubRSSorted;

    // Do not switch to PRPackedBool for this member.
    PRBool                      m_bInterrupt;

    skPtr<skIStringSimplifier>  m_pTableSimplifier;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKWordList, SK_SKWORDLIST_IID)

//
// error constants
#define err_wl_invalid                 600
#define err_wl_malloc                  601
#define err_wl_fopen                   602

#define err_wl_config                  611

#endif

