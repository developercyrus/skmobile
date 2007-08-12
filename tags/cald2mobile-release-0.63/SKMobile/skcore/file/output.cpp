/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: output.cpp,v 1.15.2.2 2005/02/17 15:29:21 krys Exp $
 *
 * Authors: W.P. Dauchy
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

#include <nspr/nspr.h>
#include <nspr/plstr.h>

#include "../skcoreconfig.h"
#include "../machine.h"
#include "../error.h"
#include "../refcount.h"
#include "../skptr.h"
#include "../envir/envir.h"

#include "skfopen.h"
#include "output.h"

#define DEFAULT_BUFFER_SIZE 8

SK_REFCOUNT_IMPL_DEFAULT(SKFileOutput)

//  --------------------------------------------------------------------------
//
//    SKFileOutput
//
//  --------------------------------------------------------------------------
SKFileOutput::SKFileOutput()
{
    m_pszFileName = NULL;
    m_pFD = NULL;
    m_pcBuffer = NULL;
    m_iBufferSize = 0;
    m_iDeadLength = 0;
    m_iWritten = 0;
}

SKFileOutput::~SKFileOutput()
{
    if(m_pFD)
        Close();
    if(m_pszFileName)
        PL_strfree(m_pszFileName);
}

SKERR SKFileOutput::Create(const char* pcFileName)
{
    return Open(pcFileName, PR_CREATE_FILE|PR_WRONLY|PR_TRUNCATE);
}

SKERR SKFileOutput::Append(const char* pcFileName)
{
    return Open(pcFileName, PR_WRONLY|PR_APPEND);
}

SKERR SKFileOutput::Open(const char* pcFileName, PRUint32 lFlags)
{
    if(m_pFD)
        Close();
    SK_ASSERT(NULL != pcFileName);
    m_pszFileName = PL_strdup(pcFileName);
    m_pFD = PR_Open(pcFileName, lFlags, 0644);
    if(!m_pFD)
        return err_failure;

    m_pcBuffer = (char*) PR_Malloc(DEFAULT_BUFFER_SIZE*sizeof(char));
    if(m_pcBuffer == 0)
        return err_memory;

   m_iBufferSize = DEFAULT_BUFFER_SIZE;

   return noErr;
}

SKERR SKFileOutput::Write(const char* pcBuffer, PRUint32 iSize)
{
    if(m_iDeadLength + iSize <= m_iBufferSize)
    {
        memmove(m_pcBuffer+m_iDeadLength, pcBuffer, iSize);
        m_iDeadLength += iSize;
        m_iWritten += iSize;
        return noErr;
    }
    if(iSize <= m_iBufferSize)
    {
        SKERR err = Flush();
        if(err != noErr)
            return err;
        return Write(pcBuffer, iSize);
    }
    m_pcBuffer = (char*) PR_Realloc(m_pcBuffer, iSize);
    m_iBufferSize = iSize;
    if(!m_pcBuffer)
        return err_memory;
    return Write(pcBuffer, iSize);
}
    
SKERR SKFileOutput::Printf(const char* pcFormat, ...)
{
    SKERR err;
    va_list ap;
    va_start(ap, pcFormat);
    err = vPrintf(pcFormat, ap);
    va_end(ap);
    return err;
}

SKERR SKFileOutput::vPrintf(const char* pcFormat, va_list va)
{
    char * pcTmp = PR_vsmprintf(pcFormat, va);

    SKERR err = Write(pcTmp, PL_strlen(pcTmp));
    
    //CD: nspr prefers PR_DELETE for this (cf. pr/src/io/prstdio.c:60)
    //PR_smprintf_free(pcTmp);
    PR_DELETE(pcTmp);
    return err;
}

SKERR SKFileOutput::Flush()
{
    SK_ASSERT(NULL != m_pFD);
    if(m_pcBuffer && m_iDeadLength)
    {
        PRUint32 iLength;
        iLength = PR_Write(m_pFD, m_pcBuffer, m_iDeadLength);
        if(iLength != m_iDeadLength)
        {
            m_iDeadLength = 0;
            return err_failure;
        }
        m_iDeadLength = 0;
    }
    return noErr;
}

SKERR SKFileOutput::Close()
{
    if(m_pFD)
    {
        SKERR err = Flush();
        PR_Close(m_pFD);
        if(err != noErr)
            return err;
    }
    if(m_pcBuffer)
        PR_Free(m_pcBuffer);
    m_pFD = 0;
    return noErr;
}
