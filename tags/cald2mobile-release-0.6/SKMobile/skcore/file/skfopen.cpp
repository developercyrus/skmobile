/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: skfopen.cpp,v 1.5.4.3 2005/02/17 15:29:21 krys Exp $
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

#include "../skcoreconfig.h"

#include "../machine.h"
#include "../error.h"

#include "../refcount.h"
#include "../skptr.h"

#include "../envir/envir.h"

#include "skfopen.h"

SKAPI char* SKGetPath()
{
    SKEnvir *pEnvir = NULL;
    SKERR err = SKEnvir::GetEnvir(&pEnvir);
    if(err != noErr)
        return NULL;

    char* pszPath = NULL;
    err = pEnvir->GetValue("PATH", &pszPath);
    if(err != noErr)
         return NULL;

    return pszPath;
}



SKAPI SKERR FindFileInPath(const char* pszFile, const char* pszPath,
                           char** ppszResult)
{
    SK_ASSERT(pszPath);
    if(!pszPath)
        return err_invalid;
    SK_ASSERT(ppszResult);
    if(!ppszResult)
        return err_invalid;

    if(PR_Access(pszFile, PR_ACCESS_EXISTS) == PR_SUCCESS)
    {
        *ppszResult = PL_strdup(pszFile);
        return noErr;
    }

    *ppszResult = NULL;
    PRBool bFound = PR_FALSE;
    char * pcPath = PL_strdup(pszPath);
    char * pcToFree = pcPath;
    char * pcTmp = (char*) PR_Malloc(PL_strlen(pszFile) + PL_strlen(pszPath) + 2);
    while(pcPath && *pcPath && !bFound)
    {
        char * pcSeparator;
        // find next separator
        pcSeparator = PL_strchr(pcPath, ';');
        if(pcSeparator) 
            *pcSeparator = '\0';
        PL_strcpy(pcTmp, pcPath);
        
        // put pcEnd on the last character of path
        char * pcEnd = pcTmp + PL_strlen(pcTmp);
        
        // rewind for ../
        while(*pszFile=='.' && *(pszFile + 1)=='.' && IS_SEPARATOR(pszFile+2))
        {
            pszFile += 3;
            while(pcEnd > pcTmp && !IS_SEPARATOR(pcEnd-1))
                pcEnd--;
        }

        if(!IS_SEPARATOR(pcEnd-1))
            *pcEnd++ = '/';
        
        PL_strcpy(pcEnd, pszFile);

        // does the file exist ?
        if(PR_Access(pcTmp, PR_ACCESS_EXISTS) == PR_SUCCESS)
        {
            bFound = PR_TRUE;
            *ppszResult = pcTmp;
        }

        if(pcSeparator)
        {
            *pcSeparator = ';';
            pcPath = pcSeparator + 1;
        }
        else
            pcPath = NULL;
    }
    PL_strfree(pcToFree);
    if(!bFound)
    {
        PR_Free(pcTmp);
        pcPath = NULL;
        return err_notfound;
    }
    return noErr;
}

SKAPI SKERR FindFileInEnvirPath(const char* pszFile, char** ppszResult)
{
    SKEnvir *pEnvir = NULL;
    SKERR err = SKEnvir::GetEnvir(&pEnvir);
    if(err != noErr)
        return err;

	char* pszPath = NULL;
	// first try it in SK_HOME
	err = pEnvir->GetValue(SK_HOME_ENVIR_VAR, &pszPath);
	if(err != noErr)
		return err;
	err = FindFileInPath(pszFile, pszPath, ppszResult);
	PL_strfree(pszPath);
	if(err == noErr)
		return noErr;

	// then try it in PATH
	err = pEnvir->GetValue("PATH", &pszPath);
    if(err != noErr)
         return err;

    err = FindFileInPath(pszFile, pszPath, ppszResult);
    PL_strfree(pszPath);
    return err;
}

//----------------------------------------------------------------------------
// skfopen
//----------------------------------------------------------------------------

#if defined( WINDOWS )
PRFileDesc* skPR_Open(const char *name, PRIntn flags, PRIntn mode)
{
	PRFileDesc* fhResult = 0;
	char * pcTmpPath = 0;
	char c = 'c';
	// if we manage to open the file, we do not do anything more
	fhResult = PR_Open(name, flags, mode);
	if (fhResult) 
		return fhResult;

	pcTmpPath = PL_strdup(name);
	
	// look if the path looks absolute (begin with "?:\")
	if(pcTmpPath && pcTmpPath[0] && pcTmpPath[1] == ':' && pcTmpPath[2] == '\\')
	{
		// try all drives
		for (c = 'c' ; !fhResult && c <= 'z' ; c++)
		{
			pcTmpPath[0] = c;
			fhResult = PR_Open(pcTmpPath, flags, mode);
		}
	}

	if(pcTmpPath) 
		PL_strfree(pcTmpPath);
	return fhResult;
}
#endif

//----------------------------------------------------------------------------
// PR_fgets
//----------------------------------------------------------------------------

char* PR_fgets(char *buf, int size, PRFileDesc *file)
{
    int i = 0, status;
    char c;
    while( i < size - 1 )
    {
        status = PR_Read(file, (void*) &c, 1);
        if( status == -1 )
            return NULL;
        else if( status == 0)
            break;

        buf[i++] = c;

        if( c == '\n' )
            break;
    }

    if( !i )
        return NULL;

    buf[i]= '\0' ;

    return buf;
}

