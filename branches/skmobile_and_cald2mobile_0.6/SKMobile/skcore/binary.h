/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: binary.h,v 1.3.2.5 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_BINARY_H_
#define __SKC_BINARY_H_

#define SK_SKBINARY_IID                                                 \
{ 0x140d4618, 0xa539, 0x4241,                                           \
    { 0x93, 0xec, 0x5a, 0x9c, 0xc1, 0x35, 0x74, 0xe3 } }

class SKAPI SKBinary : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKBinary)
    SK_REFCOUNT_INTF_CREATOR(SKBinary)(const void* pData, PRUint32 lSize);
    SK_REFCOUNT_INTF_CREATOR(SKBinary)(const SKBinary& binary);
    SK_REFCOUNT_INTF_IID(SKBinary, SK_SKBINARY_IID);

    SKBinary()
    {
        m_pData = NULL;
        m_lSize = 0;
    }
    SKBinary(const SKBinary& binary);
    SKBinary(const void* pData, PRUint32 lSize);
    ~SKBinary()
    { Empty(); }
    
    /* Binary script interface methods */
    SKERR GetSize(PRUint32* piSize) 
        { *piSize = m_lSize; return noErr; }
    
    SKERR GetStringData(char ** ppcData);

    SKERR GetWStringData(char** ppcData)
    {
        return GetStringData(ppcData);
    }
    
    SKERR SetData(const char* pcData, PRUint32 iSize);
    
    SKERR SetStringData(const char* pcData);
    
    SKERR SetWStringData(const char* pcData)
    {
        return SetStringData(pcData);
    }

    SKERR WriteToFile(const char* pszFileName);

    void*       GetSharedData() const
    {
        return m_pData;
    }
    PRUint32    GetSize() const
    {
        return m_lSize;
    }
    void        SetSharedData( void* pData, PRUint32 lSize);

    SKERR Realloc(PRUint32 lSize);
    void Empty();

protected:
    void*       m_pData;
    PRUint32    m_lSize;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKBinary, SK_SKBINARY_IID);

#else
#error "Multiple inclusions of binary.h"
#endif

