/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: integerlist.h,v 1.5.4.3 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_INTEGERLIST_H_
#define __SKC_INTEGERLIST_H_

class SKAPI SKIntegerList : public SKFile
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKIntegerList)

    SKIntegerList();
    ~SKIntegerList();

    SKERR SetFileName(const char *pszFileName,
                      const char *pszDefaultFileName = NULL);
    SKERR SetListFromArray(const PRUint32 *piArray, PRUint32 iSize,
                           PRBool bDuplicate);
    SKERR SetListFromUTF8String(const char *pszString);

    SKERR IsPresent(PRUint32 iValue, PRBool *pbResult);

    SKERR FormatToAsciiString(char** ppcResult);
    SKERR SetListFromAsciiString(const char* pcString);

private:
    PRUint32 m_iSize;
    const PRUint32 *m_pIntegerList;

    PRBool m_bOwner;

    PRUint32 *m_piUCS4Data;

    PRBool m_bInitialized;
};

#else // __SKC_INTEGERLIST_H_
#error "Multiple inclusions of integerlist.h"
#endif // __SKC_INTEGERLIST_H_

