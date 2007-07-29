/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: index.h,v 1.33.2.7 2005/02/21 14:22:46 krys Exp $
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

#ifndef __INDEX_H_
#define __INDEX_H_

class SKIndexResult;
class SKIIndexScorer;
class IndexTokens;

//  SKIndex
//  --------------------------------------------------------------------

#define SK_SKINDEX_IID                                                  \
{ 0x0c054c13, 0x5489, 0x48d3,                                           \
    { 0xb3, 0xfb, 0x58, 0x73, 0x3d, 0x36, 0x52, 0x23 } }

#define SKF_FE_INDEX_PRESORTED "SKF_FE_INDEX_PRESORTED"

class SKAPI SKIndex : public SKTextFile
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKIndex)
    SK_REFCOUNT_INTF_IID(SKIndex, SK_SKINDEX_IID)

                        SKIndex();
    virtual             ~SKIndex();

    static  SKERR       SetOperators(const char* pszOperators);

    virtual SKERR       SetFileName(const char *pszFileName,
                                    const char *pszDefaultFileName = NULL);

            SKERR       GetWordRecordSet(SKIRecordSet **ppRecordSet);

            SKERR       GetDocLinkField(SKIField **ppField);
            SKERR       GetDocIdField(SKIField **ppField);
            SKERR       GetStructureField(SKIField **ppField);
            SKERR       GetOccLinkField(SKIField **ppField);
            PRBool      HasOccurencies() { return m_pOccLinkField != NULL; }

            SKERR       SetNearThreshold(PRUint32 iThreshold);
            SKERR       GetNearThreshold(PRUint32 *piThreshold);

    // High level function that looks for a boolean expression
            SKERR       SearchExpression(const char *pszSearchString,
                                         PRBool bUseFlex,
                                         skIStringSimplifier *pSimp,
                                         SKIRecordFilter *pFilter,
                                         SKIndexResult **ppResult);

    // !!! End of the interface.
            SKERR       ResolveLin(const char *pszToken,
                                   SKIRecordSet **ppResult,
                                   PRBool bUseFlex,
                                   skIStringSimplifier* pSimp,
                                   SKIRecordFilter* pFilter);
            SKERR       BuildOccurrenceList(SKIRecordSet *pLinRS,
                                            PRUint32 *piNext,
                                            PRUint32 iMax,
                                            PRUint32 iDocId,
                                            SKCursor **ppOccList);

    /* The following methods are defined in parse.cpp */
            PRBool      IsSpace(char* pszString, char* pCurrent,
                                char **ppszNext);
            SKERR       Parse_Request(char* pszString,
                                      PRBool bProx,
                                      IndexTokens* pTokens);

            SKERR       IsStopWord(char* pszToken, PRBool* pbResult);

protected:
    virtual SKERR       ConfigureItem(char* pszSection,
                                      char* pszToken, char* pszValue);
            SKERR       Init();


private:
            SKERR       AppendFieldComparator(SKIField *pField);

    /** Merges (OR) iCount cursors into one. The result is stored in
        ppResults[0]. Each cursor should be non-NULL. */
            SKERR       MultiMerge(SKIRecordSet** ppCursors,
                                   PRUint32 iCount);

    skPtr<SKIntegerList>        m_pHardSepList;
    skPtr<SKIntegerList>        m_pSoftSepList;
    skPtr<SKStringList>         m_pSoftSepExcList;

    skPtr<SKStringList>         m_pStopWordList;
    skPtr<SKWildCardWordList>   m_pWordList;
    skPtr<SKIRecordSet>         m_pWordRecordSet;
    skPtr<SKIField>             m_pDocLinkField;
    skPtr<SKIField>             m_pDocIdField;
    skPtr<SKIField>             m_pStructureField;
    skPtr<SKIField>             m_pOccLinkField;
    skPtr<SKIField>             m_pOccField;

    PRUint32                    m_lLinkCount;
    char **                     m_ppszLinkNames;
    char **                     m_ppszSubLinkNames;
    PRBool *                    m_pbMerge;
    char *                      m_pszLinks;

    SKRecordComparatorChain     m_docComparator;

    PRUint32                    m_iNearThreshold;
    PRPackedBool                m_bInitialized;
    PRPackedBool                m_bFileName;
    PRPackedBool                m_bSubResultsStoredSorted;

    static char*                m_pszOperators;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIndex, SK_SKINDEX_IID)

//  SKIndexResult
//  --------------------------------------------------------------------

#define SK_SKINDEXRESULT_IID                                            \
{ 0x930e5e29, 0x1b14, 0x4c45,                                           \
    { 0xab, 0x5b, 0xa5, 0x6f, 0x5e, 0x08, 0x93, 0xce } }

class SKAPI SKIndexResult : public SKRefCount
{
friend class SKIndex;
public:
    SK_REFCOUNT_INTF_DEFAULT(SKIndexResult)
    SK_REFCOUNT_INTF_IID(SKIndexResult, SK_SKINDEXRESULT_IID)

                        SKIndexResult();
    virtual             ~SKIndexResult();

            SKERR GetDocumentList(SKCursor **ppDocumentList);
            SKERR GetHiliteInfo(PRUint32 iDocId, SKCursor **ppHiliteInfo);

            SKERR RunScorer(SKIIndexScorer *pScorer);
protected:
            SKERR DoSearch(SKIndex *pIndex, const char *pszSearchString,
                           PRBool bUseFlex, skIStringSimplifier* pSimp,
                           SKIRecordFilter* pFilter);

private:
    skPtr<SKIndex> m_pIndex;
    char *m_pszSearchString;
    void *m_pTokens;

    skPtr<SKCursor> m_pDocumentList;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIndexResult, SK_SKINDEXRESULT_IID)

#define SK_SKIINDEXSCORER_IID                                           \
{ 0xf1a7c6f3, 0xbd65, 0x4c1a,                                           \
    { 0x8d, 0x21, 0x5c, 0xf6, 0x13, 0x48, 0xba, 0xcf } }

class SKAPI SKIIndexScorer : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIIndexScorer, SK_SKIINDEXSCORER_IID)

    virtual SKERR Init(PRUint32 iCount) = 0;

    // Why do we give...
    //   - iDocId ? Because the application may balance the score with this
    //     information.
    //   - iStructure ? Same reason.
    //   - iOccCount ? Because the balance may not be linear with the count of
    //     occurrences.
    virtual SKERR AddScore(PRUint32 iRank, PRUint32 iDocId, PRUint32 iStructure,
                           PRUint32 iOccCount) = 0;

    virtual SKERR GetCursorScorer(SKCursorScorer **ppCursorScorer) = 0;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIIndexScorer, SK_SKIINDEXSCORER_IID)

//
// error constants
#define err_idx_invalid                 500
#define err_idx_malloc                  501
#define err_idx_fopen                   502

#define err_idx_notfound                505
#define err_idx_empty                   506
#define err_idx_badkey                  507
#define err_idx_nolc                    508
#define err_idx_nodat                   509
#define err_idx_notda                   510
#define err_idx_format                  511

#endif // __INDEX_H_
