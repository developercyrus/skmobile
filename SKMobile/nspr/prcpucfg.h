     /* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
     
     /* 
      * The contents of this file are subject to the Mozilla Public
      * License Version 1.1 (the "License"); you may not use this file
      * except in compliance with the License. You may obtain a copy of
      * the License at http://www.mozilla.org/MPL/
      * 
      * Software distributed under the License is distributed on an "AS
      * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
      * implied. See the License for the specific language governing
      * rights and limitations under the License.
      * 
      * The Original Code is the Netscape Portable Runtime (NSPR).
      * 
      * The Initial Developer of the Original Code is Netscape
      * Communications Corporation.  Portions created by Netscape are 
      * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
      * Rights Reserved.
      * 
      * Contributor(s):
      *  Garrett Arch Blythe 01/15/2002
      * 
      * Alternatively, the contents of this file may be used under the
      * terms of the GNU General Public License Version 2 or later (the
      * "GPL"), in which case the provisions of the GPL are applicable 
      * instead of those above.  If you wish to allow use of your 
      * version of this file only under the terms of the GPL and not to
      * allow others to use your version of this file under the MPL,
      * indicate your decision by deleting the provisions above and
      * replace them with the notice and other provisions required by
      * the GPL.  If you do not delete the provisions above, a recipient
      * may use your version of this file under either the MPL or the
      * GPL.
      */
     #ifndef nspr_cpucfg___
     #define nspr_cpucfg___
     
     #ifndef XP_PC
     #define XP_PC
     #endif
     
     #ifndef WIN32
     #define WIN32
     #endif
     
     #ifndef WINCE
     #define WINCE
     #endif
     
     /*
      * Some needed types herein.
      */
     #include <windows.h>
     #include <winnt.h>
     #include <stdlib.h>
     
     #define PR_AF_INET6 (100) /* IPv6 not supported yet, use standard value. */
     
     #if defined(_M_IX86) || defined(_X86_)
     
     #define IS_LITTLE_ENDIAN 1
     #undef  IS_BIG_ENDIAN
     
     #define PR_BYTES_PER_BYTE   1
     #define PR_BYTES_PER_SHORT  2
     #define PR_BYTES_PER_INT    4
     #define PR_BYTES_PER_INT64  8
     #define PR_BYTES_PER_LONG   4
     #define PR_BYTES_PER_FLOAT  4
     #define PR_BYTES_PER_WORD   4
     #define PR_BYTES_PER_DWORD  8
     #define PR_BYTES_PER_DOUBLE 8
     
     #define PR_BITS_PER_BYTE    8
     #define PR_BITS_PER_SHORT   16
     #define PR_BITS_PER_INT     32
     #define PR_BITS_PER_INT64   64
     #define PR_BITS_PER_LONG    32
     #define PR_BITS_PER_FLOAT   32
     #define PR_BITS_PER_WORD    32
     #define PR_BITS_PER_DWORD   64
     #define PR_BITS_PER_DOUBLE  64
     
     #define PR_BITS_PER_BYTE_LOG2   3
     #define PR_BITS_PER_SHORT_LOG2  4
     #define PR_BITS_PER_INT_LOG2    5
     #define PR_BITS_PER_INT64_LOG2  6
     #define PR_BITS_PER_LONG_LOG2   5
     #define PR_BITS_PER_FLOAT_LOG2  5
     #define PR_BITS_PER_WORD_LOG2	5
     #define PR_BITS_PER_DWORD_LOG2	6
     #define PR_BITS_PER_DOUBLE_LOG2 6
     
     #define PR_ALIGN_OF_SHORT   2
     #define PR_ALIGN_OF_INT     4
     #define PR_ALIGN_OF_LONG    4
     #define PR_ALIGN_OF_INT64   8
     #define PR_ALIGN_OF_FLOAT   4
     #define PR_ALIGN_OF_WORD    4
     #define PR_ALIGN_OF_DWORD   8
     #define PR_ALIGN_OF_DOUBLE  4
     #define PR_ALIGN_OF_POINTER 4
     
     #define PR_BYTES_PER_WORD_LOG2  2
     #define PR_BYTES_PER_DWORD_LOG2 2
     
     #elif defined(_ARM_)
     
     #define IS_LITTLE_ENDIAN 1
     #undef  IS_BIG_ENDIAN
     
     #define PR_BYTES_PER_BYTE   1
     #define PR_BYTES_PER_SHORT  2
     #define PR_BYTES_PER_INT    4
     #define PR_BYTES_PER_INT64  8
     #define PR_BYTES_PER_LONG   4
     #define PR_BYTES_PER_FLOAT  4
     #define PR_BYTES_PER_WORD   4
     #define PR_BYTES_PER_DWORD  8
     #define PR_BYTES_PER_DOUBLE 8
     
     #define PR_BITS_PER_BYTE    8
     #define PR_BITS_PER_SHORT   16
     #define PR_BITS_PER_INT     32
     #define PR_BITS_PER_INT64   64
     #define PR_BITS_PER_LONG    32
     #define PR_BITS_PER_FLOAT   32
     #define PR_BITS_PER_WORD    32
     #define PR_BITS_PER_DWORD   64
     #define PR_BITS_PER_DOUBLE  64
     
     #define PR_BITS_PER_BYTE_LOG2   3
     #define PR_BITS_PER_SHORT_LOG2  4
     #define PR_BITS_PER_INT_LOG2    5
     #define PR_BITS_PER_INT64_LOG2  6
     #define PR_BITS_PER_LONG_LOG2   5
     #define PR_BITS_PER_FLOAT_LOG2  5
     #define PR_BITS_PER_WORD_LOG2   5
     #define PR_BITS_PER_DWORD_LOG2  6
     #define PR_BITS_PER_DOUBLE_LOG2 6
     
     #define PR_ALIGN_OF_SHORT   2
     #define PR_ALIGN_OF_INT     4
     #define PR_ALIGN_OF_LONG    4
     #define PR_ALIGN_OF_INT64   8
     #define PR_ALIGN_OF_FLOAT   4
     #define PR_ALIGN_OF_WORD    4
     #define PR_ALIGN_OF_DWORD   8
     #define PR_ALIGN_OF_DOUBLE  4
     #define PR_ALIGN_OF_POINTER 4
     
     #define PR_BYTES_PER_WORD_LOG2  2
     #define PR_BYTES_PER_DWORD_LOG2 2
     
     #else /* defined(_M_IX86) || defined(_X86_) */
     
     #error unknown processor architecture
     
     #endif /* defined(_M_IX86) || defined(_X86_) */
     
     #define HAVE_LONG_LONG
     
     #ifndef NO_NSPR_10_SUPPORT
     
     #define BYTES_PER_BYTE      PR_BYTES_PER_BYTE
     #define BYTES_PER_SHORT     PR_BYTES_PER_SHORT
     #define BYTES_PER_INT       PR_BYTES_PER_INT
     #define BYTES_PER_INT64     PR_BYTES_PER_INT64
     #define BYTES_PER_LONG      PR_BYTES_PER_LONG
     #define BYTES_PER_FLOAT     PR_BYTES_PER_FLOAT
     #define BYTES_PER_DOUBLE    PR_BYTES_PER_DOUBLE
     #define BYTES_PER_WORD      PR_BYTES_PER_WORD
     #define BYTES_PER_DWORD     PR_BYTES_PER_DWORD
     
     #define BITS_PER_BYTE       PR_BITS_PER_BYTE
     #define BITS_PER_SHORT      PR_BITS_PER_SHORT
     #define BITS_PER_INT        PR_BITS_PER_INT
     #define BITS_PER_INT64      PR_BITS_PER_INT64
     #define BITS_PER_LONG       PR_BITS_PER_LONG
     #define BITS_PER_FLOAT      PR_BITS_PER_FLOAT
     #define BITS_PER_DOUBLE     PR_BITS_PER_DOUBLE
     #define BITS_PER_WORD       PR_BITS_PER_WORD
     
     #define BITS_PER_BYTE_LOG2  PR_BITS_PER_BYTE_LOG2
     #define BITS_PER_SHORT_LOG2 PR_BITS_PER_SHORT_LOG2
     #define BITS_PER_INT_LOG2   PR_BITS_PER_INT_LOG2
     #define BITS_PER_INT64_LOG2 PR_BITS_PER_INT64_LOG2
     #define BITS_PER_LONG_LOG2  PR_BITS_PER_LONG_LOG2
     #define BITS_PER_FLOAT_LOG2 PR_BITS_PER_FLOAT_LOG2
     #define BITS_PER_DOUBLE_LOG2    PR_BITS_PER_DOUBLE_LOG2
     #define BITS_PER_WORD_LOG2  PR_BITS_PER_WORD_LOG2
     
     #define ALIGN_OF_SHORT      PR_ALIGN_OF_SHORT
     #define ALIGN_OF_INT        PR_ALIGN_OF_INT
     #define ALIGN_OF_LONG       PR_ALIGN_OF_LONG
     #define ALIGN_OF_INT64      PR_ALIGN_OF_INT64
     #define ALIGN_OF_FLOAT      PR_ALIGN_OF_FLOAT
     #define ALIGN_OF_DOUBLE     PR_ALIGN_OF_DOUBLE
     #define ALIGN_OF_POINTER    PR_ALIGN_OF_POINTER
     #define ALIGN_OF_WORD       PR_ALIGN_OF_WORD
     
     #define BYTES_PER_WORD_LOG2     PR_BYTES_PER_WORD_LOG2
     #define BYTES_PER_DWORD_LOG2    PR_BYTES_PER_DWORD_LOG2
     #define WORDS_PER_DWORD_LOG2    PR_WORDS_PER_DWORD_LOG2
     
     #endif /* NO_NSPR_10_SUPPORT */
     
     #endif /* nspr_cpucfg___ */
     /* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
     
     /* 
      * The contents of this file are subject to the Mozilla Public
      * License Version 1.1 (the "License"); you may not use this file
      * except in compliance with the License. You may obtain a copy of
      * the License at http://www.mozilla.org/MPL/
      * 
      * Software distributed under the License is distributed on an "AS
      * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
      * implied. See the License for the specific language governing
      * rights and limitations under the License.
      * 
      * The Original Code is the Netscape Portable Runtime (NSPR).
      * 
      * The Initial Developer of the Original Code is Netscape
      * Communications Corporation.  Portions created by Netscape are 
      * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
      * Rights Reserved.
      * 
      * Contributor(s):
      *  Garrett Arch Blythe 01/15/2002
      * 
      * Alternatively, the contents of this file may be used under the
      * terms of the GNU General Public License Version 2 or later (the
      * "GPL"), in which case the provisions of the GPL are applicable 
      * instead of those above.  If you wish to allow use of your 
      * version of this file only under the terms of the GPL and not to
      * allow others to use your version of this file under the MPL,
      * indicate your decision by deleting the provisions above and
      * replace them with the notice and other provisions required by
      * the GPL.  If you do not delete the provisions above, a recipient
      * may use your version of this file under either the MPL or the
      * GPL.
      */
     #ifndef nspr_cpucfg___
     #define nspr_cpucfg___
     
     #ifndef XP_PC
     #define XP_PC
     #endif
     
     #ifndef WIN32
     #define WIN32
     #endif
     
     #ifndef WINCE
     #define WINCE
     #endif
     
     /*
      * Some needed types herein.
      */
     #include <windows.h>
     #include <winnt.h>
     #include <stdlib.h>
     
     #define PR_AF_INET6 (100) /* IPv6 not supported yet, use standard value. */
     
     #if defined(_M_IX86) || defined(_X86_)
     
     #define IS_LITTLE_ENDIAN 1
     #undef  IS_BIG_ENDIAN
     
     #define PR_BYTES_PER_BYTE   1
     #define PR_BYTES_PER_SHORT  2
     #define PR_BYTES_PER_INT    4
     #define PR_BYTES_PER_INT64  8
     #define PR_BYTES_PER_LONG   4
     #define PR_BYTES_PER_FLOAT  4
     #define PR_BYTES_PER_WORD   4
     #define PR_BYTES_PER_DWORD  8
     #define PR_BYTES_PER_DOUBLE 8
     
     #define PR_BITS_PER_BYTE    8
     #define PR_BITS_PER_SHORT   16
     #define PR_BITS_PER_INT     32
     #define PR_BITS_PER_INT64   64
     #define PR_BITS_PER_LONG    32
     #define PR_BITS_PER_FLOAT   32
     #define PR_BITS_PER_WORD    32
     #define PR_BITS_PER_DWORD   64
     #define PR_BITS_PER_DOUBLE  64
     
     #define PR_BITS_PER_BYTE_LOG2   3
     #define PR_BITS_PER_SHORT_LOG2  4
     #define PR_BITS_PER_INT_LOG2    5
     #define PR_BITS_PER_INT64_LOG2  6
     #define PR_BITS_PER_LONG_LOG2   5
     #define PR_BITS_PER_FLOAT_LOG2  5
     #define PR_BITS_PER_WORD_LOG2	5
     #define PR_BITS_PER_DWORD_LOG2	6
     #define PR_BITS_PER_DOUBLE_LOG2 6
     
     #define PR_ALIGN_OF_SHORT   2
     #define PR_ALIGN_OF_INT     4
     #define PR_ALIGN_OF_LONG    4
     #define PR_ALIGN_OF_INT64   8
     #define PR_ALIGN_OF_FLOAT   4
     #define PR_ALIGN_OF_WORD    4
     #define PR_ALIGN_OF_DWORD   8
     #define PR_ALIGN_OF_DOUBLE  4
     #define PR_ALIGN_OF_POINTER 4
     
     #define PR_BYTES_PER_WORD_LOG2  2
     #define PR_BYTES_PER_DWORD_LOG2 2
     
     #elif defined(_ARM_)
     
     #define IS_LITTLE_ENDIAN 1
     #undef  IS_BIG_ENDIAN
     
     #define PR_BYTES_PER_BYTE   1
     #define PR_BYTES_PER_SHORT  2
     #define PR_BYTES_PER_INT    4
     #define PR_BYTES_PER_INT64  8
     #define PR_BYTES_PER_LONG   4
     #define PR_BYTES_PER_FLOAT  4
     #define PR_BYTES_PER_WORD   4
     #define PR_BYTES_PER_DWORD  8
     #define PR_BYTES_PER_DOUBLE 8
     
     #define PR_BITS_PER_BYTE    8
     #define PR_BITS_PER_SHORT   16
     #define PR_BITS_PER_INT     32
     #define PR_BITS_PER_INT64   64
     #define PR_BITS_PER_LONG    32
     #define PR_BITS_PER_FLOAT   32
     #define PR_BITS_PER_WORD    32
     #define PR_BITS_PER_DWORD   64
     #define PR_BITS_PER_DOUBLE  64
     
     #define PR_BITS_PER_BYTE_LOG2   3
     #define PR_BITS_PER_SHORT_LOG2  4
     #define PR_BITS_PER_INT_LOG2    5
     #define PR_BITS_PER_INT64_LOG2  6
     #define PR_BITS_PER_LONG_LOG2   5
     #define PR_BITS_PER_FLOAT_LOG2  5
     #define PR_BITS_PER_WORD_LOG2   5
     #define PR_BITS_PER_DWORD_LOG2  6
     #define PR_BITS_PER_DOUBLE_LOG2 6
     
     #define PR_ALIGN_OF_SHORT   2
     #define PR_ALIGN_OF_INT     4
     #define PR_ALIGN_OF_LONG    4
     #define PR_ALIGN_OF_INT64   8
     #define PR_ALIGN_OF_FLOAT   4
     #define PR_ALIGN_OF_WORD    4
     #define PR_ALIGN_OF_DWORD   8
     #define PR_ALIGN_OF_DOUBLE  4
     #define PR_ALIGN_OF_POINTER 4
     
     #define PR_BYTES_PER_WORD_LOG2  2
     #define PR_BYTES_PER_DWORD_LOG2 2
     
     #else /* defined(_M_IX86) || defined(_X86_) */
     
     #error unknown processor architecture
     
     #endif /* defined(_M_IX86) || defined(_X86_) */
     
     #define HAVE_LONG_LONG
     
     #ifndef NO_NSPR_10_SUPPORT
     
     #define BYTES_PER_BYTE      PR_BYTES_PER_BYTE
     #define BYTES_PER_SHORT     PR_BYTES_PER_SHORT
     #define BYTES_PER_INT       PR_BYTES_PER_INT
     #define BYTES_PER_INT64     PR_BYTES_PER_INT64
     #define BYTES_PER_LONG      PR_BYTES_PER_LONG
     #define BYTES_PER_FLOAT     PR_BYTES_PER_FLOAT
     #define BYTES_PER_DOUBLE    PR_BYTES_PER_DOUBLE
     #define BYTES_PER_WORD      PR_BYTES_PER_WORD
     #define BYTES_PER_DWORD     PR_BYTES_PER_DWORD
     
     #define BITS_PER_BYTE       PR_BITS_PER_BYTE
     #define BITS_PER_SHORT      PR_BITS_PER_SHORT
     #define BITS_PER_INT        PR_BITS_PER_INT
     #define BITS_PER_INT64      PR_BITS_PER_INT64
     #define BITS_PER_LONG       PR_BITS_PER_LONG
     #define BITS_PER_FLOAT      PR_BITS_PER_FLOAT
     #define BITS_PER_DOUBLE     PR_BITS_PER_DOUBLE
     #define BITS_PER_WORD       PR_BITS_PER_WORD
     
     #define BITS_PER_BYTE_LOG2  PR_BITS_PER_BYTE_LOG2
     #define BITS_PER_SHORT_LOG2 PR_BITS_PER_SHORT_LOG2
     #define BITS_PER_INT_LOG2   PR_BITS_PER_INT_LOG2
     #define BITS_PER_INT64_LOG2 PR_BITS_PER_INT64_LOG2
     #define BITS_PER_LONG_LOG2  PR_BITS_PER_LONG_LOG2
     #define BITS_PER_FLOAT_LOG2 PR_BITS_PER_FLOAT_LOG2
     #define BITS_PER_DOUBLE_LOG2    PR_BITS_PER_DOUBLE_LOG2
     #define BITS_PER_WORD_LOG2  PR_BITS_PER_WORD_LOG2
     
     #define ALIGN_OF_SHORT      PR_ALIGN_OF_SHORT
     #define ALIGN_OF_INT        PR_ALIGN_OF_INT
     #define ALIGN_OF_LONG       PR_ALIGN_OF_LONG
     #define ALIGN_OF_INT64      PR_ALIGN_OF_INT64
     #define ALIGN_OF_FLOAT      PR_ALIGN_OF_FLOAT
     #define ALIGN_OF_DOUBLE     PR_ALIGN_OF_DOUBLE
     #define ALIGN_OF_POINTER    PR_ALIGN_OF_POINTER
     #define ALIGN_OF_WORD       PR_ALIGN_OF_WORD
     
     #define BYTES_PER_WORD_LOG2     PR_BYTES_PER_WORD_LOG2
     #define BYTES_PER_DWORD_LOG2    PR_BYTES_PER_DWORD_LOG2
     #define WORDS_PER_DWORD_LOG2    PR_WORDS_PER_DWORD_LOG2
     
     #endif /* NO_NSPR_10_SUPPORT */
     
     #endif /* nspr_cpucfg___ */
