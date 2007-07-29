/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: unicodesimplifier.h,v 1.6.2.2 2005/02/17 15:29:24 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#ifndef __SKC_UNICODESIMPLIFIER_H_
#define __SKC_UNICODESIMPLIFIER_H_

class skStringUnicodeSimplifier : public skIStringSimplifier
{
public:

	static SKERR CreateUnicodeSimplifier(const char* pszParam, SKRefCount** ppInstance);

    SK_REFCOUNT_INTF(skStringUnicodeSimplifier);
    SK_REFCOUNT_INTF_CREATOR(skStringUnicodeSimplifier)(const char* pszParam);

    skStringUnicodeSimplifier(const char* pszParam);
    virtual ~skStringUnicodeSimplifier();

    virtual SKERR SimplifyFirstChar(
            const char* pcIn, PRUint32 *piRead,
            char* pcOut, PRUint32* piWritten);

    virtual SKERR CompareFirstChar(
            const char* pc1, PRUint32* plLen1,
            const char* pc2, PRUint32* plLen2,
            PRInt32* iCmp = NULL);

    virtual void ComputeIdentity();

private:
    PRUint32 SimpCharUniCase(PRUint32 iChar);
    PRUint32 SimpCharUniComp(PRUint32 iChar);
    PRUint32 SimpCharUniSpace(PRUint32 iChar);
protected:
//    static PRInt32 CompareChar(PRUint32 iFlags, PRUint32 c1, PRUint32 c2);

    char* m_pszParam;

    PRBool  m_bSimpUniCase;
    PRBool  m_bSimpUniComp;
    PRBool  m_bSimpUniSpaces;
    
    PRBool  m_bFrSloppyUppercaseMatch;

    typedef struct Mapping 
    {
        PRUint32 lChar;
        char*    pcString;
    } Mapping;

    PRUint32 m_iMapSize;
    PRUint32 m_iMapCount;
    Mapping* m_pMap;
};

#endif // __SKC_UNICODESIMPLIFIER_H_

