/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: cursor.h,v 1.37.2.7 2005/03/04 10:20:54 kali Exp $
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

#ifndef __CURSOR_H_
#define __CURSOR_H_

class SKAPI SKICursorFilter;
class SKAPI SKICursorComparator;

// SKCursor

#define SK_SKCURSOR_IID                                                 \
{ 0x793499cf, 0x6a27, 0x47d0,                                           \
    { 0xa8, 0x5d, 0x3b, 0x7b, 0xb9, 0x2c, 0x3e, 0x0c } }

class SKAPI SKCursor : public SKRefCount
{
public:
    SK_REFCOUNT_INTF(SKCursor)
    SK_REFCOUNT_INTF_CREATOR(SKCursor)(PRUint32 iCount = 0,
                                       const PRUint32* pData = NULL);
    SK_REFCOUNT_INTF_CREATOR(SKCursor)(PRUint32 iCount,
                                       const PRUint32* pData,
                                       PRBool bIsBitmap);
    SK_REFCOUNT_INTF_IID(SKCursor, SK_SKCURSOR_IID);

    SKCursor(PRUint32 iCount = 0, const PRUint32* pData = NULL);
    SKCursor(PRUint32 iCount, const PRUint32* pData, PRBool bIsBitmap);
    ~SKCursor();

    SKERR InitStartCount(PRUint32 iStart, PRUint32 iCount);

    SKERR GetCount(PRUint32* pCount)
    {
        SKERR err = ComputeCursorForm();
        if (err != noErr)
            return err;

        *pCount = m_iCount;
        return noErr;
    }
#if 0    
    SKERR SetCount(PRUint32 iCount)
    {
        if(iCount > m_iCount)
            return err_failure;
        m_iCount = iCount;
        return noErr;
    }
#endif
    SKERR GetElement(PRUint32 iRank, PRUint32* piValue)
    {
        if(!piValue)
            return err_invalid;
        if(iRank > m_iCount || !m_pData)
            return err_failure;
        if (m_iCount == 0)
        {
            SKERR err = ComputeCursorForm();
            if (err != noErr)
                return err;
        }
        *piValue = m_pData[iRank];
        return noErr;
    }
    
    SKERR SetElement(PRUint32 iRank, PRUint32 iValue);
    
    SKERR Lookup(PRUint32 iValue, PRBool* pbFound);

    SKERR GetRecord(SKIRecordSet* pRecordSet, PRUint32 iId,
                    PRBool bNoLookup, SKIRecord** ppRecord);

    SKERR Filter(SKICursorFilter* pFilter);

    SKERR Sort(SKICursorComparator* pComparator);

    SKERR Merge(SKCursor* pCursor, skfOperator oper);

    SKERR MuxCursors(SKCursor** ppCursors, PRUint32 iCount,
                     PRBool *pbInterrupt);

    SKERR Clone(SKCursor** ppClone);

    SKERR Extract(PRUint32 iOffset, PRUint32 iCount, SKCursor** ppClone);
#ifdef DEBUG
    SKERR Dump();
#endif
    SKERR ComputeBitmapForm();

    SKERR ComputeCursorForm();

    SKERR DestroyBitmapForm();

    SKERR DestroyCursorForm();

    const PRUint32* GetSharedCursorDataRead()
    {
        return m_pData;
    }
    PRUint32* GetSharedCursorDataWrite()
    {
        m_bCursorCanChange = PR_TRUE;
        return m_pData;
    }
    void ReleaseSharedCursorDataWrite()
    {
        DestroyBitmapForm();
        m_bCursorCanChange = PR_FALSE;
    }
private:
    SKERR SetBitmapForm(PRUint32 iDataBitmapSize,
                        const PRUint32* piDataBitmap);
    SKERR SetCursorForm(PRUint32 iDataCount,
                        const PRUint32* piData);
    SKERR GrowBitmap(PRUint32 iDataBitmapBitSize);
    SKERR CleanBitmap();

    PRUint32                    m_iCount;
    PRUint32*                   m_pData;
    PRUint32                    m_iDataBitmapLongSize;
    PRUint32*                   m_pDataBitmap;
    PRBool                      m_bCursorCanChange;
    PRBool                      m_bMustBeSorted;
    skPtr<SKICursorComparator>  m_pCursorComparator;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKCursor, SK_SKCURSOR_IID)

SKERR SKAPI skResolveRSUNum(SKCursor *pFrom, SKIRecordSet *pRS,
                            SKIField *pField, PRBool bFieldSorted,
                            PRBool *pbInterrupt, SKCursor **ppTo);

SKERR SKAPI skResolveRSLink(SKCursor *pFrom, SKIRecordSet *pRS,
                            SKIField *pField, SKIField *pSubField,
                            PRBool bSubFieldSorted, PRBool *pbInterrupt,
                            SKCursor **ppTo);

#else
#error "Multiple inclusions of cursor.h"
#endif

