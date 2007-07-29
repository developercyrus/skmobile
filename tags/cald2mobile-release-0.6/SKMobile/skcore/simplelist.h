/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 2002-2003 IDM <skcontact @at@ idm .dot. fr>
 * $Id: simplelist.h,v 1.2.4.2 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_SIMPLELIST_H_
#define __SKC_SIMPLELIST_H_

struct t_list_element;

/** A basic linear list that stores \c void* objects. The list
    does not free its contained elements when the destructor is called.
    If the list stores reference-counted objects, the counters for
    these objects must be updated manually when constructing and
    destructing the list. */
class SKAPI skSimpleList
{
public:
     skSimpleList();
    ~skSimpleList();

    /** Adds an element \c p at the end of the list. */
    SKERR Append (void* p);

    /** Appends each element of the list to the list specified by \c pList. */
    SKERR AppendTo (skSimpleList* pList);

    /** Adds an element \c p at the beginning of the list.*/
    SKERR Prepend (void* p);

    /** Returns the size of the list. */
    PRUint32 GetCount();

    /** Returns the iPosition-th element of the list. Do not use this method
        if performances matter since a linear search is done here.
        This function does not modify the internal list pointer. */
    SKERR GetElement (PRUint32 iPosition, void** p);

    /** Sets the iPosition-th element of the list. Tabs with NULL elements
        if needed. Does not change the internal pointer. */
    SKERR SetElement (PRUint32 iPosition, void* p);

    /** Removes the iPosition-th element of the list. May corrupt the internal
        pointer. */
    SKERR RemoveElement (PRUint32 iPosition);

    /** Resets the internal list pointer. Use this in conjunction with
        \c Next(). */
    void Rewind();

    /** Gets the current element and increment the internal list pointer. */
    void* Next();

#ifndef NDEBUG
    /** Prints the address of each element :
        <CODE> (0x04000, 0x05000...) </CODE> */
    void Dump();
#endif

protected:
    t_list_element* m_pList;
    t_list_element* m_pCurrentElement;
};

#else // __SKC_SIMPLELIST_H_
#error "Multiple inclusions of simplelist.h"
#endif // __SKC_SIMPLELIST_H_
