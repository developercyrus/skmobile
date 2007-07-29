/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: refcount.cpp,v 1.3.4.4 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Petr Drahovzal
 *          Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#include "nspr/nspr.h"

#include "machine.h"
#include "error.h"
#include "refcount.h"

//============================================================================
// SKRefCount
//============================================================================

SK_REFCOUNT_IMPL_IID_noparent(SKRefCount, SK_SKREFCOUNT_IID);

SKRefCount::SKRefCount()
{
    m_pLock = NULL;
    m_lReferenceCounter = 1;
    m_lWarnCounter = -1;
}

SKRefCount::~SKRefCount()
{
    if (m_pLock != NULL)
        PR_DestroyLock(m_pLock);
}

//----------------------------------------------------------------------------
// Lock
//----------------------------------------------------------------------------

SKERR SKRefCount::Lock()
{
    if (m_pLock == NULL)
    {
        m_pLock = PR_NewLock();
        if (m_pLock == NULL)
            return SKError(err_memory,
                    "[SKRefCount::Lock] " 
                    "Cannot allocate a new PRLock");
    }
    PR_Lock(m_pLock);
    return noErr;
}

//----------------------------------------------------------------------------
// Unlock
//----------------------------------------------------------------------------

SKERR SKRefCount::Unlock()
{
    if (m_pLock == NULL)
        return SKError(err_null,
                "[SKRefCount::Unlock] " 
                "Have never been locked");
    if (PR_Unlock(m_pLock) != PR_SUCCESS)
        return SKError(err_failure,
                "[SKRefCount::Unlock] " 
                "Error while unlocking");
    return noErr;

}

