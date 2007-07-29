/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: binary.cpp,v 1.6.2.6 2005/02/17 15:29:20 krys Exp $
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

#include <nspr/nspr.h>
#include <nspr/plstr.h>

#include "skbuildconfig.h"

#include "machine.h"
#include "error.h"
#include "refcount.h"
#include "skptr.h"
#include "binary.h"

#ifdef SK_CONFIG_MEMPROF
PRInt32 skmemprof_iBinaryBytes = 0;
#endif

SK_REFCOUNT_IMPL_DEFAULT(SKBinary)
SK_REFCOUNT_IMPL_CREATOR(SKBinary)(const void* pData, PRUint32 lSize)
{
    return new SKBinary(pData, lSize);
}
SK_REFCOUNT_IMPL_CREATOR(SKBinary)(const SKBinary& binary)
{
    return new SKBinary(binary);
}
SK_REFCOUNT_IMPL_IID(SKBinary, SK_SKBINARY_IID, SKBinary);

// Searches for the last directory separator in the string
static char * SearchDirSep(const char * dir)
{
    // XXX : to be corrected for macintosh
    const char * seps = "/\\";
    char * sep = NULL, *tmp;
    
    while ((tmp = strpbrk(dir, seps)) != NULL)
    {
        sep = tmp;
        dir = tmp + 1;
    }

    return sep;
}

static SKERR MakeDir(const char * dir)
{
	PRFileInfo info;
    if(PR_FAILURE == PR_GetFileInfo(dir, &info))
    {

        if (SearchDirSep(dir) != NULL)
        {
            char * dir2 = PL_strdup(dir);
            if (dir2 == NULL)
                return err_memory;
            *SearchDirSep(dir2) = 0;
            SKERR err = MakeDir(dir2);
            PL_strfree(dir2);
            if (err != noErr)
                return err;
        }
        if (PR_MkDir(dir, 0755) == PR_FAILURE)
            return err_failure;
    }
    return noErr;
}


SKBinary::SKBinary(const SKBinary& binary)
{
    m_pData = NULL;
    m_lSize = 0;
    Realloc(binary.m_lSize);
    if(m_lSize && binary.m_pData && m_pData)
        memcpy(m_pData, binary.m_pData, m_lSize);
}

SKBinary::SKBinary(const void* pData, PRUint32 lSize)
{
    m_pData = NULL;
    m_lSize = 0;
    Realloc(lSize);
    if(m_lSize && pData && m_pData)
        memcpy(m_pData, pData, m_lSize);
}

SKERR SKBinary::Realloc(PRUint32 lSize)
{
    if (lSize == 0)
    {
        Empty();
        return noErr;
    }
    m_pData = PR_Realloc(m_pData, lSize);
#ifdef SK_CONFIG_MEMPROF
    PR_AtomicAdd(&skmemprof_iBinaryBytes, lSize - m_lSize);
#endif
    m_lSize = lSize;
    return (m_pData == NULL);
}

void SKBinary::Empty()
{
    if(m_pData)
    {
        PR_Free(m_pData);
        m_pData = NULL;
#ifdef SK_CONFIG_MEMPROF
        PR_AtomicAdd(&skmemprof_iBinaryBytes, -m_lSize);
#endif
    }
    m_lSize = 0;
}

SKERR SKBinary::GetStringData(char ** ppcData)
{
    if(!m_pData)
        *ppcData = PL_strdup("");
    else
        *ppcData = PL_strndup((char*) m_pData, SKBinary::GetSize());
    return noErr;
}
    
SKERR SKBinary::SetData(const char* pcData, PRUint32 iSize)
{
    if(m_pData)
        PR_Free(m_pData);
    m_pData = PR_Malloc(iSize);
    if(!m_pData)
        return err_memory;
    memcpy(m_pData, pcData, iSize);
#ifdef SK_CONFIG_MEMPROF
    PR_AtomicAdd(&skmemprof_iBinaryBytes, iSize - m_lSize);
#endif
    m_lSize = iSize;
    return noErr;
}

SKERR SKBinary::SetStringData(const char* pcData)
{
    return SetData(pcData, PL_strlen(pcData) + 1);
}

void SKBinary::SetSharedData(void* pData, PRUint32 lSize)
{
    SK_ASSERT(!m_pData);
#ifdef SK_CONFIG_MEMPROF
    PR_AtomicAdd(&skmemprof_iBinaryBytes, lSize);
#endif
    m_pData = pData;
    m_lSize = lSize;
}

SKERR SKBinary::WriteToFile(const char* pszFileName)
{
    // create directory recursively before trying to create the file
    if (SearchDirSep(pszFileName) != NULL)
    {
        char * dir = PL_strdup(pszFileName);
        if (dir == NULL)
            return err_memory;
        *SearchDirSep(dir) = 0;
        SKERR err = MakeDir(dir);
        PL_strfree(dir);
        if (err != noErr)
            return err;
    }

    PRFileDesc * pFile = PR_Open(pszFileName, PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE,0644);
    if (pFile == NULL)
        return err_failure;

    PRInt32 ret =
        PR_Write(pFile, m_pData, m_lSize );
    if (ret != (PRInt32)m_lSize)
    {
        PR_Close(pFile);
        return err_failure;
    }

    PR_Close(pFile);
    return noErr;
}
