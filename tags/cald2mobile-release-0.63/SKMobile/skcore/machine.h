/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: machine.h,v 1.6.2.4 2005/02/24 10:01:12 chanoir Exp $
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

#ifndef __SKC_MACHINE_H_
#define __SKC_MACHINE_H_

#include "skcoreconfig.h"

// define UNIX by default
#ifndef WIN32 
    #ifndef __MWERKS__
        #ifndef UNIX
            #define UNIX
        #endif
    #endif
#endif

#ifdef UNIX
	#define FILE_SEP '/'
	#define SKAPI
	#include <assert.h>
    #define SK_ASSERT(e) assert(e)
    #ifndef NULL
        #define NULL 0
    #endif
    #ifdef SKC_DEBUG
        #define SK_REFCOUNT_WIN32
    #else
        #define SK_REFCOUNT_UNIX
    #endif
#endif

#ifdef WIN32
    #pragma warning(disable: 4251)
    #if defined(_DEBUG) && !defined(DEBUG)
        #define DEBUG
    #endif
    #define WINDOWS
    #define FILE_SEP '\\'
//    #include <crtdbg.h>
    #include <assert.h>
    #define SK_ASSERT(e) assert(e)
    #define SKAPI __declspec(dllexport)
    #define SK_REFCOUNT_WIN32
#endif

#ifdef __MWERKS__
	#define MAC
	#define FILE_SEP ':'
	#include <MacMemory.h>
	#include <assert.h>
	#define SK_ASSERT(e) assert(e)
	#define SKAPI
    #ifdef SKC_DEBUG
        #define SK_REFCOUNT_WIN32
    #else
        #define SK_REFCOUNT_UNIX
    #endif
  #ifndef IS_BIG_ENDIAN
    #define IS_BIG_ENDIAN
  #endif
#endif


#ifdef __cplusplus

// Useful refcount macros.
// The suffix _UNIX is used every time it is possible since it is the
// counter-part of _WIN32 which allow to get rid of some WIN32
// particularities:
//   - one heap per DLL/EXE
//   - strange virtual destruction behavior in DLLs
//
// Note: in DEBUG mode the _WIN32 macros are used because they need more
// code than _UNIX.
//
// Use these macros for each class which
//   - inherits from SKRefCount
//   - can be instanciated (has no pure virtual method)
//
// Examples:
//
// // Declaration (.h)
// class Clazz1 // with a default constructor
// {
//     SK_REFCOUNT_INTF_DEFAULT(Clazz1);
//     SK_REFCOUNT_INTF_CREATOR(Clazz1)(int i);
//     clazz1();
//     clazz1(int i);
//     ~Class1();
// }
//
// class Class2 // without a default constructor
// {
//     SK_REFCOUNT_INTF(Clazz2);
//     SK_REFCOUNT_INTF_CREATOR(Clazz2)(int i);
//     clazz2(int i);
//     ~Class2();
// }
//
// // Implementation (.cpp)
// SK_REFCOUNT_IMPL_DEFAULT(Clazz1);
// SK_REFCOUNT_IMPL_CREATOR(Clazz1)(int i)
// {
//     return new Clazz1(i);
// }
// 
// SK_REFCOUNT_IMPL(Clazz2);
// SK_REFCOUNT_IMPL_CREATOR(Clazz2)(int i)
// {
//     return new Clazz2(i);
// }

    #ifdef SK_REFCOUNT_UNIX

        // .h declaration

        #define SK_REFCOUNT_INTF_CREATOR(_class)                        \
            static _class* _CreateInstance

        #define SK_REFCOUNT_INTF(_class)

        #define SK_REFCOUNT_INTF_DEFAULT(_class)                        \
            SK_REFCOUNT_INTF_CREATOR(_class)();                         \
            SK_REFCOUNT_INTF(_class)
        
        // .cpp implementation

        #define SK_REFCOUNT_IMPL_CREATOR(_class)                        \
            _class* _class::_CreateInstance

        #define SK_REFCOUNT_IMPL(_class)

        #define SK_REFCOUNT_IMPL_DEFAULT(_class)                        \
            SK_REFCOUNT_IMPL(_class)
        
        // static .h implementation
        
        #define SK_REFCOUNT_INTF_IMPL_DEFAULT(_class)                   \
            SK_REFCOUNT_INTF_CREATOR(_class)()                          \
                { return new _class; }

        #define sk_CreateInstance(_class)                               \
            new _class

        #define sk_DeleteInstance(p)                                    \
            delete (p);
    #endif // SK_REFCOUNT_UNIX

    #ifdef SK_REFCOUNT_WIN32

        // .h declaration

        #define SK_REFCOUNT_INTF_CREATOR(_class)                        \
            static _class* _CreateInstance

        #define SK_REFCOUNT_INTF(_class)                                \
            virtual void DeleteInstance();

        #define SK_REFCOUNT_INTF_DEFAULT(_class)                        \
            SK_REFCOUNT_INTF_CREATOR(_class)();                         \
            SK_REFCOUNT_INTF(_class)

        // .cpp implementation

        #define SK_REFCOUNT_IMPL_CREATOR(_class)                        \
            _class* _class::_CreateInstance

        #define SK_REFCOUNT_IMPL(_class)                                \
            void _class::DeleteInstance()                               \
            {                                                           \
                delete this;                                            \
            }

        #define SK_REFCOUNT_IMPL_DEFAULT(_class)                        \
            SK_REFCOUNT_IMPL_CREATOR(_class)()                          \
            {                                                           \
                return new _class;                                      \
            }                                                           \
            SK_REFCOUNT_IMPL(_class)

        // static .h implementation

        #define SK_REFCOUNT_INTF_IMPL_DEFAULT(_class)                   \
            SK_REFCOUNT_INTF_CREATOR(_class)()                          \
                { return new _class; }                                  \
            void DeleteInstance() { delete this; }

        #define SK_REFCOUNT_UNIX

        #define sk_CreateInstance(_class)                               \
            _class::_CreateInstance

        #define sk_DeleteInstance(p)                                    \
            (p)->DeleteInstance()
    #endif // SK_REFCOUNT_WIN32

#endif // __cplusplus


#ifndef __cplusplus
 typedef char bool;
  #define true   1
  #define false  0
#endif

#endif // __SKC_MACHINE_H_

