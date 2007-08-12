/* BEGIN LICENSE */
/*****************************************************************************
 * SKProd : data compiler platform for sk
 * Copyright (C) 2002-2003 IDM <skcontact @at@ idm .dot. fr>
 * $Id: simplelist.cpp,v 1.1.4.5 2005/02/17 15:29:20 krys Exp $
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

#include <nspr/nspr.h>
#include <nspr/plstr.h>

#include "skbuildconfig.h"

#include "skcoreconfig.h"

#include "machine.h"
#include "error.h"

#include "simplelist.h"

struct t_list_element : public PRCListStr
{
    void* p;
};

#ifdef SK_CONFIG_MEMPROF
PRInt32 skmemprof_iSimpleListsCellsCount = 0;
#endif

skSimpleList::skSimpleList()
{
    m_pList = NULL;
}

skSimpleList::~skSimpleList()
{
    if (m_pList)
    {
        t_list_element* p = (t_list_element*) PR_LIST_HEAD (m_pList);
        while (p!= m_pList)
        {
            t_list_element* pc = (t_list_element*) PR_NEXT_LINK(p);
            PR_Free (p);
#ifdef SK_CONFIG_MEMPROF
            PR_AtomicDecrement(&skmemprof_iSimpleListsCellsCount);
#endif
            p = pc;
        }
        PR_Free (m_pList);
#ifdef SK_CONFIG_MEMPROF
        PR_AtomicDecrement(&skmemprof_iSimpleListsCellsCount);
#endif
    }
}

SKERR skSimpleList::Append (void* p)
{
    if (!m_pList)
    {
        m_pList = (t_list_element*) PR_Malloc (sizeof(t_list_element));
        if (!m_pList)
            return err_memory;

#ifdef SK_CONFIG_MEMPROF
        PR_AtomicIncrement(&skmemprof_iSimpleListsCellsCount);
#endif

        PR_INIT_CLIST (m_pList);
        m_pList->p = NULL;
    }

    t_list_element* Element = (t_list_element*)
        PR_Malloc (sizeof(t_list_element));
#ifdef SK_CONFIG_MEMPROF
    PR_AtomicIncrement(&skmemprof_iSimpleListsCellsCount);
#endif
    if (Element == NULL)
        return err_memory;
    Element->p = p;

    PR_APPEND_LINK (Element, m_pList);

    return noErr;
}

SKERR skSimpleList::AppendTo (skSimpleList* pList)
{
    SK_ASSERT (NULL != pList);
    if (!pList)
        return err_null;

    t_list_element* pE = (t_list_element*) PR_LIST_HEAD(m_pList);
    SKERR err = noErr;

    for ( ; pE != NULL && err == noErr; pE = (t_list_element*) PR_NEXT_LINK(pE))
        err = pList->Append(pE->p);

    return err;
}

SKERR skSimpleList::Prepend (void* p)
{
    if (!m_pList)
    {
        m_pList = (t_list_element*) PR_Malloc (sizeof(t_list_element));
        if (!m_pList)
            return err_memory;
#ifdef SK_CONFIG_MEMPROF
        PR_AtomicIncrement(&skmemprof_iSimpleListsCellsCount);
#endif

        PR_INIT_CLIST (m_pList);
        m_pList->p = NULL;
    }

    t_list_element* Element = (t_list_element*)
        PR_Malloc (sizeof(t_list_element));
#ifdef SK_CONFIG_MEMPROF
    PR_AtomicIncrement(&skmemprof_iSimpleListsCellsCount);
#endif
    if (Element == NULL)
        return err_memory;
    Element->p = p;

    PR_INSERT_LINK (Element, m_pList);

    return noErr;
}

PRUint32 skSimpleList::GetCount()
{
    if (!m_pList)
        return 0;

    PRUint32 iCount = 0;
    t_list_element* p = (t_list_element*) PR_LIST_HEAD(m_pList);

    for ( ; p!= m_pList; p = (t_list_element*) PR_NEXT_LINK(p) )
        iCount++;

    return iCount;
}

SKERR skSimpleList::GetElement (PRUint32 iPosition, void** p)
{
    if (!m_pList)
        return err_null;

    PRUint32 iCount = 0;
    t_list_element* pE = (t_list_element*) PR_LIST_HEAD(m_pList);

    for ( ; iCount++< iPosition; pE = (t_list_element*) PR_NEXT_LINK(pE) )
        if (pE == m_pList)
            return err_invalid;

    *p = pE->p;
    return noErr;
}

SKERR skSimpleList::SetElement (PRUint32 iPosition, void* p)
{
    if (!m_pList)
    {
        m_pList = (t_list_element*) PR_Malloc (sizeof(t_list_element));
#ifdef SK_CONFIG_MEMPROF
        PR_AtomicIncrement(&skmemprof_iSimpleListsCellsCount);
#endif
        if (!m_pList)
            return err_memory;

        PR_INIT_CLIST (m_pList);
        m_pList->p = NULL;
    }

    t_list_element* pE = (t_list_element*) PR_LIST_HEAD(m_pList);

    for(PRUint32 iCount = 0; iCount < iPosition + 1; ++iCount)
    {
        if(pE == m_pList)
            Append(NULL);
        else
            pE = (t_list_element *)PR_NEXT_LINK(pE);
    }
    pE = (t_list_element *)PR_PREV_LINK(pE);

    pE->p = p;

    return noErr;
}


SKERR skSimpleList::RemoveElement (PRUint32 iPosition)
{
    if (!m_pList)
        return err_null;

    PRUint32 iCount = 0;
    t_list_element* pE = (t_list_element*) PR_LIST_HEAD(m_pList);

    for ( ; iCount++< iPosition; pE = (t_list_element*) PR_NEXT_LINK(pE) )
        if (pE == m_pList)
            return err_invalid;

    PR_REMOVE_LINK(pE);
    PR_Free(pE);
#ifdef SK_CONFIG_MEMPROF
    PR_AtomicDecrement(&skmemprof_iSimpleListsCellsCount);
#endif

    return noErr;
}

void skSimpleList::Rewind()
{
    m_pCurrentElement = NULL;
}

void* skSimpleList::Next()
{
    if(!m_pList) return NULL;
    if(!m_pCurrentElement)
        m_pCurrentElement = (t_list_element*)PR_LIST_HEAD(m_pList);
    else
        m_pCurrentElement = (t_list_element*)PR_NEXT_LINK(m_pCurrentElement);
    return m_pCurrentElement->p;
}

#ifndef NDEBUG
void skSimpleList::Dump()
{
    Rewind();
    void* pe = Next();
    if (pe == NULL)
    {
        printf (" void ");
        return;
    }

    printf ("(%p", pe);
    for (pe = Next(); pe!= NULL; pe = Next())
        printf (", %p", pe);

    printf (")");
}
#endif
