/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: stringlist.h,v 1.3.4.3 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Christopher Gautier <krys @at@ idm .dot. fr>
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

#ifndef __SKC_STRINGLIST_H_
#define __SKC_STRINGLIST_H_

class SKAPI SKStringList : public SKFile
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKStringList)

    SKStringList();
    ~SKStringList();

    PRUint32 GetCount();
    SKERR SetFileName(const char *pszFileName,
                      const char *pszDefaultFileName = NULL);
    SKERR SetListWithOneWord(const char *pszWord);

    SKERR IsPresent(char* pszUTF8Word, PRBool *pbResult);

private:
    PRUint32 m_iNbWords;
    char*    m_pszWordString;
    char**   m_pWordList;

    PRBool m_bInitialized;
};

#else // __SKC_STRINGLIST_H_
#error "Multiple inclusions of stringlist.h"
#endif // __SKC_STRINGLIST_H_

