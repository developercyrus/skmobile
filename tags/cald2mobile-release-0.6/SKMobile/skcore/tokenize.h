/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: tokenize.h,v 1.2.2.2 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_TOKENIZE_H_
#define __SKC_TOKENIZE_H_

class SKAPI skTokenize : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_DEFAULT(skTokenize)

    skTokenize();
    ~skTokenize();

    virtual SKERR   Init(   SKIntegerList* pHardSepList,
                            SKIntegerList* pSoftSepList,
                            SKStringList* pSoftSepExcList,
                            PRBool bEntityAware = PR_FALSE);

    virtual SKERR   NextEndOfSpace( PRUint32* piUCS4String, 
                                    PRUint32 iMaxLength, 
                                    PRUint32 *piResult);

    virtual SKERR   NextEndOfWord(  PRUint32* piUCS4String, 
                                    PRUint32 iMaxLength, 
                                    PRUint32 *piResult);
protected:
    skPtr<SKIntegerList>        m_pHardSepList;
    skPtr<SKIntegerList>        m_pSoftSepList;
    skPtr<SKStringList>         m_pSoftSepExcList;
    PRBool                      m_bInitialized;
    PRBool                      m_bEntityAware;
};

#else // __SKC_TOKENIZE_H_
#error "Multiple inclusions of tokenize.h"
#endif // __SKC_TOKENIZE_H_

