/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: integerlist.cpp,v 1.13.2.5 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
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

#include "skcoreconfig.h"
#include "machine.h"
#include "swap.h"
#include "error.h"
#include "refcount.h"
#include "file/skfopen.h"
#include "file/file.h"
#include "integerlist.h"
#include "unichar.h"

static int g_sComparePRUint32(const void *p1, const void *p2)
{
    return *(PRUint32*)p1 - *(PRUint32*)p2;
}

SK_REFCOUNT_IMPL_DEFAULT(SKIntegerList);

SKIntegerList::SKIntegerList()
{
    m_iSize = 0;
    m_pIntegerList = NULL;
    m_bOwner = PR_FALSE;
    m_piUCS4Data = NULL;
    m_bInitialized = PR_FALSE;
}

SKIntegerList::~SKIntegerList()
{
    if(m_bOwner && m_pIntegerList)
        delete[] (PRUint32 *)m_pIntegerList;
    if(m_piUCS4Data)
        UCS4Free(m_piUCS4Data);
}

SKERR SKIntegerList::SetFileName(const char *pszFileName,
                                 const char *pszDefaultFileName)
{
    m_bInitialized = PR_FALSE;

    if(m_bOwner && m_pIntegerList)
    {
        delete[] (PRUint32 *)m_pIntegerList;
        m_pIntegerList = NULL;
    }

    SKERR err = SKFile::SetFileName(pszFileName, pszDefaultFileName);
    if(err != noErr)
        return err;

    PRFileInfo info;
    if(PR_GetFileInfo(GetSharedFileName(), &info) != PR_SUCCESS)
        return err_failure;

    if((info.size & 0x3))
        return err_failure;

    PRFileDesc *f = skPR_Open(GetSharedFileName(), PR_RDONLY, 0);
    if(!f)
        return err_notfound;

    m_bOwner = PR_TRUE;
    m_iSize = info.size / 4;
    m_pIntegerList = new PRUint32[m_iSize];
    if(m_iSize && !m_pIntegerList)
    {
        PR_Close(f);
        return err_memory;
    }

    if(PR_Read(f, (void *)m_pIntegerList, info.size) != info.size)
    {
        PR_Close(f);
        return err_failure;
    }

#if IS_BIG_ENDIAN
    for(PRUint32 i = 0; i < m_iSize; ++i)
    {
        REVERSE_32(((PRUint32*)m_pIntegerList)[i]);
    }
#endif

    PR_Close(f);

    qsort((void *)m_pIntegerList, m_iSize, sizeof(PRUint32), &g_sComparePRUint32);
    m_bInitialized = PR_TRUE;

    return noErr;
}

SKERR SKIntegerList::SetListFromArray(const PRUint32 *piArray, PRUint32 iSize,
                                      PRBool bDuplicate)
{
    m_bInitialized = PR_FALSE;

    if(m_bOwner && m_pIntegerList)
    {
        delete[] (PRUint32 *)m_pIntegerList;
        m_pIntegerList = NULL;
    }

    m_iSize = iSize;

    if(bDuplicate)
    {
        m_bOwner = PR_TRUE;
        m_pIntegerList = new PRUint32[m_iSize];
        if(m_iSize && !m_pIntegerList)
            return err_memory;

        memcpy((void *)m_pIntegerList, piArray, iSize * sizeof(PRUint32));

        qsort((void *)m_pIntegerList, m_iSize, sizeof(PRUint32),
              &g_sComparePRUint32);
    }
    else
    {
        m_bOwner = PR_FALSE;
        m_pIntegerList = piArray;
    }

    m_bInitialized = PR_TRUE;

    return noErr;
}

SKERR SKIntegerList::SetListFromUTF8String(const char *pszString)
{
    SKERR err;

    PRUint32 iSize = UTF8ToNewUCS4(&m_piUCS4Data, pszString);
    if(!m_piUCS4Data)
        return err_memory;

    err = SetListFromArray(m_piUCS4Data, iSize, PR_FALSE);
    if(err != noErr)
        return err;

    qsort((void *)m_pIntegerList, m_iSize, sizeof(PRUint32),
          &g_sComparePRUint32);

    return noErr;
}

SKERR SKIntegerList::IsPresent(PRUint32 iValue, PRBool *pbResult)
{
    *pbResult = PR_FALSE;

    if(!m_bInitialized)
        return err_failure;

    if(!m_iSize)
        return noErr;

    if((m_pIntegerList[0] == iValue) || (m_pIntegerList[m_iSize - 1] == iValue))
    {
        *pbResult = PR_TRUE;
        return noErr;
    }

    PRUint32 iMin = 0;
    PRUint32 iMax = m_iSize - 1;
    while(iMax - iMin > 1)
    {
        PRUint32 iMid = (iMin + iMax) >> 1;
        if(m_pIntegerList[iMid] == iValue)
        {
            *pbResult = PR_TRUE;
            return noErr;
        }
        if(m_pIntegerList[iMid] < iValue)
        {
            iMin = iMid;
        }
        else
        {
            iMax = iMid;
        }
    }

    *pbResult = PR_FALSE;
    return noErr;
}

SKERR SKIntegerList::FormatToAsciiString(char** ppcResult)
{
    * ppcResult = NULL;
    for(PRUint32 i = 0; i<m_iSize; i++)
        *ppcResult = PR_sprintf_append(*ppcResult, "%x ", m_pIntegerList[i]);
    return noErr;
}

SKERR SKIntegerList::SetListFromAsciiString(const char* pcString)
{
    PRUint32 *piTmp = NULL;
    PRUint32 iSize = 0;
    PRUint32 iCount = 0;
    while(*pcString)
    {
        if(*pcString == ' ')
        {
            pcString++;
            continue;
        }
        /* a number to parse */
        if(iSize == iCount)
        {
            iSize = iSize * 2 + 10;
            piTmp = (PRUint32*) PR_Realloc(piTmp, iSize * sizeof(PRUint32));
            if(!piTmp)
                return err_memory;
        }
        char * pcEnd;
        piTmp[iCount++] = strtoul(pcString, &pcEnd, 16);
        pcString = pcEnd;
    }
    SetListFromArray(piTmp, iCount, PR_TRUE);
    PR_Free(piTmp);
    return noErr;
}

