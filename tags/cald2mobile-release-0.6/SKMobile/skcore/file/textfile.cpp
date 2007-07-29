/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: textfile.cpp,v 1.11.2.2 2005/02/17 15:29:21 krys Exp $
 *
 * Authors: Alexis Seigneurin <seigneurin @at@ idm .dot. fr>
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

#include "skfopen.h"
#include "file.h"
#include "textfile.h"
#include "inifile.h"

#define DEFAULT_BUFFER_SIZE 4096

SKTextFile::SKTextFile() : m_bReady(PR_FALSE),
                           m_pFileDesc(NULL),
                           m_pszBuffer(NULL),
                           m_iBufferSize(0),
                           m_iFileSize(0),
                           m_pDelegate(NULL)
{
}

SKTextFile::~SKTextFile()
{
    if(m_pFileDesc)
        PR_Close(m_pFileDesc);
    if(m_pszBuffer)
        PR_Free(m_pszBuffer);
}

SKERR SKTextFile::SetFileName(const char *pszFileName,
                              const char *pszDefaultFileName)
{
    if(pszFileName)
    {
        SKERR err = SKFile::SetFileName(pszFileName, pszDefaultFileName);
        if(err != noErr)
            return err;

        m_pFileDesc = skPR_Open(m_pszFileName, PR_RDONLY, 0);
        if(!m_pFileDesc)
            return err_failure;

        PRFileInfo Info;
        if (PR_GetOpenFileInfo(m_pFileDesc, &Info)!= PR_SUCCESS)
            return err_failure;

        m_iFileSize = Info.size;
    }
    else
    {
        return err_failure;
    }

    return PostInitialize();
}

SKERR SKTextFile::SetFileDesc(PRFileDesc *pFd)
{
    m_pFileDesc = pFd;
    m_iFileSize = 0;

    return PostInitialize();
}

SKERR SKTextFile::PostInitialize()
{
    m_pszBuffer = (char *)PR_Malloc((DEFAULT_BUFFER_SIZE + 1) * sizeof(char));
    if(!m_pszBuffer)
        return err_memory;

    m_iBufferSize = DEFAULT_BUFFER_SIZE;
    m_iDeadLength = 0;
    m_iReadLength = 0;
    m_bReady = PR_TRUE;
    m_bFinished = PR_FALSE;

    return noErr;
}

PRUint32 SKTextFile::GetFilePosition ()
{
    PRInt32 iPos = PR_Seek (m_pFileDesc, 0, PR_SEEK_CUR);
    // PR_Seek returns -1 if the call failed

    return (PRUint32) iPos;
}

PRUint32 SKTextFile::GetFileSize()
{
    return m_iFileSize;
}

SKERR SKTextFile::GetLine(char **ppszLine)
{
    if(!m_bReady)
        return err_failure;
    if(!ppszLine)
        return err_invalid;

    // are we done ?
    if(m_bFinished && m_iDeadLength == m_iReadLength)
    {
        *ppszLine = 0;
        PR_Free(m_pszBuffer);
        m_pszBuffer = 0;
        m_iDeadLength = 0;
        m_iReadLength = 0;
        m_iBufferSize = 0;
        return noErr;
    }

    PRUint32 iStart = m_iDeadLength;
    PRUint32 iEnd = iStart;
    while(iEnd < m_iReadLength && m_pszBuffer[iEnd] != '\n')
        iEnd++;
    // do we have a full line ?
    if(iEnd < m_iReadLength || m_bFinished)
    {
        if(iEnd < m_iReadLength)
        {
            m_pszBuffer[iEnd] = '\0';
            if ((iEnd > iStart) && (m_pszBuffer[iEnd-1] == '\r'))
                m_pszBuffer[iEnd-1] = '\0';
        }
        else
        {
            // this is the last line
            SK_ASSERT(iEnd == m_iReadLength && m_bFinished);
            m_pszBuffer[iEnd--] = '\0';
        }
        m_iDeadLength = iEnd + 1;
        *ppszLine = m_pszBuffer + iStart;
        return noErr;
    }
    // recopy the still living part of the buffer at the beginning
    if(m_iDeadLength)
    {
        for(PRUint32 i = 0; i < m_iReadLength - m_iDeadLength; i++)
            m_pszBuffer[i] = m_pszBuffer[i+m_iDeadLength];
        m_iReadLength -= m_iDeadLength;
        m_iDeadLength = 0;
    }
    // can we read more stuff in our buffer ?
    if(m_iReadLength == m_iBufferSize)
    {
        m_iBufferSize = 2 * m_iBufferSize;
        m_pszBuffer = (char*) PR_Realloc(m_pszBuffer,
                                         (m_iBufferSize + 1) * sizeof(char));
    }
    // read more data
    PRInt32 iRead = PR_Read(m_pFileDesc,
            m_pszBuffer + m_iReadLength,
            m_iBufferSize - m_iReadLength);
    if(iRead < 0)
        return err_failure;

    if(iRead == 0)
        m_bFinished = PR_TRUE;
    m_iReadLength += iRead;
    return GetLine(ppszLine);
}

SKERR SKTextFile::ParseConfiguration()
{
    IniParser parser(this);
    SKERR err        = noErr;
    char* pszSection = NULL;
    char* pszName    = NULL;
    char* pszValue   = NULL;

    PushEnvir();

    PRUint32 line = 0;
    while (    (err == noErr)
            && parser.readIniLine(&pszSection, &pszName, &pszValue, &line))
    {
        err = ConfigureItem(pszSection, pszName, pszValue);
        if(err != noErr)
            SKError(err_failure, "[SKTextFile::ParseConfiguration] "
                    "Stops parsing. Error %d on line %d of %s : %s/%s=%s",
                    err, line, GetSharedFileName(),
                    pszSection, pszName, pszValue);
    }

    PopEnvir();

    return err;
}

SKERR SKTextFile::ConfigureItem(char *pszSection, char *pszName, char *pszValue)
{
    SKERR err = err_not_handled;
    if(!pszName)
        return noErr;
#ifdef DEBUG_CONFIG
    SKDebugLog("SKFile:: [%s] %s=%s", pszSection, pszName, pszValue);
#endif
    if(m_pDelegate)
        err = m_pDelegate->ConfigureItem(pszSection, pszName, pszValue);
    if(err != err_not_handled)
        return err;

    // nobody knows about this configuration option
    return SKError(err_cnf_invalid, "[SKTextFile::Configure] "
                   "Invalid configuration option [%s] %s=%s",
                   pszSection, pszName, pszValue);
}
