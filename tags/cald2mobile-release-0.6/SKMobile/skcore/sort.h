/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: sort.h,v 1.2.4.4 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_SORT_H_
#define __SKC_SORT_H_

//============================================================================
// SKIComparator
//============================================================================

class SKAPI SKIComparator : public SKRefCount
{
public:
    virtual SKERR Compare(const void *p1, const void *p2, PRInt32 *piResult)=0;
};

//============================================================================
// free funcs
//============================================================================

SKAPI SKERR SKLFindRecord(void* pKey, void* pBase, const PRUint32 lCount,
                          const PRUint32 lSize,
                          SKIComparator* pIComparator, PRUint32* pId,
                          PRBool *pbFound);

SKAPI SKERR SKLFindSortedRecord(void* pKey, void* pBase, const PRUint32 lCount,
                                const PRUint32 lSize,
                                SKIComparator* pIComparator, PRUint32* pId,
                                PRBool *pbFound);

SKAPI SKERR SKQsortRecord(void* pBase, const PRUint32 lCount,
                          const PRUint32 lSize,
                          SKIComparator* pIComparator);

SKAPI SKERR SKQ3sortRecord(void* pBase, const PRUint32 iCount,
                           const PRUint32 iSize,
                           SKIComparator *pComparator);

SKAPI SKERR SKHsortRecord(void* pBase, const PRUint32 iCount,
                          const PRUint32 iSize,
                          SKIComparator* pIComparator);

#ifdef SKC_DEBUG
SKAPI PRBool IsSorted(void* pBase, const PRUint32 lCount, const PRUint32 lSize,
                      SKIComparator* pIComparator);
#endif

#else // __SKC_SORT_H_
#error "Multiple inclusions of sort.h"
#endif // __SKC_SORT_H_

