/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: skptr.h,v 1.8.4.3 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_SKPTR_H_
#define __SKC_SKPTR_H_

#define err_skptr_invalid    101

template <class T>
inline T* skPtrAssign(T** pp, T* lp)
{
    // XXX This code is very critical when there are reference loops because
    // XXX the call to the Release() method may re-enter this function.

    // 1/ Increment the reference counter of the new pointer
    if (lp != NULL)
        lp->AddRef();

    // 2/ Replace the pointer with the new one because it should be released
    // if the function is re-entered. The old pointer is saved so that we can
    // release it after.
    T* oldp = *pp;
    *pp = lp;

    // 3/ Release the old pointer _safely_.
    if (oldp)
        oldp->Release();

    return lp;
}

template <class T>
class _NoAddRefReleaseOnskPtr : public T
{
private:
    virtual    long AddRef()=0;
    virtual    long Release()=0;
};

template <class T>
class skPtr
{
public:
    typedef T _PtrClass;
    skPtr()
    {
        p=NULL;
    }
    skPtr(T* lp)
    {
        if ((p = lp) != NULL)
            p->AddRef();
    }
    skPtr(const skPtr<T>& lp)
    {
        if ((p = lp.p) != NULL)
            p->AddRef();
    }
    ~skPtr()
    {
        SKRefCount* pTemp = p;
        if (pTemp)
        {
            p = NULL;
            pTemp->Release();
        }
    }
    void Release()
    {
        SKRefCount* pTemp = p;
        if (pTemp)
        {
            p = NULL;
            pTemp->Release();
        }
    }
    operator T*() const
    {
        return (T*)p;
    }
    T& operator*() const
    {
        SK_ASSERT(p!=NULL);
        return *p;
    }
    // XXX This operator is an ** ATOMIC BOMB **. If you want to prevent
    // XXX the world from exploding then DON'T USE IT.
    // XXX -- bozo
    //The SK_ASSERT on operator& usually indicates a bug.  If this is really
    //what is needed, however, take the address of the p member explicitly.
/*    T** operator&()
    {
        SK_ASSERT(p==NULL);
        return &p;
    }*/
    _NoAddRefReleaseOnskPtr<T>* operator->() const
    {
        SK_ASSERT(p!=NULL);
        return (_NoAddRefReleaseOnskPtr<T>*)p;
    }
    inline T* AssignSafeIID(SKRefCount *lp);
    T* operator=(T* lp)
    {
        return (T*)skPtrAssign(&p, lp);
    }
    T* operator=(const skPtr<T>& lp)
    {
        return (T*)skPtrAssign(&p, lp.p);
    }
    bool operator!() const
    {
        return (p == NULL);
    }
    bool operator<(T* pT) const
    {
        return p < pT;
    }
    bool operator==(T* pT) const
    {
        return p == pT;
    }
    void Attach(T* p2)
    {
        if (p)
            p->Release();
        p = p2;
    }
    T* Detach()
    {
        T* pt = p;
        p = NULL;
        return pt;
    }
    SKERR CopyTo(T** ppT)
    {
        if (ppT == NULL)
            return err_skptr_invalid;
        *ppT = p;
        if (p)
            p->AddRef();
        return 0;
    }
    T** already_AddRefed()
    {
        Release();
        return &p;
    }

    T* p;
};

class skAutoLock
{
public:
    skAutoLock(SKRefCount *pObject, PRBool bNoLock = PR_FALSE)
    {
        m_bNoLock = bNoLock;
        if(pObject)
        {
            if(!m_bNoLock)
                pObject->Lock();
            m_pObject = pObject;
        }
    }
    ~skAutoLock()
    {
        if(m_pObject && !m_bNoLock)
        {
            SKRefCount *pObject = m_pObject;
            pObject->AddRef();
            m_pObject = NULL;
            pObject->ReleaseAndUnlock();
        }
    }

private:
    skPtr<SKRefCount> m_pObject;
    PRPackedBool m_bNoLock;
};

#define SK_REFCOUNT_DECLARE_INTERFACE(_class, _iid)                         \
    static const skIID &_class##_GetSKIID()                                 \
    {                                                                       \
        static const skIID iid = _iid;                                      \
        return iid;                                                         \
    }                                                                       \
    template <> inline _class* skPtr<_class>::AssignSafeIID(SKRefCount *lp) \
    {                                                                       \
        if(!lp)                                                             \
            return (_class*)skPtrAssign(&p, (_class*)lp);                   \
        Release();                                                          \
        lp->QuerySKInterface(_class##_GetSKIID(), (void **)&p);             \
        return p;                                                           \
    }

SK_REFCOUNT_DECLARE_INTERFACE(SKRefCount, SK_SKREFCOUNT_IID);

#endif /* __SKC_SKPTR_H_ */

