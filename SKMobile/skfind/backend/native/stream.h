/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: stream.h,v 1.5.2.2 2005/02/21 14:22:39 krys Exp $
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

#ifndef __STREAM_H_
#define __STREAM_H_

class SKPageFile;

class SKStream : public SKIStream
{
public:
    SK_REFCOUNT_INTF(SKStream)
    SK_REFCOUNT_INTF_CREATOR(SKStream)(SKPageFile *pFile,
                                       PRUint32 iOffset, PRUint32 iCount);

    SKStream(SKPageFile *pFile, PRUint32 iOffset, PRUint32 iCount);

    virtual SKERR Read(PRUint32 iCount, void* pDst, PRUint32 *pRead);
    virtual SKERR Seek(PRUint32 iPosition);
private:
    SKStream();
    skPtr<SKPageFile> m_pFile;
    PRUint32 m_iOffset;
    PRUint32 m_iCount;
    PRUint32 m_iTotalCount;
};

#else // __STREAM_H_
#error "Multiple inclusions of stream.h"
#endif // __STREAM_H_

