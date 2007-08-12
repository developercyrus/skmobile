/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: refcount.h,v 1.7.4.4 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_REFCOUNT_H_
#define __SKC_REFCOUNT_H_

#ifdef REFCOUNT_DBG
#define INIT_DBG_REFCOUNT               PRInt32 SKRefCount::g_refCount = 0;
#define INC_DBG_REFCOUNT                g_refCount++;
#define DEC_DBG_REFCOUNT                g_refCount--;
#define DECLARE_DBG_REFCOUNT    static  PRInt32 g_refCount;
#define GET_DBG_REFCOUNT                SKRefCount::g_refCount
#else
#define INIT_DBG_REFCOUNT
#define INC_DBG_REFCOUNT
#define DEC_DBG_REFCOUNT
#define DECLARE_DBG_REFCOUNT
#define GET_DBG_REFCOUNT                0
#endif

class SKAPI skID
{
public:
    PRUint32    m0;
    PRUint16    m1;
    PRUint16    m2;
    PRUint8     m3[8];

    PRBool Equals(const skID &id) const
    {
        return (    (m0 == id.m0) && (m1 == id.m1) && (m2 == id.m2)
                 && (*(PRUint32 *)(m3    ) == *(PRUint32 *)(id.m3    ))
                 && (*(PRUint32 *)(m3 + 4) == *(PRUint32 *)(id.m3 + 4))
               );
    }

    void snprintf(char* p, PRUint32 i) const
    {
        // Whether m3[0] and m3[1] need to be swapped depending on the
        // endianness is unclear to me. I don't care anyway. -- krys
        PR_snprintf(p, i, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            m0, m1, m2, m3[0], m3[1], m3[2], m3[3],
            m3[4], m3[5], m3[6], m3[7]);

        // Note: i should always be equal to 37 (32 + 4*'-' + final \0)
    }
};

typedef skID skIID;

template <class T> class skPtr;

#define SK_REFCOUNT_INTF_IID(_class, _iid)                              \
    virtual SKERR QuerySKInterface(const skIID &iid,                    \
                                   void **ppResult);                    \
    virtual const skIID &GetLastSKIID();                                \
    static const skIID &GetSKIID()                                      \
    {                                                                   \
        static const skIID iid = _iid;                                  \
        return iid;                                                     \
    }

#define SK_REFCOUNT_INTF_IMPL_IID(_class, _iid, _parent)                \
    virtual SKERR QuerySKInterface(const skIID &iid,                    \
                                   void **ppResult)                     \
    {                                                                   \
        *ppResult = NULL;                                               \
        skIID _skIID = _iid;                                            \
        if(iid.Equals(_skIID))                                          \
        {                                                               \
            AddRef();                                                   \
            *ppResult = this;                                           \
            return noErr;                                               \
        }                                                               \
        return _parent::QuerySKInterface(iid, ppResult);                \
    }                                                                   \
    virtual const skIID &GetLastSKIID()                                 \
    {                                                                   \
        return GetSKIID();                                              \
    }                                                                   \
    static const skIID &GetSKIID()                                      \
    {                                                                   \
        static const skIID iid = _iid;                                  \
        return iid;                                                     \
    }

#define SK_REFCOUNT_IMPL_IID(_class, _iid, _parent)                     \
    SKERR _class::QuerySKInterface(const skIID &iid,                    \
                                   void **ppResult)                     \
    {                                                                   \
        *ppResult = NULL;                                               \
        skIID _skIID = _iid;                                            \
        if(iid.Equals(_skIID))                                          \
        {                                                               \
            AddRef();                                                   \
            *ppResult = this;                                           \
            return noErr;                                               \
        }                                                               \
        return _parent::QuerySKInterface(iid, ppResult);                \
    }                                                                   \
                                                                        \
    const skIID &_class::GetLastSKIID()                                 \
    {                                                                   \
        return GetSKIID();                                              \
    }

#define SK_REFCOUNT_IMPL_IID_noparent(_class, _iid)                     \
    SKERR _class::QuerySKInterface(const skIID &iid,                    \
                                   void **ppResult)                     \
    {                                                                   \
        *ppResult = NULL;                                               \
        skIID _skIID = _iid;                                            \
        if(iid.Equals(_skIID))                                          \
        {                                                               \
            AddRef();                                                   \
            *ppResult = this;                                           \
            return noErr;                                               \
        }                                                               \
        return err_invalid;                                             \
    }                                                                   \
                                                                        \
    const skIID &_class::GetLastSKIID()                                 \
    {                                                                   \
        return GetSKIID();                                              \
    }

//============================================================================
//      Classe SKRefCount
//============================================================================

#define SK_SKREFCOUNT_IID                                               \
{ 0x7c13ea80, 0x2ea6, 0x4750,                                           \
    { 0xa2, 0x64, 0x7b, 0x84, 0x25, 0x3c, 0xa1, 0x7a } }

class SKAPI SKRefCount
{
public:
DECLARE_DBG_REFCOUNT
#ifdef SK_REFCOUNT_WIN32
    virtual void DeleteInstance() = 0;
#endif
    SK_REFCOUNT_INTF_IID(SKRefCount, SK_SKREFCOUNT_IID);

                        SKRefCount();

    virtual             ~SKRefCount();


            PRInt32     AddRef()
                        {
                            INC_DBG_REFCOUNT
                            return ++m_lReferenceCounter;
                        };

#define _SKRefCount_CheckWarn                                           \
    PRPackedBool bWarn = (m_lReferenceCounter == m_lWarnCounter + 1);
#define _SKRefCount_PreWarn                                             \
    if(bWarn) OnRefPreWarn();
#define _SKRefCount_PostWarn                                            \
    if(bWarn) OnRefPostWarn();

            PRInt32     Release()
                        {
                            SK_ASSERT(m_lReferenceCounter > 0);
                            DEC_DBG_REFCOUNT
                            _SKRefCount_CheckWarn
                            _SKRefCount_PreWarn
                            if (--m_lReferenceCounter > 0)
                            {
                                PRInt32 iReferenceCounter = m_lReferenceCounter;
                                _SKRefCount_PostWarn
                                return iReferenceCounter;
                            }
                            _SKRefCount_PostWarn
                            sk_DeleteInstance(this);
                            return 0;
                        };

            PRInt32     ReleaseAndUnlock()
                        {
                            SK_ASSERT(m_lReferenceCounter > 0);
                            DEC_DBG_REFCOUNT
                            _SKRefCount_CheckWarn
                            _SKRefCount_PreWarn
                            if (--m_lReferenceCounter > 0)
                            {
                                PRInt32 iReferenceCounter = m_lReferenceCounter;
                                _SKRefCount_PostWarn
                                Unlock();
                                return iReferenceCounter;
                            }
                            _SKRefCount_PostWarn
                            Unlock();
                            sk_DeleteInstance(this);
                            return 0;
                        };

#undef _SKRefCount_CheckWarn
#undef _SKRefCount_PreWarn
#undef _SKRefCount_PostWarn

            SKERR       Lock();

            SKERR       Unlock();

protected:
    virtual void        OnRefPreWarn() { };
    virtual void        OnRefPostWarn() { };

    PRLock *    m_pLock;
    PRInt32     m_lReferenceCounter;
    PRInt32     m_lWarnCounter;
};

#endif /* __SKC_REFCOUNT_H_ */ 

