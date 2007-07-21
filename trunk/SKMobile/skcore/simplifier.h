/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: simplifier.h,v 1.3.2.4 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_SIMPLIFIER_H_
#define __SKC_SIMPLIFIER_H_

#define SK_SKISTRINGSIMPLIFIER_IID                                      \
{ 0x0c387d53, 0x6fd1, 0x4456,                                           \
    { 0x9c, 0x7c, 0xc2, 0x47, 0xa1, 0x79, 0x70, 0x83 } }

class SKAPI skIStringSimplifier : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(skIStringSimplifier, SK_SKISTRINGSIMPLIFIER_IID)

    skIStringSimplifier() : m_pcIdentity(NULL) {};
    virtual ~skIStringSimplifier();
    
    virtual SKERR SimplifyFirstChar(
            const char* pcIn, PRUint32 *piRead,
            char* pcOut, PRUint32* iWritten) = 0;

    virtual void ComputeIdentity() = 0;

    virtual char* GetSharedIdentity()
    {
        if(!m_pcIdentity)
            ComputeIdentity();
        return m_pcIdentity;
    }
 
    virtual SKERR CompareFirstChar(
                            const char* pc1, PRUint32* plLen1,
                            const char* pc2, PRUint32* plLen2,
                            PRInt32* iCmp = NULL);

    virtual SKERR Compare(  const char* pc1, PRUint32* plLen1,
                            const char* pc2, PRUint32* plLen2,
                            PRInt32* iCmp = NULL);

    virtual SKERR SimplifyToNew(const char* pcIn, char** pcOut, 
                                PRUint32 *plSize = NULL);
    virtual SKERR SimplifyRealloc(char** pc, PRUint32* plSize = NULL);
    virtual SKERR SimplifyOutputRealloc(const char* pcIn,
                                        char** ppcOut,
                                        PRUint32* plSize);

    // For wrapping
    virtual SKERR Simplify(const char* pcIn, char** pcOut)
    {
        return SimplifyToNew(pcIn, pcOut);
    }
    
protected:
    char* m_pcIdentity;
};

SK_REFCOUNT_DECLARE_INTERFACE(skIStringSimplifier, SK_SKISTRINGSIMPLIFIER_IID)

#endif // __SKC_SIMPLIFIER_H_

