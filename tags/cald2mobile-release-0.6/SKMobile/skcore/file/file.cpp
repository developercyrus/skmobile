/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: file.cpp,v 1.8.2.3 2005/02/17 15:29:21 krys Exp $
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
#include "file.h"
#include "textfile.h"
#include "inifile.h"

SK_REFCOUNT_IMPL(SKFile)

//  --------------------------------------------------------------------------
//
//    SKFile
//
//  --------------------------------------------------------------------------
SKFile::SKFile()
{
    m_pszFileName = NULL;
}

SKFile::~SKFile()
{
    if(m_pszFileName)
        PL_strfree(m_pszFileName);
}

//  --------------------------------------------------------------------------
//
//    SetFileName
//
//  --------------------------------------------------------------------------

SKERR SKFile::SetFileName(const char *pszFileName,
                          const char *pszDefaultFileName)
{
    if (!pszFileName || !pszFileName[0])
        return err_invalid;

    if(m_pszFileName)
    {
        PR_Free(m_pszFileName);
        m_pszFileName = NULL;
    }

    // FIXME : this code will conflict with delayed and transient modes
    SKERR err = FindFileInEnvirPath(pszFileName, &m_pszFileName);
    if (err != noErr)
        return err_notfound;

    PRFileInfo info;
    PRStatus status = PR_GetFileInfo(m_pszFileName, &info);
    if(status != PR_SUCCESS)
        return err_failure;

    if(info.type == PR_FILE_DIRECTORY)
    {
        if(!pszDefaultFileName)
            return err_invalid;

        PRUint32 iLen1 = PL_strlen(m_pszFileName);
        PRUint32 iLen2 = PL_strlen(pszDefaultFileName);
        char *pszNew = (char *)PR_Malloc((iLen1 + iLen2 + 2) * sizeof(char));
        if(!pszNew)
            return err_memory;
        PL_strcpy(pszNew, m_pszFileName);
        pszNew[iLen1] = '/';
        PL_strcpy(pszNew + iLen1 + 1, pszDefaultFileName);
        PR_Free(m_pszFileName);
        m_pszFileName = pszNew;
    }

    return noErr;
}

void SKFile::PushEnvir()
{
    SKEnvir *pEnvir = NULL;
    SKEnvir::GetEnvir(&pEnvir);
    SK_ASSERT(pEnvir);
    pEnvir->Push();

    char * p = m_pszFileName + PL_strlen(m_pszFileName) - 1;
    while(p >= m_pszFileName && !IS_SEPARATOR(p))
        p--;
    if(p >= m_pszFileName)
    {
        char c1 = *p;
        char c2 = *(p + 1);
        *p = ';';
        *(p + 1) = '\0';
        pEnvir->PrependToValue("PATH", m_pszFileName);
        *p = c1;
        *(p + 1) = c2;
    }
}

void SKFile::PopEnvir()
{
    SKEnvir *pEnvir = NULL;
    SKEnvir::GetEnvir(&pEnvir);
    SK_ASSERT(pEnvir);
    pEnvir->Pop();
}

SKERR SKFile::GetFileName(const char*& psz)
{
    psz = PL_strdup(m_pszFileName);
    return (psz == NULL);
}

SKERR SKFile::ConfigureItem(char *pszSection, char *pszName, char *pszValue)
{
    // file path
    if(!PL_strcmp(pszName, "PATH"))
        return SetFileName(pszValue);

    // nobody knows about this configuration option
    return SKError(err_cnf_invalid, "[SKFile::Configure] "
                   "Invalid configuration option [%s] %s=%s",
                   pszSection, pszName, pszValue);
}

//  --------------------------------------------------------------------------
//
//    SetDescription
//
//  --------------------------------------------------------------------------
/*
void SKFile::SetDescription (const char* szDescription)
{
    strncpy (m_szDescription, szDescription, sizeof(m_szDescription) - 1);
    m_szDescription[sizeof(m_szDescription)-1] = 0;
}
*/
//  --------------------------------------------------------------------------
//
//    Check
//
//  --------------------------------------------------------------------------
SKERR SKFile::Check (void)
{
    return noErr;
}

