/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: table.h,v 1.47.2.5 2005/02/21 14:22:39 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
 *          Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
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

#ifndef __TABLE_H_
#define __TABLE_H_

class SKLCFile;

#define err_tbl_invalid 2101
#define err_tbl_fopen 2102

//---------------------------------------------------------------------------
// SKTable

#define SK_SKTABLE_IID                                                  \
{ 0x67715c46, 0x317b, 0x4ae8,                                           \
    { 0x87, 0xf2, 0xe4, 0x32, 0x08, 0x56, 0x15, 0xba } }

class SKAPI SKTable : public SKRecordSet
{
public:

	static SKERR CreateNativeRecordSet(const char* pszParam, SKRefCount **ppInstance);

    SK_REFCOUNT_INTF_DEFAULT(SKTable)
    SK_REFCOUNT_INTF_IID(SKTable, SK_SKTABLE_IID)

                            SKTable();
                            ~SKTable();
    // configuration --------------------------------------------------------

    virtual SKERR           SetFileName(const char *pszFileName);
    virtual PRBool          IsValid(void) const
            { return m_bIsValid; }

    // subtable -------------------------------------------------------------
    virtual PRUint32 GetCount () const
            { return m_bIsValid ? m_lCount : (PRUint32) -1; }

    virtual SKERR   GetCount (PRUint32 *plCount) const
    { *plCount = GetCount(); return m_bIsValid ? noErr : err_tbl_invalid; }

    virtual PRUint32   GetRecordSize() const
            { return m_bIsValid ?
                     m_pDatFile->GetWeakFldCollection()->GetRecordSize() :
                     0;
            }


    virtual SKERR   GetRecord(PRUint32 id, SKIRecord** ppIRecord);

    virtual SKERR GetFldCollection(SKIFldCollection** col)
            {
                *col = m_pDatFile->GetWeakFldCollection();
                SK_ASSERT(NULL != *col);
                (*col)->AddRef();
                return m_bIsValid   ? noErr : err_tbl_invalid;
            }

    // lookup --------------------------------------------------------------
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
    virtual SKERR   MergeToNew(SKIRecordSet *pRecordSet, skfOperator oper,
                               SKIRecordComparator *pComparator,
                               PRBool bConsiderRank,
                               SKIRecordSet **ppNewRecordSet);
    virtual SKERR   SortToNew(SKIRecordComparator *pComparator,
                              PRBool bConsiderRank,
                              SKIRecordSet **ppNewRecordSet);

    virtual SKERR   GetLookupField(SKIField ** ppField);

    virtual SKERR  ConfigureItem(char* pszToken, char* pszName, char* pszValue);

protected:
            SKERR           Check (void);

private:
            SKERR   SubLookupText(void *pBuffer, PRUint32 iCount,
                                  PRBool bIsLastPage, const char *pszSearch,
                                  skfLookupMode mode, PRUint32 *piId);
            SKERR   SubLookupUNum(void *pBuffer, PRUint32 iCount,
                                  PRUint32 iNum, skfLookupMode mode,
                                  PRUint32 *piId);
            SKERR   SubLookupSNum(void *pBuffer, PRUint32 iCount,
                                  PRInt32 iNum, skfLookupMode mode,
                                  PRUint32 *piId);

            // The DAT file
            skPtr<SKPageFile>   m_pDatFile;

            // type of the key for lookup functions
            short               m_iKeyType;

            // what is the id of the first record (for backward compatibility)
            PRUint32            m_lFirstID;

            // is the table ready to work ?
            PRPackedBool        m_bIsValid;

            // how many records do we have ?
            PRUint32            m_lCount;

            // Search options
            skPtr<SKField>      m_pSearchableField;
            skPtr<SKLCFile>     m_pLcFile;

            SKTextFile          m_File;

            PRBool              m_bIsProtected;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKTable, SK_SKTABLE_IID)

#define SKF_BE_NATIVE_DEFAULT_OPENMODE_ENVIR_VAR \
        "SKF_BE_NATIVE_DEFAULT_OPENMODE"

#define SKF_BE_NATIVE_AUTOMAPLIMIT_ENVIR_VAR \
        "SKF_BE_NATIVE_AUTOMAPLIMIT"


#endif
