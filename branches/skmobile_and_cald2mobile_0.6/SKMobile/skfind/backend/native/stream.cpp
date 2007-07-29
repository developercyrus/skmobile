/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: stream.cpp,v 1.10.2.2 2005/02/21 14:22:39 krys Exp $
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

#include <skfind/skfind.h>

#include "stream.h"
#include "field.h"
#include "record.h"
#include "buf.h"

SK_REFCOUNT_IMPL(SKStream)

SK_REFCOUNT_IMPL_CREATOR(SKStream)(SKPageFile *pFile,
                                   PRUint32 iOffset, PRUint32 iCount)
{
    return new SKStream(pFile, iOffset, iCount);
}

SKStream::SKStream()
{
}

SKStream::SKStream(SKPageFile *pFile, PRUint32 iOffset, PRUint32 iCount)
{
    m_pFile = pFile;
    m_iOffset = iOffset;
    m_iCount = iCount;
    m_iTotalCount = iCount;
}

SKERR SKStream::Read(PRUint32 iCount, void* pDst, PRUint32 *pCount)
{
    if(!m_pFile)
        return SKError(err_failure,"[SKStream::Read] " "Badly configured");

    if(iCount <= m_iCount)
        *pCount = iCount;
    else
        *pCount = m_iCount;

    if(!*pCount)
        return noErr;

    SKERR err = m_pFile->Load(m_iOffset, *pCount);

    if(err != noErr)
        return err;

    memcpy(pDst, m_pFile->GetBufferPtr(), *pCount);

    m_iOffset += *pCount;
    m_iCount -= *pCount;

    return m_pFile->Unload();
}

SKERR SKStream::Seek(PRUint32 iPosition)
{
    SK_ASSERT(iPosition < m_iTotalCount);
    if(iPosition >= m_iTotalCount)
        return err_invalid;

    // update the count field :
    // m_iOffset is an absolute position ; iPosition is relative
    m_iOffset += iPosition + m_iCount - m_iTotalCount;
    m_iCount = m_iTotalCount - iPosition;
    return noErr;
}

